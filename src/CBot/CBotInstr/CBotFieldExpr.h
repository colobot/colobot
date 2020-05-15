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
 * \brief Accessing a class field using dot operator - toto.x
 */
class CBotFieldExpr : public CBotInstr
{
public:
    CBotFieldExpr();
    ~CBotFieldExpr();

    /*!
     * \brief SetUniqNum
     * \param num
     */
    void SetUniqNum(int num);

    /*!
     * \brief ExecuteVar Find a field from the instance at compile.
     * \param pVar
     * \param pile
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;

    /*!
     * \brief ExecuteVar
     * \param pVar
     * \param pile
     * \param prevToken
     * \param bStep
     * \param bExtend
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend) override;

    /*!
     * \brief RestoreStateVar
     * \param pj
     * \param bMain
     */
    void RestoreStateVar(CBotStack* &pj, bool bMain) override;

    /*!
     * \brief Check if access to a variable is allowed or not depending on public/private/protected setting
     *
     * If this function returns true, the caller is responsible for failing the compilation with ::CBotErrPrivate error.
     * This function doesn't set the error flag itself.
     *
     * \param pStack Current compilation stack frame
     * \param pPrev Class instance which variable to check is part of, or nullptr when compiler inserts 'this.' before
     * \param pVar Variable to check
     * \param bCheckReadOnly True for operations that would modify the value of the variable
     * \return true if pVar is inaccessible in the current context, false if access should be allowed
     */
    static bool CheckProtectionError(CBotCStack* pStack, CBotVar* pPrev, CBotVar* pVar, bool bCheckReadOnly = false);

protected:
    virtual const std::string GetDebugName() override { return "CBotFieldExpr"; }
    virtual std::string GetDebugData() override;

private:
    friend class CBotExpression;
    int m_nIdent;
};

} // namespace CBot
