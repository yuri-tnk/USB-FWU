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
#include "usbiolib/src/usbio.h"
#include "usbiolib/src/UsbIoReader.h"
#include "RxPipe.h"
#include "fwu_utils.h"

#include "fwu_types.h"
#include "fwu_globals.h"

#pragma warning(disable: 4996)

//----------------------------------------------------------------------------
CRxPipe::CRxPipe()
{
}

//----------------------------------------------------------------------------
CRxPipe::~CRxPipe()
{
}

//----------------------------------------------------------------------------
void CRxPipe::MakeRxCmd(int cmd,unsigned char * buf)
{
   DWORD rc;

   memset(buf,0,CTRL_BUF_SIZE);
   buf[0] = cmd & 0xFF;
   rc = calc_crc(&buf[0],CTRL_BUF_CRC_OFF); //-- 60
   memcpy(&buf[CTRL_BUF_CRC_OFF],&rc, sizeof(__int32));
}

//----------------------------------------------------------------------------
void CRxPipe::BufErrorHandler(CUsbIoBuf *Buf)
{
}

//----------------------------------------------------------------------------
// overloaded process buffer function
void CRxPipe::ProcessBuffer(CUsbIoBuf *Buf)
{
   FillMemory(Buf->Buffer(),Buf->Size(),0x00);
   Buf->NumberOfBytesToTransfer=Buf->Size();
   Buf->BytesTransferred=0;
   Buf->OperationFinished = false;
}

//----------------------------------------------------------------------------
// overloaded process data function
void CRxPipe::ProcessData(CUsbIoBuf *Buf)
{
   BOOL fEnd = FALSE;
   int nbytes;

   if(Buf->Status == USBIO_ERR_SUCCESS)
   {
      if(Buf->BytesTransferred) //Not a 0 packet
      {
         if(g_dt.OpMode == OPMODE_PUT_FW)
         {
            ::EnterCriticalSection(&g_dt.CrSecRxDataExch);
            memcpy(g_dt.CtrlRxExch, Buf->Buffer(),CTRL_BUF_SIZE);
            ::LeaveCriticalSection(&g_dt.CrSecRxDataExch);

            ::SetEvent(g_dt.EventRxData);
         }
         else if(g_dt.OpMode == OPMODE_RXDEMO)
         {
            g_dt.BytesRXed += Buf->BytesTransferred;

            //--- processing received data

           // EnterCriticalSection(&g_CrSecFWAccess);
           // if(g_FW.IsValid)
            {
               nbytes = STREAM_BUF_SIZE;

               if(g_dt.hRxFile != NULL)
                  fwrite(Buf->Buffer(), nbytes, 1, g_dt.hRxFile);

              // g_FW.buf_ptr += nbytes;
            }
           // LeaveCriticalSection(&g_CrSecFWAccess);
            if(fEnd == TRUE)
            {
               unsigned char buf[CTRL_BUF_SIZE];

               MakeRxCmd(FW_RX_RDY,buf);
               EnterCriticalSection(&g_dt.CrSecRxDataExch);
               memcpy(g_dt.CtrlRxExch,buf,CTRL_BUF_SIZE);
               LeaveCriticalSection(&g_dt.CrSecRxDataExch);
               ::SetEvent(g_dt.EventRxData);
            }
         }
         else if(g_dt.OpMode == OPMODE_GET_FW)
         {
            //--- processing received data

            EnterCriticalSection(&g_dt.CrSecFWAccess);
            if(g_dt.FW.IsValid)
            {
               if(g_dt.FW.len < STREAM_BUF_SIZE)
                  nbytes = g_dt.FW.len;
               else
                  nbytes = STREAM_BUF_SIZE;

               if(g_dt.hRxFile != NULL)
                  fwrite(Buf->Buffer(), nbytes, 1, g_dt.hRxFile);

               g_dt.FW.buf_ptr += nbytes;
               g_dt.FW.len -= nbytes;
               if(g_dt.FW.len <= 0)  //-- End of transfer
               {
                  g_dt.FW.IsValid = 0; //-- Stop wr to buf
                  fEnd = TRUE;
               }
            }
            LeaveCriticalSection(&g_dt.CrSecFWAccess);
            if(fEnd == TRUE)
            {
               unsigned char buf[CTRL_BUF_SIZE];

               MakeRxCmd(FW_RX_RDY,buf);
               EnterCriticalSection(&g_dt.CrSecRxDataExch);
               memcpy(g_dt.CtrlRxExch, buf,CTRL_BUF_SIZE);
               LeaveCriticalSection(&g_dt.CrSecRxDataExch);
               ::SetEvent(g_dt.EventRxData);
            }
         //----
         }
      }
   }
}

//----------------------------------------------------------------------------
void CRxPipe::TerminateThread()
{
   AbortPipe();
}

//----------------------------------------------------------------------------
BOOL CRxPipe::BindPipe(int ep_phys_num)
{
   int rc;
   rc = Bind(g_dt.DeviceNumber,ep_phys_num, g_dt.DevList, &g_dt.UsbioID);
   if(rc && rc != USBIO_ERR_ALREADY_BOUND) //-- Err
   {
      char ErrBuffer[256];
      char Buffer[256];

      _snprintf(Buffer,sizeof(Buffer),"%s", CUsbIo::ErrorText(ErrBuffer,sizeof(ErrBuffer),rc));
      AfxMessageBox(Buffer);
   }
   return rc;
}

//----------------------------------------------------------------------------
void CRxPipe::Start(void)
{
   int rc;
   int buf_size;
   int num_buf;
   int maxerr;

   FreeBuffers();
   Sleep(20);

   if(g_dt.OpMode == OPMODE_PUT_FW)
   {
      buf_size = CTRL_BUF_SIZE;
      num_buf  = CTRL_NUM_OF_BUF;
      maxerr   = CTRL_MAXERR;
   }
   else
   {
      buf_size = STREAM_BUF_SIZE;
      num_buf  = STREAM_NUM_OF_BUF;
      maxerr   = STREAM_MAXERR;
   }


   rc = AllocateBuffers(buf_size,num_buf);
   if(rc)
   {
      rc = StartThread(maxerr);
      if(!rc)
         AfxMessageBox("Stream Rx Pipe - Could not start a thread.");
   }
   else
      AfxMessageBox("Stream Rx Pipe - Could not allocate buffers.");
}

//----------------------------------------------------------------------------
void CRxPipe::Stop(void)
{
   if(!ShutdownThread()) //-- shutdown thread, wait until it is terminated
   {
      AfxMessageBox("Stream Rx Pipe - Could not shutdown thread.");
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


