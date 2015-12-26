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

#include "CBot/CBotTypResult.h"

#include "CBot/CBotEnums.h"

#include "CBot/CBotClass.h"


namespace CBot
{

CBotTypResult::CBotTypResult(int type)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
    m_limite    = -1;
}

CBotTypResult::CBotTypResult(int type, const std::string& name)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
    m_limite    = -1;

    if ( type == CBotTypPointer ||
         type == CBotTypClass   ||
         type == CBotTypIntrinsic )
    {
        m_pClass = CBotClass::Find(name);
        if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
    }
}

CBotTypResult::CBotTypResult(int type, CBotClass* pClass)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = pClass;
    m_limite    = -1;

    if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
}

CBotTypResult::CBotTypResult(int type, CBotTypResult elem)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
    m_limite    = -1;

    if ( type == CBotTypArrayPointer ||
         type == CBotTypArrayBody )
        m_pNext = new CBotTypResult( elem );
}

CBotTypResult::CBotTypResult(const CBotTypResult& typ)
{
    m_type        = typ.m_type;
    m_pClass    = typ.m_pClass;
    m_pNext        = nullptr;
    m_limite    = typ.m_limite;

    if ( typ.m_pNext )
        m_pNext = new CBotTypResult( *typ.m_pNext );
}

CBotTypResult::CBotTypResult()
{
    m_type        = 0;
    m_limite    = -1;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
}

CBotTypResult::~CBotTypResult()
{
    delete    m_pNext;
}

int CBotTypResult::GetType(int mode) const
{
    if ( mode == 3 && m_type == CBotTypNullPointer ) return CBotTypPointer;
    return    m_type;
}

void CBotTypResult::SetType(int n)
{
    m_type = n;
}

CBotClass* CBotTypResult::GetClass() const
{
    return m_pClass;
}

CBotTypResult& CBotTypResult::GetTypElem() const
{
    return *m_pNext;
}

int CBotTypResult::GetLimite() const
{
    return m_limite;
}

void CBotTypResult::SetLimite(int n)
{
    m_limite = n;
}

void CBotTypResult::SetArray( int* max )
{
    m_limite = *max;
    if (m_limite < 1) m_limite = -1;

    if ( m_pNext != nullptr )                    // last dimension?
    {
        m_pNext->SetArray( max+1 );
    }
}

bool CBotTypResult::Compare(const CBotTypResult& typ) const
{
    if ( m_type != typ.m_type ) return false;

    if ( m_type == CBotTypArrayPointer ) return m_pNext->Compare(*typ.m_pNext);

    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )
    {
        return m_pClass == typ.m_pClass;
    }

    return true;
}

bool CBotTypResult::Eq(int type) const
{
    return m_type == type;
}

CBotTypResult& CBotTypResult::operator=(const CBotTypResult& src)
{
    m_type = src.m_type;
    m_limite = src.m_limite;
    m_pClass = src.m_pClass;
    m_pNext = nullptr;
    if ( src.m_pNext != nullptr )
    {
        m_pNext = new CBotTypResult(*src.m_pNext);
    }
    return *this;
}

} // namespace CBot
