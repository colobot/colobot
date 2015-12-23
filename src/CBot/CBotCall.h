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

#include "CBot/CBotUtils.h"
#include "CBot/CBotEnums.h"
#include "CBot/CBotDefines.h"

#include <string>
#include <map>
#include <memory>

class CBotStack;
class CBotCStack;
class CBotVar;
class CBotTypResult;
class CBotToken;

/**
 * \brief Class used for external calls
 *
 * \see CBotProgram::AddFunction() for information on how to add your functions to this list
 */
class CBotCall : public CBotLinkedList<CBotCall>
{
public:
    typedef bool (*RuntimeFunc)(CBotVar* args, CBotVar* result, int& exception, void* user);
    typedef CBotTypResult (*CompileFunc)(CBotVar*& args, void* user);

    /**
     * \brief Constructor
     * \param name Function name
     * \param rExec Runtime function
     * \param rCompile Compilation function
     * \see CBotProgram::AddFunction()
     */
    CBotCall(const std::string& name, RuntimeFunc rExec, CompileFunc rCompile);

    /**
     * \brief Destructor
     */
    ~CBotCall();

    /**
     * \brief Add a new function to the list
     * \param name Function name
     * \param rExec Runtime function
     * \param rCompile Compilation function
     * \return true
     */
    static bool AddFunction(const std::string& name, RuntimeFunc rExec, CompileFunc rCompile);

    /**
     * \brief Find and call compile function
     *
     * \todo Document
     */
    static CBotTypResult CompileCall(CBotToken* &p, CBotVar** ppVars, CBotCStack* pStack, long& nIdent);

    /**
     * \brief Check if function with given name has been defined
     * \param name Name to check
     * \return true if function was defined
     */
    static bool CheckCall(const std::string& name);

    /**
     * \brief Find and call runtime function
     *
     * \todo Document
     */
    static int DoCall(long& nIdent, CBotToken* token, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);

    /**
     * \brief Execute the runtime function
     * \param pStack Stack to execute the function on
     * \return false if function requested interruption, true otherwise
     */
    bool Run(CBotStack* pStack);

    /**
     * \brief Restore execution status after loading saved state
     *
     * \todo Document
     */
    static bool RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack);

    /**
     * \brief Set user pointer to pass to compile/runtime functions
     * \param pUser User pointer
     */
    static void SetUserPtr(void* pUser);

    /**
     * \brief Reset the list of registered functions
     */
    static void Clear();

private:
    static std::map<std::string, std::unique_ptr<CBotCall>> m_list;
    static void* m_user;

    long m_ident;
    std::string m_name;
    RuntimeFunc m_rExec;
    CompileFunc m_rComp;
};
