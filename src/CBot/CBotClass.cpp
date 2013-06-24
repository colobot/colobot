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
// * along with this program. If not, see  http://www.gnu.org/licenses/.//

/////////////////////////////////////////////////////////////////////
// Management of variables of class type
//

#include "CBot.h"


CBotClass* CBotClass::m_ExClass = NULL;

CBotClass::CBotClass(const char* name, CBotClass* pPapa, bool bIntrinsic)
{
    m_pParent   = pPapa;
    m_name      = name;
    m_pVar      = NULL;
    m_next      = NULL;
    m_pCalls    = NULL;
    m_pMethod   = NULL;
    m_rMaj      = NULL;
    m_IsDef     = true;
    m_bIntrinsic= bIntrinsic;
    m_cptLock   = 0;
    m_cptOne    = 0;
    m_nbVar     = m_pParent == NULL ? 0 : m_pParent->m_nbVar;

    for ( int j= 0; j< 5 ; j++ )
    {
        m_ProgInLock[j] = NULL;
    }


    // is located alone in the list
    if (m_ExClass) m_ExClass->m_ExPrev = this;
    m_ExNext  = m_ExClass;
    m_ExPrev  = NULL;
    m_ExClass = this;

}

CBotClass::~CBotClass()
{
    // removes the list of class
    if ( m_ExPrev ) m_ExPrev->m_ExNext = m_ExNext;
    else m_ExClass = m_ExNext;

    if ( m_ExNext ) m_ExNext->m_ExPrev = m_ExPrev;
    m_ExPrev = NULL;
    m_ExNext = NULL;

    delete  m_pVar;
    delete  m_pCalls;
    delete  m_pMethod;

    delete  m_next;         // releases all of them on this level
}


void CBotClass::Free()
{
    while ( m_ExClass != NULL )
    {
        delete m_ExClass;
    }
}

void CBotClass::Purge()
{
    if ( this == NULL ) return;

    delete      m_pVar;
    m_pVar      = NULL;
    delete      m_pCalls;
    m_pCalls    = NULL;
    delete      m_pMethod;
    m_pMethod   = NULL;
    m_IsDef     = false;

    m_nbVar     = m_pParent == NULL ? 0 : m_pParent->m_nbVar;

    m_next->Purge();
    m_next = NULL;          // no longer belongs to this chain
}

bool CBotClass::Lock(CBotProgram* p)
{
    int i = m_cptLock++;

    if ( i == 0 )
    {
        m_cptOne = 1;
        m_ProgInLock[0] = p;
        return true;
    }
    if ( p == m_ProgInLock[0] )
    {
        m_cptOne++;
        m_cptLock--;                                // has already been counted
        return true;
    }

    for ( int j = 1 ; j <= i ; j++)
    {
        if ( p == m_ProgInLock[j] )
        {
            m_cptLock--;
            return false;   // already pending
        }
    }

    if ( i < 5 )    // max 5 in query
    {
        m_ProgInLock[i] = p;                        // located in a queue
    }
    else
        m_cptLock--;

    return false;
}

void CBotClass::Unlock()
{
    if ( --m_cptOne > 0 ) return ;

    int i = --m_cptLock;
    if ( i<0 )
    {
        m_cptLock = 0;
        return;
    }

    for ( int j= 0; j< i ; j++ )
    {
        m_ProgInLock[j] = m_ProgInLock[j+1];
    }
    m_ProgInLock[i] = 0;
}

void CBotClass::FreeLock(CBotProgram* p)
{
    CBotClass* pClass = m_ExClass;

    while ( pClass != NULL )
    {
        if ( p == pClass->m_ProgInLock[0] )
        {
            pClass->m_cptLock -= pClass->m_cptOne;
            pClass->m_cptOne = 0;
        }

        for ( int j = 1; j < 5 ; j++ )
            if ( p == pClass->m_ProgInLock[j] )
                pClass->m_cptLock--;

        pClass = pClass->m_ExNext;
    }
}



