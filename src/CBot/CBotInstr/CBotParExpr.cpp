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

// Modules inlcude
#include "CBotParExpr.h"
#include "CBotExprUnaire.h"
#include "CBotExprVar.h"
#include "CBotInstrCall.h"
#include "CBotPostIncExpr.h"
#include "CBotPreIncExpr.h"
#include "CBotExprNum.h"
#include "CBotExprAlpha.h"
#include "CBotExprBool.h"
#include "CBotNew.h"
#include "CBotExprNull.h"
#include "CBotExprNan.h"
#include "CBotExpression.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotParExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it an expression in parentheses?
    if (IsOfType(p, ID_OPENPAR))
    {
        CBotInstr* inst = CBotExpression::Compile(p, pStk);

        if (nullptr != inst)
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                return pStack->Return(inst, pStk);
            }
            pStk->SetError(TX_CLOSEPAR, p->GetStart());
        }
        delete inst;
        return pStack->Return(nullptr, pStk);
    }

    // is this a unary operation?
    CBotInstr* inst = CBotExprUnaire::Compile(p, pStk);
    if (inst != nullptr || !pStk->IsOk())
        return pStack->Return(inst, pStk);

    // is it a variable name?
    if (p->GetType() == TokenTypVar)
    {
        // this may be a method call without the "this." before
        inst =  CBotExprVar::CompileMethode(p, pStk);
        if (inst != nullptr) return pStack->Return(inst, pStk);


        // is it a procedure call?
        inst =  CBotInstrCall::Compile(p, pStk);
        if (inst != nullptr || !pStk->IsOk())
            return pStack->Return(inst, pStk);


        CBotToken* pvar = p;
        // no, it an "ordinaty" variable
        inst =  CBotExprVar::Compile(p, pStk);

        CBotToken* pp = p;
        // post incremented or decremented?
        if (IsOfType(p, ID_INC, ID_DEC))
        {
            if (pStk->GetType() >= CBotTypBoolean)
            {
                pStk->SetError(TX_BADTYPE, pp);
                delete inst;
                return pStack->Return(nullptr, pStk);
            }

            // recompile the variable for read-only
            delete inst;
            p = pvar;
            inst =  CBotExprVar::Compile(p, pStk, PR_READ);
            p = p->GetNext();

            CBotPostIncExpr* i = new CBotPostIncExpr();
            i->SetToken(pp);
            i->m_Instr = inst;    // associated statement
            return pStack->Return(i, pStk);
        }
        return pStack->Return(inst, pStk);
    }

    // pre increpemted or pre decremented?
    CBotToken* pp = p;
    if (IsOfType(p, ID_INC, ID_DEC))
    {
        CBotPreIncExpr* i = new CBotPreIncExpr();
        i->SetToken(pp);

        if (p->GetType() == TokenTypVar)
        {
            if (nullptr != (i->m_Instr =  CBotExprVar::Compile(p, pStk, PR_READ)))
            {
                if (pStk->GetType() >= CBotTypBoolean)
                {
                    pStk->SetError(TX_BADTYPE, pp);
                    delete inst;
                    return pStack->Return(nullptr, pStk);
                }
                return pStack->Return(i, pStk);
            }
            delete i;
            return pStack->Return(nullptr, pStk);
        }
    }

    // is it a number or DefineNum?
    if (p->GetType() == TokenTypNum ||
        p->GetType() == TokenTypDef )
    {
        CBotInstr* inst = CBotExprNum::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is this a chaine?
    if (p->GetType() == TokenTypString)
    {
        CBotInstr* inst = CBotExprAlpha::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is a "true" or "false"
    if (p->GetType() == ID_TRUE ||
        p->GetType() == ID_FALSE )
    {
        CBotInstr* inst = CBotExprBool::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is an object to be created with new
    if (p->GetType() == ID_NEW)
    {
        CBotInstr* inst = CBotNew::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // is a null pointer
    if (IsOfType(p, ID_NULL))
    {
        CBotInstr* inst = new CBotExprNull ();
        inst->SetToken(pp);
        CBotVar* var = CBotVar::Create("", CBotTypNullPointer);
        pStk->SetVar(var);
        return pStack->Return(inst, pStk);
    }

    // is a number nan
    if (IsOfType(p, ID_NAN))
    {
        CBotInstr* inst = new CBotExprNan ();
        inst->SetToken(pp);
        CBotVar* var = CBotVar::Create("", CBotTypInt);
        var->SetInit(CBotVar::InitType::IS_NAN);
        pStk->SetVar(var);
        return pStack->Return(inst, pStk);
    }


    return pStack->Return(nullptr, pStk);
}
