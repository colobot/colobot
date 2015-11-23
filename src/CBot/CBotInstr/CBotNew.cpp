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
#include "CBot/CBotInstr/CBotNew.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include



////////////////////////////////////////////////////////////////////////////////
CBotNew::CBotNew()
{
    name            = "CBotNew";
    m_Parameters    = nullptr;
    m_nMethodeIdent = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotNew::~CBotNew()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotNew::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken* pp = p;
    if (!IsOfType(p, ID_NEW)) return nullptr;

    // verifies that the token is a class name
    if (p->GetType() != TokenTypVar) return nullptr;

    CBotClass* pClass = CBotClass::Find(p);
    if (pClass == nullptr)
    {
        pStack->SetError(TX_BADNEW, p);
        return nullptr;
    }

    CBotNew* inst = new CBotNew();
    inst->SetToken(pp);

    inst->m_vartoken = p;
    p = p->GetNext();

    // creates the object on the "job"
    // with a pointer to the object
    CBotVar*    pVar = CBotVar::Create("", pClass);

    // do the call of the creator
    CBotCStack* pStk = pStack->TokenStack();
    {
        // check if there are parameters
        CBotVar*    ppVars[1000];
        inst->m_Parameters = CompileParams(p, pStk, ppVars);
        if (!pStk->IsOk()) goto error;

        // constructor exist?
        CBotTypResult r = pClass->CompileMethode(pClass->GetName(), pVar, ppVars, pStk, inst->m_nMethodeIdent);
        delete pStk->TokenStack();  // release extra stack
        int typ = r.GetType();

        // if there is no constructor, and no parameters either, it's ok
        if (typ == TX_UNDEFCALL && inst->m_Parameters == nullptr) typ = 0;
        pVar->SetInit(CBotVar::InitType::DEF);    // mark the instance as init

        if (typ>20)
        {
            pStk->SetError(typ, inst->m_vartoken.GetEnd());
            goto error;
        }

        // if the constructor does not exist, but there are parameters
        if (typ<0 && inst->m_Parameters != nullptr)
        {
            pStk->SetError(TX_NOCONST, &inst->m_vartoken);
            goto error;
        }

        // makes pointer to the object on the stack
        pStk->SetVar(pVar);
        return pStack->Return(inst, pStk);
    }
error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotNew::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);    //main stack

    if (pile->IfStep()) return false;

    CBotStack*    pile1 = pj->AddStack2();  //secondary stack

    CBotVar*    pThis = nullptr;

    CBotToken*    pt = &m_vartoken;
    CBotClass*    pClass = CBotClass::Find(pt);

    // create the variable "this" pointer type to the stack

    if ( pile->GetState()==0)
    {
        // create an instance of the requested class
        // and initialize the pointer to that object


        pThis = CBotVar::Create("this", pClass);
        pThis->SetUniqNum(-2) ;

        pile1->SetVar(pThis);   // place on stack1
        pile->IncState();
    }

    // fetch the this pointer if it was interrupted
    if ( pThis == nullptr)
    {
        pThis = pile1->GetVar();    // find the pointer
    }

    // is there an assignment or parameters (constructor)
    if ( pile->GetState()==1)
    {
        // evaluates the constructor of the instance

        CBotVar*    ppVars[1000];
        CBotStack*    pile2 = pile;

        int        i = 0;

        CBotInstr*    p = m_Parameters;
        // evaluate the parameters
        // and places the values on the stack
        // to be interrupted at any time

        if (p != nullptr) while ( true)
        {
            pile2 = pile2->AddStack();  // space on the stack for the result
            if (pile2->GetState() == 0)
            {
                if (!p->Execute(pile2)) return false;   // interrupted here?
                pile2->SetState(1);
            }
            ppVars[i++] = pile2->GetVar();
            p = p->GetNext();
            if ( p == nullptr) break;
        }
        ppVars[i] = nullptr;

        // create a variable for the result
        CBotVar*    pResult = nullptr;     // constructos still void

        if ( !pClass->ExecuteMethode(m_nMethodeIdent, pClass->GetName(),
                                     pThis, ppVars,
                                     pResult, pile2, GetToken())) return false;    // interrupt

        pThis->ConstructorSet();    // indicates that the constructor has been called
    }

    return pj->Return(pile1);   // passes below
}

////////////////////////////////////////////////////////////////////////////////
void CBotNew::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);    //primary stack
    if (pile == nullptr) return;

    CBotStack*    pile1 = pj->AddStack2();  //secondary stack

    CBotToken*    pt = &m_vartoken;
    CBotClass*    pClass = CBotClass::Find(pt);

    // create the variable "this" pointer type to the object

    if ( pile->GetState()==0)
    {
        return;
    }

    CBotVar* pThis = pile1->GetVar();   // find the pointer
    pThis->SetUniqNum(-2);

    // is ther an assignment or parameters (constructor)
    if ( pile->GetState()==1)
    {
        // evaluates the constructor of the instance

        CBotVar*    ppVars[1000];
        CBotStack*    pile2 = pile;

        int        i = 0;

        CBotInstr*    p = m_Parameters;
        // evaluate the parameters
        // and places the values on the stack
        // to be interrupted at any time

        if (p != nullptr) while ( true)
        {
            pile2 = pile2->RestoreStack();  // space on the stack for the result
            if (pile2 == nullptr) return;

            if (pile2->GetState() == 0)
            {
                p->RestoreState(pile2, bMain);  // interrupt here!
                return;
            }
            ppVars[i++] = pile2->GetVar();
            p = p->GetNext();
            if ( p == nullptr) break;
        }
        ppVars[i] = nullptr;

        pClass->RestoreMethode(m_nMethodeIdent, m_vartoken.GetString(), pThis,
                               ppVars, pile2)    ;        // interrupt here!
    }
}
