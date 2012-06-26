// CMyThread.cpp : pour créer un processus pour la console
//

#include "stdafx.h"
#include "TstCBot.h"
#include "CMyThread.h"
#include "BotConsoleDlg.h"


//IMPLEMENT_DYNAMIC (CMyThread, CWinThread)
IMPLEMENT_DYNCREATE  (CMyThread, CWinThread)

/////////////////////////////////////////////////////////////////////////////
// CMyThread

BEGIN_MESSAGE_MAP(CMyThread, CWinThread)
    //{{AFX_MSG_MAP(CMyThread)
    //}}AFX_MSG_MAP
    // Standard file based document commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyThread construction

CMyThread::CMyThread()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


/////////////////////////////////////////////////////////////////////////////
// CMyThread initialization
/*
BOOL CMyThread::InitInstance()
{
    AfxEnableControlContainer();

    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();

    // ouvre une fenêtre pour afficher les sorties
    CRect       rect;
    AfxGetMainWnd()->GetClientRect( rect );
    rect += CPoint(30,30);

    CWnd* pWnd = new CWnd();
    pWnd->CreateEx( 0,
        AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
        "CBot console", WS_POPUPWINDOW|WS_CAPTION|WS_VISIBLE,
        rect,
        AfxGetMainWnd()->GetParent(), NULL, NULL);
    m_pMainWnd = pWnd;

    pApp->m_pEdit2 = new CEdit();

    m_pMainWnd->GetClientRect( rect );
    rect.bottom -= 40;
    pApp->m_pEdit2->Create( WS_VISIBLE|WS_BORDER|WS_TABSTOP|ES_MULTILINE|ES_WANTRETURN|
                            ES_AUTOVSCROLL|ES_READONLY,
                     rect, m_pMainWnd, IDC_EDIT2 );

    pApp->m_pEdit2->ReplaceSel("Les fonctions suivantes sont disponibles:\n\r");
    for ( int i = 0; i < pApp->m_Liste.RetSize(); i++ )
    {
        pApp->m_pEdit2->ReplaceSel(pApp->m_Liste[i] + "\r\n");
    }
    pApp->m_pEdit2->ReplaceSel("Entrez une commande ci-dessous.\r\r");


//  pApp->m_pEdit2->SetFocus();

    pApp->m_pEdit3 = new CEdit();
    m_pMainWnd->GetClientRect( rect );
    rect.top = rect.bottom-40;
    pApp->m_pEdit3->Create( WS_VISIBLE|WS_BORDER|WS_TABSTOP,
                     rect, m_pMainWnd, IDC_EDIT1 );
    pApp->m_pEdit3->SetFocus();

    return TRUE;
}*/

BOOL CMyThread::InitInstance()
{
    CBotConsoleDlg dlg;
    m_pMainWnd = &dlg;      // cela ferme l'application avec la DBOX !

    int nResponse = dlg.DoModal();

    return TRUE;
}


int CMyThread::ExitInstance()
{
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CMyThread message handlers


void CMyThread::OnReturn()
{
    // TODO: Add your command handler code here
    __asm int 3;
}
