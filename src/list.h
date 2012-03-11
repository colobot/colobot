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

#ifndef _LIST_H_
#define	_LIST_H_


#include "control.h"


class CD3DEngine;
class CButton;
class CScroll;


#define LISTMAXDISPLAY	20		// nb max de lignes visibles
#define LISTMAXTOTAL	100		// nb max de lignes total



class CList : public CControl
{
public:
	CList(CInstanceManager* iMan);
	~CList();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg, float expand);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		SetState(int state, BOOL bState);
	BOOL		SetState(int state);
	BOOL		ClearState(int state);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		Flush();

	void		SetTotal(int i);
	int			RetTotal();

	void		SetSelect(int i);
	int			RetSelect();

	void		SetSelectCap(BOOL bEnable);
	BOOL		RetSelectCap();

	void		SetBlink(BOOL bEnable);
	BOOL		RetBlink();

	void		SetName(int i, char* name);
	char*		RetName(int i);

	void		SetCheck(int i, BOOL bMode);
	BOOL		RetCheck(int i);

	void		SetEnable(int i, BOOL bEnable);
	BOOL		RetEnable(int i);

	void		SetTabs(int i, float pos, int justif=1);
	float		RetTabs(int i);

	void		ShowSelect(BOOL bFixed);

	EventMsg	RetEventMsgButton(int i);
	EventMsg	RetEventMsgScroll();

protected:
	BOOL		MoveAdjust();
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
	int			m_totalLine;	// nb total de lignes
	int			m_displayLine;	// nb de lignes visibles
	int			m_selectLine;	// ligne s�lectionn�e
	int			m_firstLine;	// premi�re ligne visible
	BOOL		m_bBlink;
	BOOL		m_bSelectCap;
	float		m_blinkTime;
	float		m_tabs[10];
	int			m_justifs[10];

	char		m_text[LISTMAXTOTAL][100];
	char		m_check[LISTMAXTOTAL];
	char		m_enable[LISTMAXTOTAL];
};


#endif //_LIST_H_
