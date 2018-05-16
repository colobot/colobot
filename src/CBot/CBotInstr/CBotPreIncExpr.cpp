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

#include "CBot/CBotInstr/CBotPreIncExpr.h"
#include "CBot/CBotInstr/CBotExprVar.h"

#include "CBot/CBotStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotPreIncExpr::CBotPreIncExpr()
{
    m_instr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotPreIncExpr::~CBotPreIncExpr()
{
    delete m_instr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotPreIncExpr::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*     var1;

    if (pile->GetState() == 0)
    {
        CBotStack*    pile2 = pile;
        // retrieves the variable fields and indexes according
        // pile2 is modified on return
        if (!(static_cast<CBotExprVar*>(m_instr))->ExecuteVar(var1, pile2, nullptr, true)) return false;

        if (var1->IsNAN())
        {
            pile->SetError(CBotErrNan, &m_token);
            return pj->Return(pile);    // operation performed
        }

        if (!var1->IsDefined())
        {
            pile->SetError(CBotErrNotInit, &m_token);
            return pj->Return(pile);    // operation performed
        }

        if (GetTokenType() == ID_INC) var1->Inc();
        else                          var1->Dec();  // ((CBotVarInt*)var1)->m_val

        pile->IncState();
    }

    if (!m_instr->Execute(pile)) return false;
    return pj->Return(pile);    // operation performed
}

////////////////////////////////////////////////////////////////////////////////
void CBotPreIncExpr::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);
    if (pile == nullptr) return;

    if (pile->GetState() == 0)
    {
        return;
    }

    m_instr->RestoreState(pile, bMain);
}

std::map<std::string, CBotInstr*> CBotPreIncExpr::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_instr"] = m_instr;
    return links;
}

} // namespace CBot
