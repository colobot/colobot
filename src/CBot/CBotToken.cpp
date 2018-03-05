/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

namespace CBot
{

//! \brief Keeps the string corresponding to keyword ID
//! Map is filled with id-string pars that are needed for CBot language parsing
static const std::map<TokenId, const std::string> KEYWORDS = {
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

namespace
{
static const std::string emptyString = "";
}
const std::string& LoadString(TokenId id)
{
    if (KEYWORDS.find(id) != KEYWORDS.end())
    {
        return KEYWORDS.at(id);
    }
    else
    {
        return emptyString;
    }
}

////////////////////////////////////////////////////////////////////////////////
std::map<std::string, long> CBotToken::m_defineNum;
////////////////////////////////////////////////////////////////////////////////
CBotToken::CBotToken()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotToken::CBotToken(const std::string& text, const std::string& sep, std::size_t start, std::size_t end)
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
int CBotToken::GetType()const
{
    if (m_type == TokenTypKeyWord) return m_keywordId;
    return m_type;
}

////////////////////////////////////////////////////////////////////////////////
long CBotToken::GetKeywordId()const
{
    return m_keywordId;
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotToken::GetString()const
{
    return m_text;
}

////////////////////////////////////////////////////////////////////////////////
void CBotToken::SetString(const std::string& name)
{
    m_text = name;
}

////////////////////////////////////////////////////////////////////////////////
std::size_t CBotToken::GetStart()const
{
    return m_start;
}

////////////////////////////////////////////////////////////////////////////////
std::size_t CBotToken::GetEnd()const
{
    return m_end;
}

////////////////////////////////////////////////////////////////////////////////
void CBotToken::SetPos(const std::size_t start, const std::size_t end)
{
    m_start = start;
    m_end   = end;
}

////////////////////////////////////////////////////////////////////////////////

static char    sep1[] = " \r\n\t,:()[]{}-+*/=;><!~^|&%.\"\'?";
static char    sep2[] = " \r\n\t";                           // only separators
static char    sep3[] = ",:()[]{}-+*/=;<>!~^|&%.?";          // operational separators
static char    num[]  = "0123456789";                        // point (single) is tested separately
static char    hexnum[]   = "0123456789ABCDEFabcdef";
static char    nch[]  = "\r\n\t";                            // forbidden in chains

////////////////////////////////////////////////////////////////////////////////
CBotToken*  CBotToken::NextToken(const char*& program, const bool first)
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

        // special case for numbers
        if ( CharInList(token[0], num ))
        {
            bool    bdot = false;   // found a point?
            bool    bexp = false;   // found an exponent?

            char*   liste = num;
            if (token[0] == '0' && c == 'x')          // hexadecimal value?
            {
                token += c;
                c   = *(program++);                 // next character
                liste = hexnum;
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
    std::size_t     pos = 0;

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
    for (const auto& it : KEYWORDS)
    {
        if (it.second == w) return it.first;
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
bool IsOfType(CBotToken* &p, const int type1, const int type2)
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
bool IsOfTypeList(CBotToken* &p, const int type1, ...)
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
