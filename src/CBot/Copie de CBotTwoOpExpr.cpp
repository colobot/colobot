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

CBotTwoOpExpr::CBotTwoOpExpr()
{
	m_leftop	=
	m_rightop	= NULL;			// NULL pour pouvoir faire delete sans autre
	name = "CBotTwoOpExpr";		// debug
}

CBotTwoOpExpr::~CBotTwoOpExpr()
{
	delete	m_leftop;
	delete	m_rightop;
}

// type d'op�randes accept�s par les op�rations
#define		ENTIER		((1<<CBotTypByte)|(1<<CBotTypShort)|(1<<CBotTypChar)|(1<<CBotTypInt)|(1<<CBotTypLong))
#define		FLOTANT		((1<<CBotTypFloat)|(1<<CBotTypDouble))
#define		BOOLEEN		(1<<CBotTypBoolean)
#define		CHAINE		(1<<CBotTypString)
#define		CLASSE		(1<<CBotTypClass)

// liste des op�rations (pr�c�ance)
static int	ListOp[] = 
{
	BOOLEEN,				ID_LOG_OR, 0,
	BOOLEEN,				ID_LOG_AND, 0,
	BOOLEEN|ENTIER,			ID_OR, 0,
	ENTIER,					ID_XOR, 0,
	BOOLEEN|ENTIER,			ID_AND, 0,
	BOOLEEN|ENTIER|FLOTANT,	ID_EQ, 
	BOOLEEN|ENTIER|FLOTANT,	ID_NE, 0,
	ENTIER|FLOTANT,			ID_HI,
	ENTIER|FLOTANT,			ID_LO,
	ENTIER|FLOTANT,			ID_HS,
	ENTIER|FLOTANT,			ID_LS, 0,
	ENTIER,					ID_SR,
	ENTIER,					ID_SL,
	ENTIER,					ID_ASR, 0,
	ENTIER|FLOTANT|CHAINE,	ID_ADD,
	ENTIER|FLOTANT,			ID_SUB, 0,
	ENTIER|FLOTANT,			ID_MUL, 
	ENTIER|FLOTANT,			ID_DIV, 
	ENTIER|FLOTANT,			ID_MODULO, 0,
	0,
};

BOOL IsInList( int val, int* list, int& typemasque )
{
	while (TRUE)
	{
		if ( *list == 0 ) return FALSE;
		typemasque = *list++;
		if ( *list++ == val ) return TRUE;
	}
}

BOOL TypeOk( int type, int test )
{
	while (TRUE)
	{
		if ( type == 0 ) return (test & 1);
		type--; test /= 2;
	}
}

// compile une instruction de type A op B

