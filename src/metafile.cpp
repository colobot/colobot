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

// metafile.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>

#include "language.h"
#include "metafile.h"




#if _FULL | _NET
static unsigned char table_codec[23] =
{
    0x85, 0x91, 0x73, 0xcf, 0xa2, 0xbb, 0xf4, 0x77,
    0x58, 0x39, 0x37, 0xfd, 0x2a, 0xcc, 0x5f, 0x55,
    0x96, 0x90, 0x07, 0xcd, 0x11, 0x88, 0x21,
};

void Codec(void* buffer, int len, int start)
{
    unsigned char *b = (unsigned char*)buffer;
    int     i;

    for ( i=0 ; i<len ; i++ )
    {
        b[i] ^= table_codec[(start++)%23];
    }
}
#endif

#if _SCHOOL
#if _CEEBOTDEMO
static unsigned char table_codec[136] =
{
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76,
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76,
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76,
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76,
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76,
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
    0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
};

void Codec(void* buffer, int len, int start)
{
    unsigned char *b = (unsigned char*)buffer;
    int     i;

    for ( i=0 ; i<len ; i++ )
    {
        b[i] ^= table_codec[(start++)%136];
    }
}
#else
static unsigned char table_codec[29] =
{
    0x72, 0x91, 0x37, 0xdf, 0xa1, 0xcc, 0xf5, 0x67,
    0x53, 0x40, 0xd3, 0xed, 0x3a, 0xbb, 0x5e, 0x43,
    0x67, 0x9a, 0x0c, 0xed, 0x33, 0x77, 0x2f, 0xf2,
    0xe3, 0x42, 0x11, 0x5e, 0xc2,
};

void Codec(void* buffer, int len, int start)
{
    unsigned char *b = (unsigned char*)buffer;
    int     i;

    for ( i=0 ; i<len ; i++ )
    {
        b[i] ^= table_codec[(start++)%29];
    }
}
#endif
#endif

#if _DEMO
static unsigned char table_codec[27] =
{
    0x85, 0x91, 0x77, 0xcf, 0xa3, 0xbb, 0xf4, 0x77,
    0x58, 0x39, 0x37, 0xfd, 0x2a, 0xcc, 0x7f, 0x55,
    0x96, 0x80, 0x07, 0xcd, 0x11, 0x88, 0x21, 0x44,
    0x17, 0xee, 0xf0,
};

void Codec(void* buffer, int len, int start)
{
    unsigned char *b = (unsigned char*)buffer;
    int     i;

    for ( i=0 ; i<len ; i++ )
    {
        b[i] ^= table_codec[(start++)%27];
    }
}
#endif



// Object's constructor.

CMetaFile::CMetaFile()
{
    int     i;

    for ( i=0 ; i<METAMAX ; i++ )
    {
        m_list[i].stream  = 0;
        m_list[i].headers = 0;
    }

    m_bMeta  = FALSE;
    m_bOpen  = FALSE;
    m_start  = 0;
    m_pos    = 0;
    m_len    = 0;
    m_stream = 0;
}

// Object's destructor.

CMetaFile::~CMetaFile()
{
    MetaClose();
}


// Test if a file exists.

BOOL CMetaFile::IsExist(char *metaname, char *filename)
{
    FILE*   file;
    int     index, i;

    if ( metaname[0] == 0 )
    {
        file = fopen(filename, "rb");
        if ( file == NULL )  return FALSE;
        fclose(file);
        return TRUE;
    }
    else
    {
        index = MetaOpen(metaname);
        if ( index == -1 )  return 1;

        for ( i=0 ; i<m_list[index].total ; i++ )
        {
            if ( strcmp(m_list[index].headers[i].name, filename) == 0 )
            {
                return TRUE;
            }
        }
        return FALSE;
    }
}

// Opens a file. If metaname is empty, it normally opens a file.

int CMetaFile::Open(char *metaname, char *filename)
{
    int     index, i;

    if ( m_bOpen )  // file already open?
    {
        Close();
    }

    if ( metaname[0] == 0 )
    {
        m_stream = fopen(filename, "rb");
        if ( m_stream == 0 )  return 1;
        m_bOpen = TRUE;
        m_bMeta = FALSE;
        return 0;
    }
    else
    {
        index = MetaOpen(metaname);
        if ( index == -1 )  return 1;

        for ( i=0 ; i<m_list[index].total ; i++ )
        {
            if ( strcmp(m_list[index].headers[i].name, filename) == 0 )
            {
                m_stream = m_list[index].stream;
                m_start  = m_list[index].headers[i].start;
                m_len    = m_list[index].headers[i].len;
                m_bOpen = TRUE;
                m_bMeta = TRUE;
                Seek(0);
                return 0;
            }
        }
        return 1;
    }
}

