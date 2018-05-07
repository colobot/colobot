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

#include "CBot/CBotInstr/CBotInstrCall.h"
#include "CBot/CBotInstr/CBotExprRetVar.h"
#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotStack.h"

#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotInstrCall::CBotInstrCall()
{
    m_parameters = nullptr;
    m_exprRetVar = nullptr;
    m_nFuncIdent = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstrCall::~CBotInstrCall()
{
    delete m_parameters;
    delete m_exprRetVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstrCall::Compile(CBotToken* &p, CBotCStack* pStack)
{

    CBotToken*  pp = p;
    p = p->GetNext();

    if (p->GetType() == ID_OPENPAR)
    {

        CBotVar*    ppVars[1000];

        CBotInstrCall* inst = new CBotInstrCall();
        inst->SetToken(pp);

        // compile la list of parameters
        inst->m_parameters = CompileParams(p, pStack, ppVars);

        if ( !pStack->IsOk() )
        {
            delete inst;
            return nullptr;
        }

        // the routine is known?
//      CBotClass*  pClass = nullptr;
        inst->m_typRes = pStack->CompileCall(pp, ppVars, inst->m_nFuncIdent);
        if ( inst->m_typRes.GetType() >= 20 )
        {
//          if (pVar2!=nullptr) pp = pVar2->RetToken();
            pStack->SetError( static_cast<CBotError>(inst->m_typRes.GetType()), pp );
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

        if (nullptr != (inst->m_exprRetVar = CBotExprRetVar::Compile(p, pStack)))
        {
            inst->m_exprRetVar->SetToken(&inst->m_token);
            delete pStack->TokenStack();
        }
        if ( !pStack->IsOk() )
        {
            delete inst;
            return nullptr;
        }

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

    CBotStack* pile3 = nullptr;
    if (m_exprRetVar != nullptr) // func().member
    {
        pile3 = pile->AddStack2();
        if (pile3->GetState() == 1) // function call is done?
        {
            if (!m_exprRetVar->Execute(pile3)) return false;
            return pj->Return(pile3);
        }
    }

//    CBotStack*  pile1 = pile;

    int     i = 0;

    CBotInstr*  p = m_parameters;
    // evaluates parameters
    // and places the values ​​on the stack
    // for allow of interruption at any time
    if ( p != nullptr) while ( true )
    {
        pile = pile->AddStack();                        // place on the stack for the results
        if ( pile->GetState() == 0 )
        {
            if (!p->Execute(pile)) return false;        // interrupted here?
            pile->SetState(1);                          // set state to remember that parameters were executed
        }
        ppVars[i++] = pile->GetVar();
        p = p->GetNext();
        if ( p == nullptr) break;
    }
    ppVars[i] = nullptr;

    CBotStack* pile2 = pile->AddStack();
    if ( pile2->IfStep() ) return false;

    if ( !pile2->ExecuteCall(m_nFuncIdent, GetToken(), ppVars, m_typRes)) return false; // interrupt

    if (m_exprRetVar != nullptr) // func().member
    {
        pile3->SetCopyVar( pile2->GetVar() ); // copy the result
        pile2->SetVar(nullptr);
        pile3->SetState(1);      // set call is done
        return false;            // go back to the top ^^^
    }

    return pj->Return(pile2);   // release the entire stack
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstrCall::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile  = pj->RestoreStack(this);
    if ( pile == nullptr ) return;

    if (m_exprRetVar != nullptr)    // func().member
    {
        CBotStack* pile3 = pile->AddStack2();
        if (pile3->GetState() == 1) // function call is done?
        {
            m_exprRetVar->RestoreState(pile3, bMain);
            return;
        }
    }

//    CBotStack*  pile1 = pile;

    int         i = 0;
    CBotVar*    ppVars[1000];
    CBotInstr*  p = m_parameters;
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

std::string CBotInstrCall::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << std::endl;
    //ss << "FuncID = " << m_nFuncIdent << std::endl;
    ss << "resultType = " << m_typRes.ToString();
    return ss.str();
}

std::map<std::string, CBotInstr*> CBotInstrCall::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_parameters"] = m_parameters;
    return links;
}

} // namespace CBot
