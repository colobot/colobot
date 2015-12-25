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

#include "CBot/CBotDefines.h"
#include "CBot/CBotTypResult.h"
#include "CBotEnums.h"

#include <cstdio>
#include <string>
#include <CBot/CBotVar/CBotVar.h>

class CBotInstr;
class CBotExternalCall;
class CBotVar;
class CBotProgram;
class CBotToken;

/**
 * \brief The execution stack
 *
 * \nosubgrouping
 */
class CBotStack
{
public:
    enum class IsBlock : unsigned short { INSTRUCTION = 0, BLOCK = 1, FUNCTION = 2 }; // TODO: figure out what these mean ~krzys_h
    enum class IsFunction : unsigned short { NO = 0, TRUE = 1, EXTERNAL_CALL = 2 }; // TODO: just guessing the meaning of values, should be verified ~krzys_h

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Stack memory management
    //@{

    /**
     * \brief Allocate the stack
     * \return pointer to created stack
     */
    static CBotStack* AllocateStack();

    /** \brief Remove the current stack */
    void Delete();

    CBotStack() = delete;
    ~CBotStack() = delete;

    /**
     * \brief Check for stack overflow and set error status as needed
     * \return true on stack overflow
     */
    bool StackOver();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /** \name Error management
     *
     * Be careful - errors are stored in static variables!
     * \todo Refactor that
     */
    //@{

    /**
     * \brief Get last error
     * \param[out] start Starting position in code of the error
     * \param[out] end Ending position in code of the error
     * \return Error number
     */
    CBotError GetError(int& start, int& end) { start = m_start; end = m_end; return m_error; }

    /**
     * \brief Get last error
     * \return Error number
     * \see GetError(int&, int&) for error position in code
     */
    CBotError GetError() { return m_error; }

    /**
     * \brief Check if there was an error
     * \return false if an error occured
     * \see GetError()
     */
    bool IsOk()
    {
        return m_error == CBotNoErr;
    }

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * \brief Reset the stack - resets the error and timer
     */
    void Reset();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Local variables
    //@{

    /**
     * \brief Adds a local variable
     * \param var Variable to be added
     */
    void AddVar(CBotVar* var);

    /**
     * \brief Fetch a variable by its token
     * \param pToken Token upon which search is performed
     * \param bUpdate true to automatically call update function for classes, see CBotClass::AddUpdateFunc()
     * \return Found variable, nullptr if not found
     */
    CBotVar* FindVar(CBotToken*& pToken, bool bUpdate);

    /**
     * \copydoc FindVar(CBotToken*&, bool)
     */
    CBotVar* FindVar(CBotToken& pToken, bool bUpdate);

    /**
     * \brief Fetch variable by its name
     * \param name Name of variable to find
     * \return Found variable, nullptr if not found
     */
    CBotVar* FindVar(const std::string& name);

    /**
     * \brief Fetch a variable on the stack according to its unique identifier
     *
     * This is faster than comparing names
     *
     * \param ident Unique identifier of a variable
     * \param bUpdate true to automatically call update function for classes, see CBotClass::AddUpdateFunc()
     * \return Found variable, nullptr if not found
     */
    CBotVar* FindVar(long ident, bool bUpdate);

    /**
     * \brief Find variable by its token and returns a copy of it
     *
     * \param pToken Token upon which search is performed
     * \param bUpdate true to automatically call update function for classes, see CBotClass::AddUpdateFunc()
     * \return Found variable, nullptr if not found
     */
    CBotVar*        CopyVar(CBotToken& pToken, bool bUpdate = false);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /** \name Child stacks
     *
     * When you enter a new code block or instruction, child stack is created
     * for managing everything that happens inside that block / instruction.
     */
    //@{

    /**
     * \brief Creates or gets the primary child stack
     *
     * If the stack already exists, it is returned.
     * Otherwise, a new stack is created.
     *
     * \todo Document params
     * \returns New stack element
     */
    CBotStack*        AddStack(CBotInstr* instr = nullptr, IsBlock bBlock = IsBlock::INSTRUCTION);

    /**
     * \brief Creates or gets the secondary child stack
     * \todo What is it used for?
     *
     * If the stack already exists, it is returned.
     * Otherwise, a new stack is created.
     *
     * \see AddStack()
     * \return New stack element
     */
    CBotStack*        AddStack2(IsBlock bBlock = IsBlock::INSTRUCTION);

    /**
     * \brief Adds special EOX stack marker
     *
     * \todo What is this thing?
     * Used by external calls
     *
     * \todo Document params & return
     */
    CBotStack*        AddStackEOX(CBotExternalCall* instr = nullptr, IsBlock bBlock = IsBlock::INSTRUCTION);

