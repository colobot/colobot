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
#include "CBotLeftExprVar.h"

#include "CBotStack.h"

// Local include

// Global include
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
CBotLeftExprVar::CBotLeftExprVar()
{
    name    = "CBotLeftExprVar";
    m_typevar    = -1;
    m_nIdent    =  0;
}

////////////////////////////////////////////////////////////////////////////////
CBotLeftExprVar::~CBotLeftExprVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotLeftExprVar::Compile(CBotToken* &p, CBotCStack* pStack)
{
    // verifies that the token is a variable name
    if (p->GetType() != TokenTypVar)
    {
        pStack->SetError( TX_NOVAR, p->GetStart());
        return nullptr;
    }

    CBotLeftExprVar* inst = new CBotLeftExprVar();
    inst->SetToken(p);
    p = p->GetNext();

    return inst;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotLeftExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     var1;
    CBotVar*     var2;

    var1 = CBotVar::Create(m_token.GetString(), m_typevar);
    var1->SetUniqNum(m_nIdent);                             // with the unique identifier
    pj->AddVar(var1);                                       // place it on the stack

    var2 = pj->GetVar();                                    // result on the stack
    if (var2) var1->SetVal(var2);                           // do the assignment

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotLeftExprVar::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotVar*     var1;

    var1 = pj->FindVar(m_token.GetString());
    if (var1 == nullptr) assert(0);

    var1->SetUniqNum(m_nIdent);                    // with the unique identifier
}
