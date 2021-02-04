//_ **********************************************************
//_
//_ Name: EnTabCtrl.cpp
//_ Purpose:
//_ Created: 15 September 1998
//_ Author: D.R.Godson
//_ Modified By:
//_
//_ Copyright (c) 1998 Brilliant Digital Entertainment Inc.
//_
//_ **********************************************************

// EnTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "EnTabPageDlg.h"

#include "EnTabCtrl.h"

#include "fwu_types.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnTabCtrl

DWORD CEnTabCtrl::s_dwCustomLook = 0;

enum { PADDING = 3, EDGE = 20};

//////////////////////////////////////////////////////////////////////////////
// helpers
COLORREF Darker(COLORREF crBase, float fFactor)
{
        ASSERT (fFactor < 1.0f && fFactor > 0.0f);

        fFactor = min(fFactor, 1.0f);
        fFactor = max(fFactor, 0.0f);

        BYTE bRed, bBlue, bGreen;
        BYTE bRedShadow, bBlueShadow, bGreenShadow;

        bRed = GetRValue(crBase);
        bBlue = GetBValue(crBase);
        bGreen = GetGValue(crBase);

        bRedShadow = (BYTE)(bRed * fFactor);
        bBlueShadow = (BYTE)(bBlue * fFactor);
        bGreenShadow = (BYTE)(bGreen * fFactor);

        return RGB(bRedShadow, bGreenShadow, bBlueShadow);
}

COLORREF Lighter(COLORREF crBase, float fFactor)
{
        ASSERT (fFactor > 1.0f);

        fFactor = max(fFactor, 1.0f);

        BYTE bRed, bBlue, bGreen;
        BYTE bRedHilite, bBlueHilite, bGreenHilite;

        bRed = GetRValue(crBase);
        bBlue = GetBValue(crBase);
        bGreen = GetGValue(crBase);

        bRedHilite = (BYTE)min((int)(bRed * fFactor), 255);
        bBlueHilite = (BYTE)min((int)(bBlue * fFactor), 255);
        bGreenHilite = (BYTE)min((int)(bGreen * fFactor), 255);

        return RGB(bRedHilite, bGreenHilite, bBlueHilite);
}

CSize FormatText(CString& sText, CDC* pDC, int nWidth)
{
        CRect rect(0, 0, nWidth, 20);
        UINT uFlags = DT_CALCRECT | DT_SINGLELINE | DT_MODIFYSTRING | DT_END_ELLIPSIS;

        ::DrawText(pDC->GetSafeHdc(), sText.GetBuffer(sText.GetLength() + 4), -1, rect, uFlags);
        sText.ReleaseBuffer();

        return pDC->GetTextExtent(sText);
}

////////////////////////////////////////////////////////////////////////////////////////

CEnTabCtrl::CEnTabCtrl()
{
}

CEnTabCtrl::~CEnTabCtrl()
{
}


