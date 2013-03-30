/*
 * CBotConsole.cpp
 *
 *  Created on: 08-08-2012
 *      Author: michal
 */

#include "CBotConsole.h"
#include "CClass.h"
#include <ctime>
#include <iostream>

CBotConsole::CBotConsole() {
	// TODO Auto-generated constructor stub
	m_pProg = NULL;
	m_threadinfo.m_bRun	  = false;
	m_code  = 0;

}

CBotConsole::~CBotConsole() {
	// TODO Auto-generated destructor stub
}

uint ThreadProc(ThreadInfo *info)
{
	time_t	t0,t1;
	time(&t0);

	int		Cpt = 0;

	info->m_pProg->Start("LaCommande");
	while ( !info->m_bStop && !info->m_pProg->Run() )
	{
#if	0
		const char* FunctionName;
		const char* FN;
		int start, end;

		info->m_pProg->GetRunPos(FunctionName, start, end);

		if ( FunctionName != NULL )
		{
			info->m_pEditx->SetSel(start, end);

			char	buffer[200];
			sprintf( buffer, "step %s, %d, %d",FunctionName, start, end);
			AfxMessageBox( buffer );

			int level = 0;
			do
			{
				CBotVar* t = info->m_pProg->GetStackVars(FN, level--);
				if ( FN != FunctionName ) break;
				if ( t != NULL )
				{
					CString	s ;
					while ( t != NULL )
					{
						if (s.IsEmpty()) s+= "Stack -> ";
						else			 s+= " , ";
						s += t->GetValString();
						t = t->GetNext();
					}
					AfxMessageBox(s);
				}
			} while (TRUE);
		}
#endif
		Cpt++;
		if ( Cpt%50 == 0 ) std::cout << ".";
	}

	if ( info->m_bStop )
	{
		std::cout << "\nInterrupt\n";
	}
	else if (info->m_pProg->GetError() == 0)
	{
		time(&t1);
		double prog_time = difftime(t0,t1);

		char	buffer[200];
		sprintf( buffer, "\nExecution terminated in %f seconds.\nInterrupted %d time(s).\n",
				 prog_time, Cpt);

		std::cout << buffer;
	}

//	info->m_pWndMessage->SendMessage(WM_ENDPROG, 0, 0) ;
	return 0 ;
}

long CBotConsole::EndProg()
{
	m_threadinfo.m_bRun	  = false;

	if (m_pProg->GetError(m_code, m_start, m_end))
	{
		CBotString	TextError;
		TextError = CBotProgram::GetErrorText(m_code);
		std::cout << TextError;
		return 1;
	}
	delete m_pProg;
	m_pProg = NULL;

	return 0 ;
}


void CBotConsole::OnOK()
{
	m_code  = 0;

	std::string	Commande;
	std::cin >> Commande;

	std::string s = "void LaCommande() { " + Commande + " ;}";
	m_pProg = new CBotProgram();
	CBotStringArray liste;
	m_pProg->Compile(s.c_str(), liste);

	int	err, start, end;
	if ( m_pProg->GetError(err, start, end) )
	{
		CBotString	TextError;
		TextError = CBotProgram::GetErrorText(err);
		std::cout << TextError;
		return;
	}

	std::cout << "\n" + Commande + " ->\n";

//	m_Edit2.SetWindowText("");
//	m_Edit1.SetFocus();
//	m_Edit2.EnableWindow(FALSE);
//	m_cOK.EnableWindow(FALSE);

	// lance un processus paralèle pour l'exécution
//	m_threadinfo.m_pWndMessage = this ;

//	m_threadinfo.m_pEdit1 = &m_Edit1;
//	m_threadinfo.m_pEditx = m_pEditx;
	m_threadinfo.m_pProg  = m_pProg;
	m_threadinfo.m_bStop  = false;
	m_threadinfo.m_bRun	  = true;

	ThreadProc(&m_threadinfo);

//  here program starts
//	AfxBeginThread((AFX_THREADPROC)ThreadProc, &m_threadinfo) ;
}

void CBotConsole::OnCancel()
{
	m_threadinfo.m_bStop = true ;
}

bool CBotConsole::OnInitDialog()
{
//	CDialog::OnInitDialog();

	std::cout << "Following functions are availible:\n";
	for ( int i = 0; i < m_pListe->GetSize(); i++ )
	{
		CBotString x = (*m_pListe)[i] + CBotString("\n");
		std::cout << x;
	}
	std::cout << "Enter a command:\n";


	return true;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
