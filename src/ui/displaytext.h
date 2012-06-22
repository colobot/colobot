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

// displaytext.h

#pragma once


#include "common/event.h"
#include "common/misc.h"
#include "old/d3dengine.h"


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CObject;
class CSound;


enum TextType
{
	TT_ERROR	= 1,
	TT_WARNING	= 2,
	TT_INFO		= 3,
	TT_MESSAGE	= 4,
};

const int MAXDTLINE = 4;


class CDisplayText
{
public:
	CDisplayText(CInstanceManager* iMan);
	~CDisplayText();

	void		DeleteObject();

	bool		EventProcess(const Event &event);

	void		DisplayError(Error err, CObject* pObj, float time=10.0f);
	void		DisplayError(Error err, Math::Vector goal, float height=15.0f, float dist=60.0f, float time=10.0f);
	void		DisplayText(char *text, CObject* pObj, float time=10.0f, TextType type=TT_INFO);
	void		DisplayText(char *text, Math::Vector goal, float height=15.0f, float dist=60.0f, float time=10.0f, TextType type=TT_INFO);
	void		HideText(bool bHide);
	void		ClearText();
	bool		ClearLastText();
	void		SetDelay(float factor);
	void		SetEnable(bool bEnable);

	Math::Vector	RetVisitGoal(EventMsg event);
	float		RetVisitDist(EventMsg event);
	float		RetVisitHeight(EventMsg event);

	float		RetIdealDist(CObject* pObj);
	float		RetIdealHeight(CObject* pObj);

	void		ClearVisit();
	void		SetVisit(EventMsg event);
	bool		IsVisit(EventMsg event);

protected:
	CObject*	SearchToto();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CSound*			m_sound;

	bool			m_bExist[MAXDTLINE];
	float			m_time[MAXDTLINE];
	Math::Vector		m_visitGoal[MAXDTLINE];
	float			m_visitDist[MAXDTLINE];
	float			m_visitHeight[MAXDTLINE];

	bool			m_bHide;
	bool			m_bEnable;
	float			m_delayFactor;
};


