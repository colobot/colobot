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
// * along with this program. If not, see  http://www.gnu.org/licenses/.////////////////////////////////////////////////////////////////////
// Définition pour la classe CBotVar
// gestion des variables du langage CBot

// on ne crée jamais d'instance de la class mère CBotVar


#include "CBot.h"
#include <math.h>
#include <stdio.h>

long CBotVar::m_identcpt = 0;

CBotVar::CBotVar( )
{
    m_next  = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = -1;
    m_binit = FALSE;
    m_ident = 0;
    m_bStatic = FALSE;
    m_mPrivate = 0;
}

CBotVarInt::CBotVarInt( const CBotToken* name )
{
    m_token = new CBotToken(name);
    m_next  = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = CBotTypInt;
    m_binit = FALSE;
    m_bStatic = FALSE;
    m_mPrivate = 0;

    m_val   = 0;
}

CBotVarFloat::CBotVarFloat( const CBotToken* name )
{
    m_token = new CBotToken(name);
    m_next  = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = CBotTypFloat;
    m_binit = FALSE;
    m_bStatic = FALSE;
    m_mPrivate = 0;

    m_val   = 0;
}

CBotVarString::CBotVarString( const CBotToken* name )
{
    m_token = new CBotToken(name);
    m_next  = NULL;
    m_pMyThis = NULL;
    m_pUserPtr = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type  = CBotTypString;
    m_binit = FALSE;
    m_bStatic = FALSE;
    m_mPrivate = 0;

    m_val.Empty();
}

CBotVarBoolean::CBotVarBoolean( const CBotToken* name )
{
    m_token     = new CBotToken(name);
    m_next      = NULL;
    m_pMyThis   = NULL;
    m_pUserPtr  = NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_type      = CBotTypBoolean;
    m_binit     = FALSE;
    m_bStatic = FALSE;
    m_mPrivate = 0;

    m_val       = 0;
}

CBotVarClass* CBotVarClass::m_ExClass = NULL;

CBotVarClass::CBotVarClass( const CBotToken* name, const CBotTypResult& type)
{
/*
//  int nIdent = 0;
    InitCBotVarClass( name, type ) //, nIdent );
}

CBotVarClass::CBotVarClass( const CBotToken* name, CBotTypResult& type) //, int &nIdent )
{
    InitCBotVarClass( name, type ); //, nIdent );
}

void CBotVarClass::InitCBotVarClass( const CBotToken* name, CBotTypResult& type ) //, int &nIdent )
{*/
    if ( !type.Eq(CBotTypClass)        &&
         !type.Eq(CBotTypIntrinsic)    &&               // par comodité accepte ces types
         !type.Eq(CBotTypPointer)      &&
         !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) ASM_TRAP();

    m_token     = new CBotToken(name);
    m_next      = NULL;
    m_pMyThis   = NULL;
    m_pUserPtr  = OBJECTCREATED;//NULL;
    m_InitExpr = NULL;
    m_LimExpr = NULL;
    m_pVar      = NULL;
    m_type      = type;
    if ( type.Eq(CBotTypArrayPointer) )    m_type.SetType( CBotTypArrayBody );
    else if ( !type.Eq(CBotTypArrayBody) ) m_type.SetType( CBotTypClass );
                                                // type officel pour cet object

    m_pClass    = NULL;
    m_pParent   = NULL;
    m_binit     = FALSE;
    m_bStatic   = FALSE;
    m_mPrivate  = 0;
    m_bConstructor = FALSE;
    m_CptUse    = 0;
    m_ItemIdent = type.Eq(CBotTypIntrinsic) ? 0 : CBotVar::NextUniqNum();

    // se place tout seul dans la liste
    if (m_ExClass) m_ExClass->m_ExPrev = this;
    m_ExNext  = m_ExClass;
    m_ExPrev  = NULL;
    m_ExClass = this;

    CBotClass* pClass = type.GivClass();
    CBotClass* pClass2 = pClass->GivParent();
    if ( pClass2 != NULL )
    {
        // crée également une instance dans la classe père
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

    // libère l'objet indirect s'il y a lieu
//  if ( m_Indirect != NULL )
//      m_Indirect->DecrementUse();

    // retire la classe de la liste
    if ( m_ExPrev ) m_ExPrev->m_ExNext = m_ExNext;
    else m_ExClass = m_ExNext;

    if ( m_ExNext ) m_ExNext->m_ExPrev = m_ExPrev;
    m_ExPrev = NULL;
    m_ExNext = NULL;

    delete  m_pVar;
}

void CBotVarClass::ConstructorSet()
{
    m_bConstructor = TRUE;
}


CBotVar::~CBotVar( )
{
    delete  m_token;
    delete  m_next;
}

void CBotVar::debug()
{
    const char* p = (LPCTSTR) m_token->GivString();
    CBotString  s = (LPCTSTR) GivValString();
    const char* v = (LPCTSTR) s;

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

BOOL CBotVar::Save1State(FILE* pf)
{
    // cette routine "virtual" ne doit jamais être appellée,
    // il doit y avoir une routine pour chaque classe fille (CBotVarInt, CBotVarFloat, etc)
    // ( voir le type dans m_type )
    ASM_TRAP();
    return FALSE;
}

void CBotVar::Maj(void* pUser, BOOL bContinu)
{
/*  if (!bContinu && m_pMyThis != NULL)
        m_pMyThis->Maj(pUser, TRUE);*/
}


// crée une variable selon son type

CBotVar* CBotVar::Create(const CBotToken* name, int type )
{
    CBotTypResult   t(type);
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
        // crée une nouvelle instance d'une classe
        // et retourne le POINTER sur cette instance
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
                pv = ((CBotVarArray*)pv)->GivItem(0, TRUE);         // crée au moins l'élément [0]
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
    CBotToken   name(n);

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
//          p->SetClass(type.GivClass());
            return p;
        }
    case CBotTypIntrinsic:
        {
            CBotVarClass* p = new CBotVarClass(&name, type);
//          p->SetClass(type.GivClass());
            return p;
        }

    case CBotTypClass:
        // crée une nouvelle instance d'une classe
        // et retourne le POINTER sur cette instance
        {
            CBotVarClass* instance = new CBotVarClass(&name, type);
            CBotVarPointer* pointer = new CBotVarPointer(&name, type);
            pointer->SetPointer( instance );
//          pointer->SetClass( type.GivClass() );
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
                pv = ((CBotVarArray*)pv)->GivItem(0, TRUE);         // crée au moins l'élément [0]
            }

            return array;
        }
    }

    ASM_TRAP();
    return NULL;
}

