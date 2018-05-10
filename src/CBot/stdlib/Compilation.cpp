/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/stdlib/Compilation.h"

#include "CBot/CBot.h"

namespace CBot
{

// Compiling a procedure without any parameters.

CBotTypResult cNull(CBotVar* &var, void* user)
{
    if ( var != nullptr )  return CBotErrOverParam;
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single real number.

CBotTypResult cOneFloat(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with two real numbers.

CBotTypResult cTwoFloat(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}


// Compiling a procedure with a single string.

CBotTypResult cString(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
         var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single string, returning string.

CBotTypResult cStringString(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
         var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypString);
}

// compilation of instruction with one int returning int

CBotTypResult cOneInt(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypInt);
}

// compilation of instruction with one int returning boolean

CBotTypResult cOneIntReturnBool(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}



CBotTypResult cStrStr(CBotVar*& var, void* user)
{
    // it takes a parameter
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // no second parameter
    if ( var->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the end result is a string
    return CBotTypResult( CBotTypString );
}

CBotTypResult cIntStrStr(CBotVar*& var, void* user)
{
    // it takes a parameter
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // it takes a second parameter
    var = var->GetNext();
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // no third parameter
    if ( var->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the end result is a number
    return CBotTypResult( CBotTypInt );
}

CBotTypResult cFloatStr(CBotVar*& var, void* user)
{
    // it takes a parameter
    if ( var == nullptr ) return CBotTypResult(CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // no second parameter
    if ( var->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the end result is a number
    return CBotTypResult( CBotTypFloat );
}

CBotTypResult cStrStrIntInt(CBotVar*& var, void* user)
{
    // it takes a parameter
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // it takes a second parameter
    var = var->GetNext();
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // which must be a number
    if ( var->GetType() > CBotTypDouble )
        return CBotTypResult( CBotErrBadNum );

    // third parameter optional
    if ( var->GetNext() != nullptr )
    {

        var = var->GetNext();
        // which must be a number
        if ( var->GetType() > CBotTypDouble )
            return CBotTypResult( CBotErrBadNum );

        // no fourth parameter
        if ( var->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );
    }

    // the end result is a string
    return CBotTypResult( CBotTypString );
}

CBotTypResult cStrStrInt(CBotVar*& var, void* user)
{
    // it takes a parameter
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // it takes a second parameter
    var = var->GetNext();
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // which must be a number
    if ( var->GetType() > CBotTypDouble )
        return CBotTypResult( CBotErrBadNum );

    // no third parameter
    if ( var->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the end result is a string
    return CBotTypResult( CBotTypString );
}

CBotTypResult cIntStr(CBotVar*& var, void* user)
{
    // it takes a parameter
    if ( var == nullptr ) return CBotTypResult( CBotErrLowParam );

    // to be a string
    if ( var->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadParam );

    // no second parameter
    if ( var->GetNext() != nullptr ) return CBotTypResult( CBotErrOverParam );

    // the end result is an integer
    return CBotTypResult( CBotTypInt );
}

} // namespace CBot
