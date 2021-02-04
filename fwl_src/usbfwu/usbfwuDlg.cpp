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

// usbfwuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EnTabCtrl.h"
#include "ReadFWDlg.h"
#include "WriteFWDlg.h"
#include "RegOpDlg.h"
#include "AboutTabDlg.h"
#include "TxStreamPipe.h"
#include "RxPipe.h"
#include "AnimBmp.h"

#include "fwu_types.h"
#include "fwu_utils.h"

#include "USBIOLIB/inc/usbio_i.h"

#include <Dbt.h>

#include "usbfwu.h"
#include "usbfwuDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----- Globals ---------------

char g_FilesRegAddr[] = "Software\\TNKernel\\Firmware Upgrader Demo\\";

GLODATA g_dt;
CTxStreamPipe  g_TxStreamPipe;
CRxPipe        g_RxPipe;

//---------------------------------------------------------------------------
CusbfwuDlg::CusbfwuDlg(CWnd* pParent /*=NULL*/)
                                : CDialog(CusbfwuDlg::IDD, pParent)
{
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
   m_DevNotify = NULL;
   m_cnt = 0;
}

CusbfwuDlg::~CusbfwuDlg()
{
   CUsbIo::DestroyDeviceList(g_dt.DevList);
   g_dt.DevList = NULL;
}

//---------------------------------------------------------------------------
void CusbfwuDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);

        //{{AFX_DATA_MAP(CusbfwuDlg)
        DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
//        DDX_Control(pDX, IDC_LABEL_WEB, m_HlStatic);
        //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CusbfwuDlg, CDialog)
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
   ON_WM_DEVICECHANGE()
   ON_WM_DESTROY()
   ON_WM_TIMER()
   ON_MESSAGE(CM_TAB_CHANGE, OnTabChange)
   ON_MESSAGE(WM_USER_THREAD_TERMINATED, OnMessageThreadExit)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()



//---------------------------------------------------------------------------
BOOL CusbfwuDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

        // Set the icon for this dialog.  The framework does this automatically
        //  when the application's main window is not a dialog
   SetIcon(m_hIcon, TRUE);                        // Set big icon
   SetIcon(m_hIcon, FALSE);                // Set small icon

//------- Tab control ---------

        int nPageID = 0;

   m_ReadFWDlg.Create(IDD_DIALOG_READFW, this);
   m_TabCtrl.AddPageDlg (_T(" Read Firmware"), nPageID++, &m_ReadFWDlg);

   m_WriteFWDlg.Create(IDD_DIALOG_WRITEFW, this);
   m_TabCtrl.AddPageDlg (_T(" Write Firmware"), nPageID++, &m_WriteFWDlg);

   m_RepOpDlg.Create(IDD_DIALOG_REGULAROP, this);
   m_TabCtrl.AddPageDlg (_T(" Regular Operation"), nPageID++, &m_RepOpDlg);

   m_AboutTabDlg.Create(IDD_DIALOG_ABOUT, this);
   m_TabCtrl.AddPageDlg (_T(" About..."), nPageID, &m_AboutTabDlg);

   DWORD dwFlags = 0;

   dwFlags |= ETC_FLAT;
   dwFlags |= ETC_SELECTION;
   dwFlags |= ETC_GRADIENT;

   m_TabCtrl.EnableDraw(BTC_ALL);
   CEnTabCtrl::EnableCustomLook(dwFlags , dwFlags);

   //---------------------------------
   CStatic * pStatic = (CStatic*)GetDlgItem(IDC_INFO_DEVCONN);
   pStatic->GetWindowRect(&m_rectConnStr);
   ScreenToClient(&m_rectConnStr);

   //---------------------------------
   if(!Init(&g_dt))
      return FALSE;
   //---------------------------------

  SetWindowText(_T("TNKernel USB Firmware Upgrader Demo"));

//--- Icons & BMP

  m_IconWarn = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                 MAKEINTRESOURCE(IDI_ICON_WARN),
                                 IMAGE_ICON, 16, 16,
                                 LR_DEFAULTCOLOR);

  m_AnimBmp.LoadBitmap(IDB_BITMAP_RUN);
  m_AnimCtrl.SetBaseImage(m_AnimBmp,
                 16,                 //-- Single pictures X size
                 16,                 //-- Single pictures Y size
                 8);                 //-- N pictures in BMP(especially when != rows * columns)


   SetTimer(1, 100, NULL);

   return TRUE;  // return TRUE  unless you set the focus to a control
}

