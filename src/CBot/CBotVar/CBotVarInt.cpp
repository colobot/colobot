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

// Modules inlcude
#include "CBot/CBotVar/CBotVarInt.h"

#include "CBot/CBotEnums.h"
#include "CBot/CBotToken.h"
#include "CBot/CBotUtils.h"
#include "CBot/CBotKeywordStrings.h"

// Local include

// Global include
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
CBotVarInt::CBotVarInt( const CBotToken* name )
{
    m_token    = new CBotToken(name);
    m_next    = nullptr;
    m_pMyThis = nullptr;
    m_pUserPtr = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_type  = CBotTypInt;
    m_binit = InitType::UNDEF;
    m_bStatic = false;
    m_mPrivate = ProtectionLevel::Public;

    m_val    = 0;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarInt*    p = static_cast<CBotVarInt*>(pSrc);

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_val        = p->m_val;
    m_binit        = p->m_binit;
    m_pMyThis    = nullptr;
    m_pUserPtr    = p->m_pUserPtr;

    // identificator is the same (by défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    m_defnum    = p->m_defnum;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::SetValInt(int val, const std::string& defnum)
{
    m_val = val;
    m_binit    = CBotVar::InitType::DEF;
    m_defnum = defnum;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::SetValFloat(float val)
{
    m_val = static_cast<int>(val);
    m_binit    = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVarInt::GetValInt()
{
    return    m_val;
}

////////////////////////////////////////////////////////////////////////////////
float CBotVarInt::GetValFloat()
{
    return static_cast<float>(m_val);
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarInt::GetValString()
{
    if ( !m_defnum.empty() ) return m_defnum;

    std::string res;

    if ( m_binit == CBotVar::InitType::UNDEF )
    {
        return LoadString(TX_UNDEF);
    }

    if ( m_binit == CBotVar::InitType::IS_NAN )
    {
        return LoadString(TX_NAN);
    }

    char        buffer[300];
    sprintf(buffer, "%d", m_val);
    res = buffer;

    return    res;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Mul(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() * right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Power(CBotVar* left, CBotVar* right)
{
    m_val = static_cast<int>( pow( static_cast<double>(left->GetValInt()) , static_cast<double>(right->GetValInt()) ));
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotVarInt::Div(CBotVar* left, CBotVar* right)
{
    int    r = right->GetValInt();
    if ( r != 0 )
    {
        m_val = left->GetValInt() / r;
        m_binit = CBotVar::InitType::DEF;
    }
    return ( r == 0 ? CBotErrZeroDiv : CBotNoErr );
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotVarInt::Modulo(CBotVar* left, CBotVar* right)
{
    int    r = right->GetValInt();
    if ( r != 0 )
    {
        m_val = left->GetValInt() % r;
        m_binit = CBotVar::InitType::DEF;
    }
    return ( r == 0 ? CBotErrZeroDiv : CBotNoErr );
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() + right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Sub(CBotVar* left, CBotVar* right)
{
        m_val = left->GetValInt() - right->GetValInt();
        m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::XOr(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() ^ right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::And(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() & right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Or(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() | right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::SL(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() << right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::ASR(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() >> right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::SR(CBotVar* left, CBotVar* right)
{
    int    source = left->GetValInt();
    int shift  = right->GetValInt();
    if (shift>=1) source &= 0x7fffffff;
    m_val = source >> shift;
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Neg()
{
    m_val = -m_val;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Not()
{
    m_val = ~m_val;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Inc()
{
    m_val++;
    m_defnum.empty();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarInt::Dec()
{
    m_val--;
    m_defnum.empty();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Lo(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() < right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Hi(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() > right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Ls(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() <= right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Hs(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() >= right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Eq(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() == right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Ne(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() != right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Save0State(FILE* pf)
{
    if ( !m_defnum.empty() )
    {
        if(!WriteWord(pf, 200 )) return false;            // special marker
        if(!WriteString(pf, m_defnum)) return false;    // name of the value
    }

    return CBotVar::Save0State(pf);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarInt::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);                            // the value of the variable
}
