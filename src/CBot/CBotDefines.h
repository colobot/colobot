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

#define    STACKMEM    1                /// \def preserve memory for the execution stack
#define    MAXSTACK    990              /// \def stack size reserved

#define    MAXARRAYSIZE    9999

//! Define the current CBot version
#define    CBOTVERSION    104

// for SetUserPtr when deleting an object
// \TODO define own types to distinct between different states of objects
#define OBJECTDELETED (reinterpret_cast<void*>(-1))
// value set before initialization
#define OBJECTCREATED (reinterpret_cast<void*>(-2))
