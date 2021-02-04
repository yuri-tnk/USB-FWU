/*


Copyright © 2004,2006 Yuri Tiomkin
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

//  In all functions with input parameter 'ep_num_logical'
//  this parameter has format:  bits[3:0]-ep num logical, bit[7]-direction

  //--- FW info
extern volatile unsigned int fw_product_id;
extern volatile unsigned int fw_version;
extern volatile int fw_length;
extern volatile unsigned int fw_crc;

extern volatile int gState;

//----------------------------------------------------------------------------
// Do not use inside interrupt !!!
//----------------------------------------------------------------------------
int tn_usb_connect(int mode)
{
   FWU_INTSAVE_DATA
   int cmd;
   int data;

   cmd = CMD_DEV_STATUS;
   if(mode)
      data = CON; //-- Connect
   else
      data = 0;  //-- Disconnect

   fwu_disable_interrupt();  //-- os_disable_int
   tn_usb_lpc_cmd_write(cmd,data);
   fwu_enable_interrupt();  //-- os_enable_int

   return ERR_NO_ERR;
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
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

