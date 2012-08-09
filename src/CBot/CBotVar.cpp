// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

////////////////////////////////////////////////////////////////////
// Definition for the class CBotVar
// variables management of the language CBoT

// it never creates an instance of the class mother CBotVar

#include "CBot.h"
#include <math.h>
#include <stdio.h>

long CBotVar::m_identcpt = 0;

CBotVar::CBotVar( )
{
    m_next    = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = -1;
    m_binit = false;
    m_ident = 0;
    m_bStatic = false;
    m_mPrivate = 0;
}

CBotVarInt::CBotVarInt( const CBotToken* name )
{
    m_token    = new CBotToken(name);
    m_next    = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = CBotTypInt;
    m_binit = false;
    m_bStatic = false;
    m_mPrivate = 0;

    m_val    = 0;
}

CBotVarFloat::CBotVarFloat( const CBotToken* name )
{
    m_token    = new CBotToken(name);
    m_next    = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = CBotTypFloat;
    m_binit = false;
    m_bStatic = false;
    m_mPrivate = 0;

    m_val    = 0;
}

CBotVarString::CBotVarString( const CBotToken* name )
{
    m_token    = new CBotToken(name);
    m_next    = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = CBotTypString;
    m_binit = false;
    m_bStatic = false;
    m_mPrivate = 0;

    m_val.Empty();
}

CBotVarBoolean::CBotVarBoolean( const CBotToken* name )
{
    m_token        = new CBotToken(name);
    m_next        = NULL;
    m_pMyThis    = NULL;
    m_pUserPtr    = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type        = CBotTypBoolean;
    m_binit        = false;
    m_bStatic = false;
    m_mPrivate = 0;

    m_val        = 0;
}

CBotVarClass* CBotVarClass::m_ExClass = NULL;

CBotVarClass::CBotVarClass( const CBotToken* name, const CBotTypResult& type)
{
/*
//    int    nIdent = 0;
    InitCBotVarClass( name, type ) //, nIdent );
}

CBotVarClass::CBotVarClass( const CBotToken* name, CBotTypResult& type) //, int &nIdent )
{
    InitCBotVarClass( name, type ); //, nIdent );
}

void CBotVarClass::InitCBotVarClass( const CBotToken* name, CBotTypResult& type ) //, int &nIdent )
{*/
    if ( !type.Eq(CBotTypClass)        &&
         !type.Eq(CBotTypIntrinsic)    &&                // by convenience there accepts these types
         !type.Eq(CBotTypPointer)      &&
         !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) ASM_TRAP();

    m_token        = new CBotToken(name);
    m_next        = NULL;
    m_pMyThis    = NULL;
    m_pUserPtr    = OBJECTCREATED;//NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_pVar        = NULL;
    m_type        = type;
    if ( type.Eq(CBotTypArrayPointer) )    m_type.SetType( CBotTypArrayBody );    
    else if ( !type.Eq(CBotTypArrayBody) ) m_type.SetType( CBotTypClass );    
                                                 // officel type for this object

    m_pClass    = NULL;
    m_pParent    = NULL;
    m_binit        = false;
    m_bStatic    = false;
    m_mPrivate    = 0;
    m_bConstructor = false;
    m_CptUse    = 0;
    m_ItemIdent = type.Eq(CBotTypIntrinsic) ? 0 : CBotVar::NextUniqNum();

    // se place tout seul dans la liste
    // TODO stands alone in the list (stands only in a list)
    if (m_ExClass) m_ExClass->m_ExPrev = this;
    m_ExNext  = m_ExClass;
    m_ExPrev  = NULL;
    m_ExClass = this;

    CBotClass* pClass = type.GivClass();
    CBotClass* pClass2 = pClass->GivParent();
    if ( pClass2 != NULL )
    {
        // also creates an instance of the parent class
        m_pParent = new CBotVarClass(name, CBotTypResult(type.GivType(),pClass2) ); //, nIdent);
    }

    SetClass( pClass ); //, nIdent );

}

CBotVarClass::~CBotVarClass( )
{
    if ( m_CptUse != 0 )
        ASM_TRAP();

    if ( m_pParent ) delete m_pParent;
    m_pParent = NULL;

    // frees the indirect object if necessary
//    if ( m_Indirect != NULL )
//        m_Indirect->DecrementUse();

    // removes the class list
    if ( m_ExPrev ) m_ExPrev->m_ExNext = m_ExNext;
    else m_ExClass = m_ExNext;

    if ( m_ExNext ) m_ExNext->m_ExPrev = m_ExPrev;
    m_ExPrev = NULL;
    m_ExNext = NULL;

    delete    m_pVar;
}

void CBotVarClass::ConstructorSet()
{
    m_bConstructor = true;
}


CBotVar::~CBotVar( )
{
    delete  m_token;
    delete    m_next;
}

void CBotVar::debug()
{
    const char*    p = (const char*) m_token->GivString();
    CBotString  s = (const char*) GivValString();
    const char* v = (const char*) s;

    if ( m_type.Eq(CBotTypClass) )
    {
        CBotVar*    pv = ((CBotVarClass*)this)->m_pVar;
        while (pv != NULL)
        {
            pv->debug();
            pv = pv->GivNext();
        }
    }
}

void CBotVar::ConstructorSet()
{
    // nop
}

void CBotVar::SetUserPtr(void* pUser)
{
    m_pUserPtr = pUser;
    if (m_type.Eq(CBotTypPointer) &&
        ((CBotVarPointer*)this)->m_pVarClass != NULL )
        ((CBotVarPointer*)this)->m_pVarClass->SetUserPtr(pUser);
}

void CBotVar::SetIdent(long n)
{
    if (m_type.Eq(CBotTypPointer) &&
        ((CBotVarPointer*)this)->m_pVarClass != NULL )
        ((CBotVarPointer*)this)->m_pVarClass->SetIdent(n);
}

void CBotVar::SetUniqNum(long n)
{
    m_ident = n;

    if ( n == 0 ) ASM_TRAP();
}

