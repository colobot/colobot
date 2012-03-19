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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// displaytext.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "event.h"
#include "misc.h"
#include "restext.h"
#include "iman.h"
#include "object.h"
#include "motion.h"
#include "motiontoto.h"
#include "interface.h"
#include "button.h"
#include "label.h"
#include "window.h"
#include "group.h"
#include "text.h"
#include "sound.h"
#include "displaytext.h"



#define FONTSIZE	12.0f



// Constructeur de l'objet.

CDisplayText::CDisplayText(CInstanceManager* iMan)
{
	int		i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_DISPLAYTEXT, this);

	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	for ( i=0 ; i<MAXDTLINE ; i++ )
	{
		m_bExist[i] = FALSE;
		m_visitGoal[i] = D3DVECTOR(0.0f, 0.0f, 0.0f);
		m_visitDist[i] = 0.0f;
		m_visitHeight[i] = 0.0f;
		m_time[i] = 0.0f;  // rien d'affiché
	}

	m_bHide = FALSE;
	m_bEnable = TRUE;
	m_delayFactor = 1.0f;
}

// Destructeur de l'objet.

CDisplayText::~CDisplayText()
{
	m_iMan->DeleteInstance(CLASS_DISPLAYTEXT, this);
}


// Détruit l'objet.

void CDisplayText::DeleteObject()
{
	m_interface->DeleteControl(EVENT_WINDOW2);
}


// Gestion d'un événement.

BOOL CDisplayText::EventProcess(const Event &event)
{
	int		i;

	if ( m_engine->RetPause() )  return TRUE;

	if ( event.event == EVENT_FRAME )
	{
		for ( i=0 ; i<MAXDTLINE ; i++ )
		{
			if ( !m_bExist[i] )  break;
			m_time[i] -= event.rTime;
		}
		while ( TRUE )
		{
			if ( !m_bExist[0] ||
				 m_time[0] > 0.0f )  break;
			if ( !ClearLastText() )  break;
		}
	}

	return TRUE;
}


// Affiche une erreur.

void CDisplayText::DisplayError(Error err, CObject* pObj, float time)
{
	D3DVECTOR	pos;
	float		h, d;

	if ( pObj == 0 )  return;

	pos = pObj->RetPosition(0);
	h = RetIdealHeight(pObj);
	d = RetIdealDist(pObj);
	DisplayError(err, pos, h, d, time);
}

// Affiche une erreur.

void CDisplayText::DisplayError(Error err, D3DVECTOR goal, float height,
								float dist, float time)
{
	TextType	type;
	char		text[100];

	if ( err == ERR_OK )  return;

#if 0
	type = TT_INFO;
	if ( err < INFO_FIRST )
	{
		type = TT_ERROR;
	}
	if ( err == ERR_TOWER_POWER    ||
		 err == ERR_RESEARCH_POWER ||
		 err == ERR_ENERGY_EMPTY   ||
		 err == ERR_LABO_NULL      ||
		 err == ERR_NUCLEAR_EMPTY  ||
		 err == ERR_CONVERT_EMPTY  )
	{
		type = TT_WARNING;
	}
#else
	type = TT_WARNING;
	if ( err >= INFO_FIRST )
	{
		type = TT_INFO;
	}
	if ( err == ERR_BAT_VIRUS      ||
		 err == ERR_VEH_VIRUS      ||
		 err == ERR_DELETEMOBILE   ||
		 err == ERR_DELETEBUILDING ||
		 err == ERR_TOOMANY        ||
		 err == INFO_LOST          )
	{
		type = TT_ERROR;
	}
#endif

	GetResource(RES_ERR, err, text);
	DisplayText(text, goal, height, dist, time, type);
}

// Affiche le texte.

void CDisplayText::DisplayText(char *text, CObject* pObj,
							   float time, TextType type)
{
	D3DVECTOR	pos;
	float		h, d;

	if ( pObj == 0 )  return;

	pos = pObj->RetPosition(0);
	h = RetIdealHeight(pObj);
	d = RetIdealDist(pObj);
	DisplayText(text, pos, h, d, time, type);
}

// Affiche le texte.

