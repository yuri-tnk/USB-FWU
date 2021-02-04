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


#pragma once

#include "EnTabPageDlg.h"
#include "resource.h"
#include "HistoryComboBox.h"
#include "afxwin.h"


// CReadFWDlg dialog

class CReadFWDlg : public CEnTabPageDlg
{
    DECLARE_DYNAMIC(CReadFWDlg)

public:
   CReadFWDlg(CWnd* pParent = NULL);   // standard constructor
   virtual ~CReadFWDlg();

   CHistoryComboBox m_HistoryCombo;
   CButton m_StartBtn;
   CButton m_StopBtn;
   CButton m_FileDlgBtn;
   CStatic m_FwInfoStr;
   CStatic m_StatusStr;
   CStatic m_StatusIcon;

   HICON m_IconErr;
   HICON m_IconOK;
   HICON m_IconWarn;

   int m_rc;

   void SetStatusIcon(int icon_type, BOOL fShow);
   void EraseRunnigPictureRemainder();

// Dialog Data
   enum { IDD = IDD_DIALOG_READFW };

protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   DECLARE_MESSAGE_MAP()

public:
   afx_msg void OnBnClickedButtonFileReadfw();
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedButtonStartReadfw();
   afx_msg void OnBnClickedButtonStopReadfw();
   afx_msg LRESULT OnRdFwEnd(WPARAM wpar, LPARAM lpar);

};
