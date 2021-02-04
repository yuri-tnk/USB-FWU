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

#ifndef _FWU_TYPES_H_
#define _FWU_TYPES_H_

#include "EnTabPageDlg.h"
#include "usbiolib/src/UsbIo.h"

#define  MAX_PIPES        4

#define  MESSAGEBUF_SIZE  256


//--  m_OpMode

#define  OPMODE_RXDEMO    0
#define  OPMODE_GET_FW    1
#define  OPMODE_PUT_FW    2
#define  OPMODE_ABOUT     3


// averaging interval to be set for mean data rate calculation
#define MEANRATE_AVERAGING_INTERVAL_MS                1000        // ms

//------- Ctrl

#define  CTRL_BUF_SIZE           64
#define  CTRL_NUM_OF_BUF         32
#define  CTRL_MAXERR              5

#define  CTRL_BUF_CRC_OFF        60
#define  CTRL_MAX_N_TIMEOUT       3
#define  CTRL_MAX_N_CRCERR        3

//-------- Fw

#define  FW_PRODUCT_ID_OFFSET     4
#define  FW_VERSION_OFFSET        8
#define  FW_LENGTH_OFFSET        12
#define  FW_CRC_OFFSET           16

#define  FW_ROM_PRODUCT_ID_OFFSET   0
#define  FW_ROM_VERSION_OFFSET      4
#define  FW_ROM_LENGTH_OFFSET       8
#define  FW_ROM_CRC_OFFSET         12


#define  MAX_FW_SIZE       (1024*256)
#define  FW_LOAD_ADDR      0x00002000
#define  FW_START_OFFSET         0x20

//-------- Stream

#define  STREAM_BUF_SIZE       4096
#define  STREAM_NUM_OF_BUF        4
#define  STREAM_MAXERR            5

//-------- Cmd

   //-- Host --

#define  GET_FW_INFO          0x05
#define  GET_FW               0x07
#define  FW_RX_RDY            0x09   //-- internal
#define  END_OK               0x0B
#define  END_ABORT            0x0D
#define  PUT_FW_INFO          0x10
#define  FW_WR                0x12
#define  APP_SEND_DATA        0x14
#define  APP_SEND_0           0x16
#define  USER_ABORT           0x0D

#define  APP_CMD              0x50
  //-- Application specific
#define  SEND_DEMO_DATA     0x1111


   //-- Device  --

#define  FW_INFO_ASK          0x70
#define  END_ASK              0x71
#define  ABORT_ASK            0x72
#define  PUT_FW_INFO_ASK      0x73
#define  FW_WR_RDY            0x78
#define  FW_WR_ERR            0x75
#define  APP_SEND_ASK         0x7A


//-------- Timeouts

#define  TIMEOUT_GET_FW       10000
#define  TIMEOUT_END_OK         100
#define  TIMEOUT_GET_FW_INFO    100
#define  TIMEOUT_END_ABORT     1000

#define  TIMEOUT_PUT_FW_INFO   1000
#define  TIMEOUT_WR_FW        10000

//-- User's Windows messages
#define  WM_USER_THREAD_TERMINATED  (WM_USER+100)

//-- Controls type to enable/disable

#define  BTN_START       1
#define  BTN_STOP        2
#define  BTN_FILE        3
#define  COMBO_FILE      4

//------- USB exchange

#define  ERR_RXDATA_OK       0
#define  ERR_UNKNOWN_ERR     1
#define  ERR_RXDATA_BADCRC   2
#define  ERR_TIMEOUT         3
#define  ERR_USER_ABORT      4
#define  ERR_WIN_INTERNAL    5
#define  ERR_WR_ERR          6

#define  GET_ISFWU        0x16
#define  SWITCH_TO_FWU    0x18
#define  GET_ISFWU_ASK    0x1234AA55


//-- Enable Start Button Timeouts

#define BTN_EN_TIMEOUT_GETFW   15   //-- 1.5 sec
#define BTN_EN_TIMEOUT_PUTFW   28

//-- Custom Windows messages 

#define CM_TAB_CHANGE (WM_APP + 100)
#define CM_END_OP     (WM_APP + 101)

//----------------------------------------------------------------------------
typedef struct _FW_INFO
{
   unsigned __int32 fw_product_id;
   unsigned __int32 fw_version;
   unsigned __int32 fw_length;
   unsigned __int32 fw_crc;
   int   IsValid;
   unsigned char * buf_ptr;
   int   len;
}FW_INFO;



typedef struct _GLODATA
{
   CUsbIo    UsbIo;
   GUID      UsbioID;
   HDEVINFO  DevList;

   int       DeviceNumber;

   FW_INFO   FW;

   HANDLE    EventUserAbort;
   HANDLE    EventRxData;
   HANDLE    EventTxStream;

   CRITICAL_SECTION CrSecRxDataExch;
   CRITICAL_SECTION CrSecFWAccess;

   DWORD  BytesRXed;
   FILE * hRxFile;
   char MessBuf[MESSAGEBUF_SIZE]; //-- Err & status messages
   unsigned char CtrlRxExch[CTRL_BUF_SIZE];
   unsigned char * FWBuf;

   CEnTabPageDlg * ActiveDlg;

   CEnTabPageDlg * WrDlg;
   CEnTabPageDlg * RdDlg;
   CEnTabPageDlg * RegOpDlg;
   CEnTabPageDlg * AboutDlg;

   int OpMode;
   int RdFWIcon;
   int WrFWIcon;

   BOOL NowFWLoader;
   int  BtnTimeoutCnt;
   BOOL TimerRunInfo;
   BOOL EnableTabPageChanging;
   BOOL WaitSwitchToLoader;
   BOOL fOpInProgress;
   int  OpState;

   unsigned char m_RxBuf[64];

}GLODATA;

#endif



