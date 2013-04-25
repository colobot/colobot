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
///////////////////////////////////////////////////////////////////////

// compilation of various instructions
// compile all routines as static
// and return an object according to what was found as instruction

// compiler principle:
// compile the routines return an object of the class corresponding to the operation found
// this is always a subclass of CBotInstr.
// (CBotInstr objects are never used directly)


// compiles if the routine returns NULL is that the statement is false
// or misunderstood.
// the error is then on the stack CBotCStack :: Isok () is false



#include "CBot.h"

CBotInstr::CBotInstr()
{
    name     = "CBotInstr";
    m_next   = NULL;
    m_next2b = NULL;
    m_next3  = NULL;
    m_next3b = NULL;
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
    while (p->m_next != NULL) p = p->m_next;
    p->m_next = n;
}

void CBotInstr::AddNext3(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next3 != NULL) p = p->m_next3;
    p->m_next3 = n;
}

void CBotInstr::AddNext3b(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next3b != NULL) p = p->m_next3b;
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

    if (p == NULL) return NULL;

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
             return NULL;
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

    case ID_DEBUGDD:
        return CBotStartDebugDD::Compile(p, pStack);

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
        return NULL;

    case ID_CASE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(TX_OUTCASE, p->GetEnd());
        return NULL;
    }

    pStack->SetStartError(p->GetStart());

    // should not be a reserved word DefineNum
    if (p->GetType() == TokenTypDef)
    {
        pStack->SetError(TX_RESERVED, p);
        return NULL;
    }

    // this might be an instance of class definnition
    CBotToken*    ppp = p;
    if (IsOfType(ppp, TokenTypVar))
    {
        if (CBotClass::Find(p) != NULL)
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
    return NULL;
}

bool CBotInstr::Execute(CBotStack* &pj)
{
    CBotString    ClassManquante = name;
    ASM_TRAP();            // should never go through this routine
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
    ASM_TRAP();            // should never go through this routine
                           // but use the routines of the subclasses
}


bool CBotInstr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    ASM_TRAP();            // dad do not know, see the girls
    return false;
}

bool CBotInstr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    ASM_TRAP();            // dad do not know, see the girls
    return false;
}

void CBotInstr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    ASM_TRAP();            // dad do not know, see the girls
}

// this routine is defined only for the subclass CBotCase
// this allows to make the call on all instructions CompCase
// to see if it's a case to the desired value.

bool CBotInstr::CompCase(CBotStack* &pj, int val)
{
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compiles a statement block " { i ; i ; } "

// this class have no constructor because there is never an instance of this
// class
// the object returned by Compile is usually of type CBotListInstr


CBotInstr* CBotBlock::Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal)
{
    pStack->SetStartError(p->GetStart());

    if (IsOfType(p, ID_OPBLK))
    {
        CBotInstr* inst = CBotListInstr::Compile(p, pStack, bLocal);

        if (IsOfType(p, ID_CLBLK))
        {
            return inst;
        }

        pStack->SetError(TX_CLOSEBLK, p->GetStart());    // missing parenthesis
        delete inst;
        return NULL;
    }

    pStack->SetError(TX_OPENBLK, p->GetStart());
    return NULL;
}

CBotInstr* CBotBlock::CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, bool bLocal)
{
    // is this a new block
    if (p->GetType() == ID_OPBLK) return CBotBlock::Compile(p, pStack);

    // otherwise, look for a single statement instead

    // to handle the case with local definition instruction (*)
    CBotCStack* pStk = pStack->TokenStack(p, bLocal);

    return pStack->Return( CBotInstr::Compile(p, pStk),    // a single instruction
                           pStk);
}

// (*) is the case in the following statement
// if (1 == 1) int x = 0;
// where the variable x is known only in the block following the if


//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compiles a list of instructions separated by semicolons

CBotListInstr::CBotListInstr()
{
    m_Instr        = NULL;
    name = "CBotListInstr";
}

CBotListInstr::~CBotListInstr()
{
    delete    m_Instr;
}

CBotInstr* CBotListInstr::Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal)
{
    CBotCStack* pStk = pStack->TokenStack(p, bLocal);        // variables are local

    CBotListInstr* inst = new CBotListInstr();

    while (true)
    {
        if (p == NULL) break;

        if (IsOfType(p, ID_SEP)) continue;              // empty statement ignored
        if (p->GetType() == ID_CLBLK) break;

        if (IsOfType(p, 0))
        {
            pStack->SetError(TX_CLOSEBLK, p->GetStart());
            delete inst;
            return pStack->Return(NULL, pStk);
        }

        CBotInstr* i = CBotBlock::CompileBlkOrInst(p, pStk);    // compiles next

        if (!pStk->IsOk())
        {
            delete inst;
            return pStack->Return(NULL, pStk);
        }

        if (inst->m_Instr == NULL) inst->m_Instr = i;
        else inst->m_Instr->AddNext(i);                            // added a result
    }
    return pStack->Return(inst, pStk);
}

// executes a set of instructions

bool CBotListInstr::Execute(CBotStack* &pj)
{

    CBotStack*    pile = pj->AddStack(this, true);                //needed for SetState()
    if (pile->StackOver() ) return pj->Return( pile);


    CBotInstr*    p = m_Instr;                                    // the first expression

    int        state = pile->GetState();
    while (state-->0) p = p->GetNext();                            // returns to the interrupted operation

    if (p != NULL) while (true)
    {
        if (!p->Execute(pile)) return false;
        p = p->GetNext();
        if (p == NULL) break;
        if (!pile->IncState()) ;//return false;                    // ready for next
    }

    return pj->Return(pile);
}

void CBotListInstr::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);
    if (pile == NULL) return;

    CBotInstr*    p = m_Instr;                                    // the first expression

    int        state = pile->GetState();
    while ( p != NULL && state-- > 0)
    {
        p->RestoreState(pile, false);
        p = p->GetNext();                            // returns to the interrupted operation
    }

    if (p != NULL) p->RestoreState(pile, true);
}

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// compilation of an element to the left of an assignment

CBotLeftExprVar::CBotLeftExprVar()
{
    name    = "CBotLeftExprVar";
    m_typevar    = -1;
    m_nIdent    =  0;
}

CBotLeftExprVar::~CBotLeftExprVar()
{
}

CBotInstr* CBotLeftExprVar::Compile(CBotToken* &p, CBotCStack* pStack)
{
    // verifies that the token is a variable name
    if (p->GetType() != TokenTypVar)
    {
        pStack->SetError( TX_NOVAR, p->GetStart());
        return NULL;
    }

    CBotLeftExprVar* inst = new CBotLeftExprVar();
    inst->SetToken(p);
    p = p->GetNext();

    return inst;
}

// creates a variable and assigns the result to the stack 
bool CBotLeftExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     var1;
    CBotVar*     var2;

    var1 = CBotVar::Create(m_token.GetString(), m_typevar);
    var1->SetUniqNum(m_nIdent);                             // with the unique identifier
    pj->AddVar(var1);                                       // place it on the stack

    var2 = pj->GetVar();                                    // result on the stack
    if (var2) var1->SetVal(var2);                           // do the assignment

    return true;
}

void CBotLeftExprVar::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotVar*     var1;

    var1 = pj->FindVar(m_token.GetString());
    if (var1 == NULL) ASM_TRAP();

    var1->SetUniqNum(m_nIdent);                    // with the unique identifier
}

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// defining an array of any type
// int a[12];
// point x[];

CBotInstArray::CBotInstArray()
{
    m_var      = NULL;
    m_listass = NULL;
    name = "CBotInstArray";
}

CBotInstArray::~CBotInstArray()
{
    delete m_var;
    delete m_listass;
}


CBotInstr* CBotInstArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotInstArray*    inst = new CBotInstArray();

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    // determinse the expression is valid for the item on the left side
    if (NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        if (pStk->CheckVarLocal(vartoken))                              // redefinition of the variable?
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        CBotInstr*    i;
        while (IsOfType(p,  ID_OPBRK))
        {
            if (p->GetType() != ID_CLBRK)
                i = CBotExpression::Compile(p, pStk);                   // expression for the value
            else
                i = new CBotEmpty();                                    // if no special formula

            inst->AddNext3b(i);                                         // construct a list
            type = CBotTypResult(CBotTypArrayPointer, type);

            if (!pStk->IsOk() || !IsOfType(p, ID_CLBRK ))
            {
                pStk->SetError(TX_CLBRK, p->GetStart());
                goto error;
            }
        }

        CBotVar*   var = CBotVar::Create(vartoken, type);               // create an instance
        inst->m_typevar = type;

        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);                                            // place it on the stack

        if (IsOfType(p, ID_ASS))                                        // with an assignment
        {
            inst->m_listass = CBotListArray::Compile(p, pStk, type.GetTypElem());
        }

        if (pStk->IsOk()) return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}


// executes the definition of an array

