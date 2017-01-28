// filebuffer.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "language.h"
#include "misc.h"
#include "iman.h"
#include "filebuffer.h"




// Constructeur de l'application robot.

CFileBuffer::CFileBuffer(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_FILEBUFFER, this);

	m_total  = 0;
	m_length = 0;
	m_buffer = 0;
	m_used   = 0;
}

// Destructeur de l'application robot.

CFileBuffer::~CFileBuffer()
{
	Close();
}


// Prépare un buffer vide.

void CFileBuffer::Open()
{
	Close();
	m_total  = 10000;
	m_length = 0;
	m_used   = 0;
	m_buffer = (char*)malloc(m_total);
}

// Libère tout.

void CFileBuffer::Close()
{
	if ( m_buffer != 0 )
	{
		free(m_buffer);
		m_buffer = 0;
	}
}


// Mémorise une ligne.

BOOL CFileBuffer::PutLine(int chapter, char *line)
{
	int		len, rank;

	len = strlen(line)+1;
	if ( m_length+len >= m_total )  return FALSE;
	if ( m_used >= 100 )  return FALSE;

	if ( m_used == 0 )
	{
		rank = 0;
	}
	else
	{
		if ( chapter == m_index[m_used-1].chapter )  // même chapitre ?
		{
			rank = m_index[m_used-1].rank+1;
		}
		else
		{
			rank = 0;
		}
	}

	m_index[m_used].chapter = chapter;
	m_index[m_used].rank    = rank;
	m_index[m_used].index   = m_length;
	m_used ++;

	strcpy(m_buffer+m_length, line);
	m_length += len;

	return TRUE;
}

// Redonne une ligne.

BOOL CFileBuffer::GetLine(int chapter, int rank, char *line)
{
	int		i;

	for ( i=0 ; i<m_used ; i++ )
	{
		if ( chapter == m_index[i].chapter &&
			 rank    == m_index[i].rank    )
		{
			strcpy(line, m_buffer+m_index[i].index);
			return TRUE;
		}
	}

	return FALSE;
}


