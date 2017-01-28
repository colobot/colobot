// robotmain.h

#ifndef _ROBOTMAIN_H_
#define	_ROBOTMAIN_H_



enum Phase
{
	PHASE_NULL,
	PHASE_INIT,
	PHASE_TERM,
	PHASE_NAME,
	PHASE_PERSO,
	PHASE_PUZZLE,
	PHASE_DEFI,
	PHASE_USER,
	PHASE_PROTO,
	PHASE_FADEIN,
	PHASE_LOADING,
	PHASE_SIMUL,
	PHASE_MODELe,
	PHASE_MODELi,
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

enum Scene
{
	SCENE_NULL,
	SCENE_FIX,
	SCENE_SIMUL,
};


class CInstanceManager;
class CMainDialog;
class CMainEdit;
class CMainUndo;
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
class CAuto;
class CFileBuffer;

enum ObjectType;
enum CameraType;
enum ParticuleType;
enum Sound;
enum D3DMouse;




#define MAXSHOWPARTI	200


class CRobotMain
{
public:
	CRobotMain(CInstanceManager* iMan);
	~CRobotMain();

	void		CreateIni();

	BOOL		EventProcess(const Event &event);
	void		ChangePhase(Phase phase, Phase fadeIn=(Phase)0);

	void		SetMovieLock(BOOL bLock, BOOL bFinal);
	BOOL		RetMovieLock();
	BOOL		RetFreePhoto();
	void		SetFriendAim(BOOL bFriend);
	BOOL		RetFriendAim();

	void		ChangePause(BOOL bPause);

	void		SetSpeed(float speed);
	float		RetSpeed();

	BOOL		SelectObject(CObject* pObj, BOOL bDisplayError=TRUE);
	CObject*	RetSelect();
	void		SetSelect(CObject* pSel);
	CObject*	DeselectAll();
	void		SelectOther(CObject* pSel);
	BOOL		DeleteObject();
	BOOL		MagicDetect(FPOINT mouse, D3DVECTOR &pos, CObject* &pObj, int &part);
	BOOL		DetectObject(FPOINT pos, CObject* &pObj, int &part);
	BOOL		DetectObject(FPOINT pos, BOOL bAction, CObject* &pObj, int &part);

	Error		CheckEndMission(BOOL bFrame);
	void		UpdateMap();
	BOOL		RetShowMap();

	void		StartDisplaySign();
	void		StartDisplayInfo(char *text);
	void		StopDisplayInfo();

	void		StartSuspend();
	void		StopSuspend();

	BOOL		IsGameTime();
	float		RetGameTime();

	void		SetFontSize(float size);
	float		RetFontSize();
	void		SetWindowPos(FPOINT pos);
	FPOINT		RetWindowPos();
	void		SetWindowDim(FPOINT dim);
	FPOINT		RetWindowDim();

	void		SetResume(int language, char *text);
	char*		RetResume(int language);
	char*		RetResume();
	void		SetSign(int language, char *text);
	char*		RetSign(int language);
	char*		RetSign();
	void		SetAuthor(char *text);
	char*		RetAuthor();
	BOOL		RetFixScene();
	int			RetHandleMove();
	BOOL		RetEdit();
	BOOL		RetTest();
	BOOL		RetProto();
	BOOL		RetAgain();
	BOOL		RetShowAll();
	BOOL		RetCheatRadar();
	char*		RetSavegameDir();
	char*		RetPublicDir();
	char*		RetFilesDir();
	int			RetLevel();
	int			RetTotalBlupi();
	int			RetTotalGoal();
	BOOL		RetCheatUsed();
	void		UpdateInterface();
	int			RetSuperShift19();

	void		SetGamerName(char *name);
	char*		RetGamerName();

	void		IncTotalManip(int manip=1);
	void		SetTotalManip(int manip);
	int			RetTotalManip();

	void		StartMusic();
	void		ClearInterface();
	void		ChangeColor();

	void		InitEye();
	void		MouseMoveClear();

	int			RetAdditional(int rank, int type);

	int			CheckPuzzle();
	BOOL		WriteScene();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		EventObject(const Event &event);

	void		CreateScene(BOOL bFixScene);

