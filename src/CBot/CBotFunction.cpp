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

///////////////////////////////////////////////////////////////////////
// compilation of various functions declared by the user
//

#include "CBot.h"

// various constructors / destructors
// \TODO translation:to liberate all according to esteblished tree
// pour libérer tout selon l'arbre établi
CBotFunction::CBotFunction()
{
    m_Param      = NULL;            // empty parameter list
    m_Block      = NULL;            // the instruction block
    m_next       = NULL;            // functions can be chained
    m_bPublic    = false;           // function not public
    m_bExtern    = false;           // function not extern
    m_nextpublic = NULL;
    m_prevpublic = NULL;
    m_pProg      = NULL;
//  m_nThisIdent = 0;
    m_nFuncIdent = 0;
    m_bSynchro    = false;
}

CBotFunction* CBotFunction::m_listPublic = NULL;

CBotFunction::~CBotFunction()
{
    delete  m_Param;                // empty parameter list
    delete  m_Block;                // the instruction block
    delete  m_next;

    // remove public list if there is
    if ( m_bPublic )
    {
        if ( m_nextpublic != NULL )
        {
            m_nextpublic->m_prevpublic = m_prevpublic;
        }
        if ( m_prevpublic != NULL)
        {
            m_prevpublic->m_nextpublic = m_nextpublic;
        }
        else
        {
            // if prev = next = null may not be in the list!
            if ( m_listPublic == this ) m_listPublic = m_nextpublic;
        }
    }
}

bool CBotFunction::IsPublic()
{
    return m_bPublic;
}

bool CBotFunction::IsExtern()
{
    return m_bExtern;
}

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


CBotTypResult   ArrayType(CBotToken* &p, CBotCStack* pile, CBotTypResult type)
{
    while ( IsOfType( p, ID_OPBRK ) )
    {
        if ( !IsOfType( p, ID_CLBRK ) )
        {
            pile->SetError(TX_CLBRK, p->GetStart());
            return CBotTypResult( -1 );
        }
        type = CBotTypResult( CBotTypArrayPointer, type );
    }
    return type;
}

CBotTypResult   TypeParam(CBotToken* &p, CBotCStack* pile)
{
    CBotClass*  pClass = NULL;

    switch (p->GetType())
    {
    case ID_INT:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypInt ));
    case ID_FLOAT:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypFloat ));
    case ID_BOOLEAN:
    case ID_BOOL:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypBoolean ));
    case ID_STRING:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypString ));
    case ID_VOID:
        p = p->GetNext();
        return CBotTypResult( 0 );

    case TokenTypVar:
        pClass = CBotClass::Find(p);
        if ( pClass != NULL)
        {
            p = p->GetNext();
            return ArrayType(p, pile,
                             pClass->IsIntrinsic() ?
                             CBotTypResult( CBotTypIntrinsic, pClass ) :
                             CBotTypResult( CBotTypPointer,   pClass ) );
        }
    }
    return CBotTypResult( -1 );
}

// compiles a new function
// bLocal allows of the declaration of parameters on the same level
// as the elements belonging to the class for methods
CBotFunction* CBotFunction::Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* finput, bool bLocal)
{
    CBotToken*      pp;
    CBotFunction* func = finput;
    if ( func == NULL ) func = new CBotFunction();

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
            func->m_extern = pp;        // for the position of the word "extern"
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
            CBotToken d("~" + p->GetString());
            func->m_token = d;
        }

        // un nom de fonction est-il là ?
        if (IsOfType(p, TokenTypVar))
        {
            if ( IsOfType( p, ID_DBLDOTS ) )        // method for a class
            {
                func->m_MasterClass = pp->GetString();
                CBotClass* pClass = CBotClass::Find(pp);
                if ( pClass == NULL ) goto bad;

//              pp = p;
                func->m_token = *p;
                if (!IsOfType(p, TokenTypVar)) goto bad;

            }
            func->m_openpar = p;
            func->m_Param = CBotDefParam::Compile( p, pStk );
            func->m_closepar = p->GetPrev();
            if (pStk->IsOk())
            {
                pStk->SetRetType(func->m_retTyp);   // for knowledge what type returns

                if (!func->m_MasterClass.IsEmpty())
                {
                    // return "this" known
                    CBotVar* pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, func->m_MasterClass ));
                    pThis->SetInit(2);
//                  pThis->SetUniqNum(func->m_nThisIdent = -2); //CBotVar::NextUniqNum() will not
                    pThis->SetUniqNum(-2);
                    pStk->AddVar(pThis);

                    // initialize variables acording to This
                    // only saves the pointer to the first,
                    // the rest is chained
                    CBotVar* pv = pThis->GetItemList();
//                  int num = 1;
                    while (pv != NULL)
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
                func->m_openblk = p;
                func->m_Block   = CBotBlock::Compile(p, pStk, false);
                func->m_closeblk = p->GetPrev();
                if ( pStk->IsOk() )
                {
                    if ( func->m_bPublic )  // public function, return known for all
                    {
                        CBotFunction::AddPublic(func);
                    }
                    return pStack->ReturnFunc(func, pStk);
                }
            }
        }
bad:
        pStk->SetError(TX_NOFONC, p);
    }
    pStk->SetError(TX_NOTYP, p);
    if ( finput == NULL ) delete func;
    return pStack->ReturnFunc(NULL, pStk);
}

