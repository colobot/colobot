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
 * \brief Unary expression - +a, -a, !a, ~a, not a
 */
class CBotExprUnaire : public CBotInstr
{
public:
    CBotExprUnaire();
    ~CBotExprUnaire();

    /*!
     * \brief Compile an expression with a unary operator
     * \param p[in, out] Pointer to first token of the expression, will be updated to point to first token after the expression
     * \param pStack Current compilation stack frame
     * \param bLiteral If true, compiles only literal expressions Ex: ~11, -4.0, !false, not true
     * \return The compiled instruction or nullptr
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack, bool bLiteral = false);

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
    virtual const std::string GetDebugName() override { return "CBotExprUnaire"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! Expression to be evaluated.
    CBotInstr* m_expr;
};

} // namespace CBot
