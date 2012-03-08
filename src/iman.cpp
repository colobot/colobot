// iman.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>

#include "struct.h"
#include "iman.h"




// Constructeur de l'objet.

CInstanceManager::CInstanceManager()
{
	int		i;

	for ( i=0 ; i<CLASS_MAX ; i++ )
	{
		m_table[i].totalPossible = 0;
		m_table[i].totalUsed     = 0;
		m_table[i].classPointer  = 0;
	}
}

// Destructeur de l'objet.

CInstanceManager::~CInstanceManager()
{
	int		i;

	for ( i=0 ; i<CLASS_MAX ; i++ )
	{
		if ( m_table[i].classPointer != 0 )
		{
			free(m_table[i].classPointer);
		}
	}
}


// Vide toute la liste des classes.

void CInstanceManager::Flush()
{
	int		i;

	for ( i=0 ; i<CLASS_MAX ; i++ )
	{
		if ( m_table[i].classPointer != 0 )
		{
			free(m_table[i].classPointer);
		}
		m_table[i].classPointer = 0;
	}
}

// Vide toutes les instances d'une classe donnée.

void CInstanceManager::Flush(ClassType classType)
{
	if ( classType < 0 || classType >= CLASS_MAX )  return;
	if ( m_table[classType].classPointer == 0 )  return;

	free(m_table[classType].classPointer);
	m_table[classType].classPointer = 0;
}


// Ajoute une nouvelle instance d'une classe.

BOOL CInstanceManager::AddInstance(ClassType classType, void* pointer, int max)
{
	int		i;

	if ( classType < 0 || classType >= CLASS_MAX )  return FALSE;

	if ( m_table[classType].classPointer == 0 )
	{
		m_table[classType].classPointer = (void**)malloc(max*sizeof(void*));
		m_table[classType].totalPossible = max;
		m_table[classType].totalUsed     = 0;
	}

	if ( m_table[classType].totalUsed >= m_table[classType].totalPossible )  return FALSE;

	i = m_table[classType].totalUsed++;
	m_table[classType].classPointer[i] = pointer;
	return TRUE;
}

// Supprime une instance d'une classe.

BOOL CInstanceManager::DeleteInstance(ClassType classType, void* pointer)
{
	int		i;

	if ( classType < 0 || classType >= CLASS_MAX )  return FALSE;

	for ( i=0 ; i<m_table[classType].totalUsed ; i++ )
	{
		if ( m_table[classType].classPointer[i] == pointer )
		{
			m_table[classType].classPointer[i] = 0;
		}
	}

	Compress(classType);
	return TRUE;
}

// Cherche une instance existante. Retourne 0 si elle n'existe pas.
// Doit être super-rapide !

void* CInstanceManager::SearchInstance(ClassType classType, int rank)
{
#if _DEBUG
	if ( classType < 0 || classType >= CLASS_MAX )  return 0;
	if ( m_table[classType].classPointer == 0 )  return 0;
#endif
	if ( rank >= m_table[classType].totalUsed )  return 0;

	return m_table[classType].classPointer[rank];
}


// Bouche les trous dans une table.

void CInstanceManager::Compress(ClassType classType)
{
	int		i, j;

	if ( classType < 0 || classType >= CLASS_MAX )  return;

	j = 0;
	for ( i=0 ; i<m_table[classType].totalUsed ; i++ )
	{
		if ( m_table[classType].classPointer[i] != 0 )
		{
			m_table[classType].classPointer[j++] = m_table[classType].classPointer[i];
		}
	}
	m_table[classType].totalUsed = j;
}


