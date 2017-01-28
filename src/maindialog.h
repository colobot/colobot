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
class CGamerFile;

enum Phase;
enum CameraType;
enum Phase;


#define LISTMAX			200
#define MAXSCENE		(16*9)

#define DIALOG_NULL			0
#define DIALOG_CREATEGAMER	1
#define DIALOG_DELETEGAMER	2
#define DIALOG_DELETEPUZZLE	3
#define DIALOG_NEWPUZZLE	4
#define DIALOG_INFOPUZZLE	5
#define DIALOG_ERROR		6
#define DIALOG_EXPORTPUZZLE	7
#define DIALOG_IMPORTPUZZLE	8
#define DIALOG_RENAMEPUZZLE	9

#define INDEX_PUZZLE	0
#define INDEX_DEFI		1
#define INDEX_USER		2
#define INDEX_PROTO		3
#define INDEX_MAX		4


typedef struct
{
	int				total;			// nb de voitures achetées
	int				buyable;		// nb de voitures achetables
	int				bonus;			// nb de voitures dispo en bonus
	int				level;			// niveau de difficulté (1..3)
}
GamerPerso;

typedef struct
{
	char		filename[100];
	char		bSolved;
}
ListFile;


class CMainDialog
{
public:
	CMainDialog(CInstanceManager* iMan);
	~CMainDialog();

	BOOL	EventProcess(const Event &event);
	void	ChangePhase(Phase phase, Phase fadeIn=(Phase)0);

	void	LaunchSimul(BOOL bEdit, BOOL bTest);
	char*	RetSavegameDir();
	char*	RetPublicDir();

	BOOL	RetEdit();
	BOOL	RetTest();
	BOOL	RetProto();
	BOOL	RetAgain();
	void	SetAgain(BOOL bAgain);

	void	BuildSceneName(char *filename);
	char*	RetFilesDir();

	void	StartAbort();
	void	StartCreateGamer();
	void	StartDeleteGamer(char *gamer);
	void	StartDeletePuzzle();
	void	StartNewPuzzle();
	void	StartInfoPuzzle();
	void	StartExportPuzzle();
	void	StartImportPuzzle();
	void	StartRenamePuzzle();
	void	StartError(int err);
	void	StartQuit();
	void	StartDialog(FPOINT dim, BOOL bFire, BOOL bOK, BOOL bCancel);
	void	FrameDialog(float rTime);
	void	StopDialog();
	BOOL	IsDialog();

	void	StartSuspend();
	void	StopSuspend();

	void	SetupMemorize();
	void	SetupRecall();

	BOOL	ReadGamerMission();
	BOOL	WriteGamerMission();
	void	IncGamerInfoTry();
	void	SetGamerTotalTime(float time);
	float	RetGamerTotalTime();
	void	SetGamerInfoPassed();
	BOOL	RetGamerInfoPassed(int rank);
	void	NextMission();
	BOOL	RetWriteFile();
	int		RetLevel();