CBotVar* CBotVar::Create( const char* name, int type, CBotClass* pClass)
{
    CBotToken   token( name, "" );
    CBotVar*    pVar = Create( &token, type );

    if ( type == CBotTypPointer && pClass == NULL )     // pointeur "null" ?
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
    CBotToken   token( name, "" );
    CBotVar*    pVar = Create( &token, CBotTypResult( CBotTypClass, pClass ) );
//  pVar->SetClass( pClass );
    return      pVar;
}

CBotTypResult CBotVar::GivTypResult(int mode)
{
    CBotTypResult   r = m_type;

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
    if (  m_type.Eq(CBotTypClass) ) return IS_DEF;      // toujours défini !

    return  m_binit;
}

void CBotVar::SetInit(BOOL bInit)
{
    m_binit = bInit;
    if ( bInit == 2 ) m_binit = IS_DEF;                 // cas spécial

    if ( m_type.Eq(CBotTypPointer) && bInit == 2 )
    {
        CBotVarClass* instance = GivPointer();
        if ( instance == NULL )
        {
            instance = new CBotVarClass(NULL, m_type);
//          instance->SetClass(((CBotVarPointer*)this)->m_pClass);
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
    return  m_token->GivString();
}

void CBotVar::SetName(const char* name)
{
    m_token->SetString(name);
}

CBotToken* CBotVar::GivToken()
{
    return  m_token;
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

CBotVar* CBotVar::GivItem(int row, BOOL bGrow)
{
    ASM_TRAP();
    return NULL;
}

// dit si une variable appartient à une classe donnée
BOOL CBotVar::IsElemOfClass(const char* name)
{
    CBotClass*  pc = NULL;

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
        if ( pc->GivName() == name ) return TRUE;
        pc = pc->GivParent();
    }

    return FALSE;
}


CBotVar* CBotVar::GivStaticVar()
{
    // rend le pointeur à la variable si elle est statique
    if ( m_bStatic == 0 || m_pMyThis == NULL ) return this;

    CBotClass*  pClass = m_pMyThis->GivClass();
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
            Copy(var, FALSE);
        }
        break;
    default:
        ASM_TRAP();
    }

    m_binit = var->m_binit;     // copie l'état nan s'il y a
}

void CBotVar::SetStatic(BOOL bStatic)
{
    m_bStatic = bStatic;
}

void CBotVar::SetPrivate(int mPrivate)
{
    m_mPrivate = mPrivate;
}

BOOL CBotVar::IsStatic()
{
    return m_bStatic;
}

