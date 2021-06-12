/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotRepeat.h"

#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotCStack.h"
#include "CBot/CBotStack.h"

namespace CBot
{

CBotRepeat::CBotRepeat()
{
    m_expr  = nullptr;
    m_block = nullptr;
}

CBotRepeat::~CBotRepeat()
{
    delete m_expr;
    delete m_block;
}

CBotInstr* CBotRepeat::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotRepeat* inst = new CBotRepeat();         // creates the object
    CBotToken*  pp = p;                          // preserves at the ^ token (starting position)

    if ( IsOfType( p, TokenTypVar ) && IsOfType( p, ID_DOTS ) )
        inst->m_label = pp->GetString();   // register the name of label

    inst->SetToken(p);
    if (!IsOfType(p, ID_REPEAT)) return nullptr; // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);

    if ( IsOfType(p, ID_OPENPAR ) )
    {
        CBotToken*  ppp = p;                     // preserves the ^ token (starting position)
        if ( nullptr != (inst->m_expr = CBotExpression::Compile( p, pStk )) )
        {
            if ( pStk->GetType() < CBotTypLong )
            {
                if ( IsOfType(p, ID_CLOSEPAR ) )
                {
                    IncLvl(inst->m_label);
                    inst->m_block = CBotBlock::CompileBlkOrInst( p, pStk, true );
                    DecLvl();

                    if ( pStk->IsOk() ) // the statement block is ok (it may be empty!)
                        return pStack->Return(inst, pStk);
                }
                pStack->SetError(CBotErrClosePar, p->GetStart());
            }
            pStk->SetStartError(ppp->GetStart());
            pStk->SetError(CBotErrBadType1, p->GetStart());
        }
        pStack->SetError(CBotErrBadNum, p);
    }
    pStack->SetError(CBotErrOpenPar, p->GetStart());

    delete inst;
    return pStack->Return(nullptr, pStk);
}

// execution of intruction "repeat"

bool CBotRepeat::Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);   // adds an item to the stack
                                            // or find in case of recovery
    if ( pile->IfStep() ) return false;

    while( true ) switch( pile->GetState() )    // executes the loop
    {                                           // there are two possible states (depending on recovery)
    case 0:
        // evaluates the number of iterations
        if ( !m_expr->Execute(pile) ) return false; // interrupted here ?

        // the result of the condition is on the stack

        // terminates if an error or if the condition is false
        int n;
        if ( !pile->IsOk() || ( n = pile->GetVal() ) < 1 )
            return pj->Return(pile); // releases the stack

        // puts the number of iterations +1 to the "state"

        if (!pile->SetState(n+1)) return false;         // ready for further
        continue;                                       // continue as a result

    case 1:
        // normal end of the loop
        return pj->Return(pile); //  releases the stack

    default:
        // evaluates the associated statement block
        if ( m_block != nullptr && !m_block->Execute(pile) )
        {
            if (pile->IfContinue(pile->GetState()-1, m_label)) continue;    // if continued, will return to test
            return pj->BreakReturn(pile, m_label); // releases the stack
        }

        // terminates if there is an error
        if (!pile->IsOk()) return pj->Return(pile); // releases the stack

        // returns to the test again
        if (!pile->SetState(pile->GetState()-1, 0)) return false;
        continue;
    }
}

void CBotRepeat::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;
    CBotStack* pile = pj->RestoreStack(this);   // adds an item to the stack
    if ( pile == nullptr ) return;

    switch( pile->GetState() )
    {                                           // there are two possible states (depending on recovery)
    case 0:
        // evaluates the condition
        m_expr->RestoreState(pile, bMain);
        return;

    case 1:
        // evaluates the associated statement block
        if ( m_block != nullptr ) m_block->RestoreState(pile, bMain);
        return;
    }
}

std::string CBotRepeat::GetDebugData()
{
    return !m_label.empty() ? "m_label = " + m_label : "";
}

std::map<std::string, CBotInstr*> CBotRepeat::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_expr"] = m_expr;
    links["m_block"] = m_block;
    return links;
}

} // namespace CBot
