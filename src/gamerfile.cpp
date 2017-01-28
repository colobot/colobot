// gamerfile.cpp

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
#include "cryptfile.h"
#include "global.h"
#include "language.h"
#include "misc.h"
#include "iman.h"
#include "cmdtoken.h"
#include "gamerfile.h"





// Constructeur.

CGamerFile::CGamerFile(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_GAMERFILE, this);

	Flush();
}

// Destructeur.

CGamerFile::~CGamerFile()
{
}


// Vide tout.

void CGamerFile::Flush()
{
	int		i;

	m_bWriteToDo = FALSE;
	m_filename[0] = 0;

	for ( i=0 ; i<MAXSELECT ; i++ )
	{
		m_select[i][0] = 0;
	}

	for ( i=0 ; i<MAXGAMERFILE ; i++ )
	{
		m_gamerFile[i].puzzle[0] = 0;
	}
}


// Lit le fichier de description.

BOOL CGamerFile::Read(char *filename)
{
	CCryptFile	file;
	char		line[200];
	char		op[20];
	char		puzzle[50];
	int			i;

	Flush();
	strcpy(m_filename, filename);

	if ( !file.Open(filename, "r") )  return FALSE;

	while ( file.GetLine(line, 200) )
	{
		if ( Cmd(line, "Header") )
		{
			for ( i=0 ; i<MAXSELECT ; i++ )
			{
				sprintf(op, "sel%d", i);
				OpString(line, op, m_select[i]);
			}
		}

		if ( Cmd(line, "Item") )
		{
			OpString(line, "puzzle", puzzle);
			if ( puzzle[0] == 0 )  continue;

			i = CreateIndex(puzzle);
			if ( i == -1 )  continue;

			m_gamerFile[i].numTry    = OpInt(line, "numTry", 0);
			m_gamerFile[i].bPassed   = OpInt(line, "passed", 0);
			m_gamerFile[i].totalTime = OpFloat(line, "totalTime", 0.0f);
		}
	}

	file.Close();
	m_bWriteToDo = FALSE;
	return TRUE;
}

// Ecrit le fichier de description.

BOOL CGamerFile::Write()
{
	CCryptFile	file;
	char		line[200];
	char		text[100];
	int			i;

	if ( !m_bWriteToDo )  return TRUE;
	if ( m_filename[0] == 0 )  return FALSE;

	if ( !file.Open(m_filename, "w") )  return FALSE;

	strcpy(line, "Header");
	for ( i=0 ; i<MAXSELECT ; i++ )
	{
		if ( m_select[i][0] == 0 )  continue;
		sprintf(text, " sel%d=\"%s\"", i, m_select[i]);
		strcat(line, text);
	}
	strcat(line, "\n");
	file.PutLine(line);

	for ( i=0 ; i<MAXGAMERFILE ; i++ )
	{
		if ( m_gamerFile[i].puzzle[0] == 0 )  continue;

		sprintf(line, "Item puzzle=\"%s\" numTry=%d passed=%d totalTime=%.2f\n",
							m_gamerFile[i].puzzle,
							m_gamerFile[i].numTry,
							m_gamerFile[i].bPassed,
							m_gamerFile[i].totalTime);
		file.PutLine(line);
	}

	file.Close();
	m_bWriteToDo = FALSE;
	return TRUE;
}


// Supprime un puzzle du fichier de description.

BOOL CGamerFile::Delete(char *puzzle)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  return FALSE;

	m_gamerFile[i].puzzle[0] = 0;
	m_bWriteToDo = TRUE;
	return TRUE;
}


// Gestion de la sélection.

void CGamerFile::RetSelect(int i, char *filename)
{
	if ( i < 0 || i >= MAXSELECT )  return;
	strcpy(filename, m_select[i]);
}

void CGamerFile::SetSelect(int i, char *filename)
{
	if ( i < 0 || i >= MAXSELECT )  return;
	if ( strcmp(m_select[i], filename) == 0 )  return;

	strcpy(m_select[i], filename);
	m_bWriteToDo = TRUE;
}


// Gestion du nombre de tentatives.

int CGamerFile::RetNumTry(char *puzzle)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  return 0;

	return m_gamerFile[i].numTry;
}

BOOL CGamerFile::SetNumTry(char *puzzle, int numTry)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  i = CreateIndex(puzzle);
	if ( i == -1 )  return FALSE;

	m_gamerFile[i].numTry = numTry;
	m_bWriteToDo = TRUE;
	return TRUE;
}


// Gestion de l'état "puzzle réussi".

BOOL CGamerFile::RetPassed(char *puzzle)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  return FALSE;

	return m_gamerFile[i].bPassed;
}

BOOL CGamerFile::SetPassed(char *puzzle, BOOL bPassed)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  i = CreateIndex(puzzle);
	if ( i == -1 )  return FALSE;

	m_gamerFile[i].bPassed = bPassed;
	m_bWriteToDo = TRUE;
	return TRUE;
}


// Gestion du temps total joué.

float CGamerFile::RetTotalTime(char *puzzle)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  return 0.0f;

	return m_gamerFile[i].totalTime;
}

BOOL CGamerFile::SetTotalTime(char *puzzle, float totalTime)
{
	int		i;

	i = SearchIndex(puzzle);
	if ( i == -1 )  i = CreateIndex(puzzle);
	if ( i == -1 )  return FALSE;

	m_gamerFile[i].totalTime = totalTime;
	m_bWriteToDo = TRUE;
	return TRUE;
}



// Cherche l'index selon le nom du puzzle.

int CGamerFile::SearchIndex(char *puzzle)
{
	int		i;

	for ( i=0 ; i<MAXGAMERFILE ; i++ )
	{
		if ( strcmp(puzzle, m_gamerFile[i].puzzle) == 0 )  return i;
	}
	return -1;
}

// Crée un nouvel index pour un nom du puzzle.

int CGamerFile::CreateIndex(char *puzzle)
{
	int		i;

	for ( i=0 ; i<MAXGAMERFILE ; i++ )
	{
		if ( m_gamerFile[i].puzzle[0] == 0 )
		{
			strcpy(m_gamerFile[i].puzzle, puzzle);
			m_gamerFile[i].numTry = 0;
			m_gamerFile[i].bPassed = FALSE;
			m_gamerFile[i].totalTime = 0.0f;
			return i;
		}
	}
	return -1;
}

