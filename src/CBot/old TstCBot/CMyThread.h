// CMyThread.h : pour créer un processus pour la console
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__20B3756C_5DFD_11D4_A15E_00E0189013DF__INCLUDED_)
#define AFX_MAINFRM_H__20B3756C_5DFD_11D4_A15E_00E0189013DF__INCLUDED_


#include "stdafx.h"
#include "TstCBot.h"

class CMyThread : public CWinThread
{
//  DECLARE_DYNAMIC(CMyThread)
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
