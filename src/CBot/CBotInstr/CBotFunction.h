/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotInstr.h"

#include <set>

namespace CBot
{

/**
 * \brief A function declaration in the code
 *
 * Examples:
 * \code
 * void test() { ... }
 * void test(int a, float b) { ... }
 * int test(int a, float b, string c) { ... }
 * public bool test(int a, float b, string c, SomeClass d) { ... }
 * extern void test() { ... }
 * void classname::test() { ... }
 * \endcode
 */
class CBotFunction : public CBotInstr
{
public:
    CBotFunction();
    ~CBotFunction();

    /*!
     * \brief Compile Compiles a new function
     * \param p
     * \param pStack
     * \param pFunc
     * \param bLocal allows of the declaration of parameters on the same level
     * as the elements belonging to the class for methods.
     * \return
     */
    static CBotFunction* Compile(CBotToken* &p,
                                 CBotCStack* pStack,
                                 CBotFunction* pFunc,
                                 bool bLocal = true);

    /*!
     * \brief Pre-compile a new function
     * \param p[in, out] Pointer to first token of the function, will be updated to point to first token after the function definition
     * \param pStack Compile stack
     * \param pClass If this is a class method, pointer to class this function is part of, otherwise nullptr
     *
     * This function is used to find the beginning and end of function definition.
     *
     * If any errors in the code are detected, this function will set the error on compile stack and return nullptr.
     *
     * \return Precompiled function, or nullptr in case of error
     */
    static CBotFunction* Compile1(CBotToken* &p,
                                  CBotCStack* pStack,
                                  CBotClass* pClass);

    /*!
     * \brief Execute
     * \param ppVars
     * \param pj
     * \param pInstance
     * \return
     */
    bool Execute(CBotVar** ppVars,
                 CBotStack* &pj,
                 CBotVar* pInstance = nullptr);

    using CBotInstr::Execute;

    /*!
     * \brief RestoreState
     * \param ppVars
     * \param pj
     * \param pInstance
     */
    void RestoreState(CBotVar** ppVars,
                      CBotStack* &pj,
                      CBotVar* pInstance = nullptr);

    using CBotInstr::RestoreState;

    /*!
     * \brief Compile a function call
     *
     * See FindLocalOrPublic for more detailed explanation
     *
     * \param localFunctionList Linked list of local functions to search in, can be null
     * \param name Name of the function
     * \param ppVars List of function arguments
     * \param nIdent[in, out] Unique identifier of the function
     * \return Type returned by the function or error code
     * \see FindLocalOrPublic
     */
    static CBotTypResult CompileCall(const std::list<CBotFunction*>& localFunctionList,
                                     const std::string &name, CBotVar** ppVars, long &nIdent);

    /*!
     * \brief Finds a local or public function
     *
     * <p>Finds a local or (if bPublic is true) public function to call
     *
     * <p>First, it looks for a function according to its unique identifier.<br>
     * If the identifier is not found, looks by name and parameters.
     *
     * \param localFunctionList Linked list of local functions to search in, can be null
     * \param nIdent[in, out] Unique identifier of the function
     * \param name Name of the function
     * \param ppVars List of function arguments
     * \param TypeOrError Type returned by the function or error code
     * \param bPublic Whether to look in public functions or not
     * \return Pointer to found CBotFunction instance, or nullptr in case of no match or ambiguity (see TypeOrError for error code)
     */
    static CBotFunction* FindLocalOrPublic(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                                           CBotVar** ppVars, CBotTypResult &TypeOrError, bool bPublic = true);

    /*!
     * \brief DoCall Fait un appel à une fonction.
     * \param program
     * \param localFunctionList
     * \param nIdent
     * \param name
     * \param ppVars
     * \param pStack
     * \param pToken
     * \return
     */

    static int DoCall(CBotProgram* program, const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                      CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken);

    /*!
     * \brief RestoreCall
     * \param localFunctionList
     * \param nIdent
     * \param name
     * \param ppVars
     * \param pStack
     */
    static void RestoreCall(const std::list<CBotFunction*>& localFunctionList,
                            long &nIdent, const std::string &name, CBotVar** ppVars, CBotStack* pStack);

    /*!
     * \brief DoCall Makes call of a method
     * note: this is already on the stack, the pointer pThis is just to simplify.
     * \param localFunctionList
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppVars
     * \param pStack
     * \param pToken
     * \param pClass
     * \return
     */
    static int DoCall(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name, CBotVar* pThis,
                      CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass);

    /*!
     * \brief RestoreCall
     * \param localFunctionList
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppVars
     * \param pStack
     * \param pClass
     * \return Returns true if the method call was restored.
     */
    static bool RestoreCall(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name, CBotVar* pThis,
                            CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass);

    /*!
     * \brief CheckParam See if the "signature" of parameters is identical.
     * \param pParam
     * \return
     */
    bool CheckParam(CBotDefParam* pParam);

    /*!
     * \brief AddPublic
     * \param pfunc
     */
    static void AddPublic(CBotFunction* pfunc);

    /*!
     * \brief GetName
     * \return
     */
    std::string GetName();

    /*!
     * \brief GetParams
     * \return
     */
    std::string GetParams();

    /*!
     * \brief IsPublic
     * \return
     */
    bool IsPublic();

    /*!
     * \brief IsExtern
     * \return
     */
    bool IsExtern();

    /*!
     * \brief GetPosition
     * \param start
     * \param stop
     * \param modestart
     * \param modestop
     * \return
     */
    bool GetPosition(std::size_t& start, std::size_t& stop,
                     CBotGet modestart,
                     CBotGet modestop);

    /*!
     * \brief Check if the function has a return statment that will execute.
     * \return true if a return statment was found.
     */
    bool HasReturn() override;

protected:
    virtual const std::string GetDebugName() override { return "CBotFunction"; }
    virtual std::string GetDebugData() override;
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    friend class CBotDebug;
    long m_nFuncIdent;
    //! Synchronized method.
    bool m_bSynchro;

    //! Parameter list.
    CBotDefParam* m_param;
    //! The instruction block.
    CBotInstr* m_block;
    //! If returns CBotTypClass.
    CBotToken m_retToken;
    //! Complete type of the result.
    CBotTypResult m_retTyp;
    //! Public function.
    bool m_bPublic;
    //! Extern function.
    bool m_bExtern;
    //! Name of the class we are part of
    std::string m_MasterClass;
    //! Token of the class we are part of
    CBotToken m_classToken;
    CBotProgram* m_pProg;
    //! For the position of the word "extern".
    CBotToken m_extern;
    CBotToken m_openpar;
    CBotToken m_closepar;
    CBotToken m_openblk;
    CBotToken m_closeblk;

    //! List of public functions
    static std::set<CBotFunction*> m_publicFunctions;

    friend class CBotProgram;
    friend class CBotClass;
    friend class CBotCStack;

};

} // namespace CBot
