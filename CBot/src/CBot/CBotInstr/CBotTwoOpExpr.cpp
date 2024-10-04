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

#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotParExpr.h"
#include "CBot/CBotInstr/CBotLogicExpr.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <cassert>
#include <cmath>
#include <algorithm>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotTwoOpExpr::CBotTwoOpExpr()
{
    m_leftop    = nullptr;
    m_rightop   = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotTwoOpExpr::~CBotTwoOpExpr()
{
    delete  m_leftop;
    delete  m_rightop;
}

// This list contains all possible operations
// They are sorted in reversed order of precedence (the ones that get executed first are at the end),
// the operations have equal precedence until 0 marker
// The entries are made of pairs: bitmask of acceptable parameters type and operand token

#define     INTEGER     ((1<<CBotTypByte)|(1<<CBotTypShort)|(1<<CBotTypChar)|(1<<CBotTypInt)|(1<<CBotTypLong))
#define     FLOAT       ((1<<CBotTypFloat)|(1<<CBotTypDouble))
#define     BOOLEAN     (1<<CBotTypBoolean)
#define     STRING      (1<<CBotTypString)
#define     POINTER     (1<<CBotTypPointer)
#define     INSTANCE    (1<<CBotTypClass)

static int  ListOp[] =
{
    BOOLEAN, ID_LOGIC,
    0,

    BOOLEAN, ID_TXT_OR,
    BOOLEAN, ID_LOG_OR,
    0,

    BOOLEAN, ID_TXT_AND,
    BOOLEAN, ID_LOG_AND,
    0,

    BOOLEAN | INTEGER, ID_OR,
    0,

    BOOLEAN | INTEGER, ID_XOR,
    0,

    BOOLEAN | INTEGER, ID_AND,
    0,

    BOOLEAN | INTEGER | FLOAT | STRING | POINTER | INSTANCE, ID_EQ,
    BOOLEAN | INTEGER | FLOAT | STRING | POINTER | INSTANCE, ID_NE,
    0,

    INTEGER | FLOAT | STRING, ID_HI,
    INTEGER | FLOAT | STRING, ID_LO,
    INTEGER | FLOAT | STRING, ID_HS,
    INTEGER | FLOAT | STRING, ID_LS,
    0,

    INTEGER, ID_SR,
    INTEGER, ID_SL,
    INTEGER, ID_ASR,
    0,

    INTEGER | FLOAT | STRING, ID_ADD,
    INTEGER | FLOAT, ID_SUB,
    0,

    INTEGER | FLOAT, ID_MUL,
    INTEGER | FLOAT, ID_DIV,
    INTEGER | FLOAT, ID_MODULO,
    0,

    INTEGER | FLOAT, ID_POWER,
    0,

    0, // end of list
};

static bool IsInList(int val, int* list, int& typeMask)
{
    while (true)
    {
        if ( *list == 0 ) return false;
        typeMask = *list++;
        if ( *list++ == val ) return true;
    }
}

static bool TypeOk(int type, int test)
{
    while (true)
    {
        if ( type == 0 ) return (test & 1) != 0;
        type--; test >>= 1;
    }
}

CBotInstr* CBotTwoOpExpr::Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations, bool bConstExpr)
{
    int typeMask;

    if ( pOperations == nullptr ) pOperations = ListOp;
    if ( *pOperations == 0 ) return CBotParExpr::Compile(p, pStack, bConstExpr);
    int* pOp = pOperations;
    while ( *pOp++ != 0 );              // follows the table

    CBotCStack* pStk = pStack->TokenStack();                    // one end of stack please

    // search the intructions that may be suitable to the left of the operation
    CBotInstr*  left = CBotTwoOpExpr::Compile(p, pStk, pOp, bConstExpr);

    if (left == nullptr) return pStack->Return(nullptr, pStk);        // if error,  transmit

    // did we expected the operand?
    int typeOp = p->GetType();
    if ( IsInList(typeOp, pOperations, typeMask) )
    {
        CBotTypResult    type1, type2;
        type1 = pStk->GetTypResult();                           // what kind of the first operand?

        if (!bConstExpr && typeOp == ID_LOGIC) // special case provided for: ? op1: op2;
        {
            if ( !type1.Eq(CBotTypBoolean) )
            {
                pStk->SetError( CBotErrBadType1, p);
                return pStack->Return(nullptr, pStk);
            }
            CBotLogicExpr* inst = new CBotLogicExpr();
            inst->m_condition = left;

            p = p->GetNext();                                       // skip the token of the operation
            inst->m_op1 = CBotExpression::Compile(p, pStk);
            CBotToken* pp = p;
            if ( inst->m_op1 == nullptr || !IsOfType( p, ID_DOTS ) )
            {
                pStk->SetError( CBotErrNoDoubleDots, p->GetStart());
                delete inst;
                return pStack->Return(nullptr, pStk);
            }
            type1 = pStk->GetTypResult();

            inst->m_op2 = CBotExpression::Compile(p, pStk);
            if ( inst->m_op2 == nullptr )
            {
                pStk->SetError( CBotErrNoTerminator, p->GetStart() );
                delete inst;
                return pStack->Return(nullptr, pStk);
            }
            type2 = pStk->GetTypResult();
            if (!TypeCompatible(type1, type2))
            {
                pStk->SetError( CBotErrBadType2, pp );
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

        if ( nullptr != (inst->m_rightop = CBotTwoOpExpr::Compile(p, pStk, pOp, bConstExpr)) )
                                                                // expression (...) right
        {
            // there is an second operand acceptable

            type2 = pStk->GetTypResult();                       // what kind of results?

            if ( type1.Eq(99) || type2.Eq(99) )                 // operand is void
            {
                pStack->SetError(CBotErrBadType2, &inst->m_token);
                delete inst;
                return nullptr;
            }

            // what kind of result?
            int TypeRes = std::max( type1.GetType(CBotTypResult::GetTypeMode::NULL_AS_POINTER), type2.GetType(CBotTypResult::GetTypeMode::NULL_AS_POINTER) );
            if (typeOp == ID_ADD && type1.Eq(CBotTypString))
            {
                TypeRes = CBotTypString;
                type2 = type1;  // any type convertible chain
            }
            else if (typeOp == ID_ADD && type2.Eq(CBotTypString))
            {
                TypeRes = CBotTypString;
                type1 = type2;  // any type convertible chain
            }
            else if (!TypeOk(TypeRes, typeMask)) type1.SetType(99);// error of type

            switch (typeOp)
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
            if ( TypeCompatible (type1, type2, typeOp) )               // the results are compatible
            {
                // ok so, saves the operand in the object
                inst->m_leftop = left;

                // special for evaluation of the operations of the same level from left to right
                while ( IsInList(p->GetType(), pOperations, typeMask) ) // same operation(s) follows?
                {
                    typeOp = p->GetType();
                    CBotTwoOpExpr* i = new CBotTwoOpExpr();             // element for operation
                    i->SetToken(p);                                     // stores the operation
                    i->m_leftop = inst;                                 // left operand
                    type1 = TypeRes;

                    p = p->GetNext();                                       // advance after
                    i->m_rightop = CBotTwoOpExpr::Compile(p, pStk, pOp, bConstExpr);
                    type2 = pStk->GetTypResult();

                    if ( !TypeCompatible (type1, type2, typeOp) )       // the results are compatible
                    {
                        pStk->SetError(CBotErrBadType2, &i->m_token);
                        delete i;
                        return pStack->Return(nullptr, pStk);
                    }

                    if ( TypeRes != CBotTypString )                     // keep string conversion
                        TypeRes = std::max(type1.GetType(), type2.GetType());
                    inst = i;
                }

                CBotTypResult t(type1);
                    t.SetType(TypeRes);
                // is a variable on the stack for the type of result
                pStk->SetVar(CBotVar::Create("", t));

                // and returns the requested object
                return pStack->Return(inst, pStk);
            }
            pStk->SetError(CBotErrBadType2, &inst->m_token);
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


static bool VarIsNAN(const CBotVar* var)
{
    if (var->GetType() == CBotTypFloat)
        return std::isnan(var->GetValFloat());

    if (var->GetType() == CBotTypDouble)
        return std::isnan(var->GetValDouble());

    return false;
}

static bool IsNan(CBotVar* left, CBotVar* right, CBotError* err = nullptr)
{
    if ( VarIsNAN(left) || VarIsNAN(right) )
    {
        if ( err != nullptr ) *err = CBotErrNan ;
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
            CBotVar*    res = CBotVar::Create("", CBotTypBoolean);
            res->SetValInt(false);
            pStk1->SetVar(res);
            return pStack->Return(pStk1);               // transmits the result
        }
        if ( (GetTokenType() == ID_LOG_OR||GetTokenType() == ID_TXT_OR) && pStk1->GetVal() == true )
        {
            CBotVar*    res = CBotVar::Create("", CBotTypBoolean);
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
    if (pStk2->StackOver()) return pStack->Return(pStk2);

    // 2nd state, evalute right operand
    if ( pStk2->GetState() == 0 )
    {
        if ( !m_rightop->Execute(pStk2) ) return false;     // interrupted here?
        pStk2->IncState();
    }

    assert(pStk1->GetVar() != nullptr && pStk2->GetVar() != nullptr);
    CBotTypResult       type1 = pStk1->GetVar()->GetTypResult();      // what kind of results?
    CBotTypResult       type2 = pStk2->GetVar()->GetTypResult();

    CBotStack* pStk3 = pStk2->AddStack(this);               // adds an item to the stack
    if ( pStk3->IfStep() ) return false;                    // shows the operation if step by step

    // creates a temporary variable to put the result
    // what kind of result?
    int TypeRes = std::max(type1.GetType(), type2.GetType());

    // see "any type convertible chain" in compile method
    if ( GetTokenType() == ID_ADD &&
        (type1.Eq(CBotTypString) || type2.Eq(CBotTypString)) )
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
        if (TypeRes == CBotTypFloat)
        {
            if (type1.Eq(CBotTypLong) || type2.Eq(CBotTypLong)) TypeRes = CBotTypDouble;
        }
    }

    // creates a variable for the result
    CBotVar*    result = CBotVar::Create("", TypeRes);

    // get left and right operands
    CBotVar*    left  = pStk1->GetVar();
    CBotVar*    right = pStk2->GetVar();

    // creates a variable to perform the calculation in the appropriate type
    if ( TypeRes != CBotTypString )                                     // keep string conversion
    {
        TypeRes = std::max(type1.GetType(), type2.GetType());
    }
    else
    {
        left->Update(nullptr);
        right->Update(nullptr);
    }

    if ( GetTokenType() == ID_ADD && type1.Eq(CBotTypString) )
    {
        TypeRes = CBotTypString;
    }

    CBotVar*    temp;

    if ( TypeRes == CBotTypPointer ) TypeRes = CBotTypNullPointer;
    if ( TypeRes == CBotTypClass ) temp = CBotVar::Create("", CBotTypResult(CBotTypIntrinsic, type1.GetClass() ) );
    else                           temp = CBotVar::Create("", TypeRes );

    CBotError err = CBotNoErr;
    // is a operation according to request

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
            result->SetValInt(VarIsNAN(left) == VarIsNAN(right));
        else
            result->SetValInt(temp->Eq(left , right));  // equal
        break;
    case ID_NE:
        if ( IsNan(left, right) )
            result->SetValInt(VarIsNAN(left) != VarIsNAN(right));
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

std::string CBotTwoOpExpr::GetDebugData()
{
    return m_token.GetString();
}

std::map<std::string, CBotInstr*> CBotTwoOpExpr::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_leftop"] = m_leftop;
    links["m_rightop"] = m_rightop;
    return links;
}

} // namespace CBot
