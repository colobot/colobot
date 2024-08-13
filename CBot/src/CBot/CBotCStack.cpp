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


#include "CBot/CBotCStack.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotToken.h"
#include "CBot/CBotExternalCall.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotInstr/CBotFunction.h"

namespace CBot
{

struct CBotCStack::Data
{
    //! The program currently being compiled
    CBotProgram*  prog   = nullptr;
    //! The current error state of the compile stack
    CBotError     error  = CBotNoErr;
    int           errEnd = 0;
    //! The return type of the function currently being compiled
    CBotTypResult retTyp = CBotTypResult(CBotTypVoid);
};

CBotCStack::CBotCStack(CBotCStack* ppapa)
{
    m_prev = ppapa;

    if (ppapa == nullptr)
    {
        m_data = new CBotCStack::Data;
        m_errStart = 0;
        m_bBlock = true;
    }
    else
    {
        m_data = ppapa->m_data;
        m_errStart = ppapa->m_errStart;
        m_bBlock = false;
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotCStack::~CBotCStack()
{
    if (m_prev == nullptr) delete m_data;
}

////////////////////////////////////////////////////////////////////////////////
CBotCStack* CBotCStack::TokenStack(CBotToken* pToken, bool bBlock)
{
    if (m_next) return m_next.get();                 // include on an existing stack

    m_next.reset(new CBotCStack(this));
    m_next->m_bBlock = bBlock;

    if (pToken != nullptr) m_next->SetStartError(pToken->GetStart());

    return m_next.get();
}

void CBotCStack::DeleteNext()
{
    m_next.reset();
}

CBotInstr* CBotCStack::Return(CBotInstr* inst, CBotCStack* pfils)
{
    if ( pfils == this ) return inst;

    m_var = std::move(pfils->m_var);             // result transmitted

    if (m_data->error != CBotNoErr)
    {
        m_errStart = pfils->m_errStart;          // retrieves the position of the error
    }

    m_next.reset();
    return inst;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotCStack::ReturnFunc(CBotFunction* inst, CBotCStack* pfils)
{
    m_var = std::move(pfils->m_var);             // result transmitted

    if (m_data->error != CBotNoErr)
    {
        m_errStart = pfils->m_errStart;          // retrieves the position of the error
    }

    m_next.reset();
    return inst;
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotCStack::GetError(int& start, int& end)
{
    start = m_errStart;
    end = m_data->errEnd;
    return m_data->error;
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotCStack::GetError()
{
    return m_data->error;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCStack::GetTypResult(CBotVar::GetTypeMode mode)
{
    if (m_var == nullptr)
        return CBotTypResult(99);
    return    m_var->GetTypResult(mode);
}

////////////////////////////////////////////////////////////////////////////////
int CBotCStack::GetType(CBotVar::GetTypeMode mode)
{
    if (m_var == nullptr)
        return 99;
    return    m_var->GetType(mode);
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotCStack::GetClass()
{
    if ( m_var == nullptr )
        return nullptr;
    if ( m_var->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) != CBotTypPointer ) return nullptr;

    return m_var->GetClass();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetType(CBotTypResult& type)
{
    if (m_var == nullptr) return;
    m_var->SetType( type );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::FindVar(CBotToken* &pToken)
{
    CBotCStack*    p = this;
    const auto&    name = pToken->GetString();

    while (p != nullptr)
    {
        if (p->m_bBlock) for (auto& var : p->m_listVar)
        {
            if (name == var->GetName()) return var.get();
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::FindVar(CBotToken& Token)
{
    CBotToken*    pt = &Token;
    return FindVar(pt);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::CopyVar(CBotToken& Token)
{
    CBotVar*    pVar = FindVar( Token );

    if ( pVar == nullptr) return nullptr;

    CBotVar*    pCopy = CBotVar::Create( "", pVar->GetType() );
    pCopy->Copy(pVar);
    return    pCopy;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::IsOk()
{
    return (m_data->error == CBotNoErr);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetStartError( int pos )
{
    if (m_data->error != CBotNoErr) return; // does not change existing error
    m_errStart = pos;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetError(CBotError n, int pos)
{
    if (n != CBotNoErr && m_data->error != CBotNoErr) return; // does not change existing error
    m_data->error = n;
    m_data->errEnd = pos;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetError(CBotError n, CBotToken* p)
{
    if (m_data->error != CBotNoErr) return; // does not change existing error
    m_data->error = n;
    m_errStart = p->GetStart();
    m_data->errEnd = p->GetEnd();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::ResetError(CBotError n, int start, int end)
{
    m_data->error = n;
    m_errStart = start;
    m_data->errEnd = end;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::NextToken(CBotToken* &p)
{
    CBotToken*    pp = p;

    p = p->GetNext();
    if (p!=nullptr) return true;

    SetError(CBotErrNoTerminator, pp->GetEnd());
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetProgram(CBotProgram* p)
{
    m_data->prog = p;
}

////////////////////////////////////////////////////////////////////////////////
CBotProgram* CBotCStack::GetProgram()
{
    return m_data->prog;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetRetType(CBotTypResult& type)
{
    m_data->retTyp = type;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCStack::GetRetType()
{
    return m_data->retTyp;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetVar( CBotVar* var )
{
    m_var.reset(var);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::SetCopyVar( CBotVar* var )
{
    m_var.reset();

    if ( var == nullptr ) return;
    m_var.reset(CBotVar::Create("", var->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC)));
    m_var->Copy( var );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotCStack::GetVar()
{
    return m_var.get();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::AddVar(CBotVar* pVar)
{
    CBotCStack*    p = this;

    // find the level of the current block
    while (p != nullptr && p->m_bBlock == 0) p = p->m_prev;

    if (p == nullptr || pVar == nullptr) return;

    p->m_listVar.emplace_back(pVar);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::CreateVarThis(CBotClass* pClass)
{
    if ( pClass == nullptr ) return;

    CBotVar* pThis = CBotVar::Create("this", CBotTypResult(CBotTypClass, pClass));

    pThis->SetUniqNum(-2); // special ID for "this"
    AddVar(pThis);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::CreateVarSuper(CBotClass* pClass)
{
    if ( pClass == nullptr ) return;

    CBotVar* pSuper = CBotVar::Create("super", CBotTypResult(CBotTypClass, pClass));

    pSuper->SetUniqNum(-3); // special ID for "super"
    AddVar(pSuper);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCStack::CreateMemberVars(CBotClass* pClass, bool setDefined)
{
    while (pClass != nullptr)
    {
        CBotVar* pv = pClass->GetVar();
        while (pv != nullptr)
        {
            CBotVar* pcopy = CBotVar::Create(pv);
            CBotVar::InitType initType = CBotVar::InitType::UNDEF;
            if (setDefined || pv->IsStatic())
                initType = CBotVar::InitType::DEF;
            pcopy->SetInit(initType);
            pcopy->SetUniqNum(pv->GetUniqNum());
            pcopy->SetPrivate(pv->GetPrivate());
            AddVar(pcopy);
            pv = pv->GetNext();
        }
        pClass = pClass->GetParent();
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::CheckVarLocal(CBotToken* &pToken)
{
    CBotCStack*    p = this;
    const auto&    name = pToken->GetString();

    // find the level of the current block
    while (p != nullptr && p->m_bBlock == 0) p = p->m_prev;

    if (p != nullptr) for (auto& var : p->m_listVar)
    {
        if (name == var->GetName()) return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCStack::CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent)
{
    nIdent = 0;
    CBotTypResult val(-1);

    val = GetProgram()->GetExternalCalls()->CompileCall(p, nullptr, ppVars, this);
    if (val.GetType() < 0)
    {
        val = CBotFunction::CompileCall(p->GetString(), ppVars, nIdent, GetProgram());
        if ( val.GetType() < 0 )
        {
    //        pVar = nullptr;                    // the error is not on a particular parameter
            SetError( static_cast<CBotError>(-val.GetType()), p );
            val.SetType(-val.GetType());
            return val;
        }
    }
    return val;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCStack::CheckCall(CBotToken* &pToken, CBotDefParam* pParam, const std::string& className)
{
    const auto& name = pToken->GetString();

    if ( GetProgram()->GetExternalCalls()->CheckCall(name) ) return true;

    for (CBotFunction* pp : GetProgram()->GetFunctions())
    {
        if ( name == pp->GetName() )
        {
            // ignore methods for a different class
            if ( className != pp->GetClassName() )
                continue;
            // are parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
    }

    for (CBotFunction* pp : GetProgram()->GetPublicFunctions())
    {
        if ( name == pp->GetName() )
        {
            // ignore methods for a different class
            if ( className != pp->GetClassName() )
                continue;
            // are parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
    }

    return false;
}

} // namespace CBot
