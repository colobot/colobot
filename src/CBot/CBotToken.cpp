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


//////////////////////////////////////////////////////////////////
// Managing Tokens
// the text of a program is first transformed
// into a sequence of tokens for easy interpretation
// it will only treat the case as an error
// where there is an illegal character in a string


#include "CBot.h"
#include <cstdarg>

CBotStringArray CBotToken::m_ListKeyWords;
int CBotToken::m_ListIdKeyWords[200];
CBotStringArray CBotToken::m_ListKeyDefine;
long CBotToken::m_ListKeyNums[MAXDEFNUM];

//! contructors
CBotToken::CBotToken()
{
    m_next = NULL;
    m_prev = NULL;
    m_type = TokenTypVar;           // at the beginning a default variable type
    m_IdKeyWord = -1;
}

CBotToken::CBotToken(const CBotToken* pSrc)
{
    m_next = NULL;
    m_prev = NULL;

    m_Text.Empty();
    m_Sep.Empty();

    m_type      = 0;
    m_IdKeyWord = 0;

    m_start     = 0;
    m_end       = 0;

    if ( pSrc != NULL )
    {

        m_type      = pSrc->m_type;
        m_IdKeyWord = pSrc->m_IdKeyWord;

        m_Text      = pSrc->m_Text;
        m_Sep       = pSrc->m_Sep;

        m_start     = pSrc->m_start;
        m_end       = pSrc->m_end;
    }
}

CBotToken::CBotToken(const CBotString& mot, const CBotString& sep, int start, int end)
{
    m_Text  = mot;                  // word (mot) found as token
    m_Sep   = sep;                  // separator
    m_next  = NULL;
    m_prev  = NULL;
    m_start = start;
    m_end   = end;

    m_type = TokenTypVar;           // at the beginning a default variable type
    m_IdKeyWord = -1;
}

CBotToken::CBotToken(const char* mot, const char* sep)
{
    m_Text  = mot;
    if ( sep != NULL ) m_Sep = sep;
    m_next  = NULL;
    m_prev  = NULL;

    m_type = TokenTypVar;           // at the beginning a default variable type
    m_IdKeyWord = -1;
}

CBotToken::~CBotToken()
{
    delete  m_next;                 // recursive
    m_next = NULL;
}

void CBotToken::Free()
{
    m_ListKeyDefine.SetSize(0);
}

const CBotToken& CBotToken::operator=(const CBotToken& src)
{
    if (m_next != NULL) delete(m_next);
    m_next      = NULL;
    m_prev      = NULL;

    m_Text      = src.m_Text;
    m_Sep       = src.m_Sep;

    m_type      = src.m_type;
    m_IdKeyWord = src.m_IdKeyWord;

    m_start     = src.m_start;
    m_end       = src.m_end;
    return *this;
}


int CBotToken::GetType()
{
    if (this == NULL) return 0;
    if (m_type == TokenTypKeyWord) return m_IdKeyWord;
    return m_type;
}

long CBotToken::GetIdKey()
{
    return m_IdKeyWord;
}

CBotToken* CBotToken::GetNext()
{
    if (this == NULL) return NULL;
    return      m_next;
}

CBotToken* CBotToken::GetPrev()
{
    if (this == NULL) return NULL;
    return      m_prev;
}

void CBotToken::AddNext(CBotToken* p)
{
    CBotToken*  n = new CBotToken(p);
    CBotToken*  pt = this;

    while ( pt->m_next != NULL ) pt = pt->m_next;

    pt->m_next = n;
    n->m_prev = pt;
}


CBotString& CBotToken::GetString()
{
    return  m_Text;
}

CBotString& CBotToken::GetSep()
{
    return  m_Sep;
}

void CBotToken::SetString(const char* name)
{
    m_Text = name;
}


int CBotToken::GetStart()
{
    if (this == NULL) return -1;
    return m_start;
}

int CBotToken::GetEnd()
{
    if (this == NULL) return -1;
    return m_end;
}

void CBotToken::SetPos(int start, int end)
{
    m_start = start;
    m_end   = end;
}

bool CharInList(const char c, const char* list)
{
    int     i = 0;

    while (true)
    {
        if (c == list[i++]) return true;
        if (list[i] == 0) return false;
    }
}

bool Char2InList(const char c, const char cc, const char* list)
{
    int     i = 0;

    while (true)
    {
        if (c == list[i++] &&
            cc == list[i++]) return true;

        if (list[i] == 0) return false;
    }
}