    /**
     * \brief Restore CBotInstr pointer after loading stack from file
     * \todo Check what this does exactly
     */
    CBotStack*        RestoreStack(CBotInstr* instr = nullptr);
    /**
     * \brief Restores CBotExternalCall in the EOX marker after loading stack from file
     * \todo Check what this does exactly
     */
    CBotStack*        RestoreStackEOX(CBotExternalCall* instr = nullptr);

    /**
     * \brief Return to this point - copy the result from given stack, and destroy all child stacks from here
     *
     * \todo Better description
     *
     * \param pFils Stack to copy result from
     * \return IsOk()
     */
    bool            Return(CBotStack* pFils);
    /**
     * \brief Like Return() but doesn't destroy the stacks
     *
     * \param pFils Stack to copy result from
     * \return IsOk()
     */
    bool            ReturnKeep(CBotStack* pFils);

    /**
     * \todo Document
     * in case of eventual break
     */
    bool            BreakReturn(CBotStack* pfils, const std::string& name = nullptr);
    /**
     * \todo Document
     * or "continue"
     */
    bool            IfContinue(int state, const std::string& name);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /** \name Preserving execution status
     *
     * A "state" is a number that determines how much of CBotInstr::Execute() has been already executed.
     * When CBotInstr::Execute() is called, it continues execution from the point it finished at.
     * See various CBotInstr::Execute() implementations for details.
     *
     * Call CBotStack::Execute() to resume execution of the interrupted instruction
     *
     * \todo More detailed docs on functions
     */
    //@{

    bool            SetState(int n, int lim = -10);                        // select a state
    int             GetState() { return m_state; }                         // in what state am I?
    bool            IncState(int lim = -10);                            // passes to the next state
    bool            IfStep();                                            // do step by step
    bool            Execute();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // \name Result variable

    //@{

    /**
     * \brief Set the result variable
     * \todo CBotStack takes over the ownership - use std::unique_ptr here
     * \param var Result variable to set
     */
    void            SetVar(CBotVar* var);
    /**
     * \brief Set the result variable to copy of given variable
     * \param var Variable to copy as result
     */
    void            SetCopyVar( CBotVar* var );
    /**
     * \brief Return result variable
     * \return Variable set with SetVar() or SetCopyVar()
     */
    CBotVar*        GetVar();
    /**
     * \todo Document
     */
    bool            GetRetVar(bool bRet);
    /**
     * \brief Return the result variable as int
     * \deprecated Please use GetVar()->GetValInt() instead
     * \todo Remove
     * \return GetVar()->GetValInt(), or 0 if GetVar() == nullptr
     */
    long            GetVal();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void            SetError(CBotError n, CBotToken* token = nullptr);
    void            SetPosError(CBotToken* token);
    void            ResetError(CBotError n, int start, int end);
    void            SetBreak(int val, const std::string& name);

    void            SetProgram(CBotProgram* p);
    CBotProgram* GetProgram(bool bFirst = false);
    void            SetUserPtr(void* user);
    void*           GetUserPtr();
    IsBlock GetBlock();


    bool            ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype);
    void            RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar);

    bool            SaveState(FILE* pf);
    bool            RestoreState(FILE* pf, CBotStack* &pStack);

    static void     SetTimer(int n);

    void            GetRunPos(std::string& FunctionName, int& start, int& end);
    CBotVar*        GetStackVars(std::string& FunctionName, int level);

private:
    CBotStack*        m_next;
    CBotStack*        m_next2;
    CBotStack*        m_prev;
    friend class CBotInstArray;

    int                m_state;
    int                m_step;
    static CBotError  m_error;
    static int        m_start;
    static int        m_end;
    static
    CBotVar*        m_retvar;                    // result of a return

    CBotVar*        m_var;                        // result of the operations
    CBotVar*        m_listVar;                    // variables declared at this level

    IsBlock m_bBlock;                    // is part of a block (variables are local to this block)
    bool            m_bOver;                    // stack limits?
//    bool            m_bDontDelete;                // special, not to destroy the variable during delete
    CBotProgram*    m_prog;                        // user-defined functions

    static
    int                m_initimer;
    static
    int                m_timer;
    static
    std::string        m_labelBreak;
    static
    void*            m_pUser;

    CBotInstr*        m_instr;                    // the corresponding instruction
    IsFunction m_bFunc;                    // an input of a function?
    CBotExternalCall*        m_call;                        // recovery point in a extern call
    friend class    CBotTry;
};
