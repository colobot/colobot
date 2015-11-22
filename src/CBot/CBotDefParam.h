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
#include "CBotToken.h"
#include "CBotStack.h"

// Local include

// Global include

class CBotCStack;
class CBotStack;
class CBotVar;

/*!
 * \brief The CBotDefParam class A list of parameters.
 */
class CBotDefParam
{
public:

    /*!
     * \brief CBotDefParam
     */
    CBotDefParam();

    /*!
     * \brief ~CBotDefParam
     */
    ~CBotDefParam();

    /*!
     * \brief Compile Compiles a list of parameters.
     * \param p
     * \param pStack
     * \return
     */
    static CBotDefParam* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute
     * \param ppVars
     * \param pj
     * \return
     */
    bool Execute(CBotVar** ppVars, CBotStack* &pj);

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain);

    /*!
     * \brief AddNext
     * \param p
     */
    void AddNext(CBotDefParam* p);

    /*!
     * \brief GetType
     * \return
     */
    int GetType();

    /*!
     * \brief GetTypResult
     * \return
     */
    CBotTypResult GetTypResult();

    /*!
     * \brief GetNext
     * \return
     */
    CBotDefParam* GetNext();

    /*!
     * \brief GetParamString
     * \return
     */
    CBotString GetParamString();

private:
    //! Name of the parameter.
    CBotToken m_token;
    //! Type name.
    CBotString m_typename;
    //! Type of paramteter.
    CBotTypResult m_type;
    //! Next parameter.
    CBotDefParam* m_next;
    long m_nIdent;
};
