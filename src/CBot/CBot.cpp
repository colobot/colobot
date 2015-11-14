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

// compilation of various instructions
// compile all routines as static
// and return an object according to what was found as instruction

// compiler principle:
// compile the routines return an object of the class corresponding to the operation found
// this is always a subclass of CBotInstr.
// (CBotInstr objects are never used directly)


// compiles if the routine returns nullptr is that the statement is false
// or misunderstood.
// the error is then on the stack CBotCStack :: Isok () is false


// Modules inlcude
#include "CBot.h"
#include "CBotInstr/CBotDo.h"
#include "CBotInstr/CBotFor.h"
#include "CBotInstr/CBotSwitch.h"
#include "CBotInstr/CBotBreak.h"
#include "CBotInstr/CBotTry.h"
#include "CBotInstr/CBotThrow.h"
#include "CBotInstr/CBotWhile.h"
#include "CBotInstr/CBotExprAlpha.h"
#include "CBotInstr/CBotExprNum.h"
#include "CBotInstr/CBotNew.h"
#include "CBotInstr/CBotExprNan.h"
#include "CBotInstr/CBotExprNull.h"
#include "CBotInstr/CBotExprBool.h"
#include "CBotInstr/CBotLeftExprVar.h"
#include "CBotInstr/CBotPreIncExpr.h"
#include "CBotInstr/CBotPostIncExpr.h"
#include "CBotInstr/CBotExprVar.h"
#include "CBotInstr/CBotInstrCall.h"
#include "CBotInstr/CBotListInstr.h"
#include "CBotInstr/CBotExprUnaire.h"
#include "CBotInstr/CBotBoolExpr.h"
#include "CBotInstr/CBotTwoOpExpr.h"
#include "CBotInstr/CBotExpression.h"
#include "CBotInstr/CBotIndexExpr.h"
#include "CBotInstr/CBotFieldExpr.h"
#include "CBotInstr/CBotClassInst.h"
#include "CBotInstr/CBotIString.h"
#include "CBotInstr/CBotFloat.h"
#include "CBotInstr/CBotBoolean.h"
#include "CBotInstr/CBotEmpty.h"
#include "CBotInstr/CBotReturn.h"
#include "CBotInstr/CBotIf.h"
#include "CBotInstr/CBotListArray.h"
#include "CBotInstr/CBotInstArray.h"


// Local include

// Global include
#include <cassert>


CBotInstr::CBotInstr()
{
    name     = "CBotInstr";
    m_next   = nullptr;
    m_next2b = nullptr;
    m_next3  = nullptr;
    m_next3b = nullptr;
}

CBotInstr::~CBotInstr()
{
    delete m_next;
    delete m_next2b;
    delete m_next3;
    delete m_next3b;
}

// counter of nested loops,
// to determine the break and continue valid
// list of labels used


int             CBotInstr::m_LoopLvl     = 0;
CBotStringArray CBotInstr::m_labelLvl    = CBotStringArray();

// adds a level with a label
void CBotInstr::IncLvl(CBotString& label)
{
    m_labelLvl.SetSize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = label;
    m_LoopLvl++;
}

// adds a level (switch statement)
void CBotInstr::IncLvl()
{
    m_labelLvl.SetSize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = "#SWITCH";
    m_LoopLvl++;
}

// free a level
void CBotInstr::DecLvl()
{
    m_LoopLvl--;
    m_labelLvl[m_LoopLvl].Empty();
}

// control validity of break and continue
bool CBotInstr::ChkLvl(const CBotString& label, int type)
{
    int    i = m_LoopLvl;
    while (--i>=0)
    {
        if ( type == ID_CONTINUE && m_labelLvl[i] == "#SWITCH") continue;
        if (label.IsEmpty()) return true;
        if (m_labelLvl[i] == label) return true;
    }
    return false;
}

bool CBotInstr::IsOfClass(CBotString n)
{
    return name == n;
}


////////////////////////////////////////////////////////////////////////////
// database management class CBotInstr

// set the token corresponding to the instruction

void CBotInstr::SetToken(CBotToken* p)
{
    m_token = *p;
}

// return the type of the token assicated with the instruction

int CBotInstr::GetTokenType()
{
    return m_token.GetType();
}

// return associated token

CBotToken* CBotInstr::GetToken()
{
    return &m_token;
}

// adds the statement following  the other

