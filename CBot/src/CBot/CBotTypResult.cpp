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

#include "CBot/CBotTypResult.h"

#include "CBot/CBotEnums.h"

#include "CBot/CBotClass.h"

#include <sstream>


namespace CBot
{

CBotTypResult::CBotTypResult(int type)
    : m_type(type)
{
}

CBotTypResult::CBotTypResult(int type, const std::string& name)
    : m_type(type)
{
    if ( type == CBotTypPointer ||
         type == CBotTypClass   ||
         type == CBotTypIntrinsic )
    {
        m_class = CBotClass::Find(name);
        if (m_class && m_class->IsIntrinsic() ) m_type = CBotTypIntrinsic;
    }
}

CBotTypResult::CBotTypResult(int type, CBotClass* pClass)
    : m_type(type),
      m_class(pClass)
{
    if (m_class && m_class->IsIntrinsic() ) m_type = CBotTypIntrinsic;
}

CBotTypResult::CBotTypResult(int type, const CBotTypResult &elem)
    : m_type(type)
{
    if ( type == CBotTypArrayPointer ||
         type == CBotTypArrayBody )
        m_elementType = std::make_unique<CBotTypResult>(elem);
}

CBotTypResult::CBotTypResult(const CBotTypResult& typ)
    : m_type(typ.m_type),
      m_class(typ.m_class),
      m_limite(typ.m_limite)
{

    if ( typ.m_elementType )
        m_elementType = std::make_unique<CBotTypResult>(*typ.m_elementType);
}

int CBotTypResult::GetType(GetTypeMode mode) const
{
    if ( mode == GetTypeMode::NULL_AS_POINTER && m_type == CBotTypNullPointer ) return CBotTypPointer;
    return    m_type;
}

void CBotTypResult::SetType(int n)
{
    m_type = n;
}

CBotClass* CBotTypResult::GetClass() const
{
    return m_class;
}

const CBotTypResult& CBotTypResult::GetTypElem() const
{
    return *m_elementType;
}

int CBotTypResult::GetLimite() const
{
    return m_limite;
}

void CBotTypResult::SetLimite(int n)
{
    m_limite = n;
}

void CBotTypResult::SetArray(int max[])
{
    m_limite = *max;
    if ( m_limite < 1 ) m_limite = -1;

    if ( m_elementType )
    {
        m_elementType->SetArray(max+1); // next element
    }
}

bool CBotTypResult::Compare(const CBotTypResult& typ) const
{
    if ( m_type != typ.m_type ) return false;

    if ( m_type == CBotTypArrayPointer ) return m_elementType->Compare(*typ.m_elementType);

    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )
    {
        return m_class == typ.m_class;
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
    m_class = src.m_class;
    if ( src.m_elementType )
    {
        m_elementType = std::make_unique<CBotTypResult>(*src.m_elementType);
    }
    else
    {
        m_elementType.reset();
    }
    return *this;
}

std::string CBotTypResult::ToString() const
{
    switch (m_type)
    {
        case CBotTypVoid: return "void";
        case CBotTypByte: return "byte";
        case CBotTypShort: return "short";
        case CBotTypChar: return "char";
        case CBotTypInt: return "int";
        case CBotTypLong: return "long";
        case CBotTypFloat: return "float";
        case CBotTypDouble: return "double";
        case CBotTypBoolean: return "bool";
        case CBotTypString: return "string";
        case CBotTypArrayPointer: return m_elementType->ToString() + "[]";
        case CBotTypArrayBody: return m_elementType->ToString() + "[] (by value)";
        case CBotTypPointer: return m_class->GetName();
        case CBotTypNullPointer: return m_class->GetName() + " (null)";
        case CBotTypClass: return m_class->GetName() + " (by value)";
        case CBotTypIntrinsic: return m_class->GetName() + " (intr)";
    }
    std::stringstream ss;
    ss << "UNKNOWN" << m_type;
    return ss.str();
}

} // namespace CBot
