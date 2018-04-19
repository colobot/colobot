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

#include "CBot/CBotVar/CBotVarPointer.h"
#include "CBot/CBotToken.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotEnums.h"
#include "CBot/CBotUtils.h"
#include "CBot/CBotFileUtils.h"

#include <cassert>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotVarPointer::CBotVarPointer(const CBotToken& name, CBotTypResult& type)
{
    if ( !type.Eq(CBotTypPointer) &&
         !type.Eq(CBotTypNullPointer) &&
         !type.Eq(CBotTypClass)   &&                    // for convenience accepts Class and Intrinsic
         !type.Eq(CBotTypIntrinsic) ) assert(0);

    m_token        = new CBotToken(name);
    m_next        = nullptr;
    m_pMyThis    = nullptr;
    m_pUserPtr    = nullptr;

    m_type        = type;
    if ( !type.Eq(CBotTypNullPointer) )
        m_type.SetType(CBotTypPointer);                    // anyway, this is a pointer
    m_binit        = CBotVar::InitType::UNDEF;
    m_pClass    = nullptr;
    m_pVarClass = nullptr;                                    // will be defined by a SetPointer()

    SetClass(type.GetClass());
}

////////////////////////////////////////////////////////////////////////////////
CBotVarPointer::~CBotVarPointer()
{
    if (m_pVarClass != nullptr) m_pVarClass->DecrementUse();    // decrement reference
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::Update(void* pUser)
{
    if (m_pVarClass != nullptr) m_pVarClass->Update(pUser);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarPointer::GetItem(const std::string& name)
{
    if ( m_pVarClass == nullptr)                // no existing instance?
        return m_pClass->GetItem(name);        // makes the pointer in the class itself

    return m_pVarClass->GetItem(name);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarPointer::GetItemRef(int nIdent)
{
    if ( m_pVarClass == nullptr)                // no existing instance?
        return m_pClass->GetItemRef(nIdent);// makes the pointer to the class itself

    return m_pVarClass->GetItemRef(nIdent);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarPointer::GetItemList()
{
    if ( m_pVarClass == nullptr) return nullptr;
    return m_pVarClass->GetItemList();
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarPointer::GetValString()
{
    std::string    s = "Pointer to ";
    if ( m_pVarClass == nullptr ) s = "Null pointer" ;
    else  s += m_pVarClass->GetValString();
    return s;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::ConstructorSet()
{
    if ( m_pVarClass != nullptr) m_pVarClass->ConstructorSet();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::SetPointer(CBotVar* pVarClass)
{
    m_binit = CBotVar::InitType::DEF;                            // init, even on a null pointer

    if ( m_pVarClass == pVarClass) return;    // special, not decrement and reincrement
                                            // because the decrement can destroy the object

    if ( pVarClass != nullptr )
    {
        if ( pVarClass->GetType() == CBotTypPointer )
             pVarClass = pVarClass->GetPointer();    // the real pointer to the object

//        if ( pVarClass->GetType() != CBotTypClass )
        if ( !pVarClass->m_type.Eq(CBotTypClass) )
            assert(0);

        (static_cast<CBotVarClass*>(pVarClass))->IncrementUse();            // increment the reference
        m_pClass = (static_cast<CBotVarClass*>(pVarClass))->m_pClass;
        m_pUserPtr = pVarClass->m_pUserPtr;                    // not really necessary
        m_type = CBotTypResult(CBotTypPointer, m_pClass);    // what kind of a pointer
    }

    if ( m_pVarClass != nullptr ) m_pVarClass->DecrementUse();
    m_pVarClass = static_cast<CBotVarClass*>(pVarClass);

}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarPointer::GetPointer()
{
    if ( m_pVarClass == nullptr ) return nullptr;
    return m_pVarClass->GetPointer();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::SetIdent(long n)
{
    if ( m_pVarClass == nullptr ) return;
    m_pVarClass->SetIdent( n );
}

////////////////////////////////////////////////////////////////////////////////
long CBotVarPointer::GetIdent()
{
    if ( m_pVarClass == nullptr ) return 0;
    return m_pVarClass->m_ItemIdent;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::SetClass(CBotClass* pClass)
{
//    int        nIdent = 0;
    m_type.m_class = m_pClass = pClass;
    if ( m_pVarClass != nullptr ) m_pVarClass->SetClass(pClass); //, nIdent);
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVarPointer::GetClass()
{
    if ( m_pVarClass != nullptr ) return m_pVarClass->GetClass();

    return    m_pClass;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarPointer::Save1State(FILE* pf)
{
    if ( m_type.GetClass() != nullptr )
    {
        if (!WriteString(pf, m_type.GetClass()->GetName())) return false;    // name of the class
    }
    else
    {
        if (!WriteString(pf, "")) return false;
    }

    if (!WriteLong(pf, GetIdent())) return false;        // the unique reference

    // also saves the proceedings copies
    return SaveVars(pf, GetPointer());
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::Copy(CBotVar* pSrc, bool bName)
{
    if ( pSrc->GetType() != CBotTypPointer &&
         pSrc->GetType() != CBotTypNullPointer)
        assert(0);

    CBotVarPointer*    p = static_cast<CBotVarPointer*>(pSrc);

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
//    m_pVarClass = p->m_pVarClass;
    m_pVarClass = p->GetPointer();

    if ( m_pVarClass != nullptr )
         m_pVarClass->IncrementUse();            // incerement the reference

    m_pClass    = p->m_pClass;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = nullptr;
    m_pMyThis    = nullptr;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarPointer::Eq(CBotVar* left, CBotVar* right)
{
    CBotVarClass*    l = left->GetPointer();
    CBotVarClass*    r = right->GetPointer();

    if ( l == r ) return true;
    if ( l == nullptr && r->GetUserPtr() == OBJECTDELETED ) return true;
    if ( r == nullptr && l->GetUserPtr() == OBJECTDELETED ) return true;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarPointer::Ne(CBotVar* left, CBotVar* right)
{
    CBotVarClass*    l = left->GetPointer();
    CBotVarClass*    r = right->GetPointer();

    if ( l == r ) return false;
    if ( l == nullptr && r->GetUserPtr() == OBJECTDELETED ) return false;
    if ( r == nullptr && l->GetUserPtr() == OBJECTDELETED ) return false;
    return true;
}

} // namespace CBot
