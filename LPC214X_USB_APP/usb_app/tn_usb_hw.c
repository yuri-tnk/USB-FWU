/*
TNKernel real-time kernel - USB examples  (no DMA)

Copyright � 2004,2006 Yuri Tiomkin
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

#include "tn_usb_hw.h"
#include "tn_usb.h"
#include "lpc214x.h"
#include "utils.h"
#include "../../tnkernel/tn.h"


  //--- WITHOUT  DMA


//  In all functions with input parameter 'ep_num_logical'
//  this parameter has format:  bits[3:0]-ep num logical, bit[7]-direction

extern volatile int  gUserAbort;
extern TN_SEM semEP2TX;

//----------------------------------------------------------------------------
// Do not use inside interrupt !!!
//----------------------------------------------------------------------------
int tn_usb_connect(int mode)
{
   TN_INTSAVE_DATA
   int cmd;
   int data;

   TN_CHECK_NON_INT_CONTEXT

   cmd = CMD_DEV_STATUS;
   if(mode)
      data = CON; //-- Connect
   else
      data = 0;  //-- Disconnect

   tn_disable_interrupt();  //-- os_disable_int
   tn_usb_lpc_cmd_write(cmd,data);
   tn_enable_interrupt();  //-- os_enable_int

   return TERR_NO_ERR;


//   return 0;
}

//----------------------------------------------------------------------------
static void ep_bulk_release(int ep_num_phys,int packet_size)
{
   rUSBReEp |= (1 << ep_num_phys);
   rUSBEpInd = ep_num_phys;
   rUSBMaxPSize = packet_size;
   while(!(rUSBDevIntSt & EP_RLZED));
   rUSBDevIntClr = EP_RLZED;
}

//----------------------------------------------------------------------------
void tn_usb_config_EP0(void)
{
   ep_bulk_release(0,USB_MAX_PACKET0);  //--- Configure EP0 Rx
   ep_bulk_release(1,USB_MAX_PACKET0);  //--- Configure EP0 Tx
   //-- EP int
   rUSBEpIntEn |= 3;    //-- EP0Rx & EP0Tx
}

//----------------------------------------------------------------------------
//  Reset interrupt  processing
//----------------------------------------------------------------------------
void tn_usb_int_reset(USB_DEVICE_INFO *udi)
{
   //-- Endpoints  int
   rUSBEpIntClr = 0xFFFFFFFF;

   tn_usb_config_EP0();

     //-- USB device int
   rUSBDevIntClr = 0xFFFFFFFF;
   rUSBDevIntEn |= (1<<3) | (1<<2); //-- DEV_STAT,EP_SLOW

 //-- Re-init data
   tn_usb_reset_data(udi);
}

//----------------------------------------------------------------------------
void tn_usb_int_con_ch(unsigned int dev_status,USB_DEVICE_INFO *udi)
{
   tn_usb_int_reset(udi);
}

//----------------------------------------------------------------------------
void tn_usb_int_suspend(void)
{
  //-- LPC214x- do nothing here
}

//----------------------------------------------------------------------------
void tn_usb_int_resume(void)
{
  //-- LPC214x- do nothing here
}

//----------------------------------------------------------------------------
void tn_usb_lpc_cmd(int cmd)
{
   rUSBDevIntClr = CDFULL | CCEMTY;            // clear CDFULL/CCEMTY
   rUSBCmdCode = 0x00000500 | (cmd << 16);     // write command code
   while(!(rUSBDevIntSt & CCEMTY));
   rUSBDevIntClr = CCEMTY;
}

//----------------------------------------------------------------------------
void tn_usb_lpc_cmd_write(int cmd, int data)
{
   rUSBDevIntClr = CDFULL | CCEMTY;            // clear CDFULL/CCEMTY
   rUSBCmdCode = 0x00000500 | (cmd << 16);    // write command code
   while(!(rUSBDevIntSt & CCEMTY));
   rUSBDevIntClr = CCEMTY;
   rUSBCmdCode = 0x00000100 | (data << 16);  // write command data
   while(!(rUSBDevIntSt & CCEMTY));
   rUSBDevIntClr = CCEMTY;
}

//----------------------------------------------------------------------------
int tn_usb_lpc_cmd_read(int cmd)
{
   int ret_val;

   rUSBDevIntClr = CDFULL | CCEMTY;            // clear CDFULL/CCEMTY
   rUSBCmdCode = 0x00000500 | (cmd << 16);     // write command code
   while(!(rUSBDevIntSt & CCEMTY));
   rUSBDevIntClr = CCEMTY;
   rUSBCmdCode = 0x00000200 | (cmd << 16); // get data
   while(!(rUSBDevIntSt & CDFULL));
   ret_val = rUSBCmdData;
   rUSBDevIntClr = CDFULL;

   return ret_val;
}
//----------------------------------------------------------------------------
// 'buf' must be word-aligned
//----------------------------------------------------------------------------
int tn_usb_ep_read(int ep_num_logical,BYTE * buf)
{
   unsigned int len;
   unsigned int * ptr;

   ep_num_logical &= 0x8F;
   rUSBCtrl = RD_EN | (ep_num_logical << 2);  //-- set read enable bit

   do
   {
      len = rUSBRxPLen;
   }while((len & PKT_RDY) == 0);

   len &= PKT_LNGTH_MASK;  // get length

   if(buf == NULL)
   {
      while(rUSBCtrl & RD_EN) //-- Just to empty endpoint rx buf
         ptr = (unsigned int *)rUSBRxData;
   }
   else
   {
      ptr = (unsigned int *) buf;
      while(rUSBCtrl & RD_EN)
         *ptr++ = rUSBRxData;  //-- get data in 4-byte units
   }
   rUSBCtrl = 0;

   tn_usb_lpc_cmd(CMD_EP_SELECT | ep_num_lg2ph(ep_num_logical));
   tn_usb_lpc_cmd(CMD_EP_CLEAR_BUFFER);

   return len;
}

//----------------------------------------------------------------------------
int tn_usb_ep0_write(int ep_num_logical,BYTE * buf,int nbytes)
{
   union
   {
      BYTE b[4];
      unsigned int w;
   }data;

   ep_num_logical &= 0x8F;
   rUSBCtrl = WR_EN | ((ep_num_logical & 0x0F)<< 2);
   rUSBTxPLen = nbytes;

   if(nbytes > 0)
   {
      while(rUSBCtrl & WR_EN)  //-- write data
      {
         data.b[0] = *buf++;
         data.b[1] = *buf++;
         data.b[2] = *buf++;
         data.b[3] = *buf++;
         rUSBTxData = data.w;
      }
   }

   rUSBCtrl = 0;

   tn_usb_lpc_cmd(CMD_EP_SELECT | ep_num_lg2ph(ep_num_logical));
   tn_usb_lpc_cmd(CMD_EP_VALIDATE_BUFFER);

   return nbytes;
}

//----------------------------------------------------------------------------
void tn_usb_stall_ep(int ep_num_logical,int mode)
{
   int cmd;
   int data;

   cmd = CMD_EP_SET_STATUS | ep_num_lg2ph(ep_num_logical & 0x8F);
   if(mode)
      data = EP_ST; //-- Stall
   else
      data = 0;     //-- Clear

   tn_usb_lpc_cmd_write(cmd,data);

}

//----------------------------------------------------------------------------
void tn_usb_set_addr(int addr)
{
   //-- Set address and enable device
   tn_usb_lpc_cmd_write(CMD_DEV_SET_ADDRESS, 0x80 | (addr & 0xFF)); //- 0x80 - DEV_EN
   //-- Enable IN int for NAK
   tn_usb_lpc_cmd_write(CMD_DEV_SET_MODE,(1<<5)|(1<<6));
}

//----------------------------------------------------------------------------
void tn_usb_reset_ep(int ep_num_logical)
{
   int cmd;

   cmd = CMD_EP_SET_STATUS | ep_num_lg2ph(ep_num_logical & 0x8F);

   tn_usb_lpc_cmd_write(cmd,0);

}

//----------------------------------------------------------------------------
void tn_usb_suspend(void)
{
}

//----------------------------------------------------------------------------
void tn_usb_resume(void)
{
}

//----------------------------------------------------------------------------
void tn_usb_wakeup_config(int mode)
{
}

//----------------------------------------------------------------------------
void tn_usb_configure(int mode)
{
   int data;
   unsigned int mask = 1;

   rUSBReEp = rUSBEpIntEn;         //-- Get released endpoints by copy from ep_en_int

   for(data = 0; data < 32; data++)
   {
      if(rUSBEpIntEn & mask)
         tn_usb_lpc_cmd_write(CMD_EP_SET_STATUS | data,  0); //-- O - enable
      mask <<=1;
   }

   if(mode)
      data = CONF_DEVICE;
   else
      data = 0;

   tn_usb_lpc_cmd_write(CMD_DEV_CONFIG, data);

}

//----------------------------------------------------------------------------
void tn_usb_configure_device(int mode)
{
   int data;

   if(mode)
      data = CONF_DEVICE;
   else
      data = 0;

   tn_usb_lpc_cmd_write(CMD_DEV_CONFIG, data);
}

//----------------------------------------------------------------------------
// 'ptr' should be set at USB_ENDPOINT_DESCRIPTOR structure start
//----------------------------------------------------------------------------
void tn_usb_config_ep(BYTE * ptr)
{
   int ep_num_logical;
   int ep_num_phys;

   ep_num_logical = *(ptr + USB_IND_bEndpointAddress);
   ep_num_phys = ep_num_lg2ph(ep_num_logical & 0x8F);

//-- Release Endpoint
   rUSBReEp |= (1 << ep_num_phys);
   rUSBEpInd = ep_num_phys;
   rUSBMaxPSize = *((WORD*)(ptr + USB_IND_wMaxPacketSize));
   while(!(rUSBDevIntSt & EP_RLZED));
   rUSBDevIntClr = EP_RLZED;

//-- Enable/Reset EP

   //-- Two times
   tn_usb_lpc_cmd_write(CMD_EP_SET_STATUS | ep_num_phys ,0);
   tn_usb_lpc_cmd_write(CMD_EP_SET_STATUS | ep_num_phys ,0);

//-- Enable Ep Int

   rUSBEpIntEn |= (1 << ep_num_phys);
}

//----------------------------------------------------------------------------
BOOL tn_usb_vendor_request_out(USB_DEVICE_INFO * udi)
{
   unsigned int * ptr;
   int rc = FALSE;
   USB_SETUP_PACKET  * usp = &udi->EP0SetupPacket;
   //USB_EP_STATUS * eps = &udi->EP0Status;

   if(usp->bRequest == SWITCH_TO_FWU)
   {
      //--- Set mark for FW loader
       ptr =(unsigned int *)RAM_START_ADDR;
      *ptr = 0x12345678;
       ptr++;
      *ptr = 0x43211234;

     //--- Switch to loader by WDT Reset ---------
     //--- WDT Reset gives safe hardware initialization
      rWDTC  = 0xFF;
      rWDMOD = 3;
      rWDFEED = 0xAA;
      rWDFEED = 0x55;
     //--------------------------
      rc = TRUE;
   }
   else if(usp->bRequest == USER_ABORT)
   {
      gUserAbort = TRUE;
      rc = TRUE;
   }
   if(usp->bRequest == APP_CMD)
   {
      //-- CMD
      if(usp->wValue == SEND_DEMO_DATA)
      {
         tn_sem_isignal(&semEP2TX);         //-- Enable Tx for EP2TX
      }
      rc = TRUE;
   }

   return rc;
}

//----------------------------------------------------------------------------
BOOL tn_usb_vendor_request_in(USB_DEVICE_INFO * udi)
{
   int rc = FALSE;
   unsigned int prg_info;
   USB_SETUP_PACKET  * usp = &udi->EP0SetupPacket;
   USB_EP_STATUS * eps = &udi->EP0Status;

   if(usp->bRequest == GET_ISFWU)
   {
      prg_info = 0;  //-- program is application
      s_memcpy(&eps->pdata[0],&prg_info,4);
      eps->nbytes = 8;
      rc = TRUE;
   }

   return rc;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

