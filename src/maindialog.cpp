// maindialog.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "recorder.h"
#include "particule.h"
#include "object.h"
#include "physics.h"
#include "motion.h"
#include "interface.h"
#include "button.h"
#include "color.h"
#include "gauge.h"
#include "pesetas.h"
#include "check.h"
#include "key.h"
#include "group.h"
#include "image.h"
#include "scroll.h"
#include "slider.h"
#include "list.h"
#include "label.h"
#include "window.h"
#include "edit.h"
#include "editvalue.h"
#include "text.h"
#include "array.h"
#include "camera.h"
#include "pyro.h"
#include "sound.h"
#include "cmdtoken.h"
#include "robotmain.h"
#include "maindialog.h"



#define KEY_VISIBLE		6		// nb de touches redéfinissables visibles
#define KEY_TOTAL		8		// nb total de touches redéfinissables
#define WELCOME_LENGTH	6.0f




static int perso_color[3*15] =
{
	255, 209,  67,  // jaune-orange (car01)
	 72, 153, 236,  // bleu (car03)
	196,  55,  61,  // rouge (car04)
	 64,  64,  64,  // gris-noir (car05)
	143,  88, 224,  // violet (car02)
	230, 251, 252,  // blanc-cyan
	 93,  13,  13,  // brun
	238, 132, 214,  // rose (car06)
	255, 136,  39,  // orange
	 20, 192,  20,  // vert
	255, 255,   0,  // jaune (car08)
	  0,  53, 226,  // bleu royal
	 89,  34, 172,  // violet foncé (car07)
	126, 186, 244,  // cyan
	149, 149, 149,  // gris
};




// Constructeur de l'application robot.

CMainDialog::CMainDialog(CInstanceManager* iMan)
{
	int		i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_DIALOG, this);

	m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_phase        = PHASE_NAME;
	m_phaseSetup   = PHASE_SETUPg;
	m_phaseTerm    = PHASE_MISSION;
	m_phasePerso   = PHASE_MISSION;
	m_sceneName[0] = 0;
	m_sceneBase[0] = 0;
	m_sceneRank    = 0;
	m_bSimulSetup  = FALSE;
	m_accessEnable = TRUE;
	m_accessMission= TRUE;
	m_accessUser   = TRUE;
	m_bDeleteGamer = TRUE;
	m_bGhostExist  = FALSE;
	m_bGhostEnable = TRUE;
	m_bPesetas     = FALSE;

	for ( i=0 ; i<6 ; i++ )
	{
		m_sel[i] = 0;
	}
	m_index = 0;

	FlushPerso();

	m_bTooltip       = TRUE;
	m_bGlint         = TRUE;
	m_bRain          = TRUE;
	m_bMovies        = TRUE;
	m_bNiceReset     = TRUE;
	m_bHimselfDamage = TRUE;
	m_bEffect        = TRUE;
	m_bFlash         = TRUE;
	m_bMotorBlast    = TRUE;
	m_bDialogCreate  = FALSE;
	m_bDialogDelete  = FALSE;
	m_bDialogFile    = FALSE;
	m_bDialogKid     = FALSE;
	m_bDuel          = FALSE;
	m_shotDelay      = 0;
	m_defCamera      = CAMERA_BACK;

	m_glintMouse = FPOINT(0.0f, 0.0f);
	m_glintTime  = 1000.0f;

	for ( i=0 ; i<10 ; i++ )
	{
		m_partiPhase[i] = 0;
		m_partiTime[i]  = 0.0f;
	}

	strcpy(m_sceneDir,    "scene");
	strcpy(m_savegameDir, "savegame");
	strcpy(m_publicDir,   "program");
	strcpy(m_userDir,     "user");
	strcpy(m_filesDir,    "files");
	strcpy(m_duelDir,     "duel");

	m_bDialog = FALSE;
}

// Destructeur de l'application robot.

CMainDialog::~CMainDialog()
{
}


// Change de phase.

void CMainDialog::ChangePhase(Phase phase)
{
	CWindow*		pw;
	CEdit*			pe;
	CLabel*			pl;
	CList*			pli;
	CArray*			pa;
	CCheck*			pc;
	CScroll*		ps;
	CSlider*		psl;
	CButton*		pb;
	CColor*			pco;
	CGauge*			pgg;
	CPesetas*		pp;
	CGroup*			pg;
	CImage*			pi;
	FPOINT			pos, dim, ddim;
	float			ox, oy, sx, sy;
	char			name[100];
	int				res, i;

	m_camera->SetType(CAMERA_DIALOG);
	m_engine->SetOverFront(FALSE);
	m_engine->SetOverColor(RetColor(0.0f), D3DSTATETCb);

	if ( phase == PHASE_TERM )
	{
		phase = m_phaseTerm;
	}
	if ( phase == PHASE_CAR && m_phase != PHASE_READ )
	{
		m_phasePerso = m_phase;
	}
	m_phase = phase;  // copie l'info de CRobotMain
	m_phaseTime = 0.0f;

	dim.x = 32.0f/640.0f;
	dim.y = 32.0f/480.0f;
	ox = 3.0f/640.0f;
	oy = 3.0f/480.0f;
	sx = (32.0f+2.0f)/640.0f;
	sy = (32.0f+2.0f)/480.0f;

	if ( m_phase == PHASE_INIT )
	{
		pos.x  = 0.35f;
		pos.y  = 0.10f;
		ddim.x = 0.30f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		GetResource(RES_TEXT, RT_TITLE_INIT, name);
		pw->SetName(name);

		pos.x  = 0.35f;
		pos.y  = 0.60f;
		ddim.x = 0.30f;
		ddim.y = 0.30f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.35f;
		pos.y  = 0.10f;
		ddim.x = 0.30f;
		ddim.y = 0.30f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

#if _SPANISH
		ddim.x = 0.20f;
		ddim.y = dim.y*1;
		pos.x = 0.40f;
#else
		ddim.x = 0.18f;
		ddim.y = dim.y*1;
		pos.x = 0.41f;
#endif

		pos.y = oy+sy*9.7f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MISSION);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		pos.y = oy+sy*8.6f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_FREE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(1);

		pos.y = oy+sy*7.5f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DUEL);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		pos.y = oy+sy*5.7f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUP);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(3);

		pos.y = oy+sy*4.6f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NAME);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(4);

		pos.y = oy+sy*2.8f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_QUIT);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(5);

		if ( m_engine->RetDebugMode() )
		{
			pos.x  = 20.0f/640.0f;
			pos.y  = 20.0f/480.0f;
			ddim.x = 0.09f;
			pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PROTO);
			pb->SetFontType(FONT_HILITE);
			pb->SetState(STATE_SHADOW);
		}

		pos.x  = 0.40f;
		ddim.x = 0.20f;
		pos.y  =  64.0f/480.0f;
		ddim.y =  12.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_DEV1, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(8.0f);

		pos.y -=  12.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_DEV2, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(8.0f);

		m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_NAME )
	{
		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		GetResource(RES_TEXT, RT_TITLE_NAME, name);
		pw->SetName(name);

		pos.x  = 0.10f;
		pos.y  = 0.40f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.40f;
		pos.y  = 0.10f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		pos.x  = 140.0f/640.0f;
		pos.y  = 310.0f/480.0f;
		ddim.x = 120.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_PERSO_LIST, name);
		pl = pw->CreateLabel(pos, ddim, -1, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.x  = 140.0f/640.0f;
		pos.y  = 150.0f/480.0f;
		ddim.x = 160.0f/640.0f;
		ddim.y = 160.0f/480.0f;
		pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_NLIST);
		pli->SetState(STATE_DEFAULT);
		pli->SetState(STATE_SHADOW);
		pli->SetTabOrder(1);

		pos.x  = 380.0f/640.0f;
		pos.y  = 132.0f/480.0f;
		ddim.x = 140.0f/640.0f;
		ddim.y = 124.0f/480.0f;
		pw->CreateGroup(pos, ddim, 28, EVENT_LABEL1);

		pos.x  = 390.0f/640.0f;
		pos.y  = 226.0f/480.0f;
		ddim.x = 190.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_PERSO_LEVEL, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetFontSize(9.0f);
		pl->SetJustif(1);

		pos.x  = 390.0f/640.0f;
		pos.y  = 210.0f/480.0f;
		ddim.x = 100.0f/640.0f;
		ddim.y =  14.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LEVEL1);
		pc->SetFontSize(9.0f);
		pc->SetState(STATE_RADIO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(4);
		pos.y -= 20.0f/480.0f;
		ddim.y =  16.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LEVEL2);
		pc->SetFontSize(10.0f);
		pc->SetState(STATE_RADIO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(5);
		pos.y -= 22.0f/480.0f;
		ddim.y =  18.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LEVEL3);
		pc->SetFontSize(11.0f);
		pc->SetState(STATE_RADIO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(6);
		pos.y -= 24.0f/480.0f;
		ddim.y =  20.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LEVEL4);
		pc->SetFontSize(12.0f);
		pc->SetState(STATE_RADIO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(7);

		pos.x  = 380.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 140.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		pos.x  = 380.0f/640.0f;
		pos.y  = 330.0f/480.0f;
		ddim.x = 140.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NCREATE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		if ( m_bDeleteGamer )
		{
			pos.x  = 380.0f/640.0f;
			pos.y  = 280.0f/480.0f;
			ddim.x = 140.0f/640.0f;
			ddim.y =  32.0f/480.0f;
			pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NDELETE);
			pb->SetFontType(FONT_HILITE);
			pb->SetState(STATE_SHADOW);
			pb->SetTabOrder(3);
		}

		ReadGamerInfo();
		ReadNameList();
		UpdateNameControl();

		m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_CAR )
	{
		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 15, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		GetResource(RES_TEXT, RT_TITLE_PERSO, name);
		pw->SetName(name);

		pos.x  = 0.10f;
		pos.y  = 0.44f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.40f;
		pos.y  = 0.06f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		pos.x  =  74.0f/640.0f;
		pos.y  = 370.0f/480.0f;
		ddim.x = 108.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_PGSPEED, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pos.y -=  10.0f/480.0f;
		pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGSPEED);
		pgg->SetState(STATE_SHADOW);
		pos.y -=  28.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLSPEED, "");

		pos.x  = 202.0f/640.0f;
		pos.y  = 370.0f/480.0f;
		ddim.x = 108.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_PGACCEL, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pos.y -=  10.0f/480.0f;
		pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGACCEL);
		pgg->SetState(STATE_SHADOW);
		pos.y -=  28.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLACCEL, "");

		pos.x  = 330.0f/640.0f;
		pos.y  = 370.0f/480.0f;
		ddim.x = 108.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_PGGRIP, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
		pos.y -=  10.0f/480.0f;
		pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGGRIP);
		pgg->SetState(STATE_SHADOW);
		pos.y -=  28.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLGRIP, "");

		pos.x  = 458.0f/640.0f;
		pos.y  = 370.0f/480.0f;
		ddim.x = 108.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_PGSOLID, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
		pos.y -=  10.0f/480.0f;
		pgg = pw->CreateGauge(pos, ddim, 0, EVENT_INTERFACE_PGSOLID);
		pgg->SetState(STATE_SHADOW);
		pos.y -=  28.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PLSOLID, "");

		m_persoElevation = -0.1f;
		pos.x  =  74.0f/640.0f;
		pos.y  = 239.0f/480.0f;
		ddim.x =  16.0f/640.0f;
		ddim.y = 100.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_PELEV);
		psl->SetState(STATE_SHADOW);
		psl->SetLimit(-0.7f, 0.3f);
		psl->SetArrowStep(0.1f);
		psl->SetVisibleValue(m_persoElevation);
		psl->SetTabOrder(17);

		EventMsg table[] =
		{
			EVENT_INTERFACE_PCOLOR5,  // blanc
			EVENT_INTERFACE_PCOLOR10, // jaune
			EVENT_INTERFACE_PCOLOR0,  // jaune-orange
			EVENT_INTERFACE_PCOLOR8,  // orange
			EVENT_INTERFACE_PCOLOR2,  // rouge
			EVENT_INTERFACE_PCOLOR6,  // brun
			EVENT_INTERFACE_PCOLOR3,  // noir
			EVENT_INTERFACE_PCOLOR12, // violet foncé
			EVENT_INTERFACE_PCOLOR4,  // violet
			EVENT_INTERFACE_PCOLOR11, // bleu royal
			EVENT_INTERFACE_PCOLOR1,  // bleu
			EVENT_INTERFACE_PCOLOR13, // cyan
			EVENT_INTERFACE_PCOLOR14, // gris
			EVENT_INTERFACE_PCOLOR9,  // vert
			EVENT_INTERFACE_PCOLOR7,  // rose
		};

		pos.x  = 550.0f/640.0f;
		pos.y  = 324.0f/480.0f;
		ddim.x =  15.0f/640.0f;
		ddim.y =  15.0f/480.0f;
		for ( i=0 ; i<15 ; i++ )
		{
			pco = pw->CreateColor(pos, ddim, -1, table[i]);
			pco->SetState(STATE_SHADOW);
			pco->SetTabOrder(2+i);
			pos.y -= 15.0f/480.0f;
		}
		
		pos.x  =  74.0f/640.0f;
		pos.y  = 170.0f/480.0f;
		ddim.x =  16.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD0);
		pb->SetFontSize(9.0f);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(18);
		pos.y -= 18.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD1);
		pb->SetFontSize(9.0f);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(19);
		pos.y -= 18.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD2);
		pb->SetFontSize(9.0f);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(20);
		pos.y -= 18.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PSUBMOD3);
		pb->SetFontSize(9.0f);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(21);

#if _DEMO|_SE
		pos.x  =  94.0f/640.0f;
		pos.y  = 298.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  44.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 29, EVENT_INTERFACE_PSPECB);
#endif

		pos.x  =  94.0f/640.0f;
		pos.y  = 308.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PPRICE, "");
		pl->SetFontSize(13.0f);

		pos.x  =  94.0f/640.0f;
		pos.y  = 294.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PPESETAS, "");
		pl->SetFontSize(10.0f);

#if _DEMO|_SE
		pos.x  =  94.0f/640.0f;
		pos.y  = 292.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PSPECT, "");
		pl->SetFontSize(10.0f);
#else
		pos.x  = 150.0f/640.0f;
		pos.y  = 230.0f/480.0f;
		ddim.x = 340.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 9, EVENT_INTERFACE_PSPECB);
		pg->SetState(STATE_SHADOW);
		pos.x  = 150.0f/640.0f;
		pos.y  = 222.0f/480.0f;
		ddim.x = 340.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_PSPECT, "");
		pl->SetFontSize(15.0f);
#endif

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x =  60.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, 55, EVENT_INTERFACE_PPREV);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(22);

		pos.x  = 164.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x =  60.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, 48, EVENT_INTERFACE_PNEXT);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(23);

		pos.x  = 336.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_POK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		pos.x  = 332.0f/640.0f;
		pos.y  =  38.0f/480.0f;
		ddim.x = 218.0f/640.0f;
		ddim.y =  26.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 27, EVENT_INTERFACE_GHOSTg);
		pg->SetState(STATE_SHADOW);

		pos.x  = 336.0f/640.0f;
		pos.y  =  43.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GHOSTm);
		pc->SetFontSize(9.5f);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(1);
		UpdateSceneGhost(m_sel[m_index]);

		m_persoCopy = m_perso;  // copie si annulation
		m_persoAngle = -0.6f;
		m_persoTime = 0.0f;
		m_persoRun = FALSE;
		BuyablePerso();
		m_main->ScenePerso();
		UpdatePerso();
		CameraPerso();

		m_ghostName[0] = 0;  // utilise la voiture fantome standard
	}

	if ( m_phase == PHASE_MISSION ||
		 m_phase == PHASE_FREE    ||
		 m_phase == PHASE_USER    ||
		 m_phase == PHASE_PROTO   )
	{
		if ( m_phase == PHASE_MISSION )  m_index = 2;
		if ( m_phase == PHASE_FREE    )  m_index = 3;
		if ( m_phase == PHASE_USER    )  m_index = 4;
		if ( m_phase == PHASE_PROTO   )  m_index = 5;

		if ( m_phase == PHASE_MISSION )  strcpy(m_sceneName, "scene");
		if ( m_phase == PHASE_FREE    )  strcpy(m_sceneName, "free");
		if ( m_phase == PHASE_USER    )  strcpy(m_sceneName, "user");
		if ( m_phase == PHASE_PROTO   )  strcpy(m_sceneName, "proto");
		strcpy(m_sceneBase, m_sceneName);

		m_bPesetas = (m_phase == PHASE_MISSION);

		ReadGamerMission();

		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		if ( m_phase == PHASE_MISSION )  res = RT_TITLE_MISSION;
		if ( m_phase == PHASE_FREE    )  res = RT_TITLE_FREE;
		if ( m_phase == PHASE_USER    )  res = RT_TITLE_USER;
		if ( m_phase == PHASE_PROTO   )  res = RT_TITLE_PROTO;
		GetResource(RES_TEXT, res, name);
		pw->SetName(name);

		pos.x  = 0.10f;
		pos.y  = 0.44f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.40f;
		pos.y  = 0.06f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		// Affiche la colonne gauche :
		pos.x  =  94.0f/640.0f;
		ddim.x = 210.0f/640.0f;

		pos.y  = 370.0f/480.0f;
		ddim.y =  16.0f/480.0f;
		if ( m_phase == PHASE_MISSION )  res = RT_PLAY_LISTm;
		if ( m_phase == PHASE_FREE    )  res = RT_PLAY_LISTf;
		if ( m_phase == PHASE_USER    )  res = RT_PLAY_LISTu;
		if ( m_phase == PHASE_PROTO   )  res = RT_PLAY_LISTp;
		GetResource(RES_TEXT, res, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);

		pos.y  = 200.0f/480.0f;
		ddim.y = 178.0f/480.0f;
		pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_LIST);
		pli->SetState(STATE_DEFAULT);
		pli->SetState(STATE_SHADOW);
		UpdateSceneList(m_sel[m_index]);
		pli->SetState(STATE_EXTEND);
		pli->SetTabOrder(1);

		// Affiche la colonne droite :
		if ( m_bPesetas )
		{
			pos.x  = 336.0f/640.0f;
			ddim.x = 210.0f/640.0f;

			pos.y  = 370.0f/480.0f;
			ddim.y =  16.0f/480.0f;
			GetResource(RES_TEXT, RT_PLAY_PESETAS, name);
			pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
			pl->SetJustif(1);

			pos.y  = 200.0f/480.0f;
			ddim.y = 175.0f/480.0f;
			pp = pw->CreatePesetas(pos, ddim, 0, EVENT_INTERFACE_PESETAS);
			pp->SetState(STATE_SHADOW);
			pp->SetLevelCredit((float)m_perso.pesetas);
			pp->SetLevelNext((float)ReadPesetasNext());
			UpdateScenePesetasMax(m_sel[m_index]);
		}
		else
		{
			pos.x  = 336.0f/640.0f;
			ddim.x = 210.0f/640.0f;
			pos.y  = 200.0f/480.0f;
			ddim.y = 175.0f/480.0f;
			pi = pw->CreateImage(pos, ddim, 0, EVENT_INTERFACE_IMAGE);
			pi->SetState(STATE_SHADOW);
			UpdateSceneImage(m_sel[m_index]);
		}

		// Affiche le résumé :
		pos.x  =  94.0f/640.0f;
		pos.y  = 126.0f/480.0f;
		ddim.x = 452.0f/640.0f;
		ddim.y =  58.0f/480.0f;
		pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_RESUME);
		pe->SetState(STATE_SHADOW);
		pe->SetMaxChar(500);
		pe->SetMultiFont(TRUE);
		pe->SetEditCap(FALSE);  // juste pour voir
		pe->SetHiliteCap(FALSE);
		UpdateSceneResume(m_sel[m_index]);

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		pos.x  = 336.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PLAY);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);
		BuyablePerso();
		UpdateScenePlay(m_sel[m_index]);

		m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_SETUPd  ||
		 m_phase == PHASE_SETUPg  ||
		 m_phase == PHASE_SETUPp  ||
		 m_phase == PHASE_SETUPc  ||
		 m_phase == PHASE_SETUPs  ||
		 m_phase == PHASE_SETUPds ||
		 m_phase == PHASE_SETUPgs ||
		 m_phase == PHASE_SETUPps ||
		 m_phase == PHASE_SETUPcs ||
		 m_phase == PHASE_SETUPss )
	{
		if ( m_phase == PHASE_SETUPds )
		{
			m_phaseSetup = PHASE_SETUPd;
			m_bSimulSetup = TRUE;
		}
		else if ( m_phase == PHASE_SETUPgs )
		{
			m_phaseSetup = PHASE_SETUPg;
			m_bSimulSetup = TRUE;
		}
		else if ( m_phase == PHASE_SETUPps )
		{
			m_phaseSetup = PHASE_SETUPp;
			m_bSimulSetup = TRUE;
		}
		else if ( m_phase == PHASE_SETUPcs )
		{
			m_phaseSetup = PHASE_SETUPc;
			m_bSimulSetup = TRUE;
		}
		else if ( m_phase == PHASE_SETUPss )
		{
			m_phaseSetup = PHASE_SETUPs;
			m_bSimulSetup = TRUE;
		}
		else
		{
			m_phaseSetup = m_phase;
			m_bSimulSetup = FALSE;
		}

		pos.x = 0.10f;
		pos.y = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		GetResource(RES_TEXT, RT_TITLE_SETUP, name);
		pw->SetName(name);

		pos.x  = 0.70f;
		pos.y  = 0.10f;
		ddim.x = 0.20f;
		ddim.y = 0.20f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		pos.x  = 0.10f;
		ddim.x = 0.80f;
		pos.y  = 0.76f;
		ddim.y = 0.05f;
		pw->CreateGroup(pos, ddim, 3, EVENT_NULL);  // transparent -> gris

		ddim.x = 0.78f/5-0.01f;
		ddim.y = 0.06f;
		pos.x = 0.115f;
		pos.y = 0.76f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPd);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_CARD);
		pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPd || m_phase == PHASE_SETUPds));
		pb->SetTabOrder(1);

		pos.x += ddim.x+0.01f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPg);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_CARD);
		pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPg || m_phase == PHASE_SETUPgs));
		pb->SetTabOrder(2);

		pos.x += ddim.x+0.01f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPp);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_CARD);
		pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPp || m_phase == PHASE_SETUPps));
		pb->SetTabOrder(3);

		pos.x += ddim.x+0.01f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPc);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_CARD);
		pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPc || m_phase == PHASE_SETUPcs));
		pb->SetTabOrder(4);

		pos.x += ddim.x+0.01f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPs);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_CARD);
		pb->SetState(STATE_CHECK, (m_phase == PHASE_SETUPs || m_phase == PHASE_SETUPss));
		pb->SetTabOrder(5);

		pos.x  = 0.10f;
		ddim.x = 0.80f;
		pos.y  = 0.34f;
		ddim.y = 0.42f;
		pw->CreateGroup(pos, ddim, 2, EVENT_INTERFACE_GLINTu);  // orange -> transparent
		pos.x  = 0.10f+(6.0f/640.0f);
		ddim.x = 0.80f-(11.0f/640.0f);
		pos.y  = 0.74f;
		ddim.y = 0.02f;
		pw->CreateGroup(pos, ddim, 1, EVENT_INTERFACE_GLINTb);  // barre orange

		ddim.x = dim.x*6;
		ddim.y = dim.y*1;
		pos.x = ox+sx*10;
		pos.y = oy+sy*2;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_OK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		if ( !m_bSimulSetup )
		{
			m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
			m_engine->SetBackForce(TRUE);
		}
	}

	if ( m_phase == PHASE_SETUPd  || // setup/display ?
		 m_phase == PHASE_SETUPds )
	{
		pos.x = ox+sx*3;
		pos.y = oy+sy*9;
		ddim.x = dim.x*6;
		ddim.y = dim.y*1;
		GetResource(RES_TEXT, RT_SETUP_DEVICE, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.x = ox+sx*3;
		pos.y = oy+sy*5.2f;
		ddim.x = dim.x*6;
		ddim.y = dim.y*4.5f;
		pli = pw->CreateList(pos, ddim, 0, EVENT_LIST1);
		pli->SetState(STATE_SHADOW);
		pli->SetTabOrder(6);
		UpdateDisplayDevice();

		pos.x = ox+sx*10;
		pos.y = oy+sy*9;
		ddim.x = dim.x*6;
		ddim.y = dim.y*1;
		GetResource(RES_TEXT, RT_SETUP_MODE, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetJustif(1);

		m_setupFull = m_engine->RetFullScreen();
		pos.x = ox+sx*10;
		pos.y = oy+sy*5.2f;
		ddim.x = dim.x*6;
		ddim.y = dim.y*4.5f;
		pli = pw->CreateList(pos, ddim, 0, EVENT_LIST2);
		pli->SetState(STATE_SHADOW);
		pli->SetTabOrder(7);
		UpdateDisplayMode();
		pli->SetState(STATE_ENABLE, m_setupFull);

		ddim.x = dim.x*4;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*3;
		pos.y = oy+sy*4.1f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_FULL);
		pc->SetState(STATE_SHADOW);
		pc->SetState(STATE_CHECK, m_setupFull);
		pc->SetTabOrder(8);

		ddim.x = dim.x*6;
		ddim.y = dim.y*1;
		pos.x = ox+sx*3;
		pos.y = oy+sy*2;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_APPLY);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(9);
		UpdateApply();
	}

	if ( m_phase == PHASE_SETUPg  ||  // setup/graphic ?
		 m_phase == PHASE_SETUPgs )
	{
		ddim.x = dim.x*6;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*3;
		pos.y = 0.65f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(6);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DIRTY);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(7);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SKY);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(8);
		if ( m_engine->IsVideo8MB() )  pc->ClearState(STATE_ENABLE);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LENS);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(9);
		pos.y -= 0.048f;
		if ( !m_bSimulSetup )
		{
			pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LIGHT);
			pc->SetState(STATE_SHADOW);
			pc->SetTabOrder(10);
		}
		pos.y -= 0.048f;
