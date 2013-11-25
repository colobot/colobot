// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

//Management of the stack


#include "CBot.h"

#include <cassert>
#include <cstdlib>
#include <cstring>


#define    ITIMER    100

////////////////////////////////////////////////////////////////////////////
// management of a execution of a stack
////////////////////////////////////////////////////////////////////////////

int         CBotStack::m_initimer = ITIMER;
int         CBotStack::m_timer = 0;
CBotVar*    CBotStack::m_retvar = NULL;
int         CBotStack::m_error = 0;
int         CBotStack::m_start = 0;
int         CBotStack::m_end   = 0;
CBotString  CBotStack::m_labelBreak="";
void*       CBotStack::m_pUser = NULL;

#if    STACKMEM

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

CBotStack::CBotStack(CBotStack* ppapa)
{
    // constructor must exist or the destructor is never called!
    assert(0);
}

CBotStack::~CBotStack()
{
    assert(0);    // use Delete () instead
}

void CBotStack::Delete()
{
    if ( this == NULL || this == EOX ) return;

    m_next->Delete();
    m_next2->Delete();

    if (m_prev != NULL)
    {
        if ( m_prev->m_next == this )
            m_prev->m_next = NULL;        // removes chain

        if ( m_prev->m_next2 == this )
            m_prev->m_next2 = NULL;        // removes chain
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

    if ( p == NULL )
        free( this );
}


// routine improved
CBotStack* CBotStack::AddStack(CBotInstr* instr, bool bBlock)
{
    if (m_next != NULL)
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
    while ( p->m_prev != NULL );

    m_next = p;                                    // chain an element
    p->m_bBlock         = bBlock;
    p->m_instr         = instr;
    p->m_prog         = m_prog;
    p->m_step         = 0;
    p->m_prev         = this;
    p->m_state         = 0;
    p->m_call         = NULL;
    p->m_bFunc         = false;
    return    p;
}

CBotStack* CBotStack::AddStackEOX(CBotCall* instr, bool bBlock)
{
    if (m_next != NULL)
    {
        if ( m_next == EOX )
        {
            m_next = NULL;
            return EOX;
        }
        return m_next;                // included in an existing stack
    }
    CBotStack*    p = AddStack(NULL, bBlock);
    p->m_call = instr;
    p->m_bFunc = 2;    // special
    return    p;
}

CBotStack* CBotStack::AddStack2(bool bBlock)
{
    if (m_next2 != NULL)
    {
        m_next2->m_prog = m_prog;        // special avoids RestoreStack2
        return m_next2;                    // included in an existing stack
    }

    CBotStack*    p = this;
    do
    {
        p ++;
    }
    while ( p->m_prev != NULL );

    m_next2 = p;                                // chain an element
    p->m_prev = this;
    p->m_bBlock = bBlock;
    p->m_prog = m_prog;
    p->m_step = 0;
    return    p;
}

bool CBotStack::GetBlock()
{
    return    m_bBlock;
}

bool CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != NULL) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = NULL;                        // not to destroy the variable

    m_next->Delete();m_next = NULL;                // releases the stack above
    m_next2->Delete();m_next2 = NULL;            // also the second stack (catch)

    return (m_error == 0);                        // interrupted if error
}

bool CBotStack::ReturnKeep(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != NULL) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = NULL;                        // not to destroy the variable

    return (m_error == 0);                        // interrupted if error
}

bool CBotStack::StackOver()
{
    if (!m_bOver) return false;
    m_error = TX_STACKOVER;
    return true;
}

#else

CBotStack::CBotStack(CBotStack* ppapa)
{
    m_next  = NULL;
    m_next2 = NULL;
    m_prev  = ppapa;

    m_bBlock = (ppapa == NULL) ? true : false;

    m_state = 0;
    m_step = 1;

    if (ppapa == NULL) m_timer = m_initimer;                // sets the timer at the beginning

    m_listVar = NULL;
    m_bDontDelete = false;

    m_var      = NULL;
    m_prog      = NULL;
    m_instr      = NULL;
    m_call      = NULL;
    m_bFunc      = false;
}

