/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotVar/CBotVar.h"
#include "CBot/CBotProgram.h"

namespace CBot
{

class CBotInstr;
class CBotDefParam;
class CBotToken;
class CBotFunction;
class CBotProgram;

/*!
 * \brief The CBotCStack class Management of the stack of compilation.
 */
class CBotCStack
{
public:

    /*!
     * \brief CBotCStack
     * \param ppapa
     */
    CBotCStack(CBotCStack* ppapa);

    /*!
     * \brief CBotCStack Destructor.
     */
    ~CBotCStack();

    /*!
     * \brief IsOk
     * \return
     */
    bool IsOk();

    /*!
     * \brief GetError
     * \return
     */
    CBotError GetError();

    /*!
     * \brief GetError Gives error number
     * \param start
     * \param end
     * \return
     */
    CBotError GetError(int& start, int& end);

    /*!
     * \brief SetType Set the type of instruction on the stack.
     * \param type
     */
    void SetType(CBotTypResult& type);

    /*!
     * \brief GetTypResult Gives the type of value on the stack. Type of
     * instruction on the stack.
     * \param mode
     * \return
     */
    CBotTypResult GetTypResult(CBotVar::GetTypeMode mode = CBotVar::GetTypeMode::NORMAL);

    /*!
     * \brief GetType Gives the type of value on the stack.
     * \param mode
     * \return
     */
    int GetType(CBotVar::GetTypeMode mode = CBotVar::GetTypeMode::NORMAL);

    /*!
     * \brief GetClass Gives the class of the value on the stack.
     * \return
     */
    CBotClass* GetClass();

    /*!
     * \brief AddVar Adds a local variable.
     * \param p
     */
    void AddVar(CBotVar* p);

    /*!
     * \brief FindVar Finds a variable. Seeks a variable on the stack the token
     * may be a result of TokenTypVar (object of a class) or a pointer in the
     * source.
     * \param p
     * \return
     */
    CBotVar* FindVar(CBotToken* &p);

    /*!
     * \brief FindVar
     * \param Token
     * \return
     */
    CBotVar* FindVar(CBotToken& Token);

    /*!
     * \brief CheckVarLocal Test whether a variable is already defined locally.
     * \param pToken
     * \return
     */
    bool CheckVarLocal(CBotToken* &pToken);

    /*!
     * \brief CopyVar Finds and makes a copy.
     * \param Token
     * \return
     */
    CBotVar* CopyVar(CBotToken& Token);

    /*!
     * \brief TokenStack Used only at compile.
     * \param pToken
     * \param bBlock
     * \return
     */
    CBotCStack* TokenStack(CBotToken* pToken = nullptr, bool bBlock = false);

    /*!
     * \brief Return Transmits the result upper.
     * \param p
     * \param pParent
     * \return
     */
    CBotInstr* Return(CBotInstr* p, CBotCStack* pParent);

    /*!
     * \brief ReturnFunc Transmits the result upper.
     * \param p
     * \param pParent
     * \return
     */
    CBotFunction* ReturnFunc(CBotFunction* p, CBotCStack* pParent);

    /*!
     * \brief SetVar
     * \param var
     */
    void SetVar( CBotVar* var );

    /*!
     * \brief SetCopyVar Puts on the stack a copy of a variable.
     * \param var
     */
    void SetCopyVar( CBotVar* var );

    /*!
     * \brief GetVar
     * \return
     */
    CBotVar* GetVar();

    /*!
     * \brief SetStartError
     * \param pos
     */
    void SetStartError(int pos);

    /*!
     * \brief SetError
     * \param n
     * \param pos
     */
    void SetError(CBotError n, int pos);

    /*!
     * \brief SetError
     * \param n
     * \param p
     */
    void SetError(CBotError n, CBotToken* p);

    /*!
     * \brief ResetError
     * \param n
     * \param start
     * \param end
     */
    void ResetError(CBotError n, int start, int end);

    /*!
     * \brief SetRetType
     * \param type
     */
    void SetRetType(CBotTypResult& type);

    /*!
     * \brief GetRetType
     * \return
     */
    CBotTypResult GetRetType();

    /*!
     * \brief SetProgram
     * \param p
     */
    void SetProgram(CBotProgram* p);

    /*!
     * \brief GetProgram
     * \return
     */
    CBotProgram* GetProgram();

    /*!
     * \brief CompileCall
     * \param p
     * \param ppVars
     * \param nIdent
     * \return
     */
    CBotTypResult CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent);

    /*!
     * \brief CheckCall Test if a procedure name is already defined somewhere.
     * \param pToken
     * \param pParam
     * \return
     */
    bool CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

    /*!
     * \brief NextToken
     * \param p
     * \return
     */
    bool NextToken(CBotToken* &p);

private:
    CBotCStack* m_next;
    CBotCStack* m_prev;

    static CBotError m_error;
    static int m_end;
    int m_start;

    //! Result of the operations.
    CBotVar* m_var;
    //! Is part of a block (variables are local to this block).
    bool m_bBlock;
    CBotVar* m_listVar;
    //! List of compiled functions.
    static CBotProgram* m_prog;
    static CBotTypResult m_retTyp;
};

} // namespace CBot