bool CBotClass::AddItem(CBotString name, CBotTypResult type, int mPrivate)
{
    CBotToken   token(name, CBotString());
    CBotClass*  pClass = type.GetClass();

    CBotVar*    pVar = CBotVar::Create( name, type );
/// pVar->SetUniqNum(CBotVar::NextUniqNum());
    pVar->SetPrivate( mPrivate );

    if ( pClass != NULL )
    {
//      pVar->SetClass(pClass);
        if ( type.Eq(CBotTypClass) )
        {
            // adds a new statement for the object initialization
            pVar->m_InitExpr = new CBotNew() ;
            CBotToken nom( pClass->GetName() );
            pVar->m_InitExpr->SetToken(&nom);
        }
    }
    return AddItem( pVar );
}


bool CBotClass::AddItem(CBotVar* pVar)
{
    pVar->SetUniqNum(++m_nbVar);

    if ( m_pVar == NULL ) m_pVar = pVar;
    else m_pVar->AddNext(pVar);

    return true;
}

void CBotClass::AddNext(CBotClass* pClass)
{
    CBotClass*      p = this;
    while (p->m_next != NULL) p = p->m_next;

    p->m_next = pClass;
}

CBotString  CBotClass::GetName()
{
    return m_name;
}

CBotClass*  CBotClass::GetParent()
{
    if ( this == NULL ) return NULL;
    return m_pParent;
}

bool  CBotClass::IsChildOf(CBotClass* pClass)
{
    CBotClass* p = this;
    while ( p != NULL )
    {
        if ( p == pClass ) return true;
        p = p->m_pParent;
    }
    return false;
}


CBotVar* CBotClass::GetVar()
{
    return  m_pVar;
}

CBotVar* CBotClass::GetItem(const char* name)
{
    CBotVar*    p = m_pVar;

    while ( p != NULL )
    {
        if ( p->GetName() == name ) return p;
        p = p->GetNext();
    }
    if ( m_pParent != NULL ) return m_pParent->GetItem(name);
    return NULL;
}

CBotVar* CBotClass::GetItemRef(int nIdent)
{
    CBotVar*    p = m_pVar;

    while ( p != NULL )
    {
        if ( p->GetUniqNum() == nIdent ) return p;
        p = p->GetNext();
    }
    if ( m_pParent != NULL ) return m_pParent->GetItemRef(nIdent);
    return NULL;
}

bool CBotClass::IsIntrinsic()
{
    return  m_bIntrinsic;
}

CBotClass* CBotClass::Find(CBotToken* &pToken)
{
    return Find(pToken->GetString());
}

CBotClass* CBotClass::Find(const char* name)
{
    CBotClass*  p = m_ExClass;

    while ( p != NULL )
    {
        if ( p->GetName() == name ) return p;
        p = p->m_ExNext;
    }

    return NULL;
}

bool CBotClass::AddFunction(const char* name,
                                bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception),
                                CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar))
{
    // stores pointers to the two functions
    CBotCallMethode*    p = m_pCalls;
    CBotCallMethode*    pp = NULL;

    while ( p != NULL )
    {
        if ( name == p->GetName() )
        {
            if ( pp == NULL ) m_pCalls = p->m_next;
            else              pp->m_next = p->m_next;
            delete p;
            break;
        }
        pp = p;
        p = p->m_next;
    }

    p = new CBotCallMethode(name, rExec, rCompile);

    if (m_pCalls == NULL) m_pCalls = p;
    else    m_pCalls->AddNext(p);               // added to the list

    return true;
}

bool CBotClass::AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) )
{
    m_rMaj = rMaj;
    return true;
}

// compiles a method associated with an instance of class
// the method can be declared by the user or AddFunction