// pre-compile a new function
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
        // un nom de fonction est-il là ?
        if (IsOfType(p, TokenTypVar))
        {
            if ( IsOfType( p, ID_DBLDOTS ) )        // method for a class
            {
                func->m_MasterClass = pp->GetString();
                CBotClass* pClass = CBotClass::Find(pp);
                if ( pClass == NULL )
                {
                    pStk->SetError(TX_NOCLASS, pp);
                    goto bad;
                }

                pp = p;
                func->m_token = *p;
                if (!IsOfType(p, TokenTypVar)) goto bad;

            }
            func->m_Param = CBotDefParam::Compile( p, pStk );
            if (pStk->IsOk())
            {
                // looks if the function exists elsewhere
                if (( pClass != NULL || !pStack->CheckCall(pp, func->m_Param)) &&
                    ( pClass == NULL || !pClass->CheckCall(pp, func->m_Param)) )
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
                        while (level > 0 && p != NULL);

                        return pStack->ReturnFunc(func, pStk);
                    }
                    pStk->SetError(TX_OPENBLK, p);
                }
            }
            pStk->SetError(TX_REDEF, pp);
        }
bad:
        pStk->SetError(TX_NOFONC, p);
    }
    pStk->SetError(TX_NOTYP, p);
    delete func;
    return pStack->ReturnFunc(NULL, pStk);
}

#ifdef  _DEBUG
static int xx = 0;
#endif

bool CBotFunction::Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
    CBotStack*  pile = pj->AddStack(this, 2);               // one end of stack local to this function
//  if ( pile == EOX ) return true;

    pile->SetBotCall(m_pProg);                              // bases for routines

    if ( pile->GetState() == 0 )
    {
        if ( !m_Param->Execute(ppVars, pile) ) return false;    // define parameters
        pile->IncState();
    }

    if ( pile->GetState() == 1 && !m_MasterClass.IsEmpty() )
    {
        // makes "this" known
        CBotVar* pThis ;
        if ( pInstance == NULL )
        {
            pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, m_MasterClass ));
            pThis->SetInit(2);
        }
        else
        {
            pThis = CBotVar::Create("this", CBotTypResult( CBotTypPointer, m_MasterClass ));
            pThis->SetPointer(pInstance);
            pThis->SetInit(2);
        }

//      pThis->SetUniqNum(m_nThisIdent);
        pThis->SetUniqNum(-2);
        pile->AddVar(pThis);

        pile->IncState();
    }

    if ( pile->IfStep() ) return false;

    if ( !m_Block->Execute(pile) )
    {
        if ( pile->GetError() < 0 )
            pile->SetError( 0 );
        else
            return false;
    }

    return pj->Return(pile);
}


void CBotFunction::RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
    CBotStack*  pile = pj->RestoreStack(this);          // one end of stack local to this function
    if ( pile == NULL ) return;
    CBotStack*  pile2 = pile;

    pile->SetBotCall(m_pProg);                          // bases for routines

    if ( pile->GetBlock() < 2 )
    {
        CBotStack*  pile2 = pile->RestoreStack(NULL);       // one end of stack local to this function
        if ( pile2 == NULL ) return;
        pile->SetState(pile->GetState() + pile2->GetState());
        pile2->Delete();
    }

    m_Param->RestoreState(pile2, true);                 // parameters

    if ( !m_MasterClass.IsEmpty() )
    {
        CBotVar* pThis = pile->FindVar("this");
        pThis->SetInit(2);
        pThis->SetUniqNum(-2);
    }

    m_Block->RestoreState(pile2, true);
}

void CBotFunction::AddNext(CBotFunction* p)
{
    CBotFunction*   pp = this;
    while (pp->m_next != NULL) pp = pp->m_next;

    pp->m_next = p;
}


CBotTypResult CBotFunction::CompileCall(const char* name, CBotVar** ppVars, long& nIdent)
{
    nIdent = 0;
    CBotTypResult   type;

//    CBotFunction*   pt = FindLocalOrPublic(nIdent, name, ppVars, type);
    FindLocalOrPublic(nIdent, name, ppVars, type);
    return type;
}


// is a function according to its unique identifier
// if the identifier is not found, looking by name and parameters

