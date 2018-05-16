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

#include "CBot/CBotInstr/CBotExprUnaire.h"
#include "CBot/CBotInstr/CBotParExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprUnaire::CBotExprUnaire()
{
    m_expr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotExprUnaire::~CBotExprUnaire()
{
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprUnaire::Compile(CBotToken* &p, CBotCStack* pStack, bool bLiteral)
{
    int op = p->GetType();
    CBotToken*    pp = p;
    if (!IsOfTypeList( p, ID_ADD, ID_SUB, ID_LOG_NOT, ID_TXT_NOT, ID_NOT, 0 )) return nullptr;

    CBotCStack* pStk = pStack->TokenStack(pp);

    CBotExprUnaire* inst = new CBotExprUnaire();
    inst->SetToken(pp);

    if (!bLiteral) inst->m_expr = CBotParExpr::Compile(p, pStk);
    else inst->m_expr = CBotParExpr::CompileLitExpr(p, pStk);

    if (inst->m_expr != nullptr)
    {
        if (op == ID_ADD && pStk->GetType() < CBotTypBoolean)        // only with the number
            return pStack->Return(inst, pStk);
        if (op == ID_SUB && pStk->GetType() < CBotTypBoolean)        // only with the numer
            return pStack->Return(inst, pStk);
        if (op == ID_NOT && pStk->GetType() < CBotTypFloat)        // only with an integer
            return pStack->Return(inst, pStk);
        if (op == ID_LOG_NOT && pStk->GetTypResult().Eq(CBotTypBoolean))// only with boolean
            return pStack->Return(inst, pStk);
        if (op == ID_TXT_NOT && pStk->GetTypResult().Eq(CBotTypBoolean))// only with boolean
            return pStack->Return(inst, pStk);

        pStk->SetError(CBotErrBadType1, &inst->m_token);
    }
    delete inst;
    return pStack->Return(nullptr, pStk);
}

// executes unary expression
////////////////////////////////////////////////////////////////////////////////
bool CBotExprUnaire::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->GetState() == 0)
    {
        if (!m_expr->Execute(pile)) return false;                    // interrupted ?
        pile->IncState();
    }

    CBotStack* pile2 = pile->AddStack();
    if (pile2->IfStep()) return false;

    CBotVar*    var = pile->GetVar();                                // get the result on the stack

    switch (GetTokenType())
    {
    case ID_ADD:
        break;
    case ID_SUB:
        var->Neg();                                                  // change the sign
        break;
    case ID_NOT:
    case ID_LOG_NOT:
    case ID_TXT_NOT:
        var->Not();
        break;
    }
    return pj->Return(pile);                                        // forwards below
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprUnaire::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*    pile = pj->RestoreStack(this);
    if ( pile == nullptr) return;

    if (pile->GetState() == 0)
    {
        m_expr->RestoreState(pile, bMain);                        // interrupted here!
        return;
    }
}

std::map<std::string, CBotInstr*> CBotExprUnaire::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_expr"] = m_expr;
    return links;
}

} // namespace CBot
