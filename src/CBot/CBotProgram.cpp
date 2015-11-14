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

//////////////////////////////////////////////////////////////////////
// database management of CBoT program

#include "CBot.h"

#include "CBotStack.h"

#include <stdio.h>

CBotProgram::CBotProgram()
{
    m_Prog      = nullptr;
    m_pRun      = nullptr;
    m_pClass    = nullptr;
    m_pStack    = nullptr;
    m_pInstance = nullptr;

    m_ErrorCode = 0;
    m_Ident     = 0;
}

CBotProgram::CBotProgram(CBotVar* pInstance)
{
    m_Prog      = nullptr;
    m_pRun      = nullptr;
    m_pClass    = nullptr;
    m_pStack    = nullptr;
    m_pInstance = pInstance;

    m_ErrorCode = 0;
    m_Ident     = 0;
}


CBotProgram::~CBotProgram()
{
//  delete  m_pClass;
    m_pClass->Purge();
    m_pClass    = nullptr;

    CBotClass::FreeLock(this);

    delete  m_Prog;
#if STACKMEM
    m_pStack->Delete();
#else
    delete  m_pStack;
#endif
}


bool CBotProgram::Compile( const char* program, CBotStringArray& ListFonctions, void* pUser )
{
    int         error = 0;
    Stop();

//  delete      m_pClass;
    m_pClass->Purge();      // purge the old definitions of classes
                            // but without destroying the object
    m_pClass    = nullptr;
    delete      m_Prog;     m_Prog= nullptr;

    ListFonctions.SetSize(0);
    m_ErrorCode = 0;

    // transforms the program in Tokens
    CBotToken*  pBaseToken = CBotToken::CompileTokens(program, error);
    if ( pBaseToken == nullptr ) return false;


    CBotCStack* pStack      = new CBotCStack(nullptr);
    CBotToken*  p  = pBaseToken->GetNext();                 // skips the first token (separator)

    pStack->SetBotCall(this);                               // defined used routines
    CBotCall::SetPUser(pUser);

    // first made a quick pass just to take the headers of routines and classes
    while ( pStack->IsOk() && p != nullptr && p->GetType() != 0)
    {
        if ( IsOfType(p, ID_SEP) ) continue;                // semicolons lurking

        if ( p->GetType() == ID_CLASS ||
            ( p->GetType() == ID_PUBLIC && p->GetNext()->GetType() == ID_CLASS ))
        {
            CBotClass*  nxt = CBotClass::Compile1(p, pStack);
            if (m_pClass == nullptr ) m_pClass = nxt;
            else m_pClass->AddNext(nxt);
        }
        else
        {
            CBotFunction*   next = CBotFunction::Compile1(p, pStack, nullptr);
            if (m_Prog == nullptr ) m_Prog = next;
            else m_Prog->AddNext(next);
        }
    }
    if ( !pStack->IsOk() )
    {
        m_ErrorCode = pStack->GetError(m_ErrorStart, m_ErrorEnd);
        delete m_Prog;
        m_Prog = nullptr;
        delete pBaseToken;
        return false;
    }

//  CBotFunction*   temp = nullptr;
    CBotFunction*   next = m_Prog;      // rewind the list

    p  = pBaseToken->GetNext();                             // returns to the beginning

    while ( pStack->IsOk() && p != nullptr && p->GetType() != 0 )
    {
        if ( IsOfType(p, ID_SEP) ) continue;                // semicolons lurking

        if ( p->GetType() == ID_CLASS ||
            ( p->GetType() == ID_PUBLIC && p->GetNext()->GetType() == ID_CLASS ))
        {
            m_bCompileClass = true;
            CBotClass::Compile(p, pStack);                  // completes the definition of the class
        }
        else
        {
            m_bCompileClass = false;
            CBotFunction::Compile(p, pStack, next);
            if (next->IsExtern()) ListFonctions.Add(next->GetName()/* + next->GetParams()*/);
            next->m_pProg = this;                           // keeps pointers to the module
            next = next->Next();
        }
    }

//  delete m_Prog;          // the list of first pass
//  m_Prog = temp;          // list of the second pass

    if ( !pStack->IsOk() )
    {
        m_ErrorCode = pStack->GetError(m_ErrorStart, m_ErrorEnd);
        delete m_Prog;
        m_Prog = nullptr;
    }

    delete pBaseToken;
    delete pStack;

    return (m_Prog != nullptr);
}


