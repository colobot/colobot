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

// PerformDlg.cpp : implementation file
//

#include "stdafx.h"
#include "testcbot.h"
#include "PerformDlg.h"

//#include <stdio.h>
#include <sys/timeb.h>
//#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPerformDlg dialog


CPerformDlg::CPerformDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPerformDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPerformDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CPerformDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPerformDlg)
    DDX_Control(pDX, IDC_EDIT3, m_Edit3);
    DDX_Control(pDX, IDC_EDIT1, m_Edit1);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPerformDlg, CDialog)
    //{{AFX_MSG_MAP(CPerformDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPerformDlg message handlers

/* Pauses for a specified number of milliseconds. */

/*void sleep( double waitseconds )
{
   clock_t wait = (clock_t)(waitseconds * CLOCKS_PER_SEC);
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() )
      ;
}*/

void sleep( clock_t wait )
{
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() )
      TRACE("%d \n", clock() );
}

void sleep2( clock_t wait )
{
   struct _timeb timebuffer;
   char *timeline;

   _ftime( &timebuffer );
   timeline = ctime( & ( timebuffer.time ) );
   long x = timebuffer.millitm;
   while( x == timebuffer.millitm ) _ftime( &timebuffer );
}

#define NBLP    20

UINT ThreadProc2(ThreadInfo2 *info)
{
    int     lp = NBLP;
    int     i;
    clock_t start = clock();

    while ( !info->m_bStop )
    {
        for ( i = 0; i< info->m_nbscripts; i++ )
        {
            info->m_pProg[i]->Run();
        }

#ifdef  _DEBUG
        sleep2( 1 );
#else
        CString s ( "xx" );
        for ( long z = 0x5000; z>0; z-- ) s = s.Left(1);
#endif
        if ( --lp == 0 )
        {
            clock_t finish = clock();
            double n = (double)NBLP / (double)(finish-start) * CLOCKS_PER_SEC;
            char    b[30];
            sprintf( b, "%f", n);
            info->m_pEdit->SetWindowText(b);

            n = n * 1100 / 200; // performances
            sprintf( b, "%f", n);
            info->m_pEdit3->SetWindowText(b);
            start = finish;
            lp = NBLP;
        }
    }

    return 0 ;
}

BOOL CPerformDlg::OnInitDialog()
{
    CDialog::OnInitDialog();


    CBotStringArray liste;
    // crée les scripts pour les tests
    for ( int i = 0; i < 100; i++ )
    {
        m_pProg[i] = new CBotProgram();
        m_pProg[i]->Compile(m_Script, liste);
        m_pProg[i]->Start(liste[0]);
    }

    // lance un processus paralèle pour l'exécution
//  m_threadinfo2.m_pWndMessage = this ;

    m_threadinfo2.m_pEdit  = &m_Edit1;
    m_threadinfo2.m_pEdit3 = &m_Edit3;
    m_threadinfo2.m_pProg  = m_pProg;
    m_threadinfo2.m_bStop  = FALSE;
    m_threadinfo2.m_nbscripts = 30;


    AfxBeginThread((AFX_THREADPROC)ThreadProc2, &m_threadinfo2) ;
    // TODO: Add extra initialization here

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPerformDlg::OnCancel()
{
    m_threadinfo2.m_bStop  = TRUE;
    sleep ( 2000 );

    CDialog::OnCancel();

    for ( int i = 0; i < 100; i++ )
    {
        delete m_pProg[i];
    }
}
