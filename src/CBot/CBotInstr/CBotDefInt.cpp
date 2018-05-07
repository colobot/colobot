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

#include "CBot/CBotInstr/CBotDefInt.h"

#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotDefArray.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDefInt::CBotDefInt()
{
    m_next    = nullptr;            // for multiple definitions
    m_var     = nullptr;
    m_expr    = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefInt::~CBotDefInt()
{
    delete m_var;
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotDefInt::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? nullptr : p;        // no repetition of the token "int"

    if (!cont && !IsOfType(p, ID_INT)) return nullptr;

    CBotDefInt*    inst = static_cast<CBotDefInt*>(CompileArray(p, pStack, CBotTypInt));
    if (inst != nullptr || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotDefInt();

    inst->m_expr = nullptr;

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    // determines the expression is valid for the item on the left side
    if (nullptr != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypInt;
        if (pStk->CheckVarLocal(vartoken))  // redefinition of the variable
        {
            pStk->SetError(CBotErrRedefVar, vartoken);
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))
        {
            delete inst;    // type is not CBotDefInt
            p = vartoken;   // returns the variable name

            // compiles an array declaration

            CBotInstr* inst2 = CBotDefArray::Compile(p, pStk, CBotTypInt);

            inst = static_cast<CBotDefInt*>(inst2);
            goto suite;     // no assignment, variable already created
        }

        if (IsOfType(p,  ID_ASS))   // with an assignment?
        {
            pStk->SetStartError(p->GetStart());
            if ( IsOfType(p, ID_SEP) )
            {
                pStk->SetError(CBotErrNoExpression, p->GetStart());
                goto error;
            }
            if (nullptr == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
            if (pStk->GetType() >= CBotTypBoolean)  // compatible type ?
            {
                pStk->SetError(CBotErrBadType1, p->GetStart());
                goto error;
            }
        }

        {
            CBotVar*    var = CBotVar::Create(*vartoken, CBotTypInt);// create the variable (evaluated after the assignment)
            var->SetInit(inst->m_expr != nullptr ? CBotVar::InitType::DEF : CBotVar::InitType::UNDEF);     // if initialized with assignment
            var->SetUniqNum( //set it with a unique number
                (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
            pStack->AddVar(var);    // place it on the stack
        }
suite:
        if (pStk->IsOk() && IsOfType(p,  ID_COMMA))     // chained several definitions
        {
            if (nullptr != ( inst->m_next2b = CBotDefInt::Compile(p, pStk, true, noskip)))    // compile next one
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))                    // instruction is completed
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(CBotErrNoTerminator, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDefInt::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);    // essential for SetState()

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;    // initial value // interrupted?
        m_var->Execute(pile);                                // creates and assign the result

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;                // other(s) definition(s)

    return pj->Return(pile);                                // forward below
}

////////////////////////////////////////////////////////////////////////////////
void CBotDefInt::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;
    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == nullptr) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);    // initial value // interrupted?
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b) m_next2b->RestoreState(pile, bMain);            // other(s) definition(s)
}

std::map<std::string, CBotInstr*> CBotDefInt::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_var"] = m_var;
    links["m_expr"] = m_expr;
    return links;
}

} // namespace CBot
