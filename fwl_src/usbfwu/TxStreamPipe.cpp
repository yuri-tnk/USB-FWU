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
#include "usbiolib/src/UsbIoWriter.h"
#include "TxStreamPipe.h"
#include "fwu_utils.h"

#include "fwu_types.h"
#include "fwu_globals.h"

#pragma warning(disable: 4996)

//----------------------------------------------------------------------------
CTxStreamPipe::CTxStreamPipe()
{
}

//----------------------------------------------------------------------------
CTxStreamPipe::~CTxStreamPipe()
{
}

//----------------------------------------------------------------------------
BOOL CTxStreamPipe::BindPipe(int ep_phys_num)
{
   int rc;
   rc = Bind(g_dt.DeviceNumber, ep_phys_num, g_dt.DevList, &g_dt.UsbioID);
   if(rc && rc != USBIO_ERR_ALREADY_BOUND) //-- Err
   {
      char ErrBuffer[256];
      char Buffer[256];

      _snprintf(Buffer,sizeof(Buffer),"%s", CUsbIo::ErrorText(ErrBuffer,
                                                      sizeof(ErrBuffer),rc));
      AfxMessageBox(Buffer);
   }
   return rc;
}

//----------------------------------------------------------------------------
void CTxStreamPipe::Start(void)
{
   int rc;

   FreeBuffers();
   ::Sleep(20);
   rc = AllocateBuffers(STREAM_BUF_SIZE, STREAM_NUM_OF_BUF);
   if(rc)
   {
      rc = StartThread(STREAM_MAXERR);
      if(!rc)
         AfxMessageBox("Stream Tx Pipe - Could not start a thread.");
   }
   else
      AfxMessageBox("Stream Tx Pipe - Could not allocate buffers.");
}

//----------------------------------------------------------------------------
void CTxStreamPipe::Stop(void)
{
   if(!ShutdownThread()) //-- shutdown thread, wait until it is terminated
   {
      AfxMessageBox("Control Rx Pipe - Could not shutdown thread.");
   }
}

//----------------------------------------------------------------------------
// overloaded process data function
void CTxStreamPipe::ProcessBuffer(CUsbIoBuf *Buf)
{
   DWORD rc;

   rc = WaitForSingleObject(g_dt.EventTxStream,INFINITE);
   if(rc == WAIT_OBJECT_0)
   {
      EnterCriticalSection(&g_dt.CrSecFWAccess);
      if(g_dt.FW.len > 0)
      {
         memcpy(Buf->Buffer(), g_dt.FW.buf_ptr, STREAM_BUF_SIZE);
         g_dt.FW.buf_ptr += STREAM_BUF_SIZE;
         g_dt.FW.len -= STREAM_BUF_SIZE;
      }
      else //-- dummy, should be never in use
      {
         memset(Buf->Buffer(), 0xFF, STREAM_BUF_SIZE);
      }
      LeaveCriticalSection(&g_dt.CrSecFWAccess);

      Buf->NumberOfBytesToTransfer = STREAM_BUF_SIZE;
      Buf->OperationFinished = false;
   }
}

//----------------------------------------------------------------------------
void CTxStreamPipe::TerminateThread()
{
   AbortPipe();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




