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
// * along with this program. If not, see  http://www.gnu.org/licenses/.

#if !defined(AFX_BOTCONSOLEDLG_H__A11450A2_8E09_11D4_A439_00D059085115__INCLUDED_)
#define AFX_BOTCONSOLEDLG_H__A11450A2_8E09_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CBotConsoleDlg.h : header file
//

struct ThreadInfo
{
  CEdit*        m_pEdit1 ;
  CEdit*        m_pEditx ;
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

    CBotStringArray*
                    m_pListe;
    int             m_code, m_start, m_end;
    CEdit*          m_pEditx;

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
