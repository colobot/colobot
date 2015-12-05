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
#include "CBot/CBotInstr/CBotIString.h"
#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotIString::CBotIString()
{
    m_var    =
    m_expr    = nullptr;
    name = "CBotIString";
}

////////////////////////////////////////////////////////////////////////////////
CBotIString::~CBotIString()
{
    delete m_var;
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotIString::Compile(CBotToken* &p, CBotCStack* pStack, bool cont, bool noskip)
{
    CBotToken*    pp = cont ? nullptr : p;

    if (!cont && !IsOfType(p, ID_STRING)) return nullptr;

    CBotIString*    inst = static_cast<CBotIString*>(CompileArray(p, pStack, CBotTypString));
    if (inst != nullptr || !pStack->IsOk()) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotIString();

    inst->m_expr = nullptr;

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    if (nullptr != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = CBotTypString;
        if (pStk->CheckVarLocal(vartoken))
        {
            pStk->SetStartError(vartoken->GetStart());
            pStk->SetError(TX_REDEFVAR, vartoken->GetEnd());
            goto error;
        }

        if (IsOfType(p,  ID_ASS))
        {
            if (nullptr == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
/*            if (!pStk->GetTypResult().Eq(CBotTypString))            // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }*/
        }

        CBotVar*    var = CBotVar::Create(vartoken, CBotTypString);
        var->SetInit(inst->m_expr != nullptr ? CBotVar::InitType::DEF : CBotVar::InitType::UNDEF);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);

        if (IsOfType(p,  ID_COMMA))
        {
            if (nullptr != ( inst->m_next2b = CBotIString::Compile(p, pStk, true, noskip)))
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotIString::Execute(CBotStack* &pj)
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
void CBotIString::RestoreState(CBotStack* &pj, bool bMain)
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
