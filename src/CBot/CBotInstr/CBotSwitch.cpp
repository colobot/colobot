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

#include "CBot/CBotInstr/CBotSwitch.h"
#include "CBot/CBotInstr/CBotCase.h"
#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotSwitch::CBotSwitch()
{
    m_value = nullptr;
    m_block = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotSwitch::~CBotSwitch()
{
    delete m_value;        // frees the value
    delete m_block;        // frees the instruction block
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotSwitch::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotSwitch* inst = new CBotSwitch();        // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    inst->SetToken(p);
    if (!IsOfType(p, ID_SWITCH)) return nullptr;   // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    if ( IsOfType(p, ID_OPENPAR ) )
    {
        if ( nullptr != (inst->m_value = CBotExpression::Compile(p, pStk )) )
        {
            if ( pStk->GetType() < CBotTypLong )
            {
                if ( IsOfType(p, ID_CLOSEPAR ) )
                {
                    if ( IsOfType(p, ID_OPBLK ) )
                    {
                        IncLvl();

                        while( !IsOfType( p, ID_CLBLK ) )
                        {
                            if ( p->GetType() == ID_CASE || p->GetType() == ID_DEFAULT)
                            {
                                CBotCStack* pStk2 = pStk->TokenStack(p);    // un petit bout de pile svp

                                CBotInstr* i = CBotCase::Compile( p, pStk2 );
                                if (i == nullptr)
                                {
                                    delete inst;
                                    return pStack->Return(nullptr, pStk2);
                                }
                                delete pStk2;
                                if (inst->m_block == nullptr ) inst->m_block = i;
                                else inst->m_block->AddNext(i);
                                continue;
                            }

                            if (inst->m_block == nullptr )
                            {
                                pStk->SetError(CBotErrNoCase, p->GetStart());
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }

                            CBotInstr* i = CBotBlock::CompileBlkOrInst( p, pStk, true );
                            if ( !pStk->IsOk() )
                            {
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }
                            inst->m_block->AddNext(i);

                            if ( p == nullptr )
                            {
                                pStk->SetError(CBotErrCloseBlock, -1);
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }
                        }
                        DecLvl();

                        if (inst->m_block == nullptr )
                        {
                            pStk->SetError(CBotErrNoCase, p->GetStart());
                            delete inst;
                            return pStack->Return(nullptr, pStk);
                        }
                        // the statement block is ok
                        return pStack->Return(inst, pStk);  // return an object to the application
                    }
                    pStk->SetError( CBotErrOpenBlock, p->GetStart() );
                }
                pStk->SetError( CBotErrClosePar, p->GetStart() );
            }
            pStk->SetError( CBotErrBadType1, p->GetStart() );
        }
    }
    pStk->SetError( CBotErrOpenPar, p->GetStart());

    delete inst;                                // error, frees up
    return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
}

////////////////////////////////////////////////////////////////////////////////
bool CBotSwitch :: Execute(CBotStack* &pj)
{
    CBotStack* pile1 = pj->AddStack(this);      // adds an item to the stack
//  if ( pile1 == EOX ) return true;

    CBotInstr*  p = m_block;                    // first expression

    int     state = pile1->GetState();
    if (state == 0)
    {
        if ( !m_value->Execute(pile1) ) return false;
        pile1->SetState(state = -1);
    }

    if ( pile1->IfStep() ) return false;

    if ( state == -1 )
    {
        state = 0;
        int val = pile1->GetVal();                      // result of the value

        CBotStack* pile2 = pile1->AddStack();
        while ( p != nullptr )                             // search for the corresponding case in a list
        {
            state++;
            if ( p->CompCase( pile2, val ) ) break;     // found the case
            p = p->GetNext();
        }
        pile2->Delete();

        if ( p == nullptr ) return pj->Return(pile1);      // completed if nothing

        if ( !pile1->SetState(state) ) return false;
    }

    p = m_block;                                        // returns to the beginning
    while (state-->0) p = p->GetNext();                 // advance in the list

    while( p != nullptr )
    {
        if ( !p->Execute(pile1) ) return pj->BreakReturn(pile1);
        if ( !pile1->IncState() ) return false;
        p = p->GetNext();
    }
    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotSwitch :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack* pile1 = pj->RestoreStack(this);  // adds an item to the stack
    if ( pile1 == nullptr ) return;

    CBotInstr*  p = m_block;                    // first expression

    int     state = pile1->GetState();
    if (state == 0)
    {
        m_value->RestoreState(pile1, bMain);
        return;
    }

    if ( state == -1 )
    {
        return;
    }

//  p = m_block;                                // returns to the beginning
    while ( p != nullptr && state-- > 0 )
    {
        p->RestoreState(pile1, false);
        p = p->GetNext();                       // advance in the list
    }

    if( p != nullptr )
    {
        p->RestoreState(pile1, true);
        return;
    }
}

std::map<std::string, CBotInstr*> CBotSwitch::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_value"] = m_value;
    links["m_block"] = m_block;
    return links;
}

} // namespace CBot
