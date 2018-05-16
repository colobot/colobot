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

/**
 * \file CBotEnums.h
 * \brief Some enum values used across the CBot engine
 */

namespace CBot
{

/**
 * \brief Defines known types. This types are modeled on Java types.
 *
 * Do not change the order of elements.
 */
enum CBotType
{
    CBotTypVoid         = 0,  //!< void
    CBotTypByte         = 1,  //!< byte (NOT IMPLEMENTED)
    CBotTypShort        = 2,  //!< short (NOT IMPLEMENTED)
    CBotTypChar         = 3,  //!< char (NOT IMPLEMENTED)
    CBotTypInt          = 4,  //!< int
    CBotTypLong         = 5,  //!< long (NOT IMPLEMENTED)
    CBotTypFloat        = 6,  //!< float
    CBotTypDouble       = 7,  //!< double (NOT IMPLEMENTED)
    CBotTypBoolean      = 8,  //!< bool
    CBotTypString       = 9,  //!< string

    CBotTypArrayPointer = 10, //!< Pointer to an array (::CBotTypArrayBody)
    CBotTypArrayBody    = 11, //!< Array

    CBotTypPointer      = 12, //!< Pointer to a class (::CBotTypClass or ::CBotTypIntrinsic)
    CBotTypNullPointer  = 13, //!< Null pointer
    CBotTypClass        = 15, //!< Class instance
    CBotTypIntrinsic    = 16, //!< Intrinsic class instance

    CBotTypMAX = 20
};

/**
 * \brief Different modes for CBotProgram::GetPosition
 */
enum CBotGet
{
    GetPosExtern = 1,
    GetPosNom    = 2,
    GetPosParam  = 3,
    GetPosBloc   = 4
};

/**
 * \brief This enum contains possible token types
 */
enum TokenId
{
    TokenKeyWord = 2000, //!< keywords
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

    TokenKeyVal = 2200, //!< keywords that represent values (true, false, null, nan)
    ID_TRUE = 2200,
    ID_FALSE,
    ID_NULL,
    ID_NAN,

    TokenKeyOp = 2300, //!< operators
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

/**
 * \brief Types of tokens
 */
enum TokenType
{
    TokenTypNone = 0,
    TokenTypKeyWord = 1, //!< keyword of the language (see TokenKeyWord)
    TokenTypNum = 2,     //!< number
    TokenTypString = 3,  //!< string
    TokenTypVar = 4,     //!< a variable name
    TokenTypDef = 5      //!< value according DefineNum
};

/**
 * \brief This enum contains possible CBot error values. Values in range 5000-5999 are compile errors, 6000-6999 are runtime errors
 *
 * Note that other values ​​may be returned, for example exceptions for user-defined builtin functions, or "throw" instruction
 *
 * Also note that these can't overlap with CBotType, see CBotTypResult for explanation
 */
enum CBotError : int
{
    CBotNoErr = 0,