long CBotVar::NextUniqNum()
{
    if (++m_identcpt < 10000) m_identcpt = 10000;
    return m_identcpt;
}

long CBotVar::GivUniqNum()
{
    return m_ident;
}


void* CBotVar::GivUserPtr()
{
    return m_pUserPtr;
}

bool CBotVar::Save1State(FILE* pf)
{
    // this routine "virtual" must never be called,
    // there must be a routine for each of the subclasses (CBotVarInt, CBotVarFloat, etc)
    // ( see the type in m_type )
    ASM_TRAP();
    return false;
}

void CBotVar::Maj(void* pUser, bool bContinu)
{
/*    if (!bContinu && m_pMyThis != NULL)
        m_pMyThis->Maj(pUser, true);*/
}


// creates a variable depending on its type

CBotVar* CBotVar::Create(const CBotToken* name, int type )
{
    CBotTypResult    t(type);
    return Create(name, t);
}

CBotVar* CBotVar::Create(const CBotToken* name, CBotTypResult type)
{
    switch (type.GivType())
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
                type = type.GivTypElem();
                pv = ((CBotVarArray*)pv)->GivItem(0, true);            // creates at least the element [0]
            }

            return array;
        }
    }

    ASM_TRAP();
    return NULL;
}

CBotVar* CBotVar::Create( CBotVar* pVar )
{
    CBotVar*    p = Create(pVar->m_token->GivString(), pVar->GivTypResult(2));
    return p;
}


CBotVar* CBotVar::Create( const char* n, CBotTypResult type)
{
    CBotToken    name(n);

    switch (type.GivType())
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
//            p->SetClass(type.GivClass());
            return p;
        }
    case CBotTypIntrinsic:
        {
            CBotVarClass* p = new CBotVarClass(&name, type);
//            p->SetClass(type.GivClass());
            return p;
        }

    case CBotTypClass:
        // creates a new instance of a class
        // and returns the POINTER on this instance
        {
            CBotVarClass* instance = new CBotVarClass(&name, type);
            CBotVarPointer* pointer = new CBotVarPointer(&name, type);
            pointer->SetPointer( instance );
//            pointer->SetClass( type.GivClass() );
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
                type = type.GivTypElem();
                pv = ((CBotVarArray*)pv)->GivItem(0, true);            // creates at least the element [0]
            }

            return array;
        }
    }

    ASM_TRAP();
    return NULL;
}

CBotVar* CBotVar::Create( const char* name, int type, CBotClass* pClass)
{
    CBotToken    token( name, "" );
    CBotVar*    pVar = Create( &token, type );
    
    if ( type == CBotTypPointer && pClass == NULL )        // pointer "null" ?
        return pVar;

    if ( type == CBotTypClass || type == CBotTypPointer ||
         type == CBotTypIntrinsic )
    {
        if (pClass == NULL) 
        {
            delete pVar;
            return NULL;
        }
        pVar->SetClass( pClass );
    }
    return pVar;
}

CBotVar* CBotVar::Create( const char* name, CBotClass* pClass)
{
    CBotToken    token( name, "" );
    CBotVar*    pVar = Create( &token, CBotTypResult( CBotTypClass, pClass ) );
//    pVar->SetClass( pClass );
    return        pVar;
}

CBotTypResult CBotVar::GivTypResult(int mode)
{
    CBotTypResult    r = m_type;

    if ( mode == 1 && m_type.Eq(CBotTypClass) ) 
        r.SetType(CBotTypPointer);
    if ( mode == 2 && m_type.Eq(CBotTypClass) ) 
        r.SetType(CBotTypIntrinsic);

    return r;
}

int CBotVar::GivType(int mode)
{
    if ( mode == 1 && m_type.Eq(CBotTypClass) ) 
        return CBotTypPointer;
    if ( mode == 2 && m_type.Eq(CBotTypClass) ) 
        return CBotTypIntrinsic;
    return m_type.GivType();
}

void CBotVar::SetType(CBotTypResult& type)
{
    m_type = type;
}


int CBotVar::GivInit()
{
    if (  m_type.Eq(CBotTypClass) ) return IS_DEF;        // always set!

    return    m_binit;
}

void CBotVar::SetInit(int bInit)
{
    m_binit = bInit;
    if ( bInit == 2 ) m_binit = IS_DEF;                    // cas spécial

    if ( m_type.Eq(CBotTypPointer) && bInit == 2 )
    {
        CBotVarClass* instance = GivPointer();
        if ( instance == NULL )
        {
            instance = new CBotVarClass(NULL, m_type);
//            instance->SetClass(((CBotVarPointer*)this)->m_pClass);
            SetPointer(instance);
        }
        instance->SetInit(1);
    }

    if ( m_type.Eq(CBotTypClass) || m_type.Eq(CBotTypIntrinsic) )
    {
        CBotVar*    p = ((CBotVarClass*)this)->m_pVar;
        while( p != NULL )
        {
            p->SetInit( bInit );
            p->m_pMyThis = (CBotVarClass*)this;
            p = p->GivNext();
        }
    }
}

CBotString CBotVar::GivName()
{
    return    m_token->GivString();
}

void CBotVar::SetName(const char* name)
{
    m_token->SetString(name);
}

CBotToken* CBotVar::GivToken()
{
    return    m_token;
}

CBotVar* CBotVar::GivItem(const char* name)
{
    ASM_TRAP();
    return NULL;
}

CBotVar* CBotVar::GivItemRef(int nIdent)
{
    ASM_TRAP();
    return NULL;
}

CBotVar* CBotVar::GivItemList()
{
    ASM_TRAP();
    return NULL;
}

CBotVar* CBotVar::GivItem(int row, bool bGrow)
{
    ASM_TRAP();
    return NULL;
}

