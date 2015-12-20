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
#include "CBot/CBotVar/CBotVarBoolean.h"

#include "CBot/CBotEnums.h"
#include "CBot/CBotUtils.h"
#include "CBot/CBotKeywordStrings.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotVarBoolean::CBotVarBoolean( const CBotToken* name )
{
    m_token    = new CBotToken(name);
    m_next     = nullptr;
    m_pMyThis  = nullptr;
    m_pUserPtr = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr  = nullptr;
    m_type     = CBotTypBoolean;
    m_binit    = InitType::UNDEF;
    m_bStatic  = false;
    m_mPrivate = 0;
    m_val      = 0;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarBoolean::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarBoolean*    p = static_cast<CBotVarBoolean*>(pSrc);

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
void CBotVarBoolean::SetValInt(int val, const std::string& s)
{
    m_val = static_cast<bool>(val);
    m_binit    = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarBoolean::SetValFloat(float val)
{
    m_val = static_cast<bool>(val);
    m_binit    = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVarBoolean::GetValInt()
{
    return    m_val;
}

////////////////////////////////////////////////////////////////////////////////
float CBotVarBoolean::GetValFloat()
{
    return static_cast<float>(m_val);
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarBoolean::GetValString()
{
    std::string    ret;

    std::string res;

    if ( m_binit == CBotVar::InitType::UNDEF )
    {
        res = LoadString(TX_UNDEF);
        return res;
    }
    if ( m_binit == CBotVar::InitType::IS_NAN )
    {
        res = LoadString(TX_NAN);
        return res;
    }

    ret = LoadString( m_val > 0 ? ID_TRUE : ID_FALSE );
    return    ret;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarBoolean::And(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() && right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarBoolean::Or(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() || right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarBoolean::XOr(CBotVar* left, CBotVar* right)
{
    m_val = left->GetValInt() ^ right->GetValInt();
    m_binit = CBotVar::InitType::DEF;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarBoolean::Not()
{
    m_val = m_val ? false : true ;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarBoolean::Eq(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() == right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarBoolean::Ne(CBotVar* left, CBotVar* right)
{
    return left->GetValInt() != right->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarBoolean::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);                            // the value of the variable
}
