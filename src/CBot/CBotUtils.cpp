/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
bool WriteWord(FILE* pf, unsigned short w)
{
    std::size_t  lg;

    lg = fwrite(&w, sizeof( unsigned short ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
bool WriteString(FILE* pf, std::string s)
{
    std::size_t  lg1, lg2;

    lg1 = s.size();
    if (!WriteWord(pf, lg1)) return false;

    lg2 = fwrite(s.c_str(), 1, lg1, pf );
    return (lg1 == lg2);
}

////////////////////////////////////////////////////////////////////////////////
bool WriteFloat(FILE* pf, float w)
{
    std::size_t  lg;

    lg = fwrite(&w, sizeof( float ), 1, pf );

    return (lg == 1);
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
    return num;
}

////////////////////////////////////////////////////////////////////////////////
float GetNumFloat(const std::string& str)
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
    return static_cast<float>(num);
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

std::string CodePointToUTF8(unsigned int val)
{
    std::string s = "";

    if (val < 0xD800 || (0xDFFF < val && val < 0x110000))
    {
        if (val < 0x80)
        {
            s.push_back(val);
        }
        else if (val < 0x800)
        {
            s.push_back(0xC0 + (val >> 6));
            s.push_back(0x80 + (val & 0x3F));
        }
        else if (val < 0x10000)
        {
            s.push_back(0xE0 + (val >> 12));
            s.push_back(0x80 + ((val >> 6) & 0x3F));
            s.push_back(0x80 + (val & 0x3F));
        }
        else
        {
            s.push_back(0xF0 + (val >> 18));
            s.push_back(0x80 + ((val >> 12) & 0x3F));
            s.push_back(0x80 + ((val >> 6) & 0x3F));
            s.push_back(0x80 + (val & 0x3F));
        }
    }

    return s;
}

} // namespace CBot
