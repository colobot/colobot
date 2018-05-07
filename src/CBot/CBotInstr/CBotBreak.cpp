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

#include "CBot/CBotInstr/CBotBreak.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotBreak::CBotBreak()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotBreak::~CBotBreak()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotBreak::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)
    int type = p->GetType();

    if (!IsOfType(p, ID_BREAK, ID_CONTINUE)) return nullptr;   // should never happen

    if ( !ChkLvl(std::string(), type ) )
    {
        pStack->SetError(CBotErrBreakOutside, pp);
        return nullptr;                            // no object, the error is on the stack
    }

    CBotBreak*  inst = new CBotBreak();         // creates the object
    inst->SetToken(pp);                         // keeps the operation

    pp = p;
    if ( IsOfType( p, TokenTypVar ) )
    {
        inst->m_label = pp->GetString();        // register the name of label
        if ( !ChkLvl(inst->m_label, type ) )
        {
            delete inst;
            pStack->SetError(CBotErrUndefLabel, pp);
            return nullptr;                            // no object, the error is on the stack
        }
    }

    if (IsOfType(p, ID_SEP))
    {
        return  inst;                           // return what it wants
    }
    delete inst;

    pStack->SetError(CBotErrNoTerminator, p->GetStart());
    return nullptr;                            // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotBreak :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);
//  if ( pile == EOX ) return true;

    if ( pile->IfStep() ) return false;

    pile->SetBreak(m_token.GetType()==ID_BREAK ? 1 : 2, m_label);
    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotBreak :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}

std::string CBotBreak::GetDebugData()
{
    return !m_label.empty() ? "m_label = "+m_label : "";
}

} // namespace CBot
