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
// * along with this program. If not, see  http://www.gnu.org/licenses/.///////////////////////////////////////////////////////////////////////
// compilation des diverses instructions
// toutes les routines Compile sont statiques
// et retournent un object selon ce qui a été trouvé comme instruction

// principe de compilation:
//  les routines Compile retournent un objet de la classe correspondant à l'opération trouvée
//  il s'agit toujours d'une classe fille de CBotInstr.
//  ( les objets CBotInstr ne sont jamais utilisés directement )

//  si la routine Compile retourne NULL, c'est que l'instruction est fausse
//  ou incomprise.
//  L'erreur se trouve alors sur la pile CBotCStack::IsOk() est FALSE


#include "CBot.h"




// les divers constructeurs / destructeurs
// pour libérer tout selon l'arbre établi
CBotInstr::CBotInstr()
{
    name     = "CBotInstr";
    m_next   = NULL;
    m_next2b = NULL;
    m_next3  = NULL;
    m_next3b = NULL;
}

CBotInstr::~CBotInstr()
{
    delete  m_next;
    delete  m_next2b;
    delete  m_next3;
    delete  m_next3b;
}

// compteur de boucles imbriquées,
// pour détermniner les break et continue valides
// et liste des labels utilisables

int CBotInstr::m_LoopLvl    = 0;
CBotStringArray
    CBotInstr::m_labelLvl   = CBotStringArray();

// ajoute un niveau avec un label
void CBotInstr::IncLvl(CBotString& label)
{
    m_labelLvl.SetSize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = label;
    m_LoopLvl++;
}

// ajoute un niveau (instruction switch)
void CBotInstr::IncLvl()
{
    m_labelLvl.SetSize(m_LoopLvl+1);
    m_labelLvl[m_LoopLvl] = "#SWITCH";
    m_LoopLvl++;
}

// libère un niveau
void CBotInstr::DecLvl()
{
    m_LoopLvl--;
    m_labelLvl[m_LoopLvl].Empty();
}

// controle la validité d'un break ou continu
BOOL CBotInstr::ChkLvl(const CBotString& label, int type)
{
    int i = m_LoopLvl;
    while (--i>=0)
    {
        if ( type == ID_CONTINUE && m_labelLvl[i] == "#SWITCH") continue;
        if ( label.IsEmpty() ) return TRUE;
        if ( m_labelLvl[i] == label ) return TRUE;
    }
    return FALSE;
}

BOOL CBotInstr::IsOfClass(CBotString n)
{
    return name == n;
}


////////////////////////////////////////////////////////////////////////////
// gestion de base de la classe CBotInstr

// définie le token correspondant à l'instruction

void CBotInstr::SetToken(CBotToken* p)
{
    m_token = *p;
}

void CBotInstr::SetToken(CBotString* name, int start, int end)
{
    SetToken( &CBotToken( *name, CBotString(), start, end));
}


// rend le type du token associé à l'instruction

int CBotInstr::GivTokenType()
{
    return m_token.GivType();
}

// rend le token associé

CBotToken* CBotInstr::GivToken()
{
    return &m_token;
}

// ajoute une instruction à la suite des autres

void CBotInstr::AddNext(CBotInstr* n)
{
    CBotInstr*  p = this;
    while ( p->m_next != NULL ) p = p->m_next;
    p->m_next = n;
}

void CBotInstr::AddNext3(CBotInstr* n)
{
    CBotInstr*  p = this;
    while ( p->m_next3 != NULL ) p = p->m_next3;
    p->m_next3 = n;
}

void CBotInstr::AddNext3b(CBotInstr* n)
{
    CBotInstr*  p = this;
    while ( p->m_next3b != NULL ) p = p->m_next3b;
    p->m_next3b = n;
}

// donne l'instruction suivante

CBotInstr* CBotInstr::GivNext()
{
    return m_next;
}

CBotInstr* CBotInstr::GivNext3()
{
    return m_next3;
}

CBotInstr* CBotInstr::GivNext3b()
{
    return m_next3b;
}

///////////////////////////////////////////////////////////////////////////
// compile une instruction, qui peut être
//  while, do, try, throw, if, for, switch, break, continu, return
//  int, float, boolean, string,
//  déclaration d'une instance d'une classe
//  expression quelconque

CBotInstr* CBotInstr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;

    if ( p == NULL ) return NULL;

    int type = p->GivType();            // quel est le prochaine token ?

    // y a-t-il un label ?
    if ( IsOfType( pp, TokenTypVar ) &&
         IsOfType( pp, ID_DOTS ) )
    {
         type = pp->GivType();
         // seules ces instructions acceptent un label
         if (!IsOfTypeList( pp, ID_WHILE, ID_FOR, ID_DO, ID_REPEAT, 0 ))
         {
             pStack->SetError(TX_LABEL, pp->GivStart());
             return NULL;
         }
    }

    // appel la routine de compilation correspondant au token trouvé
    switch (type)
    {
    case ID_WHILE:
        return CBotWhile::Compile(p, pStack);

    case ID_FOR:
        return CBotFor::Compile(p, pStack);

    case ID_DO:
        return CBotDo::Compile(p, pStack);

    case ID_REPEAT:
        return CBotRepeat::Compile(p, pStack);

    case ID_BREAK:
    case ID_CONTINUE:
        return CBotBreak::Compile(p, pStack);

    case ID_SWITCH:
        return CBotSwitch::Compile(p, pStack);

    case ID_TRY:
        return CBotTry::Compile(p, pStack);

    case ID_THROW:
        return CBotThrow::Compile(p, pStack);

    case ID_DEBUGDD:
        return CBotStartDebugDD::Compile(p, pStack);

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
        pStack->SetStartError(p->GivStart());
        pStack->SetError(TX_ELSEWITHOUTIF, p->GivEnd());
        return NULL;

    case ID_CASE:
        pStack->SetStartError(p->GivStart());
        pStack->SetError(TX_OUTCASE, p->GivEnd());
        return NULL;
    }

    pStack->SetStartError(p->GivStart());

    // ne doit pas être un mot réservé par DefineNum
    if ( p->GivType() == TokenTypDef )
    {
        pStack->SetError(TX_RESERVED, p);
        return NULL;
    }

    // ce peut être une définition d'instance de class
    CBotToken*  ppp = p;
    if ( IsOfType( ppp, TokenTypVar ) /* && IsOfType( ppp, TokenTypVar )*/ )
    {
        if ( CBotClass::Find(p) != NULL )
        {
            // oui, compile la déclaration de l'instance
            return CBotClassInst::Compile(p, pStack);
        }
    }

    // ce peut être une instruction arithmétique
    CBotInstr*  inst = CBotExpression::Compile(p, pStack);
    if (IsOfType(p, ID_SEP))
    {
        return inst;
    }
    pStack->SetError(TX_ENDOF, p->GivStart());
    delete inst;
    return NULL;
}

BOOL CBotInstr::Execute(CBotStack* &pj)
{
    CBotString  ClassManquante = name;
    ASM_TRAP();         // ne doit jamais passer par cette routine
                            // mais utiliser les routines des classes filles
    return FALSE;
}

BOOL CBotInstr::Execute(CBotStack* &pj, CBotVar* pVar)
{
    if ( !Execute(pj) ) return FALSE;
    pVar->SetVal( pj->GivVar() );
    return TRUE;
}

void CBotInstr::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotString  ClassManquante = name;
    ASM_TRAP();         // ne doit jamais passer par cette routine
                            // mais utiliser les routines des classes filles
}


BOOL CBotInstr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    ASM_TRAP();         // papa sait pas faire, voir les filles
    return FALSE;
}

BOOL CBotInstr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep, BOOL bExtend)
{
    ASM_TRAP();         // papa sait pas faire, voir les filles
    return FALSE;
}

void CBotInstr::RestoreStateVar(CBotStack* &pile, BOOL bMain)
{
    ASM_TRAP();         // papa sait pas faire, voir les filles
}

// cette routine n'est définie que pour la classe fille CBotCase
// cela permet de faire l'appel CompCase sur toutes les instructions
// pour savoir s'il s'agit d'un case pour la valeur désirée.

BOOL CBotInstr::CompCase(CBotStack* &pj, int val)
{
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile un bloc d'instruction " { i ; i ; } "

// cette classe n'a pas de constructeur, car il n'y a jamais d'instance de cette classe
// l'objet retourné par Compile est généralement de type CBotListInstr


CBotInstr* CBotBlock::Compile(CBotToken* &p, CBotCStack* pStack, BOOL bLocal)
{
    pStack->SetStartError(p->GivStart());

    if (IsOfType(p, ID_OPBLK))
    {
        CBotInstr* inst = CBotListInstr::Compile( p, pStack, bLocal );

        if (IsOfType(p, ID_CLBLK))
        {
            return inst;
        }

        pStack->SetError(TX_CLOSEBLK, p->GivStart());   // manque la parenthèse
        delete inst;
        return NULL;
    }

    pStack->SetError(TX_OPENBLK, p->GivStart());
    return NULL;
}

CBotInstr* CBotBlock::CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, BOOL bLocal)
{
    // est-ce un nouveau bloc ?
    if ( p->GivType() == ID_OPBLK ) return CBotBlock::Compile(p, pStack);

    // sinon, cherche une instruction unique à la place

    // pour gérer les cas avec définition local à l'instructin (*)
    CBotCStack* pStk = pStack->TokenStack(p, bLocal);

    return pStack->Return( CBotInstr::Compile(p, pStk), // une instruction unique
                           pStk);
}

// (*) c'est le cas dans l'instruction suivante
// if ( 1 == 1 ) int x = 0;
// où la variable x n'est connue que dans le bloc qui suit le if.


//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile une liste d'instruction, séparés par des points-virgules

CBotListInstr::CBotListInstr()
{
    m_Instr     = NULL;
    name = "CBotListInstr";
}

CBotListInstr::~CBotListInstr()
{
    delete  m_Instr;
}

CBotInstr* CBotListInstr::Compile(CBotToken* &p, CBotCStack* pStack, BOOL bLocal)
{
    CBotCStack* pStk = pStack->TokenStack(p, bLocal);       // les variables sont locales

    CBotListInstr* inst = new CBotListInstr();

    while (TRUE)
    {
        if ( p == NULL ) break;

        if (IsOfType(p, ID_SEP)) continue;              // instruction vide ignorée
        if ( p->GivType() == ID_CLBLK ) break;          // déja plus d'instruction

        if (IsOfType(p, 0))
        {
            pStack->SetError(TX_CLOSEBLK, p->GivStart());
            delete inst;
            return pStack->Return(NULL, pStk);
        }

        CBotInstr* i = CBotBlock::CompileBlkOrInst( p, pStk );  // compile la suivante

        if (!pStk->IsOk())
        {
            delete inst;
            return pStack->Return(NULL, pStk);
        }

        if ( inst->m_Instr == NULL ) inst->m_Instr = i;
        else inst->m_Instr->AddNext(i);                         // ajoute à la suite
    }
    return pStack->Return(inst, pStk);
}

// exécute une liste d'instructions

BOOL CBotListInstr::Execute(CBotStack* &pj)
{

    CBotStack*  pile = pj->AddStack(this, TRUE);//indispensable pour SetState()
    if ( pile->StackOver() ) return pj->Return( pile );


    CBotInstr*  p = m_Instr;                                    // la première expression

    int     state = pile->GivState();
    while (state-->0) p = p->GivNext();                         // revient sur l'opération interrompue

    if ( p != NULL ) while (TRUE)
    {
//      DEBUG( "CBotListInstr", pile->GivState(), pile );

        if ( !p->Execute(pile) ) return FALSE;
        p = p->GivNext();
        if ( p == NULL ) break;
        if (!pile->IncState()) ;//return FALSE;                 // prêt pour la suivante
    }

    return pj->Return( pile );                  // transmet en dessous
}

void CBotListInstr::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == NULL ) return;

    CBotInstr*  p = m_Instr;                                    // la première expression

    int     state = pile->GivState();
    while ( p != NULL && state-- > 0)
    {
        p->RestoreState(pile, FALSE);
        p = p->GivNext();                           // revient sur l'opération interrompue
    }

    if ( p != NULL ) p->RestoreState(pile, TRUE);
}

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// compilation d'un élément se trouvant à gauche d'une assignation

CBotLeftExprVar::CBotLeftExprVar()
{
    name    = "CBotLeftExprVar";
    m_typevar   = -1;
    m_nIdent    =  0;
}

CBotLeftExprVar::~CBotLeftExprVar()
{
}

CBotInstr* CBotLeftExprVar::Compile(CBotToken* &p, CBotCStack* pStack)
{
    // vérifie que le token est un nom de variable
    if (p->GivType() != TokenTypVar)
    {
        pStack->SetError( TX_NOVAR, p->GivStart());
        return NULL;
    }

    CBotLeftExprVar* inst = new CBotLeftExprVar();
    inst->SetToken(p);
    p = p->GivNext();

    return inst;
}

// crée une variable et lui assigne le résultat de la pile
BOOL CBotLeftExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     var1;
    CBotVar*     var2;

    var1 = CBotVar::Create(m_token.GivString(), m_typevar);
    var1->SetUniqNum(m_nIdent);                 // avec cet identificateur unique
    pj->AddVar(var1);                           // la place sur la pile

    var2 = pj->GivVar();                        // resultat sur la pile
    if ( var2 ) var1->SetVal(var2);             // fait l'assignation

    return TRUE;                                // opération faite
}

