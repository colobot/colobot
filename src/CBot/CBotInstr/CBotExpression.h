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

#include "CBot/CBotInstr/CBotLeftExpr.h"

#include "CBot/CBotInstr/CBotInstr.h"

namespace CBot
{

/**
 * \brief An arithmetic expression, with or without assignment
 *
 * Examples:
 * \code
 * x = a
 * x * y + 3
 * x = 123
 * z * 5 + 4
 * \endcode
 */

class CBotExpression : public CBotInstr
{
public:
    CBotExpression();
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

protected:
    virtual const std::string GetDebugName() override { return "CBotExpression"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! Left operand
    CBotLeftExpr* m_leftop;
    //! Right operand
    CBotInstr* m_rightop;
};

} // namespace CBot
