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
#include "CBot/CBotInstr/CBotCatch.h"
#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include

////////////////////////////////////////////////////////////////////////////////
CBotCatch::CBotCatch()
{
    m_Cond      =
    m_Block     = nullptr;     // nullptr so that delete is not possible further
    m_next      = nullptr;

    name = "CBotCatch";     // debug
}

////////////////////////////////////////////////////////////////////////////////
CBotCatch::~CBotCatch()
{
    delete  m_Cond;         // frees the list
    delete  m_Block;        // frees the instruction block
    delete  m_next;         // and subsequent
}

////////////////////////////////////////////////////////////////////////////////
CBotCatch* CBotCatch::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCatch*  inst = new CBotCatch();         // creates the object
    pStack->SetStartError(p->GetStart());

    inst->SetToken(p);
    if (!IsOfType(p, ID_CATCH)) return nullptr;    // should never happen

    if (IsOfType(p, ID_OPENPAR))
    {
        inst->m_Cond = CBotExpression::Compile(p, pStack);
        if (( pStack->GetType() < CBotTypLong ||
              pStack->GetTypResult().Eq(CBotTypBoolean) )&& pStack->IsOk() )
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStack );
                if ( pStack->IsOk() )
                    return inst;                // return an object to the application
            }
            pStack->SetError(CBotErrClosePar, p->GetStart());
        }
        pStack->SetError(CBotErrBadType1, p->GetStart());
    }
    pStack->SetError(CBotErrOpenPar, p->GetStart());
    delete inst;                                // error, frees up
    return nullptr;                                // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCatch :: Execute(CBotStack* &pj)
{
    if ( m_Block == nullptr ) return true;
    return m_Block->Execute(pj);                // executes the associated block
}

////////////////////////////////////////////////////////////////////////////////
void CBotCatch :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( bMain && m_Block != nullptr ) m_Block->RestoreState(pj, bMain);
}

////////////////////////////////////////////////////////////////////////////////
void CBotCatch :: RestoreCondState(CBotStack* &pj, bool bMain)
{
    m_Cond->RestoreState(pj, bMain);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotCatch :: TestCatch(CBotStack* &pile, int val)
{
    if ( !m_Cond->Execute(pile) ) return false;

    if ( val > 0 || pile->GetVar() == nullptr || pile->GetVar()->GetType() != CBotTypBoolean )
    {
        CBotVar* var = CBotVar::Create("", CBotTypBoolean);
        var->SetValInt( pile->GetVal() == val );
        pile->SetVar(var);                          // calls on the stack
    }

    return true;
}
