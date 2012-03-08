#if !defined(AFX_PERFORMDLG_H__EAF2D560_97D8_11D4_A439_00D059085115__INCLUDED_)
#define AFX_PERFORMDLG_H__EAF2D560_97D8_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PerformDlg.h : header file
//

struct ThreadInfo2
{
  CEdit*		m_pEdit ;
  CEdit*		m_pEdit3 ;

  CBotProgram**	m_pProg;
  BOOL			m_bStop;
  int			m_nbscripts;
};


/////////////////////////////////////////////////////////////////////////////
// CPerformDlg dialog

class CPerformDlg : public CDialog
{
// Construction
public:
	CPerformDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPerformDlg)
	enum { IDD = IDD_DIALOG1 };
	CEdit	m_Edit3;
	CEdit	m_Edit1;
	//}}AFX_DATA

	CBotProgram*	m_pProg[100];
	ThreadInfo2		m_threadinfo2;
	CString			m_Script;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPerformDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPerformDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERFORMDLG_H__EAF2D560_97D8_11D4_A439_00D059085115__INCLUDED_)
