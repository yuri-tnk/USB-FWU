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

// WriteFWDlg.cpp : implementation file
//

#include "stdafx.h"
#include "usbfwu.h"
#include "HistoryComboBox.h"
#include "fwu_utils.h"
#include "fwu_cmd.h"

#include "WriteFWDlg.h"

#include "fwu_globals.h"

// CWriteFWDlg dialog

IMPLEMENT_DYNAMIC(CWriteFWDlg, CEnTabPageDlg)

CWriteFWDlg::CWriteFWDlg(CWnd* pParent /*=NULL*/)
        : CEnTabPageDlg(CWriteFWDlg::IDD, pParent)
{
   m_rc = 0;
}

CWriteFWDlg::~CWriteFWDlg()
{
}

void CWriteFWDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);

   DDX_Control(pDX, IDC_BUTTON_START_WRITEFW, m_StartBtn);
   DDX_Control(pDX, IDC_BUTTON_STOP_WRITEFW, m_StopBtn);
   DDX_Control(pDX, IDC_BUTTON_FILE_WRITEFW, m_FileDlgBtn);
   DDX_Control(pDX, IDC_STATIC_FW_INFO_WRITEFW, m_FwInfoStr);
   DDX_Control(pDX, IDC_STATIC_STATUSTEXT_WRITEFW, m_StatusStr);
   DDX_Control(pDX, IDC_STATIC_STATUSICON_WRITEFW, m_StatusIcon);
}


BEGIN_MESSAGE_MAP(CWriteFWDlg, CEnTabPageDlg)
   ON_BN_CLICKED(IDC_BUTTON_FILE_WRITEFW, &CWriteFWDlg::OnBnClickedButtonFileWritefw)
   ON_BN_CLICKED(IDC_BUTTON_START_WRITEFW, &CWriteFWDlg::OnBnClickedButtonStartWritefw)
   ON_BN_CLICKED(IDC_BUTTON_STOP_WRITEFW, &CWriteFWDlg::OnBnClickedButtonStopWritefw)
   ON_MESSAGE(CM_END_OP, OnWrFwEnd)
END_MESSAGE_MAP()


// CWriteFWDlg message handlers