bool CBotProgram::Start(const char* name)
{
#if STACKMEM
    m_pStack->Delete();
#else
    delete m_pStack;
#endif
    m_pStack = nullptr;

    m_pRun = m_Prog;
    while (m_pRun != nullptr)
    {
        if ( m_pRun->GetName() == name ) break;
        m_pRun = m_pRun->m_next;
    }

    if ( m_pRun == nullptr )
    {
        m_ErrorCode = TX_NORUN;
        return false;
    }

#if STACKMEM
    m_pStack = CBotStack::FirstStack();
#else
    m_pStack = new CBotStack(nullptr);                 // creates an execution stack
#endif

    m_pStack->SetBotCall(this);                     // bases for routines

    return true;                                    // we are ready for Run ()
}

bool CBotProgram::GetPosition(const char* name, int& start, int& stop, CBotGet modestart, CBotGet modestop)
{
    CBotFunction* p = m_Prog;
    while (p != nullptr)
    {
        if ( p->GetName() == name ) break;
        p = p->m_next;
    }

    if ( p == nullptr ) return false;

    p->GetPosition(start, stop, modestart, modestop);
    return true;
}

bool CBotProgram::Run(void* pUser, int timer)
{
    bool    ok;

    if (m_pStack == nullptr || m_pRun == nullptr) goto error;

    m_ErrorCode = 0;

    m_pStack->Reset(pUser);                         // empty the possible previous error, and resets the timer
    if ( timer >= 0 ) m_pStack->SetTimer(timer);

    m_pStack->SetBotCall(this);                     // bases for routines

#if STACKRUN
    // resumes execution on the top of the stack
    ok = m_pStack->Execute();
    if ( ok )
    {
#ifdef  _DEBUG
        CBotVar*    ppVar[3];
        ppVar[0] = CBotVar::Create("aa", CBotTypInt);
        ppVar[1] = CBotVar::Create("bb", CBotTypInt);
        ppVar[2] = nullptr;
        ok = m_pRun->Execute(ppVar, m_pStack, m_pInstance);
#else
        // returns to normal execution
        ok = m_pRun->Execute(nullptr, m_pStack, m_pInstance);
#endif
    }
#else
    ok = m_pRun->Execute(nullptr, m_pStack, m_pInstance);
#endif

    // completed on a mistake?
    if (!ok && !m_pStack->IsOk())
    {
        m_ErrorCode = m_pStack->GetError(m_ErrorStart, m_ErrorEnd);
#if STACKMEM
        m_pStack->Delete();
#else
        delete m_pStack;
#endif
        m_pStack = nullptr;
        return true;                                // execution is finished!
    }

    if ( ok ) m_pRun = nullptr;                        // more function in execution
    return ok;

error:
    m_ErrorCode = TX_NORUN;
    return true;
}

void CBotProgram::Stop()
{
#if STACKMEM
    m_pStack->Delete();
#else
    delete m_pStack;
#endif
    m_pStack = nullptr;
    m_pRun = nullptr;
}



bool CBotProgram::GetRunPos(const char* &FunctionName, int &start, int &end)
{
    FunctionName = nullptr;
    start = end = 0;
    if (m_pStack == nullptr) return false;

    m_pStack->GetRunPos(FunctionName, start, end);
    return true;
}

CBotVar* CBotProgram::GetStackVars(const char* &FunctionName, int level)
{
    FunctionName = nullptr;
    if (m_pStack == nullptr) return nullptr;

    return m_pStack->GetStackVars(FunctionName, level);
}

