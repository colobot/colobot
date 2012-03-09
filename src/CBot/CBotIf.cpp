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

// les divers constructeurs / destructeurs
CBotIf::CBotIf()
{
	m_Condition =
	m_Block		=
	m_BlockElse	= NULL;			// NULL pour pouvoir faire delete directement
	name = "CBotIf";			// debug
}

CBotIf::~CBotIf()
{
	delete	m_Condition;		// lib�re la condition
	delete	m_Block;			// lib�re le bloc d'instruction1
	delete	m_BlockElse;		// lib�re le bloc d'instruction2
}

// compilation (routine statique)
// appel� lorsque le token "if" a �t� trouv�

CBotInstr* CBotIf::Compile(CBotToken* &p, CBotCStack* pStack)
{
	CBotToken*	pp = p;							// conserve le ^au token (d�but instruction)

	if (!IsOfType(p, ID_IF)) return NULL;		// ne doit jamais arriver

	CBotCStack* pStk = pStack->TokenStack(pp);	// un petit bout de pile svp

	CBotIf*	inst = new CBotIf();				// cr�e l'object
	inst->SetToken( pp );

	if ( NULL != (inst->m_Condition = CBotCondition::Compile( p, pStk )) )
	{
		// la condition existe bel et bien

		inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
		if ( pStk->IsOk() )
		{
			// le bloc d'instruction est ok (peut �tre vide)

			// regarde si l'instruction suivante est le token "else"
			if (IsOfType(p, ID_ELSE))
			{
				// si oui, compile le bloc d'instruction qui suit
				inst->m_BlockElse = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
				if (!pStk->IsOk())
				{
					// il n'y a pas de bloc correct apr�s le else
					// lib�re l'objet, et transmet l'erreur qui est sur la pile
					delete inst;
					return pStack->Return(NULL, pStk);
				}
			}

			// rend l'object correct � qui le demande.
			return pStack->Return(inst, pStk);
		}
	}

	// erreur, lib�re l'objet
	delete inst;
	// et transmet l'erreur qui se trouve sur la pile.
	return pStack->Return(NULL, pStk);
}


// ex�cution de l'instruction

BOOL CBotIf :: Execute(CBotStack* &pj)
{
	CBotStack* pile = pj->AddStack(this);		// ajoute un �l�ment � la pile
												// ou le retrouve en cas de reprise
//	if ( pile == EOX ) return TRUE;

	if ( pile->IfStep() ) return FALSE;

	// selon la reprise, on peut �tre dans l'un des 2 �tats
	if( pile->GivState() == 0 )
	{
		// �value la condition
		if ( !m_Condition->Execute(pile) ) return FALSE;	// interrompu ici ?

		// termine s'il y a une erreur
		if ( !pile->IsOk() )
		{
			return pj->Return(pile);						// transmet le r�sultat et lib�re la pile
		}

		// passe dans le second �tat
		if (!pile->SetState(1)) return FALSE;				// pr�t pour la suite
	}
	
	// second �tat, �value les instructions associ�es
	// le r�sultat de la condition est sur la pile

	if ( pile->GivVal() == TRUE )							// condition �tait vraie ?
	{
		if ( m_Block != NULL &&								// bloc peut �tre absent
			!m_Block->Execute(pile) ) return FALSE;			// interrompu ici ?
	}
	else
	{
		if ( m_BlockElse != NULL &&							// s'il existe un bloc alternatif
			!m_BlockElse->Execute(pile) ) return FALSE;		// interrompu ici
	}

	// transmet le r�sultat et lib�re la pile
	return pj->Return(pile);
}


void CBotIf :: RestoreState(CBotStack* &pj, BOOL bMain)
{
	if ( !bMain ) return;

	CBotStack* pile = pj->RestoreStack(this);		// ajoute un �l�ment � la pile
	if ( pile == NULL ) return;

	// selon la reprise, on peut �tre dans l'un des 2 �tats
	if( pile->GivState() == 0 )
	{
		// �value la condition
		m_Condition->RestoreState(pile, bMain);	// interrompu ici !
		return;
	}
	
	// second �tat, �value les instructions associ�es
	// le r�sultat de la condition est sur la pile

	if ( pile->GivVal() == TRUE )							// condition �tait vraie ?
	{
		if ( m_Block != NULL )								// bloc peut �tre absent
			 m_Block->RestoreState(pile, bMain);			// interrompu ici !
	}
	else
	{
		if ( m_BlockElse != NULL )							// s'il existe un bloc alternatif
			 m_BlockElse->RestoreState(pile, bMain);		// interrompu ici !
	}
}

