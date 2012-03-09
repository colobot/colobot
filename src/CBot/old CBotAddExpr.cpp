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

#include "CBot.h"

// divers constructeurs

CBotAddExpr::CBotAddExpr()
{
	m_leftop	=
	m_rightop	= NULL;			// NULL pour pouvoir faire delete sans autre
	name = "CBotAddExpr";		// debug
}

CBotAddExpr::~CBotAddExpr()
{
	delete	m_leftop;
	delete	m_rightop;
}


// compile une instruction de type A + B

CBotInstr* CBotAddExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
	CBotCStack* pStk = pStack->TokenStack();					// un bout de pile svp

	// cherche des instructions qui peuvent convenir � gauche de l'op�ration + ou -

	CBotInstr*	left = CBotMulExpr::Compile( p, pStk );		// expression A * B � gauche
	if (left == NULL) return pStack->Return(NULL, pStk);	// si erreur, la transmet

	// est-ce qu'on a le token + ou - ensuite ?

	if ( p->GetType() == ID_ADD ||
		 p->GetType() == ID_SUB)							// plus ou moins
	{
		CBotAddExpr* inst = new CBotAddExpr();				// �l�ment pour op�ration
		inst->SetToken(p);									// m�morise l'op�ration

		int			 type1, type2;
		type1 = pStk->GetType(CBotTypChar);					// de quel type le premier op�rande ?

		p = p->Next();										// saute le token de l'op�ration 

		// cherche des instructions qui peuvent convenir � droite

		if ( NULL != (inst->m_rightop = CBotAddExpr::Compile( p, pStk )) )	// expression (...) � droite
		{
			// il y a un second op�rande acceptable

			type2 = pStk->GetType(CBotTypChar);				// de quel type le r�sultat ?

			if ( type1 == type2 &&							// les r�sultats sont-ils compatibles
				 type1 != CBotTypBoolean && 
				 (inst->m_token.GetType() != ID_SUB ||
				  type1 < CBotTypBoolean ))					// pas de soustraction de cha�nes !
			{
				// si ok, enregistre l'op�rande dans l'objet
				inst->m_leftop = left;
				// et rend l'object � qui l'a demand�
				return pStack->Return(inst, pStk);
			}
			pStk->SetError(TX_BAD2TYPE, &inst->m_token);
		}

		// en cas d'erreur, lib�re les �l�ments
		delete left;
		delete inst;
		// et transmet l'erreur qui se trouve sur la pile
		return pStack->Return(NULL, pStk);
	}

	// si on n'a pas affaire � une op�ration + ou -
	// rend � qui l'a demand�, l'op�rande (de gauche) trouv�
	// � la place de l'objet "addition"
	return pStack->Return(left, pStk);
}




// fait l'op�ration d'addition ou de soustraction

BOOL CBotAddExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack();		// ajoute un �l�ment � la pile
												// ou le retrouve en cas de reprise

	// selon la reprise, on peut �tre dans l'un des 2 �tats

	if ( pStk1->GetState() == 0 &&				// 1er �tat, �value l'op�rande de gauche
		!m_leftop->Execute(pStk1) ) return FALSE; // interrompu ici ?

	// passe � l'�tape suivante
	pStk1->SetState(1);							// pr�t pour la suite

	// demande un peu plus de stack pour ne pas toucher le r�sultat de gauche
	// qui se trouve sur la pile, justement.

	CBotStack* pStk2 = pStk1->AddStack();		// ajoute un �l�ment � la pile
												// ou le retrouve en cas de reprise

	// 2e �tat, �value l'op�rande de droite
	if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrompu ici ?

	int		type1 = pStk1->GetType();			// de quels types les r�sultats ?
	int		type2 = pStk2->GetType();

	// cr�e une variable temporaire pour y mettre le r�sultat
	CBotVar*	result = CBotVar::Create( NULL, MAX(type1, type2));

	// fait l'op�ration selon la demande
	switch (GetTokenType())
	{
	case ID_ADD:
		result->Add(pStk1->GetVar(), pStk2->GetVar());		// additionne
		break;
	case ID_SUB:
		result->Sub(pStk1->GetVar(), pStk2->GetVar());		// soustrait
		break;
	}
	pStk2->SetVar(result);						// met le r�sultat sur la pile

	pStk1->Return(pStk2);						// lib�re la pile
	return pStack->Return(pStk1);				// transmet le r�sultat
}


