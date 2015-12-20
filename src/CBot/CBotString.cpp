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

/////////////////////////////////////////////////////

#include "CBot/CBotString.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <boost/algorithm/string.hpp>

CBotString::CBotString()
{
    m_str = "";
}

CBotString::~CBotString()
{
}


CBotString::CBotString(const char* p)
{
    m_str = p;
}

CBotString::CBotString(const std::string &p)
{
    m_str = p;
}

CBotString::CBotString(const CBotString& srcString)
{
    m_str = srcString.m_str;
}



int CBotString::GetLength()
{
    return m_str.length();
}



CBotString CBotString::Left(int nCount) const
{
    return CBotString(m_str.substr(0, nCount));
}

CBotString CBotString::Right(int nCount) const
{
    return CBotString(m_str.substr(m_str.length()-nCount, std::string::npos));
}

CBotString CBotString::Mid(int start, int lg)
{
    return CBotString(m_str.substr(start, lg));
}

int CBotString::Find(const char c)
{
    std::size_t pos = m_str.find(c);
    return pos != std::string::npos ? pos : -1;
}

int CBotString::Find(const char * lpsz)
{
    std::size_t pos = m_str.find(lpsz);
    return pos != std::string::npos ? pos : -1;
}

int CBotString::ReverseFind(const char c)
{
    std::size_t pos = m_str.rfind(c);
    return pos != std::string::npos ? pos : -1;
}

int CBotString::ReverseFind(const char * lpsz)
{
    std::size_t pos = m_str.rfind(lpsz);
    return pos != std::string::npos ? pos : -1;
}

void CBotString::MakeUpper()
{
    boost::to_upper(m_str);
}

void CBotString::MakeLower()
{
    boost::to_lower(m_str);
}


const CBotString& CBotString::operator=(const CBotString& stringSrc)
{
    m_str = stringSrc.m_str;
    return *this;
}

CBotString CBotString::operator+(const CBotString& stringSrc)
{
    CBotString s(*this);
    s += stringSrc;
    return s;
}

const CBotString& CBotString::operator=(const char ch)
{
    m_str = ch;
    return *this;
}

const CBotString& CBotString::operator=(const char* pString)
{
    if (pString != nullptr)
        m_str = pString;
    else
        m_str.clear();
    return *this;
}


const CBotString& CBotString::operator+=(const char ch)
{
    m_str += ch;
    return *this;
}

const CBotString& CBotString::operator+=(const CBotString& str)
{
    m_str += str.m_str;
    return *this;
}

bool CBotString::operator==(const CBotString& str)
{
    return m_str == str.m_str;
}

bool CBotString::operator==(const char* p)
{
    return m_str == p;
}

bool CBotString::operator!=(const CBotString& str)
{
    return m_str != str.m_str;
}

bool CBotString::operator!=(const char* p)
{
    return m_str != p;
}

bool CBotString::operator>(const CBotString& str)
{
    return m_str > str.m_str;
}

bool CBotString::operator>(const char* p)
{
    return m_str > p;
}

bool CBotString::operator>=(const CBotString& str)
{
    return m_str >= str.m_str;
}

bool CBotString::operator>=(const char* p)
{
    return m_str >= p;
}

bool CBotString::operator<(const CBotString& str)
{
    return m_str < str.m_str;
}

bool CBotString::operator<(const char* p)
{
    return m_str < p;
}

bool CBotString::operator<=(const CBotString& str)
{
    return m_str <= str.m_str;
}

bool CBotString::operator<=(const char* p)
{
    return m_str <= p;
}

bool CBotString::IsEmpty() const
{
    return m_str.empty();
}

void CBotString::Empty()
{
    m_str.clear();
}

static char emptyString[] = "";

CBotString::operator const char * () const
{
    if (this == nullptr) return emptyString; // TODO: can this be removed?
    return m_str.c_str();
}

const char* CBotString::CStr() const
{
    if (this == nullptr) return emptyString; // TODO: can this be removed?
    return m_str.c_str();
}