bool CBotInstArray::Execute(CBotStack* &pj)
{
    CBotStack*    pile1 = pj->AddStack(this);

    CBotStack*    pile  = pile1;

    if (pile1->GetState() == 0)
    {
        // seek the maximum dimension of the table
        CBotInstr*    p  = GetNext3b();                             // the different formulas
        int            nb = 0;

        while (p != NULL)
        {
            pile = pile->AddStack();                                // little room to work
            nb++;
            if (pile->GetState() == 0)
            {
                if (!p->Execute(pile)) return false;                // size calculation //interrupted?
                pile->IncState();
            }
            p = p->GetNext3b();
        }

        p     = GetNext3b();
        pile = pile1;                                               // returns to the stack
        int     n = 0;
        int     max[100];

        while (p != NULL)
        {
            pile = pile->AddStack();
            CBotVar*    v = pile->GetVar();                         // result
            max[n] = v->GetValInt();                                // value
            if (max[n]>MAXARRAYSIZE)
            {
                pile->SetError(TX_OUTARRAY, &m_token);
                return pj->Return (pile);
            }
            n++;
            p = p->GetNext3b();
        }
        while (n<100) max[n++] = 0;

        m_typevar.SetArray(max);                                    // store the limitations

        // create simply a NULL pointer
        CBotVar*    var = CBotVar::Create(m_var->GetToken(), m_typevar);
        var->SetPointer(NULL);
        var->SetUniqNum((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);
        pj->AddVar(var);

#if        STACKMEM
        pile1->AddStack()->Delete();
#else
        delete pile1->AddStack();                                   // need more indices
#endif
        pile1->IncState();
    }

    if (pile1->GetState() == 1)
    {
        if (m_listass != NULL)                                      // there is the assignment for this table
        {
            CBotVar* pVar = pj->FindVar((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);

            if (!m_listass->Execute(pile1, pVar)) return false;
        }
        pile1->IncState();
    }

    if (pile1->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile1 )) return false;

    return pj->Return(pile1);
}

void CBotInstArray::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile1 = pj;

    CBotVar*    var = pj->FindVar(m_var->GetToken()->GetString());
    if (var != NULL) var->SetUniqNum((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);

    if (bMain)
    {
        pile1 = pj->RestoreStack(this);
        CBotStack*    pile  = pile1;
        if (pile == NULL) return;

        if (pile1->GetState() == 0)
        {
            // seek the maximum dimension of the table
            CBotInstr*    p  = GetNext3b();

            while (p != NULL)
            {
                pile = pile->RestoreStack();
                if (pile == NULL) return;
                if (pile->GetState() == 0)
                {
                    p->RestoreState(pile, bMain);
                    return;
                }
                p = p->GetNext3b();
            }
        }
        if (pile1->GetState() == 1 && m_listass != NULL)
        {
            m_listass->RestoreState(pile1, bMain);
        }

    }

    if (m_next2b ) m_next2b->RestoreState( pile1, bMain);
}

// special case for empty indexes
bool CBotEmpty :: Execute(CBotStack* &pj)
{
    CBotVar*    pVar = CBotVar::Create("", CBotTypInt);
    pVar->SetValInt(-1);
    pj->SetVar(pVar);
    return true;
}

void CBotEmpty :: RestoreState(CBotStack* &pj, bool bMain)
{
}

//////////////////////////////////////////////////////////////////////////////////////
// defining a list table initialization
// int [ ] a [ ] = (( 1, 2, 3 ) , ( 3, 2, 1 )) ;


CBotListArray::CBotListArray()
{
    m_expr    = NULL;
    name = "CBotListArray";
}

CBotListArray::~CBotListArray()
{
    delete m_expr;
}


CBotInstr* CBotListArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotToken* pp = p;

    if (IsOfType( p, ID_NULL ))
    {
        CBotInstr* inst = new CBotExprNull ();
        inst->SetToken(pp);
        return pStack->Return(inst, pStk);            // ok with empty element
    }

    CBotListArray*    inst = new CBotListArray();

    if (IsOfType( p, ID_OPENPAR ))
    {
        // each element takes the one after the other
        if (type.Eq( CBotTypArrayPointer ))
        {
            type = type.GetTypElem();

            pStk->SetStartError(p->GetStart());
            if (NULL == ( inst->m_expr = CBotListArray::Compile( p, pStk, type ) ))
            {
                goto error;
            }

            while (IsOfType( p, ID_COMMA ))                                     // other elements?
            {
                pStk->SetStartError(p->GetStart());

                CBotInstr* i = CBotListArray::Compile(p, pStk, type);
                if (NULL == i)
                {
                    goto error;
                }

                inst->m_expr->AddNext3(i);
            }
        }
        else
        {
            pStk->SetStartError(p->GetStart());
            if (NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
            CBotVar* pv = pStk->GetVar();                                       // result of the expression

            if (pv == NULL || !TypesCompatibles( type, pv->GetTypResult()))     // compatible type?
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }

            while (IsOfType( p, ID_COMMA ))                                      // other elements?
            {
                pStk->SetStartError(p->GetStart());

                CBotInstr* i = CBotTwoOpExpr::Compile(p, pStk) ;
                if (NULL == i)
                {
                    goto error;
                }

                CBotVar* pv = pStk->GetVar();                                   // result of the expression

                if (pv == NULL || !TypesCompatibles( type, pv->GetTypResult())) // compatible type?
                {
                    pStk->SetError(TX_BADTYPE, p->GetStart());
                    goto error;
                }
                inst->m_expr->AddNext3(i);
            }
        }

        if (!IsOfType(p, ID_CLOSEPAR) )
        {
            pStk->SetError(TX_CLOSEPAR, p->GetStart());
            goto error;
        }

        return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}


// executes the definition of an array

bool CBotListArray::Execute(CBotStack* &pj, CBotVar* pVar)
{
    CBotStack*    pile1 = pj->AddStack();
    CBotVar* pVar2;

    CBotInstr* p = m_expr;

    int n = 0;

    for (; p != NULL ; n++, p = p->GetNext3())
    {
        if (pile1->GetState() > n) continue;

        pVar2 = pVar->GetItem(n, true);

        if (!p->Execute(pile1, pVar2)) return false;        // evaluate expression

        pile1->IncState();
    }

    return pj->Return(pile1);
}

void CBotListArray::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain)
    {
        CBotStack*    pile  = pj->RestoreStack(this);
        if (pile == NULL) return;

        CBotInstr* p = m_expr;

        int    state = pile->GetState();

        while(state-- > 0) p = p->GetNext3() ;

        p->RestoreState(pile, bMain);                    // size calculation //interrupted!
    }
}

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// definition of an integer variable
// int a, b = 12;

CBotInt::CBotInt()
{
    m_next    = NULL;            // for multiple definitions
    m_var    =
    m_expr    = NULL;
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
            return NULL;
        }

        CBotInstr*    inst = CompileArray(p, pStack, CBotTypResult(CBotTypArrayPointer, type), false);
        if (inst != NULL || !pStack->IsOk()) return inst;
    }

    // compiles an array declaration
    if (first) return NULL ;

    CBotInstr* inst = CBotInstArray::Compile(p, pStack, type);
    if (inst == NULL) return NULL;

    if (IsOfType(p,  ID_COMMA)) // several definitions
    {
        if (NULL != ( inst->m_next2b = CBotInstArray::CompileArray(p, pStack, type, false)))    // compiles next one
        {
            return inst;
        }
        delete inst;
        return NULL;
    }

    if (IsOfType(p,  ID_SEP))                            // end of instruction
    {
        return inst;
    }

    delete inst;
    pStack->SetError(TX_ENDOF, p->GetStart());
    return NULL;
}

CBotInstr* CBotInt::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? NULL : p;        // no repetition of the token "int"

    if (!cont && !IsOfType(p, ID_INT)) return NULL;

    CBotInt*    inst = static_cast<CBotInt*>(CompileArray(p, pStack, CBotTypInt));
    if (inst != NULL || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotInt();

    inst->m_expr = NULL;

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    // determines the expression is valid for the item on the left side
    if (NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
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
                if (NULL != ( inst2->m_next2b = CBotInt::Compile(p, pStk, true, noskip)))    // compile the next one
                {
                    return pStack->Return(inst2, pStk);
                }
            }
            inst = static_cast<CBotInt*>(inst2);
            goto suite;     // no assignment, variable already created
        }

        if (IsOfType(p,  ID_ASS))   // with an assignment?
        {
            if (NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
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
            var->SetInit(inst->m_expr != NULL);     // if initialized with assignment
            var->SetUniqNum( //set it with a unique number
                (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
            pStack->AddVar(var);    // place it on the stack
        }

        if (IsOfType(p,  ID_COMMA))     // chained several definitions
        {
            if (NULL != ( inst->m_next2b = CBotInt::Compile(p, pStk, true, noskip)))    // compile next one
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
    return pStack->Return(NULL, pStk);
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
        if (pile == NULL) return;

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


//////////////////////////////////////////////////////////////////////////////////////
// defining a boolean variable
// int a, b = false;

CBotBoolean::CBotBoolean()
{
    m_var    =
    m_expr    = NULL;
    name = "CBotBoolean";
}

CBotBoolean::~CBotBoolean()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotBoolean::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? NULL : p;

    if (!cont && !IsOfType(p, ID_BOOLEAN, ID_BOOL)) return NULL;

    CBotBoolean*    inst = static_cast<CBotBoolean*>(CompileArray(p, pStack, CBotTypBoolean));
    if (inst != NULL || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotBoolean();

    inst->m_expr = NULL;

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);
    CBotVar*    var = NULL;

    if (NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypBoolean;
        if (pStk->CheckVarLocal(vartoken))                    // redefinition of the variable
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))
        {
            delete inst;                                    // type is not CBotInt
            p = vartoken;                                    // resutns to the variable name

            // compiles an array declaration

            inst = static_cast<CBotBoolean*>(CBotInstArray::Compile(p, pStk, CBotTypBoolean));

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GetStart());
                goto error;
            }
            goto suite;            // no assignment, variable already created
        }

        if (IsOfType(p,  ID_ASS))
        {
            if (NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
            if (!pStk->GetTypResult().Eq(CBotTypBoolean))
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }
        }

        var = CBotVar::Create(vartoken, CBotTypBoolean);// create the variable (evaluated after the assignment)
        var->SetInit(inst->m_expr != NULL);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);
suite:
        if (IsOfType(p,  ID_COMMA))
        {
            if (NULL != ( inst->m_next2b = CBotBoolean::Compile(p, pStk, true, noskip)))
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// executes a boolean variable definition

bool CBotBoolean::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);//essential for SetState()

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;
        m_var->Execute(pile);

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;

    return pj->Return(pile);
}