CBotTypResult CBotClass::CompileMethode(const char* name,
                                        CBotVar* pThis, CBotVar** ppParams,
                                        CBotCStack* pStack, long& nIdent)
{
    nIdent = 0; // forget the previous one if necessary

    // find the methods declared by AddFunction

    CBotTypResult r = m_pCalls->CompileCall(name, pThis, ppParams, pStack, nIdent);
    if ( r.GetType() >= 0) return r;

    // find the methods declared by user

    r = m_pMethod->CompileCall(name, ppParams, nIdent);
    if ( r.Eq(TX_UNDEFCALL) && m_pParent != NULL )
        return m_pParent->m_pMethod->CompileCall(name, ppParams, nIdent);
    return r;
}

// executes a method

bool CBotClass::ExecuteMethode(long& nIdent, const char* name,
                               CBotVar* pThis, CBotVar** ppParams,
                               CBotVar* &pResult, CBotStack* &pStack,
                               CBotToken* pToken)
{
    int ret = m_pCalls->DoCall(nIdent, name, pThis, ppParams, pResult, pStack, pToken);
    if (ret>=0) return ret;

    ret = m_pMethod->DoCall(nIdent, name, pThis, ppParams, pStack, pToken, this);
    return ret;
}

// restored the execution stack

void CBotClass::RestoreMethode(long& nIdent, const char* name, CBotVar* pThis,
                               CBotVar** ppParams, CBotStack* &pStack)
{
    m_pMethod->RestoreCall(nIdent, name, pThis, ppParams, pStack, this);
}




bool CBotClass::SaveStaticState(FILE* pf)
{
    if (!WriteWord( pf, CBOTVERSION*2)) return false;

    // saves the state of static variables in classes
    CBotClass*  p = m_ExClass;

    while ( p != NULL )
    {
        if (!WriteWord( pf, 1)) return false;
        // save the name of the class
        if (!WriteString( pf, p->GetName() )) return false;

        CBotVar*    pv = p->GetVar();
        while( pv != NULL )
        {
            if ( pv->IsStatic() )
            {
                if (!WriteWord( pf, 1)) return false;
                if (!WriteString( pf, pv->GetName() )) return false;

                if ( !pv->Save0State(pf)) return false;             // common header
                if ( !pv->Save1State(pf) ) return false;                // saves as the child class
                if ( !WriteWord( pf, 0)) return false;
            }
            pv = pv->GetNext();
        }

        if (!WriteWord( pf, 0)) return false;
        p = p->m_ExNext;
    }

    if (!WriteWord( pf, 0)) return false;
    return true;
}

bool CBotClass::RestoreStaticState(FILE* pf)
{
    CBotString      ClassName, VarName;
    CBotClass*      pClass;
    unsigned short  w;

    if (!ReadWord( pf, w )) return false;
    if ( w != CBOTVERSION*2 ) return false;

    while (true)
    {
        if (!ReadWord( pf, w )) return false;
        if ( w == 0 ) return true;

        if (!ReadString( pf, ClassName )) return false;
        pClass = Find(ClassName);

        while (true)
        {
            if (!ReadWord( pf, w )) return false;
            if ( w == 0 ) break;

            CBotVar*    pVar = NULL;
            CBotVar*    pv = NULL;

            if (!ReadString( pf, VarName )) return false;
            if ( pClass != NULL ) pVar = pClass->GetItem(VarName);

            if (!CBotVar::RestoreState(pf, pv)) return false;   // the temp variable

            if ( pVar != NULL ) pVar->Copy(pv);
            delete pv;
        }
    }
    return true;
}


/////////////////////////////////////////////////////////////////////

CBotClassInst::CBotClassInst()
{
    m_next          = NULL;
    m_var           = NULL;
    m_Parameters    = NULL;
    m_expr          = NULL;
    m_hasParams     = false;
    m_nMethodeIdent = 0;
    name = "CBotClassInst";
}

CBotClassInst::~CBotClassInst()
{
    delete m_var;
//  delete m_next;          // done by the destructor of the base class ~CBotInstr()
}

// definition of pointer (s) to an object
// style
// CPoint A, B ;

