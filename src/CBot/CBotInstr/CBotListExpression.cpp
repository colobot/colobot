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

#include "CBot/CBotInstr/CBotDefBoolean.h"
#include "CBot/CBotInstr/CBotDefFloat.h"
#include "CBot/CBotInstr/CBotDefInt.h"
#include "CBot/CBotInstr/CBotDefString.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotListExpression.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
/// Seeks a declaration of variable or expression
static CBotInstr* CompileInstrOrDefVar(CBotToken* &p, CBotCStack* pStack)
{
    CBotInstr*  i = CBotDefInt::Compile(p, pStack, false, true );         // Is this a declaration of an integer?
    if ( i== nullptr ) i = CBotDefFloat::Compile(p, pStack, false, true );   // or a real number?
    if ( i== nullptr ) i = CBotDefBoolean::Compile(p, pStack, false, true ); // or a boolean?
    if ( i== nullptr ) i = CBotDefString::Compile(p, pStack, false, true ); // ar a string?
    if ( i== nullptr ) i = CBotExpression::Compile( p, pStack );           // compiles an expression
    return i;
}
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
CBotListExpression::CBotListExpression()
{
    m_expr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotListExpression::~CBotListExpression()
{
    delete m_expr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotListExpression::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotListExpression* inst = new CBotListExpression();

    inst->m_expr = CompileInstrOrDefVar(p, pStack );           // compile the first expression in a list
    if (pStack->IsOk())
    {
        while ( IsOfType(p, ID_COMMA) )                         // more instructions?
        {
            CBotInstr*  i = CompileInstrOrDefVar( p, pStack );      // Is this a declaration of an integer?
            inst->m_expr->AddNext(i);                           // added after
            if ( !pStack->IsOk() )
            {
                delete inst;
                return nullptr;                                    // no object, the error is on the stack
            }
        }
        return inst;
    }
    delete inst;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotListExpression::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack();                          // essential
    CBotInstr*  p = m_expr;                                     // the first expression

    int     state = pile->GetState();
    while (state-->0) p = p->GetNext();                         // returns to the interrupted operation

    if ( p != nullptr ) while (true)
    {
        if ( !p->Execute(pile) ) return false;
        p = p->GetNext();
        if ( p == nullptr ) break;
        if (!pile->IncState()) return false;                    // ready for next
    }
    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotListExpression::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotStack*  pile = pj;
    int     state = 0x7000;

    if ( bMain )
    {
        pile = pj->RestoreStack();
        if ( pile == nullptr ) return;
        state = pile->GetState();
    }

    CBotInstr*  p = m_expr;                                     // the first expression

    while (p != nullptr && state-->0)
    {
        p->RestoreState(pile, false);
        p = p->GetNext();                           // returns to the interrupted operation
    }

    if ( p != nullptr )
    {
        p->RestoreState(pile, bMain);
    }
}

std::map<std::string, CBotInstr*> CBotListExpression::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_expr"] = m_expr;
    return links;
}

} // namespace CBot
