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

////////////////////////////////////////////////////////////////////
/**
 * \file CBot.h
 * \brief Interpreter of the language CBot for COLOBOT game
 */

#pragma once

#include "resource.h"
#include "CBotDll.h"                    // public definitions
#include "CBotToken.h"                  // token management

#define    STACKRUN    1                /// \def return execution directly on a suspended routine
#define    STACKMEM    1                /// \def preserve memory for the execution stack
#define    MAXSTACK    990              /// \def stack size reserved

#define    EOX         (reinterpret_cast<CBotStack*>(-1))   /// \def tag special condition


/////////////////////////////////////////////////////////////////////
// forward declaration

class CBotParExpr;  // basic type or instruction in parenthesis
                    // Toto.truc
                    // 12.5
                    // "string"
                    // ( expression )
class CBotExprVar;  // a variable name as
                    // Toto
                    // chose.truc.machin
class CBotWhile;    // while (...) {...};
class CBotIf;       // if (...) {...} else {...}
class CBotDefParam; // paramerer list of a function



////////////////////////////////////////////////////////////////////////
// Management of the execution stack
////////////////////////////////////////////////////////////////////////

// actually, externally, the only thing it can do
// is to create an instance of a stack
// to use for routine CBotProgram :: Execute (CBotStack)


/**\class CBotStack
 * \brief Management of the execution stack.
 * \brief Actually the only thing it can do is to create an instance of a stack
 * \brief to use for routine CBotProgram :: Execute(CBotStack)*/
class CBotStack
{
public:
#if    STACKMEM
    /**
     * \brief FirstStack Allocate first stack
     * \return pointer to created stack
     */
    static CBotStack * FirstStack();

    /** \brief Delete Remove current stack */
    void Delete();
#endif

    /**
     * \brief CBotStack Constructor of the stack
     * \param ppapa Not used.
     */
    CBotStack(CBotStack* ppapa);


    /** \brief ~CBotStack Destructor */
    ~CBotStack();

    /**
     * \brief StackOver Check if end of stack is reached
     * \return true if end of stack
     */
    bool StackOver();

    /**
     * \brief GetError Get error number of the stack
     * \param [out] start beginning of the stack
     * \param [out] end end of stack
     * \return error number
     */
    int GetError(int& start, int& end);

    /**
     * \brief GetError Get error number
     * \return eror number
     */
    int GetError();// rend le numéro d'erreur retourné

    /**
     * \brief Reset Reset error at and set user
     * \param [in] pUser User of stack
     */
    void Reset(void* pUser);

    /**
     * \brief SetType Determines the type.
     * \param type Type of instruction on the stack.
     */
    void SetType(CBotTypResult& type);

    /**
     * \brief GetType Get the type of value on the stack.
     * \param [in] mode Used when getting class type (1 gives pointer, 2 gives intrinsic).
     * \return Type number.
     */
    int GetType(int mode = 0);

    /**
     * \brief Getes the type of complete value on the stack.
     * \param [in] mode Used when getting class type (1 gives pointer, 2 gives intrinsic).
     * \return  Type of an element.
     */
    CBotTypResult GetTypResult(int mode = 0);

    /**
     * \brief Adds a local variable.
     * \param [in] p Variable to be added.
     */
    void AddVar(CBotVar* p);

    /**
     * \brief Fetch a variable by its token.
     * \brief This may be a composite variable
     * \param [in] pToken Token upon which search is performed
     * \param [in] bUpdate Not used. Probably need to be removed
     * \param [in] bModif Not used. Probably need to be removed
     * \return Found variable
     */
    CBotVar* FindVar(CBotToken* &pToken, bool bUpdate = false,
                                           bool bModif  = false);

    /**
     * \brief Fetch a variable by its token.
     * \brief This may be a composite variable
     * \param [in] pToken Token upon which search is performed
     * \param [in] bUpdate Not used. Probably need to be removed
     * \param [in] bModif Not used. Probably need to be removed
     * \return Found variable
     */
    CBotVar* FindVar(CBotToken& pToken, bool bUpdate = false,
                                              bool bModif  = false);

    /**
     * \brief Fetch variable by its name
     * \param [in] name Name of variable to find
     * \return Found variable
     */
    CBotVar* FindVar(const char* name);

    /**
     * \brief Fetch a variable on the stack according to its identification number
     * \brief This is faster than comparing names
     * \param [in] ident Identifier of a variable
     * \param [in] bUpdate Not used. Probably need to be removed
     * \param [in] bModif Not used. Probably need to be removed
     * \return Found variable
     */
    CBotVar* FindVar(long ident, bool bUpdate = false,
                                        bool bModif  = false);

    /**
     * \brief Find variable by its token and returns a copy of it.
     * \param Token Token upon which search is performed
     * \param bUpdate Not used.
     * \return Found variable, nullptr if not found
     */
    CBotVar*        CopyVar(CBotToken& Token, bool bUpdate = false);


