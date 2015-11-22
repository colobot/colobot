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
#include "CBotVarFloat.h"

#include "CBotEnums.h"

#include "CBotToken.h"

#include "CBotUtils.h"

// Local include

// Global include
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
CBotVarFloat::CBotVarFloat( const CBotToken* name )
{
    m_token    = new CBotToken(name);
    m_next    = nullptr;
    m_pMyThis = nullptr;
    m_pUserPtr = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_type  = CBotTypFloat;
    m_binit = InitType::UNDEF;
    m_bStatic = false;
    m_mPrivate = 0;

    m_val    = 0;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarFloat*    p = static_cast<CBotVarFloat*>(pSrc);

    if (bName)     *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_val        = p->m_val;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = nullptr;
    m_pMyThis    = nullptr;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::SetValInt(int val, const char* s)
{
    m_val = static_cast<float>(val);
    m_binit    = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::SetValFloat(float val)
{
    m_val = val;
    m_binit    = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVarFloat::GetValInt()
{
    return    static_cast<int>(m_val);
}

////////////////////////////////////////////////////////////////////////////////
float CBotVarFloat::GetValFloat()
{
    return m_val;
}

////////////////////////////////////////////////////////////////////////////////
CBotString CBotVarFloat::GetValString()
{
    CBotString res;

    if ( m_binit == CBotVar::InitType::UNDEF )
    {
        res.LoadString(TX_UNDEF);
        return res;
    }
    if ( m_binit == CBotVar::InitType::IS_NAN )
    {
        res.LoadString(TX_NAN);
        return res;
    }

    char        buffer[300];
    sprintf(buffer, "%.2f", m_val);
    res = buffer;

    return    res;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Mul(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValFloat() * right->GetValFloat();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Power(CBotVar* left, CBotVar* right)
{
    m_val = static_cast<float>(pow( left->GetValFloat() , right->GetValFloat() ));
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVarFloat::Div(CBotVar* left, CBotVar* right)
{
    float    r = right->GetValFloat();
    if ( r != 0 )
    {
        m_val = left->GetValFloat() / r;
        m_binit = CBotVar::InitType::DEF;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

////////////////////////////////////////////////////////////////////////////////
int CBotVarFloat::Modulo(CBotVar* left, CBotVar* right)
{
    float    r = right->GetValFloat();
    if ( r != 0 )
    {
        m_val = static_cast<float>(fmod( left->GetValFloat() , r ));
        m_binit = CBotVar::InitType::DEF;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValFloat() + right->GetValFloat();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Sub(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValFloat() - right->GetValFloat();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Neg()
{
        m_val = -m_val;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Inc()
{
        m_val++;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarFloat::Dec()
{
        m_val--;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Lo(CBotVar* left, CBotVar* right)
{
    return left->GetValFloat() < right->GetValFloat();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Hi(CBotVar* left, CBotVar* right)
{
    return left->GetValFloat() > right->GetValFloat();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Ls(CBotVar* left, CBotVar* right)
{
    return left->GetValFloat() <= right->GetValFloat();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Hs(CBotVar* left, CBotVar* right)
{
    return left->GetValFloat() >= right->GetValFloat();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Eq(CBotVar* left, CBotVar* right)
{
    return left->GetValFloat() == right->GetValFloat();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Ne(CBotVar* left, CBotVar* right)
{
    return left->GetValFloat() != right->GetValFloat();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarFloat::Save1State(FILE* pf)
{
    return WriteFloat(pf, m_val); // the value of the variable
}
