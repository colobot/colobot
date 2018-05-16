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

#include "CBot/CBotInstr/CBotReturn.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotReturn::CBotReturn()
{
    m_instr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotReturn::~CBotReturn()
{
    delete m_instr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotReturn::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;

    if (!IsOfType(p, ID_RETURN)) return nullptr;   // should never happen

    CBotReturn* inst = new CBotReturn();        // creates the object
    inst->SetToken( pp );

    CBotTypResult   type = pStack->GetRetType();

    if ( type.GetType() == 0 )                  // returned void ?
    {
        if ( IsOfType( p, ID_SEP ) ) return inst;
        pStack->SetError( CBotErrBadType1, pp );
        return nullptr;
    }

    inst->m_instr = CBotExpression::Compile(p, pStack);
    if ( pStack->IsOk() )
    {
        CBotTypResult   retType = pStack->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);
        if (TypeCompatible(retType, type, ID_ASS))
        {
            if ( IsOfType( p, ID_SEP ) )
                return inst;

            pStack->SetError(CBotErrNoTerminator, p->GetStart());
        }
        pStack->SetError(CBotErrBadType1, p->GetStart());
    }

    delete inst;
    return nullptr;                            // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotReturn::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return true;

    if ( pile->GetState() == 0 )
    {
        if (m_instr != nullptr && !m_instr->Execute(pile) ) return false; // evaluate the result
        // the result is on the stack
        pile->IncState();
    }

    if ( pile->IfStep() ) return false;

    pile->SetBreak(3, std::string());
    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotReturn::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;
    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == nullptr ) return;

    if ( pile->GetState() == 0 )
    {
        if (m_instr != nullptr ) m_instr->RestoreState(pile, bMain);  // evaluate the result
        return;
    }
}

bool CBotReturn::HasReturn()
{
    return true;
}

std::map<std::string, CBotInstr*> CBotReturn::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_instr"] = m_instr;
    return links;
}

} // namespace CBot
