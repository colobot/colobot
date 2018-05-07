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

#include "CBot/CBotInstr/CBotDefArray.h"

#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotListArray.h"
#include "CBot/CBotInstr/CBotEmpty.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotDefines.h"

#include "CBot/CBotVar/CBotVar.h"

#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDefArray::CBotDefArray()
{
    m_var     = nullptr;
    m_listass = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefArray::~CBotDefArray()
{
    delete m_var;
    delete m_listass;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotDefArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotDefArray*    inst = new CBotDefArray();

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    // determinse the expression is valid for the item on the left side
    if (nullptr != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        if (pStk->CheckVarLocal(vartoken))                              // redefinition of the variable?
        {
            pStk->SetError(CBotErrRedefVar, vartoken);
            goto error;
        }

        CBotInstr*    i;
        while (IsOfType(p,  ID_OPBRK))
        {
            pStk->SetStartError(p->GetStart());
            if (p->GetType() != ID_CLBRK)
            {
                i = CBotExpression::Compile(p, pStk);                  // expression for the value
                if (i == nullptr || pStk->GetType() != CBotTypInt)     // must be a number
                {
                    pStk->SetError(CBotErrBadIndex, p->GetStart());
                    goto error;
                }
            }
            else
                i = new CBotEmpty();                                    // if no special formula

            inst->AddNext3b(i);                                         // construct a list
            type = CBotTypResult(CBotTypArrayPointer, type);

            if (IsOfType(p, ID_CLBRK)) continue;

            pStk->SetError(CBotErrCloseIndex, p->GetStart());
            goto error;
        }

        CBotVar*   var = CBotVar::Create(*vartoken, type);               // create an instance
        inst->m_typevar = type;

        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);                                            // place it on the stack

        if (IsOfType(p, ID_ASS))                                        // with an assignment
        {
            pStk->SetStartError(p->GetStart());
            if ( IsOfType(p, ID_SEP) )
            {
                pStk->SetError(CBotErrNoExpression, p->GetPrev());
                goto error;
            }
            if ( nullptr == (inst->m_listass = CBotListArray::Compile(p, pStk, type.GetTypElem())) )
            {
                if (pStk->IsOk())
                {
                    inst->m_listass = CBotTwoOpExpr::Compile(p, pStk);
                    if (inst->m_listass == nullptr || !pStk->GetTypResult().Compare(type))  // compatible type ?
                    {
                        pStk->SetError(CBotErrBadType1, p->GetStart());
                        goto error;
                    }
                }
            }

            if (pStk->IsOk()) while (true)       // mark initialized
            {
                var = var->GetItem(0, true);
                if (var == nullptr) break;
                if (var->GetType() == CBotTypArrayPointer) continue;
                if (var->GetType() <= CBotTypString) var->SetInit(CBotVar::InitType::DEF);
                break;
            }
        }

        if (pStk->IsOk()) return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDefArray::Execute(CBotStack* &pj)
{
    CBotStack*    pile1 = pj->AddStack(this);

    CBotStack*    pile  = pile1;

    if (pile1->GetState() == 0)
    {
        // seek the maximum dimension of the table
        CBotInstr*    p  = GetNext3b();                             // the different formulas
        int            nb = 0;

        while (p != nullptr)
        {
            pile = pile->AddStack();                                // little room to work
            nb++;
            if (pile->GetState() == 0)
            {
                if (!p->Execute(pile)) return false;                // size calculation //interrupted?
                pile->IncState();
            }
            p = p->GetNext3b();
        }

        p     = GetNext3b();
        pile = pile1;                                               // returns to the stack
        int     n = 0;
        int     max[100];

        while (p != nullptr)
        {
            pile = pile->AddStack();
            CBotVar*    v = pile->GetVar();                         // result
            max[n] = v->GetValInt();                                // value
            if (max[n]>MAXARRAYSIZE)
            {
                pile->SetError(CBotErrOutArray, &m_token);
                return pj->Return (pile);
            }
            n++;
            p = p->GetNext3b();
        }
        while (n<100) max[n++] = 0;

        m_typevar.SetArray(max);                                    // store the limitations

        // create simply a nullptr pointer
        CBotVar*    var = CBotVar::Create(*(m_var->GetToken()), m_typevar);
        var->SetPointer(nullptr);
        var->SetUniqNum((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);
        pj->AddVar(var);

#if        STACKMEM
        pile1->AddStack()->Delete();
#else
        delete pile1->AddStack();                                   // need more indices
#endif
        pile1->IncState();
    }

    if (pile1->GetState() == 1)
    {
        if (m_listass != nullptr)                                      // there is the assignment for this table
        {
            CBotVar* pVar = pj->FindVar((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent, false);

            if (!m_listass->Execute(pile1, pVar)) return false;
        }
        pile1->IncState();
    }

    if (pile1->IfStep()) return false;

    if ( m_next2b &&
         !m_next2b->Execute(pile1 )) return false;

    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotDefArray::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*    pile1 = pj;

    CBotVar*    var = pj->FindVar(m_var->GetToken()->GetString());
    if (var != nullptr) var->SetUniqNum((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);

    if (bMain)
    {
        pile1 = pj->RestoreStack(this);
        CBotStack*    pile  = pile1;
        if (pile == nullptr) return;

        if (pile1->GetState() == 0)
        {
            // seek the maximum dimension of the table
            CBotInstr*    p  = GetNext3b();

            while (p != nullptr)
            {
                pile = pile->RestoreStack();
                if (pile == nullptr) return;
                if (pile->GetState() == 0)
                {
                    p->RestoreState(pile, bMain);
                    return;
                }
                p = p->GetNext3b();
            }
        }
        if (pile1->GetState() == 1 && m_listass != nullptr)
        {
            m_listass->RestoreState(pile1, bMain);
        }

    }

    if (m_next2b ) m_next2b->RestoreState( pile1, bMain);
}

std::string CBotDefArray::GetDebugData()
{
    std::stringstream ss;
    ss << m_typevar.ToString();
    return ss.str();
}

std::map<std::string, CBotInstr*> CBotDefArray::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_var"] = m_var;
    links["m_listass"] = m_listass;
    return links;
}

} // namespace CBot
