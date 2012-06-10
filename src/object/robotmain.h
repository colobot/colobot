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

// robotmain.h

#pragma once


#include "graphics/d3d/d3dengine.h"
#include "common/struct.h"
#include "object/object.h"
#include "graphics/common/mainmovie.h"
#include "graphics/common/camera.h"
#include "graphics/common/particule.h"


enum Phase
{
	PHASE_INIT,
	PHASE_TERM,
	PHASE_NAME,
	PHASE_PERSO,
	PHASE_TRAINER,
	PHASE_DEFI,
	PHASE_MISSION,
	PHASE_FREE,
	PHASE_TEEN,
	PHASE_USER,
	PHASE_PROTO,
	PHASE_LOADING,
	PHASE_SIMUL,
	PHASE_MODEL,
	PHASE_SETUPd,
	PHASE_SETUPg,
	PHASE_SETUPp,
	PHASE_SETUPc,
	PHASE_SETUPs,
	PHASE_SETUPds,
	PHASE_SETUPgs,
	PHASE_SETUPps,
	PHASE_SETUPcs,
	PHASE_SETUPss,
	PHASE_WRITE,
	PHASE_READ,
	PHASE_WRITEs,
	PHASE_READs,
	PHASE_WIN,
	PHASE_LOST,
	PHASE_WELCOME1,
	PHASE_WELCOME2,
	PHASE_WELCOME3,
	PHASE_GENERIC,
};


class CInstanceManager;
class CMainDialog;
class CMainShort;
class CMainMap;
class CEvent;
class CD3DEngine;
class CLight;
class CWater;
class CCloud;
class CBlitz;
class CPlanet;
class CTerrain;
class CModel;
class CInterface;
class CWindow;
class CControl;
class CDisplayText;
class CDisplayInfo;
class CSound;


typedef struct
{
	D3DVECTOR	pos;
	float		dist;
	ObjectType	type;
	int			min;		// wins if>
	int			max;		// wins if <
	int			lost;		// lost if <=
	bool		bImmediat;
	char		message[100];
}
EndTake;


#define MAXNEWSCRIPTNAME	20

typedef struct
{
	bool		bUsed;
	ObjectType	type;
	char		name[40];
}
NewScriptName;


#define MAXSHOWLIMIT	5
#define MAXSHOWPARTI	200
#define SHOWLIMITTIME	20.0f

typedef struct
{
	bool			bUsed;
	D3DVECTOR		pos;
	float			radius;
	int				total;
	int				parti[MAXSHOWPARTI];
	CObject*		link;
	float			duration;
	float			time;
}
ShowLimit;


#define SATCOM_HUSTON	0
#define SATCOM_SAT		1
#define SATCOM_OBJECT	2
#define SATCOM_LOADING	3
#define SATCOM_PROG		4
#define SATCOM_SOLUCE	5
#define SATCOM_MAX		6



class CRobotMain
{
public:
	CRobotMain(CInstanceManager* iMan);
	~CRobotMain();

	void		CreateIni();

	void		ChangePhase(Phase phase);
	bool		EventProcess(const Event &event);

	bool		CreateShortcuts();
	void		ScenePerso();

	void		SetMovieLock(bool bLock);
	bool		RetMovieLock();
	bool		RetInfoLock();
	void		SetSatComLock(bool bLock);
	bool		RetSatComLock();
	void		SetEditLock(bool bLock, bool bEdit);
	bool		RetEditLock();
	void		SetEditFull(bool bFull);
	bool		RetEditFull();
	bool		RetFreePhoto();
	void		SetFriendAim(bool bFriend);
	bool		RetFriendAim();

	void		SetTracePrecision(float factor);
	float		RetTracePrecision();

	void		ChangePause(bool bPause);

	void		SetSpeed(float speed);
	float		RetSpeed();

	void		UpdateShortcuts();
	void		SelectHuman();
	CObject*	SearchHuman();
	CObject*	SearchToto();
	CObject*	SearchNearest(D3DVECTOR pos, CObject* pExclu);
	bool		SelectObject(CObject* pObj, bool bDisplayError=true);
	CObject*	RetSelectObject();
	CObject*	DeselectAll();
	bool		DeleteObject();

	void		ResetObject();
	void		ResetCreate();
	Error		CheckEndMission(bool bFrame);
	void		CheckEndMessage(char *message);
	int			RetObligatoryToken();
	char*		RetObligatoryToken(int i);
	int			IsObligatoryToken(char *token);
	bool		IsProhibitedToken(char *token);
	void		UpdateMap();
	bool		RetShowMap();

	MainMovieType RetMainMovie();

