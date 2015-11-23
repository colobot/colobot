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
#include "CBotInstrCall.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotInstrCall::CBotInstrCall()
{
    m_Parameters = nullptr;
    m_nFuncIdent = 0;
    name = "CBotInstrCall";
}

////////////////////////////////////////////////////////////////////////////////
CBotInstrCall::~CBotInstrCall()
{
    delete  m_Parameters;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstrCall::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotVar*    ppVars[1000];

    int         i = 0;

    CBotToken*  pp = p;
    p = p->GetNext();

    pStack->SetStartError(p->GetStart());
    CBotCStack* pile = pStack;

    if ( IsOfType(p, ID_OPENPAR) )
    {
        int start, end;
        CBotInstrCall* inst = new CBotInstrCall();
        inst->SetToken(pp);

        // compile la list of parameters
        if (!IsOfType(p, ID_CLOSEPAR)) while (true)
        {
            start = p->GetStart();
            pile = pile->TokenStack();                      // keeps the results on the stack

            CBotInstr*  param = CBotExpression::Compile(p, pile);
            end   = p->GetStart();
            if ( inst->m_Parameters == nullptr ) inst->m_Parameters = param;
            else inst->m_Parameters->AddNext(param);            // constructs the list

            if ( !pile->IsOk() )
            {
                delete inst;
                return pStack->Return(nullptr, pile);
            }

            if ( param != nullptr )
            {
                if ( pile->GetTypResult().Eq(99) )
                {
                    delete pStack->TokenStack();
                    pStack->SetError(TX_VOID, p->GetStart());
                    delete inst;
                    return nullptr;
                }
                ppVars[i] = pile->GetVar();
                ppVars[i]->GetToken()->SetPos(start, end);
                i++;

                if (IsOfType(p, ID_COMMA)) continue;            // skips the comma
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(TX_CLOSEPAR, p->GetStart());
            delete pStack->TokenStack();
            delete inst;
            return nullptr;
        }
        ppVars[i] = nullptr;

        // the routine is known?
//      CBotClass*  pClass = nullptr;
        inst->m_typRes = pStack->CompileCall(pp, ppVars, inst->m_nFuncIdent);
        if ( inst->m_typRes.GetType() >= 20 )
        {
//          if (pVar2!=nullptr) pp = pVar2->RetToken();
            pStack->SetError( inst->m_typRes.GetType(), pp );
            delete pStack->TokenStack();
            delete inst;
            return nullptr;
        }

        delete pStack->TokenStack();
        if ( inst->m_typRes.GetType() > 0 )
        {
            CBotVar* pRes = CBotVar::Create("", inst->m_typRes);
            pStack->SetVar(pRes);   // for knowing the type of the result
        }
        else pStack->SetVar(nullptr);          // routine returns void

        return inst;
    }
    p = pp;
    delete pStack->TokenStack();
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstrCall::Execute(CBotStack* &pj)
{
    CBotVar*    ppVars[1000];
    CBotStack*  pile  = pj->AddStack(this);
    if ( pile->StackOver() ) return pj->Return( pile );

//    CBotStack*  pile1 = pile;

    int     i = 0;

    CBotInstr*  p = m_Parameters;
    // evaluates parameters
    // and places the values ​​on the stack
    // for allow of interruption at any time
    if ( p != nullptr) while ( true )
    {
        pile = pile->AddStack();                        // place on the stack for the results
        if ( pile->GetState() == 0 )
        {
            if (!p->Execute(pile)) return false;        // interrupted here?
            pile->SetState(1);                          // mark as special for reknowed parameters \TODO marque spéciale pour reconnaîre parameters
        }
        ppVars[i++] = pile->GetVar();
        p = p->GetNext();
        if ( p == nullptr) break;
    }
    ppVars[i] = nullptr;

    CBotStack* pile2 = pile->AddStack();
    if ( pile2->IfStep() ) return false;

    if ( !pile2->ExecuteCall(m_nFuncIdent, GetToken(), ppVars, m_typRes)) return false; // interrupt

    return pj->Return(pile2);   // release the entire stack
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstrCall::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile  = pj->RestoreStack(this);
    if ( pile == nullptr ) return;

//    CBotStack*  pile1 = pile;

    int         i = 0;
    CBotVar*    ppVars[1000];
    CBotInstr*  p = m_Parameters;
    // evaluate parameters
    // and place the values on the stack
    // for allow of interruption at any time
    if ( p != nullptr) while ( true )
    {
        pile = pile->RestoreStack();                        // place on the stack for the results
        if ( pile == nullptr ) return;
        if ( pile->GetState() == 0 )
        {
            p->RestoreState(pile, bMain);                   // interrupt here!
            return;
        }
        ppVars[i++] = pile->GetVar();               // constructs the list of parameters
        p = p->GetNext();
        if ( p == nullptr) break;
    }
    ppVars[i] = nullptr;

    CBotStack* pile2 = pile->RestoreStack();
    if ( pile2 == nullptr ) return;

    pile2->RestoreCall(m_nFuncIdent, GetToken(), ppVars);
}
