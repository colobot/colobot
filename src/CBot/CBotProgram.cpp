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
#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotCall.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotUtils.h"
#include "CBot/CBotFileUtils.h"

#include "CBot/CBotInstr/CBotFunction.h"

#include "StringFunctions.h"

// Local include

// Global include
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::Compile( const char* program, std::vector<CBotString>& ListFonctions, void* pUser )
{
    int         error = 0;
    Stop();

//  delete      m_pClass;
    m_pClass->Purge();      // purge the old definitions of classes
                            // but without destroying the object
    m_pClass    = nullptr;
    delete      m_Prog;     m_Prog= nullptr;

    ListFonctions.clear();
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
            if (next->IsExtern()) ListFonctions.push_back(next->GetName()/* + next->GetParams()*/);
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::GetRunPos(const char* &FunctionName, int &start, int &end)
{
    FunctionName = nullptr;
    start = end = 0;
    if (m_pStack == nullptr) return false;

    m_pStack->GetRunPos(FunctionName, start, end);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotProgram::GetStackVars(const char* &FunctionName, int level)
{
    FunctionName = nullptr;
    if (m_pStack == nullptr) return nullptr;

    return m_pStack->GetStackVars(FunctionName, level);
}

////////////////////////////////////////////////////////////////////////////////
void CBotProgram::SetTimer(int n)
{
    CBotStack::SetTimer( n );
}

////////////////////////////////////////////////////////////////////////////////
int CBotProgram::GetError()
{
    return m_ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
void CBotProgram::SetIdent(long n)
{
    m_Ident = n;
}

////////////////////////////////////////////////////////////////////////////////
long CBotProgram::GetIdent()
{
    return m_Ident;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::GetError(int& code, int& start, int& end)
{
    code  = m_ErrorCode;
    start = m_ErrorStart;
    end   = m_ErrorEnd;
    return code > 0;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::GetError(int& code, int& start, int& end, CBotProgram* &pProg)
{
    code    = m_ErrorCode;
    start   = m_ErrorStart;
    end     = m_ErrorEnd;
    pProg   = this;
    return code > 0;
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotProgram::GetFunctions()
{
    return  m_Prog;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::AddFunction(const char* name,
                              bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                              CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
    // stores pointers to the two functions
    return CBotCall::AddFunction(name, rExec, rCompile);
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
CBotTypResult cSizeOf( CBotVar* &pVar, void* pUser )
{
    if ( pVar == nullptr ) return CBotTypResult( TX_LOWPARAM );
    if ( pVar->GetType() != CBotTypArrayPointer )
                        return CBotTypResult( TX_BADPARAM );
    return CBotTypResult( CBotTypInt );
}

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::DefineNum(const char* name, long val)
{
    return CBotToken::DefineNum(name, val);
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
int CBotProgram::GetVersion()
{
    return  CBOTVERSION;
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
void CBotProgram::Free()
{
    CBotToken::Free() ;
    CBotCall ::Free() ;
    CBotClass::Free() ;
}
