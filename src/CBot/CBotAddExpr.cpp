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
// * along with this program. If not, see  http://www.gnu.org/licenses/.///////////////////////////////////////////////////
// expression du genre Opérande1 + Opérande2
//                     Opérande1 - Opérande2

#include "CBot.h"

// divers constructeurs

CBotAddExpr::CBotAddExpr()
{
    m_leftop    =
    m_rightop   = NULL;         // NULL pour pouvoir faire delete sans autre
    name = "CBotAddExpr";       // debug
}

CBotAddExpr::~CBotAddExpr()
{
    delete  m_leftop;
    delete  m_rightop;
}


// compile une instruction de type A + B

CBotInstr* CBotAddExpr::Compile(CBotToken* &p, CBotStack* pStack)
{
    CBotStack* pStk = pStack->TokenStack();                 // un bout de pile svp

    // cherche des instructions qui peuvent convenir à gauche de l'opération + ou -

    CBotInstr*  left = CBotMulExpr::Compile( p, pStk );     // expression A * B à gauche
    if (left == NULL) return pStack->Return(NULL, pStk);    // si erreur, la transmet

    // est-ce qu'on a le token + ou - ensuite ?

    if ( p->GetType() == ID_ADD ||
         p->GetType() == ID_SUB)                            // plus ou moins
    {
        CBotAddExpr* inst = new CBotAddExpr();              // élément pour opération
        inst->SetToken(p);                                  // mémorise l'opération

        int          type1, type2;
        type1 = pStack->GetType();                          // de quel type le premier opérande ?

        p = p->Next();                                      // saute le token de l'opération

        // cherche des instructions qui peuvent convenir à droite

        if ( NULL != (inst->m_rightop = CBotAddExpr::Compile( p, pStk )) )  // expression (...) à droite
        {
            // il y a un second opérande acceptable

            type2 = pStack->GetType();                      // de quel type le résultat ?

            if ( type1 == type2 )                           // les résultats sont-ils compatibles
            {
                // si ok, enregistre l'opérande dans l'objet
                inst->m_leftop = left;
                // et rend l'object à qui l'a demandé
                return pStack->Return(inst, pStk);
            }
        }

        // en cas d'erreur, libère les éléments
        delete left;
        delete inst;
        // et transmet l'erreur qui se trouve sur la pile
        return pStack->Return(NULL, pStk);
    }

    // si on n'a pas affaire à une opération + ou -
    // rend à qui l'a demandé, l'opérande (de gauche) trouvé
    // à la place de l'objet "addition"
    return pStack->Return(left, pStk);
}




// fait l'opération d'addition ou de soustraction

BOOL CBotAddExpr::Execute(CBotStack* &pStack)
{
    CBotStack* pStk1 = pStack->AddStack(this);  // ajoute un élément à la pile
                                                // ou le retrouve en cas de reprise
//  if ( pSk1 == EOX ) return TRUE;


    // selon la reprise, on peut être dans l'un des 2 états

    if ( pStk1->GetState() == 0 &&              // 1er état, évalue l'opérande de gauche
        !m_leftop->Execute(pStk1) ) return FALSE; // interrompu ici ?

    // passe à l'étape suivante
    pStk1->SetState(1);                         // prêt pour la suite

    // demande un peu plus de stack pour ne pas toucher le résultat de gauche
    // qui se trouve sur la pile, justement.

    CBotStack* pStk2 = pStk1->AddStack();       // ajoute un élément à la pile
                                                // ou le retrouve en cas de reprise

    // 2e état, évalue l'opérande de droite
    if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrompu ici ?

    int     type1 = pStk1->GetType();           // de quels types les résultats ?
    int     type2 = pStk2->GetType();

    // crée une variable temporaire pour y mettre le résultat
    CBotVar*    result = new CBotVar( NULL, MAX(type1, type2));

    // fait l'opération selon la demande
    switch (GetTokenType())
    {
    case ID_ADD:
        result->Add(pStk1->GetVar(), pStk2->GetVar());      // additionne
        break;
    case ID_SUB:
        result->Sub(pStk1->GetVar(), pStk2->GetVar());      // soustrait
        break;
    }
    pStk2->SetVar(result);                      // met le résultat sur la pile

    pStk1->Return(pStk2);                       // libère la pile
    return pStack->Return(pStk1);               // transmet le résultat
}


