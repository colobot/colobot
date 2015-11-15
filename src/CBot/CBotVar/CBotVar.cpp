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
#include "CBotVar.h"

#include "CBotStack.h"

#include "CBotVar/CBotVarArray.h"
#include "CBotVar/CBotVarPointer.h"
#include "CBotVar/CBotVarClass.h"
#include "CBotVar/CBotVarBoolean.h"
#include "CBotVar/CBotVarString.h"
#include "CBotVar/CBotVarFloat.h"
#include "CBotVar/CBotVarInt.h"

#include "CBotClass.h"

// Local include

// Global include
#include <cassert>
#include <cmath>
#include <cstdio>

////////////////////////////////////////////////////////////////////////////////
long CBotVar::m_identcpt = 0;

////////////////////////////////////////////////////////////////////////////////
CBotVar::CBotVar( )
{
    m_next    = nullptr;
    m_pMyThis = nullptr;
    m_pUserPtr = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_type  = -1;
    m_binit = InitType::UNDEF;
    m_ident = 0;
    m_bStatic = false;
    m_mPrivate = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar::~CBotVar( )
{
    delete  m_token;
    delete    m_next;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::debug()
{
//    const char*    p = static_cast<const char*>( m_token->GetString());
    CBotString  s = static_cast<const char*>( GetValString());
//    const char* v = static_cast<const char*> (s);

    if ( m_type.Eq(CBotTypClass) )
    {
        CBotVar*    pv = (static_cast<CBotVarClass*>(this))->m_pVar;
        while (pv != nullptr)
        {
            pv->debug();
            pv = pv->GetNext();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::ConstructorSet()
{
    // nop
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetUserPtr(void* pUser)
{
    m_pUserPtr = pUser;
    if (m_type.Eq(CBotTypPointer) &&
        (static_cast<CBotVarPointer*>(this))->m_pVarClass != nullptr )
        (static_cast<CBotVarPointer*>(this))->m_pVarClass->SetUserPtr(pUser);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetIdent(long n)
{
    if (m_type.Eq(CBotTypPointer) &&
        (static_cast<CBotVarPointer*>(this))->m_pVarClass != nullptr )
        (static_cast<CBotVarPointer*>(this))->m_pVarClass->SetIdent(n);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetUniqNum(long n)
{
    m_ident = n;

    if ( n == 0 ) assert(0);
}

////////////////////////////////////////////////////////////////////////////////
long CBotVar::NextUniqNum()
{
    if (++m_identcpt < 10000) m_identcpt = 10000;
    return m_identcpt;
}

////////////////////////////////////////////////////////////////////////////////
long CBotVar::GetUniqNum()
{
    return m_ident;
}

////////////////////////////////////////////////////////////////////////////////
void* CBotVar::GetUserPtr()
{
    return m_pUserPtr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Save1State(FILE* pf)
{
    // this routine "virtual" must never be called,
    // there must be a routine for each of the subclasses (CBotVarInt, CBotVarFloat, etc)
    // ( see the type in m_type )
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Maj(void* pUser, bool bContinu)
{
/*    if (!bContinu && m_pMyThis != nullptr)
        m_pMyThis->Maj(pUser, true);*/
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::Create(const CBotToken* name, int type )
{
    CBotTypResult    t(type);
    return Create(name, t);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::Create(const CBotToken* name, CBotTypResult type)
{
    switch (type.GetType())
    {
    case CBotTypShort:
    case CBotTypInt:
        return new CBotVarInt(name);
    case CBotTypFloat:
        return new CBotVarFloat(name);
    case CBotTypBoolean:
        return new CBotVarBoolean(name);
    case CBotTypString:
        return new CBotVarString(name);
    case CBotTypPointer:
    case CBotTypNullPointer:
        return new CBotVarPointer(name, type);
    case CBotTypIntrinsic:
        return new CBotVarClass(name, type);

    case CBotTypClass:
        // creates a new instance of a class
        // and returns the POINTER on this instance
        {
            CBotVarClass* instance = new CBotVarClass(name, type);
            CBotVarPointer* pointer = new CBotVarPointer(name, type);
            pointer->SetPointer( instance );
            return pointer;
        }

    case CBotTypArrayPointer:
        return new CBotVarArray(name, type);

    case CBotTypArrayBody:
        {
            CBotVarClass* instance = new CBotVarClass(name, type);
            CBotVarArray* array = new CBotVarArray(name, type);
            array->SetPointer( instance );

            CBotVar*    pv = array;
            while (type.Eq(CBotTypArrayBody))
            {
                type = type.GetTypElem();
                pv = (static_cast<CBotVarArray*>(pv))->GetItem(0, true);            // creates at least the element [0]
            }

            return array;
        }
    }

    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::Create( CBotVar* pVar )
{
    CBotVar*    p = Create(pVar->m_token->GetString(), pVar->GetTypResult(2));
    return p;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::Create( const char* n, CBotTypResult type)
{
    CBotToken    name(n);

    switch (type.GetType())
    {
    case CBotTypShort:
    case CBotTypInt:
        return new CBotVarInt(&name);
    case CBotTypFloat:
        return new CBotVarFloat(&name);
    case CBotTypBoolean:
        return new CBotVarBoolean(&name);
    case CBotTypString:
        return new CBotVarString(&name);
    case CBotTypPointer:
    case CBotTypNullPointer:
        {
            CBotVarPointer* p = new CBotVarPointer(&name, type);
//            p->SetClass(type.GetClass());
            return p;
        }
    case CBotTypIntrinsic:
        {
            CBotVarClass* p = new CBotVarClass(&name, type);
//            p->SetClass(type.GetClass());
            return p;
        }

    case CBotTypClass:
        // creates a new instance of a class
        // and returns the POINTER on this instance
        {
            CBotVarClass* instance = new CBotVarClass(&name, type);
            CBotVarPointer* pointer = new CBotVarPointer(&name, type);
            pointer->SetPointer( instance );
//            pointer->SetClass( type.GetClass() );
            return pointer;
        }

    case CBotTypArrayPointer:
        return new CBotVarArray(&name, type);

    case CBotTypArrayBody:
        {
            CBotVarClass* instance = new CBotVarClass(&name, type);
            CBotVarArray* array = new CBotVarArray(&name, type);
            array->SetPointer( instance );

            CBotVar*    pv = array;
            while (type.Eq(CBotTypArrayBody))
            {
                type = type.GetTypElem();
                pv = (static_cast<CBotVarArray*>(pv))->GetItem(0, true);            // creates at least the element [0]
            }

            return array;
        }
    }

    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::Create( const char* name, int type, CBotClass* pClass)
{
    CBotToken    token( name, "" );
    CBotVar*    pVar = Create( &token, type );

    if ( type == CBotTypPointer && pClass == nullptr )        // pointer "null" ?
        return pVar;

    if ( type == CBotTypClass || type == CBotTypPointer ||
         type == CBotTypIntrinsic )
    {
        if (pClass == nullptr)
        {
            delete pVar;
            return nullptr;
        }
        pVar->SetClass( pClass );
    }
    return pVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::Create( const char* name, CBotClass* pClass)
{
    CBotToken    token( name, "" );
    CBotVar*    pVar = Create( &token, CBotTypResult( CBotTypClass, pClass ) );
//    pVar->SetClass( pClass );
    return        pVar;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Destroy(CBotVar* var)
{
    delete var;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotVar::GetTypResult(int mode)
{
    CBotTypResult    r = m_type;

    if ( mode == 1 && m_type.Eq(CBotTypClass) )
        r.SetType(CBotTypPointer);
    if ( mode == 2 && m_type.Eq(CBotTypClass) )
        r.SetType(CBotTypIntrinsic);

    return r;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVar::GetType(int mode)
{
    if ( mode == 1 && m_type.Eq(CBotTypClass) )
        return CBotTypPointer;
    if ( mode == 2 && m_type.Eq(CBotTypClass) )
        return CBotTypIntrinsic;
    return m_type.GetType();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetType(CBotTypResult& type)
{
    m_type = type;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar::InitType CBotVar::GetInit() const
{
    if ( m_type.Eq(CBotTypClass) ) return InitType::DEF;        // always set!

    return m_binit;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetInit(CBotVar::InitType bInit)
{
    m_binit = bInit;
    if ( bInit == CBotVar::InitType::IS_POINTER ) m_binit = CBotVar::InitType::DEF;                    // cas spécial

    if ( m_type.Eq(CBotTypPointer) && bInit == CBotVar::InitType::IS_POINTER )
    {
        CBotVarClass* instance = GetPointer();
        if ( instance == nullptr )
        {
            instance = new CBotVarClass(nullptr, m_type);
//            instance->SetClass((static_cast<CBotVarPointer*>(this))->m_pClass);
            SetPointer(instance);
        }
        instance->SetInit(CBotVar::InitType::DEF);
    }

    if ( m_type.Eq(CBotTypClass) || m_type.Eq(CBotTypIntrinsic) )
    {
        CBotVar*    p = (static_cast<CBotVarClass*>(this))->m_pVar;
        while( p != nullptr )
        {
            p->SetInit( bInit );
            p->m_pMyThis = static_cast<CBotVarClass*>(this);
            p = p->GetNext();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotString CBotVar::GetName()
{
    return    m_token->GetString();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetName(const char* name)
{
    m_token->SetString(name);
}

////////////////////////////////////////////////////////////////////////////////
CBotToken* CBotVar::GetToken()
{
    return    m_token;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::GetItem(const char* name)
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::GetItemRef(int nIdent)
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::GetItemList()
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::GetItem(int row, bool bGrow)
{
    assert(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::IsElemOfClass(const char* name)
{
    CBotClass*    pc = nullptr;

    if ( m_type.Eq(CBotTypPointer) )
    {
        pc = (static_cast<CBotVarPointer*>(this))->m_pClass;
    }
    if ( m_type.Eq(CBotTypClass) )
    {
        pc = (static_cast<CBotVarClass*>(this))->m_pClass;
    }

    while ( pc != nullptr )
    {
        if ( pc->GetName() == name ) return true;
        pc = pc->GetParent();
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::GetStaticVar()
{
    // makes the pointer to the variable if it is static
    if ( m_bStatic == 0 || m_pMyThis == nullptr ) return this;

    CBotClass*    pClass = m_pMyThis->GetClass();
    return pClass->GetItem( m_token->GetString() );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVar::GetNext()
{
    return m_next;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::AddNext(CBotVar* pVar)
{
    CBotVar*    p = this;
    while (p->m_next != nullptr) p = p->m_next;

    p->m_next = pVar;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetVal(CBotVar* var)
{
    switch (var->GetType())
    {
    case CBotTypBoolean:
        SetValInt(var->GetValInt());
        break;
    case CBotTypInt:
        SetValInt(var->GetValInt(), (static_cast<CBotVarInt*>(var))->m_defnum);
        break;
    case CBotTypFloat:
        SetValFloat(var->GetValFloat());
        break;
    case CBotTypString:
        SetValString(var->GetValString());
        break;
    case CBotTypPointer:
    case CBotTypNullPointer:
    case CBotTypArrayPointer:
        SetPointer(var->GetPointer());
        break;
    case CBotTypClass:
        {
            delete (static_cast<CBotVarClass*>(this))->m_pVar;
            (static_cast<CBotVarClass*>(this))->m_pVar = nullptr;
            Copy(var, false);
        }
        break;
    default:
        assert(0);
    }

    m_binit = var->m_binit;        // copie l'état nan s'il y a
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetStatic(bool bStatic)
{
    m_bStatic = bStatic;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetPrivate(int mPrivate)
{
    m_mPrivate = mPrivate;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::IsStatic()
{
    return m_bStatic;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::IsPrivate(int mode)
{
    return m_mPrivate >= mode;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVar::GetPrivate()
{
    return m_mPrivate;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetPointer(CBotVar* pVarClass)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVar::GetPointer()
{
    assert(0);
    return nullptr;
}

// All these functions must be defined in the subclasses
// derived from class CBotVar
////////////////////////////////////////////////////////////////////////////////
int CBotVar::GetValInt()
{
    assert(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
float CBotVar::GetValFloat()
{
    assert(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetValInt(int c, const char* s)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetValFloat(float c)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Mul(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Power(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
int CBotVar::Div(CBotVar* left, CBotVar* right)
{
    assert(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int CBotVar::Modulo(CBotVar* left, CBotVar* right)
{
    assert(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Add(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Sub(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Lo(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Hi(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Ls(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Hs(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Eq(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Ne(CBotVar* left, CBotVar* right)
{
    assert(0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::And(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Or(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::XOr(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::ASR(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SR(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SL(CBotVar* left, CBotVar* right)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Neg()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Not()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Inc()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Dec()
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::Copy(CBotVar* pSrc, bool bName)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetValString(const char* p)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotString CBotVar::GetValString()
{
    assert(0);
    return CBotString();
}

////////////////////////////////////////////////////////////////////////////////
void CBotVar::SetClass(CBotClass* pClass)
{
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVar::GetClass()
{
    assert(0);
    return nullptr;
}

///////////////////////////////////////////////////////
// management of results types

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::CBotTypResult(int type)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
    m_limite    = -1;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::CBotTypResult(int type, const char* name)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
    m_limite    = -1;

    if ( type == CBotTypPointer ||
         type == CBotTypClass   ||
         type == CBotTypIntrinsic )
    {
        m_pClass = CBotClass::Find(name);
        if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::CBotTypResult(int type, CBotClass* pClass)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = pClass;
    m_limite    = -1;

    if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::CBotTypResult(int type, CBotTypResult elem)
{
    m_type        = type;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
    m_limite    = -1;

    if ( type == CBotTypArrayPointer ||
         type == CBotTypArrayBody )
        m_pNext = new CBotTypResult( elem );
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::CBotTypResult(const CBotTypResult& typ)
{
    m_type        = typ.m_type;
    m_pClass    = typ.m_pClass;
    m_pNext        = nullptr;
    m_limite    = typ.m_limite;

    if ( typ.m_pNext )
        m_pNext = new CBotTypResult( *typ.m_pNext );
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::CBotTypResult()
{
    m_type        = 0;
    m_limite    = -1;
    m_pNext        = nullptr;
    m_pClass    = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult::~CBotTypResult()
{
    delete    m_pNext;
}

////////////////////////////////////////////////////////////////////////////////
int CBotTypResult::GetType(int mode) const
{
#ifdef    _DEBUG
    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )

         if ( m_pClass == nullptr ) assert(0);


    if ( m_type == CBotTypArrayPointer )
         if ( m_pNext == nullptr ) assert(0);
#endif
    if ( mode == 3 && m_type == CBotTypNullPointer ) return CBotTypPointer;
    return    m_type;
}

////////////////////////////////////////////////////////////////////////////////
void CBotTypResult::SetType(int n)
{
    m_type = n;
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotTypResult::GetClass() const
{
    return m_pClass;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult& CBotTypResult::GetTypElem() const
{
    return *m_pNext;
}

////////////////////////////////////////////////////////////////////////////////
int CBotTypResult::GetLimite() const
{
    return m_limite;
}

////////////////////////////////////////////////////////////////////////////////
void CBotTypResult::SetLimite(int n)
{
    m_limite = n;
}

////////////////////////////////////////////////////////////////////////////////
void CBotTypResult::SetArray( int* max )
{
    m_limite = *max;
    if (m_limite < 1) m_limite = -1;

    if ( m_pNext != nullptr )                    // last dimension?
    {
        m_pNext->SetArray( max+1 );
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotTypResult::Compare(const CBotTypResult& typ) const
{
    if ( m_type != typ.m_type ) return false;

    if ( m_type == CBotTypArrayPointer ) return m_pNext->Compare(*typ.m_pNext);

    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )
    {
        return m_pClass == typ.m_pClass;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotTypResult::Eq(int type) const
{
    return m_type == type;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult& CBotTypResult::operator=(const CBotTypResult& src)
{
    m_type = src.m_type;
    m_limite = src.m_limite;
    m_pClass = src.m_pClass;
    m_pNext = nullptr;
    if ( src.m_pNext != nullptr )
    {
        m_pNext = new CBotTypResult(*src.m_pNext);
    }
    return *this;
}

