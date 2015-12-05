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
#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotParExpr.h"
#include "CBot/CBotInstr/CBotLogicExpr.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include
#include <cassert>

#define    MAX(a,b)    ((a>b) ? a : b)

////////////////////////////////////////////////////////////////////////////////
CBotTwoOpExpr::CBotTwoOpExpr()
{
    m_leftop    =
    m_rightop   = nullptr;         // nullptr to be able to delete without other
    name = "CBotTwoOpExpr";     // debug
}

////////////////////////////////////////////////////////////////////////////////
CBotTwoOpExpr::~CBotTwoOpExpr()
{
    delete  m_leftop;
    delete  m_rightop;
}

// type of operands accepted by operations
#define     ENTIER      ((1<<CBotTypByte)|(1<<CBotTypShort)|(1<<CBotTypChar)|(1<<CBotTypInt)|(1<<CBotTypLong))
#define     FLOTANT     ((1<<CBotTypFloat)|(1<<CBotTypDouble))
#define     BOOLEEN     (1<<CBotTypBoolean)
#define     CHAINE      (1<<CBotTypString)
#define     POINTER     (1<<CBotTypPointer)
#define     INSTANCE    (1<<CBotTypClass)

// list of operations (précéance)
//  acceptable type, operand
//  zero ends level \TODO précéance

static int  ListOp[] =
{
    BOOLEEN,                ID_LOGIC, 0,
    BOOLEEN,                ID_TXT_OR,
    BOOLEEN,                ID_LOG_OR, 0,
    BOOLEEN,                ID_TXT_AND,
    BOOLEEN,                ID_LOG_AND, 0,
    BOOLEEN|ENTIER,         ID_OR, 0,
    BOOLEEN|ENTIER,         ID_XOR, 0,
    BOOLEEN|ENTIER,         ID_AND, 0,
    BOOLEEN|ENTIER|FLOTANT
                  |CHAINE
                  |POINTER
                  |INSTANCE,ID_EQ,
    BOOLEEN|ENTIER|FLOTANT
                  |CHAINE
                  |POINTER
                  |INSTANCE,ID_NE, 0,
    ENTIER|FLOTANT|CHAINE,  ID_HI,
    ENTIER|FLOTANT|CHAINE,  ID_LO,
    ENTIER|FLOTANT|CHAINE,  ID_HS,
    ENTIER|FLOTANT|CHAINE,  ID_LS, 0,
    ENTIER,                 ID_SR,
    ENTIER,                 ID_SL,
    ENTIER,                 ID_ASR, 0,
    ENTIER|FLOTANT|CHAINE,  ID_ADD,
    ENTIER|FLOTANT,         ID_SUB, 0,
    ENTIER|FLOTANT,         ID_MUL,
    ENTIER|FLOTANT,         ID_DIV,
    ENTIER|FLOTANT,         ID_MODULO, 0,
    ENTIER|FLOTANT,         ID_POWER, 0,
    0,
};

bool IsInList( int val, int* list, int& typemasque )
{
    while (true)
    {
        if ( *list == 0 ) return false;
        typemasque = *list++;
        if ( *list++ == val ) return true;
    }
}

