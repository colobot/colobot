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
#include "CBot/CBotStack.h"
#include "CBot/CBotCall.h"

#include "CBot/CBotInstr/CBotFunction.h"

#include "CBot/CBotVar/CBotVarPointer.h"
#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotFileUtils.h"

// Local include

// Global include
#include <cassert>
#include <cstdlib>
#include <cstring>


#define    ITIMER    100

int         CBotStack::m_initimer = ITIMER;
int         CBotStack::m_timer = 0;
CBotVar*    CBotStack::m_retvar = nullptr;
int         CBotStack::m_error = 0;
int         CBotStack::m_start = 0;
int         CBotStack::m_end   = 0;
CBotString  CBotStack::m_labelBreak="";
void*       CBotStack::m_pUser = nullptr;

#if    STACKMEM

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::FirstStack()
{
    CBotStack*    p;

    long    size = sizeof(CBotStack);
    size    *= (MAXSTACK+10);

    // request a slice of memory for the stack
    p = static_cast<CBotStack*>(malloc(size));

    // completely empty
    memset(p, 0, size);

    p-> m_bBlock = true;
    m_timer = m_initimer;                // sets the timer at the beginning

    CBotStack* pp = p;
    pp += MAXSTACK;
    int i;
    for ( i = 0 ; i< 10 ; i++ )
    {
        pp->m_bOver = true;
        pp ++;
    }
#ifdef    _DEBUG
    int    n = 1;
    pp = p;
    for ( i = 0 ; i< MAXSTACK+10 ; i++ )
    {
        pp->m_index = n++;
        pp ++;
    }
#endif

    m_error = 0;    // avoids deadlocks because m_error is static
    return p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack::CBotStack(CBotStack* ppapa)
{
    // constructor must exist or the destructor is never called!
    assert(0);
}

////////////////////////////////////////////////////////////////////////////////
CBotStack::~CBotStack()
{
    assert(0);    // use Delete () instead
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::Delete()
{
    if ( this == nullptr || this == EOX ) return;

    m_next->Delete();
    m_next2->Delete();

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
#ifdef    _DEBUG
    int            n = m_index;
#endif

    // clears the freed block
    memset(this, 0, sizeof(CBotStack));
    m_bOver    = bOver;
#ifdef    _DEBUG
    m_index = n;
#endif

    if ( p == nullptr )
        free( this );
}

// routine improved
////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStack(CBotInstr* instr, bool bBlock)
{
    if (m_next != nullptr)
    {
        return m_next;                // included in an existing stack
    }

#ifdef    _DEBUG
    int        n = 0;
#endif
    CBotStack*    p = this;
    do
    {
        p ++;
#ifdef    _DEBUG
        n ++;
#endif
    }
    while ( p->m_prev != nullptr );

    m_next = p;                                    // chain an element
    p->m_bBlock         = bBlock;
    p->m_instr         = instr;
    p->m_prog         = m_prog;
    p->m_step         = 0;
    p->m_prev         = this;
    p->m_state         = 0;
    p->m_call         = nullptr;
    p->m_bFunc         = false;
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStackEOX(CBotCall* instr, bool bBlock)
{
    if (m_next != nullptr)
    {
        if ( m_next == EOX )
        {
            m_next = nullptr;
            return EOX;
        }
        return m_next;                // included in an existing stack
    }
    CBotStack*    p = AddStack(nullptr, bBlock);
    p->m_call = instr;
    p->m_bFunc = 2;    // special
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStack2(bool bBlock)
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
    p->m_prev = this;
    p->m_bBlock = bBlock;
    p->m_prog = m_prog;
    p->m_step = 0;
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::GetBlock()
{
    return    m_bBlock;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != nullptr) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = nullptr;                        // not to destroy the variable

    m_next->Delete();m_next = nullptr;                // releases the stack above
    m_next2->Delete();m_next2 = nullptr;            // also the second stack (catch)

    return (m_error == 0);                        // interrupted if error
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::ReturnKeep(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != nullptr) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = nullptr;                        // not to destroy the variable

    return (m_error == 0);                        // interrupted if error
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::StackOver()
{
    if (!m_bOver) return false;
    m_error = TX_STACKOVER;
    return true;
}

#else

////////////////////////////////////////////////////////////////////////////////
CBotStack::CBotStack(CBotStack* ppapa)
{
    m_next  = nullptr;
    m_next2 = nullptr;
    m_prev  = ppapa;

    m_bBlock = (ppapa == nullptr) ? true : false;

    m_state = 0;
    m_step = 1;

    if (ppapa == nullptr) m_timer = m_initimer;                // sets the timer at the beginning

    m_listVar = nullptr;
    m_bDontDelete = false;

    m_var      = nullptr;
    m_prog      = nullptr;
    m_instr      = nullptr;
    m_call      = nullptr;
    m_bFunc      = false;
}

////////////////////////////////////////////////////////////////////////////////
// destructor
CBotStack::~CBotStack()
{
    if ( m_next != EOX) delete m_next;
    delete m_next2;
    if (m_prev != nullptr && m_prev->m_next == this )
            m_prev->m_next = nullptr;        // removes chain

    delete m_var;
    if ( !m_bDontDelete ) delete m_listVar;
}

////////////////////////////////////////////////////////////////////////////////
// \TODO routine has/to optimize
CBotStack* CBotStack::AddStack(CBotInstr* instr, bool bBlock)
{
    if (m_next != nullptr)
    {
        return m_next;                // included in an existing stack
    }
    CBotStack*    p = new CBotStack(this);
    m_next = p;                                    // chain an element
    p->m_bBlock = bBlock;
    p->m_instr = instr;
    p->m_prog = m_prog;
    p->m_step = 0;
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStackEOX(CBotCall* instr, bool bBlock)
{
    if (m_next != nullptr)
    {
        if ( m_next == EOX )
        {
            m_next = nullptr;
            return EOX;
        }
        return m_next;                // included in an existing stack
    }
    CBotStack*    p = new CBotStack(this);
    m_next = p;                                    // chain an element
    p->m_bBlock = bBlock;
    p->m_call = instr;
    p->m_prog = m_prog;
    p->m_step = 0;
    p->m_bFunc = 2;    // special
    return    p;
}

////////////////////////////////////////////////////////////////////////////////
CBotStack* CBotStack::AddStack2(bool bBlock)
{
    if (m_next2 != nullptr)
    {
        m_next2->m_prog = m_prog;        // special avoids RestoreStack2
        return m_next2;                    // included in an existing stack
    }

    CBotStack*    p = new CBotStack(this);
    m_next2 = p;                                // chain an element
    p->m_bBlock = bBlock;
    p->m_prog = m_prog;
    p->m_step = 0;

    return    p;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != nullptr) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = nullptr;                        // do not destroy the variable

    if ( m_next != EOX ) delete m_next;            // releases the stack above
    delete m_next2;m_next2 = nullptr;                // also the second stack (catch)

    return (m_error == 0);                        // interrupted if error
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::StackOver()
{
    return false;            // no overflow check in this version
}

#endif

////////////////////////////////////////////////////////////////////////////////
void CBotStack::Reset(void* pUser)
{
    m_timer = m_initimer;        // resets the timer
    m_error    = 0;
//    m_start = 0;
//    m_end    = 0;
    m_labelBreak.Empty();
    m_pUser = pUser;
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
CBotStack* CBotStack::RestoreStackEOX(CBotCall* instr)
{
    CBotStack*     p = RestoreStack();
    p->m_call = instr;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
// routine for execution step by step
bool CBotStack::IfStep()
{
    if ( m_initimer > 0 || m_step++ > 0 ) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::BreakReturn(CBotStack* pfils, const char* name)
{
    if ( m_error>=0 ) return false;                // normal output
    if ( m_error==-3 ) return false;            // normal output (return current)

    if (!m_labelBreak.IsEmpty() && (name[0] == 0 || m_labelBreak != name))
        return false;                            // it's not for me

    m_error = 0;
    m_labelBreak.Empty();
    return Return(pfils);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::IfContinue(int state, const char* name)
{
    if ( m_error != -2 ) return false;

    if (!m_labelBreak.IsEmpty() && (name == nullptr || m_labelBreak != name))
        return false;                            // it's not for me

    m_state = state;                            // where again?
    m_error = 0;
    m_labelBreak.Empty();
    if ( m_next != EOX ) m_next->Delete();            // purge above stack
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetBreak(int val, const char* name)
{
    m_error = -val;                                // reacts as an Exception
    m_labelBreak = name;
    if (val == 3)    // for a return
    {
        m_retvar = m_var;
        m_var = nullptr;
    }
}

// gives on the stack value calculated by the last CBotReturn
////////////////////////////////////////////////////////////////////////////////
bool CBotStack::GetRetVar(bool bRet)
{
    if (m_error == -3)
    {
        if ( m_var ) delete m_var;
        m_var        = m_retvar;
        m_retvar    = nullptr;
        m_error        = 0;
        return        true;
    }
    return bRet;                        // interrupted by something other than return
}

////////////////////////////////////////////////////////////////////////////////
int CBotStack::GetError(int& start, int& end)
{
    start = m_start;
    end      = m_end;
    return m_error;
}

////////////////////////////////////////////////////////////////////////////////
int CBotStack::GetType(int mode)
{
    if (m_var == nullptr) return -1;
    return m_var->GetType(mode);
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotStack::GetTypResult(int mode)
{
    if (m_var == nullptr) return -1;
    return m_var->GetTypResult(mode);
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetType(CBotTypResult& type)
{
    if (m_var == nullptr) return;
    m_var->SetType( type );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(CBotToken* &pToken, bool bUpdate, bool bModif)
{
    CBotStack*    p = this;
    CBotString    name = pToken->GetString();

    while (p != nullptr)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != nullptr)
        {
            if (pp->GetName() == name)
            {
                if ( bUpdate )
                    pp->Maj(m_pUser, false);

                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(const char* name)
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
CBotVar* CBotStack::FindVar(long ident, bool bUpdate, bool bModif)
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
                    pp->Maj(m_pUser, false);

                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::FindVar(CBotToken& pToken, bool bUpdate, bool bModif)
{
    CBotToken*    pt = &pToken;
    return FindVar(pt, bUpdate, bModif);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::CopyVar(CBotToken& Token, bool bUpdate)
{
    CBotVar*    pVar = FindVar( Token, bUpdate );

    if ( pVar == nullptr) return nullptr;

    CBotVar*    pCopy = CBotVar::Create(pVar);
    pCopy->Copy(pVar);
    return    pCopy;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::SetState(int n, int limite)
{
    m_state = n;

    m_timer--;                                    // decrement the operations \TODO decrement the operations
    return ( m_timer > limite );                    // interrupted if timer pass
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::IncState(int limite)
{
    m_state++;

    m_timer--;                                    // decrement the operations \TODO decompte les operations
    return ( m_timer > limite );                    // interrupted if timer pass
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetError(int n, CBotToken* token)
{
    if ( n!= 0 && m_error != 0) return;    // does not change existing error
    m_error = n;
    if (token != nullptr)
    {
        m_start = token->GetStart();
        m_end   = token->GetEnd();
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::ResetError(int n, int start, int end)
{
    m_error = n;
    m_start    = start;
    m_end    = end;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetPosError(CBotToken* token)
{
    m_start = token->GetStart();
    m_end   = token->GetEnd();
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetTimer(int n)
{
    m_initimer = n;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::Execute()
{
    CBotCall*        instr = nullptr;                        // the most highest instruction
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

    if (!instr->Run(pile)) return false;            // \TODO exécution à partir de là

#if    STACKMEM
    pile->m_next->Delete();
#else
    delete pile->m_next;
#endif

    pile->m_next = EOX;            // special for recovery
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

    m_var = CBotVar::Create("", var->GetTypResult(2));
    m_var->Copy( var );
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::GetVar()
{
    return m_var;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::GetPtVar()
{
    CBotVar*    p = m_var;
    m_var = nullptr;                // therefore will not be destroyed
    return p;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::GetCopyVar()
{
    if (m_var == nullptr) return nullptr;
    CBotVar*    v = CBotVar::Create("", m_var->GetType());
    v->Copy( m_var );
    return v;
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
    while (p != nullptr && p->m_bBlock == 0) p = p->m_prev;

    if ( p == nullptr ) return;

///    p->m_bDontDelete = bDontDelete;

    CBotVar**    pp = &p->m_listVar;
    while ( *pp != nullptr ) pp = &(*pp)->m_next;

    *pp = pVar;                    // added after

#ifdef    _DEBUG
    if ( pVar->GetUniqNum() == 0 ) assert(0);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::SetBotCall(CBotProgram* p)
{
    m_prog  = p;
    m_bFunc = true;
}

////////////////////////////////////////////////////////////////////////////////
CBotProgram*  CBotStack::GetBotCall(bool bFirst)
{
    if ( ! bFirst )    return m_prog;
    CBotStack*    p = this;
    while ( p->m_prev != nullptr ) p = p->m_prev;
    return p->m_prog;
}

////////////////////////////////////////////////////////////////////////////////
void* CBotStack::GetPUser()
{
    return m_pUser;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype)
{
    CBotTypResult        res;

    // first looks by the identifier

    res = CBotCall::DoCall(nIdent, nullptr, ppVar, this, rettype );
    if (res.GetType() >= 0) return res.GetType();

    res = m_prog->GetFunctions()->DoCall(nIdent, nullptr, ppVar, this, token );
    if (res.GetType() >= 0) return res.GetType();

    // if not found (recompile?) seeks by name

    nIdent = 0;
    res = CBotCall::DoCall(nIdent, token, ppVar, this, rettype );
    if (res.GetType() >= 0) return res.GetType();

    res = m_prog->GetFunctions()->DoCall(nIdent, token->GetString(), ppVar, this, token );
    if (res.GetType() >= 0) return res.GetType();

    SetError(TX_NOCALL, token);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar)
{
    if ( m_next == nullptr ) return;

    if ( !CBotCall::RestoreCall(nIdent, token, ppVar, this) )
        m_prog->GetFunctions()->RestoreCall(nIdent, token->GetString(), ppVar, this );
}

////////////////////////////////////////////////////////////////////////////////
bool SaveVar(FILE* pf, CBotVar* pVar)
{
    while ( true )
    {
        if ( pVar == nullptr )
        {
            return WriteWord(pf, 0);                            // is a terminator
        }

        if ( !pVar->Save0State(pf)) return false;                // common header
        if ( !pVar->Save1State(pf) ) return false;                // saves as the child class

        pVar = pVar->GetNext();
    }
}

////////////////////////////////////////////////////////////////////////////////
void CBotStack::GetRunPos(const char* &FunctionName, int &start, int &end)
{
    CBotProgram*    prog = m_prog;                        // Current program

    CBotInstr*        funct = nullptr;                        // function found
    CBotInstr*        instr = nullptr;                        // the highest intruction

    CBotStack*        p = this;

    while (p->m_next != nullptr)
    {
        if ( p->m_instr != nullptr ) instr = p->m_instr;
        if ( p->m_bFunc == 1 && p->m_instr != nullptr ) funct = p->m_instr;
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }

    if ( p->m_instr != nullptr ) instr = p->m_instr;
    if ( p->m_bFunc == 1 && p->m_instr != nullptr ) funct = p->m_instr;

    if ( funct == nullptr ) return;

    CBotToken* t = funct->GetToken();
    FunctionName = t->GetString();

//    if ( p->m_instr != nullptr ) instr = p->m_instr;

    t = instr->GetToken();
    start = t->GetStart();
    end   = t->GetEnd();
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotStack::GetStackVars(const char* &FunctionName, int level)
{
    CBotProgram*    prog = m_prog;                        // current program
    FunctionName    = nullptr;

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
    while ( p != nullptr && !p->m_bBlock )    p = p->m_prev;
    // Now p is on the beggining of the top block (with local variables)

    while ( p != nullptr && level++ < 0 )
    {
        p = p->m_prev;
        while ( p != nullptr && !p->m_bBlock )    p = p->m_prev;
    }
    // Now p is on the block "level"

    if ( p == nullptr ) return nullptr;

    // search the name of the current function
    CBotStack* pp = p;
    while ( pp != nullptr )
    {
        if ( pp->m_bFunc == 1 ) break;
        pp = pp->m_prev;
    }

    if ( pp == nullptr || pp->m_instr == nullptr ) return nullptr;

    CBotToken* t = pp->m_instr->GetToken();
    FunctionName = t->GetString();

    return p->m_listVar;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::SaveState(FILE* pf)
{
    if ( this == nullptr )                                    // end of the tree?
    {
        return WriteWord(pf, 0);                        // is a terminator
    }

    if ( m_next2 != nullptr )
    {
        if (!WriteWord(pf, 2)) return false;                // a mark of pursuit
        if (!m_next2->SaveState(pf)) return false;
    }
    else
    {
        if (!WriteWord(pf, 1)) return false;                // a mark of pursuit
    }
    if (!WriteWord(pf, m_bBlock)) return false;            // is a local block
    if (!WriteWord(pf, m_state)) return false;            // in what state?
    if (!WriteWord(pf, 0)) return false;                // by compatibility m_bDontDelete
    if (!WriteWord(pf, m_step)) return false;            // in what state?


    if (!SaveVar(pf, m_var)) return false;            // current result
    if (!SaveVar(pf, m_listVar)) return false;        // local variables

    return m_next->SaveState(pf);                        // saves the following
}

////////////////////////////////////////////////////////////////////////////////
bool CBotStack::RestoreState(FILE* pf, CBotStack* &pStack)
{
    unsigned short    w;

    pStack = nullptr;
    if (!ReadWord(pf, w)) return false;
    if ( w == 0 ) return true;

#if    STACKMEM
    if ( this == nullptr ) pStack = FirstStack();
    else pStack = AddStack();
#else
    pStack = new CBotStack(this);
#endif

    if ( w == 2 )
    {
        if (!pStack->RestoreState(pf, pStack->m_next2)) return false;
    }

    if (!ReadWord(pf, w)) return false;            // is a local block
    pStack->m_bBlock = w;

    if (!ReadWord(pf, w)) return false;            // in what state ?
    pStack->SetState(static_cast<short>(w));                    // in a good state

    if (!ReadWord(pf, w)) return false;            // dont delete?
                                                // uses more

    if (!ReadWord(pf, w)) return false;            // step by step
    pStack->m_step = w;

    if (!CBotVar::RestoreState(pf, pStack->m_var)) return false;    // temp variable
    if (!CBotVar::RestoreState(pf, pStack->m_listVar)) return false;// local variables

    return pStack->RestoreState(pf, pStack->m_next);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::Save0State(FILE* pf)
{
    if (!WriteWord(pf, 100+m_mPrivate))return false;        // private variable?
    if (!WriteWord(pf, m_bStatic))return false;                // static variable?
    if (!WriteWord(pf, m_type.GetType()))return false;        // saves the type (always non-zero)
    if (!WriteWord(pf, static_cast<unsigned short>(m_binit))) return false;                // variable defined?
    return WriteString(pf, m_token->GetString());            // and variable name
}

namespace
{
bool ParseInitType(int rawInitType, CBotVar::InitType* initType)
{
    switch (rawInitType)
    {
    case static_cast<int>(CBotVar::InitType::UNDEF):
        *initType = CBotVar::InitType::UNDEF;
        break;
    case static_cast<int>(CBotVar::InitType::DEF):
        *initType = CBotVar::InitType::DEF;
        break;
    case static_cast<int>(CBotVar::InitType::IS_POINTER):
        *initType = CBotVar::InitType::IS_POINTER;
        break;
    case static_cast<int>(CBotVar::InitType::IS_NAN):
        *initType = CBotVar::InitType::IS_NAN;
        break;
    default:
        *initType = CBotVar::InitType::UNDEF;
        return false;
    }
    return true;
}
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVar::RestoreState(FILE* pf, CBotVar* &pVar)
{
    unsigned short        w, wi, prv, st;
    float        ww;
    CBotString    name, s;

    delete pVar;

                pVar    = nullptr;
    CBotVar*    pNew    = nullptr;
    CBotVar*    pPrev    = nullptr;

    while ( true )            // retrieves a list
    {
        if (!ReadWord(pf, w)) return false;                        // private or type?
        if ( w == 0 ) return true;

        CBotString defnum;
        if ( w == 200 )
        {
            if (!ReadString(pf, defnum)) return false;            // number with identifier
            if (!ReadWord(pf, w)) return false;                    // type
        }

        prv = 100; st = 0;
        if ( w >= 100 )
        {
            prv = w;
            if (!ReadWord(pf, st)) return false;                // static
            if (!ReadWord(pf, w)) return false;                    // type
        }

        if ( w == CBotTypClass ) w = CBotTypIntrinsic;            // necessarily intrinsic

        CBotVar::InitType initType = CBotVar::InitType::UNDEF;
        if (!ReadWord(pf, wi) || !ParseInitType(wi, &initType)) return false;                    // init ?
        if (!ReadString(pf, name)) return false;                // variable name

        CBotToken token(name, CBotString());

        switch (w)
        {
        case CBotTypInt:
        case CBotTypBoolean:
            pNew = CBotVar::Create(&token, w);                        // creates a variable
            if (!ReadWord(pf, w)) return false;
            pNew->SetValInt(static_cast<short>(w), defnum);
            break;
        case CBotTypFloat:
            pNew = CBotVar::Create(&token, w);                        // creates a variable
            if (!ReadFloat(pf, ww)) return false;
            pNew->SetValFloat(ww);
            break;
        case CBotTypString:
            pNew = CBotVar::Create(&token, w);                        // creates a variable
            if (!ReadString(pf, s)) return false;
            pNew->SetValString(s);
            break;

        // returns an intrinsic object or element of an array
        case CBotTypIntrinsic:
        case CBotTypArrayBody:
            {
                CBotTypResult    r;
                long            id;
                if (!ReadType(pf, r))  return false;                // complete type
                if (!ReadLong(pf, id) ) return false;

//                if (!ReadString(pf, s)) return false;
                {
                    CBotVar* p = nullptr;
                    if ( id ) p = CBotVarClass::Find(id) ;

                    pNew = new CBotVarClass(&token, r);                // directly creates an instance
                                                                    // attention cptuse = 0
                    if ( !RestoreState(pf, (static_cast<CBotVarClass*>(pNew))->m_pVar)) return false;
                    pNew->SetIdent(id);

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
            if (!ReadString(pf, s)) return false;
            {
                pNew = CBotVar::Create(&token, CBotTypResult(w, s));// creates a variable
//                CBotVarClass* p = nullptr;
                long id;
                ReadLong(pf, id);
//                if ( id ) p = CBotVarClass::Find(id);        // found the instance (made by RestoreInstance)

                // returns a copy of the original instance
                CBotVar* pInstance = nullptr;
                if ( !CBotVar::RestoreState( pf, pInstance ) ) return false;
                (static_cast<CBotVarPointer*>(pNew))->SetPointer( pInstance );            // and point over

//                if ( p != nullptr ) (static_cast<CBotVarPointer*>(pNew))->SetPointer( p );    // rather this one

            }
            break;

        case CBotTypArrayPointer:
            {
                CBotTypResult    r;
                if (!ReadType(pf, r))  return false;

                pNew = CBotVar::Create(&token, r);                        // creates a variable

                // returns a copy of the original instance
                CBotVar* pInstance = nullptr;
                if ( !CBotVar::RestoreState( pf, pInstance ) ) return false;
                (static_cast<CBotVarPointer*>(pNew))->SetPointer( pInstance );            // and point over
            }
            break;
        default:
            assert(0);
        }

        if ( pPrev != nullptr ) pPrev->m_next = pNew;
        if ( pVar == nullptr  ) pVar = pNew;

        pNew->m_binit = initType;        //        pNew->SetInit(wi);
        pNew->SetStatic(st);
        pNew->SetPrivate(prv-100);
        pPrev = pNew;
    }
    return true;
}