#if _DEBUG
		if ( !m_bSimulSetup )
		{
			pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SUPER);
			pc->SetState(STATE_SHADOW);
			pc->SetTabOrder(11);
		}
#endif

		pos.x  = 280.0f/640.0f;
		pos.y  = 134.0f/480.0f;
		ddim.x =  18.0f/640.0f;
		ddim.y =  90.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_PARTI);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetState(STATE_VALUE);
		psl->SetState(STATE_SHADOW);
		psl->SetTabOrder(12);
		pos.y  = 320.0f/480.0f;
		ddim.x = 180.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_PARTI, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);
		pos.x +=   8.0f/640.0f;
		pos.y  = 240.0f/480.0f;
		ddim.x =   2.0f/640.0f;
		ddim.y =  90.0f/480.0f;
		pw->CreateGroup(pos, ddim, 27, EVENT_LABEL1);

		pos.x  = 305.0f/640.0f;
		pos.y  = 134.0f/480.0f;
		ddim.x =  18.0f/640.0f;
		ddim.y =  90.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_WHEEL);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetState(STATE_VALUE);
		psl->SetState(STATE_SHADOW);
		psl->SetTabOrder(13);
		pos.y  = 300.0f/480.0f;
		ddim.x = 180.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_WHEEL, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
		pl->SetJustif(1);
		pos.x +=   8.0f/640.0f;
		pos.y  = 240.0f/480.0f;
		ddim.x =   2.0f/640.0f;
		ddim.y =  70.0f/480.0f;
		pw->CreateGroup(pos, ddim, 27, EVENT_LABEL1);

		pos.x  = 330.0f/640.0f;
		pos.y  = 134.0f/480.0f;
		ddim.x =  18.0f/640.0f;
		ddim.y =  90.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_CLIP);
		psl->SetLimit(0.5f, 2.0f);
		psl->SetArrowStep(0.1f);
		psl->SetState(STATE_VALUE);
		psl->SetState(STATE_SHADOW);
		psl->SetTabOrder(14);
		pos.y  = 280.0f/480.0f;
		ddim.x = 180.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_CLIP, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
		pl->SetJustif(1);
		pos.x +=   8.0f/640.0f;
		pos.y  = 240.0f/480.0f;
		ddim.x =   2.0f/640.0f;
		ddim.y =  50.0f/480.0f;
		pw->CreateGroup(pos, ddim, 27, EVENT_LABEL1);

		pos.x  = 355.0f/640.0f;
		pos.y  = 134.0f/480.0f;
		ddim.x =  18.0f/640.0f;
		ddim.y =  90.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_DETAIL);
		psl->SetLimit(0.0f, 2.0f);
		psl->SetArrowStep(0.2f);
		psl->SetState(STATE_VALUE);
		psl->SetState(STATE_SHADOW);
		psl->SetTabOrder(15);
		pos.y  = 260.0f/480.0f;
		ddim.x = 180.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_DETAIL, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL13, name);
		pl->SetJustif(1);
		pos.x +=   8.0f/640.0f;
		pos.y  = 240.0f/480.0f;
		ddim.x =   2.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pw->CreateGroup(pos, ddim, 27, EVENT_LABEL1);

		if ( !m_bSimulSetup )
		{
			pos.x  = 380.0f/640.0f;
			pos.y  = 134.0f/480.0f;
			ddim.x =  18.0f/640.0f;
			ddim.y =  90.0f/480.0f;
			psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_GADGET);
			psl->SetLimit(0.0f, 1.0f);
			psl->SetArrowStep(0.1f);
			psl->SetState(STATE_VALUE);
			psl->SetState(STATE_SHADOW);
			psl->SetTabOrder(16);
			pos.y  = 240.0f/480.0f;
			ddim.x = 200.0f/640.0f;
			ddim.y =  16.0f/480.0f;
			GetResource(RES_EVENT, EVENT_INTERFACE_GADGET, name);
			pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL14, name);
			pl->SetJustif(1);
			pos.x +=   8.0f/640.0f;
			pos.y  = 240.0f/480.0f;
			ddim.x =   2.0f/640.0f;
			ddim.y =  10.0f/480.0f;
			pw->CreateGroup(pos, ddim, 27, EVENT_LABEL1);
		}

		ddim.x = dim.x*2;
		ddim.y = dim.y*1;
		pos.x = ox+sx*3;
		pos.y = oy+sy*2;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MIN);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(17);
		pos.x += ddim.x;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NORM);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(18);
		pos.x += ddim.x;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MAX);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(19);

		UpdateSetupButtons();
	}

	if ( m_phase == PHASE_SETUPp  ||  // setup/jeu ?
		 m_phase == PHASE_SETUPps )
	{
		ddim.x = dim.x*6;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*3;
		pos.y = 0.65f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_CBACK);
		pc->SetState(STATE_RADIO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(6);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_CBOARD);
		pc->SetState(STATE_RADIO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(7);
		pos.y -= 0.072f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_BLAST);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(8);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EFFECT);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(9);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_FLASH);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(10);

		ddim.x = dim.x*6;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*10;
		pos.y = 0.65f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_TOOLTIP);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(11);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GLINT);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(12);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_RAIN);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(13);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOUSE);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(14);

		UpdateSetupButtons();
	}

	if ( m_phase == PHASE_SETUPc  ||  // setup/commandes ?
		 m_phase == PHASE_SETUPcs )
	{
		pos.x  = ox+sx*3;
		pos.y  = 320.0f/480.0f;
		ddim.x = dim.x*15.0f;
		ddim.y = 18.0f/480.0f;
		GetResource(RES_TEXT, RT_SETUP_KEY1, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO1, name);
		pl->SetJustif(1);

		pos.x  = ox+sx*3;
		pos.y  = 302.0f/480.0f;
		ddim.x = dim.x*15.0f;
		ddim.y = 18.0f/480.0f;
		GetResource(RES_TEXT, RT_SETUP_KEY2, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO2, name);
		pl->SetJustif(1);

		ddim.x = 428.0f/640.0f;
		ddim.y = 128.0f/480.0f;
		pos.x  = 105.0f/640.0f;
		pos.y  = 174.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 7, EVENT_INTERFACE_KGROUP);
		pg->ClearState(STATE_ENABLE);
		pg->SetState(STATE_DEAD);
		pg->SetState(STATE_SHADOW);

		ddim.x =  18.0f/640.0f;
		ddim.y = (20.0f/480.0f)*KEY_VISIBLE;
		pos.x  = 510.0f/640.0f;
		pos.y  = 178.0f/480.0f;
		ps = pw->CreateScroll(pos, ddim, -1, EVENT_INTERFACE_KSCROLL);
		ps->SetVisibleRatio((float)KEY_VISIBLE/KEY_TOTAL);
		ps->SetArrowStep(1.0f/((float)KEY_TOTAL-KEY_VISIBLE));
		UpdateKey();

		ddim.x = 224.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		pos.x  = 105.0f/640.0f;
		pos.y  = 144.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_STEERING);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(6);

		ddim.x = 224.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		pos.x  = 105.0f/640.0f;
		pos.y  = 122.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_JOYPAD);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(7);

		ddim.x = 128.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		pos.x  = 344.0f/640.0f;
		pos.y  = 144.0f/480.0f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_FFBc);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(8);

		ddim.x =  90.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		pos.x  = 368.0f/640.0f;
		pos.y  = 122.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, -1, EVENT_INTERFACE_FFBs);
		psl->SetState(STATE_SHADOW);
		psl->SetState(STATE_VALUE);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetTabOrder(9);

		ddim.x = 192.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pos.x  = 105.0f/640.0f;
		pos.y  =  71.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_KDEF);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(10);

		UpdateSetupButtons();
	}

	if ( m_phase == PHASE_SETUPs  ||  // setup/sound ?
		 m_phase == PHASE_SETUPss )
	{
		pos.x  = ox+sx*3;
		pos.y  = 180.0f/480.0f;
		ddim.x =  18.0f/640.0f;
		ddim.y = 128.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_VOLSOUND);
		psl->SetState(STATE_SHADOW);
		psl->SetLimit(0.0f, MAXVOLUME);
		psl->SetArrowStep(1.0f);
		psl->SetTabOrder(6);
		pos.y  = 306.0f/480.0f;
		ddim.x = 200.0f/640.0f;
		ddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_VOLSOUND, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

#if 0
		pos.x  = ox+sx*10;
		pos.y  = 180.0f/480.0f;
		ddim.x =  18.0f/640.0f;
		ddim.y = 128.0f/480.0f;
		psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_VOLMUSIC);
		psl->SetState(STATE_SHADOW);
		psl->SetLimit(0.0f, MAXVOLUME);
		psl->SetArrowStep(1.0f);
		psl->SetTabOrder(7);
		pos.y  = 306.0f/480.0f;
		ddim.x = 200.0f/640.0f;
		ddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_VOLMUSIC, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetJustif(1);
#endif

		pos.x  = ox+sx*10;
		pos.y  = 0.60f;
		ddim.x = dim.x*6;
		ddim.y = dim.y*0.5f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SOUND3D);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(8);
#if _FRENCH|_ENGLISH|_DEUTSCH
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_COMMENTS);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(9);
#endif

		ddim.x = dim.x*3;
		ddim.y = dim.y*1;
		pos.x = ox+sx*3;
		pos.y = oy+sy*2;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SILENT);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(10);
		pos.x += ddim.x;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOISY);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(11);

		UpdateSetupButtons();
	}

	if ( m_phase == PHASE_WRITE )
	{
		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		pw->SetName(" ");

		pos.x  = 0.10f;
		pos.y  = 0.44f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.40f;
		pos.y  = 0.06f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		pos.x  =  94.0f/640.0f;
		ddim.x = 452.0f/640.0f;
		pos.y  = 350.0f/480.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_IO_LIST, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);

		pos.y  = 180.0f/480.0f;
		ddim.y = 178.0f/480.0f;
		pa = pw->CreateArray(pos, ddim, 0, EVENT_INTERFACE_IOLIST);
		pa->SetState(STATE_DEFAULT);
		pa->SetState(STATE_SHADOW);
		pa->SetTabs(0,  70.0f/640.0f,  1, FONT_COLOBOT);  // filename
		pa->SetTabs(1, 100.0f/640.0f,  1, FONT_COLOBOT);  // title
		pa->SetTabs(2, 110.0f/640.0f,  1, FONT_COLOBOT);  // car
		pa->SetTabs(3,  70.0f/640.0f,  1, FONT_COLOBOT);  // driver
		pa->SetTabs(4,  60.0f/640.0f, -1, FONT_COLOBOT);  // chrono
		pa->SetSelectCap(TRUE);
		pa->SetFontSize(9.0f);
		UpdateSceneList(m_sel[m_index]);
		pa->SetTabOrder(1);
		UpdateGhostList(TRUE);

		pos.x  =  94.0f/640.0f;
		pos.y  = 136.0f/480.0f;
		ddim.x = 120.0f/640.0f;
		ddim.y =  12.0f/480.0f;
		GetResource(RES_TEXT, RT_IO_NAME, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);

		pos.x  = 214.0f/640.0f;
		pos.y  = 138.0f/480.0f;
		ddim.x = 332.0f/640.0f;
		ddim.y =  26.0f/480.0f;
		pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_IONAME);
		pe->SetFontSize(13.0f);
		pe->SetMaxChar(25);
		pe->SetFocus(TRUE);

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_WCANCEL);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		pos.x  = 336.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_WOK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_READ )
	{
		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		pw->SetName(" ");

		pos.x  = 0.10f;
		pos.y  = 0.44f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.40f;
		pos.y  = 0.06f;
		ddim.x = 0.50f;
		ddim.y = 0.50f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		pos.x  =  94.0f/640.0f;
		ddim.x = 452.0f/640.0f;
		pos.y  = 350.0f/480.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_IO_LIST, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);

		pos.y  = 180.0f/480.0f;
		ddim.y = 178.0f/480.0f;
		pa = pw->CreateArray(pos, ddim, 0, EVENT_INTERFACE_IOLIST);
		pa->SetState(STATE_DEFAULT);
		pa->SetState(STATE_SHADOW);
		pa->SetTabs(0,  70.0f/640.0f,  1, FONT_COLOBOT);  // filename
		pa->SetTabs(1, 100.0f/640.0f,  1, FONT_COLOBOT);  // title
		pa->SetTabs(2, 110.0f/640.0f,  1, FONT_COLOBOT);  // car
		pa->SetTabs(3,  70.0f/640.0f,  1, FONT_COLOBOT);  // driver
		pa->SetTabs(4,  60.0f/640.0f, -1, FONT_COLOBOT);  // chrono
		pa->SetSelectCap(TRUE);
		pa->SetFontSize(9.0f);
		UpdateSceneList(m_sel[m_index]);
		pa->SetTabOrder(1);
		m_bPesetas = FALSE;
		BuyablePerso();
		UpdateGhostList(FALSE);

		pos.y  = 136.0f/480.0f;
		ddim.y =  12.0f/480.0f;
		GetResource(RES_TEXT, RT_IO_RINFO, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(0);

#if _DEMO|_SE
		pos.x  = 123.0f/640.0f;
		pos.y  = 243.0f/480.0f;
		ddim.x = 380.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 9, EVENT_LABEL1);
		pg->SetState(STATE_SHADOW);
		pos.x  = 123.0f/640.0f;
		pos.y  = 235.0f/480.0f;
		ddim.x = 380.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_TEXT, RT_SPEC_DEMO, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetFontSize(15.0f);
#endif

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_RCANCEL);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		pos.x  = 336.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_ROK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		SelectGhostList();

		m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_LOADING )
	{
		pos.x  = 0.35f;
		pos.y  = 0.10f;
		ddim.x = 0.30f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 10, EVENT_WINDOW4);

#if 0
		pos.x  = 0.35f;
		pos.y  = 0.60f;
		ddim.x = 0.30f;
		ddim.y = 0.30f;
		pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x  = 0.35f;
		pos.y  = 0.10f;
		ddim.x = 0.30f;
		ddim.y = 0.30f;
		pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu
#endif

		pos.x  = 204.0f/640.0f;
		pos.y  =  18.0f/480.0f;
		ddim.x = 232.0f/640.0f;
		ddim.y =  42.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 7, EVENT_NULL);
		pg->SetState(STATE_SHADOW);

		pos.x  = 200.0f/640.0f;
		pos.y  =  20.0f/480.0f;
		ddim.x = 240.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_TEXT, RT_DIALOG_LOADING, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetFontSize(12.0f);
		pl->SetJustif(0);

		m_engine->SetBackground("inter01.tga", 0,0, 0,0, TRUE, TRUE);
		m_engine->SetBackForce(TRUE);

		m_loadingCounter = 1;  // laisse le temps de s'afficher !
	}

	if ( m_phase == PHASE_WELCOME1 )
	{
		m_sound->StopMusic();
		m_sound->PlayMusic(11, FALSE);

		pos.x  = 0.0f;
		pos.y  = 0.0f;
		ddim.x = 0.0f;
		ddim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW4);

		m_engine->SetOverColor(RetColor(1.0f), D3DSTATETCb);
		m_engine->SetOverFront(TRUE);

		m_engine->SetBackground("alsyd.tga", 0,0, 0,0, TRUE, FALSE);
		m_engine->SetBackForce(TRUE);
	}
	if ( m_phase == PHASE_WELCOME2 )
	{
#if _ENGLISH
		m_sound->StopMusic();
		m_sound->PlayMusic(11, FALSE);
#endif

		pos.x  = 0.0f;
		pos.y  = 0.0f;
		ddim.x = 0.0f;
		ddim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW4);

		m_engine->SetOverColor(RetColor(1.0f), D3DSTATETCb);
		m_engine->SetOverFront(TRUE);

		m_engine->SetBackground("buzzing.tga", 0,0, 0,0, TRUE, FALSE);
		m_engine->SetBackForce(TRUE);
	}
	if ( m_phase == PHASE_WELCOME3 )
	{
		pos.x  = 0.0f;
		pos.y  = 0.0f;
		ddim.x = 0.0f;
		ddim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW4);

		m_engine->SetOverColor(RetColor(0.0f), D3DSTATETCw);
		m_engine->SetOverFront(TRUE);

#if _FRENCH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _ENGLISH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _DEUTSCH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _ITALIAN
		m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _SPANISH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
#if _PORTUGUESE
		m_engine->SetBackground("epsitecf.tga", 0,0, 0,0, TRUE, FALSE);
#endif
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase >= PHASE_GENERIC1 &&
		 m_phase <= PHASE_GENERIC5 )
	{
		pos.x  = 0.0f;
		pos.y  = 0.0f;
		ddim.x = 0.0f;
		ddim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW4);

#if 0
#if _FULL
		pos.x  =  80.0f/640.0f;
		pos.y  = 240.0f/480.0f;
		ddim.x = 490.0f/640.0f;
		ddim.y = 110.0f/480.0f;
		pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
		pe->SetGenericMode(TRUE);
		pe->SetEditCap(FALSE);
		pe->SetHiliteCap(FALSE);
		pe->SetFontType(FONT_COLOBOT);
		pe->SetFontSize(8.0f);
		pe->ReadText("help\\authors.txt");

		pos.x  =  80.0f/640.0f;
		pos.y  = 140.0f/480.0f;
		ddim.x = 490.0f/640.0f;
		ddim.y = 100.0f/480.0f;
		pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
		pe->SetGenericMode(TRUE);
		pe->SetEditCap(FALSE);
		pe->SetHiliteCap(FALSE);
		pe->SetFontType(FONT_COLOBOT);
		pe->SetFontSize(6.5f);
		pe->ReadText("help\\licences.txt");
#endif
#if _DEMO|_SE
		pos.x  =  80.0f/640.0f;
		pos.y  = 240.0f/480.0f;
		ddim.x = 490.0f/640.0f;
		ddim.y = 110.0f/480.0f;
		pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
		pe->SetGenericMode(TRUE);
		pe->SetEditCap(FALSE);
		pe->SetHiliteCap(FALSE);
		pe->SetFontType(FONT_COLOBOT);
		pe->SetFontSize(8.0f);
		pe->ReadText("help\\demo.txt");

		pos.x  =  80.0f/640.0f;
		pos.y  = 140.0f/480.0f;
		ddim.x = 490.0f/640.0f;
		ddim.y = 100.0f/480.0f;
		pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
		pe->SetGenericMode(TRUE);
		pe->SetEditCap(FALSE);
		pe->SetHiliteCap(FALSE);
		pe->SetFontType(FONT_COLOBOT);
		pe->SetFontSize(8.0f);
		pe->ReadText("help\\authors.txt");
#endif

		pos.x  =  40.0f/640.0f;
		pos.y  =  83.0f/480.0f;
		ddim.x = 246.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_DEV1, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(8.0f);

		pos.y  =  13.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_DEV2, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(8.0f);

		pos.x  = 355.0f/640.0f;
		pos.y  =  83.0f/480.0f;
		ddim.x = 246.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_EDIT1, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(8.0f);

		pos.y  =  13.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_EDIT2, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(8.0f);
#endif

#if _DEMO|_SE
		pos.x  =   4.0f/640.0f;
		pos.y  = 436.0f/480.0f;
		ddim.x =  28.0f/640.0f;
		ddim.y =  28.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, m_phase==PHASE_GENERIC1?31:55, EVENT_INTERFACE_PREV);
//?		pb->SetState(STATE_ENABLE, m_phase != PHASE_GENERIC1);
//?		pb->SetState(STATE_SHADOW);
		pos.x += 28.0f/640.0f;
		pb = pw->CreateButton(pos, ddim, m_phase==PHASE_GENERIC5?31:48, EVENT_INTERFACE_NEXT);
//?		pb->SetState(STATE_ENABLE, m_phase != PHASE_GENERIC5);
//?		pb->SetState(STATE_SHADOW);

		pos.x  = 580.0f/640.0f;
		pos.y  = 436.0f/480.0f;
		ddim.x =  28.0f/640.0f;
		ddim.y =  28.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
//?		pb->SetState(STATE_SHADOW);
		pos.x += 28.0f/640.0f;
		pb = pw->CreateButton(pos, ddim, 11, EVENT_INTERFACE_EXIT);
//?		pb->SetState(STATE_SHADOW);

		pos.x  =  64.0f/640.0f;
		pos.y  = 440.0f/480.0f;
		ddim.x = 512.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 9, EVENT_LABEL1);
//?		pg->SetState(STATE_SHADOW);
		pos.y -= 9.0f/480.0f;
		GetResource(RES_TEXT, RT_GENERIC_DEMO1+(m_phase-PHASE_GENERIC1), name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
		pl->SetFontType(FONT_COLOBOT);
		pl->SetFontSize(11.0f);
#else
		pos.x  =  16.0f/640.0f;
		pos.y  =  16.0f/480.0f;
		ddim.x =  30.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
		pb->SetState(STATE_SHADOW);
#endif

#if _DEMO|_SE
		sprintf(name, "gener%c.tga", '1'+(char)(m_phase-PHASE_GENERIC1));
		m_engine->SetBackground(name, 0,0, 0,0, TRUE, TRUE);
#else
		m_engine->SetBackground("generf.tga", 0,0, 0,0, TRUE, TRUE);
#endif
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_INIT    ||
		 m_phase == PHASE_NAME    ||
		 m_phase == PHASE_MISSION ||
		 m_phase == PHASE_FREE    ||
		 m_phase == PHASE_USER    ||
		 m_phase == PHASE_PROTO   ||
		 m_phase == PHASE_SETUPd  ||
		 m_phase == PHASE_SETUPg  ||
		 m_phase == PHASE_SETUPp  ||
		 m_phase == PHASE_SETUPc  ||
		 m_phase == PHASE_SETUPs  ||
		 m_phase == PHASE_WRITE   ||
		 m_phase == PHASE_READ    ||
		 m_phase == PHASE_LOADING ||
		 m_phase == PHASE_CAR     )
	{
		pos.x  = 540.0f/640.0f;
		pos.y  =   9.0f/480.0f;
		ddim.x =  90.0f/640.0f;
		ddim.y =  14.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 23, EVENT_NULL);

		pos.y -=   7.0f/480.0f;
		GetResource(RES_TEXT, RT_VERSION_ID, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetFontSize(8.0f);
	}

	m_engine->LoadAllTexture();
}


// Traite un événement.
// Retourne FALSE si l'événement a été traîté complètement.