static char    sep1[] = " \r\n\t,:()[]{}-+*/=;><!~^|&%.";
static char    sep2[] = " \r\n\t";                           // only separators
static char    sep3[] = ",:()[]{}-+*/=;<>!~^|&%.";           // operational separators
static char    num[]  = "0123456789";                        // point (single) is tested separately
static char    hexnum[]   = "0123456789ABCDEFabcdef";
static char    nch[]  = "\"\r\n\t";                          // forbidden in chains

//static char*  duo  = "+=-=*=/===!=<=>=++--///**/||&&";    // double operators

// looking for the next token in a sentence
// do not start with separators
// which are made in the previous token
CBotToken*  CBotToken::NextToken(char* &program, int& error, bool first)
{
    CBotString      mot;                // the word which is found
    CBotString      sep;                // separators that are after
    char            c;
    bool            stop = first;

    if (*program == 0) return NULL;

    c   = *(program++);                 // next character

    if (!first)
    {
        mot = c;                            // built the word
        c   = *(program++);                 // next character

        // special case for strings
        if ( mot[0] == '\"' )
        {
            while (c != 0 && !CharInList(c, nch))
            {
                if ( c == '\\' )
                {
                    c   = *(program++);                 // next character
                    if ( c == 'n' ) c = '\n';
                    if ( c == 'r' ) c = '\r';
                    if ( c == 't' ) c = '\t';
                }
                mot += c;
                c = *(program++);
            }
            if ( c == '\"' )
            {
                mot += c;                           // string is complete
                c   = *(program++);                 // next character
            }
            stop = true;
        }

        // special case for numbers
        if ( CharInList(mot[0], num ))
        {
            bool    bdot = false;   // found a point?
            bool    bexp = false;   // found an exponent?

            char*   liste = num;
            if (mot[0] == '0' && c == 'x')          // hexadecimal value?
            {
                mot += c;
                c   = *(program++);                 // next character
                liste = hexnum;
            }
cw:
            while (c != 0 && CharInList(c, liste))
            {
cc:             mot += c;
                c   = *(program++);                 // next character
            }
            if ( liste == num )                     // not for hexadecimal
            {
                if ( !bdot && c == '.' ) { bdot = true; goto cc; }
                if ( !bexp && ( c == 'e' || c == 'E' ) )
                {
                    bexp = true;
                    mot += c;
                    c   = *(program++);                 // next character
                    if ( c == '-' ||
                         c == '+' ) goto cc;
                    goto cw;
                }

            }
            stop = true;
        }

        if (CharInList(mot[0], sep3))               // an operational separator?
        {
            CBotString  motc = mot;
            while (motc += c, c != 0 && GetKeyWords(motc)>0)    // operand seeks the longest possible
            {
                mot += c;                           // build the word
                c = *(program++);                   // next character
            }

            stop = true;
        }
    }



    while (true)
    {
        if (stop || c == 0 || CharInList(c, sep1))
        {
            if (!first && mot.IsEmpty()) return NULL;   // end of the analysis
bis:
            while (CharInList(c, sep2))
            {
                sep += c;                           // after all the separators
                c = *(program++);
            }
            if (c == '/' && *program == '/')        // comment on the heap?
            {
                while( c != '\n' && c != 0 )
                {
                    sep += c;
                    c = *(program++);               // next character
                }
                goto bis;
            }

            if (c == '/' && *program == '*')        // comment on the heap?
            {
                while( c != 0 && (c != '*' || *program != '/'))
                {
                    sep += c;
                    c = *(program++);               // next character
                }
                if ( c != 0 )
                {
                    sep += c;
                    c = *(program++);               // next character
                    sep += c;
                    c = *(program++);               // next character
                }
                goto bis;
            }

            program--;

            CBotToken*  token = new CBotToken(mot, sep);

            if (CharInList( mot[0], num )) token->m_type = TokenTypNum;
            if (mot[0] == '\"')  token->m_type = TokenTypString;
            if (first) token->m_type = 0;

            token->m_IdKeyWord = GetKeyWords(mot);
            if (token->m_IdKeyWord > 0) token->m_type = TokenTypKeyWord;
            else GetKeyDefNum(mot, token) ;         // treats DefineNum

            return token;
        }

        mot += c;                       // built the word
        c = *(program++);               // next character
    }
}

