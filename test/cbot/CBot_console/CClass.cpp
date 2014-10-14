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
#include "CClass.h"

#include "routines.cpp"

void rMajObject( CBotVar* pThis, void* pUser )
{
	if (!pThis->IsElemOfClass("object"))
		return ;
	CBotVar*	pPos = pThis->GetItem("position");
	CBotVar*	pX = pPos->GetItem("x");
	CBotVar*	pY = pPos->GetItem("y");
	CBotVar*	pZ = pPos->GetItem("z");
//	CBotVar*	pPt = pThis->GetItem("transport");

	CBotString p = pX->GetValString();

//	pX->SetValFloat( pUser == (void*)1 ? (float)12.5 : (float)44.4 );
	pZ->SetValFloat( (float)0 );
	pY->SetValFloat( (float)-3.33 );
	pX->SetValFloat( pX->GetValFloat() + 10 ) ;

//	pX = pThis->GetItem( "xx" );
//	pX->SetValFloat( (float)22 );

	// crée une instance sur une classe object
//	CBotVar* pAutre = CBotVar::Create("autre", CBotTypClass, "object");
//	pAutre->SetUserPtr( (void*)3 );
//	pPt->SetPointer( pAutre );
//	pPt->SetPointer( NULL );
//	delete pAutre;
};

CClass::CClass()
{
	m_pClassPoint= NULL;
}

bool CClass::InitInstance()
{
//////////////////////////////////////////////
//	défini les mots clefs supplémentaires
// -------------------------------------------

	CBotProgram::Init();

//////////////////////////////////////////////
//	défini les fonctions "show()" et "print()"
// -------------------------------------------

	//CBotProgram::AddFunction("show", rShow, cShow);
	CBotProgram::AddFunction("print", rPrint, cPrint);
	CBotProgram::AddFunction("println", rPrintLn, cPrint);


///////////////////////////////////
// définie la classe globale CPoint
// --------------------------------

	m_pClassPoint	= new CBotClass("CPoint", NULL);
	// ajoute le composant ".x"
	m_pClassPoint->AddItem("x", CBotTypFloat);
	// ajoute le composant ".y"
	m_pClassPoint->AddItem("y", CBotTypFloat);

	// ajoute le constructeur pour cette classe
	m_pClassPoint->AddFunction("CPoint", rCPoint, cCPoint);

	m_pClassPointIntr	= new CBotClass("point", NULL, true);
	// ajoute le composant ".x"
	m_pClassPointIntr->AddItem("x", CBotTypFloat);
	// ajoute le composant ".y"
	m_pClassPointIntr->AddItem("y", CBotTypFloat);
	// ajoute le composant ".z"
	m_pClassPointIntr->AddItem("z", CBotTypFloat);

	// ajoute le constructeur pour cette classe
	m_pClassPointIntr->AddFunction("point", rCPoint, cCPoint);

	// défini la classe "object"
	CBotClass*	pClassObject = new CBotClass( "object", NULL ) ;
	pClassObject->AddItem( "xx", CBotTypFloat );
	pClassObject->AddItem( "position", CBotTypResult( CBotTypIntrinsic, "point" ) );
	pClassObject->AddItem( "transport", CBotTypResult( CBotTypPointer, "object" ) );
	pClassObject->AddUpdateFunc( rMajObject );

	InitClassFILE();

	return true;
}

void CClass::ExitInstance()
{
	delete	m_pFuncFile;

	CBotProgram::Free();

}
