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
#include "CBotIndexExpr.h"

#include "CBotStack.h"

#include "CBotVar/CBotVarArray.h"

// Local include

// Global include
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
CBotIndexExpr::CBotIndexExpr()
{
    m_expr    = nullptr;
    name    = "CBotIndexExpr";
}

////////////////////////////////////////////////////////////////////////////////
CBotIndexExpr::~CBotIndexExpr()
{
    delete    m_expr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotIndexExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if (pVar->GetType(1) != CBotTypArrayPointer)
        assert(0);

    pVar = (static_cast<CBotVarArray*>(pVar))->GetItem(0, false);    // at compile time makes the element [0]
    if (pVar == nullptr)
    {
        pile->SetError(TX_OUTARRAY, m_token.GetEnd());
        return false;
    }
    if (m_next3 != nullptr) return m_next3->ExecuteVar(pVar, pile);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotIndexExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotStack*    pj = pile;

    if (pVar->GetType(1) != CBotTypArrayPointer)
        assert(0);

    pile = pile->AddStack();

    if (pile->GetState() == 0)
    {
        if (!m_expr->Execute(pile)) return false;
        pile->IncState();
    }
    // handles array

    CBotVar* p = pile->GetVar();    // result on the stack

    if (p == nullptr || p->GetType() > CBotTypDouble)
    {
        pile->SetError(TX_BADINDEX, prevToken);
        return pj->Return(pile);
    }

    int n = p->GetValInt();     // position in the table

    pVar = (static_cast<CBotVarArray*>(pVar))->GetItem(n, bExtend);
    if (pVar == nullptr)
    {
        pile->SetError(TX_OUTARRAY, prevToken);
        return pj->Return(pile);
    }

    pVar->Maj(pile->GetPUser(), true);

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile, prevToken, bStep, bExtend) ) return false;

    // does not release the stack
    // to avoid recalculation of the index twice where appropriate
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotIndexExpr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    pile = pile->RestoreStack();
    if (pile == nullptr) return;

    if (bMain && pile->GetState() == 0)
    {
        m_expr->RestoreState(pile, true);
        return;
    }

    if (m_next3)
         m_next3->RestoreStateVar(pile, bMain);
}
