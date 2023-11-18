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

#pragma once

#include "CBot/CBotDefines.h"
#include "CBot/CBotTypResult.h"
#include "CBot/CBotEnums.h"
#include "CBot/CBotVar/CBotVar.h"

#include <cstdio>
#include <string>

namespace CBot
{

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
    /**
     * \brief Block type this stack represents. This determines local variable visibility (you can only see up to top FUNCTION stack)
     */
    enum class BlockVisibilityType : unsigned short
    {
        INSTRUCTION = 0, //!< Instruction (default)
        BLOCK = 1,       //!< Code block between { ... }
        FUNCTION = 2     //!< Function - variable visibility limit
    };

    enum class IsFunction : unsigned short { NO = 0, YES = 1, EXTERNAL_CALL = 2 };

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
     */
    //@{

    /**
     * \brief Get last error
     * \param[out] start Starting position in code of the error
     * \param[out] end Ending position in code of the error
     * \return Error number
     */
    CBotError       GetError(int& start, int& end);

    /**
     * \brief Get last error
     * \return Error number
     * \see GetError(int&, int&) for error position in code
     */
    CBotError       GetError();

    /**
     * \brief Check if there was an error
     * \return false if an error occurred
     * \see GetError()
     */
    bool            IsOk();

    /**
     * \brief Set execution error unless it's already set unless you are trying to reset it
     *
     * \param n Error to set
     * \param token Token to take error position from
     * \see ResetError() to force set error
     */
    void            SetError(CBotError n, CBotToken* token = nullptr);
    /**
     * \brief Set error position to position of given token
     *
     * \param token Token to take error position from
     */
    void            SetPosError(CBotToken* token);
    /**
     * \brief Set execution error even if it is already set
     *
     * \see SetError() to set error only if it is not set already
     */
    void            ResetError(CBotError n, int start, int end);
    /**
     * \todo Document
     *
     * WARNING! Changes error to -val ...
     */
    void            SetBreak(int val, const std::string& name);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * \brief Reset the stack for execution resume - resets the error and timer
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
     * \param bUpdate true to automatically call update function for classes, see CBotClass::SetUpdateFunc()
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
     * \param bUpdate true to automatically call update function for classes, see CBotClass::SetUpdateFunc()
     * \return Found variable, nullptr if not found
     */
    CBotVar* FindVar(long ident, bool bUpdate);

    /**
     * \brief Find variable by its token and returns a copy of it
     *
     * \param pToken Token upon which search is performed
     * \param bUpdate true to automatically call update function for classes, see CBotClass::SetUpdateFunc()
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
    CBotStack*        AddStack(CBotInstr* instr = nullptr, BlockVisibilityType bBlock = BlockVisibilityType::INSTRUCTION);

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
    CBotStack*        AddStack2(BlockVisibilityType bBlock = BlockVisibilityType::INSTRUCTION);

    /**
     * \brief Adds special EOX stack marker
     *
     * \todo What is this thing?
     * Used by external calls
     *
     * \todo Document params & return
     */
    CBotStack* AddStackExternalCall(CBotExternalCall* instr = nullptr,
                                    BlockVisibilityType bBlock = BlockVisibilityType::INSTRUCTION);

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
     *
     * in case of eventual break
     */
    bool            BreakReturn(CBotStack* pfils, const std::string& name = "");
    /**
     * \todo Document
     *
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
     * Each state change causes one tick on the execution timer
     */
    //@{

    /**
     * \brief Set execution state
     * \param n New state
     * \param lim Required amount of "ticks" on the timer required to allow to continue execution. By default allows a little overflow (up to 10 ticks)
     * \return false if timer requests interruption (timer <= limit)
     */
    bool            SetState(int n, int lim = -10);
    /**
     * \brief Return current execution state
     *
     * Used when resuming execution
     *
     * \return Execution state set before interruption by SetState() and IncState()
     */
    int             GetState() { return m_state; }
    /**
     * \brief Increase the execution state by one
     * \param lim Required amount of "ticks" on the timer required to allow to continue execution. By default allows a little overflow (up to 10 ticks)
     * \return false if timer requests interruption (timer <= limit)
     */
    bool            IncState(int lim = -10);

