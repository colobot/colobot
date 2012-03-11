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
#if !defined(AFX_BOTERRORDLG_H__80E73D20_7454_11D4_A439_00D059085115__INCLUDED_)
#define AFX_BOTERRORDLG_H__80E73D20_7454_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BotErrorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBotErrorDlg dialog

class CBotErrorDlg : public CDialog
{
// Construction
public:
	CBotErrorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBotErrorDlg)
	enum { IDD = IDD_DIALOG1 };
	CEdit	m_eProgram;
	CStatic	m_sMessage;
	CString	m_TextProgram;
	//}}AFX_DATA


	CString		m_TextError;
	int			m_start, m_end;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBotErrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBotErrorDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOTERRORDLG_H__80E73D20_7454_11D4_A439_00D059085115__INCLUDED_)