BOOL CMainDialog::EventProcess(const Event &event)
{
	CWindow*	pw;
	CList*		pl;
	CButton*	pb;
	CCheck*		pc;
	CSlider*	ps;
	CObject*	vehicle;
	CPyro*		pyro;
	Event		newEvent;
	EventMsg	window;
	FPOINT		pos;
	float		welcomeLength;
	int			i, err;
	BOOL		bUpDown;

	if ( event.event == EVENT_FRAME )
	{
		m_phaseTime += event.rTime;

//?		if ( m_phase == PHASE_WELCOME1 )  welcomeLength = WELCOME_LENGTH+2.0f;
//?		else                              welcomeLength = WELCOME_LENGTH;
		welcomeLength = WELCOME_LENGTH;

		if ( m_phase == PHASE_WELCOME1 ||
			 m_phase == PHASE_WELCOME2 ||
			 m_phase == PHASE_WELCOME3 )
		{
			float	intensity;
			int		mode = D3DSTATETCb;

			if ( m_phaseTime < 1.5f )
			{
				intensity = 1.0f-(m_phaseTime-0.5f);
			}
			else if ( m_phaseTime < welcomeLength-1.0f )
			{
				intensity = 0.0f;
			}
			else
			{
				intensity = m_phaseTime-(welcomeLength-1.0f);
			}
			if ( intensity < 0.0f )  intensity = 0.0f;
			if ( intensity > 1.0f )  intensity = 1.0f;

			if ( (m_phase == PHASE_WELCOME2 && m_phaseTime > welcomeLength/2.0f) ||
				 m_phase == PHASE_WELCOME3 )
			{
				intensity = 1.0f-intensity;
				mode = D3DSTATETCw;
			}

			m_engine->SetOverColor(RetColor(intensity), mode);
		}

		if ( m_phase == PHASE_WELCOME1 && m_phaseTime >= welcomeLength )
		{
			ChangePhase(PHASE_WELCOME2);
			return TRUE;
		}
		if ( m_phase == PHASE_WELCOME2 && m_phaseTime >= welcomeLength )
		{
			ChangePhase(PHASE_WELCOME3);
			return TRUE;
		}
		if ( m_phase == PHASE_WELCOME3 && m_phaseTime >= welcomeLength )
		{
			ChangePhase(PHASE_NAME);
			return TRUE;
		}

		if ( m_shotDelay > 0 && !m_bDialog )  // copie d'écran à faire ?
		{
			m_shotDelay --;
			if ( m_shotDelay == 0 )
			{
				m_engine->WriteScreenShot(m_shotName, 320, 240);
//?				m_engine->WriteScreenShot(m_shotName, 160, 120);
			}
		}

		if ( m_phase == PHASE_LOADING )
		{
			if ( m_loadingCounter == 0 )
			{
				m_main->ChangePhase(PHASE_SIMUL);
			}
			m_loadingCounter --;
			return FALSE;
		}

		if ( m_phase == PHASE_CAR )
		{
			CObject*	vehicle;
			CPhysics*	physics;
			D3DVECTOR	pos;

			vehicle = m_main->SearchObject(OBJECT_CAR);
			if ( vehicle != 0 )
			{
				if ( m_persoTime == 0.0f )
				{
					pos = vehicle->RetPosition(0);
					pos.y += 10.0f;
					vehicle->SetPosition(0, pos);
				}

				physics = vehicle->RetPhysics();
				if ( physics != 0 )
				{
					m_persoTime += event.rTime;
					m_persoAngle += event.rTime*0.5f;
					CameraPerso();

					if ( m_persoTime > 2.5f && !m_persoRun )
					{
						m_persoRun = TRUE;
						physics->SetForceSlow(TRUE);  // moteur au ralenti
					}
					if ( m_persoTime > 4.0f )
					{
						physics->ForceMotorSpeedZ(sinf((m_persoTime-4.0f)*3.0f));
					}
				}
			}
		}

		m_glintTime += event.rTime*5.0f;
		GlintMove();  // bouge les reflets

//?		FrameParticule(event.rTime);

		if ( m_bDialog )  // dialogue présent ?
		{
			FrameDialog(event.rTime);
		}

		return TRUE;
	}

	if ( event.event == EVENT_MOUSEMOVE )
	{
		m_glintMouse = event.pos;
		NiceParticule(event.pos, event.keyState&KS_MLEFT);
	}

	if ( event.event == EVENT_KEYDOWN )
	{
		window = EVENT_NULL;
		bUpDown = FALSE;
		if ( m_phase == PHASE_INIT )
		{
			window = EVENT_WINDOW4;
			bUpDown = TRUE;
		}
		if ( m_phase == PHASE_NAME    ||
			 m_phase == PHASE_CAR     ||
			 m_phase == PHASE_MISSION ||
			 m_phase == PHASE_FREE    ||
			 m_phase == PHASE_USER    ||
			 m_phase == PHASE_PROTO   ||
			 m_phase == PHASE_SETUPd  ||
			 m_phase == PHASE_SETUPg  ||
			 m_phase == PHASE_SETUPp  ||
			 m_phase == PHASE_SETUPc  ||
			 m_phase == PHASE_SETUPs  ||
			 m_phase == PHASE_SETUPds ||
			 m_phase == PHASE_SETUPgs ||
			 m_phase == PHASE_SETUPps ||
			 m_phase == PHASE_SETUPcs ||
			 m_phase == PHASE_SETUPss ||
			 m_phase == PHASE_WRITE   ||
			 m_phase == PHASE_READ    )
		{
			window = EVENT_WINDOW4;
		}
		if ( m_bDialog )  // dialogue présent ?
		{
			window = EVENT_WINDOW9;
			bUpDown = TRUE;
		}
		if ( window != EVENT_NULL )
		{
			if ( (event.param == VK_UP && bUpDown ) ||
				 event.param == VK_LEFT  ||
				 (event.param == VK_TAB && (event.keyState&KS_SHIFT) != 0) ||
				 (event.param == VK_JUP && bUpDown ) ||
				 event.param == VK_JLEFT )
			{
				ChangeTabOrder(window, -1, event.param);
			}
			if ( (event.param == VK_DOWN && bUpDown ) ||
				 event.param == VK_RIGHT  ||
				 (event.param == VK_TAB && (event.keyState&KS_SHIFT) == 0) ||
				 (event.param == VK_JDOWN && bUpDown ) ||
				 event.param == VK_JRIGHT )
			{
				ChangeTabOrder(window, 1, event.param);
			}
		}
	}

	if ( m_bDialog )  // dialogue présent ?
	{
		m_interface->EventProcess(event);

		if ( event.event == EVENT_DIALOG_OK )
		{
			StopDialog();
			if ( m_phase == PHASE_NAME )
			{
				if ( m_bDialogCreate )
				{
					NameCreate();
				}
				if ( m_bDialogDelete )
				{
					NameDelete();
				}
				if ( m_bDialogKid )
				{
					m_main->ChangePhase(PHASE_INIT);
				}
			}
			else if ( m_phase == PHASE_INIT )
			{
//?				m_event->MakeEvent(newEvent, EVENT_QUIT);
//?				m_event->AddEvent(newEvent);
				m_main->ChangePhase(PHASE_GENERIC1);
			}
			else if ( m_phase == PHASE_SIMUL )
			{
				m_main->ChangePhase(PHASE_TERM);
			}
			else if ( m_phase == PHASE_WRITE )
			{
				if ( DeleteGhostFile() )
				{
					m_main->ChangePhase(PHASE_WIN);
				}
			}
		}
		if ( event.event == EVENT_KEYDOWN && event.param == VK_RETURN )
		{
			if ( m_phase == PHASE_NAME && m_bDialogCreate )
			{
				StopDialog();
				NameCreate();
			}
		}
		if ( event.event == EVENT_DIALOG_CANCEL ||
			 (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			 (event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			StopDialog();
			pos.x = 630.0f/640.0f;
			pos.y =  10.0f/480.0f;
			m_engine->MoveMousePos(pos);  // met la souris en bas à droite
		}
		if ( event.event == EVENT_INTERFACE_SETUP )
		{
			StopDialog();
			StartSuspend();
			if ( m_phaseSetup == PHASE_SETUPd )  ChangePhase(PHASE_SETUPds);
			if ( m_phaseSetup == PHASE_SETUPg )  ChangePhase(PHASE_SETUPgs);
			if ( m_phaseSetup == PHASE_SETUPp )  ChangePhase(PHASE_SETUPps);
			if ( m_phaseSetup == PHASE_SETUPc )  ChangePhase(PHASE_SETUPcs);
			if ( m_phaseSetup == PHASE_SETUPs )  ChangePhase(PHASE_SETUPss);
		}
		if ( event.event == EVENT_INTERFACE_AGAIN )
		{
			StopDialog();
			m_main->ChangePhase(PHASE_LOADING);
		}

		return FALSE;
	}

	if ( !m_engine->RetMouseHide() &&
		 !m_interface->EventProcess(event) )
	{
		return FALSE;
	}

	if ( m_phase == PHASE_INIT )
	{
		switch( event.event )
		{
			case EVENT_KEYDOWN:
				if ( event.param == VK_ESCAPE )
				{
//?					StartQuit();  // voulez-vous quitter ?
					m_sound->Play(SOUND_TZOING);
					m_main->ChangePhase(PHASE_GENERIC1);
				}
				break;

			case EVENT_INTERFACE_QUIT:
//?				StartQuit();  // voulez-vous quitter ?
				m_sound->Play(SOUND_TZOING);
				m_main->ChangePhase(PHASE_GENERIC1);
				break;

			case EVENT_INTERFACE_MISSION:
				m_main->ChangePhase(PHASE_MISSION);
				break;

			case EVENT_INTERFACE_FREE:
				m_main->ChangePhase(PHASE_FREE);
				break;

			case EVENT_INTERFACE_DUEL:
				m_main->ChangePhase(PHASE_READ);
				break;

			case EVENT_INTERFACE_USER:
				m_main->ChangePhase(PHASE_USER);
				break;

			case EVENT_INTERFACE_PROTO:
				m_main->ChangePhase(PHASE_PROTO);
				break;

			case EVENT_INTERFACE_SETUP:
				m_main->ChangePhase(m_phaseSetup);
				break;

			case EVENT_INTERFACE_NAME:
				m_main->ChangePhase(PHASE_NAME);
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_NAME )
	{
		switch( event.event )
		{
			case EVENT_KEYDOWN:
				if ( event.param == VK_ESCAPE  ||
					 event.param == VK_DEFAULT )
				{
					if ( NameSelect() )
					{
						m_main->ChangePhase(PHASE_INIT);
					}
				}
				break;

			case EVENT_INTERFACE_LEVEL1:
				LevelSelect(1);
				break;
			case EVENT_INTERFACE_LEVEL2:
				LevelSelect(2);
				break;
			case EVENT_INTERFACE_LEVEL3:
				LevelSelect(3);
				break;
			case EVENT_INTERFACE_LEVEL4:
				LevelSelect(4);
				break;

			case EVENT_INTERFACE_NLIST:
				NameSelect();
				UpdateNameControl();
				break;

			case EVENT_INTERFACE_NOK:
				if ( NameSelect() )
				{
					if ( RetLevel() == 1 )
					{
						StartKidLevel();
					}
					else
					{
						m_main->ChangePhase(PHASE_INIT);
					}
				}
				break;

			case EVENT_INTERFACE_NCREATE:
				StartCreateGame();
				break;

			case EVENT_INTERFACE_NDELETE:
				pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
				if ( pw == 0 )  break;
				pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
				if ( pl == 0 )  break;
				StartDeleteGame(pl->RetName(pl->RetSelect()));
				break;
		}
	}

	if ( m_phase == PHASE_CAR )
	{
		switch( event.event )
		{
			case EVENT_KEYDOWN:
				if ( event.param == VK_ESCAPE  ||
					 event.param == VK_BUTTON9 )
				{
					m_perso = m_persoCopy;
					WriteGamerInfo();
					m_main->ChangePhase(m_phasePerso);
					return FALSE;
				}
				break;

			case EVENT_INTERFACE_PELEV:
				ElevationPerso();
				break;

			case EVENT_INTERFACE_PCOLOR0:
			case EVENT_INTERFACE_PCOLOR1:
			case EVENT_INTERFACE_PCOLOR2:
			case EVENT_INTERFACE_PCOLOR3:
			case EVENT_INTERFACE_PCOLOR4:
			case EVENT_INTERFACE_PCOLOR5:
			case EVENT_INTERFACE_PCOLOR6:
			case EVENT_INTERFACE_PCOLOR7:
			case EVENT_INTERFACE_PCOLOR8:
			case EVENT_INTERFACE_PCOLOR9:
			case EVENT_INTERFACE_PCOLOR10:
			case EVENT_INTERFACE_PCOLOR11:
			case EVENT_INTERFACE_PCOLOR12:
			case EVENT_INTERFACE_PCOLOR13:
			case EVENT_INTERFACE_PCOLOR14:
			case EVENT_INTERFACE_PCOLOR15:
			case EVENT_INTERFACE_PCOLOR16:
			case EVENT_INTERFACE_PCOLOR17:
			case EVENT_INTERFACE_PCOLOR18:
			case EVENT_INTERFACE_PCOLOR19:
				FixPerso(event.event-EVENT_INTERFACE_PCOLOR0);
				WriteGamerInfo();
				m_main->ScenePerso();
				UpdatePerso();
				m_persoTime = 0.01f;
				m_persoRun = FALSE;
				vehicle = m_main->SearchObject(OBJECT_CAR);
				if ( vehicle != 0 )
				{
					m_sound->Play(SOUND_FINDING);
					pyro = new CPyro(m_iMan);
					pyro->Create(PT_PAINTING, vehicle, 0.0f);
				}
				break;

			case EVENT_INTERFACE_PSUBMOD0:
			case EVENT_INTERFACE_PSUBMOD1:
			case EVENT_INTERFACE_PSUBMOD2:
			case EVENT_INTERFACE_PSUBMOD3:
				m_perso.subModel[m_perso.selectCar] = event.event-EVENT_INTERFACE_PSUBMOD0;
				DefPerso(m_perso.selectCar);  // met la couleur standard
				WriteGamerInfo();
				m_main->ScenePerso();
				UpdatePerso();
				m_persoTime = 0.01f;
				m_persoRun = FALSE;
				vehicle = m_main->SearchObject(OBJECT_CAR);
				if ( vehicle != 0 )
				{
					m_sound->Play(SOUND_FINDING);
					pyro = new CPyro(m_iMan);
					pyro->Create(PT_PAINTING, vehicle, 0.0f);
				}
				break;

			case EVENT_INTERFACE_GHOSTm:
				m_bGhostEnable = !m_bGhostEnable;
				UpdateSceneGhost(m_sel[m_index]);
				break;

			case EVENT_INTERFACE_PPREV:
				if ( m_perso.selectCar > 0 )
				{
					NextPerso(-1);
					m_main->ScenePerso();
					UpdatePerso();
					UpdateSceneGhost(m_sel[m_index]);
					m_persoTime = 0.0f;
					m_persoRun = FALSE;
				}
				break;
			case EVENT_INTERFACE_PNEXT:
#if _SE
				if ( m_perso.selectCar < m_perso.total-1 )
#else
				if ( m_perso.selectCar < m_perso.buyable-1 ||
					 m_perso.selectCar < m_perso.bonus-1   )
#endif
				{
					NextPerso(1);
					m_main->ScenePerso();
					UpdatePerso();
					UpdateSceneGhost(m_sel[m_index]);
					m_persoTime = 0.0f;
					m_persoRun = FALSE;
				}
				break;

			case EVENT_INTERFACE_POK:
				SelectPerso();
				m_bDuel = FALSE;
				LaunchSimul();
				break;
		}

		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
		if ( pw == 0 )  return FALSE;

		if ( event.event == pw->RetEventMsgClose() )
		{
			m_perso = m_persoCopy;
			WriteGamerInfo();
			m_main->ChangePhase(m_phasePerso);
			return FALSE;
		}
	}

	if ( m_phase == PHASE_MISSION ||
		 m_phase == PHASE_FREE    ||
		 m_phase == PHASE_USER    ||
		 m_phase == PHASE_PROTO   )
	{
		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
		if ( pw == 0 )  return FALSE;

		if ( event.event == pw->RetEventMsgClose() ||
			 event.event == EVENT_INTERFACE_BACK   ||
			(event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			(event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			m_main->ChangePhase(PHASE_INIT);
			return FALSE;
		}

		if ( event.event == EVENT_KEYDOWN && event.param == VK_DEFAULT )
		{
			m_phaseTerm = m_phase;
			m_bDuel = FALSE;
			m_main->ChangePhase(PHASE_CAR);
			return FALSE;
		}

		switch( event.event )
		{
			case EVENT_INTERFACE_LIST:
				pl = (CList*)pw->SearchControl(EVENT_INTERFACE_LIST);
				if ( pl == 0 )  break;
				i = pl->RetSelect();
				m_sel[m_index] = m_listInfo[i].chap*10+m_listInfo[i].scene;
				WriteGamerMission();
				UpdateSceneResume(m_sel[m_index]);
				UpdateSceneImage(m_sel[m_index]);
				UpdateScenePlay(m_sel[m_index]);
				UpdateScenePesetasMax(m_sel[m_index]);
				break;

			case EVENT_INTERFACE_PLAY:
				if ( m_phase == PHASE_PROTO && m_sel[m_index] == 11 )
				{
					m_main->ChangePhase(PHASE_MODEL);
					break;
				}
				m_phaseTerm = m_phase;
				m_bDuel = FALSE;
				m_main->ChangePhase(PHASE_CAR);
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_SETUPd ||
		 m_phase == PHASE_SETUPg ||
		 m_phase == PHASE_SETUPp ||
		 m_phase == PHASE_SETUPc ||
		 m_phase == PHASE_SETUPs )
	{
		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
		if ( pw == 0 )  return FALSE;

		if ( event.event == pw->RetEventMsgClose() ||
			 event.event == EVENT_INTERFACE_OK     ||
			(event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			(event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			SetupMemorize();
			m_engine->ApplyChange();
			m_main->ChangePhase(PHASE_INIT);
			return FALSE;
		}

		switch( event.event )
		{
			case EVENT_INTERFACE_SETUPd:
				m_main->ChangePhase(PHASE_SETUPd);
				break;

			case EVENT_INTERFACE_SETUPg:
				m_main->ChangePhase(PHASE_SETUPg);
				break;

			case EVENT_INTERFACE_SETUPp:
				m_main->ChangePhase(PHASE_SETUPp);
				break;

			case EVENT_INTERFACE_SETUPc:
				m_main->ChangePhase(PHASE_SETUPc);
				break;

			case EVENT_INTERFACE_SETUPs:
				m_main->ChangePhase(PHASE_SETUPs);
				break;
		}
	}

	if ( m_phase == PHASE_SETUPds ||
		 m_phase == PHASE_SETUPgs ||
		 m_phase == PHASE_SETUPps ||
		 m_phase == PHASE_SETUPcs ||
		 m_phase == PHASE_SETUPss )
	{
		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
		if ( pw == 0 )  return FALSE;

		if ( event.event == pw->RetEventMsgClose() ||
			 event.event == EVENT_INTERFACE_OK     ||
			(event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			(event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			SetupMemorize();
			m_engine->ApplyChange();
			m_interface->DeleteControl(EVENT_WINDOW4);
			ChangePhase(PHASE_SIMUL);
			StopSuspend();
			return FALSE;
		}

		switch( event.event )
		{
			case EVENT_INTERFACE_SETUPd:
				ChangePhase(PHASE_SETUPds);
				break;

			case EVENT_INTERFACE_SETUPg:
				ChangePhase(PHASE_SETUPgs);
				break;

			case EVENT_INTERFACE_SETUPp:
				ChangePhase(PHASE_SETUPps);
				break;

			case EVENT_INTERFACE_SETUPc:
				ChangePhase(PHASE_SETUPcs);
				break;

			case EVENT_INTERFACE_SETUPs:
				ChangePhase(PHASE_SETUPss);
				break;
		}
	}

	if ( m_phase == PHASE_SETUPd  ||  // setup/display ?
		 m_phase == PHASE_SETUPds )
	{
		switch( event.event )
		{
			case EVENT_LIST1:
			case EVENT_LIST2:
				UpdateApply();
				break;

			case EVENT_INTERFACE_FULL:
				pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
				if ( pw == 0 )  break;
				pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FULL);
				if ( pc == 0 )  break;
				pl = (CList*)pw->SearchControl(EVENT_LIST2);
				if ( pl == 0 )  break;
				if ( pc->TestState(STATE_CHECK) )
				{
					pc->ClearState(STATE_CHECK);  // fenêtré
					pl->ClearState(STATE_ENABLE);
				}
				else
				{
					pc->SetState(STATE_CHECK);  // plein écran
					pl->SetState(STATE_ENABLE);
				}
				UpdateApply();
				break;

			case EVENT_INTERFACE_APPLY:
				pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
				if ( pw == 0 )  break;
				pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_APPLY);
				if ( pb == 0 )  break;
				pb->ClearState(STATE_PRESS);
				pb->ClearState(STATE_HILIGHT);
				ChangeDisplay();
				UpdateApply();
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_SETUPg  ||  // setup/graphic ?
		 m_phase == PHASE_SETUPgs )
	{
		switch( event.event )
		{
			case EVENT_INTERFACE_SHADOW:
				m_engine->SetShadow(!m_engine->RetShadow());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_DIRTY:
				m_engine->SetDirty(!m_engine->RetDirty());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_FOG:
				m_engine->SetFog(!m_engine->RetFog());
				m_camera->SetOverBaseColor(RetColor(RetColor(0.0f)));
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_LENS:
				m_engine->SetLensMode(!m_engine->RetLensMode());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_SKY:
				m_engine->SetSkyMode(!m_engine->RetSkyMode());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_PLANET:
				m_engine->SetPlanetMode(!m_engine->RetPlanetMode());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_LIGHT:
				m_engine->SetLightMode(!m_engine->RetLightMode());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_SUPER:
				m_engine->SetSuperDetail(!m_engine->RetSuperDetail());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_PARTI:
			case EVENT_INTERFACE_WHEEL:
			case EVENT_INTERFACE_CLIP:
			case EVENT_INTERFACE_DETAIL:
			case EVENT_INTERFACE_GADGET:
			case EVENT_INTERFACE_TEXTURE:
				ChangeSetupButtons();
				break;

			case EVENT_INTERFACE_MIN:
				ChangeSetupQuality(-1);
				UpdateSetupButtons();
				break;
			case EVENT_INTERFACE_NORM:
				ChangeSetupQuality(0);
				UpdateSetupButtons();
				break;
			case EVENT_INTERFACE_MAX:
				ChangeSetupQuality(1);
				UpdateSetupButtons();
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_SETUPp  ||  // setup/jeu ?
		 m_phase == PHASE_SETUPps )
	{
		switch( event.event )
		{
			case EVENT_INTERFACE_TOOLTIP:
				m_bTooltip = !m_bTooltip;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_GLINT:
				m_bGlint = !m_bGlint;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_RAIN:
				m_bRain = !m_bRain;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_MOUSE:
				m_engine->SetNiceMouse(!m_engine->RetNiceMouse());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_MOVIES:
				m_bMovies = !m_bMovies;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_CBACK:
				m_defCamera = CAMERA_BACK;
				m_initCamera = m_defCamera;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_CBOARD:
				m_defCamera = CAMERA_ONBOARD;
				m_initCamera = m_defCamera;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_EFFECT:
				m_bEffect = !m_bEffect;
				m_camera->SetEffect(m_bEffect);
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_FLASH:
				m_bFlash = !m_bFlash;
				m_camera->SetFlash(m_bFlash);
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_BLAST:
				m_bMotorBlast = !m_bMotorBlast;
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_SETUPc  ||  // setup/commandes ?
		 m_phase == PHASE_SETUPcs )
	{
		switch( event.event )
		{
			case EVENT_INTERFACE_KSCROLL:
				UpdateKey();
				break;

			case EVENT_INTERFACE_KLEFT:
			case EVENT_INTERFACE_KRIGHT:
			case EVENT_INTERFACE_KUP:
			case EVENT_INTERFACE_KDOWN:
			case EVENT_INTERFACE_KBRAKE:
			case EVENT_INTERFACE_KHORN:
			case EVENT_INTERFACE_KCAMERA:
			case EVENT_INTERFACE_KQUIT:
			case EVENT_INTERFACE_KHELP:
				ChangeKey(event.event);
				UpdateKey();
				break;

			case EVENT_INTERFACE_KDEF:
				m_engine->ResetKey();
				m_engine->SetForce(1.0f);
				UpdateSetupButtons();
				UpdateKey();
				break;

			case EVENT_INTERFACE_STEERING:
				if ( m_engine->RetJoystick() == 1 )
				{
					m_engine->SetJoystick(0);
				}
				else
				{
					m_engine->SetJoystick(1);
				}
				UpdateSetupButtons();
				UpdateKey();
				break;
			case EVENT_INTERFACE_JOYPAD:
				if ( m_engine->RetJoystick() == 2 )
				{
					m_engine->SetJoystick(0);
				}
				else
				{
					m_engine->SetJoystick(2);
				}
				UpdateSetupButtons();
				UpdateKey();
				break;
			case EVENT_INTERFACE_FFBc:
				if ( m_engine->RetFFB() )
				{
					m_engine->SetFFB(FALSE);
				}
				else
				{
					m_engine->SetFFB(TRUE);
				}
				m_engine->SetJoystick(m_engine->RetJoystick());
				UpdateSetupButtons();
				UpdateKey();
				break;
			case EVENT_INTERFACE_FFBs:
				pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
				if ( pw == 0 )  break;
				ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_FFBs);
				if ( ps == 0 )  break;
				m_engine->SetForce(ps->RetVisibleValue());
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_SETUPs  ||  // setup/sound ?
		 m_phase == PHASE_SETUPss )
	{
		switch( event.event )
		{
			case EVENT_INTERFACE_VOLSOUND:
			case EVENT_INTERFACE_VOLMUSIC:
				ChangeSetupButtons();
				break;

			case EVENT_INTERFACE_SOUND3D:
				m_sound->SetSound3D(!m_sound->RetSound3D());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_COMMENTS:
				m_sound->SetComments(!m_sound->RetComments());
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_SILENT:
				m_sound->SetAudioVolume(0);
				m_sound->SetMidiVolume(0);
				UpdateSetupButtons();
				break;
			case EVENT_INTERFACE_NOISY:
				m_sound->SetAudioVolume(MAXVOLUME);
				m_sound->SetMidiVolume(MAXVOLUME*3/4);
				UpdateSetupButtons();
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_WRITE )
	{
		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
		if ( pw == 0 )  return FALSE;

		if ( event.event == pw->RetEventMsgClose() ||
			 event.event == EVENT_INTERFACE_OK     ||
			(event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			(event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			m_main->ChangePhase(PHASE_WIN);
			return FALSE;
		}

		switch( event.event )
		{
			case EVENT_INTERFACE_IOLIST:
				SelectGhostList();
				break;
			case EVENT_INTERFACE_WOK:
				err = WriteGhostFile();
				if ( err == 0 )  // ok ?
				{
					m_main->ChangePhase(PHASE_WIN);
				}
				if ( err == 2 )  // fichier existe déjà ?
				{
					StartDeleteFile(m_ghostName);
				}
				break;
			case EVENT_INTERFACE_WCANCEL:
				m_main->ChangePhase(PHASE_WIN);
				break;
		}
	}

	if ( m_phase == PHASE_READ )
	{
		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
		if ( pw == 0 )  return FALSE;

		if ( event.event == pw->RetEventMsgClose() ||
			 event.event == EVENT_INTERFACE_OK     ||
			(event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			(event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			m_main->ChangePhase(PHASE_INIT);
			return FALSE;
		}

		if ( event.event == EVENT_KEYDOWN && event.param == VK_DEFAULT )
		{
			if ( ReadGhostFile() )
			{
				LaunchSimul();
				return FALSE;
			}
		}

		switch( event.event )
		{
			case EVENT_INTERFACE_IOLIST:
				SelectGhostList();
				break;
			case EVENT_INTERFACE_ROK:
				if ( ReadGhostFile() )
				{
					LaunchSimul();
				}
				break;
			case EVENT_INTERFACE_RCANCEL:
				m_main->ChangePhase(PHASE_INIT);
				break;
		}
	}

	if ( m_phase == PHASE_WELCOME1 )
	{
		if ( event.event == EVENT_KEYDOWN     ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
			ChangePhase(PHASE_WELCOME2);
			return TRUE;
		}
	}
	if ( m_phase == PHASE_WELCOME2 )
	{
		if ( event.event == EVENT_KEYDOWN     ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
			ChangePhase(PHASE_WELCOME3);
			return TRUE;
		}
	}
	if ( m_phase == PHASE_WELCOME3 )
	{
		if ( event.event == EVENT_KEYDOWN     ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
			ChangePhase(PHASE_NAME);
			return TRUE;
		}
	}

	if ( m_phase >= PHASE_GENERIC1 &&
		 m_phase <= PHASE_GENERIC5 )
	{
		if ( event.event == EVENT_INTERFACE_ABORT )
		{
			ChangePhase(PHASE_INIT);
		}
#if _DEMO|_SE
		if ( event.event == EVENT_INTERFACE_EXIT )
		{
			m_event->MakeEvent(newEvent, EVENT_QUIT);
			m_event->AddEvent(newEvent);
		}
		if ( event.event == EVENT_INTERFACE_PREV )
		{
			if ( m_phase == PHASE_GENERIC1 )
			{
			}
			else if ( m_phase == PHASE_GENERIC2 )
			{
				ChangePhase(PHASE_GENERIC1);
			}
			else if ( m_phase == PHASE_GENERIC3 )
			{
				ChangePhase(PHASE_GENERIC2);
			}
			else if ( m_phase == PHASE_GENERIC4 )
			{
				ChangePhase(PHASE_GENERIC3);
			}
			else
			{
				ChangePhase(PHASE_GENERIC4);
			}
		}
		if ( event.event == EVENT_INTERFACE_NEXT )
		{
			if ( m_phase == PHASE_GENERIC1 )
			{
				ChangePhase(PHASE_GENERIC2);
			}
			else if ( m_phase == PHASE_GENERIC2 )
			{
				ChangePhase(PHASE_GENERIC3);
			}
			else if ( m_phase == PHASE_GENERIC3 )
			{
				ChangePhase(PHASE_GENERIC4);
			}
			else if ( m_phase == PHASE_GENERIC4 )
			{
				ChangePhase(PHASE_GENERIC5);
			}
			else
			{
			}
		}
#endif

		if ( event.event == EVENT_KEYDOWN )
		{
			if ( event.param == VK_ESCAPE  ||
				 event.param == VK_BUTTON2 ||
				 event.param == VK_BUTTON9 )
			{
				ChangePhase(PHASE_INIT);
			}
			else
			{
#if _DEMO|_SE
				if ( m_phase == PHASE_GENERIC1 )
				{
					ChangePhase(PHASE_GENERIC2);
				}
				else if ( m_phase == PHASE_GENERIC2 )
				{
					ChangePhase(PHASE_GENERIC3);
				}
				else if ( m_phase == PHASE_GENERIC3 )
				{
					ChangePhase(PHASE_GENERIC4);
				}
				else if ( m_phase == PHASE_GENERIC4 )
				{
					ChangePhase(PHASE_GENERIC5);
				}
				else
				{
					m_event->MakeEvent(newEvent, EVENT_QUIT);
					m_event->AddEvent(newEvent);
				}
#else
				m_event->MakeEvent(newEvent, EVENT_QUIT);
				m_event->AddEvent(newEvent);
#endif
			}
		}

		if ( event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
#if _DEMO|_SE
			if ( m_phase == PHASE_GENERIC1 )
			{
				ChangePhase(PHASE_GENERIC2);
			}
			else if ( m_phase == PHASE_GENERIC2 )
			{
				ChangePhase(PHASE_GENERIC3);
			}
			else if ( m_phase == PHASE_GENERIC3 )
			{
				ChangePhase(PHASE_GENERIC4);
			}
			else if ( m_phase == PHASE_GENERIC4 )
			{
				ChangePhase(PHASE_GENERIC5);
			}
			else
			{
				m_event->MakeEvent(newEvent, EVENT_QUIT);
				m_event->AddEvent(newEvent);
			}
#else
			m_event->MakeEvent(newEvent, EVENT_QUIT);
			m_event->AddEvent(newEvent);
#endif
		}
	}

	return TRUE;
}

// Change le bouton que a le focus.

void CMainDialog::ChangeTabOrder(EventMsg window, int dir, int param)
{
	CWindow*	pw;
	CControl*	pc;
	CControl*	pn;
	int			rank, i;

	pw = (CWindow*)m_interface->SearchControl(window);
	if ( pw == 0 )  return;

	pc = pw->SearchControl();  // cherche bouton avec le focus
	if ( pc == 0 )  return;

	rank = pc->RetTabOrder();
	if ( rank == -1 )  return;

	for ( i=0 ; i<30 ; i++ )
	{
		rank += dir;
		if ( rank <  0 )  rank = 29;
		if ( rank > 29 )  rank =  0;
		pn = pw->SearchControl(rank);  // cherche le nouveau bouton
		if ( pn == 0 )  continue;
		if ( !pn->TestState(STATE_VISIBLE) )  continue;
		if ( !pn->TestState(STATE_ENABLE ) )  continue;
		break;
	}
	if ( pn == 0 ) return;

	pc->SetFocus(FALSE);
	pn->SetFocus(TRUE);
}

// Débute une mission.

void CMainDialog::LaunchSimul()
{
	if ( m_bDuel )
	{
		if ( m_duelType == 's' )  strcpy(m_sceneName, "scene");
		if ( m_duelType == 'f' )  strcpy(m_sceneName, "free");
		if ( m_duelType == 'u' )  strcpy(m_sceneName, "user");
		if ( m_duelType == 'p' )  strcpy(m_sceneName, "proto");
		m_sceneRank = m_duelMission;
	}
	else
	{
		if ( m_phaseTerm == PHASE_MISSION )  strcpy(m_sceneName, "scene");
		if ( m_phaseTerm == PHASE_FREE    )  strcpy(m_sceneName, "free");
		if ( m_phaseTerm == PHASE_USER    )  strcpy(m_sceneName, "user");
		if ( m_phaseTerm == PHASE_PROTO   )  strcpy(m_sceneName, "proto");
		m_sceneRank = m_sel[m_index];
	}
	m_main->ChangePhase(PHASE_LOADING);
}


// Fait bouger les reflets.

void CMainDialog::GlintMove()
{
	CWindow*	pw;
	CGroup*		pg;
	FPOINT		pos, dim, zoom;

	if ( m_phase == PHASE_SIMUL )  return;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	if ( m_phase == PHASE_INIT )
	{
		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTl);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.23f);
			zoom.y = sinf(m_glintTime*0.37f);
			pos.x = 0.35f;
			pos.y = 0.90f;
			dim.x = 0.30f-0.10f*(zoom.x+1.0f)/2.0f;
			dim.y = 0.50f-0.30f*(zoom.y+1.0f)/2.0f;
			pos.y -= dim.y;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}

		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.21f);
			zoom.y = sinf(m_glintTime*0.26f);
			pos.x = 0.65f;
			pos.y = 0.10f;
			dim.x = 0.30f-0.10f*(zoom.x+1.0f)/2.0f;
			dim.y = 0.50f-0.30f*(zoom.y+1.0f)/2.0f;
			pos.x -= dim.x;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}
	}

	if ( m_phase == PHASE_NAME )
	{
		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTl);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.22f);
			zoom.y = sinf(m_glintTime*0.37f);
			pos.x = 0.10f;
			pos.y = 0.90f;
			dim.x = 0.60f+0.30f*zoom.x;
			dim.y = 0.60f+0.30f*zoom.y;
			pos.y -= dim.y;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}

		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.19f);
			zoom.y = sinf(m_glintTime*0.28f);
			pos.x = 0.90f;
			pos.y = 0.10f;
			dim.x = 0.60f+0.30f*zoom.x;
			dim.y = 0.60f+0.30f*zoom.y;
			pos.x -= dim.x;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}
	}

	if ( m_phase == PHASE_FREE    ||
		 m_phase == PHASE_USER    ||
		 m_phase == PHASE_PROTO   ||
		 m_phase == PHASE_CAR     )
	{
		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTl);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.22f);
			zoom.y = sinf(m_glintTime*0.37f);
			pos.x = 0.10f;
			pos.y = 0.90f;
			dim.x = 0.60f+0.30f*zoom.x;
			dim.y = 0.60f+0.30f*zoom.y;
			pos.y -= dim.y;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}

		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.19f);
			zoom.y = sinf(m_glintTime*0.28f);
			pos.x = 0.90f;
			pos.y = 0.10f;
			dim.x = 0.60f+0.30f*zoom.x;
			dim.y = 0.60f+0.30f*zoom.y;
			pos.x -= dim.x;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}
	}

	if ( m_phase == PHASE_SETUPd  ||
		 m_phase == PHASE_SETUPg  ||
		 m_phase == PHASE_SETUPp  ||
		 m_phase == PHASE_SETUPc  ||
		 m_phase == PHASE_SETUPs  ||
		 m_phase == PHASE_SETUPds ||
		 m_phase == PHASE_SETUPgs ||
		 m_phase == PHASE_SETUPps ||
		 m_phase == PHASE_SETUPcs ||
		 m_phase == PHASE_SETUPss )
	{
		pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GLINTr);
		if ( pg != 0 )
		{
			zoom.x = sinf(m_glintTime*0.29f);
			zoom.y = sinf(m_glintTime*0.14f);
			pos.x = 0.90f;
			pos.y = 0.10f;
			dim.x = 0.40f+0.20f*zoom.x;
			dim.y = 0.40f+0.20f*zoom.y;
			pos.x -= dim.x;
			pg->SetPos(pos);
			pg->SetDim(dim);
		}
	}
}


