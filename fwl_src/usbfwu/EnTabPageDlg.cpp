#include "stdafx.h"
#include "EnTabPageDlg.h"


//
// Derek Lakin
// "CTabCtrlSSL - An easy to use, flexible extended tab control"
// www.codeproject.com
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Construction

IMPLEMENT_DYNAMIC(CEnTabPageDlg, CDialog)

//----------------------------------------------------------------------------
CEnTabPageDlg::CEnTabPageDlg ()
{

#ifndef _AFX_NO_OCC_SUPPORT
   AfxEnableControlContainer ();
#endif // !_AFX_NO_OCC_SUPPORT

   m_bRouteCommand = false;
   m_bRouteCmdMsg = false;
   m_bRouteNotify = false;
}

//----------------------------------------------------------------------------
CEnTabPageDlg::CEnTabPageDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
                                : CDialog(nIDTemplate, pParent)
{
#ifndef _AFX_NO_OCC_SUPPORT
   AfxEnableControlContainer ();
#endif // !_AFX_NO_OCC_SUPPORT

   m_bRouteCommand = false;
   m_bRouteCmdMsg = false;
   m_bRouteNotify = false;
}

/////////////////////////////////////////////////////////////////////////////
//-------

//----------------------------------------------------------------------------
CEnTabPageDlg::~CEnTabPageDlg()
{
}

//---------------- Message Handlers -----------------------------------------

//----------------------------------------------------------------------------
void CEnTabPageDlg::OnOK(void)
{
  //-- Prevent CDialog::OnOK from calling EndDialog.
}

//----------------------------------------------------------------------------
void CEnTabPageDlg::OnCancel(void)
{
  //-- Prevent CDialog::OnCancel from calling EndDialog.
}

//----------------------------------------------------------------------------
BOOL CEnTabPageDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
        // Call base class OnCommand to allow message map processing
   BOOL bReturn = CDialog::OnCommand (wParam, lParam);

   if(true == m_bRouteCommand)
   {
                // Forward WM_COMMAND messages to the dialog's parent.
      return GetParent ()->SendMessage (WM_COMMAND, wParam, lParam);
   }
   return bReturn;
}

//----------------------------------------------------------------------------
BOOL CEnTabPageDlg::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
   BOOL bReturn = CDialog::OnNotify(wParam, lParam, pResult);

   if(true == m_bRouteNotify)
   {
      // Forward WM_NOTIFY messages to the dialog's parent.
      return GetParent ()->SendMessage (WM_NOTIFY, wParam, lParam);
   }

   return bReturn;
}

//----------------------------------------------------------------------------
BOOL CEnTabPageDlg::OnCmdMsg (UINT nID, int nCode, void* pExtra,
                                              AFX_CMDHANDLERINFO* pHandlerInfo)
{
   BOOL bReturn = CDialog::OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);

#ifndef _AFX_NO_OCC_SUPPORT
   if(true == m_bRouteCmdMsg)
   {
     // Forward ActiveX control events to the dialog's parent.

      if(nCode == CN_EVENT)
         return GetParent()->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);
   }
#endif // !_AFX_NO_OCC_SUPPORT

   return bReturn;
}

//----------------------------------------------------------------------------
void CEnTabPageDlg::ItemEnabled(int item,BOOL mode)
{
   CWnd* pWnd = GetDlgItem(item);
   pWnd->EnableWindow(mode);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

