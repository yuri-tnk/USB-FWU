/*
TNKernel USB Firmware Upgrader Demo

Copyright � 2006,2008 Yuri Tiomkin
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


#ifndef _RXPIPE_H_
#define _RXPIPE_H_

#include "usbiolib/src/UsbIoReader.h"

class CRxPipe : public CUsbIoReader
{
   public:
      CRxPipe();
      ~CRxPipe();


    //-- overload process data function
      virtual void ProcessData(CUsbIoBuf *Buf);
      virtual void ProcessBuffer(CUsbIoBuf *Buf);

      virtual void TerminateThread();
      virtual void BufErrorHandler(CUsbIoBuf* Buf);
    //-------------------------------------

      BOOL BindPipe(int ep_phys_num);
      void Start(void);
      void Stop(void);

   private:
      void MakeRxCmd(int cmd,unsigned char * buf);
};

#endif



