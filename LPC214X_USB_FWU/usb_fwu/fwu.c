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

#include "LPC214x.h"
#include "fwu_utils.h"
#include "fwu_usb_hw.h"
#include "fwu_usb.h"
#include "fwu_usb_ep.h"

//--- Globals

   //-- USB

USB_DEVICE_INFO  gUSBInfo;
BYTE             gEP0Buf[USB_MAX_PACKET0];

extern const BYTE abDescriptors[];

EP_INFO  gEP2TX_EI;
EP_INFO  gEP2RX_EI;

unsigned char gStreamRXBuf[STREAM_BUF_SIZE];


  //--- FW info
volatile unsigned int fw_product_id;
volatile unsigned int fw_version;
volatile int fw_length;
volatile unsigned int fw_crc;

volatile int gState;
volatile int gFullBlock;

  //--- CRC32 table -in RAM
unsigned int crc32_ref_table[256];


//-- for firmware debugging only ---------

//#define RUN_FW_ALWAYS 1
//-----------------------------------

#define  EP2RX_QUEUE_SIZE        8
#define  EP2TX_QUEUE_SIZE        8

#define  RX_TIMROUT          60000
#define  TX_TIMROUT          60000

FWU_DQ  queueEP2RX;
unsigned int queueEP2RXMem[(EP2RX_QUEUE_SIZE*DATA_ENTRY_SIZE)>>2];

FWU_DQ  queueEP2TX;
unsigned int queueEP2TXMem[(EP2TX_QUEUE_SIZE*DATA_ENTRY_SIZE)>>2];

//--- Prototypes
void send_to_host(int cmd);
void stream_rx(void);
void stream_tx(void);

extern volatile unsigned int flash_pcell;  //-- For IAR linker only
//----------------------------------------------------------------------------
int main()
{
   unsigned int * ptr;
   int state;

   rMEMMAP = 0x1;

   tn_arm_disable_interrupts();

   Init_CRC32_Table();

   state = flash_pcell;  //-- For IAR linker only
  //---- Set interrupts vectors
   ptr = (unsigned int *)IRQ_RAM_ADDR;
   *ptr = 0xE59FF018;                       //-- ldr pc, [pc, #24]
   ptr = (unsigned int *)FIQ_RAM_ADDR;
   *ptr = 0xE59FF018;                       //-- ldr pc, [pc, #24]

   //--- Put IRQ & FIQ vectors in RAM
   ptr = (unsigned int *)IRQ_RAM_FUNC_ADDR;
   *ptr = (unsigned int)&cpu_irq_isr;
   ptr = (unsigned int *)FIQ_RAM_FUNC_ADDR;
   *ptr = (unsigned int)&cpu_fiq_isr;

#ifndef RUN_FW_ALWAYS

   //--- Check 'Run as Loader' mark
   ptr =(unsigned int *)RAM_START_ADDR;
   if(!(*ptr == 0x12345678 && *(ptr+1) == 0x43211234))
   {
      do_switch_to_firmware();   //-- Never returns
   }
#else
   start_firmware(); //-- Never returns
#endif

 //------ Clear FW loader marks
   ptr =(unsigned int *)RAM_START_ADDR;
   *ptr = 0;
    ptr++;
   *ptr = 0;

   HardwareInit();

   //--- USB  data structs init ---

   gUSBInfo.Descriptors = (BYTE*)&abDescriptors[0];   //-- Descriptors
   gUSBInfo.EP0Status.pbuf   = &gEP0Buf[0];           //-- EP0 buffer
   tn_usb_reset_data(&gUSBInfo);

   gEP2RX_EI.queue       = &queueEP2RX;
   gEP2RX_EI.ep_num_phys = EP2_RX;

   gEP2TX_EI.queue       = &queueEP2TX;
   gEP2TX_EI.ep_num_phys = EP2_TX;

   fwu_queue_create(&queueEP2RX, EP2RX_QUEUE_SIZE,(unsigned char *) &queueEP2RXMem[0]);
   fwu_queue_create(&queueEP2TX, EP2TX_QUEUE_SIZE,(unsigned char *) &queueEP2TXMem[0]);


   tn_arm_enable_interrupts();

   tn_usb_connect(TRUE);  //-- Connect USB - here

   for(;;)
   {
      state = gState;
      if(state == PUT_FW_INFO)
      {
         gFullBlock = FALSE;
         flash_erase_sectors(FW_LOAD_ADDR,
                             fw_length,      //-- any
                             PRC_CLK);  //-- in KHz

         send_to_host(PUT_FW_INFO_ASK);
         set_state(STATE_DEFAULT);
         stream_rx();
      }
      else if(state == GET_FW)
      {
         gFullBlock = TRUE;
         set_state(STATE_DEFAULT);
         stream_tx();
      }
      else
      {
         if(state != STATE_DEFAULT)
            set_state(STATE_DEFAULT);
      }
   }

   return 1;
}