void CBotLeftExprVar::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotVar*     var1;

    var1 = pj->FindVar(m_token.GivString());
    if ( var1 == NULL ) ASM_TRAP();

    var1->SetUniqNum(m_nIdent);                 // avec cet identificateur unique
}

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// définition d'un tableau de n'importe quel type
// int a[12];
// point x[];

CBotInstArray::CBotInstArray()
{
    m_var     = NULL;
    m_listass = NULL;
    name = "CBotInstArray";
}

CBotInstArray::~CBotInstArray()
{
    delete m_var;
    delete m_listass;
}


CBotInstr* CBotInstArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotInstArray*  inst = new CBotInstArray();     // crée l'objet

    CBotToken*  vartoken = p;
    inst->SetToken(vartoken);

    // détermine l'expression valable pour l'élément gauche
    if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        if (pStk->CheckVarLocal(vartoken))                  // redéfinition de la variable ?
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        CBotInstr*  i;
        while (IsOfType(p,  ID_OPBRK))                          // avec des indices ?
        {
            if ( p->GivType() != ID_CLBRK )
                i = CBotExpression::Compile( p, pStk );         // expression pour la valeur
            else
                i = new CBotEmpty();                            // spécial si pas de formule

            inst->AddNext3b(i);                                 // construit une liste
            type = CBotTypResult(CBotTypArrayPointer, type);

            if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ) )
            {
                pStk->SetError(TX_CLBRK, p->GivStart());
                goto error;
            }
        }

        CBotVar*   var = CBotVar::Create(vartoken, type);       // crée avec une instance
        inst->m_typevar = type;

        var->SetUniqNum(
            ((CBotLeftExprVar*)inst->m_var)->m_nIdent = CBotVar::NextUniqNum());
                                                            // lui attribut un numéro unique
        pStack->AddVar(var);                                    // la place sur la pile

        if ( IsOfType(p, ID_ASS) )                          // avec une assignation
        {
            inst->m_listass = CBotListArray::Compile( p, pStk, type.GivTypElem() );
        }

        if ( pStk->IsOk() ) return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}


// exécute la définition d'un tableau

BOOL CBotInstArray::Execute(CBotStack* &pj)
{
    CBotStack*  pile1 = pj->AddStack(this);
//  if ( pile1 == EOX ) return TRUE;

    CBotStack*  pile  = pile1;

    if ( pile1->GivState() == 0 )
    {
        // cherche les dimensions max du tableau
        CBotInstr*  p  = GivNext3b();                           // les différentes formules
        int         nb = 0;

        while (p != NULL)
        {
            pile = pile->AddStack();                            // petite place pour travailler
            nb++;
            if ( pile->GivState() == 0 )
            {
                if ( !p->Execute(pile) ) return FALSE;              // calcul de la taille // interrompu?
                pile->IncState();
            }
            p = p->GivNext3b();
        }

        p    = GivNext3b();
        pile = pile1;                                           // revient sur la pile
        int  n = 0;
        int  max[100];

        while (p != NULL)
        {
            pile = pile->AddStack();                            // récupère la même petite place
            CBotVar*    v = pile->GivVar();                     // résultat
            max[n] = v->GivValInt();                            // valeur
            if (max[n]>MAXARRAYSIZE)
            {
                pile->SetError(TX_OUTARRAY, &m_token);
                return pj->Return ( pile );
            }
            n++;
            p = p->GivNext3b();
        }
        while (n<100) max[n++] = 0;

        m_typevar.SetArray( max );              // mémorise les limitations

        // crée simplement un pointeur null
        CBotVar*    var = CBotVar::Create(m_var->GivToken(), m_typevar);
        var->SetPointer(NULL);
        var->SetUniqNum(((CBotLeftExprVar*)m_var)->m_nIdent);
        pj->AddVar(var);                                        // inscrit le tableau de base sur la pile

#if     STACKMEM
        pile1->AddStack()->Delete();
#else
        delete pile1->AddStack();                               // plus besoin des indices
#endif
        pile1->IncState();
    }

    if ( pile1->GivState() == 1 )
    {
        if ( m_listass != NULL )                                // il y a des assignation pour ce tableau
        {
            CBotVar* pVar = pj->FindVar(((CBotLeftExprVar*)m_var)->m_nIdent);

            if ( !m_listass->Execute(pile1, pVar) ) return FALSE;
        }
        pile1->IncState();
    }

    if ( pile1->IfStep() ) return FALSE;                            // montre ce pas ?

    if ( m_next2b &&
         !m_next2b->Execute( pile1 ) ) return FALSE;

    return pj->Return( pile1 );                         // transmet en dessous
}

void CBotInstArray::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotStack*  pile1 = pj;

    CBotVar*    var = pj->FindVar(m_var->GivToken()->GivString());
    if ( var != NULL ) var->SetUniqNum(((CBotLeftExprVar*)m_var)->m_nIdent);

    if ( bMain )
    {
                    pile1 = pj->RestoreStack(this);
        CBotStack*  pile  = pile1;
        if ( pile == NULL ) return;

        if ( pile1->GivState() == 0 )
        {
            // cherche les dimensions max du tableau
            CBotInstr*  p  = GivNext3b();                           // les différentes formules

            while (p != NULL)
            {
                pile = pile->RestoreStack();                        // petite place pour travailler
                if ( pile == NULL ) return;
                if ( pile->GivState() == 0 )
                {
                    p->RestoreState(pile, bMain);                   // calcul de la taille // interrompu!
                    return;
                }
                p = p->GivNext3b();
            }
        }
        if ( pile1->GivState() == 1 && m_listass != NULL )
        {
            m_listass->RestoreState(pile1, bMain);
        }

    }


    if ( m_next2b ) m_next2b->RestoreState( pile1, bMain );
}

// cas particulier pour les indices vides
BOOL CBotEmpty :: Execute(CBotStack* &pj)
{
    CBotVar*    pVar = CBotVar::Create("", CBotTypInt);
    pVar->SetValInt(-1);                                    // met la valeur -1 sur la pile
    pj->SetVar(pVar);
    return TRUE;
}

void CBotEmpty :: RestoreState(CBotStack* &pj, BOOL bMain)
{
}

//////////////////////////////////////////////////////////////////////////////////////
// définition d'une liste d'initialisation pour un tableau
// int [ ] a [ ] = ( ( 1, 2, 3 ) , ( 3, 2, 1 ) ) ;


CBotListArray::CBotListArray()
{
    m_expr  = NULL;
    name = "CBotListArray";
}

CBotListArray::~CBotListArray()
{
    delete m_expr;
}


CBotInstr* CBotListArray::Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type)
{
    CBotCStack* pStk = pStack->TokenStack(p);

    CBotToken* pp = p;

    if ( IsOfType( p, ID_NULL ) )
    {
        CBotInstr* inst = new CBotExprNull ();
        inst->SetToken( pp );
//      CBotVar* var = CBotVar::Create("", CBotTypNullPointer);
//      pStk->SetVar(var);
        return pStack->Return(inst, pStk);          // ok avec élément vide
    }

    CBotListArray*  inst = new CBotListArray();     // crée l'objet

    if ( IsOfType( p, ID_OPENPAR ) )
    {
        // prend chaque élément l'un après l'autre
        if ( type.Eq( CBotTypArrayPointer ) )
        {
            type = type.GivTypElem();

            pStk->SetStartError(p->GivStart());
            if ( NULL == ( inst->m_expr = CBotListArray::Compile( p, pStk, type ) ) )
            {
                goto error;
            }

            while ( IsOfType( p, ID_COMMA ) )                       // d'autres éléments ?
            {
                pStk->SetStartError(p->GivStart());

                CBotInstr* i = CBotListArray::Compile( p, pStk, type );
                if ( NULL == i )
                {
                    goto error;
                }

                inst->m_expr->AddNext3(i);
            }
        }
        else
        {
            pStk->SetStartError(p->GivStart());
            if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
            {
                goto error;
            }
            CBotVar* pv = pStk->GivVar();                       // le résultat de l'expression

            if ( pv == NULL || !TypesCompatibles( type, pv->GivTypResult() ))       // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GivStart());
                goto error;
            }

            while ( IsOfType( p, ID_COMMA ) )                       // d'autres éléments ?
            {
                pStk->SetStartError(p->GivStart());

                CBotInstr* i = CBotTwoOpExpr::Compile( p, pStk ) ;
                if ( NULL == i )
                {
                    goto error;
                }

                CBotVar* pv = pStk->GivVar();                       // le résultat de l'expression

                if ( pv == NULL || !TypesCompatibles( type, pv->GivTypResult() ))       // type compatible ?
                {
                    pStk->SetError(TX_BADTYPE, p->GivStart());
                    goto error;
                }
                inst->m_expr->AddNext3(i);
            }
        }

        if (!IsOfType(p, ID_CLOSEPAR) )
        {
            pStk->SetError(TX_CLOSEPAR, p->GivStart());
            goto error;
        }

        return pStack->Return(inst, pStk);
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}


// exécute la définition d'un tableau

BOOL CBotListArray::Execute(CBotStack* &pj, CBotVar* pVar)
{
    CBotStack*  pile1 = pj->AddStack();
//  if ( pile1 == EOX ) return TRUE;
    CBotVar* pVar2;

    CBotInstr* p = m_expr;

    int n = 0;

    for ( ; p != NULL ; n++, p = p->GivNext3() )
    {
        if ( pile1->GivState() > n ) continue;

        pVar2 = pVar->GivItem(n, TRUE);

        if ( !p->Execute(pile1, pVar2) ) return FALSE;      // évalue l'expression

        pile1->IncState();
    }

    return pj->Return( pile1 );                         // transmet en dessous
}

void CBotListArray::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain )
    {
        CBotStack*  pile  = pj->RestoreStack(this);
        if ( pile == NULL ) return;

        CBotInstr* p = m_expr;

        int state = pile->GivState();

        while( state-- > 0 ) p = p->GivNext3() ;

        p->RestoreState(pile, bMain);                   // calcul de la taille // interrompu!
    }
}

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// définition d'une variable entière
// int a, b = 12;

CBotInt::CBotInt()
{
    m_next  = NULL;         // pour les définitions multiples
    m_var   =
    m_expr  = NULL;
    name = "CBotInt";
}

CBotInt::~CBotInt()
{
    delete m_var;
    delete m_expr;
//  delete m_next;          // fait par le destructeur de la classe de base ~CBotInstr()
}

CBotInstr* CBotInstr::CompileArray(CBotToken* &p, CBotCStack* pStack, CBotTypResult type, BOOL first)
{
    if ( IsOfType(p, ID_OPBRK) )
    {
        if ( !IsOfType(p, ID_CLBRK) )
        {
            pStack->SetError(TX_CLBRK, p->GivStart());
            return NULL;
        }

        CBotInstr*  inst = CompileArray(p, pStack, CBotTypResult( CBotTypArrayPointer, type ), FALSE);
        if ( inst != NULL || !pStack->IsOk() ) return inst;
    }

    // compile une déclaration de tableau
    if (first) return NULL ;

    CBotInstr* inst = CBotInstArray::Compile( p, pStack, type );
    if ( inst == NULL ) return NULL;

    if (IsOfType(p,  ID_COMMA))                         // plusieurs définitions enchaînées
    {
        if ( NULL != ( inst->m_next2b = CBotInstArray::CompileArray(p, pStack, type, FALSE) ))  // compile la suivante
        {
            return inst;
        }
        delete inst;
        return NULL;
    }

    if (IsOfType(p,  ID_SEP))                           // instruction terminée
    {
        return inst;
    }

    delete inst;
    pStack->SetError(TX_ENDOF, p->GivStart());
    return NULL;
}

CBotInstr* CBotInt::Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont, BOOL noskip)
{
    CBotToken*  pp = cont ? NULL : p;       // pas de répétition du token "int"

    if (!cont && !IsOfType(p, ID_INT)) return NULL;

    CBotInt*    inst = (CBotInt*)CompileArray(p, pStack, CBotTypInt);
    if ( inst != NULL || !pStack->IsOk() ) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotInt();       // crée l'objet

    inst->m_expr = NULL;

    CBotToken*  vartoken = p;
    inst->SetToken( vartoken );

    // détermine l'expression valable pour l'élément gauche
    if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        ((CBotLeftExprVar*)inst->m_var)->m_typevar = CBotTypInt;
        if (pStk->CheckVarLocal(vartoken))                  // redéfinition de la variable
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))                         // avec des indices ?
        {
            delete inst;                                    // n'est pas de type CBotInt
            p = vartoken;                                   // revient sur le nom de la variable

            // compile une déclaration de tableau

            CBotInstr* inst2 = CBotInstArray::Compile( p, pStk, CBotTypInt );

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GivStart());
                goto error;
            }

            if (IsOfType(p,  ID_COMMA))                         // plusieurs définitions enchaînées
            {
                if ( NULL != ( inst2->m_next2b = CBotInt::Compile(p, pStk, TRUE, noskip) )) // compile la suivante
                {
                    return pStack->Return(inst2, pStk);
                }
            }
            inst = (CBotInt*)inst2;
            goto suite;         // pas d'assignation, variable déjà créée
        }

        if (IsOfType(p,  ID_ASS))                           // avec une assignation ?
        {
            if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
            {
                goto error;
            }
            if ( pStk->GivType() >= CBotTypBoolean )        // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GivStart());
                goto error;
            }
        }

        {
            CBotVar*    var = CBotVar::Create(vartoken, CBotTypInt);// crée la variable (après l'assignation évaluée)
            var->SetInit(inst->m_expr != NULL);                 // la marque initialisée si avec assignation
            var->SetUniqNum(
                ((CBotLeftExprVar*)inst->m_var)->m_nIdent = CBotVar::NextUniqNum());
                                                                // lui attribut un numéro unique
            pStack->AddVar(var);                                    // la place sur la pile
        }

        if (IsOfType(p,  ID_COMMA))                         // plusieurs définitions enchaînées
        {
            if ( NULL != ( inst->m_next2b = CBotInt::Compile(p, pStk, TRUE, noskip) ))  // compile la suivante
            {
                return pStack->Return(inst, pStk);
            }
        }