BEGIN_MESSAGE_MAP(CEnTabCtrl, CBaseTabCtrl)
	//{{AFX_MSG_MAP(CEnTabCtrl)
	ON_WM_DESTROY ()
	ON_WM_SETFOCUS ()
	ON_WM_KILLFOCUS ()
	ON_NOTIFY_REFLECT (TCN_SELCHANGING, OnSelChanging)
	ON_NOTIFY_REFLECT (TCN_SELCHANGE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CEnTabCtrl message handlers

void CEnTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
        TC_ITEM     tci;
        CDC* pDC = CDC::FromHandle(lpdis->hDC);
        HIMAGELIST hilTabs = (HIMAGELIST)TabCtrl_GetImageList(GetSafeHwnd());

        BOOL bSelected = (lpdis->itemID == (UINT)GetCurSel());
        BOOL bColor = (s_dwCustomLook & ETC_COLOR);

        CRect rItem(lpdis->rcItem);

        if (bSelected)
                rItem.bottom -= 1;
        else
                rItem.bottom += 2;

        // tab
        // blend from back color to COLOR_3DFACE if 16 bit mode or better
        COLORREF crFrom = GetTabColor(bSelected);

        if (bSelected && (s_dwCustomLook & ETC_GRADIENT) && pDC->GetDeviceCaps(BITSPIXEL) >= 16)
        {
                COLORREF crTo = bSelected ? ::GetSysColor(COLOR_3DFACE) : Darker(!bColor || m_crBack == -1 ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 0.7f);

                int nROrg = GetRValue(crFrom);
                int nGOrg = GetGValue(crFrom);
                int nBOrg = GetBValue(crFrom);
                int nRDiff = GetRValue(crTo) - nROrg;
                int nGDiff = GetGValue(crTo) - nGOrg;
                int nBDiff = GetBValue(crTo) - nBOrg;

                int nHeight = rItem.Height();

                for (int nLine = 0; nLine < nHeight; nLine += 2)
                {
                        int nRed = nROrg + (nLine * nRDiff) / nHeight;
                        int nGreen = nGOrg + (nLine * nGDiff) / nHeight;
                        int nBlue = nBOrg + (nLine * nBDiff) / nHeight;

                        pDC->FillSolidRect(CRect(rItem.left, rItem.top + nLine, rItem.right, rItem.top + nLine + 2),
                                                                RGB(nRed, nGreen, nBlue));
                }
        }
        else // simple solid fill
                pDC->FillSolidRect(rItem, crFrom);

        // text & icon
        rItem.left += PADDING;
        rItem.top += PADDING + (bSelected ? 1 : 0);

        pDC->SetBkMode(TRANSPARENT);

        CString sTemp;
        tci.mask        = TCIF_TEXT | TCIF_IMAGE;
        tci.pszText     = sTemp.GetBuffer(100);
        tci.cchTextMax  = 99;
        GetItem(lpdis->itemID, &tci);
        sTemp.ReleaseBuffer();

        // icon
        if (hilTabs)
        {
                ImageList_Draw(hilTabs, tci.iImage, *pDC, rItem.left, rItem.top, ILD_TRANSPARENT);
                rItem.left += 16 + PADDING;
        }

        // text
    rItem.right -= PADDING;
//-- My
//        FormatText(sTemp, pDC, rItem.Width());
        //if(!hilTabs)
    //  rItem.left -= 16;

        pDC->SetTextColor(GetTabTextColor(bSelected));
        if(hilTabs)
           pDC->DrawText(sTemp, rItem, DT_NOPREFIX | DT_CENTER);
    else
       pDC->DrawText(sTemp, rItem, DT_NOPREFIX | DT_LEFT);
}

void CEnTabCtrl::DrawItemBorder(LPDRAWITEMSTRUCT lpdis)
{
        ASSERT (s_dwCustomLook & ETC_FLAT);

        BOOL bSelected = (lpdis->itemID == (UINT)GetCurSel());
        BOOL bBackTabs = (s_dwCustomLook & ETC_BACKTABS);

        CRect rItem(lpdis->rcItem);
        CDC* pDC = CDC::FromHandle(lpdis->hDC);

        COLORREF crTab = GetTabColor(bSelected);
//        COLORREF crHighlight = Lighter(crTab, 1.5f);
        COLORREF crShadow = Darker(crTab, 0.75f);
    COLORREF crHighlight = ::GetSysColor(COLOR_3DHILIGHT);
//        COLORREF crShadow = ::GetSysColor(COLOR_3DSHADOW);

        if (bSelected || bBackTabs)
        {
                rItem.bottom += bSelected ? -1 : 1;

                // edges
                pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.left + 1, rItem.bottom), crHighlight);
                pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.right, rItem.top + 1), crHighlight);
                pDC->FillSolidRect(CRect(rItem.right - 1, rItem.top, rItem.right, rItem.bottom), crShadow);
        }
        else // draw simple dividers
        {
                pDC->FillSolidRect(CRect(rItem.left - 1, rItem.top, rItem.left, rItem.bottom), crShadow);
                pDC->FillSolidRect(CRect(rItem.right - 1, rItem.top, rItem.right, rItem.bottom), crShadow);
        }
}