// Retourne la position pour un son.

D3DVECTOR SoundPos(FPOINT pos)
{
	D3DVECTOR	s;

	s.x = (pos.x-0.5f)*2.0f;
	s.y = (pos.y-0.5f)*2.0f;
	s.z = 0.0f;

	return s;
}

// Retourne une position aléatoire pour un son.

D3DVECTOR SoundRand()
{
	D3DVECTOR	s;

	s.x = (Rand()-0.5f)*2.0f;
	s.y = (Rand()-0.5f)*2.0f;
	s.z = 0.0f;

	return s;
}

// Fait évoluer qq joiles particules.

void CMainDialog::FrameParticule(float rTime)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		*pParti, *pGlint;
	int			 nParti,  nGlint;
	int			i, r, ii;

	static float partiPosInit[1+5*12] =
	{ //  x       x      t2    t2   type
		 12.0f,
		607.0f, 164.0f, 0.2f, 0.8f, 1.0f,  // câble sup.
		604.0f, 205.0f, 0.1f, 0.3f, 1.0f,  // câble mid.
		603.0f, 247.0f, 0.1f, 0.3f, 1.0f,  // câble inf.
		119.0f, 155.0f, 0.2f, 0.4f, 2.0f,  // tuyau gauche
		366.0f,  23.0f, 0.5f, 1.5f, 4.0f,  // tuyau sup.
		560.0f, 414.0f, 0.1f, 0.1f, 1.0f,  // bouton inf/droite
		 20.0f, 413.0f, 0.1f, 0.1f, 2.0f,  // bouton inf/gauche
		 39.0f,  78.0f, 0.1f, 0.2f, 1.0f,  // pot gauche
		 39.0f,  78.0f, 0.5f, 0.9f, 1.0f,  // pot gauche
		170.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // fumée gauche
		170.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // fumée gauche
		474.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // fumée droite
	};

	static float glintPosInit[1+2*14] =
	{
		 14.0f,
		 15.0f, 407.0f,
		 68.0f, 417.0f,
		548.0f,  36.0f,
		611.0f,  37.0f,
		611.0f, 100.0f,
		611.0f, 395.0f,
		 36.0f,  35.0f,
		166.0f,  55.0f,
		166.0f,  94.0f,
		477.0f,  56.0f,
		 31.0f, 190.0f,
		 32.0f, 220.0f,
		 65.0f, 221.0f,
		 65.0f, 250.0f,
	};

	static float partiPosBig[1+5*12] =
	{ //  x       x      t2    t2   type
		 12.0f,
		607.0f, 164.0f, 0.2f, 0.8f, 1.0f,  // câble sup.
		604.0f, 205.0f, 0.1f, 0.3f, 1.0f,  // câble mid.
		603.0f, 247.0f, 0.1f, 0.3f, 1.0f,  // câble inf.
		 64.0f, 444.0f, 0.2f, 0.8f, 1.0f,  // câble bas gauche
		113.0f, 449.0f, 0.1f, 0.3f, 1.0f,  // câble bas gauche
		340.0f, 463.0f, 0.2f, 0.8f, 1.0f,  // câble bas milieu
		 36.0f, 155.0f, 0.2f, 0.4f, 2.0f,  // tuyau gauche
		366.0f,  23.0f, 0.5f, 1.5f, 4.0f,  // tuyau sup.
		612.0f, 414.0f, 0.1f, 0.1f, 1.0f,  // bouton inf/droite
		 20.0f, 413.0f, 0.1f, 0.1f, 2.0f,  // bouton inf/gauche
		 39.0f,  78.0f, 0.1f, 0.2f, 1.0f,  // pot gauche
		 39.0f,  78.0f, 0.5f, 0.9f, 1.0f,  // pot gauche
	};

	static float glintPosBig[1+2*12] =
	{
		 12.0f,
		 15.0f, 407.0f,
		 48.0f, 399.0f,
		611.0f,  37.0f,
		611.0f, 100.0f,
		611.0f, 395.0f,
		 36.0f,  35.0f,
		 31.0f, 190.0f,
		 32.0f, 220.0f,
		 31.0f, 221.0f,
		 31.0f, 189.0f,
		255.0f,  18.0f,
		279.0f,  18.0f,
	};

	if ( m_bDialog || !m_bRain )  return;

	if ( m_phase == PHASE_INIT )
	{
		pParti = partiPosInit;
		pGlint = glintPosInit;
	}
	else if ( m_phase == PHASE_NAME    ||
			  m_phase == PHASE_MISSION ||
			  m_phase == PHASE_FREE    ||
			  m_phase == PHASE_USER    ||
			  m_phase == PHASE_PROTO   ||
			  m_phase == PHASE_SETUPd  ||
			  m_phase == PHASE_SETUPg  ||
			  m_phase == PHASE_SETUPp  ||
			  m_phase == PHASE_SETUPc  ||
			  m_phase == PHASE_SETUPs  ||
			  m_phase == PHASE_WRITE   ||
			  m_phase == PHASE_READ    )
	{
		pParti = partiPosBig;
		pGlint = glintPosBig;
	}
	else
	{
		return;
	}

	nParti = (int)(*pParti++);
	nGlint = (int)(*pGlint++);

	for ( i=0 ; i<10 ; i++ )
	{
		if ( m_partiPhase[i] == 0 )  // attente ?
		{
			m_partiTime[i] -= rTime;
			if ( m_partiTime[i] <= 0.0f )
			{
				r = rand()%3;

				if ( r == 0 )
				{
					ii = rand()%nParti;
					m_partiPos[i].x = pParti[ii*5+0]/640.0f;
					m_partiPos[i].y = (480.0f-pParti[ii*5+1])/480.0f;
					m_partiTime[i] = pParti[ii*5+2]+Rand()*pParti[ii*5+3];
					m_partiPhase[i] = (int)pParti[ii*5+4];
					if ( m_partiPhase[i] == 3 )
					{
						m_sound->Play(SOUND_PSHHH, SoundPos(m_partiPos[i]), 0.3f+Rand()*0.3f);
					}
					else
					{
						m_sound->Play(SOUND_GGG, SoundPos(m_partiPos[i]), 0.1f+Rand()*0.4f);
					}
				}

				if ( r == 1 )
				{
					ii = rand()%nGlint;
					pos.x = pGlint[ii*2+0]/640.0f;
					pos.y = (480.0f-pGlint[ii*2+1])/480.0f;
					pos.z = 0.0f;
					speed.x = 0.0f;
					speed.y = 0.0f;
					speed.z = 0.0f;
					dim.x = 0.04f+Rand()*0.04f;
					dim.y = dim.x/0.75f;
					m_particule->CreateParticule(pos, speed, dim,
												 rand()%2?PARTIGLINT:PARTICONTROL,
												 Rand()*0.4f+0.4f, 0.0f,
												 SH_INTERFACE);
					m_partiTime[i] = 0.5f+Rand()*0.5f;
				}

				if ( r == 2 )
				{
					ii = rand()%5;
					if ( ii == 0 )
					{
						m_sound->Play(SOUND_ENERGY, SoundRand(), 0.2f+Rand()*0.2f);
						m_partiTime[i] = 1.0f+Rand()*1.0f;
					}
					if ( ii == 1 )
					{
						m_sound->Play(SOUND_STATION, SoundRand(), 0.2f+Rand()*0.2f);
						m_partiTime[i] = 1.0f+Rand()*2.0f;
					}
					if ( ii == 2 )
					{
						m_sound->Play(SOUND_ALARM, SoundRand(), 0.1f+Rand()*0.1f);
						m_partiTime[i] = 2.0f+Rand()*4.0f;
					}
					if ( ii == 3 )
					{
						m_sound->Play(SOUND_INFO, SoundRand(), 0.1f+Rand()*0.1f);
						m_partiTime[i] = 2.0f+Rand()*4.0f;
					}
					if ( ii == 4 )
					{
						m_sound->Play(SOUND_RADAR, SoundRand(), 0.2f+Rand()*0.2f);
						m_partiTime[i] = 0.5f+Rand()*1.0f;
					}
				}
			}
		}

		if ( m_partiPhase[i] != 0 )  // génère ?
		{
			m_partiTime[i] -= rTime;
			if ( m_partiTime[i] > 0.0f )
			{
				if ( m_partiPhase[i] == 1 )  // étincelles ?
				{
					pos.x = m_partiPos[i].x;
					pos.y = m_partiPos[i].y;
					pos.z = 0.0f;
					pos.x += (Rand()-0.5f)*0.01f;
					pos.y += (Rand()-0.5f)*0.01f;
					speed.x = (Rand()-0.5f)*0.2f;
					speed.y = (Rand()-0.5f)*0.2f;
					speed.z = 0.0f;
					dim.x = 0.005f+Rand()*0.005f;
					dim.y = dim.x/0.75f;
					m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ,
												 Rand()*0.2f+0.2f, 0.0f,
												 SH_INTERFACE);
					pos.x = m_partiPos[i].x;
					pos.y = m_partiPos[i].y;
					pos.z = 0.0f;
					speed.x = (Rand()-0.5f)*0.5f;
					speed.y = (0.3f+Rand()*0.3f);
					speed.z = 0.0f;
					dim.x = 0.01f+Rand()*0.01f;
					dim.y = dim.x/0.75f;
					m_particule->CreateParticule(pos, speed, dim,
												 (ParticuleType)(PARTILENS1+rand()%3),
												 Rand()*0.5f+0.5f, 2.0f,
												 SH_INTERFACE);
				}
				if ( m_partiPhase[i] == 2 )  // étincelles ?
				{
					pos.x = m_partiPos[i].x;
					pos.y = m_partiPos[i].y;
					pos.z = 0.0f;
					pos.x += (Rand()-0.5f)*0.01f;
					pos.y += (Rand()-0.5f)*0.01f;
					speed.x = (Rand()-0.5f)*0.2f;
					speed.y = (Rand()-0.5f)*0.2f;
					speed.z = 0.0f;
					dim.x = 0.005f+Rand()*0.005f;
					dim.y = dim.x/0.75f;
					m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ,
												 Rand()*0.2f+0.2f, 0.0f,
												 SH_INTERFACE);
					pos.x = m_partiPos[i].x;
					pos.y = m_partiPos[i].y;
					pos.z = 0.0f;
					speed.x = (Rand()-0.5f)*0.5f;
					speed.y = (0.3f+Rand()*0.3f);
					speed.z = 0.0f;
					dim.x = 0.005f+Rand()*0.005f;
					dim.y = dim.x/0.75f;
					m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS,
												 Rand()*0.5f+0.5f, 2.0f,
												 SH_INTERFACE);
				}
				if ( m_partiPhase[i] == 3 )  // fumée ?
				{
					pos.x = m_partiPos[i].x;
					pos.y = m_partiPos[i].y;
					pos.z = 0.0f;
					pos.x += (Rand()-0.5f)*0.03f;
					pos.y += (Rand()-0.5f)*0.03f;
					speed.x = (Rand()-0.5f)*0.2f;
					speed.y = Rand()*0.5f;
					speed.z = 0.0f;
					dim.x = 0.03f+Rand()*0.07f;
					dim.y = dim.x/0.75f;
					m_particule->CreateParticule(pos, speed, dim, PARTICRASH,
												 Rand()*0.4f+0.4f, 0.0f,
												 SH_INTERFACE);
				}
			}
			else
			{
				m_partiPhase[i] = 0;
				m_partiTime[i] = 2.0f+Rand()*4.0f;
			}
		}
	}
}

// Quelques jolies particules pour suivre la souris.

void CMainDialog::NiceParticule(FPOINT mouse, BOOL bPress)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;

	if ( !m_bRain )  return;
	if ( (m_phase == PHASE_SIMUL ||
		  m_phase == PHASE_WIN   ||
		  m_phase == PHASE_LOST  ||
		  m_phase == PHASE_MODEL ) &&
		 !m_bDialog             )  return;

#if 0
	if ( TRUE )
	{
		pos.x = mouse.x;
		pos.y = mouse.y;
		pos.z = 0.0f;
		speed.x = (Rand()-0.5f)*0.5f;
		speed.y = (0.3f+Rand()*0.3f);
		speed.z = 0.0f;
		dim.x = 0.005f+Rand()*0.005f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS,
									 Rand()*0.5f+0.5f, 2.0f,
									 SH_INTERFACE);
	}
	else
	{
		pos.x = mouse.x;
		pos.y = mouse.y;
		pos.z = 0.0f;
		speed.x = (Rand()-0.5f)*0.5f;
		speed.y = (0.3f+Rand()*0.3f);
		speed.z = 0.0f;
		dim.x = 0.01f+Rand()*0.01f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 Rand()*0.5f+0.5f, 2.0f,
									 SH_INTERFACE);
	}
#else
	pos.x = mouse.x;
	pos.y = mouse.y;
	pos.z = 0.0f;
	speed.x = (Rand()-0.5f)*0.5f;
	speed.y = (0.3f+Rand()*0.3f);
	speed.z = 0.0f;
	dim.x = 0.01f+Rand()*0.01f;
	dim.y = dim.x/0.75f;
	m_particule->CreateParticule(pos, speed, dim, PARTILENS4,
								 Rand()*0.5f+0.5f, 2.0f,
								 SH_INTERFACE);
#endif
}



// Construit le nom de fichier d'une mission.

void CMainDialog::BuildSceneName(char *filename, char *base, int rank)
{
	if ( strcmp(base, "user") == 0 )
	{
		sprintf(filename, "%s\\%s\\scene%.2d.txt", m_userDir, m_userList[rank/10-1], rank%10);
	}
	else
	{
		sprintf(filename, "%s\\%s%.3d.txt", m_sceneDir, base, rank);
	}
}

// Construit le nom descriptif par défaut d'une mission.

void CMainDialog::BuildResumeName(char *filename, char *base, int rank)
{
	sprintf(filename, "Scene %s %d", base, rank);
}

// Retourne le nom du dossier où mettre les fichiers.

char* CMainDialog::RetFilesDir()
{
	return m_filesDir;
}


// Met à jour la liste des joueurs d'après les dossiers sur disque.

void CMainDialog::ReadNameList()
{
	CWindow*			pw;
	CList*				pl;
	long				hFile;
	struct _finddata_t	fBuffer;
	BOOL				bDo;
	char				dir[_MAX_FNAME];
	char				temp[_MAX_FNAME];
	char				filenames[_MAX_FNAME][100];
	char*				gamer;
	int					nbFilenames, i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
	if ( pl == 0 )  return;
	pl->Flush();
	
	nbFilenames = 0;
	sprintf(dir, "%s\\*", m_savegameDir);
	hFile = _findfirst(dir, &fBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( (fBuffer.attrib & _A_SUBDIR) && fBuffer.name[0] != '.' )
			{
				strcpy(filenames[nbFilenames++], fBuffer.name);
			}
		}
		while ( _findnext(hFile, &fBuffer) == 0 && nbFilenames < 100 );
	}
	_findclose(hFile);
	do  // trie tous les noms :
	{
		bDo = FALSE;
		for ( i=0 ; i<nbFilenames-1 ; i++ )
		{
			if ( strcmp(filenames[i], filenames[i+1]) > 0 )
			{
				strcpy(temp, filenames[i]);
				strcpy(filenames[i], filenames[i+1]);
				strcpy(filenames[i+1], temp);
				bDo = TRUE;
			}
		}
	}
	while ( bDo );

	for ( i=0 ; i<nbFilenames ; i++ )
	{
		pl->SetName(i, filenames[i]);
	}

	gamer = m_main->RetGamerName();
	for ( i=0 ; i<nbFilenames ; i++ )
	{
		if ( stricmp(gamer, pl->RetName(i)) == 0 )
		{
			pl->SetSelect(i);
			pl->ShowSelect();
			return;
		}
	}
	pl->SetSelect(-1);
}

