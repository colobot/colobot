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
#include "CBotExprUnaire.h"
#include "CBotParExpr.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotExprUnaire::CBotExprUnaire()
{
    m_Expr = nullptr;
    name = "CBotExprUnaire";
}

////////////////////////////////////////////////////////////////////////////////
CBotExprUnaire::~CBotExprUnaire()
{
    delete m_Expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprUnaire::Compile(CBotToken* &p, CBotCStack* pStack)
{
    int op = p->GetType();
    CBotToken*    pp = p;
    if (!IsOfTypeList( p, ID_ADD, ID_SUB, ID_LOG_NOT, ID_TXT_NOT, ID_NOT, 0 )) return nullptr;

    CBotCStack* pStk = pStack->TokenStack(pp);

    CBotExprUnaire* inst = new CBotExprUnaire();
    inst->SetToken(pp);

    if (nullptr != (inst->m_Expr = CBotParExpr::Compile( p, pStk )))
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

        pStk->SetError(TX_BADTYPE, &inst->m_token);
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
        if (!m_Expr->Execute(pile)) return false;                    // interrupted ?
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
        m_Expr->RestoreState(pile, bMain);                        // interrupted here!
        return;
    }
}
