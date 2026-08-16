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

#include "CBot/CBotEnums.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace CBot
{

class CBotFunction;
class CBotClass;
class CBotStack;
class CBotTypResult;
class CBotVar;
class CBotExternalCallList;

/**
 * \brief Class that manages a CBot program. This is the main entry point into the CBot engine.
 *
 * \section Init Engine initialization / destruction
 * To initialize the CBot engine, call CBotProgram::Init(). This function should be only called once.
 *
 * Afterwards, you can start defining custom functions, constants and classes. See:
 * * CBotProgram::AddFunction()
 * * CBotProgram::DefineNum()
 * * CBotClass::Create()
 *
 * Next, compile and run programs.
 * * Compile()
 * * Start()
 * * Run()
 * * Stop()
 *
 * After you are finished, free the memory used by the CBot engine by calling CBotProgram::Free().
 *
 * \section Example Example usage
 * \code
 * // Initialize the engine
 * CBotProgram::Init();
 *
 * // Add some standard functions
 * CBotProgram::AddFunction("message", rMessage, cMessage);
 *
 * // Compile the program
 * std::vector<std::string> externFunctions;
 * CBotProgram* program = new CBotProgram();
 * bool ok = program->Compile(code.c_str(), externFunctions, nullptr);
 * if (!ok)
 * {
 *     CBotError error;
 *     int cursor1, cursor2;
 *     program->GetError(error, cursor1, cursor2);
 *     // Handle the error
 * }
 *
 * // Run the program
 * program->Start(externFunctions[0]);
 * while (!program->Run());
 *
 * // Cleanup
 * CBotProgram::Free();
 * \endcode
 */
class CBotProgram
{
public:
    /**
     * \brief Constructor
     */
    CBotProgram();

    /**
     * \brief Constructor
     * \param thisVar Variable to pass to the program as "this"
     */
    CBotProgram(CBotVar* thisVar);

    /**
     * \brief Destructor
     */
    ~CBotProgram();

    /**
     * \brief Initializes the module, should be done once (and only once) at the beginning
     */
    static void Init();

    /**
     * \brief Frees the static memory areas
     */
    static void Free();

    /**
     * \brief Returns version of the CBot library
     * \return A number representing the current library version
     */
    static int GetVersion();

    /**
     * \brief Compile compiles the program given as string
     *
     * Compilation is done in a few steps:
     * 1. Convert the code into "tokens" - see CBotToken::CompileTokens()
     * 2. First pass - getting declarations of all functions an classes for use later
     * 3. Second pass - compiling definitions of all functions and classes
     *
     * \param program Code to compile
     * \param[out] externFunctions Returns the names of functions declared as extern
     * \param pUser Optional pointer to be passed to compile function (see AddFunction())
     * \return true if compilation is successful, false if an compilation error occurs
     * \see GetError() to retrieve the error
     */
    bool Compile(const std::string& program, std::vector<std::string>& externFunctions, void* pUser = nullptr);

    /**
     * \brief Returns the last error
     * \return Error code
     * \see GetError(CBotError&, int&, int&) for error location in the code
     */
    CBotError GetError();

    /**
     * \brief Returns the last error
     * \param[out] code Error code
     * \param[out] start Starting position in the code string of this error
     * \param[out] end Ending position in the code string of this error
     * \return false if no error has occurred
     */
    bool GetError(CBotError& code, int& start, int& end);

    /**
     * \brief Returns the last error
     * \param[out] code Error code
     * \param[out] start Starting position in the code string of this error
     * \param[out] end Ending position in the code string of this error
     * \param[out] pProg Program that caused the error (TODO: This always returns "this"... what?)
     * \return false if no error has occurred
     */
    bool GetError(CBotError& code, int& start, int& end, CBotProgram*& pProg);

    /**
     * \brief Starts the program using given function as an entry point. The function must be declared as "extern"
     * \param name Name of function to start
     * \return true if the program was started, false if the function name is not found
     * \see Compile() returns list of extern functions found in the code
     */
    bool Start(const std::string& name);

    /**
     * \brief Executes the program
     * \param pUser Custom pointer to be passed to execute function (see AddFunction())
     * \param timer
     * \parblock
     * * timer < 0 do nothing
     * * timer >= 0 call SetTimer(int timer) before executing
     * \endparblock
     * \return true if the program execution finished, false if the program is suspended (you then have to call Run() again)
     */
    bool Run(void* pUser = nullptr, int timer = -1);

    /**
     * \brief Gives the current position in the executing program
     * \param[out] functionName Name of the currently executed function
     * \param[out] start Starting position in the code string of currently executed instruction
     * \param[out] end Ending position in the code string of currently executed instruction
     * \return false if it is not running (program completion)
     */
    bool GetRunPos(std::string& functionName, int& start, int& end);

    /**
     * \brief Provides the pointer to the variables on the execution stack
     * \param[out] functionName Name of the function that this stack is part of
     * \param level 0 for the last level, -1, -2 etc. for previous ones
     * \return Variables on the given stack level. Process using CBotVar::GetNext(). If the stack level is invalid, returns nullptr.
     */
    CBotVar* GetStackVars(std::string& functionName, int level);

    /**
     * \brief Stops execution of the program
     */
    void Stop();

    /**
     * \brief Add a function that can be called from CBot
     *
     * To define an external function, proceed as follows:
     *
     * 1. Define a function for compilation
     *
     * This function should take a list of function arguments (types only, no values!) and a user-defined void* pointer (can be passed in Compile()) as parameters, and return CBotTypResult.
     *
     * Usually, name of this function is prefixed with "c".
     *
     * The function should iterate through the provided parameter list and verify that they match.<br>
     * If they don't, then return CBotTypResult with an appropariate error code (see ::CBotError).<br>
     * If they do, return CBotTypResult with the function's return type
     *
     * \code
     * CBotTypResult cMessage(CBotVar* &var, void* user)
     * {
     *     if (var == nullptr) return CBotTypResult(CBotErrLowParam); // Not enough parameters
     *     if (var->GetType() != CBotTypString) return CBotTypResult(CBotErrBadString); // String expected
     *
     *     var = var->GetNext(); // Get the next parameter
     *     if (var != nullptr) return CBotTypResult(CBotErrOverParam); // Too many parameters
     *
     *     return CBotTypResult(CBotTypFloat); // This function returns float (it may depend on parameters given!)
     * }
     * \endcode
     *
     * 2. Define a function for execution
     *
     * This function should take:
     * * a list of parameters
     * * pointer to a result variable (a variable of type given at compilation time will be provided)
     * * pointer to an exception variable
     * * user-defined pointer (can be passed in Run())
     *
     * This function returns true if execution of this function is finished, or false to suspend the program and call this function again on next Run() cycle.
     *
     * Usually, execution functions are prefixed with "r".
     *
     * \code
     * bool rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
     * {
     *     std::string message = var->GetValString();
     *     std::cout << message << std::endl;
     *     return true; // Execution finished
     * }
     * \endcode
     *
     * 3. Call AddFunction() to register the function in the CBot engine
     *
     * \code
     * AddFunction("message", rMessage, cMessage);
     * \endcode
     *
     * For more sophisticated examples, see the Colobot source code, mainly the src/script/scriptfunc.cpp file
     *
     * \param name Name of the function
     * \param rExec Execution function
     * \param rCompile Compilation function
     * \param rCancel Function to cancel execution
     * \return true
     */
    static bool AddFunction(const std::string& name,
                            bool rExec(CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                            CBotTypResult rCompile(CBotVar*& pVar, void* pUser),
                            void rCancel(void* pUser) = nullptr);

    /**
     * \copydoc CBotToken::DefineNum()
     * \see CBotToken::DefineNum()
     */
    static bool DefineNum(const std::string& name, long val);

    /**
     * \brief Save the current execution status into a file
     * \param ostr Output stream
     * \return true on success, false on write error
     */
    bool SaveState(std::ostream &ostr);

    /**
     * \brief Restore the execution state from a file
     *
     * The previous program code must already have been recompiled with Compile() before calling this function
     *
     * \param istr Input stream
     * \return true on success, false on read error
     */
    bool RestoreState(std::istream &istr);

    /**
     * \brief GetPosition Gives the position of a routine in the original text
     * the user can select the item to find from the beginning to the end
     * see the above modes in CBotGet.
     *
     * TODO: Document this
     *
     * \param name
     * \param start
     * \param stop
     * \param modestart
     * \param modestop
     * \return
     */
    bool GetPosition(const std::string& name,
                     int& start,
                     int& stop,
                     CBotGet modestart = GetPosExtern,
                     CBotGet modestop = GetPosBloc);

    /**
     * \brief Returns the list of all user-defined functions in this program as instances of CBotFunction
     *
     * This list includes all the functions (not only extern)
     *
     * \return List of CBotFunction instances
     */
    const std::list<CBotFunction*>& GetFunctions();

    /**
     * \brief Check if class with that name was created in this program
     * \return True if class was defined in this program, otherwise, false
     */
    bool ClassExists(std::string name);

    /**
     * \brief Returns static list of all registered external calls
     */
    static const std::unique_ptr<CBotExternalCallList>& GetExternalCalls();

private:
    //! All external calls
    static std::unique_ptr<CBotExternalCallList> m_externalCalls;
    //! All user-defined functions
    std::list<CBotFunction*> m_functions{};
    //! The entry point function
    CBotFunction* m_entryPoint = nullptr;
    //! Classes defined in this program
    std::list<CBotClass*> m_classes{};
    //! Execution stack
    CBotStack* m_stack = nullptr;
    //! "this" variable
    CBotVar* m_thisVar = nullptr;
    friend class CBotFunction;
    friend class CBotDebug;

    CBotError m_error = CBotNoErr;
    int m_errorStart = 0;
    int m_errorEnd = 0;
};

} // namespace CBot
