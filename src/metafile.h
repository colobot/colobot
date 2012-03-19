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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// metafile.h

#ifndef _METAFILE_H_
#define	_METAFILE_H_



#define METAMAX		5

typedef struct
{
	char		name[14];	// nom du fichier (8.3 max)
	int			start;		// position depuis le début du metafile
	int			len;		// longueur du fichier
}
MetaHeader;

typedef struct
{
	char		name[50];	// nom du metafile
	FILE*		stream;		// canal
	int			total;		// nb de fichiers contenus
	MetaHeader*	headers;	// headers des fichiers contenus
}
MetaFile;



class CMetaFile
{
public:
	CMetaFile();
	~CMetaFile();

	BOOL	IsExist(char *metaname, char *filename);
	int		Open(char *metaname, char *filename);
	int		RetLength();
	int		Seek(int offset);
	int		Read(void *buffer, int size);
	int		GetByte();
	int		GetWord();
	int		Close();
	int		MetaClose();

protected:
	int		MetaOpen(char *metaname);
	int		MetaSearch(char *metaname);

protected:
	MetaFile	m_list[METAMAX];	// metafile ouverts
	BOOL		m_bOpen;			// fichier ouvert
	BOOL		m_bMeta;			// metafile ouvert
	FILE*		m_stream;			// canal
	int			m_start;			// position depuis le début
	int			m_pos;				// position courante
	int			m_len;				// longueur du fichier
};


#endif //_METAFILE_H_
