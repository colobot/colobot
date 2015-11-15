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
#include "CBotCallMethode.h"

#include "CBotUtils.h"
#include "CBotStack.h"
#include "CBotCStack.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotCallMethode::CBotCallMethode(const char* name,
                   bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                   CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar))
{
    m_name       = name;
    m_rExec      = rExec;
    m_rComp      = rCompile;
    m_next       = nullptr;
    m_nFuncIdent = CBotVar::NextUniqNum();
}

////////////////////////////////////////////////////////////////////////////////
CBotCallMethode::~CBotCallMethode()
{
    delete m_next;
    m_next = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotCallMethode::CompileCall(const char* name,
                                           CBotVar* pThis,
                                           CBotVar** ppVar,
                                           CBotCStack* pStack,
                                           long& nIdent)
{
    CBotCallMethode*    pt = this;
    nIdent = 0;

    while ( pt != nullptr )
    {
        if ( pt->m_name == name )
        {
            CBotVar*    pVar = MakeListVars(ppVar, true);
            CBotVar*    pVar2 = pVar;
            CBotTypResult r = pt->m_rComp(pThis, pVar2);
            int ret = r.GetType();
            if ( ret > 20 )
            {
                if (pVar2) pStack->SetError(ret, pVar2->GetToken());
            }
            delete pVar;
            nIdent = pt->m_nFuncIdent;
            return r;
        }
        pt = pt->m_next;
    }
    return CBotTypResult(-1);
}

////////////////////////////////////////////////////////////////////////////////
CBotString CBotCallMethode::GetName()
{
    return  m_name;
}

////////////////////////////////////////////////////////////////////////////////
CBotCallMethode* CBotCallMethode::Next()
{
    return  m_next;
}

////////////////////////////////////////////////////////////////////////////////
void CBotCallMethode::AddNext(CBotCallMethode* pt)
{
    CBotCallMethode* p = this;
    while ( p->m_next != nullptr ) p = p->m_next;

    p->m_next = pt;
}

////////////////////////////////////////////////////////////////////////////////
int CBotCallMethode::DoCall(long& nIdent,
                            const char* name,
                            CBotVar* pThis,
                            CBotVar** ppVars,
                            CBotVar* &pResult,
                            CBotStack* pStack,
                            CBotToken* pToken)
{
    CBotCallMethode*    pt = this;

    // search by the identifier

    if ( nIdent ) while ( pt != nullptr )
    {
        if ( pt->m_nFuncIdent == nIdent )
        {
            // lists the parameters depending on the contents of the stack (pStackVar)

            CBotVar*    pVar = MakeListVars(ppVars, true);
            CBotVar*    pVarToDelete = pVar;

            // then calls the routine external to the module

            int         Exception = 0;
            int res = pt->m_rExec(pThis, pVar, pResult, Exception, pStack->GetPUser());
            pStack->SetVar(pResult);

            if (res == false)
            {
                if (Exception!=0)
                {
//                  pStack->SetError(Exception, pVar->GetToken());
                    pStack->SetError(Exception, pToken);
                }
                delete pVarToDelete;
                return false;
            }
            delete pVarToDelete;
            return true;
        }
        pt = pt->m_next;
    }

    // search by name

    while ( pt != nullptr )
    {
        if ( pt->m_name == name )
        {
            // lists the parameters depending on the contents of the stack (pStackVar)

            CBotVar*    pVar = MakeListVars(ppVars, true);
            CBotVar*    pVarToDelete = pVar;

            int         Exception = 0;
            int res = pt->m_rExec(pThis, pVar, pResult, Exception, pStack->GetPUser());
            pStack->SetVar(pResult);

            if (res == false)
            {
                if (Exception!=0)
                {
//                  pStack->SetError(Exception, pVar->GetToken());
                    pStack->SetError(Exception, pToken);
                }
                delete pVarToDelete;
                return false;
            }
            delete pVarToDelete;
            nIdent = pt->m_nFuncIdent;
            return true;
        }
        pt = pt->m_next;
    }

    return -1;
}
