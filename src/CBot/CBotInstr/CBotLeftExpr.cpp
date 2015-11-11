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
#include "CBotLeftExpr.h"
#include "CBotFieldExpr.h"
#include "CBotIndexExpr.h"
#include "CBotExpression.h"

// Local include

// Global include


//////////////////////////////////////////////////////////////////////////////////////
CBotLeftExpr::CBotLeftExpr()
{
    name    = "CBotLeftExpr";
    m_nIdent = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotLeftExpr::~CBotLeftExpr()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotLeftExpr* CBotLeftExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it a variable name?
    if (p->GetType() == TokenTypVar)
    {
        CBotLeftExpr* inst = new CBotLeftExpr();    // creates the object

        inst->SetToken(p);

        CBotVar*     var;

        if (nullptr != (var = pStk->FindVar(p)))   // seek if known variable
        {
            inst->m_nIdent = var->GetUniqNum();
            if (inst->m_nIdent > 0 && inst->m_nIdent < 9000)
            {
                if ( var->IsPrivate(PR_READ) &&
                     !pStk->GetBotCall()->m_bCompileClass)
                {
                    pStk->SetError(TX_PRIVATE, p);
                    goto err;
                }
                // this is an element of the current class
                // adds the equivalent of this. before
                CBotToken pthis("this");
                inst->SetToken(&pthis);
                inst->m_nIdent = -2;    // indent for this

                CBotFieldExpr* i = new CBotFieldExpr();     // new element
                i->SetToken(p);     // keeps the name of the token
                inst->AddNext3(i);  // add after

                var = pStk->FindVar(pthis);
                var = var->GetItem(p->GetString());
                i->SetUniqNum(var->GetUniqNum());
            }
            p = p->GetNext();   // next token

            while (true)
            {
                if (var->GetType() == CBotTypArrayPointer)
                {
                    if (IsOfType( p, ID_OPBRK ))
                    {
                        CBotIndexExpr* i = new CBotIndexExpr();
                        i->m_expr = CBotExpression::Compile(p, pStk);
                        inst->AddNext3(i);  // add to the chain

                        var = (static_cast<CBotVarArray*>(var))->GetItem(0,true);    // gets the component [0]

                        if (i->m_expr == nullptr)
                        {
                            pStk->SetError(TX_BADINDEX, p->GetStart());
                            goto err;
                        }

                        if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                        {
                            pStk->SetError(TX_CLBRK, p->GetStart());
                            goto err;
                        }
                        continue;
                    }
                }

                if (var->GetType(1) == CBotTypPointer)                // for classes
                {
                    if (IsOfType(p, ID_DOT))
                    {
                        CBotToken* pp = p;

                        CBotFieldExpr* i = new CBotFieldExpr();            // new element
                        i->SetToken(pp);                                // keeps the name of the token
                        inst->AddNext3(i);                                // adds after

                        if (p->GetType() == TokenTypVar)                // must be a name
                        {
                            var = var->GetItem(p->GetString());            // get item correspondent
                            if (var != nullptr)
                            {
                                if ( var->IsPrivate(PR_READ) &&
                                     !pStk->GetBotCall()->m_bCompileClass)
                                {
                                    pStk->SetError(TX_PRIVATE, pp);
                                    goto err;
                                }

                                i->SetUniqNum(var->GetUniqNum());
                                p = p->GetNext();                        // skips the name
                                continue;
                            }
                            pStk->SetError(TX_NOITEM, p);
                        }
                        pStk->SetError(TX_DOT, p->GetStart());
                        goto err;
                    }
                }
                break;
            }


            if (pStk->IsOk()) return static_cast<CBotLeftExpr*> (pStack->Return(inst, pStk));
        }
        pStk->SetError(TX_UNDEFVAR, p);
err:
        delete inst;
        return static_cast<CBotLeftExpr*> ( pStack->Return(nullptr, pStk));
    }

    return static_cast<CBotLeftExpr*> ( pStack->Return(nullptr, pStk));
}

////////////////////////////////////////////////////////////////////////////////
bool CBotLeftExpr::Execute(CBotStack* &pj, CBotStack* array)
{
    CBotStack*    pile = pj->AddStack();

    CBotVar*     var1 = nullptr;
    CBotVar*     var2 = nullptr;
    // fetch a variable (not copy)
    if (!ExecuteVar(var1, array, nullptr, false)) return false;

    if (pile->IfStep()) return false;

    if (var1)
    {
        var2 = pj->GetVar();    // result on the input stack
        if (var2)
        {
            CBotTypResult t1 = var1->GetTypResult();
            CBotTypResult t2 = var2->GetTypResult();
            if (t2.Eq(CBotTypPointer))
            {
                CBotClass*    c1 = t1.GetClass();
                CBotClass*    c2 = t2.GetClass();
                if ( !c2->IsChildOf(c1))
                {
                    CBotToken* pt = &m_token;
                    pile->SetError(TX_BADTYPE, pt);
                    return pj->Return(pile);    // operation performed
                }
            }
            var1->SetVal(var2);     // do assignment
        }
        pile->SetCopyVar(var1);     // replace the stack with the copy of the variable
                                    // (for name)
    }

    return pj->Return(pile);    // operation performed
}

////////////////////////////////////////////////////////////////////////////////
bool CBotLeftExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    pVar = pile->FindVar(m_token);
    if (pVar == nullptr) return false;

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile) ) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotLeftExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep)
{
    pile = pile->AddStack(this);

    pVar = pile->FindVar(m_nIdent);
    if (pVar == nullptr)
    {
#ifdef    _DEBUG
        assert(0);
#endif
        pile->SetError(2, &m_token);
        return false;
    }

    if (bStep && m_next3 == nullptr && pile->IfStep()) return false;

    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, true) ) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotLeftExpr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    pile = pile->RestoreStack(this);
    if (pile == nullptr) return;

    if (m_next3 != nullptr)
         m_next3->RestoreStateVar(pile, bMain);
}
