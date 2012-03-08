// TestCBotDoc.h : interface of the CTestCBotDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTCBOTDOC_H__4D1BB90B_8E74_11D4_A439_00D059085115__INCLUDED_)
#define AFX_TESTCBOTDOC_H__4D1BB90B_8E74_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CTestCBotDoc : public CDocument
{
protected: // create from serialization only
	CTestCBotDoc();
	DECLARE_DYNCREATE(CTestCBotDoc)

// Attributes
public:
	CEdit*			m_pEdit;			// pour mémoriser le texte, et l'afficher
	CBotProgram*	m_pProg;			// le programme compilé
	CString			m_DocText;
	CBotStringArray	m_Liste;
	BOOL			m_bModified;

// Operations
public:
	BOOL			Compile();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestCBotDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestCBotDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestCBotDoc)
	afx_msg void OnRun();
	afx_msg void OnChangeEdit1();
	afx_msg void OnTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTCBOTDOC_H__4D1BB90B_8E74_11D4_A439_00D059085115__INCLUDED_)
