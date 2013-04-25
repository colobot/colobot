// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.
/////////////////////////////////////////////////////

//strings management

#include "CBot.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

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
    {ID_FINAL,      "final"},
    {ID_STATIC,     "static"},
    {ID_PROTECTED,  "protected"},
    {ID_PRIVATE,    "private"},
    {ID_DEBUGDD,    "STARTDEBUGDD"},
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
    {ID_SUPER,      "super"},
    {TX_UNDEF,      "undefined"},
    {TX_NAN,        "not a number"}
};

CBotString::CBotString()
{
    m_ptr = NULL;
    m_lg  = 0;
}

CBotString::~CBotString()
{
    delete[] m_ptr;
    m_ptr = nullptr;
}


CBotString::CBotString(const char* p)
{
    m_lg = strlen(p);

    m_ptr = NULL;
    if (m_lg>0)
    {
        m_ptr = new char[m_lg+1];
        strcpy(m_ptr, p);
    }
}

CBotString::CBotString(const CBotString& srcString)
{
    m_lg = srcString.m_lg;

    m_ptr = NULL;
    if (m_lg>0)
    {
        m_ptr = new char[m_lg+1];
        strcpy(m_ptr, srcString.m_ptr);
    }
}




int CBotString::GetLength()
{
    if (m_ptr == NULL) return 0;
    return strlen( m_ptr ); 
}



CBotString CBotString::Left(int nCount) const
{
    char    chain[2000];

    int i;
    for (i = 0; i < m_lg && i < nCount && i < 1999; ++i)
    {
        chain[i] = m_ptr[i];
    }
    chain[i] = 0 ;

    return CBotString(chain);
}

CBotString CBotString::Right(int nCount) const
{
    char chain[2000];

    int i = m_lg - nCount;
    if ( i < 0 ) i = 0;

    int j;
    for (j = 0 ; i < m_lg && i < 1999; ++i)
    {
        chain[j++] = m_ptr[i];
    }
    chain[j] = 0 ;

    return CBotString(chain);
}

CBotString CBotString::Mid(int nFirst, int nCount) const
{
    char chain[2000];

    int i;
    for (i = nFirst; i < m_lg && i < 1999 && i <= nFirst + nCount; ++i)
    {
        chain[i] = m_ptr[i];
    }
    chain[i] = 0 ;

    return CBotString(chain);
}

CBotString CBotString::Mid(int nFirst) const
{
    char chain[2000];

    int i;
    for (i = nFirst; i < m_lg && i < 1999 ; ++i)
    {
        chain[i] = m_ptr[i];
    }
    chain[i] = 0 ;

    return CBotString(chain);
}


int CBotString::Find(const char c)
{
    for (int i = 0; i < m_lg; ++i)
    {
        if (m_ptr[i] == c) return i;
    }
    return -1;
}

int CBotString::Find(const char * lpsz)
{
    int l = strlen(lpsz);

    for (size_t i = 0; static_cast<int>(i) <= m_lg-l; ++i)
    {
        for (size_t j = 0; static_cast<int>(j) < l; ++j)
        {
            if (m_ptr[i+j] != lpsz[j]) goto bad;
        }
        return i;
bad:;
    }
    return -1;
}

int CBotString::ReverseFind(const char c)
{
    int i;
    for (i = m_lg-1; i >= 0; --i)
    {
        if (m_ptr[i] == c) return i;
    }
    return -1;
}

int CBotString::ReverseFind(const char * lpsz)
{
    int i, j;
    int l = strlen(lpsz);

    for (i = m_lg-l; i >= 0; --i)
    {
        for (j = 0; j < l; ++j)
        {
            if (m_ptr[i+j] != lpsz[j]) goto bad;
        }
        return i;
bad:;
    }
    return -1;
}

CBotString CBotString::Mid(int start, int lg)
{
    CBotString res;
    if (start >= m_lg) return res;

    if ( lg < 0 ) lg = m_lg - start;

    char* p = new char[m_lg+1];
    strcpy(p, m_ptr+start);
    p[lg] = 0;

    res = p;
    delete[] p;
    return res;
}

