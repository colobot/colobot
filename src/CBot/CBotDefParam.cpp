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
}

////////////////////////////////////////////////////////////////////////////////
CBotDefParam::~CBotDefParam()
{
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

        while (!IsOfType(p, ID_CLOSEPAR))
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

                    if ( type.Eq(CBotTypArrayPointer) ) type.SetType(CBotTypArrayBody);
                    CBotVar*    var = CBotVar::Create(pp->GetString(), type);       // creates the variable
//                  if ( pClass ) var->SetClass(pClass);
                    var->SetInit(CBotVar::InitType::IS_POINTER);                                    // mark initialized
                    param->m_nIdent = CBotVar::NextUniqNum();
                    var->SetUniqNum(param->m_nIdent);
                    pStack->AddVar(var);                                // place on the stack

                    if (IsOfType(p, ID_COMMA) || p->GetType() == ID_CLOSEPAR)
                        continue;
                }
                pStack->SetError(CBotErrClosePar, p->GetStart());
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
    CBotDefParam*   p = this;

    while ( p != nullptr )
    {
        // creates a local variable on the stack
        CBotVar*    newvar = CBotVar::Create(p->m_token.GetString(), p->m_type);

        // serves to make the transformation of types:
        if ( ppVars != nullptr && ppVars[i] != nullptr )
        {
            switch (p->m_type.GetType())
            {
            case CBotTypInt:
                newvar->SetValInt(ppVars[i]->GetValInt());
                break;
            case CBotTypFloat:
                newvar->SetValFloat(ppVars[i]->GetValFloat());
                break;
            case CBotTypString:
                newvar->SetValString(ppVars[i]->GetValString());
                break;
            case CBotTypBoolean:
                newvar->SetValInt(ppVars[i]->GetValInt());
                break;
            case CBotTypIntrinsic:
                (static_cast<CBotVarClass*>(newvar))->Copy(ppVars[i], false);
                break;
            case CBotTypPointer:
            case CBotTypArrayPointer:
                {
                    newvar->SetPointer(ppVars[i]->GetPointer());
                }
                break;
            default:
                assert(0);
            }
        }
        newvar->SetUniqNum(p->m_nIdent);
        pj->AddVar(newvar);     // add a variable
        p = p->m_next;
        i++;
    }

    return true;
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
