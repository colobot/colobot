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
#include "CBotString.h"

// Local include

// Global include

// Forward declaration
class CBotStack;

#define    STACKRUN    1  //! \def return execution directly on a suspended routine

/*!
 * \brief The CBotCall class. Class for routine calls (external).
 */
class CBotCall
{
public:

    /*!
     * \brief CBotCall
     * \param name
     * \param rExec
     * \param rCompile
     */
    CBotCall(const char* name,
             bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
             CBotTypResult rCompile (CBotVar* &pVar, void* pUser));

    /*!
     * \brief ~CBotCall
     */
    ~CBotCall();

    /*!
     * \brief AddFunction
     * \param name
     * \param rExec
     * \param rCompile
     * \return
     */
    static bool AddFunction(const char* name,
                            bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                            CBotTypResult rCompile (CBotVar* &pVar, void* pUser));

    /*!
     * \brief CompileCall Is acceptable by a call procedure name and given
     * parameters.
     * \param p
     * \param ppVars
     * \param pStack
     * \param nIdent
     * \return
     */
    static CBotTypResult CompileCall(CBotToken* &p, CBotVar** ppVars, CBotCStack* pStack, long& nIdent);

    /*!
     * \brief CheckCall
     * \param name
     * \return
     */
    static bool CheckCall(const char* name);

    /*!
     * \brief DoCall
     * \param nIdent
     * \param token
     * \param ppVars
     * \param pStack
     * \param rettype
     * \return
     */
    static int DoCall(long& nIdent, CBotToken* token, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);

#if STACKRUN

    /*!
     * \brief Run
     * \param pStack
     * \return
     */
    bool Run(CBotStack* pStack);

    /*!
     * \brief RestoreCall
     * \param nIdent
     * \param token
     * \param ppVar
     * \param pStack
     * \return
     */
    static bool RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack);
#endif

    /*!
     * \brief GetName
     * \return
     */
    CBotString GetName();

    /*!
     * \brief Next
     * \return
     */
    CBotCall* Next();

    /*!
     * \brief SetPUser
     * \param pUser
     */
    static void SetPUser(void* pUser);

    /*!
     * \brief Free
     */
    static void Free();


private:
    static CBotCall*  m_ListCalls;
    static void* m_pUser;
    long        m_nFuncIdent;

    CBotString m_name;
    bool (*m_rExec) (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser);
    CBotTypResult (*m_rComp) (CBotVar* &pVar, void* pUser);
    CBotCall* m_next;
};
