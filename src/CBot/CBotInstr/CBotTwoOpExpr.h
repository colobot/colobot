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

namespace CBot
{

/**
 * \brief Any expression with two operands
 *
 * Examples:
 * \code
 * op1 + op2
 * op1 > op2
 * \endcode
 *
 * \section Operations Supported operations
 *
 * This table is sorted by operator precedence
 *
 * | Operator                               |
 * | -------------------------------------- |
 * | **                                     |
 * | *, /, %                                |
 * | +, -                                   |
 * | <<, >>, >>>                            |
 * | >, <, >=, <=                           |
 * | ==, !=                                 |
 * | &                                      |
 * | ^                                      |
 * | \|                                     |
 * | &&, and                                |
 * | \|\|, or                               |
 * | a ? b : c (special, see CBotLogicExpr) |
 */
class CBotTwoOpExpr : public CBotInstr
{
public:
    CBotTwoOpExpr();
    ~CBotTwoOpExpr();

    /*!
     * \brief Compiles CBotTwoOpExpr or CBotLogicExpr
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

protected:
    virtual const std::string GetDebugName() override { return "CBotTwoOpExpr"; }
    virtual std::string GetDebugData() override;
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! Left element
    CBotInstr* m_leftop;
    //! Right element
    CBotInstr* m_rightop;
};

} // namespace CBot
