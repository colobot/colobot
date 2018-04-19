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

#include <sstream>
#include "CBot/CBotInstr/CBotExprRetVar.h"

#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotInstrMethode.h"
#include "CBot/CBotInstr/CBotIndexExpr.h"
#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprRetVar::CBotExprRetVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprRetVar::~CBotExprRetVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprRetVar::Compile(CBotToken*& p, CBotCStack* pStack, bool bMethodsOnly)
{
    if (p->GetType() == ID_DOT)
    {
        CBotVar*     var = pStack->GetVar();

        if (var == nullptr) return nullptr;

        CBotCStack* pStk = pStack->TokenStack();
        CBotInstr* inst = new CBotExprRetVar();

        while (true)
        {
            pStk->SetStartError(p->GetStart());
            if (var->GetType() == CBotTypArrayPointer)
            {
                if (bMethodsOnly) goto err;

                if (IsOfType( p, ID_OPBRK ))
                {
                    CBotIndexExpr* i = new CBotIndexExpr();
                    i->m_expr = CBotExpression::Compile(p, pStk);
                    inst->AddNext3(i);

                    var = var->GetItem(0,true);

                    if (i->m_expr == nullptr || pStk->GetType() != CBotTypInt)
                    {
                        pStk->SetError(CBotErrBadIndex, p->GetStart());
                        goto err;
                    }
                    if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                    {
                        pStk->SetError(CBotErrCloseIndex, p->GetStart());
                        goto err;
                    }
                    continue;
                }
            }
            if (var->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypPointer)
            {
                if (IsOfType(p, ID_DOT))
                {
                    CBotToken* pp = p;

                    if (p->GetType() == TokenTypVar)
                    {
                        if (p->GetNext()->GetType() == ID_OPENPAR)
                        {
                            CBotInstr* i = CBotInstrMethode::Compile(p, pStk, var, bMethodsOnly);
                            if (!pStk->IsOk()) goto err;
                            inst->AddNext3(i);
                            return pStack->Return(inst, pStk);
                        }
                        else if (bMethodsOnly)
                        {
                            p = p->GetPrev();
                            goto err;
                        }
                        else
                        {
                            CBotFieldExpr* i = new CBotFieldExpr();
                            i->SetToken(pp);
                            inst->AddNext3(i);
                            CBotVar*   preVar = var;
                            var = var->GetItem(p->GetString());
                            if (var != nullptr)
                            {
                                i->SetUniqNum(var->GetUniqNum());
                                if (CBotFieldExpr::CheckProtectionError(pStk, preVar, var))
                                {
                                    pStk->SetError(CBotErrPrivate, pp);
                                    goto err;
                                }
                            }
                        }

                        if (var != nullptr)
                        {
                            p = p->GetNext();
                            continue;
                        }
                        pStk->SetError(CBotErrUndefItem, p);
                        goto err;
                    }
                    pStk->SetError(CBotErrUndefClass, p);
                    goto err;
                }
            }
            break;
        }

        pStk->SetCopyVar(var);
        if (pStk->IsOk()) return pStack->Return(inst, pStk);

        pStk->SetError(CBotErrUndefVar, p);
err:
        delete inst;
        return pStack->Return(nullptr, pStk);
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprRetVar::Execute(CBotStack* &pj)
{

    CBotStack* pile = pj->AddStack();
    CBotStack* pile1 = pile;
    CBotVar* pVar;

    if (pile1->GetState() == 0)
    {
        pVar = pj->GetVar();
        pVar->Update(pj->GetUserPtr());
        if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypNullPointer)
        {
            pile1->SetError(CBotErrNull, &m_token);
            return pj->Return(pile1);
        }

        if ( !m_next3->ExecuteVar(pVar, pile, &m_token, true, false) )
            return false;

        if (pVar)
            pile1->SetCopyVar(pVar);
        else
            return pj->Return(pile1);

        pile1->IncState();
    }
    pVar = pile1->GetVar();

    if (pVar == nullptr)
    {
        return pj->Return(pile1);
    }

    if (pVar->IsUndefined())
    {
        pile1->SetError(CBotErrNotInit, &m_token);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprRetVar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack* pile = pj->RestoreStack();
    if ( pile == nullptr ) return;

    if (pile->GetState() == 0)
        m_next3->RestoreStateVar(pile, bMain);
}

std::string CBotExprRetVar::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << "func(...).something" << std::endl;
    return ss.str();
}

} // namespace CBot
