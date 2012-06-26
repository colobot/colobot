#if !defined(AFX_BOTCONSOLEDLG_H__A11450A2_8E09_11D4_A439_00D059085115__INCLUDED_)
#define AFX_BOTCONSOLEDLG_H__A11450A2_8E09_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BotConsoleDlg.h : header file
//

struct ThreadInfo
{
  CEdit*        m_pEdit1 ;
  CBotProgram*  m_pProg;
  CWnd*         m_pWndMessage;
  BOOL          m_bStop;
  BOOL          m_bRun;
};


/////////////////////////////////////////////////////////////////////////////
// CBotConsoleDlg dialog

class CBotConsoleDlg : public CDialog
{
// Construction
public:
    CBotConsoleDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CBotConsoleDlg)
    enum { IDD = IDD_CONSOLE };
    CButton m_cOK;
    CEdit   m_Edit2;
    CEdit   m_Edit1;
    //}}AFX_DATA

    CBotProgram*    m_pProg;
    ThreadInfo      m_threadinfo;

    int             m_code, m_start, m_end;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBotConsoleDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CBotConsoleDlg)
    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    afx_msg LONG EndProg(UINT wparam, LONG lparam) ;
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOTCONSOLEDLG_H__A11450A2_8E09_11D4_A439_00D059085115__INCLUDED_)
