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

#include "CBot/CBotToken.h"

#include <cstdarg>
#include <cassert>
#include <map>

namespace CBot
{

namespace
{
    static const std::string TX_UNDEF_VALUE = "undefined";
}

//! \brief Keeps the string corresponding to keyword ID
//! Map is filled with id-string pars that are needed for CBot language parsing
static const std::map<std::string, TokenId> KEYWORDS{
    {"if",           ID_IF},
    {"else",         ID_ELSE},
    {"while",        ID_WHILE},
    {"do",           ID_DO},
    {"for",          ID_FOR},
    {"break",        ID_BREAK},
    {"continue",     ID_CONTINUE},
    {"switch",       ID_SWITCH},
    {"case",         ID_CASE},
    {"default",      ID_DEFAULT},
    {"try",          ID_TRY},
    {"throw",        ID_THROW},
    {"catch",        ID_CATCH},
    {"finally",      ID_FINALLY},
    {"and",          ID_TXT_AND},
    {"or",           ID_TXT_OR},
    {"not",          ID_TXT_NOT},
    {"return",       ID_RETURN},
    {"class",        ID_CLASS},
    {"extends",      ID_EXTENDS},
    {"synchronized", ID_SYNCHO},
    {"new",          ID_NEW},
    {"public",       ID_PUBLIC},
    {"extern",       ID_EXTERN},
    {"static",       ID_STATIC},
    {"protected",    ID_PROTECTED},
    {"private",      ID_PRIVATE},
    {"repeat",       ID_REPEAT},
    {"int",          ID_INT},
    {"float",        ID_FLOAT},
    {"boolean",      ID_BOOLEAN},
    {"string",       ID_STRING},
    {"void",         ID_VOID},
    {"bool",         ID_BOOL},
    {"byte",         ID_BYTE},
    {"short",        ID_SHORT},
    {"char",         ID_CHAR},
    {"long",         ID_LONG},
    {"double",       ID_DOUBLE},
    {"true",         ID_TRUE},
    {"false",        ID_FALSE},
    {"null",         ID_NULL},
    {"nan",          ID_NAN},
    {"(",            ID_OPENPAR},
    {")",            ID_CLOSEPAR},
    {"{",            ID_OPBLK},
    {"}",            ID_CLBLK},
    {";",            ID_SEP},
    {",",            ID_COMMA},
    {":",            ID_DOTS},
    {".",            ID_DOT},
    {"[",            ID_OPBRK},
    {"]",            ID_CLBRK},
    {"::",           ID_DBLDOTS},
    {"?",            ID_LOGIC},
    {"+",            ID_ADD},
    {"-",            ID_SUB},
    {"*",            ID_MUL},
    {"/",            ID_DIV},
    {"=",            ID_ASS},
    {"+=",           ID_ASSADD},
    {"-=",           ID_ASSSUB},
    {"*=",           ID_ASSMUL},
    {"/=",           ID_ASSDIV},
    {"|=",           ID_ASSOR},
    {"&=",           ID_ASSAND},
    {"^=",           ID_ASSXOR},
    {"<<=",          ID_ASSSL},
    {">>>=",         ID_ASSSR},
    {">>=",          ID_ASSASR},
    {"<<",           ID_SL},
    {">>>",          ID_SR},
    {">>",           ID_ASR},
    {"++",           ID_INC},
    {"--",           ID_DEC},
    {"<",            ID_LO},
    {">",            ID_HI},
    {"<=",           ID_LS},
    {">=",           ID_HS},
    {"==",           ID_EQ},
    {"!=",           ID_NE},
    {"&",            ID_AND},
    {"^",            ID_XOR},
    {"|",            ID_OR},
    {"&&",           ID_LOG_AND},
    {"||",           ID_LOG_OR},
    {"!",            ID_LOG_NOT},
    {"~",            ID_NOT},
    {"%",            ID_MODULO},
    {"**",           ID_POWER},
    {"%=",           ID_ASSMODULO},
    {TX_UNDEF_VALUE, TX_UNDEF},
    {"not a number", TX_NAN}
};

const std::string& UndefinedTokenString()
{
    return TX_UNDEF_VALUE;
}

////////////////////////////////////////////////////////////////////////////////
std::map<std::string, long> CBotToken::m_defineNum;
////////////////////////////////////////////////////////////////////////////////
CBotToken::CBotToken()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotToken::CBotToken(const std::string& text, const std::string& sep, int start, int end)
{
    m_text  = text;
    m_sep   = sep;

    m_start = start;
    m_end   = end;
}

////////////////////////////////////////////////////////////////////////////////
CBotToken::CBotToken(const CBotToken& pSrc)
{
    m_type      = pSrc.m_type;
    m_keywordId = pSrc.m_keywordId;

    m_text      = pSrc.m_text;
    m_sep       = pSrc.m_sep;

    m_start     = pSrc.m_start;
    m_end       = pSrc.m_end;
}

////////////////////////////////////////////////////////////////////////////////
CBotToken::~CBotToken()
{
}

////////////////////////////////////////////////////////////////////////////////
void CBotToken::ClearDefineNum()
{
    m_defineNum.clear();
}

////////////////////////////////////////////////////////////////////////////////
const CBotToken& CBotToken::operator=(const CBotToken& src)
{
    assert(m_prev == nullptr);
    if (m_next != nullptr)
    {
        m_next->m_prev = nullptr;
        delete m_next;
        m_next = nullptr;
    }

    m_text      = src.m_text;
    m_sep       = src.m_sep;

    m_type      = src.m_type;
    m_keywordId = src.m_keywordId;

    m_start     = src.m_start;
    m_end       = src.m_end;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
int CBotToken::GetType()
{
    if (m_type == TokenTypKeyWord) return m_keywordId;
    return m_type;
}

////////////////////////////////////////////////////////////////////////////////
long CBotToken::GetKeywordId()
{
    return m_keywordId;
}

////////////////////////////////////////////////////////////////////////////////
const std::string& CBotToken::GetString() const
{
    return m_text;
}

////////////////////////////////////////////////////////////////////////////////
void CBotToken::SetString(const std::string& name)
{
    m_text = name;
}

////////////////////////////////////////////////////////////////////////////////
int CBotToken::GetStart()
{
    return m_start;
}

////////////////////////////////////////////////////////////////////////////////
int CBotToken::GetEnd()
{
    return m_end;
}

////////////////////////////////////////////////////////////////////////////////
void CBotToken::SetPos(int start, int end)
{
    m_start = start;
    m_end   = end;
}

////////////////////////////////////////////////////////////////////////////////

// TODO: CharInList could probably be optimized to conditions like (*c >= '0' && *c <= '9') to gain some performance
static char    sep1[] = " \r\n\t,:()[]{}-+*/=;><!~^|&%.\"\'?";
static char    sep2[] = " \r\n\t";                           // only separators
static char    sep3[] = ",:()[]{}-+*/=;<>!~^|&%.?";          // operational separators
static char    num[]  = "0123456789";                        // point (single) is tested separately
static char    hexnum[]   = "0123456789ABCDEFabcdef";
static char    nch[]  = "\r\n\t";                            // forbidden in chains

////////////////////////////////////////////////////////////////////////////////
CBotToken*  CBotToken::NextToken(const char*& program, bool first)
{
    std::string token; // found token
    std::string sep;   // separators after the token
    bool stop = first;

    if (*program == 0) return nullptr;

    char c = *(program++);                 // next character

    if (!first)
    {
        token = c;                            // built the word
        c   = *(program++);                 // next character

        // special case for strings
        if (token[0] == '\"' )
        {
            while (c != 0 && c != '\"' && !CharInList(c, nch))
            {
                if ( c == '\\' )
                {
                    token += c;
                    c = *(program++);
                    if (c == 0 || CharInList(c, nch)) break;
                }
                token += c;
                c = *(program++);
            }
            if ( c == '\"' )
            {
                token += c;                           // string is complete
                c   = *(program++);                 // next character
            }
            stop = true;
        }

        // special case for characters
        if (token[0] == '\'')
        {
            if (c == '\\')       // escape sequence
            {
                token += c;
                c = *(program++);

                if (c == 'u' || c == 'U') // unicode escape
                {
                    int maxlen = (c == 'u') ? 4 : 8;
                    token += c;
                    c = *(program++);
                    for (int i = 0; i < maxlen; i++)
                    {
                        if (c == 0 || !CharInList(c, hexnum)) break;
                        token += c;
                        c = *(program++);
                    }
                }
                else if (c != 0 && !CharInList(c, nch)) // other escape char
                {
                    token += c;
                    c = *(program++);
                }
            }
            else if (c != 0 && c != '\'' && !CharInList(c, nch)) // single character
            {
                token += c;
                c = *(program++);
            }

            if (c == '\'') // close quote
            {
                token += c;
                c = *(program++);
            }
            stop = true;
        }

        // special case for numbers
        if ( CharInList(token[0], num ))
        {
            bool    bdot = false;   // found a point?
            bool    bexp = false;   // found an exponent?

            char    bin[] = "01";
            char*   liste = num;
            if (token[0] == '0' && c == 'x')          // hexadecimal value?
            {
                token += c;
                c   = *(program++);                 // next character
                liste = hexnum;
            }
            else if (token[0] == '0' && c == 'b')   // binary literal
            {
                liste = bin;
                token += c;
                c = *(program++);
            }
cw:
            while (c != 0 && CharInList(c, liste))
            {
cc:             token += c;
                c   = *(program++);                 // next character
            }
            if ( liste == num )                     // not for hexadecimal
            {
                if ( !bdot && c == '.' ) { bdot = true; goto cc; }
                if ( !bexp && ( c == 'e' || c == 'E' ) )
                {
                    bexp = true;
                    token += c;
                    c   = *(program++);                 // next character
                    if ( c == '-' ||
                         c == '+' ) goto cc;
                    goto cw;
                }

            }
            stop = true;
        }

        if (CharInList(token[0], sep3))               // an operational separator?
        {
            std::string  motc = token;
            while (motc += c, c != 0 && GetKeyWord(motc) > 0)    // operand seeks the longest possible
            {
                token += c;                           // build the word
                c = *(program++);                   // next character
            }

            stop = true;
        }
    }



    while (true)
    {
        if (stop || c == 0 || CharInList(c, sep1))
        {
            if (!first && token.empty()) return nullptr;   // end of the analysis
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

            CBotToken* t = new CBotToken(token, sep);

            if (CharInList(token[0], num )) t->m_type = TokenTypNum;
            if (token[0] == '\"') t->m_type = TokenTypString;
            if (token[0] == '\'') t->m_type = TokenTypChar;
            if (first) t->m_type = TokenTypNone;

            t->m_keywordId = GetKeyWord(token);
            if (t->m_keywordId > 0) t->m_type = TokenTypKeyWord;
            else GetDefineNum(token, t) ;         // treats DefineNum

            return t;
        }

        token += c;                       // built the word
        c = *(program++);               // next character
    }
}

////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CBotToken> CBotToken::CompileTokens(const std::string& program)
{
    CBotToken       *nxt, *prv, *tokenbase;
    const char*     p = program.c_str();
    int             pos = 0;

    prv = tokenbase = NextToken(p, true);

    if (tokenbase == nullptr) return nullptr;

    tokenbase->m_start  = pos;
    pos += tokenbase->m_text.length();
    tokenbase->m_end    = pos;
    pos += tokenbase->m_sep.length();

    const char* pp = p;
    while (nullptr != (nxt = NextToken(p, false)))
    {
        prv->m_next = nxt;              // added after
        nxt->m_prev = prv;
        prv = nxt;                      // advance

        nxt->m_start    = pos;
        pos += (p - pp);                // total size
        nxt->m_end  = pos - nxt->m_sep.length();
        pp = p;
    }

    // terminator token
    nxt = new CBotToken();
    nxt->m_type = TokenTypNone;
    nxt->m_end = nxt->m_start = pos;
    prv->m_next = nxt;
    nxt->m_prev = prv;

    return std::unique_ptr<CBotToken>(tokenbase);
}

////////////////////////////////////////////////////////////////////////////////
int CBotToken::GetKeyWord(const std::string& w)
{
    auto it = KEYWORDS.find(w);
    if (it != KEYWORDS.end())
    {
        return it->second;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotToken::GetDefineNum(const std::string& name, CBotToken* token)
{
    if (m_defineNum.count(name) == 0)
        return false;

    token->m_type = TokenTypDef;
    token->m_keywordId = m_defineNum[name];
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotToken::DefineNum(const std::string& name, long val)
{
    if (m_defineNum.count(name) > 0)
    {
        // TODO: No access to the logger from CBot library :(
        printf("CBOT WARNING: %s redefined\n", name.c_str());
        return false;
    }

    m_defineNum[name] = val;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

} // namespace CBot
