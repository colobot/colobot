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
 * \brief Compile a comma-separated list of expressions or variable definitions
 *
 * Used by for() in initialization and increment statements
 *
 * Examples:
 * \code
 * int a
 * a = 0
 * a = 0, int b = 3
 * a = 5, b = 7
 * int a = 3, b = 8 // This declares b as new variable, not assigns to it!
 * i++
 * i++, j++
 * int a = 5, j++
 * \endcode
 *
 * \see CBotFor
 */
class CBotListExpression : public CBotInstr
{
public:
    CBotListExpression();
    ~CBotListExpression();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute
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
    virtual const std::string GetDebugName() override { return "CBotListExpression"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! The first expression to be evaluated
    CBotInstr* m_expr;
};

} // namespace CBot
