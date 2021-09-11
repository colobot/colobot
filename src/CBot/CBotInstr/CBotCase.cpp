/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotCase.h"

#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotCase::CBotCase()
{
    m_instr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotCase::~CBotCase()
{
    delete m_instr;
}

CBotInstr* CBotCase::Compile(CBotToken* &p, CBotCStack* pStack, std::unordered_map<long, CBotInstr*>& labels)
{
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    if (!IsOfType(p, ID_CASE, ID_DEFAULT)) return nullptr;     // should never happen
    pStack->SetStartError(pp->GetStart());

    long labelValue = 0;

    if (pp->GetType() == ID_CASE)
    {
        CBotInstr* i = nullptr;
        if (nullptr != (i = CBotTwoOpExpr::Compile(p, pStack, nullptr, true)))
        {
            if (pStack->GetType() <= CBotTypLong)
            {
                CBotStack* pile = CBotStack::AllocateStack();
                while ( !i->Execute(pile) );
                labelValue = pile->GetVar()->GetValLong();
                pile->Delete();

                if (labels.count(labelValue) > 0)
                {
                    pStack->SetError(CBotErrRedefCase, p->GetStart());
                }
            }
            else
                pStack->SetError(CBotErrBadNum, p->GetStart());
            delete i;
        }
        else
            pStack->SetError(CBotErrBadNum, p->GetStart());
    }

    if (pStack->IsOk() && IsOfType(p, ID_DOTS))
    {
        CBotCase* newCase = new CBotCase();
        newCase->SetToken(pp);
        if (pp->GetType() == ID_CASE)
            labels[labelValue] = newCase;
        return newCase;
    }

    pStack->SetError(CBotErrNoDoubleDots, p->GetStart());
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCase::Execute(CBotStack* &pj)
{
    if (m_instr == nullptr) return true;
    CBotStack* pile = pj->AddStack(this, CBotStack::BlockVisibilityType::BLOCK);

    int state = pile->GetState();
    CBotInstr* p = m_instr;
    while (state-- > 0) p = p->GetNext();

    while (p != nullptr)
    {
        if (!p->Execute(pile)) return false;
        pile->IncState();
        p = p->GetNext();
    }

    pile->Delete();
    return pj->IsOk();
}

////////////////////////////////////////////////////////////////////////////////
void CBotCase::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack* pile = pj->RestoreStack(this);
    if (pile == nullptr) return;

    CBotInstr* p = m_instr;

    int state = pile->GetState();
    while (p != nullptr && state-- > 0)
    {
        p->RestoreState(pile, bMain);
        p = p->GetNext();
    }

    if (p != nullptr) p->RestoreState(pile, bMain);
}

std::map<std::string, CBotInstr*> CBotCase::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_instr"] = m_instr;
    return links;
}

} // namespace CBot