    CBotStack*        AddStack(CBotInstr* instr = nullptr, bool bBlock = false);    // extends the stack
    CBotStack*        AddStackEOX(CBotCall* instr = nullptr, bool bBlock = false);    // extends the stack
    CBotStack*        RestoreStack(CBotInstr* instr = nullptr);
    CBotStack*        RestoreStackEOX(CBotCall* instr = nullptr);

    CBotStack*        AddStack2(bool bBlock = false);                        // extends the stack
    bool            Return(CBotStack* pFils);                            // transmits the result over
    bool            ReturnKeep(CBotStack* pFils);                        // transmits the result without reducing the stack
    bool            BreakReturn(CBotStack* pfils, const char* name = nullptr);
                                                                        // in case of eventual break
    bool            IfContinue(int state, const char* name);
                                                                        // or "continue"

    bool            IsOk();

    bool            SetState(int n, int lim = -10);                        // select a state
    int                GetState();                                            // in what state am I?
    bool            IncState(int lim = -10);                            // passes to the next state
    bool            IfStep();                                            // do step by step
    bool            Execute();

    void            SetVar( CBotVar* var );
    void            SetCopyVar( CBotVar* var );
    CBotVar*        GetVar();
    CBotVar*        GetCopyVar();
    CBotVar*        GetPtVar();
    bool            GetRetVar(bool bRet);
    long            GetVal();

    void            SetError(int n, CBotToken* token = nullptr);
    void            SetPosError(CBotToken* token);
    void            ResetError(int n, int start, int end);
    void            SetBreak(int val, const char* name);

    void            SetBotCall(CBotProgram* p);
    CBotProgram*    GetBotCall(bool bFirst = false);
    void*            GetPUser();
    bool            GetBlock();


    bool            ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype);
    void            RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar);

    bool            SaveState(FILE* pf);
    bool            RestoreState(FILE* pf, CBotStack* &pStack);

    static
    void            SetTimer(int n);

    void            GetRunPos(const char* &FunctionName, int &start, int &end);
    CBotVar*        GetStackVars(const char* &FunctionName, int level);

    int                m_temp;

private:
    CBotStack*        m_next;
    CBotStack*        m_next2;
    CBotStack*        m_prev;
    friend class CBotInstArray;

#ifdef    _DEBUG
    int                m_index;
#endif
    int                m_state;
    int                m_step;
    static int        m_error;
    static int        m_start;
    static int        m_end;
    static
    CBotVar*        m_retvar;                    // result of a return

    CBotVar*        m_var;                        // result of the operations
    CBotVar*        m_listVar;                    // variables declared at this level

    bool            m_bBlock;                    // is part of a block (variables are local to this block)
    bool            m_bOver;                    // stack limits?
//    bool            m_bDontDelete;                // special, not to destroy the variable during delete
    CBotProgram*    m_prog;                        // user-defined functions

    static
    int                m_initimer;
    static
    int                m_timer;
    static
    CBotString        m_labelBreak;
    static
    void*            m_pUser;

    CBotInstr*        m_instr;                    // the corresponding instruction
    bool            m_bFunc;                    // an input of a function?
    CBotCall*        m_call;                        // recovery point in a extern call
    friend class    CBotTry;
};

// inline routinees must be declared in file.h

inline bool CBotStack::IsOk()
{
    return (m_error == 0);
}

inline int CBotStack::GetState()
{
    return m_state;
}

inline int CBotStack::GetError()
{
    return m_error;
}

////////////////////////////////////////////////////////////////////////
// Management of the stack of compilation
////////////////////////////////////////////////////////////////////////


class CBotCStack
{
private:
    CBotCStack*        m_next;
    CBotCStack*        m_prev;

    static int      m_error;
    static int      m_end;
    int              m_start;

    CBotVar*        m_var;                        // result of the operations

    bool            m_bBlock;                    // is part of a block (variables are local to this block)
    CBotVar*        m_listVar;

    static
    CBotProgram*    m_prog;                        // list of compiled functions
    static
    CBotTypResult    m_retTyp;
//    static
//    CBotToken*        m_retClass;

public:
                    CBotCStack(CBotCStack* ppapa);
                    ~CBotCStack();

    bool            IsOk();
    int                GetError();
    int                GetError(int& start, int& end);
                                                // gives error number

    void            SetType(CBotTypResult& type);// determines the type
    CBotTypResult    GetTypResult(int mode = 0);    // gives the type of value on the stack
    int                GetType(int mode = 0);        // gives the type of value on the stack
    CBotClass*        GetClass();                    // gives the class of the value on the stack

    void            AddVar(CBotVar* p);            // adds a local variable
    CBotVar*        FindVar(CBotToken* &p);        // finds a variable
    CBotVar*        FindVar(CBotToken& Token);
    bool            CheckVarLocal(CBotToken* &pToken);
    CBotVar*        CopyVar(CBotToken& Token);    // finds and makes a copy