void CBotBoolean::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;
    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == NULL) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);        // initial value interrupted?
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b)
         m_next2b->RestoreState(pile, bMain);                // other(s) definition(s)
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// definition of a real/float variable
// int a, b = 12.4;

CBotFloat::CBotFloat()
{
    m_var    =
    m_expr    = NULL;
    name = "CBotFloat";
}

CBotFloat::~CBotFloat()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotFloat::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? NULL : p;

    if (!cont && !IsOfType(p, ID_FLOAT)) return NULL;

    CBotFloat*    inst = static_cast<CBotFloat*>(CompileArray(p, pStack, CBotTypFloat));
    if (inst != NULL || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotFloat();

    inst->m_expr = NULL;

    CBotToken*    vartoken = p;
    CBotVar*    var = NULL;
    inst->SetToken(vartoken);

    if (NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypFloat;
        if (pStk->CheckVarLocal(vartoken))                    // redefinition of a variable
        {
            pStk->SetStartError(vartoken->GetStart());
            pStk->SetError(TX_REDEFVAR, vartoken->GetEnd());
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))
        {
            delete inst;
            p = vartoken;
            inst = static_cast<CBotFloat*>(CBotInstArray::Compile(p, pStk, CBotTypFloat));

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GetStart());
                goto error;
            }
            goto suite;            // no assignment, variable already created
        }

        if (IsOfType(p,  ID_ASS))
        {
            if (NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
            if (pStk->GetType() >= CBotTypBoolean)
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }
        }

        var = CBotVar::Create(vartoken, CBotTypFloat);
        var->SetInit(inst->m_expr != NULL);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);
suite:
        if (IsOfType(p,  ID_COMMA))
        {
            if (NULL != ( inst->m_next2b = CBotFloat::Compile(p, pStk, true, noskip)))
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// executes the definition of a real variable

bool CBotFloat::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;
        m_var->Execute(pile);

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;

    return pj->Return(pile);
}