// check if a variable belongs to a given class
bool CBotVar::IsElemOfClass(const char* name)
{
    CBotClass*    pc = NULL;

    if ( m_type.Eq(CBotTypPointer) )
    {
        pc = ((CBotVarPointer*)this)->m_pClass;
    }
    if ( m_type.Eq(CBotTypClass) )
    {
        pc = ((CBotVarClass*)this)->m_pClass;
    }

    while ( pc != NULL )
    {
        if ( pc->GivName() == name ) return true;
        pc = pc->GivParent();
    }

    return false;
}


CBotVar* CBotVar::GivStaticVar()
{
    // makes the pointer to the variable if it is static
    if ( m_bStatic == 0 || m_pMyThis == NULL ) return this;

    CBotClass*    pClass = m_pMyThis->GivClass();
    return pClass->GivItem( m_token->GivString() );
}


CBotVar* CBotVar::GivNext()
{
    return m_next;
}

void CBotVar::AddNext(CBotVar* pVar)
{
    CBotVar*    p = this;
    while (p->m_next != NULL) p = p->m_next;

    p->m_next = pVar;
}

void CBotVar::SetVal(CBotVar* var)
{
    switch (/*var->*/GivType())
    {
    case CBotTypBoolean:
        SetValInt(var->GivValInt());
        break;
    case CBotTypInt:
        SetValInt(var->GivValInt(), ((CBotVarInt*)var)->m_defnum);
        break;
    case CBotTypFloat:
        SetValFloat(var->GivValFloat());
        break;
    case CBotTypString:
        SetValString(var->GivValString());
        break;
    case CBotTypPointer:
    case CBotTypNullPointer:
    case CBotTypArrayPointer:
        SetPointer(var->GivPointer());
        break;
    case CBotTypClass:
        {
            delete ((CBotVarClass*)this)->m_pVar;
            ((CBotVarClass*)this)->m_pVar = NULL;
            Copy(var, false);
        }
        break;
    default:
        ASM_TRAP();
    }

    m_binit = var->m_binit;        // copie l'état nan s'il y a 
}

void CBotVar::SetStatic(bool bStatic)
{
    m_bStatic = bStatic;
}

void CBotVar::SetPrivate(int mPrivate)
{
    m_mPrivate = mPrivate;
}

bool CBotVar::IsStatic()
{
    return m_bStatic;
}

bool CBotVar::IsPrivate(int mode)
{
    return m_mPrivate >= mode;
}

int CBotVar::GivPrivate()
{
    return m_mPrivate;
}


void CBotVar::SetPointer(CBotVar* pVarClass)
{
    ASM_TRAP();
}

CBotVarClass* CBotVar::GivPointer()
{
    ASM_TRAP();
    return NULL;
}

// All these functions must be defined in the subclasses
// derived from class CBotVar

int CBotVar::GivValInt()
{
    ASM_TRAP();
    return 0;
}

float CBotVar::GivValFloat()
{
    ASM_TRAP();
    return 0;
}

void CBotVar::SetValInt(int c, const char* s)
{
    ASM_TRAP();
}

void CBotVar::SetValFloat(float c)
{
    ASM_TRAP();
}

