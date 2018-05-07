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

#include "CBot/CBotInstr/CBotThrow.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotThrow::CBotThrow()
{
    m_value = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotThrow::~CBotThrow()
{
    delete m_value;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotThrow::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GetStart());

    CBotThrow*  inst = new CBotThrow();         // creates the object
    inst->SetToken(p);

    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    if (!IsOfType(p, ID_THROW)) return nullptr;    // should never happen

    inst->m_value = CBotExpression::Compile(p, pStack );

    if (pStack->GetType() < CBotTypLong && pStack->IsOk())
    {
        return inst;                            // return an object to the application
    }
    pStack->SetError(CBotErrBadType1, pp);

    delete inst;                                // error, frees up
    return nullptr;                                // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotThrow::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return true;

    if ( pile->GetState() == 0 )
    {
        if ( !m_value->Execute(pile) ) return false;
        pile->IncState();
    }

    if ( pile->IfStep() ) return false;

    int val = pile->GetVal();
    if ( val < 0 ) val = CBotErrBadThrow;
    pile->SetError( static_cast<CBotError>(val), &m_token );
    return pj->Return( pile );
}

////////////////////////////////////////////////////////////////////////////////
void CBotThrow::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == nullptr ) return;

    if ( pile->GetState() == 0 )
    {
        m_value->RestoreState(pile, bMain);
        return;
    }
}

std::map<std::string, CBotInstr*> CBotThrow::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_value"] = m_value;
    return links;
}

} // namespace CBot
