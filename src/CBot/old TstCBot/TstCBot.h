﻿// * This file is part of the COLOBOT source code
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

#if !defined(AFX_TSTCBOT_H__70B37568_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
#define AFX_TSTCBOT_H__70B37568_5DFD_11D4_A15E_00E0189013DF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "..\CBotDll.h"


class 	CMyThread;

/////////////////////////////////////////////////////////////////////////////
// CTstCBotApp:
// See TstCBot.cpp for the implementation of this class
//

class CTstCBotApp : public CWinApp
{
public:
	CTstCBotApp();

	CMyThread*		m_pThread;
	CWnd*			m_pView;
	CEdit*			m_pConsole;
	CBotStringArray m_Liste;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTstCBotApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTstCBotApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSTCBOT_H__70B37568_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)

#define WM_STARTPROG  WM_APP + 0
#define WM_ENDPROG	  WM_APP + 1
