/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotFor.h"
#include "CBot/CBotInstr/CBotListExpression.h"
#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotBoolExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotFor::CBotFor()
{
    m_init = nullptr;
    m_test = nullptr;
    m_incr = nullptr;
    m_block = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotFor::~CBotFor()
{
    delete m_init;
    delete m_test;
    delete m_incr;
    delete m_block;        // frees the instruction block
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotFor::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotFor*    inst = new CBotFor();           // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GetString();        // register the name of label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_FOR)) return nullptr;      // should never happen

    if ( !IsOfType(p, ID_OPENPAR))              // missing parenthesis ?
    {
        pStack->SetError(CBotErrOpenPar, p->GetStart());
        return nullptr;
    }

    CBotCStack* pStk = pStack->TokenStack(pp, true);    // un petit bout de pile svp

    // compiles instructions for initialization
    inst->m_init = CBotListExpression::Compile(p, pStk );
    if ( pStk->IsOk() )
    {
        if ( !IsOfType(p, ID_SEP))                      // lack the semicolon?
        {
            pStack->SetError(CBotErrOpenPar, p->GetStart());
            delete inst;
            return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
        }
        inst->m_test = CBotBoolExpr::Compile(p, pStk );
        if ( pStk->IsOk() )
        {
            if ( !IsOfType(p, ID_SEP))                      // lack the semicolon?
            {
                pStack->SetError(CBotErrOpenPar, p->GetStart());
                delete inst;
                return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
            }
            inst->m_incr = CBotListExpression::Compile(p, pStk );
            if ( pStk->IsOk() )
            {
                if ( IsOfType(p, ID_CLOSEPAR))              // missing parenthesis ?
                {
                    IncLvl(inst->m_label);
                    inst->m_block = CBotBlock::CompileBlkOrInst(p, pStk, true );
                    DecLvl();
                    if ( pStk->IsOk() )
                        return pStack->Return(inst, pStk);;
                }
                pStack->SetError(CBotErrClosePar, p->GetStart());
            }
        }
    }

    delete inst;                                // error, frees up
    return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
}

// execution of instruction "for"
////////////////////////////////////////////////////////////////////////////////
bool CBotFor :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this, CBotStack::BlockVisibilityType::BLOCK);     // adds an item to the stack (variables locales)
                                                    // or find in case of recovery
//  if ( pile == EOX ) return true;

    if ( pile->IfStep() ) return false;

    while( true ) switch( pile->GetState() )    // executes the loop
    {                                           // there are four possible states (depending on recovery)
    case 0:
        // initialize
        if (m_init != nullptr &&
            !m_init->Execute(pile) ) return false;     // interrupted here ?
        if (!pile->SetState(1)) return false;           // ready for further

    case 1:
        // evaluates the condition
        if (m_test != nullptr )                           // no strings attached? -> True!
        {
            if (!m_test->Execute(pile) ) return false;  // interrupted here ?

            // the result of the condition is on the stack

            // terminates if an error or if the condition is false
            if ( !pile->IsOk() || pile->GetVal() != true )
            {
                return pj->Return(pile);                // sends the results and releases the stack
            }
        }

        // la condition est vrai, passe à la suite
        if (!pile->SetState(2)) return false;           // ready for further

    case 2:
        // evaluates the associated statement block
        if (m_block != nullptr &&
            !m_block->Execute(pile) )
        {
            if (pile->IfContinue(3, m_label)) continue; // if continued, going on to incrementation
            return pj->BreakReturn(pile, m_label);      // sends the results and releases the stack
        }

        // terminates if there is an error
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // sends the results and releases the stack
        }

        if (!pile->SetState(3)) return false;           // ready for further

    case 3:
        // evalutate the incrementation
        if (m_incr != nullptr &&
            !m_incr->Execute(pile) ) return false;      // interrupted here ?

        // returns to the test again
        if (!pile->SetState(1, 0)) return false;            // returns to the test
        continue;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotFor :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack* pile = pj->RestoreStack(this);       // adds an item to the stack (variables locales)
    if ( pile == nullptr ) return;

    switch( pile->GetState() )
    {                                           // there are four possible states (depending on recovery)
    case 0:
        // initialize
        if (m_init != nullptr ) m_init->RestoreState(pile, true);     // interrupted here !
        return;

    case 1:
        if (m_init != nullptr ) m_init->RestoreState(pile, false);    // variables definitions

        // evaluates the condition
        if (m_test != nullptr ) m_test->RestoreState(pile, true);     // interrupted here !
        return;

    case 2:
        if (m_init != nullptr ) m_init->RestoreState(pile, false);    // variable definitions

        // evaluates the associated statement block
        if (m_block != nullptr ) m_block->RestoreState(pile, true);
        return;

    case 3:
        if (m_init != nullptr ) m_init->RestoreState(pile, false);    // variable definitions

        // evaluate the incrementation
        if (m_incr != nullptr ) m_incr->RestoreState(pile, true);     // interrupted here !
        return;
    }
}

std::string CBotFor::GetDebugData()
{
    return !m_label.empty() ? "m_label = "+m_label : "";
}

std::map<std::string, CBotInstr*> CBotFor::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_init"] = m_init;
    links["m_test"] = m_test;
    links["m_incr"] = m_incr;
    links["m_block"] = m_block;
    return links;
}

} // namespace CBot
