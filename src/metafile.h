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

// metafile.h

#ifndef _METAFILE_H_
#define _METAFILE_H_


#include <stdio.h>


#define METAMAX     5

typedef struct
{
    char            name[14];   // file name (8.3 max)
    int         start;      // position from the beginning of the metafile
    int         len;        // length of the file
}
MetaHeader;

typedef struct
{
    char        name[50];   // name of the metafile
    FILE*       stream;     // channel
    int     total;      // number of files
    MetaHeader* headers;    // headers of files
}
MetaFile;



class CMetaFile
{
public:
    CMetaFile();
    ~CMetaFile();

    BOOL    IsExist(char *metaname, char *filename);
    int     Open(char *metaname, char *filename);
    int     RetLength();
    int     Seek(int offset);
    int     Read(void *buffer, int size);
    int     GetByte();
    int     GetWord();
    int     Close();
    int     MetaClose();

protected:
    int     MetaOpen(char *metaname);
    int     MetaSearch(char *metaname);

protected:
    MetaFile    m_list[METAMAX];        // metafile open
    BOOL        m_bOpen;            // open file
    BOOL        m_bMeta;            // metafile open
    FILE*       m_stream;           // channel
    int     m_start;            // position from the beginning
    int     m_pos;              // current position
    int     m_len;              // length of the file
};


#endif //_METAFILE_H_
