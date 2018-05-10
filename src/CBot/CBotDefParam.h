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

#include "CBot/CBotToken.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotUtils.h"

namespace CBot
{

class CBotCStack;
class CBotStack;
class CBotVar;

/*!
 * \brief The CBotDefParam class A list of parameters.
 */
class CBotDefParam : public CBotLinkedList<CBotDefParam>
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
     * \brief Check if this parameter has a default value expression.
     * \return true if the parameter was compiled with a default value.
     */
    bool HasDefault();

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain);

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
     * \brief GetParamString
     * \return
     */
    std::string GetParamString();

private:
    //! Name of the parameter.
    CBotToken m_token;
    //! Type name.
    std::string m_typename;
    //! Type of paramteter.
    CBotTypResult m_type;
    long m_nIdent;

    //! Default value expression for the parameter.
    CBotInstr* m_expr;
};

} // namespace CBot
