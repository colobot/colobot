// robotmain.h

#ifndef _ROBOTMAIN_H_
#define	_ROBOTMAIN_H_



enum Phase
{
	PHASE_INIT,
	PHASE_TERM,
	PHASE_NAME,
	PHASE_PERSO,
	PHASE_MISSION,
	PHASE_FREE,
	PHASE_USER,
	PHASE_PROTO,
	PHASE_LOADING,
	PHASE_SIMUL,
	PHASE_CAR,
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
	PHASE_WIN,
	PHASE_LOST,
	PHASE_WELCOME1,
	PHASE_WELCOME2,
	PHASE_WELCOME3,
	PHASE_GENERIC1,
	PHASE_GENERIC2,
	PHASE_GENERIC3,
	PHASE_GENERIC4,
	PHASE_GENERIC5,
	PHASE_WRITE,
	PHASE_READ,
};

enum StarterType
{
	STARTER_NONE	= 0,
	STARTER_321		= 1,
	STARTER_QUICK	= 2,
};


class CInstanceManager;
class CMainMovie;
class CMainDialog;
class CMainShort;
class CMainMap;
class CEvent;
class CD3DEngine;
class CLight;
class CParticule;
class CWater;
class CCloud;
class CBlitz;
class CPlanet;
class CTerrain;
class CObject;
class CModel;
class CCamera;
class CInterface;
class CWindow;
class CControl;
class CDisplayText;
class CSound;
class CRecorder;
class CAuto;

enum ObjectType;
enum CameraType;
enum MainMovieType;
enum ParticuleType;
enum Sound;


typedef struct
{
	D3DVECTOR	pos;
	float		dist;
	ObjectType	type;
	int			min;		// gagné si >
	int			max;		// gagné si <
	int			lost;		// perdu si <=
	int			show;		// affiche ?
	int			last;
	BOOL		bImmediat;
}
EndTake;

typedef struct
{
	D3DVECTOR	eye;		// oeil caméra
	D3DVECTOR	look;		// visée caméra
	float		delay;		// durée du pas
	float		progress;	// progression de pas
	Sound		sound;		// son a générer
}
MovieStep;


#define MAXSHOWPARTI	200


class CRobotMain
{
public:
	CRobotMain(CInstanceManager* iMan);
	~CRobotMain();

	void		CreateIni();

	void		ChangePhase(Phase phase);
	Phase		RetPhase();
	BOOL		EventProcess(const Event &event);

	void		ScenePerso();

	void		SetMovieLock(BOOL bLock);
	BOOL		RetMovieLock();
	void		SetMovieAuto(CAuto *automat);
	BOOL		RetFreePhoto();
	void		SetFriendAim(BOOL bFriend);
	BOOL		RetFriendAim();
	void		SetSuperWin();
	void		SetSuperLost();
	void		SetSuperLost(D3DVECTOR lookat);

	void		ChangePause(BOOL bPause);

	void		SetSpeed(float speed);
	float		RetSpeed();

	CObject*	SearchObject(ObjectType type);
	CObject*	SearchHuman();
	BOOL		SelectObject(CObject* pObj, BOOL bDisplayError=TRUE);
	CObject*	RetSelectObject();
	CObject*	DeselectAll();
	BOOL		DeleteObject();

	Error		CheckEndMission(BOOL bFrame);
	int			RetLapProgress();
	ObjectType	RetTypeProgress();
	BOOL		IncProgress();
	void		UpdateMap();
	BOOL		RetShowMap();

	MainMovieType RetMainMovie();

	void		StartDisplayInfo();
	void		StopDisplayInfo();

	void		ShowHideButtons(BOOL bShow);
	void		StartSuspend();
	void		StopSuspend();

	void		SetStopwatch(BOOL bRun);
	void		StopStartCounter();
	BOOL		IsStartCounter();
	BOOL		IsStarter();
	BOOL		IsGameTime();
	BOOL		IsRecordTime();
	BOOL		RetEndingGame();
	float		RetGameTime();
	float		RetRecordTime();