void CBotFloat::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;
    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == NULL) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b)
         m_next2b->RestoreState(pile, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// define a string variable
// int a, b = "salut";

CBotIString::CBotIString()
{
    m_var    =
    m_expr    = NULL;
    name = "CBotIString";
}

CBotIString::~CBotIString()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotIString::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? NULL : p;

    if (!cont && !IsOfType(p, ID_STRING)) return NULL;

    CBotIString*    inst = static_cast<CBotIString*>(CompileArray(p, pStack, CBotTypString));
    if (inst != NULL || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotIString();

    inst->m_expr = NULL;

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    if (NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypString;
        if (pStk->CheckVarLocal(vartoken))
        {
            pStk->SetStartError(vartoken->GetStart());
            pStk->SetError(TX_REDEFVAR, vartoken->GetEnd());
            goto error;
        }

        if (IsOfType(p,  ID_ASS))
        {
            if (NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
/*            if (!pStk->GetTypResult().Eq(CBotTypString))            // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }*/
        }

        CBotVar*    var = CBotVar::Create(vartoken, CBotTypString);
        var->SetInit(inst->m_expr != NULL);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);

        if (IsOfType(p,  ID_COMMA))
        {
            if (NULL != ( inst->m_next2b = CBotIString::Compile(p, pStk, true, noskip)))
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// executes the definition of the string variable

bool CBotIString::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;
        m_var->Execute(pile);

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;

    return pj->Return(pile);
}

void CBotIString::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;

    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == NULL) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b)
         m_next2b->RestoreState(pile, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////
// compiles a statement such as " x = 123 " ou " z * 5 + 4 "
// with or without assignment

CBotExpression::CBotExpression()
{
    m_leftop    = NULL;
    m_rightop    = NULL;
    name = "CBotExpression";
}

CBotExpression::~CBotExpression()
{
    delete    m_leftop;
    delete    m_rightop;
}

CBotInstr* CBotExpression::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;

    CBotExpression* inst = new CBotExpression();

    inst->m_leftop = CBotLeftExpr::Compile(p, pStack);

    inst->SetToken(p);
    int  OpType = p->GetType();

    if ( pStack->IsOk() &&
         IsOfTypeList(p, ID_ASS, ID_ASSADD, ID_ASSSUB, ID_ASSMUL, ID_ASSDIV, ID_ASSMODULO, 
                                 ID_ASSAND, ID_ASSXOR, ID_ASSOR, 
                                 ID_ASSSL , ID_ASSSR,  ID_ASSASR, 0 ))
    {
        if (inst->m_leftop == NULL)
        {
            pStack->SetError(TX_BADLEFT, p->GetEnd());
            delete inst;
            return NULL;
        }

        inst->m_rightop = CBotExpression::Compile(p, pStack);
        if (inst->m_rightop == NULL)
        {
            delete inst;
            return NULL;
        }

        CBotTypResult    type1 = pStack->GetTypResult();

        // get the variable assigned to mark
        CBotVar*    var = NULL;
        inst->m_leftop->ExecuteVar(var, pStack);
        if (var == NULL)
        {
            delete inst;
            return NULL;
        }

        if (OpType != ID_ASS && var->GetInit() != IS_DEF)
        {
            pStack->SetError(TX_NOTINIT, pp);
            delete inst;
            return NULL;
        }

        CBotTypResult type2 = var->GetTypResult();

        // what types are acceptable?
        switch (OpType)
        {
        case ID_ASS:
    //        if (type2 == CBotTypClass) type2 = -1;
            if ((type1.Eq(CBotTypPointer) && type2.Eq(CBotTypPointer)) ||
                 (type1.Eq(CBotTypClass)   && type2.Eq(CBotTypClass)   ) )
            {
/*                CBotClass*    c1 = type1.GetClass();
                CBotClass*    c2 = type2.GetClass();
                if (!c1->IsChildOf(c2)) type2.SetType(-1);
//-                if (!type1.Eq(CBotTypClass)) var->SetPointer(pStack->GetVar()->GetPointer());*/
                var->SetInit(2);
            }
            else
                var->SetInit(true);

            break;
        case ID_ASSADD:
            if (type2.Eq(CBotTypBoolean) ||
                type2.Eq(CBotTypPointer) ) type2 = -1;        // numbers and strings
            break;
        case ID_ASSSUB:
        case ID_ASSMUL:
        case ID_ASSDIV:
        case ID_ASSMODULO:
            if (type2.GetType() >= CBotTypBoolean) type2 = -1;    // numbers only
            break;
        }

        if (!TypeCompatible(type1, type2, OpType))
        {
            pStack->SetError(TX_BADTYPE, &inst->m_token);
            delete inst;
            return NULL;
        }

        return inst;        // compatible type?
    }

    delete inst;
    int start, end, error = pStack->GetError(start, end);

    p = pp;                                        // returns to the top
    pStack->SetError(0,0);                        // forget the error

    CBotInstr* i = CBotTwoOpExpr::Compile(p, pStack);    // tries without assignment
    if (i != NULL && error == TX_PRIVATE && p->GetType() == ID_ASS)
        pStack->ResetError(error, start, end);
    return i;
}

// executes an expression with assignment

bool CBotExpression::Execute(CBotStack* &pj)
{
    CBotStack*    pile  = pj->AddStack(this);

//    CBotToken*    pToken = m_leftop->GetToken();
    
    CBotVar*    pVar = NULL;

    CBotStack*  pile1 = pile;

    bool        IsInit = true;
    CBotVar*    result = NULL;

    // must be done before any indexes (stack can be changed)
    if (!m_leftop->ExecuteVar(pVar, pile, NULL, false)) return false;    // variable before accessing the value on the right

    if ( pile1->GetState()==0)
    {
        pile1->SetCopyVar(pVar);    // keeps the copy on the stack (if interrupted)
        pile1->IncState();
    }

    CBotStack*    pile2 = pile->AddStack();

    if ( pile2->GetState()==0)
    {
        if (m_rightop && !m_rightop->Execute(pile2)) return false;    // initial value // interrupted?
        pile2->IncState();
    }

    if (pile1->GetState() == 1)
    {
        if (m_token.GetType() != ID_ASS)
        {
            pVar = pile1->GetVar();     // recovers if interrupted
            IsInit = pVar->GetInit();
            if (IsInit == IS_NAN)
            {
                pile2->SetError(TX_OPNAN, m_leftop->GetToken());
                return pj->Return(pile2);
             }
            result = CBotVar::Create("", pVar->GetTypResult(2));
        }

        switch (m_token.GetType())
        {
        case ID_ASS:
            break;
        case ID_ASSADD:
            result->Add(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSSUB:
            result->Sub(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSMUL:
            result->Mul(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSDIV:
            if (IsInit && 
                result->Div(pile1->GetVar(), pile2->GetVar()))
                pile2->SetError(TX_DIVZERO, &m_token);
            pile2->SetVar(result);
            break;
        case ID_ASSMODULO:
            if (IsInit && 
                result->Modulo(pile1->GetVar(), pile2->GetVar()))
                pile2->SetError(TX_DIVZERO, &m_token);
            pile2->SetVar(result);
            break;
        case ID_ASSAND:
            result->And(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSXOR:
            result->XOr(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSOR:
            result->Or(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSSL:
            result->SL(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSSR:
            result->SR(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSASR:
            result->ASR(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        default:
            ASM_TRAP();
        }
        if (!IsInit)
            pile2->SetError(TX_NOTINIT, m_leftop->GetToken());

        pile1->IncState();
    }

    if (!m_leftop->Execute( pile2, pile1 ))
        return false;

    return pj->Return(pile2);
}


void CBotExpression::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain)
    {
//        CBotToken*    pToken = m_leftop->GetToken();
//        CBotVar*    pVar = NULL;

        CBotStack*    pile  = pj->RestoreStack(this);
        if (pile == NULL) return;

        CBotStack*  pile1 = pile;

        if ( pile1->GetState()==0)
        {
            m_leftop->RestoreStateVar(pile, true); 
            return;
        }

        m_leftop->RestoreStateVar(pile, false);

        CBotStack*    pile2 = pile->RestoreStack();
        if (pile2 == NULL) return;

        if ( pile2->GetState()==0)
        {
            if (m_rightop) m_rightop->RestoreState(pile2, bMain);
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile a statement such as "(condition)"
// the condition must be Boolean

// this class has no constructor, because there is never an instance of this class
// the object returned by Compile is usually type CBotExpression


CBotInstr* CBotCondition::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GetStart());
    if (IsOfType(p, ID_OPENPAR))
    {
        CBotInstr* inst = CBotBoolExpr::Compile(p, pStack);
        if (NULL != inst)
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                return inst;
            }
            pStack->SetError(TX_CLOSEPAR, p->GetStart());    // missing parenthesis
        }
        delete inst;
    }

    pStack->SetError(TX_OPENPAR, p->GetStart());    // missing parenthesis

    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////////
// compile a statement such as "(condition)"
// the condition must be Boolean
//
// this class has no constructor, because there is never an instance of this
// class
// the object returned by Compile is usually type CBotExpression
//

CBotInstr* CBotBoolExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GetStart());

    CBotInstr* inst = CBotTwoOpExpr::Compile(p, pStack);

    if (NULL != inst)
    {
        if (pStack->GetTypResult().Eq(CBotTypBoolean))
        {
            return inst;
        }
        pStack->SetError(TX_NOTBOOL, p->GetStart());    // is not a boolean
    }

    delete inst;
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile either:
// instruction in parentheses (...)
// a unary expression (negative, not)
// variable name
// variables pre and post-incremented or decremented
// a given number DefineNum
// a constant
// procedure call
// new statement
//
// this class has no constructor, because there is never an instance of this class
// the object returned by Compile is the class corresponding to the instruction


CBotInstr* CBotParExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it an expression in parentheses?
    if (IsOfType(p, ID_OPENPAR))
    {
        CBotInstr* inst = CBotExpression::Compile(p, pStk);

        if (NULL != inst)
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                return pStack->Return(inst, pStk);
            }
            pStk->SetError(TX_CLOSEPAR, p->GetStart());
        }
        delete inst;
        return pStack->Return(NULL, pStk);
    }

    // is this a unary operation?
    CBotInstr* inst = CBotExprUnaire::Compile(p, pStk);
    if (inst != NULL || !pStk->IsOk())
        return pStack->Return(inst, pStk);

    // is it a variable name?
    if (p->GetType() == TokenTypVar)
    {
        // this may be a method call without the "this." before
        inst =  CBotExprVar::CompileMethode(p, pStk);
        if (inst != NULL) return pStack->Return(inst, pStk);


        // is it a procedure call?
        inst =  CBotInstrCall::Compile(p, pStk);
        if (inst != NULL || !pStk->IsOk())
            return pStack->Return(inst, pStk);


        CBotToken* pvar = p;
        // no, it an "ordinaty" variable
        inst =  CBotExprVar::Compile(p, pStk);

        CBotToken* pp = p;
        // post incremented or decremented?
        if (IsOfType(p, ID_INC, ID_DEC))
        {
            if (pStk->GetType() >= CBotTypBoolean)
            {
                pStk->SetError(TX_BADTYPE, pp);
                delete inst;
                return pStack->Return(NULL, pStk);
            }

            // recompile the variable for read-only
            delete inst;
            p = pvar;
            inst =  CBotExprVar::Compile(p, pStk, PR_READ);
            p = p->GetNext();

            CBotPostIncExpr* i = new CBotPostIncExpr();
            i->SetToken(pp);
            i->m_Instr = inst;    // associated statement
            return pStack->Return(i, pStk);
        }
        return pStack->Return(inst, pStk);
    }

    // pre increpemted or pre decremented?
    CBotToken* pp = p;
    if (IsOfType(p, ID_INC, ID_DEC))
    {
        CBotPreIncExpr* i = new CBotPreIncExpr();
        i->SetToken(pp);

        if (p->GetType() == TokenTypVar)
        {
            if (NULL != (i->m_Instr =  CBotExprVar::Compile(p, pStk, PR_READ)))
            {
                if (pStk->GetType() >= CBotTypBoolean)
                {
                    pStk->SetError(TX_BADTYPE, pp);
                    delete inst;
                    return pStack->Return(NULL, pStk);
                }
                return pStack->Return(i, pStk);
            }
            delete i;
            return pStack->Return(NULL, pStk);
        }
    }

    // is it a number or DefineNum?
    if (p->GetType() == TokenTypNum || 
        p->GetType() == TokenTypDef )
    {
        CBotInstr* inst = CBotExprNum::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is this a chaine?
    if (p->GetType() == TokenTypString)
    {
        CBotInstr* inst = CBotExprAlpha::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is a "true" or "false"
    if (p->GetType() == ID_TRUE ||
        p->GetType() == ID_FALSE )
    {
        CBotInstr* inst = CBotExprBool::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is an object to be created with new
    if (p->GetType() == ID_NEW)
    {
        CBotInstr* inst = CBotNew::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is a null pointer
    if (IsOfType(p, ID_NULL))
    {
        CBotInstr* inst = new CBotExprNull ();
        inst->SetToken(pp);
        CBotVar* var = CBotVar::Create("", CBotTypNullPointer);
        pStk->SetVar(var);
        return pStack->Return(inst, pStk);
    }

    // is a number nan
    if (IsOfType(p, ID_NAN))
    {
        CBotInstr* inst = new CBotExprNan ();
        inst->SetToken(pp);
        CBotVar* var = CBotVar::Create("", CBotTypInt);
        var->SetInit(IS_NAN);
        pStk->SetVar(var);
        return pStack->Return(inst, pStk);
    }


    return pStack->Return(NULL, pStk);
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// Management of pre-and post increment / decrement
// There is no routine Compiles, the object is created directly
// Compiles in CBotParExpr ::


CBotPostIncExpr::CBotPostIncExpr()
{
    m_Instr = NULL;
    name    = "CBotPostIncExpr";
}

CBotPostIncExpr::~CBotPostIncExpr()
{
    delete    m_Instr;
}

CBotPreIncExpr::CBotPreIncExpr()
{
    m_Instr = NULL;
    name    = "CBotPreIncExpr";
}

CBotPreIncExpr::~CBotPreIncExpr()
{
    delete    m_Instr;
}

bool CBotPostIncExpr::Execute(CBotStack* &pj)
{
    CBotStack*    pile1 = pj->AddStack(this);
    CBotStack*    pile2 = pile1;

    CBotVar*    var1 = NULL;

    // retrieves the variable fields and indexes according
    if (!(static_cast<CBotExprVar*>(m_Instr))->ExecuteVar(var1, pile2, NULL, true)) return false;

    pile1->SetState(1);
    pile1->SetCopyVar(var1);                                // places the result (before incrementation);

    CBotStack* pile3 = pile2->AddStack(this); 
    if (pile3->IfStep()) return false;

    if (var1->GetInit() == IS_NAN)
    {
        pile1->SetError(TX_OPNAN, &m_token);
    }

    if (var1->GetInit() != IS_DEF)
    {
        pile1->SetError(TX_NOTINIT, &m_token);
    }

    if (GetTokenType() == ID_INC) var1->Inc();
    else                          var1->Dec();

    return pj->Return(pile1);                        // operation done, result on pile2
}

void CBotPostIncExpr::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile1 = pj->RestoreStack(this);
    if (pile1 == NULL) return;

    (static_cast<CBotExprVar*>(m_Instr))->RestoreStateVar(pile1, bMain);

    if (pile1 != NULL) pile1->RestoreStack(this);
}

bool CBotPreIncExpr::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*     var1;

    if (pile->GetState() == 0)
    {
        CBotStack*    pile2 = pile;
        // retrieves the variable fields and indexes according
        // pile2 is modified on return
        if (!(static_cast<CBotExprVar*>(m_Instr))->ExecuteVar(var1, pile2, NULL, true)) return false;

        if (var1->GetInit() == IS_NAN)
        {
            pile->SetError(TX_OPNAN, &m_token);
            return pj->Return(pile);    // operation performed
        }

        if (var1->GetInit() != IS_DEF)
        {
            pile->SetError(TX_NOTINIT, &m_token);
            return pj->Return(pile);    // operation performed
        }

        if (GetTokenType() == ID_INC) var1->Inc();
        else                          var1->Dec();  // ((CBotVarInt*)var1)->m_val

        pile->IncState();
    }

    if (!m_Instr->Execute(pile)) return false;
    return pj->Return(pile);    // operation performed
}


void CBotPreIncExpr::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);
    if (pile == NULL) return;

    if (pile->GetState() == 0)
    {
        return;
    }

    m_Instr->RestoreState(pile, bMain);
}


//////////////////////////////////////////////////////////////////////////////////////
// compile an unary expression
//    +
//    -
//    not
//    !
//    ~

CBotExprUnaire::CBotExprUnaire()
{
    m_Expr = NULL;
    name = "CBotExprUnaire";
}

CBotExprUnaire::~CBotExprUnaire()
{
    delete m_Expr;
}

CBotInstr* CBotExprUnaire::Compile(CBotToken* &p, CBotCStack* pStack)
{
    int op = p->GetType();
    CBotToken*    pp = p;
    if (!IsOfTypeList( p, ID_ADD, ID_SUB, ID_LOG_NOT, ID_TXT_NOT, ID_NOT, 0 )) return NULL;

    CBotCStack* pStk = pStack->TokenStack(pp);

    CBotExprUnaire* inst = new CBotExprUnaire();
    inst->SetToken(pp);

    if (NULL != (inst->m_Expr = CBotParExpr::Compile( p, pStk )))
    {
        if (op == ID_ADD && pStk->GetType() < CBotTypBoolean)        // only with the number
            return pStack->Return(inst, pStk);
        if (op == ID_SUB && pStk->GetType() < CBotTypBoolean)        // only with the numer
            return pStack->Return(inst, pStk);
        if (op == ID_NOT && pStk->GetType() < CBotTypFloat)        // only with an integer
            return pStack->Return(inst, pStk);
        if (op == ID_LOG_NOT && pStk->GetTypResult().Eq(CBotTypBoolean))// only with boolean
            return pStack->Return(inst, pStk);
        if (op == ID_TXT_NOT && pStk->GetTypResult().Eq(CBotTypBoolean))// only with boolean
            return pStack->Return(inst, pStk);

        pStk->SetError(TX_BADTYPE, &inst->m_token);
    }
    delete inst;
    return pStack->Return(NULL, pStk);
}

// executes unary expression

bool CBotExprUnaire::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->GetState() == 0)
    {
        if (!m_Expr->Execute(pile)) return false;                    // interrupted ?
        pile->IncState();
    }

    CBotStack* pile2 = pile->AddStack();
    if (pile2->IfStep()) return false;

    CBotVar*    var = pile->GetVar();                                // get the result on the stack

    switch (GetTokenType())
    {
    case ID_ADD:
        break;
    case ID_SUB:
        var->Neg();                                                  // change the sign
        break;
    case ID_NOT:
    case ID_LOG_NOT:
    case ID_TXT_NOT:
        var->Not();
        break;
    }
    return pj->Return(pile);                                        // forwards below
}

void CBotExprUnaire::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);
    if ( pile == NULL) return;

    if (pile->GetState() == 0)
    {
        m_Expr->RestoreState(pile, bMain);                        // interrupted here!
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// index management for arrays
// array [ expression ]


CBotIndexExpr::CBotIndexExpr()
{
    m_expr    = NULL;
    name    = "CBotIndexExpr";
}

CBotIndexExpr::~CBotIndexExpr()
{
    delete    m_expr;
}

// finds a field from the instance at compile time

bool CBotIndexExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if (pVar->GetType(1) != CBotTypArrayPointer)
        ASM_TRAP();

    pVar = (static_cast<CBotVarArray*>(pVar))->GetItem(0, false);    // at compile time makes the element [0]
    if (pVar == NULL)
    {
        pile->SetError(TX_OUTARRAY, m_token.GetEnd());
        return false;
    }
    if (m_next3 != NULL) return m_next3->ExecuteVar(pVar, pile);
    return true;
}

// attention, changes the pointer to the stack intentionally
// place the index calculated on the additional stack


bool CBotIndexExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotStack*    pj = pile;

    if (pVar->GetType(1) != CBotTypArrayPointer)
        ASM_TRAP();

    pile = pile->AddStack();

    if (pile->GetState() == 0)
    {
        if (!m_expr->Execute(pile)) return false;
        pile->IncState();
    }
    // handles array

    CBotVar* p = pile->GetVar();    // result on the stack

    if (p == NULL || p->GetType() > CBotTypDouble)
    {
        pile->SetError(TX_BADINDEX, prevToken);
        return pj->Return(pile);
    }

    int n = p->GetValInt();     // position in the table

    pVar = (static_cast<CBotVarArray*>(pVar))->GetItem(n, bExtend);
    if (pVar == NULL)
    {
        pile->SetError(TX_OUTARRAY, prevToken);
        return pj->Return(pile);
    }

    pVar->Maj(pile->GetPUser(), true);

    if ( m_next3 != NULL && 
         !m_next3->ExecuteVar(pVar, pile, prevToken, bStep, bExtend) ) return false;

    // does not release the stack
    // to avoid recalculation of the index twice where appropriate
    return true;
}

void CBotIndexExpr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    pile = pile->RestoreStack();
    if (pile == NULL) return;

    if (bMain && pile->GetState() == 0)
    {
        m_expr->RestoreState(pile, true);
        return;
    }

    if (m_next3) 
         m_next3->RestoreStateVar(pile, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// field management in an instance (dot operator)
// toto.x


CBotFieldExpr::CBotFieldExpr()
{
    name        = "CBotFieldExpr";
    m_nIdent    = 0;
}

CBotFieldExpr::~CBotFieldExpr()
{
}

void CBotFieldExpr::SetUniqNum(int num)
{
    m_nIdent = num;
}


// find a field from the instance at compile

bool CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if (pVar->GetType(1) != CBotTypPointer)
        ASM_TRAP();

    pVar = pVar->GetItemRef(m_nIdent);
    if (pVar == NULL)
    {
        pile->SetError(TX_NOITEM, &m_token);
        return false;
    }

    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile) ) return false;

    return true;
}