void CBotProgram::SetTimer(int n)
{
    CBotStack::SetTimer( n );
}

int CBotProgram::GetError()
{
    return m_ErrorCode;
}

void CBotProgram::SetIdent(long n)
{
    m_Ident = n;
}

long CBotProgram::GetIdent()
{
    return m_Ident;
}

bool CBotProgram::GetError(int& code, int& start, int& end)
{
    code  = m_ErrorCode;
    start = m_ErrorStart;
    end   = m_ErrorEnd;
    return code > 0;
}

bool CBotProgram::GetError(int& code, int& start, int& end, CBotProgram* &pProg)
{
    code    = m_ErrorCode;
    start   = m_ErrorStart;
    end     = m_ErrorEnd;
    pProg   = this;
    return code > 0;
}

CBotString CBotProgram::GetErrorText(int code)
{
    CBotString TextError;

    TextError.LoadString( code );
    if (TextError.IsEmpty())
    {
        char    buf[100];
        sprintf(buf, "Exception numéro %d.", code);
        TextError = buf;
    }
    return TextError;
}


CBotFunction* CBotProgram::GetFunctions()
{
    return  m_Prog;
}

bool CBotProgram::AddFunction(const char* name,
                              bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                              CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
    // stores pointers to the two functions
    return CBotCall::AddFunction(name, rExec, rCompile);
}


bool WriteWord(FILE* pf, unsigned short w)
{
    size_t  lg;

    lg = fwrite(&w, sizeof( unsigned short ), 1, pf );

    return (lg == 1);
}

bool ReadWord(FILE* pf, unsigned short& w)
{
    size_t  lg;

    lg = fread(&w, sizeof( unsigned short ), 1, pf );

    return (lg == 1);
}

bool WriteFloat(FILE* pf, float w)
{
    size_t  lg;

    lg = fwrite(&w, sizeof( float ), 1, pf );

    return (lg == 1);
}

bool ReadFloat(FILE* pf, float& w)
{
    size_t  lg;

    lg = fread(&w, sizeof( float ), 1, pf );

    return (lg == 1);
}

bool WriteLong(FILE* pf, long w)
{
    size_t  lg;

    lg = fwrite(&w, sizeof( long ), 1, pf );

    return (lg == 1);
}

bool ReadLong(FILE* pf, long& w)
{
    size_t  lg;

    lg = fread(&w, sizeof( long ), 1, pf );

    return (lg == 1);
}

bool WriteString(FILE* pf, CBotString s)
{
    size_t  lg1, lg2;

    lg1 = s.GetLength();
    if (!WriteWord(pf, lg1)) return false;

    lg2 = fwrite(s, 1, lg1, pf );
    return (lg1 == lg2);
}

bool ReadString(FILE* pf, CBotString& s)
{
    unsigned short  w;
    char    buf[1000];
    size_t  lg1, lg2;

    if (!ReadWord(pf, w)) return false;
    lg1 = w;
    lg2 = fread(buf, 1, lg1, pf );
    buf[lg2] = 0;

    s = buf;
    return (lg1 == lg2);
}

bool WriteType(FILE* pf, CBotTypResult type)
{
    int typ = type.GetType();
    if ( typ == CBotTypIntrinsic ) typ = CBotTypClass;
    if ( !WriteWord(pf, typ) ) return false;
    if ( typ == CBotTypClass )
    {
        CBotClass* p = type.GetClass();
        if ( !WriteString(pf, p->GetName()) ) return false;
    }
    if ( type.Eq( CBotTypArrayBody ) ||
         type.Eq( CBotTypArrayPointer ) )
    {
        if ( !WriteWord(pf, type.GetLimite()) ) return false;
        if ( !WriteType(pf, type.GetTypElem()) ) return false;
    }
    return true;
}