    // Compile errors
    CBotErrOpenPar       = 5000, //!< missing the opening parenthesis
    CBotErrClosePar      = 5001, //!< missing the closing parenthesis
    CBotErrNotBoolean    = 5002, //!< expression must be a boolean
    CBotErrUndefVar      = 5003, //!< undeclared variable
    CBotErrBadLeft       = 5004, //!< assignment impossible ( 5 = ... )
    CBotErrNoTerminator  = 5005, //!< semicolon expected
    CBotErrCaseOut       = 5006, //!< case outside a switch
    CBotErrNoEnd         = 5007, //!< instructions after final closing brace
    CBotErrCloseBlock    = 5008, //!< missing " } "
    CBotErrElseWhitoutIf = 5009, //!< else without matching if
    CBotErrOpenBlock     = 5010, //!< missing " { "
    CBotErrBadType1      = 5011, //!< wrong type for the assignment
    CBotErrRedefVar      = 5012, //!< redefinition of the variable
    CBotErrBadType2      = 5013, //!< Two operands are incompatible
    CBotErrUndefCall     = 5014, //!< routine undefined
    CBotErrNoDoubleDots  = 5015, //!< " : " expected
    CBotErrNoWhile       = 5016, //!< "while" expected (in do..while)
    CBotErrBreakOutside  = 5017, //!< break outside of a loop
    CBotErrUndefLabel    = 5019, //!< label udnefined
    CBotErrLabel         = 5018, //!< label ne peut se mettre ici (label can not get here)
    CBotErrNoCase        = 5020, //!< missing " case "
    CBotErrBadNum        = 5021, //!< expected number
    CBotErrVoid          = 5022, //!< " void " not possible here
    CBotErrNoType        = 5023, //!< type declaration expected
    CBotErrNoVar         = 5024, //!< variable name expected
    CBotErrNoFunc        = 5025, //!< expected function name
    CBotErrOverParam     = 5026, //!< too many parameters
    CBotErrRedefFunc     = 5027, //!< this function already exists
    CBotErrLowParam      = 5028, //!< not enough parameters
    CBotErrBadParam      = 5029, //!< wrong types of parameters
    CBotErrNbParam       = 5030, //!< wrong number of parameters
    CBotErrUndefItem     = 5031, //!< element does not exist in the class
    CBotErrUndefClass    = 5032, //!< variable is not a class
    CBotErrNoConstruct   = 5033, //!< no appropriate constructor
    CBotErrRedefClass    = 5034, //!< class already exists
    CBotErrCloseIndex    = 5035, //!< " ] " expected
    CBotErrReserved      = 5036, //!< reserved word (for a DefineNum)
    CBotErrBadNew        = 5037, //!< wrong setting for new
    CBotErrOpenIndex     = 5038, //!< " [ " expected
    CBotErrBadString     = 5039, //!< expected string
    CBotErrBadIndex      = 5040, //!< wrong index type "[ false ]"
    CBotErrPrivate       = 5041, //!< protected item
    CBotErrNoPublic      = 5042, //!< missing word "public"
    CBotErrNoExpression  = 5043, //!< expression expected after =
    CBotErrAmbiguousCall = 5044, //!< ambiguous call to overloaded function
    CBotErrFuncNotVoid   = 5045, //!< function needs return type "void"
    CBotErrNoClassName   = 5046, //!< class name expected
    CBotErrNoReturn      = 5047, //!< non-void function needs "return;"
    CBotErrDefaultValue  = 5048, //!< this parameter needs a default value
    CBotErrEndQuote      = 5049, //!< missing end quote
    CBotErrBadEscape     = 5050, //!< unknown escape sequence
    CBotErrOctalRange    = 5051, //!< octal value out of range
    CBotErrHexDigits     = 5052, //!< missing hex digits after escape sequence
    CBotErrHexRange      = 5053, //!< hex value out of range
    CBotErrUnicodeName   = 5054, //!< invalid universal character name

    // Runtime errors
    CBotErrZeroDiv       = 6000, //!< division by zero
    CBotErrNotInit       = 6001, //!< uninitialized variable
    CBotErrBadThrow      = 6002, //!< throw a negative value
    CBotErrNoRetVal      = 6003, //!< function did not return results
    CBotErrNoRun         = 6004, //!< Run() without active function
    CBotErrUndefFunc     = 6005, //!< calling a function that no longer exists
    CBotErrNotClass      = 6006, //!< this class does not exist
    CBotErrNull          = 6007, //!< null pointer
    CBotErrNan           = 6008, //!< calculation with a NAN
    CBotErrOutArray      = 6009, //!< index out of array
    CBotErrStackOver     = 6010, //!< stack overflow
    CBotErrDeletedPtr    = 6011, //!< pointer to an object destroyed
    CBotErrFileOpen      = 6012, //!< cannot open the file
    CBotErrNotOpen       = 6013, //!< channel not open
    CBotErrRead          = 6014, //!< error while reading
    CBotErrWrite         = 6015, //!< writing error

    CBotErrMAX, //!< Max errors
};

} // namespace CBot
