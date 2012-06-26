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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// définition des fonctions sur les chaînes


// donne la longueur d'une chaîne
// exécution

BOOL rStrLen( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // pas de second paramètre
    if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // met la longueur sur la pile
    pResult->SetValInt( s.GivLength() );
    return TRUE;
}

// int xxx ( string )
// compilation

CBotTypResult cIntStr( CBotVar* &pVar, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADPARAM );

    // pas de second paramètre
    if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // le résultat final est un nombre entier
    return CBotTypResult( CBotTypInt );
}


// donne la partie gauche d'une chaîne
// exécution

BOOL rStrLeft( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être un nombre
    if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return TRUE; }

    // récupère ce nombre
    int n = pVar->GivValInt();

    // pas de 3e paramètre
    if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return TRUE; }

    // prend la partie intéressante
    s = s.Left( n );

    // la met sur la pile
    pResult->SetValString( s );
    return TRUE;
}

// string xxx ( string, int )
// compilation

CBotTypResult cStrStrInt( CBotVar* &pVar, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être un nombre
    if ( pVar->GivType() > CBotTypDouble )
                        return CBotTypResult( TX_BADNUM );

    // pas de 3e paramètre
    if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // le résultat final est une string
    return CBotTypResult( CBotTypString );
}

// donne la partie droite d'une chaîne
// exécution

BOOL rStrRight( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être un nombre
    if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return TRUE; }

    // récupère ce nombre
    int n = pVar->GivValInt();

    // pas de 3e paramètre
    if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return TRUE; }

    // prend la partie intéressante
    s = s.Right( n );

    // la met sur la pile
    pResult->SetValString( s );
    return TRUE;
}

// donne la partie centrale d'une chaîne
// exécution

BOOL rStrMid( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être un nombre
    if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return TRUE; }

    // récupère ce nombre
    int n = pVar->GivValInt();

    // 3e paramètre optionnel
    if ( pVar->GivNext() != NULL )
    {
        pVar = pVar->GivNext();

        // qui doit être un nombre
        if ( pVar->GivType() > CBotTypDouble ) { ex = TX_BADNUM ; return TRUE; }

        // récupère ce nombre
        int l = pVar->GivValInt();

        // mais pas de 4e paramètre
        if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return TRUE; }

        // prend la partie intéressante
        s = s.Mid( n, l );
    }
    else
    {
        // prend la partie intéressante
        s = s.Mid( n );
    }

    // la met sur la pile
    pResult->SetValString( s );
    return TRUE;
}

// donne la partie centrale d'une chaîne
// compilation

CBotTypResult cStrStrIntInt( CBotVar* &pVar, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être un nombre
    if ( pVar->GivType() > CBotTypDouble )
                        return CBotTypResult( TX_BADNUM );

    // 3e paramètre optionnel
    if ( pVar->GivNext() != NULL )
    {

        pVar = pVar->GivNext();
        // qui doit être un nombre
        if ( pVar->GivType() > CBotTypDouble )
                            return CBotTypResult( TX_BADNUM );

        // pas de 4e paramètre
        if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );
    }

    // le résultat final est une string
    return CBotTypResult( CBotTypString );
}


// donne le nombre contenu dans une chaîne
// exécution

BOOL rStrVal( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // mais pas de 2e paramètre
    if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return TRUE; }

    float val = GivNumFloat(s);

    // la met la valeur sur la pile
    pResult->SetValFloat( val );
    return TRUE;
}

// float xxx ( string )
// compilation

CBotTypResult cFloatStr( CBotVar* &pVar, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // pas de 2e paramètre
    if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // le résultat final est un nombre
    return CBotTypResult( CBotTypFloat );
}


// trouve une chaine dans une autre
// exécution

BOOL rStrFind( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // récupère ce nombre
    CBotString s2 = pVar->GivValString();

    // pas de 3e paramètre
    if ( pVar->GivNext() != NULL ) { ex = TX_OVERPARAM ; return TRUE; }

    // met le résultat sur la pile
    int res = s.Find(s2);
    pResult->SetValInt( res );
    if ( res < 0 ) pResult->SetInit( IS_NAN );
    return TRUE;
}

// int xxx ( string, string )
// compilation

CBotTypResult cIntStrStr( CBotVar* &pVar, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // il faut un second paramètre
    pVar = pVar->GivNext();
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // pas de 3e paramètre
    if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // le résultat final est un nombre
    return CBotTypResult( CBotTypInt );
}

// donne une chaine en majuscule
// exécution

BOOL rStrUpper( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // mais pas de 2e paramètre
    if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return TRUE; }


    s.MakeUpper();

    // la met la valeur sur la pile
    pResult->SetValString( s );
    return TRUE;
}

// donne une chaine en minuscules
// exécution

BOOL rStrLower( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return TRUE; }

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString ) { ex = TX_BADSTRING ; return TRUE; }

    // recupére le contenu de la string
    CBotString  s = pVar->GivValString();

    // mais pas de 2e paramètre
    if ( pVar->GivNext() != NULL ){ ex = TX_OVERPARAM ; return TRUE; }


    s.MakeLower();

    // la met la valeur sur la pile
    pResult->SetValString( s );
    return TRUE;
}

// string xxx ( string )
// compilation

CBotTypResult cStrStr( CBotVar* &pVar, void* pUser )
{
    // il faut un paramètre
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // qui doit être une string
    if ( pVar->GivType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // pas de 2e paramètre
    if ( pVar->GivNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // le résultat final est une string
    return CBotTypResult( CBotTypString );
}


void InitStringFunctions()
{
    CBotProgram::AddFunction("strlen",   rStrLen,   cIntStr );
    CBotProgram::AddFunction("strleft",  rStrLeft,  cStrStrInt );
    CBotProgram::AddFunction("strright", rStrRight, cStrStrInt );
    CBotProgram::AddFunction("strmid",   rStrMid,   cStrStrIntInt );

    CBotProgram::AddFunction("strval",   rStrVal,   cFloatStr );
    CBotProgram::AddFunction("strfind",  rStrFind,  cIntStrStr );

    CBotProgram::AddFunction("strupper", rStrUpper, cStrStr );
    CBotProgram::AddFunction("strlower", rStrLower, cStrStr );
}