bool ReadType(FILE* pf, CBotTypResult& type)
{
    unsigned short  w, ww;
    if ( !ReadWord(pf, w) ) return false;
    type.SetType(w);

    if ( type.Eq( CBotTypIntrinsic ) )
    {
        type = CBotTypResult( w, "point" );
    }

    if ( type.Eq( CBotTypClass ) )
    {
        CBotString  s;
        if ( !ReadString(pf, s) ) return false;
        type = CBotTypResult( w, s );
    }

    if ( type.Eq( CBotTypArrayPointer ) ||
         type.Eq( CBotTypArrayBody ) )
    {
        CBotTypResult   r;
        if ( !ReadWord(pf, ww) ) return false;
        if ( !ReadType(pf, r) ) return false;
        type = CBotTypResult( w, r );
        type.SetLimite(static_cast<short>(ww));
    }
    return true;
}


bool CBotProgram::DefineNum(const char* name, long val)
{
    return CBotToken::DefineNum(name, val);
}


bool CBotProgram::SaveState(FILE* pf)
{
    if (!WriteWord( pf, CBOTVERSION)) return false;


    if ( m_pStack != nullptr )
    {
        if (!WriteWord( pf, 1)) return false;
        if (!WriteString( pf, m_pRun->GetName() )) return false;
        if (!m_pStack->SaveState(pf)) return false;
    }
    else
    {
        if (!WriteWord( pf, 0)) return false;
    }
    return true;
}


bool CBotProgram::RestoreState(FILE* pf)
{
    unsigned short  w;
    CBotString      s;

    Stop();

    if (!ReadWord( pf, w )) return false;
    if ( w != CBOTVERSION ) return false;

    if (!ReadWord( pf, w )) return false;
    if ( w == 0 ) return true;

    if (!ReadString( pf, s )) return false;
    Start(s);       // point de reprise

#if STACKMEM
    m_pStack->Delete();
#else
    delete m_pStack;
#endif
    m_pStack = nullptr;

    // retrieves the stack from the memory
    // uses a nullptr pointer (m_pStack) but it's ok like that
    if (!m_pStack->RestoreState(pf, m_pStack)) return false;
    m_pStack->SetBotCall(this);                     // bases for routines

    // restored some states in the stack according to the structure
    m_pRun->RestoreState(nullptr, m_pStack, m_pInstance);
    return true;
}

