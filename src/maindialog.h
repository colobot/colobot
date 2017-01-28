// maindialog.h

#ifndef _MAINDIALOG_H_
#define	_MAINDIALOG_H_



class CInstanceManager;
class CRobotMain;
class CEvent;
class CD3DEngine;
class CInterface;
class CWindow;
class CControl;
class CParticule;
class CCamera;
class CSound;

enum Phase;
enum CameraType;


#define USERLISTMAX		100
#define GHOSTLISTMAX	100
#define MAXSCENE		1000

typedef struct
{
	char	numTry;
	char	bPassed;
}
SceneInfo;

typedef struct
{
	char	chap;
	char	scene;
}
ListInfo;

enum StateCar
{
	SC_SHOWCASE,	// voiture en vitrine
	SC_OWNER,		// voiture acquise
	SC_FORSALE,		// voiture à vendre
};

typedef struct
{
	int				total;			// nb de voitures achetées
	int				buyable;		// nb de voitures achetables
	int				bonus;			// nb de voitures dispo en bonus
	int				selectCar;		// voiture choisie (0..n)
	int				usedCars[50];	// voitures achetées (1..n)
	int				subModel[50];	// peintures utilisées (1..n)
	D3DCOLORVALUE	colorBody[50];	// couleurs carrosseries
	StateCar		stateCars[50];	// états des voitures (pour _SE)
	int				pesetas;		// argent disponible
	int				level;			// niveau de difficulté (1..3)
}
GamerPerso;


#define MAXRECORD	10

typedef struct
{
	char		gamerName[20];		// nom du joueur
	float		time;				// temps effectué
	float		bonus, value1, value2;
	int			points;
	short		model;				// modèle de la voiture
	short		subModel;			// sous-modèle de la voiture
	short		reserve;
}
RecordInfo;

typedef struct
{
	int			version;			// type d'enregistrement
	int			reserve;
	int			select;				// record à sélectionner
	int			total;				// nb de records
	RecordInfo	list[MAXRECORD];	// liste des records
}
RecordList;

typedef struct
{
	float		minSpeed;			// vitesse minimale
	float		maxSpeed;			// vitesse maximale
}
CarSpec;

typedef struct
{
	char		filename[100];
	char		title[100];
	char		gamer[20];
	int			type;
	int			mission;
	int			model;
	float		chrono;
}
GhostFile;



class CMainDialog
{
public:
	CMainDialog(CInstanceManager* iMan);
	~CMainDialog();

	BOOL	EventProcess(const Event &event);
	void	ChangePhase(Phase phase);

	void	LaunchSimul();
	void	SetSceneName(char* name);
	void	SetSceneRank(int rank);
	char*	RetSceneName();
	char*	RetSceneBase();
	int		RetSceneRank();
	char*	RetSceneDir();
	char*	RetSavegameDir();
	char*	RetPublicDir();
	BOOL	RetDuel();

	BOOL	RetTooltip();
	BOOL	RetGlint();
	BOOL	RetMovies();
	BOOL	RetNiceReset();
	BOOL	RetHimselfDamage();
	BOOL	RetMotorBlast();

	CameraType	RetDefCamera();
	void		SetDefCamera(CameraType type);

	void	BuildSceneName(char *filename, char *base, int rank);
	void	BuildResumeName(char *filename, char *base, int rank);
	char*	RetFilesDir();
	BOOL	ReadGhostMode(int rank);
	void	ReadCarSpec(int rank, CarSpec &spec);
	int		ReadPesetasNext();
	int		ReadPesetasLimit(int rank);
	void	UpdateScenePesetasPerso();
	void	UpdateScenePesetasMax(int rank);
	void	UpdateSceneResume(int rank);
	void	UpdateSceneResume(int rank, CEdit* pe);
	BOOL	ReadSceneTitle(char *scene, int rank, char *buffer);
	void	UpdateSceneImage(int rank);
	void	UpdateScenePlay(int rank);
	void	UpdateSceneGhost(int rank);

