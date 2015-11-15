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

/////////////////////////////////////////////////////////////////////
// Management of variables of class type
//

#include "CBot.h"

#include "CBotCall.h"

#include "CBotInstr/CBotNew.h"
#include "CBotInstr/CBotLeftExprVar.h"
#include "CBotInstr/CBotTwoOpExpr.h"

CBotClass* CBotClass::m_ExClass = nullptr;

CBotClass::CBotClass(const char* name, CBotClass* pPapa, bool bIntrinsic)
{
    m_pParent   = pPapa;
    m_name      = name;
    m_pVar      = nullptr;
    m_next      = nullptr;
    m_pCalls    = nullptr;
    m_pMethod   = nullptr;
    m_rMaj      = nullptr;
    m_IsDef     = true;
    m_bIntrinsic= bIntrinsic;
    m_cptLock   = 0;
    m_cptOne    = 0;
    m_nbVar     = m_pParent == nullptr ? 0 : m_pParent->m_nbVar;

    for ( int j= 0; j< 5 ; j++ )
    {
        m_ProgInLock[j] = nullptr;
    }


    // is located alone in the list
    if (m_ExClass) m_ExClass->m_ExPrev = this;
    m_ExNext  = m_ExClass;
    m_ExPrev  = nullptr;
    m_ExClass = this;

}

CBotClass::~CBotClass()
{
    // removes the list of class
    if ( m_ExPrev ) m_ExPrev->m_ExNext = m_ExNext;
    else m_ExClass = m_ExNext;

    if ( m_ExNext ) m_ExNext->m_ExPrev = m_ExPrev;
    m_ExPrev = nullptr;
    m_ExNext = nullptr;

    delete  m_pVar;
    delete  m_pCalls;
    delete  m_pMethod;

    delete  m_next;         // releases all of them on this level
}

CBotClass* CBotClass::Create(const char* name, CBotClass* parent, bool intrinsic)
{
    return new CBotClass(name, parent, intrinsic);
}

void CBotClass::Free()
{
    while ( m_ExClass != nullptr )
    {
        delete m_ExClass;
    }
}

void CBotClass::Purge()
{
    if ( this == nullptr ) return;

    delete      m_pVar;
    m_pVar      = nullptr;
    delete      m_pCalls;
    m_pCalls    = nullptr;
    delete      m_pMethod;
    m_pMethod   = nullptr;
    m_IsDef     = false;

    m_nbVar     = m_pParent == nullptr ? 0 : m_pParent->m_nbVar;

    m_next->Purge();
    m_next = nullptr;          // no longer belongs to this chain
}

bool CBotClass::Lock(CBotProgram* p)
{
    int i = m_cptLock++;

    if ( i == 0 )
    {
        m_cptOne = 1;
        m_ProgInLock[0] = p;
        return true;
    }
    if ( p == m_ProgInLock[0] )
    {
        m_cptOne++;
        m_cptLock--;                                // has already been counted
        return true;
    }

    for ( int j = 1 ; j <= i ; j++)
    {
        if ( p == m_ProgInLock[j] )
        {
            m_cptLock--;
            return false;   // already pending
        }
    }

    if ( i < 5 )    // max 5 in query
    {
        m_ProgInLock[i] = p;                        // located in a queue
    }
    else
        m_cptLock--;

    return false;
}

void CBotClass::Unlock()
{
    if ( --m_cptOne > 0 ) return ;

    int i = --m_cptLock;
    if ( i<0 )
    {
        m_cptLock = 0;
        return;
    }

    for ( int j= 0; j< i ; j++ )
    {
        m_ProgInLock[j] = m_ProgInLock[j+1];
    }
    m_ProgInLock[i] = nullptr;
}

void CBotClass::FreeLock(CBotProgram* p)
{
    CBotClass* pClass = m_ExClass;

    while ( pClass != nullptr )
    {
        if ( p == pClass->m_ProgInLock[0] )
        {
            pClass->m_cptLock -= pClass->m_cptOne;
            pClass->m_cptOne = 0;
        }

        for ( int j = 1; j < 5 ; j++ )
            if ( p == pClass->m_ProgInLock[j] )
                pClass->m_cptLock--;

        pClass = pClass->m_ExNext;
    }
}



