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

///////////////////////////////////////////////////////////////////////

// compilation of various instructions
// compile all routines as static
// and return an object according to what was found as instruction

// compiler principle:
// compile the routines return an object of the class corresponding to the operation found
// this is always a subclass of CBotInstr.
// (CBotInstr objects are never used directly)


// compiles if the routine returns nullptr is that the statement is false
// or misunderstood.
// the error is then on the stack CBotCStack :: Isok () is false


// Modules inlcude
#include "CBot.h"
#include "CBotInstr/CBotDo.h"
#include "CBotInstr/CBotFor.h"
#include "CBotInstr/CBotSwitch.h"
#include "CBotInstr/CBotBreak.h"
#include "CBotInstr/CBotTry.h"
#include "CBotInstr/CBotThrow.h"
#include "CBotInstr/CBotWhile.h"
#include "CBotInstr/CBotExprAlpha.h"
#include "CBotInstr/CBotExprNum.h"
#include "CBotInstr/CBotNew.h"
#include "CBotInstr/CBotExprNan.h"
#include "CBotInstr/CBotExprNull.h"
#include "CBotInstr/CBotExprBool.h"
#include "CBotInstr/CBotLeftExprVar.h"
#include "CBotInstr/CBotPreIncExpr.h"
#include "CBotInstr/CBotPostIncExpr.h"
#include "CBotInstr/CBotExprVar.h"
#include "CBotInstr/CBotInstrCall.h"
#include "CBotInstr/CBotListInstr.h"
#include "CBotInstr/CBotExprUnaire.h"
#include "CBotInstr/CBotBoolExpr.h"
#include "CBotInstr/CBotTwoOpExpr.h"
#include "CBotInstr/CBotExpression.h"
#include "CBotInstr/CBotIndexExpr.h"
#include "CBotInstr/CBotFieldExpr.h"
#include "CBotInstr/CBotClassInst.h"
#include "CBotInstr/CBotIString.h"
#include "CBotInstr/CBotFloat.h"
#include "CBotInstr/CBotBoolean.h"
#include "CBotInstr/CBotEmpty.h"
#include "CBotInstr/CBotReturn.h"
#include "CBotInstr/CBotIf.h"
#include "CBotInstr/CBotListArray.h"
#include "CBotInstr/CBotInstArray.h"
#include "CBotInstr/CBotInt.h"

#include "CBotStack.h"
#include "CBotCStack.h"
#include "CBotClass.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include
#include <cassert>

/////////////////////////////////////////////////////////////////////////////////////
// file management

// necessary because it is not possible to do the fopen in the main program
// fwrite and fread in a dll or using the FILE * returned.


FILE* fOpen(const char* name, const char* mode)
{
    return fopen(name, mode);
}

int fClose(FILE* filehandle)
{
    return fclose(filehandle);
}

size_t fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle)
{
    return fwrite(buffer, elemsize, length, filehandle);
}

size_t fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle)
{
    return fread(buffer, elemsize, length, filehandle);
}

size_t fWrite(const void *buffer, size_t length, FILE* filehandle)
{
    return fwrite(buffer, 1, length, filehandle);
}

size_t fRead(void *buffer, size_t length, FILE* filehandle)
{
    return fread(buffer, 1, length, filehandle);
}


////////////////////////////////////////

