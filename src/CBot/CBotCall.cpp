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

#include "CBot/CBotCall.h"

#include "CBot/CBotToken.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotUtils.h"

#include "CBot/CBotVar/CBotVar.h"


std::map<std::string, std::unique_ptr<CBotCall>> CBotCall::m_list = std::map<std::string, std::unique_ptr<CBotCall>>();
void* CBotCall::m_user = nullptr;

CBotCall::CBotCall(const std::string& name, RuntimeFunc rExec, CompileFunc rCompile)
{
    m_name  = name;
    m_rExec = rExec;
    m_rComp = rCompile;
    m_ident = CBotVar::NextUniqNum();
}

CBotCall::~CBotCall()
{
}

void CBotCall::Clear()
{
    m_list.clear();
}

bool CBotCall::AddFunction(const std::string& name, RuntimeFunc rExec, CompileFunc rCompile)
{
    m_list[name] = std::unique_ptr<CBotCall>(new CBotCall(name, rExec, rCompile));
    return true;
}

CBotTypResult CBotCall::CompileCall(CBotToken* &p, CBotVar** ppVar, CBotCStack* pStack, long& nIdent)
{
    nIdent = 0;
    if (m_list.count(p->GetString()) == 0)
        return -1;

    CBotCall* pt = m_list[p->GetString()].get();
    nIdent = pt->m_ident;

    std::unique_ptr<CBotVar> args = std::unique_ptr<CBotVar>(MakeListVars(ppVar));
    CBotVar* var = args.get(); // TODO: This shouldn't be a reference
    CBotTypResult r = pt->m_rComp(var, m_user);

    // if a class is returned, it is actually a pointer
    if (r.GetType() == CBotTypClass) r.SetType(CBotTypPointer);

    if (r.GetType() > 20) // error?
    {
        pStack->SetError(static_cast<CBotError>(r.GetType()), p);
    }

    return r;
}

void CBotCall::SetUserPtr(void* pUser)
{
    m_user = pUser;
}

bool CBotCall::CheckCall(const std::string& name)
{
    return m_list.count(name) > 0;
}

int CBotCall::DoCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack, CBotTypResult& rettype)
{
    CBotCall*   pt = nullptr;

    if (nIdent > 0)
    {
        for (const auto& it : m_list)
        {
            if (it.second->m_ident == nIdent)
            {
                pt = it.second.get();
            }
        }
    }

    if (pt == nullptr)
    {
        if (token != nullptr)
        {
            if (m_list.count(token->GetString()) > 0)
            {
                pt = m_list[token->GetString()].get();
                nIdent = pt->m_ident;
            }
        }
    }

    if (pt == nullptr)
        return -1;

    CBotStack*  pile = pStack->AddStackEOX(pt);
    if (pile == EOX) return true;

    // lists the parameters depending on the contents of the stack (pStackVar)
    CBotVar*    pVar = MakeListVars(ppVar, true);

    // creates a variable to the result
    CBotVar*    pResult = rettype.Eq(CBotTypVoid) ? nullptr : CBotVar::Create("", rettype);

    pile->SetVar(pVar);

    CBotStack*  pile2 = pile->AddStack();
    pile2->SetVar(pResult);

    pile->SetError(CBotNoErr, token); // save token for the position in case of error
    return pt->Run(pStack);

}

bool CBotCall::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack)
{
    if (m_list.count(token->GetString()) == 0)
        return false;

    CBotCall* pt = m_list[token->GetString()].get();
    nIdent = pt->m_ident;

    CBotStack*  pile = pStack->RestoreStackEOX(pt);
    if ( pile == nullptr ) return true;

    pile->RestoreStack();
    return true;
}

bool CBotCall::Run(CBotStack* pStack)
{
    CBotStack*  pile = pStack->AddStackEOX(this);
    if ( pile == EOX ) return true;
    CBotVar* args = pile->GetVar();

    CBotStack* pile2 = pile->AddStack();

    CBotVar* result = pile2->GetVar();

    int exception = CBotNoErr; // TODO: Change to CBotError
    bool res = m_rExec(args, result, exception, pStack->GetPUser());

    if (!res)
    {
        if (exception != CBotNoErr)
        {
            pStack->SetError(static_cast<CBotError>(exception));
        }
        return false;
    }

    if (result != nullptr) pStack->SetCopyVar(result);

    return true;
}
