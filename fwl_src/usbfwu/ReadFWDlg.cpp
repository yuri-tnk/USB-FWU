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


// ReadFWDlg.cpp : implementation file
//

#include "stdafx.h"
#include "usbfwu.h"
#include "EnTabPageDlg.h"
#include "HistoryComboBox.h"
#include "fwu_utils.h"
#include "fwu_cmd.h"

#include "ReadFWDlg.h"

#include "fwu_globals.h"

#pragma warning(disable: 4996)


// CReadFWDlg dialog

IMPLEMENT_DYNAMIC(CReadFWDlg, CEnTabPageDlg)

//----------------------------------------------------------------------------
CReadFWDlg::CReadFWDlg(CWnd* pParent /*=NULL*/)
        : CEnTabPageDlg(CReadFWDlg::IDD, pParent)
{
   m_rc = 0;
}

CReadFWDlg::~CReadFWDlg()
{
}

//----------------------------------------------------------------------------
void CReadFWDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);

   DDX_Control(pDX, IDC_BUTTON_START_READFW, m_StartBtn);
   DDX_Control(pDX, IDC_BUTTON_STOP_READFW, m_StopBtn);
   DDX_Control(pDX, IDC_BUTTON_FILE_READFW, m_FileDlgBtn);
   DDX_Control(pDX, IDC_BUTTON_FILE_READFW, m_FileDlgBtn);
   DDX_Control(pDX, IDC_STATIC_FW_INFO_READFW, m_FwInfoStr);
   DDX_Control(pDX, IDC_STATIC_STATUSTEXT_READFW, m_StatusStr);
   DDX_Control(pDX, IDC_STATIC_STATUSICON_READFW, m_StatusIcon);
}


BEGIN_MESSAGE_MAP(CReadFWDlg, CEnTabPageDlg)
   ON_BN_CLICKED(IDC_BUTTON_FILE_READFW, &CReadFWDlg::OnBnClickedButtonFileReadfw)
   ON_BN_CLICKED(IDC_BUTTON_START_READFW, &CReadFWDlg::OnBnClickedButtonStartReadfw)
   ON_BN_CLICKED(IDC_BUTTON_STOP_READFW, &CReadFWDlg::OnBnClickedButtonStopReadfw)
   ON_MESSAGE(CM_END_OP, OnRdFwEnd)
END_MESSAGE_MAP()


// CReadFWDlg message handlers

//----------------------------------------------------------------------------
BOOL CReadFWDlg::OnInitDialog()
{
   CEnTabPageDlg::OnInitDialog();

   // TODO:  Add extra initialization here

   CString RegPath = CString(g_FilesRegAddr) + CString(_T("FilesStoreTo"));
   CreateHisoryComboBox(this, &m_HistoryCombo, RegPath,
                          IDC_COMBO_PLACEHOLDER_READFW,
                          IDC_HCOMBO_READFW);

   g_dt.RdDlg = this;

   m_IconErr = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                 MAKEINTRESOURCE(IDI_ICON_ERR),
                                 IMAGE_ICON, 16, 16,
                                 LR_DEFAULTCOLOR);
   m_IconOK = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                 MAKEINTRESOURCE(IDI_ICON_OK),
                                 IMAGE_ICON, 16, 16,
                                 LR_DEFAULTCOLOR);
   m_IconWarn = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                 MAKEINTRESOURCE(IDI_ICON_WARN),
                                 IMAGE_ICON, 16, 16,
                                 LR_DEFAULTCOLOR);

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