	void		SetFontSize(float size);
	float		RetFontSize();
	void		SetWindowPos(FPOINT pos);
	FPOINT		RetWindowPos();
	void		SetWindowDim(FPOINT dim);
	FPOINT		RetWindowDim();

	void		SetIOPublic(BOOL bMode);
	BOOL		RetIOPublic();
	void		SetIOPos(FPOINT pos);
	FPOINT		RetIOPos();
	void		SetIODim(FPOINT dim);
	FPOINT		RetIODim();

	char*		RetTitle();
	char*		RetResume();
	BOOL		RetTrainerPilot();
	BOOL		RetFixScene();
	BOOL		RetGlint();
	BOOL		RetMovies();
	BOOL		RetNiceReset();
	BOOL		RetHimselfDamage();
	BOOL		RetMotorBlast();
	BOOL		RetShowAll();
	BOOL		RetCheatRadar();
	char*		RetSavegameDir();
	char*		RetPublicDir();
	char*		RetFilesDir();
	int			RetLevel();

	void		SetGamerName(char *name);
	char*		RetGamerName();

	void		FlushCarState();
	void		MemorizeCarState();
	char*		GetCarState();

	void		StartMusic();
	void		ClearInterface();
	void		ChangeColor();
	void		ChangeColorCar(int model, int subModel, D3DCOLORVALUE color);

	float		SearchNearestObject(D3DVECTOR center, CObject *exclu);
	BOOL		FreeSpace(D3DVECTOR &center, float minRadius, float maxRadius, float space, CObject *exclu);
	float		RetFlatZoneRadius(D3DVECTOR center, float maxRadius, CObject *exclu);

	void		CompileScript();

	void		InfoCollision(ObjectType type);

	void		FlushStarterType();
	StarterType	RetStarterType();
	int			RetRaceType();
	int			RetHornAction();

	void		RecorderWrite(int rank, int model);
	void		RecorderRead(int rank, int model);
	BOOL		RecorderExist(int rank, int model);
	CRecorder*	RetRecorderRecord();

	void		IncDecorStamp();
	int			RetDecorStamp();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		EventObject(const Event &event);
	void		InitEye();
	void		InitEye(CObject *pObj);

	void		Convert();
	void		CreateScene(BOOL bFixScene);

	void		ChangeTabOrder(int dir, int param);
	void		CreateModel();
	D3DVECTOR	LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );
	CObject*	CreateObject(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, int bPlumb=-1, BOOL bTrainer=FALSE, int option=0);
	void		CreateLimit(ObjectType type, int max);
	int			CreateLight(D3DVECTOR direction, D3DCOLORVALUE color);
	int			CreateSpot(D3DVECTOR pos, D3DCOLORVALUE color);
	void		CreateTooltip(FPOINT pos, char* text);
	void		ClearTooltip();
	void		ChangeCamera();
	void		AbortMovie();
	void		SelectOneObject(CObject* pObj, BOOL bDisplayError=TRUE);
	BOOL		DeselectObject();
	void		DeleteAllObjects();
	void		UpdateInfoText();
	CObject*	RetSelect();
	int			RetVehiclePart();
	int			RetConeTotal();
	float		RetBonusPoints();
	void		ExecuteCmd(char *cmd);
	BOOL		TestGadgetQuantity(int rank);
	void		UpdateProgress();
	void		UpdateTime();
	void		HiliteClear();
	void		HiliteObject(FPOINT pos);
	void		HiliteFrame(float rTime);
	void		MovieStart();
	void		MovieFrame(float rTime);
	void		MovieAbort();