// Met à jour les contrôles des joueurs.

void CMainDialog::UpdateNameControl()
{
	CWindow*	pw;
	CList*		pl;
	CButton*	pb;
	CCheck*		pc;
	int			total, sel;
	BOOL		bOK;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
	if ( pl == 0 )  return;

	total = pl->RetTotal();
	sel   = pl->RetSelect();
	bOK = ( total>0 && sel!=-1 );

//?	pl->SetState(STATE_ENABLE, bOK);
	pl->SetState(STATE_ENABLE, total>0);

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NOK);
	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bOK);
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NDELETE);
	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bOK);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL1);
	if ( pc != 0 )
	{
		pc->SetState(STATE_ENABLE, bOK);
		pc->SetState(STATE_CHECK, m_perso.level==1);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL2);
	if ( pc != 0 )
	{
		pc->SetState(STATE_ENABLE, bOK);
		pc->SetState(STATE_CHECK, m_perso.level==2);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL3);
	if ( pc != 0 )
	{
		pc->SetState(STATE_ENABLE, bOK);
		pc->SetState(STATE_CHECK, m_perso.level==3);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL4);
	if ( pc != 0 )
	{
		pc->SetState(STATE_ENABLE, bOK);
		pc->SetState(STATE_CHECK, m_perso.level==4);
	}
}

// Sélectionne un joueur.

BOOL CMainDialog::NameSelect()
{
	CWindow*	pw;
	CList*		pl;
	int			sel;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return FALSE;
	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
	if ( pl == 0 )  return FALSE;

	sel  = pl->RetSelect();
	if ( sel == -1 )  return FALSE;
	m_main->SetGamerName(pl->RetName(sel));

	ReadGamerInfo();
	SetProfileString("Gamer", "LastName", m_main->RetGamerName());
	return TRUE;
}

// Crée un nouveau joueur.

BOOL CMainDialog::NameCreate()
{
	CWindow*	pw;
	CButton*	pb;
	CList*		pl;
	CCheck*		pc;
	char		name[100];
	char		dir[100];
	char		c;
	int			len, i, j;

	strcpy(name, m_dialogName);
	if ( name[0] == 0 )
	{
		m_sound->Play(SOUND_TZOING);
		return FALSE;
	}

	len = strlen(name);
	j = 0;
	for ( i=0 ; i<len ; i++ )
	{
		c = RetNoAccent(RetToLower(name[i]));
		if ( (c >= '0' && c <= '9') ||
			 (c >= 'a' && c <= 'z') ||
			 c == ' ' ||
			 c == '-' ||
			 c == '_' ||
			 c == '.' ||
			 c == ',' ||
			 c == '\'' )
		{
			name[j++] = name[i];
		}
	}
	name[j] = 0;
	if ( j == 0 )
	{
		m_sound->Play(SOUND_TZOING);
		return FALSE;
	}

	_mkdir(m_savegameDir);  // si n'existe pas encore !

	sprintf(dir, "%s\\%s", m_savegameDir, name);
	if ( _mkdir(dir) != 0 )
	{
		m_sound->Play(SOUND_TZOING);
		return FALSE;
	}

	m_main->SetGamerName(name);
	FlushPerso();
	WriteGamerInfo();
	ReadNameList();
	UpdateNameControl();

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return TRUE;

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NOK);
	if ( pb != 0 )  pb->SetFocus(TRUE);
	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NCREATE);
	if ( pb != 0 )  pb->SetFocus(FALSE);

	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
	if ( pl != 0 )  pl->SetFocus(FALSE);

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL1);
	if ( pc != 0 )  pc->SetFocus(FALSE);
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL2);
	if ( pc != 0 )  pc->SetFocus(FALSE);
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL3);
	if ( pc != 0 )  pc->SetFocus(FALSE);
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL4);
	if ( pc != 0 )  pc->SetFocus(FALSE);

	return TRUE;
}

// Supprime un dossier et toute sa descendance.

BOOL RemoveDir(char *dirname)
{
	long				hFile;
	struct _finddata_t	fBuffer;
	char				filename[100];

	sprintf(filename, "%s\\*", dirname);
	hFile = _findfirst(filename, &fBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( fBuffer.name[0] != '.' )
			{
				if ( fBuffer.attrib & _A_SUBDIR )
				{
					sprintf(filename, "%s\\%s", dirname, fBuffer.name);
					RemoveDir(filename);
				}
				else
				{
					sprintf(filename, "%s\\%s", dirname, fBuffer.name);
					remove(filename);
				}
			}
		}
		while ( _findnext(hFile, &fBuffer) == 0 );
	}
	_findclose(hFile);

	if ( RemoveDirectory(dirname) == 0 )
	{
		return FALSE;
	}
	return TRUE;
}

// Supprime un joueur.

void CMainDialog::NameDelete()
{
	CWindow*	pw;
	CList*		pl;
	CButton*	pb;
	CCheck*		pc;
	int			sel;
	char*		gamer;
	char		dir[100];

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
	if ( pl == 0 )  return;

	sel = pl->RetSelect();
	if ( sel == -1 )
	{
		m_sound->Play(SOUND_TZOING);
		return;
	}
	gamer = pl->RetName(sel);

	// Supprime tout le contenu du dossier.
	sprintf(dir, "%s\\%s", m_savegameDir, gamer);
	if ( !RemoveDir(dir) )
	{
		m_sound->Play(SOUND_TZOING);
		return;
	}

	m_main->SetGamerName("");
	pl->SetSelect(-1);
	ReadNameList();
	UpdateNameControl();

	if ( pl->RetTotal() > 0 )
	{
		pl->SetSelect(0);
		m_main->SetGamerName(pl->RetName(0));
		ReadGamerInfo();
		UpdateNameControl();

		pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NOK);
		if ( pb != 0 )  pb->SetFocus(TRUE);
		pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NCREATE);
		if ( pb != 0 )  pb->SetFocus(FALSE);
	}
	else
	{
		pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NOK);
		if ( pb != 0 )  pb->SetFocus(FALSE);
		pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NCREATE);
		if ( pb != 0 )  pb->SetFocus(TRUE);
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_NDELETE);
	if ( pb != 0 )  pb->SetFocus(FALSE);

	pl->SetFocus(FALSE);

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL1);
	if ( pc != 0 )  pc->SetFocus(FALSE);
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL2);
	if ( pc != 0 )  pc->SetFocus(FALSE);
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL3);
	if ( pc != 0 )  pc->SetFocus(FALSE);
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LEVEL4);
	if ( pc != 0 )  pc->SetFocus(FALSE);
}

// Choix d'un autre niveau de difficulté.

void CMainDialog::LevelSelect(int level)
{
	CWindow*	pw;
	CList*		pl;
	int			sel;

	m_perso.level = level;
	UpdateNameControl();

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
	if ( pl == 0 )  return;
	sel = pl->RetSelect();
	if ( sel == -1 )  return;

	WriteGamerInfo();
}


// Teste si deux couleurs sont égales ou presque.

BOOL EqColor(const D3DCOLORVALUE &c1, const D3DCOLORVALUE &c2)
{
	return (Abs(c1.r-c2.r) < 0.01f &&
			Abs(c1.g-c2.g) < 0.01f &&
			Abs(c1.b-c2.b) < 0.01f );
}

// Met à jour tous les boutons pour le personnage.

void CMainDialog::UpdatePerso()
{
	CObject*		vehicle;
	CPhysics*		physics;
	CMotion*		motion;
	Character*		character;
	CWindow*		pw;
	CButton*		pb;
	CColor*			pco;
	CGauge*			pg;
	CGroup*			pgr;
	CLabel*			pl;
	D3DCOLORVALUE	color;
	CarSpec			missionSpec, carSpec;
	char			text[100];
	char			res[100];
	char			car[50];
	int				i, icon = 0;
	float			value;
	BOOL			bOK;

	vehicle = m_main->SearchObject(OBJECT_CAR);
	if ( vehicle == 0 )  return;
	physics = vehicle->RetPhysics();
	if ( physics == 0 )  return;
	motion = vehicle->RetMotion();
	if ( motion == 0 )  return;
	character = vehicle->RetCharacter();

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	pg = (CGauge*)pw->SearchControl(EVENT_INTERFACE_PGSPEED);
	if ( pg == 0 )  return;
	value = physics->RetLinMotionX(MO_ADVSPEED);
	pg->SetLevel(value/180.0f);
	pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PLSPEED);
	if ( pl == 0 )  return;
#if _MPH
	value *= 0.6f;  // approximatif: 100 KPH = 60 MPH !
#endif
	sprintf(text, "%d", (int)value);
	pl->SetName(text);

	pg = (CGauge*)pw->SearchControl(EVENT_INTERFACE_PGACCEL);
	if ( pg == 0 )  return;
	value = physics->RetLinMotionX(MO_ADVACCEL);
	pg->SetLevel(value/100.0f);
	pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PLACCEL);
	if ( pl == 0 )  return;
	sprintf(text, "%d", (int)value);
	pl->SetName(text);

	pg = (CGauge*)pw->SearchControl(EVENT_INTERFACE_PGGRIP);
	if ( pg == 0 )  return;
	value = 100.0f-character->gripSlide;
	pg->SetLevel(value/50.0f);
	pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PLGRIP);
	if ( pl == 0 )  return;
	sprintf(text, "%d", (int)value);
	pl->SetName(text);

	pg = (CGauge*)pw->SearchControl(EVENT_INTERFACE_PGSOLID);
	if ( pg == 0 )  return;
	value = (float)motion->RetUsedPart();
	pg->SetLevel(value/30.0f);
	pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PLSOLID);
	if ( pl == 0 )  return;
	sprintf(text, "%d", (int)value);
	pl->SetName(text);

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PPREV);
	if ( pb == 0 )  return;
	pb->SetState(STATE_ENABLE, m_perso.selectCar > 0);

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PNEXT);
	if ( pb == 0 )  return;
#if !_SE
	pb->SetState(STATE_ENABLE, m_perso.selectCar < m_perso.buyable-1 ||
							   m_perso.selectCar < m_perso.bonus-1   );
#endif

	for ( i=0 ; i<15 ; i++ )
	{
		pco = (CColor*)pw->SearchControl((EventMsg)(EVENT_INTERFACE_PCOLOR0+i));
		if ( pco == 0 )  break;
		pco->SetState(STATE_VISIBLE);
		color.r = perso_color[3*i+0]/255.0f;
		color.g = perso_color[3*i+1]/255.0f;
		color.b = perso_color[3*i+2]/255.0f;
		color.a = 0.0f;
		pco->SetColor(color);
		pco->SetState(STATE_CHECK, EqColor(color, m_perso.colorBody[m_perso.selectCar]));
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD0);
	if ( pb == 0 )  return;
	pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==0);

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD1);
	if ( pb == 0 )  return;
	pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==1);

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD2);
	if ( pb == 0 )  return;
	pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==2);

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PSUBMOD3);
	if ( pb == 0 )  return;
	pb->SetState(STATE_CHECK, m_perso.subModel[m_perso.selectCar]==3);

#if _SE
	if ( m_perso.stateCars[m_perso.selectCar] != SC_FORSALE )
#else
	if ( m_perso.selectCar < m_perso.total ||
		 m_perso.selectCar < m_perso.bonus )  // sélectionne ?
#endif
	{
		pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PPRICE);
		if ( pl == 0 )  return;
		pl->SetFontType(FONT_COLOBOT);
		pl->SetState(STATE_VISIBLE);
		NamePerso(car, m_perso.usedCars[m_perso.selectCar]);
		pl->SetName(car);  // nom du véhicule

		pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PPESETAS);
		if ( pl == 0 )  return;
		pl->ClearState(STATE_VISIBLE);

		pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_POK);
		if ( pb == 0 )  return;
		GetResource(RES_EVENT, EVENT_INTERFACE_POK, res);
		pb->SetName(res);
		pb->ClearState(STATE_CHECK);
	}
	else	// achète ?
	{
		pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PPRICE);
		if ( pl == 0 )  return;
		pl->SetFontType(FONT_HILITE);
		pl->SetState(STATE_VISIBLE);
		GetResource(RES_EVENT, EVENT_INTERFACE_PPRICE, res);
		NamePerso(car, m_perso.usedCars[m_perso.selectCar]);
		sprintf(text, res, car, PricePerso(m_perso.usedCars[m_perso.selectCar]));
		pl->SetName(text);  // prix du véhicule

		pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PPESETAS);
		if ( pl == 0 )  return;
		pl->SetState(STATE_VISIBLE);
		GetResource(RES_EVENT, EVENT_INTERFACE_PPESETAS, res);
		sprintf(text, res, m_perso.pesetas);
		pl->SetName(text);  // somme disponible

		pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_POK);
		if ( pb == 0 )  return;
		GetResource(RES_TEXT, RT_PERSO_BUY, res);
		pb->SetName(res);
		pb->SetState(STATE_CHECK);
	}

	// Teste si la voiture choisie a des spécifications compatibles
	// avec la mission à effectuer.
	bOK = TRUE;
	res[0] = 0;
	ReadCarSpec(m_sel[m_index], missionSpec);
	SpecPerso(carSpec, m_perso.usedCars[m_perso.selectCar]);
//?	if ( m_perso.level < 3 && carSpec.minSpeed != NAN )
#if _DEMO
	if ( m_perso.selectCar != 2 )  // pas tijuana taxi ?
	{
		bOK = FALSE;
		GetResource(RES_TEXT, RT_SPEC_DEMO, res);
	}
#else
#if _SE
	icon = 29;  // rouge
	if ( m_perso.stateCars[m_perso.selectCar] == SC_SHOWCASE )  // en vitrine ?
	{
		if ( m_perso.usedCars[m_perso.selectCar] == 4 ||  // firecraker ?
			 m_perso.usedCars[m_perso.selectCar] == 5 ||  // hooligan ?
			 m_perso.usedCars[m_perso.selectCar] == 1 )   // tijuana taxi ?
		{
			GetResource(RES_TEXT, RT_SPEC_DISPO, res);
			icon = 30;  // bleu
		}
		else
		{
			GetResource(RES_TEXT, RT_SPEC_DEMO, res);
		}
		bOK = FALSE;
	}
#else
	if ( carSpec.minSpeed != NAN )
	{
		if ( missionSpec.minSpeed != NAN &&
			 carSpec.minSpeed < missionSpec.minSpeed )
		{
			bOK = FALSE;
			GetResource(RES_TEXT, RT_SPEC_MINSPEED, res);
		}
		if ( missionSpec.maxSpeed != NAN &&
			 carSpec.minSpeed > missionSpec.maxSpeed )
		{
			bOK = FALSE;
			GetResource(RES_TEXT, RT_SPEC_MAXSPEED, res);
		}
	}
#endif
#endif

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_POK);
	if ( pb == 0 )  return;
	pb->SetState(STATE_ENABLE, bOK);

	pgr = (CGroup*)pw->SearchControl(EVENT_INTERFACE_PSPECB);
	if ( pgr == 0 )  return;
	pgr->SetState(STATE_VISIBLE, !bOK);
#if _SE
	pgr->SetIcon(icon);
#endif
	pl = (CLabel*)pw->SearchControl(EVENT_INTERFACE_PSPECT);
	if ( pl == 0 )  return;
	pl->SetState(STATE_VISIBLE, !bOK);
	pl->SetName(res);
}

// Met à jour l'élévation de la caméra selon le slider correspondant.

void CMainDialog::ElevationPerso()
{
	CWindow*	pw;
	CSlider*	ps;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PELEV);
	if ( ps == 0 )  return;

	m_persoElevation = ps->RetVisibleValue();
}

// Met à jour la caméra pour le personnage.

void CMainDialog::CameraPerso()
{
	D3DVECTOR	eye, look;

	eye = RotateView(D3DVECTOR(0.0f, 0.0f, 0.0f), m_persoAngle, m_persoElevation, 20.0f);
//?	look.y -= powf(eye.y, 5.0f)/10000.0f;  // empirique !
	look.y -= powf(eye.y, 7.0f)/4000000.0f;  // empirique !
	look.x = 0.0f;
	look.z = 0.0f;
	eye.y  += 2.0f+(0.7f+m_persoElevation)*2.0f;
	look.y += 2.0f+(0.7f+m_persoElevation)*2.0f;
	m_camera->Init(eye, look, 0.0f);

	m_camera->SetType(CAMERA_SCRIPT);
	m_camera->FixCamera();
}

// Gestion de l'argent du joueur.

void CMainDialog::SetPesetas(int value)
{
	if ( value > 99999 )  value = 99999;
	m_perso.pesetas = value;
	WriteGamerInfo();
}

int CMainDialog::RetPesetas()
{
	return m_perso.pesetas;
}

// Indique s'il faut utiliser une voiture fantome.

BOOL CMainDialog::RetGhost()
{
	return (m_bGhostExist && m_bGhostEnable);
}

BOOL CMainDialog::RetGhostExist()
{
	return m_bGhostExist;
}

// Indique le niveau de difficulté.

int CMainDialog::RetLevel()
{
	if ( m_bDuel )
	{
		return m_duelLevel;
	}
	else
	{
		return m_perso.level;
	}
}

// Indique s'il est possible d'acheter une ou plusieurs voitures.

