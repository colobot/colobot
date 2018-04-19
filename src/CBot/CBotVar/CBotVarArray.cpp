/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotVar/CBotVarArray.h"
#include "CBot/CBotVar/CBotVarClass.h"
#include "CBot/CBotToken.h"
#include "CBot/CBotFileUtils.h"

#include "CBot/CBotEnums.h"

#include <cassert>


namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotVarArray::CBotVarArray(const CBotToken& name, CBotTypResult& type)
{
    if ( !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) assert(0);

    m_token        = new CBotToken(name);
    m_next        = nullptr;
    m_pMyThis    = nullptr;
    m_pUserPtr    = nullptr;

    m_type        = type;
    m_type.SetType(CBotTypArrayPointer);
    m_binit        = CBotVar::InitType::UNDEF;

    m_pInstance    = nullptr;                        // the list of the array elements
}

////////////////////////////////////////////////////////////////////////////////
CBotVarArray::~CBotVarArray()
{
    if ( m_pInstance != nullptr ) m_pInstance->DecrementUse();    // the lowest reference
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarArray::Copy(CBotVar* pSrc, bool bName)
{
    if ( pSrc->GetType() != CBotTypArrayPointer )
        assert(0);

    CBotVarArray*    p = static_cast<CBotVarArray*>(pSrc);

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_pInstance = p->GetPointer();

    if ( m_pInstance != nullptr )
         m_pInstance->IncrementUse();            // a reference increase

    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pMyThis    = nullptr;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarArray::SetPointer(CBotVar* pVarClass)
{
    m_binit = CBotVar::InitType::DEF;         // init, even on a null pointer

    if ( m_pInstance == pVarClass) return;    // Special, not decrement and reincrement
                                            // because the decrement can destroy the object

    if ( pVarClass != nullptr )
    {
        if ( pVarClass->GetType() == CBotTypArrayPointer )
             pVarClass = pVarClass->GetPointer();    // the real pointer to the object

        if ( !pVarClass->m_type.Eq(CBotTypClass) &&
             !pVarClass->m_type.Eq(CBotTypArrayBody))
            assert(0);

        (static_cast<CBotVarClass*>(pVarClass))->IncrementUse();            // incement the reference
    }

    if ( m_pInstance != nullptr ) m_pInstance->DecrementUse();
    m_pInstance = static_cast<CBotVarClass*>(pVarClass);
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarArray::GetPointer()
{
    if ( m_pInstance == nullptr ) return nullptr;
    return m_pInstance->GetPointer();
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarArray::GetItem(int n, bool bExtend)
{
    if ( m_pInstance == nullptr )
    {
        if ( !bExtend ) return nullptr;
        // creates an instance of the table

        CBotVarClass* instance = new CBotVarClass(CBotToken(), m_type);
        SetPointer( instance );
    }
    return m_pInstance->GetItem(n, bExtend);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarArray::GetItemList()
{
    if ( m_pInstance == nullptr) return nullptr;
    return m_pInstance->GetItemList();
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarArray::GetValString()
{
    if ( m_pInstance == nullptr ) return ( std::string( "Null pointer" ) ) ;
    return m_pInstance->GetValString();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarArray::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return false;
    return SaveVars(pf, m_pInstance);                        // saves the instance that manages the table
}

} // namespace CBot
