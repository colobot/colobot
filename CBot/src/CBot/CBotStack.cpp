/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotStack.h"

#include "CBot/CBotClass.h"

#include "CBot/CBotInstr/CBotFunction.h"

#include "CBot/CBotVar/CBotVarPointer.h"
#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotUtils.h"
#include "CBot/CBotExternalCall.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>


namespace CBot
{

const int DEFAULT_TIMER = 100;

struct CBotStack::Data
{
    int          initimer   = DEFAULT_TIMER;
    int          timer      = 0;

    CBotError    error      = CBotNoErr;
    int          errStart   = 0;
    int          errEnd     = 0;

    std::string  labelBreak = "";

    CBotProgram* baseProg   = nullptr;
    CBotStack*   topStack   = nullptr;
    void*        pUser      = nullptr;

    std::unique_ptr<CBotVar> retvar;
};

CBotStack* CBotStack::AllocateStack()
{
    CBotStack*    p;

    long    size = sizeof(CBotStack);
    size    *= (MAXSTACK+10);

    // request a slice of memory for the stack
    p = static_cast<CBotStack*>(malloc(size));

    // completely empty
    memset(p, 0, size);

    p->m_block = BlockVisibilityType::BLOCK;

    CBotStack* pp = p;
    pp += MAXSTACK;
    int i;
    for ( i = 0 ; i< 10 ; i++ )
    {
        pp->m_bOver = true;
        pp ++;
    }

    p->m_data = new CBotStack::Data;
    p->m_data->topStack = p;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::Delete()
{
    if (m_next != nullptr) m_next->Delete();
    if (m_next2 != nullptr) m_next2->Delete();

    if (m_prev != nullptr)
    {
        if ( m_prev->m_next == this )
            m_prev->m_next = nullptr;        // removes chain

        if ( m_prev->m_next2 == this )
            m_prev->m_next2 = nullptr;        // removes chain
    }

    delete m_var;
    delete m_listVar;

    CBotStack*    p = m_prev;
    bool        bOver = m_bOver;
    if ( m_prev == nullptr ) delete m_data;

    // clears the freed block
    memset(this, 0, sizeof(CBotStack));
    m_bOver    = bOver;

    if ( p == nullptr )
        free( this );
}

// routine improved
////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStack(CBotInstr* instr, BlockVisibilityType bBlock)
{
    if (m_next != nullptr)
    {
        return m_next;                // included in an existing stack
    }

    CBotStack*    p = this;
    do
    {
        p ++;
    }
    while ( p->m_prev != nullptr );

    m_next = p;                                    // chain an element
    p->m_data   = m_data;
    p->m_block  = bBlock;
    p->m_instr  = instr;
    p->m_prog   = m_prog;
    p->m_step   = 0;
    p->m_prev   = this;
    p->m_state  = 0;
    p->m_call   = nullptr;
    p->m_func   = IsFunction::NO;
    p->m_callFinished = false;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStackExternalCall(CBotExternalCall* instr, BlockVisibilityType bBlock)
{
    assert(!m_callFinished);
    if (m_next != nullptr)
    {
        return m_next;                // included in an existing stack
    }
    CBotStack*    p = AddStack(nullptr, bBlock);
    p->m_call = instr;
    p->m_func = IsFunction::EXTERNAL_CALL;
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStack2(BlockVisibilityType bBlock)
{
    if (m_next2 != nullptr)
    {
        m_next2->m_prog = m_prog;        // special avoids RestoreStack2
        return m_next2;                    // included in an existing stack
    }

    CBotStack*    p = this;
    do
    {
        p ++;
    }
    while ( p->m_prev != nullptr );

    m_next2 = p;                                // chain an element
    p->m_data = m_data;
    p->m_prev = this;
    p->m_block = bBlock;
    p->m_prog = m_prog;
    p->m_step = 0;
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack::BlockVisibilityType CBotStack::GetBlock()
{
    return m_block;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != nullptr) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = nullptr;                        // not to destroy the variable

    if (m_next != nullptr)
    {
        // releases the stack above
        m_next->Delete();
        m_next = nullptr;
    }
    if (m_next2 != nullptr)
    {
        // also the second stack (catch)
        m_next2->Delete();
        m_next2 = nullptr;
    }

    return IsOk();                        // interrupted if error
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::ReturnKeep(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != nullptr) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = nullptr;                        // not to destroy the variable

    return IsOk();                        // interrupted if error
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::StackOver()
{
    if (!m_bOver) return false;
    m_data->error = CBotErrStackOver;
    return true;
}

CBotError CBotStack::GetError(int& start, int& end)
{
    start = m_data->errStart;
    end = m_data->errEnd;
    return m_data->error;
}

CBotError CBotStack::GetError()
{
    return m_data->error;
}

bool CBotStack::IsOk()
{
    return m_data->error == CBotNoErr;
}

void CBotStack::Reset()
{
    m_data->timer = m_data->initimer; // resets the timer
    m_data->error = CBotNoErr;
    m_data->labelBreak.clear();
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::RestoreStack(CBotInstr* instr)
{
    if (m_next != nullptr)
    {
        m_next->m_instr = instr;    // reset (if recovery after )
        m_next->m_prog = m_prog;
        return m_next;                // included in an existing stack
    }
    return    nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::RestoreStackEOX(CBotExternalCall* instr)
{
    CBotStack*     p = RestoreStack();
    p->m_call = instr;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
// routine for execution step by step
bool CBotStack::IfStep()
{
    if (m_data->initimer > 0 || m_step++ > 0) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::BreakReturn(CBotStack* pfils, const std::string& name)
{
    if (m_data->error >= CBotNoErr) return false;  // normal output
    if (m_data->error == CBotError(-3)) return false;      // normal output (return current)

    if (!m_data->labelBreak.empty() && (name.empty() || m_data->labelBreak != name))
        return false;                            // it's not for me

    m_data->error = CBotNoErr;
    m_data->labelBreak.clear();
    return Return(pfils);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::IfContinue(int state, const std::string& name)
{
    if (m_data->error != CBotError(-2)) return false;

    if (!m_data->labelBreak.empty() && (name.empty() || m_data->labelBreak != name))
        return false;                            // it's not for me

    m_state = state;                            // where again?
    m_data->error = CBotNoErr;
    m_data->labelBreak.clear();
    if (m_next != nullptr) m_next->Delete();            // purge above stack
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetBreak(int val, const std::string& name)
{
    m_data->error = static_cast<CBotError>(-val); // reacts as an Exception
    m_data->labelBreak = name;
    if (val == 3)    // for a return
    {
        m_data->retvar.reset(m_var);
        m_var = nullptr;
    }
}

// gives on the stack value calculated by the last CBotReturn
////////////////////////////////////////////////////////////////////////////////
bool CBotStack::GetRetVar(bool bRet)
{
    if (m_data->error == CBotError(-3))
    {
        if ( m_var ) delete m_var;
        m_var         = m_data->retvar.release();
        m_data->error = CBotNoErr;
        return        true;
    }
    return bRet;                        // interrupted by something other than return
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(CBotToken*& pToken, bool bUpdate)
{
    CBotStack*    p = this;
    const auto&   name = pToken->GetString();

    while (p != nullptr)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != nullptr)
        {
            if (pp->GetName() == name)
            {
                if ( bUpdate )
                    pp->Update(m_data->pUser);

                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(const std::string& name)
{
    CBotStack*    p = this;
    while (p != nullptr)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != nullptr)
        {
            if (pp->GetName() == name)
            {
                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(long ident, bool bUpdate)
{
    CBotStack*    p = this;
    while (p != nullptr)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != nullptr)
        {
            if (pp->GetUniqNum() == ident)
            {
                if ( bUpdate )
                    pp->Update(m_data->pUser);

                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(CBotToken& pToken, bool bUpdate)
{
    CBotToken*    pt = &pToken;
    return FindVar(pt, bUpdate);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::CopyVar(CBotToken& pToken, bool bUpdate)
{
    CBotVar*    pVar = FindVar(pToken, bUpdate);

    if ( pVar == nullptr) return nullptr;

    CBotVar*    pCopy = CBotVar::Create(pVar);
    pCopy->Copy(pVar);
    return    pCopy;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::SetState(int n, int limite)
{
    m_state = n;

    m_data->timer--;                              // decrement the timer
    return (m_data->timer > limite);                // interrupted if timer pass
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::IncState(int limite)
{
    m_state++;

    m_data->timer--;                              // decrement the timer
    return (m_data->timer > limite);                // interrupted if timer pass
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetError(CBotError n, CBotToken* token)
{
    if (n != CBotNoErr && m_data->error != CBotNoErr) return; // does not change existing error
    m_data->error = n;
    if (token != nullptr)
    {
        m_data->errStart = token->GetStart();
        m_data->errEnd = token->GetEnd();
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::ResetError(CBotError n, int start, int end)
{
    m_data->error = n;
    m_data->errStart = start;
    m_data->errEnd = end;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetPosError(CBotToken* token)
{
    m_data->errStart = token->GetStart();
    m_data->errEnd = token->GetEnd();
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetTimer(int n)
{
    m_data->initimer = n;
}

int CBotStack::GetTimer()
{
    return m_data->initimer;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::Execute()
{
    CBotExternalCall*        instr = nullptr;                        // the most highest instruction
    CBotStack*        pile;

    CBotStack*        p = this;

    while (p != nullptr)
    {
        if ( p->m_next2 != nullptr ) break;
        if ( p->m_call != nullptr )
        {
            instr = p->m_call;
            pile  = p->m_prev ;
        }
        p = p->m_next;
    }

    if ( instr == nullptr ) return true;                // normal execution request

    if (!instr->Run(nullptr, pile)) return false;            // resume interrupted execution

    if (pile->m_next != nullptr) pile->m_next->Delete();

    pile->m_callFinished = true;
    return true;
}

// puts on the stack pointer to a variable
////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetVar( CBotVar* var )
{
    if (m_var) delete m_var;    // replacement of a variable
    m_var = var;
}

// puts on the stack a copy of a variable
////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetCopyVar( CBotVar* var )
{
    if (m_var) delete m_var;    // replacement of a variable

    m_var = CBotVar::Create("", var->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC));
    m_var->Copy( var );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::GetVar()
{
    return m_var;
}

////////////////////////////////////////////////////////////////////////////////
long CBotStack::GetVal()
{
    if (m_var == nullptr) return 0;
    return m_var->GetValInt();
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::AddVar(CBotVar* pVar)
{
    CBotStack*    p = this;

    // returns to the father element
    while (p != nullptr && p->m_block == BlockVisibilityType::INSTRUCTION) p = p->m_prev;

    if ( p == nullptr ) return;

    CBotVar**    pp = &p->m_listVar;
    while ( *pp != nullptr ) pp = &(*pp)->m_next;

    *pp = pVar;                    // added after
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetProgram(CBotProgram* p)
{
    m_prog = p;
    m_func = IsFunction::YES;
    if (this == m_data->topStack) m_data->baseProg = p;
}

////////////////////////////////////////////////////////////////////////////////
CBotProgram*  CBotStack::GetProgram(bool bFirst)
{
    if ( ! bFirst )    return m_prog;
    return m_data->baseProg;
}

////////////////////////////////////////////////////////////////////////////////
void* CBotStack::GetUserPtr()
{
    return m_data->pUser;
}

void CBotStack::SetUserPtr(void* user)
{
    m_data->pUser = user;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, const CBotTypResult& rettype)
{
    int res;

    // first looks by the identifier

    res = CBotFunction::DoCall(m_prog, m_prog->GetFunctions(), nIdent, "", ppVar, this, token);
    if (res >= 0) return res;

    // if not found (recompile?) seeks by name

    nIdent = 0;
    res = m_prog->GetExternalCalls()->DoCall(token, nullptr, ppVar, this, rettype);
    if (res >= 0) return res;

    res = CBotFunction::DoCall(m_prog, m_prog->GetFunctions(), nIdent, token->GetString(), ppVar, this, token);
    if (res >= 0) return res;

    SetError(CBotErrUndefFunc, token);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar)
{
    if (m_next == nullptr) return;

    if (m_prog->GetExternalCalls()->RestoreCall(token, nullptr, ppVar, this))
        return;

    CBotFunction::RestoreCall(m_prog->GetFunctions(), nIdent, token->GetString(), ppVar, this);
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::GetRunPos(std::string& functionName, int& start, int& end)
{
    CBotProgram*    prog = m_prog;                        // Current program

    CBotInstr*        funct = nullptr;                        // function found
    CBotInstr*        instr = nullptr;                        // the highest intruction

    CBotStack*        p = this;

    while (p->m_next != nullptr)
    {
        if ( p->m_instr != nullptr ) instr = p->m_instr;
        if (p->m_func == IsFunction::YES && p->m_instr != nullptr ) funct = p->m_instr;
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }

    if ( p->m_instr != nullptr ) instr = p->m_instr;
    if (p->m_func == IsFunction::YES && p->m_instr != nullptr ) funct = p->m_instr;

    if ( funct == nullptr ) return;

    CBotToken* t = funct->GetToken();
    functionName = t->GetString();

//    if ( p->m_instr != nullptr ) instr = p->m_instr;

    t = instr->GetToken();
    start = t->GetStart();
    end   = t->GetEnd();
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::GetStackVars(std::string& functionName, int level)
{
    CBotProgram*    prog = m_prog;                        // current program
    functionName = "";

    // back the stack in the current module
    CBotStack*        p = this;

    while (p->m_next != nullptr)
    {
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }
    // Now p is the highest element on the stack


    // descends upon the elements of block
    while ( p != nullptr && p->m_block == BlockVisibilityType::INSTRUCTION) p = p->m_prev;
    // Now p is on the beggining of the top block (with local variables)

    while ( p != nullptr && level++ < 0 )
    {
        p = p->m_prev;
        while ( p != nullptr && p->m_block == BlockVisibilityType::INSTRUCTION) p = p->m_prev;
    }
    // Now p is on the block "level"

    if ( p == nullptr ) return nullptr;

    // search the name of the current function
    CBotStack* pp = p;
    while ( pp != nullptr )
    {
        if (pp->m_func == IsFunction::YES) break;
        pp = pp->m_prev;
    }

    if ( pp == nullptr || pp->m_instr == nullptr ) return nullptr;

    CBotToken* t = pp->m_instr->GetToken();
    functionName = t->GetString();

    return p->m_listVar;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::SaveState(std::ostream &ostr)
{
    if (m_next2 != nullptr)
    {
        if (!WriteWord(ostr, 2)) return false; // a marker of type (m_next2)
        if (!m_next2->SaveState(ostr)) return false; // saves the next element
    }
    else
    {
        if (!WriteWord(ostr, 1)) return false; // a marker of type (m_next)
    }
    if (!WriteWord(ostr, static_cast<unsigned short>(m_block))) return false;
    if (!WriteInt(ostr, m_state)) return false;
    if (!WriteWord(ostr, 0)) return false; // for backwards combatibility (m_bDontDelete)
    if (!WriteInt(ostr, m_step)) return false;

    if (!SaveVars(ostr, m_var)) return false;          // current result
    if (!SaveVars(ostr, m_listVar)) return false;      // local variables

    if (m_next != nullptr)
    {
        if (!m_next->SaveState(ostr)) return false; // saves the next element
    }
    else
    {
        if (!WriteWord(ostr, 0)) return false; // 0 - CBotStack::SaveState terminator
    }
    return true;
}

bool SaveVars(std::ostream &ostr, CBotVar* pVar)
{
    while (pVar != nullptr)
    {
        if (!pVar->Save0State(ostr)) return false; // common header
        if (!pVar->Save1State(ostr)) return false; // saves the data

        pVar = pVar->GetNext();
    }
    return WriteWord(ostr, 0); // 0 - CBot::SaveVars terminator
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::RestoreState(std::istream &istr, CBotStack* &pStack)
{
    unsigned short w;

    if (pStack != this) pStack = nullptr;
    if (!ReadWord(istr, w)) return false;
    if ( w == 0 ) return true; // 0 - CBotStack::SaveState terminator

    if (pStack == nullptr) pStack = AddStack();

    if ( w == 2 ) // 2 - m_next2
    {
        if (!pStack->RestoreState(istr, pStack->m_next2)) return false;
    }

    if (!ReadWord(istr, w)) return false;
    pStack->m_block = static_cast<BlockVisibilityType>(w);

    int state;
    if (!ReadInt(istr, state)) return false;
    pStack->SetState(state);

    if (!ReadWord(istr, w)) return false; // backwards compatibility (m_bDontDelete)

    if (!ReadInt(istr, state)) return false;
    pStack->m_step = state;

    if (!CBotVar::RestoreState(istr, pStack->m_var)) return false;     // temp variable
    if (!CBotVar::RestoreState(istr, pStack->m_listVar)) return false; // local variables

    return pStack->RestoreState(istr, pStack->m_next);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Save0State(std::ostream &ostr)
{
    if (!WriteWord(ostr, 100+static_cast<int>(m_mPrivate)))return false; // private variable?
    if (!WriteWord(ostr, m_bStatic))return false;            // static variable?
    if (!WriteWord(ostr, m_type.GetType()))return false;     // saves the type (always non-zero)

    if (m_type.Eq(CBotTypPointer) && GetPointer() != nullptr)
    {
        if (GetPointer()->m_bConstructor)                    // constructor was called?
        {
            if (!WriteWord(ostr, (2000 + static_cast<unsigned short>(m_binit)) )) return false;
            return WriteString(ostr, m_token->GetString());  // and variable name
        }
    }

    if (!WriteWord(ostr, static_cast<unsigned short>(m_binit))) return false;        // variable defined?
    return WriteString(ostr, m_token->GetString());          // and variable name
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::RestoreState(std::istream &istr, CBotVar* &pVar)
{
    unsigned short        w, wi, prv, st;

    delete pVar;

                pVar    = nullptr;
    CBotVar*    pNew    = nullptr;
    CBotVar*    pPrev    = nullptr;

    while ( true )            // retrieves a list
    {
        if (!ReadWord(istr, w)) return false;                      // private or type?
        if ( w == 0 ) return true; // 0 - CBot::SaveVars terminator

        std::string defnum;
        if ( w == 200 )
        {
            if (!ReadString(istr, defnum)) return false;          // number with identifier
            if (!ReadWord(istr, w)) return false;                 // type
        }

        prv = 100; st = 0;
        if ( w >= 100 )
        {
            prv = w;
            if (!ReadWord(istr, st)) return false;              // static
            if (!ReadWord(istr, w)) return false;               // type
        }

        if ( w == CBotTypClass ) w = CBotTypIntrinsic;            // necessarily intrinsic

        if (!ReadWord(istr, wi)) return false;                  // init ?
        bool bConstructor = false;
        if (w == CBotTypPointer && wi >= 2000)
        {
            wi -= 2000;
            bConstructor = true;
        }

        CBotVar::InitType initType = static_cast<CBotVar::InitType>(wi);
        std::string varname;
        if (!ReadString(istr, varname)) return false;           // variable name
        CBotToken token(varname, std::string());

        bool isClass = false;

        switch (w)
        {
        case CBotTypBoolean:
            char valBool;
            if (!ReadByte(istr, valBool)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValInt(valBool);
            break;
        case CBotTypByte:
            char valByte;
            if (!ReadByte(istr, valByte)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValByte(valByte);
            break;
        case CBotTypShort:
            short valShort;
            if (!ReadShort(istr, valShort)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValShort(valShort);
            break;
        case CBotTypChar:
            uint32_t valChar;
            if (!ReadUInt32(istr, valChar)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValChar(valChar);
            break;
        case CBotTypInt:
            int valInt;
            if (!ReadInt(istr, valInt)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValInt(valInt, defnum);
            break;
        case CBotTypLong:
            long valLong;
            if (!ReadLong(istr, valLong)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValInt(valLong);
            break;
        case CBotTypFloat:
            float valFloat;
            if (!ReadFloat(istr, valFloat)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValFloat(valFloat);
            break;
        case CBotTypDouble:
            double valDouble;
            if (!ReadDouble(istr, valDouble)) return false;
            pNew = CBotVar::Create(token, w);                        // creates a variable
            pNew->SetValDouble(valDouble);
            break;
        case CBotTypString:
            {
                std::string valString;
                if (!ReadString(istr, valString)) return false;
                pNew = CBotVar::Create(token, w);                    // creates a variable
                pNew->SetValString(valString);
                break;
            }
        // returns an intrinsic object or element of an array
        case CBotTypIntrinsic:
            isClass = true;
        case CBotTypArrayBody:
            {
                CBotTypResult    r;
                long            id;
                if (!ReadType(istr, r)) return false;               // complete type
                if (!ReadLong(istr, id)) return false;

                {
                    CBotVar* p = nullptr;
                    if ( id ) p = CBotVarClass::Find(id) ;

                    pNew = new CBotVarClass(token, r);                // directly creates an instance
                                                                    // attention cptuse = 0
                    if (!RestoreState(istr, (static_cast<CBotVarClass*>(pNew))->m_pVar)) return false;
                    pNew->SetIdent(id);

                    if (isClass && p == nullptr) // set id for each item in this instance
                    {
                        CBotClass* pClass = pNew->GetClass();
                        CBotVar* pVars = (static_cast<CBotVarClass*>(pNew))->m_pVar;
                        while (pClass != nullptr && pVars != nullptr)
                        {
                            CBotVar* pv = pClass->GetVar();
                            while (pVars != nullptr && pv != nullptr)
                            {
                                pVars->m_ident = pv->m_ident;
                                pVars = pVars->m_next;
                                pv = pv->m_next;
                            }
                            pClass = pClass->GetParent();
                        }
                    }

                    if ( p != nullptr )
                    {
                        delete pNew;
                        pNew = p;            // resume known element
                    }
                }
            }
            break;

        case CBotTypPointer:
        case CBotTypNullPointer:
        {
            std::string className;
            if (!ReadString(istr, className)) return false; // name of the class
            {
//                CBotVarClass* p = nullptr;
                long id;
                if (!ReadLong(istr, id)) return false;
//                if ( id ) p = CBotVarClass::Find(id);        // found the instance (made by RestoreInstance)

                CBotTypResult ptrType(w, className);
                pNew = CBotVar::Create(token, ptrType);        // creates a variable
                // returns a copy of the original instance
                CBotVar* pInstance = nullptr;
                if (!CBotVar::RestoreState(istr, pInstance)) return false;
                (static_cast<CBotVarPointer*>(pNew))->SetPointer( pInstance );            // and point over

                if (bConstructor) pNew->ConstructorSet(); // constructor was called
                if (ptrType.Eq(CBotTypPointer)) pNew->SetType(ptrType); // keep pointer type

//                if ( p != nullptr ) (static_cast<CBotVarPointer*>(pNew))->SetPointer( p );    // rather this one

            }
            break;
        }
        case CBotTypArrayPointer:
            {
                CBotTypResult    r;
                if (!ReadType(istr, r)) return false;

                pNew = CBotVar::Create(token, r);                        // creates a variable

                // returns a copy of the original instance
                CBotVar* pInstance = nullptr;
                if (!CBotVar::RestoreState(istr, pInstance)) return false;
                (static_cast<CBotVarPointer*>(pNew))->SetPointer( pInstance );            // and point over
            }
            break;
        default:
            return false; // signal error
        }

        if ( pPrev != nullptr ) pPrev->m_next = pNew;
        if ( pVar == nullptr  ) pVar = pNew;

        pNew->m_binit = initType;        //        pNew->SetInit(wi);
        pNew->SetStatic(st);
        pNew->SetPrivate(static_cast<ProtectionLevel>(prv-100));
        pPrev = pNew;
    }
    return true;
}

bool CBotStack::IsCallFinished()
{
    return m_callFinished;
}

void CBotStack::CancelExternal()
{
    if (m_next != nullptr) m_next->CancelExternal();
    if (m_next2 != nullptr) m_next2->CancelExternal();
    if (m_call != nullptr)
    {
        m_call->Cancel(this);
        // Prevent cancellation from being repeated
        Delete();
    }
}

} // namespace CBot
