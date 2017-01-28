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
	int		i;

	for ( i=0 ; i<len ; i++ )
	{
		b[i] ^= table_codec[(start++)%23];
	}
}
#endif

#if _DEMO | _SE
static unsigned char table_codec[136] =
{
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
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
};

void Codec(void* buffer, int len, int start)
{
	unsigned char *b = (unsigned char*)buffer;
	int		i;

	for ( i=0 ; i<len ; i++ )
	{
		b[i] ^= table_codec[(start++)%136];
	}
}
#endif



// Copie un nom de fichier en ajoutant la lettre supplémentaire.
// Par exemple, "blupimania1.dat" devient "blupimania1b.dat".

void FileSuppl(char *buffer, char *filename, char letter)
{
	char	c;

	while ( *filename != 0 )
	{
		c = *filename++;
		if ( c == '.' )
		{
			*buffer++ = letter;
		}
		*buffer++ = c;
	}
	*buffer = 0;
}


// Constructeur de l'objet.

CMetaFile::CMetaFile()
{
	int		i;

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

// Destructeur de l'objet.

CMetaFile::~CMetaFile()
{
	MetaClose();
}


// Teste si un fichier existe.

BOOL CMetaFile::IsExist(char *metaname, char *filename, char *suppl)
{
	char	metasuppl[50];

	if ( metaname[0] == 0 || *suppl == 0 )
	{
		return IsExistOne(metaname, filename);
	}

	while ( *suppl != 0 )
	{
		FileSuppl(metasuppl, metaname, *suppl++);
		if ( IsExistOne(metasuppl, filename) )  return TRUE;
	}
	return IsExistOne(metaname, filename);
}

// Teste si un fichier existe.

BOOL CMetaFile::IsExistOne(char *metaname, char *filename)
{
	FILE*	file;
	int		index, i;

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
		if ( index == -1 )  return FALSE;

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

// Ouvre un fichier. Si le metaname est vide, on ouvre
// normalement un fichier.

int CMetaFile::Open(char *metaname, char *filename, char *suppl)
{
	char	metasuppl[50];
	int		err;

	if ( metaname[0] == 0 || *suppl == 0 )
	{
		return OpenOne(metaname, filename);
	}

	while ( *suppl != 0 )
	{
		FileSuppl(metasuppl, metaname, *suppl++);
		err = OpenOne(metasuppl, filename);
		if ( err == 0 )  return 0;
	}
	return OpenOne(metaname, filename);
}

// Ouvre un fichier. Si le metaname est vide, on ouvre
// normalement un fichier.

int CMetaFile::OpenOne(char *metaname, char *filename)
{
	int		index, i;

	if ( m_bOpen )  // fichier déjà ouvert ?
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

// Retourne la longueur d'un fichier.

int CMetaFile::RetLength()
{
	int		len;

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

// Positionnement dans le fichier, relatif au début.

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

// Lit n bytes.

int CMetaFile::Read(void *buffer, int size)
{
	int		err;

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

// Lit un byte.

int CMetaFile::GetByte()
{
	BYTE	b;

	if ( !m_bOpen )  return 1;

	b = getc(m_stream);
	if ( m_bMeta )
	{
		Codec(&b, 1, m_pos);
		m_pos += 1;
	}
	return b;
}

// Lit 2 bytes.

int CMetaFile::GetWord()
{
	WORD	w;

	if ( !m_bOpen )  return 1;

	w = getc(m_stream);
	if ( m_bMeta )
	{
		Codec(&w, 2, m_pos);
		m_pos += 2;
	}
	return w;
}

// Ferme le fichier.

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


// Ouvre un metafile. Retourne l'index ou -1.

int CMetaFile::MetaOpen(char *metaname)
{
	int		i, j, offset;

	i = MetaSearch(metaname);
	if ( i != -1 )  return i;

	for ( i=0 ; i<METAMAX ; i++ )
	{
		if ( m_list[i].stream == 0 )
		{
			m_list[i].stream = fopen(metaname, "rb");
			if ( m_list[i].stream == 0 )  return -1;

			strcpy(m_list[i].name, metaname);  // mémorise le nom

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

// Cherche si le metafile est déjà ouvert. Retourne l'index ou -1.

int CMetaFile::MetaSearch(char *metaname)
{
	int		i;

	for ( i=0 ; i<METAMAX ; i++ )
	{
		if ( m_list[i].stream != 0 )
		{
			if ( strcmp(m_list[i].name, metaname) == 0 )  return i;
		}
	}

	return -1;
}

// Ferme tous ls metafiles.

int CMetaFile::MetaClose()
{
	int		i;

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



