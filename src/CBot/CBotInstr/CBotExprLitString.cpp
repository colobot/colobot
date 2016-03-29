/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotExprLitString.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprLitString::CBotExprLitString()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprLitString::~CBotExprLitString()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprLitString::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprLitString* inst = new CBotExprLitString();

    inst->SetToken(p);
    p = p->GetNext();

    CBotVar*    var = CBotVar::Create("", CBotTypString);
    pStk->SetVar(var);

    return pStack->Return(inst, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprLitString::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create("", CBotTypString);

    std::string    chaine = m_token.GetString();
    chaine = chaine.substr(1, chaine.length()-2);    // removes the quotes

    var->SetValString(chaine);                    // value of the number

    pile->SetVar(var);                            // put on the stack

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprLitString::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

std::string CBotExprLitString::GetDebugData()
{
    return m_token.GetString();
}

} // namespace CBot