	void		FlushDisplayInfo();
	void		StartDisplayInfo(int index, bool bMovie);
	void		StartDisplayInfo(char *filename, int index);
	void		StopDisplayInfo();
	char*		RetDisplayInfoName(int index);
	int			RetDisplayInfoPosition(int index);
	void		SetDisplayInfoPosition(int index, int pos);

	void		StartSuspend();
	void		StopSuspend();

	float		RetGameTime();

	void		SetFontSize(float size);
	float		RetFontSize();
	void		SetWindowPos(FPOINT pos);
	FPOINT		RetWindowPos();
	void		SetWindowDim(FPOINT dim);
	FPOINT		RetWindowDim();

	void		SetIOPublic(bool bMode);
	bool		RetIOPublic();
	void		SetIOPos(FPOINT pos);
	FPOINT		RetIOPos();
	void		SetIODim(FPOINT dim);
	FPOINT		RetIODim();

	char*		RetTitle();
	char*		RetResume();
	char*		RetScriptName();
	char*		RetScriptFile();
	bool		RetTrainerPilot();
	bool		RetFixScene();
	bool		RetGlint();
	bool		RetSoluce4();
	bool		RetMovies();
	bool		RetNiceReset();
	bool		RetHimselfDamage();
	bool		RetShowSoluce();
	bool		RetSceneSoluce();
	bool		RetShowAll();
	bool		RetCheatRadar();
	char*		RetSavegameDir();
	char*		RetPublicDir();
	char*		RetFilesDir();

	void		SetGamerName(char *name);
	char*		RetGamerName();
	int			RetGamerFace();
	int			RetGamerGlasses();
	bool		RetGamerOnlyHead();
	float		RetPersoAngle();

	void		StartMusic();
	void		ClearInterface();
	void		ChangeColor();

	float		SearchNearestObject(D3DVECTOR center, CObject *exclu);
	bool		FreeSpace(D3DVECTOR &center, float minRadius, float maxRadius, float space, CObject *exclu);
	float		RetFlatZoneRadius(D3DVECTOR center, float maxRadius, CObject *exclu);
	void		HideDropZone(CObject* metal);
	void		ShowDropZone(CObject* metal, CObject* truck);
	void		FlushShowLimit(int i);
	void		SetShowLimit(int i, ParticuleType parti, CObject *pObj, D3DVECTOR pos, float radius, float duration=SHOWLIMITTIME);
	void		AdjustShowLimit(int i, D3DVECTOR pos);
	void		StartShowLimit();
	void		FrameShowLimit(float rTime);

	void		CompileScript(bool bSoluce);
	void		LoadOneScript(CObject *pObj, int &nbError);
	void		LoadFileScript(CObject *pObj, char* filename, int objRank, int &nbError);
	void		SaveAllScript();
	void		SaveOneScript(CObject *pObj);
	void		SaveFileScript(CObject *pObj, char* filename, int objRank);
	bool		SaveFileStack(CObject *pObj, FILE *file, int objRank);
	bool		ReadFileStack(CObject *pObj, FILE *file, int objRank);

	bool		FlushNewScriptName();
	bool		AddNewScriptName(ObjectType type, char *name);
	char*		RetNewScriptName(ObjectType type, int rank);

	void		WriteFreeParam();
	void		ReadFreeParam();

	bool		IsBusy();
	bool		IOWriteScene(char *filename, char *filecbot, char *info);
	CObject*	IOReadScene(char *filename, char *filecbot);
	void		IOWriteObject(FILE *file, CObject* pObj, char *cmd);
	CObject*	IOReadObject(char *line, char* filename, int objRank);

	int			CreateSpot(D3DVECTOR pos, D3DCOLORVALUE color);

protected:
	bool		EventFrame(const Event &event);
	bool		EventObject(const Event &event);
	void		InitEye();

	void		Convert();
	void		CreateScene(bool bSoluce, bool bFixScene, bool bResetObject);

	void		CreateModel();
	D3DVECTOR	LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );
	CObject*	CreateObject(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, float power=1.0f, bool bTrainer=false, bool bToy=false, int option=0);
	int			CreateLight(D3DVECTOR direction, D3DCOLORVALUE color);
	void		HiliteClear();
	void		HiliteObject(FPOINT pos);
	void		HiliteFrame(float rTime);
	void		CreateTooltip(FPOINT pos, char* text);
	void		ClearTooltip();
	CObject*	DetectObject(FPOINT pos);
	void		ChangeCamera();
	void		RemoteCamera(float pan, float zoom, float rTime);
	void		KeyCamera(EventMsg event, long param);
	void		AbortMovie();
	bool		IsSelectable(CObject* pObj);
	void		SelectOneObject(CObject* pObj, bool bDisplayError=true);
	void		HelpObject();
	bool		DeselectObject();
	void		DeleteAllObjects();
	void		UpdateInfoText();
	CObject*	SearchObject(ObjectType type);
	CObject*	RetSelect();
	void		StartDisplayVisit(EventMsg event);
	void		FrameVisit(float rTime);
	void		StopDisplayVisit();
	void		ExecuteCmd(char *cmd);
	bool		TestGadgetQuantity(int rank);