void CBotVar::Mul(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::Power(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

int CBotVar::Div(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return 0;
}

int CBotVar::Modulo(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return 0;
}

void CBotVar::Add(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::Sub(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

bool CBotVar::Lo(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return false;
}

bool CBotVar::Hi(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return false;
}

bool CBotVar::Ls(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return false;
}

bool CBotVar::Hs(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return false;
}

bool CBotVar::Eq(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return false;
}

bool CBotVar::Ne(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return false;
}

void CBotVar::And(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::Or(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::XOr(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::ASR(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::SR(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::SL(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
}

void CBotVar::Neg()
{
    ASM_TRAP();
}

void CBotVar::Not()
{
    ASM_TRAP();
}

void CBotVar::Inc()
{
    ASM_TRAP();
}
void CBotVar::Dec()
{
    ASM_TRAP();
}

void CBotVar::Copy(CBotVar* pSrc, bool bName)
{
    ASM_TRAP();
}

void CBotVar::SetValString(const char* p)
{
    ASM_TRAP();
}

CBotString CBotVar::GivValString()
{
    ASM_TRAP();
    return CBotString();
}

void CBotVar::SetClass(CBotClass* pClass)
{
    ASM_TRAP();
}

CBotClass* CBotVar::GivClass()
{
    ASM_TRAP();
    return NULL;
}

/*
void CBotVar::SetIndirection(CBotVar* pVar)
{
    // nop, only  CBotVarPointer::SetIndirection
}
*/

//////////////////////////////////////////////////////////////////////////////////////

// copy a variable in to another
void CBotVarInt::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarInt*    p = (CBotVarInt*)pSrc;

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_val        = p->m_val;
    m_binit        = p->m_binit;
    m_pMyThis    = NULL;
    m_pUserPtr    = p->m_pUserPtr;

    // identificator is the same (by défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    m_defnum    = p->m_defnum;
}




void CBotVarInt::SetValInt(int val, const char* defnum)
{
    m_val = val;
    m_binit    = true;
    m_defnum = defnum;
}



void CBotVarInt::SetValFloat(float val)
{
    m_val = (int)val;
    m_binit    = true;
}

int CBotVarInt::GivValInt()
{
    return    m_val;
}

float CBotVarInt::GivValFloat()
{
    return (float)m_val;
}

CBotString CBotVarInt::GivValString()
{
    if ( !m_defnum.IsEmpty() ) return m_defnum;

    CBotString res;

    if ( !m_binit )
    {
        res.LoadString(TX_UNDEF);
        return res;
    }
    if ( m_binit == IS_NAN )
    {
        res.LoadString(TX_NAN);
        return res;
    }

    char        buffer[300];
    sprintf(buffer, "%d", m_val);
    res = buffer;

    return    res;
}


void CBotVarInt::Mul(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() * right->GivValInt();
    m_binit = true;
}

void CBotVarInt::Power(CBotVar* left, CBotVar* right)
{
    m_val = (int) pow( (double) left->GivValInt() , (double) right->GivValInt() );
    m_binit = true;
}

int CBotVarInt::Div(CBotVar* left, CBotVar* right)
{
    int    r = right->GivValInt();
    if ( r != 0 )
    {
        m_val = left->GivValInt() / r;
        m_binit = true;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

int CBotVarInt::Modulo(CBotVar* left, CBotVar* right)
{
    int    r = right->GivValInt();
    if ( r != 0 )
    {
        m_val = left->GivValInt() % r;
        m_binit = true;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

void CBotVarInt::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() + right->GivValInt();
    m_binit = true;
}

void CBotVarInt::Sub(CBotVar* left, CBotVar* right)
{
        m_val = left->GivValInt() - right->GivValInt();
        m_binit = true;
}

void CBotVarInt::XOr(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() ^ right->GivValInt();
    m_binit = true;
}

void CBotVarInt::And(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() & right->GivValInt();
    m_binit = true;
}

void CBotVarInt::Or(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() | right->GivValInt();
    m_binit = true;
}

void CBotVarInt::SL(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() << right->GivValInt();
    m_binit = true;
}

void CBotVarInt::ASR(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() >> right->GivValInt();
    m_binit = true;
}

void CBotVarInt::SR(CBotVar* left, CBotVar* right)
{
    int    source = left->GivValInt();
    int shift  = right->GivValInt();
    if (shift>=1) source &= 0x7fffffff;
    m_val = source >> shift;
    m_binit = true;
}

void CBotVarInt::Neg()
{
        m_val = -m_val;
}

void CBotVarInt::Not()
{
        m_val = ~m_val;
}

void CBotVarInt::Inc()
{
        m_val++;
        m_defnum.Empty();
}

void CBotVarInt::Dec()
{
        m_val--;
        m_defnum.Empty();
}

bool CBotVarInt::Lo(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() < right->GivValInt();
}

bool CBotVarInt::Hi(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() > right->GivValInt();
}

bool CBotVarInt::Ls(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() <= right->GivValInt();
}

bool CBotVarInt::Hs(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() >= right->GivValInt();
}

bool CBotVarInt::Eq(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() == right->GivValInt();
}

bool CBotVarInt::Ne(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() != right->GivValInt();
}


//////////////////////////////////////////////////////////////////////////////////////

// copy a variable into another
void CBotVarFloat::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarFloat*    p = (CBotVarFloat*)pSrc;

    if (bName)     *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_val        = p->m_val;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = NULL;
    m_pMyThis    = NULL;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}




void CBotVarFloat::SetValInt(int val, const char* s)
{
    m_val = (float)val;
    m_binit    = true;
}

void CBotVarFloat::SetValFloat(float val)
{
    m_val = val;
    m_binit    = true;
}

int CBotVarFloat::GivValInt()
{
    return    (int)m_val;
}

float CBotVarFloat::GivValFloat()
{
    return m_val;
}

CBotString CBotVarFloat::GivValString()
{
    CBotString res;

    if ( !m_binit )
    {
        res.LoadString(TX_UNDEF);
        return res;
    }
    if ( m_binit == IS_NAN )
    {
        res.LoadString(TX_NAN);
        return res;
    }

    char        buffer[300];
    sprintf(buffer, "%.2f", m_val);
    res = buffer;

    return    res;
}


void CBotVarFloat::Mul(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValFloat() * right->GivValFloat();
    m_binit = true;
}

void CBotVarFloat::Power(CBotVar* left, CBotVar* right)
{
    m_val = (float)pow( left->GivValFloat() , right->GivValFloat() );
    m_binit = true;
}

int CBotVarFloat::Div(CBotVar* left, CBotVar* right)
{
    float    r = right->GivValFloat();
    if ( r != 0 )
    {
        m_val = left->GivValFloat() / r;
        m_binit = true;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

int CBotVarFloat::Modulo(CBotVar* left, CBotVar* right)
{
    float    r = right->GivValFloat();
    if ( r != 0 )
    {
        m_val = (float)fmod( left->GivValFloat() , r );
        m_binit = true;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

void CBotVarFloat::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValFloat() + right->GivValFloat();
    m_binit = true;
}

void CBotVarFloat::Sub(CBotVar* left, CBotVar* right)
{
        m_val = left->GivValFloat() - right->GivValFloat();
        m_binit = true;
}

void CBotVarFloat::Neg()
{
        m_val = -m_val;
}

void CBotVarFloat::Inc()
{
        m_val++;
}

void CBotVarFloat::Dec()
{
        m_val--;
}


bool CBotVarFloat::Lo(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() < right->GivValFloat();
}

bool CBotVarFloat::Hi(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() > right->GivValFloat();
}

bool CBotVarFloat::Ls(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() <= right->GivValFloat();
}

bool CBotVarFloat::Hs(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() >= right->GivValFloat();
}

bool CBotVarFloat::Eq(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() == right->GivValFloat();
}

bool CBotVarFloat::Ne(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() != right->GivValFloat();
}


//////////////////////////////////////////////////////////////////////////////////////

// copy a variable into another
void CBotVarBoolean::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarBoolean*    p = (CBotVarBoolean*)pSrc;

    if (bName)    *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_val        = p->m_val;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = NULL;
    m_pMyThis    = NULL;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}




void CBotVarBoolean::SetValInt(int val, const char* s)
{
    m_val = (bool)val;
    m_binit    = true;
}

void CBotVarBoolean::SetValFloat(float val)
{
    m_val = (bool)val;
    m_binit    = true;
}

int CBotVarBoolean::GivValInt()
{
    return    m_val;
}

float CBotVarBoolean::GivValFloat()
{
    return (float)m_val;
}

CBotString CBotVarBoolean::GivValString()
{
    CBotString    ret;

    CBotString res;

    if ( !m_binit )
    {
        res.LoadString(TX_UNDEF);
        return res;
    }
    if ( m_binit == IS_NAN )
    {
        res.LoadString(TX_NAN);
        return res;
    }

    ret.LoadString( m_val > 0 ? ID_TRUE : ID_FALSE );
    return    ret;
}

void CBotVarBoolean::And(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() && right->GivValInt();
    m_binit = true;
}
void CBotVarBoolean::Or(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() || right->GivValInt();
    m_binit = true;
}

void CBotVarBoolean::XOr(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() ^ right->GivValInt();
    m_binit = true;
}

void CBotVarBoolean::Not()
{
    m_val = m_val ? false : true ; 
}

bool CBotVarBoolean::Eq(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() == right->GivValInt();
}

bool CBotVarBoolean::Ne(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() != right->GivValInt();
}

//////////////////////////////////////////////////////////////////////////////////////

// copy a variable into another
void CBotVarString::Copy(CBotVar* pSrc, bool bName)
{
    CBotVarString*    p = (CBotVarString*)pSrc;

    if (bName)    *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_val        = p->m_val;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = NULL;
    m_pMyThis    = NULL;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}


void CBotVarString::SetValString(const char* p)
{
    m_val = p;
    m_binit    = true;
}

CBotString CBotVarString::GivValString()
{
    if ( !m_binit )
    {
        CBotString res;
        res.LoadString(TX_UNDEF);
        return res;
    }
    if ( m_binit == IS_NAN )
    {
        CBotString res;
        res.LoadString(TX_NAN);
        return res;
    }

    return    m_val;
}


void CBotVarString::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValString() + right->GivValString();
    m_binit = true;
}

bool CBotVarString::Eq(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

bool CBotVarString::Ne(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() != right->GivValString());
}


bool CBotVarString::Lo(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

bool CBotVarString::Hi(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

bool CBotVarString::Ls(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

bool CBotVarString::Hs(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}


////////////////////////////////////////////////////////////////

// copy a variable into another
void CBotVarClass::Copy(CBotVar* pSrc, bool bName)
{
    pSrc = pSrc->GivPointer();                    // if source given by a pointer

    if ( pSrc->GivType() != CBotTypClass )
        ASM_TRAP();

    CBotVarClass*    p = (CBotVarClass*)pSrc;

    if (bName)    *m_token    = *p->m_token;

    m_type        = p->m_type;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pClass    = p->m_pClass;
    if ( p->m_pParent )
    {
        ASM_TRAP();        "que faire du pParent";
    }

//    m_next        = NULL;
    m_pUserPtr    = p->m_pUserPtr;
    m_pMyThis    = NULL;//p->m_pMyThis;
    m_ItemIdent = p->m_ItemIdent;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    delete        m_pVar;
    m_pVar        = NULL;

    CBotVar*    pv = p->m_pVar;
    while( pv != NULL )
    {
        CBotVar*    pn = CBotVar::Create(pv);
        pn->Copy( pv );
        if ( m_pVar == NULL ) m_pVar = pn;
        else m_pVar->AddNext(pn);

        pv = pv->GivNext();
    }
}

void CBotVarClass::SetItemList(CBotVar* pVar)
{
    delete    m_pVar;
    m_pVar    = pVar;    // replaces the existing pointer
}

void CBotVarClass::SetIdent(long n)
{
    m_ItemIdent = n;
}

void CBotVarClass::SetClass(CBotClass* pClass)//, int &nIdent)
{
    m_type.m_pClass = pClass;

    if ( m_pClass == pClass ) return;

    m_pClass = pClass;

    // initializes the variables associated with this class
    delete m_pVar;
    m_pVar = NULL;

    if (pClass == NULL) return;

    CBotVar*    pv = pClass->GivVar();                // first on a list
    while ( pv != NULL )
    {
        // seeks the maximum dimensions of the table
        CBotInstr*    p  = pv->m_LimExpr;                            // the different formulas
        if ( p != NULL )
        {
            CBotStack* pile = CBotStack::FirstStack();    // an independent stack 
            int     n = 0;
            int     max[100];

            while (p != NULL)
            {
                while( pile->IsOk() && !p->Execute(pile) ) ;        // calculate size without interruptions
                CBotVar*    v = pile->GivVar();                        // result
                max[n] = v->GivValInt();                            // value
                n++;
                p = p->GivNext3();
            }
            while (n<100) max[n++] = 0;

            pv->m_type.SetArray( max );                    // stores the limitations
            pile->Delete();
        }

        CBotVar*    pn = CBotVar::Create( pv );        // a copy
        pn->SetStatic(pv->IsStatic());
        pn->SetPrivate(pv->GivPrivate());

        if ( pv->m_InitExpr != NULL )                // expression for initialization?
        {
#if    STACKMEM
            CBotStack* pile = CBotStack::FirstStack();    // an independent stack

            while(pile->IsOk() && !pv->m_InitExpr->Execute(pile, pn));    // evaluates the expression without timer

            pile->Delete();
#else
            CBotStack* pile = new CBotStack(NULL);     // an independent stack
            while(!pv->m_InitExpr->Execute(pile));    // evaluates the expression without timer
            pn->SetVal( pile->GivVar() ) ;
            delete pile;
#endif
        }

//        pn->SetUniqNum(CBotVar::NextUniqNum());        // enumerate elements
        pn->SetUniqNum(pv->GivUniqNum());    //++nIdent
        pn->m_pMyThis = this;

        if ( m_pVar == NULL) m_pVar = pn;
        else m_pVar->AddNext( pn );
        pv = pv->GivNext();
    }
}

CBotClass* CBotVarClass::GivClass()
{
    return    m_pClass;
}


void CBotVarClass::Maj(void* pUser, bool bContinu)
{
/*    if (!bContinu && m_pMyThis != NULL) 
        m_pMyThis->Maj(pUser, true);*/

    // an update routine exist?

    if ( m_pClass->m_rMaj == NULL ) return;

    // retrieves the user pointer according to the class
    // or according to the parameter passed to CBotProgram::Run()

    if ( m_pUserPtr != NULL) pUser = m_pUserPtr;
    if ( pUser == OBJECTDELETED ||
         pUser == OBJECTCREATED ) return;
    m_pClass->m_rMaj( this, pUser );
}

CBotVar* CBotVarClass::GivItem(const char* name)
{
    CBotVar*    p = m_pVar;

    while ( p != NULL )
    {
        if ( p->GivName() == name ) return p;
        p = p->GivNext();
    }

    if ( m_pParent != NULL ) return m_pParent->GivItem(name);
    return NULL;
}

CBotVar* CBotVarClass::GivItemRef(int nIdent)
{
    CBotVar*    p = m_pVar;

    while ( p != NULL )
    {
        if ( p->GivUniqNum() == nIdent ) return p;
        p = p->GivNext();
    }

    if ( m_pParent != NULL ) return m_pParent->GivItemRef(nIdent);
    return NULL;
}

// for the management of an array
// bExtend can enlarge the table, but not beyond the threshold size of SetArray ()

CBotVar* CBotVarClass::GivItem(int n, bool bExtend)
{
    CBotVar*    p = m_pVar;

    if ( n < 0 ) return NULL;
    if ( n > MAXARRAYSIZE ) return NULL;

    if ( m_type.GivLimite() >= 0 && n >= m_type.GivLimite() ) return NULL;

    if ( p == NULL && bExtend )
    {
        p = CBotVar::Create("", m_type.GivTypElem());
        m_pVar = p;
    }

    if ( n == 0 ) return p;

    while ( n-- > 0 )
    {
        if ( p->m_next == NULL )
        {
            if ( bExtend ) p->m_next = CBotVar::Create("", m_type.GivTypElem());
            if ( p->m_next == NULL ) return NULL;
        }
        p = p->m_next;
    }

    return p;
}

CBotVar* CBotVarClass::GivItemList()
{
    return m_pVar;
}


CBotString CBotVarClass::GivValString()
{
//    if ( m_Indirect != NULL) return m_Indirect->GivValString();

    CBotString    res;

    if ( m_pClass != NULL )                        // not used for an array
    {
        res = m_pClass->GivName() + CBotString("( ");

        CBotVarClass*    my = this;
        while ( my != NULL )
        {
            CBotVar*    pv = my->m_pVar;
            while ( pv != NULL )
            {
                res += pv->GivName() + CBotString("=");

                if ( pv->IsStatic() )
                {
                    CBotVar* pvv = my->m_pClass->GivItem(pv->GivName());
                    res += pvv->GivValString();
                }
                else
                {
                    res += pv->GivValString();
                }
                pv = pv->GivNext();
                if ( pv != NULL ) res += ", ";
            }
            my = my->m_pParent;
            if ( my != NULL )
            {
                res += ") extends ";
                res += my->m_pClass->GivName();
                res += " (";
            }
        }
    }
    else
    {
        res = "( ";

        CBotVar*    pv = m_pVar;
        while ( pv != NULL )
        {
            res += pv->GivValString();
            if ( pv->GivNext() != NULL ) res += ", ";
            pv = pv->GivNext();
        }
    }

    res += " )";
    return    res;
}

void CBotVarClass::IncrementUse()
{
    m_CptUse++;
}

void CBotVarClass::DecrementUse()
{
    m_CptUse--;
    if ( m_CptUse == 0 ) 
    {
        // if there is one, call the destructor
        // but only if a constructor had been called.
        if ( m_bConstructor )
        {
            m_CptUse++;    // does not return to the destructor

            // m_error is static in the stack
            // saves the value for return
            int    err, start, end;
            CBotStack*    pile = NULL;
            err = pile->GivError(start,end);    // stack == NULL it does not bother!

            pile = CBotStack::FirstStack();        // clears the error
            CBotVar*    ppVars[1];
            ppVars[0] = NULL;

            CBotVar*    pThis  = CBotVar::Create("this", CBotTypNullPointer);
            pThis->SetPointer(this);
            CBotVar*    pResult = NULL;

            CBotString    nom = "~" + m_pClass->GivName();
            long        ident = 0;

            while ( pile->IsOk() && !m_pClass->ExecuteMethode(ident, nom, pThis, ppVars, pResult, pile, NULL)) ;    // waits for the end

            pile->ResetError(err, start,end);

            pile->Delete();
            delete pThis;
            m_CptUse--;
        }

        delete this; // self-destructs!
    }
}

CBotVarClass* CBotVarClass::GivPointer()
{
    return this;
}


// makes an instance according to its unique number

CBotVarClass* CBotVarClass::Find(long id)
{
    CBotVarClass*    p = m_ExClass;

    while ( p != NULL )
    {
        if ( p->m_ItemIdent == id ) return p;
        p = p->m_ExNext;
    }

    return NULL;
}

bool CBotVarClass::Eq(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GivItemList();
    CBotVar*    r = right->GivItemList();

    while ( l != NULL && r != NULL )
    {
        if ( l->Ne(l, r) ) return false;
        l = l->GivNext();
        r = r->GivNext();
    }

    // should always arrived simultaneously at the end (same classes)
    return l == r;
}

bool CBotVarClass::Ne(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GivItemList();
    CBotVar*    r = right->GivItemList();

    while ( l != NULL && r != NULL )
    {
        if ( l->Ne(l, r) ) return true;
        l = l->GivNext();
        r = r->GivNext();
    }

    // should always arrived simultaneously at the end (same classes)
    return l != r;
}

/////////////////////////////////////////////////////////////////////////////
// management of arrays

CBotVarArray::CBotVarArray(const CBotToken* name, CBotTypResult& type )
{
    if ( !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) ASM_TRAP();

    m_token        = new CBotToken(name);
    m_next        = NULL;
    m_pMyThis    = NULL;
    m_pUserPtr    = NULL;

    m_type        = type;
    m_type.SetType(CBotTypArrayPointer);
    m_binit        = false;

    m_pInstance    = NULL;                        // the list of the array elements
}

CBotVarArray::~CBotVarArray()
{
    if ( m_pInstance != NULL ) m_pInstance->DecrementUse();    // the lowest reference
}

// copy a variable into another
void CBotVarArray::Copy(CBotVar* pSrc, bool bName)
{
    if ( pSrc->GivType() != CBotTypArrayPointer )
        ASM_TRAP();

    CBotVarArray*    p = (CBotVarArray*)pSrc;

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_pInstance = p->GivPointer();

    if ( m_pInstance != NULL )
         m_pInstance->IncrementUse();            // a reference increase

    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pMyThis    = NULL;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

void CBotVarArray::SetPointer(CBotVar* pVarClass)
{
    m_binit = true;                            // init, even on a null pointer

    if ( m_pInstance == pVarClass) return;    // Special, not decrement and reincrement
                                            // because the decrement can destroy the object

    if ( pVarClass != NULL )
    {
        if ( pVarClass->GivType() == CBotTypArrayPointer )
             pVarClass = pVarClass->GivPointer();    // the real pointer to the object

        if ( !pVarClass->m_type.Eq(CBotTypClass) &&
             !pVarClass->m_type.Eq(CBotTypArrayBody))
            ASM_TRAP();

        ((CBotVarClass*)pVarClass)->IncrementUse();            // incement the reference
    }

    if ( m_pInstance != NULL ) m_pInstance->DecrementUse();
    m_pInstance = (CBotVarClass*)pVarClass;
}


CBotVarClass* CBotVarArray::GivPointer()
{
    if ( m_pInstance == NULL ) return NULL;
    return m_pInstance->GivPointer();
}

CBotVar* CBotVarArray::GivItem(int n, bool bExtend)
{
    if ( m_pInstance == NULL )
    {
        if ( !bExtend ) return NULL;
        // creates an instance of the table

        CBotVarClass* instance = new CBotVarClass(NULL, m_type);
        SetPointer( instance );
    }
    return m_pInstance->GivItem(n, bExtend);
}

CBotVar* CBotVarArray::GivItemList()
{
    if ( m_pInstance == NULL) return NULL;
    return m_pInstance->GivItemList();
}

CBotString CBotVarArray::GivValString()
{
    if ( m_pInstance == NULL ) return ( CBotString( "Null pointer" ) ) ;
    return m_pInstance->GivValString();
}

bool CBotVarArray::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return false;
    return SaveVar(pf, m_pInstance);                        // saves the instance that manages the table
}


/////////////////////////////////////////////////////////////////////////////
// gestion des pointeurs à une instance donnée
// TODO management of pointers to a given instance

CBotVarPointer::CBotVarPointer(const CBotToken* name, CBotTypResult& type )
{
    if ( !type.Eq(CBotTypPointer) &&
         !type.Eq(CBotTypNullPointer) &&
         !type.Eq(CBotTypClass)   &&                    // for convenience accepts Class and Intrinsic
         !type.Eq(CBotTypIntrinsic) ) ASM_TRAP();

    m_token        = new CBotToken(name);
    m_next        = NULL;
    m_pMyThis    = NULL;
    m_pUserPtr    = NULL;

    m_type        = type;
    if ( !type.Eq(CBotTypNullPointer) )
        m_type.SetType(CBotTypPointer);                    // anyway, this is a pointer
    m_binit        = false;
    m_pClass    = NULL;
    m_pVarClass = NULL;                                    // will be defined by a SetPointer()

    SetClass(type.GivClass() );
}

CBotVarPointer::~CBotVarPointer()
{
    if ( m_pVarClass != NULL ) m_pVarClass->DecrementUse();    // decrement reference
}


void CBotVarPointer::Maj(void* pUser, bool bContinu)
{
/*    if ( !bContinu && m_pMyThis != NULL )
         m_pMyThis->Maj(pUser, false);*/

    if ( m_pVarClass != NULL) m_pVarClass->Maj(pUser, false);
}

CBotVar* CBotVarPointer::GivItem(const char* name)
{
    if ( m_pVarClass == NULL)                // no existing instance?
        return m_pClass->GivItem(name);        // makes the pointer in the class itself

    return m_pVarClass->GivItem(name);
}

CBotVar* CBotVarPointer::GivItemRef(int nIdent)
{
    if ( m_pVarClass == NULL)                // no existing instance?
        return m_pClass->GivItemRef(nIdent);// makes the pointer to the class itself

    return m_pVarClass->GivItemRef(nIdent);
}

CBotVar* CBotVarPointer::GivItemList()
{
    if ( m_pVarClass == NULL) return NULL;
    return m_pVarClass->GivItemList();
}

CBotString CBotVarPointer::GivValString()
{
    CBotString    s = "Pointer to ";
    if ( m_pVarClass == NULL ) s = "Null pointer" ;
    else  s += m_pVarClass->GivValString();
    return s;
}


void CBotVarPointer::ConstructorSet()
{
    if ( m_pVarClass != NULL) m_pVarClass->ConstructorSet();
}

// initializes the pointer to the instance of a class

void CBotVarPointer::SetPointer(CBotVar* pVarClass)
{
    m_binit = true;                            // init, even on a null pointer

    if ( m_pVarClass == pVarClass) return;    // special, not decrement and reincrement
                                            // because the decrement can destroy the object

    if ( pVarClass != NULL )
    {
        if ( pVarClass->GivType() == CBotTypPointer )
             pVarClass = pVarClass->GivPointer();    // the real pointer to the object

//        if ( pVarClass->GivType() != CBotTypClass )
        if ( !pVarClass->m_type.Eq(CBotTypClass) )
            ASM_TRAP();

        ((CBotVarClass*)pVarClass)->IncrementUse();            // increment the reference
        m_pClass = ((CBotVarClass*)pVarClass)->m_pClass;
        m_pUserPtr = pVarClass->m_pUserPtr;                    // not really necessary
        m_type = CBotTypResult(CBotTypPointer, m_pClass);    // what kind of a pointer
    }

    if ( m_pVarClass != NULL ) m_pVarClass->DecrementUse();
    m_pVarClass = (CBotVarClass*)pVarClass;

}

CBotVarClass* CBotVarPointer::GivPointer()
{
    if ( m_pVarClass == NULL ) return NULL;
    return m_pVarClass->GivPointer();
}

void CBotVarPointer::SetIdent(long n)
{
    if ( m_pVarClass == NULL ) return;
    m_pVarClass->SetIdent( n );
}

long CBotVarPointer::GivIdent()
{
    if ( m_pVarClass == NULL ) return 0;
    return m_pVarClass->m_ItemIdent;
}


void CBotVarPointer::SetClass(CBotClass* pClass)
{
//    int        nIdent = 0;
    m_type.m_pClass = m_pClass = pClass;
    if ( m_pVarClass != NULL ) m_pVarClass->SetClass(pClass); //, nIdent);
}

CBotClass* CBotVarPointer::GivClass()
{
    if ( m_pVarClass != NULL ) return m_pVarClass->GivClass();

    return    m_pClass;
}


bool CBotVarPointer::Save1State(FILE* pf)
{
    if ( m_pClass )
    {
        if (!WriteString(pf, m_pClass->GivName())) return false;    // name of the class
    }
    else
    {
        if (!WriteString(pf, "")) return false;
    }

    if (!WriteLong(pf, GivIdent())) return false;        // the unique reference

    // also saves the proceedings copies  
    return SaveVar(pf, GivPointer());
}

// copy a variable into another
void CBotVarPointer::Copy(CBotVar* pSrc, bool bName)
{
    if ( pSrc->GivType() != CBotTypPointer &&
         pSrc->GivType() != CBotTypNullPointer)
        ASM_TRAP();

    CBotVarPointer*    p = (CBotVarPointer*)pSrc;

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
//    m_pVarClass = p->m_pVarClass;
    m_pVarClass = p->GivPointer();

    if ( m_pVarClass != NULL )
         m_pVarClass->IncrementUse();            // incerement the reference

    m_pClass    = p->m_pClass;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = NULL;
    m_pMyThis    = NULL;//p->m_pMyThis;
    m_pUserPtr    = p->m_pUserPtr;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

bool CBotVarPointer::Eq(CBotVar* left, CBotVar* right)
{
    CBotVarClass*    l = left->GivPointer();
    CBotVarClass*    r = right->GivPointer();

    if ( l == r ) return true;
    if ( l == NULL && r->GivUserPtr() == OBJECTDELETED ) return true;
    if ( r == NULL && l->GivUserPtr() == OBJECTDELETED ) return true;
    return false;
}

bool CBotVarPointer::Ne(CBotVar* left, CBotVar* right)
{
    CBotVarClass*    l = left->GivPointer();
    CBotVarClass*    r = right->GivPointer();

    if ( l == r ) return false;
    if ( l == NULL && r->GivUserPtr() == OBJECTDELETED ) return false;
    if ( r == NULL && l->GivUserPtr() == OBJECTDELETED ) return false;
    return true;
}



///////////////////////////////////////////////////////
// management of results types 


CBotTypResult::CBotTypResult(int type)
{
    m_type        = type;
    m_pNext        = NULL;
    m_pClass    = NULL;
    m_limite    = -1;
}

CBotTypResult::CBotTypResult(int type, const char* name)
{
    m_type        = type;
    m_pNext        = NULL;
    m_pClass    = NULL;
    m_limite    = -1;

    if ( type == CBotTypPointer ||
         type == CBotTypClass   ||
         type == CBotTypIntrinsic )
    {
        m_pClass = CBotClass::Find(name);
        if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
    }
}

CBotTypResult::CBotTypResult(int type, CBotClass* pClass)
{
    m_type        = type;
    m_pNext        = NULL;
    m_pClass    = pClass;
    m_limite    = -1;

    if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
}

CBotTypResult::CBotTypResult(int type, CBotTypResult elem)
{
    m_type        = type;
    m_pNext        = NULL;
    m_pClass    = NULL;
    m_limite    = -1;

    if ( type == CBotTypArrayPointer ||
         type == CBotTypArrayBody )
        m_pNext = new CBotTypResult( elem );
}

CBotTypResult::CBotTypResult(const CBotTypResult& typ)
{
    m_type        = typ.m_type;
    m_pClass    = typ.m_pClass;
    m_pNext        = NULL;
    m_limite    = typ.m_limite;

    if ( typ.m_pNext )
        m_pNext = new CBotTypResult( *typ.m_pNext );
}

CBotTypResult::CBotTypResult()
{
    m_type        = 0;
    m_limite    = -1;
    m_pNext        = NULL;
    m_pClass    = NULL;
}

CBotTypResult::~CBotTypResult()
{
    delete    m_pNext;
}

int CBotTypResult::GivType(int mode) const
{
#ifdef    _DEBUG
    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )

         if ( m_pClass == NULL ) ASM_TRAP();

        
    if ( m_type == CBotTypArrayPointer )
         if ( m_pNext == NULL ) ASM_TRAP();
#endif
    if ( mode == 3 && m_type == CBotTypNullPointer ) return CBotTypPointer;
    return    m_type;
}

void CBotTypResult::SetType(int n)
{
    m_type = n;
}

CBotClass* CBotTypResult::GivClass() const
{
    return m_pClass;
}

CBotTypResult& CBotTypResult::GivTypElem() const
{
    return *m_pNext;
}

int CBotTypResult::GivLimite() const
{
    return m_limite;
}

void CBotTypResult::SetLimite(int n)
{
    m_limite = n;
}

void CBotTypResult::SetArray( int* max )
{
    m_limite = *max;
    if (m_limite < 1) m_limite = -1;

    if ( m_pNext != NULL )                    // last dimension?
    {
        m_pNext->SetArray( max+1 );
    }
}



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

bool CBotTypResult::Eq(int type) const
{
    return m_type == type;
}

CBotTypResult&
    CBotTypResult::operator=(const CBotTypResult& src)
{
    m_type = src.m_type;
    m_limite = src.m_limite;
    m_pClass = src.m_pClass;
    m_pNext = NULL;
    if ( src.m_pNext != NULL )
    {
        m_pNext = new CBotTypResult(*src.m_pNext);
    }
    return *this;
}


