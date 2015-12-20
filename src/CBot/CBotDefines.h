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


#define    STACKMEM    1                /// \def preserve memory for the execution stack
#define    MAXSTACK    990              /// \def stack size reserved

#define    EOX         (reinterpret_cast<CBotStack*>(-1))   /// \def tag special condition

#define    MAXARRAYSIZE    9999

// variable type SetPrivate / IsPrivate
#define PR_PUBLIC    0        // public variable
#define PR_READ      1        // read only
#define PR_PROTECT   2        // protected (inheritance)
#define PR_PRIVATE   3        // strictly private

//! Define the current CBot version
#define    CBOTVERSION    104

// for SetUserPtr when deleting an object
// \TODO define own types to distinct between different states of objects
#define OBJECTDELETED (reinterpret_cast<void*>(-1))
// value set before initialization
#define OBJECTCREATED (reinterpret_cast<void*>(-2))


////////////////////////////////////////////////////////////////////////
// Error Handling of compilation and execution
////////////////////////////////////////////////////////////////////////

// TODO: Why are all of those duplicated? This needs to be unified across the source code ~krzys_h

// Here are the list of errors that can be returned by the module
// for compilation

#define CBotErrOpenPar       5000    // missing the opening parenthesis
#define CBotErrClosePar      5001    // missing the closing parenthesis
#define CBotErrNotBoolean    5002    // expression must be a boolean
#define CBotErrUndefVar      5003    // undeclared variable
#define CBotErrBadLeft       5004    // assignment impossible ( 5 = ... )
#define CBotErrNoTerminator  5005    // semicolon expected
#define CBotErrCaseOut       5006    // case outside a switch
#define CBotErrCloseBlock    5008    // missing " } "
#define CBotErrElseWhitoutIf 5009    // else without matching if
#define CBotErrOpenBlock     5010    // missing " { "
#define CBotErrBadType1      5011    // wrong type for the assignment
#define CBotErrRedefVar      5012    // redefinition of the variable
#define CBotErrBadType2      5013    // Two operands are incompatible
#define CBotErrUndefCall     5014    // routine undefined
#define CBotErrNoDoubleDots  5015    // " : " expected
#define CBotErrBreakOutside  5017    // break outside of a loop
#define CBotErrUndefLabel    5019    // label udnefined
#define CBotErrLabel         5018    // label ne peut se mettre ici (label can not get here)
#define CBotErrNoCase        5020    // missing " case "
#define CBotErrBadNum        5021    // expected number
#define CBotErrVoid          5022    // " void " not possible here
#define CBotErrNoType        5023    // type declaration expected
#define CBotErrNoVar         5024    // variable name expected
#define CBotErrNoFunc        5025    // expected function name
#define CBotErrOverParam     5026    // too many parameters
#define CBotErrRedefFunc     5027    // this function already exists
#define CBotErrLowParam      5028    // not enough parameters
#define CBotErrBadParam      5029    // wrong types of parameters
#define CBotErrNbParam       5030    // wrong number of parameters
#define CBotErrUndefItem     5031    // element does not exist in the class
#define CBotErrUndefClass    5032    // variable is not a class
#define CBotErrNoConstruct   5033    // no appropriate constructor
#define CBotErrRedefClass    5034    // class already exists
#define CBotErrCloseIndex    5035    // " ] " expected
#define CBotErrReserved      5036    // reserved word (for a DefineNum)
#define CBotErrBadNew        5037    // wrong setting for new
#define CBotErrOpenIndex     5038    // " [ " expected
#define CBotErrBadString     5039    // expected string
#define CBotErrBadIndex      5040    // wrong index type "[ false ]"
#define CBotErrPrivate       5041    // protected item
#define CBotErrNoPublic      5042    // missing word "public"

// here is the list of errors that can be returned by the module
// for the execution

#define CBotErrZeroDiv       6000    // division by zero
#define CBotErrNotInit       6001    // uninitialized variable
#define CBotErrBadThrow      6002    // throw a negative value
#define CBotErrNoRetVal      6003    // function did not return results
#define CBotErrNoRun         6004    // Run() without active function
#define CBotErrUndefFunc     6005    // calling a function that no longer exists
#define CBotErrNotClass      6006    // this class does not exist
#define CBotErrNull          6007    // null pointer
#define CBotErrNan           6008    // calculation with a NAN
#define CBotErrOutArray      6009    // index out of array
#define CBotErrStackOver     6010    // stack overflow
#define CBotErrDeletedPtr    6011    // pointer to an object destroyed
#define CBotErrFileOpen      6012    // cannot open the file
#define CBotErrNotOpen       6013    // channel not open
#define CBotErrRead          6014    // error while reading
#define CBotErrWrite         6015    // writing error

// other values ​​may be returned
// for example exceptions returned by external routines
// and " throw " with any number.

// TODO: refactor & change to enum!

// Compile errors
#define TX_OPENPAR                      5000
#define TX_CLOSEPAR                     5001
#define TX_NOTBOOL                      5002
#define TX_UNDEFVAR                     5003
#define TX_BADLEFT                      5004
#define TX_ENDOF                        5005
#define TX_OUTCASE                      5006
#define TX_NOTERM                       5007
#define TX_CLOSEBLK                     5008
#define TX_ELSEWITHOUTIF                5009
#define TX_OPENBLK                      5010
#define TX_BADTYPE                      5011
#define TX_REDEFVAR                     5012
#define TX_BAD2TYPE                     5013
#define TX_UNDEFCALL                    5014
#define TX_MISDOTS                      5015
#define TX_WHILE                        5016
#define TX_BREAK                        5017
#define TX_LABEL                        5018
#define TX_NOLABEL                      5019
#define TX_NOCASE                       5020
#define TX_BADNUM                       5021
#define TX_VOID                         5022
#define TX_NOTYP                        5023
#define TX_NOVAR                        5024
#define TX_NOFONC                       5025
#define TX_OVERPARAM                    5026
#define TX_REDEF                        5027
#define TX_LOWPARAM                     5028
#define TX_BADPARAM                     5029
#define TX_NUMPARAM                     5030
#define TX_NOITEM                       5031
#define TX_DOT                          5032
#define TX_NOCONST                      5033
#define TX_REDEFCLASS                   5034
#define TX_CLBRK                        5035
#define TX_RESERVED                     5036
#define TX_BADNEW                       5037
#define TX_OPBRK                        5038
#define TX_BADSTRING                    5039
#define TX_BADINDEX                     5040
#define TX_PRIVATE                      5041
#define TX_NOPUBLIC                     5042

// Runtime errors
#define TX_DIVZERO                      6000
#define TX_NOTINIT                      6001
#define TX_BADTHROW                     6002
#define TX_NORETVAL                     6003
#define TX_NORUN                        6004
#define TX_NOCALL                       6005
#define TX_NOCLASS                      6006
#define TX_NULLPT                       6007
#define TX_OPNAN                        6008
#define TX_OUTARRAY                     6009
#define TX_STACKOVER                    6010
#define TX_DELETEDPT                    6011
#define TX_FILEOPEN                     6012
#define TX_NOTOPEN                      6013
#define TX_ERRREAD                      6014
#define TX_ERRWRITE                     6015

// Max errors (NOTE: See above TODO near file errors)
#define TX_MAX                          6016