void CDisplayText::DisplayText(char *text, D3DVECTOR goal, float height,
							   float dist, float time, TextType type)
{
	CObject*	toto;
	CMotion*	motion;
	CWindow*	pw;
	CButton*	button;
	CGroup*		group;
	CLabel*		label;
	FPOINT		pos, ppos, dim;
	Sound		sound;
	float		hLine, hBox;
	int			nLine, icon, i;

	if ( !m_bEnable )  return;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw == 0 )
	{
		pos.x = 0.0f;
		pos.y = 0.0f;
		dim.x = 0.0f;
		dim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, dim, 10, EVENT_WINDOW2);
	}

	hBox = 0.045f;
	hLine = m_engine->RetText()->RetHeight(FONTSIZE, FONT_COLOBOT);

	nLine = 0;
	for ( i=0 ; i<MAXDTLINE ; i++ )
	{
		group = (CGroup*)pw->SearchControl(EventMsg(EVENT_DT_GROUP0+i));
		if ( group == 0 )  break;
		nLine ++;
	}

	if ( nLine == MAXDTLINE )
	{
		ClearLastText();
		nLine --;
	}

	pos.x = 0.10f;
	pos.y = 0.92f-hBox-hBox*nLine;
	dim.x = 0.80f;
	dim.y = hBox;

	icon = 1;  // jaune
	if ( type == TT_ERROR   )  icon =  9;  // rouge
	if ( type == TT_WARNING )  icon = 10;  // bleu
	if ( type == TT_INFO    )  icon =  8;  // vert
	if ( type == TT_MESSAGE )  icon = 11;  // jaune
	pw->CreateGroup(pos, dim, icon, EventMsg(EVENT_DT_GROUP0+nLine));

	pw->SetTrashEvent(FALSE);

	ppos = pos;
	ppos.y -= hLine/2.0f;
	label = pw->CreateLabel(ppos, dim, -1, EventMsg(EVENT_DT_LABEL0+nLine), text);
	if ( label != 0 )
	{
		label->SetFontSize(FONTSIZE);
	}

	dim.x = dim.y*0.75f;
	pos.x -= dim.x;
	button = pw->CreateButton(pos, dim, 14, EventMsg(EVENT_DT_VISIT0+nLine));

	if ( goal.x == 0.0f &&
		 goal.y == 0.0f &&
		 goal.z == 0.0f )
	{
		button->ClearState(STATE_ENABLE);
	}

	m_bExist[nLine] = TRUE;
	m_visitGoal[nLine] = goal;
	m_visitDist[nLine] = dist;
	m_visitHeight[nLine] = height;
	m_time[nLine] = time*m_delayFactor;

	toto = SearchToto();
	if ( toto != 0 )
	{
		motion = toto->RetMotion();
		if ( motion != 0 )
		{
			if ( type == TT_ERROR )
			{
				motion->SetAction(MT_ERROR, 4.0f);
			}
			if ( type == TT_WARNING )
			{
				motion->SetAction(MT_WARNING, 4.0f);
			}
			if ( type == TT_INFO )
			{
				motion->SetAction(MT_INFO, 4.0f);
			}
			if ( type == TT_MESSAGE )
			{
				motion->SetAction(MT_MESSAGE, 4.0f);
			}
		}
	}

	if ( m_bHide )
	{
		HideText(m_bHide);  // cache tout
	}
	else
	{
		sound = SOUND_CLICK;
		if ( type == TT_ERROR   )  sound = SOUND_ERROR;
		if ( type == TT_WARNING )  sound = SOUND_WARNING;
		if ( type == TT_INFO    )  sound = SOUND_INFO;
		if ( type == TT_MESSAGE )  sound = SOUND_MESSAGE;

		if ( sound != SOUND_CLICK )
		{
			m_sound->Play(sound);
		}
	}
}

// Efface tous les textes.

void CDisplayText::ClearText()
{
	CWindow*	pw;
	int			i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);

	for ( i=0 ; i<MAXDTLINE ; i++ )
	{
		if ( pw != 0 )
		{
			pw->DeleteControl(EventMsg(EVENT_DT_GROUP0+i));
			pw->DeleteControl(EventMsg(EVENT_DT_LABEL0+i));
			pw->DeleteControl(EventMsg(EVENT_DT_VISIT0+i));
		}
		m_bExist[i] = FALSE;
		m_visitGoal[i] = D3DVECTOR(0.0f, 0.0f, 0.0f);
		m_visitDist[i] = 0.0f;
		m_visitHeight[i] = 0.0f;
		m_time[i] = 0.0f;
	}
}

