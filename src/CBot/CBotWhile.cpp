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
// compiles instruction "switch"

CBotSwitch::CBotSwitch()
{
    m_Value     =
    m_Block     = nullptr;         // nullptr so that delete is not possible further
    name = "CBotSwitch";        // debug
}

CBotSwitch::~CBotSwitch()
{
    delete  m_Value;        // frees the value
    delete  m_Block;        // frees the instruction block
}


CBotInstr* CBotSwitch::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotSwitch* inst = new CBotSwitch();        // creates the object
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)

    inst->SetToken(p);
    if (!IsOfType(p, ID_SWITCH)) return nullptr;   // should never happen

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    if ( IsOfType(p, ID_OPENPAR ) )
    {
        if ( nullptr != (inst->m_Value = CBotExpression::Compile( p, pStk )) )
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
                                if ( inst->m_Block == nullptr ) inst->m_Block = i;
                                else inst->m_Block->AddNext(i);
                                continue;
                            }

                            if ( inst->m_Block == nullptr )
                            {
                                pStk->SetError(TX_NOCASE, p->GetStart());
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }

                            CBotInstr* i = CBotBlock::CompileBlkOrInst( p, pStk, true );
                            if ( !pStk->IsOk() )
                            {
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }
                            inst->m_Block->AddNext(i);

                            if ( p == nullptr )
                            {
                                pStk->SetError(TX_CLOSEBLK, -1);
                                delete inst;
                                return pStack->Return(nullptr, pStk);
                            }
                        }
                        DecLvl();

                        if ( inst->m_Block == nullptr )
                        {
                            pStk->SetError(TX_NOCASE, p->GetStart());
                            delete inst;
                            return pStack->Return(nullptr, pStk);
                        }
                        // the statement block is ok
                        return pStack->Return(inst, pStk);  // return an object to the application
                    }
                    pStk->SetError( TX_OPENBLK, p->GetStart() );
                }
                pStk->SetError( TX_CLOSEPAR, p->GetStart() );
            }
            pStk->SetError( TX_BADTYPE, p->GetStart() );
        }
    }
    pStk->SetError( TX_OPENPAR, p->GetStart());

    delete inst;                                // error, frees up
    return pStack->Return(nullptr, pStk);          // no object, the error is on the stack
}

// executes instruction "switch"

bool CBotSwitch :: Execute(CBotStack* &pj)
{
    CBotStack* pile1 = pj->AddStack(this);      // adds an item to the stack
//  if ( pile1 == EOX ) return true;

    CBotInstr*  p = m_Block;                    // first expression

    int     state = pile1->GetState();
    if (state == 0)
    {
        if ( !m_Value->Execute(pile1) ) return false;
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

    p = m_Block;                                        // returns to the beginning
    while (state-->0) p = p->GetNext();                 // advance in the list

    while( p != nullptr )
    {
        if ( !p->Execute(pile1) ) return pj->BreakReturn(pile1);
        if ( !pile1->IncState() ) return false;
        p = p->GetNext();
    }
    return pj->Return(pile1);
}

void CBotSwitch :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack* pile1 = pj->RestoreStack(this);  // adds an item to the stack
    if ( pile1 == nullptr ) return;

    CBotInstr*  p = m_Block;                    // first expression

    int     state = pile1->GetState();
    if (state == 0)
    {
        m_Value->RestoreState(pile1, bMain);
        return;
    }

    if ( state == -1 )
    {
        return;
    }

//  p = m_Block;                                // returns to the beginning
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

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compiles instruction "case"
// we are bound to the statement block "switch"

CBotCase::CBotCase()
{
    m_Value     = nullptr;     // nullptr so that delete is not possible further
    name = "CBotCase";      // debug
}

CBotCase::~CBotCase()
{
    delete  m_Value;        // frees the value
}


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

// execution of instruction "case"

bool CBotCase::Execute(CBotStack* &pj)
{
    return true;                                // the "case" statement does nothing!
}

void CBotCase::RestoreState(CBotStack* &pj, bool bMain)
{
}

// routine to find the entry point of "case"
// corresponding to the value seen

bool CBotCase::CompCase(CBotStack* &pile, int val)
{
    if ( m_Value == nullptr ) return true;         // "default" case

    while (!m_Value->Execute(pile));            // puts the value on the correspondent stack (without interruption)
    return (pile->GetVal() == val);             // compared with the given value
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compiles instruction "break" or "continu"

CBotBreak::CBotBreak()
{
    name = "CBotBreak";     // debug
}

CBotBreak::~CBotBreak()
{
}

CBotInstr* CBotBreak::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;                         // preserves at the ^ token (starting position)
    int type = p->GetType();

    if (!IsOfType(p, ID_BREAK, ID_CONTINUE)) return nullptr;   // should never happen

    if ( !ChkLvl(CBotString(), type ) )
    {
        pStack->SetError(TX_BREAK, pp);
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
            pStack->SetError(TX_NOLABEL, pp);
            return nullptr;                            // no object, the error is on the stack
        }
    }

    if (IsOfType(p, ID_SEP))
    {
        return  inst;                           // return what it wants
    }
    delete inst;

    pStack->SetError(TX_ENDOF, p->GetStart());
    return nullptr;                            // no object, the error is on the stack
}

// execution of statement "break" or "continu"

bool CBotBreak :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);
//  if ( pile == EOX ) return true;

    if ( pile->IfStep() ) return false;

    pile->SetBreak(m_token.GetType()==ID_BREAK ? 1 : 2, m_label);
    return pj->Return(pile);
}

void CBotBreak :: RestoreState(CBotStack* &pj, bool bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}


///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compiles instruction "try"

CBotTry::CBotTry()
{
    m_ListCatch = nullptr;
    m_FinalInst =
    m_Block     = nullptr;     // nullptr so that delete is not possible further
    name = "CBotTry";       // debug
}

CBotTry::~CBotTry()
{
    delete  m_ListCatch;    // frees the list
    delete  m_Block;        // frees the instruction block
    delete  m_FinalInst;
}

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

// execution of instruction Try
// manages the return of exceptions
// stops (judgements) by suspension
// and "finally"

bool CBotTry :: Execute(CBotStack* &pj)
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


void CBotTry :: RestoreState(CBotStack* &pj, bool bMain)
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

///////////////////////////////////////////////////////////////////////////

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