suite:
        if (noskip || IsOfType(p,  ID_SEP))                 // instruction terminée
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GivStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute la définition de la variable entière

BOOL CBotInt::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);  //indispensable pour SetState()
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return FALSE; // valeur initiale // interrompu?
        m_var->Execute( pile );                             // crée et fait l'assigation du résultat

        if (!pile->SetState(1)) return FALSE;
    }

    if ( pile->IfStep() ) return FALSE;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return FALSE;                // autre(s) définition(s)

    return pj->Return( pile );                              // transmet en dessous
}

void CBotInt::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotStack*  pile = pj;
    if ( bMain )
    {
        pile = pj->RestoreStack(this);
        if ( pile == NULL ) return;

        if ( pile->GivState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);  // valeur initiale // interrompu!
            return;
        }
    }

    m_var->RestoreState(pile, bMain);

    if ( m_next2b ) m_next2b->RestoreState(pile, bMain);            // autre(s) définition(s)
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// définition d'une variable booléen
// int a, b = false;

CBotBoolean::CBotBoolean()
{
    m_var   =
    m_expr  = NULL;
    name = "CBotBoolean";
}

CBotBoolean::~CBotBoolean()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotBoolean::Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont, BOOL noskip)
{
    CBotToken*  pp = cont ? NULL : p;

    if (!cont && !IsOfType(p, ID_BOOLEAN, ID_BOOL)) return NULL;

    CBotBoolean*    inst = (CBotBoolean*)CompileArray(p, pStack, CBotTypBoolean);
    if ( inst != NULL || !pStack->IsOk() ) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotBoolean();

    inst->m_expr = NULL;

    CBotToken*  vartoken = p;
    inst->SetToken( vartoken );
    CBotVar*    var = NULL;

    if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        ((CBotLeftExprVar*)inst->m_var)->m_typevar = CBotTypBoolean;
        if (pStk->CheckVarLocal(vartoken))                  // redéfinition de la variable
        {
            pStk->SetError(TX_REDEFVAR, vartoken);
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))                         // avec des indices ?
        {
            delete inst;                                    // n'est pas de type CBotInt
            p = vartoken;                                   // revient sur le nom de la variable

            // compile une déclaration de tableau

            inst = (CBotBoolean*)CBotInstArray::Compile( p, pStk, CBotTypBoolean );

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GivStart());
                goto error;
            }
            goto suite;         // pas d'assignation, variable déjà créée
        }

        if (IsOfType(p,  ID_ASS))                           // avec une assignation ?
        {
            if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
            {
                goto error;
            }
            if ( !pStk->GivTypResult().Eq(CBotTypBoolean) )     // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GivStart());
                goto error;
            }
        }

        var = CBotVar::Create(vartoken, CBotTypBoolean);// crée la variable (après l'assignation évaluée)
        var->SetInit(inst->m_expr != NULL);                 // la marque initialisée si avec assignation
        var->SetUniqNum(
            ((CBotLeftExprVar*)inst->m_var)->m_nIdent = CBotVar::NextUniqNum());
                                                            // lui attribut un numéro unique
        pStack->AddVar(var);                                    // la place sur la pile
suite:
        if (IsOfType(p,  ID_COMMA))                         // plusieurs définitions enchaînées
        {
            if ( NULL != ( inst->m_next2b = CBotBoolean::Compile(p, pStk, TRUE, noskip) ))  // compile la suivante
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))                         // instruction terminée
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GivStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute une définition de variable booléenne

BOOL CBotBoolean::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);//indispensable pour SetState()
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return FALSE; // valeur initiale // interrompu?
        m_var->Execute( pile );                             // crée et fait l'assigation du résultat

        if (!pile->SetState(1)) return FALSE;
    }

    if ( pile->IfStep() ) return FALSE;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return FALSE;    // autre(s) définition(s)

    return pj->Return( pile );                              // transmet en dessous
}

void CBotBoolean::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotStack*  pile = pj;
    if ( bMain )
    {
        pile = pj->RestoreStack(this);
        if ( pile == NULL ) return;

        if ( pile->GivState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);      // valeur initiale interrompu?
            return;
        }
    }

    m_var->RestoreState( pile, bMain );             //

    if ( m_next2b )
         m_next2b->RestoreState(pile, bMain);               // autre(s) définition(s)
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// définition d'une variable réelle
// int a, b = 12.4;

CBotFloat::CBotFloat()
{
    m_var   =
    m_expr  = NULL;
    name = "CBotFloat";
}

CBotFloat::~CBotFloat()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotFloat::Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont, BOOL noskip)
{
    CBotToken*  pp = cont ? NULL : p;

    if (!cont && !IsOfType(p, ID_FLOAT)) return NULL;

    CBotFloat*  inst = (CBotFloat*)CompileArray(p, pStack, CBotTypFloat);
    if ( inst != NULL || !pStack->IsOk() ) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotFloat();

    inst->m_expr = NULL;

    CBotToken*  vartoken = p;
    CBotVar*    var = NULL;
    inst->SetToken(vartoken);

    if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        ((CBotLeftExprVar*)inst->m_var)->m_typevar = CBotTypFloat;
        if (pStk->CheckVarLocal(vartoken))                  // redéfinition de la variable
        {
            pStk->SetStartError(vartoken->GivStart());
            pStk->SetError(TX_REDEFVAR, vartoken->GivEnd());
            goto error;
        }

        if (IsOfType(p,  ID_OPBRK))                         // avec des indices ?
        {
            delete inst;                                    // n'est pas de type CBotInt
            p = vartoken;                                   // revient sur le nom de la variable

            // compile une déclaration de tableau

            inst = (CBotFloat*)CBotInstArray::Compile( p, pStk, CBotTypFloat );

            if (!pStk->IsOk() )
            {
                pStk->SetError(TX_CLBRK, p->GivStart());
                goto error;
            }
            goto suite;         // pas d'assignation, variable déjà créée
        }

        if (IsOfType(p,  ID_ASS))                           // avec une assignation ?
        {
            if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
            {
                goto error;
            }
            if ( pStk->GivType() >= CBotTypBoolean )        // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GivStart());
                goto error;
            }
        }

        var = CBotVar::Create(vartoken, CBotTypFloat);      // crée la variable (après l'assignation évaluée)
        var->SetInit(inst->m_expr != NULL);                 // la marque initialisée si avec assignation
        var->SetUniqNum(
            ((CBotLeftExprVar*)inst->m_var)->m_nIdent = CBotVar::NextUniqNum());
                                                            // lui attribut un numéro unique
        pStack->AddVar(var);                                    // la place sur la pile
suite:
        if (IsOfType(p,  ID_COMMA))                         // plusieurs définitions enchaînées
        {
            if ( NULL != ( inst->m_next2b = CBotFloat::Compile(p, pStk, TRUE, noskip) ))    // compile la suivante
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))                         // instruction terminée
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GivStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute la défintion de la variable réelle

BOOL CBotFloat::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);//indispensable pour SetState()
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return FALSE; // valeur initiale // interrompu?
        m_var->Execute( pile );                             // crée et fait l'assigation du résultat

        if (!pile->SetState(1)) return FALSE;
    }

    if ( pile->IfStep() ) return FALSE;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return FALSE;    // autre(s) définition(s)

    return pj->Return( pile );                              // transmet en dessous
}

void CBotFloat::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotStack*  pile = pj;
    if ( bMain )
    {
        pile = pj->RestoreStack(this);
        if ( pile == NULL ) return;

        if ( pile->GivState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);      // valeur initiale interrompu?
            return;
        }
    }

    m_var->RestoreState( pile, bMain );             //

    if ( m_next2b )
         m_next2b->RestoreState(pile, bMain);               // autre(s) définition(s)
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// définition d'une variable chaîne de caractères
// int a, b = "salut";

CBotIString::CBotIString()
{
    m_var   =
    m_expr  = NULL;
    name = "CBotIString";
}

CBotIString::~CBotIString()
{
    delete m_var;
    delete m_expr;
}

CBotInstr* CBotIString::Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont, BOOL noskip)
{
    CBotToken*  pp = cont ? NULL : p;

    if (!cont && !IsOfType(p, ID_STRING)) return NULL;

    CBotIString*    inst = (CBotIString*)CompileArray(p, pStack, CBotTypString);
    if ( inst != NULL || !pStack->IsOk() ) return inst;

    CBotCStack* pStk = pStack->TokenStack(pp);

    inst = new CBotIString();

    inst->m_expr = NULL;

    CBotToken*  vartoken = p;
    inst->SetToken( vartoken );

    if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
    {
        ((CBotLeftExprVar*)inst->m_var)->m_typevar = CBotTypString;
        if (pStk->CheckVarLocal(vartoken))                  // redéfinition de la variable
        {
            pStk->SetStartError(vartoken->GivStart());
            pStk->SetError(TX_REDEFVAR, vartoken->GivEnd());
            goto error;
        }

        if (IsOfType(p,  ID_ASS))                           // avec une assignation ?
        {
            if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
            {
                goto error;
            }
/*          if ( !pStk->GivTypResult().Eq(CBotTypString) )          // type compatible ?
            {
                pStk->SetError(TX_BADTYPE, p->GivStart());
                goto error;
            }*/
        }

        CBotVar*    var = CBotVar::Create(vartoken, CBotTypString);     // crée la variable (après l'assignation évaluée)
        var->SetInit(inst->m_expr != NULL);                 // la marque initialisée si avec assignation
        var->SetUniqNum(
            ((CBotLeftExprVar*)inst->m_var)->m_nIdent = CBotVar::NextUniqNum());
                                                            // lui attribut un numéro unique
        pStack->AddVar(var);                                    // la place sur la pile

        if (IsOfType(p,  ID_COMMA))                         // plusieurs définitions enchaînées
        {
            if ( NULL != ( inst->m_next2b = CBotIString::Compile(p, pStk, TRUE, noskip) ))  // compile la suivante
            {
                return pStack->Return(inst, pStk);
            }
        }

        if (noskip || IsOfType(p,  ID_SEP))                         // instruction terminée
        {
            return pStack->Return(inst, pStk);
        }

        pStk->SetError(TX_ENDOF, p->GivStart());
    }

error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute la définition de la variable string

BOOL CBotIString::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);//indispensable pour SetState()
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState()==0)
    {
        if (m_expr && !m_expr->Execute(pile)) return FALSE; // valeur initiale // interrompu?
        m_var->Execute( pile );                             // crée et fait l'assigation du résultat

        if (!pile->SetState(1)) return FALSE;
    }

    if ( pile->IfStep() ) return FALSE;

    if ( m_next2b &&
         !m_next2b->Execute(pile)) return FALSE;    // autre(s) définition(s)

    return pj->Return( pile );                              // transmet en dessous
}

void CBotIString::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotStack*  pile = pj;

    if ( bMain )
    {
        pile = pj->RestoreStack(this);
        if ( pile == NULL ) return;

        if ( pile->GivState()==0)
        {
            if (m_expr) m_expr->RestoreState(pile, bMain);      // valeur initiale interrompu?
            return;
        }
    }

    m_var->RestoreState( pile, bMain );             //

    if ( m_next2b )
         m_next2b->RestoreState(pile, bMain);               // autre(s) définition(s)
}

//////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////
// compile une instruction de type " x = 123 " ou " z * 5 + 4 "
// avec ou sans assignation donc

CBotExpression::CBotExpression()
{
    m_leftop    = NULL;
    m_rightop   = NULL;
    name = "CBotExpression";
}

CBotExpression::~CBotExpression()
{
    delete  m_leftop;
    delete  m_rightop;
}

CBotInstr* CBotExpression::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;

    CBotExpression* inst = new CBotExpression();

    inst->m_leftop = CBotLeftExpr::Compile(p, pStack);

    inst->SetToken(p);
    int  OpType = p->GivType();

    if ( pStack->IsOk() &&
         IsOfTypeList(p, ID_ASS, ID_ASSADD, ID_ASSSUB, ID_ASSMUL, ID_ASSDIV, ID_ASSMODULO,
                                 ID_ASSAND, ID_ASSXOR, ID_ASSOR,
                                 ID_ASSSL , ID_ASSSR,  ID_ASSASR, 0 ))
    {
        if ( inst->m_leftop == NULL )
        {
            pStack->SetError(TX_BADLEFT, p->GivEnd());
            delete inst;
            return NULL;
        }

        inst->m_rightop = CBotExpression::Compile(p, pStack);
        if (inst->m_rightop == NULL)
        {
            delete inst;
            return NULL;
        }

        CBotTypResult   type1 = pStack->GivTypResult();

        // récupère la variable pour la marquer assignée
        CBotVar*    var = NULL;
        inst->m_leftop->ExecuteVar(var, pStack);
        if ( var == NULL )
        {
            delete inst;
            return NULL;
        }

        if (OpType != ID_ASS && var->GivInit() != IS_DEF)
        {
            pStack->SetError(TX_NOTINIT, pp);
            delete inst;
            return NULL;
        }

        CBotTypResult type2 = var->GivTypResult();

        // quels sont les types acceptables ?
        switch (OpType)
        {
        case ID_ASS:
    //      if (type2 == CBotTypClass) type2 = -1;      // pas de classe
            if ( (type1.Eq(CBotTypPointer) && type2.Eq(CBotTypPointer) ) ||
                 (type1.Eq(CBotTypClass)   && type2.Eq(CBotTypClass)   ) )
            {
/*              CBotClass*  c1 = type1.GivClass();
                CBotClass*  c2 = type2.GivClass();
                if ( !c1->IsChildOf(c2) ) type2.SetType(-1);    // pas la même classe
//-             if ( !type1.Eq(CBotTypClass) ) var->SetPointer(pStack->GivVar()->GivPointer());*/
                var->SetInit(2);
            }
            else
                var->SetInit(TRUE);

            break;
        case ID_ASSADD:
            if (type2.Eq(CBotTypBoolean) ||
                type2.Eq(CBotTypPointer) ) type2 = -1;      // nombres et chaines
            break;
        case ID_ASSSUB:
        case ID_ASSMUL:
        case ID_ASSDIV:
        case ID_ASSMODULO:
            if (type2.GivType() >= CBotTypBoolean) type2 = -1;  // nombres uniquement
            break;
        }

        if (!TypeCompatible( type1, type2, OpType ))
        {
            pStack->SetError(TX_BADTYPE, &inst->m_token);
            delete inst;
            return NULL;
        }

        return inst;        // types compatibles ?
    }

    delete inst;
