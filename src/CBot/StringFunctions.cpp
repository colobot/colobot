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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// d�finition des fonctions sur les cha�nes


// donne la longueur d'une cha�ne
// ex�cution

bool rStrLen( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// pas de second param�tre
	if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// met la longueur sur la pile
	pResult->SetValInt( s.GivLength() );
	return true;
}

// int xxx ( string )
// compilation

CBotTypResult cIntStr( CBotVar* &pVar, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADPARAM );

	// pas de second param�tre
	if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

	// le r�sultat final est un nombre entier
	return CBotTypResult( CBotTypInt );
}


// donne la partie gauche d'une cha�ne
// ex�cution

bool rStrLeft( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre un nombre
	if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

	// r�cup�re ce nombre
	int n = pVar->GivValInt();

	// pas de 3e param�tre
	if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

	// prend la partie int�ressante
	s = s.Left( n );

	// la met sur la pile
	pResult->SetValString( s );
	return true;
}

// string xxx ( string, int )
// compilation

CBotTypResult cStrStrInt( CBotVar* &pVar, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADSTRING );

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre un nombre
	if ( pVar->GivType() > CBotTypDouble )
						return CBotTypResult( TX_BADNUM );

	// pas de 3e param�tre
	if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

	// le r�sultat final est une string
	return CBotTypResult( CBotTypString );
}

// donne la partie droite d'une cha�ne
// ex�cution

bool rStrRight( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre un nombre
	if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

	// r�cup�re ce nombre
	int n = pVar->GivValInt();

	// pas de 3e param�tre
	if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

	// prend la partie int�ressante
	s = s.Right( n );

	// la met sur la pile
	pResult->SetValString( s );
	return true;
}

// donne la partie centrale d'une cha�ne
// ex�cution

bool rStrMid( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre un nombre
	if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

	// r�cup�re ce nombre
	int n = pVar->GivValInt();

	// 3e param�tre optionnel
	if ( pVar->GivNext() != NULL )
	{
		pVar = pVar->GivNext();
	
		// qui doit �tre un nombre
		if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }
	
		// r�cup�re ce nombre
		int l = pVar->GivValInt();

		// mais pas de 4e param�tre
		if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return true; }

		// prend la partie int�ressante
		s = s.Mid( n, l );
	}
	else
	{
		// prend la partie int�ressante
		s = s.Mid( n );
	}

	// la met sur la pile
	pResult->SetValString( s );
	return true;
}

// donne la partie centrale d'une cha�ne
// compilation

CBotTypResult cStrStrIntInt( CBotVar* &pVar, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADSTRING );

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre un nombre
	if ( pVar->GivType() > CBotTypDouble )
						return CBotTypResult( TX_BADNUM );

	// 3e param�tre optionnel
	if ( pVar->GivNext() != NULL )
	{
	
		pVar = pVar->GivNext();
		// qui doit �tre un nombre
		if ( pVar->GivType() > CBotTypDouble )
							return CBotTypResult( TX_BADNUM );

		// pas de 4e param�tre
		if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );
	}

	// le r�sultat final est une string
	return CBotTypResult( CBotTypString );
}


// donne le nombre contenu dans une cha�ne
// ex�cution

bool rStrVal( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// mais pas de 2e param�tre
	if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return true; }

	float val = GivNumFloat(s);

	// la met la valeur sur la pile
	pResult->SetValFloat( val );
	return true;
}

// float xxx ( string )
// compilation

CBotTypResult cFloatStr( CBotVar* &pVar, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADSTRING );

	// pas de 2e param�tre
	if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

	// le r�sultat final est un nombre
	return CBotTypResult( CBotTypFloat );
}


// trouve une chaine dans une autre
// ex�cution

bool rStrFind( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// r�cup�re ce nombre
	CBotString s2 = pVar->GivValString();

	// pas de 3e param�tre
	if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

	// met le r�sultat sur la pile
	int res = s.Find(s2);
	pResult->SetValInt( res );
	if ( res < 0 ) pResult->SetInit( IS_NAN );
	return true;
}

// int xxx ( string, string )
// compilation

CBotTypResult cIntStrStr( CBotVar* &pVar, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADSTRING );

	// il faut un second param�tre
	pVar = pVar->GivNext();
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADSTRING );

	// pas de 3e param�tre
	if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

	// le r�sultat final est un nombre
	return CBotTypResult( CBotTypInt );
}

// donne une chaine en majuscule
// ex�cution

bool rStrUpper( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// mais pas de 2e param�tre
	if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return true; }


	s.MakeUpper();

	// la met la valeur sur la pile
	pResult->SetValString( s );
	return true;
}

// donne une chaine en minuscules
// ex�cution

bool rStrLower( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

	// recup�re le contenu de la string
	CBotString	s = pVar->GivValString();

	// mais pas de 2e param�tre
	if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return true; }


	s.MakeLower();

	// la met la valeur sur la pile
	pResult->SetValString( s );
	return true;
}

// string xxx ( string )
// compilation

CBotTypResult cStrStr( CBotVar* &pVar, void* pUser )
{
	// il faut un param�tre
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

	// qui doit �tre une string
	if ( pVar->GivType() != CBotTypString )
						return CBotTypResult( TX_BADSTRING );

	// pas de 2e param�tre
	if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

	// le r�sultat final est une string
	return CBotTypResult( CBotTypString );
}


void InitStringFunctions()
{
	CBotProgram::AddFunction("strlen",   rStrLen,   cIntStr );
	CBotProgram::AddFunction("strleft",  rStrLeft,  cStrStrInt );
	CBotProgram::AddFunction("strright", rStrRight, cStrStrInt );
	CBotProgram::AddFunction("strmid",   rStrMid,   cStrStrIntInt );

	CBotProgram::AddFunction("strval",   rStrVal,   cFloatStr );
	CBotProgram::AddFunction("strfind",  rStrFind,  cIntStrStr );

	CBotProgram::AddFunction("strupper", rStrUpper, cStrStr );
	CBotProgram::AddFunction("strlower", rStrLower, cStrStr );
}
