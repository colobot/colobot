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

//////////////////////////////////////////////////////////////////////////////////////////

// compile a list of parameters

CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars)
{
    bool        first = true;
    CBotInstr*    ret = nullptr;   // to return to the list

    CBotCStack*    pile = pStack;
    int            i = 0;

    if (IsOfType(p, ID_OPENPAR))
    {
        int    start, end;
        if (!IsOfType(p, ID_CLOSEPAR)) while (true)
        {
            start = p->GetStart();
            pile = pile->TokenStack();  // keeps the result on the stack

            if (first) pStack->SetStartError(start);
            first = false;

            CBotInstr*    param = CBotExpression::Compile(p, pile);
            end      = p->GetStart();

            if (!pile->IsOk())
            {
                return pStack->Return(nullptr, pile);
            }

            if (ret == nullptr) ret = param;
            else ret->AddNext(param);   // construct the list

            if (param != nullptr)
            {
                if (pile->GetTypResult().Eq(99))
                {
                    delete pStack->TokenStack();
                    pStack->SetError(TX_VOID, p->GetStart());
                    return nullptr;
                }
                ppVars[i] = pile->GetVar();
                ppVars[i]->GetToken()->SetPos(start, end);
                i++;

                if (IsOfType(p, ID_COMMA)) continue;    // skips the comma
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(TX_CLOSEPAR, p->GetStart());
            delete pStack->TokenStack();
            return nullptr;
        }
    }
    ppVars[i] = nullptr;
    return    ret;
}

/////////////////////////////////////////////////////////////
// check if two results are consistent to make an operation

bool TypeCompatible(CBotTypResult& type1, CBotTypResult& type2, int op)
{
    int    t1 = type1.GetType();
    int    t2 = type2.GetType();

    int max = (t1 > t2) ? t1 : t2;

    if (max == 99) return false;    // result is void?

    // special case for strin concatenation
    if (op == ID_ADD && max >= CBotTypString) return true;
    if (op == ID_ASSADD && max >= CBotTypString) return true;
    if (op == ID_ASS && t1 == CBotTypString) return true;

    if (max >= CBotTypBoolean)
    {
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypPointer && t2 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypPointer && t1 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypArrayPointer && t2 == CBotTypNullPointer)) return true;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypArrayPointer && t1 == CBotTypNullPointer)) return true;
        if (t2 != t1) return false;
        if (t1 == CBotTypArrayPointer) return type1.Compare(type2);
        if (t1 == CBotTypPointer ||
            t1 == CBotTypClass   ||
            t1 == CBotTypIntrinsic )
        {
            CBotClass*    c1 = type1.GetClass();
            CBotClass*    c2 = type2.GetClass();

            return c1->IsChildOf(c2) || c2->IsChildOf(c1);
            // accept the case in reverse
            // the transaction will be denied at runtime if the pointer is not
            // compatible
        }

        return true;
    }

    type1.SetType(max);
    type2.SetType(max);
    return true;
}

// check if two variables are compatible for parameter passing

bool TypesCompatibles(const CBotTypResult& type1, const CBotTypResult& type2)
{
    int    t1 = type1.GetType();
    int    t2 = type2.GetType();

    if (t1 == CBotTypIntrinsic) t1 = CBotTypClass;
    if (t2 == CBotTypIntrinsic) t2 = CBotTypClass;

    int max = (t1 > t2) ? t1 : t2;

    if (max == 99) return false;                    // result is void?

    if (max >= CBotTypBoolean)
    {
        if (t2 != t1) return false;

        if (max == CBotTypArrayPointer)
            return TypesCompatibles(type1.GetTypElem(), type2.GetTypElem());

        if (max == CBotTypClass || max == CBotTypPointer)
            return type1.GetClass() == type2.GetClass() ;

        return true ;
    }
    return true;
}


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

