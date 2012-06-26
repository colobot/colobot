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
// ce fichier défini les instructions suivantes:
//      CBotWhile   "while (condition) {instructions}"
//      CBotDo      "do {instructions} while (condition)"
//      CBotFor     "for (init, condition, incr) {instructions}"
//      CBotSwitch  "switch (val) {instructions}"
//      CBotCase    "case val:"
//      CBotBreak   "break", "break label", "continu", "continu label"
//      CBotTry     "try {instructions}"
//      CBotCatch   "catch (condition) {instructions}" ou "finally"
//      CBotThrow   "throw execption"


#include "CBot.h"

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "while"

CBotWhile::CBotWhile()
{
    m_Condition =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotWhile";     // debug
}

CBotWhile::~CBotWhile()
{
    delete  m_Condition;    // libère la condition
    delete  m_Block;        // libère le bloc d'instruction
}

CBotInstr* CBotWhile::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotWhile*  inst = new CBotWhile();         // crée l'objet
    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GivString();        // enregistre le nom du label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_WHILE)) return NULL;    // ne devrait jamais arriver

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    if ( NULL != (inst->m_Condition = CBotCondition::Compile( p, pStk )) )
    {
        // la condition existe

        IncLvl(inst->m_label);
        inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
        DecLvl();

        if ( pStk->IsOk() )
        {
            // le bloc d'instruction est ok (il peut être vide !

            return pStack->Return(inst, pStk);  // rend l'objet à qui le demande
        }
    }

    delete inst;                                // erreur, libère la place
    return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
}

// exécute une instruction "while"

BOOL CBotWhile :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);   // ajoute un élément à la pile
                                            // ou le retrouve en cas de reprise
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    while( TRUE ) switch( pile->GivState() )    // exécute la boucle
    {                                           // il y a 2 états possibles (selon reprise)
    case 0:
        // évalue la condition
        if ( !m_Condition->Execute(pile) ) return FALSE; // interrompu ici ?

        // le résultat de la condition est sur la pile

        // termine s'il y a une erreur ou si la condition est fausse
        if ( !pile->IsOk() || pile->GivVal() != TRUE )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        // la condition est vrai, passe dans le second mode

        if (!pile->SetState(1)) return FALSE;           // prêt pour la suite

    case 1:
        // évalue le bloc d'instruction associé
        if ( m_Block != NULL &&
            !m_Block->Execute(pile) )
        {
            if (pile->IfContinue(0, m_label)) continue; // si continue, repasse au test
            return pj->BreakReturn(pile, m_label);      // transmet le résultat et libère la pile
        }

        // termine s'il y a une erreur
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        // repasse au test pour recommencer
        if (!pile->SetState(0, 0)) return FALSE;
        continue;
    }
}

void CBotWhile :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;
    CBotStack* pile = pj->RestoreStack(this);   // ajoute un élément à la pile
    if ( pile == NULL ) return;

    switch( pile->GivState() )
    {                                           // il y a 2 états possibles (selon reprise)
    case 0:
        // évalue la condition
        m_Condition->RestoreState(pile, bMain);
        return;

    case 1:
        // évalue le bloc d'instruction associé
        if ( m_Block != NULL ) m_Block->RestoreState(pile, bMain);
        return;
    }
}


///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "repeat"

CBotRepeat::CBotRepeat()
{
    m_NbIter =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotRepeat";        // debug
}

CBotRepeat::~CBotRepeat()
{
    delete  m_NbIter;   // libère la condition
    delete  m_Block;        // libère le bloc d'instruction
}

CBotInstr* CBotRepeat::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotRepeat* inst = new CBotRepeat();            // crée l'objet
    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GivString();        // enregistre le nom du label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_REPEAT)) return NULL;   // ne devrait jamais arriver

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    if ( IsOfType(p, ID_OPENPAR ) )
    {
        CBotToken*  ppp = p;                    // conserve le ^au token (position début)
        if ( NULL != (inst->m_NbIter = CBotExpression::Compile( p, pStk )) )
        {
            if ( pStk->GivType() < CBotTypLong )
            {
                if ( IsOfType(p, ID_CLOSEPAR ) )
                {

                    IncLvl(inst->m_label);
                    inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
                    DecLvl();

                    if ( pStk->IsOk() )
                    {
                        // le bloc d'instruction est ok (il peut être vide !

                        return pStack->Return(inst, pStk);  // rend l'objet à qui le demande
                    }
                }
                pStack->SetError(TX_CLOSEPAR, p->GivStart());
            }
            pStk->SetStartError(ppp->GivStart());
            pStk->SetError( TX_BADTYPE, p->GivStart() );
        }
        pStack->SetError(TX_ENDOF, p);
    }
    pStack->SetError(TX_OPENPAR, p->GivStart());    // manque la parenthèse

    delete inst;                                // erreur, libère la place
    return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
}

