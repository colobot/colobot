/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotInstr/CBotExpression.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotTwoOpExpr.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <cassert>

namespace CBot
{

//////////////////////////////////////////////////////////////////////////////////////
CBotExpression::CBotExpression()
{
    m_leftop    = nullptr;
    m_rightop   = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotExpression::~CBotExpression()
{
    delete    m_leftop;
    delete    m_rightop;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExpression::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;

    CBotExpression* inst = new CBotExpression();

    inst->m_leftop = CBotLeftExpr::Compile(p, pStack);

    inst->SetToken(p);
    int  OpType = p->GetType();

    if ( pStack->IsOk() &&
         IsOfTypeList(p, ID_ASS, ID_ASSADD, ID_ASSSUB, ID_ASSMUL, ID_ASSDIV, ID_ASSMODULO,
                                 ID_ASSAND, ID_ASSXOR, ID_ASSOR,
                                 ID_ASSSL , ID_ASSSR,  ID_ASSASR, 0 ))
    {
        if (inst->m_leftop == nullptr)
        {
            pStack->SetError(CBotErrBadLeft, p->GetEnd());
            delete inst;
            return nullptr;
        }

        if ( p->GetType() == ID_SEP )
        {
            pStack->SetError(CBotErrNoExpression, p);
            delete inst;
            return nullptr;
        }

        inst->m_rightop = CBotExpression::Compile(p, pStack);
        if (inst->m_rightop == nullptr)
        {
            delete inst;
            return nullptr;
        }

        CBotTypResult    type1 = pStack->GetTypResult();

        // get the variable assigned to mark
        CBotVar*    var = nullptr;
        inst->m_leftop->ExecuteVar(var, pStack);
        if (var == nullptr)
        {
            delete inst;
            return nullptr;
        }

        if (OpType != ID_ASS && !var->IsDefined())
        {
            pStack->SetError(CBotErrNotInit, pp);
            delete inst;
            return nullptr;
        }

        CBotTypResult type2 = var->GetTypResult();

        // what types are acceptable?
        switch (OpType)
        {
        case ID_ASS:
    //        if (type2 == CBotTypClass) type2 = -1;
            if ((type1.Eq(CBotTypPointer) && type2.Eq(CBotTypPointer)) ||
                 (type1.Eq(CBotTypClass)   && type2.Eq(CBotTypClass)   ) )
            {
/*                CBotClass*    c1 = type1.GetClass();
                CBotClass*    c2 = type2.GetClass();
                if (!c1->IsChildOf(c2)) type2.SetType(-1);
//-                if (!type1.Eq(CBotTypClass)) var->SetPointer(pStack->GetVar()->GetPointer());*/
                var->SetInit(CBotVar::InitType::IS_POINTER);
            }
            else
                var->SetInit(CBotVar::InitType::DEF);

            break;
        case ID_ASSADD:
            if (type2.Eq(CBotTypBoolean) ||
                type2.GetType() > CBotTypString ) type2.SetType(-1);    // numbers and strings
            break;
        case ID_ASSSUB:
        case ID_ASSMUL:
        case ID_ASSDIV:
        case ID_ASSMODULO:
            if (type2.GetType() >= CBotTypBoolean) type2.SetType(-1);    // numbers only
            break;
        }

        if (!TypeCompatible(type1, type2, OpType))
        {
            pStack->SetError(CBotErrBadType1, &inst->m_token);
            delete inst;
            return nullptr;
        }

        return inst;        // compatible type?
    }

    delete inst;
    std::size_t  start, end;
    CBotError error = pStack->GetError(start, end);

    p = pp;                                        // returns to the top
    pStack->SetError(CBotNoErr,static_cast<std::size_t>(0)); // forget the error

    CBotInstr* i = CBotTwoOpExpr::Compile(p, pStack);    // tries without assignment
    if (i != nullptr && error == CBotErrPrivate && p->GetType() == ID_ASS)
        pStack->ResetError(error, start, end);
    return i;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExpression::Execute(CBotStack* &pj)
{
    CBotStack*    pile  = pj->AddStack(this);

//    CBotToken*    pToken = m_leftop->GetToken();

    CBotVar*    pVar = nullptr;

    CBotStack*  pile1 = pile;

    CBotVar::InitType initKind = CBotVar::InitType::DEF;
    CBotVar*    result = nullptr;

    // must be done before any indexes (stack can be changed)
    if (!m_leftop->ExecuteVar(pVar, pile, nullptr, false)) return false;    // variable before accessing the value on the right

    if ( pile1->GetState()==0)
    {
        pile1->SetCopyVar(pVar);    // keeps the copy on the stack (if interrupted)
        pile1->IncState();
    }

    CBotStack*    pile2 = pile->AddStack();

    if ( pile2->GetState()==0)
    {
        if (m_rightop && !m_rightop->Execute(pile2)) return false;    // initial value // interrupted?
        if (m_rightop)
        {
            CBotVar* var = pile1->GetVar();
            CBotVar* value = pile2->GetVar();
            if (var->GetType() == CBotTypString && value->GetType() != CBotTypString)
            {
                CBotVar* newVal = CBotVar::Create("", var->GetTypResult());
                value->Update(pj->GetUserPtr());
                newVal->SetValString(value->GetValString());
                pile2->SetVar(newVal);
            }
        }
        pile2->IncState();
    }

    if (pile1->GetState() == 1)
    {
        if (m_token.GetType() != ID_ASS)
        {
            pVar = pile1->GetVar();     // recovers if interrupted
            initKind = pVar->GetInit();
            if (initKind == CBotVar::InitType::IS_NAN)
            {
                pile2->SetError(CBotErrNan, m_leftop->GetToken());
                return pj->Return(pile2);
            }
            result = CBotVar::Create("", pVar->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC));
        }

        switch (m_token.GetType())
        {
        case ID_ASS:
            break;
        case ID_ASSADD:
            result->Add(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSSUB:
            result->Sub(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSMUL:
            result->Mul(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSDIV:
            if (initKind != CBotVar::InitType::UNDEF &&
                result->Div(pile1->GetVar(), pile2->GetVar()))
                pile2->SetError(CBotErrZeroDiv, &m_token);
            pile2->SetVar(result);
            break;
        case ID_ASSMODULO:
            if (initKind != CBotVar::InitType::UNDEF &&
                result->Modulo(pile1->GetVar(), pile2->GetVar()))
                pile2->SetError(CBotErrZeroDiv, &m_token);
            pile2->SetVar(result);
            break;
        case ID_ASSAND:
            result->And(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSXOR:
            result->XOr(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSOR:
            result->Or(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSSL:
            result->SL(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSSR:
            result->SR(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        case ID_ASSASR:
            result->ASR(pile1->GetVar(), pile2->GetVar());
            pile2->SetVar(result);
            break;
        default:
            assert(0);
        }
        if (initKind == CBotVar::InitType::UNDEF)
            pile2->SetError(CBotErrNotInit, m_leftop->GetToken());

        pile1->IncState();
    }

    if (!m_leftop->Execute( pile2, pile1 ))
        return false;

    return pj->Return(pile2);
}

////////////////////////////////////////////////////////////////////////////////
void CBotExpression::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain)
    {
//        CBotToken*    pToken = m_leftop->GetToken();
//        CBotVar*    pVar = nullptr;

        CBotStack*    pile  = pj->RestoreStack(this);
        if (pile == nullptr) return;

        CBotStack*  pile1 = pile;

        if ( pile1->GetState()==0)
        {
            m_leftop->RestoreStateVar(pile, true);
            return;
        }

        m_leftop->RestoreStateVar(pile, false);

        CBotStack*    pile2 = pile->RestoreStack();
        if (pile2 == nullptr) return;

        if ( pile2->GetState()==0)
        {
            if (m_rightop) m_rightop->RestoreState(pile2, bMain);
            return;
        }
    }
}

std::map<std::string, CBotInstr*> CBotExpression::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_leftop"] = m_leftop;
    links["m_rightop"] = m_rightop;
    return links;
}

} // namespace CBot
