/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotUtils.h"

#include "CBot/CBotToken.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <cstring>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotVar* MakeListVars(CBotVar** ppVars, bool bSetVal)
{
    int     i = 0;
    CBotVar*    pVar = nullptr;

    while( true )
    {
//        ppVars[i];
        if ( ppVars[i] == nullptr ) break;

        CBotVar*    pp = CBotVar::Create(ppVars[i]);
        if (bSetVal) pp->Copy(ppVars[i]);
        else
            if ( ppVars[i]->GetType() == CBotTypPointer )
                pp->SetClass( ppVars[i]->GetClass());
// copy the pointer according to indirections
        if (pVar == nullptr) pVar = pp;
        else pVar->AddNext(pp);
        i++;
    }
    return pVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult TypeParam(CBotToken* &p, CBotCStack* pile)
{
    CBotClass*  pClass = nullptr;

    switch (p->GetType())
    {
    case ID_BYTE:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypByte ));
    case ID_SHORT:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypShort ));
    case ID_CHAR:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypChar ));
    case ID_INT:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypInt ));
    case ID_LONG:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypLong ));
    case ID_FLOAT:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypFloat ));
    case ID_DOUBLE:
        p = p->GetNext();
        return ArrayType(p, pile, CBotTypResult( CBotTypDouble ));
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
        if ( pClass != nullptr)
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

////////////////////////////////////////////////////////////////////////////////
CBotTypResult ArrayType(CBotToken* &p, CBotCStack* pile, CBotTypResult type)
{
    while ( IsOfType( p, ID_OPBRK ) )
    {
        if ( !IsOfType( p, ID_CLBRK ) )
        {
            pile->SetError(CBotErrCloseIndex, p->GetStart());
            return CBotTypResult( -1 );
        }
        type = CBotTypResult( CBotTypArrayPointer, type );
    }
    return type;
}

////////////////////////////////////////////////////////////////////////////////
long GetNumInt(const std::string& str)
{
    const char* p = str.c_str();
    long    num = 0;
    while (*p >= '0' && *p <= '9')
    {
        num = num * 10 + *p - '0';
        p++;
    }
    if (*p == 'x' || *p == 'X')
    {
        while (*++p != 0)
        {
            if (*p >= '0' && *p <= '9')
            {
                num = num * 16 + *p - '0';
                continue;
            }
            if (*p >= 'A' && *p <= 'F')
            {
                num = num * 16 + *p - 'A' + 10;
                continue;
            }
            if (*p >= 'a' && *p <= 'f')
            {
                num = num * 16 + *p - 'a' + 10;
                continue;
            }
            break;
        }
    }
    else if (*p == 'b')
    {
        while (*++p != 0)
        {
            if (*p == '0' || *p == '1')
            {
                num = (num << 1) + *p - '0';
                continue;
            }
            break;
        }
    }
    return num;
}

////////////////////////////////////////////////////////////////////////////////
double GetNumFloat(const std::string& str)
{
    const char* p = str.c_str();
    double    num = 0;
    double    div    = 10;
    bool    bNeg = false;

    if (*p == '-')
    {
        bNeg = true;
        p++;
    }
    while (*p >= '0' && *p <= '9')
    {
        num = num * 10. + (*p - '0');
        p++;
    }

    if (*p == '.')
    {
        p++;
        while (*p >= '0' && *p <= '9')
        {
            num = num + (*p - '0') / div;
            div = div * 10;
            p++;
        }
    }

    int    exp = 0;
    if (*p == 'e' || *p == 'E')
    {
        char neg = 0;
        p++;
        if (*p == '-' || *p == '+') neg = *p++;

        while (*p >= '0' && *p <= '9')
        {
            exp = exp * 10 + (*p - '0');
            p++;
        }
        if (neg == '-') exp = -exp;
    }

    while (exp > 0)
    {
        num *= 10.0;
        exp--;
    }

    while (exp < 0)
    {
        num /= 10.0;
        exp++;
    }

    if (bNeg) num = -num;
    return num;
}

bool CharInList(const char c, const char* list)
{
    int i = 0;

    while (list[i] != 0)
    {
        if (c == list[i++]) return true;
    }

    return false;
}

} // namespace CBot
