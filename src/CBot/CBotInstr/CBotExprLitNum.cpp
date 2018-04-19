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

#include "CBot/CBotInstr/CBotExprLitNum.h"
#include "CBot/CBotStack.h"

#include "CBot/CBotCStack.h"
#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotUtils.h"

#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprLitNum::CBotExprLitNum()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprLitNum::~CBotExprLitNum()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprLitNum::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprLitNum* inst = new CBotExprLitNum();

    inst->SetToken(p);
    std::string    s = p->GetString();

    inst->m_numtype = CBotTypInt;
    if (p->GetType() == TokenTypDef)
    {
        inst->m_valint = p->GetKeywordId();
    }
    else
    {
        if (s.find('.') != std::string::npos || ( s.find('x') == std::string::npos && ( s.find_first_of("eE") != std::string::npos ) ))
        {
            inst->m_numtype = CBotTypFloat;
            inst->m_valfloat = GetNumFloat(s);
        }
        else
        {
            inst->m_valint = GetNumInt(s);
        }
    }

    if (pStk->NextToken(p))
    {
        CBotVar*    var = CBotVar::Create("", inst->m_numtype);
        pStk->SetVar(var);

        return pStack->Return(inst, pStk);
    }
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprLitNum::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create("", m_numtype);

    std::string    nombre ;
    if (m_token.GetType() == TokenTypDef)
    {
        nombre = m_token.GetString();
    }

    switch (m_numtype)
    {
    case CBotTypShort:
    case CBotTypInt:
        var->SetValInt(m_valint, nombre);
        break;
    case CBotTypFloat:
        var->SetValFloat(m_valfloat);
        break;
    default:
        assert(false);
    }
    pile->SetVar(var);                            // place on the stack

    return pj->Return(pile);                        // it's ok
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprLitNum::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

std::string CBotExprLitNum::GetDebugData()
{
    std::stringstream ss;
    ss << "(" << (m_numtype == CBotTypFloat ? "float" : "int") << ") " << (m_numtype == CBotTypFloat ? m_valfloat : m_valint);
    return ss.str();
}

} // namespace CBot
