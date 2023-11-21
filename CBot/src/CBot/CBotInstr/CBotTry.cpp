/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotTry.h"

#include "CBot/CBotInstr/CBotBlock.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotTry::CBotTry()
{
    m_catchList = nullptr;
    m_finallyBlock = nullptr;
    m_block = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotTry::~CBotTry()
{
    delete m_catchList;    // frees the list
    delete m_block;        // frees the instruction block
    delete m_finallyBlock;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotTry::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotTry*    inst = new CBotTry();           // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    inst->SetToken(p);
    if (!IsOfType(p, ID_TRY)) return nullptr;      // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // some space for a stack, plz

    inst->m_block = CBotBlock::CompileBlkOrInst(p, pStk );
    CBotCatch** pn = &inst->m_catchList;

    while (pStk->IsOk() && p->GetType() == ID_CATCH)
    {
        CBotCatch*  i = CBotCatch::Compile(p, pStk);
        *pn = i;
        pn = &i->m_next;
    }

    if (pStk->IsOk() && IsOfType( p, ID_FINALLY) )
    {
        inst->m_finallyBlock = CBotBlock::CompileBlkOrInst(p, pStk );
    }

    if (pStk->IsOk())
    {
        return pStack->Return(inst, pStk);  // return an object to the application
    }

    delete inst;                                // error, frees up
    return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
}


////////////////////////////////////////////////////////////////////////////////
bool CBotTry::Execute(CBotStack* &pj)
{
    int     val;

    CBotStack* pile1 = pj->AddStack(this);  // adds an item to the stack
//  if ( pile1 == EOX ) return true;

    if ( pile1->IfStep() ) return false;
                                                    // or find in case of recovery
    CBotStack* pile0 = pj->AddStack2();             // adds an element to the secondary stack
    CBotStack* pile2 = pile0->AddStack();

    if ( pile1->GetState() == 0 )
    {
        if ( m_block->Execute(pile1) )
        {
            if (m_finallyBlock == nullptr ) return pj->Return(pile1);
            pile1->SetState(-2);                                // passes final
        }

        val = pile1->GetError();

        pile1->IncState();
        pile2->SetState(val);                                   // stores the error number
        pile1->SetError(CBotNoErr);         // for now there are more errors!

        if ( val == CBotNoErr && pile1->GetTimer() < 0 )            // mode step?
            return false;                                       // does not make the catch
    }

    // there was an interruption
    // see what it returns

    CBotCatch*  pc = m_catchList;
    int state = static_cast<short>(pile1->GetState());                       // where were we?
    val = pile2->GetState();                                    // what error?
    pile0->SetState(1);                                         // marking the GetRunPos

    if ( val >= 0 && state > 0 ) while ( pc != nullptr )
    {
        if ( --state <= 0 )
        {
            // ask to the catch block if it feels concerned
            if ( !pc->TestCatch(pile2, val) ) return false;     // suspend !
            if (pile2->GetVal() != 0 && pile1->IsChildSuspended())
            {
                CBotProgram::CancelExternal(pj);
            }
            pile1->IncState();
        }
        if ( --state <= 0 )
        {
            if (pile2->GetVal() != 0)
            {
//              pile0->SetState(1);

                if ( !pc->Execute(pile2) ) return false;        // performs the operation
                if (m_finallyBlock == nullptr )
                    return pj->Return(pile2);                   // ends the try

                pile1->SetState(-2);                            // passes final
                break;
            }
            pile1->IncState();
        }
        pc = pc->m_next;
    }
    if (m_finallyBlock != nullptr &&
         pile1->GetState() > 0 && val != 0 ) pile1->SetState(-1);// if stop then made the final

    if (pile1->GetState() <= -1)
    {
//      pile0->SetState(1);

        if (!m_finallyBlock->Execute(pile2) && pile2->IsOk()) return false;
        if (!pile2->IsOk()) return pj->Return(pile2);           // keep this exception
        pile2->SetError(pile1->GetState()==-1 ? static_cast<CBotError>(val) : CBotNoErr);       // gives the initial error
        return pj->Return(pile2);
    }

    pile1->SetState(0);                                         // returns to the evaluation
    pile0->SetState(0);                                         // returns to the evaluation
    if ( val != 0 && m_catchList == nullptr && m_finallyBlock == nullptr )
                            return pj->Return(pile2);           // ends the try without exception

    pile1->SetError(static_cast<CBotError>(val));                                       // gives the error
    return false;                                               // it's not for us
}

////////////////////////////////////////////////////////////////////////////////
void CBotTry::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    int     val;
    CBotStack* pile1 = pj->RestoreStack(this);  // adds an item to the stack
    if ( pile1 == nullptr ) return;
                                                    // or find in case of recovery
    CBotStack* pile0 = pj->AddStack2();             // adds an item to the secondary stack
    if ( pile0 == nullptr ) return;

    CBotStack* pile2 = pile0->RestoreStack();
    if ( pile2 == nullptr ) return;

    m_block->RestoreState(pile1, bMain);
    if ( pile0->GetState() == 0 )
    {
        return;
    }

    // there was an interruption
    // see what it returns

    CBotCatch*  pc = m_catchList;
    int state = pile1->GetState();                              // where were we ?
    val = pile2->GetState();                                    // what error ?

    if ( val >= 0 && state > 0 ) while ( pc != nullptr )
    {
        if ( --state <= 0 )
        {
            // ask to the catch block if it feels concerned
            pc->RestoreCondState(pile2, bMain);     // suspend !
            return;
        }
        if ( --state <= 0 )
        {
            if (pile2->GetVal() != 0)
            {
                pc->RestoreState(pile2, bMain);         // execute the operation
                return;
            }
        }
        pc = pc->m_next;
    }

    if (pile1->GetState() <= -1)
    {
        m_finallyBlock->RestoreState(pile2, bMain);
        return;
    }
}

std::map<std::string, CBotInstr*> CBotTry::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_block"] = m_block;
    links["m_catchList"] = m_catchList;
    links["m_finallyBlock"] = m_finallyBlock;
    return links;
}

} // namespace CBot
