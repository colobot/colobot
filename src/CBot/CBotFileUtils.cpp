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

#include "CBot/CBotFileUtils.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotEnums.h"
#include "CBot/CBotUtils.h"

namespace CBot
{


// file management

// necessary because it is not possible to do the fopen in the main program
// fwrite and fread in a dll or using the FILE * returned.

////////////////////////////////////////////////////////////////////////////////
FILE* fOpen(const char* name, const char* mode)
{
    return fopen(name, mode);
}

////////////////////////////////////////////////////////////////////////////////
int fClose(FILE* filehandle)
{
    return fclose(filehandle);
}

////////////////////////////////////////////////////////////////////////////////
std::size_t fWrite(const void *buffer,
                   std::size_t elemsize,
                   std::size_t length,
                   FILE* filehandle)
{
    return fwrite(buffer, elemsize, length, filehandle);
}

////////////////////////////////////////////////////////////////////////////////
std::size_t fRead(void *buffer,
                  std::size_t elemsize,
                  std::size_t length,
                  FILE* filehandle)
{
    return fread(buffer, elemsize, length, filehandle);
}


////////////////////////////////////////////////////////////////////////////////
bool ReadWord(FILE* pf, unsigned short& w)
{
    std::size_t  lg;

    lg = fread(&w, sizeof( unsigned short ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
bool ReadFloat(FILE* pf, float& w)
{
    std::size_t  lg;

    lg = fread(&w, sizeof( float ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
bool WriteLong(FILE* pf, long w)
{
    std::size_t  lg;

    lg = fwrite(&w, sizeof( long ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
bool ReadLong(FILE* pf, long& w)
{
    std::size_t  lg;

    lg = fread(&w, sizeof( long ), 1, pf );

    return (lg == 1);
}

////////////////////////////////////////////////////////////////////////////////
bool ReadString(FILE* pf, std::string& s)
{
    unsigned short  w;
    char    buf[1000];
    std::size_t  lg1, lg2;

    if (!ReadWord(pf, w)) return false;
    lg1 = w;
    lg2 = fread(buf, 1, lg1, pf );
    buf[lg2] = 0;

    s = buf;
    return (lg1 == lg2);
}

////////////////////////////////////////////////////////////////////////////////
bool WriteType(FILE* pf, const CBotTypResult &type)
{
    int typ = type.GetType();
    if ( typ == CBotTypIntrinsic ) typ = CBotTypClass;
    if ( !WriteWord(pf, typ) ) return false;
    if ( typ == CBotTypClass )
    {
        CBotClass* p = type.GetClass();
        if ( !WriteString(pf, p->GetName()) ) return false;
    }
    if ( type.Eq( CBotTypArrayBody ) ||
         type.Eq( CBotTypArrayPointer ) )
    {
        if ( !WriteWord(pf, type.GetLimite()) ) return false;
        if ( !WriteType(pf, type.GetTypElem()) ) return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ReadType(FILE* pf, CBotTypResult &type)
{
    unsigned short  w, ww;
    if ( !ReadWord(pf, w) ) return false;
    type.SetType(w);

    if ( type.Eq( CBotTypIntrinsic ) )
    {
        type = CBotTypResult( w, "point" );
    }

    if ( type.Eq( CBotTypClass ) )
    {
        std::string  s;
        if ( !ReadString(pf, s) ) return false;
        type = CBotTypResult( w, s );
    }

    if ( type.Eq( CBotTypArrayPointer ) ||
         type.Eq( CBotTypArrayBody ) )
    {
        CBotTypResult   r;
        if ( !ReadWord(pf, ww) ) return false;
        if ( !ReadType(pf, r) ) return false;
        type = CBotTypResult( w, r );
        type.SetLimite(static_cast<short>(ww));
    }
    return true;
}

} // namespace CBot
