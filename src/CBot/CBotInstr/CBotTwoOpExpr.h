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
#include "CBot/CBotInstr/CBotInstr.h"

// Local include

// Global include


/*!
 * \brief The CBotTwoOpExpr class All operations with two operands.
 * eg :
 *      - Opérande1 + Opérande2
 *      - Opérande1 > Opérande2
 */
class CBotTwoOpExpr : public CBotInstr
{
public:

    /*!
     * \brief CBotTwoOpExpr
     */
    CBotTwoOpExpr();

    /*!
     * \brief ~CBotTwoOpExpr
     */
    ~CBotTwoOpExpr();

    /*!
     * \brief Compile Compiles a instruction of type A op B.
     * \param p
     * \param pStack
     * \param pOperations
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations = nullptr);

    /*!
     * \brief Execute Performes the operation on two operands.
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
    //! Left element
    CBotInstr* m_leftop;
    //! Right element
    CBotInstr* m_rightop;
};
