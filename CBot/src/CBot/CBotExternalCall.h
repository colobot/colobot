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

#include "CBot/CBotUtils.h"
#include "CBot/CBotEnums.h"
#include "CBot/CBotDefines.h"

#include <string>
#include <map>
#include <memory>

namespace CBot
{

class CBotStack;
class CBotCStack;
class CBotVar;
class CBotTypResult;
class CBotToken;

/**
 * \brief Interface for external CBot calls
 *
 * \see CBotExternalCallList
 * \see CBotExternalCallDefault
 */
class CBotExternalCall
{
public:
    /**
     * \brief Constructor
     * \see CBotProgram::AddFunction()
     */
    CBotExternalCall();

    /**
     * \brief Destructor
     */
    virtual ~CBotExternalCall();

    /**
     * \brief Compile the function
     *
     * \param thisVar "this" variable for class calls, nullptr for normal calls
     * \param args Arguments (only types!) passed to the function
     * \param user User pointer provided to CBotProgram::Compile()
     */
    virtual CBotTypResult Compile(CBotVar* thisVar, CBotVar* args, void* user) = 0;

    /**
     * \brief Execute the function
     *
     * \param thisVar "this" variable for class calls, nullptr for normal calls
     * \param pStack Stack to execute the function on
     * \return false to request program interruption, true otherwise
     */
    virtual bool Run(CBotVar* thisVar, CBotStack* pStack) = 0;

    virtual void Cancel(CBotStack* pStack) = 0;
};

/**
 * \brief Default implementation of CBot external call, using compilation and runtime functions
 */
class CBotExternalCallDefault : public CBotExternalCall
{
public:
    typedef bool (*RuntimeFunc)(CBotVar* args, CBotVar* result, int& exception, void* user);
    typedef CBotTypResult (*CompileFunc)(CBotVar*& args, void* user);
    typedef void (*CancelFunc)(void* user);

    /**
     * \brief Constructor
     * \param rExec Runtime function
     * \param rCompile Compilation function
     * \param rCancel Function to cancel execution
     * \see CBotProgram::AddFunction()
     */
    CBotExternalCallDefault(RuntimeFunc rExec, CompileFunc rCompile, CancelFunc rCancel);

    /**
     * \brief Destructor
     */
    virtual ~CBotExternalCallDefault();

    virtual CBotTypResult Compile(CBotVar* thisVar, CBotVar* args, void* user) override;
    virtual bool Run(CBotVar* thisVar, CBotStack* pStack) override;
    virtual void Cancel(CBotStack* pStack) override;

private:
    RuntimeFunc m_rExec;
    CompileFunc m_rComp;
    CancelFunc m_rCancel;
};

/**
 * \brief Implementation of CBot external call for class methods, using compilation and runtime functions
 */
class CBotExternalCallClass : public CBotExternalCall
{
public:
    typedef bool (*RuntimeFunc)(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    typedef CBotTypResult (*CompileFunc)(CBotVar* pThis, CBotVar*& pVar);

    /**
     * \brief Constructor
     * \param rExec Runtime function
     * \param rCompile Compilation function
     * \see CBotProgram::AddFunction()
     */
    CBotExternalCallClass(RuntimeFunc rExec, CompileFunc rCompile);

    /**
     * \brief Destructor
     */
    virtual ~CBotExternalCallClass();

    virtual CBotTypResult Compile(CBotVar* thisVar, CBotVar* args, void* user) override;
    virtual bool Run(CBotVar* thisVar, CBotStack* pStack) override;
    virtual void Cancel(CBotStack* pStack) override;

private:
    RuntimeFunc m_rExec;
    CompileFunc m_rComp;
};


/**
 * \brief Class for mangaging CBot external calls
 *
 * \see CBotProgram::AddFunction() for information on how to add your functions to this list
 */
class CBotExternalCallList
{
public:
    /**
     * \brief Add a new function to the list
     * \param name Function name
     * \param call Function to add
     * \return true
     */
    bool AddFunction(const std::string& name, std::unique_ptr<CBotExternalCall> call);

    /**
     * \brief Find and call compile function
     *
     * This function sets an error in compilation stack in case of failure
     *
     * \param p Token representing the function name
     * \param thisVar "this" variable for class calls, nullptr for normal calls
     * \param ppVars List of arguments (only types!)
     * \param pStack Compilation stack
     * \return CBotTypResult representing the return type of the function (::CBotType), or an error (::CBotError)
     */
    CBotTypResult CompileCall(CBotToken*& p, CBotVar* thisVar, CBotVar** ppVars, CBotCStack* pStack);

    /**
     * \brief Check if function with given name has been defined
     * \param name Name to check
     * \return true if function was defined
     */
    bool CheckCall(const std::string& name);

    /**
     * \brief Find and call runtime function
     *
     * This function sets an error in runtime stack in case of failure
     *
     * \param token Token representing the function name
     * \param thisVar "this" variable for class calls, nullptr for normal calls
     * \param ppVars List of arguments
     * \param pStack Runtime stack
     * \param rettype Return type of the function, as returned by CompileCall()
     * \return -1 if call failed (no such function), 0 if function requested interruption, 1 on success
     */
    int DoCall(CBotToken* token, CBotVar* thisVar, CBotVar** ppVars, CBotStack* pStack, const CBotTypResult& rettype);

    /**
     * \brief Restore execution status after loading saved state
     *
     * \param token Token representing the function name
     * \param thisVar "this" variable for class calls, nullptr for normal calls
     * \param ppVar List of arguments
     * \param pStack Runtime stack
     * \return false on failure (e.g. function doesn't exist)
     */
    bool RestoreCall(CBotToken* token, CBotVar* thisVar, CBotVar** ppVar, CBotStack* pStack);

    /**
     * \brief Set user pointer to pass to compile functions
     *
     * This is for compile functions only, runtime functions use CBotStack::GetUserPtr()
     *
     * \param pUser User pointer
     */
    void SetUserPtr(void* pUser);

    /**
     * \brief Reset the list of registered functions
     */
    void Clear();

private:
    std::map<std::string, std::unique_ptr<CBotExternalCall>> m_list{};
    void* m_user = nullptr;
};

} // namespace CBot
