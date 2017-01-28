// cryptfile.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>

#include "language.h"
#include "cryptfile.h"



#define CRYPT_MARK	0xff	// 1er byte d'un fichier crypté
#define LENGTH_MAX	50000	// utile uniquement en écriture



#if _DEMO | _SE

static unsigned char table_codec[77] =
{
	0x33, 0x3e, 0x0e, 0x48, 0xd0, 0xe1, 0x13, 0x9e, 0x42, 0xf3,
	0xcc, 0x45, 0x78, 0x99, 0x03, 0x40, 0x79, 0xac, 0xfe, 0x54,
	0x09, 0x49, 0x50, 0xf3, 0x87, 0x17, 0xa5, 0x4a, 0xaa, 0xf2,
	0xdd, 0xcc, 0xde, 0x50, 0x12, 0xd2, 0xe8, 0x57, 0x1c, 0xdf,
	0x51, 0x37, 0x41, 0xde, 0xed, 0x37, 0x42, 0xca, 0xac, 0xaa,
	0x85, 0x91, 0x73, 0xcf, 0xa2, 0x33, 0xbb, 0xf4, 0xaa, 0x77,
	0x74, 0x61, 0x78, 0x8b, 0xb1, 0x78, 0x75, 0x4c, 0xdd, 0x64,
	0x96, 0x90, 0x71, 0xdc, 0x11, 0x88, 0x21,
};

void Crypt(void* buffer, int len)
{
	unsigned char *b = (unsigned char*)buffer;
	unsigned char last, current;
	int		i;

	last = 0x17;
	for ( i=0 ; i<len ; i++ )
	{
		current = b[i]^last;
		last = b[i];
		b[i] = current^table_codec[i%77];
	}
}

void Decrypt(void* buffer, int len)
{
	unsigned char *b = (unsigned char*)buffer;
	unsigned char last;
	int		i;

	last = 0x17;
	for ( i=0 ; i<len ; i++ )
	{
		b[i] = b[i]^table_codec[i%77]^last;
		last = b[i];
	}
}

unsigned short Checksum(void* buffer, int len)
{
	unsigned char *b = (unsigned char*)buffer;
	int		i;
	unsigned short	checkum = 0;

	for ( i=0 ; i<len ; i++ )
	{
		switch(i%4)
		{
			case 0:
				checkum += (b[i]<<1)|(b[i]&0x80)?1:0;
				break;
			case 1:
				checkum += b[i]^0x63;
				break;
			case 2:
				checkum += b[i]^0xa5;
				break;
			case 3:
				checkum += b[i]-11;
				break;
		}
	}

	return checkum;
}

#else

static unsigned char table_codec[87] =
{
	0x51, 0x37, 0x41, 0xde, 0xed, 0x37, 0x42, 0xca, 0xac, 0xaa,
	0x39, 0xab, 0x58, 0x37, 0x12, 0xfd, 0x2a, 0xcc, 0x5f, 0x55,
	0x85, 0x91, 0x73, 0xcf, 0xa2, 0x33, 0xbb, 0xf4, 0xaa, 0x77,
	0xcc, 0x45, 0x78, 0x99, 0x03, 0x40, 0x79, 0xac, 0xfe, 0x50,
	0xcc, 0xdd, 0xde, 0x50, 0x12, 0xd2, 0xe8, 0x57, 0x1c, 0xdf,
	0x33, 0x2f, 0x0e, 0x48, 0xd0, 0xe1, 0x13, 0x9e, 0x42, 0xf3,
	0x74, 0x61, 0x78, 0x8b, 0xa1, 0x78, 0x77, 0x3c, 0xdd, 0x64,
	0x09, 0x49, 0x53, 0xf3, 0x82, 0x17, 0xa5, 0x49, 0xaa, 0xf1,
	0x96, 0x90, 0x07, 0xcd, 0x11, 0x88, 0x21,
};

void Crypt(void* buffer, int len)
{
	unsigned char *b = (unsigned char*)buffer;
	unsigned char last, current;
	int		i;

	last = 0x53;
	for ( i=0 ; i<len ; i++ )
	{
		current = b[i]^last;
		last = b[i];
		b[i] = current^table_codec[i%87];
	}
}

void Decrypt(void* buffer, int len)
{
	unsigned char *b = (unsigned char*)buffer;
	unsigned char last;
	int		i;

	last = 0x53;
	for ( i=0 ; i<len ; i++ )
	{
		b[i] = b[i]^table_codec[i%87]^last;
		last = b[i];
	}
}

