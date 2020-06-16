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

#include "CBot/CBotInstr/CBotLogicExpr.h"

#include "CBot/CBotStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotLogicExpr::CBotLogicExpr()
{
    m_condition = nullptr;
    m_op1       = nullptr;
    m_op2       = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotLogicExpr::~CBotLogicExpr()
{
    delete  m_condition;
    delete  m_op1;
    delete  m_op2;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotLogicExpr::Execute(CBotStack* &pStack)
{
    CBotStack* pStk1 = pStack->AddStack(this);  // adds an item to the stack
                                                // or return in case of recovery
//  if ( pStk1 == EOX ) return true;

    if ( pStk1->GetState() == 0 )
    {
        if ( !m_condition->Execute(pStk1) ) return false;
        if (!pStk1->SetState(1)) return false;
    }

    if (pStk1->GetVal() != 0)
    {
        if ( !m_op1->Execute(pStk1) ) return false;
    }
    else
    {
        if ( !m_op2->Execute(pStk1) ) return false;
    }

    return pStack->Return(pStk1);                   // transmits the result
}

////////////////////////////////////////////////////////////////////////////////
void CBotLogicExpr::RestoreState(CBotStack* &pStack, bool bMain)
{
    if ( !bMain ) return;

    CBotStack* pStk1 = pStack->RestoreStack(this);  // adds an item to the stack
    if ( pStk1 == nullptr ) return;

    if ( pStk1->GetState() == 0 )
    {
        m_condition->RestoreState(pStk1, bMain);
        return;
    }

    if (pStk1->GetVal() != 0)
    {
        m_op1->RestoreState(pStk1, bMain);
    }
    else
    {
        m_op2->RestoreState(pStk1, bMain);
    }
}

std::map<std::string, CBotInstr*> CBotLogicExpr::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_op1"] = m_op1;
    links["m_condition"] = m_condition;
    links["m_op2"] = m_op2;
    return links;
}

} // namespace CBot
