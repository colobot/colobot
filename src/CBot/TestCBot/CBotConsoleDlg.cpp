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

// CBotConsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestCBot.h"
#include "CBotConsoleDlg.h"

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
    m_code  = 0;
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
#if 0
        const char* FunctionName;
        const char* FN;
        int start, end;

        info->m_pProg->GetRunPos(FunctionName, start, end);

        if ( FunctionName != NULL )
        {
            info->m_pEditx->SetSel(start, end);

            char    buffer[200];
            sprintf( buffer, "step %s, %d, %d",FunctionName, start, end);
            AfxMessageBox( buffer );

            int level = 0;
            do
            {
                CBotVar* t = info->m_pProg->GivStackVars(FN, level--);
                if ( FN != FunctionName ) break;
                if ( t != NULL )
                {
                    CString s ;
                    while ( t != NULL )
                    {
                        if (s.IsEmpty()) s+= "Stack -> ";
                        else             s+= " , ";
                        s += t->GivValString();
                        t = t->GivNext();
                    }
                    AfxMessageBox(s);
                }
            } while (TRUE);
        }
#endif
        Cpt++;
        if ( Cpt%50 == 0 ) info->m_pEdit1->ReplaceSel(".");
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
        CBotString  TextError;
        TextError = CBotProgram::GivErrorText(m_code);
        AfxMessageBox(TextError);
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
    CTestCBotApp* pApp = (CTestCBotApp*)AfxGetApp();
    pApp->m_pConsole = &m_Edit1;
    m_code  = 0;

    CString Commande;
    m_Edit2.GetWindowText(Commande);

    CString s = "void LaCommande() { " + Commande + " ;}";
    m_pProg = new CBotProgram();
    CBotStringArray liste;
    m_pProg->Compile(s, liste);

    int err, start, end;
    if ( m_pProg->GetError(err, start, end) )
    {
        CBotString  TextError;
        TextError = CBotProgram::GivErrorText(err);
        AfxMessageBox(TextError);
        m_Edit2.SetSel(start-20, end-20);
        return;
    }

    m_Edit1.ReplaceSel("\r\n" + Commande + " ->\r\n");

    m_Edit2.SetWindowText("");
    m_Edit1.SetFocus();
    m_Edit2.EnableWindow(FALSE);
    m_cOK.EnableWindow(FALSE);

    // lance un processus paralèle pour l'exécution
    m_threadinfo.m_pWndMessage = this ;

    m_threadinfo.m_pEdit1 = &m_Edit1;
    m_threadinfo.m_pEditx = m_pEditx;
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
    CDialog::OnInitDialog();

    m_Edit1.ReplaceSel("Les fonctions suivantes sont disponibles:\r\n");
    for ( int i = 0; i < m_pListe->GivSize(); i++ )
    {
        CBotString x = (*m_pListe)[i] + CBotString("\r\n");
        m_Edit1.ReplaceSel(x);
    }
    m_Edit1.ReplaceSel("Entrez une commande ci-dessous.\r\n\r\n");


    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
