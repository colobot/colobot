// TestCBotView.h : interface of the CTestCBotView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTCBOTVIEW_H__4D1BB90D_8E74_11D4_A439_00D059085115__INCLUDED_)
#define AFX_TESTCBOTVIEW_H__4D1BB90D_8E74_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CTestCBotView : public CView
{
protected: // create from serialization only
	CTestCBotView();
	DECLARE_DYNCREATE(CTestCBotView)

// Attributes
public:
	CTestCBotDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestCBotView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestCBotView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestCBotView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LONG ActWindow(UINT wparam, LONG lparam) ;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TestCBotView.cpp
inline CTestCBotDoc* CTestCBotView::GetDocument()
   { return (CTestCBotDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTCBOTVIEW_H__4D1BB90D_8E74_11D4_A439_00D059085115__INCLUDED_)
