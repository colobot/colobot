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
#include <unordered_map>

namespace CBot
{

namespace
{
    static const std::string TX_UNDEF_VALUE = "undefined";
}

//! \brief Keeps the string corresponding to keyword ID
//! Map is filled with id-string pars that are needed for CBot language parsing
static const std::unordered_map<std::string, TokenId> KEYWORDS{
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
const std::string& CBotToken::GetString()
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

namespace
{
inline bool IsSpaceAfterToken(const char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool IsTokenSeparator(const char c)
{
    return ('\040' < c && c < '\060') || ('\071' < c && c < '\100') ||
           ('\132' < c && c < '\137') || ('\172' < c && c < '\177');
}

inline bool OmitFromStrings(const char c)
{
    return c == 0 || c == '\t' || c == '\n' || c == '\r';
}

inline int CharacterLength(const unsigned char c)
{
    return (c < 0x80) ? 1:
           ((c >> 5) == 0x6) ? 2:
           ((c >> 4) == 0xE) ? 3:
           ((c >> 3) == 0x1E) ? 4 : 0;
}
} // namespace

CBotToken* CBotToken::NextToken(const char*& program, CBotToken::Data& tokendata, bool first)
{

    if (*program == 0) return nullptr;

    char c = *(program++);                 // next character

    std::string token; // found token
    //size_t tokenLine = tokendata.currentLineIndex;
    //size_t tokenColumn = tokendata.currentColumnIndex;
    //size_t tokenLength = 0; // length in Unicode code points

    auto NextCharacter = [&program, &tokendata, &c, &token]()
    {
        for (int len = CharacterLength(c); len > 0 && c != 0; len--)
        {
            CBot::UpdateCRC32(c, tokendata.signature);
            token.push_back(c);
            c = *(program++);              // next byte
        }
        //tokenLength ++;
        tokendata.currentColumnIndex ++;
    };

    if (!first)
    {
        NextCharacter();
        if (token.empty()) return nullptr;

        // special case for strings
        if (token[0] == '\"' )
        {
            while (c != '\"' && !OmitFromStrings(c))
            {
                if ( c == '\\' )
                {
                    NextCharacter();
                    if (OmitFromStrings(c)) break;
                }
                NextCharacter();
            }
            if ( c == '\"' )
            {
                NextCharacter();
            }
        }

        // special case for characters
        else if (token[0] == '\'')
        {
            if (c == '\\')       // escape sequence
            {
                NextCharacter();

                if (c == 'u' || c == 'U') // unicode escape
                {
                    int maxlen = (c == 'u') ? 4 : 8;
                    NextCharacter();
                    for (int i = 0; i < maxlen; i++)
                    {
                        if (!CharIsHexNum(c)) break;
                        NextCharacter();
                    }
                }
                else if ( !OmitFromStrings(c) ) // other escape char
                {
                    NextCharacter();
                }
            }
            else if (c != '\'' && !OmitFromStrings(c)) // single character
            {
                NextCharacter();
            }

            if (c == '\'') // close quote
            {
                NextCharacter();
            }
        }

        // special case for numbers
        else if (CharIsNum(token[0]))
        {
            bool    bdot = false;   // found a point?
            bool    bexp = false;   // found an exponent?

            if (token[0] == '0' && c == 'x')          // hexadecimal value?
            {
                do NextCharacter();
                while (CharIsHexNum(c));
            }
            else if (token[0] == '0' && c == 'b')   // binary literal
            {
                do NextCharacter();
                while (c == '0' || c == '1');
            }
            else while (true)
            {
                while (CharIsNum(c)) NextCharacter();

                if ( !bdot && c == '.' ) { bdot = true; NextCharacter(); }
                else if ( !bexp && ( c == 'e' || c == 'E' ) )
                {
                    bexp = true;
                    NextCharacter();
                    if ( c == '-' || c == '+' ) NextCharacter();
                }
                else
                    break;
            }
        }

        else if (IsTokenSeparator(token[0]))     // an operational separator?
        {
            std::string  motc = token;
            while (motc += c, c != 0 && GetKeyWord(motc) > 0)    // operand seeks the longest possible
            {
                NextCharacter();
            }
        }

        else while (c != 0 && !IsSpaceAfterToken(c) && !IsTokenSeparator(c))
        {
            NextCharacter();
        }

        CBot::UpdateCRC32(' ', tokendata.signature); // space to mark seperate tokens
    }

    std::string sep; // whitespace and comments after the token

    auto NextSeparator = [&program, &tokendata, &c, &sep]()
    {
        tokendata.currentColumnIndex ++;
        if (c == '\n')
        {
            tokendata.currentLineIndex ++;
            tokendata.currentColumnIndex = 0;
        }
        for (int len = CharacterLength(c); len > 0 && c != 0; len--)
        {
            sep.push_back(c);
            c = *(program++);              // next byte
        }
    };

    while (true)
    {
        while (IsSpaceAfterToken(c)) NextSeparator();

        if (c == '/' && *program == '/')   // comment on the heap?
        {
            do NextSeparator();
            while (c != '\n' && c != 0);
            continue;
        }

        if (c == '/' && *program == '*')   // comment on the heap?
        {
            NextSeparator();
            do NextSeparator();
            while (c != 0 && (c != '*' || *program != '/'));
            if (c != 0)
            {
                NextSeparator();
                NextSeparator();
            }
            continue;
        }
        break; // done
    }

    program--;

    CBotToken* t = new CBotToken;

    if (first) t->m_type = TokenTypNone;
    else if (CharIsNum(token[0])) t->m_type = TokenTypNum;
    else if (token[0] == '\"') t->m_type = TokenTypString;
    else if (token[0] == '\'') t->m_type = TokenTypChar;
    else
    {
        t->m_keywordId = GetKeyWord(token);
        if (t->m_keywordId > 0) t->m_type = TokenTypKeyWord;
    }

    token.swap(t->m_text);
    sep.swap(t->m_sep);
    return t;
}

CBotTokenUPtr CBotToken::CompileTokens(const std::string& program, CBotToken::Data* tokendata)
{
    CBotToken       *nxt, *prv, *tokenbase;
    const char*     p = program.c_str();
    int             pos = 0;

    std::unique_ptr<CBotToken::Data> tempdata;
    if (tokendata == nullptr)
    {
        tempdata = std::make_unique<CBotToken::Data>();
        tokendata = tempdata.get();
    }

    tokendata->currentLineIndex = 0;
    tokendata->currentColumnIndex = 0;
    CBot::InitCRC32(tokendata->signature);

    prv = tokenbase = NextToken(p, *tokendata, true);

    if (tokenbase == nullptr) return nullptr;

    tokenbase->m_start  = pos;
    pos += tokenbase->m_text.length();
    tokenbase->m_end    = pos;
    pos += tokenbase->m_sep.length();

    const char* pp = p;
    while (nullptr != (nxt = NextToken(p, *tokendata, false)))
    {
        prv->m_next = nxt;              // added after
        nxt->m_prev = prv;
        prv = nxt;                      // advance

        nxt->m_start    = pos;
        pos += (p - pp);                // total size
        nxt->m_end  = pos - nxt->m_sep.length();
        pp = p;

        if (nxt->m_type == TokenTypVar)
        {
           GetDefineNum(nxt->m_text, nxt);
        }
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

uint32_t CBotToken::GetTokenSignature(CBotToken* first, CBotToken* last)
{
    uint32_t result;
    CBot::InitCRC32(result);
    for (auto p = first; p != nullptr; p = p->m_next)
    {
        for (const auto& c : p->m_text)
            CBot::UpdateCRC32(c, result);
        CBot::UpdateCRC32(' ', result); // space to mark seperate tokens
        if (p == last) break;
    }
    return result;
}

} // namespace CBot