BOOL CMainDialog::IsBuyablePerso()
{
	int		i, model;

	if ( !m_bPesetas )  return FALSE;  // course simple ?

	for ( i=0 ; i<50 ; i++ )
	{
		model = RetPersoModel(i);
		if ( model != 0 &&
			 !UsedPerso(model) &&
			 PricePerso(model) <= m_perso.pesetas )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// Met à jour toutes les voitures que le joueur peut acheter.

void CMainDialog::BuyablePerso()
{
	int		i, index, model, max;

	m_perso.bonus = m_perso.total;

	if ( !m_bPesetas )  // course simple ?
	{
		m_perso.buyable = m_perso.total;

		if ( !UsedPerso(8) )  // torpedo ?
		{
			index = m_perso.total;
			m_perso.usedCars[index] = 8;
			m_perso.subModel[index] = 0;
			DefPerso(index);  // met la couleur standard
			m_perso.bonus = index+1;
		}
		return;
	}

#if _SE
	i = 0;
	m_perso.buyable = m_perso.total;
	for ( index=0 ; index<m_perso.total ; index++ )
	{
		model = m_perso.usedCars[index];
		if ( model != 0 &&
			 (model == 4 ||    // firecraker ?
			  model == 5 ||    // hooligan ?
			  model == 1 ) &&  // tijuana taxi ?
			 !UsedPerso(model) &&
			 PricePerso(model) <= m_perso.pesetas )
		{
			m_perso.subModel[index] = 0;
			DefPerso(index);  // met la couleur standard
			m_perso.stateCars[index] = SC_FORSALE;  // voiture à vendre
			m_perso.buyable = index;
			m_perso.selectCar = index;
		}
	}
#else
	index = m_perso.total;
	for ( i=0 ; i<50 ; i++ )
	{
		model = RetPersoModel(i);
		if ( model != 0 &&
			 !UsedPerso(model) &&
			 PricePerso(model) <= m_perso.pesetas )
		{
			m_perso.usedCars[index] = model;
			m_perso.subModel[index] = 0;
			DefPerso(index);  // met la couleur standard
			index ++;
		}
	}
	m_perso.buyable = index;
	
	if ( index > m_perso.total )
	{
		m_perso.selectCar = index-1;  // sélectionne la voiture à acheter
	}

	max = m_perso.total;
	if ( max < m_perso.bonus   )  max = m_perso.bonus;
	if ( max < m_perso.buyable )  max = m_perso.buyable;
	if ( m_perso.selectCar >= max && max > 0 )
	{
		m_perso.selectCar = max-1;
	}
#endif
}

// Sélectionne la voiture suivante ou précédente.

void CMainDialog::NextPerso(int dir)
{
	int		i;

	if ( dir > 0 )  // voiture suivante ?
	{
		i = m_perso.selectCar+1;
#if _SE
		if ( i >= m_perso.total )  i = 0;
#else
		if ( i >= m_perso.buyable &&
			 i >= m_perso.bonus   )  i = 0;
#endif
		m_perso.selectCar = i;
	}
	else	// voiture précédente ?
	{
		i = m_perso.selectCar-1;
#if _SE
		if ( i < 0 )  i = m_perso.total-1;
#else
		if ( i < 0 )
		{
			if ( m_perso.buyable > m_perso.bonus )	i = m_perso.buyable-1;
			else									i = m_perso.bonus-1;
		}
#endif
		m_perso.selectCar = i;
	}
}

// Sélectionne ou achète la voiture courante.

void CMainDialog::SelectPerso()
{
#if _SE
	if ( m_perso.stateCars[m_perso.selectCar] != SC_FORSALE )
#else
	if ( m_perso.selectCar < m_perso.total ||
		 m_perso.selectCar < m_perso.bonus )  // sélectionne ?
#endif
	{
		WriteGamerInfo();
	}
	else	// achète ?
	{
		BuyPerso();
	}
}

// Achète une voiture.

void CMainDialog::BuyPerso()
{
	int		model, sel;

	if ( m_perso.total >= 49 )  return;

	sel = m_perso.selectCar;
	model = m_perso.usedCars[m_perso.selectCar];

#if _SE
	m_perso.stateCars[m_perso.selectCar] = SC_OWNER;
#else
	m_perso.selectCar = m_perso.total;
	m_perso.total ++;
#endif

	m_perso.usedCars[m_perso.selectCar] = model;
	m_perso.subModel[m_perso.selectCar] = m_perso.subModel[sel];
	m_perso.colorBody[m_perso.selectCar] = m_perso.colorBody[sel];

	m_perso.pesetas -= PricePerso(model);  // débite le compte du joueur

	WriteGamerInfo();
}

// Achète toutes les voitures.

void CMainDialog::BuyAllPerso()
{
	int		i, rank;

	m_perso.total = 0;

	for ( i=0 ; i<50 ; i++ )
	{
		rank = RetPersoModel(i);
		if ( rank == 0 )  break;

		m_perso.usedCars[i] = rank;
		m_perso.subModel[i] = 0;
		m_perso.stateCars[i] = i==0?SC_OWNER:SC_SHOWCASE;
		DefPerso(i);
		m_perso.total ++;
	}

	m_perso.buyable = m_perso.total;
	m_perso.bonus   = m_perso.total;
	m_perso.selectCar = 0;

#if !_SE
	WriteGamerInfo();
#endif
}

// Retourne le modèle d'une voiture dans l'ordre des performances.

int CMainDialog::RetPersoModel(int rank)
{
	if ( rank == 0 )  return 4;  // firecraker
	if ( rank == 1 )  return 5;  // hooligan
	if ( rank == 2 )  return 1;  // tijuana taxi
	if ( rank == 3 )  return 3;  // pickup
	if ( rank == 4 )  return 7;  // reo
	if ( rank == 5 )  return 2;  // ford 32
	if ( rank == 6 )  return 6;  // chevy
	if ( rank == 7 )  return 8;  // torpedo
	return 0;  // terminateur
}

// Retourne l'index d'un modèle de voiture.

int CMainDialog::IndexPerso(int model)
{
	int		i, max;

	if ( m_perso.total > m_perso.bonus )  max = m_perso.total;
	else                                  max = m_perso.bonus;

	for ( i=0 ; i<max ; i++ )
	{
		if ( model == m_perso.usedCars[i] )  return i;
	}
	return -1;
}

// Indique si un modèle de voiture est acheté par le joueur.

BOOL CMainDialog::UsedPerso(int model)
{
	int		i, max;

	if ( m_perso.total > m_perso.bonus )  max = m_perso.total;
	else                                  max = m_perso.bonus;

	for ( i=0 ; i<max ; i++ )
	{
#if _SE
		if ( model == m_perso.usedCars[i] &&
			 m_perso.stateCars[i] == SC_OWNER )  return TRUE;
#else
		if ( model == m_perso.usedCars[i] )  return TRUE;
#endif
	}
	return FALSE;
}

// Retourne le prix d'un modèle de voiture.

int CMainDialog::PricePerso(int model)
{
	if ( model == 4 )  return     2;  // firecraker
	if ( model == 5 )  return    10;  // hooligan
	if ( model == 1 )  return    40;  // tijuana taxi
	if ( model == 3 )  return   150;  // pickup
	if ( model == 7 )  return   600;  // reo
	if ( model == 2 )  return  2500;  // ford 32
	if ( model == 6 )  return 10000;  // chevy
	if ( model == 8 )  return 12000;  // torpedo
	return 0;
}

// Retourne le nom d'un modèle de voiture.

void CMainDialog::NamePerso(char *buffer, int model)
{
	strcpy(buffer, "?");
	if ( model == 4 )  strcpy(buffer, "Basic Buzzing Buggy");  // firecraker
	if ( model == 5 )  strcpy(buffer, "Hooly Wheely");         // hooligan
	if ( model == 1 )  strcpy(buffer, "Mellow Cab");           // tijuana taxi
	if ( model == 3 )  strcpy(buffer, "Daddy's Pickup");       // pickup
	if ( model == 7 )  strcpy(buffer, "Don Carleone");         // reo
	if ( model == 2 )  strcpy(buffer, "Drag Star");            // ford 32
	if ( model == 6 )  strcpy(buffer, "Big Buzzing Bee");      // chevy
	if ( model == 8 )  strcpy(buffer, "Speedy Torpedo");       // torpedo
}

// Retourne les spécifications d'un modèle de voiture.

void CMainDialog::SpecPerso(CarSpec &spec, int model)
{
	spec.minSpeed = NAN;
	spec.maxSpeed = NAN;

	if ( model == 4 )  // firecraker
	{
		spec.minSpeed = 120.0f;
		spec.maxSpeed = 120.0f;
	}
	if ( model == 5 )  // hooligan
	{
		spec.minSpeed = 120.0f;
		spec.maxSpeed = 120.0f;
	}
	if ( model == 1 )  // tijuana taxi
	{
		spec.minSpeed = 120.0f;
		spec.maxSpeed = 120.0f;
	}
	if ( model == 3 )  // pickup
	{
		spec.minSpeed = 130.0f;
		spec.maxSpeed = 130.0f;
	}
	if ( model == 7 )  // reo
	{
		spec.minSpeed = 140.0f;
		spec.maxSpeed = 140.0f;
	}
	if ( model == 2 )  // ford 32
	{
		spec.minSpeed = 150.0f;
		spec.maxSpeed = 150.0f;
	}
	if ( model == 6 )  // chevy
	{
		spec.minSpeed = 160.0f;
		spec.maxSpeed = 160.0f;
	}
	if ( model == 8 )  // torpedo
	{
		spec.minSpeed = 170.0f;
		spec.maxSpeed = 170.0f;
	}
}

// Met une couleur fixe.

void CMainDialog::FixPerso(int rank)
{
	m_perso.colorBody[m_perso.selectCar].r = perso_color[rank*3+0]/255.0f;
	m_perso.colorBody[m_perso.selectCar].g = perso_color[rank*3+1]/255.0f;
	m_perso.colorBody[m_perso.selectCar].b = perso_color[rank*3+2]/255.0f;
}

// Initialise le personnage de base lors de la création d'un joueur.

void CMainDialog::FlushPerso()
{
	ZeroMemory(&m_perso, sizeof(GamerPerso));
	m_perso.usedCars[0] = 4;  // firecraker
	m_perso.subModel[0] = 0;  // peinture de base
	m_perso.total = 1;
	m_perso.bonus = 1;
	m_perso.selectCar = 0;
	m_perso.pesetas = 0;
	m_perso.level = 3;  // niveau intermédiaire (costaud)
	DefPerso(0);  // met la couleur standard
}

// Met la couleur standard à une voiture.

void CMainDialog::DefPerso(int rank)
{
	int		model, color;

	model = m_perso.usedCars[rank];
	color = 0;  // couleur standard selon model
	if ( m_perso.subModel[rank] == 0 )
	{
		if ( model == 1 )  color =  0;  // jaune  - tijuana taxi
		if ( model == 2 )  color =  4;  // violet - ford 32
		if ( model == 3 )  color =  1;  // bleu   - pickup
		if ( model == 4 )  color =  2;  // rouge  - firecraker
		if ( model == 5 )  color =  3;  // noir   - hooligan
		if ( model == 6 )  color =  7;  // rose   - chevy
		if ( model == 7 )  color = 12;  // violet - reo
		if ( model == 8 )  color =  2;  // rouge  - torpedo
	}
	if ( m_perso.subModel[rank] == 1 )
	{
		if ( model == 1 )  color = 13;  // cyan   - tijuana taxi
		if ( model == 2 )  color = 13;  // cyan   - ford 32
		if ( model == 3 )  color =  6;  // brun   - pickup
		if ( model == 4 )  color = 12;  // violet - firecraker
		if ( model == 5 )  color =  4;  // violet - hooligan
		if ( model == 6 )  color = 11;  // bleu   - chevy
		if ( model == 7 )  color =  6;  // brun   - reo
		if ( model == 8 )  color = 12;  // violet - torpedo
	}
	if ( m_perso.subModel[rank] == 2 )
	{
		if ( model == 1 )  color = 12;  // violet - tijuana taxi
		if ( model == 2 )  color = 12;  // violet - ford 32
		if ( model == 3 )  color = 12;  // violet - pickup
		if ( model == 4 )  color =  4;  // violet - firecraker
		if ( model == 5 )  color = 12;  // violet - hooligan
		if ( model == 6 )  color =  4;  // violet - chevy
		if ( model == 7 )  color =  7;  // rose   - reo
		if ( model == 8 )  color =  6;  // brun   - torpedo
	}
	if ( m_perso.subModel[rank] == 3 )
	{
		if ( model == 1 )  color =  1;  // bleu   - tijuana taxi
		if ( model == 2 )  color =  0;  // jaune  - ford 32
		if ( model == 3 )  color =  3;  // noir   - pickup
		if ( model == 4 )  color =  5;  // blanc  - firecraker
		if ( model == 5 )  color =  1;  // bleu   - hooligan
		if ( model == 6 )  color = 12;  // violet - chevy
		if ( model == 7 )  color =  3;  // noir   - reo
		if ( model == 8 )  color = 10;  // jaune  - torpedo
	}

	m_perso.colorBody[rank].r = perso_color[color*3+0]/255.0f;
	m_perso.colorBody[rank].g = perso_color[color*3+1]/255.0f;
	m_perso.colorBody[rank].b = perso_color[color*3+2]/255.0f;
	m_perso.colorBody[rank].a = 0.0f;
}

// Retourne le modèle de la voiture choisie.

int CMainDialog::RetModel()
{
	if ( m_bDuel )
	{
		return m_duelModel;
	}
	else
	{
		return m_perso.usedCars[m_perso.selectCar];
	}
}

int CMainDialog::RetSubModel()
{
	if ( m_bDuel )
	{
		return m_duelSubModel;
	}
	else
	{
		return m_perso.subModel[m_perso.selectCar];
	}
}


// Met à jour les listes selon le cheat code.

void CMainDialog::AllMissionUpdate()
{
	if ( m_phase == PHASE_MISSION ||
		 m_phase == PHASE_FREE    ||
		 m_phase == PHASE_USER    ||
		 m_phase == PHASE_PROTO   )
	{
		UpdateSceneList(m_sel[m_index]);
	}
}

// Met à jour la liste des missions.

void CMainDialog::UpdateSceneList(int &sel)
{
	FILE*		file = NULL;
	CWindow*	pw;
	CList*		pl;
	char		filename[_MAX_FNAME];
	char		op[100];
	char		line[500];
	char		name[100];
	int			i, j, jj, k, rank, rsel, look;
	BOOL		bPassed;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_INTERFACE_LIST);
	if ( pl == 0 )  return;

	ZeroMemory(&m_listInfo, sizeof(ListInfo)*MAXSCENE);

	pl->Flush();
	rank = 0;
	rsel = 0;
	look = 1;
	for ( k=0 ; k<99 ; k++ )
	{
#if _DEMO
		if ( k >= 1 )  break;
#endif
#if _SE
		if ( k >= 3 )  break;
#endif
		BuildSceneName(filename, m_sceneName, (k+1)*10);
		file = fopen(filename, "r");
		if ( file == NULL )  break;
		fclose(file);

		jj = 9;
		if ( m_phase == PHASE_FREE )  jj = 19;
		for ( j=0 ; j<jj ; j++ )
		{
#if _DEMO
			if ( j >= 1 )  break;
#endif
#if _SE
			if ( j >= 3 )  break;
#endif
			BuildSceneName(filename, m_sceneName, (k+1)*10+(j+1));
			file = fopen(filename, "r");
			if ( file == NULL )  break;

			BuildResumeName(name, m_sceneName, j+1);  // nom par défaut
			while ( fgets(line, 500, file) != NULL )
			{
				for ( i=0 ; i<500 ; i++ )
				{
					if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
					if ( line[i] == '/' && line[i+1] == '/' )
					{
						line[i] = 0;
						break;
					}
				}

				sprintf(op, "Title.%c", RetLanguageLetter());
				if ( Cmd(line, op) )
				{
					OpString(line, "text", name);
					break;
				}
			}
			fclose(file);

			bPassed = RetGamerInfoPassed((k+1)*10+(j+1));
			if ( m_bPesetas )
			{
				sprintf(line, "%d/%d: %s", k+1, j+1, name);
			}
			else
			{
				sprintf(line, "%d: %s", j+1, name);
			}
			pl->SetName(rank, line);
			pl->SetCheck(rank, bPassed);
			pl->SetEnable(rank, TRUE);
			pl->SetLook(rank, look);
			m_listInfo[rank].chap = k+1;
			m_listInfo[rank].scene = j+1;
			if ( sel == (k+1)*10+(j+1) )  rsel = rank;
			rank ++;
		}

		bPassed = RetGamerInfoPassed((k+1)*10);
		if ( !m_main->RetShowAll() && !bPassed )
		{
			j ++;
			break;
		}

		look ++;
		if ( look > 2 )  look = 1;
	}

	pl->SetSelect(rsel);
	pl->ShowSelect();  // montre la ligne sélectionnée
}

// Retourne s'il peut exister une voiture fantome.

BOOL CMainDialog::ReadGhostMode(int rank)
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		line[500];
	int			i, mode;

	mode = FALSE;

	BuildSceneName(filename, m_sceneBase, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return mode;

	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		if ( Cmd(line, "Ghost") )
		{
			mode = OpInt(line, "mode", 0);
			break;
		}
	}
	fclose(file);
	return mode;
}

// Retourne les spécifications de la voiture requise.

void CMainDialog::ReadCarSpec(int rank, CarSpec &spec)
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		line[500];
	int			i;

	spec.minSpeed = NAN;
	spec.maxSpeed = NAN;

	BuildSceneName(filename, m_sceneBase, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return;

	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		if ( Cmd(line, "CarSpec") )
		{
			spec.minSpeed = OpFloat(line, "minSpeed", NAN);
			spec.maxSpeed = OpFloat(line, "maxSpeed", NAN);
			break;
		}
	}
	fclose(file);
}

// Retourne le nb de pesetas limite à accumuler pour passer plus loin.

int CMainDialog::ReadPesetasNext()
{
#if _DEMO
	return ReadPesetasLimit(10);
#else
	int		i;

	for ( i=0 ; i<100 ; i++ )
	{
		if ( !m_sceneInfo[i*10].bPassed )
		{
			return ReadPesetasLimit(i*10);
		}
	}
	return 0;
#endif
}

// Retourne le nb de pesetas limite à accumuler pour passer plus loin.

int CMainDialog::ReadPesetasLimit(int rank)
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		line[500];
	int			i, limit;

	BuildSceneName(filename, m_sceneName, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return 0;

	limit = 0;
	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		if ( Cmd(line, "Pesetas") )
		{
			limit = OpInt(line, "limit", 0);
			break;
		}
	}
	fclose(file);

	return limit;
}

// Met à jour le gain du joueur.

void CMainDialog::UpdateScenePesetasPerso()
{
	CWindow*	pw;
	CPesetas*	pp;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pp = (CPesetas*)pw->SearchControl(EVENT_INTERFACE_PESETAS);
	if ( pp == 0 )  return;

	pp->SetLevelCredit((float)m_perso.pesetas);
}

// Met à jour le gain max possible d'une mission.

void CMainDialog::UpdateScenePesetasMax(int rank)
{
	CWindow*	pw;
	CPesetas*	pp;
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		line[500];
	int			i, max;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pp = (CPesetas*)pw->SearchControl(EVENT_INTERFACE_PESETAS);
	if ( pp == 0 )  return;

	BuildSceneName(filename, m_sceneName, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return;

	max = 0;
	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		if ( Cmd(line, "Pesetas") )
		{
			max = OpInt(line, "max", 0);
			break;
		}
	}
	fclose(file);

	pp->SetLevelMission((float)max);
}

// Met à jour un résumé d'exercice ou de mission.

void CMainDialog::UpdateSceneResume(int rank)
{
	CWindow*	pw;
	CEdit*		pe;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_RESUME);
	if ( pe == 0 )  return;

	UpdateSceneResume(rank, pe);
}

// Met à jour un résumé d'exercice ou de mission.

void CMainDialog::UpdateSceneResume(int rank, CEdit *pe)
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		op[100];
	char		line[500];
	char		name[500];
	int			i;

	BuildSceneName(filename, m_sceneName, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return;

	name[0] = 0;
	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		sprintf(op, "Resume.%c", RetLanguageLetter());
		if ( Cmd(line, op) )
		{
			OpString(line, "text", name);
			break;
		}
	}
	fclose(file);

	pe->SetText(name);
}

// Met à jour un résumé d'exercice ou de mission.

BOOL CMainDialog::ReadSceneTitle(char *scene, int rank, char *buffer)
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		op[100];
	char		line[500];
	int			i;

	BuildSceneName(filename, scene, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return FALSE;

	if ( strcmp(scene, "scene") == 0 )
	{
		sprintf(buffer, "%d/%d: ", rank/10, rank%10);
	}
	else if ( strcmp(scene, "free") == 0 )
	{
		sprintf(buffer, "%d: ", rank-10);
	}
	else
	{
		buffer[0] = 0;
	}

	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		sprintf(op, "Title.%c", RetLanguageLetter());
		if ( Cmd(line, op) )
		{
			OpString(line, "text", buffer+strlen(buffer));
			break;
		}
	}
	fclose(file);
	return TRUE;
}

// Met à jour une image d'exercice ou de mission.

void CMainDialog::UpdateSceneImage(int rank)
{
	CWindow*	pw;
	CImage*		pi;
	char		filename[_MAX_FNAME];

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pi = (CImage*)pw->SearchControl(EVENT_INTERFACE_IMAGE);
	if ( pi == 0 )  return;

	sprintf(filename, "diagram\\%s%.3d.bmp", m_sceneName, rank);
	pi->SetFilenameImage(filename);
}

// Met à jour le bouton play pour un exercice ou une mission.

void CMainDialog::UpdateScenePlay(int rank)
{
	CWindow*	pw;
	CButton*	pb;
	CarSpec		missionSpec, carSpec;
	int			i, max;

	if ( m_bPesetas )  return;  // pas course simple ?

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PLAY);
	if ( pb == 0 )  return;

	ReadCarSpec(rank, missionSpec);
	if ( missionSpec.minSpeed == NAN )
	{
		pb->SetState(STATE_ENABLE, TRUE);
		return;
	}

	pb->SetState(STATE_ENABLE, FALSE);
	if ( m_perso.total > m_perso.bonus )  max = m_perso.total;
	else                                  max = m_perso.bonus;
	for ( i=0 ; i<max ; i++ )
	{
		SpecPerso(carSpec, m_perso.usedCars[i]);
		if ( carSpec.minSpeed != NAN )
		{
			if ( carSpec.minSpeed >= missionSpec.minSpeed )
			{
				pb->SetState(STATE_ENABLE, TRUE);
				return;
			}
		}
	}
}

// Met à jour le bouton "voiture fantome".

void CMainDialog::UpdateSceneGhost(int rank)
{
	CWindow*	pw;
	CCheck*		pc;
	CGroup*		pg;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_GHOSTm);
	if ( pc == 0 )  return;
	pg = (CGroup*)pw->SearchControl(EVENT_INTERFACE_GHOSTg);
	if ( pg == 0 )  return;

	m_bGhostExist = ReadGhostMode(rank);

	if ( m_bGhostExist )
	{
		if ( m_main->RecorderExist(rank, m_perso.usedCars[m_perso.selectCar]) )
		{
			pg->SetState(STATE_VISIBLE, TRUE);
			pc->SetState(STATE_VISIBLE, TRUE);
			pc->SetState(STATE_ENABLE,  TRUE);
			pc->SetState(STATE_CHECK,   m_bGhostEnable);
		}
		else
		{
			pg->SetState(STATE_VISIBLE, TRUE);
			pc->SetState(STATE_VISIBLE, TRUE);
			pc->SetState(STATE_ENABLE,  FALSE);
			pc->SetState(STATE_CHECK,   FALSE);
		}
	}
	else
	{
		pg->SetState(STATE_VISIBLE, FALSE);
		pc->SetState(STATE_VISIBLE, FALSE);
		pc->SetState(STATE_CHECK,   FALSE);
	}
}

// Met à jour la liste des devices.

void CMainDialog::UpdateDisplayDevice()
{
	CWindow*	pw;
	CList*		pl;
	char		bufDevices[1000];
	char		bufModes[5000];
	int			i, j, totalDevices, selectDevices, totalModes, selectModes;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_LIST1);
	if ( pl == 0 )  return;
	pl->Flush();

	m_engine->EnumDevices(bufDevices, 1000,
						  bufModes,   5000,
						  totalDevices, selectDevices,
						  totalModes,   selectModes);

	i = 0;
	j = 0;
	while ( bufDevices[i] != 0 )
	{
		pl->SetName(j++, bufDevices+i);
		while ( bufDevices[i++] != 0 );
	}

	pl->SetSelect(selectDevices);
	pl->ShowSelect();

	m_setupSelDevice = selectDevices;
}

// Met à jour la liste des modes.

void CMainDialog::UpdateDisplayMode()
{
	CWindow*	pw;
	CList*		pl;
	char		bufDevices[1000];
	char		bufModes[5000];
	int			i, j, totalDevices, selectDevices, totalModes, selectModes;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pl = (CList*)pw->SearchControl(EVENT_LIST2);
	if ( pl == 0 )  return;
	pl->Flush();

	m_engine->EnumDevices(bufDevices, 1000,
						  bufModes,   5000,
						  totalDevices, selectDevices,
						  totalModes,   selectModes);

	i = 0;
	j = 0;
	while ( bufModes[i] != 0 )
	{
		pl->SetName(j++, bufModes+i);
		while ( bufModes[i++] != 0 );
	}

	pl->SetSelect(selectModes);
	pl->ShowSelect();

	m_setupSelMode = selectModes;
}

// Change le mode graphique.

void CMainDialog::ChangeDisplay()
{
	CWindow*	pw;
	CList*		pl;
	CCheck*		pc;
	char*		device;
	char*		mode;
	BOOL		bFull;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	pl = (CList*)pw->SearchControl(EVENT_LIST1);
	if ( pl == 0 )  return;
	m_setupSelDevice = pl->RetSelect();
	device = pl->RetName(m_setupSelDevice);

	pl = (CList*)pw->SearchControl(EVENT_LIST2);
	if ( pl == 0 )  return;
	m_setupSelMode = pl->RetSelect();
	mode = pl->RetName(m_setupSelMode);

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FULL);
	if ( pc == 0 )  return;
	bFull = pc->TestState(STATE_CHECK);
	m_setupFull = bFull;

	m_engine->ChangeDevice(device, mode, bFull);

	if ( m_bSimulSetup )
	{
		m_main->ChangeColor();
		m_main->UpdateMap();
	}
}


// Calcule le checksum d'un fichier quelconque.

int CheckFile(char *filename)
{
	FILE*	file;
	int		len, i, check;
	char*	buffer;

	file = fopen(filename, "rb");
	if ( file == NULL )  return 0;

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (char*)malloc(len);
	fread(buffer, len, 1, file);

	check = 0;
	for ( i=0 ; i<len ; i++ )
	{
		check += buffer[i];
	}

	free(buffer);
	fclose(file);
	return check;
}

// Calcule les checksums d'une mission.

BOOL CMainDialog::ComputeCheck(int rank, int check[])
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];
	char		line[500];
	char		name[100];
	char		dir[100];
	char*		p;
	int			i, len;

	for ( i=0 ; i<10 ; i++ )
	{
		check[i] = 0;
	}

	BuildSceneName(filename, m_sceneBase, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return FALSE;

	while ( fgets(line, 500, file) != NULL )
	{
		for ( i=0 ; i<500 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		p = strstr(line, "Title.");
		if ( p == line )  continue;

		p = strstr(line, "Resume.");
		if ( p == line )  continue;

		len = strlen(line);
		for ( i=0 ; i<len ; i++ )
		{
			check[0] += line[i];
		}

		if ( Cmd(line, "TerrainRelief") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "textures");
			check[1] = CheckFile(dir);
		}
	}
	fclose(file);
	return TRUE;
}


// Met à jour le bouton "appliquer".

void CMainDialog::UpdateApply()
{
	CWindow*	pw;
	CButton*	pb;
	CList*		pl;
	CCheck*		pc;
	int			sel1, sel2;
	BOOL		bFull;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_APPLY);
	if ( pb == 0 )  return;

	pl = (CList*)pw->SearchControl(EVENT_LIST1);
	if ( pl == 0 )  return;
	sel1 = pl->RetSelect();

	pl = (CList*)pw->SearchControl(EVENT_LIST2);
	if ( pl == 0 )  return;
	sel2 = pl->RetSelect();

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FULL);
	bFull = pc->TestState(STATE_CHECK);

	if ( sel1 == m_setupSelDevice &&
		 sel2 == m_setupSelMode   &&
		 bFull == m_setupFull     )
	{
		pb->ClearState(STATE_ENABLE);
		pb->ClearState(STATE_FLASH);
	}
	else
	{
		pb->SetState(STATE_ENABLE);
		pb->SetState(STATE_FLASH);
	}
}

// Met à jour les boutons pendant la phase de setup.

void CMainDialog::UpdateSetupButtons()
{
	CWindow*	pw;
	CCheck*		pc;
	CSlider*	ps;
	float		value;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_TOOLTIP);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bTooltip);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_GLINT);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bGlint);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_RAIN);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bRain);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_MOUSE);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetNiceMouse());
		pc->SetState(STATE_ENABLE, m_engine->RetNiceMouseCap());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_MOVIES);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bMovies);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_CBACK);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_defCamera == CAMERA_BACK);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_CBOARD);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_defCamera == CAMERA_ONBOARD);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EFFECT);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bEffect);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FLASH);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bFlash);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_BLAST);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_bMotorBlast);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SHADOW);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetShadow());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_DIRTY);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetDirty());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FOG);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetFog());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LENS);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetLensMode());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SKY);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSkyMode());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_PLANET);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetPlanetMode());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LIGHT);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetLightMode());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SUPER);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSuperDetail());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_STEERING);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetJoystick()==1);
	}
	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_JOYPAD);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetJoystick()==2);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_FFBc);
	if ( pc != 0 )
	{
		if ( m_engine->RetJoystick() == 0 )
		{
			pc->SetState(STATE_ENABLE, FALSE);
			pc->SetState(STATE_CHECK, FALSE);
		}
		else
		{
			pc->SetState(STATE_ENABLE, TRUE);
			pc->SetState(STATE_CHECK, m_engine->RetFFB());
		}
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_FFBs);
	if ( ps != 0 )
	{
		if ( m_engine->RetJoystick() == 0 || !m_engine->RetFFB() )
		{
			ps->SetState(STATE_ENABLE, FALSE);
		}
		else
		{
			ps->SetState(STATE_ENABLE, TRUE);
		}
		ps->SetVisibleValue(m_engine->RetForce());
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PARTI);
	if ( ps != 0 )
	{
		value = m_engine->RetParticuleDensity();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_WHEEL);
	if ( ps != 0 )
	{
		value = m_engine->RetWheelTraceQuantity();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_CLIP);
	if ( ps != 0 )
	{
		value = m_engine->RetClippingDistance();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_DETAIL);
	if ( ps != 0 )
	{
		value = m_engine->RetObjectDetail();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_GADGET);
	if ( ps != 0 )
	{
		value = m_engine->RetGadgetQuantity();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_TEXTURE);
	if ( ps != 0 )
	{
		value = (float)m_engine->RetTextureQuality();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLSOUND);
	if ( ps != 0 )
	{
		value = (float)m_sound->RetAudioVolume();
		ps->SetVisibleValue(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLMUSIC);
	if ( ps != 0 )
	{
		value = (float)m_sound->RetMidiVolume();
		ps->SetVisibleValue(value);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SOUND3D);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_sound->RetSound3D());
		pc->SetState(STATE_ENABLE, m_sound->RetSound3DCap());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_COMMENTS);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_sound->RetComments());
	}
}

// Met à jour le moteur en fonction des boutons après la phase de setup.

void CMainDialog::ChangeSetupButtons()
{
	CWindow*	pw;
	CSlider*	ps;
	float		value;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_PARTI);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_engine->SetParticuleDensity(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_WHEEL);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_engine->SetWheelTraceQuantity(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_CLIP);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_engine->SetClippingDistance(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_DETAIL);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_engine->SetObjectDetail(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_GADGET);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_engine->SetGadgetQuantity(value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_TEXTURE);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_engine->SetTextureQuality((int)value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLSOUND);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_sound->SetAudioVolume((int)value);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLMUSIC);
	if ( ps != 0 )
	{
		value = ps->RetVisibleValue();
		m_sound->SetMidiVolume((int)value);
	}
}


// Mémorise tous les réglages.

