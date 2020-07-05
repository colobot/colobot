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

    CBotCStack* pStk = pStack->TokenStack(pp);  // some space for a stack, plz

    if ( IsOfType(p, ID_OPENPAR ) )
    {
        if ( nullptr != (inst->m_value = CBotExpression::Compile(p, pStk )) )
        {
            if ( pStk->GetType() <= CBotTypLong )
            {
                if ( IsOfType(p, ID_CLOSEPAR ) )
                {
                    if ( IsOfType(p, ID_OPBLK ) )
                    {
                        IncLvl();

                        CBotCase* caseInst = nullptr;
                        CBotCStack* pStk2 = nullptr;
                        while( !IsOfType( p, ID_CLBLK ) )
                        {
                            if ( p->GetType() == ID_CASE || p->GetType() == ID_DEFAULT)
                            {
                                delete pStk2;
                                pStk2 = pStk->TokenStack(p, true);          // some space for a stack, plz

                                caseInst = static_cast<CBotCase*>(CBotCase::Compile(p, pStk2, inst->m_labels));
                                if (caseInst == nullptr)
                                {
                                    delete inst;
                                    return pStack->Return(nullptr, pStk2);
                                }

                                if (inst->m_block == nullptr ) inst->m_block = caseInst;
                                else inst->m_block->AddNext(caseInst);

                                if (ID_DEFAULT == caseInst->GetTokenType())
                                {
                                    if (inst->m_default != nullptr)
                                    {
                                        pStk->SetError(CBotErrRedefCase, caseInst->GetToken());
                                        delete inst;
                                        return pStack->Return(nullptr, pStk);
                                    }
                                    inst->m_default = caseInst;
                                }
                                continue;
                            }

                            if (inst->m_block == nullptr )
                            {
                                pStk->SetError(CBotErrNoCase, p->GetStart());
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }

                            CBotInstr* i = CBotBlock::CompileBlkOrInst(p, pStk2);
                            if ( !pStk2->IsOk() )
                            {
                                delete inst;
                                return pStack->Return(nullptr, pStk2);
                            }
                            if (caseInst->m_instr == nullptr ) caseInst->m_instr = i;
                            else caseInst->m_instr->AddNext(i);

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

    int     state = pile1->GetState();
    if (state == 0)
    {
        if ( !m_value->Execute(pile1) ) return false;
        pile1->SetState(state = 1);
    }

    if ( pile1->IfStep() ) return false;

    auto it = m_labels.find(pile1->GetVar()->GetValLong());

    CBotInstr* p = (it != m_labels.end()) ? it->second : m_default;

    while (--state > 0) p = p->GetNext();

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

    int     state = pile1->GetState();
    if (state == 0)
    {
        m_value->RestoreState(pile1, bMain);
        return;
    }

    auto it = m_labels.find(pile1->GetVar()->GetValLong());

    CBotInstr* p = (it != m_labels.end()) ? it->second : m_default;

    while (p != nullptr && --state > 0)
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
