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
#include "CBot/CBotClass.h"

#include "CBot/CBotInstr/CBotNew.h"
#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotFunction.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotListArray.h"
#include "CBot/CBotInstr/CBotEmpty.h"

#include "CBot/CBotCall.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotUtils.h"
#include "CBot/CBotFileUtils.h"
#include "CBot/CBotCallMethode.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::m_ExClass = nullptr;

////////////////////////////////////////////////////////////////////////////////
CBotClass::CBotClass(const std::string& name,
                     CBotClass* pPapa,
                     bool bIntrinsic)
{
    m_pParent   = pPapa;
    m_name      = name;
    m_pVar      = nullptr;
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

////////////////////////////////////////////////////////////////////////////////
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
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Create(const std::string& name,
                             CBotClass* parent,
                             bool intrinsic)
{
    return new CBotClass(name, parent, intrinsic);
}

////////////////////////////////////////////////////////////////////////////////
void CBotClass::Free()
{
    while ( m_ExClass != nullptr )
    {
        delete m_ExClass;
    }
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::AddItem(std::string name,
                        CBotTypResult type,
                        CBotVar::ProtectionLevel mPrivate)
{
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

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::AddItem(CBotVar* pVar)
{
    pVar->SetUniqNum(++m_nbVar);

    if ( m_pVar == nullptr ) m_pVar = pVar;
    else m_pVar->AddNext(pVar);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
std::string  CBotClass::GetName()
{
    return m_name;
}

////////////////////////////////////////////////////////////////////////////////
CBotClass*  CBotClass::GetParent()
{
    if ( this == nullptr ) return nullptr;
    return m_pParent;
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotClass::GetVar()
{
    return  m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotClass::GetItem(const std::string& name)
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::IsIntrinsic()
{
    return  m_bIntrinsic;
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Find(CBotToken* &pToken)
{
    return Find(pToken->GetString());
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Find(const std::string& name)
{
    CBotClass*  p = m_ExClass;

    while ( p != nullptr )
    {
        if ( p->GetName() == name ) return p;
        p = p->m_ExNext;
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::AddFunction(const std::string& name,
                            bool rExec(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                            CBotTypResult rCompile(CBotVar* pThis, CBotVar*& pVar))
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

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) )
{
    m_rMaj = rMaj;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotClass::CompileMethode(const std::string& name,
                                        CBotVar* pThis,
                                        CBotVar** ppParams,
                                        CBotCStack* pStack,
                                        long& nIdent)
{
    nIdent = 0; // forget the previous one if necessary

    // find the methods declared by AddFunction

    CBotTypResult r = m_pCalls->CompileCall(name, pThis, ppParams, pStack, nIdent);
    if ( r.GetType() >= 0) return r;

    // find the methods declared by user

    r = m_pMethod->CompileCall(name, ppParams, nIdent);
    if ( r.Eq(CBotErrUndefCall) && m_pParent != nullptr )
        return m_pParent->m_pMethod->CompileCall(name, ppParams, nIdent);
    return r;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::ExecuteMethode(long& nIdent,
                               const std::string& name,
                               CBotVar* pThis,
                               CBotVar** ppParams,
                               CBotVar*& pResult,
                               CBotStack*& pStack,
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

////////////////////////////////////////////////////////////////////////////////
void CBotClass::RestoreMethode(long& nIdent,
                               const std::string& name,
                               CBotVar* pThis,
                               CBotVar** ppParams,
                               CBotStack*& pStack)
{
    m_pMethod->RestoreCall(nIdent, name, pThis, ppParams, pStack, this);
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::RestoreStaticState(FILE* pf)
{
    std::string      ClassName, VarName;
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

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::CheckCall(CBotToken* &pToken,
                          CBotDefParam* pParam)
{
    std::string  name = pToken->GetString();

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

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Compile1(CBotToken* &p, CBotCStack* pStack)
{
    if ( !IsOfType(p, ID_PUBLIC) )
    {
        pStack->SetError(CBotErrNoPublic, p);
        return nullptr;
    }

    if ( !IsOfType(p, ID_CLASS) ) return nullptr;

    std::string name = p->GetString();

    CBotClass* pOld = CBotClass::Find(name);
    if ( pOld != nullptr && pOld->m_IsDef )
    {
        pStack->SetError( CBotErrRedefClass, p );
        return nullptr;
    }

    // a name of the class is there?
    if (IsOfType(p, TokenTypVar))
    {
        CBotClass* pPapa = nullptr;
        if ( IsOfType( p, ID_EXTENDS ) )
        {
            std::string name = p->GetString();
            pPapa = CBotClass::Find(name);

            if (!IsOfType(p, TokenTypVar) || pPapa == nullptr )
            {
                pStack->SetError( CBotErrNotClass, p );
                return nullptr;
            }
        }
        CBotClass* classe = (pOld == nullptr) ? new CBotClass(name, pPapa) : pOld;
        classe->Purge();                            // emptythe old definitions
        classe->m_IsDef = false;                    // current definition

        if ( !IsOfType( p, ID_OPBLK) )
        {
            pStack->SetError(CBotErrOpenBlock, p);
            return nullptr;
        }

        while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
        {
            classe->CompileDefItem(p, pStack, false);
        }

        if (pStack->IsOk()) return classe;
    }
    pStack->SetError(CBotErrNoTerminator, p);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::CompileDefItem(CBotToken* &p, CBotCStack* pStack, bool bSecond)
{
    bool    bStatic = false;
    CBotVar::ProtectionLevel mProtect = CBotVar::ProtectionLevel::Public;
    bool    bSynchro = false;

    while (IsOfType(p, ID_SEP)) ;

    CBotTypResult   type( -1 );

    if ( IsOfType(p, ID_SYNCHO) ) bSynchro = true;
    CBotToken*      pBase = p;

    if ( IsOfType(p, ID_STATIC) ) bStatic = true;
    if ( IsOfType(p, ID_PUBLIC) ) mProtect = CBotVar::ProtectionLevel::Public;
    if ( IsOfType(p, ID_PRIVATE) ) mProtect = CBotVar::ProtectionLevel::Private;
    if ( IsOfType(p, ID_PROTECTED) ) mProtect = CBotVar::ProtectionLevel::Protected;
    if ( IsOfType(p, ID_STATIC) ) bStatic = true;

//  CBotClass* pClass = nullptr;
    type = TypeParam(p, pStack);        // type of the result

    if ( type.Eq(-1) )
    {
        pStack->SetError(CBotErrNoType, p);
        return false;
    }

    while (pStack->IsOk())
    {
        std::string pp = p->GetString();
        if ( IsOfType(p, ID_NOT) )
        {
            pp = std::string("~") + p->GetString();
        }

        if (IsOfType(p, TokenTypVar))
        {
            CBotInstr* limites = nullptr;
            while ( IsOfType( p, ID_OPBRK ) )   // a table?
            {
                CBotInstr* i = nullptr;

                if ( p->GetType() != ID_CLBRK )
                    i = CBotExpression::Compile( p, pStack );           // expression for the value
                else
                    i = new CBotEmpty();                            // special if not a formula

                type = CBotTypResult(CBotTypArrayPointer, type);

                if (!pStack->IsOk() || !IsOfType( p, ID_CLBRK ) )
                {
                    pStack->SetError(CBotErrCloseIndex, p->GetStart());
                    return false;
                }

/*              CBotVar* pv = pStack->GetVar();
                if ( pv->GetType()>= CBotTypBoolean )
                {
                    pStack->SetError(CBotErrBadType1, p->GetStart());
                    return false;
                }*/

                if (limites == nullptr) limites = i;
                else limites->AddNext3(i);
            }

            if ( p->GetType() == ID_OPENPAR )
            {
                if ( !bSecond )
                {
                    p = pBase;
                    CBotFunction* f =
                    CBotFunction::Compile1(p, pStack, this);

                    if ( f == nullptr ) return false;

                    if (m_pMethod == nullptr) m_pMethod = f;
                    else m_pMethod->AddNext(f);
                }
                else
                {
                    // return a method precompiled in pass 1
                    CBotFunction*   pf = m_pMethod;
                    CBotFunction*   prev = nullptr;
                    while ( pf != nullptr )
                    {
                        if (pf->GetName() == pp) break;
                        prev = pf;
                        pf = pf->Next();
                    }

                    bool bConstructor = (pp == GetName());
                    CBotCStack* pile = pStack->TokenStack(nullptr, true);

                    // make "this" known
                    CBotToken TokenThis(std::string("this"), std::string());
                    CBotVar* pThis = CBotVar::Create(TokenThis, CBotTypResult( CBotTypClass, this ) );
                    pThis->SetUniqNum(-2);
                    pile->AddVar(pThis);

                    if ( m_pParent )
                    {
                        // makes "super" known
                        CBotToken TokenSuper(std::string("super"), std::string());
                        CBotVar* pThis = CBotVar::Create(TokenSuper, CBotTypResult( CBotTypClass, m_pParent ) );
                        pThis->SetUniqNum(-3);
                        pile->AddVar(pThis);
                    }

//                  int num = 1;
                    CBotClass*  my = this;
                    while (my != nullptr)
                    {
                        // places a copy of variables of a class (this) on a stack
                        CBotVar* pv = my->m_pVar;
                        while (pv != nullptr)
                        {
                            CBotVar* pcopy = CBotVar::Create(pv);
                            CBotVar::InitType initType = CBotVar::InitType::UNDEF;
                            if (!bConstructor || pv->IsStatic())
                                initType = CBotVar::InitType::DEF;
                            pcopy->SetInit(initType);
                            pcopy->SetUniqNum(pv->GetUniqNum());
                            pile->AddVar(pcopy);
                            pv = pv->GetNext();
                        }
                        my = my->m_pParent;
                    }

                    // compiles a method
                    p = pBase;
                    CBotFunction* f =
                    CBotFunction::Compile(p, pile, nullptr/*, false*/);

                    if ( f != nullptr )
                    {
                        f->m_pProg = pStack->GetProgram();
                        f->m_bSynchro = bSynchro;
                        // replaces the element in the chain
                        f->m_next = pf->m_next;
                        pf->m_next = nullptr;
                        delete pf;
                        if (prev == nullptr) m_pMethod = f;
                        else prev->m_next = f;
                    }
                    pStack->Return(nullptr, pile);
                }

                return pStack->IsOk();
            }

            // definition of an element
            if (type.Eq(0))
            {
                pStack->SetError(CBotErrNoTerminator, p);
                return false;
            }

            CBotInstr* i = nullptr;
            if ( IsOfType(p, ID_ASS ) )
            {
                if ( type.Eq(CBotTypArrayPointer) )
                {
                    i = CBotListArray::Compile(p, pStack, type.GetTypElem());
                }
                else
                {
                    // it has an assignmet to calculate
                    i = CBotTwoOpExpr::Compile(p, pStack);
                }
                if ( !pStack->IsOk() ) return false;
            }


            if ( !bSecond )
            {
                CBotVar*    pv = CBotVar::Create(pp, type);
                pv -> SetStatic( bStatic );
                pv -> SetPrivate( mProtect );

                AddItem( pv );

                pv->m_InitExpr = i;
                pv->m_LimExpr = limites;


                if ( pv->IsStatic() && pv->m_InitExpr != nullptr )
                {
                    CBotStack* pile = CBotStack::FirstStack();              // independent stack
                    while(pile->IsOk() && !pv->m_InitExpr->Execute(pile));  // evaluates the expression without timer
                    pv->SetVal( pile->GetVar() ) ;
                    pile->Delete();
                }
            }
            else
                delete i;

            if ( IsOfType(p, ID_COMMA) ) continue;
            if ( IsOfType(p, ID_SEP) ) break;
        }
        pStack->SetError(CBotErrNoTerminator, p);
    }
    return pStack->IsOk();
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Compile(CBotToken* &p, CBotCStack* pStack)
{
    if ( !IsOfType(p, ID_PUBLIC) ) return nullptr;
    if ( !IsOfType(p, ID_CLASS) ) return nullptr;

    std::string name = p->GetString();

    // a name for the class is there?
    if (IsOfType(p, TokenTypVar))
    {
        // the class was created by Compile1
        CBotClass* pOld = CBotClass::Find(name);

        if ( IsOfType( p, ID_EXTENDS ) )
        {
            // TODO: Not sure how correct is that - I have no idea how the precompilation (Compile1 method) works ~krzys_h
            std::string name = p->GetString();
            CBotClass* pPapa = CBotClass::Find(name);

            if (!IsOfType(p, TokenTypVar) || pPapa == nullptr)
            {
                pStack->SetError( CBotErrNotClass, p );
                return nullptr;
            }
            pOld->m_pParent = pPapa;
        }
        else
        {
            if (pOld != nullptr)
            {
                pOld->m_pParent = nullptr;
            }
        }
        IsOfType( p, ID_OPBLK); // necessarily

        while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
        {
            pOld->CompileDefItem(p, pStack, true);
        }

        pOld->m_IsDef = true;           // complete definition
        if (pStack->IsOk()) return pOld;
    }
    pStack->SetError(CBotErrNoTerminator, p);
    return nullptr;
}