protected:
	CInstanceManager* m_iMan;
	CMainMovie*		m_movie;
	CMainDialog*	m_dialog;
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
	CSound*			m_sound;

	float			m_time;
	float			m_gameTime;
	float			m_recordTime;
	float			m_checkEndTime;
	float			m_winDelay;
	float			m_lostDelay;
	BOOL			m_bFixScene;	// scène fixe, sans interraction
	CObject*		m_selectObject;
	char*			m_carState;		// état du véhicule
	char			m_sceneName[20];

	Phase			m_phase;
	int				m_cameraRank;
	D3DCOLORVALUE	m_color;
	BOOL			m_bFreePhoto;
	BOOL			m_bCmdEdit;
	BOOL			m_bShowPos;
	BOOL			m_bSelectInsect;
	BOOL			m_bShowAll;
	BOOL			m_bCheatRadar;
	BOOL			m_bAudioRepeat;
	BOOL			m_bSuperWin;
	BOOL			m_bSuperLost;
	BOOL			m_bLostLookat;
	D3DVECTOR		m_lostLookat;
	int				m_geiger;
	int				m_audioTrack;
	int				m_movieInfoIndex;
	int				m_decorStamp;
	FPOINT			m_lastMousePos;

	BOOL			m_bMovieLock;		// film en cours ?
	BOOL			m_bPause;			// simulation en pause
	BOOL			m_bTrainerPilot;	// télécommande trainer ?
	BOOL			m_bSuspend;
	BOOL			m_bFriendAim;
	BOOL			m_bSkipFrame;
	BOOL			m_bDisplayInfo;
	BOOL			m_bStopwatch;
	BOOL			m_bCheatUsed;
	CAuto*			m_movieAuto;

	FPOINT			m_tooltipPos;
	char			m_tooltipName[100];
	float			m_tooltipTime;

	CObject*		m_infoObject;

	char			m_title[100];
	char			m_resume[500];
	int				m_endingWinRank;
	int				m_endingLostRank;
	BOOL			m_bWinTerminate;

	float			m_fontSize;
	FPOINT			m_windowPos;
	FPOINT			m_windowDim;

	BOOL			m_IOPublic;
	FPOINT			m_IOPos;
	FPOINT			m_IODim;

	int				m_endTakeTotal;
	EndTake			m_endTake[10];
	long			m_endTakeResearch;
	float			m_endTakeWinDelay;
	float			m_endTakeLostDelay;

	int				m_movieTotal;
	int				m_movieIndex;
	MovieStep		m_movieTable[20];
	CameraType		m_movieType;

	int				m_progressTotal;
	int				m_progressLap;
	int				m_progressAdd;
	int				m_progressLevel;
	ObjectType		m_progressType;

	float			m_bonusLimit;
	float			m_bonusRecord;
	int				m_maxPesetas;

	int				m_startCounter;
	float			m_startDelay;

	char			m_gamerName[100];

	long			m_freeBuild;		// bâtiments constructibles
	long			m_freeResearch;		// recherches effectuées

	int				m_statStartVehicle;
	int				m_statEndVehicle;
	int				m_statStartCone;
	int				m_statEndCone;
	float			m_statStartTime;
	float			m_statEndTime;
	float			m_statLapTime;
	float			m_statBestTime;
	BOOL			m_bStatRecordAll;
	BOOL			m_bStatRecordOne;

	int				m_starterTry;		// nb d'animation déjà effectuées
	int				m_starterLast;		// dernière mission jouée avec starter
	int				m_raceType;
	int				m_hornAction;

	D3DCOLORVALUE	m_colorRefVeh;
	D3DCOLORVALUE	m_colorNewVeh;
	D3DCOLORVALUE	m_colorRefBot;
	D3DCOLORVALUE	m_colorNewBot;
	D3DCOLORVALUE	m_colorRefAlien;
	D3DCOLORVALUE	m_colorNewAlien;
	D3DCOLORVALUE	m_colorRefGreen;
	D3DCOLORVALUE	m_colorNewGreen;
	D3DCOLORVALUE	m_colorRefWater;
	D3DCOLORVALUE	m_colorNewWater;
	float			m_colorShiftWater;

	CRecorder*		m_recorderRecord;
	CRecorder*		m_recorderPlay;

	int				m_repeat;			// 1 -> répétition d'une mission
	int				m_lastRank;			// dernière mission jouée

	float			m_messageTime;
	Sound			m_messageSound;
	char			m_messageText[100];
	float			m_messageDelay;
	float			m_messageSize;
};


#endif //_ROBOTMAIN_H_