// exécute une instruction "repeat"

BOOL CBotRepeat :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);   // ajoute un élément à la pile
                                            // ou le retrouve en cas de reprise
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    while( TRUE ) switch( pile->GivState() )    // exécute la boucle
    {                                           // il y a 2 états possibles (selon reprise)
    case 0:
        // évalue le nombre d'itération
        if ( !m_NbIter->Execute(pile) ) return FALSE; // interrompu ici ?

        // le résultat de la condition est sur la pile

        // termine s'il y a une erreur ou si la condition est fausse
        int     n;
        if ( !pile->IsOk() || ( n = pile->GivVal() ) < 1 )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        // met le nombre d'itération +1 dans le "state"

        if (!pile->SetState(n+1)) return FALSE;         // prêt pour la suite
        continue;                                       // passe à la suite

    case 1:
        // fin normale de la boucle
        return pj->Return(pile);                    // transmet le résultat et libère la pile

    default:
        // évalue le bloc d'instruction associé
        if ( m_Block != NULL &&
            !m_Block->Execute(pile) )
        {
            if (pile->IfContinue(pile->GivState()-1, m_label)) continue;    // si continue, repasse au test
            return pj->BreakReturn(pile, m_label);      // transmet le résultat et libère la pile
        }

        // termine s'il y a une erreur
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        // repasse au test pour recommencer
        if (!pile->SetState(pile->GivState()-1, 0)) return FALSE;
        continue;
    }
}

void CBotRepeat :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;
    CBotStack* pile = pj->RestoreStack(this);   // ajoute un élément à la pile
    if ( pile == NULL ) return;

    switch( pile->GivState() )
    {                                           // il y a 2 états possibles (selon reprise)
    case 0:
        // évalue la condition
        m_NbIter->RestoreState(pile, bMain);
        return;

    case 1:
        // évalue le bloc d'instruction associé
        if ( m_Block != NULL ) m_Block->RestoreState(pile, bMain);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "do"

CBotDo::CBotDo()
{
    m_Condition =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotDo";        // debug
}

CBotDo::~CBotDo()
{
    delete  m_Condition;    // libère la condition
    delete  m_Block;        // libère le bloc d'instruction
}

CBotInstr* CBotDo::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotDo* inst = new CBotDo();                // crée l'objet

    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GivString();        // enregistre le nom du label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_DO)) return NULL;       // ne devrait jamais arriver

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp


    // cherche un bloc d'instruction après le do
    IncLvl(inst->m_label);
    inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
    DecLvl();

    if ( pStk->IsOk() )
    {
        if (IsOfType(p, ID_WHILE))
        {
            if ( NULL != (inst->m_Condition = CBotCondition::Compile( p, pStk )) )
            {
                // la condition existe
                if (IsOfType(p, ID_SEP))
                {
                    return pStack->Return(inst, pStk);  // rend l'objet à qui le demande
                }
                pStk->SetError(TX_ENDOF, p->GivStart());
            }
        }
        pStk->SetError(TX_WHILE, p->GivStart());
    }

    delete inst;                                // erreur, libère la place
    return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
}

// exécute une instruction "do"

BOOL CBotDo :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);       // ajoute un élément à la pile
                                                // ou le retrouve en cas de reprise
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    while( TRUE ) switch( pile->GivState() )            // exécute la boucle
    {                                                   // il y a 2 états possibles (selon reprise)
    case 0:
        // évalue le bloc d'instruction associé
        if ( m_Block != NULL &&
            !m_Block->Execute(pile) )
        {
            if (pile->IfContinue(1, m_label)) continue; // si continue, repasse au test
            return pj->BreakReturn(pile, m_label);      // transmet le résultat et libère la pile
        }

        // termine s'il y a une erreur
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        if (!pile->SetState(1)) return FALSE;           // prêt pour la suite

    case 1:
        // évalue la condition
        if ( !m_Condition->Execute(pile) ) return FALSE; // interrompu ici ?

        // le résultat de la condition est sur la pile

        // termine s'il y a une erreur ou si la condition est fausse
        if ( !pile->IsOk() || pile->GivVal() != TRUE )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        // repasse au bloc d'instruction pour recommencer
        if (!pile->SetState(0, 0)) return FALSE;
        continue;
    }
}

void CBotDo :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack* pile = pj->RestoreStack(this);           // ajoute un élément à la pile
    if ( pile == NULL ) return;

    switch( pile->GivState() )
    {                                                   // il y a 2 états possibles (selon reprise)
    case 0:
        // restitue le bloc d'instruction associé
        if ( m_Block != NULL ) m_Block->RestoreState(pile, bMain);
        return;

    case 1:
        // restitue la condition
        m_Condition->RestoreState(pile, bMain);
        return;
    }
}


