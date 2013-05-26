// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

///////////////////////////////////////////////////
// expressions of type Operand1 + Operand2
//                     Operand1 - Operand2

#include "CBot.h"

// various constructors

CBotAddExpr::CBotAddExpr()
{
    m_leftop    =
    m_rightop   = NULL;         // NULL to be able to delete without further
    name = "CBotAddExpr";       // debug
}

CBotAddExpr::~CBotAddExpr()
{
    delete  m_leftop;
    delete  m_rightop;
}


// compile une instruction de type A + B

CBotInstr* CBotAddExpr::Compile(CBotToken* &p, CBotStack* pStack)
{
    CBotStack* pStk = pStack->TokenStack();                 // one end of stack please

    // looking statements that may be suitable to the left of the operation + or -

    CBotInstr*  left = CBotMulExpr::Compile( p, pStk );     // expression A * B left
    if (left == NULL) return pStack->Return(NULL, pStk);    // if error, transmit

    // do we have the token + or - next?

    if ( p->GetType() == ID_ADD ||
         p->GetType() == ID_SUB)                            // more or less
    {
        CBotAddExpr* inst = new CBotAddExpr();              // element for operation
        inst->SetToken(p);                                  // stores the operation

        int          type1, type2;
        type1 = pStack->GetType();                          // what kind of the first operand?

        p = p->Next();                                      // skip the token of the operation

        // looking statements that may be suitable for right

        if ( NULL != (inst->m_rightop = CBotAddExpr::Compile( p, pStk )) )  // expression (...) rigth
        {
            // there is an acceptable second operand

            type2 = pStack->GetType();                      // what kind of results?

            if ( type1 == type2 )                           // are the results consistent ?
            {
                // ok so, saves the operand in the object
                inst->m_leftop = left;
                // and makes the object on demand
                return pStack->Return(inst, pStk);
            }
        }

        // in case of error, free the elements
        delete left;
        delete inst;
        // and transmits the error that is on the stack
        return pStack->Return(NULL, pStk);
    }

    // if we are not dealing with an operation + or -
    // goes to that requested, the operand (left) found
    // place the object "addition"
    return pStack->Return(left, pStk);
}




// operation is addition or subtraction

bool CBotAddExpr::Execute(CBotStack* &pStack)
{
    CBotStack* pStk1 = pStack->AddStack(this);  // adds an item to the stack
                                                // or is found in case of recovery
//  if ( pSk1 == EOX ) return TRUE;


    // according to recovery, it may be in one of two states

    if ( pStk1->GetState() == 0 &&              // first state, evaluates the left operand
        !m_leftop->Execute(pStk1) ) return FALSE; // interrupted here?

    // passes to the next step
    pStk1->SetState(1);                         // ready for further

    // requires a little more stack to not touch the result of the left
    // which is on the stack, precisely.

    CBotStack* pStk2 = pStk1->AddStack();       // adds an item to the stack
                                                // or is found in case of recovery

    // Second state, evaluates the right operand
    if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrupted here?

    int     type1 = pStk1->GetType();           // what kind of results?
    int     type2 = pStk2->GetType();

    // creates a temporary variable to put the result
    CBotVar*    result = new CBotVar( NULL, MAX(type1, type2));

    // is the operation as requested
    switch (GetTokenType())
    {
    case ID_ADD:
        result->Add(pStk1->GetVar(), pStk2->GetVar());      // addition
        break;
    case ID_SUB:
        result->Sub(pStk1->GetVar(), pStk2->GetVar());      // subtraction
        break;
    }
    pStk2->SetVar(result);                      // puts the result on the stack

    pStk1->Return(pStk2);                       // frees the stack
    return pStack->Return(pStk1);               // transmits the result
}

