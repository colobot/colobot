// mainundo.cpp

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
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "motion.h"
#include "auto.h"
#include "pyro.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "tasklist.h"
#include "mainundo.h"





// Constructeur de l'application robot.

CMainUndo::CMainUndo(CInstanceManager* iMan)
{
	int		i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_UNDO, this);

	m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_terrain   = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);

	m_currentIndex = 0;
	m_currentMax   = 0;
	m_currentState = 0;

	for ( i=0 ; i<MAXUNDO ; i++ )
	{
		m_undoBuffer[i] = 0;
		m_undoResource[i] = 0;
		m_undoLockZone[i] = 0;
		m_undoTotalManip[i] = 0;
	}
	m_undoIndex = 0;
}

// Destructeur de l'application robot.

CMainUndo::~CMainUndo()
{
}


// Efface toutes les situations mémorisées.

void CMainUndo::Flush()
{
	int		i;

	for ( i=0 ; i<m_undoIndex ; i++ )
	{
		free(m_undoBuffer[i]);
		free(m_undoResource[i]);
		free(m_undoLockZone[i]);
		m_undoBuffer[i] = 0;
		m_undoResource[i] = 0;
		m_undoLockZone[i] = 0;
		m_undoTotalManip[i] = 0;
	}
	m_undoIndex = 0;
}


// Mémorise la situation actuelle.

BOOL CMainUndo::Record()
{
	CObject*	pObj;
	int			i;

	if ( !IsOperable() )  return FALSE;

	WriteOpen();

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !IsUndoObject(pObj) )  continue;

		m_currentLine[0] = 0;
		pObj->WriteSituation();
		WriteLine(m_currentLine);
	}

	WriteClose();

	return TRUE;
}


// Indique s'il est possible d'annuler une action.

BOOL CMainUndo::IsUndoable()
{
	if ( !IsOperable() )  return FALSE;
	return (m_undoIndex > 0);
}

// Revient à la situation mémorisée précédente.

BOOL CMainUndo::Undo()
{
	CObject*	pObj;
	ObjectType	type;
	D3DVECTOR	iPos, nPos;
	float		iAngle, nAngle;
	int			iLock, nLock, iDead, nDead;
	int			additionnal[10];
	int			i, j, id, it;
	char		op[10];
	BOOL		bCreate;

	if ( m_undoIndex == 0 )  return FALSE;
	if ( !IsOperable() )  return FALSE;

	i = m_undoIndex-1;

	m_currentRead = m_undoBuffer[i];
	while ( *m_currentRead != 0 )
	{
		if ( ReadTokenInt("id", id) )
		{
			pObj = SearchObjectID(id);

			// Objet en train d'exploser ?
			if ( pObj != 0 && pObj->RetExplo() )
			{
				DeletePyro(pObj);
				pObj->DeleteObject();  // il sera recréé après
				delete pObj;
				pObj = 0;
			}

			if ( pObj == 0 )
			{
				iPos = D3DVECTOR(NAN, NAN, NAN);
				iAngle = NAN;
				iLock = NAN;
				iDead = NAN;
			}
			else
			{
				iPos = pObj->RetPosition(0);
				iAngle = pObj->RetAngleY(0);
				iLock = pObj->RetLock();
				iDead = pObj->RetDead();
			}

			ReadTokenPos("pos", nPos);
			ReadTokenFloat("ay", nAngle);
			ReadTokenInt("lock", nLock);
			ReadTokenInt("dead", nDead);

			for ( j=0 ; j<10 ; j++ )
			{
				additionnal[j] = 0;
				sprintf(op, "add%d", j);
				ReadTokenInt(op, additionnal[j]);
			}

			bCreate = FALSE;
			if ( nDead != iDead )  bCreate = TRUE;

			if ( bCreate )
			{
				if ( pObj != 0 )
				{
					pObj->DeleteObject();
					delete pObj;
				}

				ReadTokenInt("type", it);
				type = (ObjectType)it;
				pObj = new CObject(m_iMan);
				pObj->CreateObject(nPos, nAngle, 1.0f, 0.0f, type, 0,
									additionnal[0],
									additionnal[1],
									additionnal[2],
									additionnal[3],
									additionnal[4]);
				pObj->SetID(id);  // remet l'id original
				pObj->SetLock(nLock);
				pObj->SetDead(nDead);
			}
			else
			{
				pObj->ReadSituation();
				pObj->SetLock(nLock);
				pObj->SetDead(nDead);
				pObj->SetExplo(FALSE);
				DeletePyro(pObj);
			}
		}

		while ( *m_currentRead++ != 0 );
	}
	
	m_terrain->PutResource(m_undoResource[i]);
	m_terrain->PutLockZone(m_undoLockZone[i]);
	m_main->SetTotalManip(m_undoTotalManip[i]);

	free(m_undoBuffer[i]);
	free(m_undoResource[i]);
	free(m_undoLockZone[i]);
	m_undoBuffer[i] = 0;
	m_undoResource[i] = 0;
	m_undoLockZone[i] = 0;
	m_undoTotalManip[i] = 0;

	m_undoIndex --;
	return TRUE;
}