bool CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotStack*    pj = pile;
    pile = pile->AddStack(this);    // changes in output stack
    if (pile == EOX) return true;


    if (pVar->GetType(1) != CBotTypPointer)
        ASM_TRAP();

    CBotVarClass* pItem = pVar->GetPointer();
    if (pItem == NULL)
    {
        pile->SetError(TX_NULLPT, prevToken);
        return pj->Return(pile);
    }
    if (pItem->GetUserPtr() == OBJECTDELETED)
    {
        pile->SetError(TX_DELETEDPT, prevToken);
        return pj->Return(pile);
    }

    if (bStep && pile->IfStep()) return false;

    pVar = pVar->GetItemRef(m_nIdent);
    if (pVar == NULL)
    {
        pile->SetError(TX_NOITEM, &m_token);
        return pj->Return(pile);
    }

    if (pVar->IsStatic())
    {
        // for a static variable, takes it in the class itself
        CBotClass* pClass = pItem->GetClass();
        pVar = pClass->GetItem(m_token.GetString());
    }

    // request the update of the element, if applicable
    pVar->Maj(pile->GetPUser(), true);

    if ( m_next3 != NULL && 
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, bExtend) ) return false;

    // does not release the stack
    // to maintain the state SetState () corresponding to step

    return true;
}

void CBotFieldExpr::RestoreStateVar(CBotStack* &pj, bool bMain)
{
    pj = pj->RestoreStack(this);
    if (pj == NULL) return;

    if (m_next3 != NULL) 
         m_next3->RestoreStateVar(pj, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// compile a left operand for an assignment
CBotLeftExpr::CBotLeftExpr()
{
    name    = "CBotLeftExpr";
    m_nIdent = 0;
}

CBotLeftExpr::~CBotLeftExpr()
{
}

// compiles an expression for a left-operand (left of an assignment)
// this can be
// toto
// toto[ 3 ]
// toto.x
// toto.pos.x
// toto[2].pos.x
// toto[1].pos[2].x
// toto[1][2][3]

CBotLeftExpr* CBotLeftExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it a variable name?
    if (p->GetType() == TokenTypVar)
    {
        CBotLeftExpr* inst = new CBotLeftExpr();    // creates the object

        inst->SetToken(p);

        CBotVar*     var;

        if (NULL != (var = pStk->FindVar(p)))   // seek if known variable
        {
            inst->m_nIdent = var->GetUniqNum();
            if (inst->m_nIdent > 0 && inst->m_nIdent < 9000)
            {
                if ( var->IsPrivate(PR_READ) &&
                     !pStk->GetBotCall()->m_bCompileClass)
                {
                    pStk->SetError(TX_PRIVATE, p);
                    goto err;
                }
                // this is an element of the current class
                // adds the equivalent of this. before
                CBotToken pthis("this");
                inst->SetToken(&pthis);
                inst->m_nIdent = -2;    // indent for this

                CBotFieldExpr* i = new CBotFieldExpr();     // new element
                i->SetToken(p);     // keeps the name of the token
                inst->AddNext3(i);  // add after

                var = pStk->FindVar(pthis);
                var = var->GetItem(p->GetString());
                i->SetUniqNum(var->GetUniqNum());
            }
            p = p->GetNext();   // next token

            while (true)
            {
                if (var->GetType() == CBotTypArrayPointer)
                {
                    if (IsOfType( p, ID_OPBRK ))
                    {
                        CBotIndexExpr* i = new CBotIndexExpr();
                        i->m_expr = CBotExpression::Compile(p, pStk);
                        inst->AddNext3(i);  // add to the chain

                        var = (static_cast<CBotVarArray*>(var))->GetItem(0,true);    // gets the component [0]

                        if (i->m_expr == NULL)
                        {
                            pStk->SetError(TX_BADINDEX, p->GetStart());
                            goto err;
                        }

                        if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                        {
                            pStk->SetError(TX_CLBRK, p->GetStart());
                            goto err;
                        }
                        continue;
                    }
                }

                if (var->GetType(1) == CBotTypPointer)                // for classes
                {
                    if (IsOfType(p, ID_DOT))
                    {
                        CBotToken* pp = p;

                        CBotFieldExpr* i = new CBotFieldExpr();            // new element
                        i->SetToken(pp);                                // keeps the name of the token
                        inst->AddNext3(i);                                // adds after

                        if (p->GetType() == TokenTypVar)                // must be a name
                        {
                            var = var->GetItem(p->GetString());            // get item correspondent
                            if (var != NULL)
                            {
                                if ( var->IsPrivate(PR_READ) &&
                                     !pStk->GetBotCall()->m_bCompileClass)
                                {
                                    pStk->SetError(TX_PRIVATE, pp);
                                    goto err;
                                }

                                i->SetUniqNum(var->GetUniqNum());
                                p = p->GetNext();                        // skips the name
                                continue;
                            }
                            pStk->SetError(TX_NOITEM, p);
                        }
                        pStk->SetError(TX_DOT, p->GetStart());
                        goto err;
                    }
                }
                break;
            }


            if (pStk->IsOk()) return static_cast<CBotLeftExpr*> (pStack->Return(inst, pStk));
        }
        pStk->SetError(TX_UNDEFVAR, p);
err:
        delete inst;
        return static_cast<CBotLeftExpr*> ( pStack->Return(NULL, pStk));
    }

    return static_cast<CBotLeftExpr*> ( pStack->Return(NULL, pStk));
}

// runs, is a variable and assigns the result to the stack
bool CBotLeftExpr::Execute(CBotStack* &pj, CBotStack* array)
{
    CBotStack*    pile = pj->AddStack();

    CBotVar*     var1 = NULL;
    CBotVar*     var2 = NULL;
    // fetch a variable (not copy)
    if (!ExecuteVar(var1, array, NULL, false)) return false;

    if (pile->IfStep()) return false;

    if (var1)
    {
        var2 = pj->GetVar();    // result on the input stack
        if (var2)
        {
            CBotTypResult t1 = var1->GetTypResult();
            CBotTypResult t2 = var2->GetTypResult();
            if (t2.Eq(CBotTypPointer))
            {
                CBotClass*    c1 = t1.GetClass();
                CBotClass*    c2 = t2.GetClass();
                if ( !c2->IsChildOf(c1))
                {
                    CBotToken* pt = &m_token;
                    pile->SetError(TX_BADTYPE, pt);
                    return pj->Return(pile);    // operation performed
                }
            }
            var1->SetVal(var2);     // do assignment
        }
        pile->SetCopyVar(var1);     // replace the stack with the copy of the variable
                                    // (for name)
    }

    return pj->Return(pile);    // operation performed
}

// fetch a variable during compilation

bool CBotLeftExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    pVar = pile->FindVar(m_token);
    if (pVar == NULL) return false;

    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile) ) return false;

    return true;
}

