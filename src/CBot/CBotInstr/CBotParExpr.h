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
#include "CBotInstr.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
// possibly an expression in parentheses ( ... )
// there is never an instance of this class
// being the object returned inside the parenthesis
////////////////////////////////////////////////////////////////////////////////
// compile either:
// instruction in parentheses (...)
// a unary expression (negative, not)
// variable name
// variables pre and post-incremented or decremented
// a given number DefineNum
// a constant
// procedure call
// new statement
//
// this class has no constructor, because there is never an instance of this class
// the object returned by Compile is the class corresponding to the instruction
////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CBotParExpr class
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
};
