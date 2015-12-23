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
#include "CBot/CBotVar/CBotVarString.h"

#include "CBot/CBotEnums.h"
#include "CBot/CBotToken.h"
#include "CBot/CBotUtils.h"

// Local include

// Global include

////////////////////////////////////////////////////////////////////////////////
CBotVarString::CBotVarString(const CBotToken& name)
{
    m_token    = new CBotToken(name);
    m_next    = nullptr;
    m_pMyThis = nullptr;
    m_pUserPtr = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_type  = CBotTypString;
    m_binit = InitType::UNDEF;
    m_bStatic = false;
    m_mPrivate = ProtectionLevel::Public;

    m_val.clear();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarString::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarString*    p = static_cast<CBotVarString*>(pSrc);

    if (bName)    *m_token    = *p->m_token;
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
void CBotVarString::SetValString(const std::string& val)
{
    m_val = val;
    m_binit    = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarString::GetValString()
{
    if ( m_binit == CBotVar::InitType::UNDEF )
    {
        return LoadString(TX_UNDEF);
    }
    if ( m_binit == CBotVar::InitType::IS_NAN )
    {
        return LoadString(TX_NAN);
    }

    return    m_val;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarString::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValString() + right->GetValString();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Eq(CBotVar* left, CBotVar* right)
{
    return (left->GetValString() == right->GetValString());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Ne(CBotVar* left, CBotVar* right)
{
    return (left->GetValString() != right->GetValString());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Lo(CBotVar* left, CBotVar* right)
{
    return (left->GetValString() == right->GetValString());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Hi(CBotVar* left, CBotVar* right)
{
    return (left->GetValString() == right->GetValString());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Ls(CBotVar* left, CBotVar* right)
{
    return (left->GetValString() == right->GetValString());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Hs(CBotVar* left, CBotVar* right)
{
    return (left->GetValString() == right->GetValString());
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarString::Save1State(FILE* pf)
{
    return WriteString(pf, m_val);                            // the value of the variable
}
