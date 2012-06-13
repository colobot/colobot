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

// studio.h

#pragma once


#include "object/object.h"
#include "script/script.h"


class CInstanceManager;
class CD3DEngine;
class CEvent;
class CRobotMain;
class CCamera;
class CSound;
class CInterface;
class CScript;
class CList;
class CEdit;



enum StudioDialog
{
	SD_NULL,
	SD_OPEN,
	SD_SAVE,
	SD_FIND,
	SD_REPLACE,
};



class CStudio
{
public:
	CStudio(CInstanceManager* iMan);
	~CStudio();

	bool		EventProcess(const Event &event);

	void		StartEditScript(CScript *script, char* name, int rank);
	bool		StopEditScript(bool bCancel);

protected:
	bool		EventFrame(const Event &event);
	void		SearchToken(CEdit* edit);
	void		ColorizeScript(CEdit* edit);
	void		AdjustEditScript();
	void		SetInfoText(char *text, bool bClickable);
	void		ViewEditScript();
	void		UpdateFlux();
	void		UpdateButtons();

	void		StartDialog(StudioDialog type);
	void		StopDialog();
	void		AdjustDialog();
	bool		EventDialog(const Event &event);
	void		UpdateChangeList();
	void		UpdateChangeEdit();
	void		UpdateDialogAction();
	void		UpdateDialogPublic();
	void		UpdateDialogList();
	void		SearchDirectory(char *dir, bool bCreate);
	bool		ReadProgram();
	bool		WriteProgram();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*	m_engine;
	CEvent*		m_event;
	CRobotMain*	m_main;
	CCamera*	m_camera;
	CSound*		m_sound;
	CInterface*	m_interface;

	int			m_rank;
	CScript*	m_script;

	bool		m_bEditMaximized;
	bool		m_bEditMinimized;

	CameraType	m_editCamera;
	Math::Point		m_editActualPos;
	Math::Point		m_editActualDim;
	Math::Point		m_editFinalPos;
	Math::Point		m_editFinalDim;

	float		m_time;
	float		m_fixInfoTextTime;
	bool		m_bRunning;
	bool		m_bRealTime;
	bool		m_bInitPause;
	char		m_helpFilename[100];

	StudioDialog m_dialog;
};


