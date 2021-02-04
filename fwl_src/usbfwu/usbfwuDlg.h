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


// usbfwuDlg.h : header file
//

#pragma once

#include "EnTabCtrl.h"
#include "ReadFWDlg.h"
#include "WriteFWDlg.h"
#include "RegOpDlg.h"
#include "AboutTabDlg.h"

#include "fwu_types.h"
#include "TxStreamPipe.h"
#include "RxPipe.h"
#include "AnimBmp.h"

// CusbfwuDlg dialog
class CusbfwuDlg : public CDialog
{
// Construction
public:
    CusbfwuDlg(CWnd* pParent = NULL);        // standard constructor
   ~CusbfwuDlg();


// Dialog Data
   enum { IDD = IDD_TN_USB_FW_UPGR_1_DIALOG };

   CEnTabCtrl  m_TabCtrl;

    //-- Tabs dialogs
   CReadFWDlg   m_ReadFWDlg;
   CWriteFWDlg  m_WriteFWDlg;
   CRegOpDlg    m_RepOpDlg;
   CAboutTabDlg m_AboutTabDlg;

   HICON m_IconWarn;
   CBitmap m_AnimBmp;
   CAnimBmp m_AnimCtrl;
   CRect m_rectConnStr;

  // m_advancedTab.Create (IDD_TAB_ADVANCED, this);

   BOOL Init(GLODATA * pGD);
   void UpdateDeviceList(GLODATA * pGD);
   void ShowDevConn(BOOL fShow);
   BOOLEAN RegisterDevNotify(const GUID *InterfaceClassGuid,
                                   PVOID *hDevNotify);
   void OwWrFwEnd();

protected:
   virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support


// Implementation
protected:
   HICON m_hIcon;

   HDEVNOTIFY m_DevNotify;
   int m_cnt;

        // Generated message map functions
   virtual BOOL OnInitDialog();
   afx_msg void OnPaint();
   afx_msg HCURSOR OnQueryDragIcon();
   afx_msg LRESULT OnTabChange(WPARAM wpar, LPARAM lpar);
   afx_msg LRESULT OnMessageThreadExit(WPARAM wpar, LPARAM lpar);
   afx_msg void OnDestroy();
   afx_msg void OnTimer(UINT_PTR nIDEvent);
   afx_msg BOOL OnDeviceChange( UINT nEventType, DWORD dwData );

   DECLARE_MESSAGE_MAP()

};