    CBotCStack*        TokenStack(CBotToken* pToken = nullptr, bool bBlock = false);
    CBotInstr*        Return(CBotInstr* p, CBotCStack* pParent);    // transmits the result upper
    CBotFunction*    ReturnFunc(CBotFunction* p, CBotCStack* pParent);    // transmits the result upper

    void            SetVar( CBotVar* var );
    void            SetCopyVar( CBotVar* var );
    CBotVar*        GetVar();

    void            SetStartError(int pos);
    void            SetError(int n, int pos);
    void            SetError(int n, CBotToken* p);
    void            ResetError(int n, int start, int end);

    void            SetRetType(CBotTypResult& type);
    CBotTypResult    GetRetType();

//    void            SetBotCall(CBotFunction* &pFunc);
    void            SetBotCall(CBotProgram* p);
    CBotProgram*    GetBotCall();
    CBotTypResult    CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent);
    bool            CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

    bool            NextToken(CBotToken* &p);
};


extern bool SaveVar(FILE* pf, CBotVar* pVar);


/////////////////////////////////////////////////////////////////////
// class defining an instruction
class CBotInstr
{
private:
    static
    CBotStringArray
                m_labelLvl;
protected:
    CBotToken    m_token;                // keeps the token
    CBotString    name;                    // debug
    CBotInstr*    m_next;                    // linked command
    CBotInstr*    m_next2b;                // second list definition chain
    CBotInstr*    m_next3;                // third list for indices and fields
    CBotInstr*    m_next3b;                // necessary for reporting tables
/*
    for example, the following program
    int        x[]; x[1] = 4;
    int        y[x[1]][10], z;
    is generated
    CBotInstrArray
    m_next3b-> CBotEmpty
    m_next->
    CBotExpression ....
    m_next->
    CBotInstrArray
    m_next3b-> CBotExpression ('x') ( m_next3-> CBotIndexExpr ('1') )
    m_next3b-> CBotExpression ('10')
    m_next2-> 'z'
    m_next->...

*/

    static
    int                m_LoopLvl;
    friend class    CBotClassInst;
    friend class    CBotInt;
    friend class    CBotListArray;

public:
                CBotInstr();
                virtual
                ~CBotInstr();

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    static
    CBotInstr*    CompileArray(CBotToken* &p, CBotCStack* pStack, CBotTypResult type, bool first = true);

    virtual
    bool        Execute(CBotStack* &pj);
    virtual
    bool        Execute(CBotStack* &pj, CBotVar* pVar);
    virtual
    void        RestoreState(CBotStack* &pj, bool bMain);

    virtual
    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
    virtual
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend);
    virtual
    void        RestoreStateVar(CBotStack* &pile, bool bMain);

    virtual
    bool        CompCase(CBotStack* &pj, int val);

    void        SetToken(CBotToken* p);
    int            GetTokenType();
    CBotToken*    GetToken();

    void        AddNext(CBotInstr* n);
    CBotInstr*    GetNext();
    void        AddNext3(CBotInstr* n);
    CBotInstr*    GetNext3();
    void        AddNext3b(CBotInstr* n);
    CBotInstr*    GetNext3b();

    static
    void        IncLvl(CBotString& label);
    static
    void        IncLvl();
    static
    void        DecLvl();
    static
    bool        ChkLvl(const CBotString& label, int type);

    bool        IsOfClass(CBotString name);
};

