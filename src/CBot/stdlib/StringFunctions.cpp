/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/stdlib/stdlib.h"

#include "CBot/CBot.h"

#include "CBot/CBotUtils.h"
#include "core/stringutils.h"

namespace CBot
{
namespace
{

////////////////////////////////////////////////////////////////////////////////
bool rStrLen( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // no second parameter
    if ( pVar->GetNext() != nullptr ) { ex = CBotErrOverParam ; return true; }

    // get the contents of the string
    std::string  s = pVar->GetValString();

    // puts the length of the stack
    pResult->SetValInt( s.length() );
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrLeft( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble ) { ex = CBotErrBadNum ; return true; }

    // retrieves this number
    int n = pVar->GetValInt();

    if (n > static_cast<int>(s.length())) n = s.length();
    if (n < 0) n = 0;

    // no third parameter
    if ( pVar->GetNext() != nullptr ) { ex = CBotErrOverParam ; return true; }

    // takes the interesting part
    s = s.substr(0, n);

    // puts on the stack
    pResult->SetValString( s );
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrRight( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble ) { ex = CBotErrBadNum ; return true; }

    // retrieves this number
    int n = pVar->GetValInt();

    if (n > static_cast<int>(s.length())) n = s.length();
    if (n < 0) n = 0;

    // no third parameter
    if ( pVar->GetNext() != nullptr ) { ex = CBotErrOverParam ; return true; }

    // takes the interesting part
    s = s.substr(s.length()-n, std::string::npos);

    // puts on the stack
    pResult->SetValString( s );
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrMid( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // which must be a number
    if ( pVar->GetType() > CBotTypDouble ) { ex = CBotErrBadNum ; return true; }

    // retrieves this number
    int n = pVar->GetValInt();

    if (n > static_cast<int>(s.length())) n = s.length();
    if (n < 0) n = 0;

    // third parameter optional
    if ( pVar->GetNext() != nullptr )
    {
        pVar = pVar->GetNext();

        // which must be a number
        if ( pVar->GetType() > CBotTypDouble ) { ex = CBotErrBadNum ; return true; }

        // retrieves this number
        int l = pVar->GetValInt();

        if (l > static_cast<int>(s.length())) l = s.length();
        if (l < 0) l = 0;

        // but no fourth parameter
        if ( pVar->GetNext() != nullptr ){ ex = CBotErrOverParam ; return true; }

        // takes the interesting part
        s = s.substr(n, l);
    }
    else
    {
        // takes the interesting part
        s = s.substr(n);
    }

    // puts on the stack
    pResult->SetValString( s );
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrVal( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string  s = pVar->GetValString();

    // but no second parameter
    if ( pVar->GetNext() != nullptr ){ ex = CBotErrOverParam ; return true; }

    float val = GetNumFloat(s);

    // puts the value on the stack
    pResult->SetValFloat( val );
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrFind( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string  s = pVar->GetValString();

    // it takes a second parameter
    pVar = pVar->GetNext();
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // retrieves this number
    std::string s2 = pVar->GetValString();

    // no third parameter
    if ( pVar->GetNext() != nullptr ) { ex = CBotErrOverParam ; return true; }

    // puts the result on the stack
    std::size_t res = s.find(s2);
    if (res != std::string::npos)
    {
        pResult->SetValInt(res);
    }
    else
    {
        pResult->SetValInt(-1);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrUpper( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string s = pVar->GetValString();

    // but no second parameter
    if ( pVar->GetNext() != nullptr ){ ex = CBotErrOverParam ; return true; }

    StrUtils::ToUpper(s);

    // puts the value on the stack
    pResult->SetValString( s );
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool rStrLower( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    // it takes a parameter
    if ( pVar == nullptr ) { ex = CBotErrLowParam ; return true; }

    // to be a string
    if ( pVar->GetType() != CBotTypString ) { ex = CBotErrBadString ; return true; }

    // get the contents of the string
    std::string s = pVar->GetValString();

    // but no second parameter
    if ( pVar->GetNext() != nullptr ){ ex = CBotErrOverParam ; return true; }

    StrUtils::ToLower(s);

    // puts the value on the stack
    pResult->SetValString( s );
    return true;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
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

} // namespace CBot