//  p = p->GivNext();
    int start, end, error = pStack->GivError(start, end);

    p = pp;                                     // revient au début
    pStack->SetError(0,0);                      // oublie l'erreur

//  return CBotTwoOpExpr::Compile(p, pStack);   // essaie sans assignation
    CBotInstr* i = CBotTwoOpExpr::Compile(p, pStack);   // essaie sans assignation
    if ( i != NULL && error == TX_PRIVATE && p->GivType() == ID_ASS )
        pStack->ResetError( error, start, end );
    return i;
}

// exécute une expression avec assignation

BOOL CBotExpression::Execute(CBotStack* &pj)
{
    CBotStack*  pile  = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    CBotToken*  pToken = m_leftop->GivToken();
    CBotVar*    pVar = NULL;

    CBotStack*  pile1 = pile;

    BOOL        IsInit = TRUE;
    CBotVar*    result = NULL;

    // doit être fait avant pour les indices éventuels (pile peut être changée)
    if ( !m_leftop->ExecuteVar(pVar, pile, NULL, FALSE) ) return FALSE; // variable avant évaluation de la valeur droite

//  DEBUG( "CBotExpression::Execute", -1, pj);
    if ( pile1->GivState()==0)
    {
        pile1->SetCopyVar(pVar);                                // garde une copie sur la pile (si interrompu)
        pile1->IncState();
    }

    CBotStack*  pile2 = pile->AddStack();                       // attention pile et surtout pas pile1

    if ( pile2->GivState()==0)
    {
//      DEBUG( "CBotExpression::Execute", -2, pj);
        if (m_rightop && !m_rightop->Execute(pile2)) return FALSE;  // valeur initiale // interrompu?
        pile2->IncState();
    }

    if ( pile1->GivState() == 1 )
    {
//      DEBUG( "CBotExpression::Execute", -3, pj);
        if ( m_token.GivType() != ID_ASS )
        {
            pVar = pile1->GivVar();                                     // récupére si interrompu
            IsInit = pVar->GivInit();
            if ( IsInit == IS_NAN )
            {
                pile2->SetError(TX_OPNAN, m_leftop->GivToken());
                return pj->Return(pile2);
            }
            result = CBotVar::Create("", pVar->GivTypResult(2));
        }

        switch ( m_token.GivType() )
        {
        case ID_ASS:
            break;
        case ID_ASSADD:
            result->Add(pile1->GivVar(), pile2->GivVar());          // additionne
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSSUB:
            result->Sub(pile1->GivVar(), pile2->GivVar());          // soustrait
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSMUL:
            result->Mul(pile1->GivVar(), pile2->GivVar());          // multiplie
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSDIV:
            if (IsInit &&
                result->Div(pile1->GivVar(), pile2->GivVar()))      // divise
                pile2->SetError(TX_DIVZERO, &m_token);
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSMODULO:
            if (IsInit &&
                result->Modulo(pile1->GivVar(), pile2->GivVar()))   // reste de la division
                pile2->SetError(TX_DIVZERO, &m_token);
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSAND:
            result->And(pile1->GivVar(), pile2->GivVar());          // multiplie
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSXOR:
            result->XOr(pile1->GivVar(), pile2->GivVar());
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSOR:
            result->Or(pile1->GivVar(), pile2->GivVar());
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSSL:
            result->SL(pile1->GivVar(), pile2->GivVar());
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSSR:
            result->SR(pile1->GivVar(), pile2->GivVar());
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        case ID_ASSASR:
            result->ASR(pile1->GivVar(), pile2->GivVar());
            pile2->SetVar(result);                                  // re-place le résultat
            break;
        default:
            ASM_TRAP();
        }
        if (!IsInit)
            pile2->SetError(TX_NOTINIT, m_leftop->GivToken());

        pile1->IncState();
    }

//  DEBUG( "CBotExpression::Execute", -4, pj);
    if ( !m_leftop->Execute( pile2, pile1 ) )
        return FALSE;                                           // crée et fait l'assigation du résultat

    return pj->Return( pile2 );                                 // transmet en dessous
}


void CBotExpression::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain )
    {
        CBotToken*  pToken = m_leftop->GivToken();
        CBotVar*    pVar = NULL;

        CBotStack*  pile  = pj->RestoreStack(this);
        if ( pile == NULL ) return;

        CBotStack*  pile1 = pile;


        if ( pile1->GivState()==0)
        {
            m_leftop->RestoreStateVar(pile, TRUE);  // variable avant évaluation de la valeur droite
            return;
        }

        m_leftop->RestoreStateVar(pile, FALSE); // variable avant évaluation de la valeur droite

        CBotStack*  pile2 = pile->RestoreStack();                       // attention pile et surtout pas pile1
        if ( pile2 == NULL ) return;

        if ( pile2->GivState()==0)
        {
            if (m_rightop) m_rightop->RestoreState(pile2, bMain);       // valeur initiale // interrompu?
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile une instruction de type " ( condition ) "
// la condition doit être de type booléen

// cette classe n'a pas de constructeur, car il n'y a jamais d'instance de cette classe
// l'objet retourné par Compile est généralement de type CBotExpression

CBotInstr* CBotCondition::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GivStart());
    if ( IsOfType(p, ID_OPENPAR ))
    {
        CBotInstr* inst = CBotBoolExpr::Compile( p, pStack );
        if ( NULL != inst )
        {
            if ( IsOfType(p, ID_CLOSEPAR ))
            {
                return inst;
            }
            pStack->SetError(TX_CLOSEPAR, p->GivStart());   // manque la parenthèse
        }
        delete inst;
    }

    pStack->SetError(TX_OPENPAR, p->GivStart());    // manque la parenthèse

    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////////
// compile une instruction de type " condition "
// la condition doit être de type booléen

// cette classe n'a pas de constructeur, car il n'y a jamais d'instance de cette classe
// l'objet retourné par Compile est généralement de type CBotExpression

CBotInstr* CBotBoolExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GivStart());

    CBotInstr* inst = CBotTwoOpExpr::Compile( p, pStack );

    if ( NULL != inst )
    {
        if ( pStack->GivTypResult().Eq(CBotTypBoolean) )
        {
            return inst;
        }
        pStack->SetError(TX_NOTBOOL, p->GivStart());    // n'est pas un booléan
    }

    delete inst;
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile soit :
//  une instruction entre parenthèses (...)
//  une expression unaire (négatif, not)
//  nom de variable
//  les variables prè et post incrémentées ou décrémentées
//  un nombre donné par DefineNum
//  une constante
//  un appel de procédure
//  l'instruction new

// cette classe n'a pas de constructeur, car il n'y a jamais d'instance de cette classe
// l'objet retourné par Compile est de la classe correspondant à l'instruction

CBotInstr* CBotParExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GivStart());

    // est-ce une expression entre parenthèse ?
    if (IsOfType(p, ID_OPENPAR))
    {
        CBotInstr* inst = CBotExpression::Compile( p, pStk );

        if ( NULL != inst )
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                return pStack->Return(inst, pStk);
            }
            pStk->SetError(TX_CLOSEPAR, p->GivStart());
        }
        delete inst;
        return pStack->Return(NULL, pStk);
    }

    // est-ce une opération unaire ?
    CBotInstr* inst = CBotExprUnaire::Compile(p, pStk);
    if (inst != NULL || !pStk->IsOk())
        return pStack->Return(inst, pStk);

    // est-ce un nom de variable ?
    if (p->GivType() == TokenTypVar)
    {
        // c'est peut-être un appel de méthode sans le "this." devant
        inst =  CBotExprVar::CompileMethode(p, pStk);
        if ( inst != NULL ) return pStack->Return(inst, pStk);


        // est-ce un appel de procédure ?
        inst =  CBotInstrCall::Compile(p, pStk);
        if ( inst != NULL || !pStk->IsOk() )
            return pStack->Return(inst, pStk);


        CBotToken* pvar = p;
        // non, c'est une variable "ordinaire"
        inst =  CBotExprVar::Compile(p, pStk);

        CBotToken* pp = p;
        // post incrémenté ou décrémenté ?
        if (IsOfType(p, ID_INC, ID_DEC))
        {
            if ( pStk->GivType() >= CBotTypBoolean )
            {
                pStk->SetError(TX_BADTYPE, pp);
                delete inst;
                return pStack->Return(NULL, pStk);
            }

            // recompile la variable pour read-only
            delete inst;
            p = pvar;
            inst =  CBotExprVar::Compile(p, pStk, PR_READ);
            p = p->GivNext();

            CBotPostIncExpr* i = new CBotPostIncExpr();
            i->SetToken(pp);
            i->m_Instr = inst;  // instruction associée
            return pStack->Return(i, pStk);
        }
        return pStack->Return(inst, pStk);
    }

    // est-ce une variable préincrémentée ou prédécrémentée ?
    CBotToken* pp = p;
    if (IsOfType(p, ID_INC, ID_DEC))
    {
        CBotPreIncExpr* i = new CBotPreIncExpr();
        i->SetToken(pp);

        if (p->GivType() == TokenTypVar)
        {
            if (NULL != (i->m_Instr =  CBotExprVar::Compile(p, pStk, PR_READ)))
            {
                if ( pStk->GivType() >= CBotTypBoolean )
                {
                    pStk->SetError(TX_BADTYPE, pp);
                    delete inst;
                    return pStack->Return(NULL, pStk);
                }
                return pStack->Return(i, pStk);
            }
            delete i;
            return pStack->Return(NULL, pStk);
        }
    }

    // est-ce un nombre  ou un DefineNum ?
    if (p->GivType() == TokenTypNum ||
        p->GivType() == TokenTypDef )
    {
        CBotInstr* inst = CBotExprNum::Compile( p, pStk );
        return pStack->Return(inst, pStk);
    }

    // est-ce une chaine ?
    if (p->GivType() == TokenTypString)
    {
        CBotInstr* inst = CBotExprAlpha::Compile(p, pStk);
        return pStack->Return(inst, pStk);
    }

    // est un élément "true" ou "false"
    if (p->GivType() == ID_TRUE ||
        p->GivType() == ID_FALSE )
    {
        CBotInstr* inst = CBotExprBool::Compile( p, pStk );
        return pStack->Return(inst, pStk);
    }

    // est un objet à créer avec new
    if (p->GivType() == ID_NEW)
    {
        CBotInstr* inst = CBotNew::Compile( p, pStk );
        return pStack->Return(inst, pStk);
    }

    // est un pointeur nul
    if (IsOfType( p, ID_NULL ))
    {
        CBotInstr* inst = new CBotExprNull ();
        inst->SetToken( pp );
        CBotVar* var = CBotVar::Create("", CBotTypNullPointer);
        pStk->SetVar(var);
        return pStack->Return(inst, pStk);
    }

    // est un nombre nan
    if (IsOfType( p, ID_NAN ))
    {
        CBotInstr* inst = new CBotExprNan ();
        inst->SetToken( pp );
        CBotVar* var = CBotVar::Create("", CBotTypInt);
        var->SetInit(IS_NAN);
        pStk->SetVar(var);
        return pStack->Return(inst, pStk);
    }


    return pStack->Return(NULL, pStk);
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// gestion du post et pré- incrément/décrément

// il n'y a pas de routine Compile, l'objet est créé directement
// dans CBotParExpr::Compile

CBotPostIncExpr::CBotPostIncExpr()
{
    m_Instr = NULL;
    name    = "CBotPostIncExpr";
}

CBotPostIncExpr::~CBotPostIncExpr()
{
    delete  m_Instr;
}

CBotPreIncExpr::CBotPreIncExpr()
{
    m_Instr = NULL;
    name    = "CBotPreIncExpr";
}

CBotPreIncExpr::~CBotPreIncExpr()
{
    delete  m_Instr;
}