// Cache ou montre tous les textes.

void CDisplayText::HideText(BOOL bHide)
{
	CWindow*	pw;
	CGroup*		pg;
	CLabel*		pl;
	CButton*	pb;
	int			i;

	m_bHide = bHide;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw == 0 )  return;

	for ( i=0 ; i<MAXDTLINE ; i++ )
	{
		pg = (CGroup*)pw->SearchControl(EventMsg(EVENT_DT_GROUP0+i));
		if ( pg != 0 )
		{
			pg->SetState(STATE_VISIBLE, !bHide);
		}

		pl = (CLabel* )pw->SearchControl(EventMsg(EVENT_DT_LABEL0+i));
		if ( pl != 0 )
		{
			pl->SetState(STATE_VISIBLE, !bHide);
		}

		pb = (CButton*)pw->SearchControl(EventMsg(EVENT_DT_VISIT0+i));
		if ( pb != 0 )
		{
			pb->SetState(STATE_VISIBLE, !bHide);
		}
	}
}

// Efface le dernier texte (en haut de la liste).

BOOL CDisplayText::ClearLastText()
{
	CWindow		*pw;
	CButton		*pb1, *pb2;
	CGroup		*pg1, *pg2;
	CLabel		*pl1, *pl2;
	int			i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw == 0 )  return FALSE;

	pb2 = (CButton*)pw->SearchControl(EVENT_DT_VISIT0);
	if ( pb2 == 0 )  return FALSE;  // même pas de première ligne
	pg2 = (CGroup*)pw->SearchControl(EVENT_DT_GROUP0);
	if ( pg2 == 0 )  return FALSE;
	pl2 = (CLabel*)pw->SearchControl(EVENT_DT_LABEL0);
	if ( pl2 == 0 )  return FALSE;

	for ( i=0 ; i<MAXDTLINE-1 ; i++ )
	{
		pb1 = pb2;
		pg1 = pg2;
		pl1 = pl2;

		pb2 = (CButton*)pw->SearchControl(EventMsg(EVENT_DT_VISIT0+i+1));
		if ( pb2 == 0 )  break;

		pg2 = (CGroup*)pw->SearchControl(EventMsg(EVENT_DT_GROUP0+i+1));
		if ( pg2 == 0 )  break;

		pl2 = (CLabel*)pw->SearchControl(EventMsg(EVENT_DT_LABEL0+i+1));
		if ( pl2 == 0 )  break;

		pb1->SetState(STATE_ENABLE, pb2->TestState(STATE_ENABLE));
		pg1->SetIcon(pg2->RetIcon());
		pl1->SetName(pl2->RetName());

		m_time[i]        = m_time[i+1];
		m_visitGoal[i]   = m_visitGoal[i+1];
		m_visitDist[i]   = m_visitDist[i+1];
		m_visitHeight[i] = m_visitHeight[i+1];  // shift
	}

	pw->DeleteControl(EventMsg(EVENT_DT_VISIT0+i));
	pw->DeleteControl(EventMsg(EVENT_DT_GROUP0+i));
	pw->DeleteControl(EventMsg(EVENT_DT_LABEL0+i));
	m_bExist[i] = FALSE;
	return TRUE;
}


// Spécifie le facteur du délai.

void CDisplayText::SetDelay(float factor)
{
	m_delayFactor = factor;
}


// Active l'affichage des textes.

void CDisplayText::SetEnable(BOOL bEnable)
{
	m_bEnable = bEnable;
}


// Retourne le goal lors d'une visite.

D3DVECTOR CDisplayText::RetVisitGoal(EventMsg event)
{
	int		i;

	i = event-EVENT_DT_VISIT0;
	if ( i < 0 || i >= MAXDTLINE )  return D3DVECTOR(0.0f, 0.0f, 0.0f);
	return m_visitGoal[i];
}

// Retourne la distance lors d'une visite.

