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

CBotLogicExpr::CBotLogicExpr()
{
	m_condition	=
	m_op1		=
	m_op2		= NULL;			// NULL pour pouvoir faire delete sans autre
	name = "CBotLogicExpr";		// debug
}

CBotLogicExpr::~CBotLogicExpr()
{
	delete	m_condition;
	delete	m_op1;
	delete	m_op2;
}


// type d'op�randes accept�s par les op�rations
#define		ENTIER		((1<<CBotTypByte)|(1<<CBotTypShort)|(1<<CBotTypChar)|(1<<CBotTypInt)|(1<<CBotTypLong))
#define		FLOTANT		((1<<CBotTypFloat)|(1<<CBotTypDouble))
#define		BOOLEEN		(1<<CBotTypBoolean)
#define		CHAINE		(1<<CBotTypString)
#define		POINTER		(1<<CBotTypPointer)
#define		INSTANCE	(1<<CBotTypClass)

// liste des op�rations (pr�c�ance)
//	type acceptable, op�rande
//	le z�ro termine un niveau de pr�c�ance

static int	ListOp[] = 
{
	BOOLEEN,				ID_LOGIC, 0,
	BOOLEEN,				ID_TXT_OR, 
	BOOLEEN,				ID_LOG_OR, 0,
	BOOLEEN,				ID_TXT_AND,
	BOOLEEN,				ID_LOG_AND, 0,
	BOOLEEN|ENTIER,			ID_OR, 0,
	BOOLEEN|ENTIER,			ID_XOR, 0,
	BOOLEEN|ENTIER,			ID_AND, 0,
	BOOLEEN|ENTIER|FLOTANT
				  |CHAINE
				  |POINTER
				  |INSTANCE,ID_EQ, 
	BOOLEEN|ENTIER|FLOTANT
				  |CHAINE
				  |POINTER
				  |INSTANCE,ID_NE, 0,
	ENTIER|FLOTANT|CHAINE,	ID_HI,
	ENTIER|FLOTANT|CHAINE,	ID_LO,
	ENTIER|FLOTANT|CHAINE,	ID_HS,
	ENTIER|FLOTANT|CHAINE,	ID_LS, 0,
	ENTIER,					ID_SR,
	ENTIER,					ID_SL,
	ENTIER,					ID_ASR, 0,
	ENTIER|FLOTANT|CHAINE,	ID_ADD,
	ENTIER|FLOTANT,			ID_SUB, 0,
	ENTIER|FLOTANT,			ID_MUL, 
	ENTIER|FLOTANT,			ID_DIV, 
	ENTIER|FLOTANT,			ID_MODULO, 0,
	ENTIER|FLOTANT,			ID_POWER, 0,
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
	int	TypeOp = p->GivType();
	if ( IsInList( TypeOp, pOperations, typemasque ) )
	{
		CBotTypResult	 type1, type2;
		type1 = pStk->GivTypResult();							// de quel type le premier op�rande ?

		if ( TypeOp == ID_LOGIC )		// cas sp�cial pour condition ? op1 : op2 ;
		{
			if ( !type1.Eq(CBotTypBoolean) )
			{
				pStk->SetError( TX_BADTYPE, p);
				return pStack->Return(NULL, pStk);
			}
			CBotLogicExpr* inst = new CBotLogicExpr();
			inst->m_condition = left;

			p = p->GivNext();										// saute le token de l'op�ration 
			inst->m_op1 = CBotExpression::Compile(p, pStk);
			CBotToken* pp = p;
			if ( inst->m_op1 == NULL || !IsOfType( p, ID_DOTS ) )
			{
				pStk->SetError( TX_MISDOTS, p->GivStart());
				delete inst;
				return pStack->Return(NULL, pStk);
			}
			type1 = pStk->GivTypResult();

			inst->m_op2 = CBotExpression::Compile(p, pStk);
			if ( inst->m_op2 == NULL )
			{
				pStk->SetError( TX_ENDOF, p->GivStart() );
				delete inst;
				return pStack->Return(NULL, pStk);
			}
			type2 = pStk->GivTypResult();
			if (!TypeCompatible(type1, type2))
			{
				pStk->SetError( TX_BAD2TYPE, pp );
				delete inst;
				return pStack->Return(NULL, pStk);
			}

			pStk->SetType(type1);		// le plus grand des 2 types

			return pStack->Return(inst, pStk);
		}

		CBotTwoOpExpr* inst = new CBotTwoOpExpr();				// �l�ment pour op�ration
		inst->SetToken(p);										// m�morise l'op�ration


		p = p->GivNext();											// saute le token de l'op�ration 

		// cherche des instructions qui peuvent convenir � droite

		if ( NULL != (inst->m_rightop = CBotTwoOpExpr::Compile( p, pStk, pOp )) )
																// expression (...) � droite
		{
			// il y a un second op�rande acceptable

			type2 = pStk->GivTypResult();						// de quel type le r�sultat ?

			// quel est le type du r�sultat ?
			int	TypeRes = MAX( type1.GivType(3), type2.GivType(3) );
			if ( TypeOp == ID_ADD && type1.Eq(CBotTypString) )
			{
				TypeRes = CBotTypString;
				type2 = type1;	// tout type convertible en cha�ne
			}
			else if ( TypeOp == ID_ADD && type2.Eq(CBotTypString) )
			{
				TypeRes = CBotTypString;
				type1 = type2;	// tout type convertible en cha�ne
			}
			else if (!TypeOk( TypeRes, typemasque )) type1.SetType(99);// erreur de type

			switch ( TypeOp )
			{
			case ID_LOG_OR:
			case ID_LOG_AND:
			case ID_TXT_OR:
			case ID_TXT_AND:
			case ID_EQ:
			case ID_NE:
			case ID_HI:
			case ID_LO:
			case ID_HS:
			case ID_LS:
				TypeRes = CBotTypBoolean;
			}
			if ( TypeCompatible (type1, type2, TypeOp ) )				// les r�sultats sont-ils compatibles
			{
				// si ok, enregistre l'op�rande dans l'objet
				inst->m_leftop = left;

				// sp�cial pour �valuer les op�rations de m�me niveau de gauche � droite
				while ( IsInList( p->GivType(), pOperations, typemasque ) ) // m�me(s) op�ration(s) suit ?
				{
					TypeOp = p->GivType();
					CBotTwoOpExpr* i = new CBotTwoOpExpr();				// �l�ment pour op�ration
					i->SetToken(p);										// m�morise l'op�ration
					i->m_leftop = inst;									// op�rande de gauche
					type1 = TypeRes;

					p = p->GivNext();										// avance � la suite
					i->m_rightop = CBotTwoOpExpr::Compile( p, pStk, pOp );
					type2 = pStk->GivTypResult();

					if ( !TypeCompatible (type1, type2, TypeOp) )		// les r�sultats sont-ils compatibles
					{
						pStk->SetError(TX_BAD2TYPE, &i->m_token);
						delete i;
						return pStack->Return(NULL, pStk);
					}

					if ( TypeRes != CBotTypString )
						TypeRes = MAX(type1.GivType(), type2.GivType());
					inst = i;
				}

				CBotTypResult t(type1);
					t.SetType(TypeRes);
				// met une variable sur la pile pour avoir le type de r�sultat
				pStk->SetVar(CBotVar::Create((CBotToken*)NULL, t));

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


BOOL IsNan(CBotVar* left, CBotVar* right, int* err = NULL)
{
	if ( left ->GivInit() > IS_DEF || right->GivInit() > IS_DEF )
	{
		if ( err != NULL ) *err = TX_OPNAN ;
		return TRUE;
	}
	return FALSE;
}


// fait l'op�ration sur 2 op�randes

BOOL CBotTwoOpExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack(this);	// ajoute un �l�ment � la pile
												// ou le retrouve en cas de reprise
//	if ( pStk1 == EOX ) return TRUE;

	// selon la reprise, on peut �tre dans l'un des 2 �tats

	if ( pStk1->GivState() == 0 )					// 1er �tat, �value l'op�rande de gauche
	{
		if (!m_leftop->Execute(pStk1) ) return FALSE;	// interrompu ici ?

		// pour les OU et ET logique, n'�value pas la seconde expression si pas n�cessaire
		if ( (GivTokenType() == ID_LOG_AND || GivTokenType() == ID_TXT_AND ) && pStk1->GivVal() == FALSE )
		{
			CBotVar*	res = CBotVar::Create( (CBotToken*)NULL, CBotTypBoolean);
			res->SetValInt(FALSE);
			pStk1->SetVar(res);
			return pStack->Return(pStk1);				// transmet le r�sultat
		}
		if ( (GivTokenType() == ID_LOG_OR||GivTokenType() == ID_TXT_OR) && pStk1->GivVal() == TRUE )
		{
			CBotVar*	res = CBotVar::Create( (CBotToken*)NULL, CBotTypBoolean);
			res->SetValInt(TRUE);
			pStk1->SetVar(res);
			return pStack->Return(pStk1);				// transmet le r�sultat
		}

		// passe � l'�tape suivante
		pStk1->SetState(1);			// pr�t pour la suite
	}


	// demande un peu plus de stack pour ne pas toucher le r�sultat de gauche
	// qui se trouve sur la pile, justement.

	CBotStack* pStk2 = pStk1->AddStack();				// ajoute un �l�ment � la pile
														// ou le retrouve en cas de reprise

	// 2e �tat, �value l'op�rande de droite
	if ( pStk2->GivState() == 0 )
	{
		if ( !m_rightop->Execute(pStk2) ) return FALSE;		// interrompu ici ?
		pStk2->IncState();
	}

	CBotTypResult		type1 = pStk1->GivTypResult();		// de quels types les r�sultats ?
	CBotTypResult		type2 = pStk2->GivTypResult();

	CBotStack* pStk3 = pStk2->AddStack(this);				// ajoute un �l�ment � la pile
	if ( pStk3->IfStep() ) return FALSE;					// montre l'op�ration si step by step

	// cr�e une variable temporaire pour y mettre le r�sultat
	// quel est le type du r�sultat ?
	int	TypeRes = MAX(type1.GivType(), type2.GivType());

	if ( GivTokenType() == ID_ADD && type1.Eq(CBotTypString) )
	{
		TypeRes = CBotTypString;
	}

	switch ( GivTokenType() )
	{
	case ID_LOG_OR:
	case ID_LOG_AND:
	case ID_TXT_OR:
	case ID_TXT_AND:
	case ID_EQ:
	case ID_NE:
	case ID_HI:
	case ID_LO:
	case ID_HS:
	case ID_LS:
		TypeRes = CBotTypBoolean;
		break;
	case ID_DIV:
		TypeRes = MAX(TypeRes, CBotTypFloat);
	}

	// cr�e une variable pour le r�sultat
	CBotVar*	result = CBotVar::Create( (CBotToken*)NULL, TypeRes);

	// cr�e une variable pour effectuer le calcul dans le type adapt�
	TypeRes = MAX(type1.GivType(), type2.GivType());

	if ( GivTokenType() == ID_ADD && type1.Eq(CBotTypString) )
	{
		TypeRes = CBotTypString;
	}

	CBotVar*	temp;

	if ( TypeRes == CBotTypPointer ) TypeRes = CBotTypNullPointer;
	if ( TypeRes == CBotTypClass ) temp = CBotVar::Create( (CBotToken*)NULL, CBotTypResult(CBotTypIntrinsic, type1.GivClass() ) );
	else						   temp = CBotVar::Create( (CBotToken*)NULL, TypeRes );

	int	err = 0;
	// fait l'op�ration selon la demande
	CBotVar*	left  = pStk1->GivVar();
	CBotVar*	right = pStk2->GivVar();

	switch (GivTokenType())
	{
	case ID_ADD:
		if ( !IsNan(left, right, &err) )	result->Add(left , right);		// additionne
		break;
	case ID_SUB:
		if ( !IsNan(left, right, &err) )	result->Sub(left , right);		// soustrait
		break;
	case ID_MUL:
		if ( !IsNan(left, right, &err) )	result->Mul(left , right);		// multiplie
		break;
	case ID_POWER:
		if ( !IsNan(left, right, &err) )	result->Power(left , right);	// puissance
		break;
	case ID_DIV:
		if ( !IsNan(left, right, &err) )	err = result->Div(left , right);// divise
		break;
	case ID_MODULO:
		if ( !IsNan(left, right, &err) )	err = result->Modulo(left , right);// reste de division
		break;
	case ID_LO:
		if ( !IsNan(left, right, &err) )
			result->SetValInt(temp->Lo(left , right));	// inf�rieur
		break;
	case ID_HI:
		if ( !IsNan(left, right, &err) )
			result->SetValInt(temp->Hi(left , right));	// sup�rieur
		break;
	case ID_LS:
		if ( !IsNan(left, right, &err) )
			result->SetValInt(temp->Ls(left , right));	// inf�rieur ou �gal
		break;
	case ID_HS:
		if ( !IsNan(left, right, &err) )
			result->SetValInt(temp->Hs(left , right));	// sup�rieur ou �gal
		break;
	case ID_EQ:
		if ( IsNan(left, right) )
			result->SetValInt(left->GivInit() ==  right->GivInit()) ;
		else
			result->SetValInt(temp->Eq(left , right));	// �gal
		break;
	case ID_NE:
		if ( IsNan(left, right) )
			 result->SetValInt(left ->GivInit() !=  right->GivInit()) ;
		else
			result->SetValInt(temp->Ne(left , right));	// diff�rent
		break;
	case ID_TXT_AND:
	case ID_LOG_AND:
	case ID_AND:
		if ( !IsNan(left, right, &err) )	result->And(left , right);		// ET
		break;
	case ID_TXT_OR:
	case ID_LOG_OR:
	case ID_OR:
		if ( !IsNan(left, right, &err) )	result->Or(left , right);		// OU
		break;
	case ID_XOR:
		if ( !IsNan(left, right, &err) )	result->XOr(left , right);		// OU exclusif
		break;
	case ID_ASR:
		if ( !IsNan(left, right, &err) )	result->ASR(left , right);
		break;
	case ID_SR:
		if ( !IsNan(left, right, &err) )	result->SR(left , right);
		break;
	case ID_SL:
		if ( !IsNan(left, right, &err) )	result->SL(left , right);
		break;
	default:
		__asm int 3;
	}
	delete temp;

	pStk2->SetVar(result);						// met le r�sultat sur la pile
	if ( err ) pStk2->SetError(err, &m_token);	// et l'erreur �ventuelle (division par z�ro)

//	pStk1->Return(pStk2);						// lib�re la pile
	return pStack->Return(pStk2);				// transmet le r�sultat
}

void CBotTwoOpExpr::RestoreState(CBotStack* &pStack, BOOL bMain)
{
	if ( !bMain ) return;
	CBotStack* pStk1 = pStack->RestoreStack(this);	// ajoute un �l�ment � la pile
	if ( pStk1 == NULL ) return;

	// selon la reprise, on peut �tre dans l'un des 2 �tats

	if ( pStk1->GivState() == 0 )					// 1er �tat, �value l'op�rande de gauche
	{
		m_leftop->RestoreState(pStk1, bMain);		// interrompu ici !
		return;
	}

	CBotStack* pStk2 = pStk1->RestoreStack();			// ajoute un �l�ment � la pile
	if ( pStk2 == NULL ) return;

	// 2e �tat, �value l'op�rande de droite
	if ( pStk2->GivState() == 0 )
	{
		m_rightop->RestoreState(pStk2, bMain);			// interrompu ici !
		return;
	}
}


BOOL CBotLogicExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack(this);	// ajoute un �l�ment � la pile
												// ou le retrouve en cas de reprise
//	if ( pStk1 == EOX ) return TRUE;

	if ( pStk1->GivState() == 0 )
	{
		if ( !m_condition->Execute(pStk1) ) return FALSE;
		if (!pStk1->SetState(1)) return FALSE;
	}

	if ( pStk1->GivVal() == TRUE )
	{
		if ( !m_op1->Execute(pStk1) ) return FALSE;
	}
	else
	{
		if ( !m_op2->Execute(pStk1) ) return FALSE;
	}

	return pStack->Return(pStk1);					// transmet le r�sultat
}

void CBotLogicExpr::RestoreState(CBotStack* &pStack, BOOL bMain)
{
	if ( !bMain ) return;

	CBotStack* pStk1 = pStack->RestoreStack(this);	// ajoute un �l�ment � la pile
	if ( pStk1 == NULL ) return;

	if ( pStk1->GivState() == 0 )
	{
		m_condition->RestoreState(pStk1, bMain);
		return;
	}

	if ( pStk1->GivVal() == TRUE )
	{
		m_op1->RestoreState(pStk1, bMain);
	}
	else
	{
		m_op2->RestoreState(pStk1, bMain);
	}
}

#if 0
void t()
{
	int x,y;
	1>0 ? x = 0 : y = 0;
}
#endif

#if 01
void t(BOOL t)
{
	int	 x;
	x = 1 + t ? 1 : 3 + 4 * 2 ;
	t ? 0 : "test";
}
#endif