CBotFunction* CBotFunction::FindLocalOrPublic(long& nIdent, const char* name, CBotVar** ppVars, CBotTypResult& TypeOrError, bool bPublic)
{
    TypeOrError.SetType(TX_UNDEFCALL);      // no routine of the name
    CBotFunction*   pt;

    if ( nIdent )
    {
        if ( this != NULL ) for ( pt = this ; pt != NULL ; pt = pt->m_next )
        {
            if ( pt->m_nFuncIdent == nIdent )
            {
                TypeOrError = pt->m_retTyp;
                return pt;
            }
        }

        // search the list of public functions

        for ( pt = m_listPublic ; pt != NULL ; pt = pt->m_nextpublic )
        {
            if ( pt->m_nFuncIdent == nIdent )
            {
                TypeOrError = pt->m_retTyp;
                return pt;
            }
        }
    }

    if ( name == NULL ) return NULL;

    int     delta   = 99999;                // seeks the lowest signature
    CBotFunction*   pFunc = NULL;           // the best function found

    if ( this != NULL )
    {
        for ( pt = this ; pt != NULL ; pt = pt->m_next )
        {
            if ( pt->m_token.GetString() == name )
            {
                int i = 0;
                int alpha = 0;                          // signature of parameters
                // parameters are compatible?
                CBotDefParam* pv = pt->m_Param;         // expected list of parameters
                CBotVar* pw = ppVars[i++];              // provided list parameter
                while ( pv != NULL && pw != NULL)
                {
                    if (!TypesCompatibles(pv->GetTypResult(), pw->GetTypResult()))
                    {
                        if ( pFunc == NULL ) TypeOrError = TX_BADPARAM;
                        break;
                    }
                    int d = pv->GetType() - pw->GetType(2);
                    alpha += d>0 ? d : -10*d;       // quality loss, 10 times more expensive!

                    pv = pv->GetNext();
                    pw = ppVars[i++];
                }
                if ( pw != NULL )
                {
                    if ( pFunc != NULL ) continue;
                    if ( TypeOrError.Eq(TX_LOWPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
                    if ( TypeOrError.Eq(TX_UNDEFCALL)) TypeOrError.SetType(TX_OVERPARAM);
                    continue;                   // too many parameters
                }
                if ( pv != NULL )
                {
                    if ( pFunc != NULL ) continue;
                    if ( TypeOrError.Eq(TX_OVERPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
                    if ( TypeOrError.Eq(TX_UNDEFCALL) ) TypeOrError.SetType(TX_LOWPARAM);
                    continue;                   // not enough parameters
                }

                if (alpha == 0)                 // perfect signature
                {
                    nIdent = pt->m_nFuncIdent;
                    TypeOrError = pt->m_retTyp;
                    return pt;
                }

                if ( alpha < delta )            // a better signature?
                {
                    pFunc = pt;
                    delta = alpha;
                }
            }
        }
    }

    if ( bPublic )
    {
        for ( pt = m_listPublic ; pt != NULL ; pt = pt->m_nextpublic )
        {
            if ( pt->m_token.GetString() == name )
            {
                int i = 0;
                int alpha = 0;                          // signature of parameters
                // parameters sont-ils compatibles ?
                CBotDefParam* pv = pt->m_Param;         // list of expected parameters
                CBotVar* pw = ppVars[i++];              // list of provided parameters
                while ( pv != NULL && pw != NULL)
                {
                    if (!TypesCompatibles(pv->GetTypResult(), pw->GetTypResult()))
                    {
                        if ( pFunc == NULL ) TypeOrError = TX_BADPARAM;
                        break;
                    }
                    int d = pv->GetType() - pw->GetType(2);
                    alpha += d>0 ? d : -10*d;       // quality loss, 10 times more expensive!

                    pv = pv->GetNext();
                    pw = ppVars[i++];
                }
                if ( pw != NULL )
                {
                    if ( pFunc != NULL ) continue;
                    if ( TypeOrError.Eq(TX_LOWPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
                    if ( TypeOrError.Eq(TX_UNDEFCALL)) TypeOrError.SetType(TX_OVERPARAM);
                    continue;                   // to many parameters
                }
                if ( pv != NULL )
                {
                    if ( pFunc != NULL ) continue;
                    if ( TypeOrError.Eq(TX_OVERPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
                    if ( TypeOrError.Eq(TX_UNDEFCALL) ) TypeOrError.SetType(TX_LOWPARAM);
                    continue;                   // not enough parameters
                }

                if (alpha == 0)                 // perfect signature
                {
                    nIdent = pt->m_nFuncIdent;
                    TypeOrError = pt->m_retTyp;
                    return pt;
                }

                if ( alpha < delta )            // a better signature?
                {
                    pFunc = pt;
                    delta = alpha;
                }
            }
        }
    }

    if ( pFunc != NULL )
    {
        nIdent = pFunc->m_nFuncIdent;
        TypeOrError = pFunc->m_retTyp;
        return pFunc;
    }
    return NULL;
}


// fait un appel à une fonction

int CBotFunction::DoCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken)
{
    CBotTypResult   type;
    CBotFunction*   pt = NULL;

    pt = FindLocalOrPublic(nIdent, name, ppVars, type);

    if ( pt != NULL )
    {
        CBotStack*  pStk1 = pStack->AddStack(pt, 2);    // to put "this"
//      if ( pStk1 == EOX ) return true;

        pStk1->SetBotCall(pt->m_pProg);                 // it may have changed module

        if ( pStk1->IfStep() ) return false;

        CBotStack*  pStk3 = pStk1->AddStack(NULL, true);    // parameters

        // preparing parameters on the stack

        if ( pStk1->GetState() == 0 )
        {
            if ( !pt->m_MasterClass.IsEmpty() )
            {
                CBotVar* pInstance = m_pProg->m_pInstance;
                // make "this" known
                CBotVar* pThis ;
                if ( pInstance == NULL )
                {
                    pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, pt->m_MasterClass ));
                    pThis->SetInit(2);
                }
                else
                {
                    pThis = CBotVar::Create("this", CBotTypResult( CBotTypPointer, pt->m_MasterClass ));
                    pThis->SetPointer(pInstance);
                    pThis->SetInit(2);
                }

                pThis->SetUniqNum(-2);
                pStk1->AddVar(pThis);

            }

            // initializes the variables as parameters
            pt->m_Param->Execute(ppVars, pStk3);            // cannot be interrupted

            pStk1->IncState();
        }

        // finally execution of the found function

        if ( !pStk3->GetRetVar(                     // puts the result on the stack
            pt->m_Block->Execute(pStk3) ))          // GetRetVar said if it is interrupted
        {
            if ( !pStk3->IsOk() && pt->m_pProg != m_pProg )
            {
#ifdef _DEBUG
                if ( m_pProg->GetFunctions()->GetName() == "LaCommande" ) return false;
#endif
                pStk3->SetPosError(pToken);         // indicates the error on the procedure call
            }
            return false;   // interrupt !
        }

        return pStack->Return( pStk3 );
    }
    return -1;
}

void CBotFunction::RestoreCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack)
{
    CBotTypResult   type;
    CBotFunction*   pt = NULL;
    CBotStack*      pStk1;
    CBotStack*      pStk3;

    // search function to return the ok identifier

    pt = FindLocalOrPublic(nIdent, name, ppVars, type);

    if ( pt != NULL )
    {
        pStk1 = pStack->RestoreStack(pt);
        if ( pStk1 == NULL ) return;

        pStk1->SetBotCall(pt->m_pProg);                 // it may have changed module

        if ( pStk1->GetBlock() < 2 )
        {
            CBotStack* pStk2 = pStk1->RestoreStack(NULL); // used more
            if ( pStk2 == NULL ) return;
            pStk3 = pStk2->RestoreStack(NULL);
            if ( pStk3 == NULL ) return;
        }
        else
        {
            pStk3 = pStk1->RestoreStack(NULL);
            if ( pStk3 == NULL ) return;
        }

        // preparing parameters on the stack

        {
            if ( !pt->m_MasterClass.IsEmpty() )
            {
//                CBotVar* pInstance = m_pProg->m_pInstance;
                // make "this" known
                CBotVar* pThis = pStk1->FindVar("this");
                pThis->SetInit(2);
                pThis->SetUniqNum(-2);
            }
        }

        if ( pStk1->GetState() == 0 )
        {
            pt->m_Param->RestoreState(pStk3, true);
            return;
        }

        // initializes the variables as parameters
        pt->m_Param->RestoreState(pStk3, false);
        pt->m_Block->RestoreState(pStk3, true);
    }
}



// makes call of a method
// note: this is already on the stack, the pointer pThis is just to simplify

int CBotFunction::DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass)
{
    CBotTypResult   type;
    CBotProgram*    pProgCurrent = pStack->GetBotCall();

    CBotFunction*   pt = FindLocalOrPublic(nIdent, name, ppVars, type, false);

    if ( pt != NULL )
    {
//      DEBUG( "CBotFunction::DoCall" + pt->GetName(), 0, pStack);

        CBotStack*  pStk = pStack->AddStack(pt, 2);
//      if ( pStk == EOX ) return true;

        pStk->SetBotCall(pt->m_pProg);                  // it may have changed module
        CBotStack*  pStk3 = pStk->AddStack(NULL, true); // to set parameters passed

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
            pt->m_Param->Execute(ppVars, pStk3);            // cannot be interrupted
            pStk->IncState();
        }

        if ( pStk->GetState() == 1 )
        {
            if ( pt->m_bSynchro )
            {
                CBotProgram* pProgBase = pStk->GetBotCall(true);
                if ( !pClass->Lock(pProgBase) ) return false;       // expected to power \TODO attend de pouvoir
            }
            pStk->IncState();
        }
        // finally calls the found function

        if ( !pStk3->GetRetVar(                         // puts the result on the stack
            pt->m_Block->Execute(pStk3) ))          // GetRetVar said if it is interrupted
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

void CBotFunction::RestoreCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass)
{
    CBotTypResult   type;
    CBotFunction*   pt = FindLocalOrPublic(nIdent, name, ppVars, type);

    if ( pt != NULL )
    {
        CBotStack*  pStk = pStack->RestoreStack(pt);
        if ( pStk == NULL ) return;
        pStk->SetBotCall(pt->m_pProg);                  // it may have changed module

        CBotVar*    pthis = pStk->FindVar("this");
        pthis->SetUniqNum(-2);

        CBotStack*  pStk3 = pStk->RestoreStack(NULL);   // to set parameters passed
        if ( pStk3 == NULL ) return;

        pt->m_Param->RestoreState(pStk3, true);                 // parameters

        if ( pStk->GetState() > 1 &&                        // latching is effective?
             pt->m_bSynchro )
            {
                CBotProgram* pProgBase = pStk->GetBotCall(true);
                pClass->Lock(pProgBase);                    // locks the class
            }

        // finally calls the found function

        pt->m_Block->RestoreState(pStk3, true);                 // interrupt !
    }
}

// see if the "signature" of parameters is identical
bool CBotFunction::CheckParam(CBotDefParam* pParam)
{
    CBotDefParam*   pp = m_Param;
    while ( pp != NULL && pParam != NULL )
    {
        CBotTypResult type1 = pp->GetType();
        CBotTypResult type2 = pParam->GetType();
        if ( !type1.Compare(type2) ) return false;
        pp = pp->GetNext();
        pParam = pParam->GetNext();
    }
    return ( pp == NULL && pParam == NULL );
}

CBotString CBotFunction::GetName()
{
    return  m_token.GetString();
}

CBotString CBotFunction::GetParams()
{
    if ( m_Param == NULL ) return CBotString("()");

    CBotString      params = "( ";
    CBotDefParam*   p = m_Param;        // list of parameters

    while (p != NULL)
    {
        params += p->GetParamString();
        p = p->GetNext();
        if ( p != NULL ) params += ", ";
    }

    params += " )";
    return params;
}

CBotFunction* CBotFunction::Next()
{
    return  m_next;
}

void CBotFunction::AddPublic(CBotFunction* func)
{
    if ( m_listPublic != NULL )
    {
        func->m_nextpublic = m_listPublic;
        m_listPublic->m_prevpublic = func;
    }
    m_listPublic = func;
}



/////////////////////////////////////////////////////////////////////////
// management of parameters


CBotDefParam::CBotDefParam()
{
    m_next   = NULL;
    m_nIdent = 0;
}

CBotDefParam::~CBotDefParam()
{
    delete  m_next;
}


// compiles a list of parameters
CBotDefParam* CBotDefParam::Compile(CBotToken* &p, CBotCStack* pStack)
{
    // mainly not pStack->TokenStack here
    // declared variables must remain visible thereafter

    pStack->SetStartError(p->GetStart());

    if (IsOfType(p, ID_OPENPAR))
    {
        CBotDefParam* list = NULL;

        while (!IsOfType(p, ID_CLOSEPAR))
        {
            CBotDefParam* param = new CBotDefParam();
            if (list == NULL) list = param;
            else list->AddNext(param);          // added to the list

//            CBotClass*  pClass = NULL;//= CBotClass::Find(p);
            param->m_typename = p->GetString();
            CBotTypResult type = param->m_type = TypeParam(p, pStack);
//          if ( type == CBotTypPointer ) type = CBotTypClass;          // we must create a new object

            if (param->m_type.GetType() > 0)
            {
                CBotToken*  pp = p;
                param->m_token = *p;
                if (pStack->IsOk() && IsOfType(p, TokenTypVar) )
                {

                    // variable already declared?
                    if (pStack->CheckVarLocal(pp))
                    {
                        pStack->SetError(TX_REDEFVAR, pp);
                        break;
                    }

                    if ( type.Eq(CBotTypArrayPointer) ) type.SetType(CBotTypArrayBody);
                    CBotVar*    var = CBotVar::Create(pp->GetString(), type);       // creates the variable
//                  if ( pClass ) var->SetClass(pClass);
                    var->SetInit(2);                                    // mark initialized
                    param->m_nIdent = CBotVar::NextUniqNum();
                    var->SetUniqNum(param->m_nIdent);
                    pStack->AddVar(var);                                // place on the stack

                    if (IsOfType(p, ID_COMMA) || p->GetType() == ID_CLOSEPAR)
                        continue;
                }
                pStack->SetError(TX_CLOSEPAR, p->GetStart());
            }
            pStack->SetError(TX_NOTYP, p);
            delete list;
            return NULL;
        }
        return list;
    }
    pStack->SetError(TX_OPENPAR, p->GetStart());
    return NULL;
}

void CBotDefParam::AddNext(CBotDefParam* p)
{
    CBotDefParam*   pp = this;
    while (pp->m_next != NULL) pp = pp->m_next;

    pp->m_next = p;
}


bool CBotDefParam::Execute(CBotVar** ppVars, CBotStack* &pj)
{
    int             i = 0;
    CBotDefParam*   p = this;

    while ( p != NULL )
    {
        // creates a local variable on the stack
        CBotVar*    newvar = CBotVar::Create(p->m_token.GetString(), p->m_type);

        // serves to make the transformation of types:
        if ( ppVars != NULL && ppVars[i] != NULL )
        {
            switch (p->m_type.GetType())
            {
            case CBotTypInt:
                newvar->SetValInt(ppVars[i]->GetValInt());
                break;
            case CBotTypFloat:
                newvar->SetValFloat(ppVars[i]->GetValFloat());
                break;
            case CBotTypString:
                newvar->SetValString(ppVars[i]->GetValString());
                break;
            case CBotTypBoolean:
                newvar->SetValInt(ppVars[i]->GetValInt());
                break;
            case CBotTypIntrinsic:
                (static_cast<CBotVarClass*>(newvar))->Copy(ppVars[i], false);
                break;
            case CBotTypPointer:
            case CBotTypArrayPointer:
                {
                    newvar->SetPointer(ppVars[i]->GetPointer());
                }
                break;
            default:
                ASM_TRAP();
            }
        }
        newvar->SetUniqNum(p->m_nIdent);
        pj->AddVar(newvar);     // add a variable
        p = p->m_next;
        i++;
    }

    return true;
}

void CBotDefParam::RestoreState(CBotStack* &pj, bool bMain)
{
//    int             i = 0;
    CBotDefParam*   p = this;

    while ( p != NULL )
    {
        // creates a local variable on the stack
        CBotVar*    var = pj->FindVar(p->m_token.GetString());
        var->SetUniqNum(p->m_nIdent);
        p = p->m_next;
    }
}

int CBotDefParam::GetType()
{
    return  m_type.GetType();
}

CBotTypResult CBotDefParam::GetTypResult()
{
    return  m_type;
}

CBotDefParam* CBotDefParam::GetNext()
{
    return  m_next;
}

CBotString CBotDefParam::GetParamString()
{
    CBotString  param;

    param = m_typename;
    param += ' ';

    param += m_token.GetString();
    return param;
}



//////////////////////////////////////////////////////////////////////////
// return parameters

CBotReturn::CBotReturn()
{
    m_Instr = NULL;
    name = "CBotReturn";        // debug
}

CBotReturn::~CBotReturn()
{
    delete  m_Instr;
}

CBotInstr* CBotReturn::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;

    if (!IsOfType(p, ID_RETURN)) return NULL;   // should never happen

    CBotReturn* inst = new CBotReturn();        // creates the object
    inst->SetToken( pp );

    CBotTypResult   type = pStack->GetRetType();

    if ( type.GetType() == 0 )                  // returned void ?
    {
        if ( IsOfType( p, ID_SEP ) ) return inst;
        pStack->SetError( TX_BADTYPE, pp );
        return NULL;
    }

    inst->m_Instr = CBotExpression::Compile(p, pStack);
    if ( pStack->IsOk() )
    {
        CBotTypResult   retType = pStack->GetTypResult(2);
        if (TypeCompatible(retType, type, ID_ASS))
        {
            if ( IsOfType( p, ID_SEP ) )
                return inst;

            pStack->SetError(TX_ENDOF, p->GetStart());
        }
        pStack->SetError(TX_BADTYPE, p->GetStart());
    }

    delete inst;
    return NULL;                            // no object, the error is on the stack
}

bool CBotReturn::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return true;

    if ( pile->GetState() == 0 )
    {
        if ( m_Instr != NULL && !m_Instr->Execute(pile) ) return false; // evaluate the result
        // the result is on the stack
        pile->IncState();
    }

    if ( pile->IfStep() ) return false;

    pile->SetBreak(3, CBotString());
    return pj->Return(pile);
}

void CBotReturn::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;
    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == NULL ) return;

    if ( pile->GetState() == 0 )
    {
        if ( m_Instr != NULL ) m_Instr->RestoreState(pile, bMain);  // evaluate the result
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Calls of these functions

CBotInstrCall::CBotInstrCall()
{
    m_Parameters = NULL;
    m_nFuncIdent = 0;
    name = "CBotInstrCall";
}

CBotInstrCall::~CBotInstrCall()
{
    delete  m_Parameters;
}

CBotInstr* CBotInstrCall::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotVar*    ppVars[1000];

    int         i = 0;

    CBotToken*  pp = p;
    p = p->GetNext();

    pStack->SetStartError(p->GetStart());
    CBotCStack* pile = pStack;

    if ( IsOfType(p, ID_OPENPAR) )
    {
        int start, end;
        CBotInstrCall* inst = new CBotInstrCall();
        inst->SetToken(pp);

        // compile la list of parameters
        if (!IsOfType(p, ID_CLOSEPAR)) while (true)
        {
            start = p->GetStart();
            pile = pile->TokenStack();                      // keeps the results on the stack

            CBotInstr*  param = CBotExpression::Compile(p, pile);
            end   = p->GetStart();
            if ( inst->m_Parameters == NULL ) inst->m_Parameters = param;
            else inst->m_Parameters->AddNext(param);            // constructs the list

            if ( !pile->IsOk() )
            {
                delete inst;
                return pStack->Return(NULL, pile);
            }

            if ( param != NULL )
            {
                if ( pile->GetTypResult().Eq(99) )
                {
                    delete pStack->TokenStack();
                    pStack->SetError(TX_VOID, p->GetStart());
                    delete inst;
                    return NULL;
                }
                ppVars[i] = pile->GetVar();
                ppVars[i]->GetToken()->SetPos(start, end);
                i++;

                if (IsOfType(p, ID_COMMA)) continue;            // skips the comma
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(TX_CLOSEPAR, p->GetStart());
            delete pStack->TokenStack();
            delete inst;
            return NULL;
        }
        ppVars[i] = NULL;

        // the routine is known?
//      CBotClass*  pClass = NULL;
        inst->m_typRes = pStack->CompileCall(pp, ppVars, inst->m_nFuncIdent);
        if ( inst->m_typRes.GetType() >= 20 )
        {
//          if (pVar2!=NULL) pp = pVar2->RetToken();
            pStack->SetError( inst->m_typRes.GetType(), pp );
            delete pStack->TokenStack();
            delete inst;
            return NULL;
        }

        delete pStack->TokenStack();
        if ( inst->m_typRes.GetType() > 0 )
        {
            CBotVar* pRes = CBotVar::Create("", inst->m_typRes);
            pStack->SetVar(pRes);   // for knowing the type of the result
        }
        else pStack->SetVar(NULL);          // routine returns void

        return inst;
    }
    p = pp;
    delete pStack->TokenStack();
    return NULL;
}

bool CBotInstrCall::Execute(CBotStack* &pj)
{
    CBotVar*    ppVars[1000];
    CBotStack*  pile  = pj->AddStack(this);
    if ( pile->StackOver() ) return pj->Return( pile );

//    CBotStack*  pile1 = pile;

    int     i = 0;

    CBotInstr*  p = m_Parameters;
    // evaluates parameters
    // and places the values ​​on the stack
    // for allow of interruption at any time
    if ( p != NULL) while ( true )
    {
        pile = pile->AddStack();                        // place on the stack for the results
        if ( pile->GetState() == 0 )
        {
            if (!p->Execute(pile)) return false;        // interrupted here?
            pile->SetState(1);                          // mark as special for reknowed parameters \TODO marque spéciale pour reconnaîre parameters
        }
        ppVars[i++] = pile->GetVar();
        p = p->GetNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotStack* pile2 = pile->AddStack();
    if ( pile2->IfStep() ) return false;

    if ( !pile2->ExecuteCall(m_nFuncIdent, GetToken(), ppVars, m_typRes)) return false; // interrupt

    return pj->Return(pile2);   // release the entire stack
}

void CBotInstrCall::RestoreState(CBotStack* &pj, bool bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile  = pj->RestoreStack(this);
    if ( pile == NULL ) return;

//    CBotStack*  pile1 = pile;

    int         i = 0;
    CBotVar*    ppVars[1000];
    CBotInstr*  p = m_Parameters;
    // evaluate parameters
    // and place the values on the stack
    // for allow of interruption at any time
    if ( p != NULL) while ( true )
    {
        pile = pile->RestoreStack();                        // place on the stack for the results
        if ( pile == NULL ) return;
        if ( pile->GetState() == 0 )
        {
            p->RestoreState(pile, bMain);                   // interrupt here!
            return;
        }
        ppVars[i++] = pile->GetVar();               // constructs the list of parameters
        p = p->GetNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotStack* pile2 = pile->RestoreStack();
    if ( pile2 == NULL ) return;

    pile2->RestoreCall(m_nFuncIdent, GetToken(), ppVars);
}

//////////////////////////////////////////////////////////////////////////////
// statement of user classes

// pre-compile a new class
// analysis is complete except the body of routines

CBotClass* CBotClass::Compile1(CBotToken* &p, CBotCStack* pStack)
{
    if ( !IsOfType(p, ID_PUBLIC) )
    {
        pStack->SetError(TX_NOPUBLIC, p);
        return NULL;
    }

    if ( !IsOfType(p, ID_CLASS) ) return NULL;

    CBotString name = p->GetString();

    CBotClass* pOld = CBotClass::Find(name);
    if ( pOld != NULL && pOld->m_IsDef )
    {
        pStack->SetError( TX_REDEFCLASS, p );
        return NULL;
    }

    // a name of the class is there?
    if (IsOfType(p, TokenTypVar))
    {
        CBotClass* pPapa = NULL;
        if ( IsOfType( p, ID_EXTENDS ) )
        {
            CBotString name = p->GetString();
            pPapa = CBotClass::Find(name);

            if (!IsOfType(p, TokenTypVar) || pPapa == NULL )
            {
                pStack->SetError( TX_NOCLASS, p );
                return NULL;
            }
        }
        CBotClass* classe = (pOld == NULL) ? new CBotClass(name, pPapa) : pOld;
        classe->Purge();                            // emptythe old definitions
        classe->m_IsDef = false;                    // current definition

        if ( !IsOfType( p, ID_OPBLK) )
        {
            pStack->SetError(TX_OPENBLK, p);
            return NULL;
        }

        while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
        {
            classe->CompileDefItem(p, pStack, false);
        }

        if (pStack->IsOk()) return classe;
    }
    pStack->SetError(TX_ENDOF, p);
    return NULL;
}

bool CBotClass::CompileDefItem(CBotToken* &p, CBotCStack* pStack, bool bSecond)
{
    bool    bStatic = false;
    int     mProtect = PR_PUBLIC;
    bool    bSynchro = false;

    while (IsOfType(p, ID_SEP)) ;

    CBotTypResult   type( -1 );

    if ( IsOfType(p, ID_SYNCHO) ) bSynchro = true;
    CBotToken*      pBase = p;

    if ( IsOfType(p, ID_STATIC) ) bStatic = true;
    if ( IsOfType(p, ID_PUBLIC) ) mProtect = PR_PUBLIC;
    if ( IsOfType(p, ID_PRIVATE) ) mProtect = PR_PRIVATE;
    if ( IsOfType(p, ID_PROTECTED) ) mProtect = PR_PROTECT;
    if ( IsOfType(p, ID_STATIC) ) bStatic = true;

//  CBotClass* pClass = NULL;
    type = TypeParam(p, pStack);        // type of the result

    if ( type.Eq(-1) )
    {
        pStack->SetError(TX_NOTYP, p);
        return false;
    }

    while (pStack->IsOk())
    {
        CBotToken*  pp = p;
        IsOfType(p, ID_NOT);    // skips ~ eventual (destructor)

        if (IsOfType(p, TokenTypVar))
        {
            CBotInstr* limites = NULL;
            while ( IsOfType( p, ID_OPBRK ) )   // a table?
            {
                CBotInstr* i = NULL;

                if ( p->GetType() != ID_CLBRK )
                    i = CBotExpression::Compile( p, pStack );           // expression for the value
                else
                    i = new CBotEmpty();                            // special if not a formula

                type = CBotTypResult(CBotTypArrayPointer, type);

                if (!pStack->IsOk() || !IsOfType( p, ID_CLBRK ) )
                {
                    pStack->SetError(TX_CLBRK, p->GetStart());
                    return false;
                }

/*              CBotVar* pv = pStack->GetVar();
                if ( pv->GetType()>= CBotTypBoolean )
                {
                    pStack->SetError(TX_BADTYPE, p->GetStart());
                    return false;
                }*/

                if (limites == NULL) limites = i;
                else limites->AddNext3(i);
            }

            if ( p->GetType() == ID_OPENPAR )
            {
                if ( !bSecond )
                {
                    p = pBase;
                    CBotFunction* f =
                    CBotFunction::Compile1(p, pStack, this);

                    if ( f == NULL ) return false;

                    if (m_pMethod == NULL) m_pMethod = f;
                    else m_pMethod->AddNext(f);
                }
                else
                {
                    // return a method precompiled in pass 1
                    CBotFunction*   pf = m_pMethod;
                    CBotFunction*   prev = NULL;
                    while ( pf != NULL )
                    {
                        if (pf->GetName() == pp->GetString()) break;
                        prev = pf;
                        pf = pf->Next();
                    }

                    bool bConstructor = (pp->GetString() == GetName());
                    CBotCStack* pile = pStack->TokenStack(NULL, true);

                    // make "this" known
                    CBotToken TokenThis(CBotString("this"), CBotString());
                    CBotVar* pThis = CBotVar::Create(&TokenThis, CBotTypResult( CBotTypClass, this ) );
                    pThis->SetUniqNum(-2);
                    pile->AddVar(pThis);

                    if ( m_pParent )
                    {
                        // makes "super" known
                        CBotToken TokenSuper(CBotString("super"), CBotString());
                        CBotVar* pThis = CBotVar::Create(&TokenSuper, CBotTypResult( CBotTypClass, m_pParent ) );
                        pThis->SetUniqNum(-3);
                        pile->AddVar(pThis);
                    }

//                  int num = 1;
                    CBotClass*  my = this;
                    while (my != NULL)
                    {
                        // places a copy of variables of a class (this) on a stack
                        CBotVar* pv = my->m_pVar;
                        while (pv != NULL)
                        {
                            CBotVar* pcopy = CBotVar::Create(pv);
                            pcopy->SetInit(!bConstructor || pv->IsStatic());
                            pcopy->SetUniqNum(pv->GetUniqNum());
                            pile->AddVar(pcopy);
                            pv = pv->GetNext();
                        }
                        my = my->m_pParent;
                    }

                    // compiles a method
                    p = pBase;
                    CBotFunction* f =
                    CBotFunction::Compile(p, pile, NULL/*, false*/);

                    if ( f != NULL )
                    {
                        f->m_pProg = pStack->GetBotCall();
                        f->m_bSynchro = bSynchro;
                        // replaces the element in the chain
                        f->m_next = pf->m_next;
                        pf->m_next = NULL;
                        delete pf;
                        if (prev == NULL) m_pMethod = f;
                        else prev->m_next = f;
                    }
                    pStack->Return(NULL, pile);
                }

                return pStack->IsOk();
            }

            // definition of an element
            if (type.Eq(0))
            {
                pStack->SetError(TX_ENDOF, p);
                return false;
            }

            CBotInstr* i = NULL;
            if ( IsOfType(p, ID_ASS ) )
            {
                if ( type.Eq(CBotTypArrayPointer) )
                {
                    i = CBotListArray::Compile(p, pStack, type.GetTypElem());
                }
                else
                {
                    // it has an assignmet to calculate
                    i = CBotTwoOpExpr::Compile(p, pStack);
                }
                if ( !pStack->IsOk() ) return false;
            }


            if ( !bSecond )
            {
                CBotVar*    pv = CBotVar::Create(pp->GetString(), type);
                pv -> SetStatic( bStatic );
                pv -> SetPrivate( mProtect );

                AddItem( pv );

                pv->m_InitExpr = i;
                pv->m_LimExpr = limites;


                if ( pv->IsStatic() && pv->m_InitExpr != NULL )
                {
                    CBotStack* pile = CBotStack::FirstStack();              // independent stack
                    while(pile->IsOk() && !pv->m_InitExpr->Execute(pile));  // evaluates the expression without timer
                    pv->SetVal( pile->GetVar() ) ;
                    pile->Delete();
                }
            }
            else
                delete i;

            if ( IsOfType(p, ID_COMMA) ) continue;
            if ( IsOfType(p, ID_SEP) ) break;
        }
        pStack->SetError(TX_ENDOF, p);
    }
    return pStack->IsOk();
}


CBotClass* CBotClass::Compile(CBotToken* &p, CBotCStack* pStack)
{
    if ( !IsOfType(p, ID_PUBLIC) ) return NULL;
    if ( !IsOfType(p, ID_CLASS) ) return NULL;

    CBotString name = p->GetString();

    // a name for the class is there?
    if (IsOfType(p, TokenTypVar))
    {
        // the class was created by Compile1
        CBotClass* pOld = CBotClass::Find(name);

        if ( IsOfType( p, ID_EXTENDS ) )
        {
            IsOfType(p, TokenTypVar); // necessarily
        }
        IsOfType( p, ID_OPBLK); // necessarily

        while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
        {
            pOld->CompileDefItem(p, pStack, true);
        }

        pOld->m_IsDef = true;           // complete definition
        if (pStack->IsOk()) return pOld;
    }
    pStack->SetError(TX_ENDOF, p);
    return NULL;
}

