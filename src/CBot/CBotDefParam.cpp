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

#include "CBot/CBotDefParam.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"
#include "CBot/CBotInstr/CBotParExpr.h"

#include "CBot/CBotUtils.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVarClass.h"

#include <cassert>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotDefParam::CBotDefParam()
{
    m_nIdent = 0;
    m_expr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefParam::~CBotDefParam()
{
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotDefParam* CBotDefParam::Compile(CBotToken* &p, CBotCStack* pStack)
{
    // mainly not pStack->TokenStack here
    // declared variables must remain visible thereafter

    pStack->SetStartError(p->GetStart());

    if (IsOfType(p, ID_OPENPAR))
    {
        CBotDefParam* list = nullptr;
        bool prevHasDefault = false;

        if (!IsOfType(p, ID_CLOSEPAR)) while (true)
        {
            CBotDefParam* param = new CBotDefParam();
            if (list == nullptr) list = param;
            else list->AddNext(param);          // added to the list

//            CBotClass*  pClass = nullptr;//= CBotClass::Find(p);
            param->m_typename = p->GetString();
            CBotTypResult type = param->m_type = TypeParam(p, pStack);
//          if ( type == CBotTypPointer ) type = CBotTypClass;          // we must create a new object

            if (param->m_type.GetType() > 0)
            {
                CBotToken*  pp = p;
                param->m_token = *p;
                if (pStack->IsOk() && IsOfType(p, TokenTypVar) )
                {

                    // variable already declared?
                    if (pStack->CheckVarLocal(pp))
                    {
                        pStack->SetError(CBotErrRedefVar, pp);
                        break;
                    }

                    if (IsOfType(p, ID_ASS))       // default value assignment
                    {
                        CBotCStack* pStk = pStack->TokenStack(nullptr, true);
                        if (nullptr != (param->m_expr = CBotParExpr::CompileLitExpr(p, pStk)))
                        {
                            CBotTypResult valueType = pStk->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);

                            if (!TypesCompatibles(type, valueType))
                                pStack->SetError(CBotErrBadType1, p->GetPrev());

                            prevHasDefault = true;
                        }
                        else pStack->SetError(CBotErrNoExpression, p);
                        delete pStk;
                    }
                    else
                        if (prevHasDefault) pStack->SetError(CBotErrDefaultValue, p->GetPrev());

                    if (!pStack->IsOk()) break;

                    if ( type.Eq(CBotTypArrayPointer) ) type.SetType(CBotTypArrayBody);
                    CBotVar*    var = CBotVar::Create(pp->GetString(), type);       // creates the variable
//                  if ( pClass ) var->SetClass(pClass);
                    var->SetInit(CBotVar::InitType::IS_POINTER);                                    // mark initialized
                    param->m_nIdent = CBotVar::NextUniqNum();
                    var->SetUniqNum(param->m_nIdent);
                    pStack->AddVar(var);                                // place on the stack

                    if (IsOfType(p, ID_COMMA)) continue;
                    if (IsOfType(p, ID_CLOSEPAR)) break;

                    pStack->SetError(CBotErrClosePar, p->GetStart());
                }
                pStack->SetError(CBotErrNoVar, p->GetStart());
            }
            pStack->SetError(CBotErrNoType, p);
            delete list;
            return nullptr;
        }
        return list;
    }
    pStack->SetError(CBotErrOpenPar, p->GetStart());
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDefParam::Execute(CBotVar** ppVars, CBotStack* &pj)
{
    int             i = 0;
    assert(this != nullptr);
    CBotDefParam*   p = this;

    bool useDefault = false;

    while ( p != nullptr )
    {
        // creates a local variable on the stack
        CBotVar*    newvar = CBotVar::Create(p->m_token.GetString(), p->m_type);

        CBotVar*   pVar = nullptr;
        CBotStack* pile = nullptr; // stack for default expression

        if (useDefault || (ppVars == nullptr || ppVars[i] == nullptr))
        {
            assert(p->m_expr != nullptr);

            pile = pj->AddStack();
            useDefault = true;

            while (pile->IsOk() && !p->m_expr->Execute(pile));
            if (!pile->IsOk()) return pj->Return(pile);      // return the error

            pVar = pile->GetVar();
        }
        else
            pVar = ppVars[i];

        // serves to make the transformation of types:
        if ((useDefault && pVar != nullptr) ||
            (ppVars != nullptr && pVar != nullptr))
        {
            switch (p->m_type.GetType())
            {
            case CBotTypInt:
                newvar->SetValInt(pVar->GetValInt());
                newvar->SetInit(pVar->GetInit()); // copy nan
                break;
            case CBotTypFloat:
                newvar->SetValFloat(pVar->GetValFloat());
                newvar->SetInit(pVar->GetInit()); // copy nan
                break;
            case CBotTypString:
                newvar->SetValString(pVar->GetValString());
                break;
            case CBotTypBoolean:
                newvar->SetValInt(pVar->GetValInt());
                break;
            case CBotTypIntrinsic:
                (static_cast<CBotVarClass*>(newvar))->Copy(pVar, false);
                break;
            case CBotTypPointer:
                {
                    newvar->SetPointer(pVar->GetPointer());
                    newvar->SetType(p->m_type);     // keep pointer type
                }
                break;
            case CBotTypArrayPointer:
                {
                    newvar->SetPointer(pVar->GetPointer());
                }
                break;
            default:
                assert(0);
            }
        }
        newvar->SetUniqNum(p->m_nIdent);
        pj->AddVar(newvar);     // add a variable
        p = p->m_next;
        if (!useDefault) i++;
        if (pile != nullptr) pile->Delete();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotDefParam::HasDefault()
{
    return (m_expr != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
void CBotDefParam::RestoreState(CBotStack* &pj, bool bMain)
{
//    int             i = 0;
    CBotDefParam*   p = this;

    while ( p != nullptr )
    {
        // creates a local variable on the stack
        CBotVar*    var = pj->FindVar(p->m_token.GetString());
        var->SetUniqNum(p->m_nIdent);
        p = p->m_next;
    }
}

////////////////////////////////////////////////////////////////////////////////
int CBotDefParam::GetType()
{
    return  m_type.GetType();
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotDefParam::GetTypResult()
{
    return  m_type;
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotDefParam::GetParamString()
{
    std::string  param;

    param = m_typename;
    param += ' ';

    param += m_token.GetString();
    return param;
}

} // namespace CBot
