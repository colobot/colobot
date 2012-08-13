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

////////////////////////////////////////////////////////////////////
// routine show()
// utilisable depuis le programme écrit en CBot


#include <iostream>
#include <string>
#include "CBot/ClassFILE.cpp"
//std::string s;
/*
// execution
bool	rShow( CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser )
{
	string::string		s;

	while ( pVar != NULL )
	{
		string::string	ss;

		ss.LoadString( TX_TYPENAMES + pVar->GetType() );
		s += ss + " ";

		ss = pVar->GetName();
		if (ss.IsEmpty()) ss = "<sans nom>";
		s += ss + " = ";

		s += pVar->GetValString();
		s += "\n";
		pVar = pVar->GetNext();
	}

	AfxMessageBox(s, MB_OK|MB_ICONINFORMATION);

	return	TRUE;			// pas d'interruption
}

CBotTypResult	cShow( CBotVar* &pVar, void* pUser)
{
	if ( pVar == NULL ) return CBotTypResult(5028);
	return CBotTypResult(0);		// tous paramètres acceptés, void en retour
}

*/

////////////////////////////////////////////////////////////////////
// routine print()
// utilisable depuis le programme écrit en CBot

// exécution

bool rPrintLn( CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser )
{
	std::string s;
	while ( pVar != NULL )
	{
		if ( !s.empty() ) s += " ";
		s += pVar->GetValString();
		pVar = pVar->GetNext();
	}
	s += "\n";

	std::cout << s;
	return	true;			// pas d'interruption
}

bool	rPrint( CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser )
{
	std::string s;
	while ( pVar != NULL )
	{
		if ( !s.empty() ) s += " ";
		s += pVar->GetValString();
		pVar = pVar->GetNext();
	}
	s += " ";
	std::cout << s;
	return	true;			// pas d'interruption
}

CBotTypResult	cPrint( CBotVar* &pVar, void* pUser)
{
	return CBotTypResult(0);				// tous paramètres acceptés, un entier en retour
}


//////////////////////////////////////////////////////////////////
// class CPoint pour essayer

// exécution
bool	rCPoint( CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception )
{
	if ( pVar == NULL )return true;			// constructor with no parameters is ok

	CBotVar*	pX = pThis->GetItem("x");
	pX->SetValFloat( pVar->GetValFloat() );
	pVar	= pVar->GetNext();

	CBotVar*	pY = pThis->GetItem("y");
	pY->SetValFloat( pVar->GetValFloat() );
	pVar	= pVar->GetNext();

	return	true;			// pas d'interruption
}

CBotTypResult cCPoint( CBotVar* pThis, CBotVar* &pVar)
{
	// ok if no parameters!
	if ( pVar == NULL ) return CBotTypResult(0);

	// numeric type of parameter please
	if ( pVar->GetType() > CBotTypDouble ) return CBotTypResult(5011);
	pVar	= pVar->GetNext();

	// there must be a second parameter
	if ( pVar == NULL ) return 5028;
	// also numeric
	if ( pVar->GetType() > CBotTypDouble )return CBotTypResult(5011);
	pVar	= pVar->GetNext();

	// and not more than 2 parameters please
	if ( pVar != NULL ) return CBotTypResult(5026);

	return CBotTypResult(0);		// This function returns void
}

