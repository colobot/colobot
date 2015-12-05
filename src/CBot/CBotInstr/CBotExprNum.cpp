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
#include "CBot/CBotInstr/CBotExprNum.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include

////////////////////////////////////////////////////////////////////////////////
CBotExprNum::CBotExprNum()
{
    name    = "CBotExprNum";
}

////////////////////////////////////////////////////////////////////////////////
CBotExprNum::~CBotExprNum()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprNum::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprNum* inst = new CBotExprNum();

    inst->SetToken(p);
    CBotString    s = p->GetString();

    inst->m_numtype = CBotTypInt;
    if (p->GetType() == TokenTypDef)
    {
        inst->m_valint = p->GetIdKey();
    }
    else
    {
        if (s.Find('.') >= 0 || ( s.Find('x') < 0 && ( s.Find('e') >= 0 || s.Find('E') >= 0 ) ))
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
        CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(nullptr), inst->m_numtype);
        pStk->SetVar(var);

        return pStack->Return(inst, pStk);
    }
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprNum::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(nullptr), m_numtype);

    CBotString    nombre ;
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
    }
    pile->SetVar(var);                            // place on the stack

    return pj->Return(pile);                        // it's ok
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprNum::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

