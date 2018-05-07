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

#include "CBot/CBotInstr/CBotDo.h"
#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotCondition.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDo::CBotDo()
{
    m_condition = nullptr;
    m_block = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDo::~CBotDo()
{
    delete m_condition;    // frees the condition
    delete m_block;        // frees the instruction block
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotDo::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotDo* inst = new CBotDo();                // creates the object

    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GetString();        // register the name of label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_DO)) return nullptr;       // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp


    // looking for a statement block after the do
    IncLvl(inst->m_label);
    inst->m_block = CBotBlock::CompileBlkOrInst(p, pStk, true );
    DecLvl();

    if ( pStk->IsOk() )
    {
        if (IsOfType(p, ID_WHILE))
        {
            if ( nullptr != (inst->m_condition = CBotCondition::Compile(p, pStk )) )
            {
                // the condition exists
                if (IsOfType(p, ID_SEP))
                {
                    return pStack->Return(inst, pStk);  // return an object to the application
                }
                pStk->SetError(CBotErrNoTerminator, p->GetStart());
            }
        }
        pStk->SetError(CBotErrNoWhile, p->GetStart());
    }

    delete inst;                                // error, frees up
    return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDo :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);       // adds an item to the stack
                                                // or find in case of recovery
//  if ( pile == EOX ) return true;

    if ( pile->IfStep() ) return false;

    while( true ) switch( pile->GetState() )            // executes the loop
    {                                                   // there are two possible states (depending on recovery)
    case 0:
        // evaluates the associated statement block
        if (m_block != nullptr &&
            !m_block->Execute(pile) )
        {
            if (pile->IfContinue(1, m_label)) continue; // if continued, will return to test
            return pj->BreakReturn(pile, m_label);      // sends the results and releases the stack
        }

        // terminates if there is an error
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // sends the results and releases the stack
        }

        if (!pile->SetState(1)) return false;           // ready for further

    case 1:
        // evaluates the condition
        if ( !m_condition->Execute(pile) ) return false; // interrupted here ?

        // the result of the condition is on the stack

        // terminates if an error or if the condition is false
        if ( !pile->IsOk() || pile->GetVal() != true )
        {
            return pj->Return(pile);                    // sends the results and releases the stack
        }

        // returns to instruction block to start
        if (!pile->SetState(0, 0)) return false;
        continue;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotDo :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack* pile = pj->RestoreStack(this);           // adds an item to the stack
    if ( pile == nullptr ) return;

    switch( pile->GetState() )
    {                                                   // there are two possible states (depending on recovery)
    case 0:
        // restores the assosiated statement's block
        if (m_block != nullptr ) m_block->RestoreState(pile, bMain);
        return;

    case 1:
        // restores the condition
        m_condition->RestoreState(pile, bMain);
        return;
    }
}

std::string CBotDo::GetDebugData()
{
    return !m_label.empty() ? "m_label = "+m_label : "";
}

std::map<std::string, CBotInstr*> CBotDo::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_block"] = m_block;
    links["m_condition"] = m_condition;
    return links;
}

} // namespace CBot
