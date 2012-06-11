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

// brain.h

#pragma once


#include "common/misc.h"
#include "common/event.h"
#include "object/object.h"
#include "object/task/taskmanip.h"
#include "object/task/taskflag.h"
#include "object/task/taskshield.h"


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CWater;
class CCamera;
class CObject;
class CPhysics;
class CMotion;
class CTaskManager;
class CInterface;
class CWindow;
class CDisplayText;
class CScript;
class CRobotMain;
class CStudio;
class CSound;
class CParticule;


const int BRAINMAXSCRIPT = 10;



enum TraceOper
{
	TO_STOP			= 0,	// stop
	TO_ADVANCE		= 1,	// advance
	TO_RECEDE		= 2,	// back
	TO_TURN			= 3,	// rotate
	TO_PEN			= 4,	// color change
};

struct TraceRecord
{
	TraceOper	oper;
	float		param;
};



class CBrain
{
public:
	CBrain(CInstanceManager* iMan, CObject* object);
	~CBrain();

	void		DeleteObject(bool bAll=false);

	void		SetPhysics(CPhysics* physics);
	void		SetMotion(CMotion* motion);

	bool		EventProcess(const Event &event);
	bool		CreateInterface(bool bSelect);

	bool		Write(char *line);
	bool		Read(char *line);

	bool		IsBusy();
	void		SetActivity(bool bMode);
	bool		RetActivity();
	bool		IsProgram();
	bool		ProgramExist(int rank);
	void		RunProgram(int rank);
	int			FreeProgram();
	int			RetProgram();
	void		StopProgram();
	void		StopTask();

	bool		IntroduceVirus();
	void		SetActiveVirus(bool bActive);
	bool		RetActiveVirus();

	void		SetScriptRun(int rank);
	int			RetScriptRun();
	void		SetScriptName(int rank, char *name);
	char*		RetScriptName(int rank);
	void		SetSoluceName(char *name);
	char*		RetSoluceName();

	bool		ReadSoluce(char* filename);
	bool		ReadProgram(int rank, char* filename);
	bool		RetCompile(int rank);
	bool		WriteProgram(int rank, char* filename);
	bool		ReadStack(FILE *file);
	bool		WriteStack(FILE *file);

	Error		StartTaskTake();
	Error		StartTaskManip(TaskManipOrder order, TaskManipArm arm);
	Error		StartTaskFlag(TaskFlagOrder order, int rank);
	Error		StartTaskBuild(ObjectType type);
	Error		StartTaskSearch();
	Error		StartTaskTerraform();
	Error		StartTaskPen(bool bDown, int color);
	Error		StartTaskRecover();
	Error		StartTaskShield(TaskShieldMode mode);
	Error		StartTaskFire(float delay);
	Error		StartTaskFireAnt(D3DVECTOR impact);
	Error		StartTaskGunGoal(float dirV, float dirH);
	Error		StartTaskReset(D3DVECTOR goal, D3DVECTOR angle);

	void		UpdateInterface(float rTime);
	void		UpdateInterface();

protected:
	bool		EventFrame(const Event &event);

	void		StartEditScript(int rank, char* name);
	void		StopEditScript(bool bCancel);

	Error		EndedTask();

	void		GroundFlat();
	void		ColorFlag(int color);

	void		UpdateScript(CWindow *pw);
	int			RetSelScript();
	void		BlinkScript(bool bEnable);

	void		CheckInterface(CWindow *pw, EventMsg event, bool bState);
	void		EnableInterface(CWindow *pw, EventMsg event, bool bState);
	void		DeadInterface(CWindow *pw, EventMsg event, bool bState);
	void		DefaultEnter(CWindow *pw, EventMsg event, bool bState=true);

	void		TraceRecordStart();
	void		TraceRecordFrame();
	void		TraceRecordStop();
	bool		TraceRecordOper(TraceOper oper, float param);
	bool		TraceRecordPut(char *buffer, int max, TraceOper oper, float param);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CObject*		m_object;
	CPhysics*		m_physics;
	CMotion*		m_motion;
	CInterface*		m_interface;
	CDisplayText*	m_displayText;
	CRobotMain*		m_main;
	CStudio*		m_studio;
	CSound*			m_sound;
	CParticule*		m_particule;
	CTaskManager*	m_primaryTask;
	CTaskManager*	m_secondaryTask;

	CScript*	m_script[BRAINMAXSCRIPT];
	int			m_selScript;		// rank of the selected script
	int			m_program;		// rank of the executed program / ​​-1
	bool		m_bActivity;
	bool		m_bBurn;
	bool		m_bActiveVirus;

	int			m_scriptRun;
	char		m_scriptName[BRAINMAXSCRIPT][50];
	char		m_soluceName[50];

	EventMsg	m_buttonAxe;
	EventMsg	m_manipStyle;
	EventMsg	m_defaultEnter;
	EventMsg	m_interfaceEvent[100];

	CObject*	m_antTarget;
	CObject*	m_beeBullet;
	float		m_beeBulletSpeed;
	D3DVECTOR	m_startPos;
	float		m_time;
	float		m_burnTime;
	float		m_lastUpdateTime;
	float		m_lastHumanTime;
	float		m_lastSpiderTime;
	float		m_lastWormTime;
	float		m_lastBulletTime;
	float		m_lastAlarmTime;
	int			m_soundChannelAlarm;
	int			m_flagColor;

	bool		m_bTraceRecord;
	TraceOper	m_traceOper;
	D3DVECTOR	m_tracePos;
	float		m_traceAngle;
	int			m_traceColor;
	int			m_traceRecordIndex;
	TraceRecord* m_traceRecordBuffer;
};

