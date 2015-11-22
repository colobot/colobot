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

#pragma once

// Modules inlcude
#include "CBot.h"

#include "CBotDefines.h"

// Local include

// Global include

// Forward declaration
class CBotInstr;
class CBotCall;

/*!
 * \class CBotStack
 * \brief The CBotStack class Management of the execution stack. Actually the
 * only thing it can do is to create an instance of a stack. To use for routine
 * CBotProgram :: Execute(CBotStack)
 */
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
    int             GetState();                                            // in what state am I?
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
    void*           GetPUser();
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
