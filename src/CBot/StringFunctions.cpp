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

// definition of string functions


// gives the length of a chain
// execution

bool rStrLen( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // no second parameter
    if ( pVar->GetNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // puts the length of the stack
    pResult->SetValInt( s.GetLength() );
    return true;
}

// int xxx ( string )
// compilation

CBotTypResult cIntStr( CBotVar* &pVar, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADPARAM );

    // no second parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // the end result is an integer
    return CBotTypResult( CBotTypInt );
}


// gives the left side of a chain
// execution

bool rStrLeft( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

    // retrieves this number
    int n = pVar->GetValInt();

    // no third parameter
    if ( pVar->GetNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

    // takes the interesting part
    s = s.Left( n );

    // puts on the stack
    pResult->SetValString( s );
    return true;
}

// string xxx ( string, int )
// compilation

CBotTypResult cStrStrInt( CBotVar* &pVar, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble )
                        return CBotTypResult( TX_BADNUM );

    // no third parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // the end result is a string
    return CBotTypResult( CBotTypString );
}

// gives the right of a string
// execution

bool rStrRight( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

    // retrieves this number
    int n = pVar->GetValInt();

    // no third parameter
    if ( pVar->GetNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

    // takes the interesting part
    s = s.Right( n );

    // puts on the stack
    pResult->SetValString( s );
    return true;
}

// gives the central part of a chain
// execution

bool rStrMid( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

    // retrieves this number
    int n = pVar->GetValInt();

    // third parameter optional
    if ( pVar->GetNext() != NULL )
    {
        pVar = pVar->GetNext();

        // which must be a number
        if ( pVar->GetType() > CBotTypDouble ) { ex = TX_BADNUM ; return true; }

        // retrieves this number
        int l = pVar->GetValInt();

        // but no fourth parameter
        if ( pVar->GetNext() != NULL ){ ex = TX_OVERPARAM ; return true; }

        // takes the interesting part
        s = s.Mid( n, l );
    }
    else
    {
        // takes the interesting part
        s = s.Mid( n );
    }

    // puts on the stack
    pResult->SetValString( s );
    return true;
}

// gives the central part of a chain
// compilation

CBotTypResult cStrStrIntInt( CBotVar* &pVar, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble )
                        return CBotTypResult( TX_BADNUM );

    // third parameter optional
    if ( pVar->GetNext() != NULL )
    {

        pVar = pVar->GetNext();
        // which must be a number
        if ( pVar->GetType() > CBotTypDouble )
                            return CBotTypResult( TX_BADNUM );

        // no fourth parameter
        if ( pVar->GetNext() != NULL ) return CBotTypResult( TX_OVERPARAM );
    }

    // the end result is a string
    return CBotTypResult( CBotTypString );
}


// gives the number stored in a string
// execution

bool rStrVal( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // but no second parameter
    if ( pVar->GetNext() != NULL ){ ex = TX_OVERPARAM ; return true; }

    float val = GetNumFloat(s);

    // puts the value on the stack
    pResult->SetValFloat( val );
    return true;
}

// float xxx ( string )
// compilation

CBotTypResult cFloatStr( CBotVar* &pVar, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // no second parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // the end result is a number
    return CBotTypResult( CBotTypFloat );
}


// find string in other
// exécution

bool rStrFind( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // retrieves this number
    CBotString s2 = pVar->GetValString();

    // no third parameter
    if ( pVar->GetNext() != NULL ) { ex = TX_OVERPARAM ; return true; }

    // puts the result on the stack
    int res = s.Find(s2);
    pResult->SetValInt( res );
    if ( res < 0 ) pResult->SetInit( IS_NAN );
    return true;
}

// int xxx ( string, string )
// compilation

CBotTypResult cIntStrStr( CBotVar* &pVar, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // no third parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // the end result is a number
    return CBotTypResult( CBotTypInt );
}

// gives a string to uppercase
// exécution

bool rStrUpper( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // but no second parameter
    if ( pVar->GetNext() != NULL ){ ex = TX_OVERPARAM ; return true; }


    s.MakeUpper();

    // puts the value on the stack
    pResult->SetValString( s );
    return true;
}

// gives a string to lowercase
// exécution

bool rStrLower( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) { ex = TX_LOWPARAM ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = TX_BADSTRING ; return true; }

    // get the contents of the string
    CBotString  s = pVar->GetValString();

    // but no second parameter
    if ( pVar->GetNext() != NULL ){ ex = TX_OVERPARAM ; return true; }


    s.MakeLower();

    // puts the value on the stack
    pResult->SetValString( s );
    return true;
}

// string xxx ( string )
// compilation

CBotTypResult cStrStr( CBotVar* &pVar, void* pUser )
{
    // it takes a parameter
    if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );

    // to be a string
    if ( pVar->GetType() != CBotTypString )
                        return CBotTypResult( TX_BADSTRING );

    // no second parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( TX_OVERPARAM );

    // the end result is a string
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

