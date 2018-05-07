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
 * \brief Compilation of left side of an assignment
 *
 * Some examples:
 * \code
 * varname
 * varname[3]
 * varname.x
 * varname.pos.x
 * varname[2].pos.x
 * varname[1].pos[2].x
 * varname[1][2][3]
 * \endcode
 */
class CBotLeftExpr : public CBotInstr
{
public:
    CBotLeftExpr();
    ~CBotLeftExpr();

    /*!
     * \brief Compile Compiles an expression for a left-operand
     * (left of an assignment).
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

    using CBotInstr::Execute;

    /*!
     * \brief ExecuteVar Fetch a variable during compilation.
     * \param pVar
     * \param pile
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;

    using CBotInstr::ExecuteVar;

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

protected:
    virtual const std::string GetDebugName() override { return "CBotLeftExpr"; }
    virtual std::string GetDebugData() override;

private:
    long m_nIdent;
};

} // namespace CBot