	void	FlushPerso();
	void	AllMissionUpdate();

protected:
	BOOL	IsAccessibleDefiUser();
	void	ChangeTabOrder(EventMsg window, int dir, int param);
	void	GlintMove();
	void	FrameMove(float rTime);
	void	FrameParticule(float rTime);
	void	NiceParticule(FPOINT mouse, BOOL bPress);
	void	ReadNameList();
	void	UpdateNameControl();
	BOOL	ReadScene(char *filename, char *univers, char *resume, char *author, BOOL &bSolved, int &environment);
	void	UpdatePuzzleScroll();
	void	UpdatePuzzleButtons(BOOL bInit);
	BOOL	IsAccessibleDefi(int environment);
	void	UpdateSceneList(int &sel);
	void	UpdateButtonList();
	BOOL	NameSelect();
	BOOL	NameCreate();
	void	NameDelete();
	void	UpdateDisplayDevice();
	void	UpdateDisplayMode();
	void	ChangeDisplay();
	void	UpdateApply();
	void	UpdateSetupButtons();
	void	ChangeSetupButtons();
	void	ChangeSetupQuality(int quality);
	void	UpdateKey();
	void	ChangeKey(EventMsg event);
	void	UpdateNewPuzzle();
	void	UpdateImportPuzzle(CArray *pa);
	void	UpdateExportType();
	BOOL	CreateNewPuzzle(int environment, char *filename);
	BOOL	DeletePuzzle(int i);
	void	DeleteGamerFile(char *puzzle);
	BOOL	ExportPuzzle(int i);
	BOOL	ImportPuzzle(int i);
	BOOL	RenamePuzzle(int i, char *newName);
	BOOL	SearchNewName(char *dir, char *base, char *filename, char *quick);
	BOOL	CopyFile(char *filenameSrc, char *filenameDst);
	BOOL	SolvedPuzzle(int i, BOOL bSolved, int totalManip);
	void	GamerChanged();

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
	Phase			m_phaseFadeIn;		// phase après le fade in
	float			m_phaseTime;

	GamerPerso		m_perso;			// perso: description

	char			m_sceneDir[_MAX_FNAME];		// dossier scene\ 
	char			m_savegameDir[_MAX_FNAME];	// dossier savegame\ 
	char			m_defiDir[_MAX_FNAME];		// dossier defi\ 
	char			m_publicDir[_MAX_FNAME];	// dossier program\ 
	char			m_filesDir[_MAX_FNAME];		// dossier files\ 

	int				m_index;			// INDEX_*
	int				m_sel;				// mission choisie (0..n)
	int				m_list;				// offset dans m_listBuffer
	char			m_sceneName[20];	// nom de la scène à jouer
	BOOL			m_bSimulSetup;		// réglages pendant le jeu
	BOOL			m_bDeleteGamer;
	BOOL			m_bEdit;
	BOOL			m_bTest;
	BOOL			m_bProto;
	char			m_selectFilename[INDEX_MAX][100];
	int				m_scrollOffset;

	float			m_fadeOutDelay;
	float			m_fadeOutProgress;

	int				m_shotDelay;		// nb de frames avant copie
	char			m_shotName[100];	// nom du fichier à générer

	int				m_setupSelDevice;
	int				m_setupSelMode;
	BOOL			m_setupFull;

	BOOL			m_bRain;			// pluie dans l'interface ?
	BOOL			m_bAgain;			// joue après clic "Recommencer"
	BOOL			m_bPlayEnable;

	FPOINT			m_glintMouse;
	float			m_glintTime;

	int				m_loadingCounter;

	BOOL			m_bDialog;			// dialogue présent ?
	BOOL			m_bDialogFire;		// cadre en feu ?
	int				m_dialogType;		// type du dialogue en cours
	BOOL			m_bWriteFile;
	FPOINT			m_dialogPos;
	FPOINT			m_dialogDim;
	float			m_dialogParti;
	float			m_dialogTime;
	BOOL			m_bInitPause;
	CameraType		m_initCamera;
	char			m_dialogName[100];
	int				m_environment;
	char			m_newPuzzleFilename[100];

	BOOL			m_bMoveAnimation;
	CButton*		m_moveButton;
	FPOINT			m_moveButtonPos;
	FPOINT			m_moveButtonDim;
	int				m_movePhase;
	float			m_moveProgress;
	FPOINT			m_moveCenter;
	float			m_moveZoom;
	float			m_moveAngle;

	int				m_partiPhase[10];
	float			m_partiTime[10];
	FPOINT			m_partiPos[10];

	CGamerFile*		m_gamerFile;

	int				m_listTotal;
	ListFile		m_listBuffer[LISTMAX];

	int				m_importSelect;
	int				m_importTotal;
	ListFile		m_importBuffer[LISTMAX];

	int				m_exportType;
};


#endif //_MAINDIALOG_H_