// destructor
CBotStack::~CBotStack()
{
    if ( m_next != EOX) delete m_next;
    delete m_next2;
    if (m_prev != NULL && m_prev->m_next == this )
            m_prev->m_next = NULL;        // removes chain

    delete m_var;
    if ( !m_bDontDelete ) delete m_listVar;
}

// \TODO routine has/to optimize
CBotStack* CBotStack::AddStack(CBotInstr* instr, bool bBlock)
{
    if (m_next != NULL)
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

CBotStack* CBotStack::AddStackEOX(CBotCall* instr, bool bBlock)
{
    if (m_next != NULL)
    {
        if ( m_next == EOX )
        {
            m_next = NULL;
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

CBotStack* CBotStack::AddStack2(bool bBlock)
{
    if (m_next2 != NULL)
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

bool CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return true;    // special

    if (m_var != NULL) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = NULL;                        // do not destroy the variable

    if ( m_next != EOX ) delete m_next;            // releases the stack above
    delete m_next2;m_next2 = NULL;                // also the second stack (catch)

    return (m_error == 0);                        // interrupted if error
}

bool CBotStack::StackOver()
{
    return false;            // no overflow check in this version
}

#endif

void CBotStack::Reset(void* pUser)
{
    m_timer = m_initimer;        // resets the timer
    m_error    = 0;
//    m_start = 0;
//    m_end    = 0;
    m_labelBreak.Empty();
    m_pUser = pUser;
}




CBotStack* CBotStack::RestoreStack(CBotInstr* instr)
{
    if (m_next != NULL)
    {
        m_next->m_instr = instr;    // reset (if recovery after )
        m_next->m_prog = m_prog;
        return m_next;                // included in an existing stack
    }
    return    NULL;
}

CBotStack* CBotStack::RestoreStackEOX(CBotCall* instr)
{
    CBotStack*     p = RestoreStack();
    p->m_call = instr;
    return p;
}



// routine for execution step by step
bool CBotStack::IfStep()
{
    if ( m_initimer > 0 || m_step++ > 0 ) return false;
    return true;
}


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

bool CBotStack::IfContinue(int state, const char* name)
{
    if ( m_error != -2 ) return false;

    if (!m_labelBreak.IsEmpty() && (name == NULL || m_labelBreak != name))
        return false;                            // it's not for me

    m_state = state;                            // where again?
    m_error = 0;
    m_labelBreak.Empty();
    if ( m_next != EOX ) m_next->Delete();            // purge above stack
    return true;
}

void CBotStack::SetBreak(int val, const char* name)
{
    m_error = -val;                                // reacts as an Exception
    m_labelBreak = name;
    if (val == 3)    // for a return
    {
        m_retvar = m_var;
        m_var = NULL;
    }
}

// gives on the stack value calculated by the last CBotReturn

bool CBotStack::GetRetVar(bool bRet)
{
    if (m_error == -3)
    {
        if ( m_var ) delete m_var;
        m_var        = m_retvar;
        m_retvar    = NULL;
        m_error        = 0;
        return        true;
    }
    return bRet;                        // interrupted by something other than return
}

int CBotStack::GetError(int& start, int& end)
{
    start = m_start;
    end      = m_end;
    return m_error;
}


int CBotStack::GetType(int mode)
{
    if (m_var == NULL) return -1;
    return m_var->GetType(mode);
}

CBotTypResult CBotStack::GetTypResult(int mode)
{
    if (m_var == NULL) return -1;
    return m_var->GetTypResult(mode);
}

void CBotStack::SetType(CBotTypResult& type)
{
    if (m_var == NULL) return;
    m_var->SetType( type );
}


CBotVar* CBotStack::FindVar(CBotToken* &pToken, bool bUpdate, bool bModif)
{
    CBotStack*    p = this;
    CBotString    name = pToken->GetString();

    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
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
    return NULL;
}

CBotVar* CBotStack::FindVar(const char* name)
{
    CBotStack*    p = this;
    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (pp->GetName() == name)
            {
                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return NULL;
}

CBotVar* CBotStack::FindVar(long ident, bool bUpdate, bool bModif)
{
    CBotStack*    p = this;
    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
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
    return NULL;
}


CBotVar* CBotStack::FindVar(CBotToken& Token, bool bUpdate, bool bModif)
{
    CBotToken*    pt = &Token;
    return FindVar(pt, bUpdate, bModif);
}


CBotVar* CBotStack::CopyVar(CBotToken& Token, bool bUpdate)
{
    CBotVar*    pVar = FindVar( Token, bUpdate );

    if ( pVar == NULL) return NULL;

    CBotVar*    pCopy = CBotVar::Create(pVar);
    pCopy->Copy(pVar);
    return    pCopy;
}


bool CBotStack::SetState(int n, int limite)
{
    m_state = n;

    m_timer--;                                    // decrement the operations \TODO decrement the operations
    return ( m_timer > limite );                    // interrupted if timer pass
}

bool CBotStack::IncState(int limite)
{
    m_state++;

    m_timer--;                                    // decrement the operations \TODO decompte les operations
    return ( m_timer > limite );                    // interrupted if timer pass
}


void CBotStack::SetError(int n, CBotToken* token)
{
    if ( n!= 0 && m_error != 0) return;    // does not change existing error
    m_error = n;
    if (token != NULL)
    {
        m_start = token->GetStart();
        m_end   = token->GetEnd();
    }
}

void CBotStack::ResetError(int n, int start, int end)
{
    m_error = n;
    m_start    = start;
    m_end    = end;
}

void CBotStack::SetPosError(CBotToken* token)
{
    m_start = token->GetStart();
    m_end   = token->GetEnd();
}

void CBotStack::SetTimer(int n)
{
    m_initimer = n;
}

bool CBotStack::Execute()
{
    CBotCall*        instr = NULL;                        // the most highest instruction
    CBotStack*        pile;

    CBotStack*        p = this;

    while (p != NULL)
    {
        if ( p->m_next2 != NULL ) break;
        if ( p->m_call != NULL )
        {
            instr = p->m_call;
            pile  = p->m_prev ;
        }
        p = p->m_next;
    }

    if ( instr == NULL ) return true;                // normal execution request

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
void CBotStack::SetVar( CBotVar* var )
{
    if (m_var) delete m_var;    // replacement of a variable
    m_var = var;
}

// puts on the stack a copy of a variable
void CBotStack::SetCopyVar( CBotVar* var )
{
    if (m_var) delete m_var;    // replacement of a variable

    m_var = CBotVar::Create("", var->GetTypResult(2));
    m_var->Copy( var );
}

CBotVar* CBotStack::GetVar()
{
    return m_var;
}

CBotVar* CBotStack::GetPtVar()
{
    CBotVar*    p = m_var;
    m_var = NULL;                // therefore will not be destroyed
    return p;
}

CBotVar* CBotStack::GetCopyVar()
{
    if (m_var == NULL) return NULL;
    CBotVar*    v = CBotVar::Create("", m_var->GetType());
    v->Copy( m_var );
    return v;
}

long CBotStack::GetVal()
{
    if (m_var == NULL) return 0;
    return m_var->GetValInt();
}




void CBotStack::AddVar(CBotVar* pVar)
{
    CBotStack*    p = this;

    // returns to the father element
    while (p != NULL && p->m_bBlock == 0) p = p->m_prev;

    if ( p == NULL ) return;

///    p->m_bDontDelete = bDontDelete;

    CBotVar**    pp = &p->m_listVar;
    while ( *pp != NULL ) pp = &(*pp)->m_next;

    *pp = pVar;                    // added after

#ifdef    _DEBUG
    if ( pVar->GetUniqNum() == 0 ) assert(0);
#endif
}

/*void CBotStack::RestoreVar(CBotVar* pVar)
{
    if ( !m_bDontDelete ) __asm int 3;
    delete    m_listVar;
    m_listVar = pVar;        // direct replacement
}*/

void CBotStack::SetBotCall(CBotProgram* p)
{
    m_prog  = p;
    m_bFunc = true;
}

CBotProgram*  CBotStack::GetBotCall(bool bFirst)
{
    if ( ! bFirst )    return m_prog;
    CBotStack*    p = this;
    while ( p->m_prev != NULL ) p = p->m_prev;
    return p->m_prog;
}

void* CBotStack::GetPUser()
{
    return m_pUser;
}


bool CBotStack::ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype)
{
    CBotTypResult        res;

    // first looks by the identifier

    res = CBotCall::DoCall(nIdent, NULL, ppVar, this, rettype );
    if (res.GetType() >= 0) return res.GetType();

    res = m_prog->GetFunctions()->DoCall(nIdent, NULL, ppVar, this, token );
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

void CBotStack::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar)
{
    if ( m_next == NULL ) return;

    if ( !CBotCall::RestoreCall(nIdent, token, ppVar, this) )
        m_prog->GetFunctions()->RestoreCall(nIdent, token->GetString(), ppVar, this );
}


bool SaveVar(FILE* pf, CBotVar* pVar)
{
    while ( true )
    {
        if ( pVar == NULL )
        {
            return WriteWord(pf, 0);                            // is a terminator
        }

        if ( !pVar->Save0State(pf)) return false;                // common header
        if ( !pVar->Save1State(pf) ) return false;                // saves as the child class

        pVar = pVar->GetNext();
    }
}

void CBotStack::GetRunPos(const char* &FunctionName, int &start, int &end)
{
    CBotProgram*    prog = m_prog;                        // Current program

    CBotInstr*        funct = NULL;                        // function found
    CBotInstr*        instr = NULL;                        // the highest intruction

    CBotStack*        p = this;

    while (p->m_next != NULL)
    {
        if ( p->m_instr != NULL ) instr = p->m_instr;
        if ( p->m_bFunc == 1 ) funct = p->m_instr;
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }

    if ( p->m_instr != NULL ) instr = p->m_instr;
    if ( p->m_bFunc == 1 ) funct = p->m_instr;

    if ( funct == NULL ) return;

    CBotToken* t = funct->GetToken();
    FunctionName = t->GetString();

//    if ( p->m_instr != NULL ) instr = p->m_instr;

    t = instr->GetToken();
    start = t->GetStart();
    end      = t->GetEnd();
}

CBotVar* CBotStack::GetStackVars(const char* &FunctionName, int level)
{
    CBotProgram*    prog = m_prog;                        // current program
    FunctionName    = NULL;

    // back the stack in the current module
    CBotStack*        p = this;

    while (p->m_next != NULL)
    {
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }


    // descends upon the elements of block
    while ( p != NULL && !p->m_bBlock )    p = p->m_prev;

    while ( p != NULL && level++ < 0 )
    {
        p = p->m_prev;
        while ( p != NULL && !p->m_bBlock )    p = p->m_prev;
    }

    if ( p == NULL ) return NULL;

    // search the name of the current function
    CBotStack* pp = p;
    while ( pp != NULL )
    {
        if ( pp->m_bFunc == 1 ) break;
        pp = pp->m_prev;
    }

    if ( pp == NULL || pp->m_instr == NULL ) return NULL;

    CBotToken* t = pp->m_instr->GetToken();
    FunctionName = t->GetString();

    return p->m_listVar;
}

bool CBotStack::SaveState(FILE* pf)
{
    if ( this == NULL )                                    // end of the tree?
    {
        return WriteWord(pf, 0);                        // is a terminator
    }

    if ( m_next2 != NULL )
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


bool CBotStack::RestoreState(FILE* pf, CBotStack* &pStack)
{
    unsigned short    w;

    pStack = NULL;
    if (!ReadWord(pf, w)) return false;
    if ( w == 0 ) return true;

#if    STACKMEM
    if ( this == NULL ) pStack = FirstStack();
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


bool CBotVar::Save0State(FILE* pf)
{
    if (!WriteWord(pf, 100+m_mPrivate))return false;        // private variable?
    if (!WriteWord(pf, m_bStatic))return false;                // static variable?
    if (!WriteWord(pf, m_type.GetType()))return false;        // saves the type (always non-zero)
    if (!WriteWord(pf, m_binit))return false;                // variable defined?
    return WriteString(pf, m_token->GetString());            // and variable name
}

bool CBotVarInt::Save0State(FILE* pf)
{
    if ( !m_defnum.IsEmpty() )
    {
        if(!WriteWord(pf, 200 )) return false;            // special marker
        if(!WriteString(pf, m_defnum)) return false;    // name of the value
    }

    return CBotVar::Save0State(pf);
}

bool CBotVarInt::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);                            // the value of the variable
}

bool CBotVarBoolean::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);                            // the value of the variable
}

bool CBotVarFloat::Save1State(FILE* pf)
{
    return WriteFloat(pf, m_val);                            // the value of the variable
}

bool CBotVarString::Save1State(FILE* pf)
{
    return WriteString(pf, m_val);                            // the value of the variable
}



bool CBotVarClass::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return false;
    if ( !WriteLong(pf, m_ItemIdent) ) return false;

    return SaveVar(pf, m_pVar);                                // content of the object
}

bool CBotVar::RestoreState(FILE* pf, CBotVar* &pVar)
{
    unsigned short        w, wi, prv, st;
    float        ww;
    CBotString    name, s;

    delete pVar;

                pVar    = NULL;
    CBotVar*    pNew    = NULL;
    CBotVar*    pPrev    = NULL;

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

        if (!ReadWord(pf, wi)) return false;                    // init ?

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
                    CBotVar* p = NULL;
                    if ( id ) p = CBotVarClass::Find(id) ;

                    pNew = new CBotVarClass(&token, r);                // directly creates an instance
                                                                    // attention cptuse = 0
                    if ( !RestoreState(pf, (static_cast<CBotVarClass*>(pNew))->m_pVar)) return false;
                    pNew->SetIdent(id);

                    if ( p != NULL )
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
//                CBotVarClass* p = NULL;
                long id;
                ReadLong(pf, id);
//                if ( id ) p = CBotVarClass::Find(id);        // found the instance (made by RestoreInstance)

                // returns a copy of the original instance
                CBotVar* pInstance = NULL;
                if ( !CBotVar::RestoreState( pf, pInstance ) ) return false;
                (static_cast<CBotVarPointer*>(pNew))->SetPointer( pInstance );            // and point over

//                if ( p != NULL ) (static_cast<CBotVarPointer*>(pNew))->SetPointer( p );    // rather this one

            }
            break;

        case CBotTypArrayPointer:
            {
                CBotTypResult    r;
                if (!ReadType(pf, r))  return false;

                pNew = CBotVar::Create(&token, r);                        // creates a variable

                // returns a copy of the original instance
                CBotVar* pInstance = NULL;
                if ( !CBotVar::RestoreState( pf, pInstance ) ) return false;
                (static_cast<CBotVarPointer*>(pNew))->SetPointer( pInstance );            // and point over
            }
            break;
        default:
            assert(0);
        }

        if ( pPrev != NULL ) pPrev->m_next = pNew;
        if ( pVar == NULL  ) pVar = pNew;

        pNew->m_binit = wi;        //        pNew->SetInit(wi);
        pNew->SetStatic(st);
        pNew->SetPrivate(prv-100);
        pPrev = pNew;
    }
    return true;
}




