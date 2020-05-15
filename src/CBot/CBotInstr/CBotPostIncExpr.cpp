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

#include "CBot/CBotInstr/CBotPostIncExpr.h"
#include "CBot/CBotInstr/CBotExprVar.h"

#include "CBot/CBotStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotPostIncExpr::CBotPostIncExpr()
{
    m_instr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotPostIncExpr::~CBotPostIncExpr()
{
    delete m_instr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotPostIncExpr::Execute(CBotStack* &pj)
{
    CBotStack*    pile1 = pj->AddStack(this);
    CBotStack*    pile2 = pile1;

    CBotVar*    var1 = nullptr;

    // retrieves the variable fields and indexes according
    if (!(static_cast<CBotExprVar*>(m_instr))->ExecuteVar(var1, pile2, nullptr, true)) return false;

    pile1->SetState(1);
    pile1->SetCopyVar(var1);                                // places the result (before incrementation);

    CBotStack* pile3 = pile2->AddStack(this);
    if (pile3->IfStep()) return false;

    if (var1->IsNAN())
    {
        pile1->SetError(CBotErrNan, &m_token);
    }

    if (!var1->IsDefined())
    {
        pile1->SetError(CBotErrNotInit, &m_token);
    }

    if (GetTokenType() == ID_INC) var1->Inc();
    else                          var1->Dec();

    return pj->Return(pile1);                        // operation done, result on pile2
}

////////////////////////////////////////////////////////////////////////////////
void CBotPostIncExpr::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile1 = pj->RestoreStack(this);
    if (pile1 == nullptr) return;

    (static_cast<CBotExprVar*>(m_instr))->RestoreStateVar(pile1, bMain);

    if (pile1 != nullptr) pile1->RestoreStack(this);
}

std::map<std::string, CBotInstr*> CBotPostIncExpr::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_instr"] = m_instr;
    return links;
}

} // namespace CBot
