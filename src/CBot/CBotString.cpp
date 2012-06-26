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
// * along with this program. If not, see  http://www.gnu.org/licenses/./////////////////////////////////////////////////////
// gestion de chaine
// basé sur le CString de MFC
// mais moins complet

#include "CBot.h"

#include <string.h>

HINSTANCE CBotString::m_hInstance = (HINSTANCE)LoadLibrary("Cbot.dll"); // comment le récupérer autrement ??


CBotString::CBotString()
{
    m_ptr = NULL;               // chaine vide
    m_lg  = 0;
}

CBotString::~CBotString()
{
    if (m_ptr != NULL) free(m_ptr);
}


CBotString::CBotString(const char* p)
{
    m_lg = lstrlen( p );

    m_ptr = NULL;
    if (m_lg>0)
    {
        m_ptr = (char*)malloc(m_lg+1);
        lstrcpy(m_ptr, p);
    }
}

CBotString::CBotString(const CBotString& srcString)
{
    m_lg = srcString.m_lg;

    m_ptr = NULL;
    if (m_lg>0)
    {
        m_ptr = (char*)malloc(m_lg+1);
        lstrcpy(m_ptr, srcString.m_ptr);
    }
}




int CBotString::GivLength()
{
    if ( m_ptr == NULL ) return 0;
    return lstrlen( m_ptr );
}



CBotString CBotString::Left(int nCount) const
{
    char    chaine[2000];

    int     i;
    for (i = 0; i < m_lg && i < nCount && i < 1999; i++)
    {
        chaine[i] = m_ptr[i];
    }
    chaine[i] = 0 ;

    return CBotString( chaine );
}

CBotString CBotString::Right(int nCount) const
{
    char    chaine[2000];

    int     i = m_lg - nCount;
    if ( i < 0 ) i = 0;

    int j;
    for ( j = 0 ; i < m_lg && i < 1999; i++)
    {
        chaine[j++] = m_ptr[i];
    }
    chaine[j] = 0 ;

    return CBotString( chaine );
}

CBotString CBotString::Mid(int nFirst, int nCount) const
{
    char    chaine[2000];

    int     i;

    for ( i = nFirst; i < m_lg && i < 1999 && i <= nFirst + nCount; i++)
    {
        chaine[i] = m_ptr[i];
    }
    chaine[i] = 0 ;

    return CBotString( chaine );
}

CBotString CBotString::Mid(int nFirst) const
{
    char    chaine[2000];

    int     i;

    for ( i = nFirst; i < m_lg && i < 1999 ; i++)
    {
        chaine[i] = m_ptr[i];
    }
    chaine[i] = 0 ;

    return CBotString( chaine );
}


int CBotString::Find(const char c)
{
    int     i;
    for (i = 0; i < m_lg; i++)
    {
        if (m_ptr[i] == c) return i;
    }
    return -1;
}