	void	StartAbort();
	void	StartChoiceLevel();
	void	StartCreateGame();
	void	StartDeleteGame(char *gamer);
	void	StartDeleteFile(char *filename);
	void	StartKidLevel();
	void	StartQuit();
	void	StartDialog(FPOINT dim, BOOL bFire, BOOL bOK, BOOL bCancel);
	void	FrameDialog(float rTime);
	void	StopDialog();
	BOOL	IsDialog();

	void	StartSuspend();
	void	StopSuspend();

	void	SetupMemorize();
	void	SetupRecall();

	BOOL	ReadGamerInfo();
	BOOL	WriteGamerInfo();
	BOOL	ReadGamerMission();
	BOOL	WriteGamerMission();
	D3DCOLORVALUE RetGamerColorCar();
	void	SetGamerInfoTry(int rank, int numTry);
	int		RetGamerInfoTry(int rank);
	void	SetGamerInfoPassed(int rank, BOOL bPassed);
	BOOL	RetGamerInfoPassed(int rank);
	void	NextMission();
	void	PesetasUnlock();
	void	SetPesetas(int value);
	int		RetPesetas();
	BOOL	RetGhost();
	BOOL	RetGhostExist();
	int		RetLevel();
	void	BuyAllPerso();

	int		RetModel();
	int		RetSubModel();

	void	AllMissionUpdate();

	BOOL	ReadRecord(int rank, int type);
	BOOL	WriteRecord(int rank, int type);
	BOOL	AddRecord(float time, float bonus, float value1, float value2, int points, int type);
	float	TimeRecord(float time, int type);
	BOOL	FirstRecord(int type);
	void	UpdateRecord(CArray *pa, int type);
	char*	RetGhostRead();

	BOOL	ComputeCheck(int rank, int check[]);

protected:
	void	ChangeTabOrder(EventMsg window, int dir, int param);
	void	GlintMove();
	void	FrameParticule(float rTime);
	void	NiceParticule(FPOINT mouse, BOOL bPress);
	void	ReadNameList();
	void	UpdateNameControl();
	BOOL	NameSelect();
	BOOL	NameCreate();
	void	NameDelete();
	void	LevelSelect(int level);
	void	UpdatePerso();
	void	ElevationPerso();
	void	CameraPerso();
	BOOL	IsBuyablePerso();
	void	BuyablePerso();
	void	NextPerso(int dir);
	void	SelectPerso();
	void	BuyPerso();
	int		RetPersoModel(int rank);
	int		IndexPerso(int model);
	BOOL	UsedPerso(int model);
	int		PricePerso(int model);
	void	NamePerso(char *buffer, int model);
	void	SpecPerso(CarSpec &spec, int model);
	void	FixPerso(int rank);
	void	FlushPerso();
	void	DefPerso(int rank);
	void	UpdateSceneList(int &sel);
	void	UpdateDisplayDevice();
	void	UpdateDisplayMode();
	void	ChangeDisplay();
	void	UpdateApply();
	void	UpdateSetupButtons();
	void	ChangeSetupButtons();
	void	ChangeSetupQuality(int quality);
	void	UpdateKey();
	void	ChangeKey(EventMsg event);
	BOOL	MissionExist(int rank);
	void	UpdateGhostList(BOOL bAll);
	void	SelectGhostList();
	int		WriteGhostFile();
	BOOL	DeleteGhostFile();
	BOOL	ReadGhostFile();

protected:
	CInstanceManager* m_iMan;
	CRobotMain*		m_main;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CParticule*		m_particule;
	CCamera*		m_camera;
	CSound*			m_sound;

	Phase			m_phase;			// copie de CRobotMain
	Phase			m_phaseSetup;		// onglet choisi
	Phase			m_phaseTerm;		// phase trainer/scene/proto
	Phase			m_phasePerso;		// phase trainer/scene/proto
	float			m_phaseTime;

