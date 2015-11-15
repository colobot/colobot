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
#include "CBotInstrMethode.h"

#include "CBotStack.h"
#include "CBotClass.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotInstrMethode::CBotInstrMethode()
{
    m_Parameters    = nullptr;
    m_MethodeIdent  = 0;
    name = "CBotInstrMethode";
}

////////////////////////////////////////////////////////////////////////////////
CBotInstrMethode::~CBotInstrMethode()
{
    delete    m_Parameters;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstrMethode::Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* var)
{
    CBotInstrMethode* inst = new CBotInstrMethode();
    inst->SetToken(p);  // corresponding token

    if (nullptr != var)
    {
        CBotToken*    pp = p;
        p = p->GetNext();

        if (p->GetType() == ID_OPENPAR)
        {
            inst->m_NomMethod = pp->GetString();

            // compiles the list of parameters
            CBotVar*    ppVars[1000];
            inst->m_Parameters = CompileParams(p, pStack, ppVars);

            if (pStack->IsOk())
            {
                CBotClass* pClass = var->GetClass();    // pointer to the class
                inst->m_ClassName = pClass->GetName();  // name of the class
                CBotTypResult r = pClass->CompileMethode(inst->m_NomMethod, var, ppVars,
                                                         pStack, inst->m_MethodeIdent);
                delete pStack->TokenStack();    // release parameters on the stack
                inst->m_typRes = r;

                if (inst->m_typRes.GetType() > 20)
                {
                    pStack->SetError(inst->m_typRes.GetType(), pp);
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
    }
    pStack->SetError(1234, p);
    delete inst;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstrMethode::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pj->AddStack(this, true);     // a place for the copy of This

    if (pVar->GetPointer() == nullptr)
    {
        pj->SetError(TX_NULLPT, prevToken);
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
        pile1->AddVar(pThis);
        pile1->IncState();
    }
    int        i = 0;

    CBotInstr*    p = m_Parameters;
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

    CBotClass*    pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pThis  = pile1->FindVar(-2);
    CBotVar*    pResult = nullptr;
    if (m_typRes.GetType() > 0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
        pResult->SetClass(m_typRes.GetClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_NomMethod,
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

    CBotVar*    pThis  = pile1->FindVar("this");
    pThis->SetUniqNum(-2);

    int        i = 0;

    CBotInstr*    p = m_Parameters;
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

    CBotClass*    pClass = CBotClass::Find(m_ClassName);
//    CBotVar*    pResult = nullptr;

//    CBotVar*    pRes = pResult;

    pClass->RestoreMethode(m_MethodeIdent, m_NomMethod,
                                 pThis, ppVars, pile2);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstrMethode::Execute(CBotStack* &pj)
{
    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pj->AddStack(this, true);        // place for the copy of This

    if (pile1->IfStep()) return false;

    CBotStack*    pile2 = pile1->AddStack();                // and for the parameters coming

    if ( pile1->GetState() == 0)
    {
        CBotVar*    pThis = pile1->CopyVar(m_token);
        // this value should be taken before the evaluation parameters
        // Test.Action (Test = Other);
        // Action must act on the value before test = Other!
        pThis->SetName("this");
        pile1->AddVar(pThis);
        pile1->IncState();
    }
    int        i = 0;

    CBotInstr*    p = m_Parameters;
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

    CBotClass*    pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pThis  = pile1->FindVar("this");
    CBotVar*    pResult = nullptr;
    if (m_typRes.GetType()>0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
        pResult->SetClass(m_typRes.GetClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_NomMethod,
                                 pThis, ppVars,
                                 pResult, pile2, GetToken())) return false;    // interupted

    // set the new value of this in place of the old variable
    CBotVar*    old = pile1->FindVar(m_token);
    old->Copy(pThis, false);

    if (pRes != pResult) delete pRes;

    return pj->Return(pile2);    // release the entire stack
}