// Supprime un effet pyrotechnique sur un objet.

void CMainUndo::DeletePyro(CObject *pObj)
{
	CPyro*	pPyro;
	int		i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pPyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, i);
		if ( pPyro == 0 )  break;

		if ( pPyro->IsObject(pObj) )
		{
			pPyro->DeleteObject();
			delete pPyro;
			break;
		}
	}
}

// Indique si un objet est mobile, donc s'il est concerné par les undo.

BOOL CMainUndo::IsUndoObject(CObject *pObj)
{
	ObjectType	type;

	type = pObj->RetType();

	if ( type == OBJECT_BLUPI  ||
		 type == OBJECT_GOAL   ||
		 type == OBJECT_CRAZY  ||
		 type == OBJECT_TRAX   ||
		 type == OBJECT_PERFO  ||
		 type == OBJECT_GUN    ||
		 type == OBJECT_MAX1X  ||
		 type == OBJECT_DOCK   ||
		 type == OBJECT_MINE   ||
		 type == OBJECT_FIOLE  ||
		 type == OBJECT_GLU    ||
		 type == OBJECT_GLASS1 ||
		 type == OBJECT_GLASS2 )  return TRUE;

	if ( type >= OBJECT_BOX1  &&
		 type <= OBJECT_BOX20 )  return TRUE;

	if ( type >= OBJECT_KEY1  &&
		 type <= OBJECT_KEY5  )  return TRUE;

	if ( type >= OBJECT_DOOR1 &&
		 type <= OBJECT_DOOR5 )  return TRUE;

	return FALSE;
}

// Cherche s'il est possible d'opérer. Si un seul objet est en
// train d'accomplir une tâche, il est impossible d'opérer !

BOOL CMainUndo::IsOperable()
{
	CObject*	pObj;
	CTaskList*	taskList;
	CAuto*		pAuto;
	ObjectType	type;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetNoUndoable() )  return FALSE;

		type = pObj->RetType();
//?		if ( type == OBJECT_CRAZY )  continue;

		taskList = pObj->RetTaskList();
		if ( taskList != 0 )
		{
//?			if ( taskList->IsRunning() )  return FALSE;
			if ( !taskList->IsUndoable() )  return FALSE;
		}

		pAuto = pObj->RetAuto();
		if ( pAuto != 0 )
		{
			if ( pAuto->IsRunning() )  return FALSE;
		}
	}

	return TRUE;
}


// Ajoute un élement "token=n" dans la ligne courante.

void CMainUndo::WriteTokenInt(char *token, int value)
{
	char	cmd[100];

	sprintf(cmd, " %s=%d", token, value);
	strcat(m_currentLine, cmd);
}

// Ajoute un élement "token=n" dans la ligne courante.

void CMainUndo::WriteTokenFloat(char *token, float value)
{
	char	cmd[100];

	sprintf(cmd, " %s=%.2f", token, value);
	strcat(m_currentLine, cmd);
}

// Ajoute un élement "token=n" dans la ligne courante.

