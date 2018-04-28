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

// FIXME:  with clang, converting "define" to "static const int" there
//  cause this error :
//  ==>
//  ~~/src/CBot/CBotInstr/CBotDefArray.cpp:199:9: error: attempt to use a deleted function
//         delete pile1->AddStack();                                   // need more indices
//         ^
// ~~/src/CBot/CBotStack.h:78:5: note: '~CBotStack' has been explicitly marked deleted here
//     ~CBotStack() = delete;

//  try it,removing next slash
//*
#define    STACKMEM    1                /// \def preserve memory for the execution stack
/*/static const int STACKMEM = 1;
//  */

//#define    MAXSTACK    990              /// \def stack size reserved
static const int MAXSTACK = 990;

//#define    MAXARRAYSIZE    9999
static const int MAXARRAYSIZE = 9999;


//! Define the current CBot version
//#define    CBOTVERSION    104
static const int CBOTVERSION = 104;

// for SetUserPtr when deleting an object
// \TODO define own types to distinct between different states of objects
#define OBJECTDELETED (reinterpret_cast<void*>(-1))
// value set before initialization
#define OBJECTCREATED (reinterpret_cast<void*>(-2))
