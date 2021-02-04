/*
TNKernel USB Firmware Upgrader Demo

Copyright © 2006,2008 Yuri Tiomkin
All rights reserved.

Permission to use, copy, modify, and distribute this software in source
and binary forms and its documentation for any purpose and without fee
is hereby granted, provided that the above copyright notice appear
in all copies and that both that copyright notice and this permission
notice appear in supporting documentation.

THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS "AS IS" AND
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

#include "stdafx.h"
#include "fwu_utils.h"

#include "fwu_types.h"
#include "fwu_globals.h"

#include "fwu_cmd.h"

#pragma warning(disable: 4996)

//----------------------------------------------------------------------------
void cmd_to_app(int cmd)
{
   USBIO_CLASS_OR_VENDOR_REQUEST  Request;
   DWORD ByteCount;

   ByteCount = 0; //-- No data phase
   Request.Type = RequestTypeVendor;
   Request.Recipient = RecipientDevice;
   Request.Request = APP_CMD;
   Request.Value = cmd;

   g_dt.UsbIo.ClassOrVendorOutRequest(NULL, //-- No data phase
                                  ByteCount,
                                  &Request);
}

//----------------------------------------------------------------------------
BOOL cmd_GET_FW_INFO(void)
{
   USBIO_CLASS_OR_VENDOR_REQUEST  Request;
   DWORD ByteCount;
   unsigned char mbuf[64];
   DWORD rc,tmp;

   g_dt.FW.IsValid = FALSE;

   memset(mbuf, 0, 64);
   ByteCount = 64;
   Request.Type = RequestTypeVendor;
   Request.Recipient = RecipientDevice;
   Request.Request   = GET_FW_INFO;

   g_dt.UsbIo.ClassOrVendorInRequest(mbuf, //-- No data phase
                                  ByteCount,
                                  &Request);

   rc = calc_crc(&mbuf[0],CTRL_BUF_CRC_OFF); //-- 60
   tmp =  *((DWORD*)(&mbuf[CTRL_BUF_CRC_OFF]));
   if(rc != tmp) //-- crc not o.k.
      return FALSE;

 //-- Store fw_crc,fw_product_id,fw_version,fw_length ----

   EnterCriticalSection(&g_dt.CrSecFWAccess);

      //-- Little endian
   memcpy(&g_dt.FW.fw_product_id, &mbuf[FW_PRODUCT_ID_OFFSET], sizeof(__int32));
   memcpy(&g_dt.FW.fw_version,    &mbuf[FW_VERSION_OFFSET],    sizeof(__int32));
   memcpy(&g_dt.FW.fw_length,     &mbuf[FW_LENGTH_OFFSET],     sizeof(__int32));
   memcpy(&g_dt.FW.fw_crc,        &mbuf[FW_CRC_OFFSET],        sizeof(__int32));

   g_dt.FW.buf_ptr = g_dt.FWBuf;
   g_dt.FW.len     = g_dt.FW.fw_length;
   g_dt.FW.IsValid = TRUE;

   if(g_dt.hRxFile != NULL)
      rewind(g_dt.hRxFile);

   LeaveCriticalSection(&g_dt.CrSecFWAccess);

   return TRUE;
}

//----------------------------------------------------------------------------
void cmd_PUT_FW_INFO(FW_INFO * pfw)
{

   USBIO_CLASS_OR_VENDOR_REQUEST  Request;
   DWORD ByteCount;
   DWORD rc;
   unsigned char buf[64];

   memset(buf,0,CTRL_BUF_SIZE);

     //-- Little endian
   memcpy(&buf[FW_PRODUCT_ID_OFFSET],&pfw->fw_product_id,sizeof(__int32));
   memcpy(&buf[FW_VERSION_OFFSET],   &pfw->fw_version,   sizeof(__int32));
   memcpy(&buf[FW_LENGTH_OFFSET],    &pfw->fw_length,    sizeof(__int32));
   memcpy(&buf[FW_CRC_OFFSET],       &pfw->fw_crc,       sizeof(__int32));

   buf[0] = PUT_FW_INFO;
   rc = calc_crc(&buf[0],CTRL_BUF_CRC_OFF); //-- 60
   memcpy(&buf[CTRL_BUF_CRC_OFF],&rc, sizeof(__int32));


   ByteCount = 64;
   Request.Type = RequestTypeVendor;
   Request.Recipient = RecipientDevice;
   Request.Request = PUT_FW_INFO;

   g_dt.UsbIo.ClassOrVendorOutRequest(&buf[0], //-- No data phase
                                  ByteCount,
                                  &Request);
}

//----------------------------------------------------------------------------
void cmd_no_data(int cmd)
{
   USBIO_CLASS_OR_VENDOR_REQUEST  Request;
   DWORD ByteCount;

   ByteCount = 0; //-- No data phase
   Request.Type = RequestTypeVendor;
   Request.Recipient = RecipientDevice;
   Request.Request = cmd;

   g_dt.UsbIo.ClassOrVendorOutRequest(NULL, //-- No data phase
                                  ByteCount,
                                  &Request);
}