    /**
     * \brief Check if we are in step by step execution mode
     * \return true if step by step, false otherwise
     */
    bool            IfStep();

    /**
     * \brief Resumes execution of interrupted external call
     * \return true if external call finished, false if interrupted again
     */
    bool            Execute();

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Result variable
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
    void            SetCopyVar(CBotVar* var);
    /**
     * \brief Return result variable
     * \return Variable set with SetVar() or SetCopyVar()
     */
    CBotVar*        GetVar();

    /**
     * \todo Document
     *
     * Copies the result value from m_data->retvar (m_var at a moment of SetBreak(3)) to this stack result
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

    /**
     * \brief Set program this stack level is in. Additionally marks this block as function block.
     * Note: for public functions different stack levels might be in different programs
     * \todo Refactor this two-in-one thing
     * \param p CBotProgram we are currently in
     */
    void            SetProgram(CBotProgram* p);
    /**
     * \brief Get program we are currently in
     * \param bFirst if true, get the main CBotProgram instance (the one that has the main function)
     */
    CBotProgram*    GetProgram(bool bFirst = false);

    /**
     * \brief Set user pointer for external calls
     *
     * Execution calls only - see CBotExternalCallList::SetUserPtr() for compilation calls
     *
     * \param user User pointer to set
     */
    void            SetUserPtr(void* user);
    /**
     * \brief Get user pointer for external calls
     * \returns User pointer for external execution calls
     * \see SetUserPtr()
     */
    void*           GetUserPtr();

    /**
     * \brief Get the block type this stack represents - instruction, code block or function
     * \see BlockVisibilityType enum
     */
    BlockVisibilityType GetBlock();


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Function calls
    //@{

    /**
     * \brief Execute a function call, either external or user-defined
     * \param[in, out] nIdent Unique function identifier, if not found will be updated
     * \param token Function name token
     * \param ppVar Array of function arguments
     * \param rettype Expected return type
     */
    bool            ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, const CBotTypResult& rettype);
    /**
     * \brief Restore a function call after the program state has been restored from a file
     * \param[in, out] nIdent Unique function identifier, if not found will be updated
     * \param token Function name token
     * \param ppVar Array of function arguments
     */
    void            RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar);

    //@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //! \name Write to file
    //@{

    bool            SaveState(std::ostream &ostr);
    bool            RestoreState(std::istream &istr, CBotStack* &pStack);

    //@}

    /**
     * \brief Set the maximum number of "timer ticks" (parts of instructions) to execute
     *
     * This setting gets applied on next call to Reset()
     *
     * \todo Full documentation of the timer
     */
    void            SetTimer(int n);
    /**
     * \brief Get the current configured maximum number of "timer ticks" (parts of instructions) to execute
     */
    int             GetTimer();

    /**
     * \brief Get current position in the program
     * \param[out] functionName Current function name, nullptr if not found
     * \param[out] start Start position of currently executed token
     * \param[out] end End position of currently executed token
     */
    void            GetRunPos(std::string& functionName, int& start, int& end);

    /**
     * \brief Get local variables at the given stack level
     * \param[out] functionName Name of instruction being executed at this level
     * \param level 0 for current level, -1, -2, -3 etc. for next levels
     */
    CBotVar*        GetStackVars(std::string& functionName, int level);

    bool            IsCallFinished();

    void SetExternalCallSuspended(bool);
    bool IsChildSuspended();

private:
    CBotStack*        m_next;
    CBotStack*        m_next2;
    CBotStack*        m_prev;

    int               m_state;
    int               m_step;

    struct Data;

    CBotStack::Data* m_data;

    CBotVar*        m_var;                        // result of the operations
    CBotVar*        m_listVar;                    // variables declared at this level

    BlockVisibilityType m_block;                    // is part of a block (variables are local to this block)
    bool            m_bOver;                    // stack limits?
    //! CBotProgram instance the execution is in in this stack level
    CBotProgram*    m_prog;

    //! The corresponding instruction
    CBotInstr* m_instr;
    //! If this stack level holds a function call
    IsFunction m_func;
    //! Extern call on this level (only if m_func == IsFunction::EXTERNAL_CALL)
    CBotExternalCall* m_call;

    bool m_callFinished;
    bool m_externalCallSuspended;
};

} // namespace CBot