protected:
	CInstanceManager* m_iMan;
	CMainMovie*		m_movie;
	CMainDialog*	m_dialog;
	CMainShort*		m_short;
	CMainMap*		m_map;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CParticule*		m_particule;
	CWater*			m_water;
	CCloud*			m_cloud;
	CBlitz*			m_blitz;
	CPlanet*		m_planet;
	CLight*			m_light;
	CTerrain*		m_terrain;
	CModel*			m_model;
	CInterface*		m_interface;
	CCamera*		m_camera;
	CDisplayText*	m_displayText;
	CDisplayInfo*	m_displayInfo;
	CSound*			m_sound;

	float			m_time;
	float			m_gameTime;
	float			m_checkEndTime;
	float			m_winDelay;
	float			m_lostDelay;
	bool			m_bFixScene;		// scene fixed, no interraction
	bool			m_bBase;		// OBJECT_BASE exists in mission
	FPOINT			m_lastMousePos;
	CObject*		m_selectObject;

	Phase			m_phase;
	int				m_cameraRank;
	D3DCOLORVALUE	m_color;
	bool			m_bFreePhoto;
	bool			m_bCmdEdit;
	bool			m_bShowPos;
	bool			m_bSelectInsect;
	bool			m_bShowSoluce;
	bool			m_bShowAll;
	bool			m_bCheatRadar;
	bool			m_bAudioRepeat;
	bool			m_bShortCut;
	int				m_audioTrack;
	int				m_delayWriteMessage;
	int				m_movieInfoIndex;

	bool			m_bImmediatSatCom;	// SatCom immediately?
	bool			m_bBeginSatCom;		// messages SatCom poster?
	bool			m_bMovieLock;		// movie in progress?
	bool			m_bSatComLock;		// call of SatCom is possible?
	bool			m_bEditLock;		// edition in progress?
	bool			m_bEditFull;		// edition in full screen?
	bool			m_bPause;		// simulation paused
	bool			m_bHilite;
	bool			m_bTrainerPilot;	// remote trainer?
	bool			m_bSuspend;
	bool			m_bFriendAim;
	bool			m_bResetCreate;
	bool			m_bMapShow;
	bool			m_bMapImage;
	char			m_mapFilename[100];

	FPOINT			m_tooltipPos;
	char			m_tooltipName[100];
	float			m_tooltipTime;

	char			m_infoFilename[SATCOM_MAX][100]; // names of text files
	CObject*		m_infoObject;
	int				m_infoIndex;
	int				m_infoPos[SATCOM_MAX];
	int				m_infoUsed;

	char			m_title[100];
	char			m_resume[500];
	char			m_scriptName[100];
	char			m_scriptFile[100];
	int				m_endingWinRank;
	int				m_endingLostRank;
	bool			m_bWinTerminate;

	float			m_fontSize;
	FPOINT			m_windowPos;
	FPOINT			m_windowDim;

	bool			m_IOPublic;
	FPOINT			m_IOPos;
	FPOINT			m_IODim;

	NewScriptName	m_newScriptName[MAXNEWSCRIPTNAME];

	float			m_cameraPan;
	float			m_cameraZoom;

	EventMsg		m_visitLast;
	CObject*		m_visitObject;
	CObject*		m_visitArrow;
	float			m_visitTime;
	float			m_visitParticule;
	D3DVECTOR		m_visitPos;
	D3DVECTOR		m_visitPosArrow;

	int				m_endTakeTotal;
	EndTake			m_endTake[10];
	long			m_endTakeResearch;
	float			m_endTakeWinDelay;
	float			m_endTakeLostDelay;

	int				m_obligatoryTotal;
	char			m_obligatoryToken[100][20];
	int				m_prohibitedTotal;
	char			m_prohibitedToken[100][20];

	char			m_gamerName[100];

	long			m_freeBuild;		// constructible buildings
	long			m_freeResearch;		// researches possible

	ShowLimit		m_showLimit[MAXSHOWLIMIT];

	D3DCOLORVALUE	m_colorRefBot;
	D3DCOLORVALUE	m_colorNewBot;
	D3DCOLORVALUE	m_colorRefAlien;
	D3DCOLORVALUE	m_colorNewAlien;
	D3DCOLORVALUE	m_colorRefGreen;
	D3DCOLORVALUE	m_colorNewGreen;
	D3DCOLORVALUE	m_colorRefWater;
	D3DCOLORVALUE	m_colorNewWater;
	float			m_colorShiftWater;
};