void CMainDialog::SetupMemorize()
{
	float	fValue;
	int		iValue, i, j;
	char	key[500];
	char	num[10];

	SetProfileString("Directory", "scene",    m_sceneDir);
	SetProfileString("Directory", "savegame", m_savegameDir);
	SetProfileString("Directory", "public",   m_publicDir);
	SetProfileString("Directory", "user",     m_userDir);
	SetProfileString("Directory", "files",    m_filesDir);
	SetProfileString("Directory", "duel",     m_duelDir);

	iValue = m_engine->RetTotoMode();
	SetProfileInt("Setup", "TotoMode", iValue);

	iValue = m_bTooltip;
	SetProfileInt("Setup", "Tooltips", iValue);

	iValue = m_bGlint;
	SetProfileInt("Setup", "InterfaceGlint", iValue);

	iValue = m_bRain;
	SetProfileInt("Setup", "InterfaceGlint", iValue);

	iValue = m_engine->RetNiceMouse();
	SetProfileInt("Setup", "NiceMouse", iValue);

	iValue = m_bMovies;
	SetProfileInt("Setup", "Movies", iValue);

	iValue = m_bNiceReset;
	SetProfileInt("Setup", "NiceReset", iValue);

	iValue = m_bHimselfDamage;
	SetProfileInt("Setup", "HimselfDamage", iValue);

	iValue = m_defCamera;
	SetProfileInt("Setup", "DefaultCamera", iValue);

	iValue = m_bEffect;
	SetProfileInt("Setup", "InterfaceEffect", iValue);

	iValue = m_bFlash;
	SetProfileInt("Setup", "ScreenFlash", iValue);

	iValue = m_bMotorBlast;
	SetProfileInt("Setup", "MotorBlast", iValue);

	iValue = m_engine->RetShadow();
	SetProfileInt("Setup", "GroundShadow", iValue);

	iValue = m_engine->RetGroundSpot();
	SetProfileInt("Setup", "GroundSpot", iValue);

	iValue = m_engine->RetDirty();
	SetProfileInt("Setup", "ObjectDirty", iValue);

	iValue = m_engine->RetFog();
	SetProfileInt("Setup", "FogMode", iValue);

	iValue = m_engine->RetLensMode();
	SetProfileInt("Setup", "LensMode", iValue);

	iValue = m_engine->RetSkyMode();
	SetProfileInt("Setup", "SkyMode", iValue);

	iValue = m_engine->RetPlanetMode();
	SetProfileInt("Setup", "PlanetMode", iValue);

	iValue = m_engine->RetLightMode();
	SetProfileInt("Setup", "LightMode", iValue);

	iValue = m_engine->RetSuperDetail();
	SetProfileInt("Setup", "SuperDetail", iValue);

	fValue = m_engine->RetForce();
	SetProfileFloat("Setup", "JoystickForce", fValue);

	iValue = m_engine->RetFFB();
	SetProfileInt("Setup", "JoystickFFB", iValue);

	iValue = m_engine->RetJoystick();
	SetProfileInt("Setup", "UseJoystick", iValue);

	fValue = m_engine->RetParticuleDensity();
	SetProfileFloat("Setup", "ParticuleDensity", fValue);

	fValue = m_engine->RetWheelTraceQuantity();
	SetProfileFloat("Setup", "WheelTraceQuantity", fValue);

	fValue = m_engine->RetClippingDistance();
	SetProfileFloat("Setup", "ClippingDistance", fValue);

	fValue = m_engine->RetObjectDetail();
	SetProfileFloat("Setup", "ObjectDetail", fValue);

	fValue = m_engine->RetGadgetQuantity();
	SetProfileFloat("Setup", "GadgetQuantity", fValue);

	iValue = m_engine->RetTextureQuality();
	SetProfileInt("Setup", "TextureQuality", iValue);

	iValue = m_sound->RetAudioVolume();
	SetProfileInt("Setup", "AudioVolume", iValue);

	iValue = m_sound->RetMidiVolume();
	SetProfileInt("Setup", "MidiVolume", iValue);

	iValue = m_sound->RetSound3D();
	SetProfileInt("Setup", "Sound3D", iValue);

	iValue = m_sound->RetComments();
	SetProfileInt("Setup", "SoundComments", iValue);

	iValue = m_engine->RetEditIndentMode();
	SetProfileInt("Setup", "EditIndentMode", iValue);

	iValue = m_engine->RetEditIndentValue();
	SetProfileInt("Setup", "EditIndentValue", iValue);

	key[0] = 0;
	for ( i=0 ; i<100 ; i++ )
	{
		if ( m_engine->RetKey(i, 0) == 0 )  break;

		for ( j=0 ; j<2 ; j++ )
		{
			iValue = m_engine->RetKey(i, j);
			sprintf(num, "%d%c", iValue, j==0?'+':' ');
			strcat(key, num);
		}
	}
	SetProfileString("Setup", "KeyMap", key);

	iValue = m_bDeleteGamer;
	SetProfileInt("Setup", "DeleteGamer", iValue);

	m_engine->WriteProfile();
}

// Rappelle tous les réglages.

void CMainDialog::SetupRecall()
{
	float	fValue;
	int		iValue, i, j;
	char	key[500];
	char*	p;

	if ( GetProfileString("Directory", "scene", key, _MAX_FNAME) )
	{
		strcpy(m_sceneDir, key);
	}

	if ( GetProfileString("Directory", "savegame", key, _MAX_FNAME) )
	{
		strcpy(m_savegameDir, key);
	}

	if ( GetProfileString("Directory", "public", key, _MAX_FNAME) )
	{
		strcpy(m_publicDir, key);
	}

	if ( GetProfileString("Directory", "user", key, _MAX_FNAME) )
	{
		strcpy(m_userDir, key);
	}

	if ( GetProfileString("Directory", "files", key, _MAX_FNAME) )
	{
		strcpy(m_filesDir, key);
	}

	if ( GetProfileString("Directory", "duel", key, _MAX_FNAME) )
	{
		strcpy(m_duelDir, key);
	}


	if ( GetProfileInt("Setup", "TotoMode", iValue) )
	{
		m_engine->SetTotoMode(iValue);
	}

	if ( GetProfileInt("Setup", "Tooltips", iValue) )
	{
		m_bTooltip = iValue;
	}

	if ( GetProfileInt("Setup", "InterfaceGlint", iValue) )
	{
		m_bGlint = iValue;
	}

	if ( GetProfileInt("Setup", "InterfaceGlint", iValue) )
	{
		m_bRain = iValue;
	}

	if ( GetProfileInt("Setup", "NiceMouse", iValue) )
	{
		m_engine->SetNiceMouse(iValue);
	}

	if ( GetProfileInt("Setup", "Movies", iValue) )
	{
		m_bMovies = iValue;
	}

	if ( GetProfileInt("Setup", "NiceReset", iValue) )
	{
		m_bNiceReset = iValue;
	}

	if ( GetProfileInt("Setup", "HimselfDamage", iValue) )
	{
		m_bHimselfDamage = iValue;
	}

	if ( GetProfileInt("Setup", "DefaultCamera", iValue) )
	{
		m_defCamera = (CameraType)iValue;
	}

	if ( GetProfileInt("Setup", "InterfaceEffect", iValue) )
	{
		m_bEffect = iValue;
		m_camera->SetEffect(m_bEffect);
	}

	if ( GetProfileInt("Setup", "ScreenFlash", iValue) )
	{
		m_bFlash = iValue;
		m_camera->SetFlash(m_bFlash);
	}

	if ( GetProfileInt("Setup", "MotorBlast", iValue) )
	{
		m_bMotorBlast = iValue;
	}

	if ( GetProfileInt("Setup", "GroundShadow", iValue) )
	{
		m_engine->SetShadow(iValue);
	}

	if ( GetProfileInt("Setup", "GroundSpot", iValue) )
	{
		m_engine->SetGroundSpot(iValue);
	}

	if ( GetProfileInt("Setup", "ObjectDirty", iValue) )
	{
		m_engine->SetDirty(iValue);
	}

	if ( GetProfileInt("Setup", "FogMode", iValue) )
	{
		m_engine->SetFog(iValue);
		m_camera->SetOverBaseColor(RetColor(RetColor(0.0f)));
	}

	if ( GetProfileInt("Setup", "LensMode", iValue) )
	{
		m_engine->SetLensMode(iValue);
	}

	if ( GetProfileInt("Setup", "SkyMode", iValue) )
	{
		m_engine->SetSkyMode(iValue);
	}

	if ( GetProfileInt("Setup", "PlanetMode", iValue) )
	{
		m_engine->SetPlanetMode(iValue);
	}

	if ( GetProfileInt("Setup", "LightMode", iValue) )
	{
		m_engine->SetLightMode(iValue);
	}

	if ( GetProfileInt("Setup", "SuperDetail", iValue) )
	{
		m_engine->SetSuperDetail(iValue);
	}

	if ( GetProfileFloat("Setup", "JoystickForce", fValue) )
	{
		m_engine->SetForce(fValue);
	}
	if ( GetProfileInt("Setup", "JoystickFFB", iValue) )
	{
		m_engine->SetFFB(iValue);
	}
	if ( GetProfileInt("Setup", "UseJoystick", iValue) )
	{
		m_engine->SetJoystick(iValue);
	}

	if ( GetProfileFloat("Setup", "ParticuleDensity", fValue) )
	{
		m_engine->SetParticuleDensity(fValue);
	}

	if ( GetProfileFloat("Setup", "WheelTraceQuantity", fValue) )
	{
		m_engine->SetWheelTraceQuantity(fValue);
	}

	if ( GetProfileFloat("Setup", "ClippingDistance", fValue) )
	{
		m_engine->SetClippingDistance(fValue);
	}

	if ( GetProfileFloat("Setup", "ObjectDetail", fValue) )
	{
		m_engine->SetObjectDetail(fValue);
	}

	if ( GetProfileFloat("Setup", "GadgetQuantity", fValue) )
	{
		m_engine->SetGadgetQuantity(fValue);
	}

	if ( GetProfileInt("Setup", "TextureQuality", iValue) )
	{
		m_engine->SetTextureQuality(iValue);
	}

	if ( GetProfileInt("Setup", "AudioVolume", iValue) )
	{
		m_sound->SetAudioVolume(iValue);
	}

	if ( GetProfileInt("Setup", "MidiVolume", iValue) )
	{
		m_sound->SetMidiVolume(iValue);
	}

	if ( GetProfileInt("Setup", "SoundComments", iValue) )
	{
		m_sound->SetComments(iValue);
	}

	if ( GetProfileInt("Setup", "EditIndentMode", iValue) )
	{
		m_engine->SetEditIndentMode(iValue);
	}

	if ( GetProfileInt("Setup", "EditIndentValue", iValue) )
	{
		m_engine->SetEditIndentValue(iValue);
	}

	if ( GetProfileString("Setup", "KeyMap", key, 500) )
	{
		p = key;
		for ( i=0 ; i<100 ; i++ )
		{
			if ( p[0] == 0 )  break;

			for ( j=0 ; j<2 ; j++ )
			{
				sscanf(p, "%d", &iValue);
				m_engine->SetKey(i, j, iValue);
				while ( *p >= '0' && *p <= '9' )  p++;
				while ( *p == ' ' || *p == '+' )  p++;
			}
		}
	}

	if ( GetProfileInt("Setup", "DeleteGamer", iValue) )
	{
		m_bDeleteGamer = iValue;
	}
}


// Change le niveau général de qualité.

void CMainDialog::ChangeSetupQuality(int quality)
{
	BOOL	bEnable;
	float	value;
	int		iValue;

	bEnable = (quality >= 0);
	m_engine->SetShadow(bEnable);
	m_engine->SetGroundSpot(bEnable);
	m_engine->SetDirty(bEnable);
	m_engine->SetFog(bEnable);
	m_engine->SetLensMode(bEnable);
	m_engine->SetSkyMode(bEnable);
	m_engine->SetPlanetMode(bEnable);
	m_engine->SetLightMode(bEnable);
	m_camera->SetOverBaseColor(RetColor(RetColor(0.0f)));

	if ( quality <  0 )  value = 0.0f;
	if ( quality == 0 )  value = 1.0f;
	if ( quality >  0 )  value = 1.0f;
	m_engine->SetParticuleDensity(value);

	if ( quality <  0 )  value = 0.0f;
	if ( quality == 0 )  value = 1.0f;
	if ( quality >  0 )  value = 1.0f;
	m_engine->SetWheelTraceQuantity(value);

	if ( quality <  0 )  value = 0.5f;
	if ( quality == 0 )  value = 1.0f;
	if ( quality >  0 )  value = 2.0f;
	m_engine->SetClippingDistance(value);

	if ( quality <  0 )  value = 0.0f;
	if ( quality == 0 )  value = 1.0f;
	if ( quality >  0 )  value = 2.0f;
	m_engine->SetObjectDetail(value);

	if ( quality <  0 )  value = 0.5f;
	if ( quality == 0 )  value = 1.0f;
	if ( quality >  0 )  value = 1.0f;
	m_engine->SetGadgetQuantity(value);

	if ( quality <  0 )  iValue = 0;
	if ( quality == 0 )  iValue = 1;
	if ( quality >  0 )  iValue = 2;
	m_engine->SetTextureQuality(iValue);

	m_engine->SetSuperDetail(FALSE);

	m_engine->FirstExecuteAdapt(FALSE);
}


// Touches redéfinissables :

static int key_table[KEY_TOTAL] =
{
	KEYRANK_LEFT,
	KEYRANK_RIGHT,
	KEYRANK_UP,
	KEYRANK_DOWN,
	KEYRANK_BRAKE,
	KEYRANK_HORN,
	KEYRANK_CAMERA,
	KEYRANK_HELP,
};

static EventMsg key_event[KEY_TOTAL] =
{
	EVENT_INTERFACE_KLEFT,
	EVENT_INTERFACE_KRIGHT,
	EVENT_INTERFACE_KUP,
	EVENT_INTERFACE_KDOWN,
	EVENT_INTERFACE_KBRAKE,
	EVENT_INTERFACE_KHORN,
	EVENT_INTERFACE_KCAMERA,
	EVENT_INTERFACE_KHELP,
};

// Met à jour la liste des touches.

void CMainDialog::UpdateKey()
{
	CWindow*	pw;
	CScroll*	ps;
	CKey*		pk;
	FPOINT		pos, dim;
	int			first, i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	ps = (CScroll*)pw->SearchControl(EVENT_INTERFACE_KSCROLL);
	if ( ps == 0 )  return;

	first = (int)(ps->RetVisibleValue()*(KEY_TOTAL-KEY_VISIBLE));

	for ( i=0 ; i<KEY_TOTAL ; i++ )
	{
		pw->DeleteControl(key_event[i]);
	}

	dim.x = 400.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pos.x = 110.0f/640.0f;
	pos.y = 178.0f/480.0f + dim.y*(KEY_VISIBLE-1);
	for ( i=0 ; i<KEY_VISIBLE ; i++ )
	{
		pw->CreateKey(pos, dim, -1, key_event[first+i]);
		pk = (CKey*)pw->SearchControl(key_event[first+i]);
		if ( pk == 0 )  break;
		pk->SetKey(0, m_engine->RetKey(key_table[first+i], 0));
		pk->SetKey(1, m_engine->RetKey(key_table[first+i], 1));
		pos.y -= dim.y;
	}
}

// Change une touche.

void CMainDialog::ChangeKey(EventMsg event)
{
	CWindow*	pw;
	CScroll*	ps;
	CKey*		pk;
	int			i;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	ps = (CScroll*)pw->SearchControl(EVENT_INTERFACE_KSCROLL);
	if ( ps == 0 )  return;

	for ( i=0 ; i<KEY_TOTAL ; i++ )
	{
		if ( key_event[i] == event )
		{
			pk = (CKey*)pw->SearchControl(key_event[i]);
			if ( pk == 0 )  break;
			m_engine->SetKey(key_table[i], 0, pk->RetKey(0));
			m_engine->SetKey(key_table[i], 1, pk->RetKey(1));
		}
	}
}



// Voulez-vous quitter la mission en cours ?

void CMainDialog::StartAbort()
{
	CWindow*	pw;
	CButton*	pb;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.3f, 0.8f), TRUE, FALSE, FALSE);
	m_bDialogCreate = FALSE;
	m_bDialogDelete = FALSE;
	m_bDialogFile   = FALSE;
	m_bDialogKid    = FALSE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.35f;
	pos.y = 0.60f;
	dim.x = 0.30f;
	dim.y = 0.30f;
	pw->CreateGroup(pos, dim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
	pos.x = 0.35f;
	pos.y = 0.10f;
	dim.x = 0.30f;
	dim.y = 0.30f;
	pw->CreateGroup(pos, dim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

	pos.x = 0.40f;
	dim.x = 0.20f;
	dim.y = 32.0f/480.0f;

	pos.y = 0.66f;
	pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);
	pb->SetFontType(FONT_HILITE);
	pb->SetState(STATE_SHADOW);
	pb->SetTabOrder(0);
	pb->SetFocus(TRUE);
	GetResource(RES_TEXT, RT_DIALOG_NO, name);
	pb->SetName(name);

	pos.y = 0.55f;
	pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_SETUP);
	pb->SetFontType(FONT_HILITE);
	pb->SetState(STATE_SHADOW);
	pb->SetTabOrder(1);

	pos.y = 0.35f;
	pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_AGAIN);
	pb->SetFontType(FONT_HILITE);
	pb->SetState(STATE_SHADOW);
	pb->SetState(STATE_WARNING);
	pb->SetTabOrder(2);

	pos.y = 0.24f;
	pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_OK);
	pb->SetFontType(FONT_HILITE);
	pb->SetState(STATE_SHADOW);
	pb->SetState(STATE_WARNING);
	pb->SetTabOrder(3);
	GetResource(RES_TEXT, RT_DIALOG_YES, name);
	pb->SetName(name);
}

// Création d'un nouveau joueur.

void CMainDialog::StartCreateGame()
{
	CWindow*	pw;
	CButton*	pb;
	CEdit*		pe;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.7f, 0.6f), FALSE, TRUE, TRUE);
	m_bDialogCreate = TRUE;
	m_bDialogDelete = FALSE;
	m_bDialogFile   = FALSE;
	m_bDialogKid    = FALSE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.00f;
	pos.y = 0.52f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_TEXT, RT_DIALOG_NEWGAME, name);
	pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

	pos.x = 0.35f;
	pos.y = 0.46f;
	dim.x = 0.30f;
	dim.y = 0.05f;
	pe = pw->CreateEdit(pos, dim, 0, EVENT_INTERFACE_NEDIT);
	if ( pe == 0 )  return;
	pe->SetMaxChar(15);
	pe->SetFocus(TRUE);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESNEW, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NONEW, name);
	pb->SetName(name);
}

// Voulez-vous détruire le joueur ?

void CMainDialog::StartDeleteGame(char *gamer)
{
	CWindow*	pw;
	CButton*	pb;
	FPOINT		pos, dim;
	char		name[100];
	char		text[100];

	StartDialog(FPOINT(0.7f, 0.3f), FALSE, TRUE, TRUE);
	m_bDialogCreate = FALSE;
	m_bDialogDelete = TRUE;
	m_bDialogFile   = FALSE;
	m_bDialogKid    = FALSE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.00f;
	pos.y = 0.50f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_TEXT, RT_DIALOG_DELGAME, name);
	sprintf(text, name, gamer);
	pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, text);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
	pb->SetName(name);
}

// Voulez-vous détruire le fichier ?

void CMainDialog::StartDeleteFile(char *filename)
{
	CWindow*	pw;
	CButton*	pb;
	FPOINT		pos, dim;
	char		name[100];
	char		text[100];
	char*		p;

	StartDialog(FPOINT(0.7f, 0.4f), FALSE, TRUE, TRUE);
	m_bDialogCreate = FALSE;
	m_bDialogDelete = FALSE;
	m_bDialogFile   = TRUE;
	m_bDialogKid    = FALSE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	p = strchr(filename, '\\');
	if ( p == 0 )
	{
		p = filename;
	}
	else
	{
		p ++;  // saute "duel\"
	}

	pos.x = 0.00f;
	pos.y = 0.50f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_TEXT, RT_DIALOG_DELFILE, name);
	sprintf(text, name, p);
	pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, text);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
	pb->SetName(name);
}

// Voulez-vous vraiment jouer en mode CN ?

void CMainDialog::StartKidLevel()
{
	CWindow*	pw;
	CButton*	pb;
	CEdit*		pe;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.7f, 0.6f), FALSE, TRUE, TRUE);
	m_bDialogCreate = FALSE;
	m_bDialogDelete = FALSE;
	m_bDialogFile   = FALSE;
	m_bDialogKid    = TRUE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.20f;
	pos.y = 0.35f;
	dim.x = 0.60f;
	dim.y = 0.35f;
	pe = pw->CreateEdit(pos, dim, 0, EVENT_EDIT1);
	if ( pe == 0 )  return;
	sprintf(name, "script\\kid-%c.txt", RetLanguageLetter());
	pe->ReadText(name);
	pe->SetEditCap(FALSE);  // juste pour voir
	pe->SetHiliteCap(FALSE);
	pe->SetState(STATE_SHADOW);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESKID, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NOKID, name);
	pb->SetName(name);
}

// Voulez-vous quitter le jeu ?

void CMainDialog::StartQuit()
{
	CWindow*	pw;
	CButton*	pb;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.6f, 0.3f), FALSE, TRUE, TRUE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.00f;
	pos.y = 0.50f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_TEXT, RT_DIALOG_QUIT, name);
	pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESQUIT, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NOQUIT, name);
	pb->SetName(name);
}

// Début de l'affichage d'un dialogue.

void CMainDialog::StartDialog(FPOINT dim, BOOL bFire, BOOL bOK, BOOL bCancel)
{
	CWindow*	pw;
	CButton*	pb;
	FPOINT		pos, ddim;
	char		name[100];

	StartSuspend();

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW3);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW6);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW7);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW8);
	if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
	if ( pb != 0 )
	{
		pb->ClearState(STATE_VISIBLE);
	}

	m_bDialogFire = bFire;

	pos.x = (1.0f-dim.x)/2.0f;
	pos.y = (1.0f-dim.y)/2.0f;
	pw = m_interface->CreateWindows(pos, dim, bFire?16:8, EVENT_WINDOW9);
	pw->SetState(STATE_SHADOW);
	GetResource(RES_TEXT, RT_TITLE_BASE, name);
	pw->SetName(name);

	m_dialogPos = pos;
	m_dialogDim = dim;
	m_dialogTime = 0.0f;
	m_dialogParti = 999.0f;

	if ( bOK )
	{
		pos.x  = 0.50f-0.15f-0.02f;
		pos.y  = 0.50f-dim.y/2.0f+0.03f;
		ddim.x = 0.15f;
		ddim.y = 0.06f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_OK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		GetResource(RES_EVENT, EVENT_DIALOG_OK, name);
		pb->SetName(name);
	}

	if ( bCancel )
	{
		pos.x  = 0.50f+0.02f;
		pos.y  = 0.50f-dim.y/2.0f+0.03f;
		ddim.x = 0.15f;
		ddim.y = 0.06f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_CANCEL);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, name);
		pb->SetName(name);
	}

	m_sound->Play(SOUND_TZOING);
	m_bDialog = TRUE;
}

// Animation d'un dialogue.

void CMainDialog::FrameDialog(float rTime)
{
	CWindow*	pw;
	D3DVECTOR	pos, speed;
	FPOINT		dim, dpos, ddim;
	float		zoom;
	int			i;

	dpos = m_dialogPos;
	ddim = m_dialogDim;

	m_dialogTime += rTime;
	if ( m_dialogTime < 1.0f )
	{
		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
		if ( pw != 0 )
		{
			if ( m_dialogTime < 0.50f )
			{
				zoom = Bounce(m_dialogTime/0.50f);
			}
			else
			{
				zoom = 1.0f;
			}

			dpos.x += ddim.x/2.0f;
			dpos.y += ddim.y/2.0f;

			ddim.x *= zoom;
//?			ddim.y *= zoom;

			dpos.x -= ddim.x/2.0f;
			dpos.y -= ddim.y/2.0f;

			pw->SetPos(dpos);
			pw->SetDim(ddim);
		}
	}

	if ( !m_bGlint )  return;

	m_dialogParti += rTime;
	if ( m_dialogParti < m_engine->ParticuleAdapt(0.05f) )  return;
	m_dialogParti = 0.0f;

	if ( !m_bDialogFire )  return;

	dpos = m_dialogPos;
	ddim = m_dialogDim;

	pos.z = 0.0f;
	speed = D3DVECTOR(0.0f, 0.0f, 0.0f);

	for ( i=0 ; i<2 ; i++ )
	{
		// En bas.
		pos.x = dpos.x + ddim.x*Rand();
		pos.y = dpos.y;
		pos.x += (Rand()-0.5f)*(6.0f/640.0f);
		pos.y += Rand()*(16.0f/480.0f)-(10.0f/480.0f);
		dim.x = 0.01f+Rand()*0.01f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 1.0f, 0.0f, SH_INTERFACE);

		// En haut.
		pos.x = dpos.x + ddim.x*Rand();
		pos.y = dpos.y + ddim.y;
		pos.x += (Rand()-0.5f)*(6.0f/640.0f);
		pos.y -= Rand()*(16.0f/480.0f)-(10.0f/480.0f);
		dim.x = 0.01f+Rand()*0.01f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 1.0f, 0.0f, SH_INTERFACE);

		// A gauche.
		pos.y = dpos.y + ddim.y*Rand();
		pos.x = dpos.x;
		pos.x += Rand()*(16.0f/640.0f)-(10.0f/640.0f);
		pos.y += (Rand()-0.5f)*(6.0f/480.0f);
		dim.x = 0.01f+Rand()*0.01f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 1.0f, 0.0f, SH_INTERFACE);

		// A droite.
		pos.y = dpos.y + ddim.y*Rand();
		pos.x = dpos.x + ddim.x;
		pos.x -= Rand()*(16.0f/640.0f)-(10.0f/640.0f);
		pos.y += (Rand()-0.5f)*(6.0f/480.0f);
		dim.x = 0.01f+Rand()*0.01f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 1.0f, 0.0f, SH_INTERFACE);
	}
}

