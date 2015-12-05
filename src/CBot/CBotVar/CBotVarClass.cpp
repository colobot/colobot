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
#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotDefines.h"

#include "CBot/CBotFileUtils.h"

#include "CBot/CBotInstr/CBotInstr.h"

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

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::Copy(CBotVar* pSrc, bool bName)
{
    pSrc = pSrc->GetPointer();                    // if source given by a pointer

    if ( pSrc->GetType() != CBotTypClass )
        assert(0);

    CBotVarClass*    p = static_cast<CBotVarClass*>(pSrc);

    if (bName)    *m_token    = *p->m_token;

    m_type        = p->m_type;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pClass    = p->m_pClass;
    if ( p->m_pParent )
    {
        assert(0);       // "que faire du pParent";
    }

//    m_next        = nullptr;
    m_pUserPtr    = p->m_pUserPtr;
    m_pMyThis    = nullptr;//p->m_pMyThis;
    m_ItemIdent = p->m_ItemIdent;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    delete        m_pVar;
    m_pVar        = nullptr;

    CBotVar*    pv = p->m_pVar;
    while( pv != nullptr )
    {
        CBotVar*    pn = CBotVar::Create(pv);
        pn->Copy( pv );
        if ( m_pVar == nullptr ) m_pVar = pn;
        else m_pVar->AddNext(pn);

        pv = pv->GetNext();
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::SetItemList(CBotVar* pVar)
{
    delete    m_pVar;
    m_pVar    = pVar;    // replaces the existing pointer
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::SetIdent(long n)
{
    m_ItemIdent = n;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::SetClass(CBotClass* pClass)//, int &nIdent)
{
    m_type.m_pClass = pClass;

    if ( m_pClass == pClass ) return;

    m_pClass = pClass;

    // initializes the variables associated with this class
    delete m_pVar;
    m_pVar = nullptr;

    if (pClass == nullptr) return;

    CBotVar*    pv = pClass->GetVar();                // first on a list
    while ( pv != nullptr )
    {
        // seeks the maximum dimensions of the table
        CBotInstr*    p  = pv->m_LimExpr;                            // the different formulas
        if ( p != nullptr )
        {
            CBotStack* pile = CBotStack::FirstStack();    // an independent stack
            int     n = 0;
            int     max[100];

            while (p != nullptr)
            {
                while( pile->IsOk() && !p->Execute(pile) ) ;        // calculate size without interruptions
                CBotVar*    v = pile->GetVar();                        // result
                max[n] = v->GetValInt();                            // value
                n++;
                p = p->GetNext3();
            }
            while (n<100) max[n++] = 0;

            pv->m_type.SetArray( max );                    // stores the limitations
            pile->Delete();
        }

        CBotVar*    pn = CBotVar::Create( pv );        // a copy
        pn->SetStatic(pv->IsStatic());
        pn->SetPrivate(pv->GetPrivate());

        if ( pv->m_InitExpr != nullptr )                // expression for initialization?
        {
#if    STACKMEM
            CBotStack* pile = CBotStack::FirstStack();    // an independent stack

            while(pile->IsOk() && !pv->m_InitExpr->Execute(pile, pn));    // evaluates the expression without timer

            pile->Delete();
#else
            CBotStack* pile = new CBotStack(nullptr);     // an independent stack
            while(!pv->m_InitExpr->Execute(pile));    // evaluates the expression without timer
            pn->SetVal( pile->GetVar() ) ;
            delete pile;
#endif
        }

//        pn->SetUniqNum(CBotVar::NextUniqNum());        // enumerate elements
        pn->SetUniqNum(pv->GetUniqNum());    //++nIdent
        pn->m_pMyThis = this;

        if ( m_pVar == nullptr) m_pVar = pn;
        else m_pVar->AddNext( pn );
        pv = pv->GetNext();
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVarClass::GetClass()
{
    return    m_pClass;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::Maj(void* pUser, bool bContinu)
{
/*    if (!bContinu && m_pMyThis != nullptr)
        m_pMyThis->Maj(pUser, true);*/

    // an update routine exist?

    if ( m_pClass->m_rMaj == nullptr ) return;

    // retrieves the user pointer according to the class
    // or according to the parameter passed to CBotProgram::Run()

    if ( m_pUserPtr != nullptr) pUser = m_pUserPtr;
    if ( pUser == OBJECTDELETED ||
         pUser == OBJECTCREATED ) return;
    m_pClass->m_rMaj( this, pUser );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarClass::GetItem(const char* name)
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
CBotVar* CBotVarClass::GetItemRef(int nIdent)
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
CBotVar* CBotVarClass::GetItem(int n, bool bExtend)
{
    CBotVar*    p = m_pVar;

    if ( n < 0 ) return nullptr;
    if ( n > MAXARRAYSIZE ) return nullptr;

    if ( m_type.GetLimite() >= 0 && n >= m_type.GetLimite() ) return nullptr;

    if ( p == nullptr && bExtend )
    {
        p = CBotVar::Create("", m_type.GetTypElem());
        m_pVar = p;
    }

    if ( n == 0 ) return p;

    while ( n-- > 0 )
    {
        if ( p->m_next == nullptr )
        {
            if ( bExtend ) p->m_next = CBotVar::Create("", m_type.GetTypElem());
            if ( p->m_next == nullptr ) return nullptr;
        }
        p = p->m_next;
    }

    return p;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarClass::GetItemList()
{
    return m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotString CBotVarClass::GetValString()
{
//    if ( m_Indirect != nullptr) return m_Indirect->GetValString();

    CBotString    res;

    if ( m_pClass != nullptr )                        // not used for an array
    {
        res = m_pClass->GetName() + CBotString("( ");

        CBotVarClass*    my = this;
        while ( my != nullptr )
        {
            CBotVar*    pv = my->m_pVar;
            while ( pv != nullptr )
            {
                res += pv->GetName() + CBotString("=");

                if ( pv->IsStatic() )
                {
                    CBotVar* pvv = my->m_pClass->GetItem(pv->GetName());
                    res += pvv->GetValString();
                }
                else
                {
                    res += pv->GetValString();
                }
                pv = pv->GetNext();
                if ( pv != nullptr ) res += ", ";
            }
            my = my->m_pParent;
            if ( my != nullptr )
            {
                res += ") extends ";
                res += my->m_pClass->GetName();
                res += " (";
            }
        }
    }
    else
    {
        res = "( ";

        CBotVar*    pv = m_pVar;
        while ( pv != nullptr )
        {
            res += pv->GetValString();
            if ( pv->GetNext() != nullptr ) res += ", ";
            pv = pv->GetNext();
        }
    }

    res += " )";
    return    res;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::IncrementUse()
{
    m_CptUse++;
}

////////////////////////////////////////////////////////////////////////////////
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
            CBotStack*    pile = nullptr;
            err = pile->GetError(start,end);    // stack == nullptr it does not bother!

            pile = CBotStack::FirstStack();        // clears the error
            CBotVar*    ppVars[1];
            ppVars[0] = nullptr;

            CBotVar*    pThis  = CBotVar::Create("this", CBotTypNullPointer);
            pThis->SetPointer(this);
            CBotVar*    pResult = nullptr;

            CBotString    nom = CBotString("~") + m_pClass->GetName();
            long        ident = 0;

            while ( pile->IsOk() && !m_pClass->ExecuteMethode(ident, nom, pThis, ppVars, pResult, pile, nullptr)) ;    // waits for the end

            pile->ResetError(err, start,end);

            pile->Delete();
            delete pThis;
            m_CptUse--;
        }

        delete this; // self-destructs!
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarClass::GetPointer()
{
    return this;
}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass* CBotVarClass::Find(long id)
{
    CBotVarClass*    p = m_ExClass;

    while ( p != nullptr )
    {
        if ( p->m_ItemIdent == id ) return p;
        p = p->m_ExNext;
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Eq(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GetItemList();
    CBotVar*    r = right->GetItemList();

    while ( l != nullptr && r != nullptr )
    {
        if ( l->Ne(l, r) ) return false;
        l = l->GetNext();
        r = r->GetNext();
    }

    // should always arrived simultaneously at the end (same classes)
    return l == r;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Ne(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GetItemList();
    CBotVar*    r = right->GetItemList();

    while ( l != nullptr && r != nullptr )
    {
        if ( l->Ne(l, r) ) return true;
        l = l->GetNext();
        r = r->GetNext();
    }

    // should always arrived simultaneously at the end (same classes)
    return l != r;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return false;
    if ( !WriteLong(pf, m_ItemIdent) ) return false;

    return SaveVar(pf, m_pVar);                                // content of the object
}
