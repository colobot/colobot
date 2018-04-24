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
#include "CBot/CBotInstr/CBotExprVar.h"
#include "CBot/CBotInstr/CBotInstrMethode.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotIndexExpr.h"
#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVarArray.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprVar::CBotExprVar()
{
    m_nIdent = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotExprVar::~CBotExprVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprVar::Compile(CBotToken*& p, CBotCStack* pStack, bool bCheckReadOnly)
{
//    CBotToken*    pDebut = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GetStart());

    // is it a variable?
    if (p->GetType() == TokenTypVar)
    {
        CBotInstr* inst = new CBotExprVar();    // create the object

        inst->SetToken(p);

        CBotVar*     var;

        if (nullptr != (var = pStk->FindVar(p)))   // seek if known variable
        {
            int        ident = var->GetUniqNum();
            (static_cast<CBotExprVar*>(inst))->m_nIdent = ident;     // identifies variable by its number

            if (ident > 0 && ident < 9000)
            {
                if (CBotFieldExpr::CheckProtectionError(pStk, nullptr, var, bCheckReadOnly))
                {
                    pStk->SetError(CBotErrPrivate, p);
                    goto err;
                }

                // This is an element of the current class
                // ads the equivalent of this. before
                CBotToken token("this");
                // invisible 'this.' highlights member token on error
                token.SetPos(p->GetStart(), p->GetEnd());
                inst->SetToken(&token);
                (static_cast<CBotExprVar*>(inst))->m_nIdent = -2;    // identificator for this

                CBotFieldExpr* i = new CBotFieldExpr();     // new element
                i->SetToken(p);     // keeps the name of the token
                i->SetUniqNum(ident);
                inst->AddNext3(i);  // added after
            }

            p = p->GetNext();   // next token

            while (true)
            {
                if (var->GetType() == CBotTypArrayPointer)
                {
                    if (IsOfType( p, ID_OPBRK ))    // check if there is an aindex
                    {
                        CBotIndexExpr* i = new CBotIndexExpr();
                        i->m_expr = CBotExpression::Compile(p, pStk);   // compile the formula
                        inst->AddNext3(i);  // add to the chain

                        var = (static_cast<CBotVarArray*>(var))->GetItem(0,true);    // gets the component [0]

                        if (i->m_expr == nullptr)
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
                if (var->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypPointer)  // for classes
                {
                    if (IsOfType(p, ID_DOT))
                    {
                        CBotToken* pp = p;

                        if (p->GetType() == TokenTypVar)    // must be a name
                        {
                            if (p->GetNext()->GetType() == ID_OPENPAR)  // a method call?
                            {
                                if (bCheckReadOnly) goto err; // don't allow increment a method call "++"

                                CBotInstr* i = CBotInstrMethode::Compile(p, pStk, var);
                                if (!pStk->IsOk()) goto err;
                                inst->AddNext3(i);  // added after
                                return pStack->Return(inst, pStk);
                            }
                            else
                            {
                                CBotFieldExpr* i = new CBotFieldExpr();     // new element
                                i->SetToken(pp);                            // keeps the name of the token
                                inst->AddNext3(i);                          // add after
                                CBotVar*   preVar = var;
                                var = var->GetItem(p->GetString());         // get item correspondent
                                if (var != nullptr)
                                {
                                    i->SetUniqNum(var->GetUniqNum());
                                    if (CBotFieldExpr::CheckProtectionError(pStk, preVar, var, bCheckReadOnly))
                                    {
                                        pStk->SetError(CBotErrPrivate, pp);
                                        goto err;
                                    }
                                }
                            }


                            if (var != nullptr)
                            {
                                p = p->GetNext();   // skips the name
                                continue;
                            }
                            pStk->SetError(CBotErrUndefItem, p);
                            goto err;
                        }
                        pStk->SetError(CBotErrUndefClass, p->GetStart());
                        goto err;
                    }
                }

                break;
            }

            pStk->SetCopyVar(var);  // place the copy of the variable on the stack (for type)
            if (pStk->IsOk()) return pStack->Return(inst, pStk);
        }
        pStk->SetError(CBotErrUndefVar, p);
err:
        delete inst;
        return pStack->Return(nullptr, pStk);
    }

    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprVar::CompileMethode(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(pp->GetStart());

    // is it a variable ?
    if (pp->GetType() == TokenTypVar)
    {
        CBotToken pthis("this");
        CBotVar*     var = pStk->FindVar(pthis);
        if (var == nullptr) return pStack->Return(nullptr, pStk);

        CBotInstr* inst = new CBotExprVar();

        // this is an element of the current class
        // adds the equivalent of this. before

        // invisible 'this.' highlights member token on error
        pthis.SetPos(pp->GetStart(), pp->GetEnd());
        inst->SetToken(&pthis);
        (static_cast<CBotExprVar*>(inst))->m_nIdent = -2;    // ident for this

        CBotToken* pp = p;

        if (pp->GetType() == TokenTypVar)
        {
            if (pp->GetNext()->GetType() == ID_OPENPAR)        // a method call?
            {
                CBotInstr* i = CBotInstrMethode::Compile(pp, pStk, var);
                if (pStk->IsOk())
                {
                    inst->AddNext3(i);                            // add after
                    p = pp;                                       // previous instruction
                    return pStack->Return(inst, pStk);
                }
                pStk->SetError(CBotNoErr, static_cast<std::size_t>(0));             // the error is not adressed here
            }
        }
        delete inst;
    }
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     pVar = nullptr;
    CBotStack*     pile  = pj->AddStack(this);

    CBotStack*     pile1 = pile;

    if (pile1->GetState() == 0)
    {
        if (!ExecuteVar(pVar, pile, nullptr, true)) return false;        // Get the variable fields and indexes according

        if (pVar) pile1->SetCopyVar(pVar);                            // place a copy on the stack
        else
        {
            return pj->Return(pile1);
        }
        pile1->IncState();
    }

    pVar = pile1->GetVar();

    if (pVar == nullptr)
    {
        return pj->Return(pile1);
    }

    if (pVar->IsUndefined())
    {
        CBotToken* pt = &m_token;
        while (pt->GetNext() != nullptr) pt = pt->GetNext();
        pile1->SetError(CBotErrNotInit, pt);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);   // operation completed
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprVar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack*     pile  = pj->RestoreStack(this);
    if (pile == nullptr) return;

    CBotStack*     pile1 = pile;

    if (pile1->GetState() == 0)
    {
        RestoreStateVar(pile, bMain);   // retrieves the variable fields and indexes according
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprVar::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep)
{
    CBotStack*    pile = pj;
    pj = pj->AddStack(this);

    if (bStep && m_nIdent>0 && pj->IfStep()) return false;

    pVar = pj->FindVar(m_nIdent, true);     // tries with the variable update if necessary
    if (pVar == nullptr)
    {
        assert(false);
        //pj->SetError(static_cast<CBotError>(1), &m_token); // TODO: yeah, don't care that this exception doesn't exist ~krzys_h
        return false;
    }
    if ( m_next3 != nullptr &&
         !m_next3->ExecuteVar(pVar, pj, &m_token, bStep, false) )
            return false;   // field of an instance, table, methode

    return pile->ReturnKeep(pj);    // does not put on stack but get the result if a method was called
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprVar::RestoreStateVar(CBotStack* &pj, bool bMain)
{
    pj = pj->RestoreStack(this);
    if (pj == nullptr) return;

    if (m_next3 != nullptr)
         m_next3->RestoreStateVar(pj, bMain);
}

std::string CBotExprVar::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << std::endl;
    //ss << "VarID = " << m_nIdent;
    return ss.str();
}

} // namespace CBot
