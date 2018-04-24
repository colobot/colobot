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

namespace CBot
{

class CBotInstr;
class CBotToken;
class CBotCStack;
class CBotVar;
class CBotTypResult;

/*!
 * \brief CompileParams Compile a list of parameters.
 * \param p
 * \param pStack
 * \param ppVars
 * \return
 */
CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars);

/*!
 * \brief TypeCompatible Check if two results are consistent to make an
 * operation. TypeCompatible is used in two ways:
 * For non-assignment operations:  see CBotTwoOpExpr::Compile
 * TypeCompatible( leftType, rightType, opType )

 * For assignment or compound assignment operations (it's reversed):
 * see CBotReturn::Compile & CBotExpression::Compile
 * TypeCompatible( valueType, varType, opType )
 * \param type1
 * \param type2
 * \param op
 * \return
 */
bool TypeCompatible(CBotTypResult& type1, CBotTypResult& type2, int op = 0);

/*!
 * \brief TypesCompatibles Check if two variables are compatible for parameter
 * passing.
 * \param type1
 * \param type2
 * \return
 */
bool TypesCompatibles(const CBotTypResult& type1, const CBotTypResult& type2);

} // namespace CBot