	void		StopAction();
	BOOL		FilterMousePos(FPOINT pos);
	void		ChangeTabOrder(int dir, int param);
	void		CreateModel(int type);
	CObject*	CreateObject(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, int option=0, int addHat=0, int addGlass=0, int addGlove=0, int addShoe=0, int addBag=0);
	void		CreateLimit(ObjectType type, int max);
	void		CreateSunbeam(int total, float min, float max, float desym, ParticuleType type);
	int			CreateLight(D3DVECTOR direction, D3DCOLORVALUE color);
	int			CreateSpot(D3DVECTOR pos, D3DCOLORVALUE color);
	void		CreateTooltip(FPOINT pos, char* text);
	void		ClearTooltip();
	void		AbortMovie();
	void		SelectBegin(FPOINT mouse);
	void		SelectMove(FPOINT mouse);
	void		SelectEnd(FPOINT mouse);
	void		ScrollBegin(FPOINT mouse);
	void		ScrollMove(FPOINT mouse);
	BOOL		ScrollEnd(FPOINT mouse);
	void		GroupOper(FPOINT mouse);
	void		SelectOneObject(CObject* pObj, BOOL bDisplayError=TRUE);
	BOOL		DeselectObject();
	BOOL		IsSelectable(CObject* pObj);
	void		DeleteAllObjects();
	void		UpdateInfoText();
	void		ExecuteCmd(char *cmd);
	void		UpdateTime();
	void		HiliteClear();
	void		HiliteObject(FPOINT pos);
	void		HiliteFrame(float rTime);
	void		StartLift();
	CObject*	SearchObject(D3DVECTOR center, float radius, BOOL bAll);
	void		ReadSuperShift();
	void		Crypt(char *dir, BOOL bCrypt);
	BOOL		CryptFile(char *filename, BOOL bCrypt);

protected:
	CInstanceManager* m_iMan;
	CMainDialog*	m_dialog;
	CMainEdit*		m_edit;
	CMainUndo*		m_undo;
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
	CFileBuffer*	m_fileBuffer;

	float			m_time;
	float			m_gameTime;
	float			m_checkEndTime;
	float			m_winDelay;
	float			m_lostDelay;
	float			m_firstInfoDelay;
	Scene			m_scene;
	int				m_handleMode;
	int				m_adviseMode;
	CObject*		m_selectObject;
	char			m_sceneName[20];
	D3DMouse		m_defMouse;
	int				m_undoCounter;
	int				m_stopCounter;
	BOOL			m_typeButton;
	int				m_totalBlupi;
	int				m_totalGoal;
	BYTE			m_superShift[256];
	int				m_superShift19;
	int				m_lastSuperShift19;
	int				m_totalManip;

	Phase			m_phase;
	Phase			m_lastPhase;
	BOOL			m_bFreePhoto;
	BOOL			m_bCmdEdit;
	BOOL			m_bShowPos;
	BOOL			m_bShowAll;
	BOOL			m_bAudioRepeat;
	int				m_audioTrack;
	FPOINT			m_lastMousePos;
	FPOINT			m_infoPos;
	FPOINT			m_infoDim;

	BOOL			m_bMovieLock;		// film en cours ?
	BOOL			m_bMovieFinal;		// film de fin (envol)
	BOOL			m_bPause;			// simulation en pause
	BOOL			m_bHilite;
	BOOL			m_bSuspend;
	BOOL			m_bFriendAim;
	BOOL			m_bSkipFrame;
	BOOL			m_bDisplayInfo;
	BOOL			m_bCheatUsed;

	BOOL			m_bSelect;
	FPOINT			m_selectBegin;
	FPOINT			m_selectEnd;
	BOOL			m_bScroll;
	FPOINT			m_scrollBegin;
	FPOINT			m_scrollEnd;
	int				m_scrollQuantity;

	D3DVECTOR		m_terrainSpritePos;
	int				m_terrainSpriteChannel;

	FPOINT			m_tooltipPos;
	char			m_tooltipName[100];
	float			m_tooltipTime;

	CObject*		m_infoObject;

	char			m_resume[26][1000];
	char			m_sign[26][1000];
	char			m_author[100];
	int				m_endingWinRank;
	int				m_endingLostRank;

	float			m_fontSize;
	FPOINT			m_windowPos;
	FPOINT			m_windowDim;

	char			m_gamerName[100];

	D3DCOLORVALUE	m_colorRefBot;
	D3DCOLORVALUE	m_colorNewBot;
	D3DCOLORVALUE	m_colorRefGreen;
	D3DCOLORVALUE	m_colorNewGreen;
	D3DCOLORVALUE	m_colorRefWater;
	D3DCOLORVALUE	m_colorNewWater;
	float			m_colorShiftWater;

	int				m_repeat;			// 1 -> répétition d'une mission
	char			m_lastFilename[100]; // dernière mission jouée

	int				m_additionalTotal;
	int				m_additional[10][10];	// accessoires
};


#endif //_ROBOTMAIN_H_
