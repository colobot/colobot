// TstCBot.h : main header file for the TSTCBOT application
//

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


class   CMyThread;

/////////////////////////////////////////////////////////////////////////////
// CTstCBotApp:
// See TstCBot.cpp for the implementation of this class
//

class CTstCBotApp : public CWinApp
{
public:
    CTstCBotApp();

    CMyThread*      m_pThread;
    CWnd*           m_pView;
    CEdit*          m_pConsole;
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
#define WM_ENDPROG    WM_APP + 1