// fetch the variable at runtume

bool CBotLeftExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep)
{
    pile = pile->AddStack(this);

    pVar = pile->FindVar(m_nIdent);
    if (pVar == NULL)
    {
#ifdef    _DEBUG
        ASM_TRAP();
#endif
        pile->SetError(2, &m_token);
        return false;
    }

    if (bStep && m_next3 == NULL && pile->IfStep()) return false;

    if ( m_next3 != NULL && 
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, true) ) return false;

    return true;
}

void CBotLeftExpr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    pile = pile->RestoreStack(this);
    if (pile == NULL) return;

    if (m_next3 != NULL) 
         m_next3->RestoreStateVar(pile, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

// converts a string into integer
// may be of the form 0xabc123

long GetNumInt(const char* p)
{
    long    num = 0;
    while (*p >= '0' && *p <= '9')
    {
        num = num * 10 + *p - '0';
        p++;
    }
    if (*p == 'x' || *p == 'X')
    {
        while (*++p != 0)
        {
            if (*p >= '0' && *p <= '9')
            {
                num = num * 16 + *p - '0';
                continue;
            }
            if (*p >= 'A' && *p <= 'F')
            {
                num = num * 16 + *p - 'A' + 10;
                continue;
            }
            if (*p >= 'a' && *p <= 'f')
            {
                num = num * 16 + *p - 'a' + 10;
                continue;
            }
            break;
        }
    }
    return num;
}

// converts a string into a float number
extern float GetNumFloat(const char* p)
{
    double    num = 0;
    double    div    = 10;
    bool    bNeg = false;

    if (*p == '-')
    {
        bNeg = true;
        p++;
    }
    while (*p >= '0' && *p <= '9')
    {
        num = num * 10. + (*p - '0');
        p++;
    }

    if (*p == '.')
    {
        p++;
        while (*p >= '0' && *p <= '9')
        {
            num = num + (*p - '0') / div;
            div = div * 10;
            p++;
        }
    }

    int    exp = 0;
    if (*p == 'e' || *p == 'E')
    {
        char neg = 0;
        p++;
        if (*p == '-' || *p == '+') neg = *p++;

        while (*p >= '0' && *p <= '9')
        {
            exp = exp * 10 + (*p - '0');
            p++;
        }
        if (neg == '-') exp = -exp;
    }

    while (exp > 0)
    {
        num *= 10.0;
        exp--;
    }

    while (exp < 0)
    {
        num /= 10.0;
        exp++;
    }

    if (bNeg) num = -num;
    return static_cast<float>(num);
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compiles a token representing a number
CBotExprNum::CBotExprNum()
{
    name    = "CBotExprNum";
}

CBotExprNum::~CBotExprNum()
{
}

CBotInstr* CBotExprNum::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprNum* inst = new CBotExprNum();

    inst->SetToken(p);
    CBotString    s = p->GetString();

    inst->m_numtype = CBotTypInt;
    if (p->GetType() == TokenTypDef)
    {
        inst->m_valint = p->GetIdKey();
    }
    else
    {
        if (s.Find('.') >= 0 || ( s.Find('x') < 0 && ( s.Find('e') >= 0 || s.Find('E') >= 0 ) ))
        {
            inst->m_numtype = CBotTypFloat;
            inst->m_valfloat = GetNumFloat(s);
        }
        else
        {
            inst->m_valint = GetNumInt(s);
        }
    }

    if (pStk->NextToken(p))
    {
        CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), inst->m_numtype);
        pStk->SetVar(var);

        return pStack->Return(inst, pStk);
    }
    delete inst;
    return pStack->Return(NULL, pStk);
}

// execute, returns the corresponding number

bool CBotExprNum::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), m_numtype);

    CBotString    nombre ;
    if (m_token.GetType() == TokenTypDef)
    {
        nombre = m_token.GetString();
    }

    switch (m_numtype)
    {
    case CBotTypShort:
    case CBotTypInt:
        var->SetValInt(m_valint, nombre);
        break;
    case CBotTypFloat:
        var->SetValFloat(m_valfloat);
        break;
    }
    pile->SetVar(var);                            // place on the stack

    return pj->Return(pile);                        // it's ok
}

void CBotExprNum::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile a token representing a string

CBotExprAlpha::CBotExprAlpha()
{
    name    = "CBotExprAlpha";
}

CBotExprAlpha::~CBotExprAlpha()
{
}

CBotInstr* CBotExprAlpha::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprAlpha* inst = new CBotExprAlpha();

    inst->SetToken(p);
    p = p->GetNext();

    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), CBotTypString);
    pStk->SetVar(var);

    return pStack->Return(inst, pStk);
}

// execute, returns the corresponding string

bool CBotExprAlpha::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), CBotTypString);

    CBotString    chaine = m_token.GetString();
    chaine = chaine.Mid(1, chaine.GetLength()-2);    // removes the quotes

    var->SetValString(chaine);                    // value of the number

    pile->SetVar(var);                            // put on the stack

    return pj->Return(pile);
}

void CBotExprAlpha::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile a token representing true or false

CBotExprBool::CBotExprBool()
{
    name = "CBotExprBool";
}

CBotExprBool::~CBotExprBool()
{
}

CBotInstr* CBotExprBool::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();
    CBotExprBool* inst = NULL;

    if ( p->GetType() == ID_TRUE ||
         p->GetType() == ID_FALSE )
    {
        inst = new CBotExprBool();
        inst->SetToken(p);  // stores the operation false or true
        p = p->GetNext();

        CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), CBotTypBoolean);
        pStk->SetVar(var);
    }

    return pStack->Return(inst, pStk);
}

// executes, returns true or false

bool CBotExprBool::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), CBotTypBoolean);

    if (GetTokenType() == ID_TRUE)      var->SetValInt(1);
    else                              var->SetValInt(0);

    pile->SetVar(var);  // put on the stack
    return pj->Return(pile);    // forwards below
}

void CBotExprBool::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////

// management of the operand "null"

CBotExprNull::CBotExprNull()
{
    name = "CBotExprNull";
}

CBotExprNull::~CBotExprNull()
{
}

// executes, returns an empty pointer

bool CBotExprNull::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;
    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), CBotTypNullPointer);

    var->SetInit(true);         // null pointer valid
    pile->SetVar(var);          // place on the stack
    return pj->Return(pile);    // forwards below
}

void CBotExprNull::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////

// management of the operand "nan"

CBotExprNan::CBotExprNan()
{
    name = "CBotExprNan";
}

CBotExprNan::~CBotExprNan()
{
}

// executes, returns null pointer

bool CBotExprNan::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;
    CBotVar*    var = CBotVar::Create(static_cast<CBotToken*>(NULL), CBotTypInt);

    var->SetInit(IS_NAN);       // nan
    pile->SetVar(var);          // put on the stack
    return pj->Return(pile);    // forward below
}

void CBotExprNan::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////
// compile a variable name
// check that it is known on the stack
// and it has been initialized

CBotExprVar::CBotExprVar()
{
    name    = "CBotExprVar";
    m_nIdent = 0;
}

CBotExprVar::~CBotExprVar()
{
}

