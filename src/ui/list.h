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

// list.h

#pragma once


#include "ui/control.h"
#include "common/event.h"


class CD3DEngine;
class CButton;
class CScroll;


#define LISTMAXDISPLAY	20		// maximum number of visible lines
#define LISTMAXTOTAL	100		// maximum total number of lines



class CList : public CControl
{
public:
	CList(CInstanceManager* iMan);
	~CList();

	bool		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg, float expand);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	bool		SetState(int state, bool bState);
	bool		SetState(int state);
	bool		ClearState(int state);

	bool		EventProcess(const Event &event);
	void		Draw();

	void		Flush();

	void		SetTotal(int i);
	int			RetTotal();

	void		SetSelect(int i);
	int			RetSelect();

	void		SetSelectCap(bool bEnable);
	bool		RetSelectCap();

	void		SetBlink(bool bEnable);
	bool		RetBlink();

	void		SetName(int i, char* name);
	char*		RetName(int i);

	void		SetCheck(int i, bool bMode);
	bool		RetCheck(int i);

	void		SetEnable(int i, bool bEnable);
	bool		RetEnable(int i);

	void		SetTabs(int i, float pos, int justif=1);
	float		RetTabs(int i);

	void		ShowSelect(bool bFixed);

	EventMsg	RetEventMsgButton(int i);
	EventMsg	RetEventMsgScroll();

protected:
	bool		MoveAdjust();
	void		UpdateButton();
	void		UpdateScroll();
	void		MoveScroll();
	void		DrawCase(char *text, FPOINT pos, float width, int justif);

protected:
	CButton*	m_button[LISTMAXDISPLAY];
	CScroll*	m_scroll;

	EventMsg	m_eventButton[LISTMAXDISPLAY];
	EventMsg	m_eventScroll;

	float		m_expand;
	int			m_totalLine;	// total number of lines
	int			m_displayLine;	// number of visible lines
	int			m_selectLine;	// selected line
	int			m_firstLine;	// first visible line
	bool		m_bBlink;
	bool		m_bSelectCap;
	float		m_blinkTime;
	float		m_tabs[10];
	int			m_justifs[10];

	char		m_text[LISTMAXTOTAL][100];
	char		m_check[LISTMAXTOTAL];
	char		m_enable[LISTMAXTOTAL];
};


