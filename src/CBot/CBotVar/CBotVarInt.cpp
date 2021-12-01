/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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
    m_defnum.clear();
}
void CBotVarInt::Inc()
{
    CBotVarNumber::Inc();
    m_defnum.clear();
}
void CBotVarInt::Dec()
{
    CBotVarNumber::Dec();
    m_defnum.clear();
}

void CBotVarInt::SR(CBotVar* left, CBotVar* right)
{
    SetValInt(static_cast<unsigned>(left->GetValInt()) >> right->GetValInt());
}

void CBotVarInt::Not()
{
    m_val = ~m_val;
    m_defnum.clear();
}

bool CBotVarInt::Save0State(std::ostream &ostr)
{
    if (!m_defnum.empty())
    {
        if(!WriteWord(ostr, 200)) return false; // special marker
        if(!WriteString(ostr, m_defnum)) return false;
    }

    return CBotVar::Save0State(ostr);
}

bool CBotVarInt::Save1State(std::ostream &ostr)
{
    return WriteInt(ostr, m_val);
}

} // namespace CBot
