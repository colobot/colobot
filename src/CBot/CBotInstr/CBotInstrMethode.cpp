/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include "CBot/CBotInstr/CBotInstrMethode.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVar.h"
#include "CBot/CBotVar/CBotVarClass.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotInstrMethode::CBotInstrMethode()
{
    m_parameters = nullptr;
    m_MethodeIdent = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstrMethode::~CBotInstrMethode()
{
    delete m_parameters;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstrMethode::Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* var)
{
    CBotInstrMethode* inst = new CBotInstrMethode();
    inst->SetToken(p);  // corresponding token

    CBotToken*    pp = p;
    p = p->GetNext();

    // Allow method names to start with '~' in order to allow calling parent
    // destructors from a child class's destructor.
    std::string name = pp->GetString();
    if ( IsOfType(pp, ID_NOT) )
    {
        name = std::string("~") + p->GetString();
        p = p->GetNext();
    }

    if (p->GetType() == ID_OPENPAR)
    {
        inst->m_methodName = name;

        // compiles the list of parameters
        CBotVar*    ppVars[1000];
        inst->m_parameters = CompileParams(p, pStack, ppVars);

        if (pStack->IsOk())
        {
            CBotClass* pClass = var->GetClass();    // pointer to the class
            inst->m_className = pClass->GetName();  // name of the class
            CBotTypResult r = pClass->CompileMethode(inst->m_methodName, var, ppVars,
                                                     pStack, inst->m_MethodeIdent);
            delete pStack->TokenStack();    // release parameters on the stack
            inst->m_typRes = r;

            if (inst->m_typRes.GetType() > 20)
            {
                pStack->SetError(static_cast<CBotError>(inst->m_typRes.GetType()), pp);
                delete    inst;
                return    nullptr;
            }
            // put the result on the stack to have something
            if (inst->m_typRes.GetType() > 0)
            {
                CBotVar*    pResult = CBotVar::Create("", inst->m_typRes);
                if (inst->m_typRes.Eq(CBotTypClass))
                {
                    pResult->SetClass(inst->m_typRes.GetClass());
                }
                pStack->SetVar(pResult);
            }
            return inst;
        }
        delete inst;
        return nullptr;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstrMethode::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pj->AddStack(this, CBotStack::BlockVisibilityType::BLOCK);     // a place for the copy of This

    if (pVar->GetPointer() == nullptr)
    {
        pj->SetError(CBotErrNull, prevToken);
        return pj->Return(pile1);
    }

    if (pile1->IfStep()) return false;

    CBotStack*    pile2 = pile1->AddStack();    // for the next parameters

    if ( pile1->GetState() == 0)
    {
        CBotVar*    pThis = CBotVar::Create(pVar);
        pThis->Copy(pVar);
        // this value should be taken before the evaluation parameters
        // Test.Action (Test = Other);
        // action must act on the value before test = Other!

        pThis->SetName("this");
        pThis->SetUniqNum(-2);
        pile1->SetVar(pThis);
        pile1->IncState();
    }
    int        i = 0;

    CBotInstr*    p = m_parameters;
    // evaluate the parameters
    // and places the values on the stack
    // to be interrupted at any time

    if (p != nullptr) while ( true)
    {
        if (pile2->GetState() == 0)
        {
            if (!p->Execute(pile2)) return false;   // interrupted here?
            if (!pile2->SetState(1)) return false;  // special mark to recognize parameters
        }
        ppVars[i++] = pile2->GetVar();              // construct the list of pointers
        pile2 = pile2->AddStack();                  // space on the stack for the result
        p = p->GetNext();
        if ( p == nullptr) break;
    }
    ppVars[i] = nullptr;

    CBotVar*    pThis  = pile1->GetVar();
    CBotVar*    pResult = nullptr;
    CBotClass*  pClass;
    if (pThis->GetPointer() != nullptr)
    {
        pClass = pThis->GetPointer()->GetClass();
    }
    else
    {
        pClass = pThis->GetClass();
    }
    if (m_typRes.GetType() > 0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
        pResult->SetClass(m_typRes.GetClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_methodName,
                                 pThis, ppVars,
                                 pResult, pile2, GetToken())) return false;
    if (pRes != pResult) delete pRes;

    pVar = nullptr;                // does not return value for this
    return pj->Return(pile2);   // release the entire stack
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstrMethode::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    if (!bMain) return;

    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pile->RestoreStack(this);     // place for the copy of This
    if (pile1 == nullptr) return;

    CBotStack*    pile2 = pile1->RestoreStack();        // and for the parameters coming
    if (pile2 == nullptr) return;

    CBotVar*    pThis  = pile1->GetVar();

    assert(pThis != nullptr); // see fix for issues #256 & #436

    pThis->SetUniqNum(-2);

    int        i = 0;

    CBotInstr*    p = m_parameters;
    // evaluate the parameters
    // and places the values on the stack
    // to be interrupted at any time

    if (p != nullptr) while ( true)
    {
        if (pile2->GetState() == 0)
        {
            p->RestoreState(pile2, true);   // interrupted here!
            return;
        }
        ppVars[i++] = pile2->GetVar();      // construct the list of pointers
        pile2 = pile2->RestoreStack();
        if (pile2 == nullptr) return;

        p = p->GetNext();
        if ( p == nullptr) break;
    }
    ppVars[i] = nullptr;

    CBotClass*    pClass = CBotClass::Find(m_className);
//    CBotVar*    pResult = nullptr;

//    CBotVar*    pRes = pResult;

    pClass->RestoreMethode(m_MethodeIdent, m_methodName,
                           pThis, ppVars, pile2);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstrMethode::Execute(CBotStack* &pj)
{
    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pj->AddStack(this, CBotStack::BlockVisibilityType::BLOCK);        // place for the copy of This

    if (pile1->IfStep()) return false;

    CBotStack*    pile2 = pile1->AddStack();                // and for the parameters coming

    if ( pile1->GetState() == 0)
    {
        CBotVar*    pThis = pile1->CopyVar(m_token);
        // this value should be taken before the evaluation parameters
        // Test.Action (Test = Other);
        // Action must act on the value before test = Other!
        pThis->SetName("this");
        pile1->SetVar(pThis);
        pile1->IncState();
    }
    int        i = 0;

    CBotInstr*    p = m_parameters;
    // evaluate the parameters
    // and places the values on the stack
    // to be interrupted at any time
    if (p != nullptr) while ( true)
    {
        if (pile2->GetState() == 0)
        {
            if (!p->Execute(pile2)) return false;   // interrupted here?
            if (!pile2->SetState(1)) return false;  // special mark to recognize parameters
        }
        ppVars[i++] = pile2->GetVar();              // construct the list of pointers
        pile2 = pile2->AddStack();                  // space on the stack for the results
        p = p->GetNext();
        if ( p == nullptr) break;
    }
    ppVars[i] = nullptr;

    CBotVar*    pThis  = pile1->GetVar();
    CBotVar*    pResult = nullptr;
    CBotClass*  pClass;
    if (pThis->GetPointer() != nullptr)
    {
        pClass = pThis->GetPointer()->GetClass();
    }
    else
    {
        pClass = pThis->GetClass();
    }
    if (m_typRes.GetType()>0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
        pResult->SetClass(m_typRes.GetClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_methodName,
                                 pThis, ppVars,
                                 pResult, pile2, GetToken())) return false;    // interupted

    // set the new value of this in place of the old variable
    CBotVar*    old = pile1->FindVar(m_token, false);
    old->Copy(pThis, false);

    if (pRes != pResult) delete pRes;

    return pj->Return(pile2);    // release the entire stack
}

std::string CBotInstrMethode::GetDebugData()
{
    std::stringstream ss;
    ss << m_methodName << std::endl;
    ss << "MethodID = " << m_MethodeIdent << std::endl;
    ss << "result = " << m_typRes.ToString();
    return ss.str();
}

std::map<std::string, CBotInstr*> CBotInstrMethode::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_parameters"] = m_parameters;
    return links;
}

} // namespace CBot