void CMainUndo::WriteTokenPos(char *token, D3DVECTOR value)
{
	char	cmd[100];

	sprintf(cmd, " %s=%.2f;%.2f;%.2f", token, value.x, value.y, value.z);
	strcat(m_currentLine, cmd);
}


// Cherche un opérateur.

char* UndoSearchOp(char *line, char *op)
{
	char	opeq[50];
	char*	p;

	strcpy(opeq, " ");
	strcat(opeq, op);
	strcat(opeq, "=");

	p = strstr(line, opeq);
	if ( p == 0 )  return 0;  // pas trouvé ?
	return p+strlen(opeq);  // pointe après le "="
}

// Lit un élément "token=n" dans la ligne courante.

BOOL CMainUndo::ReadTokenInt(char *token, int &value)
{
	char*	op;

	op = UndoSearchOp(m_currentRead, token);
	if ( op == 0 )  return FALSE;

	sscanf(op, "%d", &value);
	return TRUE;
}

// Lit un élément "token=n" dans la ligne courante.

BOOL CMainUndo::ReadTokenFloat(char *token, float &value)
{
	char*	op;

	op = UndoSearchOp(m_currentRead, token);
	if ( op == 0 )  return FALSE;

	sscanf(op, "%f", &value);
	return TRUE;
}

// Lit un élément "token=n" dans la ligne courante.

BOOL CMainUndo::ReadTokenPos(char *token, D3DVECTOR &value)
{
	char*	op;

	op = UndoSearchOp(m_currentRead, token);
	if ( op == 0 )  return FALSE;

	sscanf(op, "%f;%f;%f", &value.x, &value.y, &value.z);
	return TRUE;
}


// Prépare le fichier en éciture.

void CMainUndo::WriteOpen()
{
	if ( m_currentState != 0 )
	{
		free(m_currentState);
		m_currentState = 0;
	}

	m_currentIndex = 0;
	m_currentMax = 10000;
	m_currentState = (char*)malloc(m_currentMax);
	m_currentState[0] = 0;
}

// Ecrit une ligne dans le fichier.

BOOL CMainUndo::WriteLine(char *text)
{
	int		len;

	len = strlen(text)+1;
	if ( m_currentIndex+len >= m_currentMax )  return FALSE;

	strcpy(m_currentState+m_currentIndex, text);
	m_currentIndex += len;
	m_currentState[m_currentIndex-1] = 0;
	m_currentState[m_currentIndex] = 0;

	return TRUE;
}

// Mémorise et ferme le fichier.

void CMainUndo::WriteClose()
{
	Add(m_currentState, m_currentIndex+1);

	free(m_currentState);
	m_currentState = 0;
	m_currentIndex = 0;
	m_currentMax   = 0;
}

// Ajoute une situation complète.

void CMainUndo::Add(char *situation, int len)
{
	int		i;

	if ( m_undoIndex == MAXUNDO )  // buffer plein ?
	{
		free(m_undoBuffer[0]);
		free(m_undoResource[0]);
		free(m_undoLockZone[0]);

		for ( i=0 ; i<MAXUNDO-1 ; i++ )
		{
			m_undoBuffer[i]     = m_undoBuffer[i+1];
			m_undoResource[i]   = m_undoResource[i+1];
			m_undoLockZone[i]   = m_undoLockZone[i+1];
			m_undoTotalManip[i] = m_undoTotalManip[i+1];
		}
		m_undoIndex --;
	}

	i = m_undoIndex;

	m_undoBuffer[i] = (char*)malloc(len);
	memcpy(m_undoBuffer[i], situation, len);

	m_undoResource[i] = (char*)malloc(m_terrain->LenResource());
	m_terrain->GetResource(m_undoResource[i]);

	m_undoLockZone[i] = (char*)malloc(m_terrain->LenLockZone());
	m_terrain->GetLockZone(m_undoLockZone[i]);

	m_undoTotalManip[i] = m_main->RetTotalManip();

	m_undoIndex ++;
}


// Cherche un objet d'après son identificateur.

CObject* CMainUndo::SearchObjectID(int id)
{
	CObject*	pObj;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetID() == id )  return pObj;
	}

	return 0;
}