BOOL CBotPostIncExpr::Execute(CBotStack* &pj)
{
    CBotStack*  pile1 = pj->AddStack(this);
    CBotStack*  pile2 = pile1;

    CBotVar*    var1 = NULL;

    if ( !((CBotExprVar*)m_Instr)->ExecuteVar(var1, pile2, NULL, TRUE) ) return FALSE;      // récupère la variable selon champs et index

    pile1->SetState(1);
    pile1->SetCopyVar(var1);                                // place le résultat (avant incrémentation);

    CBotStack* pile3 = pile2->AddStack(this);
    if ( pile3->IfStep() ) return FALSE;

    if ( var1->GivInit() == IS_NAN )
    {
        pile1->SetError( TX_OPNAN, &m_token );
    }

    if ( var1->GivInit() != IS_DEF )
    {
        pile1->SetError( TX_NOTINIT, &m_token );
    }

    if (GivTokenType() == ID_INC) var1->Inc();
    else                          var1->Dec();

    return pj->Return(pile1);                       // opération faite, résultat sur pile2
}

void CBotPostIncExpr::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile1 = pj->RestoreStack(this);
    if ( pile1 == NULL ) return;

    ((CBotExprVar*)m_Instr)->RestoreStateVar(pile1, bMain);

    if ( pile1 != NULL ) pile1->RestoreStack(this);
}

BOOL CBotPreIncExpr::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    CBotVar*     var1;

    if ( pile->GivState() == 0 )
    {
        CBotStack*  pile2 = pile;
        if ( !((CBotExprVar*)m_Instr)->ExecuteVar(var1, pile2, NULL, TRUE) ) return FALSE;      // récupère la variable selon champs et index
                                                                                                // pile2 est modifié en retour

        if ( var1->GivInit() == IS_NAN )
        {
            pile->SetError( TX_OPNAN, &m_token );
            return pj->Return(pile);                        // opération faite
        }

        if ( var1->GivInit() != IS_DEF )
        {
            pile->SetError( TX_NOTINIT, &m_token );
            return pj->Return(pile);                        // opération faite
        }

        if (GivTokenType() == ID_INC) var1->Inc();
        else                          var1->Dec();      // ((CBotVarInt*)var1)->m_val

        pile->IncState();
    }

    if ( !m_Instr->Execute(pile) ) return FALSE;
    return pj->Return(pile);                        // opération faite
}


void CBotPreIncExpr::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == NULL ) return;

    if ( pile->GivState() == 0 )
    {
        return;
    }

    m_Instr->RestoreState(pile, bMain);
}


//////////////////////////////////////////////////////////////////////////////////////
// compile une expression unaire
//  +
//  -
//  not
//  !
//  ~

CBotExprUnaire::CBotExprUnaire()
{
    m_Expr = NULL;
    name = "CBotExprUnaire";
}

CBotExprUnaire::~CBotExprUnaire()
{
    delete m_Expr;
}

CBotInstr* CBotExprUnaire::Compile(CBotToken* &p, CBotCStack* pStack)
{
    int op = p->GivType();
    CBotToken*  pp = p;
    if ( !IsOfTypeList( p, ID_ADD, ID_SUB, ID_LOG_NOT, ID_TXT_NOT, ID_NOT, 0 ) ) return NULL;

    CBotCStack* pStk = pStack->TokenStack(pp);

    CBotExprUnaire* inst = new CBotExprUnaire();
    inst->SetToken(pp);

    if ( NULL != (inst->m_Expr = CBotParExpr::Compile( p, pStk )) )
    {
        if ( op == ID_ADD && pStk->GivType() < CBotTypBoolean )     // seulement avec des nombre
            return pStack->Return(inst, pStk);
        if ( op == ID_SUB && pStk->GivType() < CBotTypBoolean )     // seulement avec des nombre
            return pStack->Return(inst, pStk);
        if ( op == ID_NOT && pStk->GivType() < CBotTypFloat )       // seulement avec des entiers
            return pStack->Return(inst, pStk);
        if ( op == ID_LOG_NOT && pStk->GivTypResult().Eq(CBotTypBoolean) )// seulement avec des booléens
            return pStack->Return(inst, pStk);
        if ( op == ID_TXT_NOT && pStk->GivTypResult().Eq(CBotTypBoolean) )// seulement avec des booléens
            return pStack->Return(inst, pStk);

        pStk->SetError(TX_BADTYPE, &inst->m_token);
    }
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute l'expresson unaire

BOOL CBotExprUnaire::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState() == 0 )
    {
        if (!m_Expr->Execute( pile )) return FALSE;                 // interrompu ?
        pile->IncState();
    }

    CBotStack* pile2 = pile->AddStack();
    if ( pile2->IfStep() ) return FALSE;

    CBotVar*    var = pile->GivVar();                               // récupère le résultat sur la pile

    switch (GivTokenType())
    {
    case ID_ADD:
        break;                                                      // ne fait donc rien
    case ID_SUB:
        var->Neg();                                                 // change le signe
        break;
    case ID_NOT:
    case ID_LOG_NOT:
    case ID_TXT_NOT:
        var->Not();
        break;
    }
    return pj->Return(pile);                                        // transmet en dessous
}

void CBotExprUnaire::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == NULL) return;

    if ( pile->GivState() == 0 )
    {
        m_Expr->RestoreState( pile, bMain );                        // interrompu ici !
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// gestion des index pour les tableaux
// array [ expression ]


CBotIndexExpr::CBotIndexExpr()
{
    m_expr  = NULL;
    name    = "CBotIndexExpr";
}

CBotIndexExpr::~CBotIndexExpr()
{
    delete  m_expr;
}

// trouve un champ à partir de l'instance à la compilation

BOOL CBotIndexExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if ( pVar->GivType(1) != CBotTypArrayPointer )
        ASM_TRAP();

    pVar = ((CBotVarArray*)pVar)->GivItem(0, FALSE);            // à la compilation rend l'élément [0]
    if ( pVar == NULL )
    {
        pile->SetError(TX_OUTARRAY, m_token.GivEnd());
        return FALSE;
    }
    if ( m_next3 != NULL ) return m_next3->ExecuteVar(pVar, pile);
    return TRUE;
}

// idem à l'exécution
// attention, modifie le pointeur à la pile volontairement
// place les index calculés sur la pile supplémentaire

BOOL CBotIndexExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep, BOOL bExtend)
{
    CBotStack*  pj = pile;
//  DEBUG( "CBotIndexExpr::ExecuteVar", -1 , pj);

    if ( pVar->GivType(1) != CBotTypArrayPointer )
        ASM_TRAP();

    pile = pile->AddStack();
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState() == 0 )
    {
        if ( !m_expr->Execute(pile) ) return FALSE;
        pile->IncState();
    }
    // traite les tableaux

    CBotVar* p = pile->GivVar();                            // résultat sur la pile

    if ( p == NULL || p->GivType() > CBotTypDouble )
    {
        pile->SetError(TX_BADINDEX, prevToken);
        return pj->Return(pile);
    }

    int n = p->GivValInt();                                 // position dans le tableau
//  DEBUG( "CBotIndexExpr::ExecuteVar", n , pj);

    pVar = ((CBotVarArray*)pVar)->GivItem(n, bExtend);
    if ( pVar == NULL )
    {
        pile->SetError(TX_OUTARRAY, prevToken);
        return pj->Return(pile);
    }

//  DEBUG( "CBotIndexExpr::ExecuteVar", -2 , pj);
    //if ( bUpdate )
        pVar->Maj(pile->GivPUser(), TRUE);

//  DEBUG( "CBotIndexExpr::ExecuteVar", -3 , pj);
    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile, prevToken, bStep, bExtend) ) return FALSE;

//  DEBUG( "CBotIndexExpr::ExecuteVar", -4 , pj);
    return TRUE;                                        // ne libère pas la pile
                                                        // pour éviter de recalculer les index deux fois le cas échéant
}

void CBotIndexExpr::RestoreStateVar(CBotStack* &pile, BOOL bMain)
{
    pile = pile->RestoreStack();
    if ( pile == NULL ) return;

    if ( bMain && pile->GivState() == 0 )
    {
        m_expr->RestoreState(pile, TRUE);
        return;
    }

    if ( m_next3 )
         m_next3->RestoreStateVar(pile, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// gestion des champs dans une instance (opérateur point)
// toto.x


CBotFieldExpr::CBotFieldExpr()
{
    name        = "CBotFieldExpr";
    m_nIdent    = 0;
}

CBotFieldExpr::~CBotFieldExpr()
{
}

void CBotFieldExpr::SetUniqNum(int num)
{
    m_nIdent = num;
}


// trouve un champ à partir de l'instance à la compilation

BOOL CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    if ( pVar->GivType(1) != CBotTypPointer )
        ASM_TRAP();

//  pVar = pVar->GivItem(m_token.GivString());
    pVar = pVar->GivItemRef(m_nIdent);
    if ( pVar == NULL )
    {
        pile->SetError(TX_NOITEM, &m_token);
        return FALSE;
    }

    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile) ) return FALSE;

    return TRUE;
}

// idem à l'exécution

BOOL CBotFieldExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep, BOOL bExtend)
{
    CBotStack*  pj = pile;
    pile = pile->AddStack(this);                // modifie pile en sortie
    if ( pile == EOX ) return TRUE;

//  DEBUG( "CBotFieldExpre::ExecuteVar "+m_token.GivString(), 0, pj );

    if ( pVar->GivType(1) != CBotTypPointer )
        ASM_TRAP();

    CBotVarClass* pItem = pVar->GivPointer();
    if ( pItem == NULL )
    {
        pile->SetError(TX_NULLPT, prevToken);
        return pj->Return( pile );
    }
    if ( pItem->GivUserPtr() == OBJECTDELETED )
    {
        pile->SetError(TX_DELETEDPT, prevToken);
        return pj->Return( pile );
    }

    if ( bStep && pile->IfStep() ) return FALSE;

//  pVar = pVar->GivItem(m_token.GivString());
    pVar = pVar->GivItemRef(m_nIdent);
    if ( pVar == NULL )
    {
        pile->SetError(TX_NOITEM, &m_token);
        return pj->Return( pile );
    }

    if ( pVar->IsStatic() )
    {
//      DEBUG( "IsStatic", 0, pj) ;
        // pour une variable statique, la prend dans la classe elle-même
        CBotClass* pClass = pItem->GivClass();
        pVar = pClass->GivItem(m_token.GivString());
//      DEBUG( "done "+pVar->GivName(), 0, pj) ;
    }

    // demande la mise à jour de l'élément, s'il y a lieu
    pVar->Maj(pile->GivPUser(), TRUE);

    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, bExtend) ) return FALSE;

    return TRUE;                                        // ne libère pas la pile
                                                        // pour conserver l'état SetState() correspondant à l'étape
}

void CBotFieldExpr::RestoreStateVar(CBotStack* &pj, BOOL bMain)
{
    pj = pj->RestoreStack(this);                // modifie pj en sortie
    if ( pj == NULL ) return;

    if ( m_next3 != NULL )
         m_next3->RestoreStateVar(pj, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// compile un opérande gauche pour une assignation

CBotLeftExpr::CBotLeftExpr()
{
    name    = "CBotLeftExpr";
    m_nIdent = 0;
}

CBotLeftExpr::~CBotLeftExpr()
{
}

// compile une expression pour un left-opérande ( à gauche d'une assignation)
// cela peut être
// toto
// toto[ 3 ]
// toto.x
// toto.pos.x
// toto[2].pos.x
// toto[1].pos[2].x
// toto[1][2][3]

CBotLeftExpr* CBotLeftExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GivStart());

    // est-ce un nom de variable ?
    if (p->GivType() == TokenTypVar)
    {
        CBotLeftExpr* inst = new CBotLeftExpr();                    // crée l'objet

        inst->SetToken(p);

        CBotVar*     var;

        if ( NULL != (var = pStk->FindVar(p)) )                     // cherche si variable connue
        {
            inst->m_nIdent = var->GivUniqNum();
            if (inst->m_nIdent > 0 && inst->m_nIdent < 9000)
            {
                if ( var->IsPrivate(PR_READ) &&
                     !pStk->GivBotCall()->m_bCompileClass)
                {
                    pStk->SetError( TX_PRIVATE, p );
                    goto err;
                }
                // il s'agit d'un élement de la classe courante
                // ajoute l'équivalent d'un this. devant
                CBotToken pthis("this");
                inst->SetToken(&pthis);
                inst->m_nIdent = -2;                            // ident pour this

                CBotFieldExpr* i = new CBotFieldExpr();         // nouvel élément
                i->SetToken( p );                               // garde le nom du token
                inst->AddNext3(i);                              // ajoute à la suite

                var = pStk->FindVar(pthis);
                var = var->GivItem(p->GivString());
                i->SetUniqNum(var->GivUniqNum());
            }
            p = p->GivNext();                                       // token suivant

            while (TRUE)
            {
                if ( var->GivType() == CBotTypArrayPointer )        // s'il sagit d'un tableau
                {
                    if ( IsOfType( p, ID_OPBRK ) )                      // regarde s'il y a un index
                    {
                        CBotIndexExpr* i = new CBotIndexExpr();
                        i->m_expr = CBotExpression::Compile(p, pStk);       // compile la formule
                        inst->AddNext3(i);                                  // ajoute à la chaine

                        var = ((CBotVarArray*)var)->GivItem(0,TRUE);        // donne le composant [0]

                        if ( i->m_expr == NULL )
                        {
                            pStk->SetError( TX_BADINDEX, p->GivStart() );
                            goto err;
                        }

                        if ( !pStk->IsOk() || !IsOfType( p, ID_CLBRK ) )
                        {
                            pStk->SetError( TX_CLBRK, p->GivStart() );
                            goto err;
                        }
                        continue;
                    }
                }

                if ( var->GivType(1) == CBotTypPointer )                // pour les classes
                {
                    if ( IsOfType(p, ID_DOT) )
                    {
                        CBotToken* pp = p;

                        CBotFieldExpr* i = new CBotFieldExpr();         // nouvel élément
                        i->SetToken( pp );                              // garde le nom du token
                        inst->AddNext3(i);                              // ajoute à la suite

                        if ( p->GivType() == TokenTypVar )              // doit être un nom
                        {
                            var = var->GivItem(p->GivString());         // récupère l'item correpondant
                            if ( var != NULL )
                            {
                                if ( var->IsPrivate(PR_READ) &&
                                     !pStk->GivBotCall()->m_bCompileClass)
                                {
                                    pStk->SetError( TX_PRIVATE, pp );
                                    goto err;
                                }

                                i->SetUniqNum(var->GivUniqNum());
                                p = p->GivNext();                       // saute le nom
                                continue;
                            }
                            pStk->SetError( TX_NOITEM, p );
                        }
                        pStk->SetError( TX_DOT, p->GivStart() );
                        goto err;
                    }
                }
                break;
            }


            if ( pStk->IsOk() ) return (CBotLeftExpr*) pStack->Return(inst, pStk);
        }
        pStk->SetError(TX_UNDEFVAR, p);
err:
        delete inst;
        return (CBotLeftExpr*) pStack->Return(NULL, pStk);
    }

    return (CBotLeftExpr*) pStack->Return(NULL, pStk);
}

