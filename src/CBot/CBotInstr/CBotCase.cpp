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
#include "CBotCase.h"
#include "CBotExprNum.h"

#include "CBotStack.h"

// Local include

// Global include

////////////////////////////////////////////////////////////////////////////////
CBotCase::CBotCase()
{
    m_Value     = nullptr;     // nullptr so that delete is not possible further
    name = "CBotCase";      // debug
}

////////////////////////////////////////////////////////////////////////////////
CBotCase::~CBotCase()
{
    delete  m_Value;        // frees the value
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotCase::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCase*   inst = new CBotCase();          // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    inst->SetToken(p);
    if (!IsOfType(p, ID_CASE, ID_DEFAULT)) return nullptr;     // should never happen

    if ( pp->GetType() == ID_CASE )
    {
        pp = p;
        inst->m_Value = CBotExprNum::Compile(p, pStack);
        if ( inst->m_Value == nullptr )
        {
            pStack->SetError( TX_BADNUM, pp );
            delete inst;
            return nullptr;
        }
    }
    if ( !IsOfType( p, ID_DOTS ))
    {
        pStack->SetError( TX_MISDOTS, p->GetStart() );
        delete inst;
        return nullptr;
    }

    return inst;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCase::Execute(CBotStack* &pj)
{
    return true;                                // the "case" statement does nothing!
}

////////////////////////////////////////////////////////////////////////////////
void CBotCase::RestoreState(CBotStack* &pj, bool bMain)
{
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCase::CompCase(CBotStack* &pile, int val)
{
    if ( m_Value == nullptr ) return true;         // "default" case

    while (!m_Value->Execute(pile));            // puts the value on the correspondent stack (without interruption)
    return (pile->GetVal() == val);             // compared with the given value
}
