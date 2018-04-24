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

#include "CBot/CBotInstr/CBotInstr.h"

#include "CBot/CBotInstr/CBotBreak.h"
#include "CBot/CBotInstr/CBotDefArray.h"
#include "CBot/CBotInstr/CBotDefBoolean.h"
#include "CBot/CBotInstr/CBotDefClass.h"
#include "CBot/CBotInstr/CBotDefFloat.h"
#include "CBot/CBotInstr/CBotDefInt.h"
#include "CBot/CBotInstr/CBotDefString.h"
#include "CBot/CBotInstr/CBotDo.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotFor.h"
#include "CBot/CBotInstr/CBotIf.h"
#include "CBot/CBotInstr/CBotReturn.h"
#include "CBot/CBotInstr/CBotSwitch.h"
#include "CBot/CBotInstr/CBotThrow.h"
#include "CBot/CBotInstr/CBotTry.h"
#include "CBot/CBotInstr/CBotWhile.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotStack.h"

#include <cassert>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
int CBotInstr::m_LoopLvl = 0;
std::vector<std::string> CBotInstr::m_labelLvl = std::vector<std::string>();

////////////////////////////////////////////////////////////////////////////////
CBotInstr::CBotInstr()
{
    m_next   = nullptr;
    m_next2b = nullptr;
    m_next3  = nullptr;
    m_next3b = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr::~CBotInstr()
{
    delete m_next;
    delete m_next2b;
    delete m_next3;
    delete m_next3b;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::IncLvl(std::string& label)
{
    m_labelLvl.resize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = label;
    m_LoopLvl++;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::IncLvl()
{
    m_labelLvl.resize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = "#SWITCH";
    m_LoopLvl++;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::DecLvl()
{
    m_LoopLvl--;
    m_labelLvl[m_LoopLvl].clear();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::ChkLvl(const std::string& label, int type)
{
    int    i = m_LoopLvl;
    while (--i>=0)
    {
        if ( type == ID_CONTINUE && m_labelLvl[i] == "#SWITCH") continue;
        if (label.empty()) return true;
        if (m_labelLvl[i] == label) return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::SetToken(CBotToken* p)
{
    m_token = *p;
}

////////////////////////////////////////////////////////////////////////////////
int CBotInstr::GetTokenType()
{
    return m_token.GetType();
}

////////////////////////////////////////////////////////////////////////////////
CBotToken* CBotInstr::GetToken()
{
    return &m_token;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::AddNext(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next != nullptr) p = p->m_next;
    p->m_next = n;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::AddNext3(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next3 != nullptr) p = p->m_next3;
    p->m_next3 = n;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::AddNext3b(CBotInstr* n)
{
    CBotInstr*    p = this;
    while (p->m_next3b != nullptr) p = p->m_next3b;
    p->m_next3b = n;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstr::GetNext()
{
    return m_next;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstr::GetNext3()
{
    return m_next3;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstr::GetNext3b()
{
    return m_next3b;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*    pp = p;

    if (p == nullptr) return nullptr;

    int type = p->GetType(); // what is the next token?
    if (IsOfType(pp, TokenTypVar) && IsOfType(pp, ID_DOTS)) // is it a label?
    {
         type = pp->GetType();
         // Allow only instructions that accept a label
         if (!IsOfTypeList(pp, ID_WHILE, ID_FOR, ID_DO, 0))
         {
             pStack->SetError(CBotErrLabel, pp->GetStart());
             return nullptr;
         }
    }

    // Call Compile() function for the given token type
    switch (type)
    {
    case ID_WHILE:
        return CBotWhile::Compile(p, pStack);

    case ID_FOR:
        return CBotFor::Compile(p, pStack);

    case ID_DO:
        return CBotDo::Compile(p, pStack);

    case ID_BREAK:
    case ID_CONTINUE:
        return CBotBreak::Compile(p, pStack);

    case ID_SWITCH:
        return CBotSwitch::Compile(p, pStack);

    case ID_TRY:
        return CBotTry::Compile(p, pStack);

    case ID_THROW:
        return CBotThrow::Compile(p, pStack);

    case ID_INT:
        return CBotDefInt::Compile(p, pStack);

    case ID_FLOAT:
        return CBotDefFloat::Compile(p, pStack);

    case ID_STRING:
        return CBotDefString::Compile(p, pStack);

    case ID_BOOLEAN:
    case ID_BOOL:
        return CBotDefBoolean::Compile(p, pStack);

    case ID_IF:
        return CBotIf::Compile(p, pStack);

    case ID_RETURN:
        return CBotReturn::Compile(p, pStack);

    case ID_ELSE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(CBotErrElseWhitoutIf, p->GetEnd());
        return nullptr;

    case ID_CASE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(CBotErrCaseOut, p->GetEnd());
        return nullptr;
    }

    pStack->SetStartError(p->GetStart());

    // Should not be a reserved constant defined with DefineNum
    if (p->GetType() == TokenTypDef)
    {
        pStack->SetError(CBotErrReserved, p);
        return nullptr;
    }

    // If not, this might be an instance of class definnition
    CBotToken*    ppp = p;
    if (IsOfType(ppp, TokenTypVar))
    {
        if (CBotClass::Find(p) != nullptr) // Does class with this name exist?
        {
            // Yes, compile the declaration of the instance
            return CBotDefClass::Compile(p, pStack);
        }
    }

    // This can be an arithmetic expression
    CBotInstr* inst = CBotExpression::Compile(p, pStack);
    if (IsOfType(p, ID_SEP))
    {
        return inst;
    }
    pStack->SetError(CBotErrNoTerminator, p->GetStart());
    delete inst;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::Execute(CBotStack* &pj)
{
    assert(0);            // should never go through this routine
                            // but use the routines of the subclasses
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::Execute(CBotStack* &pj, CBotVar* pVar)
{
    if (!Execute(pj)) return false;
    pVar->SetVal(pj->GetVar());
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::RestoreState(CBotStack* &pj, bool bMain)
{
    assert(0);            // should never go through this routine
                           // but use the routines of the subclasses
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    assert(0);            // dad do not know, see the girls
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend)
{
    assert(0);            // dad do not know, see the girls
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::RestoreStateVar(CBotStack* &pile, bool bMain)
{
    assert(0);            // dad do not know, see the girls
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::CompCase(CBotStack* &pj, int val)
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotInstr::CompileArray(CBotToken* &p, CBotCStack* pStack, CBotTypResult type, bool first)
{
    if (IsOfType(p, ID_OPBRK))
    {
        if (!IsOfType(p, ID_CLBRK))
        {
            pStack->SetError(CBotErrCloseIndex, p->GetStart());
            return nullptr;
        }

        CBotInstr*    inst = CompileArray(p, pStack, CBotTypResult(CBotTypArrayPointer, type), false);
        if (inst != nullptr || !pStack->IsOk()) return inst;
    }

    // compiles an array declaration
    if (first) return nullptr ;

    CBotInstr* inst = CBotDefArray::Compile(p, pStack, type);
    if (inst == nullptr) return nullptr;

    if (IsOfType(p,  ID_COMMA)) // several definitions
    {
        if (nullptr != ( inst->m_next2b = CBotDefArray::CompileArray(p, pStack, type, false)))    // compiles next one
        {
            return inst;
        }
        delete inst;
        return nullptr;
    }

    if (IsOfType(p,  ID_SEP))                            // end of instruction
    {
        return inst;
    }

    delete inst;
    pStack->SetError(CBotErrNoTerminator, p->GetStart());
    return nullptr;
}

bool CBotInstr::HasReturn()
{
    if (m_next != nullptr) return m_next->HasReturn();
    return false; // end of the list
}

std::map<std::string, CBotInstr*> CBotInstr::GetDebugLinks()
{
    return {
        {"m_next", m_next},
        {"m_next2b", m_next2b},
        {"m_next3", m_next3},
        {"m_next3b", m_next3b}
    };
}

} // namespace CBot
