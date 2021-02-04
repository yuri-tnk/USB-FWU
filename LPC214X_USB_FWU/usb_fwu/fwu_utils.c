/*
   USB Firmware Upgrader

Copyright © 2006 Yuri Tiomkin
All rights reserved.

Permission to use, copy, modify, and distribute this software in source
and binary forms and its documentation for any purpose and without fee
is hereby granted, provided that the above copyright notice appear
in all copies and that both that copyright notice and this permission
notice appear in supporting documentation.

THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL YURI TIOMKIN OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include "lpc214x.h"
#include "fwu_utils.h"
#include "fwu_usb_hw.h"
#include "fwu_usb.h"


//--- Checking .data segment only - you may omit it
volatile int chk_data_seg = 0x12345678;
//---------------------------------------------

extern unsigned int crc32_ref_table[];
extern volatile int fw_length;
extern volatile unsigned int fw_crc;
extern volatile int gState;


typedef void (*IAP)(unsigned int [],unsigned int[]);

//----------------------------------------------------------------------------
void * s_memset(void * dst,  int ch, int length)
{
  register char *ptr = (char *)dst - 1;

  while(length--)
     *++ptr = ch;

  return dst;
}

//----------------------------------------------------------------------------
void * s_memcpy(void * s1, const void *s2, int n)
{
   register int mask = (int)s1 | (int)s2 | n;

   if(n == 0)
      return s1;

   if(mask & 0x1)
   {
      register const char *src = s2;
      register char *dst = s1;

      do
      {
         *dst++ = *src++;
      }while(--n);

      return s1;
   }

   if(mask & 0x2)
   {
      register const short *src = s2;
      register       short *dst = s1;

      do
      {
        *dst++ = *src++;
      }while( n -= 2);

      return s1;
   }
   else
   {
      register const int *src = s2;
      register       int *dst = s1;

      do
      {
         *dst++ = *src++;
      }while (n -= 4);

      return s1;
   }
}

//----------------------------------------------------------------------------
unsigned int calc_crc(unsigned char * buf,int nbytes)
{
   unsigned int result;
   int i;

   result = 0xffffffff;
   for(i=0; i < nbytes; i++)
      result = (result >> 8) ^ crc32_ref_table[(result & 0xFF) ^ buf[i]];
   result ^= 0xffffffff;

   return result;
}

//----------------------------------------------------------------------------
void make_cmd(int cmd,unsigned char * buf)
{
   unsigned int rc;

   buf[0] = cmd & 0xFF;
   rc = calc_crc(buf,CTRL_BUF_CRC_OFF); //-- 60
   s_memcpy(&buf[CTRL_BUF_CRC_OFF],&rc, sizeof(int)); //-- Little Endian
}

//----------------------------------------------------------------------------
static int get_sector_number(unsigned int in_addr)
{
   static const SECTORGEOMETRY lpc2148_flash_sectors[] =
   {
      {4096,   8},   //-- 4K  * 8
      {32768, 14},   //-- 32K  *14
      {4096,   5},   //-- 4K  * 5
      {0,      0}    //-- End
   };

   SECTORGEOMETRY * psg;
   unsigned int end_addr;
   unsigned int start_addr;
   int n_sector;
   int i;

   start_addr = 0; //-- Flash start address
   n_sector = 0;
   psg = (SECTORGEOMETRY * )&lpc2148_flash_sectors[0];
   while(psg->qty != 0)
   {
      for(i = 0; i < psg->qty; i++)
      {
         end_addr = start_addr + psg->size;
         if(in_addr >= start_addr && in_addr < end_addr)
            return n_sector;
         n_sector++;
         start_addr = end_addr;
      }
      psg++;
   }
   return (-1); //-- If here - err
}

//----------------------------------------------------------------------------
int flash_write(unsigned int flash_addr,
                unsigned int ram_addr,
                int prc_clk, //-- in KHz
                int len)     //-- 256,512, etc up to min sector size(4096)
{
   FWU_INTSAVE_DATA

   int n_sector;
   unsigned int rc = ERR_NO_ERR;

   n_sector = get_sector_number(flash_addr);
   if(n_sector == (-1)) //-- Err
      return ERR_WRONG_PARAM;

   fwu_disable_interrupt();

   //--- prepare to write
   rc = iap_command(IAP_CMD_PREPARE_SECTORS,n_sector,n_sector,0,0,0);
   if(rc == IAP_CMD_SUCCESS)
   {
      for(;;)
      {
         rc = iap_command(IAP_CMD_COPY_RAM_TO_FLASH,flash_addr,ram_addr,len,prc_clk,0);
         if(rc != IAP_BUSY)
            break;
      }
   }
   if(rc == IAP_CMD_SUCCESS)
      rc = ERR_NO_ERR;
   else
      rc = ERR_WSTATE;

   fwu_enable_interrupt();

   return rc;
}

//----------------------------------------------------------------------------
int flash_erase_sectors(unsigned int start_addr,
                        int len,      //-- any
                        int prc_clk)  //-- in KHz
{
   FWU_INTSAVE_DATA

   int start_sector;
   int end_sector;
   unsigned int rc = ERR_NO_ERR;

   start_sector = get_sector_number(start_addr);
   end_sector   = get_sector_number(start_addr + len - 1);

   fwu_disable_interrupt();

   if(start_sector != -1 && end_sector != -1)
   {
      rc = iap_command(IAP_CMD_PREPARE_SECTORS, start_sector, end_sector, 0, 0, 0);
      if(rc == IAP_CMD_SUCCESS)
      {
         for(;;)
         {
            rc =  iap_command(IAP_CMD_ERASE_SECTORS, start_sector, end_sector, prc_clk, 0, 0);
            if(rc != IAP_BUSY)
               break;
         }
      }
   }

   fwu_enable_interrupt();

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
void do_switch_to_firmware(void)
{
   unsigned int * ptr;
   unsigned int crc;
     //--- Read FW length & crc; if ok then start firmware
   ptr =(unsigned int *)(FW_LOAD_ADDR + FW_ROM_LENGTH_OFFSET);
   fw_length = *ptr;  //-- Little endian
   ptr++;
   fw_crc    = *ptr;

   if(fw_length > 0  && fw_length <= FLASH_SIZE - FW_LOAD_ADDR)
   {
      crc = calc_crc((unsigned char *)(FW_LOAD_ADDR + FW_START_OFFSET),
                       fw_length - FW_START_OFFSET);
      if(crc == fw_crc)
         start_firmware(); //-- Never return
   }
}

//----------------------------------------------------------------------------
void switch_to_firmware(void)
{
   tn_arm_disable_interrupts();
   do_switch_to_firmware();
   tn_arm_enable_interrupts();
}

//----------------------------------------------------------------------------
void set_state(int state)
{
   FWU_INTSAVE_DATA

   fwu_disable_interrupt();
   gState = state;
   fwu_enable_interrupt();
}
 //----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------





