///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "for"

CBotFor::CBotFor()
{
    m_Init      =
    m_Test      =
    m_Incr      =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotFor";       // debug
}

CBotFor::~CBotFor()
{
    delete  m_Init;
    delete  m_Test;
    delete  m_Incr;
    delete  m_Block;        // libère le bloc d'instruction
}

CBotInstr* CBotFor::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotFor*    inst = new CBotFor();           // crée l'objet
    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    if ( IsOfType( p, TokenTypVar ) &&
         IsOfType( p, ID_DOTS ) )
    {
        inst->m_label = pp->GivString();        // enregistre le nom du label
    }

    inst->SetToken(p);
    if (!IsOfType(p, ID_FOR)) return NULL;      // ne devrait jamais arriver

    if ( !IsOfType(p, ID_OPENPAR))              // manque la parenthèse ?
    {
        pStack->SetError(TX_OPENPAR, p->GivStart());
        return NULL;
    }

    CBotCStack* pStk = pStack->TokenStack(pp, TRUE);    // un petit bout de pile svp

    // compile les instructions pour initialisation
    inst->m_Init = CBotListExpression::Compile( p, pStk );
    if ( pStk->IsOk() )
    {
        if ( !IsOfType(p, ID_SEP))                      // manque le point-virgule ?
        {
            pStack->SetError(TX_OPENPAR, p->GivStart());
            delete inst;
            return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
        }
        inst->m_Test = CBotBoolExpr::Compile( p, pStk );
        if ( pStk->IsOk() )
        {
            if ( !IsOfType(p, ID_SEP))                      // manque le point-virgule ?
            {
                pStack->SetError(TX_OPENPAR, p->GivStart());
                delete inst;
                return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
            }
            inst->m_Incr = CBotListExpression::Compile( p, pStk );
            if ( pStk->IsOk() )
            {
                if ( IsOfType(p, ID_CLOSEPAR))              // manque la parenthèse ?
                {
                    IncLvl(inst->m_label);
                    inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
                    DecLvl();
                    if ( pStk->IsOk() )
                        return pStack->Return(inst, pStk);;
                }
                pStack->SetError(TX_CLOSEPAR, p->GivStart());
            }
        }
    }

    delete inst;                                // erreur, libère la place
    return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
}

// exécute l'instruction "for"

BOOL CBotFor :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this, TRUE);     // ajoute un élément à la pile (variables locales)
                                                    // ou le retrouve en cas de reprise
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    while( TRUE ) switch( pile->GivState() )    // exécute la boucle
    {                                           // il y a 4 états possibles (selon reprise)
    case 0:
        // évalue l'initialisation
        if ( m_Init != NULL &&
             !m_Init->Execute(pile) ) return FALSE;     // interrompu ici ?
        if (!pile->SetState(1)) return FALSE;           // prêt pour la suite

    case 1:
        // évalue la condition
        if ( m_Test != NULL )                           // pas de condition ? -> vrai !
        {
            if (!m_Test->Execute(pile) ) return FALSE;  // interrompu ici ?

            // le résultat de la condition est sur la pile

            // termine s'il y a une erreur ou si la condition est fausse
            if ( !pile->IsOk() || pile->GivVal() != TRUE )
            {
                return pj->Return(pile);                // transmet le résultat et libère la pile
            }
        }

        // la condition est vrai, passe à la suite
        if (!pile->SetState(2)) return FALSE;           // prêt pour la suite

    case 2:
        // évalue le bloc d'instruction associé
        if ( m_Block != NULL &&
            !m_Block->Execute(pile) )
        {
            if (pile->IfContinue(3, m_label)) continue; // si continue, passe à l'incrémentation
            return pj->BreakReturn(pile, m_label);      // transmet le résultat et libère la pile
        }

        // termine s'il y a une erreur
        if ( !pile->IsOk() )
        {
            return pj->Return(pile);                    // transmet le résultat et libère la pile
        }

        if (!pile->SetState(3)) return FALSE;           // prêt pour la suite

    case 3:
        // évalue l'incrémentation
        if ( m_Incr != NULL &&
            !m_Incr->Execute(pile) ) return FALSE;      // interrompu ici ?

        // repasse au test pour recommencer
        if (!pile->SetState(1, 0)) return FALSE;            // revient au test
        continue;
    }
}

