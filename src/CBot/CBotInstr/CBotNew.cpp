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

#include <sstream>
#include "CBot/CBotInstr/CBotNew.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotInstr/CBotExprRetVar.h"
#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotNew::CBotNew()
{
    m_parameters = nullptr;
    m_exprRetVar = nullptr;
    m_nMethodeIdent = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotNew::~CBotNew()
{
    delete m_parameters;
    delete m_exprRetVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotNew::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken* pp = p;
    if (!IsOfType(p, ID_NEW)) return nullptr;

    // verifies that the token is a class name
    if (p->GetType() != TokenTypVar)
    {
        pStack->SetError(CBotErrBadNew, p);
        return nullptr;
    }

    CBotClass* pClass = CBotClass::Find(p);
    if (pClass == nullptr)
    {
        pStack->SetError(CBotErrBadNew, p);
        return nullptr;
    }

    CBotNew* inst = new CBotNew();
    inst->SetToken(pp);

    inst->m_vartoken = *p;
    p = p->GetNext();

    // creates the object on the stack
    // with a pointer to the object
    CBotVar*    pVar = CBotVar::Create("", pClass);

    // do the call of the creator
    CBotCStack* pStk = pStack->TokenStack();
    {
        // check if there are parameters
        CBotVar*    ppVars[1000];
        inst->m_parameters = CompileParams(p, pStk, ppVars);
        if (!pStk->IsOk()) goto error;

        // constructor exist?
        CBotTypResult r = pClass->CompileMethode(&inst->m_vartoken, pVar, ppVars, pStk, inst->m_nMethodeIdent);
        delete pStk->TokenStack();  // release extra stack
        int typ = r.GetType();

        // if there is no constructor, and no parameters either, it's ok
        if (typ == CBotErrUndefCall && inst->m_parameters == nullptr) typ = 0;
        pVar->SetInit(CBotVar::InitType::DEF);    // mark the instance as init

        if (typ>20)
        {
            pStk->SetError(static_cast<CBotError>(typ), inst->m_vartoken.GetEnd());
            goto error;
        }

        // if the constructor does not exist, but there are parameters
        if (typ<0 && inst->m_parameters != nullptr)
        {
            pStk->SetError(CBotErrNoConstruct, &inst->m_vartoken);
            goto error;
        }

        // makes pointer to the object on the stack
        pStk->SetVar(pVar);

        pp = p;
        // chained method ?
        if (nullptr != (inst->m_exprRetVar = CBotExprRetVar::Compile(p, pStk, true)))
        {
            inst->m_exprRetVar->SetToken(pp);
            delete pStk->TokenStack();
        }

        if (pStack->IsOk())
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

    if (m_exprRetVar != nullptr) // new Class().method()
    {
        if (pile->GetState() == 2)
        {
            CBotStack* pile3 = pile->AddStack();
            if (!m_exprRetVar->Execute(pile3)) return false;
            return pj->Return(pile3);
        }
    }

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

        CBotInstr*    p = m_parameters;
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

        if ( !pClass->ExecuteMethode(m_nMethodeIdent, pThis, ppVars, CBotTypResult(CBotTypVoid), pile2, &m_vartoken)) return false;    // interrupt

        pThis->ConstructorSet();    // indicates that the constructor has been called
    }

    if (m_exprRetVar != nullptr) // new Class().method()
    {
        pile->AddStack()->Delete();          // release pile2 stack
        CBotStack* pile3 = pile->AddStack(); // add new stack
        pile3->SetCopyVar(pThis); // copy the pointer (from pile1)
        pile1->Delete();          // release secondary stack(pile1)
        pile->SetState(2);
        return false;             // go back to the top ^^^
    }

    return pj->Return(pile1);   // passes below
}

////////////////////////////////////////////////////////////////////////////////
void CBotNew::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);    //primary stack
    if (pile == nullptr) return;

    if (m_exprRetVar != nullptr)    // new Class().method()
    {
        if (pile->GetState() == 2)
        {
            CBotStack* pile3 = pile->RestoreStack();
            m_exprRetVar->RestoreState(pile3, bMain);
            return;
        }
    }

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

        CBotInstr*    p = m_parameters;
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

        pClass->RestoreMethode(m_nMethodeIdent, &m_vartoken, pThis, ppVars, pile2);        // interrupt here!
    }
}

std::string CBotNew::GetDebugData()
{
    std::stringstream ss;
    ss << "ConstructorID = " << m_nMethodeIdent;
    return ss.str();
}

std::map<std::string, CBotInstr*> CBotNew::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_parameters"] = m_parameters;
    return links;
}

} // namespace CBot
