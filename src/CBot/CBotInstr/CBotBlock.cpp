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

#include "CBot/CBotInstr/CBotBlock.h"

#include "CBot/CBotCStack.h"
#include "CBot/CBotInstr/CBotListInstr.h"

#include "CBot/CBotDefines.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotBlock::Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal)
{
    pStack->SetStartError(p->GetStart());

    if (IsOfType(p, ID_OPBLK))
    {
        CBotInstr* inst = CBotListInstr::Compile(p, pStack, bLocal);

        if (IsOfType(p, ID_CLBLK))
        {
            return inst;
        }

        pStack->SetError(CBotErrCloseBlock, p->GetStart());    // missing parenthesis
        delete inst;
        return nullptr;
    }

    pStack->SetError(CBotErrOpenBlock, p->GetStart());
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotBlock::CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, bool bLocal)
{
    // is this a new block
    if (p->GetType() == ID_OPBLK) return CBotBlock::Compile(p, pStack);

    // otherwise, look for a single statement instead

    // to handle the case with local definition instruction (*)
    CBotCStack* pStk = pStack->TokenStack(p, bLocal);

    return pStack->Return( CBotInstr::Compile(p, pStk),    // a single instruction
                           pStk);
}

// (*) is the case in the following statement
// if (1 == 1) int x = 0;
// where the variable x is known only in the block following the if

} // namespace CBot
