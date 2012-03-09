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
// * along with this program. If not, see .

#if !defined(AFX_MAINFRM_H__20B3756C_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
#define AFX_MAINFRM_H__20B3756C_5DFD_11D4_A15E_00E0189013DF__INCLUDED_


#include "stdafx.h"
#include "TstCBot.h"

class CMyThread : public CWinThread
{
//	DECLARE_DYNAMIC(CMyThread)
	DECLARE_DYNCREATE(CMyThread)

public:


// Constructor
	CMyThread();
	virtual BOOL InitInstance();
	virtual int ExitInstance(); // return app exit code

// Implementation

	//{{AFX_MSG(CTstCBotApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnReturn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__20B3756C_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
