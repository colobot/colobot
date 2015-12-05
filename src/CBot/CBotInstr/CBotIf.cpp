/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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

// Modules inlcude
#include "CBot/CBotInstr/CBotIf.h"
#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotCondition.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotIf::CBotIf()
{
    m_Condition =
    m_Block     =
    m_BlockElse = nullptr;         // nullptr so that delete is not possible further
    name = "CBotIf";            // debug
}

////////////////////////////////////////////////////////////////////////////////
CBotIf::~CBotIf()
{
    delete  m_Condition;        // frees the condition
    delete  m_Block;            // frees the block of instruction1
    delete  m_BlockElse;        // frees the block of instruction2
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotIf::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;                         // preserves at the ^ token (starting instruction)

    if (!IsOfType(p, ID_IF)) return nullptr;       // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    CBotIf* inst = new CBotIf();                // create the object
    inst->SetToken( pp );

    if ( nullptr != (inst->m_Condition = CBotCondition::Compile( p, pStk )) )
    {
        // the condition does exist

        inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, true );
        if ( pStk->IsOk() )
        {
            // the statement block is ok (can be empty)

            // see if the next instruction is the token "else"
            if (IsOfType(p, ID_ELSE))
            {
                // if so, compiles the following statement block
                inst->m_BlockElse = CBotBlock::CompileBlkOrInst( p, pStk, true );
                if (!pStk->IsOk())
                {
                    // there is no correct block after the else
                    // frees the object, and transmits the error that is on the stack
                    delete inst;
                    return pStack->Return(nullptr, pStk);
                }
            }

            // return the corrent object to the application
            return pStack->Return(inst, pStk);
        }
    }

    // error, frees the object
    delete inst;
    // and transmits the error that is on the stack.
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotIf :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);       // adds an item to the stack
                                                // or found in case of recovery
//  if ( pile == EOX ) return true;

    if ( pile->IfStep() ) return false;

    // according to recovery, it may be in one of two states
    if( pile->GetState() == 0 )
    {
        // evaluates the condition
        if ( !m_Condition->Execute(pile) ) return false;    // interrupted here?

        // terminates if there is an error
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                        // returns the results and releases the stack
        }

        // passes into the second state
        if (!pile->SetState(1)) return false;               // ready for further
    }

    // second state, evaluates the associated instructions
    // the result of the condition is on the stack

    if ( pile->GetVal() == true )                           // condition was true?
    {
        if ( m_Block != nullptr &&                             // block may be absent
            !m_Block->Execute(pile) ) return false;         // interrupted here?
    }
    else
    {
        if ( m_BlockElse != nullptr &&                         // if there is an alternate block
            !m_BlockElse->Execute(pile) ) return false; // interrupted here
    }

    // sends the results and releases the stack
    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotIf :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack* pile = pj->RestoreStack(this);       // adds an item to the stack
    if ( pile == nullptr ) return;

    // according to recovery, it may be in one of two states
    if( pile->GetState() == 0 )
    {
        // evaluates the condition
        m_Condition->RestoreState(pile, bMain); // interrupted here!
        return;
    }

    // second state, evaluates the associated instructions
    // the result of the condition is on the stack

    if ( pile->GetVal() == true )                           // condition was true?
    {
        if ( m_Block != nullptr )                              // block may be absent
             m_Block->RestoreState(pile, bMain);            // interrupted here!
    }
    else
    {
        if ( m_BlockElse != nullptr )                          // if there is an alternate block
             m_BlockElse->RestoreState(pile, bMain);        // interrupted here!
    }
}

