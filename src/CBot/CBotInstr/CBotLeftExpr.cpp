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
#include "CBot/CBotInstr/CBotLeftExpr.h"
#include "CBot/CBotInstr/CBotFieldExpr.h"
#include "CBot/CBotInstr/CBotIndexExpr.h"
#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"

#include "CBot/CBotVar/CBotVarArray.h"

namespace CBot
{

//////////////////////////////////////////////////////////////////////////////////////
CBotLeftExpr::CBotLeftExpr()
{
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
                if (CBotFieldExpr::CheckProtectionError(pStk, nullptr, var, true))
                {
                    pStk->SetError(CBotErrPrivate, p);
                    goto err;
                }
                // this is an element of the current class
                // adds the equivalent of this. before
                CBotToken pthis("this");
                // invisible 'this.' highlights member token on error
                pthis.SetPos(p->GetStart(), p->GetEnd());
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

                if (var->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypPointer)                // for classes
                {
                    if (IsOfType(p, ID_DOT))
                    {
                        CBotToken* pp = p;

                        CBotFieldExpr* i = new CBotFieldExpr();            // new element
                        i->SetToken(pp);                                // keeps the name of the token
                        inst->AddNext3(i);                                // adds after

                        if (p->GetType() == TokenTypVar)                // must be a name
                        {
                            CBotVar*   preVar = var;
                            var = var->GetItem(p->GetString());            // get item correspondent
                            if (var != nullptr)
                            {
                                if (CBotFieldExpr::CheckProtectionError(pStk, preVar, var, true))
                                {
                                    pStk->SetError(CBotErrPrivate, pp);
                                    goto err;
                                }

                                i->SetUniqNum(var->GetUniqNum());
                                p = p->GetNext();                        // skips the name
                                continue;
                            }
                            pStk->SetError(CBotErrUndefItem, p);
                        }
                        pStk->SetError(CBotErrUndefClass, p->GetStart());
                        goto err;
                    }
                }
                break;
            }


            if (pStk->IsOk()) return static_cast<CBotLeftExpr*> (pStack->Return(inst, pStk));
        }
        pStk->SetError(CBotErrUndefVar, p);
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
                CBotClass*    c2 = var2->GetClass();
                if ( !c2->IsChildOf(c1))
                {
                    CBotToken* pt = &m_token;
                    pile->SetError(CBotErrBadType1, pt);
                    return pj->Return(pile);    // operation performed
                }
                var1->SetVal(var2);     // set pointer
                var1->SetType(t1);      // keep pointer type
            }
            else
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

    pVar = pile->FindVar(m_nIdent, false);
    if (pVar == nullptr)
    {
        assert(false);
        //pile->SetError(static_cast<CBotError>(2), &m_token); // TODO: yup, another unknown error ~krzys_h
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

std::string CBotLeftExpr::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString();
    //ss << "VarID = " << m_nIdent;
    return ss.str();
}

} // namespace CBot