void CBotFor :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack* pile = pj->RestoreStack(this);       // ajoute un élément à la pile (variables locales)
    if ( pile == NULL ) return;

    switch( pile->GivState() )
    {                                           // il y a 4 états possibles (selon reprise)
    case 0:
        // évalue l'initialisation
        if ( m_Init != NULL ) m_Init->RestoreState(pile, TRUE);     // interrompu ici !
        return;

    case 1:
        if ( m_Init != NULL ) m_Init->RestoreState(pile, FALSE);    // définitions variables

        // évalue la condition
        if ( m_Test != NULL ) m_Test->RestoreState(pile, TRUE);     // interrompu ici !
        return;

    case 2:
        if ( m_Init != NULL ) m_Init->RestoreState(pile, FALSE);    // définitions variables

        // évalue le bloc d'instruction associé
        if ( m_Block != NULL ) m_Block->RestoreState(pile, TRUE);
        return;

    case 3:
        if ( m_Init != NULL ) m_Init->RestoreState(pile, FALSE);    // définitions variables

        // évalue l'incrémentation
        if ( m_Incr != NULL ) m_Incr->RestoreState(pile, TRUE);     // interrompu ici !
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////
// compile une liste d'expression
// n'est utilisé que pour l'instruction for
// dans l'intitialisation et dans l'incrémentation

CBotListExpression::CBotListExpression()
{
    m_Expr  = NULL;
    name = "CBotListExpression";
}

CBotListExpression::~CBotListExpression()
{
    delete  m_Expr;
}

// cherche une déclaration de variable ou une expression

static CBotInstr* CompileInstrOrDefVar(CBotToken* &p, CBotCStack* pStack)
{
    CBotInstr*  i = CBotInt::Compile( p, pStack, FALSE, TRUE );         // est-ce une déclaration d'un entier ?
    if ( i== NULL ) i = CBotFloat::Compile( p, pStack, FALSE, TRUE );   // ou d'un nombre réel ?
    if ( i== NULL ) i = CBotBoolean::Compile( p, pStack, FALSE, TRUE ); // ou d'un booléen ?
    if ( i== NULL ) i = CBotIString::Compile( p, pStack, FALSE, TRUE ); // ou d'une chaîne ?
    if ( i== NULL ) i = CBotExpression::Compile( p, pStack );           // compile une expression
    return i;
}

CBotInstr* CBotListExpression::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotListExpression* inst = new CBotListExpression();

    inst->m_Expr = CompileInstrOrDefVar( p, pStack );           // compile la première expression de la liste
    if (pStack->IsOk())
    {
        while ( IsOfType(p, ID_COMMA) )                         // plusieurs instructions ?
        {
            CBotInstr*  i = CompileInstrOrDefVar( p, pStack );      // est-ce une déclaration d'un entier ?
            inst->m_Expr->AddNext(i);                           // ajoute à la suite
            if ( !pStack->IsOk() )
            {
                delete inst;
                return NULL;                                    // pas d'objet, l'erreur est sur la pile
            }
        }
        return inst;
    }
    delete inst;
    return NULL;
}

BOOL CBotListExpression::Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack();// indispensable
    CBotInstr*  p = m_Expr;                                     // la première expression

    int     state = pile->GivState();
    while (state-->0) p = p->GivNext();                         // revient sur l'opération interrompue

    if ( p != NULL ) while (TRUE)
    {
        if ( !p->Execute(pile) ) return FALSE;
        p = p->GivNext();
        if ( p == NULL ) break;
        if (!pile->IncState()) return FALSE;                    // prêt pour la suivante
    }
    return pj->Return(pile);
}

