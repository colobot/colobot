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
#include "CBotInstArray.h"

#include "CBotLeftExprVar.h"
#include "CBotExpression.h"
#include "CBotListArray.h"
#include "CBotEmpty.h"

#include "CBotStack.h"

#include "CBotDefines.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include


////////////////////////////////////////////////////////////////////////////////
CBotInstArray::CBotInstArray()
{
    m_var      = nullptr;
    m_listass = nullptr;
    name = "CBotInstArray";
}

////////////////////////////////////////////////////////////////////////////////
CBotInstArray::~CBotInstArray()
{
    delete m_var;
    delete m_listass;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotInstArray*    inst = new CBotInstArray();

    CBotToken*    vartoken = p;
    inst->SetToken(vartoken);

    // determinse the expression is valid for the item on the left side
    if (nullptr != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )))
    {
        if (pStk->CheckVarLocal(vartoken))                              // redefinition of the variable?
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        CBotInstr*    i;
        while (IsOfType(p,  ID_OPBRK))
        {
            if (p->GetType() != ID_CLBRK)
                i = CBotExpression::Compile(p, pStk);                   // expression for the value
            else
                i = new CBotEmpty();                                    // if no special formula

            inst->AddNext3b(i);                                         // construct a list
            type = CBotTypResult(CBotTypArrayPointer, type);

            if (!pStk->IsOk() || !IsOfType(p, ID_CLBRK ))
            {
                pStk->SetError(TX_CLBRK, p->GetStart());
                goto error;
            }
        }

        CBotVar*   var = CBotVar::Create(vartoken, type);               // create an instance
        inst->m_typevar = type;

        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
        pStack->AddVar(var);                                            // place it on the stack

        if (IsOfType(p, ID_ASS))                                        // with an assignment
        {
            inst->m_listass = CBotListArray::Compile(p, pStk, type.GetTypElem());
        }

        if (pStk->IsOk()) return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstArray::Execute(CBotStack* &pj)
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
                pile->SetError(TX_OUTARRAY, &m_token);
                return pj->Return (pile);
            }
            n++;
            p = p->GetNext3b();
        }
        while (n<100) max[n++] = 0;

        m_typevar.SetArray(max);                                    // store the limitations

        // create simply a nullptr pointer
        CBotVar*    var = CBotVar::Create(m_var->GetToken(), m_typevar);
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
            CBotVar* pVar = pj->FindVar((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);

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
void CBotInstArray::RestoreState(CBotStack* &pj, bool bMain)
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