//------------------ Start Btn -----------------------------------------------
void CReadFWDlg::OnBnClickedButtonStartReadfw()
{
  // AfxMessageBox("Start_Readfw");
   BOOL fExit = FALSE;
   CString s,s1,s_err;
   DWORD rc;

   g_dt.NowFWLoader = FALSE;
   g_dt.fOpInProgress = TRUE;

   m_StartBtn.EnableWindow(FALSE);
   m_FileDlgBtn.EnableWindow(FALSE);

   m_HistoryCombo.HistoryChanged();
   m_HistoryCombo.EnableWindow(FALSE);

   SetStatusIcon(0, FALSE);

   m_FwInfoStr.SetWindowText(_T("Not avaliable."));
   m_StatusStr.SetWindowText(_T(""));

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
   if(fExit)  //-- Error processing
   {
      AfxMessageBox(s1);

      m_StartBtn.EnableWindow(TRUE);
      m_FileDlgBtn.EnableWindow(TRUE);

      m_HistoryCombo.EnableWindow(TRUE);
      m_HistoryCombo.SetFocus();

      SetStatusIcon(0, TRUE);

      g_dt.fOpInProgress = FALSE;
      return;
   }
 //-------------------------------------------

   g_dt.EnableTabPageChanging = FALSE;
   g_dt.TimerRunInfo = TRUE;
   ::ResetEvent(g_dt.EventUserAbort);

   g_dt.WaitSwitchToLoader = FALSE;

   m_StatusStr.SetWindowText(_T("Check running program ..."));

   OpenDevice();
   DoSetConfiguration();

//--What program is running now in device - boot loader or app

   if(IsFWU()) //--boot loader
   {
      g_dt.NowFWLoader = TRUE;
   }
   else
   {
      m_StatusStr.SetWindowText(_T("Switching to loader ..."));

      cmd_no_data(SWITCH_TO_FWU);

      DWORD rc = wait_for_flag(g_dt.WaitSwitchToLoader,3000);
      if(rc == WAIT_OBJECT_0) //-- OK,continue
      {
         OpenDevice();
         DoSetConfiguration();
      }
      else  //-- Stop and exit
      {
         const char str1[] = "Switching to loader failed.";
         if(g_dt.UsbIo.IsOpen())
            g_dt.UsbIo.Close();  //-- close device

         g_dt.TimerRunInfo = FALSE;

         EraseRunnigPictureRemainder();

         m_StatusStr.SetWindowText(_T(str1));

         SetStatusIcon(IDI_ICON_ERR, TRUE);

         m_HistoryCombo.EnableWindow(TRUE);
         m_StartBtn.EnableWindow(TRUE);
         m_FileDlgBtn.EnableWindow(TRUE);

         g_dt.EnableTabPageChanging = TRUE;
         g_dt.fOpInProgress = FALSE;
         return;

      }
   }

   m_StopBtn.EnableWindow(TRUE);
   m_StatusStr.SetWindowText(_T("Reading in progress..."));

//---- Rx protocol --------------------------------------

   g_RxPipe.BindPipe(0x82);
   g_RxPipe.Start();

   if(cmd_GET_FW_INFO())
   {
      cmd_no_data(GET_FW);
      rc = WaitDevResponse(TIMEOUT_GET_FW);
      if(rc == ERR_RXDATA_OK)      //-- Rx from Device
      {
         if(g_dt.m_RxBuf[0] == FW_RX_RDY)
         {
              //-- Do nothing
         }
         else
            rc = ERR_UNKNOWN_ERR;

      }
   }
   else //-- FW info crc err
      rc = ERR_RXDATA_BADCRC;

   m_rc = rc;

   //--- Final

   g_dt.UsbIo.Close();
   g_dt.DeviceNumber = -1;

   g_RxPipe.Stop();
   g_RxPipe.Close();

   if(g_dt.NowFWLoader == TRUE)
      g_dt.BtnTimeoutCnt = BTN_EN_TIMEOUT_GETFW;

}

//------------------ Stop Btn ------------------------------------------------
void CReadFWDlg::OnBnClickedButtonStopReadfw()
{
  // AfxMessageBox("Stop_Readfw");
   ::SetEvent(g_dt.EventUserAbort);
}

//------------------ File dialog ---------------------------------------------
void CReadFWDlg::OnBnClickedButtonFileReadfw()
{
   const char szFilters[] = "Firmware Contents Files (*.bin)|*.bin|All Files (*.*)|*.*||";
   CFileDialog fileDlg (FALSE, "bin", "*.bin", OFN_HIDEREADONLY | OFN_ENABLESIZING, szFilters, this);

   if(fileDlg.DoModal() == IDOK)
   {
      CString pathName = fileDlg.GetPathName();
      m_HistoryCombo.SetWindowText(pathName);
      m_HistoryCombo.HistoryChanged();
   }
}

//---------------------------------------------------------------------------
LRESULT CReadFWDlg::OnRdFwEnd(WPARAM wpar, LPARAM lpar)
{
   CString s_err, s;

   int icon_id = ErrProc(s_err, m_rc);

   m_StopBtn.EnableWindow(FALSE);
   m_StopBtn.UpdateWindow();      //-- Safe disable

   g_dt.TimerRunInfo = FALSE;

   EraseRunnigPictureRemainder();

   m_FileDlgBtn.EnableWindow(TRUE);
   if(g_dt.hRxFile != NULL)
   {
      fclose(g_dt.hRxFile);
      g_dt.hRxFile = NULL;
   }
   m_HistoryCombo.EnableWindow(TRUE);

   SetStatusIcon(icon_id, TRUE);
   m_StatusStr.SetWindowText(s_err);

   //-- Show FW info
   s.Format("Product_id=%08X  Version=%08X  "
            "Length=%d  CRC=%08XH",
          g_dt.FW.fw_product_id,
          g_dt.FW.fw_version,
          g_dt.FW.fw_length,
          g_dt.FW.fw_crc);

   m_FwInfoStr.SetWindowText(s);

   return 0;
}

//---------------------------------------------------------------------------
void CReadFWDlg::SetStatusIcon(int icon_type, BOOL fShow)
{
   int par = SW_HIDE;
   if(icon_type != 0 && fShow == TRUE)
   {
      if(icon_type == IDI_ICON_ERR)
         m_StatusIcon.SetIcon(m_IconErr);
      else if(icon_type == IDI_ICON_OK)
         m_StatusIcon.SetIcon(m_IconOK);
      else if(icon_type == IDI_ICON_WARN)
         m_StatusIcon.SetIcon(m_IconWarn);
      par = SW_SHOW;
   }
   m_StatusIcon.ShowWindow(par);
}

//----------------------------------------------------------------------------
void CReadFWDlg::EraseRunnigPictureRemainder()
{
   CRect r;

   m_StatusIcon.GetWindowRect(&r);
   ScreenToClient(&r);

   CDC * pDC = GetDC();
   CBrush br(::GetSysColor(COLOR_BTNFACE));
   pDC->FillRect(&r,&br);
   ReleaseDC(pDC);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


