// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

#pragma once


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
    ID_FINAL,
    ID_STATIC,
    ID_PROTECTED,
    ID_PRIVATE,
    ID_DEBUGDD,
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
    TX_NAN,
    ID_SUPER = 6000
};

// TODO: refactor & change to enum!

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

#define TX_MAX                          6016