// exécute, trouve une variable et lui assigne le résultat de la pile

BOOL CBotLeftExpr::Execute(CBotStack* &pj, CBotStack* array)
{
    CBotStack*  pile = pj->AddStack();
//  if ( pile == EOX ) return TRUE;

//  if ( pile->IfStep() ) return FALSE;

    CBotVar*     var1 = NULL;
    CBotVar*     var2 = NULL;

//  var1 = pile->FindVar(m_token, FALSE, TRUE);
    if (!ExecuteVar( var1, array, NULL, FALSE )) return FALSE;
                                                            // retrouve la variable (et pas la copie)
    if (pile->IfStep()) return FALSE;

    if ( var1 )
    {
        var2 = pj->GivVar();                                // resultat sur la pile d'entrée
        if ( var2 )
        {
            CBotTypResult t1 = var1->GivTypResult();
            CBotTypResult t2 = var2->GivTypResult();
            if ( t2.Eq(CBotTypPointer) )
            {
                CBotClass*  c1 = t1.GivClass();
                CBotClass*  c2 = t2.GivClass();
                if ( !c2->IsChildOf(c1))
                {
                    CBotToken* pt = &m_token;
                    pile->SetError(TX_BADTYPE, pt);
                    return pj->Return(pile);                                // opération faite
                }
            }
            var1->SetVal(var2);                     // fait l'assignation
        }
        pile->SetCopyVar( var1 );                           // remplace sur la pile par une copie de la variable elle-même
                                                            // (pour avoir le nom)
    }

    return pj->Return(pile);                                // opération faite
}

// retrouve une variable pendant la compilation

BOOL CBotLeftExpr::ExecuteVar(CBotVar* &pVar, CBotCStack* &pile)
{
    pVar = pile->FindVar(m_token);
    if ( pVar == NULL ) return FALSE;

    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile) ) return FALSE;

    return TRUE;
}

// retrouve une variable à l'exécution

BOOL CBotLeftExpr::ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep)
{
    pile = pile->AddStack( this );          // déplace la pile

    pVar = pile->FindVar(m_nIdent);
    if ( pVar == NULL )
    {
#ifdef  _DEBUG
        ASM_TRAP();
#endif
        pile->SetError(2, &m_token);
        return FALSE;
    }

    if ( bStep && m_next3 == NULL && pile->IfStep() ) return FALSE;

    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pile, &m_token, bStep, TRUE) ) return FALSE;

    return TRUE;
}

void CBotLeftExpr::RestoreStateVar(CBotStack* &pile, BOOL bMain)
{
    pile = pile->RestoreStack( this );          // déplace la pile
    if ( pile == NULL ) return;

    if ( m_next3 != NULL )
         m_next3->RestoreStateVar(pile, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

// transforme une chaîne en nombre entier
// peut être de la forme 0xabc123

long GivNumInt( const char* p )
{
    long    num = 0;
    while (*p >= '0' && *p <= '9')
    {
        num = num * 10 + *p - '0';
        p++;
    }
    if ( *p == 'x' || *p == 'X' )
    {
        while (*++p != 0)
        {
            if ( *p >= '0' && *p <= '9' )
            {
                num = num * 16 + *p - '0';
                continue;
            }
            if ( *p >= 'A' && *p <= 'F' )
            {
                num = num * 16 + *p - 'A' + 10;
                continue;
            }
            if ( *p >= 'a' && *p <= 'f' )
            {
                num = num * 16 + *p - 'a' + 10;
                continue;
            }
            break;
        }
    }
    return num;
}

// transforme une chaîne en un nombre réel

extern float GivNumFloat( const char* p )
{
    double  num = 0;
    double  div = 10;
    BOOL    bNeg = FALSE;

    if (*p == '-')
    {
        bNeg = TRUE;
        p++;
    }
    while (*p >= '0' && *p <= '9')
    {
        num = num * 10. + (*p - '0');
        p++;
    }

    if ( *p == '.' )
    {
        p++;
        while (*p >= '0' && *p <= '9')
        {
            num = num + (*p - '0') / div;
            div = div * 10;
            p++;
        }
    }

    int exp = 0;
    if ( *p == 'e' || *p == 'E' )
    {
        char neg = 0;
        p++;
        if ( *p == '-' || *p == '+' ) neg = *p++;

        while (*p >= '0' && *p <= '9')
        {
            exp = exp * 10 + (*p - '0');
            p++;
        }
        if ( neg == '-' ) exp = -exp;
    }

    while ( exp > 0 )
    {
        num *= 10.0;
        exp--;
    }

    while ( exp < 0 )
    {
        num /= 10.0;
        exp++;
    }

    if ( bNeg ) num = -num;
    return (float)num;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile un token représentant un nombre

CBotExprNum::CBotExprNum()
{
    name    = "CBotExprNum";
}

CBotExprNum::~CBotExprNum()
{
}

CBotInstr* CBotExprNum::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprNum* inst = new CBotExprNum();

    inst->SetToken(p);
    CBotString  s = p->GivString();

    inst->m_numtype = CBotTypInt;
    if ( p->GivType() == TokenTypDef )
    {
        inst->m_valint = p->GivIdKey();
    }
    else
    {
        if ( s.Find('.') >= 0 || ( s.Find('x') < 0 && ( s.Find('e') >= 0 || s.Find('E') >= 0 ) ) )
        {
            inst->m_numtype = CBotTypFloat;
            inst->m_valfloat = GivNumFloat(s);
        }
        else
        {
            inst->m_valint = GivNumInt(s);
        }
    }

    if (pStk->NextToken(p))
    {
        CBotVar*    var = CBotVar::Create((CBotToken*)NULL, inst->m_numtype);
        pStk->SetVar(var);

        return pStack->Return(inst, pStk);
    }
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute, retourne le nombre correspondant

BOOL CBotExprNum::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    CBotVar*    var = CBotVar::Create((CBotToken*)NULL, m_numtype);

    CBotString  nombre ;
    if ( m_token.GivType() == TokenTypDef )
    {
        nombre = m_token.GivString();
    }

    switch (m_numtype)
    {
    case CBotTypShort:
    case CBotTypInt:
        var->SetValInt( m_valint, nombre );         // valeur du nombre
        break;
    case CBotTypFloat:
        var->SetValFloat( m_valfloat );             // valeur du nombre
        break;
    }
    pile->SetVar( var );                            // mis sur la pile

    return pj->Return(pile);                        // c'est ok
}

void CBotExprNum::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile un token représentant une chaine de caractères

CBotExprAlpha::CBotExprAlpha()
{
    name    = "CBotExprAlpha";
}

CBotExprAlpha::~CBotExprAlpha()
{
}

CBotInstr* CBotExprAlpha::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    CBotExprAlpha* inst = new CBotExprAlpha();

    inst->SetToken(p);
    p = p->GivNext();

    CBotVar*    var = CBotVar::Create((CBotToken*)NULL, CBotTypString);
    pStk->SetVar(var);

    return pStack->Return(inst, pStk);
}

// exécute, retourne la chaîne correspondante

BOOL CBotExprAlpha::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    CBotVar*    var = CBotVar::Create((CBotToken*)NULL, CBotTypString);

    CBotString  chaine = m_token.GivString();
    chaine = chaine.Mid(1, chaine.GivLength()-2);   // enlève les guillemets

    var->SetValString( chaine );                    // valeur du nombre

    pile->SetVar( var );                            // mis sur la pile

    return pj->Return(pile);
}

void CBotExprAlpha::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile un token représentant true ou false

CBotExprBool::CBotExprBool()
{
    name = "CBotExprBool";
}

CBotExprBool::~CBotExprBool()
{
}

CBotInstr* CBotExprBool::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();
    CBotExprBool* inst = NULL;

    if ( p->GivType() == ID_TRUE ||
         p->GivType() == ID_FALSE )
    {
        inst = new CBotExprBool();
        inst->SetToken(p);                          // mémorise l'opération false ou true
        p = p->GivNext();

        CBotVar*    var = CBotVar::Create((CBotToken*)NULL, CBotTypBoolean);
        pStk->SetVar(var);
    }

    return pStack->Return(inst, pStk);
}

// exécute, retourne true ou false

BOOL CBotExprBool::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    CBotVar*    var = CBotVar::Create((CBotToken*)NULL, CBotTypBoolean);

    if (GivTokenType() == ID_TRUE)    var->SetValInt(1);
    else                              var->SetValInt(0);

    pile->SetVar( var );                                        // mis sur la pile
    return pj->Return(pile);                                    // transmet en dessous
}

void CBotExprBool::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////

// gestion de l'opérande "null"

CBotExprNull::CBotExprNull()
{
    name = "CBotExprNull";
}

CBotExprNull::~CBotExprNull()
{
}

// exécute, retourne un pointeur vide

BOOL CBotExprNull::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;
    CBotVar*    var = CBotVar::Create((CBotToken*)NULL, CBotTypNullPointer);

    var->SetInit(TRUE);                             // pointeur null valide
    pile->SetVar( var );                            // mis sur la pile
    return pj->Return(pile);                        // transmet en dessous
}

void CBotExprNull::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////////

// gestion de l'opérande "nan"

CBotExprNan::CBotExprNan()
{
    name = "CBotExprNan";
}

CBotExprNan::~CBotExprNan()
{
}

// exécute, retourne un pointeur vide

BOOL CBotExprNan::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;
    CBotVar*    var = CBotVar::Create((CBotToken*)NULL, CBotTypInt);

    var->SetInit(IS_NAN);                           // nombre nan
    pile->SetVar( var );                            // mis sur la pile
    return pj->Return(pile);                        // transmet en dessous
}

void CBotExprNan::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}

//////////////////////////////////////////////////////////////////////////////////////
// compile un nom de variable
// vérifie qu'elle est connue sur la pile
// et qu'elle a été initialisée

CBotExprVar::CBotExprVar()
{
    name    = "CBotExprVar";
    m_nIdent = 0;
}

CBotExprVar::~CBotExprVar()
{
}

CBotInstr* CBotExprVar::Compile(CBotToken* &p, CBotCStack* pStack, int privat)
{
    CBotToken*  pDebut = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(p->GivStart());

    // est-ce un nom de variable ?
    if (p->GivType() == TokenTypVar)
    {
        CBotInstr* inst = new CBotExprVar();                    // crée l'objet

        inst->SetToken(p);

        CBotVar*     var;

        if ( NULL != (var = pStk->FindVar(p)) )                 // cherche si variable connue
        {
            int     ident = var->GivUniqNum();
            ((CBotExprVar*)inst)->m_nIdent = ident;             // l'identifie par son numéro

            if (ident > 0 && ident < 9000)
            {
                if ( var->IsPrivate(privat) &&
                     !pStk->GivBotCall()->m_bCompileClass)
                {
                    pStk->SetError( TX_PRIVATE, p );
                    goto err;
                }

                // il s'agit d'un élement de la classe courante
                // ajoute l'équivalent d'un this. devant
                inst->SetToken(&CBotToken("this"));
                ((CBotExprVar*)inst)->m_nIdent = -2;            // ident pour this

                CBotFieldExpr* i = new CBotFieldExpr();         // nouvel élément
                i->SetToken( p );                               // garde le nom du token
                i->SetUniqNum(ident);
                inst->AddNext3(i);                              // ajoute à la suite
            }

            p = p->GivNext();                                       // token suivant

            while (TRUE)
            {
                if ( var->GivType() == CBotTypArrayPointer )            // s'il sagit d'un tableau
                {
                    if ( IsOfType( p, ID_OPBRK ) )                      // regarde s'il y a un index
                    {
                        CBotIndexExpr* i = new CBotIndexExpr();
                        i->m_expr = CBotExpression::Compile(p, pStk);       // compile la formule
                        inst->AddNext3(i);                                  // ajoute à la chaine

                        var = ((CBotVarArray*)var)->GivItem(0,TRUE);        // donne le composant [0]

                        if ( i->m_expr == NULL )
                        {
                            pStk->SetError( TX_BADINDEX, p->GivStart() );
                            goto err;
                        }
                        if ( !pStk->IsOk() || !IsOfType( p, ID_CLBRK ) )
                        {
                            pStk->SetError( TX_CLBRK, p->GivStart() );
                            goto err;
                        }
                        continue;
                    }
    ////            pStk->SetError( TX_OPBRK, p->GivStart() );
                }
                if ( var->GivType(1) == CBotTypPointer )                // pour les classes
                {
                    if ( IsOfType(p, ID_DOT) )
                    {
                        CBotToken* pp = p;

                        if ( p->GivType() == TokenTypVar )              // doit être un nom
                        {
                            if ( p->GivNext()->GivType() == ID_OPENPAR )// un appel de méthode ?
                            {
                                CBotInstr* i = CBotInstrMethode::Compile(p, pStk, var);
                                if ( !pStk->IsOk() ) goto err;
                                inst->AddNext3(i);                              // ajoute à la suite
                                return pStack->Return(inst, pStk);
                            }
                            else
                            {
                                CBotFieldExpr* i = new CBotFieldExpr();         // nouvel élément
                                i->SetToken( pp );                              // garde le nom du token
                                inst->AddNext3(i);                              // ajoute à la suite
                                var = var->GivItem(p->GivString());         // récupère l'item correpondant
                                if ( var != NULL )
                                {
                                    i->SetUniqNum(var->GivUniqNum());
                                    if ( var->IsPrivate() &&
                                     !pStk->GivBotCall()->m_bCompileClass)
                                    {
                                        pStk->SetError( TX_PRIVATE, pp );
                                        goto err;
                                    }
                                }
                            }


                            if ( var != NULL )
                            {
                                p = p->GivNext();                       // saute le nom
                                continue;
                            }
                            pStk->SetError( TX_NOITEM, p );
                            goto err;
                        }
                        pStk->SetError( TX_DOT, p->GivStart() );
                        goto err;
                    }
                }

                break;
            }

            pStk->SetCopyVar(var);                                  // place une copie de la variable sur la pile (pour le type)
            if ( pStk->IsOk() ) return pStack->Return(inst, pStk);
        }
        pStk->SetError(TX_UNDEFVAR, p);
err:
        delete inst;
        return pStack->Return(NULL, pStk);
    }

    return pStack->Return(NULL, pStk);
}