void CBotString::MakeUpper()
{
    for (size_t i = 0; static_cast<int>(i) < m_lg && static_cast<int>(i) < 1999 ; ++i)
    {
        char c = m_ptr[i];
        if ( c >= 'a' && c <= 'z' ) m_ptr[i] = c - 'a' + 'A';
    }
}

void CBotString::MakeLower()
{
    for (size_t i = 0; static_cast<int>(i) < m_lg && static_cast<int>(i) < 1999 ; ++i)
    {
        char    c = m_ptr[i];
        if ( c >= 'A' && c <= 'Z' ) m_ptr[i] = c - 'A' + 'a';
    }
}

bool CBotString::LoadString(unsigned int id)
{
    const char * str = nullptr;
    str = MapIdToString(static_cast<EID>(id));
    if (m_ptr != nullptr)
        delete[] m_ptr;

    m_lg = strlen(str);
    m_ptr = NULL;
    if (m_lg > 0)
    {
        m_ptr = new char[m_lg+1];
        strcpy(m_ptr, str);
        return true;
    }
    return false;
}


const CBotString& CBotString::operator=(const CBotString& stringSrc)
{
    delete[] m_ptr;
    m_ptr = nullptr;

    m_lg = stringSrc.m_lg; 

    if (m_lg > 0)
    {
        m_ptr = new char[m_lg+1];
        strcpy(m_ptr, stringSrc.m_ptr);
    }

    return *this;
}

CBotString operator+(const CBotString& string, const char * lpsz)
{
    CBotString s(string);
    s += lpsz;
    return s;
}

const CBotString& CBotString::operator+(const CBotString& stringSrc)
{
    char* p = new char[m_lg+stringSrc.m_lg+1];

    if (m_ptr!=NULL) strcpy(p, m_ptr);
    char* pp = p + m_lg;
    if (stringSrc.m_ptr!=NULL) strcpy(pp, stringSrc.m_ptr);

    delete[] m_ptr;
    m_ptr = p;
    m_lg += stringSrc.m_lg;

    return *this;
}

const CBotString& CBotString::operator=(const char ch)
{
    delete[] m_ptr;

    m_lg = 1; 

    m_ptr = new char[2];
    m_ptr[0] = ch;
    m_ptr[1] = 0;

    return *this;
}

const CBotString& CBotString::operator=(const char* pString)
{
    delete[] m_ptr;
    m_ptr = nullptr;

    if (pString != nullptr)
    {
        m_lg = strlen(pString); 

        if (m_lg != 0)
        {
            m_ptr = new char[m_lg+1];
            strcpy(m_ptr, pString);
        }
    }

    return *this;
}


const CBotString& CBotString::operator+=(const char ch)
{
    char* p = new char[m_lg+2];

    if (m_ptr != nullptr) strcpy(p, m_ptr);
    p[m_lg++] = ch;
    p[m_lg]   = 0;

    delete[] m_ptr;

    m_ptr = p;

    return *this;
}

const CBotString& CBotString::operator+=(const CBotString& str)
{
    char* p = new char[m_lg+str.m_lg+1];

    strcpy(p, m_ptr);
    char* pp = p + m_lg;
    strcpy(pp, str.m_ptr);

    m_lg = m_lg + str.m_lg;

    delete[] m_ptr;

    m_ptr = p;

    return *this;
}

bool CBotString::operator==(const CBotString& str)
{
    return Compare(str) == 0;
}

bool CBotString::operator==(const char* p)
{
    return Compare(p) == 0;
}

bool CBotString::operator!=(const CBotString& str)
{
    return Compare(str) != 0;
}

bool CBotString::operator!=(const char* p)
{
    return Compare(p) != 0;
}

bool CBotString::operator>(const CBotString& str)
{
    return Compare(str) > 0;
}

bool CBotString::operator>(const char* p)
{
    return Compare(p) > 0;
}

