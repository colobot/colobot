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
 * \brief Instruction accessing an array element - array[x]
 */
class CBotIndexExpr : public CBotInstr
{
public:
    CBotIndexExpr();
    ~CBotIndexExpr();

    /*!
     * \brief ExecuteVar Finds a field from the instance at compile time.
     * \param pVar
     * \param pile
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;

    /*!
     * \brief ExecuteVar Warning, changes the pointer to the stack intentionally
     * place the index calculated on the additional stack.
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

protected:
    virtual const std::string GetDebugName() override { return "CBotIndexExpr"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! Expression for calculating the index.
    CBotInstr* m_expr;
    friend class CBotLeftExpr;
    friend class CBotExprVar;
    friend class CBotExprRetVar;
};

} // namespace CBot