CBotInstr* CBotTwoOpExpr::Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations)
{
	int	typemasque;

	if ( pOperations == NULL ) pOperations = ListOp;
	int* pOp = pOperations;
	while ( *pOp++ != 0 );				// suite de la table

	CBotCStack* pStk = pStack->TokenStack();					// un bout de pile svp

	// cherche des instructions qui peuvent convenir � gauche de l'op�ration
	CBotInstr*	left = (*pOp == 0) ?
						CBotParExpr::Compile( p, pStk ) :		// expression (...) � gauche
						CBotTwoOpExpr::Compile( p, pStk, pOp );	// expression A * B � gauche

	if (left == NULL) return pStack->Return(NULL, pStk);		// si erreur, la transmet

	// est-ce qu'on a l'op�rande pr�vu ensuite ?
	int	TypeOp = p->GetType();
	if ( IsInList( TypeOp, pOperations, typemasque ) )
	{
		CBotTwoOpExpr* inst = new CBotTwoOpExpr();				// �l�ment pour op�ration
		inst->SetToken(p);										// m�morise l'op�ration

		int			 type1, type2;
		type1 = pStk->GetType();								// de quel type le premier op�rande ?

		p = p->Next();											// saute le token de l'op�ration 

		// cherche des instructions qui peuvent convenir � droite

		if ( NULL != (inst->m_rightop = CBotTwoOpExpr::Compile( p, pStk, pOperations )) )
																// expression (...) � droite
		{
			// il y a un second op�rande acceptable

			type2 = pStk->GetType();							// de quel type le r�sultat ?

			// quel est le type du r�sultat ?
			int	TypeRes = MAX( type1, type2 );
			if (!TypeOk( TypeRes, typemasque )) type1 = 99;		// erreur de type

			switch ( TypeOp )
			{
			case ID_LOG_OR:
			case ID_LOG_AND:
			case ID_EQ:
			case ID_NE:
			case ID_HI:
			case ID_LO:
			case ID_HS:
			case ID_LS:
				TypeRes = CBotTypBoolean;
			}
			if ( TypeCompatible (type1, type2) ||				// les r�sultats sont-ils compatibles
			// cas particulier pour les concat�nation de cha�nes
				 (TypeOp == ID_ADD && (type1 == CBotTypString || type2 == CBotTypString)))
			{
				// si ok, enregistre l'op�rande dans l'objet
				inst->m_leftop = left;
				// met une variable sur la pile pour avoir le type de r�sultat
				pStk->SetVar(new CBotVar(NULL, TypeRes));
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

BOOL CBotTwoOpExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack();			// ajoute un �l�ment � la pile
													// ou le retrouve en cas de reprise

	// selon la reprise, on peut �tre dans l'un des 2 �tats

	if ( pStk1->GetState() == 0 &&					// 1er �tat, �value l'op�rande de gauche
		!m_leftop->Execute(pStk1) ) return FALSE;	// interrompu ici ?

	// passe � l'�tape suivante
	pStk1->SetState(1);								// pr�t pour la suite

	// pour les OU et ET logique, n'�value pas la seconde expression si pas n�cessaire
	if ( GetTokenType() == ID_LOG_AND && pStk1->GetVal() == FALSE )
	{
		CBotVar*	res = CBotVar::Create( NULL, CBotTypBoolean);
		res->SetValInt(FALSE);
		pStk1->SetVar(res);
		return pStack->Return(pStk1);				// transmet le r�sultat
	}
	if ( GetTokenType() == ID_LOG_OR && pStk1->GetVal() == TRUE )
	{
		CBotVar*	res = CBotVar::Create( NULL, CBotTypBoolean);
		res->SetValInt(TRUE);
		pStk1->SetVar(res);
		return pStack->Return(pStk1);				// transmet le r�sultat
	}

	// demande un peu plus de stack pour ne pas toucher le r�sultat de gauche
	// qui se trouve sur la pile, justement.

	CBotStack* pStk2 = pStk1->AddStack();			// ajoute un �l�ment � la pile
													// ou le retrouve en cas de reprise

	// 2e �tat, �value l'op�rande de droite
	if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrompu ici ?

	int		type1 = pStk1->GetType();				// de quels types les r�sultats ?
	int		type2 = pStk2->GetType();

	// cr�e une variable temporaire pour y mettre le r�sultat
	// quel est le type du r�sultat ?
	int	TypeRes = MAX(type1, type2);
	switch ( GetTokenType() )
	{
	case ID_LOG_OR:
	case ID_LOG_AND:
	case ID_EQ:
	case ID_NE:
	case ID_HI:
	case ID_LO:
	case ID_HS:
	case ID_LS:
		TypeRes = CBotTypBoolean;
	}
	CBotVar*	result = CBotVar::Create( NULL, TypeRes);
	CBotVar*	temp   = CBotVar::Create( NULL, MAX(type1, type2) );

	int	err = 0;
	// fait l'op�ration selon la demande
	switch (GetTokenType())
	{
	case ID_ADD:
		result->Add(pStk1->GetVar(), pStk2->GetVar());		// additionne
		break;
	case ID_SUB:
		result->Sub(pStk1->GetVar(), pStk2->GetVar());		// soustrait
		break;
	case ID_MUL:
		result->Mul(pStk1->GetVar(), pStk2->GetVar());		// multiplie
		break;
	case ID_DIV:
		err = result->Div(pStk1->GetVar(), pStk2->GetVar());// divise
		break;
	case ID_MODULO:
		err = result->Modulo(pStk1->GetVar(), pStk2->GetVar());// reste de division
		break;
	case ID_LO:
		temp->Lo(pStk1->GetVar(), pStk2->GetVar());			// inf�rieur
		result->SetValInt(temp->GetValInt());				// converti le r�sultat
		break;
	case ID_HI:
		temp->Hi(pStk1->GetVar(), pStk2->GetVar());			// sup�rieur
		result->SetValInt(temp->GetValInt());				// converti le r�sultat
		break;
	case ID_LS:
		temp->Ls(pStk1->GetVar(), pStk2->GetVar());			// inf�rieur ou �gal
		result->SetValInt(temp->GetValInt());				// converti le r�sultat
		break;
	case ID_HS:
		temp->Hs(pStk1->GetVar(), pStk2->GetVar());			// sup�rieur ou �gal
		result->SetValInt(temp->GetValInt());				// converti le r�sultat
		break;
	case ID_EQ:
		temp->Eq(pStk1->GetVar(), pStk2->GetVar());			// �gal
		result->SetValInt(temp->GetValInt());				// converti le r�sultat
		break;
	case ID_NE:
		temp->Ne(pStk1->GetVar(), pStk2->GetVar());			// diff�rent
		result->SetValInt(temp->GetValInt());				// converti le r�sultat
		break;
	case ID_LOG_AND:
	case ID_AND:
		result->And(pStk1->GetVar(), pStk2->GetVar());		// ET
		break;
	case ID_LOG_OR:
	case ID_OR:
		result->Or(pStk1->GetVar(), pStk2->GetVar());		// OU
		break;
	case ID_XOR:
		result->XOr(pStk1->GetVar(), pStk2->GetVar());		// OU exclusif
		break;
	case ID_ASR:
		result->ASR(pStk1->GetVar(), pStk2->GetVar());
		break;
	case ID_SR:
		result->SR(pStk1->GetVar(), pStk2->GetVar());
		break;
	case ID_SL:
		result->SL(pStk1->GetVar(), pStk2->GetVar());
		break;
	default:
		__asm int 3;
	}
	delete temp;

	pStk2->SetVar(result);						// met le r�sultat sur la pile
	if ( err ) pStk2->SetError(err, &m_token);	// et l'erreur �ventuelle (division par z�ro)

	pStk1->Return(pStk2);						// lib�re la pile
	return pStack->Return(pStk1);				// transmet le r�sultat
}