int CBotProgram::GetVersion()
{
    return  CBOTVERSION;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

CBotCall* CBotCall::m_ListCalls = nullptr;

CBotCall::CBotCall(const char* name,
                   bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                   CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
    m_name       = name;
    m_rExec      = rExec;
    m_rComp      = rCompile;
    m_next       = nullptr;
    m_nFuncIdent = CBotVar::NextUniqNum();
}

CBotCall::~CBotCall()
{
    if (m_next) delete m_next;
    m_next = nullptr;
}

void CBotCall::Free()
{
    delete CBotCall::m_ListCalls;
}

bool CBotCall::AddFunction(const char* name,
                           bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                           CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
    CBotCall*   p = m_ListCalls;
    CBotCall*   pp = nullptr;

    if ( p != nullptr ) while ( p->m_next != nullptr )
    {
        if ( p->GetName() == name )
        {
            // frees redefined function
            if ( pp ) pp->m_next = p->m_next;
            else      m_ListCalls = p->m_next;
            pp = p;
            p = p->m_next;
            pp->m_next = nullptr;  // not to destroy the following list
            delete pp;
            continue;
        }
        pp = p;             // previous pointer
        p = p->m_next;
    }

    pp = new CBotCall(name, rExec, rCompile);

    if (p) p->m_next = pp;
    else m_ListCalls = pp;

    return true;
}


// transforms the array of pointers to variables
// in a chained list of variables
CBotVar* MakeListVars(CBotVar** ppVars, bool bSetVal=false)
{
    int     i = 0;
    CBotVar*    pVar = nullptr;

    while( true )
    {
//        ppVars[i];
        if ( ppVars[i] == nullptr ) break;

        CBotVar*    pp = CBotVar::Create(ppVars[i]);
        if (bSetVal) pp->Copy(ppVars[i]);
        else
            if ( ppVars[i]->GetType() == CBotTypPointer )
                pp->SetClass( ppVars[i]->GetClass());
// copy the pointer according to indirections
        if (pVar == nullptr) pVar = pp;
        else pVar->AddNext(pp);
        i++;
    }
    return pVar;
}

// is acceptable by a call procedure name
// and given parameters

CBotTypResult CBotCall::CompileCall(CBotToken* &p, CBotVar** ppVar, CBotCStack* pStack, long& nIdent)
{
    nIdent = 0;
    CBotCall*   pt = m_ListCalls;
    CBotString  name = p->GetString();

    while ( pt != nullptr )
    {
        if ( pt->m_name == name )
        {
            CBotVar*    pVar = MakeListVars(ppVar);
            CBotVar*    pVar2 = pVar;
            CBotTypResult r = pt->m_rComp(pVar2, m_pUser);
            int ret = r.GetType();

            // if a class is returned, it is actually a pointer
            if ( ret == CBotTypClass ) r.SetType( ret = CBotTypPointer );

            if ( ret > 20 )
            {
                if (pVar2) pStack->SetError(ret, p /*pVar2->GetToken()*/ );
            }
            delete pVar;
            nIdent = pt->m_nFuncIdent;
            return r;
        }
        pt = pt->m_next;
    }
    return -1;
}

void* CBotCall::m_pUser = nullptr;

void CBotCall::SetPUser(void* pUser)
{
    m_pUser = pUser;
}

bool CBotCall::CheckCall(const char* name)
{
    CBotCall* p = m_ListCalls;

    while ( p != nullptr )
    {
        if ( name == p->GetName() ) return true;
        p = p->m_next;
    }
    return false;
}



CBotString CBotCall::GetName()
{
    return  m_name;
}

CBotCall* CBotCall::Next()
{
    return  m_next;
}


int CBotCall::DoCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack, CBotTypResult& rettype)
{
    CBotCall*   pt = m_ListCalls;

    if ( nIdent ) while ( pt != nullptr )
    {
        if ( pt->m_nFuncIdent == nIdent )
        {
            goto fund;
        }
        pt = pt->m_next;
    }

    pt = m_ListCalls;

    if ( token != nullptr )
    {
        CBotString name = token->GetString();
        while ( pt != nullptr )
        {
            if ( pt->m_name == name )
            {
                nIdent = pt->m_nFuncIdent;
                goto fund;
            }
            pt = pt->m_next;
        }
    }

    return -1;

fund:
#if !STACKRUN
    // lists the parameters depending on the contents of the stack (pStackVar)

    CBotVar*    pVar = MakeListVars(ppVar, true);
    CBotVar*    pVarToDelete = pVar;

    // creates a variable to the result
    CBotVar*    pResult = rettype.Eq(0) ? nullptr : CBotVar::Create("", rettype);

    CBotVar*    pRes = pResult;
    int         Exception = 0;
    int res = pt->m_rExec(pVar, pResult, Exception, pStack->GetPUser());

    if ( pResult != pRes ) delete pRes; // different result if made
    delete pVarToDelete;

    if (res == false)
    {
        if (Exception!=0)
        {
            pStack->SetError(Exception, token);
        }
        delete pResult;
        return false;
    }
    pStack->SetVar(pResult);

    if ( rettype.GetType() > 0 && pResult == nullptr )
    {
        pStack->SetError(TX_NORETVAL, token);
    }
    nIdent = pt->m_nFuncIdent;
    return true;

#else

    CBotStack*  pile = pStack->AddStackEOX(pt);
    if ( pile == EOX ) return true;

    // lists the parameters depending on the contents of the stack (pStackVar)

    CBotVar*    pVar = MakeListVars(ppVar, true);
//    CBotVar*    pVarToDelete = pVar;

    // creates a variable to the result
    CBotVar*    pResult = rettype.Eq(0) ? nullptr : CBotVar::Create("", rettype);

    pile->SetVar( pVar );

    CBotStack*  pile2 = pile->AddStack();
    pile2->SetVar( pResult );

    pile->SetError(0, token);           // for the position on error + away
    return pt->Run( pStack );

#endif

}

