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

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotExternalCall.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotUtils.h"
#include "CBot/CBotFileUtils.h"

#include "CBot/CBotInstr/CBotFunction.h"

#include "CBot/stdlib/stdlib.h"

namespace CBot
{

CBotExternalCallList* CBotProgram::m_externalCalls = new CBotExternalCallList();

CBotProgram::CBotProgram()
{
}

CBotProgram::CBotProgram(CBotVar* thisVar)
: m_thisVar(thisVar)
{
}

CBotProgram::~CBotProgram()
{
//  delete  m_classes;
    m_classes->Purge();
    m_classes = nullptr;

    CBotClass::FreeLock(this);

    delete m_functions;
    m_stack->Delete();
}

bool CBotProgram::Compile(const std::string& program, std::vector<std::string>& functions, void* pUser)
{
    // Cleanup the previously compiled program
    Stop();

//  delete      m_classes;
    m_classes->Purge();      // purge the old definitions of classes
                            // but without destroying the object
    m_classes = nullptr;
    delete m_functions; m_functions = nullptr;

    functions.clear();
    m_error = CBotNoErr;

    // Step 1. Process the code into tokens
    auto tokens = CBotToken::CompileTokens(program);
    if (tokens == nullptr) return false;

    auto pStack = std::unique_ptr<CBotCStack>(new CBotCStack(nullptr));
    CBotToken* p = tokens.get()->GetNext();                 // skips the first token (separator)

    pStack->SetProgram(this);                               // defined used routines
    m_externalCalls->SetUserPtr(pUser);

    // Step 2. Find all function and class definitions
    while ( pStack->IsOk() && p != nullptr && p->GetType() != 0)
    {
        if ( IsOfType(p, ID_SEP) ) continue;                // semicolons lurking

        if ( p->GetType() == ID_CLASS ||
            ( p->GetType() == ID_PUBLIC && p->GetNext()->GetType() == ID_CLASS ))
        {
            CBotClass*  nxt = CBotClass::Compile1(p, pStack.get());
            if (m_classes == nullptr ) m_classes = nxt;
            else m_classes->AddNext(nxt);
        }
        else
        {
            CBotFunction*   next = CBotFunction::Compile1(p, pStack.get(), nullptr);
            if (m_functions == nullptr ) m_functions = next;
            else m_functions->AddNext(next);
        }
    }
    if ( !pStack->IsOk() )
    {
        m_error = pStack->GetError(m_errorStart, m_errorEnd);
        delete m_functions;
        m_functions = nullptr;
        return false;
    }

    // Step 3. Real compilation
//  CBotFunction*   temp = nullptr;
    CBotFunction*   next = m_functions;      // rewind the list

    p  = tokens.get()->GetNext();                             // returns to the beginning

    while ( pStack->IsOk() && p != nullptr && p->GetType() != 0 )
    {
        if ( IsOfType(p, ID_SEP) ) continue;                // semicolons lurking

        if ( p->GetType() == ID_CLASS ||
            ( p->GetType() == ID_PUBLIC && p->GetNext()->GetType() == ID_CLASS ))
        {
            m_bCompileClass = true;
            CBotClass::Compile(p, pStack.get());                  // completes the definition of the class
        }
        else
        {
            m_bCompileClass = false;
            CBotFunction::Compile(p, pStack.get(), next);
            if (next->IsExtern()) functions.push_back(next->GetName()/* + next->GetParams()*/);
            next->m_pProg = this;                           // keeps pointers to the module
            next = next->Next();
        }
    }

//  delete m_Prog;          // the list of first pass
//  m_Prog = temp;          // list of the second pass

    if ( !pStack->IsOk() )
    {
        m_error = pStack->GetError(m_errorStart, m_errorEnd);
        delete m_functions;
        m_functions = nullptr;
    }

    return (m_functions != nullptr);
}

bool CBotProgram::Start(const std::string& name)
{
    Stop();

    m_entryPoint = m_functions;
    while (m_entryPoint != nullptr)
    {
        if (m_entryPoint->GetName() == name ) break;
        m_entryPoint = m_entryPoint->m_next;
    }

    if (m_entryPoint == nullptr)
    {
        m_error = CBotErrNoRun;
        return false;
    }

    m_stack = CBotStack::AllocateStack();
    m_stack->SetProgram(this);

    return true; // we are ready for Run()
}

bool CBotProgram::GetPosition(const std::string& name, int& start, int& stop, CBotGet modestart, CBotGet modestop)
{
    CBotFunction* p = m_functions;
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
    if (m_stack == nullptr || m_entryPoint == nullptr)
    {
        m_error = CBotErrNoRun;
        return true;
    }

    m_error = CBotNoErr;

    m_stack->SetUserPtr(pUser);
    if ( timer >= 0 ) m_stack->SetTimer(timer); // TODO: Check if changing order here fixed ipf()
    m_stack->Reset();                         // reset the possible previous error, and resets the timer

    m_stack->SetProgram(this);                     // bases for routines

    // resumes execution on the top of the stack
    bool ok = m_stack->Execute();
    if (ok)
    {
        // returns to normal execution
        ok = m_entryPoint->Execute(nullptr, m_stack, m_thisVar);
    }

    // completed on a mistake?
    if (!ok && !m_stack->IsOk())
    {
        m_error = m_stack->GetError(m_errorStart, m_errorEnd);
        m_stack->Delete();
        m_stack = nullptr;
        return true;                                // execution is finished!
    }

    if ( ok ) m_entryPoint = nullptr;                        // more function in execution
    return ok;
}

void CBotProgram::Stop()
{
    m_stack->Delete();
    m_stack = nullptr;
    m_entryPoint = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::GetRunPos(std::string& functionName, int& start, int& end)
{
    functionName = "";
    start = end = 0;
    if (m_stack == nullptr) return false;

    m_stack->GetRunPos(functionName, start, end);
    return true;
}


CBotVar* CBotProgram::GetStackVars(std::string& functionName, int level)
{
    functionName.clear();
    if (m_stack == nullptr) return nullptr;

    return m_stack->GetStackVars(functionName, level);
}

////////////////////////////////////////////////////////////////////////////////
void CBotProgram::SetTimer(int n)
{
    CBotStack::SetTimer( n );
}

////////////////////////////////////////////////////////////////////////////////
CBotError CBotProgram::GetError()
{
    return m_error;
}


bool CBotProgram::GetError(CBotError& code, int& start, int& end)
{
    code  = m_error;
    start = m_errorStart;
    end   = m_errorEnd;
    return code > 0;
}


bool CBotProgram::GetError(CBotError& code, int& start, int& end, CBotProgram*& pProg)
{
    code    = m_error;
    start   = m_errorStart;
    end     = m_errorEnd;
    pProg   = this;
    return code > 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotProgram::GetFunctions()
{
    return m_functions;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult cSizeOf( CBotVar* &pVar, void* pUser )
{
    if ( pVar == nullptr ) return CBotTypResult( CBotErrLowParam );
    if ( pVar->GetType() != CBotTypArrayPointer )
                        return CBotTypResult( CBotErrBadParam );
    return CBotTypResult( CBotTypInt );
}

bool rSizeOf( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    if ( pVar == nullptr ) return CBotErrLowParam;

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
bool CBotProgram::AddFunction(const std::string& name,
                              bool rExec(CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),
                              CBotTypResult rCompile(CBotVar*& pVar, void* pUser))
{
    return m_externalCalls->AddFunction(name, std::unique_ptr<CBotExternalCall>(new CBotExternalCallDefault(rExec, rCompile)));
}

bool CBotProgram::DefineNum(const std::string& name, long val)
{
    CBotToken::DefineNum(name, val);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotProgram::SaveState(FILE* pf)
{
    if (!WriteWord( pf, CBOTVERSION)) return false;


    if (m_stack != nullptr )
    {
        if (!WriteWord( pf, 1)) return false;
        if (!WriteString( pf, m_entryPoint->GetName() )) return false;
        if (!m_stack->SaveState(pf)) return false;
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
    std::string      s;

    Stop();

    if (!ReadWord( pf, w )) return false;
    if ( w != CBOTVERSION ) return false;

    if (!ReadWord( pf, w )) return false;
    if ( w == 0 ) return true;

    if (!ReadString( pf, s )) return false;
    Start(s);       // point de reprise

    m_stack->Delete();
    m_stack = nullptr;

    // retrieves the stack from the memory
    // uses a nullptr pointer (m_stack) but it's ok like that
    if (!m_stack->RestoreState(pf, m_stack)) return false;
    m_stack->SetProgram(this);                     // bases for routines

    // restored some states in the stack according to the structure
    m_entryPoint->RestoreState(nullptr, m_stack, m_thisVar);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

int CBotProgram::GetVersion()
{
    return  CBOTVERSION;
}

void CBotProgram::Init()
{
    CBotProgram::DefineNum("CBotErrZeroDiv",    CBotErrZeroDiv);     // division by zero
    CBotProgram::DefineNum("CBotErrNotInit",    CBotErrNotInit);     // uninitialized variable
    CBotProgram::DefineNum("CBotErrBadThrow",   CBotErrBadThrow);    // throw a negative value
    CBotProgram::DefineNum("CBotErrNoRetVal",   CBotErrNoRetVal);    // function did not return results
    CBotProgram::DefineNum("CBotErrNoRun",      CBotErrNoRun);       // active Run () without a function // TODO: Is this actually a runtime error?
    CBotProgram::DefineNum("CBotErrUndefFunc",  CBotErrUndefFunc);   // Calling a function that no longer exists
    CBotProgram::DefineNum("CBotErrNotClass",   CBotErrNotClass);    // Class no longer exists
    CBotProgram::DefineNum("CBotErrNull",       CBotErrNull);        // Attempted to use a null pointer
    CBotProgram::DefineNum("CBotErrNan",        CBotErrNan);         // Can't do operations on nan
    CBotProgram::DefineNum("CBotErrOutArray",   CBotErrOutArray);    // Attempted access out of bounds of an array
    CBotProgram::DefineNum("CBotErrStackOver",  CBotErrStackOver);   // Stack overflow
    CBotProgram::DefineNum("CBotErrDeletedPtr", CBotErrDeletedPtr);  // Attempted to use deleted object

    CBotProgram::AddFunction("sizeof", rSizeOf, cSizeOf);

    InitStringFunctions();
    InitMathFunctions();
    InitFileFunctions();
}

void CBotProgram::Free()
{
    CBotToken::ClearDefineNum();
    m_externalCalls->Clear();
    CBotClass::ClearPublic();
}

CBotExternalCallList* CBotProgram::GetExternalCalls()
{
    return m_externalCalls;
}

} // namespace CBot