CBotInstr* CBotClassInst::Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass)
{
    // seeks the corresponding classes
    if ( pClass == NULL )
    {
        pStack->SetStartError(p->GetStart());
        pClass = CBotClass::Find(p);
        if ( pClass == NULL )
        {
            // not found? is bizare
            pStack->SetError(TX_NOCLASS, p);
            return NULL;
        }
        p = p->GetNext();
    }

    bool        bIntrinsic = pClass->IsIntrinsic();
    CBotTypResult type = CBotTypResult( bIntrinsic ? CBotTypIntrinsic : CBotTypPointer, pClass );
    CBotClassInst*  inst = static_cast<CBotClassInst*>(CompileArray(p, pStack, type));
    if ( inst != NULL || !pStack->IsOk() ) return inst;

    CBotCStack* pStk = pStack->TokenStack();

    inst = new CBotClassInst();
    /// \TODO Need to be revised and fixed after adding unit tests
    CBotToken token(pClass->GetName(), CBotString(), p->GetStart(), p->GetEnd());
    inst->SetToken(&token);
    CBotToken*  vartoken = p;

    if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        (static_cast<CBotLeftExprVar*>(inst->m_var))->m_typevar = type;
        if (pStk->CheckVarLocal(vartoken))                  // redefinition of the variable
        {
            pStk->SetStartError(vartoken->GetStart());
            pStk->SetError(TX_REDEFVAR, vartoken->GetEnd());
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))                         // with any clues?
        {
            delete inst;                                    // is not type CBotInt
            p = vartoken;                                   // returns to the variable name

            // compiles declaration an array

            inst = static_cast<CBotClassInst*>(CBotInstArray::Compile( p, pStk, type ));

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GetStart());
                goto error;
            }
            goto suite;         // no assignment, variable already created
        }


        CBotVar*    var;
        var = CBotVar::Create(vartoken->GetString(), type); // creates the instance
//      var->SetClass(pClass);
        var->SetUniqNum(
            (static_cast<CBotLeftExprVar*>(inst->m_var))->m_nIdent = CBotVar::NextUniqNum());
                                                            // its attribute a unique number
        pStack->AddVar(var);                                // placed on the stack

        // look if there are parameters
        inst->m_hasParams = (p->GetType() == ID_OPENPAR);

        CBotVar*    ppVars[1000];
        inst->m_Parameters = CompileParams(p, pStk, ppVars);
        if ( !pStk->IsOk() ) goto error;

        // if there are parameters, is the equivalent to the stament "new"
        // CPoint A ( 0, 0 ) is equivalent to
        // CPoint A = new CPoint( 0, 0 )

//      if ( NULL != inst->m_Parameters )
        if ( inst->m_hasParams )
        {
            // the constructor is there?
//          CBotString  noname;
            CBotTypResult r = pClass->CompileMethode(pClass->GetName(), var, ppVars, pStk, inst->m_nMethodeIdent);
            delete pStk->TokenStack();                          // releases the supplement stack
            int typ = r.GetType();

            if (typ == TX_UNDEFCALL)
            {
                // si le constructeur n'existe pas
                if (inst->m_Parameters != NULL)                 // with parameters
                {
                    pStk->SetError(TX_NOCONST, vartoken);
                    goto error;
                }
                typ = 0;
            }

            if (typ>20)
            {
                pStk->SetError(typ, vartoken->GetEnd());
                goto error;
            }

        }

        if (IsOfType(p,  ID_ASS))                           // with a assignment?
        {
            if (inst->m_hasParams)
            {
                pStk->SetError(TX_ENDOF, p->GetStart());
                goto error;
            }

            if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
            {
                goto error;
            }
            CBotClass* result = pStk->GetClass();
            if ( !pStk->GetTypResult(1).Eq(CBotTypNullPointer) &&
               ( !pStk->GetTypResult(1).Eq(CBotTypPointer) ||
                 ( result != NULL && !pClass->IsChildOf(result) )))     // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GetStart());
                goto error;
            }
