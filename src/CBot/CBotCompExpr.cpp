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

#include "CBot.h"

// divers constructeurs

CBotCompExpr::CBotCompExpr()
{
	m_leftop	=
	m_rightop	= NULL;
	name = "CBotCompExpr";
}

CBotCompExpr::~CBotCompExpr()
{
	delete	m_leftop;
	delete	m_rightop;
}

fichier plus utilise;

// compile une instruction de type A < B

CBotInstr* CBotCompExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
	CBotCStack* pStk = pStack->AddStack();

	CBotInstr*	left = CBotAddExpr::Compile( p, pStk );		// expression A + B � gauche
	if (left == NULL) return pStack->Return(NULL, pStk);	// erreur

	if ( p->GetType() == ID_HI ||
		 p->GetType() == ID_LO ||
		 p->GetType() == ID_HS ||
		 p->GetType() == ID_LS ||
		 p->GetType() == ID_EQ ||
		 p->GetType() == ID_NE)								// les diverses comparaisons
	{
		CBotCompExpr* inst = new CBotCompExpr();			// �l�ment pour op�ration
		inst->SetToken(p);									// m�morise l'op�ration

		int			 type1, type2;
		type1 = pStack->GetType();

		p = p->Next();
		if ( NULL != (inst->m_rightop = CBotAddExpr::Compile( p, pStk )) )	// expression A + B � droite
		{
			type2 = pStack->GetType();
			// les r�sultats sont-ils compatibles
			if ( type1 == type2 )
			{
				inst->m_leftop = left;
				pStk->SetVar(new CBotVar(NULL, CBotTypBoolean));
															// le r�sultat est un boolean
				return pStack->Return(inst, pStk);
			}
		}

		delete left;
		delete inst;
		return pStack->Return(NULL, pStk);
	}

	return pStack->Return(left, pStk);
}


// fait l'op�ration

BOOL CBotCompExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack(this);
//	if ( pStk1 == EOX ) return TRUE;

	if ( pStk1->GetState() == 0 && !m_leftop->Execute(pStk1) ) return FALSE; // interrompu ici ?

	pStk1->SetState(1);		// op�ration termin�e

	// demande un peu plus de stack pour ne pas toucher le r�sultat de gauche
	CBotStack* pStk2 = pStk1->AddStack();

	if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrompu ici ?

	int		type1 = pStk1->GetType();
	int		type2 = pStk2->GetType();

	CBotVar*	result = new CBotVar( NULL, CBotTypBoolean );

	switch (GetTokenType())
	{
	case ID_LO:
		result->Lo(pStk1->GetVar(), pStk2->GetVar());		// inf�rieur
		break;
	case ID_HI:
		result->Hi(pStk1->GetVar(), pStk2->GetVar());		// sup�rieur
		break;
	case ID_LS:
		result->Ls(pStk1->GetVar(), pStk2->GetVar());		// inf�rieur ou �gal
		break;
	case ID_HS:
		result->Hs(pStk1->GetVar(), pStk2->GetVar());		// sup�rieur ou �gal
		break;
	case ID_EQ:
		result->Eq(pStk1->GetVar(), pStk2->GetVar());		// �gal
		break;
	case ID_NE:
		result->Ne(pStk1->GetVar(), pStk2->GetVar());		// diff�rent
		break;
	}
	pStk2->SetVar(result);				// met le r�sultat sur la pile

	pStk1->Return(pStk2);				// lib�re la pile
	return pStack->Return(pStk1);		// transmet le r�sultat
}