void CBotListExpression::RestoreState(CBotStack* &pj, BOOL bMain)
{
    CBotStack*  pile = pj;
    int     state = 0x7000;

    if ( bMain )
    {
        pile = pj->RestoreStack();
        if ( pile == NULL ) return;
        state = pile->GivState();
    }

    CBotInstr*  p = m_Expr;                                     // la première expression

    while (p != NULL && state-->0)
    {
        p->RestoreState(pile, FALSE);
        p = p->GivNext();                           // revient sur l'opération interrompue
    }

    if ( p != NULL )
    {
        p->RestoreState(pile, bMain);
    }
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "switch"

CBotSwitch::CBotSwitch()
{
    m_Value     =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotSwitch";        // debug
}

CBotSwitch::~CBotSwitch()
{
    delete  m_Value;        // libère la valeur
    delete  m_Block;        // libère le bloc d'instruction
}


CBotInstr* CBotSwitch::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotSwitch* inst = new CBotSwitch();        // crée l'objet
    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    inst->SetToken(p);
    if (!IsOfType(p, ID_SWITCH)) return NULL;   // ne devrait jamais arriver

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    if ( IsOfType(p, ID_OPENPAR ) )
    {
        if ( NULL != (inst->m_Value = CBotExpression::Compile( p, pStk )) )
        {
            if ( pStk->GivType() < CBotTypLong )
            {
                if ( IsOfType(p, ID_CLOSEPAR ) )
                {
                    if ( IsOfType(p, ID_OPBLK ) )
                    {
                        IncLvl();

                        while( !IsOfType( p, ID_CLBLK ) )
                        {
                            if ( p->GivType() == ID_CASE || p->GivType() == ID_DEFAULT)
                            {
                                CBotCStack* pStk2 = pStk->TokenStack(p);    // un petit bout de pile svp

                                CBotInstr* i = CBotCase::Compile( p, pStk2 );
                                if (i == NULL)
                                {
                                    delete inst;
                                    return pStack->Return(NULL, pStk2);
                                }
                                delete pStk2;
                                if ( inst->m_Block == NULL ) inst->m_Block = i;
                                else inst->m_Block->AddNext(i);
                                continue;
                            }

                            if ( inst->m_Block == NULL )
                            {
                                pStk->SetError(TX_NOCASE, p->GivStart());
                                delete inst;
                                return pStack->Return(NULL, pStk);
                            }

                            CBotInstr* i = CBotBlock::CompileBlkOrInst( p, pStk, TRUE );
                            if ( !pStk->IsOk() )
                            {
                                delete inst;
                                return pStack->Return(NULL, pStk);
                            }
                            inst->m_Block->AddNext(i);

                            if ( p == NULL )
                            {
                                pStk->SetError(TX_CLOSEBLK, -1);
                                delete inst;
                                return pStack->Return(NULL, pStk);
                            }
                        }
                        DecLvl();

                        if ( inst->m_Block == NULL )
                        {
                            pStk->SetError(TX_NOCASE, p->GivStart());
                            delete inst;
                            return pStack->Return(NULL, pStk);
                        }
                        // le bloc d'instruction est ok
                        return pStack->Return(inst, pStk);  // rend l'objet à qui le demande
                    }
                    pStk->SetError( TX_OPENBLK, p->GivStart() );
                }
                pStk->SetError( TX_CLOSEPAR, p->GivStart() );
            }
            pStk->SetError( TX_BADTYPE, p->GivStart() );
        }
    }
    pStk->SetError( TX_OPENPAR, p->GivStart());

    delete inst;                                // erreur, libère la place
    return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
}

// exécute une instruction "switch"

BOOL CBotSwitch :: Execute(CBotStack* &pj)
{
    CBotStack* pile1 = pj->AddStack(this);      // ajoute un élément à la pile
//  if ( pile1 == EOX ) return TRUE;

    CBotInstr*  p = m_Block;                    // la première expression

    int     state = pile1->GivState();
    if (state == 0)
    {
        if ( !m_Value->Execute(pile1) ) return FALSE;
        pile1->SetState(state = -1);
    }

    if ( pile1->IfStep() ) return FALSE;

    if ( state == -1 )
    {
        state = 0;
        int val = pile1->GivVal();                      // résultat de la valeur

        CBotStack* pile2 = pile1->AddStack();
        while ( p != NULL )                             // recherche le case correspondant dans la liste
        {
            state++;
            if ( p->CompCase( pile2, val ) ) break;     // trouvé le case
            p = p->GivNext();
        }
        pile2->Delete();

        if ( p == NULL ) return pj->Return(pile1);      // terminé si plus rien

        if ( !pile1->SetState(state) ) return FALSE;
    }

    p = m_Block;                                        // revient au début
    while (state-->0) p = p->GivNext();                 // avance dans la liste

    while( p != NULL )
    {
        if ( !p->Execute(pile1) ) return pj->BreakReturn(pile1);
        if ( !pile1->IncState() ) return FALSE;
        p = p->GivNext();
    }
    return pj->Return(pile1);
}

void CBotSwitch :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack* pile1 = pj->RestoreStack(this);  // ajoute un élément à la pile
    if ( pile1 == NULL ) return;

    CBotInstr*  p = m_Block;                    // la première expression

    int     state = pile1->GivState();
    if (state == 0)
    {
        m_Value->RestoreState(pile1, bMain);
        return;
    }

    if ( state == -1 )
    {
        return;
    }

//  p = m_Block;                                // revient au début
    while ( p != NULL && state-- > 0 )
    {
        p->RestoreState(pile1, FALSE);
        p = p->GivNext();                       // avance dans la liste
    }

    if( p != NULL )
    {
        p->RestoreState(pile1, TRUE);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "case"
// on est forcément dans un bloc d'instruction "switch"

CBotCase::CBotCase()
{
    m_Value     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotCase";      // debug
}

CBotCase::~CBotCase()
{
    delete  m_Value;        // libère la valeur
}


CBotInstr* CBotCase::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCase*   inst = new CBotCase();          // crée l'objet
    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    inst->SetToken(p);
    if (!IsOfType(p, ID_CASE, ID_DEFAULT)) return NULL;     // ne devrait jamais arriver

    if ( pp->GivType() == ID_CASE )
    {
        pp = p;
        inst->m_Value = CBotExprNum::Compile(p, pStack);
        if ( inst->m_Value == NULL )
        {
            pStack->SetError( TX_BADNUM, pp );
            delete inst;
            return NULL;
        }
    }
    if ( !IsOfType( p, ID_DOTS ))
    {
        pStack->SetError( TX_MISDOTS, p->GivStart() );
        delete inst;
        return NULL;
    }

    return inst;
}

