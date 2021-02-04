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

//----------------------------------------------------------------------------
// This file  is compiled in THUMB mode for GCC compilers
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//    VALID ONLY FOR 'LPC2146 USB FIRMWARE UPGRADER' PROJECT
//----------------------------------------------------------------------------

/*-------- Project specific pinout description ------

                                        I/O dir set            I/O dir
                                       -------------          ---------

 P0.0  - TxD0          U-TXD-2             auto                  out
 P0.1  - RxD0          U-RXD0-2            auto                  in
 P0.2 -  GPIO          Not used            manual                Out
 P0.3  - GPIO          Not used            manual                out
 P0.4  - GPIO          Not used            manual                out
 P0.5  - GPIO          Not used            manual                out
 P0.6  - GPIO          Not used            manual                out
 P0.7  - GPIO          Not used            manual                out
 P0.8  - GPIO          Not used            manual                out
 P0.9  - GPIO          Not used            manual                out
 P0.10 - GPIO          Not used            manual                out
 P0.11 - GPIO          Not used            manual                out
 P0.12 - GPIO          Not used            manual                out
 P0.13 - GPIO          Not used            manual                out
 P0.14 - sys input     LED                 manual                out
 P0.15 - GPIO          Not used            manual                out
 P0.16 - GPIO          Not used            manual                out
 P0.17 - GPIO          Not used            manual                out
 P0.18 - GPIO          Not used            manual                out
 P0.19 - GPIO          Not used            manual                out
 P0.20 - GPIO          Not used            manual                out
 P0.21 - GPIO          Not used            manual                out
 P0.22 - GPIO          Not used            manual                out
 P0.23 - VBUS          VBUS                auto                  in
 P0.24 - not exists    ???
 P0.25 - GPIO          Not used            manual                out
 P0.26 - GPIO          Not used            manual                out
 P0.27 - GPIO          AUX2                manual                out
 P0.28 - GPIO          AUX4                manual                out
 P0.29 - GPIO          AUX5                manual                out
 P0.30 - GPIO          Not used            manual                out
 P0.31 - CONNECT       CONNECT             auto                  out

 P1.16 - GPIO          Not used            manual                out
 P1.17 - GPIO          AUX3                manual                out
 P1.18 - GPIO          Not used            manual                out
 P1.19 - GPIO          Not used            manual                out
 P1.20 - GPIO          Not used            auto                  in
 P1.21 - GPIO          Not used            manual                out
 P1.22 - GPIO          Not used            manual                out
 P1.23 - GPIO          Not used            manual                out
 P1.24 - GPIO          Not used            manual                out
 P1.25 - GPIO          Not used            manual                out
 P1.26 - GPIO          Not used            auto                  in
 P1.27 - TDO
 P1.28 - TDI
 P1.29 - TCK
 P1.30 - TMS
 P1.31 - TRST
----------------------------------------------------------------------------*/

static void InitUSB(void);  //-- external

extern unsigned int crc32_ref_table[];