BOOL CBotVar::IsPrivate(int mode)
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

// toutes ces fonctions doivent être définies dans les classes filles
// dérivées de la classe CBotVar

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

BOOL CBotVar::Lo(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return FALSE;
}

BOOL CBotVar::Hi(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return FALSE;
}

BOOL CBotVar::Ls(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return FALSE;
}

BOOL CBotVar::Hs(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return FALSE;
}

BOOL CBotVar::Eq(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return FALSE;
}

BOOL CBotVar::Ne(CBotVar* left, CBotVar* right)
{
    ASM_TRAP();
    return FALSE;
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

void CBotVar::Copy(CBotVar* pSrc, BOOL bName)
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
    // nop, uniquement pour CBotVarPointer::SetIndirection
}
*/

//////////////////////////////////////////////////////////////////////////////////////

// copie une variable dans une autre
void CBotVarInt::Copy(CBotVar* pSrc, BOOL bName)
{
    CBotVarInt* p = (CBotVarInt*)pSrc;

    if ( bName) *m_token    = *p->m_token;
    m_type      = p->m_type;
    m_val       = p->m_val;
    m_binit     = p->m_binit;
    m_pMyThis   = NULL;
    m_pUserPtr  = p->m_pUserPtr;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    m_defnum    = p->m_defnum;
}




void CBotVarInt::SetValInt(int val, const char* defnum)
{
    m_val = val;
    m_binit = TRUE;
    m_defnum = defnum;
}



void CBotVarInt::SetValFloat(float val)
{
    m_val = (int)val;
    m_binit = TRUE;
}

int CBotVarInt::GivValInt()
{
    return  m_val;
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

    return  res;
}


