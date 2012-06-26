// BotConsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TstCBot.h"
#include "BotConsoleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBotConsoleDlg dialog


CBotConsoleDlg::CBotConsoleDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CBotConsoleDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CBotConsoleDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_pProg = NULL;
    m_threadinfo.m_bRun   = FALSE;
}


void CBotConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CBotConsoleDlg)
    DDX_Control(pDX, IDOK, m_cOK);
    DDX_Control(pDX, IDC_EDIT2, m_Edit2);
    DDX_Control(pDX, IDC_EDIT1, m_Edit1);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBotConsoleDlg, CDialog)
    //{{AFX_MSG_MAP(CBotConsoleDlg)
    ON_MESSAGE(WM_ENDPROG, EndProg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBotConsoleDlg message handlers

UINT ThreadProc(ThreadInfo *info)
{
    CTime   t0  = CTime::GetCurrentTime();
    int     Cpt = 0;

    info->m_pProg->Start("LaCommande");
    while ( !info->m_bStop && !info->m_pProg->Run() )
    {
        Cpt++;
        if ( Cpt%20 == 0 ) info->m_pEdit1->ReplaceSel(".");
    }

    if ( info->m_bStop )
    {
        info->m_pEdit1->ReplaceSel("\r\nInterrompu\r\n");
    }
    else if (info->m_pProg->GivError() == 0)
    {
        CTime   t = CTime::GetCurrentTime();
        CTimeSpan ts = t - t0;

        char    buffer[200];
        sprintf( buffer, "\r\nExécution terminée en %d secondes.\r\nInterrompue %d fois.\r\n",
                 ts.GetTotalSeconds(), Cpt);

        info->m_pEdit1->ReplaceSel(buffer);
    }

    info->m_pWndMessage->SendMessage(WM_ENDPROG, 0, 0) ;
    return 0 ;
}

LONG CBotConsoleDlg::EndProg(UINT wparam, LONG lparam)
{
    m_threadinfo.m_bRun   = FALSE;

    if (m_pProg->GetError(m_code, m_start, m_end))
    {
        AfxMessageBox(m_code);
        CDialog::OnCancel();
        return 1;
    }
    delete m_pProg;
    m_pProg = NULL;

    m_Edit2.EnableWindow(TRUE);
    m_cOK.EnableWindow(TRUE);

    m_Edit2.SetWindowText("");
    m_Edit2.SetFocus();
    return 0 ;
}

void CBotConsoleDlg::OnOK()
{
    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();
    pApp->m_pConsole = &m_Edit1;

    CString Commande;
    m_Edit2.GetWindowText(Commande);

    CString s = "void LaCommande() { " + Commande + " ;}";
    m_pProg = new CBotProgram();
    CBotStringArray liste;
    m_pProg->Compile(s, liste);
    int err, start, end;
    if ( m_pProg->GetError(err, start, end) )
    {
        AfxMessageBox(err);
        m_Edit2.SetSel(start-20, end-20);
        return;
    }

    m_Edit1.ReplaceSel(Commande + " ->\r\n");

    m_Edit2.SetWindowText("");
    m_Edit1.SetFocus();
    m_Edit2.EnableWindow(FALSE);
    m_cOK.EnableWindow(FALSE);

    // lance un processus paralèle pour l'exécution
    m_threadinfo.m_pWndMessage = this ;

    m_threadinfo.m_pEdit1 = &m_Edit1;
    m_threadinfo.m_pProg  = m_pProg;
    m_threadinfo.m_bStop  = FALSE;
    m_threadinfo.m_bRun   = TRUE;

    AfxBeginThread((AFX_THREADPROC)ThreadProc, &m_threadinfo) ;
}

void CBotConsoleDlg::OnCancel()
{
    if (!m_threadinfo.m_bRun) CDialog::OnCancel();
    m_threadinfo.m_bStop = TRUE ;
}


BOOL CBotConsoleDlg::OnInitDialog()
{
    CTstCBotApp* pApp = (CTstCBotApp*)AfxGetApp();

    CDialog::OnInitDialog();

    m_Edit1.ReplaceSel("Les fonctions suivantes sont disponibles:\r\n");
    for ( int i = 0; i < pApp->m_Liste.RetSize(); i++ )
    {
        CBotString x = CString(pApp->m_Liste[i]) + "\r\n";
        m_Edit1.ReplaceSel(x);
    }
    m_Edit1.ReplaceSel("Entrez une commande ci-dessous.\r\n\r\n");


    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