////////////////////////////////////////////////////////////////////////////
// management of the compile stack
////////////////////////////////////////////////////////////////////////////

CBotProgram*    CBotCStack::m_prog    = NULL;            // init the static variable
int                CBotCStack::m_error   = 0;
int                CBotCStack::m_end      = 0;
CBotTypResult    CBotCStack::m_retTyp  = CBotTypResult(0);
//CBotToken*        CBotCStack::m_retClass= NULL;


CBotCStack::CBotCStack(CBotCStack* ppapa)
{
    m_next = NULL;
    m_prev = ppapa;

    if (ppapa == NULL)
    {
        m_error = 0;
        m_start = 0;
        m_end    = 0;
        m_bBlock = true;
    }
    else
    {
        m_start = ppapa->m_start;
        m_bBlock = false;
    }

    m_listVar = NULL;
    m_var      = NULL;
}

// destructor
CBotCStack::~CBotCStack()
{
    if (m_next != NULL) delete m_next;
    if (m_prev != NULL) m_prev->m_next = NULL;        // removes chain

    delete m_var;
    delete m_listVar;
}

// used only at compile
CBotCStack* CBotCStack::TokenStack(CBotToken* pToken, bool bBlock)
{
    if (m_next != NULL) return m_next;            // include on an existing stack

    CBotCStack*    p = new CBotCStack(this);
    m_next = p;                                    // channel element
    p->m_bBlock = bBlock;

    if (pToken != NULL) p->SetStartError(pToken->GetStart());

    return    p;
}