void CBotVarInt::Mul(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() * right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::Power(CBotVar* left, CBotVar* right)
{
    m_val = (int) pow( (double) left->GivValInt() , (double) right->GivValInt() );
    m_binit = TRUE;
}

int CBotVarInt::Div(CBotVar* left, CBotVar* right)
{
    int r = right->GivValInt();
    if ( r != 0 )
    {
        m_val = left->GivValInt() / r;
        m_binit = TRUE;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

int CBotVarInt::Modulo(CBotVar* left, CBotVar* right)
{
    int r = right->GivValInt();
    if ( r != 0 )
    {
        m_val = left->GivValInt() % r;
        m_binit = TRUE;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

void CBotVarInt::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() + right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::Sub(CBotVar* left, CBotVar* right)
{
        m_val = left->GivValInt() - right->GivValInt();
        m_binit = TRUE;
}

void CBotVarInt::XOr(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() ^ right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::And(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() & right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::Or(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() | right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::SL(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() << right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::ASR(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() >> right->GivValInt();
    m_binit = TRUE;
}

void CBotVarInt::SR(CBotVar* left, CBotVar* right)
{
    int source = left->GivValInt();
    int shift  = right->GivValInt();
    if (shift>=1) source &= 0x7fffffff;
    m_val = source >> shift;
    m_binit = TRUE;
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

BOOL CBotVarInt::Lo(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() < right->GivValInt();
}

BOOL CBotVarInt::Hi(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() > right->GivValInt();
}

BOOL CBotVarInt::Ls(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() <= right->GivValInt();
}

BOOL CBotVarInt::Hs(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() >= right->GivValInt();
}

BOOL CBotVarInt::Eq(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() == right->GivValInt();
}

BOOL CBotVarInt::Ne(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() != right->GivValInt();
}


//////////////////////////////////////////////////////////////////////////////////////

// copie une variable dans une autre
void CBotVarFloat::Copy(CBotVar* pSrc, BOOL bName)
{
    CBotVarFloat*   p = (CBotVarFloat*)pSrc;

    if (bName)  *m_token    = *p->m_token;
    m_type      = p->m_type;
    m_val       = p->m_val;
    m_binit     = p->m_binit;
//- m_bStatic   = p->m_bStatic;
    m_next      = NULL;
    m_pMyThis   = NULL;//p->m_pMyThis;
    m_pUserPtr  = p->m_pUserPtr;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}




void CBotVarFloat::SetValInt(int val, const char* s)
{
    m_val = (float)val;
    m_binit = TRUE;
}

void CBotVarFloat::SetValFloat(float val)
{
    m_val = val;
    m_binit = TRUE;
}

int CBotVarFloat::GivValInt()
{
    return  (int)m_val;
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

    return  res;
}


void CBotVarFloat::Mul(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValFloat() * right->GivValFloat();
    m_binit = TRUE;
}

void CBotVarFloat::Power(CBotVar* left, CBotVar* right)
{
    m_val = (float)pow( left->GivValFloat() , right->GivValFloat() );
    m_binit = TRUE;
}

int CBotVarFloat::Div(CBotVar* left, CBotVar* right)
{
    float   r = right->GivValFloat();
    if ( r != 0 )
    {
        m_val = left->GivValFloat() / r;
        m_binit = TRUE;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

int CBotVarFloat::Modulo(CBotVar* left, CBotVar* right)
{
    float   r = right->GivValFloat();
    if ( r != 0 )
    {
        m_val = (float)fmod( left->GivValFloat() , r );
        m_binit = TRUE;
    }
    return ( r == 0 ? TX_DIVZERO : 0 );
}

void CBotVarFloat::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValFloat() + right->GivValFloat();
    m_binit = TRUE;
}

void CBotVarFloat::Sub(CBotVar* left, CBotVar* right)
{
        m_val = left->GivValFloat() - right->GivValFloat();
        m_binit = TRUE;
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


BOOL CBotVarFloat::Lo(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() < right->GivValFloat();
}

BOOL CBotVarFloat::Hi(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() > right->GivValFloat();
}

BOOL CBotVarFloat::Ls(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() <= right->GivValFloat();
}

BOOL CBotVarFloat::Hs(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() >= right->GivValFloat();
}

BOOL CBotVarFloat::Eq(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() == right->GivValFloat();
}

BOOL CBotVarFloat::Ne(CBotVar* left, CBotVar* right)
{
    return left->GivValFloat() != right->GivValFloat();
}


//////////////////////////////////////////////////////////////////////////////////////

// copie une variable dans une autre
void CBotVarBoolean::Copy(CBotVar* pSrc, BOOL bName)
{
    CBotVarBoolean* p = (CBotVarBoolean*)pSrc;

    if (bName)  *m_token    = *p->m_token;
    m_type      = p->m_type;
    m_val       = p->m_val;
    m_binit     = p->m_binit;
//- m_bStatic   = p->m_bStatic;
    m_next      = NULL;
    m_pMyThis   = NULL;//p->m_pMyThis;
    m_pUserPtr  = p->m_pUserPtr;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}




void CBotVarBoolean::SetValInt(int val, const char* s)
{
    m_val = (BOOL)val;
    m_binit = TRUE;
}

void CBotVarBoolean::SetValFloat(float val)
{
    m_val = (BOOL)val;
    m_binit = TRUE;
}

int CBotVarBoolean::GivValInt()
{
    return  m_val;
}

float CBotVarBoolean::GivValFloat()
{
    return (float)m_val;
}

CBotString CBotVarBoolean::GivValString()
{
    CBotString  ret;

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
    return  ret;
}

void CBotVarBoolean::And(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() && right->GivValInt();
    m_binit = TRUE;
}
void CBotVarBoolean::Or(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() || right->GivValInt();
    m_binit = TRUE;
}

void CBotVarBoolean::XOr(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValInt() ^ right->GivValInt();
    m_binit = TRUE;
}

void CBotVarBoolean::Not()
{
    m_val = m_val ? FALSE : TRUE ;
}

BOOL CBotVarBoolean::Eq(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() == right->GivValInt();
}

BOOL CBotVarBoolean::Ne(CBotVar* left, CBotVar* right)
{
    return left->GivValInt() != right->GivValInt();
}

//////////////////////////////////////////////////////////////////////////////////////

// copie une variable dans une autre
void CBotVarString::Copy(CBotVar* pSrc, BOOL bName)
{
    CBotVarString*  p = (CBotVarString*)pSrc;

    if (bName)  *m_token    = *p->m_token;
    m_type      = p->m_type;
    m_val       = p->m_val;
    m_binit     = p->m_binit;
//- m_bStatic   = p->m_bStatic;
    m_next      = NULL;
    m_pMyThis   = NULL;//p->m_pMyThis;
    m_pUserPtr  = p->m_pUserPtr;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}


void CBotVarString::SetValString(const char* p)
{
    m_val = p;
    m_binit = TRUE;
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

    return  m_val;
}


void CBotVarString::Add(CBotVar* left, CBotVar* right)
{
    m_val = left->GivValString() + right->GivValString();
    m_binit = TRUE;
}

BOOL CBotVarString::Eq(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

BOOL CBotVarString::Ne(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() != right->GivValString());
}


BOOL CBotVarString::Lo(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

BOOL CBotVarString::Hi(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

BOOL CBotVarString::Ls(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}

BOOL CBotVarString::Hs(CBotVar* left, CBotVar* right)
{
    return (left->GivValString() == right->GivValString());
}


////////////////////////////////////////////////////////////////

// copie une variable dans une autre
void CBotVarClass::Copy(CBotVar* pSrc, BOOL bName)
{
    pSrc = pSrc->GivPointer();                  // si source donné par un pointeur

    if ( pSrc->GivType() != CBotTypClass )
        ASM_TRAP();

    CBotVarClass*   p = (CBotVarClass*)pSrc;

    if (bName)  *m_token    = *p->m_token;

    m_type      = p->m_type;
    m_binit     = p->m_binit;
//- m_bStatic   = p->m_bStatic;
    m_pClass    = p->m_pClass;
    if ( p->m_pParent )
    {
        ASM_TRAP();     "que faire du pParent";
    }

//  m_next      = NULL;
    m_pUserPtr  = p->m_pUserPtr;
    m_pMyThis   = NULL;//p->m_pMyThis;
    m_ItemIdent = p->m_ItemIdent;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    delete      m_pVar;
    m_pVar      = NULL;

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
    delete  m_pVar;
    m_pVar  = pVar; // remplace le pointeur existant
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

    // initialise les variables associées à cette classe
    delete m_pVar;
    m_pVar = NULL;

    if (pClass == NULL) return;

    CBotVar*    pv = pClass->GivVar();              // premier de la liste
    while ( pv != NULL )
    {
        // cherche les dimensions max du tableau
        CBotInstr*  p  = pv->m_LimExpr;                         // les différentes formules
        if ( p != NULL )
        {
            CBotStack* pile = CBotStack::FirstStack();  // une pile indépendante
            int  n = 0;
            int  max[100];

            while (p != NULL)
            {
                while( pile->IsOk() && !p->Execute(pile) ) ;        // calcul de la taille sans interruptions
                CBotVar*    v = pile->GivVar();                     // résultat
                max[n] = v->GivValInt();                            // valeur
                n++;
                p = p->GivNext3();
            }
            while (n<100) max[n++] = 0;

            pv->m_type.SetArray( max );                 // mémorise les limitations
            pile->Delete();
        }

        CBotVar*    pn = CBotVar::Create( pv );     // une copie
        pn->SetStatic(pv->IsStatic());
        pn->SetPrivate(pv->GivPrivate());

        if ( pv->m_InitExpr != NULL )               // expression pour l'initialisation ?
        {
#if STACKMEM
            CBotStack* pile = CBotStack::FirstStack();  // une pile indépendante

            while(pile->IsOk() && !pv->m_InitExpr->Execute(pile, pn));  // évalue l'expression sans timer

            pile->Delete();
#else
            CBotStack* pile = new CBotStack(NULL);  // une pile indépendante
            while(!pv->m_InitExpr->Execute(pile));  // évalue l'expression sans timer
            pn->SetVal( pile->GivVar() ) ;
            delete pile;
#endif
        }

//      pn->SetUniqNum(CBotVar::NextUniqNum());     // numérote les éléments
        pn->SetUniqNum(pv->GivUniqNum());   //++nIdent
        pn->m_pMyThis = this;

        if ( m_pVar == NULL) m_pVar = pn;
        else m_pVar->AddNext( pn );
        pv = pv->GivNext();
    }
}

CBotClass* CBotVarClass::GivClass()
{
    return  m_pClass;
}


void CBotVarClass::Maj(void* pUser, BOOL bContinu)
{
/*  if (!bContinu && m_pMyThis != NULL)
        m_pMyThis->Maj(pUser, TRUE);*/

    // une routine de mise à jour existe-elle ?

    if ( m_pClass->m_rMaj == NULL ) return;

    // récupère le pointeur user selon la classe
    // ou selon le paramètre passé au CBotProgram::Run()

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

// pour la gestion d'un tableau
// bExtend permet d'agrandir le tableau, mais pas au dela de la taille fixée par SetArray()

CBotVar* CBotVarClass::GivItem(int n, BOOL bExtend)
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
//  if ( m_Indirect != NULL) return m_Indirect->GivValString();

    CBotString  res;

    if ( m_pClass != NULL )                     // pas utilisé pour un array
    {
        res = m_pClass->GivName() + CBotString("( ");

        CBotVarClass*   my = this;
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
    return  res;
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
        // s'il y en a un, appel le destructeur
        // mais seulement si un constructeur avait été appelé.
        if ( m_bConstructor )
        {
            m_CptUse++; // ne revient pas dans le destructeur

            // m_error est static dans le stack
            // sauve la valeur pour la remettre ensuite
            int err, start, end;
            CBotStack*  pile = NULL;
            err = pile->GivError(start,end);    // pile == NULL ça ne derange pas !!

            pile = CBotStack::FirstStack();     // efface l'erreur
            CBotVar*    ppVars[1];
            ppVars[0] = NULL;

            CBotVar*    pThis  = CBotVar::Create("this", CBotTypNullPointer);
            pThis->SetPointer(this);
            CBotVar*    pResult = NULL;

            CBotString  nom = "~" + m_pClass->GivName();
            long        ident = 0;

            while ( pile->IsOk() && !m_pClass->ExecuteMethode(ident, nom, pThis, ppVars, pResult, pile, NULL)) ;    // attend la fin

            pile->ResetError(err, start,end);

            pile->Delete();
            delete pThis;
            m_CptUse--;
        }

        delete this; // s'auto-détruit !!
    }
}

CBotVarClass* CBotVarClass::GivPointer()
{
    return this;
}


// trouve une instance selon son numéro unique

CBotVarClass* CBotVarClass::Find(long id)
{
    CBotVarClass*   p = m_ExClass;

    while ( p != NULL )
    {
        if ( p->m_ItemIdent == id ) return p;
        p = p->m_ExNext;
    }

    return NULL;
}

BOOL CBotVarClass::Eq(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GivItemList();
    CBotVar*    r = right->GivItemList();

    while ( l != NULL && r != NULL )
    {
        if ( l->Ne(l, r) ) return FALSE;
        l = l->GivNext();
        r = r->GivNext();
    }

    // devrait toujours arrivé simultanément au bout (mêmes classes)
    return l == r;
}

BOOL CBotVarClass::Ne(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GivItemList();
    CBotVar*    r = right->GivItemList();

    while ( l != NULL && r != NULL )
    {
        if ( l->Ne(l, r) ) return TRUE;
        l = l->GivNext();
        r = r->GivNext();
    }

    // devrait toujours arrivé simultanément au bout (mêmes classes)
    return l != r;
}

/////////////////////////////////////////////////////////////////////////////
// gestion des tableaux de variables

CBotVarArray::CBotVarArray(const CBotToken* name, CBotTypResult& type )
{
    if ( !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) ASM_TRAP();

    m_token     = new CBotToken(name);
    m_next      = NULL;
    m_pMyThis   = NULL;
    m_pUserPtr  = NULL;

    m_type      = type;
    m_type.SetType(CBotTypArrayPointer);
    m_binit     = FALSE;

    m_pInstance = NULL;                     // la liste des éléments du tableau
}

CBotVarArray::~CBotVarArray()
{
    if ( m_pInstance != NULL ) m_pInstance->DecrementUse(); // une référence en moins
}

// copie une variable dans une autre
void CBotVarArray::Copy(CBotVar* pSrc, BOOL bName)
{
    if ( pSrc->GivType() != CBotTypArrayPointer )
        ASM_TRAP();

    CBotVarArray*   p = (CBotVarArray*)pSrc;

    if ( bName) *m_token    = *p->m_token;
    m_type      = p->m_type;
    m_pInstance = p->GivPointer();

    if ( m_pInstance != NULL )
         m_pInstance->IncrementUse();           // une référence en plus

    m_binit     = p->m_binit;
//- m_bStatic   = p->m_bStatic;
    m_pMyThis   = NULL;//p->m_pMyThis;
    m_pUserPtr  = p->m_pUserPtr;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

void CBotVarArray::SetPointer(CBotVar* pVarClass)
{
    m_binit = TRUE;                         // init, même sur un pointeur null

    if ( m_pInstance == pVarClass) return;  // spécial, ne pas décrémenter et réincrémenter
                                            // car le décrément peut détruire l'object

    if ( pVarClass != NULL )
    {
        if ( pVarClass->GivType() == CBotTypArrayPointer )
             pVarClass = pVarClass->GivPointer();   // le vrai pointeur à l'objet

        if ( !pVarClass->m_type.Eq(CBotTypClass) &&
             !pVarClass->m_type.Eq(CBotTypArrayBody))
            ASM_TRAP();

        ((CBotVarClass*)pVarClass)->IncrementUse();         // une référence en plus
    }

    if ( m_pInstance != NULL ) m_pInstance->DecrementUse();
    m_pInstance = (CBotVarClass*)pVarClass;
}


CBotVarClass* CBotVarArray::GivPointer()
{
    if ( m_pInstance == NULL ) return NULL;
    return m_pInstance->GivPointer();
}

CBotVar* CBotVarArray::GivItem(int n, BOOL bExtend)
{
    if ( m_pInstance == NULL )
    {
        if ( !bExtend ) return NULL;
        // crée une instance pour le tableau

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

BOOL CBotVarArray::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return FALSE;
    return SaveVar(pf, m_pInstance);                        // sauve l'instance qui gère le tableau
}


/////////////////////////////////////////////////////////////////////////////
// gestion des pointeurs à une instance donnée

CBotVarPointer::CBotVarPointer(const CBotToken* name, CBotTypResult& type )
{
    if ( !type.Eq(CBotTypPointer) &&
         !type.Eq(CBotTypNullPointer) &&
         !type.Eq(CBotTypClass)   &&                    // par commodité accepte Class et Intrinsic
         !type.Eq(CBotTypIntrinsic) ) ASM_TRAP();

    m_token     = new CBotToken(name);
    m_next      = NULL;
    m_pMyThis   = NULL;
    m_pUserPtr  = NULL;

    m_type      = type;
    if ( !type.Eq(CBotTypNullPointer) )
        m_type.SetType(CBotTypPointer);                 // quoi qu'il en soit, c'est un pointeur
    m_binit     = FALSE;
    m_pClass    = NULL;
    m_pVarClass = NULL;                                 // sera défini par un SetPointer()

    SetClass(type.GivClass() );
}

CBotVarPointer::~CBotVarPointer()
{
    if ( m_pVarClass != NULL ) m_pVarClass->DecrementUse(); // une référence en moins
}


void CBotVarPointer::Maj(void* pUser, BOOL bContinu)
{
/*  if ( !bContinu && m_pMyThis != NULL )
         m_pMyThis->Maj(pUser, FALSE);*/

    if ( m_pVarClass != NULL) m_pVarClass->Maj(pUser, FALSE);
}

CBotVar* CBotVarPointer::GivItem(const char* name)
{
    if ( m_pVarClass == NULL)               // pas d'instance existant ?
        return m_pClass->GivItem(name);     // rend le pointeur dans la classe elle-même

    return m_pVarClass->GivItem(name);
}

CBotVar* CBotVarPointer::GivItemRef(int nIdent)
{
    if ( m_pVarClass == NULL)               // pas d'instance existant ?
        return m_pClass->GivItemRef(nIdent);// rend le pointeur dans la classe elle-même

    return m_pVarClass->GivItemRef(nIdent);
}

CBotVar* CBotVarPointer::GivItemList()
{
    if ( m_pVarClass == NULL) return NULL;
    return m_pVarClass->GivItemList();
}

CBotString CBotVarPointer::GivValString()
{
    CBotString  s = "Pointer to ";
    if ( m_pVarClass == NULL ) s = "Null pointer" ;
    else  s += m_pVarClass->GivValString();
    return s;
}


void CBotVarPointer::ConstructorSet()
{
    if ( m_pVarClass != NULL) m_pVarClass->ConstructorSet();
}

// initialise le pointeur vers l'instance d'une classe

void CBotVarPointer::SetPointer(CBotVar* pVarClass)
{
    m_binit = TRUE;                         // init, même sur un pointeur null

    if ( m_pVarClass == pVarClass) return;  // spécial, ne pas décrémenter et réincrémenter
                                            // car le décrément peut détruire l'object

    if ( pVarClass != NULL )
    {
        if ( pVarClass->GivType() == CBotTypPointer )
             pVarClass = pVarClass->GivPointer();   // le vrai pointeur à l'objet

//      if ( pVarClass->GivType() != CBotTypClass )
        if ( !pVarClass->m_type.Eq(CBotTypClass) )
            ASM_TRAP();

        ((CBotVarClass*)pVarClass)->IncrementUse();         // une référence en plus
        m_pClass = ((CBotVarClass*)pVarClass)->m_pClass;
        m_pUserPtr = pVarClass->m_pUserPtr;                 // pas vraiment indispensable
        m_type = CBotTypResult(CBotTypPointer, m_pClass);   // un pointeur de quel genre
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
//  int     nIdent = 0;
    m_type.m_pClass = m_pClass = pClass;
    if ( m_pVarClass != NULL ) m_pVarClass->SetClass(pClass); //, nIdent);
}

CBotClass* CBotVarPointer::GivClass()
{
    if ( m_pVarClass != NULL ) return m_pVarClass->GivClass();

    return  m_pClass;
}


BOOL CBotVarPointer::Save1State(FILE* pf)
{
    if ( m_pClass )
    {
        if (!WriteString(pf, m_pClass->GivName())) return FALSE;    // nom de la classe
    }
    else
    {
        if (!WriteString(pf, "")) return FALSE;
    }

    if (!WriteLong(pf, GivIdent())) return FALSE;       // la référence unique

    // sauve aussi une copie de l'instance
    return SaveVar(pf, GivPointer());
}

// copie une variable dans une autre
void CBotVarPointer::Copy(CBotVar* pSrc, BOOL bName)
{
    if ( pSrc->GivType() != CBotTypPointer &&
         pSrc->GivType() != CBotTypNullPointer)
        ASM_TRAP();

    CBotVarPointer* p = (CBotVarPointer*)pSrc;

    if ( bName) *m_token    = *p->m_token;
    m_type      = p->m_type;
//  m_pVarClass = p->m_pVarClass;
    m_pVarClass = p->GivPointer();

    if ( m_pVarClass != NULL )
         m_pVarClass->IncrementUse();           // une référence en plus

    m_pClass    = p->m_pClass;
    m_binit     = p->m_binit;
//- m_bStatic   = p->m_bStatic;
    m_next      = NULL;
    m_pMyThis   = NULL;//p->m_pMyThis;
    m_pUserPtr  = p->m_pUserPtr;

    // garde le même idendificateur (par défaut)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

BOOL CBotVarPointer::Eq(CBotVar* left, CBotVar* right)
{
    CBotVarClass*   l = left->GivPointer();
    CBotVarClass*   r = right->GivPointer();

    if ( l == r ) return TRUE;
    if ( l == NULL && r->GivUserPtr() == OBJECTDELETED ) return TRUE;
    if ( r == NULL && l->GivUserPtr() == OBJECTDELETED ) return TRUE;
    return FALSE;
}

BOOL CBotVarPointer::Ne(CBotVar* left, CBotVar* right)
{
    CBotVarClass*   l = left->GivPointer();
    CBotVarClass*   r = right->GivPointer();

    if ( l == r ) return FALSE;
    if ( l == NULL && r->GivUserPtr() == OBJECTDELETED ) return FALSE;
    if ( r == NULL && l->GivUserPtr() == OBJECTDELETED ) return FALSE;
    return TRUE;
}



///////////////////////////////////////////////////////
// gestion des types de résultats


CBotTypResult::CBotTypResult(int type)
{
    m_type      = type;
    m_pNext     = NULL;
    m_pClass    = NULL;
    m_limite    = -1;
}

CBotTypResult::CBotTypResult(int type, const char* name)
{
    m_type      = type;
    m_pNext     = NULL;
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
    m_type      = type;
    m_pNext     = NULL;
    m_pClass    = pClass;
    m_limite    = -1;

    if ( m_pClass && m_pClass->IsIntrinsic() ) m_type = CBotTypIntrinsic;
}

CBotTypResult::CBotTypResult(int type, CBotTypResult elem)
{
    m_type      = type;
    m_pNext     = NULL;
    m_pClass    = NULL;
    m_limite    = -1;

    if ( type == CBotTypArrayPointer ||
         type == CBotTypArrayBody )
        m_pNext = new CBotTypResult( elem );
}

CBotTypResult::CBotTypResult(const CBotTypResult& typ)
{
    m_type      = typ.m_type;
    m_pClass    = typ.m_pClass;
    m_pNext     = NULL;
    m_limite    = typ.m_limite;

    if ( typ.m_pNext )
        m_pNext = new CBotTypResult( *typ.m_pNext );
}

CBotTypResult::CBotTypResult()
{
    m_type      = 0;
    m_limite    = -1;
    m_pNext     = NULL;
    m_pClass    = NULL;
}

CBotTypResult::~CBotTypResult()
{
    delete  m_pNext;
}

int CBotTypResult::GivType(int mode) const
{
#ifdef  _DEBUG
    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )

         if ( m_pClass == NULL ) ASM_TRAP();


    if ( m_type == CBotTypArrayPointer )
         if ( m_pNext == NULL ) ASM_TRAP();
#endif
    if ( mode == 3 && m_type == CBotTypNullPointer ) return CBotTypPointer;
    return  m_type;
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

    if ( m_pNext != NULL )                  // dernière dimension ?
    {
        m_pNext->SetArray( max+1 );
    }
}



BOOL CBotTypResult::Compare(const CBotTypResult& typ) const
{
    if ( m_type != typ.m_type ) return FALSE;

    if ( m_type == CBotTypArrayPointer ) return m_pNext->Compare(*typ.m_pNext);

    if ( m_type == CBotTypPointer ||
         m_type == CBotTypClass   ||
         m_type == CBotTypIntrinsic )
    {
        return m_pClass == typ.m_pClass;
    }

    return TRUE;
}

BOOL CBotTypResult::Eq(int type) const
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


