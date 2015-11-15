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
#include "CBotListArray.h"

#include "CBotExprNull.h"
#include "CBotTwoOpExpr.h"

#include "CBotStack.h"
#include "CBotCStack.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include

////////////////////////////////////////////////////////////////////////////////
CBotListArray::CBotListArray()
{
    m_expr    = nullptr;
    name = "CBotListArray";
}

////////////////////////////////////////////////////////////////////////////////
CBotListArray::~CBotListArray()
{
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotListArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotToken* pp = p;

    if (IsOfType( p, ID_NULL ))
    {
        CBotInstr* inst = new CBotExprNull ();
        inst->SetToken(pp);
        return pStack->Return(inst, pStk);            // ok with empty element
    }

    CBotListArray*    inst = new CBotListArray();

    if (IsOfType( p, ID_OPENPAR ))
    {
        // each element takes the one after the other
        if (type.Eq( CBotTypArrayPointer ))
        {
            type = type.GetTypElem();

            pStk->SetStartError(p->GetStart());
            if (nullptr == ( inst->m_expr = CBotListArray::Compile( p, pStk, type ) ))
            {
                goto error;
            }

            while (IsOfType( p, ID_COMMA ))                                     // other elements?
            {
                pStk->SetStartError(p->GetStart());

                CBotInstr* i = CBotListArray::Compile(p, pStk, type);
                if (nullptr == i)
                {
                    goto error;
                }

                inst->m_expr->AddNext3(i);
            }
        }
        else
        {
            pStk->SetStartError(p->GetStart());
            if (nullptr == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )))
            {
                goto error;
            }
            CBotVar* pv = pStk->GetVar();                                       // result of the expression

            if (pv == nullptr || !TypesCompatibles( type, pv->GetTypResult()))     // compatible type?
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }

            while (IsOfType( p, ID_COMMA ))                                      // other elements?
            {
                pStk->SetStartError(p->GetStart());

                CBotInstr* i = CBotTwoOpExpr::Compile(p, pStk) ;
                if (nullptr == i)
                {
                    goto error;
                }

                CBotVar* pv = pStk->GetVar();                                   // result of the expression

                if (pv == nullptr || !TypesCompatibles( type, pv->GetTypResult())) // compatible type?
                {
                    pStk->SetError(TX_BADTYPE, p->GetStart());
                    goto error;
                }
                inst->m_expr->AddNext3(i);
            }
        }

        if (!IsOfType(p, ID_CLOSEPAR) )
        {
            pStk->SetError(TX_CLOSEPAR, p->GetStart());
            goto error;
        }

        return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotListArray::Execute(CBotStack* &pj, CBotVar* pVar)
{
    CBotStack*    pile1 = pj->AddStack();
    CBotVar* pVar2;

    CBotInstr* p = m_expr;

    int n = 0;

    for (; p != nullptr ; n++, p = p->GetNext3())
    {
        if (pile1->GetState() > n) continue;

        pVar2 = pVar->GetItem(n, true);

        if (!p->Execute(pile1, pVar2)) return false;        // evaluate expression

        pile1->IncState();
    }

    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotListArray::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain)
    {
        CBotStack*    pile  = pj->RestoreStack(this);
        if (pile == nullptr) return;

        CBotInstr* p = m_expr;

        int    state = pile->GetState();

        while(state-- > 0) p = p->GetNext3() ;

        p->RestoreState(pile, bMain);                    // size calculation //interrupted!
    }
}