int CBotString::Find(LPCTSTR lpsz)
{
    int     i, j;
    int     l = lstrlen(lpsz);

    for (i = 0; i <= m_lg-l; i++)
    {
        for (j = 0; j < l; j++)
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
    int     i;
    for (i = m_lg-1; i >= 0; i--)
    {
        if (m_ptr[i] == c) return i;
    }
    return -1;
}

int CBotString::ReverseFind(LPCTSTR lpsz)
{
    int     i, j;
    int     l = lstrlen(lpsz);

    for (i = m_lg-l; i >= 0; i--)
    {
        for (j = 0; j < l; j++)
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
    CBotString  res;
    if (start >= m_lg) return res;

    if ( lg < 0 ) lg = m_lg - start;

    char* p = (char*)malloc(m_lg+1);
    lstrcpy(p, m_ptr+start);
    p[lg] = 0;

    res = p;
    free(p);
    return res;
}

void CBotString::MakeUpper()
{
    int     i;

    for ( i = 0; i < m_lg && i < 1999 ; i++)
    {
        char    c = m_ptr[i];
        if ( c >= 'a' && c <= 'z' ) m_ptr[i] = c - 'a' + 'A';
    }
}

void CBotString::MakeLower()
{
    int     i;

    for ( i = 0; i < m_lg && i < 1999 ; i++)
    {
        char    c = m_ptr[i];
        if ( c >= 'A' && c <= 'Z' ) m_ptr[i] = c - 'A' + 'a';
    }
}



#define MAXSTRING   256

BOOL CBotString::LoadString(UINT id)
{
    char    buffer[MAXSTRING];

    m_lg = ::LoadString( m_hInstance, id, buffer, MAXSTRING );

    if (m_ptr != NULL) free(m_ptr);

    m_ptr = NULL;
    if (m_lg > 0)
    {
        m_ptr = (char*)malloc(m_lg+1);
        lstrcpy(m_ptr, buffer);
        return TRUE;
    }
    return FALSE;
}


const CBotString& CBotString::operator=(const CBotString& stringSrc)
{
    if (m_ptr != NULL) free(m_ptr);

    m_lg = stringSrc.m_lg;
    m_ptr = NULL;

    if (m_lg > 0)
    {
        m_ptr = (char*)malloc(m_lg+1);
        lstrcpy(m_ptr, stringSrc.m_ptr);
    }

    return *this;
}

CBotString operator+(const CBotString& string, LPCTSTR lpsz)
{
    CBotString s ( string );
    s += lpsz;
    return s;
}

const CBotString& CBotString::operator+(const CBotString& stringSrc)
{
    char*   p = (char*)malloc(m_lg+stringSrc.m_lg+1);

    lstrcpy(p, m_ptr);
    char*   pp = p + m_lg;
    lstrcpy(pp, stringSrc.m_ptr);

    if (m_ptr != NULL) free(m_ptr);
    m_ptr = p;
    m_lg += stringSrc.m_lg;

    return *this;
}

const CBotString& CBotString::operator=(const char ch)
{
    if (m_ptr != NULL) free(m_ptr);

    m_lg = 1;

    m_ptr = (char*)malloc(2);
    m_ptr[0] = ch;
    m_ptr[1] = 0;

    return *this;
}

const CBotString& CBotString::operator=(const char* pString)
{
    if (m_ptr != NULL) free(m_ptr);
    m_ptr = NULL;

    if ( pString != NULL )
    {
        m_lg = lstrlen(pString);

        if (m_lg != 0)
        {
            m_ptr = (char*)malloc(m_lg+1);
            lstrcpy(m_ptr, pString);
        }
    }

    return *this;
}


const CBotString& CBotString::operator+=(const char ch)
{
    char*   p = (char*)malloc(m_lg+2);

    if (m_ptr!=NULL) lstrcpy(p, m_ptr);
    p[m_lg++] = ch;
    p[m_lg]   = 0;

    if (m_ptr != NULL) free(m_ptr);

    m_ptr = p;

    return *this;
}

const CBotString& CBotString::operator+=(const CBotString& str)
{
    char*   p = (char*)malloc(m_lg+str.m_lg+1);

    lstrcpy(p, m_ptr);
    char*   pp = p + m_lg;
    lstrcpy(pp, str.m_ptr);

    m_lg = m_lg + str.m_lg;

    if (m_ptr != NULL) free(m_ptr);

    m_ptr = p;

    return *this;
}

BOOL CBotString::operator==(const CBotString& str)
{
    return Compare(str) == 0;
}

BOOL CBotString::operator==(const char* p)
{
    return Compare(p) == 0;
}

BOOL CBotString::operator!=(const CBotString& str)
{
    return Compare(str) != 0;
}

BOOL CBotString::operator!=(const char* p)
{
    return Compare(p) != 0;
}

BOOL CBotString::operator>(const CBotString& str)
{
    return Compare(str) > 0;
}

BOOL CBotString::operator>(const char* p)
{
    return Compare(p) > 0;
}

BOOL CBotString::operator>=(const CBotString& str)
{
    return Compare(str) >= 0;
}

BOOL CBotString::operator>=(const char* p)
{
    return Compare(p) >= 0;
}

BOOL CBotString::operator<(const CBotString& str)
{
    return Compare(str) < 0;
}

BOOL CBotString::operator<(const char* p)
{
    return Compare(p) < 0;
}

BOOL CBotString::operator<=(const CBotString& str)
{
    return Compare(str) <= 0;
}

BOOL CBotString::operator<=(const char* p)
{
    return Compare(p) <= 0;
}

BOOL CBotString::IsEmpty() const
{
    return (m_lg == 0);
}

void CBotString::Empty()
{
    if (m_ptr != NULL) free(m_ptr);
    m_ptr = NULL;
    m_lg = 0;
}

static char nilstring[] = {0};

CBotString::operator LPCTSTR() const
{
    if (this == NULL || m_ptr == NULL) return nilstring;
    return m_ptr;
}


int CBotString::Compare(LPCTSTR lpsz) const
{
    char*   p = m_ptr;
    if (lpsz  == NULL) lpsz = nilstring;
    if (m_ptr == NULL) p = nilstring;
    return strcmp(p, lpsz); // wcscmp
}



///////////////////////////////////////////////////////////////////////////////////////////
// tableaux de chaines

CBotStringArray::CBotStringArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = 0;
}

CBotStringArray::~CBotStringArray()
{
    SetSize(0);                 // détruit les données !
}


int CBotStringArray::GivSize()
{
    return  m_nSize;
}

void CBotStringArray::Add(const CBotString& str)
{
    SetSize(m_nSize+1);

    m_pData[m_nSize-1] = str;
}


///////////////////////////////////////////////////////////////////////
// routines utilitaires

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

static void CopyElements(CBotString* pDest, CBotString* pSrc, int nCount)
{
    while (nCount--)
    {
        *pDest = *pSrc;
        ++pDest;
        ++pSrc;
    }
}



// sélect la taille du tableau

void CBotStringArray::SetSize(int nNewSize)
{
    if (nNewSize == 0)
    {
        // shrink to nothing

        DestructElements(m_pData, m_nSize);
        delete[] (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
        // create one with exact size
        m_pData = (CBotString*) new BYTE[nNewSize * sizeof(CBotString)];

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
            nGrowBy = min(1024, max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;  // granularity
        else
            nNewMax = nNewSize;  // no slush

        CBotString* pNewData = (CBotString*) new BYTE[nNewMax * sizeof(CBotString)];

        // copy new data from old
        memcpy(pNewData, m_pData, m_nSize * sizeof(CBotString));

        // construct remaining elements
        ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


        // Ret rid of old stuff (note: no destructors called)
        delete[] (BYTE*)m_pData;
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