//          if ( !bIntrinsic ) var->SetPointer(pStk->GetVar()->GetPointer());
            if ( !bIntrinsic )
            {
                // does not use the result on the stack, to impose the class
                CBotVar* pvar = CBotVar::Create("", pClass);
                var->SetPointer( pvar );                    // variable already declared instance pointer
                delete pvar;                                // removes the second pointer
            }
            var->SetInit(true);                         // marks the pointer as init
        }
        else if (inst->m_hasParams)
        {
            // creates the object on the "job" (\TODO "tas")
            // with a pointer to the object
            if ( !bIntrinsic )
            {
                CBotVar* pvar = CBotVar::Create("", pClass);
                var->SetPointer( pvar );                    // variable already declared instance pointer
                delete pvar;                                // removes the second pointer
            }
            var->SetInit(2);                            // marks the pointer as init
        }
suite:
        if (IsOfType(p,  ID_COMMA))                         // several chained definitions
        {
            if ( NULL != ( inst->m_next = CBotClassInst::Compile(p, pStk, pClass) ))    // compiles the following
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (IsOfType(p,  ID_SEP))                           // complete instruction
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GetStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// declaration of the instance of a class, for example:
//  CPoint A, B;

bool CBotClassInst::Execute(CBotStack* &pj)
{
    CBotVar*    pThis = NULL;

    CBotStack*  pile = pj->AddStack(this);//essential for SetState()
//  if ( pile == EOX ) return true;

    CBotToken*  pt = &m_token;
    CBotClass*  pClass = CBotClass::Find(pt);

    bool bIntrincic = pClass->IsIntrinsic();

    // creates the variable of type pointer to the object

    if ( pile->GetState()==0)
    {
        CBotString  name = m_var->m_token.GetString();
        if ( bIntrincic )
        {
            pThis = CBotVar::Create(name, CBotTypResult( CBotTypIntrinsic, pClass ));
        }
        else
        {
            pThis = CBotVar::Create(name, CBotTypResult( CBotTypPointer, pClass ));
        }

        pThis->SetUniqNum((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent); // its attribute as unique number
        pile->AddVar(pThis);                                    // place on the stack
        pile->IncState();
    }

    if ( pThis == NULL ) pThis = pile->FindVar((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent);

    if ( pile->GetState()<3)
    {
        // ss there an assignment or parameters (contructor)

//      CBotVarClass* pInstance = NULL;

        if ( m_expr != NULL )
        {
            // evaluates the expression for the assignment
            if (!m_expr->Execute(pile)) return false;

            if ( bIntrincic )
            {
                CBotVar*    pv = pile->GetVar();
                if ( pv == NULL || pv->GetPointer() == NULL )
                {
                    pile->SetError(TX_NULLPT, &m_token);
                    return pj->Return(pile);
                }
                pThis->Copy(pile->GetVar(), false);
            }
            else
            {
                CBotVarClass* pInstance;
                pInstance = (static_cast<CBotVarPointer*>(pile->GetVar()))->GetPointer();    // value for the assignment
                pThis->SetPointer(pInstance);
            }
            pThis->SetInit(true);
        }

        else if ( m_hasParams )
        {
            // evaluates the constructor of an instance

            if ( !bIntrincic && pile->GetState() == 1)
            {
                CBotToken*  pt = &m_token;
                CBotClass* pClass = CBotClass::Find(pt);

                // creates an instance of the requested class

                CBotVarClass* pInstance;
                pInstance = static_cast<CBotVarClass*>(CBotVar::Create("", pClass));
                pThis->SetPointer(pInstance);
                delete pInstance;

                pile->IncState();
            }

            CBotVar*    ppVars[1000];
            CBotStack*  pile2 = pile;

            int     i = 0;

            CBotInstr*  p = m_Parameters;
            // evaluates the parameters
            // and places the values ​​on the stack
            // to (can) be interrupted (broken) at any time

            if ( p != NULL) while ( true )
            {
                pile2 = pile2->AddStack();                      // place on the stack for the results
                if ( pile2->GetState() == 0 )
                {
                    if (!p->Execute(pile2)) return false;       // interrupted here?
                    pile2->SetState(1);
                }
                ppVars[i++] = pile2->GetVar();
                p = p->GetNext();
                if ( p == NULL) break;
            }
            ppVars[i] = NULL;

            // creates a variable for the result
            CBotVar*    pResult = NULL;     // constructor still void

            if ( !pClass->ExecuteMethode(m_nMethodeIdent, pClass->GetName(),
                                         pThis, ppVars,
                                         pResult, pile2, GetToken())) return false; // interrupt

            pThis->SetInit(true);
            pThis->ConstructorSet();        // indicates that the constructor has been called
            pile->Return(pile2);                                // releases a piece of stack

//          pInstance = pThis->GetPointer();

        }

//      if ( !bIntrincic ) pThis->SetPointer(pInstance);        // a pointer to the instance

        pile->SetState(3);                                  // finished this part
    }

    if ( pile->IfStep() ) return false;

    if ( m_next2b != NULL &&
        !m_next2b->Execute(pile)) return false;             // other (s) definition (s)

    return pj->Return( pile );                              // transmits below (further)
}



void CBotClassInst::RestoreState(CBotStack* &pj, bool bMain)
{
    CBotVar*    pThis = NULL;

    CBotStack*  pile = pj;
    if ( bMain ) pile = pj->RestoreStack(this);
    if ( pile == NULL ) return;

    // creates the variable of type pointer to the object
    {
        CBotString  name = m_var->m_token.GetString();
        pThis = pile->FindVar(name);
        pThis->SetUniqNum((static_cast<CBotLeftExprVar*>(m_var))->m_nIdent); // its attribute a unique number
    }

    CBotToken*  pt = &m_token;
    CBotClass*  pClass = CBotClass::Find(pt);
    bool bIntrincic = pClass->IsIntrinsic();

    if ( bMain && pile->GetState()<3)
    {
        // is there an assignment or parameters (constructor)

//      CBotVarClass* pInstance = NULL;

        if ( m_expr != NULL )
        {
            // evaluates the expression for the assignment
            m_expr->RestoreState(pile, bMain);
            return;
        }

        else if ( m_hasParams )
        {
            // evaluates the constructor of an instance

            if ( !bIntrincic && pile->GetState() == 1)
            {
                return;
            }

            CBotVar*    ppVars[1000];
            CBotStack*  pile2 = pile;

            int     i = 0;

            CBotInstr*  p = m_Parameters;
            // evaluates the parameters
            // and the values an the stack
            // for the ability to be interrupted at any time (\TODO pour pouvoir être interrompu n'importe quand)

            if ( p != NULL) while ( true )
            {
                pile2 = pile2->RestoreStack();                      // place on the stack for the results
                if ( pile2 == NULL ) return;

                if ( pile2->GetState() == 0 )
                {
                    p->RestoreState(pile2, bMain);      // interrupted here?
                    return;
                }
                ppVars[i++] = pile2->GetVar();
                p = p->GetNext();
                if ( p == NULL) break;
            }
            ppVars[i] = NULL;

            // creates a variable for the result
//            CBotVar*    pResult = NULL;     // constructor still void

            pClass->RestoreMethode(m_nMethodeIdent, pClass->GetName(), pThis, ppVars, pile2);
            return;
        }
    }

    if ( m_next2b != NULL )
         m_next2b->RestoreState(pile, bMain);                   // other(s) definition(s)
}


// test if a procedure name is already defined somewhere

bool CBotClass::CheckCall(CBotToken* &pToken, CBotDefParam* pParam)
{
    CBotString  name = pToken->GetString();

    if ( CBotCall::CheckCall(name) ) return true;

    CBotFunction*   pp = m_pMethod;
    while ( pp != NULL )
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are their parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
        pp = pp->Next();
    }

    return false;
}