CBotInstr* CBotCStack::Return(CBotInstr* inst, CBotCStack* pfils)
{
    if ( pfils == this ) return inst;

    if (m_var != NULL) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = NULL;                        // not to destroy the variable

    if (m_error)
    {
        m_start = pfils->m_start;                // retrieves the position of the error
        m_end    = pfils->m_end;
    }

    delete pfils;
    return inst;
}

CBotFunction* CBotCStack::ReturnFunc(CBotFunction* inst, CBotCStack* pfils)
{
    if (m_var != NULL) delete m_var;            // value replaced?
    m_var = pfils->m_var;                        // result transmitted
    pfils->m_var = NULL;                        // not to destroy the variable

    if (m_error)
    {
        m_start = pfils->m_start;                // retrieves the position of the error
        m_end    = pfils->m_end;
    }

    delete pfils;
    return inst;
}

int CBotCStack::GetError(int& start, int& end)
{
    start = m_start;
    end      = m_end;
    return m_error;
}

int CBotCStack::GetError()
{
    return m_error;
}

// type of instruction on the stack
CBotTypResult CBotCStack::GetTypResult(int mode)
{
    if (m_var == NULL)
        return CBotTypResult(99);
    return    m_var->GetTypResult(mode);
}

// type of instruction on the stack
int CBotCStack::GetType(int mode)
{
    if (m_var == NULL)
        return 99;
    return    m_var->GetType(mode);
}