void CBotInstr::AddNext(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next != nullptr) p = p->m_next;
    p->m_next = n;
}

void CBotInstr::AddNext3(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next3 != nullptr) p = p->m_next3;
    p->m_next3 = n;
}

void CBotInstr::AddNext3b(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next3b != nullptr) p = p->m_next3b;
    p->m_next3b = n;
}

// returns next statement

CBotInstr* CBotInstr::GetNext()
{
    return m_next;
}

CBotInstr* CBotInstr::GetNext3()
{
    return m_next3;
}

CBotInstr* CBotInstr::GetNext3b()
{
    return m_next3b;
}

///////////////////////////////////////////////////////////////////////////
// compile an instruction which can be
// while, do, try, throw, if, for, switch, break, continue, return
// int, float, boolean, string,
// declaration of an instance of a class
// arbitrary expression


CBotInstr* CBotInstr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;

    if (p == nullptr) return nullptr;

    int type = p->GetType();            // what is the next token

    // is it a lable?
    if (IsOfType(pp, TokenTypVar) &&
         IsOfType(pp, ID_DOTS))
    {
         type = pp->GetType();
         // these instructions accept only lable
         if (!IsOfTypeList(pp, ID_WHILE, ID_FOR, ID_DO, 0))
         {
             pStack->SetError(TX_LABEL, pp->GetStart());
             return nullptr;
         }
    }

    // call routine corresponding to the compilation token found
    switch (type)
    {
    case ID_WHILE:
        return CBotWhile::Compile(p, pStack);

    case ID_FOR:
        return CBotFor::Compile(p, pStack);

    case ID_DO:
        return CBotDo::Compile(p, pStack);

    case ID_BREAK:
    case ID_CONTINUE:
        return CBotBreak::Compile(p, pStack);

    case ID_SWITCH:
        return CBotSwitch::Compile(p, pStack);

    case ID_TRY:
        return CBotTry::Compile(p, pStack);

    case ID_THROW:
        return CBotThrow::Compile(p, pStack);

    case ID_INT:
        return CBotInt::Compile(p, pStack);

    case ID_FLOAT:
        return CBotFloat::Compile(p, pStack);

    case ID_STRING:
        return CBotIString::Compile(p, pStack);

    case ID_BOOLEAN:
    case ID_BOOL:
        return CBotBoolean::Compile(p, pStack);

    case ID_IF:
        return CBotIf::Compile(p, pStack);

    case ID_RETURN:
        return CBotReturn::Compile(p, pStack);

    case ID_ELSE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(TX_ELSEWITHOUTIF, p->GetEnd());
        return nullptr;

    case ID_CASE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(TX_OUTCASE, p->GetEnd());
        return nullptr;
    }

    pStack->SetStartError(p->GetStart());

    // should not be a reserved word DefineNum
    if (p->GetType() == TokenTypDef)
    {
        pStack->SetError(TX_RESERVED, p);
        return nullptr;
    }

    // this might be an instance of class definnition
    CBotToken*    ppp = p;
    if (IsOfType(ppp, TokenTypVar))
    {
        if (CBotClass::Find(p) != nullptr)
        {
            // yes, compiles the declaration of the instance
            return CBotClassInst::Compile(p, pStack);
        }
    }

    // this can be an arythmetic instruction
    CBotInstr*    inst = CBotExpression::Compile(p, pStack);
    if (IsOfType(p, ID_SEP))
    {
        return inst;
    }
    pStack->SetError(TX_ENDOF, p->GetStart());
    delete inst;
    return nullptr;
}

bool CBotInstr::Execute(CBotStack* &pj)
{
    CBotString    ClassManquante = name;
    assert(0);            // should never go through this routine
                            // but use the routines of the subclasses
    return false;
}

bool CBotInstr::Execute(CBotStack* &pj, CBotVar* pVar)
{
    if (!Execute(pj)) return false;
    pVar->SetVal(pj->GetVar());
    return true;
}

void CBotInstr::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotString    ClassManquante = name;
    assert(0);            // should never go through this routine
                           // but use the routines of the subclasses
}


bool CBotInstr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    assert(0);            // dad do not know, see the girls
    return false;
}

bool CBotInstr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    assert(0);            // dad do not know, see the girls
    return false;
}

void CBotInstr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    assert(0);            // dad do not know, see the girls
}