bool CBotString::operator>=(const CBotString& str)
{
    return Compare(str) >= 0;
}

bool CBotString::operator>=(const char* p)
{
    return Compare(p) >= 0;
}

bool CBotString::operator<(const CBotString& str)
{
    return Compare(str) < 0;
}

bool CBotString::operator<(const char* p)
{
    return Compare(p) < 0;
}

bool CBotString::operator<=(const CBotString& str)
{
    return Compare(str) <= 0;
}

bool CBotString::operator<=(const char* p)
{
    return Compare(p) <= 0;
}

bool CBotString::IsEmpty() const
{
    return (m_lg == 0);
}

void CBotString::Empty()
{
    delete[] m_ptr;
    m_ptr = nullptr;
    m_lg = 0;
}

static char emptyString[] = {0};

CBotString::operator const char * () const
{
    if (this == NULL || m_ptr == NULL) return emptyString;
    return m_ptr;
}


int CBotString::Compare(const char * lpsz) const
{
    char* p = m_ptr;
    if (lpsz  == NULL) lpsz = emptyString;
    if (m_ptr == NULL) p = emptyString;
    return strcmp(p, lpsz);    // wcscmp 
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

///////////////////////////////////////////////////////////////////////////////////////////
// arrays of strings

CBotStringArray::CBotStringArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = 0;
}

CBotStringArray::~CBotStringArray()
{
    SetSize(0);                    // destroys data !
}


int CBotStringArray::GetSize()
{
    return m_nSize;
}

void CBotStringArray::Add(const CBotString& str)
{
    SetSize(m_nSize+1);

    m_pData[m_nSize-1] = str;
}

///////////////////////////////////////////////////////////////////////
// utility routines

static inline void ConstructElement(CBotString* pNewData)
{
    memset(pNewData, 0, sizeof(CBotString));
}

static inline void DestructElement(CBotString* pOldData)
{
    pOldData->~CBotString();
}

static inline void CopyElement(CBotString* pSrc, CBotString* pDest)
{
    *pSrc = *pDest;
}

static void ConstructElements(CBotString* pNewData, int nCount)
{
    while (nCount--)
    {
        ConstructElement(pNewData);
        pNewData++;
    }
}

static void DestructElements(CBotString* pOldData, int nCount)
{
    while (nCount--)
    {
        DestructElement(pOldData);
        pOldData++;
    }
}

// set the array size

void CBotStringArray::SetSize(int nNewSize)
{
    if (nNewSize == 0)
    {
        // shrink to nothing

        DestructElements(m_pData, m_nSize);
        delete[] reinterpret_cast<unsigned char *>(m_pData);
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
        // create one with exact size
        m_pData = reinterpret_cast<CBotString*> (new unsigned char[nNewSize * sizeof(CBotString)]);

        ConstructElements(m_pData, nNewSize);

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
        // it fits
        if (nNewSize > m_nSize)
        {
            // initialize the new elements

            ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);

        }

        else if (m_nSize > nNewSize)  // destroy the old elements
            DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);

        m_nSize = nNewSize;
    }
    else
    {
        // otherwise, grow array
        int nGrowBy;
        {
            // heuristically determine growth when nGrowBy == 0
            //  (this avoids heap fragmentation in many situations)
            nGrowBy = std::min(1024, std::max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;  // granularity
        else
            nNewMax = nNewSize;  // no slush

        CBotString* pNewData = reinterpret_cast<CBotString*> (new unsigned char[nNewMax * sizeof(CBotString)]);

        // copy new data from old
        memcpy(pNewData, m_pData, m_nSize * sizeof(CBotString));

        // construct remaining elements
        ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


        // Get rid of old stuff (note: no destructors called)
        delete[] reinterpret_cast<unsigned char *>(m_pData);
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
}


CBotString& CBotStringArray::operator[](int nIndex)
{
    return ElementAt(nIndex);
}

CBotString& CBotStringArray::ElementAt(int nIndex)
{
    return m_pData[nIndex];
}

