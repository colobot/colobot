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

////////////////////////////////////////////////////////////////////
/**
 * \file CBot.h
 * \brief Interpreter of the language CBot for COLOBOT game
 */

#pragma once

#include "resource.h"
#include "CBotToken.h"                  // token management
#include "CBotProgram.h"

#define    STACKMEM    1                /// \def preserve memory for the execution stack
#define    MAXSTACK    990              /// \def stack size reserved

#define    EOX         (reinterpret_cast<CBotStack*>(-1))   /// \def tag special condition


/////////////////////////////////////////////////////////////////////
// forward declaration

class CBotParExpr;  // basic type or instruction in parenthesis
                    // Toto.truc
                    // 12.5
                    // "string"
                    // ( expression )
class CBotExprVar;  // a variable name as
                    // Toto
                    // chose.truc.machin
class CBotWhile;    // while (...) {...};
class CBotIf;       // if (...) {...} else {...}
class CBotDefParam; // paramerer list of a function

#if 0
extern void DEBUG( const char* text, int val, CBotStack* pile );
#endif
