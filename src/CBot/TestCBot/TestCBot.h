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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// TestCBot.h : main header file for the TESTCBOT application
//

#if !defined(AFX_TESTCBOT_H__4D1BB903_8E74_11D4_A439_00D059085115__INCLUDED_)
#define AFX_TESTCBOT_H__4D1BB903_8E74_11D4_A439_00D059085115__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
//#include "../CbotDll.h"   // librairie CBot
#include "../Cbot.h"        // complet pour Browse

class CTestCBotView;

/////////////////////////////////////////////////////////////////////////////
// CTestCBotApp:
// See TestCBot.cpp for the implementation of this class
//

class CTestCBotApp : public CWinApp
{
public:
    CTestCBotApp();

    CEdit*          m_pConsole;
    CTestCBotView*  m_LastActive;
    CBotClass*      m_pClassPoint;
    CBotClass*      m_pClassPointIntr;


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTestCBotApp)
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    //}}AFX_VIRTUAL

// Implementation

    //{{AFX_MSG(CTestCBotApp)
    afx_msg void OnAppAbout();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTCBOT_H__4D1BB903_8E74_11D4_A439_00D059085115__INCLUDED_)


#define WM_STARTPROG  WM_APP + 0
#define WM_ENDPROG    WM_APP + 1
#define WM_ACTWINDOW  WM_APP + 2
