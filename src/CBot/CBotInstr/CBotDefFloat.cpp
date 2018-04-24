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

#include "CBot/CBotInstr/CBotDefFloat.h"

#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotDefArray.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDefFloat::CBotDefFloat()
{
    m_var    = nullptr;
    m_expr   = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefFloat::~CBotDefFloat()
{
    delete m_var;
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotDefFloat::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? nullptr : p;

    if (!cont && !IsOfType(p, ID_FLOAT)) return nullptr;

    CBotDefFloat*    inst = static_cast<CBotDefFloat*>(CompileArray(p, pStack, CBotTypFloat));
    if (inst != nullptr || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotDefFloat();

    inst->m_expr = nullptr;

    CBotToken*    vartoken = p;
    CBotVar*    var = nullptr;
    inst->SetToken(vartoken);

    if (nullptr != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypFloat;
        if (pStk->CheckVarLocal(vartoken))                    // redefinition of a variable
        {
            pStk->SetStartError(vartoken->GetStart());
            pStk->SetError(CBotErrRedefVar, vartoken->GetEnd());
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))
        {
            delete inst;
            p = vartoken;
            inst = static_cast<CBotDefFloat*>(CBotDefArray::Compile(p, pStk, CBotTypFloat));

            goto suite;            // no assignment, variable already created
        }

        if (IsOfType(p,  ID_ASS))
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
            if (pStk->GetType() >= CBotTypBoolean)
            {
                pStk->SetError(CBotErrBadType1, p->GetStart());
                goto error;
            }
        }

        var = CBotVar::Create(*vartoken, CBotTypFloat);
        var->SetInit(inst->m_expr != nullptr ? CBotVar::InitType::DEF : CBotVar::InitType::UNDEF);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);
suite:
        if (pStk->IsOk() && IsOfType(p,  ID_COMMA))
        {
            if (nullptr != ( inst->m_next2b = CBotDefFloat::Compile(p, pStk, true, noskip)))
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))
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
bool CBotDefFloat::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if ( pile->GetState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return false;
        m_var->Execute(pile);

        if (!pile->SetState(1)) return false;
    }

    if (pile->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return false;

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotDefFloat::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile = pj;
    if (bMain)
    {
        pile = pj->RestoreStack(this);
        if (pile == nullptr) return;

        if ( pile->GetState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if (m_next2b)
         m_next2b->RestoreState(pile, bMain);
}

std::map<std::string, CBotInstr*> CBotDefFloat::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_var"] = m_var;
    links["m_expr"] = m_expr;
    return links;
}

} // namespace CBot
