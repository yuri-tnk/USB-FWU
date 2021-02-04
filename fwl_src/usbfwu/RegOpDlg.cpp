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


// RegOpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "usbfwu.h"
#include "EnTabPageDlg.h"
#include "HistoryComboBox.h"
#include "fwu_utils.h"
#include "fwu_cmd.h"

#include "RegOpDlg.h"

#include "fwu_globals.h"

#pragma warning(disable: 4996)


// CRegOpDlg dialog

IMPLEMENT_DYNAMIC(CRegOpDlg, CEnTabPageDlg)

//----------------------------------------------------------------------------
CRegOpDlg::CRegOpDlg(CWnd* pParent /*=NULL*/)
        : CEnTabPageDlg(CRegOpDlg::IDD, pParent)
{

}

CRegOpDlg::~CRegOpDlg()
{
}

//----------------------------------------------------------------------------
void CRegOpDlg::DoDataExchange(CDataExchange* pDX)
{
   CEnTabPageDlg::DoDataExchange(pDX);

   //{{AFX_DATA_MAP(CRegOpDlg)
   DDX_Control(pDX, IDC_BUTTON_START_REGOP, m_StartBtn);
   DDX_Control(pDX, IDC_BUTTON_STOP_REGOP, m_StopBtn);
   DDX_Control(pDX, IDC_BUTTON_FILE_REGOP, m_FileDlgBtn);
   DDX_Control(pDX, IDC_STATIC_RATE_REGOP, m_RxRate);
   DDX_Control(pDX, IDC_STATIC_NUM_RXBYTES_REGOP, m_NumRxBytes);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegOpDlg, CEnTabPageDlg)
   ON_BN_CLICKED(IDC_BUTTON_FILE_REGOP, OnBnClickedButtonFileRegop)
   ON_BN_CLICKED(IDC_BUTTON_START_REGOP, OnBnClickedButtonStartRegop)
   ON_BN_CLICKED(IDC_BUTTON_STOP_REGOP, OnBnClickedButtonStopRegop)
END_MESSAGE_MAP()


//----------------------------------------------------------------------------
BOOL CRegOpDlg::OnInitDialog()
{
   CEnTabPageDlg::OnInitDialog();

   // TODO:  Add extra initialization here

   CString RegPath = CString(g_FilesRegAddr) + CString(_T("FilesRegOpTo"));
   CreateHisoryComboBox(this, &m_HistoryCombo, RegPath,
                          IDC_COMBO_PLACEHOLDER_REGOP,
                          IDC_HCOMBO_REGOP);

   g_dt.RegOpDlg = this;

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}


// CRegOpDlg message handlers

//------------------ File dialog ---------------------------------------------
void CRegOpDlg::OnBnClickedButtonFileRegop()
{
  // AfxMessageBox("Regop_Readfw");
   const char szFilters[] = "Data Files (*.dat)|*.dat|All Files (*.*)|*.*||";
   CFileDialog fileDlg (FALSE, "dat", "*.dat", OFN_HIDEREADONLY | OFN_ENABLESIZING, szFilters, this);

   if(fileDlg.DoModal() == IDOK)
   {
      CString pathName = fileDlg.GetPathName();
      m_HistoryCombo.SetWindowText(pathName);
      m_HistoryCombo.HistoryChanged();
   }
}

//------------------ Start Btn -----------------------------------------------
void CRegOpDlg::OnBnClickedButtonStartRegop()
{
  // AfxMessageBox("Start_Readfw");
   BOOL fExit = FALSE;
   CString s;
   CString s1;

   g_dt.NowFWLoader = FALSE;

   m_RxRate.SetWindowText(_T("0"));
   m_NumRxBytes.SetWindowText(_T("0"));

   m_HistoryCombo.HistoryChanged();
   m_HistoryCombo.EnableWindow(FALSE);

   //--- File operation -------
   for(;;) //-- Single iteration loop
   {
      m_HistoryCombo.GetWindowText(s);
      if(s.GetLength() == 0)
      {
         s1 = _T("File name not specified.");
         fExit = TRUE;
      }
      else //-- Open file
      {
         g_dt.hRxFile = fopen((LPCSTR)s,"w+b");
         if(g_dt.hRxFile == NULL)
         {
            s1 = _T("Could not open file: \n") + s;
            fExit = TRUE;
         }
      }
      break;
   }

   //-------------------------------------------
   if(fExit)  //-- Error processing
   {
      AfxMessageBox(s1);

      m_StartBtn.EnableWindow(FALSE);
      m_FileDlgBtn.EnableWindow(FALSE);

      m_HistoryCombo.EnableWindow(TRUE);
      m_HistoryCombo.SetFocus();

      return;
   }
   //-------------------------------------------

   g_dt.EnableTabPageChanging = FALSE;
   g_dt.TimerRunInfo = TRUE;
   ::ResetEvent(g_dt.EventUserAbort);

   g_dt.WaitSwitchToLoader = FALSE;

   OpenDevice();
   DoSetConfiguration();

   if(IsFWU()) //--boot loader is running now
   {
      g_dt.UsbIo.Close();
      g_dt.DeviceNumber = -1;

      AfxMessageBox(_T("Could not start - now firmware upgrader is running."));

      m_HistoryCombo.EnableWindow(TRUE);
      m_StartBtn.EnableWindow(TRUE);
      m_FileDlgBtn.EnableWindow(TRUE);

      g_dt.EnableTabPageChanging = TRUE;
      g_dt.TimerRunInfo = FALSE;

      return;
   }

   m_StopBtn.EnableWindow(TRUE);

   g_RxPipe.BindPipe(0x82);
   g_RxPipe.ResetPipeStatistics();
   g_RxPipe.SetupPipeStatistics(MEANRATE_AVERAGING_INTERVAL_MS);
   g_RxPipe.Start();

   cmd_to_app(SEND_DEMO_DATA);

   for(;;)
   {
      MSG msg;
      while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
      {
         if(!AfxGetThread()->PumpMessage())
         {
            ASSERT(AfxGetCurrentMessage()->message == WM_QUIT);
            AfxPostQuitMessage(AfxGetCurrentMessage()->wParam);
            return; //-- Never reached
         }
      }

      DWORD rc = WaitForSingleObject(g_dt.EventUserAbort, 50);
      if(rc == WAIT_OBJECT_0)
      {
         cmd_no_data(USER_ABORT);
         break;
      }
   }

   //--- Final

   g_dt.UsbIo.Close();
   g_dt.DeviceNumber = -1;

   m_StopBtn.EnableWindow(FALSE);
   m_StopBtn.UpdateWindow();      //-- Safe disable

   g_dt.TimerRunInfo = FALSE;

   if(g_dt.hRxFile != NULL)
   {
      fclose(g_dt.hRxFile);
      g_dt.hRxFile = NULL;
   }

   UpdatePipeStatistics();

   g_RxPipe.Stop();
   g_RxPipe.Close();

   m_HistoryCombo.EnableWindow(TRUE);

   m_StartBtn.EnableWindow(TRUE);
   m_FileDlgBtn.EnableWindow(TRUE);

   g_dt.EnableTabPageChanging = TRUE;
}

//------------------ Stop Btn ------------------------------------------------
void CRegOpDlg::OnBnClickedButtonStopRegop()
{
  // AfxMessageBox("Stop_Readfw");
   ::SetEvent(g_dt.EventUserAbort);
}