// exécution de l'instruction "case"

BOOL CBotCase::Execute(CBotStack* &pj)
{
    return TRUE;                                // l'instruction "case" ne fait rien !
}

void CBotCase::RestoreState(CBotStack* &pj, BOOL bMain)
{
}

// routine permettant de trouver le point d'entrée "case"
// correspondant à la valeur cherchée

BOOL CBotCase::CompCase(CBotStack* &pile, int val)
{
    if ( m_Value == NULL ) return TRUE;         // cas pour "default"

    while (!m_Value->Execute(pile));            // met sur la pile la valeur correpondant (sans interruption)
    return (pile->GivVal() == val);             // compare avec la valeur cherchée
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "break" ou "continu"

CBotBreak::CBotBreak()
{
    name = "CBotBreak";     // debug
}

CBotBreak::~CBotBreak()
{
}

CBotInstr* CBotBreak::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotToken*  pp = p;                         // conserve le ^au token (position début)
    int type = p->GivType();

    if (!IsOfType(p, ID_BREAK, ID_CONTINUE)) return NULL;   // ne devrait jamais arriver

    if ( !ChkLvl(CBotString(), type ) )
    {
        pStack->SetError(TX_BREAK, pp);
        return NULL;                            // pas d'objet, l'erreur est sur la pile
    }

    CBotBreak*  inst = new CBotBreak();         // crée l'objet
    inst->SetToken(pp);                         // garde l'opération

    pp = p;
    if ( IsOfType( p, TokenTypVar ) )
    {
        inst->m_label = pp->GivString();        // enregistre le nom du label
        if ( !ChkLvl(inst->m_label, type ) )
        {
            delete inst;
            pStack->SetError(TX_NOLABEL, pp);
            return NULL;                            // pas d'objet, l'erreur est sur la pile
        }
    }

    if (IsOfType(p, ID_SEP))
    {
        return  inst;                           // et le donne à qui veux
    }
    delete inst;

    pStack->SetError(TX_ENDOF, p->GivStart());
    return NULL;                            // pas d'objet, l'erreur est sur la pile
}

// exécution l'instructino "break" ou "continu"

BOOL CBotBreak :: Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->IfStep() ) return FALSE;

    pile->SetBreak(m_token.GivType()==ID_BREAK ? 1 : 2, m_label);
    return pj->Return(pile);
}

void CBotBreak :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain ) pj->RestoreStack(this);
}


///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "try"

CBotTry::CBotTry()
{
    m_ListCatch = NULL;
    m_FinalInst =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    name = "CBotTry";       // debug
}

CBotTry::~CBotTry()
{
    delete  m_ListCatch;    // libère la liste
    delete  m_Block;        // libère le bloc d'instruction
    delete  m_FinalInst;
}

CBotInstr* CBotTry::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotTry*    inst = new CBotTry();           // crée l'objet
    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    inst->SetToken(p);
    if (!IsOfType(p, ID_TRY)) return NULL;      // ne devrait jamais arriver

    CBotCStack* pStk = pStack->TokenStack(pp);  // un petit bout de pile svp

    inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStk );
    CBotCatch** pn = &inst->m_ListCatch;

    while (pStk->IsOk() && p->GivType() == ID_CATCH)
    {
        CBotCatch*  i = CBotCatch::Compile(p, pStk);
        *pn = i;
        pn = &i->m_next;
    }

    if (pStk->IsOk() && IsOfType( p, ID_FINALLY) )
    {
        inst->m_FinalInst = CBotBlock::CompileBlkOrInst( p, pStk );
    }

    if (pStk->IsOk())
    {
        return pStack->Return(inst, pStk);  // rend l'objet à qui le demande
    }

    delete inst;                                // erreur, libère la place
    return pStack->Return(NULL, pStk);          // pas d'objet, l'erreur est sur la pile
}

// exécute l'instruction Try
// gère le retour d'exceptions
// les arrêts par suspension
// et les "finaly"

