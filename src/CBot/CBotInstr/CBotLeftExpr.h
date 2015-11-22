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
#include "CBotInstr.h"

// Local include

// Global include


/*!
 * \brief The CBotLeftExpr class Accept the expressions that be to the left of
 * assignment.
 */
class CBotLeftExpr : public CBotInstr
{
public:

    /*!
     * \brief CBotLeftExpr
     */
    CBotLeftExpr();

    /*!
     * \brief ~CBotLeftExpr
     */
    ~CBotLeftExpr();

    /*!
     * \brief Compile Compiles an expression for a left-operand
     * (left of an assignment).
     * eg :
     *     - toto
     *     - toto[ 3 ]
     *     - toto.x
     *     - toto.pos.x
     *     - toto[2].pos.x
     *     - toto[1].pos[2].x
     *     - toto[1][2][3]
     *
     * \param p
     * \param pStack
     * \return
     */
    static CBotLeftExpr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute Runs, is a variable and assigns the result to the stack.
     * \param pStack
     * \param array
     * \return
     */
    bool Execute(CBotStack* &pStack, CBotStack* array);

    /*!
     * \brief ExecuteVar Fetch a variable during compilation.
     * \param pVar
     * \param pile
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;

    /*!
     * \brief ExecuteVar Fetch the variable at runtume.
     * \param pVar
     * \param pile
     * \param prevToken
     * \param bStep
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep);

    /*!
     * \brief RestoreStateVar
     * \param pile
     * \param bMain
     */
    void RestoreStateVar(CBotStack* &pile, bool bMain) override;

private:
    long m_nIdent;
};