float CDisplayText::RetVisitDist(EventMsg event)
{
	int		i;

	i = event-EVENT_DT_VISIT0;
	if ( i < 0 || i >= MAXDTLINE )  return 0.0f;
	return m_visitDist[i];
}

// Retourne la hauteur lors d'une visite.

float CDisplayText::RetVisitHeight(EventMsg event)
{
	int		i;

	i = event-EVENT_DT_VISIT0;
	if ( i < 0 || i >= MAXDTLINE )  return 0.0f;
	return m_visitHeight[i];
}


// Retourne la distance de visite idéale pour un objet donné.

float CDisplayText::RetIdealDist(CObject* pObj)
{
	ObjectType	type;

	if ( pObj == 0 )  return 40.0f;

	type = pObj->RetType();
	if ( type == OBJECT_PORTICO )  return 200.0f;
	if ( type == OBJECT_BASE    )  return 200.0f;
	if ( type == OBJECT_NUCLEAR )  return 100.0f;
	if ( type == OBJECT_PARA    )  return 100.0f;
	if ( type == OBJECT_SAFE    )  return 100.0f;
	if ( type == OBJECT_TOWER   )  return  80.0f;

	return 60.0f;
}

// Retourne la hauteur de visite idéale pour un objet donné.

float CDisplayText::RetIdealHeight(CObject* pObj)
{
	ObjectType	type;

	if ( pObj == 0 )  return 5.0f;

	type = pObj->RetType();
	if ( type == OBJECT_DERRICK  )  return 35.0f;
	if ( type == OBJECT_FACTORY  )  return 22.0f;
	if ( type == OBJECT_REPAIR   )  return 30.0f;
	if ( type == OBJECT_DESTROYER)  return 30.0f;
	if ( type == OBJECT_STATION  )  return 13.0f;
	if ( type == OBJECT_CONVERT  )  return 20.0f;
	if ( type == OBJECT_TOWER    )  return 30.0f;
	if ( type == OBJECT_RESEARCH )  return 22.0f;
	if ( type == OBJECT_RADAR    )  return 19.0f;
	if ( type == OBJECT_INFO     )  return 19.0f;
	if ( type == OBJECT_ENERGY   )  return 20.0f;
	if ( type == OBJECT_LABO     )  return 16.0f;
	if ( type == OBJECT_NUCLEAR  )  return 40.0f;
	if ( type == OBJECT_PARA     )  return 40.0f;
	if ( type == OBJECT_SAFE     )  return 20.0f;

	return 15.0f;
}


// Supprime toutes les visites.

void CDisplayText::ClearVisit()
{
	CWindow*	pw;
	CButton*	pb;
	int			i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw == 0 )  return;

	for ( i=0 ; i<MAXDTLINE ; i++ )
	{
		pb = (CButton*)pw->SearchControl(EventMsg(EVENT_DT_VISIT0+i));
		if ( pb == 0 )  break;
		pb->SetIcon(14);  // yeux
	}
}

// Met un bouton en mode "visite".

void CDisplayText::SetVisit(EventMsg event)
{
	CWindow*	pw;
	CButton*	pb;
	int			i;

	i = event-EVENT_DT_VISIT0;
	if ( i < 0 || i >= MAXDTLINE )  return;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw == 0 )  return;
	pb = (CButton*)pw->SearchControl(EventMsg(EVENT_DT_VISIT0+i));
	if ( pb == 0 )  return;
	pb->SetIcon(48);  // >
}

// Indique si un bouton est en mode "visite".

BOOL CDisplayText::IsVisit(EventMsg event)
{
	CWindow*	pw;
	CButton*	pb;
	int			i;

	i = event-EVENT_DT_VISIT0;
	if ( i < 0 || i >= MAXDTLINE )  return FALSE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw == 0 )  return FALSE;
	pb = (CButton*)pw->SearchControl(EventMsg(EVENT_DT_VISIT0+i));
	if ( pb == 0 )  return FALSE;
	return (pb->RetIcon() == 48);  // > ?
}


// Retourne l'objet de toto.

CObject* CDisplayText::SearchToto()
{
	ObjectType	type;
	CObject*	pObj;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type == OBJECT_TOTO )
		{
			return pObj;
		}
	}
	return 0;
}