//----------------------------------------------------------------------------
void  HardwareInit(void)
{
    int i;

  //-- Crystal 12 MHz

  //---  Peripheral - on max speed  bits 1..0 = 01 -> prc;
    rVPBDIV = 0x00000001;

  //--- PLL(Fout = 60MHz) - multiplay at 5 (F = Fosc * MSEL * 2 * P / 2*P)

    rPLL0CFG = (1<<5) | 0x4;  //-- MSEL = 5 [4:0] = 00100;   P=2 ([6:5] = 01)
    rPLL0CON = 0x03;          //-- PLLE = 1; PLLC = 1

    for(i=0;i<100;i++);
    rPLL0FEED = 0xAA;
    rPLL0FEED = 0x55;
    for(i=0;i<100;i++);

   //--- Disable all int ------

    VICIntEnable = 0;

    VICDefVectAddr = (unsigned int)tn_int_default_func; //-- default int processing

  //--- Timer 0  - interrupt 10 ms

    rT0PR = 0;  //-- Prscaler = 0
    rT0PC = 0;

    rT0MR0 = 60000 * 10;
    rT0MCR = 3; //-- bit 0=1 -int on MR0 , bit 1=1 - Reset on MR0

    rT0TCR = 1; //-- Timer 0 - run

    //--  int for Timer 0
    VICVectAddr1 = (unsigned int)tn_timer0_int_func;   //-- Timer 0 int - priority top-1
    VICVectCntl1 = 0x20 | 4;
    VICIntEnable = (1<<4);

  //---- USB ----

   InitUSB();

   //-- int for USB
   VICVectAddr0 = (unsigned int)tn_usb_int_func;   //-- USB int - top priority
   VICVectCntl0 = 0x20 | 22;
   VICIntEnable = (1 << 22);

 //--- Pins - to output & set to initial values ------------------------------

//   rIO0DIR |= 1<<2;  //-- output
//   rIO0SET |= 1<<2;  //-- Set P0.2 to 1

//-- P0.2 - P0.22  - GPIO/out

#define MASK_PINS_P0_2_P0_22   0x007FFFFC

   rIO0DIR |= MASK_PINS_P0_2_P0_22;  //-- output
   rIO0SET |= MASK_PINS_P0_2_P0_22;  //-- Set  to 1

//-- P0.23 -  VBUS                auto                in
//-- P0.25 - P0.30 - GPIO/out

#define MASK_PINS_P0_25_P0_30  0x7E000000

   rIO0DIR |= MASK_PINS_P0_25_P0_30;  //-- output
   rIO0SET |= MASK_PINS_P0_25_P0_30;  //-- Set  to 1

//-- P1.16 - P1.19 - GPIO/out

#define MASK_PINS_P1_16_P1_19  0x000F0000

   rIO0DIR |= MASK_PINS_P1_16_P1_19;  //-- output
   rIO0SET |= MASK_PINS_P1_16_P1_19;  //-- Set  to 1

//-- P1.17 - AUX3 set to 0

   rIO0CLR |= (1<<17);

//-- P1.21 - P1.25 - GPIO/out

#define MASK_PINS_P1_21_P1_25  0x03E00000

   rIO0DIR |= MASK_PINS_P1_21_P1_25;  //-- output
   rIO0SET |= MASK_PINS_P1_21_P1_25;  //-- Set  to 1
//-----------------------------
}

//----------------------------------------------------------------------------
static void InitUSB(void)  //-- Hardware init
{
   int i;

  //-- VBUS & SoftConnect pins
   rPINSEL1 |= 1<<14;      //-- P0.23 -> VBUS
   rPINSEL1 |= 2u << 30;   //-- P0.31 -> CONNECT

  //--- USB PLL -PLL1(Fout = 48MHz) - multiplay at 4 (F = Fosc * MSEL * 2 * P / 2*P)

   rPLL1CFG = 0x23;  //-- b00100011 bits 4..0 - MSEL4:0 = 4(3+1); bits 6:5 - PSEL 1:0 = 2(1+1);
   rPLL1CON = 0x03;  //-- PLLE = 1; PLLC = 1

   for(i=0;i<100;i++);
   rPLL1FEED = 0xAA;
   rPLL1FEED = 0x55;
   while(!(rPLL1STAT & (1<<10))); // Wait for PLL Lock

  //-- Enable USB hardware

   rPCONP |= 1u << 31;

  //-- Cmd- disconnect

   tn_usb_lpc_cmd_write(CMD_DEV_STATUS,0);

     //-- Endpoints  int
   rUSBEpIntClr = 0xFFFFFFFF;

   tn_usb_config_EP0();

     //-- USB device int
   rUSBDevIntClr = 0xFFFFFFFF;
   rUSBDevIntEn |= (1<<3) | (1<<2); //-- DEV_STAT,EP_SLOW

   tn_usb_set_addr(0);
}

//---------------------------------------------------------------------------
static unsigned int Reflect(unsigned int ref, char ch)
{
// Used only by Init_CRC32_Table().
   int i;
   unsigned int value = 0;

      // Swap bit 0 for bit 7
      // bit 1 for bit 6, etc.
   for(i = 1; i < (ch + 1); i++)
   {
      if(ref & 1)
         value |= 1 << (ch - i);
      ref >>= 1;
   }
   return value;
}

//---------------------------------------------------------------------------
void Init_CRC32_Table(void)
{
      // This is the official polynomial used by CRC-32
      // in PKZip, WinZip and Ethernet.
   unsigned int ulPolynomial = 0x04c11db7;
   int i,j;
      // 256 values representing ASCII character codes.
   for(i = 0; i <= 0xFF; i++)
   {
       crc32_ref_table[i] = Reflect(i, 8) << 24;
       for(j = 0; j < 8; j++)
          crc32_ref_table[i] = (crc32_ref_table[i] << 1) ^ (crc32_ref_table[i] & 0x80000000 ? ulPolynomial : 0);
       crc32_ref_table[i] = Reflect(crc32_ref_table[i], 32);
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



