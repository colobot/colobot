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
#include "CBotTry.h"
#include "CBotBlock.h"

#include "CBotStack.h"
#include "CBotCStack.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotTry::CBotTry()
{
    m_ListCatch = nullptr;
    m_FinalInst =
    m_Block     = nullptr;     // nullptr so that delete is not possible further
    name = "CBotTry";       // debug
}

////////////////////////////////////////////////////////////////////////////////
CBotTry::~CBotTry()
{
    delete  m_ListCatch;    // frees the list
    delete  m_Block;        // frees the instruction block
    delete  m_FinalInst;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotTry::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotTry*    inst = new CBotTry();           // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    inst->SetToken(p);
    if (!IsOfType(p, ID_TRY)) return nullptr;      // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk );
    CBotCatch** pn = &inst->m_ListCatch;

    while (pStk->IsOk() && p->GetType() == ID_CATCH)
    {
        CBotCatch*  i = CBotCatch::Compile(p, pStk);
        *pn = i;
        pn = &i->m_next;
    }

    if (pStk->IsOk() && IsOfType( p, ID_FINALLY) )
    {
        inst->m_FinalInst = CBotBlock::CompileBlkOrInst( p, pStk );
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
        if ( m_Block->Execute(pile1) )
        {
            if ( m_FinalInst == nullptr ) return pj->Return(pile1);
            pile1->SetState(-2);                                // passes final
        }

        val = pile1->GetError();
        if ( val == 0 && CBotStack::m_initimer == 0 )           // mode step?
            return false;                                       // does not make the catch

        pile1->IncState();
        pile2->SetState(val);                                   // stores the error number
        pile1->SetError(0);                                     // for now there is are more errors!

        if ( val == 0 && CBotStack::m_initimer < 0 )            // mode step?
            return false;                                       // does not make the catch
    }

    // there was an interruption
    // see what it returns

    CBotCatch*  pc = m_ListCatch;
    int state = static_cast<short>(pile1->GetState());                       // where were we?
    val = pile2->GetState();                                    // what error?
    pile0->SetState(1);                                         // marking the GetRunPos

    if ( val >= 0 && state > 0 ) while ( pc != nullptr )
    {
        if ( --state <= 0 )
        {
            // request to the catch block if they feel concerned
            // demande au bloc catch s'il se sent concerné
            if ( !pc->TestCatch(pile2, val) ) return false;     // suspend !
            pile1->IncState();
        }
        if ( --state <= 0 )
        {
            if ( pile2->GetVal() == true )
            {
//              pile0->SetState(1);

                if ( !pc->Execute(pile2) ) return false;        // performs the operation
                if ( m_FinalInst == nullptr )
                    return pj->Return(pile2);                   // ends the try

                pile1->SetState(-2);                            // passes final
                break;
            }
            pile1->IncState();
        }
        pc = pc->m_next;
    }
    if ( m_FinalInst != nullptr &&
         pile1->GetState() > 0 && val != 0 ) pile1->SetState(-1);// if stop then made the final

    if (pile1->GetState() <= -1)
    {
//      pile0->SetState(1);

        if (!m_FinalInst->Execute(pile2) && pile2->IsOk()) return false;
        if (!pile2->IsOk()) return pj->Return(pile2);           // keep this exception
        pile2->SetError(pile1->GetState()==-1 ? val : 0);       // gives the initial error
        return pj->Return(pile2);
    }

    pile1->SetState(0);                                         // returns to the evaluation
    pile0->SetState(0);                                         // returns to the evaluation
    if ( val != 0 && m_ListCatch == nullptr && m_FinalInst == nullptr )
                            return pj->Return(pile2);           // ends the try without exception

    pile1->SetError(val);                                       // gives the error
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

    m_Block->RestoreState(pile1, bMain);
    if ( pile0->GetState() == 0 )
    {
        return;
    }

    // there was an interruption
    // see what it returns

    CBotCatch*  pc = m_ListCatch;
    int state = pile1->GetState();                              // where were we ?
    val = pile2->GetState();                                    // what error ?

    if ( val >= 0 && state > 0 ) while ( pc != nullptr )
    {
        if ( --state <= 0 )
        {
            // request to the catch block if they feel concerned
            // demande au bloc catch s'il se sent concerné
            pc->RestoreCondState(pile2, bMain);     // suspend !
            return;
        }
        if ( --state <= 0 )
        {
            if ( pile2->GetVal() == true )
            {
                pc->RestoreState(pile2, bMain);         // execute the operation
                return;
            }
        }
        pc = pc->m_next;
    }

    if (pile1->GetState() <= -1)
    {
        m_FinalInst->RestoreState(pile2, bMain);
        return;
    }
}
