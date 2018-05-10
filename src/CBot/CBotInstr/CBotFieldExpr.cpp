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

#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVarClass.h"

#include <cassert>
#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotFieldExpr::CBotFieldExpr()
{
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
    if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) != CBotTypPointer)
        assert(0);

    pVar = pVar->GetItemRef(m_nIdent);
    if (pVar == nullptr)
    {
        pile->SetError(CBotErrUndefItem, &m_token);
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


    if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) != CBotTypPointer)
        assert(0);

    CBotVarClass* pItem = pVar->GetPointer();
    if (pItem == nullptr)
    {
        pile->SetError(CBotErrNull, prevToken);
        return pj->Return(pile);
    }
    if (pItem->GetUserPtr() == OBJECTDELETED)
    {
        pile->SetError(CBotErrDeletedPtr, prevToken);
        return pj->Return(pile);
    }

    if (bStep && pile->IfStep()) return false;

    pVar = pVar->GetItemRef(m_nIdent);
    if (pVar == nullptr)
    {
        pile->SetError(CBotErrUndefItem, &m_token);
        return pj->Return(pile);
    }

    if (pVar->IsStatic())
    {
        // for a static variable, takes it in the class itself
        CBotClass* pClass = pItem->GetClass();
        pVar = pClass->GetItem(m_token.GetString());
    }

    // request the update of the element, if applicable
    pVar->Update(pile->GetUserPtr());

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

std::string CBotFieldExpr::GetDebugData()
{
    std::stringstream ss;
    ss << "VarID = " << m_nIdent;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFieldExpr::CheckProtectionError(CBotCStack* pStack, CBotVar* pPrev, CBotVar* pVar, bool bCheckReadOnly)
{
    CBotVar::ProtectionLevel varPriv = pVar->GetPrivate();

    if (bCheckReadOnly && varPriv == CBotVar::ProtectionLevel::ReadOnly)
        return true;

    if (varPriv == CBotVar::ProtectionLevel::Public) return false;

    std::string prevName = (pPrev == nullptr) ? "" : pPrev->GetName();

    // implicit 'this.'var,  this.var,  or super.var
    if (pPrev == nullptr || prevName == "this" || prevName == "super") // member of the current class
    {
        if (varPriv == CBotVar::ProtectionLevel::Private)  // var is private ?
        {
            CBotToken  token("this");
            CBotVar*   pThis = pStack->FindVar(token);
            CBotClass* pClass = pThis->GetClass();         // the current class

            CBotVar* pVarList = pClass->GetVar();

            int ident = pVar->GetUniqNum();
            // check if var is inherited from a parent class
            if (pVarList == nullptr || ident < pVarList->GetUniqNum())
                return true;
        }
    }
    else                                                                // any other context
    {
        if (pVar->IsPrivate())    // var is protected or private ?
        {
            CBotToken token("this");
            CBotVar*  pThis = pStack->FindVar(token);

            if (pThis == nullptr) return true;                   // inside a function ?
            if (pThis->GetType() != CBotTypPointer) return true;

            CBotClass* pClass = pThis->GetClass();               // the current class

            if (!pClass->IsChildOf(pPrev->GetClass()))     // var is member of some other class ?
                return true;

            if (varPriv == CBotVar::ProtectionLevel::Private &&  // private member of a parent class
                pClass != pPrev->GetClass()) return true;
        }
    }

    return false;
}

} // namespace CBot
