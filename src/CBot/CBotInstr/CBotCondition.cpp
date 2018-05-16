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

#include "CBot/CBotInstr/CBotCondition.h"
#include "CBot/CBotInstr/CBotBoolExpr.h"

#include "CBot/CBotCStack.h"

#include "CBot/CBotDefines.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotCondition::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GetStart());
    if (IsOfType(p, ID_OPENPAR))
    {
        CBotInstr* inst = CBotBoolExpr::Compile(p, pStack);
        if (nullptr != inst)
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                return inst;
            }
            pStack->SetError(CBotErrClosePar, p->GetStart());    // missing parenthesis
        }
        delete inst;
    }

    pStack->SetError(CBotErrOpenPar, p->GetStart());    // missing parenthesis

    return nullptr;
}

} // namespace CBot
