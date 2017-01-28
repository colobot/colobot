// displaytext.cpp

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
#include "interface.h"
#include "button.h"
#include "label.h"
#include "window.h"
#include "group.h"
#include "text.h"
#include "sound.h"
#include "displaytext.h"




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
		m_time[i] = 0.0f;  // rien d'affiché
	}

	m_bHide = FALSE;
	m_bEnable = TRUE;
	m_delayFactor = 1.0f;
	m_channelSound = -1;
}

// Destructeur de l'objet.

CDisplayText::~CDisplayText()
{
	m_iMan->DeleteInstance(CLASS_DISPLAYTEXT, this);
}


// Détruit l'objet.

void CDisplayText::DeleteObject()
{
	m_interface->DeleteControl(EVENT_WINDOW1);
}


// Gestion d'un événement.

BOOL CDisplayText::EventProcess(const Event &event)
{
	D3DVECTOR	eye;
	int			i;

	if ( m_engine->RetPause() )  return TRUE;

	if ( event.event == EVENT_FRAME )
	{
		if ( m_channelSound != -1 )
		{
			eye = m_engine->RetEyePt();
			if ( !m_sound->Position(m_channelSound, eye) )
			{
				m_channelSound = -1;
			}
		}

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

void CDisplayText::DisplayError(Error err, float time, float size)
{
	TextType	type;
	char		text[100];

	if ( err == ERR_OK )  return;

	type = TT_WARNING;
	if ( err >= INFO_FIRST )
	{
		type = TT_INFO;
	}
	if ( err == INFO_LOST )
	{
		type = TT_ERROR;
	}

	GetResource(RES_ERR, err, text);
	DisplayText(text, time, size, type);
}

// Affiche le texte.

void CDisplayText::DisplayText(char *text, float time, float size,
							   TextType type, Sound sound)
{
	CWindow*	pw;
	CGroup*		group;
	CLabel*		label;
	D3DVECTOR	eye;
	FPOINT		pos, ppos, dim;
	float		hLine, hBox;
	int			nLine, icon, i;

	if ( !m_bEnable )  return;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
	if ( pw == 0 )
	{
		pos.x = 0.0f;
		pos.y = 0.0f;
		dim.x = 0.0f;
		dim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, dim, 10, EVENT_WINDOW1);
	}

	hBox = 0.045f*size/FONTSIZE;
	hLine = m_engine->RetText()->RetHeight(size, FONT_COLOBOT);

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
	if ( type == TT_START   )  icon =  8;  // vert
	pw->CreateGroup(pos, dim, icon, EventMsg(EVENT_DT_GROUP0+nLine));

	pw->SetTrashEvent(FALSE);

	ppos = pos;
	ppos.y -= hLine/2.0f;
	label = pw->CreateLabel(ppos, dim, -1, EventMsg(EVENT_DT_LABEL0+nLine), text);
	if ( label != 0 )
	{
		label->SetFontSize(size);
	}

	m_bExist[nLine] = TRUE;
	m_time[nLine] = time*m_delayFactor;

	if ( m_bHide )
	{
		HideText(m_bHide);  // cache tout
	}
	else
	{
		if ( sound == SOUND_CLICK )
		{
			if ( type == TT_ERROR   )  sound = SOUND_ERROR;
			if ( type == TT_WARNING )  sound = SOUND_WARNING;
			if ( type == TT_INFO    )  sound = SOUND_INFO;
			if ( type == TT_MESSAGE )  sound = SOUND_MESSAGE;
			if ( type == TT_START   )  sound = SOUND_CLICK;
		}

		if ( sound != SOUND_CLICK )
		{
			eye = m_engine->RetEyePt();
			m_channelSound = m_sound->Play(sound, eye);
		}
	}
}

// Efface tous les textes.

void CDisplayText::ClearText()
{
	CWindow*	pw;
	int			i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);

	for ( i=0 ; i<MAXDTLINE ; i++ )
	{
		if ( pw != 0 )
		{
			pw->DeleteControl(EventMsg(EVENT_DT_GROUP0+i));
			pw->DeleteControl(EventMsg(EVENT_DT_LABEL0+i));
		}
		m_bExist[i] = FALSE;
		m_time[i] = 0.0f;
	}
}

// Cache ou montre tous les textes.

void CDisplayText::HideText(BOOL bHide)
{
	CWindow*	pw;
	CGroup*		pg;
	CLabel*		pl;
	int			i;

	m_bHide = bHide;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
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
	}
}

// Efface le dernier texte (en haut de la liste).

BOOL CDisplayText::ClearLastText()
{
	CWindow		*pw;
	CGroup		*pg1, *pg2;
	CLabel		*pl1, *pl2;
	int			i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
	if ( pw == 0 )  return FALSE;

	pg2 = (CGroup*)pw->SearchControl(EVENT_DT_GROUP0);
	if ( pg2 == 0 )  return FALSE;
	pl2 = (CLabel*)pw->SearchControl(EVENT_DT_LABEL0);
	if ( pl2 == 0 )  return FALSE;  // même pas de première ligne

	for ( i=0 ; i<MAXDTLINE-1 ; i++ )
	{
		pg1 = pg2;
		pl1 = pl2;

		pg2 = (CGroup*)pw->SearchControl(EventMsg(EVENT_DT_GROUP0+i+1));
		if ( pg2 == 0 )  break;

		pl2 = (CLabel*)pw->SearchControl(EventMsg(EVENT_DT_LABEL0+i+1));
		if ( pl2 == 0 )  break;

		pg1->SetIcon(pg2->RetIcon());
		pl1->SetName(pl2->RetName());

		m_time[i] = m_time[i+1];
	}

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