CBotInstr* CBotExprVar::CompileMethode(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;
    CBotCStack* pStk = pStack->TokenStack();

    pStk->SetStartError(pp->GivStart());

    // est-ce un nom de variable ?
    if (pp->GivType() == TokenTypVar)
    {
        CBotToken pthis("this");
        CBotVar*     var = pStk->FindVar(pthis);
        if ( var == 0 ) return pStack->Return(NULL, pStk);

        CBotInstr* inst = new CBotExprVar();                    // crée l'objet

        // il s'agit d'un élement de la classe courante
        // ajoute l'équivalent d'un this. devant
        inst->SetToken(&pthis);
        ((CBotExprVar*)inst)->m_nIdent = -2;                    // ident pour this

        CBotToken* pp = p;

        if ( pp->GivType() == TokenTypVar )                     // doit être un nom
        {
            if ( pp->GivNext()->GivType() == ID_OPENPAR )       // un appel de méthode ?
            {
                CBotInstr* i = CBotInstrMethode::Compile(pp, pStk, var);
                if ( pStk->IsOk() )
                {
                    inst->AddNext3(i);                          // ajoute à la suite
                    p = pp;                                     // instructions passées
                    return pStack->Return(inst, pStk);
                }
                pStk->SetError(0,0);                            // l'erreur n'est pas traitée ici
            }
        }
        delete inst;
    }
    return pStack->Return(NULL, pStk);
}


// exécute, rend la valeur d'une variable

BOOL CBotExprVar::Execute(CBotStack* &pj)
{
    CBotVar*     pVar = NULL;
    CBotStack*   pile  = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

//  if ( pile->IfStep() ) return FALSE;

    CBotStack*   pile1 = pile;

    if ( pile1->GivState() == 0 )
    {
        if ( !ExecuteVar(pVar, pile, NULL, TRUE) ) return FALSE;        // récupère la variable selon champs et index
//      DEBUG("CBotExprVar::Execute", 1 , pj);

        if ( pVar ) pile1->SetCopyVar(pVar);                            // la place une copie sur la pile
        else
        {
//--        pile1->SetVar(NULL);                                // la pile contient déjà le resultat (méthode)
            return pj->Return(pile1);
        }
        pile1->IncState();
    }

    pVar = pile1->GivVar();                                     // récupère si interrompu

    if ( pVar == NULL )
    {
//      pile1->SetError(TX_NULLPT, &m_token);
        return pj->Return(pile1);
    }

    if ( pVar->GivInit() == IS_UNDEF )
    {
        CBotToken* pt = &m_token;
        while ( pt->GivNext() != NULL ) pt = pt->GivNext();
        pile1->SetError(TX_NOTINIT, pt);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);                                   // opération faite
}

void CBotExprVar::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*   pile  = pj->RestoreStack(this);
    if ( pile == NULL ) return;

    CBotStack*   pile1 = pile;

    if ( pile1->GivState() == 0 )
    {
        RestoreStateVar(pile, bMain);   // récupère la variable selon champs et index
        return;
    }
}

// retrouve une variable à l'exécution

BOOL CBotExprVar::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, BOOL bStep)
{
    CBotStack*  pile = pj;
    pj = pj->AddStack( this );

    if ( bStep && m_nIdent>0 && pj->IfStep() ) return FALSE;

    pVar = pj->FindVar(m_nIdent, TRUE);                     // cherche la variable avec mise à jour si nécessaire
    if ( pVar == NULL )
    {
#ifdef  _DEBUG
        ASM_TRAP();
#endif
        pj->SetError(1, &m_token);
        return FALSE;
    }
    if ( m_next3 != NULL &&
         !m_next3->ExecuteVar(pVar, pj, &m_token, bStep, FALSE) )
            return FALSE;                                       // Champs d'une instance, tableau, méthode ?

    return pile->ReturnKeep( pj );                              // ne rend pas la pile mais récupère le résultat si une méthode a été appelée
}


// retrouve une variable à l'exécution

void CBotExprVar::RestoreStateVar(CBotStack* &pj, BOOL bMain)
{
    pj = pj->RestoreStack( this );
    if ( pj == NULL ) return;

    if ( m_next3 != NULL )
         m_next3->RestoreStateVar(pj, bMain);
}

//////////////////////////////////////////////////////////////////////////////////////////

// compile une liste de paramètres

CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars)
{
    BOOL        first = TRUE;
    CBotInstr*  ret = NULL;                         // pour la liste à retourner

//  pStack->SetStartError(p->GivStart());
    CBotCStack* pile = pStack;
    int         i = 0;

    if ( IsOfType(p, ID_OPENPAR) )
    {
        int start, end;
        if (!IsOfType(p, ID_CLOSEPAR)) while (TRUE)
        {
            start = p->GivStart();
            pile = pile->TokenStack();                      // garde les résultats sur la pile

            if ( first ) pStack->SetStartError(start);
            first = FALSE;

            CBotInstr*  param = CBotExpression::Compile(p, pile);
            end   = p->GivStart();

            if ( !pile->IsOk() )
            {
                return pStack->Return(NULL, pile);
            }

            if ( ret == NULL ) ret = param;
            else ret->AddNext(param);                       // construit la liste

            if ( param != NULL )
            {
                if ( pile->GivTypResult().Eq(99) )
                {
                    delete pStack->TokenStack();
                    pStack->SetError(TX_VOID, p->GivStart());
                    return NULL;
                }
                ppVars[i] = pile->GivVar();
                ppVars[i]->GivToken()->SetPos(start, end);
                i++;

                if (IsOfType(p, ID_COMMA)) continue;        // saute la virgule
                if (IsOfType(p, ID_CLOSEPAR)) break;
            }

            pStack->SetError(TX_CLOSEPAR, p->GivStart());
            delete pStack->TokenStack();
            return NULL;
        }
    }
    ppVars[i] = NULL;
    return  ret;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// compile un appel d'une méthode

CBotInstrMethode::CBotInstrMethode()
{
    m_Parameters    = NULL;
    m_MethodeIdent  = 0;
//  m_nThisIdent    = 0;
    name = "CBotInstrMethode";
}

CBotInstrMethode::~CBotInstrMethode()
{
    delete  m_Parameters;
}

CBotInstr* CBotInstrMethode::Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* var)
{
    CBotInstrMethode* inst = new CBotInstrMethode();
    inst->SetToken(p);                                  // token correspondant

//  inst->m_nThisIdent = CBotVar::NextUniqNum();

    if ( NULL != var )
    {
        CBotToken*  pp = p;
        p = p->GivNext();

        if ( p->GivType() == ID_OPENPAR )
        {
            inst->m_NomMethod = pp->GivString();

            // compile la liste des paramètres
            CBotVar*    ppVars[1000];
            inst->m_Parameters = CompileParams(p, pStack, ppVars);

            if ( pStack->IsOk() )
            {
                CBotClass* pClass = var->GivClass();    // pointeur à la classe
                inst->m_ClassName = pClass->GivName();  // le nom de la classe
                CBotTypResult r = pClass->CompileMethode(inst->m_NomMethod, var, ppVars,
                                                         pStack, inst->m_MethodeIdent);
                delete pStack->TokenStack();            // libères les paramètres encore sur la pile
                inst->m_typRes = r;

                if (inst->m_typRes.GivType() > 20)
                {
                    pStack->SetError(inst->m_typRes.GivType(), pp);
                    delete  inst;
                    return  NULL;
                }
                // met un résultat sur la pile pour avoir quelque chose
                if (inst->m_typRes.GivType() > 0)
                {
                    CBotVar*    pResult = CBotVar::Create("", inst->m_typRes);
                    if (inst->m_typRes.Eq(CBotTypClass))
                    {
//                      CBotClass*  pClass = CBotClass::Find(inst->m_RetClassName);
                        pResult->SetClass(inst->m_typRes.GivClass());
                    }
                    pStack->SetVar(pResult);
                }
                return inst;
            }
            delete inst;
            return NULL;
        }
    }
    pStack->SetError( 1234, p );
    delete inst;
    return NULL;
}

// exécute l'appel de méthode