CBotToken* CBotToken::CompileTokens(const char* program, int& error)
{
    CBotToken       *nxt, *prv, *tokenbase;
    char*           p = const_cast<char*> ( program);
    int             pos = 0;

    error = 0;
    prv = tokenbase = NextToken(p, error, true);

    if (tokenbase == NULL) return NULL;

    tokenbase->m_start  = pos;
    pos += tokenbase->m_Text.GetLength();
    tokenbase->m_end    = pos;
    pos += tokenbase->m_Sep.GetLength();

    char* pp = p;
    while (NULL != (nxt = NextToken(p, error)))
    {
        prv->m_next = nxt;              // added after
        nxt->m_prev = prv;
        prv = nxt;                      // advance

        nxt->m_start    = pos;
/*      pos += nxt->m_Text.GetLength(); // chain may be shorter (BOA deleted)
        nxt->m_end  = pos;
        pos += nxt->m_Sep.GetLength();*/
        pos += (p - pp);                // total size
        nxt->m_end  = pos - nxt->m_Sep.GetLength();
        pp = p;
    }

    // adds a token as a terminator
    // ( useful for the previous )
    nxt = new CBotToken();
    nxt->m_type = 0;
    prv->m_next = nxt;              // added after
    nxt->m_prev = prv;

    return tokenbase;
}

void CBotToken::Delete(CBotToken* pToken)
{
    delete pToken;
}


// search if a word is part of the keywords

int CBotToken::GetKeyWords(const char* w)
{
    int     i;
    int     l = m_ListKeyWords.GetSize();

    if (l == 0)
    {
        LoadKeyWords();                         // takes the list for the first time
        l = m_ListKeyWords.GetSize();
    }

    for (i = 0; i < l; i++)
    {
        if (m_ListKeyWords[i] == w) return m_ListIdKeyWords[ i ];
    }

    return -1;
}

bool CBotToken::GetKeyDefNum(const char* w, CBotToken* &token)
{
    int     i;
    int     l = m_ListKeyDefine.GetSize();

    for (i = 0; i < l; i++)
    {
        if (m_ListKeyDefine[i] == w)
        {
            token->m_IdKeyWord = m_ListKeyNums[i];
            token->m_type      = TokenTypDef;
            return true;
        }
    }

    return false;
}


/// \todo Fixme Figure out how this should work.

// recreates the list of keywords and its IDs basing on some resources
// defines of TokenKey.. are in CBotDll.h

void CBotToken::LoadKeyWords()
{
    CBotString      s;
    int             i, n = 0;

    i = TokenKeyWord; //start with keywords of the language
    while (s.LoadString(i))
    {
        m_ListKeyWords.Add(s);
        m_ListIdKeyWords[n++] = i++;
    }

    i = TokenKeyDeclare; //keywords of declarations
    while (s.LoadString(i))
    {
        m_ListKeyWords.Add(s);
        m_ListIdKeyWords[n++] = i++;
    }


    i = TokenKeyVal;  //keywords of values
    while (s.LoadString(i))
    {
        m_ListKeyWords.Add(s);
        m_ListIdKeyWords[n++] = i++;
    }

    i = TokenKeyOp; //operators
    while (s.LoadString(i))
    {
        m_ListKeyWords.Add(s);
        m_ListIdKeyWords[n++] = i++;
    }
}

bool CBotToken::DefineNum(const char* name, long val)
{
    int     i;
    int     l = m_ListKeyDefine.GetSize();

    for (i = 0; i < l; i++)
    {
        if (m_ListKeyDefine[i] == name) return false;
    }
    if ( i == MAXDEFNUM ) return false;

    m_ListKeyDefine.Add( name );
    m_ListKeyNums[i] = val;
    return true;
}

bool IsOfType(CBotToken* &p, int type1, int type2)
{
    if (p->GetType() == type1 ||
        p->GetType() == type2 )
    {
        p = p->GetNext();
        return true;
    }
    return false;
}
// Same with any number of arguments
// There must be a zero as the last argument
bool IsOfTypeList(CBotToken* &p, int type1, ...)
{
    int     i = type1;
    int     max = 20;
    int     type = p->GetType();

    va_list marker;
    va_start( marker, type1 );     /* Initialize variable arguments. */

    while (true)
    {
        if (type == i)
        {
            p = p->GetNext();
            va_end( marker );              /* Reset variable arguments.      */
            return true;
        }
        if (--max == 0 || 0 == (i = va_arg( marker, int)))
        {
           va_end( marker );              /* Reset variable arguments.      */
           return false;
        }
    }
}

