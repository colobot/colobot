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

	// ouvre une fen�tre pour afficher les sorties
	CRect		rect;
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


//	pApp->m_pEdit2->SetFocus();

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
	m_pMainWnd = &dlg;		// cela ferme l'application avec la DBOX !

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
