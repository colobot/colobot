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

#include "CBot/CBotInstr/CBotFunction.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"

#include "CBot/CBotInstr/CBotBlock.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotEmpty.h"
#include "CBot/CBotInstr/CBotListArray.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotDefParam.h"
#include "CBot/CBotUtils.h"

#include "CBot/CBotVar/CBotVar.h"

#include <cassert>
#include <sstream>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotFunction::CBotFunction()
{
    m_param = nullptr;            // empty parameter list
    m_block = nullptr;            // the instruction block
    m_bPublic    = false;           // function not public
    m_bExtern    = false;           // function not extern
    m_pProg      = nullptr;
//  m_nThisIdent = 0;
    m_nFuncIdent = 0;
    m_bSynchro    = false;
    m_MasterClass = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
std::set<CBotFunction*> CBotFunction::m_publicFunctions{};

////////////////////////////////////////////////////////////////////////////////
CBotFunction::~CBotFunction()
{
    delete m_param;                // empty parameter list
    delete m_block;                // the instruction block

    // remove public list if there is
    if (m_bPublic)
    {
        m_publicFunctions.erase(this);
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::IsPublic()
{
    return m_bPublic;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::IsExtern()
{
    return m_bExtern;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::GetPosition(int& start, int& stop, CBotGet modestart, CBotGet modestop)
{
    start = m_extern.GetStart();
    stop = m_closeblk.GetEnd();

    if (modestart == GetPosExtern)
    {
        start = m_extern.GetStart();
    }
    if (modestop == GetPosExtern)
    {
        stop = m_extern.GetEnd();
    }
    if (modestart == GetPosNom)
    {
        start = m_token.GetStart();
    }
    if (modestop == GetPosNom)
    {
        stop = m_token.GetEnd();
    }
    if (modestart == GetPosParam)
    {
        start = m_openpar.GetStart();
    }
    if (modestop == GetPosParam)
    {
        stop = m_closepar.GetEnd();
    }
    if (modestart == GetPosBloc)
    {
        start = m_openblk.GetStart();
    }
    if (modestop == GetPosBloc)
    {
        stop = m_closeblk.GetEnd();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotFunction::Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* finput, bool bLocal)
{
    CBotToken*      pp;
    CBotFunction* func = finput;
    if ( func == nullptr ) func = new CBotFunction();

    CBotCStack* pStk = pStack->TokenStack(p, bLocal);

//  func->m_nFuncIdent = CBotVar::NextUniqNum();

    while (true)
    {
        if ( IsOfType(p, ID_PUBLIC) )
        {
            func->m_bPublic = true;
            continue;
        }
        pp = p;
        if ( IsOfType(p, ID_EXTERN) )
        {
            func->m_extern = *pp;        // for the position of the word "extern"
            func->m_bExtern = true;
//          func->m_bPublic = true;     // therefore also public!
            continue;
        }
        break;
    }

    func->m_retToken = *p;
//  CBotClass*  pClass;
    func->m_retTyp = TypeParam(p, pStk);        // type of the result

    if (func->m_retTyp.GetType() >= 0)
    {
        CBotToken*  pp = p;
        func->m_token = *p;

        if ( IsOfType(p, ID_NOT) )
        {
            CBotToken d(std::string("~") + p->GetString());
            func->m_token = d;
        }

        // un nom de fonction est-il là ?
        if (IsOfType(p, TokenTypVar))
        {
            if ( IsOfType( p, ID_DBLDOTS ) )        // method for a class
            {
                CBotClass* masterClass = CBotClass::Find(pp, pStack->GetProgram());

                if ( masterClass == nullptr ) goto bad;

                func->m_MasterClass = masterClass;
                func->m_classToken = *pp;

                func->m_token = *p;
                if (!IsOfType(p, TokenTypVar)) goto bad;
            }
            func->m_openpar = *p;
            func->m_param = CBotDefParam::Compile(p, pStk );
            func->m_closepar = *(p->GetPrev());
            if (pStk->IsOk())
            {
                pStk->SetRetType(func->m_retTyp);   // for knowledge what type returns

                if (func->m_MasterClass)
                {
                    // return "this" known
                    CBotVar* pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, func->m_MasterClass ));
                    pThis->SetInit(CBotVar::InitType::IS_POINTER);
//                  pThis->SetUniqNum(func->m_nThisIdent = -2); //CBotVar::NextUniqNum() will not
                    pThis->SetUniqNum(-2);
                    pStk->AddVar(pThis);

                    // initialize variables acording to This
                    // only saves the pointer to the first,
                    // the rest is chained
                    CBotVar* pv = pThis->GetItemList();
//                  int num = 1;
                    while (pv != nullptr)
                    {
                        CBotVar* pcopy = CBotVar::Create(pv);
//                      pcopy->SetInit(2);
                        pcopy->Copy(pv);
                        pcopy->SetPrivate(pv->GetPrivate());
//                      pcopy->SetUniqNum(pv->GetUniqNum()); //num++);
                        pStk->AddVar(pcopy);
                        pv = pv->GetNext();
                    }
                }

                // and compiles the following instruction block
                func->m_openblk = *p;
                func->m_block = CBotBlock::Compile(p, pStk, false);
                func->m_closeblk = (p != nullptr && p->GetPrev() != nullptr) ? *(p->GetPrev()) : CBotToken();
                if ( pStk->IsOk() )
                {
                    return pStack->ReturnFunc(func, pStk);
                }
            }
        }
bad:
        pStk->SetError(CBotErrNoFunc, p);
    }
    pStk->SetError(CBotErrNoType, p);
    if ( finput == nullptr ) delete func;
    return pStack->ReturnFunc(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotFunction::Compile1(CBotToken* &p, CBotCStack* pStack, CBotClass*  pClass)
{
    CBotFunction* func = new CBotFunction();
    func->m_nFuncIdent = CBotVar::NextUniqNum();

    CBotCStack* pStk = pStack->TokenStack(p, true);

    while (true)
    {
        if ( IsOfType(p, ID_PUBLIC) )
        {
        //  func->m_bPublic = true;     // will be done in two passes
            continue;
        }
        if ( IsOfType(p, ID_EXTERN) )
        {
            func->m_bExtern = true;
            continue;
        }
        break;
    }

    func->m_retToken = *p;
    func->m_retTyp = TypeParam(p, pStack);      // type of the result

    if (func->m_retTyp.GetType() >= 0)
    {
        CBotToken*  pp = p;
        func->m_token = *p;

        if ( IsOfType(p, ID_NOT) )
        {
            CBotToken d(std::string("~") + p->GetString());
            func->m_token = d;
        }

        // un nom de fonction est-il là ?
        if (IsOfType(p, TokenTypVar))
        {
            if ( IsOfType( p, ID_DBLDOTS ) )        // method for a class
            {
                IsOfType(p, TokenTypVar); // skip the function name

            }
            func->m_param = CBotDefParam::Compile(p, pStk );
            if (pStk->IsOk())
            {
                // looks if the function exists elsewhere
                if (( pClass != nullptr || !pStack->CheckCall(pp, func->m_param)) &&
                    ( pClass == nullptr || !pClass->CheckCall(pStack->GetProgram(), func->m_param, pp)) )
                {
                    if (IsOfType(p, ID_OPBLK))
                    {
                        int level = 1;
                        // and skips the following instruction block
                        do
                        {
                            int type = p->GetType();
                            p = p->GetNext();
                            if (type == ID_OPBLK) level++;
                            if (type == ID_CLBLK) level--;
                        }
                        while (level > 0 && p != nullptr);

                        return pStack->ReturnFunc(func, pStk);
                    }
                    pStk->SetError(CBotErrOpenBlock, p);
                }
            }
            pStk->SetError(CBotErrRedefFunc, pp);
        }
    }
    pStk->SetError(CBotErrNoType, p);
    delete func;
    return pStack->ReturnFunc(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
    CBotStack*  pile = pj->AddStack(this, CBotStack::BlockVisibilityType::FUNCTION);               // one end of stack local to this function
//  if ( pile == EOX ) return true;

    pile->SetProgram(m_pProg);                              // bases for routines

    if ( pile->GetState() == 0 )
    {
        if (m_param != nullptr)
        {
            if ( !m_param->Execute(ppVars, pile) ) return false;    // define parameters
        }
        pile->IncState();
    }

    if ( pile->GetState() == 1 && m_MasterClass )
    {
        // makes "this" known
        CBotVar* pThis = nullptr;
        if ( pInstance == nullptr )
        {
            pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, m_MasterClass ));
        }
        else
        {
            if (m_MasterClass != pInstance->GetClass())
            {
                pile->SetError(CBotErrBadType2, &m_classToken);
                return false;
            }

            pThis = CBotVar::Create("this", CBotTypResult( CBotTypPointer, m_MasterClass ));
            pThis->SetPointer(pInstance);
        }
        assert(pThis != nullptr);
        pThis->SetInit(CBotVar::InitType::IS_POINTER);

//      pThis->SetUniqNum(m_nThisIdent);
        pThis->SetUniqNum(-2);
        pile->AddVar(pThis);

        pile->IncState();
    }

    if ( pile->IfStep() ) return false;

    if ( !m_block->Execute(pile) )
    {
        if ( pile->GetError() < 0 )
            pile->SetError( CBotNoErr );
        else
            return false;
    }

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotFunction::RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
    CBotStack*  pile = pj->RestoreStack(this);          // one end of stack local to this function
    if ( pile == nullptr ) return;
    CBotStack*  pile2 = pile;

    pile->SetProgram(m_pProg);                          // bases for routines

    if ( pile->GetBlock() != CBotStack::BlockVisibilityType::FUNCTION)
    {
        CBotStack*  pile2 = pile->RestoreStack(nullptr);       // one end of stack local to this function
        if ( pile2 == nullptr ) return;
        pile->SetState(pile->GetState() + pile2->GetState());
        pile2->Delete();
    }

    m_param->RestoreState(pile2, true);                 // parameters

    if ( m_MasterClass )
    {
        CBotVar* pThis = pile->FindVar("this");
        pThis->SetInit(CBotVar::InitType::IS_POINTER);
        pThis->SetUniqNum(-2);
    }

    m_block->RestoreState(pile2, true);
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotFunction::CompileCall(const std::list<CBotFunction*>& localFunctionList, const std::string &name, CBotVar** ppVars, long &nIdent)
{
    CBotTypResult type;
    if (!FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type))
    {
        // Reset the identifier to "not found" value
        nIdent = 0;
    }
    return type;
}

////////////////////////////////////////////////////////////////////////////////
CBotFunction* CBotFunction::FindLocalOrPublic(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                                              CBotVar** ppVars, CBotTypResult &TypeOrError, bool bPublic)
{
    TypeOrError.SetType(CBotErrUndefCall);      // no routine of the name

    if ( nIdent )
    {
        for (CBotFunction* pt : localFunctionList)
        {
            if (pt->m_nFuncIdent == nIdent)
            {
                TypeOrError = pt->m_retTyp;
                return pt;
            }
        }

        // search the list of public functions
        for (CBotFunction* pt : m_publicFunctions)
        {
            if (pt->m_nFuncIdent == nIdent)
            {
                TypeOrError = pt->m_retTyp;
                return pt;
            }
        }
    }

    if ( name.empty() ) return nullptr;

    std::map<CBotFunction*, int> funcMap;

    for (CBotFunction* pt : localFunctionList)
    {
        if ( pt->m_token.GetString() == name )
        {
            int i = 0;
            int alpha = 0;                          // signature of parameters
            // parameters are compatible?
            CBotDefParam* pv = pt->m_param;         // expected list of parameters
            CBotVar* pw = ppVars[i++];              // provided list parameter
            while ( pv != nullptr && pw != nullptr)
            {
                CBotTypResult paramType = pv->GetTypResult();
                CBotTypResult argType = pw->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);

                if (!TypesCompatibles(paramType, argType))
                {
                    if ( funcMap.empty() ) TypeOrError.SetType(CBotErrBadParam);
                    break;
                }

                if (paramType.Eq(CBotTypPointer) && !argType.Eq(CBotTypNullPointer))
                {
                    CBotClass* c1 = paramType.GetClass();
                    CBotClass* c2 = argType.GetClass();
                    while (c2 != c1 && c2 != nullptr)    // implicit cast
                    {
                        alpha += 10;
                        c2 = c2->GetParent();
                    }
                }
                else
                {
                    int d = pv->GetType() - pw->GetType(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);
                    alpha += d>0 ? d : -10*d;       // quality loss, 10 times more expensive!
                }
                pv = pv->GetNext();
                pw = ppVars[i++];
            }
            if ( pw != nullptr )
            {
                if ( !funcMap.empty() ) continue;
                if ( TypeOrError.Eq(CBotErrLowParam) ) TypeOrError.SetType(CBotErrNbParam);
                if ( TypeOrError.Eq(CBotErrUndefCall)) TypeOrError.SetType(CBotErrOverParam);
                continue;                   // too many parameters
            }
            if ( pv != nullptr )
            {
                if ( !funcMap.empty() ) continue;
                if ( TypeOrError.Eq(CBotErrOverParam) ) TypeOrError.SetType(CBotErrNbParam);
                if ( TypeOrError.Eq(CBotErrUndefCall) ) TypeOrError.SetType(CBotErrLowParam);
                continue;                   // not enough parameters
            }
            funcMap.insert( std::pair<CBotFunction*, int>(pt, alpha) );
        }
    }

    if ( bPublic )
    {
        for (CBotFunction* pt : m_publicFunctions)
        {
            if ( pt->m_token.GetString() == name )
            {
                int i = 0;
                int alpha = 0;                          // signature of parameters
                // parameters sont-ils compatibles ?
                CBotDefParam* pv = pt->m_param;         // list of expected parameters
                CBotVar* pw = ppVars[i++];              // list of provided parameters
                while ( pv != nullptr && pw != nullptr)
                {
                    CBotTypResult paramType = pv->GetTypResult();
                    CBotTypResult argType = pw->GetTypResult(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);

                    if (!TypesCompatibles(paramType, argType))
                    {
                        if ( funcMap.empty() ) TypeOrError.SetType(CBotErrBadParam);
                        break;
                    }

                    if (paramType.Eq(CBotTypPointer) && !argType.Eq(CBotTypNullPointer))
                    {
                        CBotClass* c1 = paramType.GetClass();
                        CBotClass* c2 = argType.GetClass();
                        while (c2 != c1 && c2 != nullptr)    // implicit cast
                        {
                            alpha += 10;
                            c2 = c2->GetParent();
                        }
                    }
                    else
                    {
                        int d = pv->GetType() - pw->GetType(CBotVar::GetTypeMode::CLASS_AS_INTRINSIC);
                        alpha += d>0 ? d : -10*d;       // quality loss, 10 times more expensive!
                    }
                    pv = pv->GetNext();
                    pw = ppVars[i++];
                }
                if ( pw != nullptr )
                {
                    if ( !funcMap.empty() ) continue; // previous useable function
                    if ( TypeOrError.Eq(CBotErrLowParam) ) TypeOrError.SetType(CBotErrNbParam);
                    if ( TypeOrError.Eq(CBotErrUndefCall)) TypeOrError.SetType(CBotErrOverParam);
                    continue;                   // to many parameters
                }
                if ( pv != nullptr )
                {
                    if ( !funcMap.empty() ) continue; // previous useable function
                    if ( TypeOrError.Eq(CBotErrOverParam) ) TypeOrError.SetType(CBotErrNbParam);
                    if ( TypeOrError.Eq(CBotErrUndefCall) ) TypeOrError.SetType(CBotErrLowParam);
                    continue;                   // not enough parameters
                }
                funcMap.insert( std::pair<CBotFunction*, int>(pt, alpha) );
            }
        }
    }

    if ( !funcMap.empty() )
    {
        auto it = funcMap.begin();
        CBotFunction* pFunc = it->first;        // the best function found
        signed int    delta = it->second;       // seeks the lowest signature

        for (++it ; it != funcMap.end() ; it++)
        {
            if (it->second < delta) // a better signature?
            {
                TypeOrError.SetType(CBotNoErr);
                pFunc = it->first;
                delta = it->second;
                continue;
            }

            if (it->second == delta) TypeOrError.SetType(CBotErrAmbiguousCall);
        }

        if (TypeOrError.Eq(CBotErrAmbiguousCall)) return nullptr;
        nIdent = pFunc->m_nFuncIdent;
        TypeOrError = pFunc->m_retTyp;
        return pFunc;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
int CBotFunction::DoCall(CBotProgram* program, const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name,
                         CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken)
{
    CBotTypResult   type;
    CBotFunction*   pt = nullptr;

    pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type);

    if ( pt != nullptr )
    {
        CBotStack*  pStk1 = pStack->AddStack(pt, CBotStack::BlockVisibilityType::FUNCTION);    // to put "this"
//      if ( pStk1 == EOX ) return true;

        pStk1->SetProgram(pt->m_pProg);                 // it may have changed module

        if ( pStk1->IfStep() ) return false;

        CBotStack*  pStk3 = pStk1->AddStack(nullptr, CBotStack::BlockVisibilityType::BLOCK);    // parameters

        // preparing parameters on the stack

        if ( pStk1->GetState() == 0 )
        {
            if ( pt->m_MasterClass )
            {
                CBotVar* pInstance = program->m_thisVar;
                // make "this" known
                CBotVar* pThis ;
                if ( pInstance == nullptr )
                {
                    pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, pt->m_MasterClass ));
                }
                else
                {
                    if (pt->m_MasterClass != pInstance->GetClass())
                    {
                        pStack->SetError(CBotErrBadType2, &pt->m_classToken);
                        return false;
                    }

                    pThis = CBotVar::Create("this", CBotTypResult( CBotTypPointer, pt->m_MasterClass ));
                    pThis->SetPointer(pInstance);
                }
                assert(pThis != nullptr);
                pThis->SetInit(CBotVar::InitType::IS_POINTER);

                pThis->SetUniqNum(-2);
                pStk1->AddVar(pThis);
            }

            // initializes the variables as parameters
            if (pt->m_param != nullptr)
            {
                pt->m_param->Execute(ppVars, pStk3);            // cannot be interrupted
            }

            pStk1->IncState();
        }

        // finally execution of the found function

        if ( !pStk3->GetRetVar(                     // puts the result on the stack
            pt->m_block->Execute(pStk3) ))          // GetRetVar said if it is interrupted
        {
            if ( !pStk3->IsOk() && pt->m_pProg != program )
            {
                pStk3->SetPosError(pToken);         // indicates the error on the procedure call
            }
            return false;   // interrupt !
        }

        return pStack->Return( pStk3 );
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
void CBotFunction::RestoreCall(const std::list<CBotFunction*>& localFunctionList,
                               long &nIdent, const std::string &name, CBotVar** ppVars, CBotStack* pStack)
{
    CBotTypResult   type;
    CBotFunction*   pt = nullptr;
    CBotStack*      pStk1;
    CBotStack*      pStk3;

    // search function to return the ok identifier

    pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type);

    if ( pt != nullptr )
    {
        pStk1 = pStack->RestoreStack(pt);
        if ( pStk1 == nullptr ) return;

        pStk1->SetProgram(pt->m_pProg);                 // it may have changed module

        if ( pStk1->GetBlock() != CBotStack::BlockVisibilityType::FUNCTION)
        {
            CBotStack* pStk2 = pStk1->RestoreStack(nullptr); // used more
            if ( pStk2 == nullptr ) return;
            pStk3 = pStk2->RestoreStack(nullptr);
            if ( pStk3 == nullptr ) return;
        }
        else
        {
            pStk3 = pStk1->RestoreStack(nullptr);
            if ( pStk3 == nullptr ) return;
        }

        // preparing parameters on the stack

        {
            if ( pt->m_MasterClass )
            {
//                CBotVar* pInstance = m_pProg->m_thisVar;
                // make "this" known
                CBotVar* pThis = pStk1->FindVar("this");
                pThis->SetInit(CBotVar::InitType::IS_POINTER);
                pThis->SetUniqNum(-2);
            }
        }

        if ( pStk1->GetState() == 0 )
        {
            pt->m_param->RestoreState(pStk3, true);
            return;
        }

        // initializes the variables as parameters
        pt->m_param->RestoreState(pStk3, false);
        pt->m_block->RestoreState(pStk3, true);
    }
}

////////////////////////////////////////////////////////////////////////////////
int CBotFunction::DoCall(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name, CBotVar* pThis,
                         CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass)
{
    CBotTypResult   type;
    CBotProgram*    pProgCurrent = pStack->GetProgram();

    CBotFunction*   pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type, false);

    if ( pt != nullptr )
    {
//      DEBUG( "CBotFunction::DoCall" + pt->GetName(), 0, pStack);

        CBotStack*  pStk = pStack->AddStack(pt, CBotStack::BlockVisibilityType::FUNCTION);
//      if ( pStk == EOX ) return true;

        pStk->SetProgram(pt->m_pProg);                  // it may have changed module
        CBotStack*  pStk3 = pStk->AddStack(nullptr, CBotStack::BlockVisibilityType::BLOCK); // to set parameters passed

        // preparing parameters on the stack

        if ( pStk->GetState() == 0 )
        {
            // sets the variable "this" on the stack
            CBotVar* pthis = CBotVar::Create("this", CBotTypNullPointer);
            pthis->Copy(pThis, false);
            pthis->SetUniqNum(-2);      // special value
            pStk->AddVar(pthis);

            CBotClass*  pClass = pThis->GetClass()->GetParent();
            if ( pClass )
            {
                // sets the variable "super" on the stack
                CBotVar* psuper = CBotVar::Create("super", CBotTypNullPointer);
                psuper->Copy(pThis, false); // in fact identical to "this"
                psuper->SetUniqNum(-3);     // special value
                pStk->AddVar(psuper);
            }
            // initializes the variables as parameters
            if (pt->m_param != nullptr)
            {
                pt->m_param->Execute(ppVars, pStk3);            // cannot be interrupted
            }
            pStk->IncState();
        }

        if ( pStk->GetState() == 1 )
        {
            if ( pt->m_bSynchro )
            {
                CBotProgram* pProgBase = pStk->GetProgram(true);
                if ( !pClass->Lock(pProgBase) ) return false; // try to lock, interrupt if failed
            }
            pStk->IncState();
        }
        // finally calls the found function

        if ( !pStk3->GetRetVar(                         // puts the result on the stack
            pt->m_block->Execute(pStk3) ))          // GetRetVar said if it is interrupted
        {
            if ( !pStk3->IsOk() )
            {
                if ( pt->m_bSynchro )
                {
                    pClass->Unlock();                   // release function
                }

                if ( pt->m_pProg != pProgCurrent )
                {
                    pStk3->SetPosError(pToken);         // indicates the error on the procedure call
                }
            }
            return false;   // interrupt !
        }

        if ( pt->m_bSynchro )
        {
            pClass->Unlock();                           // release function
        }

        return pStack->Return( pStk3 );
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::RestoreCall(const std::list<CBotFunction*>& localFunctionList, long &nIdent, const std::string &name, CBotVar* pThis,
                               CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass)
{
    CBotTypResult   type;
    CBotFunction*   pt = FindLocalOrPublic(localFunctionList, nIdent, name, ppVars, type);

    if ( pt != nullptr )
    {
        CBotStack*  pStk = pStack->RestoreStack(pt);
        if ( pStk == nullptr ) return true;
        pStk->SetProgram(pt->m_pProg);                  // it may have changed module

        CBotVar*    pthis = pStk->FindVar("this");
        pthis->SetUniqNum(-2);

        if (pClass->GetParent() != nullptr)
        {
            CBotVar* psuper = pStk->FindVar("super");
            if (psuper != nullptr) psuper->SetUniqNum(-3);
        }

        CBotStack*  pStk3 = pStk->RestoreStack(nullptr);   // to set parameters passed
        if ( pStk3 == nullptr ) return true;

        pt->m_param->RestoreState(pStk3, true);                 // parameters

        if ( pStk->GetState() > 1 &&                        // latching is effective?
             pt->m_bSynchro )
            {
                CBotProgram* pProgBase = pStk->GetProgram(true);
                pClass->Lock(pProgBase);                    // locks the class
            }

        // finally calls the found function

        pt->m_block->RestoreState(pStk3, true);                 // interrupt !
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotFunction::CheckParam(CBotDefParam* pParam)
{
    CBotDefParam*   pp = m_param;
    while ( pp != nullptr && pParam != nullptr )
    {
        CBotTypResult type1 = pp->GetTypResult();
        CBotTypResult type2 = pParam->GetTypResult();
        if ( !type1.Compare(type2) ) return false;
        pp = pp->GetNext();
        pParam = pParam->GetNext();
    }
    return ( pp == nullptr && pParam == nullptr );
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotFunction::GetName()
{
    return  m_token.GetString();
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotFunction::GetParams()
{
    if (m_param == nullptr ) return std::string("()");

    std::string      params = "( ";
    CBotDefParam*   p = m_param;        // list of parameters

    while (p != nullptr)
    {
        params += p->GetParamString();
        p = p->GetNext();
        if ( p != nullptr ) params += ", ";
    }

    params += " )";
    return params;
}

////////////////////////////////////////////////////////////////////////////////
void CBotFunction::AddPublic(CBotFunction* func)
{
    m_publicFunctions.insert(func);
}

std::string CBotFunction::GetDebugData()
{
    std::stringstream ss;
    if (IsPublic()) ss << "public ";
    if (IsExtern()) ss << "extern ";
    ss << GetName() << GetParams();
    //ss << "FuncID = " << m_nFuncIdent;
    return ss.str();
}

std::map<std::string, CBotInstr*> CBotFunction::GetDebugLinks()
{
    auto links = CBotInstr::GetDebugLinks();
    links["m_block"] = m_block;
    return links;
}

} // namespace CBot
