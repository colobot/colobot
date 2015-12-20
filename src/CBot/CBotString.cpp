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

//strings management
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <boost/algorithm/string.hpp>

//Map is filled with id-string pars that are needed for CBot language parsing
const std::map<EID,const char *> CBotString::s_keywordString =
{
    {ID_IF,         "if"},
    {ID_ELSE,       "else"},
    {ID_WHILE,      "while"},
    {ID_DO,         "do"},
    {ID_FOR,        "for"},
    {ID_BREAK,      "break"},
    {ID_CONTINUE,   "continue"},
    {ID_SWITCH,     "switch"},
    {ID_CASE,       "case"},
    {ID_DEFAULT,    "default"},
    {ID_TRY,        "try"},
    {ID_THROW,      "throw"},
    {ID_CATCH,      "catch"},
    {ID_FINALLY,    "finally"},
    {ID_TXT_AND,    "and"},
    {ID_TXT_OR,     "or"},
    {ID_TXT_NOT,    "not"},
    {ID_RETURN,     "return"},
    {ID_CLASS,      "class"},
    {ID_EXTENDS,    "extends"},
    {ID_SYNCHO,     "synchronized"},
    {ID_NEW,        "new"},
    {ID_PUBLIC,     "public"},
    {ID_EXTERN,     "extern"},
    {ID_STATIC,     "static"},
    {ID_PROTECTED,  "protected"},
    {ID_PRIVATE,    "private"},
    {ID_INT,        "int"},
    {ID_FLOAT,      "float"},
    {ID_BOOLEAN,    "boolean"},
    {ID_STRING,     "string"},
    {ID_VOID,       "void"},
    {ID_BOOL,       "bool"},
    {ID_TRUE,       "true"},
    {ID_FALSE,      "false"},
    {ID_NULL,       "null"},
    {ID_NAN,        "nan"},
    {ID_OPENPAR,    "("},
    {ID_CLOSEPAR,   ")"},
    {ID_OPBLK,      "{"},
    {ID_CLBLK,      "}"},
    {ID_SEP,        ";"},
    {ID_COMMA,      ","},
    {ID_DOTS,       ":"},
    {ID_DOT,        "."},
    {ID_OPBRK,      "["},
    {ID_CLBRK,      "]"},
    {ID_DBLDOTS,    "::"},
    {ID_LOGIC,      "?"},
    {ID_ADD,        "+"},
    {ID_SUB,        "-"},
    {ID_MUL,        "*"},
    {ID_DIV,        "/"},
    {ID_ASS,        "="},
    {ID_ASSADD,     "+="},
    {ID_ASSSUB,     "-="},
    {ID_ASSMUL,     "*="},
    {ID_ASSDIV,     "/="},
    {ID_ASSOR,      "|="},
    {ID_ASSAND,     "&="},
    {ID_ASSXOR,     "^="},
    {ID_ASSSL,      "<<="},
    {ID_ASSSR,      ">>>="},
    {ID_ASSASR,     ">>="},
    {ID_SL,         "<<"},
    {ID_SR,         ">>"},
    {ID_ASR,        ">>"},
    {ID_INC,        "++"},
    {ID_DEC,        "--"},
    {ID_LO,         "<"},
    {ID_HI,         ">"},
    {ID_LS,         "<="},
    {ID_HS,         ">="},
    {ID_EQ,         "=="},
    {ID_NE,         "!="},
    {ID_AND,        "&"},
    {ID_XOR,        "^"},
    {ID_OR,         "|"},
    {ID_LOG_AND,    "&&"},
    {ID_LOG_OR,     "||"},
    {ID_LOG_NOT,    "!"},
    {ID_NOT,        "~"},
    {ID_MODULO,     "%"},
    {ID_POWER,      "**"},
    {ID_ASSMODULO,  "%="},
    {TX_UNDEF,      "undefined"},
    {TX_NAN,        "not a number"}
};

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

bool CBotString::LoadString(unsigned int id)
{
    m_str = MapIdToString(static_cast<EID>(id));
    return !m_str.empty();
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

const char * CBotString::MapIdToString(EID id)
{
    if (s_keywordString.find(id) != s_keywordString.end())
    {
        return s_keywordString.at(id);
    }
    else
    {
        return emptyString;
    }
}