bool TypeOk( int type, int test )
{
    while (true)
    {
        if ( type == 0 ) return (test & 1);
        type--; test /= 2;
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotTwoOpExpr::Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations)
{
    int typemasque;

    if ( pOperations == nullptr ) pOperations = ListOp;
    int* pOp = pOperations;
    while ( *pOp++ != 0 );              // follows the table

    CBotCStack* pStk = pStack->TokenStack();                    // one end of stack please

    // search the intructions that may be suitable to the left of the operation
    CBotInstr*  left = (*pOp == 0) ?
                        CBotParExpr::Compile( p, pStk ) :       // expression (...) left
                        CBotTwoOpExpr::Compile( p, pStk, pOp ); // expression A * B left

    if (left == nullptr) return pStack->Return(nullptr, pStk);        // if error,  transmit

    // did we expected the operand?
    int TypeOp = p->GetType();
    if ( IsInList( TypeOp, pOperations, typemasque ) )
    {
        CBotTypResult    type1, type2;
        type1 = pStk->GetTypResult();                           // what kind of the first operand?

        if ( TypeOp == ID_LOGIC )       // special case provided for: ? op1: op2;
        {
            if ( !type1.Eq(CBotTypBoolean) )
            {
                pStk->SetError( TX_BADTYPE, p);
                return pStack->Return(nullptr, pStk);
            }
            CBotLogicExpr* inst = new CBotLogicExpr();
            inst->m_condition = left;

            p = p->GetNext();                                       // skip the token of the operation
            inst->m_op1 = CBotExpression::Compile(p, pStk);
            CBotToken* pp = p;
            if ( inst->m_op1 == nullptr || !IsOfType( p, ID_DOTS ) )
            {
                pStk->SetError( TX_MISDOTS, p->GetStart());
                delete inst;
                return pStack->Return(nullptr, pStk);
            }
            type1 = pStk->GetTypResult();

            inst->m_op2 = CBotExpression::Compile(p, pStk);
            if ( inst->m_op2 == nullptr )
            {
                pStk->SetError( TX_ENDOF, p->GetStart() );
                delete inst;
                return pStack->Return(nullptr, pStk);
            }
            type2 = pStk->GetTypResult();
            if (!TypeCompatible(type1, type2))
            {
                pStk->SetError( TX_BAD2TYPE, pp );
                delete inst;
                return pStack->Return(nullptr, pStk);
            }

            pStk->SetType(type1);       // the greatest of 2 types

            return pStack->Return(inst, pStk);
        }

        CBotTwoOpExpr* inst = new CBotTwoOpExpr();              // element for operation
        inst->SetToken(p);                                      // stores the operation


        p = p->GetNext();                                           // skip the token of the operation

        // looking statements that may be suitable for right

        if ( nullptr != (inst->m_rightop = CBotTwoOpExpr::Compile( p, pStk, pOp )) )
                                                                // expression (...) right
        {
            // there is an second operand acceptable

            type2 = pStk->GetTypResult();                       // what kind of results?

            // what kind of result?
            int TypeRes = MAX( type1.GetType(3), type2.GetType(3) );
            if ( TypeOp == ID_ADD && type1.Eq(CBotTypString) )
            {
                TypeRes = CBotTypString;
                type2 = type1;  // any type convertible chain
            }
            else if ( TypeOp == ID_ADD && type2.Eq(CBotTypString) )
            {
                TypeRes = CBotTypString;
                type1 = type2;  // any type convertible chain
            }
            else if (!TypeOk( TypeRes, typemasque )) type1.SetType(99);// error of type

            switch ( TypeOp )
            {
            case ID_LOG_OR:
            case ID_LOG_AND:
            case ID_TXT_OR:
            case ID_TXT_AND:
            case ID_EQ:
            case ID_NE:
            case ID_HI:
            case ID_LO:
            case ID_HS:
            case ID_LS:
                TypeRes = CBotTypBoolean;
            }
            if ( TypeCompatible (type1, type2, TypeOp ) )               // the results are compatible
            {
                // ok so, saves the operand in the object
                inst->m_leftop = left;

                // special for evaluation of the operations of the same level from left to right
                while ( IsInList( p->GetType(), pOperations, typemasque ) ) // same operation(s) follows?
                {
                    TypeOp = p->GetType();
                    CBotTwoOpExpr* i = new CBotTwoOpExpr();             // element for operation
                    i->SetToken(p);                                     // stores the operation
                    i->m_leftop = inst;                                 // left operand
                    type1 = TypeRes;

                    p = p->GetNext();                                       // advance after
                    i->m_rightop = CBotTwoOpExpr::Compile( p, pStk, pOp );
                    type2 = pStk->GetTypResult();

                    if ( !TypeCompatible (type1, type2, TypeOp) )       // the results are compatible
                    {
                        pStk->SetError(TX_BAD2TYPE, &i->m_token);
                        delete i;
                        return pStack->Return(nullptr, pStk);
                    }

                    if ( TypeRes != CBotTypString )
                        TypeRes = MAX(type1.GetType(), type2.GetType());
                    inst = i;
                }

                CBotTypResult t(type1);
                    t.SetType(TypeRes);
                // is a variable on the stack for the type of result
                pStk->SetVar(CBotVar::Create(static_cast<CBotToken*>(nullptr), t));

                // and returns the requested object
                return pStack->Return(inst, pStk);
            }
            pStk->SetError(TX_BAD2TYPE, &inst->m_token);
        }

        // in case of error, releases the elements
        delete left;
        delete inst;
        // and transmits the error to the stack
        return pStack->Return(nullptr, pStk);
    }

    // if we are not dealing with an operation + or -
    // goes to that requested, the operand (left) found
    // instead of the object "addition"
    return pStack->Return(left, pStk);
}


bool VarIsNAN(const CBotVar* var)
{
    return var->GetInit() > CBotVar::InitType::DEF;
}

bool IsNan(CBotVar* left, CBotVar* right, int* err = nullptr)
{
    if ( VarIsNAN(left) || VarIsNAN(right) )
    {
        if ( err != nullptr ) *err = TX_OPNAN ;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotTwoOpExpr::Execute(CBotStack* &pStack)
{
    CBotStack* pStk1 = pStack->AddStack(this);  // adds an item to the stack
                                                // or return in case of recovery
//  if ( pStk1 == EOX ) return true;

    // according to recovery, it may be in one of two states

    if ( pStk1->GetState() == 0 )                   // first state, evaluates the left operand
    {
        if (!m_leftop->Execute(pStk1) ) return false;   // interrupted here?

        // for OR and AND logic does not evaluate the second expression if not necessary
        if ( (GetTokenType() == ID_LOG_AND || GetTokenType() == ID_TXT_AND ) && pStk1->GetVal() == false )
        {
            CBotVar*    res = CBotVar::Create( static_cast<CBotToken*>(nullptr), CBotTypBoolean);
            res->SetValInt(false);
            pStk1->SetVar(res);
            return pStack->Return(pStk1);               // transmits the result
        }
        if ( (GetTokenType() == ID_LOG_OR||GetTokenType() == ID_TXT_OR) && pStk1->GetVal() == true )
        {
            CBotVar*    res = CBotVar::Create( static_cast<CBotToken*>(nullptr), CBotTypBoolean);
            res->SetValInt(true);
            pStk1->SetVar(res);
            return pStack->Return(pStk1);               // transmits the result
        }

        // passes to the next step
        pStk1->SetState(1);         // ready for further
    }


    // requires a little more stack to avoid touching the result
    // of which is left on the stack, precisely

    CBotStack* pStk2 = pStk1->AddStack();               // adds an item to the stack
                                                        // or return in case of recovery

    // 2e état, évalue l'opérande de droite
    if ( pStk2->GetState() == 0 )
    {
        if ( !m_rightop->Execute(pStk2) ) return false;     // interrupted here?
        pStk2->IncState();
    }

    CBotTypResult       type1 = pStk1->GetTypResult();      // what kind of results?
    CBotTypResult       type2 = pStk2->GetTypResult();

    CBotStack* pStk3 = pStk2->AddStack(this);               // adds an item to the stack
    if ( pStk3->IfStep() ) return false;                    // shows the operation if step by step

    // creates a temporary variable to put the result
    // what kind of result?
    int TypeRes = MAX(type1.GetType(), type2.GetType());

    if ( GetTokenType() == ID_ADD && type1.Eq(CBotTypString) )
    {
        TypeRes = CBotTypString;
    }

    switch ( GetTokenType() )
    {
    case ID_LOG_OR:
    case ID_LOG_AND:
    case ID_TXT_OR:
    case ID_TXT_AND:
    case ID_EQ:
    case ID_NE:
    case ID_HI:
    case ID_LO:
    case ID_HS:
    case ID_LS:
        TypeRes = CBotTypBoolean;
        break;
    case ID_DIV:
        TypeRes = MAX(TypeRes, CBotTypFloat);
    }

    // creates a variable for the result
    CBotVar*    result = CBotVar::Create( static_cast<CBotToken*>(nullptr), TypeRes);

    // creates a variable to perform the calculation in the appropriate type
    TypeRes = MAX(type1.GetType(), type2.GetType());

    if ( GetTokenType() == ID_ADD && type1.Eq(CBotTypString) )
    {
        TypeRes = CBotTypString;
    }

    CBotVar*    temp;

    if ( TypeRes == CBotTypPointer ) TypeRes = CBotTypNullPointer;
    if ( TypeRes == CBotTypClass ) temp = CBotVar::Create( static_cast<CBotToken*>(nullptr), CBotTypResult(CBotTypIntrinsic, type1.GetClass() ) );
    else                           temp = CBotVar::Create( static_cast<CBotToken*>(nullptr), TypeRes );

    int err = 0;
    // is a operation according to request
    CBotVar*    left  = pStk1->GetVar();
    CBotVar*    right = pStk2->GetVar();

    switch (GetTokenType())
    {
    case ID_ADD:
        if ( !IsNan(left, right, &err) )    result->Add(left , right);      // addition
        break;
    case ID_SUB:
        if ( !IsNan(left, right, &err) )    result->Sub(left , right);      // substraction
        break;
    case ID_MUL:
        if ( !IsNan(left, right, &err) )    result->Mul(left , right);      // multiplies
        break;
    case ID_POWER:
        if ( !IsNan(left, right, &err) )    result->Power(left , right);    // power
        break;
    case ID_DIV:
        if ( !IsNan(left, right, &err) )    err = result->Div(left , right);// division
        break;
    case ID_MODULO:
        if ( !IsNan(left, right, &err) )    err = result->Modulo(left , right);// remainder of division
        break;
    case ID_LO:
        if ( !IsNan(left, right, &err) )
            result->SetValInt(temp->Lo(left , right));  // lower
        break;
    case ID_HI:
        if ( !IsNan(left, right, &err) )
            result->SetValInt(temp->Hi(left , right));  // top
        break;
    case ID_LS:
        if ( !IsNan(left, right, &err) )
            result->SetValInt(temp->Ls(left , right));  // less than or equal
        break;
    case ID_HS:
        if ( !IsNan(left, right, &err) )
            result->SetValInt(temp->Hs(left , right));  // greater than or equal
        break;
    case ID_EQ:
        if ( IsNan(left, right) )
            result->SetValInt(left->GetInit() ==  right->GetInit()) ;
        else
            result->SetValInt(temp->Eq(left , right));  // equal
        break;
    case ID_NE:
        if ( IsNan(left, right) )
             result->SetValInt(left ->GetInit() !=  right->GetInit()) ;
        else
            result->SetValInt(temp->Ne(left , right));  // different
        break;
    case ID_TXT_AND:
    case ID_LOG_AND:
    case ID_AND:
        if ( !IsNan(left, right, &err) )    result->And(left , right);      // AND
        break;
    case ID_TXT_OR:
    case ID_LOG_OR:
    case ID_OR:
        if ( !IsNan(left, right, &err) )    result->Or(left , right);       // OR
        break;
    case ID_XOR:
        if ( !IsNan(left, right, &err) )    result->XOr(left , right);      // exclusive OR
        break;
    case ID_ASR:
        if ( !IsNan(left, right, &err) )    result->ASR(left , right);
        break;
    case ID_SR:
        if ( !IsNan(left, right, &err) )    result->SR(left , right);
        break;
    case ID_SL:
        if ( !IsNan(left, right, &err) )    result->SL(left , right);
        break;
    default:
        assert(0);
    }
    delete temp;

    pStk2->SetVar(result);                      // puts the result on the stack
    if ( err ) pStk2->SetError(err, &m_token);  // and the possible error (division by zero)

//  pStk1->Return(pStk2);                       // releases the stack
    return pStack->Return(pStk2);               // transmits the result
}

////////////////////////////////////////////////////////////////////////////////
void CBotTwoOpExpr::RestoreState(CBotStack* &pStack, bool bMain)
{
    if ( !bMain ) return;
    CBotStack* pStk1 = pStack->RestoreStack(this);  // adds an item to the stack
    if ( pStk1 == nullptr ) return;

    // according to recovery, it may be in one of two states

    if ( pStk1->GetState() == 0 )                   // first state, evaluates the left operand
    {
        m_leftop->RestoreState(pStk1, bMain);       // interrupted here!
        return;
    }

    CBotStack* pStk2 = pStk1->RestoreStack();           // adds an item to the stack
    if ( pStk2 == nullptr ) return;

    // second state, evaluates the right operand
    if ( pStk2->GetState() == 0 )
    {
        m_rightop->RestoreState(pStk2, bMain);          // interrupted here!
        return;
    }
}
