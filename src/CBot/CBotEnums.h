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

// Local include

// Global include

/*! \brief CBotType Defines known types. This types are modeled on Java types.
 * Do not change the order of elements
 */
enum CBotType
{
    CBotTypVoid         = 0,
    CBotTypByte         = 1,                //n
    CBotTypShort        = 2,                //n
    CBotTypChar         = 3,                //n
    CBotTypInt          = 4,
    CBotTypLong         = 5,                //n
    CBotTypFloat        = 6,
    CBotTypDouble       = 7,                //n
    CBotTypBoolean      = 8,
    CBotTypString       = 9,

    CBotTypArrayPointer = 10,                // array of variables
    CBotTypArrayBody    = 11,                // same but creates an instance

    CBotTypPointer      = 12,                // pointer to an instance
    CBotTypNullPointer  = 13,                // null pointer is special
    CBotTypClass        = 15,
    CBotTypIntrinsic    = 16                // instance of a class intrinsic
};

//! \brief CBotGet Different modes for GetPosition.
enum CBotGet
{
    GetPosExtern = 1,
    GetPosNom    = 2,
    GetPosParam  = 3,
    GetPosBloc   = 4
};


enum EID
{
    ID_IF = 2000,
    ID_ELSE,
    ID_WHILE,
    ID_DO,
    ID_FOR,
    ID_BREAK,
    ID_CONTINUE,
    ID_SWITCH,
    ID_CASE,
    ID_DEFAULT,
    ID_TRY,
    ID_THROW,
    ID_CATCH,
    ID_FINALLY,
    ID_TXT_AND,
    ID_TXT_OR,
    ID_TXT_NOT,
    ID_RETURN,
    ID_CLASS,
    ID_EXTENDS,
    ID_SYNCHO,
    ID_NEW,
    ID_PUBLIC,
    ID_EXTERN,
    ID_STATIC,
    ID_PROTECTED,
    ID_PRIVATE,
    ID_INT,
    ID_FLOAT,
    ID_BOOLEAN,
    ID_STRING,
    ID_VOID,
    ID_BOOL,

    ID_TRUE = 2200,
    ID_FALSE,
    ID_NULL,
    ID_NAN,

    ID_OPENPAR = 2300,
    ID_CLOSEPAR,
    ID_OPBLK,
    ID_CLBLK,
    ID_SEP,
    ID_COMMA,
    ID_DOTS,
    ID_DOT,
    ID_OPBRK,
    ID_CLBRK,
    ID_DBLDOTS,
    ID_LOGIC,
    ID_ADD,
    ID_SUB,
    ID_MUL,
    ID_DIV,
    ID_ASS,
    ID_ASSADD,
    ID_ASSSUB,
    ID_ASSMUL,
    ID_ASSDIV,
    ID_ASSOR,
    ID_ASSAND,
    ID_ASSXOR,
    ID_ASSSL,
    ID_ASSSR,
    ID_ASSASR,
    ID_SL,
    ID_SR,
    ID_ASR,
    ID_INC,
    ID_DEC,
    ID_LO,
    ID_HI,
    ID_LS,
    ID_HS,
    ID_EQ,
    ID_NE,
    ID_AND,
    ID_XOR,
    ID_OR,
    ID_LOG_AND,
    ID_LOG_OR,
    ID_LOG_NOT,
    ID_NOT,
    ID_MODULO,
    ID_POWER,
    ID_ASSMODULO,
    TX_UNDEF = 4000,
    TX_NAN
};
