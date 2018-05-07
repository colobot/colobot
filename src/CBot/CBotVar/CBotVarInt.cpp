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

#include "CBot/CBotVar/CBotVarInt.h"

namespace CBot
{

void CBotVarInt::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarNumber::Copy(pSrc, bName);
    CBotVarInt* p = static_cast<CBotVarInt*>(pSrc);
    m_defnum = p->m_defnum;
}

void CBotVarInt::SetValInt(int val, const std::string& defnum)
{
    CBotVarNumber::SetValInt(val, defnum);
    m_defnum = defnum;
}

std::string CBotVarInt::GetValString()
{
    if (!m_defnum.empty()) return m_defnum;
    return CBotVarValue::GetValString();
}


void CBotVarInt::Neg()
{
    CBotVarNumber::Neg();
    m_defnum.empty();
}
void CBotVarInt::Inc()
{
    CBotVarNumber::Inc();
    m_defnum.empty();
}
void CBotVarInt::Dec()
{
    CBotVarNumber::Dec();
    m_defnum.empty();
}

void CBotVarInt::XOr(CBotVar* left, CBotVar* right)
{
    SetValInt(left->GetValInt() ^ right->GetValInt());
}
void CBotVarInt::And(CBotVar* left, CBotVar* right)
{
    SetValInt(left->GetValInt() & right->GetValInt());
}
void CBotVarInt::Or(CBotVar* left, CBotVar* right)
{
    SetValInt(left->GetValInt() | right->GetValInt());
}

void CBotVarInt::SL(CBotVar* left, CBotVar* right)
{
    SetValInt(left->GetValInt() << right->GetValInt());
}
void CBotVarInt::ASR(CBotVar* left, CBotVar* right)
{
    SetValInt(left->GetValInt() >> right->GetValInt());
}
void CBotVarInt::SR(CBotVar* left, CBotVar* right)
{
    int source = left->GetValInt();
    int shift  = right->GetValInt();
    if (shift >= 1) source &= 0x7fffffff;
    SetValInt(source >> shift);
}

void CBotVarInt::Not()
{
    m_val = ~m_val;
}

bool CBotVarInt::Save0State(FILE* pf)
{
    if (!m_defnum.empty())
    {
        if(!WriteWord(pf, 200)) return false; // special marker
        if(!WriteString(pf, m_defnum)) return false;
    }

    return CBotVar::Save0State(pf);
}

bool CBotVarInt::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);
}

} // namespace CBot
