#pragma once


//
// Derek Lakin
// "CTabCtrlSSL - An easy to use, flexible extended tab control"
// www.codeproject.com
//

//----------------------------------------------------------------------------
class CEnTabPageDlg : public CDialog
{

	DECLARE_DYNAMIC(CEnTabPageDlg)

   public:


      // Default Constructor
      CEnTabPageDlg();
      // Standard Constructor
      CEnTabPageDlg(UINT nIDTemplate, CWnd* pParent = NULL);

     ~CEnTabPageDlg();

        // Enable/disable command routing to the parent.
      void EnableRouteCommand(bool bRoute = true)
      {
         m_bRouteCommand = bRoute;
      }
      bool IsRouteCommand()
      {
         return m_bRouteCommand;
      }
        // Enable CmdMsg routing to the parent.
      void EnableRouteCmdMsg(bool bRoute = true)
      {
         m_bRouteCmdMsg = bRoute;
      }
      bool IsRouteCmdMsg()
      {
         return m_bRouteCmdMsg;
      };
        // Enable/Disable Notify routing to the parent.
      void EnableRouteNotify(bool bRoute = true)
      {
         m_bRouteNotify = bRoute;
      };
      bool IsRouteNotify()
      {
         return m_bRouteNotify;
      };

      void ItemEnabled(int item, BOOL mode);

protected:

// Message Handlers
      virtual BOOL OnCommand (WPARAM wParam, LPARAM lParam);
      virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult);
      virtual void OnOK (void);
      virtual void OnCancel (void);
      virtual BOOL OnCmdMsg (UINT nID, int nCode, void* pExtra,
                                         AFX_CMDHANDLERINFO* pHandlerInfo);

// Routing flags
      bool m_bRouteCommand;
      bool m_bRouteCmdMsg;
      bool m_bRouteNotify;
};