#if STACKRUN

bool CBotCall::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack)
{
    CBotCall*   pt = m_ListCalls;

    {
        CBotString name = token->GetString();
        while ( pt != nullptr )
        {
            if ( pt->m_name == name )
            {
                nIdent = pt->m_nFuncIdent;

                CBotStack*  pile = pStack->RestoreStackEOX(pt);
                if ( pile == nullptr ) return true;

 //               CBotStack*  pile2 = pile->RestoreStack();
                pile->RestoreStack();
                return true;
            }
            pt = pt->m_next;
        }
    }

    return false;
}

bool CBotCall::Run(CBotStack* pStack)
{
    CBotStack*  pile = pStack->AddStackEOX(this);
    if ( pile == EOX ) return true;
    CBotVar*    pVar = pile->GetVar();

    CBotStack*  pile2 = pile->AddStack();
    CBotVar*    pResult = pile2->GetVar();
    CBotVar*    pRes = pResult;

    int         Exception = 0;
    int res = m_rExec(pVar, pResult, Exception, pStack->GetPUser());

    if (res == false)
    {
        if (Exception!=0)
        {
            pStack->SetError(Exception);
        }
        if ( pResult != pRes ) delete pResult;  // different result if made
        return false;
    }

    if ( pResult != nullptr ) pStack->SetCopyVar( pResult );
    if ( pResult != pRes ) delete pResult;  // different result if made

    return true;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////

CBotCallMethode::CBotCallMethode(const char* name,
                   bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                   CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar))
{
    m_name       = name;
    m_rExec      = rExec;
    m_rComp      = rCompile;
    m_next       = nullptr;
    m_nFuncIdent = CBotVar::NextUniqNum();
}

CBotCallMethode::~CBotCallMethode()
{
    delete m_next;
    m_next = nullptr;
}

// is acceptable by a call procedure name
// and given parameters

CBotTypResult CBotCallMethode::CompileCall(const char* name, CBotVar* pThis,
                                           CBotVar** ppVar, CBotCStack* pStack,
                                           long& nIdent)
{
    CBotCallMethode*    pt = this;
    nIdent = 0;

    while ( pt != nullptr )
    {
        if ( pt->m_name == name )
        {
            CBotVar*    pVar = MakeListVars(ppVar, true);
            CBotVar*    pVar2 = pVar;
            CBotTypResult r = pt->m_rComp(pThis, pVar2);
            int ret = r.GetType();
            if ( ret > 20 )
            {
                if (pVar2) pStack->SetError(ret, pVar2->GetToken());
            }
            delete pVar;
            nIdent = pt->m_nFuncIdent;
            return r;
        }
        pt = pt->m_next;
    }
    return CBotTypResult(-1);
}


CBotString CBotCallMethode::GetName()
{
    return  m_name;
}

CBotCallMethode* CBotCallMethode::Next()
{
    return  m_next;
}

void CBotCallMethode::AddNext(CBotCallMethode* pt)
{
    CBotCallMethode* p = this;
    while ( p->m_next != nullptr ) p = p->m_next;

    p->m_next = pt;
}


