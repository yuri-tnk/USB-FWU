/************************************************************************
 *
 *  Module:       UsbIoThread.cpp
 *  Long name:    CUsbIoThread class
 *  Description:  Input/Output Thread, base class for Reader and Writer
 *
 *  Runtime Env.: Win32, Part of UsbioLib
 *  Author(s):    Guenter Hildebrandt, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

// for shorter and faster windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
// unicode is not supported by USBIOLIB
#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <process.h>
#include "usbiothread.h"



CUsbIoThread::CUsbIoThread()
{
	mThreadPriority = THREAD_PRIORITY_HIGHEST;
  ThreadHandle = NULL;
  ThreadID = 0;
  TerminateFlag = FALSE;
  MaxErrorCount = 3;

  FirstPending = NULL;
  LastPending = NULL;

}


CUsbIoThread::~CUsbIoThread()
{
  // thread must have been terminated at this point
#ifdef _DEBUG
  if ( ThreadHandle != NULL ) {
    OutputDebugString(
      "CUsbIoThread::~CUsbIoThread(): Thread is still running! "
      "ShutdownThread() must be called before the object is destroyed!\n");
  }
#endif
  
  FreeBuffers();
}




BOOL CUsbIoThread::AllocateBuffers(DWORD SizeOfBuffer, DWORD NumberOfBuffers)
{
  return BufPool.Allocate(SizeOfBuffer, NumberOfBuffers);
}



void CUsbIoThread::FreeBuffers()
{
  BufPool.Free();
}



BOOL CUsbIoThread::StartThread(DWORD MaxIoErrorCount/*=3*/)
{
  BOOL succ = FALSE;
  
  if ( ThreadHandle != NULL ) {
    // already started
  } else {
  
    TerminateFlag = FALSE;
    MaxErrorCount = MaxIoErrorCount;

    // we use _beginthreadex, it is a wrapper for the Win32 API CreateThread
    ThreadHandle = (HANDLE)
      _beginthreadex( 
            NULL, //void *security,
            0,    //unsigned stack_size, 
            beginthread_routine,  //unsigned ( __stdcall *start_address )( void * ),
            this, //void *arglist,
            0,    //unsigned initflag,
            &ThreadID     //unsigned *thrdaddr
            );
    if ( ThreadHandle == NULL ) {
      // failed
    } else {
      // success
      succ = TRUE;
    }
  }

  return succ;
}



BOOL CUsbIoThread::ShutdownThread()
{
  BOOL succ = TRUE;
  HANDLE h;


  h = (HANDLE)InterlockedExchange((long*)&ThreadHandle,0);
  if ( h != NULL ) {

    TerminateFlag = TRUE;
    // call virtual member function
    TerminateThread();

    // wait on thread handle
    DWORD err = WaitForSingleObject(h,INFINITE);  
    if ( err ) {
      succ = FALSE;
    }
    // close thread handle
    CloseHandle(h);
  }
  return succ;
}



// system thread routine
unsigned int __stdcall CUsbIoThread::beginthread_routine(void *StartContext)
{
  HANDLE h;


  CUsbIoThread *This = (CUsbIoThread*)StartContext;
	// adjust thread priority
	::SetThreadPriority(::GetCurrentThread(),This->mThreadPriority);
  // call virtual member function
  This->ThreadRoutine();
  // call notification handler
  This->OnThreadExit();
  // close thread handle
  h=(HANDLE)InterlockedExchange((long*)&This->ThreadHandle,0);
  if ( h != NULL ) {
    CloseHandle(h);
  }
  // terminate the thread
  return 0;
}




//
// default implementation of virtual functions
//

void CUsbIoThread::OnThreadExit()
{
  // empty
}



void CUsbIoThread::ProcessBuffer(CUsbIoBuf* Buf)
{
  ZeroMemory(Buf->Buffer(),Buf->Size());
  Buf->NumberOfBytesToTransfer=Buf->Size();
  Buf->BytesTransferred=0;
}



void CUsbIoThread::ProcessData(CUsbIoBuf* Buf)
{
  // empty
}



void CUsbIoThread::BufErrorHandler(CUsbIoBuf* Buf)
{
  // empty
}

 
/*************************** EOF **************************************/