BOOL CBotTry :: Execute(CBotStack* &pj)
{
    int     val;

    CBotStack* pile1 = pj->AddStack(this);  // ajoute un élément à la pile
//  if ( pile1 == EOX ) return TRUE;

    if ( pile1->IfStep() ) return FALSE;
                                                    // ou le retrouve en cas de reprise
    CBotStack* pile0 = pj->AddStack2();             // ajoute un élément à la pile secondaire
    CBotStack* pile2 = pile0->AddStack();

    if ( pile1->GivState() == 0 )
    {
        if ( m_Block->Execute(pile1) )
        {
            if ( m_FinalInst == NULL ) return pj->Return(pile1);
            pile1->SetState(-2);                                // passe au final
        }

        val = pile1->GivError();
        if ( val == 0 && CBotStack::m_initimer == 0 )           // en mode de step ?
            return FALSE;                                       // ne fait pas le catch

        pile1->IncState();
        pile2->SetState(val);                                   // mémorise le numéro de l'erreur
        pile1->SetError(0);                                     // pour l'instant il n'y a plus d'erreur !

        if ( val == 0 && CBotStack::m_initimer < 0 )            // en mode de step ?
            return FALSE;                                       // ne fait pas le catch
    }

    // il y a eu une interruption
    // voir de quoi il en retourne

    CBotCatch*  pc = m_ListCatch;
    int state = (short)pile1->GivState();                               // où en étions-nous ?
    val = pile2->GivState();                                    // pour quelle erreur ?
    pile0->SetState(1);                                         // marquage pour GetRunPos

    if ( val >= 0 && state > 0 ) while ( pc != NULL )
    {
        if ( --state <= 0 )
        {
            // demande au bloc catch s'il se sent concerné
            if ( !pc->TestCatch(pile2, val) ) return FALSE;     // suspendu !
            pile1->IncState();
        }
        if ( --state <= 0 )
        {
            if ( pile2->GivVal() == TRUE )
            {
//              pile0->SetState(1);

                if ( !pc->Execute(pile2) ) return FALSE;        // exécute l'opération
                if ( m_FinalInst == NULL )
                    return pj->Return(pile2);                   // termine le try

                pile1->SetState(-2);                            // passe au final
                break;
            }
            pile1->IncState();
        }
        pc = pc->m_next;
    }
    if ( m_FinalInst != NULL &&
         pile1->GivState() > 0 && val != 0 ) pile1->SetState(-1);// si arret alors fait le final

    if (pile1->GivState() <= -1)
    {
//      pile0->SetState(1);

        if (!m_FinalInst->Execute(pile2) && pile2->IsOk()) return FALSE;
        if (!pile2->IsOk()) return pj->Return(pile2);           // garde cette exception
        pile2->SetError(pile1->GivState()==-1 ? val : 0);       // remet l'erreur initiale
        return pj->Return(pile2);
    }

    pile1->SetState(0);                                         // revient à l'évaluation
    pile0->SetState(0);                                         // revient à l'évaluation
    if ( val != 0 && m_ListCatch == NULL && m_FinalInst == NULL )
                            return pj->Return(pile2);           // termine le try sans exception aucune

    pile1->SetError(val);                                       // remet l'erreur
    return FALSE;                                               // ce n'est pas pour nous
}


void CBotTry :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    int     val;
    CBotStack* pile1 = pj->RestoreStack(this);  // ajoute un élément à la pile
    if ( pile1 == NULL ) return;
                                                    // ou le retrouve en cas de reprise
    CBotStack* pile0 = pj->AddStack2();             // ajoute un élément à la pile secondaire
    if ( pile0 == NULL ) return;

    CBotStack* pile2 = pile0->RestoreStack();
    if ( pile2 == NULL ) return;

    m_Block->RestoreState(pile1, bMain);
    if ( pile0->GivState() == 0 )
    {
        return;
    }

    // il y a eu une interruption
    // voir de quoi il en retourne

    CBotCatch*  pc = m_ListCatch;
    int state = pile1->GivState();                              // où en étions-nous ?
    val = pile2->GivState();                                    // pour quelle erreur ?

    if ( val >= 0 && state > 0 ) while ( pc != NULL )
    {
        if ( --state <= 0 )
        {
            // demande au bloc catch s'il se sent concerné
            pc->RestoreCondState(pile2, bMain);     // suspendu !
            return;
        }
        if ( --state <= 0 )
        {
            if ( pile2->GivVal() == TRUE )
            {
                pc->RestoreState(pile2, bMain);                 // exécute l'opération
                return;
            }
        }
        pc = pc->m_next;
    }

    if (pile1->GivState() <= -1)
    {
        m_FinalInst->RestoreState(pile2, bMain);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "catch"

CBotCatch::CBotCatch()
{
    m_Cond      =
    m_Block     = NULL;     // NULL pour que delete soit possible sans autre
    m_next      = NULL;

    name = "CBotCatch";     // debug
}

CBotCatch::~CBotCatch()
{
    delete  m_Cond;         // libère la liste
    delete  m_Block;        // libère le bloc d'instruction
    delete  m_next;         // et la suite
}

CBotCatch* CBotCatch::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCatch*  inst = new CBotCatch();         // crée l'objet
    pStack->SetStartError(p->GivStart());

    inst->SetToken(p);
    if (!IsOfType(p, ID_CATCH)) return NULL;    // ne devrait jamais arriver

    if (IsOfType(p, ID_OPENPAR))
    {
        inst->m_Cond = CBotExpression::Compile(p, pStack);
        if (( pStack->GivType() < CBotTypLong ||
              pStack->GivTypResult().Eq(CBotTypBoolean) )&& pStack->IsOk() )
        {
            if (IsOfType(p, ID_CLOSEPAR))
            {
                inst->m_Block = CBotBlock::CompileBlkOrInst( p, pStack );
                if ( pStack->IsOk() )
                    return inst;                // rend l'objet à qui le demande
            }
            pStack->SetError(TX_CLOSEPAR, p->GivStart());
        }
        pStack->SetError(TX_BADTYPE, p->GivStart());
    }
    pStack->SetError(TX_OPENPAR, p->GivStart());
    delete inst;                                // erreur, libère la place
    return NULL;                                // pas d'objet, l'erreur est sur la pile
}

