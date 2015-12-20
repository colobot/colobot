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
#include "CBot/CBotCall.h"

#include "CBot/CBotToken.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotUtils.h"

#include "CBot/CBotVar/CBotVar.h"


// Local include

// Global include



////////////////////////////////////////////////////////////////////////////////

CBotCall* CBotCall::m_ListCalls = nullptr;
void* CBotCall::m_pUser = nullptr;

////////////////////////////////////////////////////////////////////////////////
CBotCall::CBotCall(const std::string& name,
                   bool rExec(CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                   CBotTypResult rCompile(CBotVar*& pVar, void* pUser))
{
    m_name       = name;
    m_rExec      = rExec;
    m_rComp      = rCompile;
    m_next       = nullptr;
    m_nFuncIdent = CBotVar::NextUniqNum();
}

////////////////////////////////////////////////////////////////////////////////
CBotCall::~CBotCall()
{
    if (m_next) delete m_next;
    m_next = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCall::Free()
{
    delete CBotCall::m_ListCalls;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCall::AddFunction(const std::string& name,
                           bool rExec(CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                           CBotTypResult rCompile(CBotVar*& pVar, void* pUser))
{
    CBotCall*   p = m_ListCalls;
    CBotCall*   pp = nullptr;

    if ( p != nullptr ) while ( p->m_next != nullptr )
    {
        if ( p->GetName() == name )
        {
            // frees redefined function
            if ( pp ) pp->m_next = p->m_next;
            else      m_ListCalls = p->m_next;
            pp = p;
            p = p->m_next;
            pp->m_next = nullptr;  // not to destroy the following list
            delete pp;
            continue;
        }
        pp = p;             // previous pointer
        p = p->m_next;
    }

    pp = new CBotCall(name, rExec, rCompile);

    if (p) p->m_next = pp;
    else m_ListCalls = pp;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCall::CompileCall(CBotToken* &p, CBotVar** ppVar, CBotCStack* pStack, long& nIdent)
{
    nIdent = 0;
    CBotCall*   pt = m_ListCalls;
    std::string  name = p->GetString();

    while ( pt != nullptr )
    {
        if ( pt->m_name == name )
        {
            CBotVar*    pVar = MakeListVars(ppVar);
            CBotVar*    pVar2 = pVar;
            CBotTypResult r = pt->m_rComp(pVar2, m_pUser);
            int ret = r.GetType();

            // if a class is returned, it is actually a pointer
            if ( ret == CBotTypClass ) r.SetType( ret = CBotTypPointer );

            if ( ret > 20 )
            {
                if (pVar2) pStack->SetError(static_cast<CBotError>(ret), p /*pVar2->GetToken()*/ );
            }
            delete pVar;
            nIdent = pt->m_nFuncIdent;
            return r;
        }
        pt = pt->m_next;
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCall::SetPUser(void* pUser)
{
    m_pUser = pUser;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCall::CheckCall(const std::string& name)
{
    CBotCall* p = m_ListCalls;

    while ( p != nullptr )
    {
        if ( name == p->GetName() ) return true;
        p = p->m_next;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotCall::GetName()
{
    return  m_name;
}

////////////////////////////////////////////////////////////////////////////////
CBotCall* CBotCall::Next()
{
    return  m_next;
}

////////////////////////////////////////////////////////////////////////////////
int CBotCall::DoCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack, CBotTypResult& rettype)
{
    CBotCall*   pt = m_ListCalls;

    if ( nIdent ) while ( pt != nullptr )
    {
        if ( pt->m_nFuncIdent == nIdent )
        {
            goto fund;
        }
        pt = pt->m_next;
    }

    pt = m_ListCalls;

    if ( token != nullptr )
    {
        std::string name = token->GetString();
        while ( pt != nullptr )
        {
            if ( pt->m_name == name )
            {
                nIdent = pt->m_nFuncIdent;
                goto fund;
            }
            pt = pt->m_next;
        }
    }

    return -1;

fund:
#if !STACKRUN
    // lists the parameters depending on the contents of the stack (pStackVar)

    CBotVar*    pVar = MakeListVars(ppVar, true);
    CBotVar*    pVarToDelete = pVar;

    // creates a variable to the result
    CBotVar*    pResult = rettype.Eq(0) ? nullptr : CBotVar::Create("", rettype);

    CBotVar*    pRes = pResult;
    int         Exception = 0;
    int res = pt->m_rExec(pVar, pResult, Exception, pStack->GetPUser());

    if ( pResult != pRes ) delete pRes; // different result if made
    delete pVarToDelete;

    if (res == false)
    {
        if (Exception!=0)
        {
            pStack->SetError(Exception, token);
        }
        delete pResult;
        return false;
    }
    pStack->SetVar(pResult);

    if ( rettype.GetType() > 0 && pResult == nullptr )
    {
        pStack->SetError(CBotErrNoRetVal, token);
    }
    nIdent = pt->m_nFuncIdent;
    return true;

#else

    CBotStack*  pile = pStack->AddStackEOX(pt);
    if ( pile == EOX ) return true;

    // lists the parameters depending on the contents of the stack (pStackVar)

    CBotVar*    pVar = MakeListVars(ppVar, true);
//    CBotVar*    pVarToDelete = pVar;

    // creates a variable to the result
    CBotVar*    pResult = rettype.Eq(0) ? nullptr : CBotVar::Create("", rettype);

    pile->SetVar( pVar );

    CBotStack*  pile2 = pile->AddStack();
    pile2->SetVar( pResult );

    pile->SetError(0, token);           // for the position on error + away
    return pt->Run( pStack );

#endif

}

#if STACKRUN

////////////////////////////////////////////////////////////////////////////////
bool CBotCall::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack)
{
    CBotCall*   pt = m_ListCalls;

    {
        std::string name = token->GetString();
        while ( pt != nullptr )
        {
            if ( pt->m_name == name )
            {
                nIdent = pt->m_nFuncIdent;

                CBotStack*  pile = pStack->RestoreStackEOX(pt);
                if ( pile == nullptr ) return true;

 //               CBotStack*  pile2 = pile->RestoreStack();
                pile->RestoreStack();
                return true;
            }
            pt = pt->m_next;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCall::Run(CBotStack* pStack)
{
    CBotStack*  pile = pStack->AddStackEOX(this);
    if ( pile == EOX ) return true;
    CBotVar*    pVar = pile->GetVar();

    CBotStack*  pile2 = pile->AddStack();
    CBotVar*    pResult = pile2->GetVar();
    CBotVar*    pRes = pResult;

    int         Exception = 0;
    int res = m_rExec(pVar, pResult, Exception, pStack->GetPUser());

    if (res == false)
    {
        if (Exception!=0)
        {
            pStack->SetError(Exception);
        }
        if ( pResult != pRes ) delete pResult;  // different result if made
        return false;
    }

    if ( pResult != nullptr ) pStack->SetCopyVar( pResult );
    if ( pResult != pRes ) delete pResult;  // different result if made

    return true;
}

#endif
