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
#include "CBotVarClass.h"

#include "CBotClass.h"

// Local include

// Global include
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarClass::m_ExClass = nullptr;

////////////////////////////////////////////////////////////////////////////////
CBotVarClass::CBotVarClass( const CBotToken* name, const CBotTypResult& type)
{
    if ( !type.Eq(CBotTypClass)        &&
         !type.Eq(CBotTypIntrinsic)    &&                // by convenience there accepts these types
         !type.Eq(CBotTypPointer)      &&
         !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) assert(0);

    m_token        = new CBotToken(name);
    m_next        = nullptr;
    m_pMyThis    = nullptr;
    m_pUserPtr    = OBJECTCREATED;//nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_pVar        = nullptr;
    m_type        = type;
    if ( type.Eq(CBotTypArrayPointer) )    m_type.SetType( CBotTypArrayBody );
    else if ( !type.Eq(CBotTypArrayBody) ) m_type.SetType( CBotTypClass );
                                                 // officel type for this object

    m_pClass    = nullptr;
    m_pParent    = nullptr;
    m_binit        = InitType::UNDEF;
    m_bStatic    = false;
    m_mPrivate    = 0;
    m_bConstructor = false;
    m_CptUse    = 0;
    m_ItemIdent = type.Eq(CBotTypIntrinsic) ? 0 : CBotVar::NextUniqNum();

    // se place tout seul dans la liste
    // TODO stands alone in the list (stands only in a list)
    if (m_ExClass) m_ExClass->m_ExPrev = this;
    m_ExNext  = m_ExClass;
    m_ExPrev  = nullptr;
    m_ExClass = this;

    CBotClass* pClass = type.GetClass();
    CBotClass* pClass2 = pClass->GetParent();
    if ( pClass2 != nullptr )
    {
        // also creates an instance of the parent class
        m_pParent = new CBotVarClass(name, CBotTypResult(type.GetType(),pClass2) ); //, nIdent);
    }

    SetClass( pClass ); //, nIdent );

}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass::~CBotVarClass( )
{
    if ( m_CptUse != 0 )
        assert(0);

    if ( m_pParent ) delete m_pParent;
    m_pParent = nullptr;

    // frees the indirect object if necessary
//    if ( m_Indirect != nullptr )
//        m_Indirect->DecrementUse();

    // removes the class list
    if ( m_ExPrev ) m_ExPrev->m_ExNext = m_ExNext;
    else m_ExClass = m_ExNext;

    if ( m_ExNext ) m_ExNext->m_ExPrev = m_ExPrev;
    m_ExPrev = nullptr;
    m_ExNext = nullptr;

    delete    m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::ConstructorSet()
{
    m_bConstructor = true;
}
