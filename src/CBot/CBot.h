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
