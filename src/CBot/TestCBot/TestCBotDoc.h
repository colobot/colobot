// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.// TestCBotDoc.h : interface of the CTestCBotDoc class
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
    CEdit*          m_pEdit;            // pour mémoriser le texte, et l'afficher
    CBotProgram*    m_pProg;            // le programme compilé
    CString         m_DocText;
    CBotStringArray m_Liste;
    BOOL            m_bModified;

// Operations
public:
    BOOL            Compile();

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