BOOL CBotInstrMethode::ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, BOOL bStep, BOOL bExtend)
{
    CBotVar*    ppVars[1000];
    CBotStack*  pile1 = pj->AddStack(this, TRUE);       // une place pour la copie de This
//  if ( pile1 == EOX ) return TRUE;

//  DEBUG( "CBotInstrMethode::ExecuteVar", 0, pj );

    if ( pVar->GivPointer() == NULL )
    {
        pj->SetError( TX_NULLPT, prevToken );
    }

    if ( pile1->IfStep() ) return FALSE;

    CBotStack*  pile2 = pile1->AddStack();              // et pour les paramètres à venir

    if ( pile1->GivState() == 0)
    {
        CBotVar*    pThis = CBotVar::Create(pVar);
        pThis->Copy(pVar);
        // la valeur de This doit être prise avant l'évaluation des paramètres
        // Test.Action( Test = Autre );
        // Action doit agir sur la valeur avant Test = Autre !!
        pThis->SetName("this");
//      pThis->SetUniqNum(m_nThisIdent);
        pThis->SetUniqNum(-2);
        pile1->AddVar(pThis);
        pile1->IncState();
    }
    int     i = 0;

    CBotInstr*  p = m_Parameters;
    // évalue les paramètres
    // et place les valeurs sur la pile
    // pour pouvoir être interrompu n'importe quand
    if ( p != NULL) while ( TRUE )
    {
        if ( pile2->GivState() == 0 )
        {
            if (!p->Execute(pile2)) return FALSE;       // interrompu ici ?
            if (!pile2->SetState(1)) return FALSE;      // marque spéciale pour reconnaîre les paramètres
        }
        ppVars[i++] = pile2->GivVar();                  // construit la liste des pointeurs
        pile2 = pile2->AddStack();                      // de la place sur la pile pour les résultats
        p = p->GivNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotClass*  pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pThis  = pile1->FindVar(-2);
    CBotVar*    pResult = NULL;
    if (m_typRes.GivType() > 0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
//      CBotClass*  pClass = CBotClass::Find(m_RetClassName);
        pResult->SetClass(m_typRes.GivClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_NomMethod,
                                 pThis, ppVars,
                                 pResult, pile2, GivToken())) return FALSE; // interrompu
    if (pRes != pResult) delete pRes;

    pVar = NULL;                // ne retourne pas une valeur par cela
    return pj->Return(pile2);   // libère toute la pile
}

void CBotInstrMethode::RestoreStateVar(CBotStack* &pile, BOOL bMain)
{
    if ( !bMain ) return;

    CBotVar*    ppVars[1000];
    CBotStack*  pile1 = pile->RestoreStack(this);       // une place pour la copie de This
    if ( pile1 == NULL ) return;

    CBotStack*  pile2 = pile1->RestoreStack();          // et pour les paramètres à venir
    if ( pile2 == NULL ) return;

    CBotVar*    pThis  = pile1->FindVar("this");
//  pThis->SetUniqNum(m_nThisIdent);
    pThis->SetUniqNum(-2);

    int     i = 0;

    CBotInstr*  p = m_Parameters;
    // évalue les paramètres
    // et place les valeurs sur la pile
    // pour pouvoir être interrompu n'importe quand
    if ( p != NULL) while ( TRUE )
    {
        if ( pile2->GivState() == 0 )
        {
            p->RestoreState(pile2, TRUE);               // interrompu ici !
            return;
        }
        ppVars[i++] = pile2->GivVar();                  // construit la liste des pointeurs
        pile2 = pile2->RestoreStack();
        if ( pile2 == NULL ) return;

        p = p->GivNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotClass*  pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pResult = NULL;

    CBotVar*    pRes = pResult;

    pClass->RestoreMethode(m_MethodeIdent, m_NomMethod,
                                 pThis, ppVars, pile2);
}


BOOL CBotInstrMethode::Execute(CBotStack* &pj)
{
    CBotVar*    ppVars[1000];
    CBotStack*  pile1 = pj->AddStack(this, TRUE);       // une place pour la copie de This
//  if ( pile1 == EOX ) return TRUE;

    if ( pile1->IfStep() ) return FALSE;

    CBotStack*  pile2 = pile1->AddStack();              // et pour les paramètres à venir

    if ( pile1->GivState() == 0)
    {
        CBotVar*    pThis = pile1->CopyVar(m_token);
        // la valeur de This doit être prise avant l'évaluation des paramètres
        // Test.Action( Test = Autre );
        // Action doit agir sur la valeur avant Test = Autre !!
        pThis->SetName("this");
        pile1->AddVar(pThis);
        pile1->IncState();
    }
    int     i = 0;

    CBotInstr*  p = m_Parameters;
    // évalue les paramètres
    // et place les valeurs sur la pile
    // pour pouvoir être interrompu n'importe quand
    if ( p != NULL) while ( TRUE )
    {
        if ( pile2->GivState() == 0 )
        {
            if (!p->Execute(pile2)) return FALSE;       // interrompu ici ?
            if (!pile2->SetState(1)) return FALSE;      // marque spéciale pour reconnaîre les paramètres
        }
        ppVars[i++] = pile2->GivVar();                  // construit la liste des pointeurs
        pile2 = pile2->AddStack();                      // de la place sur la pile pour les résultats
        p = p->GivNext();
        if ( p == NULL) break;
    }
    ppVars[i] = NULL;

    CBotClass*  pClass = CBotClass::Find(m_ClassName);
    CBotVar*    pThis  = pile1->FindVar("this");
    CBotVar*    pResult = NULL;
    if (m_typRes.GivType()>0) pResult = CBotVar::Create("", m_typRes);
    if (m_typRes.Eq(CBotTypClass))
    {
//      CBotClass*  pClass = CBotClass::Find(m_RetClassName);
        pResult->SetClass(m_typRes.GivClass());
    }
    CBotVar*    pRes = pResult;

    if ( !pClass->ExecuteMethode(m_MethodeIdent, m_NomMethod,
                                 pThis, ppVars,
                                 pResult, pile2, GivToken())) return FALSE; // interrompu

    // met la nouvelle valeur de this à la place de l'ancienne variable
    CBotVar*    old = pile1->FindVar(m_token);
    old->Copy(pThis, FALSE);

    if (pRes != pResult) delete pRes;

    return pj->Return(pile2);   // libère toute la pile
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "new"

CBotNew::CBotNew()
{
    name            = "CBotNew";
    m_Parameters    = NULL;
    m_nMethodeIdent = 0;
//  m_nThisIdent    = 0;
}

CBotNew::~CBotNew()
{
}

CBotInstr* CBotNew::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken* pp = p;
    if ( !IsOfType(p, ID_NEW) ) return NULL;

    // vérifie que le token est un nom de classe
    if (p->GivType() != TokenTypVar) return NULL;

    CBotClass* pClass = CBotClass::Find(p);
    if (pClass == NULL)
    {
        pStack->SetError(TX_BADNEW, p);
        return NULL;
    }
/*  if ( !pClass->m_IsDef )
    {
        pStack->SetError(TX_BADNEW, p);
        return NULL;
    }*/

    CBotNew* inst = new CBotNew();
    inst->SetToken(pp);

    inst->m_vartoken = p;
    p = p->GivNext();

    // crée l'objet sur le "tas"
    // avec un pointeur sur cet objet
    CBotVar*    pVar = CBotVar::Create("", pClass);
//  inst->m_nThisIdent = CBotVar::NextUniqNum();

    // fait l'appel du créateur
    CBotCStack* pStk = pStack->TokenStack();
    {
        // regarde s'il y a des paramètres
        CBotVar*    ppVars[1000];
        inst->m_Parameters = CompileParams(p, pStk, ppVars);
        if ( !pStk->IsOk() ) goto error;

        // le constructeur existe-il ?
//      CBotString  noname;
        CBotTypResult r = pClass->CompileMethode(pClass->GivName(), pVar, ppVars, pStk, inst->m_nMethodeIdent);
        delete pStk->TokenStack();                          // libère le supplément de pile
        int typ = r.GivType();

        // s'il n'y a pas de constructeur, et pas de paramètres non plus, c'est ok
        if ( typ == TX_UNDEFCALL && inst->m_Parameters == NULL ) typ = 0;
        pVar->SetInit(TRUE);                                // marque l'instance comme init

        if (typ>20)
        {
            pStk->SetError(typ, inst->m_vartoken.GivEnd());
            goto error;
        }

        // si le constructeur n'existe pas, mais qu'il y a des paramètres
        if (typ<0 && inst->m_Parameters != NULL)
        {
            pStk->SetError(TX_NOCONST, &inst->m_vartoken);
            goto error;
        }

        // rend le pointeur à l'objet sur la pile
        pStk->SetVar(pVar);
        return pStack->Return(inst, pStk);
    }
error:
    delete inst;
    return pStack->Return(NULL, pStk);
}

// exécute une instruction "new"

BOOL CBotNew::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);              //pile principale
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    CBotStack*  pile1 = pj->AddStack2();                //pile secondaire

    CBotVar*    pThis = NULL;

    CBotToken*  pt = &m_vartoken;
    CBotClass*  pClass = CBotClass::Find(pt);

    // crée la variable "this" de type pointeur à l'objet

    if ( pile->GivState()==0)
    {
        // crée une instance de la classe demandée
        // et initialise le pointeur à cet objet

        pThis = CBotVar::Create("this", pClass);
//      pThis->SetUniqNum( m_nThisIdent ) ;
        pThis->SetUniqNum( -2 ) ;

        pile1->SetVar(pThis);                               // la place sur la pile1
        pile->IncState();
    }

    // retrouve le pointeur this si on a été interrompu
    if ( pThis == NULL)
    {
        pThis = pile1->GivVar();                                // retrouve le pointeur
    }

    // y a-t-il une assignation ou des paramètres (constructeur)
    if ( pile->GivState()==1)
    {
        // évalue le constructeur de l'instance

        CBotVar*    ppVars[1000];
        CBotStack*  pile2 = pile;

        int     i = 0;

        CBotInstr*  p = m_Parameters;
        // évalue les paramètres
        // et place les valeurs sur la pile
        // pour pouvoir être interrompu n'importe quand

        if ( p != NULL) while ( TRUE )
        {
            pile2 = pile2->AddStack();                      // de la place sur la pile pour les résultats
            if ( pile2->GivState() == 0 )
            {
                if (!p->Execute(pile2)) return FALSE;       // interrompu ici ?
                pile2->SetState(1);
            }
            ppVars[i++] = pile2->GivVar();
            p = p->GivNext();
            if ( p == NULL) break;
        }
        ppVars[i] = NULL;

        // crée une variable pour le résultat
        CBotVar*    pResult = NULL;                     // constructeurs toujours void

        if ( !pClass->ExecuteMethode(m_nMethodeIdent, pClass->GivName(),
                                     pThis, ppVars,
                                     pResult, pile2, GivToken())) return FALSE; // interrompu

        pThis->ConstructorSet();        // signale que le constructeur a été appelé
//      pile->Return(pile2);                            // libère un bout de pile

//      pile->IncState();
    }

    return pj->Return( pile1 );                         // transmet en dessous
}

void CBotNew::RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);          //pile principale
    if ( pile == NULL ) return;

    CBotStack*  pile1 = pj->AddStack2();                //pile secondaire

    CBotToken*  pt = &m_vartoken;
    CBotClass*  pClass = CBotClass::Find(pt);

    // crée la variable "this" de type pointeur à l'objet

    if ( pile->GivState()==0)
    {
        return;
    }

    CBotVar* pThis = pile1->GivVar();                   // retrouve le pointeur
//  pThis->SetUniqNum( m_nThisIdent );
    pThis->SetUniqNum( -2 );

    // y a-t-il une assignation ou des paramètres (constructeur)
    if ( pile->GivState()==1)
    {
        // évalue le constructeur de l'instance

        CBotVar*    ppVars[1000];
        CBotStack*  pile2 = pile;

        int     i = 0;

        CBotInstr*  p = m_Parameters;
        // évalue les paramètres
        // et place les valeurs sur la pile
        // pour pouvoir être interrompu n'importe quand

        if ( p != NULL) while ( TRUE )
        {
            pile2 = pile2->RestoreStack();              // de la place sur la pile pour les résultats
            if ( pile2 == NULL ) return;

            if ( pile2->GivState() == 0 )
            {
                p->RestoreState(pile2, bMain);          // interrompu ici !
                return;
            }
            ppVars[i++] = pile2->GivVar();
            p = p->GivNext();
            if ( p == NULL) break;
        }
        ppVars[i] = NULL;

        pClass->RestoreMethode(m_nMethodeIdent, m_vartoken.GivString(), pThis,
                               ppVars, pile2)   ;       // interrompu ici !
    }
}

/////////////////////////////////////////////////////////////
// regarde si deux résultats sont compatibles pour faire une opération

BOOL TypeCompatible( CBotTypResult& type1, CBotTypResult& type2, int op )
{
    int t1 = type1.GivType();
    int t2 = type2.GivType();

    int max = (t1 > t2) ? t1 : t2;

    if ( max == 99 ) return FALSE;                  // un résultat est void ?

    // cas particulier pour les concaténation de chaînes
    if (op == ID_ADD && max >= CBotTypString) return TRUE;
    if (op == ID_ASSADD && max >= CBotTypString) return TRUE;
    if (op == ID_ASS && t1 == CBotTypString) return TRUE;

    if ( max >= CBotTypBoolean )
    {
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypPointer && t2 == CBotTypNullPointer)) return TRUE;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypPointer && t1 == CBotTypNullPointer)) return TRUE;
        if ( (op == ID_EQ || op == ID_NE) &&
             (t1 == CBotTypArrayPointer && t2 == CBotTypNullPointer)) return TRUE;
        if ( (op == ID_EQ || op == ID_NE || op == ID_ASS) &&
             (t2 == CBotTypArrayPointer && t1 == CBotTypNullPointer)) return TRUE;
        if (t2 != t1) return FALSE;
        if (t1 == CBotTypArrayPointer) return type1.Compare(type2);
        if (t1 == CBotTypPointer ||
            t1 == CBotTypClass   ||
            t1 == CBotTypIntrinsic )
        {
            CBotClass*  c1 = type1.GivClass();
            CBotClass*  c2 = type2.GivClass();

            return c1->IsChildOf(c2) || c2->IsChildOf(c1);
            // accepte le caste à l'envers,
            // l'opération sera refusée à l'exécution si le pointeur n'est pas compatible
        }

        return TRUE;
    }

    type1.SetType(max);
    type2.SetType(max);
    return TRUE;
}

// regarde si deux variables sont compatible pour un passage de paramètre

BOOL TypesCompatibles( const CBotTypResult& type1, const CBotTypResult& type2 )
{
    int t1 = type1.GivType();
    int t2 = type2.GivType();

    if ( t1 == CBotTypIntrinsic ) t1 = CBotTypClass;
    if ( t2 == CBotTypIntrinsic ) t2 = CBotTypClass;

    int max = (t1 > t2) ? t1 : t2;

    if ( max == 99 ) return FALSE;                  // un résultat est void ?

    if ( max >= CBotTypBoolean )
    {
        if ( t2 != t1 ) return FALSE;

        if ( max == CBotTypArrayPointer )
            return TypesCompatibles(type1.GivTypElem(), type2.GivTypElem());

        if ( max == CBotTypClass || max == CBotTypPointer )
            return type1.GivClass() == type2.GivClass() ;

        return TRUE ;
    }
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////
// Gestion des fichiers

// nécessaire car il n'est pas possible de faire le fopen dans le programme principal
// et les fwrite ou fread dans une dll en utilisant le FILE* rendu.

FILE* fOpen(const char* name, const char* mode)
{
    return fopen(name, mode);
}

int fClose(FILE* filehandle)
{
    return fclose(filehandle);
}

size_t fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle)
{
    return fwrite(buffer, elemsize, length, filehandle);
}

size_t fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle)
{
    return fread(buffer, elemsize, length, filehandle);
}

size_t fWrite(const void *buffer, size_t length, FILE* filehandle)
{
    return fwrite(buffer, 1, length, filehandle);
}

size_t fRead(void *buffer, size_t length, FILE* filehandle)
{
    return fread(buffer, 1, length, filehandle);
}


////////////////////////////////////////


#if FALSE

CBotString num(int n)
{
    CBotString  s;
    if ( n<0 ) {n = -n; s += "-";}
    if ( n > 9 )
    {
        s += num(n/10);
    }
    s += '0' + n%10;
    return s;
}

extern void DEBUG( const char* text, int val, CBotStack* pile )
{
    CBotProgram* p = pile->GivBotCall(TRUE);
    if ( !p->m_bDebugDD ) return;

    FILE* pf = fopen("CbotDebug.txt", "a");

    fputs( text, pf );

    CBotString v = " " + num(val) + "\n";
    fputs( v, pf );

    fclose( pf);
}

#endif
