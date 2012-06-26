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
// * along with this program. If not, see  http://www.gnu.org/licenses/.///////////////////////////////////////////////////////////////////////
// instruction if (condition) opération1 else opération2;

#include "CBot.h"

// les divers constructeurs / destructeurs
CBotIf::CBotIf()
{
    m_Condition =
    m_Block     =
    m_BlockElse = NULL;         // NULL pour pouvoir faire delete directement
    name = "CBotIf";            // debug
}

CBotIf::~CBotIf()
{
    delete  m_Condition;        // libère la condition
    delete  m_Block;            // libère le bloc d'instruction1
    delete  m_BlockElse;        // libère le bloc d'instruction2
}

// compilation (routine statique)
// appelé lorsque le token "if" a été trouvé

CBotInstr* CBotIf::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;                         // conserve le ^au token (début instruction)

    if (!IsOfType(p, ID_IF)) return NULL;       // ne doit jamais arriver

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    CBotIf* inst = new CBotIf();                // crée l'object
    inst->SetToken( pp );

    if ( NULL != (inst->m_Condition = CBotCondition::Compile( p, pStk )) )
    {
        // la condition existe bel et bien

        inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
        if ( pStk->IsOk() )
        {
            // le bloc d'instruction est ok (peut être vide)

            // regarde si l'instruction suivante est le token "else"
            if (IsOfType(p, ID_ELSE))
            {
                // si oui, compile le bloc d'instruction qui suit
                inst->m_BlockElse = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
                if (!pStk->IsOk())
                {
                    // il n'y a pas de bloc correct après le else
                    // libère l'objet, et transmet l'erreur qui est sur la pile
                    delete inst;
                    return pStack->Return(NULL, pStk);
                }
            }

            // rend l'object correct à qui le demande.
            return pStack->Return(inst, pStk);
        }
    }

    // erreur, libère l'objet
    delete inst;
    // et transmet l'erreur qui se trouve sur la pile.
    return pStack->Return(NULL, pStk);
}


// exécution de l'instruction

BOOL CBotIf :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);       // ajoute un élément à la pile
                                                // ou le retrouve en cas de reprise
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    // selon la reprise, on peut être dans l'un des 2 états
    if( pile->GivState() == 0 )
    {
        // évalue la condition
        if ( !m_Condition->Execute(pile) ) return FALSE;    // interrompu ici ?

        // termine s'il y a une erreur
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                        // transmet le résultat et libère la pile
        }

        // passe dans le second état
        if (!pile->SetState(1)) return FALSE;               // prêt pour la suite
    }

    // second état, évalue les instructions associées
    // le résultat de la condition est sur la pile

    if ( pile->GivVal() == TRUE )                           // condition était vraie ?
    {
        if ( m_Block != NULL &&                             // bloc peut être absent
            !m_Block->Execute(pile) ) return FALSE;         // interrompu ici ?
    }
    else
    {
        if ( m_BlockElse != NULL &&                         // s'il existe un bloc alternatif
            !m_BlockElse->Execute(pile) ) return FALSE;     // interrompu ici
    }

    // transmet le résultat et libère la pile
    return pj->Return(pile);
}


void CBotIf :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack* pile = pj->RestoreStack(this);       // ajoute un élément à la pile
    if ( pile == NULL ) return;

    // selon la reprise, on peut être dans l'un des 2 états
    if( pile->GivState() == 0 )
    {
        // évalue la condition
        m_Condition->RestoreState(pile, bMain); // interrompu ici !
        return;
    }

    // second état, évalue les instructions associées
    // le résultat de la condition est sur la pile

    if ( pile->GivVal() == TRUE )                           // condition était vraie ?
    {
        if ( m_Block != NULL )                              // bloc peut être absent
             m_Block->RestoreState(pile, bMain);            // interrompu ici !
    }
    else
    {
        if ( m_BlockElse != NULL )                          // s'il existe un bloc alternatif
             m_BlockElse->RestoreState(pile, bMain);        // interrompu ici !
    }
}

