/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

// Modules inlcude
#include "CBotUtils.h"

#include "CBotToken.h"
#include "CBotClass.h"
#include "CBotStack.h"
#include "CBotCStack.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include


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
            pile->SetError(TX_CLBRK, p->GetStart());
            return CBotTypResult( -1 );
        }
        type = CBotTypResult( CBotTypArrayPointer, type );
    }
    return type;
}

////////////////////////////////////////////////////////////////////////////////
bool WriteWord(FILE* pf, unsigned short w)
{
    size_t  lg;

    lg = fwrite(&w, sizeof( unsigned short ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
bool WriteString(FILE* pf, CBotString s)
{
    size_t  lg1, lg2;

    lg1 = s.GetLength();
    if (!WriteWord(pf, lg1)) return false;

    lg2 = fwrite(s, 1, lg1, pf );
    return (lg1 == lg2);
}

////////////////////////////////////////////////////////////////////////////////
bool WriteFloat(FILE* pf, float w)
{
    size_t  lg;

    lg = fwrite(&w, sizeof( float ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
void ConstructElement(CBotString* pNewData)
{
    memset(pNewData, 0, sizeof(CBotString));
}

////////////////////////////////////////////////////////////////////////////////
void DestructElement(CBotString* pOldData)
{
    pOldData->~CBotString();
}

////////////////////////////////////////////////////////////////////////////////
void CopyElement(CBotString* pSrc, CBotString* pDest)
{
    *pSrc = *pDest;
}

////////////////////////////////////////////////////////////////////////////////
void ConstructElements(CBotString* pNewData, int nCount)
{
    while (nCount--)
    {
        ConstructElement(pNewData);
        pNewData++;
    }
}

////////////////////////////////////////////////////////////////////////////////
void DestructElements(CBotString* pOldData, int nCount)
{
    while (nCount--)
    {
        DestructElement(pOldData);
        pOldData++;
    }
}
