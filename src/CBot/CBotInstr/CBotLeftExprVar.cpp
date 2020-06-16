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

#include "CBot/CBotInstr/CBotLeftExprVar.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <cassert>
#include <sstream>

namespace CBot
{

CBotLeftExprVar::CBotLeftExprVar()
{
}

CBotLeftExprVar::~CBotLeftExprVar()
{
}

CBotInstr* CBotLeftExprVar::Compile(CBotToken* &p, CBotCStack* pStack)
{
    // Verifies that the token is a variable name
    if (p->GetType() != TokenTypVar)
    {
        pStack->SetError(CBotErrNoVar, p);
        return nullptr;
    }

    CBotLeftExprVar* inst = new CBotLeftExprVar();
    inst->SetToken(p);
    p = p->GetNext();

    return inst;
}

bool CBotLeftExprVar::Execute(CBotStack* &pj)
{
    // Create the variable
    CBotVar* var1 = CBotVar::Create(m_token.GetString(), m_typevar);
    var1->SetUniqNum(m_nIdent);
    pj->AddVar(var1);

    CBotVar* var2 = pj->GetVar(); // Initial value on the stack
    if (var2 != nullptr)
    {
        if (m_typevar.Eq(CBotTypString) && var2->GetType() != CBotTypString)
        {
            var2->Update(pj->GetUserPtr());
            var1->SetValString(var2->GetValString());
            return true;
        }
        var1->SetVal(var2); // Set the value
    }

    return true;
}

void CBotLeftExprVar::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotVar*     var1;

    var1 = pj->FindVar(m_token.GetString());
    if (var1 == nullptr) assert(false);

    var1->SetUniqNum(m_nIdent); // Restore the identifier
}

std::string CBotLeftExprVar::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << std::endl;
    //ss << "VarID = " << m_nIdent << std::endl;
    ss << "type = " << m_typevar.ToString();
    return ss.str();
}

} // namespace CBot