// exécution de "catch"

BOOL CBotCatch :: Execute(CBotStack* &pj)
{
    if ( m_Block == NULL ) return TRUE;
    return m_Block->Execute(pj);                // exécute le bloc associé
}

void CBotCatch :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( bMain && m_Block != NULL ) m_Block->RestoreState(pj, bMain);
}

void CBotCatch :: RestoreCondState(CBotStack* &pj, BOOL bMain)
{
    m_Cond->RestoreState(pj, bMain);
}

// routine pour savoir si le catch est à faire ou non

BOOL CBotCatch :: TestCatch(CBotStack* &pile, int val)
{
    if ( !m_Cond->Execute(pile) ) return FALSE;

    if ( val > 0 || pile->GivType() != CBotTypBoolean )
    {
        CBotVar* var = CBotVar::Create((CBotToken*)NULL, CBotTypBoolean);
        var->SetValInt( pile->GivVal() == val );
        pile->SetVar(var);                          // remet sur la pile
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// compile une instruction "throw"

CBotThrow::CBotThrow()
{
    m_Value     = NULL;     // NULL pour que delete soit possible sans autre

    name = "CBotThrow";     // debug
}

CBotThrow::~CBotThrow()
{
    delete  m_Value;
}

CBotInstr* CBotThrow::Compile(CBotToken* &p, CBotCStack* pStack)
{
    pStack->SetStartError(p->GivStart());

    CBotThrow*  inst = new CBotThrow();         // crée l'objet
    inst->SetToken(p);

    CBotToken*  pp = p;                         // conserve le ^au token (position début)

    if (!IsOfType(p, ID_THROW)) return NULL;    // ne devrait jamais arriver

    inst->m_Value = CBotExpression::Compile( p, pStack );

    if (pStack->GivType() < CBotTypLong && pStack->IsOk())
    {
        return inst;                            // rend l'objet à qui le demande
    }
    pStack->SetError(TX_BADTYPE, pp);

    delete inst;                                // erreur, libère la place
    return NULL;                                // pas d'objet, l'erreur est sur la pile
}

// exécute l'instruction "throw"

BOOL CBotThrow :: Execute(CBotStack* &pj)
{
    CBotStack*  pile = pj->AddStack(this);
//  if ( pile == EOX ) return TRUE;

    if ( pile->GivState() == 0 )
    {
        if ( !m_Value->Execute(pile) ) return FALSE;
        pile->IncState();
    }

    if ( pile->IfStep() ) return FALSE;

    int val = pile->GivVal();
    if ( val < 0 ) val = TX_BADTHROW;
    pile->SetError( val, &m_token );
    return pj->Return( pile );
}

void CBotThrow :: RestoreState(CBotStack* &pj, BOOL bMain)
{
    if ( !bMain ) return;

    CBotStack*  pile = pj->RestoreStack(this);
    if ( pile == NULL ) return;

    if ( pile->GivState() == 0 )
    {
        m_Value->RestoreState(pile, bMain);
        return;
    }
}



////////////////////////////////////////////////////////////


CBotStartDebugDD::CBotStartDebugDD()
{
    name = "CBotStartDebugDD";      // debug
}

CBotStartDebugDD::~CBotStartDebugDD()
{
}

CBotInstr* CBotStartDebugDD::Compile(CBotToken* &p, CBotCStack* pStack)
{

    if (!IsOfType(p, ID_DEBUGDD)) return NULL;  // ne devrait jamais arriver

    return new CBotStartDebugDD();          // crée l'objet

}

// exécute l'instruction "throw"

BOOL CBotStartDebugDD :: Execute(CBotStack* &pj)
{
    CBotProgram* p = pj->GivBotCall();
    p->m_bDebugDD = TRUE;

    return TRUE;
}

