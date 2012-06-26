// TstCBotDoc.h : interface of the CTstCBotDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTCBOTDOC_H__70B3756E_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
#define AFX_TSTCBOTDOC_H__70B3756E_5DFD_11D4_A15E_00E0189013DF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CTstCBotDoc : public CDocument
{
protected: // create from serialization only
    CTstCBotDoc();
    DECLARE_DYNCREATE(CTstCBotDoc)

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTstCBotDoc)
    public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CTstCBotDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    //{{AFX_MSG(CTstCBotDoc)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSTCBOTDOC_H__70B3756E_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
