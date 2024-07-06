/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotExternalCall.h"

#include "CBot/CBotToken.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotUtils.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

void CBotExternalCallList::Clear()
{
    m_list.clear();
}

bool CBotExternalCallList::AddFunction(const std::string& name, std::unique_ptr<CBotExternalCall> call)
{
    m_list[name] = std::move(call);
    return true;
}

CBotTypResult CBotExternalCallList::CompileCall(CBotToken*& p, CBotVar* thisVar, CBotVar** ppVar, CBotCStack* pStack)
{
    if (m_list.count(p->GetString()) == 0)
        return -1;

    CBotExternalCall* pt = m_list[p->GetString()].get();

    std::unique_ptr<CBotVar> args = std::unique_ptr<CBotVar>(MakeListVars(ppVar));
    CBotTypResult r = pt->Compile(thisVar, args.get(), m_user);

    // if a class is returned, it is actually a pointer
    if (r.GetType() == CBotTypClass) r.SetType(CBotTypPointer);

    if (r.GetType() > CBotTypMAX) // error?
    {
        pStack->SetError(static_cast<CBotError>(r.GetType()), p);
    }

    return r;
}

void CBotExternalCallList::SetUserPtr(void* pUser)
{
    m_user = pUser;
}

bool CBotExternalCallList::CheckCall(const std::string& name)
{
    return m_list.count(name) > 0;
}

int CBotExternalCallList::DoCall(CBotToken* token, CBotVar* thisVar, CBotVar** ppVar, CBotStack* pStack,
                                 const CBotTypResult& rettype)
{
    if (token == nullptr)
        return -1;

    if (m_list.count(token->GetString()) == 0)
        return -1;

    CBotExternalCall* pt = m_list[token->GetString()].get();

    if (thisVar == nullptr && pStack->IsCallFinished()) return true;  // only for non-method external call

    // if this is a method call we need to use AddStack()
    CBotStack* pile = (thisVar != nullptr) ? pStack->AddStack() : pStack->AddStackExternalCall(pt);

    if (pile->GetState() == 0) // the first time?
    {
        // lists the parameters depending on the contents of the stack
        CBotVar* pVar = MakeListVars(ppVar, true);
        pile->SetVar(pVar);

        CBotStack* pile2 = pile->AddStack();
        // creates a variable to the result
        CBotVar* pResult = rettype.Eq(CBotTypVoid) ? nullptr : CBotVar::Create("", rettype);
        pile2->SetVar(pResult);
        pile->IncState(); // increment state to mark this step done
    }

    pile->SetError(CBotNoErr, token); // save token for the position in case of error
    return pt->Run(thisVar, pStack);
}

bool CBotExternalCallList::RestoreCall(CBotToken* token, CBotVar* thisVar, CBotVar** ppVar, CBotStack* pStack)
{
    if (m_list.count(token->GetString()) == 0)
        return false;

    CBotExternalCall* pt = m_list[token->GetString()].get();

    // if this is a method call we need to use RestoreStack()
    CBotStack* pile = (thisVar != nullptr) ? pStack->RestoreStack() : pStack->RestoreStackEOX(pt);
    if (pile == nullptr) return true;

    pile->RestoreStack();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBotExternalCall::CBotExternalCall()
{
}

CBotExternalCall::~CBotExternalCall()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBotExternalCallDefault::CBotExternalCallDefault(RuntimeFunc rExec, CompileFunc rCompile, CancelFunc rCancel)
{
    m_rExec = rExec;
    m_rComp = rCompile;
    m_rCancel = rCancel;
}

CBotExternalCallDefault::~CBotExternalCallDefault()
{
}

CBotTypResult CBotExternalCallDefault::Compile(CBotVar* thisVar, CBotVar* args, void* user)
{
    return m_rComp(args, user);
}

bool CBotExternalCallDefault::Run(CBotVar* thisVar, CBotStack* pStack)
{
    if (pStack->IsCallFinished()) return true;
    CBotStack* pile = pStack->AddStackExternalCall(this);
    CBotVar* args = pile->GetVar();

    CBotStack* pile2 = pile->AddStack();

    CBotVar* result = pile2->GetVar();

    int exception = CBotNoErr; // TODO: Change to CBotError
    bool res = m_rExec(args, result, exception, pStack->GetUserPtr());

    if (!res)
    {
        if (exception != CBotNoErr)
        {
            pStack->SetError(static_cast<CBotError>(exception));
            pile->Delete();
        }
        return false;
    }

    pStack->Return(pile2); // return 'result' and clear extra stack
    return true;
}

void CBotExternalCallDefault::Cancel(CBotStack* pStack)
{
    if (m_rCancel != nullptr) m_rCancel(pStack->GetUserPtr());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBotExternalCallClass::CBotExternalCallClass(RuntimeFunc rExec, CompileFunc rCompile)
{
    m_rExec = rExec;
    m_rComp = rCompile;
}

CBotExternalCallClass::~CBotExternalCallClass()
{
}

CBotTypResult CBotExternalCallClass::Compile(CBotVar* thisVar, CBotVar* args, void* user)
{
    return m_rComp(thisVar, args);
}

bool CBotExternalCallClass::Run(CBotVar* thisVar, CBotStack* pStack)
{
    assert(thisVar != nullptr);
    CBotStack* pile = pStack->AddStack();
    CBotVar* args = pile->GetVar();

    CBotStack* pile2 = pile->AddStack();

    CBotVar* result = pile2->GetVar();

    int exception = CBotNoErr; // TODO: Change to CBotError
    bool res = m_rExec(thisVar, args, result, exception, pStack->GetUserPtr());

    if (!res)
    {
        if (exception != CBotNoErr)
        {
            pStack->SetError(static_cast<CBotError>(exception));
        }
        return false;
    }

    pStack->Return(pile2); // return 'result' and clear extra stack
    return true;
}

void CBotExternalCallClass::Cancel(CBotStack* pStack)
{
}

} // namespace CBot