// Returns the length of a file.

int CMetaFile::RetLength()
{
    int     len;

    if ( !m_bOpen )  return 0;

    if ( m_bMeta )
    {
        len = m_len;
    }
    else
    {
        fseek(m_stream, 0, SEEK_END);
        len = ftell(m_stream);
        fseek(m_stream, 0, SEEK_SET);
    }
    return len;
}

// Positioning in the file, relative to the beginning.

int CMetaFile::Seek(int offset)
{
    if ( !m_bOpen )  return 1;

    if ( m_bMeta )
    {
        m_pos = m_start+offset;
        return fseek(m_stream, m_start+offset, SEEK_SET);
    }
    else
    {
        return fseek(m_stream, offset, SEEK_SET);
    }
}

// Reads bytes number.

int CMetaFile::Read(void *buffer, int size)
{
    int     err;

    if ( !m_bOpen )  return 1;

    if ( m_bMeta )
    {
        err = fread(buffer, size, 1, m_stream);
        Codec(buffer, size, m_pos);
        m_pos += size;
        return err;
    }
    else
    {
        return fread(buffer, size, 1, m_stream);
    }
}

// Reads a byte.

int CMetaFile::GetByte()
{
    BYTE    b;

    if ( !m_bOpen )  return 1;

    b = getc(m_stream);
    if ( m_bMeta )
    {
        Codec(&b, 1, m_pos);
        m_pos += 1;
    }
    return b;
}

// Reads 2 bytes.

int CMetaFile::GetWord()
{
    WORD    w;

    if ( !m_bOpen )  return 1;

    w = getc(m_stream);
    if ( m_bMeta )
    {
        Codec(&w, 2, m_pos);
        m_pos += 2;
    }
    return w;
}

// Closes the file.

int CMetaFile::Close()
{
    if ( !m_bOpen )  return 1;

    if ( !m_bMeta )
    {
        fclose(m_stream);
    }
    m_bOpen = FALSE;
    m_stream = 0;

    return 0;
}


// Opens a metafile. Returns the index or -1.

int CMetaFile::MetaOpen(char *metaname)
{
    int     i, j, offset;

    i = MetaSearch(metaname);
    if ( i != -1 )  return i;

    for ( i=0 ; i<METAMAX ; i++ )
    {
        if ( m_list[i].stream == 0 )
        {
            m_list[i].stream = fopen(metaname, "rb");
            if ( m_list[i].stream == 0 )  return -1;

            strcpy(m_list[i].name, metaname);  // memorized the name

            fread(&m_list[i].total, sizeof(int), 1, m_list[i].stream);
            m_list[i].headers = (MetaHeader*)malloc(sizeof(MetaHeader)*m_list[i].total);

            offset = 4;
            for ( j=0 ; j<m_list[i].total ; j++ )
            {
                fread(&m_list[i].headers[j], sizeof(MetaHeader), 1, m_list[i].stream);
                Codec(&m_list[i].headers[j], sizeof(MetaHeader), offset);
                offset += sizeof(MetaHeader);
            }
            return i;
        }
    }

    return -1;
}

// Seeks if the metafile is already open. Returns the index or -1.

int CMetaFile::MetaSearch(char *metaname)
{
    int     i;

    for ( i=0 ; i<METAMAX ; i++ )
    {
        if ( m_list[i].stream != 0 )
        {
            if ( strcmp(m_list[i].name, metaname) == 0 )  return i;
        }
    }

    return -1;
}

// Closes all metafiles.

int CMetaFile::MetaClose()
{
    int     i;

    if ( m_stream != 0 )
    {
        fclose(m_stream);
        m_stream = 0;
    }

    for ( i=0 ; i<METAMAX ; i++ )
    {
        if ( m_list[i].stream != 0 )
        {
            free(m_list[i].headers);
            m_list[i].headers = 0;

            fclose(m_list[i].stream);
            m_list[i].stream = 0;
        }
    }

    return 0;
}