// this routine is defined only for the subclass CBotCase
// this allows to make the call on all instructions CompCase
// to see if it's a case to the desired value.

bool CBotInstr::CompCase(CBotStack* &pj, int val)
{
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////
// definition of an integer variable
// int a, b = 12;

CBotInt::CBotInt()
{
    m_next    = nullptr;            // for multiple definitions
    m_var    =
    m_expr    = nullptr;
    name = "CBotInt";
}

CBotInt::~CBotInt()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotInstr::CompileArray(CBotToken* &p, CBotCStack* pStack, CBotTypResult type, bool first)
{
    if (IsOfType(p, ID_OPBRK))
    {
        if (!IsOfType(p, ID_CLBRK))
        {
            pStack->SetError(TX_CLBRK, p->GetStart());
            return nullptr;
        }

        CBotInstr*    inst = CompileArray(p, pStack, CBotTypResult(CBotTypArrayPointer, type), false);
        if (inst != nullptr || !pStack->IsOk()) return inst;
    }

    // compiles an array declaration
    if (first) return nullptr ;

    CBotInstr* inst = CBotInstArray::Compile(p, pStack, type);
    if (inst == nullptr) return nullptr;

    if (IsOfType(p,  ID_COMMA)) // several definitions
    {
        if (nullptr != ( inst->m_next2b = CBotInstArray::CompileArray(p, pStack, type, false)))    // compiles next one
        {
            return inst;
        }
        delete inst;
        return nullptr;
    }

    if (IsOfType(p,  ID_SEP))                            // end of instruction
    {
        return inst;
    }

    delete inst;
    pStack->SetError(TX_ENDOF, p->GetStart());
    return nullptr;
}

CBotInstr* CBotInt::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? nullptr : p;        // no repetition of the token "int"

    if (!cont && !IsOfType(p, ID_INT)) return nullptr;

    CBotInt*    inst = static_cast<CBotInt*>(CompileArray(p, pStack, CBotTypInt));
    if (inst != nullptr || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotInt();

    inst->m_expr = nullptr;

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    // determines the expression is valid for the item on the left side
    if (nullptr != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypInt;
        if (pStk->CheckVarLocal(vartoken))  // redefinition of the variable
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))
        {
            delete inst;    // type is not CBotInt
            p = vartoken;   // returns the variable name

            // compiles an array declaration

            CBotInstr* inst2 = CBotInstArray::Compile(p, pStk, CBotTypInt);

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GetStart());
                goto error;
            }

            if (IsOfType(p,  ID_COMMA))     // several definition chained
            {
                if (nullptr != ( inst2->m_next2b = CBotInt::Compile(p, pStk, true, noskip)))    // compile the next one
                {
                    return pStack->Return(inst2, pStk);
                }
            }
            inst = static_cast<CBotInt*>(inst2);
            goto suite;     // no assignment, variable already created
        }

        if (IsOfType(p,  ID_ASS))   // with an assignment?
        {
            if (nullptr == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
            if (pStk->GetType() >= CBotTypBoolean)  // compatible type ?
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }
        }

        {
            CBotVar*    var = CBotVar::Create(vartoken, CBotTypInt);// create the variable (evaluated after the assignment)
            var->SetInit(inst->m_expr != nullptr ? CBotVar::InitType::DEF : CBotVar::InitType::UNDEF);     // if initialized with assignment
            var->SetUniqNum( //set it with a unique number
                (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
            pStack->AddVar(var);    // place it on the stack
        }

        if (IsOfType(p,  ID_COMMA))     // chained several definitions
        {
            if (nullptr != ( inst->m_next2b = CBotInt::Compile(p, pStk, true, noskip)))    // compile next one
            {
                return pStack->Return(inst, pStk);
            }
        }
suite:
        if (noskip || IsOfType(p,  ID_SEP))                    // instruction is completed
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

// execute the definition of the integer variable

bool CBotInt::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);    // essential for SetState()

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;    // initial value // interrupted?
        m_var->Execute(pile);                                // creates and assign the result

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;                // other(s) definition(s)

    return pj->Return(pile);                                // forward below
}

void CBotInt::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;
    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == nullptr) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);    // initial value // interrupted?
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b) m_next2b->RestoreState(pile, bMain);            // other(s) definition(s)
}

//////////////////////////////////////////////////////////////////////////////////////////

// compile a list of parameters

CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars)
{
    bool        first = true;
    CBotInstr*    ret = nullptr;   // to return to the list

    CBotCStack*    pile = pStack;
    int            i = 0;

    if (IsOfType(p, ID_OPENPAR))
    {
        int    start, end;
        if (!IsOfType(p, ID_CLOSEPAR)) while (true)
        {
            start = p->GetStart();
            pile = pile->TokenStack();  // keeps the result on the stack

            if (first) pStack->SetStartError(start);
            first = false;

            CBotInstr*    param = CBotExpression::Compile(p, pile);
            end      = p->GetStart();

            if (!pile->IsOk())
            {
                return pStack->Return(nullptr, pile);
            }

            if (ret == nullptr) ret = param;
            else ret->AddNext(param);   // construct the list

            if (param != nullptr)
            {
                if (pile->GetTypResult().Eq(99))
                {
                    delete pStack->TokenStack();
                    pStack->SetError(TX_VOID, p->GetStart());
                    return nullptr;
                }
                ppVars[i] = pile->GetVar();
                ppVars[i]->GetToken()->SetPos(start, end);
                i++;

                if (IsOfType(p, ID_COMMA)) continue;    // skips the comma
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(TX_CLOSEPAR, p->GetStart());
            delete pStack->TokenStack();
            return nullptr;
        }
    }
    ppVars[i] = nullptr;
    return    ret;
}

/////////////////////////////////////////////////////////////
// check if two results are consistent to make an operation

bool TypeCompatible(CBotTypResult& type1, CBotTypResult& type2, int op)
{
    int    t1 = type1.GetType();
    int    t2 = type2.GetType();

    int max = (t1 > t2) ? t1 : t2;

    if (max == 99) return false;    // result is void?

    // special case for strin concatenation
    if (op == ID_ADD && max >= CBotTypString) return true;
    if (op == ID_ASSADD && max >= CBotTypString) return true;
    if (op == ID_ASS && t1 == CBotTypString) return true;

    if (max >= CBotTypBoolean)
    {
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypPointer && t2 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypPointer && t1 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypArrayPointer && t2 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypArrayPointer && t1 == CBotTypNullPointer)) return true;
        if (t2 != t1) return false;
        if (t1 == CBotTypArrayPointer) return type1.Compare(type2);
        if (t1 == CBotTypPointer ||
            t1 == CBotTypClass   ||
            t1 == CBotTypIntrinsic )
        {
            CBotClass*    c1 = type1.GetClass();
            CBotClass*    c2 = type2.GetClass();

            return c1->IsChildOf(c2) || c2->IsChildOf(c1);
            // accept the case in reverse
            // the transaction will be denied at runtime if the pointer is not
            // compatible
        }

        return true;
    }

    type1.SetType(max);
    type2.SetType(max);
    return true;
}

// check if two variables are compatible for parameter passing

bool TypesCompatibles(const CBotTypResult& type1, const CBotTypResult& type2)
{
    int    t1 = type1.GetType();
    int    t2 = type2.GetType();

    if (t1 == CBotTypIntrinsic) t1 = CBotTypClass;
    if (t2 == CBotTypIntrinsic) t2 = CBotTypClass;

    int max = (t1 > t2) ? t1 : t2;

    if (max == 99) return false;                    // result is void?

    if (max >= CBotTypBoolean)
    {
        if (t2 != t1) return false;

        if (max == CBotTypArrayPointer)
            return TypesCompatibles(type1.GetTypElem(), type2.GetTypElem());

        if (max == CBotTypClass || max == CBotTypPointer)
            return type1.GetClass() == type2.GetClass() ;

        return true ;
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
// file management

// necessary because it is not possible to do the fopen in the main program
// fwrite and fread in a dll or using the FILE * returned.


FILE* fOpen(const char* name, const char* mode)
{
    return fopen(name, mode);
}

int fClose(FILE* filehandle)
{
    return fclose(filehandle);
}

size_t fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle)
{
    return fwrite(buffer, elemsize, length, filehandle);
}

size_t fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle)
{
    return fread(buffer, elemsize, length, filehandle);
}

size_t fWrite(const void *buffer, size_t length, FILE* filehandle)
{
    return fwrite(buffer, 1, length, filehandle);
}

size_t fRead(void *buffer, size_t length, FILE* filehandle)
{
    return fread(buffer, 1, length, filehandle);
}


////////////////////////////////////////

