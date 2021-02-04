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


extern USB_DEVICE_INFO  gUSBInfo;
extern EP_INFO  gEP2TX_EI;
extern EP_INFO  gEP2RX_EI;

typedef void (*int_func)(void);

//----------------------------------------------------------------------------
void cpu_irq_handler(void)
{
   int_func ifunc;
   ifunc = (int_func)VICVectAddr;
   if(ifunc != NULL)
      (*ifunc)();
}
//----------------------------------------------------------------------------
void tn_usb_int_func(void)
{
   unsigned int dev_status;
   unsigned int ep_int_status;

   if(rUSBDevIntSt & DEV_STAT) //-- USB Bus reset,Connect change,Suspend change
   {
      dev_status = tn_usb_lpc_cmd_read(CMD_DEV_STATUS);

      if(dev_status & RST)        //-- Bus reset
         tn_usb_int_reset(&gUSBInfo);
      if(dev_status & SUS_CH)     //-- Suspend/Resume toggle
      {
         if(dev_status & SUS)     //-- Suspend
            tn_usb_int_suspend();
         else
            tn_usb_int_resume();
      }
      if(dev_status & CON_CH)     //-- Connect changed
         tn_usb_int_con_ch(dev_status,&gUSBInfo); // & CON);

      rUSBDevIntClr = DEV_STAT;
   }

   if(rUSBDevIntSt & EP_SLOW)     //-- Endpoints
   {
      ep_int_status = rUSBEpIntSt;

      //-- EP0 - Control
      if(ep_int_status & 1)
         tn_usb_EP0_rx_int_func(&gUSBInfo); //-- OUT
      else if(ep_int_status & (1<<1))
         tn_usb_EP0_tx_int_func(&gUSBInfo); //-- IN

      //-- EP2 - Bulk
      if(ep_int_status & (1<<EP2_RX))
         tn_usb_ep_rx_int(&gEP2RX_EI); //-- OUT
      if(ep_int_status & (1<<EP2_TX))
         tn_usb_ep_tx_int(&gEP2TX_EI); //-- IN

      rUSBDevIntClr = EP_SLOW; // clear EP_SLOW
   }
   VICVectAddr  = 0;
}

//----------------------------------------------------------------------------
void tn_timer0_int_func(void)
{
#define BLINK_VAL 10
   static int cnt = BLINK_VAL;
   static int flag = 0;

   rT0IR = 0xFF;                //-- clear int source

   cnt--;
   if(cnt <=0)
   {
      cnt = BLINK_VAL;
      //-- if no configuration then  Blink LED
      if(gUSBInfo.Configuration == 0)
      {
         if(flag == 0)
         {
            rIO0SET |= (1<<14);
            flag = 1;
         }
         else
         {
            rIO0CLR |= (1<<14);
            flag = 0;
         }
      }
   }


   VICVectAddr = 0xFF;
}

//----------------------------------------------------------------------------
void tn_int_default_func(void)
{
   VICVectAddr = 0xFF;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------









