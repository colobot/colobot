/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */
/*
 * CBotDoc.cpp
 *
 *  Created on: 08-08-2012
 *      Author: michal
 */
#include "CBotDoc.h"
#include "CBotConsole.h"
#include <iostream>

CBotDoc::CBotDoc(std::string s) {
	// TODO Auto-generated constructor stub
	// TODO set m_DocText
//	m_pEdit		= NULL;
	m_pProg		= NULL;
//	m_bModified = FALSE;
	m_DocText = s;
	std::cout << s << std::endl;
//	std::cout << "Enter to continue..." << std::endl;
//	getchar();
}

CBotDoc::~CBotDoc() {

//	delete	m_pEdit;
	delete	m_pProg;
}


//static bool test = false;

void CBotDoc::OnRun()
{

//	m_pEdit->GetWindowText(m_DocText);
	CBotString s;

	std::string		TextError;
	int				code, start, end;

	if ( m_pProg == NULL ) m_pProg = new CBotProgram();

	if (!m_pProg->Compile(m_DocText.c_str(), m_Liste, NULL))
	{
		m_pProg->GetError(code, start, end);
		delete m_pProg;
		m_pProg = NULL;

		TextError = CBotProgram::GetErrorText( code );
		std::cout << TextError << std::endl;
		return;
	}

	if( m_Liste.GetSize() == 0 )
	{
		std::cout << "No function marked \"extern\" !\n";
		return;
	}

	for ( int i = 0; i < m_Liste.GetSize(); i++ )
	{
		int	start, stop;
		m_pProg->GetPosition(m_Liste[i], start, stop, GetPosNom, GetPosParam);
		CBotString s(m_DocText.substr( start, stop-start ).c_str());
		m_Liste[i] = s;
	}
//	TODO
	CBotConsole dlg;
	dlg.m_pListe = &m_Liste;
//	dlg.m_pEditx = m_pEdit;

	dlg.OnInitDialog();
	dlg.OnOK();
	dlg.EndProg();
//	if ( dlg.m_code>0 )
//	{
//		std::string	TextError;
//
//		TextError = m_pProg->GetErrorText( dlg.m_code );
//
//		std::cout <<TextError;
//	}

	return;
}

bool CBotDoc::Compile()
{
//	m_pEdit->GetWindowText(m_DocText);

	std::string			TextError;
	int				code, start, end;

	if ( m_pProg == NULL ) m_pProg = new CBotProgram();

	char	buffer[100];
	strcpy(buffer, "a pointer move to see");

	if (!m_pProg->Compile(m_DocText.c_str(), m_Liste, static_cast<void*>(buffer)))
	{
		m_pProg->GetError(code, start, end);
		delete m_pProg;
		m_pProg = NULL;

//		m_pEdit->SetSel( start, end );
//		m_pEdit->SetFocus();				// higlights part of problem

		TextError = CBotProgram::GetErrorText( code );
		std::cout << TextError ;

		return false;
	}

//	if ( m_pProg->GetPosition( "TheTest", start, end) )
//	{
//		m_pEdit->SetSel( start, end );
//		m_pEdit->SetFocus();				// higlights part of problem
//	}

//	m_bModified = FALSE;
	return true;
}
