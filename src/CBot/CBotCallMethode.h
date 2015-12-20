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

// Local include

// Global include
class CBotVar;
class CBotCStack;
class CBotStack;
class CBotToken;

/*!
 * \brief The CBotCallMethode class Class managing the methods declared by
 * AddFunction on a class.
 */
class CBotCallMethode
{
public:

    /*!
     * \brief CBotCallMethode
     * \param name
     * \param rExec
     * \param rCompile
     */
    CBotCallMethode(const std::string& name,
                    bool rExec(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                    CBotTypResult rCompile(CBotVar* pThis, CBotVar*& pVar));

    /*!
     * \brief ~CBotCallMethode
     */
    ~CBotCallMethode();

    /*!
     * \brief CompileCall Is acceptable by a call procedure name and given
     * parameters.
     * \param name
     * \param pThis
     * \param ppVars
     * \param pStack
     * \param nIdent
     * \return
     */
    CBotTypResult CompileCall(const std::string& name,
                              CBotVar* pThis,
                              CBotVar** ppVars,
                              CBotCStack* pStack,
                              long& nIdent);

    /*!
     * \brief DoCall
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppVars
     * \param pResult
     * \param pStack
     * \param pFunc
     * \return
     */
    int DoCall(long& nIdent,
               const std::string& name,
               CBotVar* pThis,
               CBotVar** ppVars,
               CBotVar*& pResult,
               CBotStack* pStack,
               CBotToken* pFunc);

    /*!
     * \brief GetName
     * \return
     */
    std::string GetName();

    /*!
     * \brief Next
     * \return
     */
    CBotCallMethode* Next();

    /*!
     * \brief AddNext
     * \param p
     */
    void AddNext(CBotCallMethode* p);

private:
    std::string m_name;
    bool (*m_rExec) (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    CBotTypResult (*m_rComp) (CBotVar* pThis, CBotVar* &pVar);
    CBotCallMethode* m_next;
    friend class CBotClass;
    long m_nFuncIdent;

};