CBotInstr* CBotExprVar::Compile(CBotToken* &p, CBotCStack* pStack, int privat)
{
//    CBotToken*    pDebut = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it a variable?
    if (p->GetType() == TokenTypVar)
    {
        CBotInstr* inst = new CBotExprVar();    // create the object

        inst->SetToken(p);

        CBotVar*     var;

        if (NULL != (var = pStk->FindVar(p)))   // seek if known variable
        {
            int        ident = var->GetUniqNum();
            (static_cast<CBotExprVar*>(inst))->m_nIdent = ident;     // identifies variable by its number

            if (ident > 0 && ident < 9000)
            {
                if ( var->IsPrivate(privat) &&
                     !pStk->GetBotCall()->m_bCompileClass)
                {
                    pStk->SetError(TX_PRIVATE, p);
                    goto err;
                }

                // This is an element of the current class
                // ads the equivalent of this. before
                CBotToken token("this");
                inst->SetToken(&token);
                (static_cast<CBotExprVar*>(inst))->m_nIdent = -2;    // identificator for this

                CBotFieldExpr* i = new CBotFieldExpr();     // new element
                i->SetToken(p);     // keeps the name of the token
                i->SetUniqNum(ident);
                inst->AddNext3(i);  // added after
            }

            p = p->GetNext();   // next token

            while (true)
            {
                if (var->GetType() == CBotTypArrayPointer)
                {
                    if (IsOfType( p, ID_OPBRK ))    // check if there is an aindex 
                    {
                        CBotIndexExpr* i = new CBotIndexExpr();
                        i->m_expr = CBotExpression::Compile(p, pStk);   // compile the formula
                        inst->AddNext3(i);  // add to the chain

                        var = (static_cast<CBotVarArray*>(var))->GetItem(0,true);    // gets the component [0]

                        if (i->m_expr == NULL)
                        {
                            pStk->SetError(TX_BADINDEX, p->GetStart());
                            goto err;
                        }
                        if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                        {
                            pStk->SetError(TX_CLBRK, p->GetStart());
                            goto err;
                        }
                        continue;
                    }
                }
                if (var->GetType(1) == CBotTypPointer)  // for classes
                {
                    if (IsOfType(p, ID_DOT))
                    {
                        CBotToken* pp = p;

                        if (p->GetType() == TokenTypVar)    // must be a name
                        {
                            if (p->GetNext()->GetType() == ID_OPENPAR)  // a method call?
                            {
                                CBotInstr* i = CBotInstrMethode::Compile(p, pStk, var);
                                if (!pStk->IsOk()) goto err;
                                inst->AddNext3(i);  // added after
                                return pStack->Return(inst, pStk);
                            }
                            else
                            {
                                CBotFieldExpr* i = new CBotFieldExpr();     // new element
                                i->SetToken(pp);                            // keeps the name of the token
                                inst->AddNext3(i);                          // add after
                                var = var->GetItem(p->GetString());         // get item correspondent
                                if (var != NULL)
                                {
                                    i->SetUniqNum(var->GetUniqNum());
                                    if ( var->IsPrivate() &&
                                     !pStk->GetBotCall()->m_bCompileClass)
                                    {
                                        pStk->SetError(TX_PRIVATE, pp);
                                        goto err;
                                    }
                                }
                            }


                            if (var != NULL)
                            {
                                p = p->GetNext();   // skips the name
                                continue;
                            }
                            pStk->SetError(TX_NOITEM, p);
                            goto err;
                        }
                        pStk->SetError(TX_DOT, p->GetStart());
                        goto err;
                    }
                }

                break;
            }

            pStk->SetCopyVar(var);  // place the copy of the variable on the stack (for type)
            if (pStk->IsOk()) return pStack->Return(inst, pStk);
        }
        pStk->SetError(TX_UNDEFVAR, p);
err:
        delete inst;
        return pStack->Return(NULL, pStk);
    }

    return pStack->Return(NULL, pStk);
}

CBotInstr* CBotExprVar::CompileMethode(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(pp->GetStart());

    // is it a variable ?
    if (pp->GetType() == TokenTypVar)
    {
        CBotToken pthis("this");
        CBotVar*     var = pStk->FindVar(pthis);
        if (var == 0) return pStack->Return(NULL, pStk);

        CBotInstr* inst = new CBotExprVar();

        // this is an element of the current class
        // adds the equivalent of this. before

        inst->SetToken(&pthis);
        (static_cast<CBotExprVar*>(inst))->m_nIdent = -2;    // ident for this

        CBotToken* pp = p;

        if (pp->GetType() == TokenTypVar)
        {
            if (pp->GetNext()->GetType() == ID_OPENPAR)        // a method call?
            {
                CBotInstr* i = CBotInstrMethode::Compile(pp, pStk, var);
                if (pStk->IsOk())
                {
                    inst->AddNext3(i);                            // add after
                    p = pp;                                        // previous instruction
                    return pStack->Return(inst, pStk);
                }
                pStk->SetError(0,0);                            // the error is not adressed here
            }
        }
        delete inst;
    }
    return pStack->Return(NULL, pStk);
}


// execute, making the value of a variable

bool CBotExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     pVar = NULL;
    CBotStack*     pile  = pj->AddStack(this);

    CBotStack*     pile1 = pile;

    if (pile1->GetState() == 0)
    {
        if (!ExecuteVar(pVar, pile, NULL, true)) return false;        // Get the variable fields and indexes according

        if (pVar) pile1->SetCopyVar(pVar);                            // place a copy on the stack
        else
        {
            return pj->Return(pile1);
        }
        pile1->IncState();
    }

    pVar = pile1->GetVar();

    if (pVar == NULL)
    {
        return pj->Return(pile1);
    }

    if (pVar->GetInit() == IS_UNDEF)
    {
        CBotToken* pt = &m_token;
        while (pt->GetNext() != NULL) pt = pt->GetNext();
        pile1->SetError(TX_NOTINIT, pt);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);   // operation completed
}

void CBotExprVar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*     pile  = pj->RestoreStack(this);
    if (pile == NULL) return;

    CBotStack*     pile1 = pile;

    if (pile1->GetState() == 0)
    {
        RestoreStateVar(pile, bMain);   // retrieves the variable fields and indexes according
        return;
    }
}

// fetch a variable at runtime

bool CBotExprVar::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep)
{
    CBotStack*    pile = pj;
    pj = pj->AddStack(this);

    if (bStep && m_nIdent>0 && pj->IfStep()) return false;

    pVar = pj->FindVar(m_nIdent, true);     // tries with the variable update if necessary
    if (pVar == NULL)
    {
#ifdef    _DEBUG
        ASM_TRAP();
#endif
        pj->SetError(1, &m_token);
        return false;
    }
    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pj, &m_token, bStep, false) )
            return false;   // field of an instance, table, methode

    return pile->ReturnKeep(pj);    // does not put on stack but get the result if a method was called
}


// fetch variable at runtime

void CBotExprVar::RestoreStateVar(CBotStack* &pj, bool bMain)
{
    pj = pj->RestoreStack(this);
    if (pj == NULL) return;

    if (m_next3 != NULL)
         m_next3->RestoreStateVar(pj, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

// compile a list of parameters

CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars)
{
    bool        first = true;
    CBotInstr*    ret = NULL;   // to return to the list

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
                return pStack->Return(NULL, pile);
            }

            if (ret == NULL) ret = param;
            else ret->AddNext(param);   // construct the list

            if (param != NULL)
            {
                if (pile->GetTypResult().Eq(99))
                {
                    delete pStack->TokenStack();
                    pStack->SetError(TX_VOID, p->GetStart());
                    return NULL;
                }
                ppVars[i] = pile->GetVar();
                ppVars[i]->GetToken()->SetPos(start, end);
                i++;

                if (IsOfType(p, ID_COMMA)) continue;    // skips the comma
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(TX_CLOSEPAR, p->GetStart());
            delete pStack->TokenStack();
            return NULL;
        }
    }
    ppVars[i] = NULL;
    return    ret;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile a method call

CBotInstrMethode::CBotInstrMethode()
{
    m_Parameters    = NULL;
    m_MethodeIdent  = 0;
    name = "CBotInstrMethode";
}

CBotInstrMethode::~CBotInstrMethode()
{
    delete    m_Parameters;
}

CBotInstr* CBotInstrMethode::Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* var)
{
    CBotInstrMethode* inst = new CBotInstrMethode();
    inst->SetToken(p);  // corresponding token

    if (NULL != var)
    {
        CBotToken*    pp = p;
        p = p->GetNext();

        if (p->GetType() == ID_OPENPAR)
        {
            inst->m_NomMethod = pp->GetString();

            // compiles the list of parameters
            CBotVar*    ppVars[1000];
            inst->m_Parameters = CompileParams(p, pStack, ppVars);

            if (pStack->IsOk())
            {
                CBotClass* pClass = var->GetClass();    // pointer to the class
                inst->m_ClassName = pClass->GetName();  // name of the class
                CBotTypResult r = pClass->CompileMethode(inst->m_NomMethod, var, ppVars, 
                                                         pStack, inst->m_MethodeIdent);
                delete pStack->TokenStack();    // release parameters on the stack
                inst->m_typRes = r;

                if (inst->m_typRes.GetType() > 20)
                {
                    pStack->SetError(inst->m_typRes.GetType(), pp);
                    delete    inst;
                    return    NULL;
                }
                // put the result on the stack to have something
                if (inst->m_typRes.GetType() > 0)
                {
                    CBotVar*    pResult = CBotVar::Create("", inst->m_typRes);
                    if (inst->m_typRes.Eq(CBotTypClass))
                    {
                        pResult->SetClass(inst->m_typRes.GetClass());
                    }
                    pStack->SetVar(pResult);
                }
                return inst;
            }
            delete inst;
            return NULL;
        }
    }
    pStack->SetError(1234, p);
    delete inst;
    return NULL;
}

// execute the method call

