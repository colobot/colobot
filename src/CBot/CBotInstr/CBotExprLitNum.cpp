/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <limits>
#include <sstream>

namespace CBot
{

template <>
CBotExprLitNum<int>::CBotExprLitNum(int val) : m_numtype(CBotTypInt), m_value(val)
{
}

template <>
CBotExprLitNum<long>::CBotExprLitNum(long val) : m_numtype(CBotTypLong), m_value(val)
{
}

template <>
CBotExprLitNum<float>::CBotExprLitNum(float val) : m_numtype(CBotTypFloat), m_value(val)
{
}

template <>
CBotExprLitNum<double>::CBotExprLitNum(double val) : m_numtype(CBotTypDouble), m_value(val)
{
}

template <typename T>
CBotExprLitNum<T>::~CBotExprLitNum()
{
}

CBotInstr* CompileExprLitNum(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    const auto& s = p->GetString();

    CBotInstr* inst = nullptr;
    CBotType numtype = CBotTypInt;

    if (p->GetType() == TokenTypDef)
    {
        inst = new CBotExprLitNum<int>(static_cast<int>(p->GetKeywordId()));
    }
    else
    {
        if (s.find('.') != std::string::npos || ( s.find('x') == std::string::npos && ( s.find_first_of("eE") != std::string::npos ) ))
        {
            double val = GetNumFloat(s);
            if (val > static_cast<double>(std::numeric_limits<float>::max()))
            {
                numtype = CBotTypDouble;
                inst = new CBotExprLitNum<double>(val);
            }
            else
            {
                numtype = CBotTypFloat;
                inst = new CBotExprLitNum<float>(static_cast<float>(val));
            }
        }
        else
        {
            long val = GetNumInt(s);
            if (val > std::numeric_limits<int>::max())
            {
                numtype = CBotTypLong;
                inst = new CBotExprLitNum<long>(val);
            }
            else
            {
                inst = new CBotExprLitNum<int>(static_cast<int>(val));
            }
        }
    }

    inst->SetToken(p);
    if (pStk->NextToken(p))
    {
        CBotVar* var = CBotVar::Create("", numtype);
        pStk->SetVar(var);

        return pStack->Return(inst, pStk);
    }
    delete inst;
    return pStack->Return(nullptr, pStk);
}

CBotInstr* CompileSizeOf(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken* pp = p;

    if (!IsOfType(p, TokenTypVar)) return nullptr;
    if (pp->GetString() == "sizeof" && IsOfType(p, ID_OPENPAR))
    {
        CBotCStack* pStk = pStack->TokenStack();

        int value;

        if (IsOfType(p, ID_BYTE)) value = sizeof(signed char);
        else if (IsOfType(p, ID_SHORT)) value = sizeof(short);
        else if (IsOfType(p, ID_CHAR)) value = sizeof(uint32_t);
        else if (IsOfType(p, ID_INT)) value = sizeof(int);
        else if (IsOfType(p, ID_LONG)) value = sizeof(long);
        else if (IsOfType(p, ID_FLOAT)) value = sizeof(float);
        else if (IsOfType(p, ID_DOUBLE)) value = sizeof(double);
        else
        {
            p = pp;
            return pStack->Return(nullptr, pStk);
        }

        if (IsOfType(p, ID_CLOSEPAR))
        {
            auto inst = new CBotExprLitNum<int>(value);
            inst->SetToken(pp);

            CBotVar* var = CBotVar::Create("", CBotTypInt);
            pStk->SetVar(var);
            return pStack->Return(inst, pStk);
        }
        pStk->SetError(CBotErrClosePar, p->GetStart());
        return pStack->Return(nullptr, pStk);
    }
    p = pp;
    return nullptr;
}

template <typename T>
bool CBotExprLitNum<T>::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create("", m_numtype);

    if (m_token.GetType() == TokenTypDef)
    {
        var->SetValInt(m_value, m_token.GetString());
    }
    else
    {
        *var = m_value;
    }
    pile->SetVar(var);                            // place on the stack

    return pj->Return(pile);                        // it's ok
}

template <typename T>
void CBotExprLitNum<T>::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

template <typename T>
std::string CBotExprLitNum<T>::GetDebugData()
{
    std::stringstream ss;
    switch (m_numtype)
    {
        case CBotTypInt   : ss << "(int) "; break;
        case CBotTypLong  : ss << "(long) "; break;
        case CBotTypFloat : ss << "(float) "; break;
        case CBotTypDouble: ss << "(double) "; break;
        default: assert(false);
    }
    ss << m_value;
    return ss.str();
}

} // namespace CBot
