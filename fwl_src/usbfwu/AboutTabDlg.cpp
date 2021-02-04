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


// AboutTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "usbfwu.h"
#include "EnTabPageDlg.h"
#include "hlstatic.h"

#include "AboutTabDlg.h"

#include "fwu_globals.h"

// CAboutTabDlg dialog

IMPLEMENT_DYNAMIC(CAboutTabDlg, CEnTabPageDlg)

CAboutTabDlg::CAboutTabDlg(CWnd* pParent /*=NULL*/)
        : CEnTabPageDlg(CAboutTabDlg::IDD, pParent)
{

}

CAboutTabDlg::~CAboutTabDlg()
{
}

void CAboutTabDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CAboutTabDlg)
        DDX_Control(pDX, IDC_LABEL_WEB, m_HlStatic);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutTabDlg, CEnTabPageDlg)

END_MESSAGE_MAP()


// CAboutTabDlg message handlers

BOOL CAboutTabDlg::OnInitDialog()
{
   CEnTabPageDlg::OnInitDialog();
     
   g_dt.AboutDlg = this;

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}