bool CBotClass::AddItem(CBotString name, CBotTypResult type, int mPrivate)
{
    CBotToken   token(name, CBotString());
    CBotClass*  pClass = type.GetClass();

    CBotVar*    pVar = CBotVar::Create( name, type );
/// pVar->SetUniqNum(CBotVar::NextUniqNum());
    pVar->SetPrivate( mPrivate );

    if ( pClass != nullptr )
    {
//      pVar->SetClass(pClass);
        if ( type.Eq(CBotTypClass) )
        {
            // adds a new statement for the object initialization
            pVar->m_InitExpr = new CBotNew() ;
            CBotToken nom( pClass->GetName() );
            pVar->m_InitExpr->SetToken(&nom);
        }
    }
    return AddItem( pVar );
}


bool CBotClass::AddItem(CBotVar* pVar)
{
    pVar->SetUniqNum(++m_nbVar);

    if ( m_pVar == nullptr ) m_pVar = pVar;
    else m_pVar->AddNext(pVar);

    return true;
}

void CBotClass::AddNext(CBotClass* pClass)
{
    CBotClass*      p = this;
    while (p->m_next != nullptr) p = p->m_next;

    p->m_next = pClass;
}

CBotString  CBotClass::GetName()
{
    return m_name;
}

CBotClass*  CBotClass::GetParent()
{
    if ( this == nullptr ) return nullptr;
    return m_pParent;
}

bool  CBotClass::IsChildOf(CBotClass* pClass)
{
    CBotClass* p = this;
    while ( p != nullptr )
    {
        if ( p == pClass ) return true;
        p = p->m_pParent;
    }
    return false;
}


CBotVar* CBotClass::GetVar()
{
    return  m_pVar;
}

CBotVar* CBotClass::GetItem(const char* name)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetName() == name ) return p;
        p = p->GetNext();
    }
    if ( m_pParent != nullptr ) return m_pParent->GetItem(name);
    return nullptr;
}

CBotVar* CBotClass::GetItemRef(int nIdent)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetUniqNum() == nIdent ) return p;
        p = p->GetNext();
    }
    if ( m_pParent != nullptr ) return m_pParent->GetItemRef(nIdent);
    return nullptr;
}

bool CBotClass::IsIntrinsic()
{
    return  m_bIntrinsic;
}

CBotClass* CBotClass::Find(CBotToken* &pToken)
{
    return Find(pToken->GetString());
}

CBotClass* CBotClass::Find(const char* name)
{
    CBotClass*  p = m_ExClass;

    while ( p != nullptr )
    {
        if ( p->GetName() == name ) return p;
        p = p->m_ExNext;
    }

    return nullptr;
}

bool CBotClass::AddFunction(const char* name,
                                bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                                CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar))
{
    // stores pointers to the two functions
    CBotCallMethode*    p = m_pCalls;
    CBotCallMethode*    pp = nullptr;

    while ( p != nullptr )
    {
        if ( name == p->GetName() )
        {
            if ( pp == nullptr ) m_pCalls = p->m_next;
            else              pp->m_next = p->m_next;
            delete p;
            break;
        }
        pp = p;
        p = p->m_next;
    }

    p = new CBotCallMethode(name, rExec, rCompile);

    if (m_pCalls == nullptr) m_pCalls = p;
    else    m_pCalls->AddNext(p);               // added to the list

    return true;
}

bool CBotClass::AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) )
{
    m_rMaj = rMaj;
    return true;
}

// compiles a method associated with an instance of class
// the method can be declared by the user or AddFunction

