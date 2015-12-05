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
#include "CBot/CBotTypResult.h"
#include "CBot/CBotString.h"
#include "CBot/CBotStringArray.h"

#include "CBot/CBotEnums.h"

// Local include

// Global include

// Forward declaration
class CBotFunction;
class CBotClass;
class CBotStack;
class CBotVar;

/*!
 * \brief The CBotProgram class Main class managing CBot program.
 */
class CBotProgram
{
public:

    /*!
     * \brief CBotProgram
     */
    CBotProgram();

    /*!
     * \brief CBotProgram
     * \param pInstance
     */
    CBotProgram(CBotVar* pInstance);

    /*!
     * \brief ~CBotProgram
     */
    ~CBotProgram();

    /*!
     * \brief Init Initializes the module (defined keywords for errors) should
     * be done once (and only one) at the beginning.
     */
    static void Init();

    /*!
     * \brief Free Frees the static memory areas.
     */
    static void Free();

    /*!
     * \brief GetVersion Gives the version of the library CBOT.
     * \return
     */
    static int GetVersion();

    /*!
     * \brief Compile compiles the program given in text.
     * \param program
     * \param ListFonctions Returns the names of functions declared as extern.
     * \param pUser Can pass a pointer to routines defined by AddFunction.
     * \return false if an error at compile.
     * \see GetCompileError() to retrieve the error.
     */
    bool Compile( const char* program, CBotStringArray& ListFonctions, void* pUser = nullptr);

    /*!
     * \brief SetIdent Associates an identifier with the instance CBotProgram.
     * \param n
     */
    void SetIdent(long n);

    /*!
     * \brief GetIdent Gives the identifier.
     * \return
     */
    long GetIdent();

    /*!
     * \brief GetError
     * \return
     */
    int GetError();

    /*!
     * \brief GetError
     * \param code
     * \param start
     * \param end
     * \return
     */
    bool GetError(int& code, int& start, int& end);

    /*!
     * \brief GetError
     * \param code
     * \param start Delimits the start block where the error occured.
     * \param end Delimits the end block where the error occured.
     * \param pProg Lets you know what "module" has produced runtime error.
     * \return If true gives the error found in the compilation or execution.
     */
    bool GetError(int& code, int& start, int& end, CBotProgram* &pProg);

    /*!
     * \brief GetErrorText
     * \param code
     * \return
     */
    static CBotString GetErrorText(int code);

    /*!
     * \brief Start Defines what function should be executed. The program does
     * nothing, we must call Run () for this.
     * \param name
     * \return false if the funtion name is not found
     */
    bool Start(const char* name);

    /*!
     * \brief Run Executes the program.
     * \param pUser
     * \param timer timer = 0 allows to advance step by step.
     * \return false if the program was suspended, true if the program ended
     * with or without error.
     */
    bool Run(void* pUser = nullptr, int timer = -1);

    /*!
     * \brief GetRunPos Gives the position in the executing program
     * \param FunctionName is a pointer made to the name of the function
     * \param start start and end position in the text of the token processing
     * \param end
     * \return false if it is not running (program completion)
     */
    bool GetRunPos(const char* &FunctionName, int &start, int &end);

    /*!
     * \brief GetStackVars provides the pointer to the variables on the
     * execution stack level is an input parameter,  0 for the last level, -1,
     * -2, etc. for the other levels the return value (CBotVar *) is a variable.
     * that can be processed as the list of parameters received by a routine
     * list (or nullptr)
     * \param FunctionName gives the name of the function where are these
     * variables. FunctionName == nullptr means that is more in a program
     * (depending on  level)
     * \param level
     * \return
     */
    CBotVar* GetStackVars(const char* &FunctionName, int level);

    /*!
     * \brief Stop stops execution of the program therefore quits "suspend" mode
     */
    void Stop();

    /*!
     * \brief SetTimer defines the number of steps (parts of instructions) to
     * done in Run() before rendering hand "false"
     * \TODO avant de rendre la main "false"
     * \param n
     */
    static void SetTimer(int n);
    //
    //

    /*!
     * \brief AddFunction Call this to add externally (**) a new function used
     * by the program CBoT.
     * \param name
     * \param rExec
     * \param rCompile
     * \return
     */
    static bool AddFunction(const char* name,
                                bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                                CBotTypResult rCompile (CBotVar* &pVar, void* pUser));

    /*!
     * \brief DefineNum
     * \param name
     * \param val
     * \return
     */
    static bool DefineNum(const char* name, long val);

    /*!
     * \brief SaveState Backup the execution status in the file the file must
     * have been opened with the fopen call this dll (\TODO this library??)
     * if the system crashes
     * \param pf
     * \return
     */
    bool SaveState(FILE* pf);

    /*!
     * \brief RestoreState Restores the state of execution from file the
     * compiled program must obviously be the same.
     * \param pf
     * \return
     */
    bool RestoreState(FILE* pf);

    /*!
     * \brief GetPosition Gives the position of a routine in the original text
     * the user can select the item to find from the beginning to the end
     * see the above modes in CBotGet.
     * \param name
     * \param start
     * \param stop
     * \param modestart
     * \param modestop
     * \return
     */
    bool GetPosition(const char* name,
                     int& start,
                     int& stop,
                     CBotGet modestart = GetPosExtern,
                     CBotGet modestop  = GetPosBloc);

    /*!
     * \brief GetFunctions
     * \return
     */
    CBotFunction* GetFunctions();

    /*!
     * \brief m_bCompileClass
     */
    bool m_bCompileClass;

private:

    //! The user-defined functions.
    CBotFunction* m_Prog;
    //! The basic function for the execution.
    CBotFunction* m_pRun;
    //! Classes defined in this part.
    CBotClass* m_pClass;
    //! Execution stack.
    CBotStack* m_pStack;
    //! Instance of the parent class.
    CBotVar* m_pInstance;
    friend class CBotFunction;

    int m_ErrorCode;
    int m_ErrorStart;
    int m_ErrorEnd;
    //! Associated identifier.
    long m_Ident;
};
