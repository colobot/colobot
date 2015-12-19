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
#include "CBot/CBotInstr/CBotInstr.h"

#include "CBot/CBotInstr/CBotFor.h"
#include "CBot/CBotInstr/CBotDo.h"
#include "CBot/CBotInstr/CBotBreak.h"
#include "CBot/CBotInstr/CBotSwitch.h"
#include "CBot/CBotInstr/CBotTry.h"
#include "CBot/CBotInstr/CBotThrow.h"
#include "CBot/CBotInstr/CBotInt.h"
#include "CBot/CBotInstr/CBotFloat.h"
#include "CBot/CBotInstr/CBotWhile.h"
#include "CBot/CBotInstr/CBotIString.h"
#include "CBot/CBotInstr/CBotBoolean.h"
#include "CBot/CBotInstr/CBotIf.h"
#include "CBot/CBotInstr/CBotReturn.h"
#include "CBot/CBotInstr/CBotClassInst.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotInstArray.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotStack.h"


// Local include

// Global include
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
int CBotInstr::m_LoopLvl = 0;
CBotStringArray CBotInstr::m_labelLvl = CBotStringArray();

////////////////////////////////////////////////////////////////////////////////
CBotInstr::CBotInstr()
{
    name     = "CBotInstr";
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
void CBotInstr::IncLvl(CBotString& label)
{
    m_labelLvl.SetSize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = label;
    m_LoopLvl++;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::IncLvl()
{
    m_labelLvl.SetSize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = "#SWITCH";
    m_LoopLvl++;
}

////////////////////////////////////////////////////////////////////////////////
void CBotInstr::DecLvl()
{
    m_LoopLvl--;
    m_labelLvl[m_LoopLvl].Empty();
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::ChkLvl(const CBotString& label, int type)
{
    int    i = m_LoopLvl;
    while (--i>=0)
    {
        if ( type == ID_CONTINUE && m_labelLvl[i] == "#SWITCH") continue;
        if (label.IsEmpty()) return true;
        if (m_labelLvl[i] == label) return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::IsOfClass(CBotString n)
{
    return name == n;
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

    int type = p->GetType();            // what is the next token

    // is it a lable?
    if (IsOfType(pp, TokenTypVar) &&
         IsOfType(pp, ID_DOTS))
    {
         type = pp->GetType();
         // these instructions accept only lable
         if (!IsOfTypeList(pp, ID_WHILE, ID_FOR, ID_DO, 0))
         {
             pStack->SetError(TX_LABEL, pp->GetStart());
             return nullptr;
         }
    }

    // call routine corresponding to the compilation token found
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
        return CBotInt::Compile(p, pStack);

    case ID_FLOAT:
        return CBotFloat::Compile(p, pStack);

    case ID_STRING:
        return CBotIString::Compile(p, pStack);

    case ID_BOOLEAN:
    case ID_BOOL:
        return CBotBoolean::Compile(p, pStack);

    case ID_IF:
        return CBotIf::Compile(p, pStack);

    case ID_RETURN:
        return CBotReturn::Compile(p, pStack);

    case ID_ELSE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(TX_ELSEWITHOUTIF, p->GetEnd());
        return nullptr;

    case ID_CASE:
        pStack->SetStartError(p->GetStart());
        pStack->SetError(TX_OUTCASE, p->GetEnd());
        return nullptr;
    }

    pStack->SetStartError(p->GetStart());

    // should not be a reserved word DefineNum
    if (p->GetType() == TokenTypDef)
    {
        pStack->SetError(TX_RESERVED, p);
        return nullptr;
    }

    // this might be an instance of class definnition
    CBotToken*    ppp = p;
    if (IsOfType(ppp, TokenTypVar))
    {
        if (CBotClass::Find(p) != nullptr)
        {
            // yes, compiles the declaration of the instance
            return CBotClassInst::Compile(p, pStack);
        }
    }

    // this can be an arythmetic instruction
    CBotInstr*    inst = CBotExpression::Compile(p, pStack);
    if (IsOfType(p, ID_SEP))
    {
        return inst;
    }
    pStack->SetError(TX_ENDOF, p->GetStart());
    delete inst;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotInstr::Execute(CBotStack* &pj)
{
    CBotString    ClassManquante = name;
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
    CBotString    ClassManquante = name;
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
            pStack->SetError(TX_CLBRK, p->GetStart());
            return nullptr;
        }

        CBotInstr*    inst = CompileArray(p, pStack, CBotTypResult(CBotTypArrayPointer, type), false);
        if (inst != nullptr || !pStack->IsOk()) return inst;
    }

    // compiles an array declaration
    if (first) return nullptr ;

    CBotInstr* inst = CBotInstArray::Compile(p, pStack, type);
    if (inst == nullptr) return nullptr;

    if (IsOfType(p,  ID_COMMA)) // several definitions
    {
        if (nullptr != ( inst->m_next2b = CBotInstArray::CompileArray(p, pStack, type, false)))    // compiles next one
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
    pStack->SetError(TX_ENDOF, p->GetStart());
    return nullptr;
}
