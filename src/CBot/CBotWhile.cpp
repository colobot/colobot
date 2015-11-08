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

///////////////////////////////////////////////////////////////////////
// This file defined the following statements:
//      CBotWhile   "while (condition) {instructions}"
//      CBotDo      "do {instructions} while (condition)"
//      CBotFor     "for (init, condition, incr) {instructions}"
//      CBotSwitch  "switch (val) {instructions}"
//      CBotCase    "case val:"
//      CBotBreak   "break", "break label", "continu", "continu label"
//      CBotTry     "try {instructions}"
//      CBotCatch   "catch (condition) {instructions}" or "finally"
//      CBotThrow   "throw execption"


#include "CBot.h"

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile an instruction "while"

CBotWhile::CBotWhile()
{
    m_Condition =
    m_Block     = nullptr;     // nullptr so that delete is not possible further
    name = "CBotWhile";     // debug
}

CBotWhile::~CBotWhile()
{
    delete  m_Condition;    // frees the condition
    delete  m_Block;        // releases the block instruction
}

CBotInstr* CBotWhile::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotWhile*  inst = new CBotWhile();         // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GetString();        // records the name of the label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_WHILE)) return nullptr;    // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp
                                                // a bit of battery please (??)

    if ( nullptr != (inst->m_Condition = CBotCondition::Compile( p, pStk )) )
    {
        // the condition exists

        IncLvl(inst->m_label);
        inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, true );
        DecLvl();

        if ( pStk->IsOk() )
        {
            // the statement block is ok (it may be empty!

            return pStack->Return(inst, pStk);  // return an object to the application
                                                // makes the object to which the application
        }
    }

    delete inst;                                // error, frees the place
    return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
}

// executes a "while" instruction

bool CBotWhile :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);   // adds an item to the stack
                                            // or find in case of recovery
//  if ( pile == EOX ) return true;

    if ( pile->IfStep() ) return false;

    while( true ) switch( pile->GetState() )    // executes the loop
    {                                           // there are two possible states (depending on recovery)
    case 0:
        // evaluates the condition
        if ( !m_Condition->Execute(pile) ) return false; // interrupted here?

        // the result of the condition is on the stack

        // terminates if an error or if the condition is false
        if ( !pile->IsOk() || pile->GetVal() != true )
        {
            return pj->Return(pile);                    // sends the results and releases the stack
        }

        // the condition is true, pass in the second mode

        if (!pile->SetState(1)) return false;           // ready for further

    case 1:
        // evaluates the associated statement block
        if ( m_Block != nullptr &&
            !m_Block->Execute(pile) )
        {
            if (pile->IfContinue(0, m_label)) continue; // if continued, will return to test
            return pj->BreakReturn(pile, m_label);      // sends the results and releases the stack
        }

        // terminates if there is an error
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // sends the results and releases the stack
        }

        // returns to the test again
        if (!pile->SetState(0, 0)) return false;
        continue;
    }
}

void CBotWhile :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;
    CBotStack* pile = pj->RestoreStack(this);   // adds an item to the stack
    if ( pile == nullptr ) return;

    switch( pile->GetState() )
    {                                           // there are two possible states (depending on recovery)
    case 0:
        // evaluates the condition
        m_Condition->RestoreState(pile, bMain);
        return;

    case 1:
        // evaluates the associated statement block
        if ( m_Block != nullptr ) m_Block->RestoreState(pile, bMain);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////
// compiles instruction "catch"

CBotCatch::CBotCatch()
{
    m_Cond      =
    m_Block     = nullptr;     // nullptr so that delete is not possible further
    m_next      = nullptr;

    name = "CBotCatch";     // debug
}

CBotCatch::~CBotCatch()
{
    delete  m_Cond;         // frees the list
    delete  m_Block;        // frees the instruction block
    delete  m_next;         // and subsequent
}

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
            pStack->SetError(TX_CLOSEPAR, p->GetStart());
        }
        pStack->SetError(TX_BADTYPE, p->GetStart());
    }
    pStack->SetError(TX_OPENPAR, p->GetStart());
    delete inst;                                // error, frees up
    return nullptr;                                // no object, the error is on the stack
}

// execution of "catch"

bool CBotCatch :: Execute(CBotStack* &pj)
{
    if ( m_Block == nullptr ) return true;
    return m_Block->Execute(pj);                // executes the associated block
}

void CBotCatch :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( bMain && m_Block != nullptr ) m_Block->RestoreState(pj, bMain);
}

void CBotCatch :: RestoreCondState(CBotStack* &pj, bool bMain)
{
    m_Cond->RestoreState(pj, bMain);
}

// routine to see if the catch is to do or not

bool CBotCatch :: TestCatch(CBotStack* &pile, int val)
{
    if ( !m_Cond->Execute(pile) ) return false;

    if ( val > 0 || pile->GetType() != CBotTypBoolean )
    {
        CBotVar* var = CBotVar::Create(static_cast<CBotToken*>(nullptr), CBotTypBoolean);
        var->SetValInt( pile->GetVal() == val );
        pile->SetVar(var);                          // calls on the stack
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compiles instruction "throw"

CBotThrow::CBotThrow()
{
    m_Value     = nullptr;     // nullptr so that delete is not possible further

    name = "CBotThrow";     // debug
}

CBotThrow::~CBotThrow()
{
    delete  m_Value;
}

CBotInstr* CBotThrow::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GetStart());

    CBotThrow*  inst = new CBotThrow();         // creates the object
    inst->SetToken(p);

    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    if (!IsOfType(p, ID_THROW)) return nullptr;    // should never happen

    inst->m_Value = CBotExpression::Compile( p, pStack );

    if (pStack->GetType() < CBotTypLong && pStack->IsOk())
    {
        return inst;                            // return an object to the application
    }
    pStack->SetError(TX_BADTYPE, pp);

    delete inst;                                // error, frees up
    return nullptr;                                // no object, the error is on the stack
}

// execution of instruction "throw"

bool CBotThrow :: Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return true;

    if ( pile->GetState() == 0 )
    {
        if ( !m_Value->Execute(pile) ) return false;
        pile->IncState();
    }

    if ( pile->IfStep() ) return false;

    int val = pile->GetVal();
    if ( val < 0 ) val = TX_BADTHROW;
    pile->SetError( val, &m_token );
    return pj->Return( pile );
}

void CBotThrow :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == nullptr ) return;

    if ( pile->GetState() == 0 )
    {
        m_Value->RestoreState(pile, bMain);
        return;
    }
}