bool CBotInstrMethode::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend)
{
    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pj->AddStack(this, true);     // a place for the copy of This

    if (pVar->GetPointer() == NULL)
    {
        pj->SetError(TX_NULLPT, prevToken);
    }

    if (pile1->IfStep()) return false;

    CBotStack*    pile2 = pile1->AddStack();    // for the next parameters

    if ( pile1->GetState() == 0)
    {
        CBotVar*    pThis = CBotVar::Create(pVar);
        pThis->Copy(pVar);
        // this value should be taken before the evaluation parameters
        // Test.Action (Test = Other);
        // action must act on the value before test = Other!

        pThis->SetName("this");
        pThis->SetUniqNum(-2);
        pile1->AddVar(pThis);
        pile1->IncState();
    }
    int        i = 0;

    CBotInstr*    p = m_Parameters;
    // evaluate the parameters
    // and places the values on the stack
    // to be interrupted at any time

    if (p != NULL) while ( true)
    {
        if (pile2->GetState() == 0)
        {
            if (!p->Execute(pile2)) return false;   // interrupted here?
            if (!pile2->SetState(1)) return false;  // special mark to recognize parameters
        }
        ppVars[i++] = pile2->GetVar();              // construct the list of pointers
        pile2 = pile2->AddStack();                  // space on the stack for the result
        p = p->GetNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotClass*    pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pThis  = pile1->FindVar(-2);
    CBotVar*    pResult = NULL;
    if (m_typRes.GetType() > 0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
        pResult->SetClass(m_typRes.GetClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_NomMethod, 
                                 pThis, ppVars, 
                                 pResult, pile2, GetToken())) return false;
    if (pRes != pResult) delete pRes;

    pVar = NULL;                // does not return value for this
    return pj->Return(pile2);   // release the entire stack
}

void CBotInstrMethode::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    if (!bMain) return;

    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pile->RestoreStack(this);     // place for the copy of This
    if (pile1 == NULL) return;

    CBotStack*    pile2 = pile1->RestoreStack();        // and for the parameters coming
    if (pile2 == NULL) return;

    CBotVar*    pThis  = pile1->FindVar("this");
    pThis->SetUniqNum(-2);

    int        i = 0;

    CBotInstr*    p = m_Parameters;
    // evaluate the parameters
    // and places the values on the stack
    // to be interrupted at any time

    if (p != NULL) while ( true)
    {
        if (pile2->GetState() == 0)
        {
            p->RestoreState(pile2, true);   // interrupted here!
            return;
        }
        ppVars[i++] = pile2->GetVar();      // construct the list of pointers
        pile2 = pile2->RestoreStack();
        if (pile2 == NULL) return;

        p = p->GetNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotClass*    pClass = CBotClass::Find(m_ClassName);
//    CBotVar*    pResult = NULL;

//    CBotVar*    pRes = pResult;

    pClass->RestoreMethode(m_MethodeIdent, m_NomMethod, 
                                 pThis, ppVars, pile2);
}


bool CBotInstrMethode::Execute(CBotStack* &pj)
{
    CBotVar*    ppVars[1000];
    CBotStack*    pile1 = pj->AddStack(this, true);        // place for the copy of This

    if (pile1->IfStep()) return false;

    CBotStack*    pile2 = pile1->AddStack();                // and for the parameters coming

    if ( pile1->GetState() == 0)
    {
        CBotVar*    pThis = pile1->CopyVar(m_token);
        // this value should be taken before the evaluation parameters
        // Test.Action (Test = Other);
        // Action must act on the value before test = Other!
        pThis->SetName("this");
        pile1->AddVar(pThis);
        pile1->IncState();
    }
    int        i = 0;

    CBotInstr*    p = m_Parameters;
    // evaluate the parameters
    // and places the values on the stack
    // to be interrupted at any time
    if (p != NULL) while ( true)
    {
        if (pile2->GetState() == 0)
        {
            if (!p->Execute(pile2)) return false;   // interrupted here?
            if (!pile2->SetState(1)) return false;  // special mark to recognize parameters
        }
        ppVars[i++] = pile2->GetVar();              // construct the list of pointers
        pile2 = pile2->AddStack();                  // space on the stack for the results
        p = p->GetNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotClass*    pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pThis  = pile1->FindVar("this");
    CBotVar*    pResult = NULL;
    if (m_typRes.GetType()>0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
        pResult->SetClass(m_typRes.GetClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_NomMethod, 
                                 pThis, ppVars, 
                                 pResult, pile2, GetToken())) return false;    // interupted

    // set the new value of this in place of the old variable
    CBotVar*    old = pile1->FindVar(m_token);
    old->Copy(pThis, false);

    if (pRes != pResult) delete pRes;

    return pj->Return(pile2);    // release the entire stack
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile an instruction "new" 

CBotNew::CBotNew()
{
    name            = "CBotNew";
    m_Parameters    = NULL;
    m_nMethodeIdent = 0;
}

CBotNew::~CBotNew()
{
}

CBotInstr* CBotNew::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken* pp = p;
    if (!IsOfType(p, ID_NEW)) return NULL;

    // verifies that the token is a class name
    if (p->GetType() != TokenTypVar) return NULL;

    CBotClass* pClass = CBotClass::Find(p);
    if (pClass == NULL)
    {
        pStack->SetError(TX_BADNEW, p);
        return NULL;
    }

    CBotNew* inst = new CBotNew();
    inst->SetToken(pp);

    inst->m_vartoken = p;
    p = p->GetNext();

    // creates the object on the "job"
    // with a pointer to the object
    CBotVar*    pVar = CBotVar::Create("", pClass);

    // do the call of the creator
    CBotCStack* pStk = pStack->TokenStack();
    {
        // check if there are parameters
        CBotVar*    ppVars[1000];
        inst->m_Parameters = CompileParams(p, pStk, ppVars);
        if (!pStk->IsOk()) goto error;

        // constructor exist?
        CBotTypResult r = pClass->CompileMethode(pClass->GetName(), pVar, ppVars, pStk, inst->m_nMethodeIdent);
        delete pStk->TokenStack();  // release extra stack
        int typ = r.GetType();

        // if there is no constructor, and no parameters either, it's ok
        if (typ == TX_UNDEFCALL && inst->m_Parameters == NULL) typ = 0;
        pVar->SetInit(true);    // mark the instance as init

        if (typ>20)
        {
            pStk->SetError(typ, inst->m_vartoken.GetEnd());
            goto error;
        }

        // if the constructor does not exist, but there are parameters
        if (typ<0 && inst->m_Parameters != NULL)
        {
            pStk->SetError(TX_NOCONST, &inst->m_vartoken);
            goto error;
        }

        // makes pointer to the object on the stack
        pStk->SetVar(pVar);
        return pStack->Return(inst, pStk);
    }
error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// executes instruction "new"

bool CBotNew::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);    //main stack

    if (pile->IfStep()) return false;

    CBotStack*    pile1 = pj->AddStack2();  //secondary stack

    CBotVar*    pThis = NULL;

    CBotToken*    pt = &m_vartoken;
    CBotClass*    pClass = CBotClass::Find(pt);

    // create the variable "this" pointer type to the stack

    if ( pile->GetState()==0)
    {
        // create an instance of the requested class
        // and initialize the pointer to that object


        pThis = CBotVar::Create("this", pClass);
        pThis->SetUniqNum(-2) ;

        pile1->SetVar(pThis);   // place on stack1
        pile->IncState();
    }

    // fetch the this pointer if it was interrupted
    if ( pThis == NULL)
    {
        pThis = pile1->GetVar();    // find the pointer
    }

    // is there an assignment or parameters (constructor)
    if ( pile->GetState()==1)
    {
        // evaluates the constructor of the instance

        CBotVar*    ppVars[1000];
        CBotStack*    pile2 = pile;

        int        i = 0;

        CBotInstr*    p = m_Parameters;
        // evaluate the parameters
        // and places the values on the stack
        // to be interrupted at any time

        if (p != NULL) while ( true)
        {
            pile2 = pile2->AddStack();  // space on the stack for the result
            if (pile2->GetState() == 0)
            {
                if (!p->Execute(pile2)) return false;   // interrupted here?
                pile2->SetState(1);
            }
            ppVars[i++] = pile2->GetVar();
            p = p->GetNext();
            if ( p == NULL) break;
        }
        ppVars[i] = NULL;

        // create a variable for the result
        CBotVar*    pResult = NULL;     // constructos still void

        if ( !pClass->ExecuteMethode(m_nMethodeIdent, pClass->GetName(), 
                                     pThis, ppVars, 
                                     pResult, pile2, GetToken())) return false;    // interrupt

        pThis->ConstructorSet();    // indicates that the constructor has been called
    }

    return pj->Return(pile1);   // passes below
}

void CBotNew::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);    //primary stack
    if (pile == NULL) return;

    CBotStack*    pile1 = pj->AddStack2();  //secondary stack

    CBotToken*    pt = &m_vartoken;
    CBotClass*    pClass = CBotClass::Find(pt);

    // create the variable "this" pointer type to the object

    if ( pile->GetState()==0)
    {
        return;
    }

    CBotVar* pThis = pile1->GetVar();   // find the pointer
    pThis->SetUniqNum(-2);

    // is ther an assignment or parameters (constructor)
    if ( pile->GetState()==1)
    {
        // evaluates the constructor of the instance

        CBotVar*    ppVars[1000];
        CBotStack*    pile2 = pile;

        int        i = 0;

        CBotInstr*    p = m_Parameters;
        // evaluate the parameters
        // and places the values on the stack
        // to be interrupted at any time

        if (p != NULL) while ( true)
        {
            pile2 = pile2->RestoreStack();  // space on the stack for the result
            if (pile2 == NULL) return;

            if (pile2->GetState() == 0)
            {
                p->RestoreState(pile2, bMain);  // interrupt here!
                return;
            }
            ppVars[i++] = pile2->GetVar();
            p = p->GetNext();
            if ( p == NULL) break;
        }
        ppVars[i] = NULL;

        pClass->RestoreMethode(m_nMethodeIdent, m_vartoken.GetString(), pThis,
                               ppVars, pile2)    ;        // interrupt here!
    }
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

