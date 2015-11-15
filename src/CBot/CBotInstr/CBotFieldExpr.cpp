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
#include "CBotFieldExpr.h"

#include "CBotStack.h"
#include "CBotClass.h"

#include "CBotVar/CBotVarClass.h"

// Local include

// Global include
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
CBotFieldExpr::CBotFieldExpr()
{
    name        = "CBotFieldExpr";
    m_nIdent    = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotFieldExpr::~CBotFieldExpr()
{
}

////////////////////////////////////////////////////////////////////////////////
void CBotFieldExpr::SetUniqNum(int num)
{
    m_nIdent = num;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if (pVar->GetType(1) != CBotTypPointer)
        assert(0);

    pVar = pVar->GetItemRef(m_nIdent);
    if (pVar == nullptr)
    {
        pile->SetError(TX_NOITEM, &m_token);
        return false;
    }

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile) ) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotStack*    pj = pile;
    pile = pile->AddStack(this);    // changes in output stack
    if (pile == EOX) return true;


    if (pVar->GetType(1) != CBotTypPointer)
        assert(0);

    CBotVarClass* pItem = pVar->GetPointer();
    if (pItem == nullptr)
    {
        pile->SetError(TX_NULLPT, prevToken);
        return pj->Return(pile);
    }
    if (pItem->GetUserPtr() == OBJECTDELETED)
    {
        pile->SetError(TX_DELETEDPT, prevToken);
        return pj->Return(pile);
    }

    if (bStep && pile->IfStep()) return false;

    pVar = pVar->GetItemRef(m_nIdent);
    if (pVar == nullptr)
    {
        pile->SetError(TX_NOITEM, &m_token);
        return pj->Return(pile);
    }

    if (pVar->IsStatic())
    {
        // for a static variable, takes it in the class itself
        CBotClass* pClass = pItem->GetClass();
        pVar = pClass->GetItem(m_token.GetString());
    }

    // request the update of the element, if applicable
    pVar->Maj(pile->GetPUser(), true);

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, bExtend) ) return false;

    // does not release the stack
    // to maintain the state SetState () corresponding to step

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotFieldExpr::RestoreStateVar(CBotStack* &pj, bool bMain)
{
    pj = pj->RestoreStack(this);
    if (pj == nullptr) return;

    if (m_next3 != nullptr)
         m_next3->RestoreStateVar(pj, bMain);
}