void CEnTabCtrl::DrawMainBorder(LPDRAWITEMSTRUCT lpdis)
{
        CRect rBorder(lpdis->rcItem);
        CDC* pDC = CDC::FromHandle(lpdis->hDC);

        COLORREF crTab = GetTabColor();
//        COLORREF crHighlight = Lighter(crTab, 1.5f);
//        COLORREF crShadow = Darker(crTab, 0.75f);
    COLORREF crHighlight = ::GetSysColor(COLOR_3DHILIGHT);
        COLORREF crShadow = ::GetSysColor(COLOR_3DSHADOW);

    pDC->Draw3dRect(rBorder, crHighlight, crShadow);
}

COLORREF CEnTabCtrl::GetTabColor(BOOL bSelected)
{
        BOOL bColor = (s_dwCustomLook & ETC_COLOR);
        BOOL bHiliteSel = (s_dwCustomLook & ETC_SELECTION);
        BOOL bBackTabs = (s_dwCustomLook & ETC_BACKTABS);
        BOOL bFlat = (s_dwCustomLook & ETC_FLAT);

        if (bSelected && bHiliteSel)
        {
                if (bColor)
                        return Lighter((m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 1.2f);
                else
                        return Lighter(::GetSysColor(COLOR_3DFACE), 1.2f);
        }
        else if (!bSelected)
        {
                if (bBackTabs || !bFlat)
                {
                        if (bColor)
                                return Darker((m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 0.9f);
                        else
                                return Darker(::GetSysColor(COLOR_3DFACE), 0.9f);
                }
                else
                        return (m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack;
        }

        // else
        return ::GetSysColor(COLOR_3DFACE);
}

COLORREF CEnTabCtrl::GetTabTextColor(BOOL bSelected)
{
        BOOL bColor = (s_dwCustomLook & ETC_COLOR);
        BOOL bFlat = (s_dwCustomLook & ETC_FLAT);

        if (bSelected)
        {
                return ::GetSysColor(COLOR_WINDOWTEXT);
        }
        else
        {
                if (bColor || bFlat)
                        return Darker((m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 0.5f);
                else
                        return Darker(::GetSysColor(COLOR_3DFACE), 0.5f);
        }

        // else
        return Darker(::GetSysColor(COLOR_3DFACE), 0.5f);
}

void CEnTabCtrl::EnableCustomLook(BOOL bEnable, DWORD dwStyle)
{
        if (!bEnable)
                dwStyle = 0;

        s_dwCustomLook = dwStyle;
}

void CEnTabCtrl::PreSubclassWindow()
{
        CBaseTabCtrl::PreSubclassWindow();

        if (s_dwCustomLook)
                ModifyStyle(0, TCS_OWNERDRAWFIXED);
}

//----------------------------------------------------------------------------
int CEnTabCtrl::AddPage(LPCTSTR pszTitle, int nPageID, TabDelete tabDelete) 
{
   // Add a page to the tab control.
   TC_ITEM item;
   item.mask = TCIF_TEXT;
   item.pszText = (LPTSTR) pszTitle;
   int nIndex = GetItemCount ();

   if(InsertItem (nIndex, &item) == -1)
      return -1;

   if(NULL == tabDelete.pTabPage) 
   {
      // Fail - no point calling the function with a NULL pointer!
      DeleteItem (nIndex);
      return -1;
   }
   else 
   {
      // Record the address of the dialog object and the page ID.
      int nArrayIndex = m_tabs.Add (tabDelete);
      ASSERT (nIndex == nArrayIndex);

      nArrayIndex = m_nPageIDs.Add (nPageID);
      ASSERT (nIndex == nArrayIndex);

      // Size and position the dialog box within the view.
      tabDelete.pTabPage->SetParent (this); // Just to be sure

      CRect rect;
      GetClientRect (&rect);

      if(rect.Width () > 0 && rect.Height () > 0)
         ResizeDialog (nIndex, rect.Width(), rect.Height());

      // Initialize the page.
      if(OnInitPage (nIndex, nPageID)) 
      {
         // Make sure the first control in the dialog is the one that
         // receives the input focus when the page is displayed.
         HWND hwndFocus = tabDelete.pTabPage->GetTopWindow()->m_hWnd;
         nArrayIndex = m_hFocusWnd.Add(hwndFocus);
         ASSERT (nIndex == nArrayIndex);
      }
      else 
      {
         // Make the control that currently has the input focus is the one
         // that receives the input focus when the page is displayed.
         m_hFocusWnd.Add (::GetFocus());
      }

      // If this is the first page added to the view, make it visible.
      if(nIndex == 0)
         tabDelete.pTabPage->ShowWindow(SW_SHOW);
   }
   return nIndex;
}

//----------------------------------------------------------------------------
void CEnTabCtrl::ResizeDialog (int nIndex, int cx, int cy) 
{
   if(nIndex != -1) 
   {
      TabDelete tabDelete = m_tabs[nIndex];
      CEnTabPageDlg * pDialog = tabDelete.pTabPage;

      if(pDialog != NULL) 
      {
         CRect rect;
         GetItemRect (nIndex, &rect);

         int x, y, nWidth, nHeight;
         DWORD dwStyle = GetStyle();

         if (dwStyle & TCS_VERTICAL) 
         { // Vertical tabs
            int nTabWidth = rect.Width () * GetRowCount ();
            x = (dwStyle & TCS_RIGHT) ? 4 : nTabWidth + 4;
            y = 4;
            nWidth = cx - nTabWidth - 8;
            nHeight = cy - 8;
         }
         else 
         { // Horizontal tabs
            int nTabHeight =
            rect.Height () * GetRowCount ();
            x = 4;
            y = (dwStyle & TCS_BOTTOM) ? 4 : nTabHeight + 4;
            nWidth = cx - 8;
            nHeight = cy - nTabHeight - 8;
         }
         pDialog->SetWindowPos (NULL, x, y, nWidth, nHeight, SWP_NOZORDER);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
// Overridables

BOOL CEnTabCtrl::OnInitPage (int nIndex, int nPageID) 
{
	// TODO: Override in derived class to initialise pages.
	return TRUE;
}

void CEnTabCtrl::OnActivatePage (int nIndex, int nPageID) 
{
	// TODO: Override in derived class to respond to page activations.
}

void CEnTabCtrl::OnDeactivatePage (int nIndex, int nPageID) 
{
	// TODO: Override in derived class to respond to page deactivations.
}

void CEnTabCtrl::OnDestroyPage (int nIndex, int nPageID) 
{
	// TODO: Override in derived class to free resources.
}



//----------------------------------------------------------------------------
int CEnTabCtrl::AddPageDlg (LPCTSTR pszTitle, int nPageID, CEnTabPageDlg * pTabPage) 
{
	// Add a page to the tab control.

   TabDelete tabDelete;
   tabDelete.pTabPage = pTabPage;
   tabDelete.bDelete = FALSE;

   return AddPage (pszTitle, nPageID, tabDelete);
}

//----------------------------------------------------------------------------
int CEnTabCtrl::AddPageDlg (LPCTSTR pszTitle, int nPageID, LPCTSTR pszTemplateName) 
{
	// Verify that the dialog template is compatible with CTabCtrlSSL
	// (debug builds only). If your app asserts here, make sure the dialog
	// resource you're adding to the view is a borderless child window and
	// is not marked visible.
/*
#ifdef _DEBUG
	if (pszTemplateName != NULL) 
   {
		BOOL bResult = CheckDialogTemplate (pszTemplateName);
		ASSERT (bResult);
	}
#endif // _DEBUG
*/
	// Add a page to the tab control.
	// Create a modeless dialog box.

	CEnTabPageDlg * pDialog = new CEnTabPageDlg;

	if(pDialog == NULL) 
		return -1;

	if (!pDialog->Create (pszTemplateName, this)) 
   {
		pDialog->DestroyWindow ();
		delete pDialog;
		return -1;
	}
    
    TabDelete tabDelete;
    tabDelete.pTabPage = pDialog;
    tabDelete.bDelete = TRUE;

    return AddPage (pszTitle, nPageID, tabDelete);
}


/////////////////////////////////////////////////////////////////////////////
// Message handlers

void CEnTabCtrl::OnSelChanging (NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Notify derived classes that the selection is changing.
	int nIndex = GetCurSel ();
	if (nIndex == -1)
		return;

   //--- Check if changing is enabled
   BOOL fEnable = (BOOL)::SendMessage(::GetParent(m_hWnd),   
                         CM_TAB_CHANGE,  
                         (WPARAM) 1,     
                         (LPARAM) 0);    
   if(!fEnable) 
   {
	   *pResult = 1; //-- Disable tabs changing
      return;
   }
	OnDeactivatePage (nIndex, m_nPageIDs[nIndex]);

	// Save the input focus and hide the old page.
    TabDelete tabDelete = m_tabs[nIndex];
    CEnTabPageDlg * pDialog = tabDelete.pTabPage;

	if (pDialog != NULL) 
   {
		m_hFocusWnd[nIndex] = ::GetFocus ();
		pDialog->ShowWindow (SW_HIDE);
	}

  // CBaseTabCtrl::OnSelChanging(pNMHDR, pResult); 

	//*pResult = 0;
}

void CEnTabCtrl::OnSelChange (NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nIndex = GetCurSel ();
	if (nIndex == -1)
		return;

	// Show the new page.
    TabDelete tabDelete = m_tabs[nIndex];
    CEnTabPageDlg * pDialog = tabDelete.pTabPage;

	if (pDialog != NULL) 
   {
		::SetFocus (m_hFocusWnd[nIndex]);
		CRect rect;
		GetClientRect (&rect);
		ResizeDialog (nIndex, rect.Width (), rect.Height ());
		pDialog->ShowWindow (SW_SHOW);
	}

	// Notify derived classes that the selection has changed.
	OnActivatePage (nIndex, m_nPageIDs[nIndex]);

   ::SendMessage(::GetParent(m_hWnd),   
                        CM_TAB_CHANGE,  
                        (WPARAM) 0,     
                        (LPARAM) 0);    

	*pResult = 0;
}

void CEnTabCtrl::OnSetFocus (CWnd* pOldWnd) 
{
	CTabCtrl::OnSetFocus (pOldWnd);
	
	// Set the focus to a control on the current page.
	int nIndex = GetCurSel ();
	if (nIndex != -1)
		::SetFocus (m_hFocusWnd[nIndex]);	
}

void CEnTabCtrl::OnKillFocus (CWnd* pNewWnd) 
{
	CTabCtrl::OnKillFocus (pNewWnd);
	
	// Save the HWND of the control that holds the input focus.
	int nIndex = GetCurSel ();
	if (nIndex != -1)
		m_hFocusWnd[nIndex] = ::GetFocus ();	
}

// My thanks to Tomasz Sowinski for all his help coming up with a workable
// solution to the stack versus heap object destruction
void CEnTabCtrl::OnDestroy (void) 
{
	int nCount = m_tabs.GetSize ();

	// Destroy dialogs and delete CTabCtrlSSL objects.
	if (nCount > 0) 
   {
		for (int i=nCount - 1; i>=0; i--) 
      {
			OnDestroyPage (i, m_nPageIDs[i]);
         TabDelete tabDelete = m_tabs[i];

         CEnTabPageDlg * pDialog = tabDelete.pTabPage;
			if (pDialog != NULL) 
         {
				pDialog->DestroyWindow ();
                if (TRUE == tabDelete.bDelete) 
                {
                    delete pDialog;
                }
			}
		}
	}

	// Clean up the internal arrays.
	m_tabs.RemoveAll ();
	m_hFocusWnd.RemoveAll ();
	m_nPageIDs.RemoveAll ();

	CTabCtrl::OnDestroy ();
}

BOOL CEnTabCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// Forward WM_COMMAND messages to the dialog's parent.
	return GetParent ()->SendMessage (WM_COMMAND, wParam, lParam);
}

BOOL CEnTabCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// Forward WM_NOTIFY messages to the dialog's parent.
	return GetParent ()->SendMessage (WM_NOTIFY, wParam, lParam);
}

BOOL CEnTabCtrl::OnCmdMsg(UINT nID, int nCode, void* pExtra,
                                      AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// Forward ActiveX control events to the dialog's parent.
#ifndef _AFX_NO_OCC_SUPPORT
	if(nCode == CN_EVENT)
		return GetParent ()->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);
#endif // !_AFX_NO_OCC_SUPPORT

	return CTabCtrl::OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------