// Fin de l'affichage d'un dialogue.

void CMainDialog::StopDialog()
{
	CWindow*	pw;
	CButton*	pb;
	CEdit*		pe;

	m_dialogName[0] = 0;
	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw != 0 )
	{
		pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
		if ( pe != 0 )
		{
			pe->GetText(m_dialogName, 100);
		}
	}

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW2);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW5);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW6);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW7);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW8);
	if ( pw != 0 )  pw->SetState(STATE_ENABLE);

	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
	if ( pb != 0 )
	{
		pb->SetState(STATE_VISIBLE);
	}

	StopSuspend();
	m_interface->DeleteControl(EVENT_WINDOW9);
	m_bDialog = FALSE;
}

// Suspend la simulation pour une phase de dialogue.

void CMainDialog::StartSuspend()
{
	m_sound->MuteAll(TRUE);
	m_main->ClearInterface();
	m_bInitPause = m_engine->RetPause();
	m_engine->SetPause(TRUE);
	m_engine->SetOverFront(FALSE);  // over plane derrière
	m_main->StartSuspend();
	m_initCamera = m_camera->RetType();
	m_camera->SetType(CAMERA_DIALOG);
}

// Reprend la simulation après une phase de dialogue.

void CMainDialog::StopSuspend()
{
	m_sound->MuteAll(FALSE);
	m_main->ClearInterface();
	if ( !m_bInitPause )  m_engine->SetPause(FALSE);
	m_engine->SetOverFront(TRUE);  // over plane devant
	m_main->StopSuspend();
	m_camera->SetType(m_initCamera);
}


// Indique s'il faut utiliser les tooltips.

BOOL CMainDialog::RetTooltip()
{
	return m_bTooltip;
}

// Indique si un dialogue est affiché.

BOOL CMainDialog::IsDialog()
{
	return m_bDialog;
}


// Spécifie le nom de la scène choisie pour jouer.

void CMainDialog::SetSceneName(char* name)
{
	strcpy(m_sceneName, name);
}

// Retourne le nom de la scène choisie pour jouer.

char* CMainDialog::RetSceneName()
{
	return m_sceneName;
}

char* CMainDialog::RetSceneBase()
{
	return m_sceneBase;
}

// Spécifie le rang de la scène choisie pour jouer.

void CMainDialog::SetSceneRank(int rank)
{
	m_sceneRank = rank;
}

// Retourne le rang de la scène choisie pour jouer.

int CMainDialog::RetSceneRank()
{
	return m_sceneRank;
}

// Retourne nom de dossier de la scène utilisateur choisie pour jouer.

char* CMainDialog::RetSceneDir()
{
	int		i;

	i = (m_sceneRank/10)-1;

	if ( i < 0 || i >= m_userTotal )  return 0;
	return m_userList[i];
}

// Retourne le nom du dossier pour sauvegarder.

char* CMainDialog::RetSavegameDir()
{
	return m_savegameDir;
}

// Retourne le nom du dossier public.

char* CMainDialog::RetPublicDir()
{
	return m_publicDir;
}

// Indique s'il on est en train de jouer un duel.

BOOL CMainDialog::RetDuel()
{
	return m_bDuel;
}


// Indique s'il y a des reflets sur les boutons.

BOOL CMainDialog::RetGlint()
{
	return m_bGlint;
}

// Indique s'il faut montrer les cinématiques.

BOOL CMainDialog::RetMovies()
{
	return m_bMovies;
}

// Indique s'il faut faire une animation dans CTaskReset.

BOOL CMainDialog::RetNiceReset()
{
	return m_bNiceReset;
}

// Indique si les tirs provoquent des dommages à ses propres unités.

BOOL CMainDialog::RetHimselfDamage()
{
	return m_bHimselfDamage;
}

// Indique si le moteur peut exploser au démarrage.

BOOL CMainDialog::RetMotorBlast()
{
	return m_bMotorBlast;
}

// Gestion de la caméra par défaut.

CameraType CMainDialog::RetDefCamera()
{
	return m_defCamera;
}

void CMainDialog::SetDefCamera(CameraType type)
{
	m_defCamera = type;
}


// Lit le fichier du joueur.

BOOL CMainDialog::ReadGamerInfo()
{
	FILE*			file;
	D3DCOLORVALUE	color;
	char			line[100];
	int				index;

	FlushPerso();

	sprintf(line, "%s\\%s\\data.gam", m_savegameDir, m_main->RetGamerName());
	file = fopen(line, "r");
	if ( file == NULL )
	{
#if _DEMO
		BuyAllPerso();  // toutes les voitures
		m_perso.selectCar = 2;  // tijuana taxi
#endif
		return FALSE;
	}

#if _SE
	BuyAllPerso();  // toutes les voitures
#endif

	index = 0;
	while ( fgets(line, 100, file) != NULL )
	{
		if ( Cmd(line, "CarUsed") )
		{
#if _SE
			index = IndexPerso(OpInt(line, "model", 1));
			m_perso.subModel[index] = OpInt(line, "subModel", 0);

			color.r = 0.0f;
			color.g = 0.0f;
			color.b = 0.0f;
			color.a = 0.0f;
			m_perso.colorBody[index] = OpColorValue(line, "colorBody", color);

			m_perso.stateCars[index] = SC_OWNER;
#else
			if ( index < 49 )
			{
				m_perso.usedCars[index] = OpInt(line, "model", 1);
				m_perso.subModel[index] = OpInt(line, "subModel", 0);

				color.r = 0.0f;
				color.g = 0.0f;
				color.b = 0.0f;
				color.a = 0.0f;
				m_perso.colorBody[index] = OpColorValue(line, "colorBody", color);

				index ++;
			}
#endif
		}

		if ( Cmd(line, "CarSelect") )
		{
			m_perso.selectCar = OpInt(line, "model", 1);
		}

		if ( Cmd(line, "Pesetas") )
		{
			m_perso.pesetas = OpInt(line, "total", 0);
		}

		if ( Cmd(line, "GameMode") )
		{
			m_perso.level = OpInt(line, "level", 2);
		}
	}
#if !_SE
	if ( index > 0 )
	{
		m_perso.total = index;
		m_perso.bonus = index;
	}
#endif

	fclose(file);

#if _DEMO
	BuyAllPerso();  // toutes les voitures
	m_perso.selectCar = 2;  // tijuana taxi
#endif
	return TRUE;
}

// Ecrit le fichier du joueur.

BOOL CMainDialog::WriteGamerInfo()
{
	FILE*	file;
	char	line[100];
	int		i;

	sprintf(line, "%s\\%s\\data.gam", m_savegameDir, m_main->RetGamerName());
	file = fopen(line, "w");
	if ( file == NULL )  return FALSE;

	for ( i=0 ; i<m_perso.total ; i++ )
	{
#if _SE
		if ( m_perso.stateCars[i] != SC_OWNER )  continue;
#endif
		sprintf(line, "CarUsed model=%d subModel=%d colorBody=%.2f;%.2f;%.2f;%.2f\n",
					m_perso.usedCars[i],
					m_perso.subModel[i],
					m_perso.colorBody[i].r, m_perso.colorBody[i].g, m_perso.colorBody[i].b, m_perso.colorBody[i].a);
		fputs(line, file);
	}

	sprintf(line, "CarSelect model=%d\n", m_perso.selectCar);
	fputs(line, file);

	sprintf(line, "Pesetas total=%d\n", m_perso.pesetas);
	fputs(line, file);

	sprintf(line, "GameMode level=%d\n", m_perso.level);
	fputs(line, file);

	fclose(file);
	return TRUE;
}


// Lit le fichier des missions.

BOOL CMainDialog::ReadGamerMission()
{
	FILE*	file;
	char	line[100];
	int		chap, scene, i;

	if ( m_sceneName[0] == 0 )  return TRUE;

	for ( i=0 ; i<MAXSCENE ; i++ )
	{
		m_sceneInfo[i].numTry = 0;
		m_sceneInfo[i].bPassed = FALSE;
	}
	m_sceneInfo[0].numTry = 1;
	m_sceneInfo[0].bPassed = TRUE;
	m_sel[m_index] = 11;

	sprintf(line, "%s\\%s\\%s.gam", m_savegameDir, m_main->RetGamerName(), m_sceneName);
	file = fopen(line, "r");
	if ( file == NULL )  return FALSE;

	while ( fgets(line, 100, file) != NULL )
	{
		if ( Cmd(line, "Current") )
		{
			m_sel[m_index] = OpInt(line, "sel", 0);
		}

		if ( Cmd(line, "Mission") )
		{
			chap = OpInt(line, "chapter", 0);
			scene = OpInt(line, "scene", 0);
			i = chap*10+scene;
			if ( i >= 0 && i < MAXSCENE )
			{
				m_sceneInfo[i].numTry  = OpInt(line, "numTry", 0);
				m_sceneInfo[i].bPassed = OpInt(line, "passed", 0);
			}
		}
	}

	fclose(file);
	return TRUE;
}

// Ecrit le fichier des missions.

BOOL CMainDialog::WriteGamerMission()
{
	FILE*	file;
	char	line[100];
	int		i;

	if ( m_sceneName[0] == 0 )  return TRUE;

	sprintf(line, "%s\\%s\\%s.gam", m_savegameDir, m_main->RetGamerName(), m_sceneName);
	file = fopen(line, "w");
	if ( file == NULL )  return FALSE;

	sprintf(line, "Current sel=%d\n", m_sel[m_index]);
	fputs(line, file);

	for ( i=0 ; i<MAXSCENE ; i++ )
	{
		if ( m_sceneInfo[i].numTry == 0 )  continue;

		sprintf(line, "Mission chapter=%d scene=%d numTry=%d passed=%d\n",
				i/10, i%10, m_sceneInfo[i].numTry, m_sceneInfo[i].bPassed);
		fputs(line, file);
	}

	fclose(file);
	return TRUE;
}


// Retourne la couleur de carrosserie du véhicule sélectionné.

D3DCOLORVALUE CMainDialog::RetGamerColorCar()
{
	if ( m_bDuel )
	{
		return m_duelColor;
	}
	else
	{
		return m_perso.colorBody[m_perso.selectCar];
	}
}

void CMainDialog::SetGamerInfoTry(int rank, int numTry)
{
	if ( numTry > 100 )  numTry = 100;
	m_sceneInfo[rank].numTry = numTry;
}

int CMainDialog::RetGamerInfoTry(int rank)
{
	return m_sceneInfo[rank].numTry;
}

void CMainDialog::SetGamerInfoPassed(int rank, BOOL bPassed)
{
	m_sceneInfo[rank].bPassed = bPassed;
}

BOOL CMainDialog::RetGamerInfoPassed(int rank)
{
	return m_sceneInfo[rank].bPassed;
}

// Teste si le fichier d'une mission existe.

BOOL CMainDialog::MissionExist(int rank)
{
	FILE*		file = NULL;
	char		filename[_MAX_FNAME];

	BuildSceneName(filename, m_sceneName, rank);
	file = fopen(filename, "r");
	if ( file == NULL )  return FALSE;
	fclose(file);
	return TRUE;
}

// Cherche la mission suivante par défaut à effectuer.

void CMainDialog::NextMission()
{
	int		i, chap, rank;

	if ( !m_bPesetas )  return;  // course libre ?

	// Cherche une mission non passée dans le même chapitre.
	chap = m_sel[m_index]/10;
	for ( i=0 ; i<9 ; i++ )
	{
		rank = chap*10+(i+1);
		if ( !MissionExist(rank) )  break;

		if ( !m_sceneInfo[rank].bPassed )  // mission à faire ?
		{
			m_sel[m_index] = rank;
			return;
		}
	}

	if ( !m_sceneInfo[chap*10].bPassed )  // chapitre suivant pas accessible ?
	{
		rank = chap*10+1;  // refait 1ère mission du chapitre
		m_sel[m_index] = rank;
		return;
	}

	rank = (chap+1)*10+1;  // 1ère mission du chapitre suivant
	if ( MissionExist(rank) )
	{
		m_sel[m_index] = rank;
		return;
	}

	m_sel[m_index] = 11;  // recommence au tout début !
}

// Débloque les niveaux suivants si on a économisé assez de pesetas.

void CMainDialog::PesetasUnlock()
{
	int		i, limit;

	for ( i=0 ; i<100 ; i++ )
	{
		if ( !m_sceneInfo[i*10].bPassed )
		{
			limit = ReadPesetasLimit(i*10);
			if ( limit > m_perso.pesetas )  return;  // pas assez

			m_sceneInfo[i*10].bPassed = TRUE;
			if ( m_sceneInfo[i*10].numTry < 100 )
			{
				m_sceneInfo[i*10].numTry ++;
			}
			WriteGamerInfo();
			return;
		}
	}
}


// Lit le fichier des records d'une mission.

BOOL CMainDialog::ReadRecord(int rank, int type)
{
	RecordList*	record;
	FILE*		file;
	char		filename[100];

	if ( type == 0 )  record = &m_recordAll;
	else              record = &m_recordOne;

	ZeroMemory(record, sizeof(RecordList));

	sprintf(filename, "%s\\%s%.2d%.1d.%s", m_savegameDir, m_sceneName, rank/10, rank%10, (type==0)?"all":"one");
	file = fopen(filename, "rb");
	if ( file == NULL )  return FALSE;

	fread(record, sizeof(RecordList), 1, file);
	fclose(file);

	record->select = -1;
	return TRUE;
}

// Ecrit le fichier des records d'une mission.

BOOL CMainDialog::WriteRecord(int rank, int type)
{
	RecordList*	record;
	FILE*		file;
	char		filename[100];

	if ( type == 0 )  record = &m_recordAll;
	else              record = &m_recordOne;

	sprintf(filename, "%s\\%s%.2d%.1d.%s", m_savegameDir, m_sceneName, rank/10, rank%10, (type==0)?"all":"one");
	file = fopen(filename, "wb");
	if ( file == NULL )  return FALSE;

	fwrite(record, sizeof(RecordList), 1, file);
	fclose(file);

	return TRUE;
}

// Ajoute un nouveau record.
// Retourne TRUE s'il s'agit d'un record du monde pour la voiture utilisée !

BOOL CMainDialog::AddRecord(float time, float bonus, float value1, float value2, int points, int type)
{
	RecordList*	record;
	int			i, j;

	if ( type == 0 )  record = &m_recordAll;
	else              record = &m_recordOne;

	for ( i=0 ; i<record->total ; i++ )
	{
		if ( time <= record->list[i].time )
		{
			j = record->total;
			if ( j == MAXRECORD )  j --;
			while ( j > i )
			{
				record->list[j] = record->list[j-1];
				j --;
			}
			break;
		}
	}

	if ( i < MAXRECORD )  // ajoute à la fin de la liste ?
	{
		strcpy(record->list[i].gamerName, m_main->RetGamerName());
		record->list[i].time   = time;
		record->list[i].bonus  = bonus;
		record->list[i].value1 = value1;
		record->list[i].value2 = value2;
		record->list[i].points = points;
		record->list[i].model  = RetModel();
		record->select = i;

		if ( record->total < MAXRECORD )
		{
			record->total ++;
		}
	}

	for ( j=i-1 ; j>=0 ; j-- )
	{
		if ( record->list[j].model == RetModel() )  return FALSE;
	}
	return TRUE;  // c'est un record du monde
}

// Indique si un temps donné est un record absolu.
// Retourne une valeur >= 0 correspondant à l'amélioration du temps.

float CMainDialog::TimeRecord(float time, int type)
{
	RecordList*	record;

	if ( type == 0 )  record = &m_recordAll;
	else              record = &m_recordOne;

	if ( record->total == 0 )  return 0.0f;
	return record->list[0].time - time;
}

// Indique s'il n'existe aucun record.

BOOL CMainDialog::FirstRecord(int type)
{
	RecordList*	record;

	if ( type == 0 )  record = &m_recordAll;
	else              record = &m_recordOne;

	return ( record->total == 0 );
}

// Met à jour le tableau des records.

void CMainDialog::UpdateRecord(CArray *pa, int type)
{
	RecordList*	record;
	char		text[100];
	char		car[50];
	char		time[50];
	int			i;

	if ( type == 0 )  record = &m_recordAll;
	else              record = &m_recordOne;

	pa->Flush();  // vide la liste
	if ( m_bPesetas )  i = RT_WIN_HEADERm;
	else               i = RT_WIN_HEADERf;
	GetResource(RES_TEXT, i, text);
	pa->SetName(-1, text);  // texte de la légende

	for ( i=0 ; i<record->total ; i++ )
	{
		NamePerso(car, record->list[i].model);
		PutTime(time, record->list[i].time);
		if ( m_bPesetas )
		{
			sprintf(text, "%d:\t  %s\t%s\t%s\t%d", i+1,
					car, record->list[i].gamerName, time,
					record->list[i].points);
		}
		else
		{
			sprintf(text, "%d:\t  %s\t%s\t%s\t", i+1,
					car, record->list[i].gamerName, time);
		}
		pa->SetName(i, text);
	}

	pa->SetSelect(record->select);
	pa->ShowSelect();
}


// Met à jour la liste des voitures fantomes.

void CMainDialog::UpdateGhostList(BOOL bAll)
{
#if _DEMO|_SE
	return;
#else
	CWindow*	pw;
	CArray*		pa;
	CRecorder*	recorder;
	GhostFile	gf;
	long		hFile;
	struct _finddata_t fileBuffer;
	char		name[100];
	char		filename[100];
	char		sub[100];
	int			i, j;
	BOOL		bDo, bSwap;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pa = (CArray*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
	if ( pa == 0 )  return;

	pa->Flush();

//?	if ( !ComputeCheck(m_sel[m_index], check) )  return;
	recorder = new CRecorder(m_iMan);

	j = 0;
	strcpy(name, m_duelDir);
	strcat(name, "\\*.rec");
	hFile = _findfirst(name, &fileBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( (fileBuffer.attrib & _A_SUBDIR) == 0 )
			{
				strcpy(filename, m_duelDir);
				strcat(filename, "\\");
				strcat(filename, fileBuffer.name);
				if ( !recorder->ReadHeader(filename) )  continue;

				strcpy(m_ghostList[j].filename, fileBuffer.name);
				strcpy(m_ghostList[j].gamer, recorder->RetGamer());
				m_ghostList[j].type = recorder->RetType();
				m_ghostList[j].mission = recorder->RetMission();
				m_ghostList[j].model = recorder->RetModel();
				m_ghostList[j].chrono = recorder->RetChrono();

				strcpy(sub, "");
				if ( recorder->RetType() == 's' )  strcpy(sub, "scene");
				if ( recorder->RetType() == 'f' )  strcpy(sub, "free");
				if ( recorder->RetType() == 'u' )  strcpy(sub, "user");
				if ( recorder->RetType() == 'p' )  strcpy(sub, "proto");
				if ( ReadSceneTitle(sub, recorder->RetMission(), m_ghostList[j].title) )
				{
					j ++;
				}
			}
		}
		while ( _findnext(hFile, &fileBuffer) == 0 && j < GHOSTLISTMAX );
	}
	m_ghostTotal = j;

	do  // trie tous les noms :
	{
		bDo = FALSE;
		for ( i=0 ; i<m_ghostTotal-1 ; i++ )
		{
//?			if ( stricmp(m_ghostList[i].filename, m_ghostList[i+1].filename) > 0 )

			if ( m_ghostList[i].type != m_ghostList[i+1].type )
			{
				bSwap = ( m_ghostList[i].type < m_ghostList[i+1].type );
			}
			else
			{
				if ( m_ghostList[i].mission != m_ghostList[i+1].mission )
				{
					bSwap = ( m_ghostList[i].mission > m_ghostList[i+1].mission );
				}
				else
				{
					if ( PricePerso(m_ghostList[i].model) != PricePerso(m_ghostList[i+1].model) )
					{
						bSwap = ( PricePerso(m_ghostList[i].model) > PricePerso(m_ghostList[i+1].model) );
					}
					else
					{
						if ( m_ghostList[i].chrono != m_ghostList[i+1].chrono )
						{
							bSwap = ( m_ghostList[i].chrono > m_ghostList[i+1].chrono );
						}
						else
						{
							bSwap = FALSE;
						}
					}
				}
			}

			if ( bSwap )
			{
				gf               = m_ghostList[i];
				m_ghostList[i]   = m_ghostList[i+1];
				m_ghostList[i+1] = gf;
				bDo = TRUE;
			}
		}
	}
	while ( bDo );

	GetResource(RES_TEXT, RT_IO_HEADER, name);
	pa->SetName(-1, name);  // texte de la légende

	for ( j=0 ; j<m_ghostTotal ; j++ )
	{
		strcpy(name, m_ghostList[j].filename);
		i = strlen(name);
		if ( i > 4 )  name[i-4] = 0;  // enlève ".rec"

		strcat(name, "\t");
		strcat(name, m_ghostList[j].title);  // titre de la mission

		NamePerso(sub, m_ghostList[j].model);  // nom de la voiture
		strcat(name, "\t");
		strcat(name, sub);

		strcat(name, "\t");
		strcat(name, m_ghostList[j].gamer);  // nom du joueur

		PutTime(sub, m_ghostList[j].chrono);  // temps effectué
		strcat(name, "\t");
		strcat(name, sub);

		pa->SetName(j, name);

		if ( !bAll )
		{
			pa->SetEnable(j, UsedPerso(m_ghostList[j].model));
		}
	}

	if ( m_ghostSelect >= pa->RetTotal() )
	{
		m_ghostSelect = -1;
	}
	else
	{
		pa->SetSelect(m_ghostSelect);  // sélectionne comme précédemment
		pa->ShowSelect();  // montre
	}

	delete recorder;
#endif
}

// Sélectionne une voiture fantome dans la liste.

void CMainDialog::SelectGhostList()
{
	CWindow*	pw;
	CArray*		pa;
	CEdit*		pe;
	CButton*	pb;
	BOOL		bEnable;
	char		filename[200];
	char*		p;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pa = (CArray*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
	if ( pa == 0 )  return;
	pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_IONAME);
	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_ROK);

	m_ghostSelect = pa->RetSelect();
	bEnable = (m_ghostSelect != -1);
	if ( bEnable && !pa->RetEnable(m_ghostSelect) )  bEnable = FALSE;
#if _DEMO|_SE
	bEnable = FALSE;
#endif

	if ( bEnable && pe != 0 )
	{
		strcpy(filename, pa->RetName(m_ghostSelect));
		p = strchr(filename, '\t');
		if ( p != 0 )  *p = 0;

		pe->SetText(filename);
		pe->SetCursor(100, 0);
		pe->SetFocus(TRUE);
	}

	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bEnable);
	}
}

// Ecrit le fichier de la voiture fantome.

int CMainDialog::WriteGhostFile()
{
	FILE*		file;
	CWindow*	pw;
	CEdit*		pe;
	CRecorder*	recorder;
	char		name[100];

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return 1;
	pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_IONAME);
	if ( pe == 0 )  return 1;

	pe->GetText(name, 100);
	if ( name[0] == 0 )  return 1;

	_mkdir(m_duelDir);  // si n'existe pas encore !

	strcpy(m_ghostName, m_duelDir);
	strcat(m_ghostName, "\\");
	strcat(m_ghostName, name);
	strcat(m_ghostName, ".rec");

	file = fopen(m_ghostName, "rb");
	if ( file != NULL )  // fichier existe déjà ?
	{
		fclose(file);
		return 2;
	}

	recorder = m_main->RetRecorderRecord();
	if ( recorder == 0 )  return 1;
	if ( !recorder->Write(m_ghostName) )  return 1;
	return 0;
}

// Détruit le fichier de la voiture fantome.

BOOL CMainDialog::DeleteGhostFile()
{
	remove(m_ghostName);
	if ( WriteGhostFile() != 0 )  return FALSE;
	return TRUE;
}

// Lit le fichier de la voiture fantome.

BOOL CMainDialog::ReadGhostFile()
{
	CWindow*	pw;
	CArray*		pa;
	CRecorder*	recorder;
	char		filename[200];
	char*		p;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return FALSE;
	pa = (CArray*)pw->SearchControl(EVENT_INTERFACE_IOLIST);
	if ( pa == 0 )  return FALSE;

	if ( m_ghostSelect == -1 )  return FALSE;
	if ( !pa->RetEnable(m_ghostSelect) )  return FALSE;

	strcpy(filename, pa->RetName(m_ghostSelect));
	p = strchr(filename, '\t');
	if ( p != 0 )  *p = 0;

	strcpy(m_ghostName, m_duelDir);
	strcat(m_ghostName, "\\");
	strcat(m_ghostName, filename);
	strcat(m_ghostName, ".rec");

	recorder = new CRecorder(m_iMan);
	if ( !recorder->ReadHeader(m_ghostName) )
	{
		delete recorder;
		return FALSE;
	}

	m_duelLevel    = recorder->RetLevel();
	m_duelType     = recorder->RetType();
	m_duelMission  = recorder->RetMission();
	m_duelModel    = recorder->RetModel();
	m_duelSubModel = recorder->RetSubModel();
	m_duelColor    = recorder->RetColor();

	m_bDuel = TRUE;
	m_bGhostExist = TRUE;
	m_bGhostEnable = TRUE;
	m_phaseTerm = PHASE_READ;

	delete recorder;
	return TRUE;
}

// Retourne le nom du fichier de la voiture fantome à utiliser.

char* CMainDialog::RetGhostRead()
{
	return m_ghostName;
}