unsigned short Checksum(void* buffer, int len)
{
	unsigned char *b = (unsigned char*)buffer;
	int		i;
	unsigned short	checkum = 0;

	for ( i=0 ; i<len ; i++ )
	{
		switch(i%4)
		{
			case 0:
				checkum += b[i]^0x35;
				break;
			case 1:
				checkum += (b[i]<<1)|(b[i]&0x80)?1:0;
				break;
			case 2:
				checkum += b[i]^0x80;
				break;
			case 3:
				checkum += b[i]-7;
				break;
		}
	}

	return checkum;
}

#endif




// Constructeur de l'objet.

CCryptFile::CCryptFile()
{
	m_filename[0] = 0;
	m_mode = 0;
	m_bCrypt = TRUE;
	m_bWriteCrypt = TRUE;
	m_data = 0;
	m_length = 0;
	m_seek = 0;
}

// Destructeur de l'objet.

CCryptFile::~CCryptFile()
{
	free(m_data);
}


// Ouvre un fichier.
// En lecture, le fichier peut être crypté ou non.
// En écriture, il est crypté ou non selon bWriteCrypt.

BOOL CCryptFile::Open(char *filename, char *mode, BOOL bWriteCrypt)
{
	FILE*	file;
	unsigned short	checksum;

	strcpy(m_filename, filename);
	m_mode = *mode;
	m_bWriteCrypt = bWriteCrypt;

	if ( m_mode == 'r' )
	{
		file = fopen(filename, "rb");
		if ( file == 0 )  return FALSE;

		fseek(file, 0, SEEK_END);
		m_length = ftell(file);
		fseek(file, 0, SEEK_SET);

		free(m_data);
		m_data = (unsigned char*)malloc(m_length);
		fread(m_data, m_length, 1, file);
		fclose(file);

		if ( m_data[0] == CRYPT_MARK )
		{
			m_bCrypt = TRUE;
			Decrypt(m_data+3, m_length-3);  // décrypte

			checksum = m_data[1];
			checksum += m_data[2]<<8;
			if ( checksum != Checksum(m_data+3, m_length-3) )
			{
				return FALSE;
			}

			m_seek = 3;
		}
		else
		{
#if !_DEBUG
			return FALSE;
#endif
			m_bCrypt = FALSE;
			m_seek = 0;
		}
	}

	if ( m_mode == 'w' )
	{
		free(m_data);
		m_data = (unsigned char*)malloc(LENGTH_MAX);
		m_seek = 0;

		if ( m_bWriteCrypt )
		{
			m_data[m_seek++] = CRYPT_MARK;
			m_data[m_seek++] = 0;
			m_data[m_seek++] = 0;  // checksum
		}
	}

	return TRUE;
}

// Lit une ligne dans le fichier.

BOOL CCryptFile::GetLine(char *buffer, int max)
{
	char	byte;

	if ( m_data == 0 )  return FALSE;
	if ( m_mode != 'r' )  return FALSE;

	while ( TRUE )
	{
		if ( m_seek >= m_length )  return FALSE;

		byte = m_data[m_seek++];
		if ( byte == '\r' )
		{
			if ( m_seek < m_length && m_data[m_seek] == '\n' )  m_seek ++;
			break;
		}

		if ( --max <= 0 )  break;
		*buffer++ = byte;
	}
	*buffer++ = 0;

	return TRUE;
}

// Ecrit une ligne dans le fichier.

BOOL CCryptFile::PutLine(char *buffer)
{
	if ( m_data == 0 )  return FALSE;
	if ( m_mode != 'w' )  return FALSE;

	while ( *buffer !=  0   &&
			*buffer != '\r' &&
			*buffer != '\n' )
	{
		m_data[m_seek++] = *buffer++;
	}
	m_data[m_seek++] = '\r';  // 0x0d
	m_data[m_seek++] = '\n';  // 0x0a

	return TRUE;
}

// Ferme le fichier.

BOOL CCryptFile::Close()
{
	FILE*	file;
	unsigned short	checksum;

	if ( m_data == 0 )  return FALSE;

	if ( m_mode == 'w' )
	{
		remove(m_filename);

		file = fopen(m_filename, "wb");
		if ( file == 0 )  return FALSE;

		if ( m_bWriteCrypt )
		{
			checksum = Checksum(m_data+3, m_seek-3);
			m_data[1] = checksum&0x00ff;
			m_data[2] = (checksum>>8)&0x00ff;

			Crypt(m_data+3, m_seek-3);  // crypte
		}
		fwrite(m_data, m_seek, 1, file);
		fclose(file);
	}

	free(m_data);
	m_data = 0;
	return TRUE;
}

