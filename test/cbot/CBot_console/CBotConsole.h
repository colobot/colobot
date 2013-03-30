/*
 * CBotConsole.h
 *
 *  Created on: 08-08-2012
 *      Author: michal
 */

#pragma once

#include "CClass.h"

struct ThreadInfo
{
//  CEdit*		m_pEdit1 ;
//  CEdit*		m_pEditx ;
  CBotProgram*	m_pProg;
//  CWnd*			m_pWndMessage;
  bool			m_bStop;
  bool			m_bRun;
};

class CBotConsole {

public:
	CBotConsole();
	virtual ~CBotConsole();

//	CEdit	m_Edit1;

	CBotProgram*		m_pProg;
	ThreadInfo			m_threadinfo;

	CBotStringArray*	m_pListe;
	int					m_code, m_start, m_end;
//	CEdit*			m_pEditx;

	// Implementation
	void OnOK();
	void OnCancel();
	bool OnInitDialog();
	long EndProg() ;
};