//---------------------------------------------------------------------------
void CusbfwuDlg::OnPaint()
{
        if(IsIconic())
        {
                CPaintDC dc(this); // device context for painting

                SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

                // Center icon in client rectangle
                int cxIcon = GetSystemMetrics(SM_CXICON);
                int cyIcon = GetSystemMetrics(SM_CYICON);
                CRect rect;
                GetClientRect(&rect);
                int x = (rect.Width() - cxIcon + 1) / 2;
                int y = (rect.Height() - cyIcon + 1) / 2;

                // Draw the icon
                dc.DrawIcon(x, y, m_hIcon);
        }
        else
        {
                CDialog::OnPaint();
        }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CusbfwuDlg::OnQueryDragIcon()
{
        return static_cast<HCURSOR>(m_hIcon);
}

//----------------------------------------------------------------------------
BOOL CusbfwuDlg::Init(GLODATA * pGD)
{
   GUID qaz = USBIO_IID; //-- if not demo - change it

   pGD->UsbioID = qaz;

   pGD->EventUserAbort = ::CreateEvent(NULL,FALSE,FALSE,NULL);
   pGD->EventRxData    = ::CreateEvent(NULL,FALSE,FALSE,NULL);
   pGD->EventTxStream  = ::CreateEvent(NULL,TRUE,FALSE,NULL); //--Manual reset

   InitializeCriticalSection(&pGD->CrSecRxDataExch);
   InitializeCriticalSection(&pGD->CrSecFWAccess);

   pGD->FWBuf = (unsigned char *)malloc(MAX_FW_SIZE);

   pGD->BytesRXed = 0;
   pGD->hRxFile = NULL;

   pGD->OpMode = OPMODE_GET_FW;
   pGD->RdFWIcon = 0;
   pGD->WrFWIcon = 0;

   pGD->NowFWLoader  = FALSE;
   pGD->TimerRunInfo = FALSE;
   pGD->EnableTabPageChanging = TRUE;
   pGD->WaitSwitchToLoader    = FALSE;
   pGD->fOpInProgress = FALSE;
   pGD->OpState = 0;
   pGD->BtnTimeoutCnt = 0;



   pGD->ActiveDlg = pGD->RdDlg;  //-- Alredy should be created

   if(pGD->EventUserAbort == NULL || pGD->EventRxData == NULL ||
      pGD->EventTxStream == NULL || pGD->FWBuf == NULL)
   {
      AfxMessageBox(_T("FATAL ERROR: Out of system resources."));
      return FALSE;
   }

   memset(pGD->FWBuf, 0, MAX_FW_SIZE);
   memset(&pGD->FW, 0, sizeof(FW_INFO));

   UpdateDeviceList(pGD);

   if(!RegisterDevNotify(&pGD->UsbioID, &m_DevNotify))
   {
      AfxMessageBox(_T("ERROR: Unable to register device notification."));
      free(pGD->FWBuf);
      return FALSE;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
void CusbfwuDlg::UpdateDeviceList(GLODATA * pGD)
{
   CString str;
   CString s;
   CUsbIo UsbDev;
   DWORD Status;
   USB_DEVICE_DESCRIPTOR desc;

   int devcount = 0;

   CUsbIo::DestroyDeviceList(pGD->DevList);             //-- destroy current device list
   pGD->DevList = CUsbIo::CreateDeviceList(&pGD->UsbioID); //-- create new device list
   if(pGD->DevList != NULL)
   {
      for(int i=0; ;i++)  //-- iterate through the list
      {
         Status = UsbDev.GetDeviceInstanceDetails(i, pGD->DevList, &pGD->UsbioID);
         if(Status != USBIO_ERR_SUCCESS) //-- we have reached the end of the list
            break;
         Status = UsbDev.Open(i, pGD->DevList, &pGD->UsbioID);
         if(Status == USBIO_ERR_SUCCESS)
         {
            // get device descriptor and print out IDs
            Status = UsbDev.GetDeviceDescriptor(&desc);
            if(Status == USBIO_ERR_SUCCESS)
            {
            }
            else
            {
            //        PrintError(Status, E_ERROR, "GetDeviceDescriptor failed: ");
            }
            UsbDev.Close();
         }
         devcount++;
      }
   }

   if(devcount == 0)
   {
      ShowDevConn(FALSE);

      EnableItem(pGD, BTN_STOP, FALSE);
      EnableItem(pGD, BTN_START, FALSE);
   }
   else
   {
      ShowDevConn(TRUE);

      if(g_dt.fOpInProgress == FALSE)
      {    
         EnableItem(pGD, BTN_START, TRUE);
         EnableItem(pGD, BTN_STOP, FALSE);
      }
   }
}

//----------------------------------------------------------------------------
void CusbfwuDlg::ShowDevConn(BOOL fShow)
{
   CRect rIcon,rStr, rc; 

#define  ICON_CONN_SIZE   16

    //------ Icon
   CStatic * pStatic = (CStatic*)GetDlgItem(IDC_ICON_CONN);
   pStatic->GetWindowRect(&rIcon);
   ScreenToClient(&rIcon);

   if(fShow)
   {
    //------ Icon
      pStatic->ShowWindow(SW_HIDE);

      rStr.top     = m_rectConnStr.top;
      rStr.left    = rIcon.left;
      rStr.right   = rStr.left + (m_rectConnStr.right  - m_rectConnStr.left);
      rStr.bottom  = rStr.top  + (m_rectConnStr.bottom - m_rectConnStr.top);

    //------ String
      pStatic = (CStatic*)GetDlgItem(IDC_INFO_DEVCONN);
      pStatic->MoveWindow(&rStr);
      pStatic->SetWindowText(_T("Device on-line."));
   }
   else
   {
    //------ Icon
      pStatic = (CStatic*)GetDlgItem(IDC_ICON_CONN);
      pStatic->MoveWindow(rIcon.left + (((rIcon.right - rIcon.left)  - ICON_CONN_SIZE)>>1), 
                         (m_rectConnStr.top + ((m_rectConnStr.bottom - m_rectConnStr.top)>>1))
                                              - (ICON_CONN_SIZE>>1),
                         ICON_CONN_SIZE, ICON_CONN_SIZE);
      pStatic->ShowWindow(SW_SHOW);

    //------ String
      pStatic = (CStatic*)GetDlgItem(IDC_INFO_DEVCONN);
      pStatic->MoveWindow(&m_rectConnStr);
      pStatic->SetWindowText(_T("No connection to Device."));
   }
}
//----------------------------------------------------------------------------
BOOLEAN CusbfwuDlg::RegisterDevNotify(const GUID *InterfaceClassGuid,
                                   PVOID *hDevNotify)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = *InterfaceClassGuid;

    // device notifications should be send to the main dialog
    *hDevNotify = RegisterDeviceNotification(
                      m_hWnd,
                      &NotificationFilter,
                      DEVICE_NOTIFY_WINDOW_HANDLE
                      );
    if(!(*hDevNotify))
    {
       DWORD Err = GetLastError();
       return FALSE;
    }

    return TRUE;
}

//--------- Tab changed -----------------------------------------------------------------

LRESULT CusbfwuDlg::OnTabChange(WPARAM wpar, LPARAM lpar)
{
   if(wpar == 0) //-- Changed
   {
      int pos = m_TabCtrl.GetCurSel();
      switch(pos)
      {
         case 0:
            g_dt.OpMode = OPMODE_GET_FW;
            break;
         case 1:
            g_dt.OpMode = OPMODE_PUT_FW;
            break;
         case 2:
            g_dt.OpMode = OPMODE_RXDEMO;
            break;
         case 3:
            g_dt.OpMode = OPMODE_ABOUT;
            break;
      }
      return 0;
   }
   else if(wpar == 1) //-- Changing
   {
      return g_dt.EnableTabPageChanging;  //-- Enable or not tab changing
   }

   return 0;
}

//------ Destroy -------------------------------------------------------------

void CusbfwuDlg::OnDestroy()
{
   if(g_dt.UsbIo.IsOpen())
      g_dt.UsbIo.Close();  //-- close device

   if(m_DevNotify != NULL)
   {
      UnregisterDeviceNotification(m_DevNotify);
      m_DevNotify = NULL;
   }

   if(g_dt.hRxFile != NULL)
      fclose(g_dt.hRxFile);
   if(g_dt.OpMode == OPMODE_GET_FW || g_dt.OpMode == OPMODE_RXDEMO)
   {
      g_RxPipe.Stop();
      g_RxPipe.Close();
   }
   else if(g_dt.OpMode == OPMODE_PUT_FW)
   {
      g_TxStreamPipe.Stop();
      g_TxStreamPipe.Close();
   }

   ::CloseHandle(g_dt.EventUserAbort);
   ::CloseHandle(g_dt.EventRxData);
   ::CloseHandle(g_dt.EventTxStream);

   ::DeleteCriticalSection(&g_dt.CrSecRxDataExch);
   ::DeleteCriticalSection(&g_dt.CrSecFWAccess);

   free(g_dt.FWBuf);

   KillTimer(1);

   CDialog::OnDestroy();
}

//------ Timer ---------------------------------------------------------------

void CusbfwuDlg::OnTimer(UINT_PTR nIDEvent)
{
   static int cnt =0;

   if(g_dt.TimerRunInfo == TRUE)
   {
      if(g_dt.OpMode == OPMODE_GET_FW || g_dt.OpMode == OPMODE_PUT_FW)
      {
         CDC * pDC;
         CRect r;
         CStatic * pStatic;
         CWnd *pWnd;
         int id;

         if(g_dt.OpMode == OPMODE_GET_FW)
         {
            pWnd = g_dt.RdDlg;
            id = IDC_STATIC_STATUSICON_READFW;
         }
         else if(g_dt.OpMode == OPMODE_PUT_FW)
         {
            pWnd = g_dt.WrDlg;
            id = IDC_STATIC_STATUSICON_WRITEFW;
         }

         pStatic = (CStatic *)pWnd->GetDlgItem(id);
         pStatic->GetWindowRect(&r);
         pWnd->ScreenToClient(&r);
         pDC  = pWnd->GetDC();
         m_AnimCtrl.DrawAnimFrame(pDC, r.left, r.top);
         pWnd->ReleaseDC(pDC);
      }
      else if(g_dt.OpMode == OPMODE_RXDEMO)
      {
         cnt++;
         if(cnt%3 == 0)  //-- Each 300 mS
            UpdatePipeStatistics();
      }
   }

   if(g_dt.BtnTimeoutCnt > 0)
   {
      g_dt.BtnTimeoutCnt--;
      if(g_dt.BtnTimeoutCnt == 0)
      {
         if(g_dt.OpMode == OPMODE_GET_FW || g_dt.OpMode == OPMODE_PUT_FW)
         {
            g_dt.fOpInProgress = FALSE; 
            g_dt.EnableTabPageChanging = TRUE;

            m_cnt = 0;

         //if(g_dt.UsbIo.IsOpen())
            EnableItem(&g_dt, BTN_START, TRUE);

            if(g_dt.OpMode == OPMODE_GET_FW)
               ::PostMessage(g_dt.RdDlg->GetSafeHwnd(), CM_END_OP, 0, 0);
            else if(g_dt.OpMode == OPMODE_PUT_FW)
               ::PostMessage(g_dt.WrDlg->GetSafeHwnd(), CM_END_OP, 0, 0);
         }
      }
   }

   CDialog::OnTimer(nIDEvent);
}

//----------------------------------------------------------------------------
LRESULT CusbfwuDlg::OnMessageThreadExit(WPARAM wpar, LPARAM lpar)
{
   return 0;
}

//-------- Device Change ---------------------------------------------------------------

BOOL CusbfwuDlg::OnDeviceChange( UINT nEventType, DWORD dwData )
{
  static int cnt = 0;

  DEV_BROADCAST_DEVICEINTERFACE *data = (DEV_BROADCAST_DEVICEINTERFACE*)dwData;

  //-- checking data
  if(data == NULL || data->dbcc_name == NULL || strlen(data->dbcc_name)==0)
     return FALSE;

  //-- convert interface name to CString
  CString Name(data->dbcc_name);

  // Thesycon:
  // "there is some strange behavior in Win98
  // there are notifications with dbcc_name = "."
  // we ignore this"
  if(Name.GetLength() < 5)
     return FALSE;

  switch (nEventType)
  {
     case DBT_DEVICEREMOVECOMPLETE:

       //-- TODO - Change status message "Canceled by user"
       //--        to correct - "Canceled - no connections"

        if(g_dt.fOpInProgress) 
           g_dt.OpState = 1;

        if(g_dt.UsbIo.GetDevicePathName() && (0==Name.CompareNoCase(g_dt.UsbIo.GetDevicePathName())) )
        {
            g_dt.UsbIo.Close();
            g_dt.DeviceNumber = -1;
        }

        UpdateDeviceList(&g_dt); // re-create device list
        break;

     case DBT_DEVICEARRIVAL:

       ::ResetEvent(g_dt.EventUserAbort);
        UpdateDeviceList(&g_dt); // re-create device list

        g_dt.WaitSwitchToLoader = TRUE;

        if(g_dt.fOpInProgress) 
        {
           //-- Start Timeout for Start Btn enable -
           //-- after "device was removed and then attached" sequence
           if(g_dt.OpState == 1)  
           {
              g_dt.OpState = 0;

              //-- Start "Enable Start Button Timeout"
              if(g_dt.OpMode == OPMODE_GET_FW)
                 g_dt.BtnTimeoutCnt = BTN_EN_TIMEOUT_GETFW;
              else if(g_dt.OpMode == OPMODE_PUT_FW)
              {  
                // m_cnt++;
                // if(m_cnt == 1)
                //    g_dt.BtnTimeoutCnt = (BTN_EN_TIMEOUT_PUTFW)<<1;
                // else
                    g_dt.BtnTimeoutCnt = BTN_EN_TIMEOUT_PUTFW;
              }
           }
        }

        break;

     case DBT_DEVICEQUERYREMOVE:
        break;

     case DBT_DEVICEREMOVEPENDING:
        break;

     default:
        break;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------






