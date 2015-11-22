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
#include "CBotLeftExpr.h"

#include "CBotInstr.h"

// Local include

// Global include


/*!
 * \brief The CBotExpression class Compiles a statement with or without
 * assignment.
 * eg :
 *     - x = a;
 *     - x * y + 3;
 *     - x = 123
 *     - z * 5 + 4
 */


// compiles a statement such as "  " ou " z * 5 + 4 "
//

class CBotExpression : public CBotInstr
{
public:

    /*!
     * \brief CBotExpression
     */
    CBotExpression();

    /*!
     * \brief ~CBotExpression
     */
    ~CBotExpression();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute Executes an expression with assignment.
     * \param pStack
     * \return
     */
    bool Execute(CBotStack* &pStack) override;

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain) override;

private:
    //! Left operand
    CBotLeftExpr* m_leftop;
    //! Right operand
    CBotInstr* m_rightop;
};
