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
 * \brief An expression
 *
 * There is never an instance of this class
 *
 * Compiles either:
 * * an arithmetic expression in parentheses -- CBotExpression -- (...)
 * * unary operation -- CBotExprUnaire -- -a, +a, ~a, !a, not a
 * * a variable name -- CBotExprVar
 * * pre- or post- incremented or decremented variable -- CBotPreIncExpr, CBotPostIncExpr -- a++, ++a, a--, --a
 * * a function call -- CBotInstrCall
 * * a class method call -- CBotInstrMethode
 * * number literal (or numerical constant from CBotToken::DefineNum()) -- CBotExprLitNum
 * * string literal -- CBotExprLitString
 * * boolean literal -- CBotExprLitBool -- true/false
 * * null -- CBotExprLitNull
 * * nan -- CBotExprLitNan
 * * class instance creation with "new" -- CBotNew
 */
class CBotParExpr : public CBotInstr
{
public:
    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Compile a literal expression ("string", number, true, false, null, nan, new)
     * \param p[in, out] Pointer to first token of the expression, will be updated to point to first token after the expression
     * \param pStack Current compilation stack frame
     * \return The compiled instruction or nullptr on error
     */
    static CBotInstr* CompileLitExpr(CBotToken* &p, CBotCStack* pStack);

private:
    CBotParExpr() = delete;
    CBotParExpr(const CBotParExpr&) = delete;
};

} // namespace CBot