// pointer on the stack is in what class?
CBotClass* CBotCStack::GetClass()
{
    if ( m_var == NULL )
        return NULL;
    if ( m_var->GetType(1) != CBotTypPointer ) return NULL;

    return m_var->GetClass();
}

// type of instruction on the stack
void CBotCStack::SetType(CBotTypResult& type)
{
    if (m_var == NULL) return;
    m_var->SetType( type );
}

// seeks a variable on the stack
// the token may be a result of TokenTypVar (object of a class)
// or a pointer in the source

CBotVar* CBotCStack::FindVar(CBotToken* &pToken)
{
    CBotCStack*    p = this;
    CBotString    name = pToken->GetString();

    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (name == pp->GetName())
            {
                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return NULL;
}

CBotVar* CBotCStack::FindVar(CBotToken& Token)
{
    CBotToken*    pt = &Token;
    return FindVar(pt);
}

CBotVar* CBotCStack::CopyVar(CBotToken& Token)
{
    CBotVar*    pVar = FindVar( Token );

    if ( pVar == NULL) return NULL;

    CBotVar*    pCopy = CBotVar::Create( "", pVar->GetType() );
    pCopy->Copy(pVar);
    return    pCopy;
}

bool CBotCStack::IsOk()
{
    return (m_error == 0);
}


void CBotCStack::SetStartError( int pos )
{
    if ( m_error != 0) return;            // does not change existing error
    m_start = pos;
}

void CBotCStack::SetError(int n, int pos)
{
    if ( n!= 0 && m_error != 0) return;    // does not change existing error
    m_error = n;
    m_end    = pos;
}

void CBotCStack::SetError(int n, CBotToken* p)
{
    if (m_error) return;    // does not change existing error
    m_error = n;
    m_start    = p->GetStart();
    m_end    = p->GetEnd();
}

void CBotCStack::ResetError(int n, int start, int end)
{
    m_error = n;
    m_start    = start;
    m_end    = end;
}

bool CBotCStack::NextToken(CBotToken* &p)
{
    CBotToken*    pp = p;

    p = p->GetNext();
    if (p!=NULL) return true;

    SetError(TX_ENDOF, pp->GetEnd());
    return false;
}

void CBotCStack::SetBotCall(CBotProgram* p)
{
    m_prog = p;
}

CBotProgram* CBotCStack::GetBotCall()
{
    return m_prog;
}

void CBotCStack::SetRetType(CBotTypResult& type)
{
    m_retTyp = type;
}

CBotTypResult CBotCStack::GetRetType()
{
    return m_retTyp;
}

void CBotCStack::SetVar( CBotVar* var )
{
    if (m_var) delete m_var;    // replacement of a variable
    m_var = var;
}

// puts on the stack a copy of a variable
void CBotCStack::SetCopyVar( CBotVar* var )
{
    if (m_var) delete m_var;    // replacement of a variable

    if ( var == NULL ) return;
    m_var = CBotVar::Create("", var->GetTypResult(2));
    m_var->Copy( var );
}

CBotVar* CBotCStack::GetVar()
{
    return m_var;
}

void CBotCStack::AddVar(CBotVar* pVar)
{
    CBotCStack*    p = this;

    // returns to the father element
    while (p != NULL && p->m_bBlock == 0) p = p->m_prev;

    if ( p == NULL ) return;

    CBotVar**    pp = &p->m_listVar;
    while ( *pp != NULL ) pp = &(*pp)->m_next;

    *pp = pVar;                    // added after

#ifdef    _DEBUG
    if ( pVar->GetUniqNum() == 0 ) assert(0);
#endif
}

// test whether a variable is already defined locally

bool CBotCStack::CheckVarLocal(CBotToken* &pToken)
{
    CBotCStack*    p = this;
    CBotString    name = pToken->GetString();

    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (name == pp->GetName())
                return true;
            pp = pp->m_next;
        }
        if ( p->m_bBlock ) return false;
        p = p->m_prev;
    }
    return false;
}

CBotTypResult CBotCStack::CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent)
{
    nIdent = 0;
    CBotTypResult val(-1);

    val = CBotCall::CompileCall(p, ppVars, this, nIdent);
    if (val.GetType() < 0)
    {
        val = m_prog->GetFunctions()->CompileCall(p->GetString(), ppVars, nIdent);
        if ( val.GetType() < 0 )
        {
    //        pVar = NULL;                    // the error is not on a particular parameter
            SetError( -val.GetType(), p );
            val.SetType(-val.GetType());
            return val;
        }
    }
    return val;
}

// test if a procedure name is already defined somewhere

bool CBotCStack::CheckCall(CBotToken* &pToken, CBotDefParam* pParam)
{
    CBotString    name = pToken->GetString();

    if ( CBotCall::CheckCall(name) ) return true;

    CBotFunction*    pp = m_prog->GetFunctions();
    while ( pp != NULL )
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
        pp = pp->Next();
    }

    pp = CBotFunction::m_listPublic;
    while ( pp != NULL )
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
        pp = pp->m_nextpublic;
    }

    return false;
}

