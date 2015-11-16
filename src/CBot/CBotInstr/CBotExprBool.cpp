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
#include "CBotExprBool.h"

#include "CBotStack.h"
#include "CBotCStack.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotExprBool::CBotExprBool()
{
    name = "CBotExprBool";
}

////////////////////////////////////////////////////////////////////////////////
CBotExprBool::~CBotExprBool()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprBool::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();
    CBotExprBool* inst = nullptr;

    if ( p->GetType() == ID_TRUE ||
         p->GetType() == ID_FALSE )
    {
        inst = new CBotExprBool();
        inst->SetToken(p);  // stores the operation false or true
        p = p->GetNext();

        CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(nullptr), CBotTypBoolean);
        pStk->SetVar(var);
    }

    return pStack->Return(inst, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprBool::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(nullptr), CBotTypBoolean);

    if (GetTokenType() == ID_TRUE)      var->SetValInt(1);
    else                              var->SetValInt(0);

    pile->SetVar(var);  // put on the stack
    return pj->Return(pile);    // forwards below
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprBool::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}