//----------------------------------------------------------------------------
BOOL CWriteFWDlg::OnInitDialog()
{
   CEnTabPageDlg::OnInitDialog();

   CString RegPath = CString(g_FilesRegAddr) + CString(_T("FilesLoadFrom"));
   CreateHisoryComboBox(this, &m_HistoryCombo, RegPath,
                          IDC_COMBO_PLACEHOLDER_WRITEFW,
                          IDC_HCOMBO_WRITEFW);
   g_dt.WrDlg = this;

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

//------------------ File dialog ---------------------------------------------
void CWriteFWDlg::OnBnClickedButtonFileWritefw()
{
   //   const char szFilters[] = "Intel Hex Files (*.hex)|*.hex|Motorola Hex Files (*.srec)|*.srec|ELF files (*.elf)|*.elf|All Files (*.*)|*.*||";
   const char szFilters[] = "Intel Hex Files (*.hex)|*.hex|All Files (*.*)|*.*||";
   CFileDialog fileDlg (TRUE, NULL, "*.hex", OFN_HIDEREADONLY | OFN_ENABLESIZING, szFilters, this);

   if(fileDlg.DoModal() == IDOK)
   {
      CString pathName = fileDlg.GetPathName();
      m_HistoryCombo.SetWindowText(pathName);
      m_HistoryCombo.HistoryChanged();
   }
}

//------------------ Start Btn -----------------------------------------------
void CWriteFWDlg::OnBnClickedButtonStartWritefw()
{
//   AfxMessageBox("Start_WriteFW");

   BOOL fExit = FALSE;
   CString s, s1;
   

   DWORD rc;
   BYTE cmd;

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
         int filelen;
         BOOL rc = LoadHexFile(s,&filelen);
         if(rc == TRUE)
         {
            //-- Prepare FW info
            m_FW.fw_product_id = *((unsigned __int32 *)&g_dt.FWBuf[FW_ROM_PRODUCT_ID_OFFSET]);
            m_FW.fw_version    = *((unsigned __int32 *)&g_dt.FWBuf[FW_ROM_VERSION_OFFSET]);
            m_FW.fw_length     = filelen;
            m_FW.fw_crc        = calc_crc(g_dt.FWBuf + FW_START_OFFSET, filelen - FW_START_OFFSET);
            //-- Size aligned to STREAM_BUF_SIZE
            g_dt.FW.len = ((filelen + (STREAM_BUF_SIZE-1))/STREAM_BUF_SIZE) * STREAM_BUF_SIZE;
            g_dt.FW.buf_ptr = g_dt.FWBuf;
         }
         else
         {
            s1 = _T("");
            fExit = TRUE;
         }
      }
      break;
   }
   if(fExit)  //-- Error processing
   {
      if(s1.GetLength() != 0)
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

      DWORD rc = wait_for_flag(g_dt.WaitSwitchToLoader, 3000);
      if(rc == WAIT_OBJECT_0) //-- OK,continue
      {
         OpenDevice();
         DoSetConfiguration();
      }
      else  //-- Stop and exit
      {
         const char str1[] = "Switching to loader failed.";
         if(g_dt.UsbIo.IsOpen())
            g_dt.UsbIo.Close();

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

   m_StatusStr.SetWindowText(_T("Loading in progress..."));

//---- Tx protocol --------------------------------------

   ::ResetEvent(g_dt.EventTxStream);

   g_TxStreamPipe.BindPipe(0x02);
   g_TxStreamPipe.ResetPipeStatistics();
   g_TxStreamPipe.SetupPipeStatistics(MEANRATE_AVERAGING_INTERVAL_MS);
   g_TxStreamPipe.Start();

   g_RxPipe.BindPipe(0x82);
   g_RxPipe.Start();

   cmd_PUT_FW_INFO(&m_FW);
   rc = WaitDevResponse(TIMEOUT_PUT_FW_INFO);
   if(rc == ERR_RXDATA_OK)      //-- Rx from Device
   {
      cmd = g_dt.m_RxBuf[0];
      if(cmd == PUT_FW_INFO_ASK)
      {
        //-- enable tx stream
         ::Sleep(50);                  //-- For worst case in microprocessor delays
         ::SetEvent(g_dt.EventTxStream);  //-- Enable data tx

         rc = WaitDevResponse(TIMEOUT_WR_FW);
         if(rc == ERR_RXDATA_OK)      //-- Rx from Device
         {
            cmd =  g_dt.m_RxBuf[0];
            if(cmd == FW_WR_RDY)
            {
              //-- Do nothing
            }
            else if(cmd == FW_WR_ERR)
            {
               rc = ERR_WR_ERR;
            }
         }
      }
   }

   m_rc = rc;

   if(g_dt.NowFWLoader == TRUE)
      g_dt.BtnTimeoutCnt = BTN_EN_TIMEOUT_PUTFW;

   //--- Final

   g_dt.UsbIo.Close();
   g_dt.DeviceNumber = -1;

   g_RxPipe.Stop();
   g_RxPipe.Close();

   g_TxStreamPipe.Stop();
   g_TxStreamPipe.Close();

  ::ResetEvent(g_dt.EventTxStream);
}

//------------------ Stop Btn ------------------------------------------------
void CWriteFWDlg::OnBnClickedButtonStopWritefw()
{
  // AfxMessageBox("Stop_WriteFW");
   ::SetEvent(g_dt.EventUserAbort);
}

//---------------------------------------------------------------------------
LRESULT CWriteFWDlg::OnWrFwEnd(WPARAM wpar, LPARAM lpar)
{
   CString s_err, s;

   int icon_id = ErrProc(s_err, m_rc);

   m_StopBtn.EnableWindow(FALSE);
   m_StopBtn.UpdateWindow();      //-- Safe disable

   g_dt.TimerRunInfo = FALSE;

   EraseRunnigPictureRemainder();

   m_FileDlgBtn.EnableWindow(TRUE);
   m_HistoryCombo.EnableWindow(TRUE);

   SetStatusIcon(icon_id, TRUE);
   m_StatusStr.SetWindowText(s_err);

     //-- Show FW info
   s.Format("Product_id=%08X  Version=%08X  "
               "Length=%d  CRC=%08XH",
             m_FW.fw_product_id,
             m_FW.fw_version,
             m_FW.fw_length,
             m_FW.fw_crc);

   m_FwInfoStr.SetWindowText(s);

   return 0;
}

//---------------------------------------------------------------------------
void CWriteFWDlg::SetStatusIcon(int icon_type, BOOL fShow)
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
void CWriteFWDlg::EraseRunnigPictureRemainder()
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
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