//----------------------------------------------------------------------------
void stream_rx(void)
{
   volatile int dly;
   unsigned char * ptr;
   int  buf_ind;
   int  nbytes;
   unsigned int crc;
   int  flash_addr;
   int Timeout = RX_TIMROUT;

   buf_ind = 0;
   nbytes  = 0;
   crc = 0xffffffff;
   flash_addr = FW_LOAD_ADDR;
   for(;;)
   {
      ptr = fwu_queue_get(&queueEP2RX);
      if(ptr != NULL)
      {
         Timeout = RX_TIMROUT;
         s_memcpy(&gStreamRXBuf[buf_ind],ptr,CTRL_BUF_SIZE);
         buf_ind += CTRL_BUF_SIZE;
         nbytes += CTRL_BUF_SIZE;

         if(buf_ind >= STREAM_BUF_SIZE)
         {
            buf_ind = 0;

            //--- Payload -------
            if(flash_addr == FW_LOAD_ADDR)
            {
               s_memcpy(&gStreamRXBuf[FW_ROM_PRODUCT_ID_OFFSET], (int*)&fw_product_id, sizeof(int));
               s_memcpy(&gStreamRXBuf[FW_ROM_VERSION_OFFSET],    (int*)&fw_version,    sizeof(int));
               s_memcpy(&gStreamRXBuf[FW_ROM_LENGTH_OFFSET],     (int*)&fw_length,     sizeof(int));
               s_memcpy(&gStreamRXBuf[FW_ROM_CRC_OFFSET],        (int*)&fw_crc,        sizeof(int));
            }
            flash_write(flash_addr,
                        (unsigned int)&gStreamRXBuf[0],
                        PRC_CLK, //-- in KHz
                        STREAM_BUF_SIZE);      //-- 256,512, etc up to min sector size(4096)

            flash_addr += STREAM_BUF_SIZE;

             //-------------------
            if(nbytes >= fw_length) //-- End
            {
              //--- Calc crc for last piece(may be any size) -----
               crc = calc_crc((unsigned char *)(FW_LOAD_ADDR +
                                           FW_START_OFFSET),fw_length - FW_START_OFFSET);
              //--------------------------------------------------------
               if(crc == fw_crc)
               {
                  send_to_host(FW_WR_RDY);
                  for(dly=0;dly<10000;dly++); //--10000
                  switch_to_firmware();
               }
               else
               {
                  send_to_host(FW_WR_ERR);
               }
               break;
            }
         }
      }
      else //-- ptr == NULL
      {
         Timeout--;
         if(Timeout <= 0)
         {
            set_state(STATE_DEFAULT);
            break;
         }
      }
   }//-- for(;;)
}

//----------------------------------------------------------------------------
void  send_to_host(int cmd)
{
   unsigned char buf[64];
   unsigned char * ptr;

   //--- Prepare cmd
   s_memset(buf,0,64);
   make_cmd(cmd,buf);
   //--- Tx --
   ptr = fwu_queue_tst(&queueEP2TX);
   if(ptr != NULL)
   {
      s_memcpy(ptr,buf,64);
      fwu_queue_put(&queueEP2TX);
   }
}
//----------------------------------------------------------------------------
void stream_tx(void)
{
   unsigned char * ptr;
   unsigned char * flash_ptr;
   unsigned char * last_pos;
   volatile int dly;
   int chkbreak_cnt = 0;
   int fEnd = 0;
   int nbytes = 0;
   int Timeout = TX_TIMROUT;

   flash_ptr = (unsigned char *) FW_LOAD_ADDR;
   if(fw_length <=0 || fw_length > FLASH_SIZE - FW_LOAD_ADDR)
      last_pos  = flash_ptr;
   else
      last_pos  = flash_ptr + fw_length;

   for(;;)
   {
    //--- Payload -----------------------------------------
      ptr = fwu_queue_tst(&queueEP2TX);
      if(ptr != NULL)
      {
         Timeout = TX_TIMROUT;
         s_memcpy(ptr,flash_ptr,64);
         flash_ptr += 64;
         nbytes += 64;
        //-----------------------------
         fwu_queue_put(&queueEP2TX);
      }
      else
      {
         Timeout--;
         if(Timeout <= 0)
         {
            set_state(STATE_DEFAULT);
            break;
         }
      }
      if(flash_ptr >= last_pos)
         fEnd = TRUE;
    //-----------------------------------------------------
      if(nbytes % STREAM_BUF_SIZE == 0)
      {
         if(gFullBlock == TRUE && fEnd == TRUE)
         {
            for(dly=0;dly<20000;dly++);
            switch_to_firmware();
            //-- Here only if there are no firmware
            set_state(STATE_DEFAULT);
            break;
         }
      }
      chkbreak_cnt++;
      if(chkbreak_cnt >= NOW_CHK_BREAK)
      {
         chkbreak_cnt = 0;
         if(fEnd == TRUE && gFullBlock == FALSE)
            break;
      }
   }
  //-------------
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

