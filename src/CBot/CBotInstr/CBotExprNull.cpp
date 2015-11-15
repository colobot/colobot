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
#include "CBotExprNull.h"

#include "CBotStack.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotExprNull::CBotExprNull()
{
    name = "CBotExprNull";
}

////////////////////////////////////////////////////////////////////////////////
CBotExprNull::~CBotExprNull()
{
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprNull::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;
    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(nullptr), CBotTypNullPointer);

    var->SetInit(CBotVar::InitType::DEF);         // null pointer valid
    pile->SetVar(var);          // place on the stack
    return pj->Return(pile);    // forwards below
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprNull::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}
