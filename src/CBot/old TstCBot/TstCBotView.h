// TstCBotView.h : interface of the CTstCBotView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTCBOTVIEW_H__70B37570_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
#define AFX_TSTCBOTVIEW_H__70B37570_5DFD_11D4_A15E_00E0189013DF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CBotProgram;
class CBotClass;


class CTstCBotView : public CView
{
protected: // create from serialization only
    CTstCBotView();
    DECLARE_DYNCREATE(CTstCBotView)

    CEdit*          m_pEdit;        // texte en édition
    CWnd*           m_pWnd;
    CBotProgram*    m_pProg;        // programme compilé

// Attributes
public:
    CTstCBotDoc* GetDocument();

// Operations
public:
    void            LoadEdition(const char* name);
    void            SaveEdition(const char* name);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTstCBotView)
    public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
    protected:
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CTstCBotView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    //{{AFX_MSG(CTstCBotView)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnCp1();
    afx_msg void OnExe();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TstCBotView.cpp
inline CTstCBotDoc* CTstCBotView::GetDocument()
   { return (CTstCBotDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSTCBOTVIEW_H__70B37570_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)