class CBotWhile : public CBotInstr
{
private:
    CBotInstr*    m_Condition;        // condition
    CBotInstr*    m_Block;            // instructions
    CBotString    m_label;            // a label if there is

public:
                CBotWhile();
                ~CBotWhile();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotReturn : public CBotInstr
{
private:
    CBotInstr*    m_Instr;            // paramter of return

public:
                CBotReturn();
                ~CBotReturn();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotThrow : public CBotInstr
{
private:
    CBotInstr*    m_Value;            // the value to send

public:
                CBotThrow();
                ~CBotThrow();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


class CBotIf : public CBotInstr
{
private:
    CBotInstr*    m_Condition;        // condition
    CBotInstr*    m_Block;            // instructions
    CBotInstr*    m_BlockElse;        // instructions

public:
                CBotIf();
                ~CBotIf();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


// definition of an integer

class CBotInt : public CBotInstr
{
private:
    CBotInstr*    m_var;                // the variable to initialize
    CBotInstr*    m_expr;                // a value to put, if there is
///    CBotInstr*    m_next;                // several definitions chained

public:
                CBotInt();
                ~CBotInt();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip = false);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// definition of an array

class CBotInstArray : public CBotInstr
{
private:
    CBotInstr*    m_var;                // the variables to initialize
    CBotInstr*    m_listass;            // list of assignments for array
    CBotTypResult
                m_typevar;            // type of elements
//    CBotString    m_ClassName;

public:
                CBotInstArray();
                ~CBotInstArray();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


// definition of a assignment list for a table
// int [ ] a [ ] = ( ( 1, 2, 3 ) , ( 3, 2, 1 ) ) ;

class CBotListArray : public CBotInstr
{
private:
    CBotInstr*    m_expr;                // an expression for an element
                                    // others are linked with CBotInstr :: m_next3;
public:
                CBotListArray();
                ~CBotListArray();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);
    bool        Execute(CBotStack* &pj, CBotVar* pVar) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


class CBotEmpty : public CBotInstr
{
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// defininition of a boolean

class CBotBoolean : public CBotInstr
{
private:
    CBotInstr*    m_var;                // variable to initialise
    CBotInstr*    m_expr;                // a value to put, if there is

public:
                CBotBoolean();
                ~CBotBoolean();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip=false);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


// definition of a real number

class CBotFloat : public CBotInstr
{
private:
    CBotInstr*    m_var;                // variable to initialise
    CBotInstr*    m_expr;                // a value to put, if there is

public:
                CBotFloat();
                ~CBotFloat();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip=false);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// definition of an element string

class CBotIString : public CBotInstr
{
private:
    CBotInstr*    m_var;                // variable to initialise
    CBotInstr*    m_expr;                // a value to put, if there is

public:
                CBotIString();
                ~CBotIString();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip=false);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// definition of an element of any class

class CBotClassInst : public CBotInstr
{
private:
    CBotInstr*    m_var;                // variable to initialise
    CBotClass*    m_pClass;            // reference to the class
    CBotInstr*    m_Parameters;        // parameters to be evaluated for the contructor
    CBotInstr*    m_expr;                // a value to put, if there is
    bool        m_hasParams;        // has it parameters?
    long        m_nMethodeIdent;

public:
                CBotClassInst();
                ~CBotClassInst();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass = nullptr);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotCondition : public CBotInstr
{
private:

public:

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
};


// left operand
// accept the expressions that be to the left of assignment

class CBotLeftExpr : public CBotInstr
{
private:
    long        m_nIdent;

public:
                CBotLeftExpr();
                ~CBotLeftExpr();
    static
    CBotLeftExpr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack, CBotStack* array);

    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep);
    void        RestoreStateVar(CBotStack* &pile, bool bMain) override;
};


// management of the fields of an instance

class CBotFieldExpr : public CBotInstr
{
private:
    friend class CBotExpression;
    int            m_nIdent;

public:
                CBotFieldExpr();
                ~CBotFieldExpr();
    void        SetUniqNum(int num);
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend) override;
    void        RestoreStateVar(CBotStack* &pj, bool bMain) override;
};

// management of indices of the tables

class CBotIndexExpr : public CBotInstr
{
private:
    CBotInstr*     m_expr;                    // expression for calculating the index
    friend class CBotLeftExpr;
    friend class CBotExprVar;

public:
                CBotIndexExpr();
                ~CBotIndexExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend) override;
    void        RestoreStateVar(CBotStack* &pj, bool bMain) override;
};

// expressions like
// x = a;
// x * y + 3;

class CBotExpression : public CBotInstr
{
private:
    CBotLeftExpr*    m_leftop;            // left operand
    CBotInstr*        m_rightop;            // right operant

public:
                CBotExpression();
                ~CBotExpression();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotLogicExpr : public CBotInstr
{
private:
    CBotInstr*    m_condition;        // test to evaluate
    CBotInstr*    m_op1;                // left element
    CBotInstr*    m_op2;                // right element
    friend class CBotTwoOpExpr;

public:
                CBotLogicExpr();
                ~CBotLogicExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};



class CBotBoolExpr : public CBotInstr
{
private:

public:
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
};



// possibly an expression in parentheses ( ... )
// there is never an instance of this class
// being the object returned inside the parenthesis
class CBotParExpr : public CBotInstr
{
private:

public:
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
};

// unary expression
class CBotExprUnaire : public CBotInstr
{
private:
    CBotInstr*    m_Expr;                // expression to be evaluated
public:
                CBotExprUnaire();
                ~CBotExprUnaire();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// all operations with two operands

class CBotTwoOpExpr : public CBotInstr
{
private:
    CBotInstr*    m_leftop;            // left element
    CBotInstr*    m_rightop;            // right element
public:
                CBotTwoOpExpr();
                ~CBotTwoOpExpr();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations = nullptr);
    bool        Execute(CBotStack* &pStack) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};




// an instruction block { .... }
class CBotBlock : public CBotInstr
{
public:
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal = true);
    static
    CBotInstr*    CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, bool bLocal = false);
private:
    CBotBlock() = delete;
    CBotBlock(const CBotBlock &) = delete;
};


// the content of a block of instructions ... ; ... ; ... ; ... ;
class CBotListInstr : public CBotInstr
{
private:
    CBotInstr*    m_Instr;            // instructions to do

public:
                CBotListInstr();
                ~CBotListInstr();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal = true);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


class CBotInstrCall : public CBotInstr
{
private:
    CBotInstr*    m_Parameters;        // the parameters to be evaluated
//    int            m_typeRes;            // type of the result
//    CBotString    m_RetClassName;        // class of the result
    CBotTypResult
                m_typRes;            // complete type of the result
    long        m_nFuncIdent;        // id of a function

public:
                CBotInstrCall();
                ~CBotInstrCall();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// a call of method

class CBotInstrMethode : public CBotInstr
{
private:
    CBotInstr*    m_Parameters;        // the parameters to be evaluated
//    int            m_typeRes;            // type of the result
//    CBotString    m_RetClassName;        // class of the result
    CBotTypResult
                m_typRes;            // complete type of the result

