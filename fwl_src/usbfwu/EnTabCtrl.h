//_ **********************************************************
//_
//_ Name: EnTabCtrl.h
//_ Purpose:
//_ Created: 15 September 1998
//_ Author: D.R.Godson
//_ Modified By:
//_
//_ Copyright (c) 1998 Brilliant Digital Entertainment Inc.
//_
//_ **********************************************************

#pragma once
#include "basetabctrl.h"
#include "EnTabPageDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CEnTabCtrl window

// custom look
enum
{
        ETC_FLAT = 1,
        ETC_COLOR = 2, // draw tabs with color
        ETC_SELECTION = 4, // highlight the selected tab
        ETC_GRADIENT = 8, // draw colors with a gradient
        ETC_BACKTABS = 16,
};

class CEnTabCtrl : public CBaseTabCtrl
{
// Construction
public:
        CEnTabCtrl();

        static void EnableCustomLook(BOOL bEnable = TRUE, DWORD dwStyle = ETC_FLAT | ETC_COLOR);

// Attributes
public:

protected:
        static DWORD s_dwCustomLook;

  //----- 
    struct TabDelete 
    {
       CEnTabPageDlg * pTabPage;
                BOOL   bDelete;
    };
    CArray<TabDelete, TabDelete> m_tabs;
	 CArray<HWND, HWND> m_hFocusWnd;
	 CArray<int, int> m_nPageIDs;
  //----- 
	int AddPage (LPCTSTR pszTitle, int nPageID, TabDelete tabDelete);

   virtual BOOL OnInitPage (int nIndex, int nPageID);

public:
   void ResizeDialog (int nIndex, int cx, int cy); 

// Operations
public:

   int AddPageDlg(LPCTSTR pszTitle, int nPageID, LPCTSTR pszTemplateName); 
   int AddPageDlg(LPCTSTR pszTitle, int nPageID, CEnTabPageDlg * pTabPage); 


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CEnTabCtrl)
        protected:
        virtual void PreSubclassWindow();
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CEnTabCtrl();

        // Generated message map functions
protected:
        virtual void DrawMainBorder(LPDRAWITEMSTRUCT lpDrawItemStruct);
        virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
        virtual void DrawItemBorder(LPDRAWITEMSTRUCT lpDrawItemStruct);
        virtual COLORREF GetTabColor(BOOL bSelected = FALSE);
        virtual COLORREF GetTabTextColor(BOOL bSelected = FALSE);


//    virtual BOOL OnInitPage (int nIndex, int nPageID);
	virtual void OnActivatePage (int nIndex, int nPageID);
	virtual void OnDeactivatePage (int nIndex, int nPageID);
	virtual void OnDestroyPage (int nIndex, int nPageID);
	virtual BOOL OnCommand (WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCmdMsg (UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

#ifdef _DEBUG
	BOOL CheckDialogTemplate (LPCTSTR pszTemplateName);
#endif // _DEBUG
//	void ResizeDialog (int nIndex, int cx, int cy);
	// Generated message map functions
protected:
	//{{AFX_MSG(CTabCtrlSSL)
	afx_msg void OnDestroy (void);
	afx_msg void OnSetFocus (CWnd* pOldWnd);
	afx_msg void OnKillFocus (CWnd* pNewWnd);
	afx_msg void OnSelChanging (NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChange (NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////