CBotTypResult CBotClass::CompileMethode(const char* name,
                                        CBotVar* pThis, CBotVar** ppParams,
                                        CBotCStack* pStack, long& nIdent)
{
    nIdent = 0; // forget the previous one if necessary

    // find the methods declared by AddFunction

    CBotTypResult r = m_pCalls->CompileCall(name, pThis, ppParams, pStack, nIdent);
    if ( r.GetType() >= 0) return r;

    // find the methods declared by user

    r = m_pMethod->CompileCall(name, ppParams, nIdent);
    if ( r.Eq(TX_UNDEFCALL) && m_pParent != nullptr )
        return m_pParent->m_pMethod->CompileCall(name, ppParams, nIdent);
    return r;
}

// executes a method

bool CBotClass::ExecuteMethode(long& nIdent, const char* name,
                               CBotVar* pThis, CBotVar** ppParams,
                               CBotVar* &pResult, CBotStack* &pStack,
                               CBotToken* pToken)
{
    int ret = m_pCalls->DoCall(nIdent, name, pThis, ppParams, pResult, pStack, pToken);
    if (ret>=0) return ret;

    ret = m_pMethod->DoCall(nIdent, name, pThis, ppParams, pStack, pToken, this);
    if (ret >= 0) return ret;

    if (m_pParent != nullptr)
    {
        ret = m_pParent->m_pCalls->DoCall(nIdent, name, pThis, ppParams, pResult, pStack, pToken);
        if (ret >= 0) return ret;
        ret = m_pParent->m_pMethod->DoCall(nIdent, name, pThis, ppParams, pStack, pToken, m_pParent);
    }
    return ret;
}

// restored the execution stack

void CBotClass::RestoreMethode(long& nIdent, const char* name, CBotVar* pThis,
                               CBotVar** ppParams, CBotStack* &pStack)
{
    m_pMethod->RestoreCall(nIdent, name, pThis, ppParams, pStack, this);
}




bool CBotClass::SaveStaticState(FILE* pf)
{
    if (!WriteWord( pf, CBOTVERSION*2)) return false;

    // saves the state of static variables in classes
    CBotClass*  p = m_ExClass;

    while ( p != nullptr )
    {
        if (!WriteWord( pf, 1)) return false;
        // save the name of the class
        if (!WriteString( pf, p->GetName() )) return false;

        CBotVar*    pv = p->GetVar();
        while( pv != nullptr )
        {
            if ( pv->IsStatic() )
            {
                if (!WriteWord( pf, 1)) return false;
                if (!WriteString( pf, pv->GetName() )) return false;

                if ( !pv->Save0State(pf)) return false;             // common header
                if ( !pv->Save1State(pf) ) return false;                // saves as the child class
                if ( !WriteWord( pf, 0)) return false;
            }
            pv = pv->GetNext();
        }

        if (!WriteWord( pf, 0)) return false;
        p = p->m_ExNext;
    }

    if (!WriteWord( pf, 0)) return false;
    return true;
}

bool CBotClass::RestoreStaticState(FILE* pf)
{
    CBotString      ClassName, VarName;
    CBotClass*      pClass;
    unsigned short  w;

    if (!ReadWord( pf, w )) return false;
    if ( w != CBOTVERSION*2 ) return false;

    while (true)
    {
        if (!ReadWord( pf, w )) return false;
        if ( w == 0 ) return true;

        if (!ReadString( pf, ClassName )) return false;
        pClass = Find(ClassName);

        while (true)
        {
            if (!ReadWord( pf, w )) return false;
            if ( w == 0 ) break;

            CBotVar*    pVar = nullptr;
            CBotVar*    pv = nullptr;

            if (!ReadString( pf, VarName )) return false;
            if ( pClass != nullptr ) pVar = pClass->GetItem(VarName);

            if (!CBotVar::RestoreState(pf, pv)) return false;   // the temp variable

            if ( pVar != nullptr ) pVar->Copy(pv);
            delete pv;
        }
    }
    return true;
}

// test if a procedure name is already defined somewhere

bool CBotClass::CheckCall(CBotToken* &pToken, CBotDefParam* pParam)
{
    CBotString  name = pToken->GetString();

    if ( CBotCall::CheckCall(name) ) return true;

    CBotFunction*   pp = m_pMethod;
    while ( pp != nullptr )
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are their parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
        pp = pp->Next();
    }

    return false;
}