    CBotString    m_NomMethod;        // name of the method
    long        m_MethodeIdent;        // identifier of the method
//    long        m_nThisIdent;        // identifier for "this"
    CBotString    m_ClassName;        // name of the class

public:
                CBotInstrMethode();
                ~CBotInstrMethode();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* pVar);
    bool        Execute(CBotStack* &pj) override;
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend) override;
    void        RestoreStateVar(CBotStack* &pj, bool bMain) override;
};

// expression for the variable name

class CBotExprVar : public CBotInstr
{
private:
    long        m_nIdent;
    friend class CBotPostIncExpr;
    friend class CBotPreIncExpr;

public:
                CBotExprVar();
                ~CBotExprVar();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, int privat=PR_PROTECT);
    static
    CBotInstr*    CompileMethode(CBotToken* &p, CBotCStack* pStack);

    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep);
    void        RestoreStateVar(CBotStack* &pj, bool bMain) override;
};

class CBotPostIncExpr : public CBotInstr
{
private:
    CBotInstr*    m_Instr;
    friend class CBotParExpr;

public:
                CBotPostIncExpr();
                ~CBotPostIncExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotPreIncExpr : public CBotInstr
{
private:
    CBotInstr*    m_Instr;
    friend class CBotParExpr;

public:
                CBotPreIncExpr();
                ~CBotPreIncExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


class CBotLeftExprVar : public CBotInstr
{
private:
public:
    CBotTypResult
                m_typevar;            // type of variable declared
    long        m_nIdent;            // unique identifier for that variable

public:
                CBotLeftExprVar();
                ~CBotLeftExprVar();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


class CBotExprBool : public CBotInstr
{
private:

public:
                CBotExprBool();
                ~CBotExprBool();

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


class CBotExprNull : public CBotInstr
{
private:

public:
                CBotExprNull();
                ~CBotExprNull();

    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotExprNan : public CBotInstr
{
private:

public:
                CBotExprNan();
                ~CBotExprNan();

    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

class CBotNew : public CBotInstr
{
private:
    CBotInstr*    m_Parameters;        // the parameters to be evaluated
    long        m_nMethodeIdent;
//    long        m_nThisIdent;
    CBotToken    m_vartoken;

public:
                CBotNew();
                ~CBotNew();

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};

// expression representing a number

class CBotExprNum : public CBotInstr
{
private:
    int            m_numtype;                    // et the type of number
    long        m_valint;                    // value for an int
    float        m_valfloat;                    // value for a float

public:
                CBotExprNum();
                ~CBotExprNum();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};



// expression representing a string

class CBotExprAlpha : public CBotInstr
{
private:

public:
                CBotExprAlpha();
                ~CBotExprAlpha();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj) override;
    void        RestoreState(CBotStack* &pj, bool bMain) override;
};


#define    MAX(a,b)    ((a>b) ? a : b)


// class for the management of integer numbers (int)
class CBotVarInt : public CBotVar
{
private:
    int            m_val;            // the value
    CBotString    m_defnum;        // the name if given by DefineNum
    friend class CBotVar;

public:
                CBotVarInt( const CBotToken* name );
//                ~CBotVarInt();

    void        SetValInt(int val, const char* s = nullptr) override;
    void        SetValFloat(float val) override;
    int            GetValInt() override;
    float        GetValFloat() override;
    CBotString    GetValString() override;

    void        Copy(CBotVar* pSrc, bool bName=true) override;


    void        Add(CBotVar* left, CBotVar* right) override;    // addition
    void        Sub(CBotVar* left, CBotVar* right) override;    // substraction
    void        Mul(CBotVar* left, CBotVar* right) override;    // multiplication
    int            Div(CBotVar* left, CBotVar* right) override;    // division
    int            Modulo(CBotVar* left, CBotVar* right) override;    // remainder of division
    void        Power(CBotVar* left, CBotVar* right) override;    // power

    bool        Lo(CBotVar* left, CBotVar* right) override;
    bool        Hi(CBotVar* left, CBotVar* right) override;
    bool        Ls(CBotVar* left, CBotVar* right) override;
    bool        Hs(CBotVar* left, CBotVar* right) override;
    bool        Eq(CBotVar* left, CBotVar* right) override;
    bool        Ne(CBotVar* left, CBotVar* right) override;

    void        XOr(CBotVar* left, CBotVar* right) override;
    void        Or(CBotVar* left, CBotVar* right) override;
    void        And(CBotVar* left, CBotVar* right) override;

    void        SL(CBotVar* left, CBotVar* right) override;
    void        SR(CBotVar* left, CBotVar* right) override;
    void        ASR(CBotVar* left, CBotVar* right) override;

    void        Neg() override;
    void        Not() override;
    void        Inc() override;
    void        Dec() override;

    bool        Save0State(FILE* pf) override;
    bool        Save1State(FILE* pf) override;

};

// Class for managing real numbers (float)
class CBotVarFloat : public CBotVar
{
private:
    float        m_val;        // the value

public:
                CBotVarFloat( const CBotToken* name );
//                ~CBotVarFloat();

    void        SetValInt(int val, const char* s = nullptr) override;
    void        SetValFloat(float val) override;
    int            GetValInt() override;
    float        GetValFloat() override;
    CBotString    GetValString() override;

    void        Copy(CBotVar* pSrc, bool bName=true) override;


    void        Add(CBotVar* left, CBotVar* right) override;    // addition
    void        Sub(CBotVar* left, CBotVar* right) override;    // substraction
    void        Mul(CBotVar* left, CBotVar* right) override;    // multiplication
    int         Div(CBotVar* left, CBotVar* right) override;    // division
    int            Modulo(CBotVar* left, CBotVar* right) override;    // remainder of division
    void        Power(CBotVar* left, CBotVar* right) override;    // power

    bool        Lo(CBotVar* left, CBotVar* right) override;
    bool        Hi(CBotVar* left, CBotVar* right) override;
    bool        Ls(CBotVar* left, CBotVar* right) override;
    bool        Hs(CBotVar* left, CBotVar* right) override;
    bool        Eq(CBotVar* left, CBotVar* right) override;
    bool        Ne(CBotVar* left, CBotVar* right) override;

    void        Neg() override;
    void        Inc() override;
    void        Dec() override;

    bool        Save1State(FILE* pf) override;
};


// class for management of strings (String)
class CBotVarString : public CBotVar
{
private:
    CBotString    m_val;        // the value

public:
                CBotVarString( const CBotToken* name );
//                ~CBotVarString();

    void        SetValString(const char* p) override;
    CBotString    GetValString() override;

    void        Copy(CBotVar* pSrc, bool bName=true) override;

    void        Add(CBotVar* left, CBotVar* right) override;    // addition

    bool        Lo(CBotVar* left, CBotVar* right) override;
    bool        Hi(CBotVar* left, CBotVar* right) override;
    bool        Ls(CBotVar* left, CBotVar* right) override;
    bool        Hs(CBotVar* left, CBotVar* right) override;
    bool        Eq(CBotVar* left, CBotVar* right) override;
    bool        Ne(CBotVar* left, CBotVar* right) override;

    bool        Save1State(FILE* pf) override;
};

// class for the management of boolean
class CBotVarBoolean : public CBotVar
{
private:
    bool        m_val;        // the value

public:
                CBotVarBoolean( const CBotToken* name );
//                ~CBotVarBoolean();

    void        SetValInt(int val, const char* s = nullptr) override;
    void        SetValFloat(float val) override;
    int            GetValInt() override;
    float        GetValFloat() override;
    CBotString    GetValString() override;

    void        Copy(CBotVar* pSrc, bool bName=true) override;

    void        And(CBotVar* left, CBotVar* right) override;
    void        Or(CBotVar* left, CBotVar* right) override;
    void        XOr(CBotVar* left, CBotVar* right) override;
    void        Not() override;
    bool        Eq(CBotVar* left, CBotVar* right) override;
    bool        Ne(CBotVar* left, CBotVar* right) override;

    bool        Save1State(FILE* pf) override;
};


// class management class instances
class CBotVarClass : public CBotVar
{
private:
    static
    CBotVarClass*    m_ExClass;        // list of existing instances at some point
    CBotVarClass*    m_ExNext;        // for this general list
    CBotVarClass*    m_ExPrev;        // for this general list

private:
    CBotClass*        m_pClass;        // the class definition
    CBotVarClass*    m_pParent;        // the instance of a parent class
    CBotVar*        m_pVar;            // contents
    friend class    CBotVar;        // my daddy is a buddy WHAT? :D(\TODO mon papa est un copain )
    friend class    CBotVarPointer;    // and also the pointer
    int                m_CptUse;        // counter usage
    long            m_ItemIdent;    // identifier (unique) of an instance
    bool            m_bConstructor;    // set if a constructor has been called

public:
                CBotVarClass( const CBotToken* name, const CBotTypResult& type );
//                CBotVarClass( const CBotToken* name, CBotTypResult& type, int &nIdent );
                ~CBotVarClass();
//    void        InitCBotVarClass( const CBotToken* name, CBotTypResult& type, int &nIdent );

    void        Copy(CBotVar* pSrc, bool bName=true) override;
    void        SetClass(CBotClass* pClass) override; //, int &nIdent);
    CBotClass*    GetClass() override;
    CBotVar*    GetItem(const char* name) override;    // return an element of a class according to its name (*)
    CBotVar*    GetItemRef(int nIdent) override;

    CBotVar*    GetItem(int n, bool bExtend) override;
    CBotVar*    GetItemList() override;

    CBotString    GetValString() override;

    bool        Save1State(FILE* pf) override;
    void        Maj(void* pUser, bool bContinue) override;

    void        IncrementUse();                // a reference to incrementation
    void        DecrementUse();                // a reference to decrementation

    CBotVarClass*
                GetPointer() override;
    void        SetItemList(CBotVar* pVar);

    void        SetIdent(long n) override;

    static CBotVarClass* Find(long id);


//    CBotVar*    GetMyThis();

    bool        Eq(CBotVar* left, CBotVar* right) override;
    bool        Ne(CBotVar* left, CBotVar* right) override;

    void        ConstructorSet() override;
};


// class for the management of pointers to a class instances
class CBotVarPointer : public CBotVar
{
private:
    CBotVarClass*
                m_pVarClass;        // contents
    CBotClass*    m_pClass;            // class provided for this pointer
    friend class CBotVar;            // my daddy is a buddy

public:
                CBotVarPointer( const CBotToken* name, CBotTypResult& type );
                ~CBotVarPointer();

    void        Copy(CBotVar* pSrc, bool bName=true) override;
    void        SetClass(CBotClass* pClass) override;
    CBotClass*    GetClass() override;
    CBotVar*    GetItem(const char* name) override;    // return an element of a class according to its name (*)
    CBotVar*    GetItemRef(int nIdent) override;
    CBotVar*    GetItemList() override;

    CBotString    GetValString() override;
    void        SetPointer(CBotVar* p) override;
    CBotVarClass*
                GetPointer() override;

    void        SetIdent(long n) override;            // associates an identification number (unique)
    long        GetIdent();                    // gives the identification number associated with
    void        ConstructorSet() override;

    bool        Save1State(FILE* pf) override;
    void        Maj(void* pUser, bool bContinue) override;

    bool        Eq(CBotVar* left, CBotVar* right) override;
    bool        Ne(CBotVar* left, CBotVar* right) override;
};


// classe pour les tableaux

#define    MAXARRAYSIZE    9999

class CBotVarArray : public CBotVar
{
private:
    CBotVarClass*
                m_pInstance;                // instance manager of table

    friend class CBotVar;                    // my daddy is a buddy

public:
                CBotVarArray( const CBotToken* name, CBotTypResult& type );
                ~CBotVarArray();

    void        SetPointer(CBotVar* p) override;
    CBotVarClass*
                GetPointer() override;

    void        Copy(CBotVar* pSrc, bool bName=true) override;
    CBotVar*    GetItem(int n, bool bGrow=false) override;    // makes an element according to its numeric index
                                                // enlarged the table if necessary if bExtend
//    CBotVar*    GetItem(const char* name);        // makes a element by  literal index
    CBotVar*    GetItemList() override;                    // gives the first item in the list

    CBotString    GetValString() override;                    // gets the contents of the array into a string

    bool        Save1State(FILE* pf) override;
};


extern CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars);

extern bool TypeCompatible( CBotTypResult& type1, CBotTypResult& type2, int op = 0 );
extern bool TypesCompatibles( const CBotTypResult& type1, const CBotTypResult& type2 );

extern bool WriteWord(FILE* pf, unsigned short w);
extern bool ReadWord(FILE* pf, unsigned short& w);
extern bool ReadLong(FILE* pf, long& w);
extern bool WriteFloat(FILE* pf, float w);
extern bool WriteLong(FILE* pf, long w);
extern bool ReadFloat(FILE* pf, float& w);
extern bool WriteString(FILE* pf, CBotString s);
extern bool ReadString(FILE* pf, CBotString& s);
extern bool WriteType(FILE* pf, CBotTypResult type);
extern bool ReadType(FILE* pf, CBotTypResult& type);

extern float GetNumFloat( const char* p );

#if 0
extern void DEBUG( const char* text, int val, CBotStack* pile );
#endif

///////////////////////////////////////////
// class for routine calls (external)

class CBotCall
{
private:
    static
    CBotCall*    m_ListCalls;
    static
    void*        m_pUser;
    long        m_nFuncIdent;

private:
    CBotString    m_name;
    bool        (*m_rExec) (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser) ;
    CBotTypResult
                (*m_rComp) (CBotVar* &pVar, void* pUser)    ;
    CBotCall*    m_next;

public:
                CBotCall(const char* name,
                         bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                         CBotTypResult rCompile (CBotVar* &pVar, void* pUser));
                ~CBotCall();

    static
    bool        AddFunction(const char* name,
                            bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                            CBotTypResult rCompile (CBotVar* &pVar, void* pUser));

    static
    CBotTypResult
                CompileCall(CBotToken* &p, CBotVar** ppVars, CBotCStack* pStack, long& nIdent);
    static
    bool        CheckCall(const char* name);

//    static
//    int            DoCall(CBotToken* &p, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);
    static
    int            DoCall(long& nIdent, CBotToken* token, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);
#if    STACKRUN
    bool        Run(CBotStack* pStack);
    static
    bool        RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack);
#endif

    CBotString    GetName();
    CBotCall*    Next();

    static void    SetPUser(void* pUser);
    static void    Free();
};

// class managing the methods declared by AddFunction on a class

class CBotCallMethode
{
private:
    CBotString    m_name;
    bool        (*m_rExec) (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    CBotTypResult
                (*m_rComp) (CBotVar* pThis, CBotVar* &pVar);
    CBotCallMethode*    m_next;
    friend class CBotClass;
    long        m_nFuncIdent;

public:
                CBotCallMethode(const char* name,
                         bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                         CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));
                ~CBotCallMethode();

    CBotTypResult
                CompileCall(const char* name, CBotVar* pThis,
                            CBotVar** ppVars, CBotCStack* pStack,
                            long& nIdent);

    int            DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotVar* &pResult, CBotStack* pStack, CBotToken* pFunc);

    CBotString    GetName();
    CBotCallMethode*    Next();
    void        AddNext(CBotCallMethode* p);

};

// a list of parameters

class CBotDefParam
{
private:
    CBotToken        m_token;        // name of the parameter
    CBotString        m_typename;        // type name
    CBotTypResult    m_type;            // type of paramteter
    CBotDefParam*    m_next;            // next parameter
    long            m_nIdent;

public:
                    CBotDefParam();
                    ~CBotDefParam();
    static
    CBotDefParam*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool            Execute(CBotVar** ppVars, CBotStack* &pj);
    void            RestoreState(CBotStack* &pj, bool bMain);

    void            AddNext(CBotDefParam* p);
    int                GetType();
    CBotTypResult    GetTypResult();
    CBotDefParam*    GetNext();

    CBotString        GetParamString();
};


// a function declaration

class CBotFunction : CBotInstr
{
private:
    // management of list of (static) public functions
    static
    CBotFunction*    m_listPublic;
    CBotFunction*    m_nextpublic;
    CBotFunction*    m_prevpublic;
    friend class    CBotCStack;
//    long            m_nThisIdent;
    long            m_nFuncIdent;
    bool            m_bSynchro;        // synchronized method?

private:
    CBotDefParam*    m_Param;        // parameter list
    CBotInstr*        m_Block;        // the instruction block
    CBotFunction*    m_next;
    CBotToken        m_retToken;        // if returns CBotTypClass
    CBotTypResult    m_retTyp;        // complete type of the result

    bool            m_bPublic;        // public function
    bool            m_bExtern;        // extern function
    CBotString        m_MasterClass;    // name of the class we derive
    CBotProgram*    m_pProg;
    friend class CBotProgram;
    friend class CBotClass;

    CBotToken        m_extern;        // for the position of the word "extern"
    CBotToken        m_openpar;
    CBotToken        m_closepar;
    CBotToken        m_openblk;
    CBotToken        m_closeblk;
public:
                    CBotFunction();
                    ~CBotFunction();
    static
    CBotFunction*    Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* pFunc, bool bLocal = true);
    static
    CBotFunction*    Compile1(CBotToken* &p, CBotCStack* pStack, CBotClass*    pClass);
    bool            Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance = nullptr);
    void            RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance = nullptr);

    void            AddNext(CBotFunction* p);
    CBotTypResult    CompileCall(const char* name, CBotVar** ppVars, long& nIdent);
    CBotFunction*    FindLocalOrPublic(long& nIdent, const char* name, CBotVar** ppVars, CBotTypResult& TypeOrError, bool bPublic = true);

    int                DoCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken);
    void            RestoreCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack);
    int                DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass);
    void            RestoreCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass);
    bool            CheckParam(CBotDefParam* pParam);

    static
    void            AddPublic(CBotFunction* pfunc);

    CBotString        GetName();
    CBotString        GetParams();
    bool            IsPublic();
    bool            IsExtern();
    CBotFunction*    Next();

    bool            GetPosition(int& start, int& stop, CBotGet modestart, CBotGet modestop);
};