	GamerPerso		m_perso;			// perso: description
	GamerPerso		m_persoCopy;		// perso: copie si annulation
	float			m_persoElevation;	// perso: élévation caméra
	float			m_persoAngle;		// perso: angle de présentation
	float			m_persoTime;		// perso: temps absolu
	BOOL			m_persoRun;			// perso: moteur tourne ?

	char			m_sceneDir[_MAX_FNAME];		// dossier scene\ 
	char			m_savegameDir[_MAX_FNAME];	// dossier savegame\ 
	char			m_publicDir[_MAX_FNAME];	// dossier program\ 
	char			m_userDir[_MAX_FNAME];		// dossier user\ 
	char			m_filesDir[_MAX_FNAME];		// dossier files\ 
	char			m_duelDir[_MAX_FNAME];		// dossier duel\ 

	int				m_index;			// 0..5
	int				m_sel[6];			// mission choisie (0..99)
	char			m_sceneName[20];	// nom de la scène à jouer
	char			m_sceneBase[20];	// nom de la scène à jouer
	int				m_sceneRank;		// rang de la scène à jouer
	BOOL			m_bSimulSetup;		// réglages pendant le jeu
	BOOL			m_accessEnable;
	BOOL			m_accessMission;
	BOOL			m_accessUser;
	BOOL			m_bDeleteGamer;
	BOOL			m_bGhostExist;
	BOOL			m_bGhostEnable;
	BOOL			m_bPesetas;

	int				m_userTotal;
	char			m_userList[USERLISTMAX][100];

	int				m_shotDelay;		// nb de frames avant copie
	char			m_shotName[100];	// nom du fichier à générer

	int				m_setupSelDevice;
	int				m_setupSelMode;
	BOOL			m_setupFull;

	BOOL			m_bTooltip;			// info-bulles à afficher ?
	BOOL			m_bGlint;			// reflets sur boutons ?
	BOOL			m_bRain;			// pluie dans l'interface ?
	BOOL			m_bMovies;			// cinématiques ?
	BOOL			m_bNiceReset;		// pour CTaskReset
	BOOL			m_bHimselfDamage;	// pour les tirs
	BOOL			m_bOnBoard;			// caméra sur le capot
	BOOL			m_bEffect;			// pour CCamera
	BOOL			m_bFlash;			// pour CCamera
	BOOL			m_bMotorBlast;		// explosion du moteur
	CameraType		m_defCamera;		// caméra par défaut

	FPOINT			m_glintMouse;
	float			m_glintTime;

	int				m_loadingCounter;

	BOOL			m_bDialog;			// dialogue présent ?
	BOOL			m_bDialogFire;		// cadre en feu ?
	BOOL			m_bDialogCreate;
	BOOL			m_bDialogDelete;
	BOOL			m_bDialogFile;
	BOOL			m_bDialogKid;
	FPOINT			m_dialogPos;
	FPOINT			m_dialogDim;
	float			m_dialogParti;
	float			m_dialogTime;
	BOOL			m_bInitPause;
	CameraType		m_initCamera;
	char			m_dialogName[100];

	int				m_partiPhase[10];
	float			m_partiTime[10];
	FPOINT			m_partiPos[10];

	SceneInfo		m_sceneInfo[MAXSCENE];
	ListInfo		m_listInfo[MAXSCENE];

	RecordList		m_recordAll;		// records glogaux mission en cours
	RecordList		m_recordOne;		// records tours mission en cours

	int				m_ghostTotal;
	int				m_ghostSelect;
	GhostFile		m_ghostList[GHOSTLISTMAX];
	char			m_ghostName[100];

	BOOL			m_bDuel;
	int				m_duelLevel;
	int				m_duelType;
	int				m_duelMission;
	int				m_duelModel;
	int				m_duelSubModel;
	D3DCOLORVALUE	m_duelColor;
};


#endif //_MAINDIALOG_H_