int CBotCallMethode::DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotVar* &pResult, CBotStack* pStack, CBotToken* pToken)
{
    CBotCallMethode*    pt = this;

    // search by the identifier

    if ( nIdent ) while ( pt != nullptr )
    {
        if ( pt->m_nFuncIdent == nIdent )
        {
            // lists the parameters depending on the contents of the stack (pStackVar)

            CBotVar*    pVar = MakeListVars(ppVars, true);
            CBotVar*    pVarToDelete = pVar;

            // then calls the routine external to the module

            int         Exception = 0;
            int res = pt->m_rExec(pThis, pVar, pResult, Exception, pStack->GetPUser());
            pStack->SetVar(pResult);

            if (res == false)
            {
                if (Exception!=0)
                {
//                  pStack->SetError(Exception, pVar->GetToken());
                    pStack->SetError(Exception, pToken);
                }
                delete pVarToDelete;
                return false;
            }
            delete pVarToDelete;
            return true;
        }
        pt = pt->m_next;
    }

    // search by name

    while ( pt != nullptr )
    {
        if ( pt->m_name == name )
        {
            // lists the parameters depending on the contents of the stack (pStackVar)

            CBotVar*    pVar = MakeListVars(ppVars, true);
            CBotVar*    pVarToDelete = pVar;

            int         Exception = 0;
            int res = pt->m_rExec(pThis, pVar, pResult, Exception, pStack->GetPUser());
            pStack->SetVar(pResult);

            if (res == false)
            {
                if (Exception!=0)
                {
//                  pStack->SetError(Exception, pVar->GetToken());
                    pStack->SetError(Exception, pToken);
                }
                delete pVarToDelete;
                return false;
            }
            delete pVarToDelete;
            nIdent = pt->m_nFuncIdent;
            return true;
        }
        pt = pt->m_next;
    }

    return -1;
}

bool rSizeOf( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    if ( pVar == nullptr ) return TX_LOWPARAM;

    int i = 0;
    pVar = pVar->GetItemList();

    while ( pVar != nullptr )
    {
        i++;
        pVar = pVar->GetNext();
    }

    pResult->SetValInt(i);
    return true;
}

CBotTypResult cSizeOf( CBotVar* &pVar, void* pUser )
{
    if ( pVar == nullptr ) return CBotTypResult( TX_LOWPARAM );
    if ( pVar->GetType() != CBotTypArrayPointer )
                        return CBotTypResult( TX_BADPARAM );
    return CBotTypResult( CBotTypInt );
}


// TODO: Refactor this - including .cpp files is bad
#include "StringFunctions.cpp"

void CBotProgram::Init()
{
    CBotToken::DefineNum("CBotErrZeroDiv",       TX_DIVZERO);     // division by zero
    CBotToken::DefineNum("CBotErrNotInit",       TX_NOTINIT);     // uninitialized variable
    CBotToken::DefineNum("CBotErrBadThrow",      TX_BADTHROW);    // throw a negative value
    //CBotToken::DefineNum("CBotErrNoRetVal",      6003);           // function did not return results // TODO: Not used. I'm pretty sure not returning a value crashes the game :P
    CBotToken::DefineNum("CBotErrNoRun",         TX_NORUN);       // active Run () without a function // TODO: Is this actually a runtime error?
    CBotToken::DefineNum("CBotErrUndefFunc",     TX_NOCALL);      // Calling a function that no longer exists
    CBotToken::DefineNum("CBotErrUndefClass",    TX_NOCLASS);     // Class no longer exists
    CBotToken::DefineNum("CBotErrNullPointer",   TX_NULLPT);      // Attempted to use a null pointer
    CBotToken::DefineNum("CBotErrNan",           TX_OPNAN);       // Can't do operations on nan
    CBotToken::DefineNum("CBotErrOutOfBounds",   TX_OUTARRAY);    // Attempted access out of bounds of an array
    CBotToken::DefineNum("CBotErrStackOverflow", TX_STACKOVER);   // Stack overflow
    CBotToken::DefineNum("CBotErrDeletedObject", TX_DELETEDPT);   // Attempted to use deleted object

    CBotProgram::AddFunction("sizeof", rSizeOf, cSizeOf );

    InitStringFunctions();
}

void CBotProgram::Free()
{
    CBotToken::Free() ;
    CBotCall ::Free() ;
    CBotClass::Free() ;
}
