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
    CBotFunction(CBotProgram& prog);
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
     * \param name Name of the function
     * \param ppVars List of function arguments
     * \param nIdent[in, out] Unique identifier of the function
     * \param program The current program, to search for functions.
     * \return Type returned by the function or error code
     * \see FindLocalOrPublic
     */
    static CBotTypResult CompileCall(const std::string &name, CBotVar** ppVars,
                                     long &nIdent, CBotProgram* program);

    /*!
     * \brief Finds a local or public function
     *
     * <p>First, it looks for a function according to its unique identifier.<br>
     * If the identifier is not found, looks by name and parameters.
     *
     * \param localFunctionList Linked list of local functions to search in, can be null
     * \param nIdent[in, out] Unique identifier of the function
     * \param name Name of the function
     * \param ppVars List of function arguments
     * \param TypeOrError Type returned by the function or error code
     * \param baseProg Initial program, for context of the object/bot
     * \return Pointer to found CBotFunction instance, or nullptr in case of no match or ambiguity (see TypeOrError for error code)
     */
    static CBotFunction* FindLocalOrPublic(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                                           CBotVar** ppVars, CBotTypResult &TypeOrError, CBotProgram* baseProg);

    /*!
     * \brief Find all functions that match the name and arguments.
     * \param functionList List of functions to search, can be empty.
     * \param name Name of the function to find.
     * \param ppVars Arguments to compare with parameters.
     * \param TypeOrError Contains a CBotError when no useable function has been found.
     * \param funcMap Container for suitable functions and their signature values.
     * \param pClass Pointer to class when searching for methods.
     */
    static void SearchList(const std::list<CBotFunction*>& functionList,
                           const std::string& name, CBotVar** ppVars, CBotTypResult& TypeOrError,
                           std::map<CBotFunction*, int>& funcMap, CBotClass* pClass = nullptr);

    /*!
     * \brief Find the function with the lowest signature value. If there is more
     * than one of the same signature value, TypeOrError is set to CBotErrAmbiguousCall.
     * \param funcMap List of functions and their signature values, can be empty.
     * \param[out] nIdent Unique identifier of the function.
     * \param TypeOrError Type returned by the function or error code.
     * \return Pointer to the function with the lowest signature or nullptr.
     */
    static CBotFunction* BestFunction(std::map<CBotFunction*, int>& funcMap,
                                      long& nIdent, CBotTypResult& TypeOrError);

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
     * \brief Find a method matching the name and arguments.
     * \param name Name of the method to find.
     * \param ppVars Arguments to compare with parameters.
     * \param[out] nIdent Unique identifier of the method.
     * \param pStack Current compilation stack frame.
     * \param pClass Pointer to the class.
     * \return The return type for the method or a CBotError.
     */
    static CBotTypResult CompileMethodCall(const std::string& name, CBotVar** ppVars,
                                           long& nIdent, CBotCStack* pStack, CBotClass* pClass);

    /*!
     * \brief Find a method by its unique identifier or by name and parameters.
     * \param[in,out] nIdent Unique identifier of the method.
     * \param name Name of the method to find.
     * \param ppVars Arguments to compare with parameters.
     * \param TypeOrError The return type for the method or a CBotError.
     * \param pClass Pointer to the class.
     * \param program The current program, to search for out-of-class methods.
     * \return Pointer to the method that best matches the given arguments or nullptr.
     */
    static CBotFunction* FindMethod(long& nIdent, const std::string& name,
                                    CBotVar** ppVars, CBotTypResult& TypeOrError,
                                    CBotClass* pClass, CBotProgram* program);

    /*!
     * \brief DoCall Makes call of a method
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppVars
     * \param pStack
     * \param pToken
     * \param pClass
     * \return
     */
    static int DoCall(long &nIdent, const std::string &name, CBotVar* pThis,
                      CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass);

    /*!
     * \brief RestoreCall
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppVars
     * \param pStack
     * \param pClass
     * \return Returns true if the method call was restored.
     */
    static bool RestoreCall(long &nIdent, const std::string &name, CBotVar* pThis,
                            CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass);

    /*!
     * \brief CheckParam See if the "signature" of parameters is identical.
     * \param pParam
     * \return
     */
    bool CheckParam(CBotDefParam* pParam);

    /*!
     * \brief GetName
     * \return
     */
    const std::string& GetName();

    /*!
     * \brief GetParams
     * \return
     */
    std::string GetParams();

    /*!
     * \brief Get the name of the class for a method.
     * \return The name of a class or empty string if it's not a method.
     */
    const std::string& GetClassName() const;

    /*!
     * \brief IsPublic
     * \return
     */
    bool IsPublic();

    /*!
     * \brief Check if a method is protected.
     * \return true if a method was compiled with "protected" keyword.
     */
    bool IsProtected() const;

    /*!
     * \brief Check if a method is private.
     * \return true if a method was compiled with "private" keyword.
     */
    bool IsPrivate() const;

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
    bool GetPosition(int& start, int& stop,
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
    /*!
     * \brief Find all public functions that match the name and arguments.
     * \param name Name of the function to find.
     * \param ppVars Arguments to compare with parameters.
     * \param TypeOrError Contains a CBotError when no useable function has been found.
     * \param funcMap Container for suitable functions and their signature values.
     * \param pClass Pointer to class when searching for methods.
     */
    static void SearchPublic(const std::string& name, CBotVar** ppVars, CBotTypResult& TypeOrError,
                             std::map<CBotFunction*, int>& funcMap, CBotClass* pClass, CBotProgram* program);

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
    //! Protected method.
    bool m_bProtect = false;
    //! Private method.
    bool m_bPrivate = false;
    //! Extern function.
    bool m_bExtern;
    //! Name of the class we are part of
    std::string m_MasterClass;
    //! Token of the class we are part of
    CBotToken m_classToken;
    CBotProgram& m_prog;
    //! For the position of the word "extern".
    CBotToken m_extern;
    CBotToken m_openpar;
    CBotToken m_closepar;
    CBotToken m_openblk;
    CBotToken m_closeblk;

    friend class CBotProgram;
    friend class CBotClass;
    friend class CBotCStack;

};

} // namespace CBot
