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
// * along with this program. If not, see .

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


#define USERLISTMAX	100
#define MAXSCENE	1000

typedef struct
{
	char	numTry;
	char	bPassed;
}
SceneInfo;

typedef struct
{
	int				face;			// visage
	int				glasses;		// lunettes
	D3DCOLORVALUE	colorHair;		// couleur cheveux
	D3DCOLORVALUE	colorCombi;		// couleur combinaison
	D3DCOLORVALUE	colorBand;		// couleur bandes
}
GamerPerso;



class CMainDialog
{
public:
	CMainDialog(CInstanceManager* iMan);
	~CMainDialog();

	BOOL	EventProcess(const Event &event);
	void	ChangePhase(Phase phase);

	void	SetSceneRead(char* name);
	void	SetStackRead(char* name);
	void	SetSceneName(char* name);
	void	SetSceneRank(int rank);
	char*	RetSceneRead();
	char*	RetStackRead();
	char*	RetSceneName();
	int		RetSceneRank();
	char*	RetSceneDir();
	BOOL	RetSceneSoluce();
	char*	RetSavegameDir();
	char*	RetPublicDir();

	BOOL	RetTooltip();
	BOOL	RetGlint();
	BOOL	RetSoluce4();
	BOOL	RetMovies();
	BOOL	RetNiceReset();
	BOOL	RetHimselfDamage();

	void	SetUserDir(char *base, int rank);
	void	BuildSceneName(char *filename, char *base, int rank);
	void	BuildResumeName(char *filename, char *base, int rank);
	char*	RetFilesDir();

	void	StartAbort();
	void	StartDeleteObject();
	void	StartDeleteGame(char *gamer);
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
	void	SetGamerInfoTry(int rank, int numTry);
	int		RetGamerInfoTry(int rank);
	void	SetGamerInfoPassed(int rank, BOOL bPassed);
	BOOL	RetGamerInfoPassed(int rank);
	BOOL	NextMission();

	void	WriteGamerPerso(char *gamer);
	void	ReadGamerPerso(char *gamer);
	void	SetGamerFace(char *gamer, int face);
	int		RetGamerFace(char *gamer);
	int		RetGamerFace();
	int		RetGamerGlasses();
	BOOL	RetGamerOnlyHead();
	float	RetPersoAngle();
	D3DCOLORVALUE RetGamerColorHair();
	D3DCOLORVALUE RetGamerColorCombi();
	D3DCOLORVALUE RetGamerColorBand();

	void	AllMissionUpdate();
	void	ShowSoluceUpdate();

protected:
	void	GlintMove();
	void	FrameParticule(float rTime);
	void	NiceParticule(FPOINT mouse, BOOL bPress);
	void	ReadNameList();
	void	UpdateNameList();
	void	UpdateNameEdit();
	void	UpdateNameControl();
	void	UpdateNameFace();
	void	NameSelect();
	void	NameCreate();
	void	NameDelete();
	void	UpdatePerso();
	void	CameraPerso();
	void	FixPerso(int rank, int index);
	void	ColorPerso();
	void	DefPerso();
	BOOL	IsIOReadScene();
	void	IOReadName();
	void	IOReadList();
	void	IOUpdateList();
	void	IODeleteScene();
	BOOL	IOWriteScene();
	BOOL	IOReadScene();
	int		RetChapPassed();
	void	UpdateSceneChap(int &chap);
	void	UpdateSceneList(int chap, int &sel);
	void	UpdateSceneResume(int rank);
	void	UpdateDisplayDevice();
	void	UpdateDisplayMode();
	void	ChangeDisplay();
	void	UpdateApply();
	void	UpdateSetupButtons();
	void	ChangeSetupButtons();
	void	ChangeSetupQuality(int quality);
	void	UpdateKey();
	void	ChangeKey(EventMsg event);

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
	float			m_phaseTime;

	GamerPerso		m_perso;			// perso: description
	GamerPerso		m_persoCopy;		// perso: copie pour annulation
	int				m_persoTab;			// perso: onglet choisi
	float			m_persoAngle;		// perso: angle de pr�sentation

	char			m_sceneDir[_MAX_FNAME];		// dossier scene\ 
	char			m_savegameDir[_MAX_FNAME];	// dossier savegame\ 
	char			m_publicDir[_MAX_FNAME];	// dossier program\ 
	char			m_userDir[_MAX_FNAME];		// dossier user\ 
	char			m_filesDir[_MAX_FNAME];		// dossier files\ 

	int				m_index;			// 0..4
	int				m_chap[10];			// chapitre choisi (0..8)
	int				m_sel[10];			// mission choisie (0..98)
	int				m_maxList;
	int				m_accessChap;
	char			m_sceneRead[100];	// nom de la sc�ne � lire
	char			m_stackRead[100];	// nom de la sc�ne � lire
	char			m_sceneName[20];	// nom de la sc�ne � jouer
	int				m_sceneRank;		// rang de la sc�ne � jouer
	BOOL			m_bSceneSoluce;		// montre la solution
	BOOL			m_bSimulSetup;		// r�glages pendant le jeu
	BOOL			m_accessEnable;
	BOOL			m_accessMission;
	BOOL			m_accessUser;
	BOOL			m_bDeleteGamer;

	int				m_userTotal;
	char			m_userList[USERLISTMAX][100];

	int				m_shotDelay;		// nb de frames avant copie
	char			m_shotName[100];	// nom du fichier � g�n�rer

	int				m_setupSelDevice;
	int				m_setupSelMode;
	BOOL			m_setupFull;

	BOOL			m_bTooltip;			// info-bulles � afficher ?
	BOOL			m_bGlint;			// reflets sur boutons ?
	BOOL			m_bRain;			// pluie dans l'interface ?
	BOOL			m_bSoluce4;			// solutions dans programme 4 ?
	BOOL			m_bMovies;			// cin�matiques ?
	BOOL			m_bNiceReset;		// pour CTaskReset
	BOOL			m_bHimselfDamage;	// pour les tirs
	BOOL			m_bCameraScroll;	// pour CCamera
	BOOL			m_bCameraInvertX;	// pour CCamera
	BOOL			m_bCameraInvertY;	// pour CCamera
	BOOL			m_bEffect;			// pour CCamera

	FPOINT			m_glintMouse;
	float			m_glintTime;

	int				m_loadingCounter;

	BOOL			m_bDialog;			// dialogue pr�sent ?
	BOOL			m_bDialogFire;		// cadre en feu ?
	BOOL			m_bDialogDelete;
	FPOINT			m_dialogPos;
	FPOINT			m_dialogDim;
	float			m_dialogParti;
	float			m_dialogTime;
	BOOL			m_bInitPause;
	CameraType		m_initCamera;

	int				m_partiPhase[10];
	float			m_partiTime[10];
	FPOINT			m_partiPos[10];

	SceneInfo		m_sceneInfo[MAXSCENE];
};


#endif //_MAINDIALOG_H_
