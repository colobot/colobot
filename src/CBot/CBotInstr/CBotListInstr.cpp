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

#include "CBot/CBotInstr/CBotListInstr.h"
#include "CBot/CBotInstr/CBotBlock.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotListInstr::CBotListInstr()
{
    m_instr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotListInstr::~CBotListInstr()
{
    delete m_instr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotListInstr::Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal)
{
    CBotCStack* pStk = pStack->TokenStack(p, bLocal);        // variables are local

    CBotListInstr* inst = new CBotListInstr();

    while (true)
    {
        if (p == nullptr) break;

        if (IsOfType(p, ID_SEP)) continue;              // empty statement ignored
        if (p->GetType() == ID_CLBLK) break;

        if (p->GetType() == TokenTypNone)
        {
            pStack->SetError(CBotErrCloseBlock, p->GetStart());
            delete inst;
            return pStack->Return(nullptr, pStk);
        }

        CBotInstr* i = CBotBlock::CompileBlkOrInst(p, pStk);    // compiles next

        if (!pStk->IsOk())
        {
            delete inst;
            return pStack->Return(nullptr, pStk);
        }

        if (inst->m_instr == nullptr) inst->m_instr = i;
        else inst->m_instr->AddNext(i);                            // added a result
    }
    return pStack->Return(inst, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotListInstr::Execute(CBotStack* &pj)
{

    CBotStack*    pile = pj->AddStack(this, CBotStack::BlockVisibilityType::BLOCK);                //needed for SetState()
    if (pile->StackOver() ) return pj->Return( pile);


    CBotInstr*    p = m_instr;                                    // the first expression

    int        state = pile->GetState();
    while (state-->0) p = p->GetNext();                            // returns to the interrupted operation

    if (p != nullptr) while (true)
    {
        if (!p->Execute(pile)) return false;
        p = p->GetNext();
        if (p == nullptr) break;
        (void)pile->IncState();                                  // ready for next
    }

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotListInstr::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);
    if (pile == nullptr) return;

    CBotInstr*    p = m_instr;                                    // the first expression

    int        state = pile->GetState();
    while ( p != nullptr && state-- > 0)
    {
        p->RestoreState(pile, false);
        p = p->GetNext();                            // returns to the interrupted operation
    }

    if (p != nullptr) p->RestoreState(pile, true);
}

bool CBotListInstr::HasReturn()
{
    if (m_instr != nullptr && m_instr->HasReturn()) return true;
    return CBotInstr::HasReturn(); // check next block or instruction
}

std::map<std::string, CBotInstr*> CBotListInstr::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_instr"] = m_instr;
    return links;
}

} // namespace CBot
