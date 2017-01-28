// maindialog.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <d3d.h>
#include <shlobj.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "cryptfile.h"
#include "global.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "gamerfile.h"
#include "particule.h"
#include "object.h"
#include "motion.h"
#include "interface.h"
#include "button.h"
#include "color.h"
#include "gauge.h"
#include "check.h"
#include "key.h"
#include "group.h"
#include "image.h"
#include "scroll.h"
#include "slider.h"
#include "list.h"
#include "label.h"
#include "link.h"
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




// Construit la clé d'un puzzle "officiel" pour CGamerFile.
// On utilise un nom de fichier "bizarre" que l'utilisateur
// ne risque pas trop d'utiliser !

void KeyPuzzle(char *buffer, int rank)
{
	sprintf(buffer, "_pzz_%.3d", rank);
}

// Copie un nom de fichier, mais sans l'extention.
// Peut remplacer strcpy() !

void fnmcpy(char *dst, char *src)
{
	int		len, i;

	strcpy(dst, src);

	len = strlen(dst);
	for ( i=len-1 ; i-- ; i>=0 )
	{
		if ( dst[i] == '.' )
		{
			dst[i] = 0;
			break;
		}
	}
}




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

	m_gamerFile = new CGamerFile(m_iMan);

	m_phase        = PHASE_NAME;
	m_phaseSetup   = PHASE_SETUPg;
	m_phaseTerm    = PHASE_PUZZLE;
	m_sceneName[0] = 0;
	m_bSimulSetup  = FALSE;
	m_bDeleteGamer = TRUE;
	m_bEdit        = FALSE;
	m_bTest        = FALSE;
	m_bProto       = FALSE;
	m_environment  = 0;
	m_exportType   = 0;

	GamerChanged();
	m_index = INDEX_PUZZLE;

	FlushPerso();

	m_bRain          = TRUE;
	m_bAgain         = FALSE;
	m_bPlayEnable    = TRUE;
	m_dialogType     = DIALOG_NULL;
	m_bWriteFile     = FALSE;
	m_bMoveAnimation = FALSE;
	m_shotDelay      = 0;
	m_fadeOutDelay   = 0.0f;
	
	m_movePhase = 0;

	m_glintMouse = FPOINT(0.0f, 0.0f);
	m_glintTime  = 1000.0f;

	for ( i=0 ; i<10 ; i++ )
	{
		m_partiPhase[i] = 0;
		m_partiTime[i]  = 0.0f;
	}

	strcpy(m_sceneDir,    "scene");
	strcpy(m_savegameDir, "savegame");
	strcpy(m_defiDir,     "defi");
	strcpy(m_publicDir,   "program");
	strcpy(m_filesDir,    "files");

	m_bDialog = FALSE;

#if _DEMO | _SE
	char	dir[100];

	_mkdir(m_savegameDir);  // si n'existe pas encore !

	sprintf(dir, "%s\\%s", m_savegameDir, "demo");
	_mkdir(dir);
#endif
}

// Destructeur de l'application robot.

CMainDialog::~CMainDialog()
{
}


// Indique si les boutons défi/atelier sont accessibles.

BOOL CMainDialog::IsAccessibleDefiUser()
{
	char	puzzle[MAXFILENAME+2];
	int		i;

	for ( i=0 ; i<16 ; i++ )
	{
		KeyPuzzle(puzzle, i);
		if ( !m_gamerFile->RetPassed(puzzle) )  return FALSE;
	}
	return TRUE;
}

// Change de phase.

void CMainDialog::ChangePhase(Phase phase, Phase fadeIn)
{
	CWindow*		pw;
	CLabel*			pl;
	CList*			pli;
	CArray*			pa;
	CCheck*			pc;
	CScroll*		ps;
	CSlider*		psl;
	CButton*		pb;
	CGroup*			pg;
	CImage*			pi;
	FPOINT			pos, ppos, p1, p2, dim, ddim, dddim;
	float			ox, oy, sx, sy;
	char			name[100];
	char			text[100];
	int				res, i, j, k, n;
	BOOL			bEnable;

	m_phaseTime = 0.0f;

	if ( phase == PHASE_TERM )
	{
		phase = m_phaseTerm;
	}
	m_phase = phase;  // copie l'info de CRobotMain
	m_phaseFadeIn = fadeIn;

	m_moveButton = 0;
	m_engine->SetInterfaceMat(FPOINT(0.5f, 0.5f), 1.0f, 0.0f);
	
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
		pw->SetName(" ");

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

		ReadGamerMission();

		pos.y = oy+sy*9.7f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PUZZLE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		bEnable = IsAccessibleDefiUser();
#if _DEMO | _SE
		bEnable = FALSE;
#endif

		pos.y = oy+sy*8.6f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DEFI);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_ENABLE, bEnable);
#if _DEMO | _SE
		pb->SetState(STATE_DEMO);
#endif
		pb->SetTabOrder(1);

		pos.y = oy+sy*7.5f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_USER);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetState(STATE_ENABLE, bEnable);
#if _DEMO | _SE
		pb->SetState(STATE_DEMO);
#endif
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
#if _DEMO | _SE
		pb->SetState(STATE_ENABLE, FALSE);
		pb->SetState(STATE_DEMO);
#endif
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
	}

	if ( m_phase == PHASE_NAME )
	{
		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetName(" ");

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

		pos.x  = 144.0f/640.0f;
		pos.y  = 330.0f/480.0f;
		ddim.x = 120.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_PERSO_LIST, name);
		pl = pw->CreateLabel(pos, ddim, -1, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.x  = 140.0f/640.0f;
		pos.y  =  93.0f/480.0f;
		ddim.x = 160.0f/640.0f;
		ddim.y = 236.0f/480.0f;
		pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_NLIST);
		pli->SetState(STATE_DEFAULT);
		pli->SetState(STATE_SHADOW);
		pli->SetTabOrder(1);

		pos.x  = 380.0f/640.0f;
		pos.y  =  93.0f/480.0f;
		ddim.x = 140.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		pos.x  = 380.0f/640.0f;
		pos.y  = 291.0f/480.0f;
		ddim.x = 140.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NCREATE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		if ( m_bDeleteGamer )
		{
			pos.x  = 380.0f/640.0f;
			pos.y  = 247.0f/480.0f;
			ddim.x = 140.0f/640.0f;
			ddim.y =  32.0f/480.0f;
			pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NDELETE);
			pb->SetFontType(FONT_HILITE);
			pb->SetState(STATE_SHADOW);
			pb->SetTabOrder(3);
		}

		ReadNameList();
		UpdateNameControl();
	}

	if ( m_phase == PHASE_PUZZLE ||
		 m_phase == PHASE_PROTO  )
	{
		if ( m_phase == PHASE_PUZZLE )  m_index = INDEX_PUZZLE;
		if ( m_phase == PHASE_PROTO  )  m_index = INDEX_PROTO;

		if ( m_phase == PHASE_PUZZLE )  strcpy(m_sceneName, "puzzle");
		if ( m_phase == PHASE_PROTO  )  strcpy(m_sceneName, "proto");

		ReadGamerMission();
		if ( m_selectFilename[m_index][0] != 0 )
		{
			sscanf(m_selectFilename[m_index], "_pzz_%d", &m_sel);
			m_list = m_sel;
		}

		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		pw->SetName(" ");

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

		pos.x  =  94.0f/640.0f;
		ddim.x = 210.0f/640.0f;
		pos.y  = 370.0f/480.0f;
		ddim.y =  16.0f/480.0f;
		if ( m_phase == PHASE_PUZZLE )  res = RT_PLAY_LISTm;
		if ( m_phase == PHASE_PROTO  )  res = RT_PLAY_LISTp;
		GetResource(RES_TEXT, res, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);

		m_scrollOffset = (m_sel-32)/16;
		if ( m_scrollOffset < 0 )  m_scrollOffset = 0;

		pos.x = 526.0f/640.0f;
		pos.y = 122.0f/480.0f;
		dim.x =  16.0f/640.0f;
		dim.y = 252.0f/480.0f;
		ps = pw->CreateScroll(pos, dim, -1, EVENT_PUZZLE_SCROLL);
		ps->SetState(STATE_SHADOW);
		ps->SetVisibleRatio(0.6f);
		ps->SetVisibleValue(1.0f-m_scrollOffset/2.0f);
		ps->SetArrowStep(0.5f);

		pos.x =  94.0f/640.0f;
		pos.y = 122.0f/480.0f;
		for ( i=0 ; i<3 ; i++ )
		{
			dim.x = 426.0f/640.0f;
			dim.y =  80.0f/480.0f;
			pg = pw->CreateGroup(pos, dim, 40, (EventMsg)(EVENT_GROUP100+i));
			pg->SetState(STATE_SHADOW);

#if _DEMO | _SE
			p1.x = pos.x;
			p1.y = pos.y+17.0f/480.0f;
			dim.x = 380.0f/640.0f;
			dim.y =  26.0f/480.0f;
			GetResource(RES_TEXT, RT_DEMO, name);
			pl = pw->CreateLabel(p1, dim, 0, (EventMsg)(EVENT_LOCK100+i), name);
			pl->SetJustif(0);
			pl->SetFontSize(10.0f);
#else
			p1.x = pos.x+160.0f/640.0f;
			p1.y = pos.y+ 16.0f/480.0f;
			dim.x = 40.0f/640.0f;
			dim.y = 48.0f/480.0f;
			pw->CreateGroup(p1, dim, 18, (EventMsg)(EVENT_LOCK100+i));
#endif

			p1.x = pos.x+390.0f/640.0f;
			p1.y = pos.y;
			ddim.x = 100.0f/640.0f;
			ddim.y =  26.0f/480.0f;
			pl = pw->CreateLabel(p1, ddim, 0, (EventMsg)(EVENT_LABEL100+i), "");
			pl->SetJustif(1);
			pl->SetFontSize(30.0f);

			p1.x = pos.x+8.0f/640.0f;
			p1.y = pos.y+8.0f/480.0f;
			for ( j=0 ; j<4 ; j++ )
			{
#if _EGAMES & _DEMO
				if ( j >= 2 )  break;
#endif
				p2.x = p1.x;
				p2.y = p1.y;
				ddim.x = 64.0f/640.0f;
				ddim.y = 64.0f/480.0f;
				pi = pw->CreateImage(p2, ddim, 0, (EventMsg)(EVENT_IMAGE110+i*4+j));

				p2.x = p1.x+64.0f/640.0f;
				p2.y = p1.y;
				ddim.x = 16.0f/640.0f;
				ddim.y = 16.0f/480.0f;
				for ( k=0 ; k<4 ; k++ )
				{
					n = i*16+j*4+k;
					pb = pw->CreateButton(p2, ddim, -1, (EventMsg)(EVENT_LEVEL111+n));
					sprintf(name, "%d", k+1);
					pb->SetName(name);
					pb->SetFontSize(8.0f);
					p2.y += 16.0f/480.0f;
				}
				p1.x += 94.0f/640.0f;
			}
			pos.y += 86.0f/480.0f;
		}

		pos.x = 0.0f/640.0f;
		pos.y = 0.0f/480.0f;
		dim.x = 0.0f/640.0f;
		dim.y = 0.0f/480.0f;
		pw->CreateLink(pos, dim, -1, EVENT_PUZZLE_LINK);

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		pos.x  = 224.0f/640.0f;
		pos.y  =  66.0f/480.0f;
		ddim.x = 112.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, "");

		pos.x  = 336.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PLAY);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		if ( m_engine->RetDebugMode() )
		{
			pos.y -= 40.0f/480.0f;
			pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_EDIT);
			pb->SetFontType(FONT_HILITE);
			pb->SetState(STATE_SHADOW);
			pb->SetTabOrder(1);
		}

		if ( m_bMoveAnimation )
		{
			m_bMoveAnimation = FALSE;
			m_movePhase = 1;
			m_moveProgress = 0.0f;
		}
		else
		{
			m_moveCenter.x = 0.5f;
			m_moveCenter.y = 0.5f;
			m_moveZoom = 1.0f;
			m_moveAngle = -PI*0.5f;
			m_movePhase = 20;
			m_moveProgress = 0.0f;
		}
		UpdatePuzzleScroll();
		UpdatePuzzleButtons(TRUE);
		if ( m_moveButton == 0 )
		{
			m_movePhase = 0;
		}
	}

	if ( m_phase == PHASE_DEFI )
	{
		m_index = INDEX_DEFI;
		strcpy(m_sceneName, "defi");

		ReadGamerMission();

		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		pw->SetName(" ");

		pos.x  =  70.0f/640.0f;
		pos.y  =  53.0f/480.0f;
		ddim.x = 500.0f/640.0f;
		ddim.y = 347.0f/480.0f;
		pw->CreateGroup(pos, ddim, 45, EVENT_NULL);

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

		pos.x  =  94.0f/640.0f;
		pos.y  = 360.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_PLAY_LISTd, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);
		pl->SetFontSize(12.0f);
		pl->SetFontType(FONT_HILITE);

		pos.x  =  94.0f/640.0f;
		pos.y  = 155.0f/480.0f;
		ddim.x = 450.0f/640.0f;
		ddim.y = 212.0f/480.0f;
		pa = pw->CreateArray(pos, ddim, 0, EVENT_INTERFACE_LIST);
		pa->SetState(STATE_DEFAULT);
		pa->SetState(STATE_SHADOW);
		pa->SetState(STATE_EXTEND);  // x/v tout à droite
		pa->SetTabs(0, 110.0f/640.0f, 1, FONT_COLOBOT);  // filename
		pa->SetTabs(1,  70.0f/640.0f, 1, FONT_COLOBOT);  // univers
		pa->SetTabs(2,  70.0f/640.0f, 1, FONT_COLOBOT);  // auteur
		pa->SetTabs(3, 145.0f/640.0f, 1, FONT_COLOBOT);  // resumé
		pa->SetSelectCap(TRUE);
		pa->SetFontSize(9.0f);
		UpdateSceneList(m_sel);
		pa->SetTabOrder(6);

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(4);

		pos.x  =  94.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IMPORT);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(5);

		pos.x  = 234.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  90.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DELETE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(3);

#if _DEBUG
		pos.x  = 336.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_EDIT);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(1);
#endif

		pos.x  = 336.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PLAY);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		UpdateButtonList();

		m_bMoveAnimation = FALSE;
		m_movePhase = 1;
		m_moveProgress = 0.0f;
		m_moveButton = 0;
	}

	if ( m_phase == PHASE_USER )
	{
		m_index = INDEX_USER;
		strcpy(m_sceneName, "user");

		ReadGamerMission();

		pos.x  = 0.10f;
		pos.y  = 0.10f;
		ddim.x = 0.80f;
		ddim.y = 0.80f;
		pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW4);
		pw->SetClosable(TRUE);
		pw->SetName(" ");

#if 1
		pos.x  =  70.0f/640.0f;
		pos.y  =  64.0f/480.0f;
		ddim.x = 125.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);

		pos.x  =  70.0f/640.0f;
		pos.y  =  94.0f/480.0f;
		ddim.x = 125.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);

		pos.x  =  70.0f/640.0f;
		pos.y  = 124.0f/480.0f;
		ddim.x = 125.0f/640.0f;
		ddim.y =  30.0f/480.0f;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
		pos.x += ddim.x;
		pw->CreateGroup(pos, ddim, 32, EVENT_NULL);
#endif

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

		pos.x  =  94.0f/640.0f;
		pos.y  = 360.0f/480.0f;
		ddim.x = 210.0f/640.0f;
		ddim.y =  16.0f/480.0f;
		GetResource(RES_TEXT, RT_PLAY_LISTu, text);
		sprintf(name, text, m_main->RetGamerName());
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
		pl->SetJustif(1);
		pl->SetFontSize(12.0f);
		pl->SetFontType(FONT_HILITE);

		pos.x  =  94.0f/640.0f;
		pos.y  = 155.0f/480.0f;
		ddim.x = 450.0f/640.0f;
		ddim.y = 212.0f/480.0f;
		pa = pw->CreateArray(pos, ddim, 0, EVENT_INTERFACE_LIST);
		pa->SetState(STATE_DEFAULT);
		pa->SetState(STATE_SHADOW);
		pa->SetTabs(0, 110.0f/640.0f, 1, FONT_COLOBOT);  // filename
		pa->SetTabs(1,  70.0f/640.0f, 1, FONT_COLOBOT);  // univers
		pa->SetTabs(2,  45.0f/640.0f, 1, FONT_COLOBOT);  // testé
		pa->SetTabs(3, 190.0f/640.0f, 1, FONT_COLOBOT);  // resumé
		pa->SetSelectCap(TRUE);
		pa->SetFontSize(9.0f);
		UpdateSceneList(m_sel);
		pa->SetTabOrder(7);

		pos.x  =  94.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 130.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(5);

#if _DEUTSCH
		pos.x  =  94.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  77.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NEW);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(6);

		pos.x  = 171.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  97.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_RENAME);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(4);

		pos.x  = 268.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  87.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DELETE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(3);
#else
		pos.x  =  94.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  87.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NEW);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(6);

		pos.x  = 181.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  87.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_RENAME);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(4);

		pos.x  = 268.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x =  87.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_DELETE);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(3);
#endif

		pos.x  = 234.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 121.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_EXPORT);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);

		pos.x  = 367.0f/640.0f;
		pos.y  = 115.0f/480.0f;
		ddim.x = 179.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_EDIT);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(1);

		pos.x  = 367.0f/640.0f;
		pos.y  =  74.0f/480.0f;
		ddim.x = 179.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_PLAY);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		UpdateButtonList();

		m_bMoveAnimation = FALSE;
		m_movePhase = 1;
		m_moveProgress = 0.0f;
		m_moveButton = 0;
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
		pw->SetName(" ");

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
		ddim.x = dim.x*8;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*3;
		pos.y = 0.67f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LENSFLARE);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(6);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SUNBEAM);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(7);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_METEO);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(8);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_AMBIANCE);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(9);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(10);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DIRTY);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(11);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DETAIL);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(12);
		pos.y -= 0.048f;
		if ( !m_bSimulSetup )
		{
			pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DECOR);
			pc->SetState(STATE_SHADOW);
			pc->SetTabOrder(13);
		}
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_NICEMOUSE);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(13);

		ddim.x = dim.x*3;
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

		UpdateSetupButtons();
	}

	if ( m_phase == PHASE_SETUPp  ||  // setup/jeu ?
		 m_phase == PHASE_SETUPps )
	{
		ddim.x = dim.x*8;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*3;
		pos.y = 0.67f;

		dddim.x = 100.0f/640.0f;
		dddim.y = ddim.y;
		psl = pw->CreateSlider(pos, dddim, 0, EVENT_INTERFACE_SPEEDSCH);
		psl->SetState(STATE_SHADOW);
		psl->SetState(STATE_VALUE);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetTabOrder(6);
		ppos.x = pos.x+110.0f/640.0f;
		ppos.y = pos.y- 10.0f/480.0f;
		dddim.x = 200.0f/640.0f;
		dddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_SPEEDSCH, name);
		pl = pw->CreateLabel(ppos, dddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.y -= 0.048f;
		dddim.x = 100.0f/640.0f;
		dddim.y = ddim.y;
		psl = pw->CreateSlider(pos, dddim, 0, EVENT_INTERFACE_SPEEDSCV);
		psl->SetState(STATE_SHADOW);
		psl->SetState(STATE_VALUE);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetTabOrder(7);
		ppos.x = pos.x+110.0f/640.0f;
		ppos.y = pos.y- 10.0f/480.0f;
		dddim.x = 200.0f/640.0f;
		dddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_SPEEDSCV, name);
		pl = pw->CreateLabel(ppos, dddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOUSESCROLL);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(8);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_INVSCH);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(9);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_INVSCV);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(10);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EXPLOVIB);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(11);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOVIE);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(12);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_HELP);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(13);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_TOOLTIPS);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(14);
		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_ACCEL);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(15);

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
		ddim.x = dim.x*8;
		ddim.y = dim.y*0.5f;
		pos.x = ox+sx*3;
		pos.y = 0.65f;

		dddim.x = 100.0f/640.0f;
		dddim.y = ddim.y;
		psl = pw->CreateSlider(pos, dddim, 0, EVENT_INTERFACE_VOLBLUPI);
		psl->SetState(STATE_SHADOW);
		psl->SetState(STATE_VALUE);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetTabOrder(6);
		ppos.x = pos.x+110.0f/640.0f;
		ppos.y = pos.y- 10.0f/480.0f;
		dddim.x = 200.0f/640.0f;
		dddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_VOLBLUPI, name);
		pl = pw->CreateLabel(ppos, dddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.y -= 0.048f;
		dddim.x = 100.0f/640.0f;
		dddim.y = ddim.y;
		psl = pw->CreateSlider(pos, dddim, 0, EVENT_INTERFACE_VOLSOUND);
		psl->SetState(STATE_SHADOW);
		psl->SetState(STATE_VALUE);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetTabOrder(7);
		ppos.x = pos.x+110.0f/640.0f;
		ppos.y = pos.y- 10.0f/480.0f;
		dddim.x = 200.0f/640.0f;
		dddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_VOLSOUND, name);
		pl = pw->CreateLabel(ppos, dddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.y -= 0.048f;
		dddim.x = 100.0f/640.0f;
		dddim.y = ddim.y;
		psl = pw->CreateSlider(pos, dddim, 0, EVENT_INTERFACE_VOLAMBIANCE);
		psl->SetState(STATE_SHADOW);
		psl->SetState(STATE_VALUE);
		psl->SetLimit(0.0f, 1.0f);
		psl->SetArrowStep(0.1f);
		psl->SetTabOrder(8);
		ppos.x = pos.x+110.0f/640.0f;
		ppos.y = pos.y- 10.0f/480.0f;
		dddim.x = 200.0f/640.0f;
		dddim.y =  18.0f/480.0f;
		GetResource(RES_EVENT, EVENT_INTERFACE_VOLAMBIANCE, name);
		pl = pw->CreateLabel(ppos, dddim, 0, EVENT_LABEL1, name);
		pl->SetJustif(1);

		pos.y -= 0.048f;
		pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SOUND3D);
		pc->SetState(STATE_SHADOW);
		pc->SetTabOrder(9);

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
	}

	if ( m_phase == PHASE_READ )
	{
	}

	if ( m_phase == PHASE_FADEIN )  // transparent -> noir
	{
		m_engine->SetOverColor(RetColor(1.0f), D3DSTATETCw);
		m_engine->SetOverFront(TRUE);

		m_movePhase = 30;
		m_moveProgress = 0.0f;
		m_moveButton = 0;
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

#if 0
		pos.x  = 204.0f/640.0f;
		pos.y  = 230.0f/480.0f;
		ddim.x = 232.0f/640.0f;
		ddim.y =  42.0f/480.0f;
		pg = pw->CreateGroup(pos, ddim, 7, EVENT_NULL);
		pg->SetState(STATE_SHADOW);

		pos.x  = 200.0f/640.0f;
		pos.y  = 230.0f/480.0f;
		ddim.x = 240.0f/640.0f;
		ddim.y =  20.0f/480.0f;
		GetResource(RES_TEXT, RT_DIALOG_LOADING, name);
		pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
		pl->SetFontSize(12.0f);
		pl->SetJustif(0);
#endif

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

		m_engine->SetBackground("alsyd.tga", 0,0, 0.0f, TRUE, FALSE);
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

		m_engine->SetBackground("buzzing.tga", 0,0, 0.0f, TRUE, FALSE);
		m_engine->SetBackForce(TRUE);
	}
	if ( m_phase == PHASE_WELCOME3 )
	{
		pos.x  = 0.0f;
		pos.y  = 0.0f;
		ddim.x = 0.0f;
		ddim.y = 0.0f;
		pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW4);

#if _FRENCH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0.0f, TRUE, FALSE);
#endif
#if _ENGLISH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0.0f, TRUE, FALSE);
#endif
#if _DEUTSCH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0.0f, TRUE, FALSE);
#endif
#if _ITALIAN
		m_engine->SetBackground("epsitecf.tga", 0,0, 0.0f, TRUE, FALSE);
#endif
#if _SPANISH
		m_engine->SetBackground("epsitecf.tga", 0,0, 0.0f, TRUE, FALSE);
#endif
#if _PORTUGUESE
		m_engine->SetBackground("epsitecf.tga", 0,0, 0.0f, TRUE, FALSE);
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
#if _DEMO | _SE
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

#if _DEMO | _SE
		pos.x  =  20.0f/640.0f;
		pos.y  = 436.0f/480.0f;
		ddim.x =  28.0f/640.0f;
		ddim.y =  28.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, m_phase==PHASE_GENERIC1?32:55, EVENT_INTERFACE_PREV);
//?		pb->SetState(STATE_ENABLE, m_phase != PHASE_GENERIC1);
//?		pb->SetState(STATE_SHADOW);
		pos.x += 28.0f/640.0f;
		pb = pw->CreateButton(pos, ddim, m_phase==PHASE_GENERIC5?32:48, EVENT_INTERFACE_NEXT);
//?		pb->SetState(STATE_ENABLE, m_phase != PHASE_GENERIC5);
//?		pb->SetState(STATE_SHADOW);

		pos.x  = 564.0f/640.0f;
		pos.y  = 436.0f/480.0f;
		ddim.x =  28.0f/640.0f;
		ddim.y =  28.0f/480.0f;
		pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
//?		pb->SetState(STATE_SHADOW);
		pos.x += 28.0f/640.0f;
		pb = pw->CreateButton(pos, ddim, 11, EVENT_INTERFACE_EXIT);
//?		pb->SetState(STATE_SHADOW);

		pos.x  =  80.0f/640.0f;
		pos.y  = 440.0f/480.0f;
		ddim.x = 480.0f/640.0f;
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

#if _DEMO | _SE
		sprintf(name, "gener%c.tga", '1'+(char)(m_phase-PHASE_GENERIC1));
		m_engine->SetBackground(name, 0,0, 0.0f, TRUE, TRUE);
#else
		m_engine->SetBackground("generf.tga", 0,0, 0.0f, TRUE, TRUE);
#endif
		m_engine->SetBackForce(TRUE);
	}

	if ( m_phase == PHASE_INIT    ||
		 m_phase == PHASE_NAME    ||
		 m_phase == PHASE_PUZZLE  ||
		 m_phase == PHASE_DEFI    ||
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
	Event		newEvent;
	EventMsg	window;
	FPOINT		pos;
	float		welcomeLength, intensity;
	BOOL		bUpDown;
	int			err;

	if ( event.event == EVENT_FRAME )
	{
		m_phaseTime += event.rTime;

		FrameMove(event.rTime);

//?		if ( m_phase == PHASE_WELCOME1 )  welcomeLength = WELCOME_LENGTH+2.0f;
//?		else                              welcomeLength = WELCOME_LENGTH;
		welcomeLength = WELCOME_LENGTH;

		if ( m_phase == PHASE_WELCOME1 ||
			 m_phase == PHASE_WELCOME2 ||
			 m_phase == PHASE_WELCOME3 )
		{
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

//?			m_engine->SetOverColor(RetColor(intensity), mode);
		}

		if ( m_phase == PHASE_WELCOME1 && m_phaseTime >= welcomeLength )
		{
			m_main->ChangePhase(PHASE_WELCOME2);
			return TRUE;
		}
		if ( m_phase == PHASE_WELCOME2 && m_phaseTime >= welcomeLength )
		{
			m_main->ChangePhase(PHASE_WELCOME3);
			return TRUE;
		}
		if ( m_phase == PHASE_WELCOME3 && m_phaseTime >= welcomeLength )
		{
#if _DEMO | _SE
			m_main->ChangePhase(PHASE_INIT);
#else
			m_main->ChangePhase(PHASE_NAME);
#endif
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

		if ( m_phase == PHASE_FADEIN )  // transparent -> noir
		{
			if ( m_phaseTime < 0.5f )
			{
				intensity = 1.0f-(m_phaseTime/0.5f);
			}
			else
			{
				intensity = 0.0f;
			}
			m_engine->SetOverColor(RetColor(intensity), D3DSTATETCw);

			if ( m_phaseTime >= 0.5f )
			{
				if ( m_phaseFadeIn == PHASE_LOADING )
				{
					m_fadeOutDelay = 1.0f;
				}
				else
				{
					m_fadeOutDelay = 0.5f;
				}
				m_fadeOutProgress = 0.0f;
				m_main->ChangePhase(m_phaseFadeIn);
			}
			return FALSE;
		}

		if ( m_fadeOutDelay > 0.0f )  // noir -> transparent
		{
			m_fadeOutProgress += event.rTime;
			if ( m_fadeOutProgress < m_fadeOutDelay )
			{
				intensity = m_fadeOutProgress/m_fadeOutDelay;
				m_engine->SetOverColor(RetColor(intensity), D3DSTATETCw);
			}
			else
			{
				m_engine->SetOverFront(FALSE);
				m_engine->SetOverColor(RetColor(0.0f), D3DSTATETCb);
				m_fadeOutDelay = 0.0f;
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
			 m_phase == PHASE_PUZZLE  ||
			 m_phase == PHASE_DEFI    ||
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
				if ( m_dialogType == DIALOG_CREATEGAMER )
				{
					NameCreate();
				}
				if ( m_dialogType == DIALOG_DELETEGAMER )
				{
					NameDelete();
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
				if ( m_bEdit )
				{
					if ( m_dialogType != DIALOG_INFOPUZZLE &&
						 m_dialogType != DIALOG_ERROR      )
					{
						err = m_main->CheckPuzzle();
						if ( err == 0 )
						{
							m_bWriteFile = TRUE;
							m_main->ChangePhase(PHASE_FADEIN, PHASE_TERM);
						}
						else
						{
							StartError(err);  // affiche l'erreur
						}
					}
				}
				else
				{
					m_main->ChangePhase(PHASE_FADEIN, PHASE_TERM);
					m_main->UpdateInterface();
				}
			}
			else if ( m_phase == PHASE_WRITE )
			{
				m_main->ChangePhase(PHASE_WIN);
			}
			else if ( m_phase == PHASE_DEFI )
			{
				if ( m_dialogType == DIALOG_DELETEPUZZLE )
				{
					DeletePuzzle(m_list);
					UpdateSceneList(m_sel);
					UpdateButtonList();
				}
				if ( m_dialogType == DIALOG_IMPORTPUZZLE )
				{
					ImportPuzzle(m_importSelect);
					UpdateSceneList(m_sel);
					UpdateButtonList();
				}
			}
			else if ( m_phase == PHASE_USER )
			{
				if ( m_dialogType == DIALOG_NEWPUZZLE )
				{
					CreateNewPuzzle(m_environment, m_newPuzzleFilename);
					UpdateSceneList(m_sel);
					UpdateButtonList();
				}
				if ( m_dialogType == DIALOG_DELETEPUZZLE )
				{
					DeletePuzzle(m_list);
					UpdateSceneList(m_sel);
					UpdateButtonList();
				}
				if ( m_dialogType == DIALOG_EXPORTPUZZLE )
				{
					ExportPuzzle(m_list);
				}
				if ( m_dialogType == DIALOG_RENAMEPUZZLE )
				{
					RenamePuzzle(m_list, m_newPuzzleFilename);
					UpdateSceneList(m_sel);
					UpdateButtonList();
				}
			}
		}
		if ( event.event == EVENT_INTERFACE_EXPORTdefi )
		{
			m_exportType = 0;
			UpdateExportType();
		}
		if ( event.event == EVENT_INTERFACE_EXPORTdoc )
		{
			m_exportType = 1;
			UpdateExportType();
		}
		if ( event.event == EVENT_KEYDOWN && event.param == VK_RETURN )
		{
			if ( m_phase == PHASE_NAME &&
				 m_dialogType == DIALOG_CREATEGAMER )
			{
				StopDialog();
				NameCreate();
			}
			if ( m_phase == PHASE_USER &&
				 m_dialogType == DIALOG_RENAMEPUZZLE )
			{
				StopDialog();
				RenamePuzzle(m_list, m_newPuzzleFilename);
				UpdateSceneList(m_sel);
				UpdateButtonList();
			}
		}
		if ( event.event == EVENT_DIALOG_CANCEL ||
			 (event.event == EVENT_KEYDOWN && event.param == VK_ESCAPE) ||
			 (event.event == EVENT_KEYDOWN && event.param == VK_BUTTON9) )
		{
			StopDialog();
		}
		if ( event.event == EVENT_INTERFACE_SETUP )
		{
			StopDialog();
			StartSuspend();
			if ( m_phaseSetup == PHASE_SETUPd )  m_main->ChangePhase(PHASE_SETUPds);
			if ( m_phaseSetup == PHASE_SETUPg )  m_main->ChangePhase(PHASE_SETUPgs);
			if ( m_phaseSetup == PHASE_SETUPp )  m_main->ChangePhase(PHASE_SETUPps);
			if ( m_phaseSetup == PHASE_SETUPc )  m_main->ChangePhase(PHASE_SETUPcs);
			if ( m_phaseSetup == PHASE_SETUPs )  m_main->ChangePhase(PHASE_SETUPss);
		}
		if ( event.event == EVENT_INTERFACE_AGAIN )
		{
			StopDialog();
			if ( m_bEdit )
			{
				m_bWriteFile = FALSE;
				m_main->ChangePhase(PHASE_FADEIN, PHASE_TERM);
			}
			else
			{
				m_bAgain = TRUE;
				m_main->ChangePhase(PHASE_FADEIN, PHASE_LOADING);
			}
		}

		if ( m_dialogType == DIALOG_NEWPUZZLE &&
			 event.event == EVENT_PUZZLE_SCROLL )
		{
			UpdateNewPuzzle();
		}

		if ( m_dialogType == DIALOG_NEWPUZZLE &&
			 event.event >= EVENT_BUTTON0  &&
			 event.event <= EVENT_BUTTON11 )
		{
			m_environment = m_scrollOffset*4+event.event-EVENT_BUTTON0;
			UpdateNewPuzzle();
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
//?					m_sound->Play(SOUND_TZOING);
					m_main->ChangePhase(PHASE_GENERIC1);
				}
				break;

			case EVENT_INTERFACE_QUIT:
//?				StartQuit();  // voulez-vous quitter ?
//?				m_sound->Play(SOUND_TZOING);
				m_main->ChangePhase(PHASE_GENERIC1);
				break;

			case EVENT_INTERFACE_PUZZLE:
				m_main->ChangePhase(PHASE_PUZZLE);
				break;

			case EVENT_INTERFACE_DEFI:
				m_main->ChangePhase(PHASE_DEFI);
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

			case EVENT_INTERFACE_NLIST:
				NameSelect();
				UpdateNameControl();
				break;

			case EVENT_INTERFACE_NOK:
				if ( NameSelect() )
				{
					m_main->ChangePhase(PHASE_INIT);
				}
				break;

			case EVENT_INTERFACE_NCREATE:
				StartCreateGamer();
				break;

			case EVENT_INTERFACE_NDELETE:
				pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
				if ( pw == 0 )  break;
				pl = (CList*)pw->SearchControl(EVENT_INTERFACE_NLIST);
				if ( pl == 0 )  break;
				StartDeleteGamer(pl->RetName(pl->RetSelect()));
				break;
		}
	}

	if ( m_phase == PHASE_PUZZLE ||
		 m_phase == PHASE_DEFI   ||
		 m_phase == PHASE_USER   ||
		 m_phase == PHASE_PROTO  )
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
			if ( m_phase == PHASE_DEFI && !m_bPlayEnable )  return FALSE;
			m_phaseTerm = m_phase;
			LaunchSimul(FALSE, (m_phase==PHASE_USER));
			if ( m_phaseTerm == PHASE_USER )  m_bAgain = TRUE;
			return FALSE;
		}

		if ( event.event == EVENT_PUZZLE_SCROLL )
		{
			UpdatePuzzleScroll();
			UpdatePuzzleButtons(FALSE);
		}

		if ( event.event >= EVENT_LEVEL111 &&
			 event.event <= EVENT_LEVEL344 )
		{
			m_sel = m_scrollOffset*16+event.event-EVENT_LEVEL111;
			m_list = m_sel;
			KeyPuzzle(m_selectFilename[m_index], m_sel);
			WriteGamerMission();
			UpdatePuzzleButtons(FALSE);
		}

		if ( event.event == EVENT_INTERFACE_LIST )
		{
			UpdateButtonList();
			WriteGamerMission();
		}

		if ( event.event == EVENT_INTERFACE_PLAY )
		{
			if ( m_phase == PHASE_PROTO && m_sel == 0 )
			{
				m_main->ChangePhase(PHASE_MODELe);
				return FALSE;
			}
			if ( m_phase == PHASE_PROTO && m_sel == 4 )
			{
				m_main->ChangePhase(PHASE_MODELi);
				return FALSE;
			}
			m_phaseTerm = m_phase;
			LaunchSimul(FALSE, (m_phase==PHASE_USER));
			if ( m_phaseTerm == PHASE_USER )  m_bAgain = TRUE;
		}

		if ( event.event == EVENT_INTERFACE_NEW )
		{
			StartNewPuzzle();
		}

		if ( event.event == EVENT_INTERFACE_DELETE )
		{
			StartDeletePuzzle();
		}

		if ( event.event == EVENT_INTERFACE_RENAME )
		{
			StartRenamePuzzle();
		}

		if ( event.event == EVENT_INTERFACE_EXPORT )
		{
			StartExportPuzzle();
		}

		if ( event.event == EVENT_INTERFACE_IMPORT )
		{
			StartImportPuzzle();
		}

		if ( event.event == EVENT_INTERFACE_EDIT )
		{
			m_phaseTerm = m_phase;
			LaunchSimul(TRUE, FALSE);
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
			m_main->ChangePhase(PHASE_SIMUL);
			StopSuspend();
			return FALSE;
		}

		switch( event.event )
		{
			case EVENT_INTERFACE_SETUPd:
				m_main->ChangePhase(PHASE_SETUPds);
				break;

			case EVENT_INTERFACE_SETUPg:
				m_main->ChangePhase(PHASE_SETUPgs);
				break;

			case EVENT_INTERFACE_SETUPp:
				m_main->ChangePhase(PHASE_SETUPps);
				break;

			case EVENT_INTERFACE_SETUPc:
				m_main->ChangePhase(PHASE_SETUPcs);
				break;

			case EVENT_INTERFACE_SETUPs:
				m_main->ChangePhase(PHASE_SETUPss);
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
				if ( m_engine->RetSetup(ST_SHADOW) == 0.0f )
				{
					m_engine->SetSetup(ST_SHADOW, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_SHADOW, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_DIRTY:
				if ( m_engine->RetSetup(ST_DIRTY) == 0.0f )
				{
					m_engine->SetSetup(ST_DIRTY, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_DIRTY, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_SUNBEAM:
				if ( m_engine->RetSetup(ST_SUNBEAM) == 0.0f )
				{
					m_engine->SetSetup(ST_SUNBEAM, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_SUNBEAM, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_LENSFLARE:
				if ( m_engine->RetSetup(ST_LENSFLARE) == 0.0f )
				{
					m_engine->SetSetup(ST_LENSFLARE, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_LENSFLARE, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_DECOR:
				if ( m_engine->RetSetup(ST_DECOR) == 0.0f )
				{
					m_engine->SetSetup(ST_DECOR, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_DECOR, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_DETAIL:
				if ( m_engine->RetSetup(ST_DETAIL) == 0.0f )
				{
					m_engine->SetSetup(ST_DETAIL, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_DETAIL, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_AMBIANCE:
				if ( m_engine->RetSetup(ST_AMBIANCE) == 0.0f )
				{
					m_engine->SetSetup(ST_AMBIANCE, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_AMBIANCE, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_METEO:
				if ( m_engine->RetSetup(ST_METEO) == 0.0f )
				{
					m_engine->SetSetup(ST_METEO, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_METEO, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_MIN:
				ChangeSetupQuality(0);
				UpdateSetupButtons();
				break;
			case EVENT_INTERFACE_NORM:
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
			case EVENT_INTERFACE_EXPLOVIB:
				if ( m_engine->RetSetup(ST_EXPLOVIB) == 0.0f )
				{
					m_engine->SetSetup(ST_EXPLOVIB, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_EXPLOVIB, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_SPEEDSCH:
			case EVENT_INTERFACE_SPEEDSCV:
				ChangeSetupButtons();
				break;

			case EVENT_INTERFACE_MOUSESCROLL:
				if ( m_engine->RetSetup(ST_MOUSESCROLL) == 0.0f )
				{
					m_engine->SetSetup(ST_MOUSESCROLL, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_MOUSESCROLL, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_INVSCH:
				if ( m_engine->RetSetup(ST_INVSCH) == 0.0f )
				{
					m_engine->SetSetup(ST_INVSCH, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_INVSCH, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_INVSCV:
				if ( m_engine->RetSetup(ST_INVSCV) == 0.0f )
				{
					m_engine->SetSetup(ST_INVSCV, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_INVSCV, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_MOVIE:
				if ( m_engine->RetSetup(ST_MOVIE) == 0.0f )
				{
					m_engine->SetSetup(ST_MOVIE, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_MOVIE, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_HELP:
				if ( m_engine->RetSetup(ST_HELP) == 0.0f )
				{
					m_engine->SetSetup(ST_HELP, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_HELP, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_TOOLTIPS:
				if ( m_engine->RetSetup(ST_TOOLTIPS) == 0.0f )
				{
					m_engine->SetSetup(ST_TOOLTIPS, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_TOOLTIPS, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_NICEMOUSE:
				if ( m_engine->RetSetup(ST_NICEMOUSE) == 0.0f )
				{
					m_engine->SetSetup(ST_NICEMOUSE, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_NICEMOUSE, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_ACCEL:
				if ( m_engine->RetSetup(ST_ACCEL) == 0.0f )
				{
					m_engine->SetSetup(ST_ACCEL, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_ACCEL, 0.0f);
				}
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
			case EVENT_INTERFACE_KROTCW:
			case EVENT_INTERFACE_KROTCCW:
			case EVENT_INTERFACE_KSTOP:
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
		}
		return FALSE;
	}

	if ( m_phase == PHASE_SETUPs  ||  // setup/sound ?
		 m_phase == PHASE_SETUPss )
	{
		switch( event.event )
		{
			case EVENT_INTERFACE_VOLBLUPI:
			case EVENT_INTERFACE_VOLSOUND:
			case EVENT_INTERFACE_VOLAMBIANCE:
				ChangeSetupButtons();
				break;

			case EVENT_INTERFACE_SOUND3D:
				if ( m_engine->RetSetup(ST_SOUND3D) == 0.0f )
				{
					m_engine->SetSetup(ST_SOUND3D, 1.0f);
				}
				else
				{
					m_engine->SetSetup(ST_SOUND3D, 0.0f);
				}
				ChangeSetupButtons();
				UpdateSetupButtons();
				break;

			case EVENT_INTERFACE_SILENT:
				m_engine->SetSetup(ST_VOLBLUPI,    0.0f);
				m_engine->SetSetup(ST_VOLSOUND,    0.0f);
				m_engine->SetSetup(ST_VOLAMBIANCE, 0.0f);
				UpdateSetupButtons();
				break;
			case EVENT_INTERFACE_NOISY:
				m_engine->SetSetup(ST_VOLBLUPI,    0.8f);
				m_engine->SetSetup(ST_VOLSOUND,    0.8f);
				m_engine->SetSetup(ST_VOLAMBIANCE, 0.8f);
				UpdateSetupButtons();
				break;
		}
		return FALSE;
	}

	if ( m_phase == PHASE_WRITE )
	{
	}

	if ( m_phase == PHASE_READ )
	{
	}

	if ( m_phase == PHASE_WELCOME1 )
	{
		if ( event.event == EVENT_KEYDOWN     ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
			m_main->ChangePhase(PHASE_WELCOME2);
			return TRUE;
		}
	}
	if ( m_phase == PHASE_WELCOME2 )
	{
		if ( event.event == EVENT_KEYDOWN     ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
			m_main->ChangePhase(PHASE_WELCOME3);
			return TRUE;
		}
	}
	if ( m_phase == PHASE_WELCOME3 )
	{
		if ( event.event == EVENT_KEYDOWN     ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
#if _DEMO | _SE
			m_main->ChangePhase(PHASE_INIT);
#else
			m_main->ChangePhase(PHASE_NAME);
#endif
			return TRUE;
		}
	}

	if ( m_phase >= PHASE_GENERIC1 &&
		 m_phase <= PHASE_GENERIC5 )
	{
		if ( event.event == EVENT_INTERFACE_ABORT )
		{
			m_main->ChangePhase(PHASE_INIT);
		}
#if _DEMO | _SE
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
				m_main->ChangePhase(PHASE_GENERIC1);
			}
			else if ( m_phase == PHASE_GENERIC3 )
			{
				m_main->ChangePhase(PHASE_GENERIC2);
			}
			else if ( m_phase == PHASE_GENERIC4 )
			{
				m_main->ChangePhase(PHASE_GENERIC3);
			}
			else
			{
				m_main->ChangePhase(PHASE_GENERIC4);
			}
		}
		if ( event.event == EVENT_INTERFACE_NEXT )
		{
			if ( m_phase == PHASE_GENERIC1 )
			{
				m_main->ChangePhase(PHASE_GENERIC2);
			}
			else if ( m_phase == PHASE_GENERIC2 )
			{
				m_main->ChangePhase(PHASE_GENERIC3);
			}
			else if ( m_phase == PHASE_GENERIC3 )
			{
				m_main->ChangePhase(PHASE_GENERIC4);
			}
			else if ( m_phase == PHASE_GENERIC4 )
			{
				m_main->ChangePhase(PHASE_GENERIC5);
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
				m_main->ChangePhase(PHASE_INIT);
			}
#if !_DEMO & !_SE
			else
			{
				m_event->MakeEvent(newEvent, EVENT_QUIT);
				m_event->AddEvent(newEvent);
			}
#endif

#if _DEMO | _SE
			if ( event.param < 0x100 )
			{
				if ( m_phase == PHASE_GENERIC1 )
				{
					m_main->ChangePhase(PHASE_GENERIC2);
				}
				else if ( m_phase == PHASE_GENERIC2 )
				{
					m_main->ChangePhase(PHASE_GENERIC3);
				}
				else if ( m_phase == PHASE_GENERIC3 )
				{
					m_main->ChangePhase(PHASE_GENERIC4);
				}
				else if ( m_phase == PHASE_GENERIC4 )
				{
					m_main->ChangePhase(PHASE_GENERIC5);
				}
				else
				{
					m_event->MakeEvent(newEvent, EVENT_QUIT);
					m_event->AddEvent(newEvent);
				}
			}
#endif
		}

		if ( event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
#if _DEMO | _SE
			if ( m_phase == PHASE_GENERIC1 )
			{
				m_main->ChangePhase(PHASE_GENERIC2);
			}
			else if ( m_phase == PHASE_GENERIC2 )
			{
				m_main->ChangePhase(PHASE_GENERIC3);
			}
			else if ( m_phase == PHASE_GENERIC3 )
			{
				m_main->ChangePhase(PHASE_GENERIC4);
			}
			else if ( m_phase == PHASE_GENERIC4 )
			{
				m_main->ChangePhase(PHASE_GENERIC5);
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

// Evolution de mouvement de l'interface.

void CMainDialog::FrameMove(float rTime)
{
	FPOINT		center, pos;
	float		progress, zoom, angle;

	if ( m_movePhase != 30 )
	{
		if ( m_movePhase  == 0 ||
			 m_moveButton == 0 )  return;
	}

	if ( m_movePhase == 1 )  // début ?
	{
		m_engine->SetInterfaceMat(m_moveCenter, m_moveZoom, m_moveAngle);
		m_sound->Play(SOUND_OPEN);
		m_moveButton->SetState(STATE_FLASH, FALSE);
		m_moveButton->SetState(STATE_CHECK, TRUE);
		m_movePhase = 2;
		m_moveProgress = 0.0f;
	}

	if ( m_movePhase == 2 )  // bouton monte ?
	{
		m_moveProgress += rTime*(1.0f/2.0f);
		progress = Norm(m_moveProgress);

		pos = m_moveButtonPos;
		pos.y -= m_moveButtonDim.y;
		pos.y += m_moveButtonDim.y*progress;
		m_moveButton->SetPos(pos);

		m_engine->SetInterfaceMat(m_moveCenter, m_moveZoom, m_moveAngle);

		if ( m_moveProgress >= 1.0f )
		{
			m_moveButton->SetState(STATE_FLASH, TRUE);
			m_moveButton->SetState(STATE_CHECK, FALSE);
			m_sound->Play(SOUND_CLOSE);
			m_movePhase = 3;
			m_moveProgress = 0.0f;
		}
	}

	if ( m_movePhase == 3 )  // zoom arrière ?
	{
		m_moveProgress += rTime*(1.0f/0.4f);
		progress = Norm(m_moveProgress);

		center.x = m_moveCenter.x+(0.5f-m_moveCenter.x)*progress;
		center.y = m_moveCenter.y+(0.5f-m_moveCenter.y)*progress;
		zoom = m_moveZoom+(1.0f-m_moveZoom)*progress;
		angle = m_moveAngle+(0.0f-m_moveAngle)*progress;
		m_engine->SetInterfaceMat(center, zoom, angle);

		if ( m_moveProgress >= 1.0f )
		{
			m_movePhase = 0;
			UpdatePuzzleButtons(FALSE);
		}
	}

	if ( m_movePhase == 10 )  // écran monte ?
	{
		m_engine->SetInterfaceMat(m_moveCenter, m_moveZoom, m_moveAngle);
		m_sound->Play(SOUND_OPEN);
		m_moveButton->SetState(STATE_FLASH, TRUE);
		m_moveButton->SetState(STATE_CHECK, FALSE);
		m_movePhase = 11;
		m_moveProgress = 0.0f;
	}

	if ( m_movePhase == 11 )  // écran monte ?
	{
		m_moveProgress += rTime*(1.0f/2.0f);
		progress = Norm(m_moveProgress);

		center.x = m_moveCenter.x+(0.5f-m_moveCenter.x)*progress;
		center.y = m_moveCenter.y+(0.5f-m_moveCenter.y)*progress;
		zoom = m_moveZoom+(1.0f-m_moveZoom)*progress;
		angle = m_moveAngle+(0.0f-m_moveAngle)*progress;
		m_engine->SetInterfaceMat(center, zoom, angle);

		if ( m_moveProgress >= 1.0f )
		{
			m_sound->Play(SOUND_CLOSE);
			m_movePhase = 0;
			UpdatePuzzleButtons(FALSE);
		}
	}

	if ( m_movePhase == 20 )  // rotation ?
	{
		m_engine->SetInterfaceMat(m_moveCenter, m_moveZoom, m_moveAngle);
		m_sound->Play(SOUND_CLOWN, 1.0f, 0.9f);
		m_moveButton->SetState(STATE_FLASH, TRUE);
		m_moveButton->SetState(STATE_CHECK, FALSE);
		m_movePhase = 21;
		m_moveProgress = 0.0f;
	}

	if ( m_movePhase == 21 )  // rotation ?
	{
		m_moveProgress += rTime*(1.0f/0.5f);
		progress = Norm(m_moveProgress);

		center.x = m_moveCenter.x+(0.5f-m_moveCenter.x)*progress;
		center.y = m_moveCenter.y+(0.5f-m_moveCenter.y)*progress;
		zoom = m_moveZoom+(1.0f-m_moveZoom)*progress;
		angle = m_moveAngle+(0.0f-m_moveAngle)*progress;
		m_engine->SetInterfaceMat(center, zoom, angle);

		if ( m_moveProgress >= 1.0f )
		{
			m_sound->Play(SOUND_CLOSE, 1.0f, 2.0f);
			m_movePhase = 0;
			UpdatePuzzleButtons(FALSE);
		}
	}

	if ( m_movePhase == 30 )  // disparition ?
	{
		m_moveProgress += rTime*(1.0f/0.5f);
		progress = Norm(m_moveProgress);

		zoom = 1.0f-progress;
		m_engine->SetInterfaceMat(FPOINT(0.5f, 0.5f), zoom, 0.0f);

		if ( m_moveProgress >= 1.0f )
		{
			m_engine->SetInterfaceMat(FPOINT(0.5f, 0.5f), 1.0f, 0.0f);
			m_movePhase = 0;
		}
	}
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

void CMainDialog::LaunchSimul(BOOL bEdit, BOOL bTest)
{
	m_bEdit = bEdit;
	m_bTest = bTest;
	m_bProto = (m_phaseTerm == PHASE_PROTO);

	if ( m_phaseTerm == PHASE_PUZZLE )  strcpy(m_sceneName, "puzzle");
	if ( m_phaseTerm == PHASE_DEFI   )  strcpy(m_sceneName, "defi");
	if ( m_phaseTerm == PHASE_USER   )  strcpy(m_sceneName, "user");
	if ( m_phaseTerm == PHASE_PROTO  )  strcpy(m_sceneName, "proto");

	m_bAgain = FALSE;
	m_main->ChangePhase(PHASE_FADEIN, PHASE_LOADING);
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

	if ( m_phase == PHASE_PUZZLE ||
		 m_phase == PHASE_DEFI   ||
		 m_phase == PHASE_USER   ||
		 m_phase == PHASE_PROTO  )
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

// Fait évoluer qq jolies particules.

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

	if ( m_bDialog || !m_bRain || m_movePhase != 0 )  return;

	if ( m_phase == PHASE_INIT )
	{
		pParti = partiPosInit;
		pGlint = glintPosInit;
	}
	else if ( m_phase == PHASE_NAME   ||
			  m_phase == PHASE_PUZZLE ||
			  m_phase == PHASE_DEFI   ||
			  m_phase == PHASE_USER   ||
			  m_phase == PHASE_PROTO  ||
			  m_phase == PHASE_SETUPd ||
			  m_phase == PHASE_SETUPg ||
			  m_phase == PHASE_SETUPp ||
			  m_phase == PHASE_SETUPc ||
			  m_phase == PHASE_SETUPs ||
			  m_phase == PHASE_WRITE  ||
			  m_phase == PHASE_READ   )
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
	if ( (m_phase == PHASE_SIMUL  ||
		  m_phase == PHASE_WIN    ||
		  m_phase == PHASE_LOST   ||
		  m_phase == PHASE_MODELe ||
		  m_phase == PHASE_MODELi ) &&
		 !m_bDialog             )  return;
	if ( m_movePhase != 0 )  return;

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



// Construit le nom de fichier d'un puzzle.

void CMainDialog::BuildSceneName(char *filename)
{
	int		chap, group, level;

	filename[0] = 0;

	if ( strcmp(m_sceneName, "puzzle") == 0 )
	{
		chap  = (m_sel/16)+1;
		group = ((m_sel/4)%4)+1,
		level = (m_sel%4)+1,
		sprintf(filename, "%s\\%s%d%d%d.bm2",
							m_sceneDir, m_sceneName,
							chap, group, level);
	}

	if ( strcmp(m_sceneName, "defi") == 0 )
	{
		sprintf(filename, "%s\\%s.bm2",
							m_defiDir,
							m_listBuffer[m_list].filename);
	}

	if ( strcmp(m_sceneName, "user") == 0 )
	{
		sprintf(filename, "%s\\%s\\%s.bm2",
							m_savegameDir, m_main->RetGamerName(),
							m_listBuffer[m_list].filename);
	}

	if ( strcmp(m_sceneName, "proto") == 0 )
	{
		chap  = (m_sel/16)+1;
		group = ((m_sel/4)%4)+1,
		level = (m_sel%4)+1,
		sprintf(filename, "%s\\%s%d%d%d.bm2",
							m_sceneDir, m_sceneName,
							chap, group, level);
	}
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
			if ( stricmp(filenames[i], filenames[i+1]) > 0 )
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

	SetProfileString("Gamer", "LastName", m_main->RetGamerName());
	GamerChanged();
	return TRUE;
}

// Crée un nouveau joueur.

BOOL CMainDialog::NameCreate()
{
	CWindow*	pw;
	CButton*	pb;
	CList*		pl;
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
}


// Indique le niveau de difficulté.

int CMainDialog::RetLevel()
{
	return m_perso.level;
}

// Initialise le personnage de base lors de la création d'un joueur.

void CMainDialog::FlushPerso()
{
	ZeroMemory(&m_perso, sizeof(GamerPerso));
	m_perso.total = 1;
	m_perso.bonus = 1;
	m_perso.level = 3;  // niveau intermédiaire (costaud)
}

// Met à jour les listes selon le cheat code.

void CMainDialog::AllMissionUpdate()
{
	if ( m_phase == PHASE_PUZZLE )
	{
		UpdatePuzzleButtons(FALSE);
	}
}

// Lit les caractéristiques d'un fichier.

BOOL CMainDialog::ReadScene(char *filename, char *univers, char *resume,
							char *author, BOOL &bSolved, int &environment)
{
	CCryptFile	file;
	char		line[1000];
	char		text[1000];
	char		op[100];
	char		language;
	int			i;
#if !_DEMO & !_SE
	int			res;
#endif

	univers[0] = 0;
	resume[0] = 0;
	author[0] = 0;
	bSolved = FALSE;

	if ( !file.Open(filename, "r") )  return FALSE;

	language = RetLanguageLetter();
	environment = -1;
	while ( file.GetLine(line, 1000) )
	{
		if ( Cmd(line, "Environment") )
		{
			environment = OpInt(line, "type", -1);
		}

		for ( i=0 ; i<26 ; i++ )
		{
			sprintf(op, "Resume.%c", 'A'+i);
			if ( Cmd(line, op) )
			{
				OpString(line, "text", text);

				if ( language == 'A'+i && text[0] != 0 )
				{
					SpaceEscape(resume, text);
				}

				if ( resume[0] == 0 && text[0] != 0 )
				{
					SpaceEscape(resume, text);
				}
			}
		}

		if ( Cmd(line, "Author") )
		{
			OpString(line, "name", author);
		}

		if ( Cmd(line, "Solved") )
		{
			bSolved = OpInt(line, "state", 0);
		}

		if ( Cmd(line, "AmbiantColor") )
		{
			break;
		}
	}
	file.Close();

	if ( environment >= 0 && environment <= 19 )
	{
#if _DEMO | _SE
#if _DEMO
		sprintf(univers, "Demo %d", environment+1);
#endif
#if _SE
		sprintf(univers, "Value Ware %d", environment+1);
#endif
#else
		res = RT_UNIVERS0+environment;
		GetResource(RES_TEXT, res, univers);
#endif
	}

	return TRUE;
}

// Indique si un défi est accessible.

BOOL CMainDialog::IsAccessibleDefi(int environment)
{
	char	puzzle[MAXFILENAME+2];
	int		first, i;

	first = environment*4;

	for ( i=first ; i<first+4 ; i++ )
	{
		KeyPuzzle(puzzle, i);
		if ( !m_gamerFile->RetPassed(puzzle) )  return FALSE;
	}
	return TRUE;
}

// Met à jour la liste des missions.

void CMainDialog::UpdateSceneList(int &sel)
{
	CWindow*	pw;
	CArray*		pa;
	long		hFile;
	struct _finddata_t fileBuffer;
	char		dir[200];
	char		name[200];
	char		filename[100];
	char		univers[100];
	char		resume[1000];
	char		author[100];
	int			i, environment;
	BOOL		bSolved, bCheck;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pa = (CArray*)pw->SearchControl(EVENT_INTERFACE_LIST);
	if ( pa == 0 )  return;

	pa->Flush();

	if ( m_phase == PHASE_DEFI )
	{
		strcpy(dir, m_defiDir);
		strcat(dir, "\\");
	}
	if ( m_phase == PHASE_USER )
	{
		strcpy(dir, m_savegameDir);
		strcat(dir, "\\");
		strcat(dir, m_main->RetGamerName());
		strcat(dir, "\\");
	}

	strcpy(name, dir);
	strcat(name, "*.bm2");

	i = 0;
	hFile = _findfirst(name, &fileBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( (fileBuffer.attrib & _A_SUBDIR) == 0 )
			{
				strcpy(filename, dir);
				strcat(filename, fileBuffer.name);
				fnmcpy(m_listBuffer[i].filename, fileBuffer.name);
				i ++;
			}
		}
		while ( _findnext(hFile, &fileBuffer) == 0 && i < LISTMAX );
	}
	m_listTotal = i;

	GetResource(RES_TEXT, (m_phase==PHASE_DEFI)?RT_DEFI_HEADER:RT_USER_HEADER, name);
	pa->SetName(-1, name);  // texte de la légende

	sel = -1;
	for ( i=0 ; i<m_listTotal ; i++ )
	{
		sprintf(filename, "%s%s.bm2", dir, m_listBuffer[i].filename);
		ReadScene(filename, univers, resume, author, bSolved, environment);

		if ( m_phase == PHASE_DEFI )
		{
			bCheck = m_gamerFile->RetPassed(m_listBuffer[i].filename);
		}
		else
		{
			bCheck = bSolved;
		}

		m_listBuffer[i].bSolved = bCheck;

		strcpy(name, m_listBuffer[i].filename);

		strcat(name, "\t");
		strcat(name, univers);

		if ( m_phase == PHASE_DEFI )
		{
			strcat(name, "\t");
			strcat(name, author);
		}

		if ( m_phase == PHASE_USER )
		{
			strcat(name, "\t");
			if ( bCheck )  strcat(name, "v");
		}

		strcat(name, "\t");
		strncat(name, resume, 100);

		pa->SetName(i, name);
		pa->SetCheck(i, bCheck);
		if ( m_phase != PHASE_USER )
		{
			pa->SetEnable(i, bSolved);
		}
		pa->SetIndex(i, i);
		pa->SetSortValue(i, environment);

		if ( m_phase == PHASE_DEFI )
		{
			if ( !IsAccessibleDefi(environment) )
			{
				pa->SetEnable(i, FALSE);
			}
		}

		if ( strcmp(m_listBuffer[i].filename, m_selectFilename[m_index]) == 0 &&
			 pa->RetEnable(i) )
		{
			sel = i;
		}
	}
	if ( m_listTotal == 0 )  sel = -1;

	pa->SetSelect(sel);  // sélectionne comme précédemment
	pa->Sort();
	pa->ShowSelect();  // montre
}

// Met à jour les boutons selon la liste.

void CMainDialog::UpdateButtonList()
{
	CWindow*	pw;
	CArray*		pa;
	CButton*	pb;
	BOOL		bEnable;
	char		text[100];

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;
	pa = (CArray*)pw->SearchControl(EVENT_INTERFACE_LIST);
	if ( pa == 0 )  return;

	m_sel = pa->RetSelect();
	m_list = pa->RetIndex(m_sel);

	bEnable = (m_sel != -1);
	if ( !pa->RetEnable(m_sel) )  bEnable = FALSE;

	m_bPlayEnable = bEnable;

	if ( bEnable )
	{
		strcpy(m_selectFilename[m_index], m_listBuffer[m_list].filename);
	}
	else
	{
		strcpy(m_selectFilename[m_index], "");
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PLAY);
	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bEnable);

		if ( m_phase == PHASE_DEFI )
		{
			GetResource(RES_TEXT, RT_DIALOG_DEFI, text);
			pb->SetName(text);
		}
		if ( m_phase == PHASE_USER )
		{
			GetResource(RES_TEXT, RT_DIALOG_TEST, text);
			pb->SetName(text);
		}
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_EDIT);
	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bEnable);
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_DELETE);
	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bEnable);
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_RENAME);
	if ( pb != 0 )
	{
		pb->SetState(STATE_ENABLE, bEnable);
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_EXPORT);
	if ( pb != 0 )
	{
		if ( bEnable )
		{
			if ( !m_listBuffer[m_list].bSolved )  bEnable = FALSE;
		}
		pb->SetState(STATE_ENABLE, bEnable);
	}
}

// Met à jour les boutons selon l'ascenseur.

void CMainDialog::UpdatePuzzleScroll()
{
	CWindow*	pw;
	CScroll*	ps;
	CImage*		pi;
	CLabel*		pl;
	CGroup*		pg;
	char		name[50];
	int			i, j;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	ps = (CScroll*)pw->SearchControl(EVENT_PUZZLE_SCROLL);
	if ( ps == 0 )  return;

	m_scrollOffset = (int)((1.0f-(ps->RetVisibleValue()-0.25f))*2.0f);  // 2..0

	for ( i=0 ; i<3 ; i++ )
	{
		pg = (CGroup*)pw->SearchControl((EventMsg)(EVENT_GROUP100+i));
		if ( pg != 0 )
		{
			pg->SetIcon(40+m_scrollOffset+i);
		}

		pl = (CLabel*)pw->SearchControl((EventMsg)(EVENT_LABEL100+i));
		if ( pl != 0 )
		{
			sprintf(name, "%d", m_scrollOffset+i+1);
			pl->SetName(name);
		}

		for ( j=0 ; j<4 ; j++ )
		{
			pi = (CImage*)pw->SearchControl((EventMsg)(EVENT_IMAGE110+i*4+j));
			if ( pi != 0 )
			{
				sprintf(name, "diagram\\%s%d%d0.bmp", m_sceneName, m_scrollOffset+i+1, j+1);
				pi->SetFilenameImage(name);
			}
		}
	}
}

// Met à jour les boutons des puzzles.

void CMainDialog::UpdatePuzzleButtons(BOOL bInit)
{
	CWindow*	pw;
	CButton*	pb;
	CImage*		pi;
#if !_DEMO & !_SE
	CGroup*		pg;
#endif
	CLink*		pl;
	CLabel*		pla;
	FPOINT		src, dst, pos, dim, ppos;
	int			rChap, aChap, group, level, passed;
	char		puzzle[MAXFILENAME+2];
	char		text[100];
#if !_DEMO & !_SE
	char		univers[100];
#endif
	BOOL		bVisible, bGreen;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	pl = (CLink*)pw->SearchControl(EVENT_PUZZLE_LINK);
	if ( pl == 0 )  return;
	src.x = NAN;

	m_moveButton = 0;

	for ( rChap=0 ; rChap<3 ; rChap++ )
	{
		aChap = rChap+m_scrollOffset;

		if ( aChap == 0 )
		{
			passed = 16;
		}
		else
		{
			passed = 0;
#if !_DEMO & !_SE
			for ( group=0 ; group<16 ; group++ )
			{
				KeyPuzzle(puzzle, (aChap-1)*16+group);
				if ( m_gamerFile->RetPassed(puzzle) )  passed ++;
			}
#endif
		}
		bVisible = (passed == 16);
		if ( m_main->RetShowAll() )  bVisible = TRUE;

#if _DEMO | _SE
		pla = (CLabel*)pw->SearchControl((EventMsg)(EVENT_LOCK100+rChap));
		if ( pla != 0 )  pla->SetState(STATE_VISIBLE, !bVisible);
#else
		pg = (CGroup*)pw->SearchControl((EventMsg)(EVENT_LOCK100+rChap));
		if ( pg != 0 )  pg->SetState(STATE_VISIBLE, !bVisible);
#endif

		for ( group=0 ; group<4 ; group++ )
		{
			pi = (CImage*)pw->SearchControl((EventMsg)(EVENT_IMAGE110+rChap*4+group));
			if ( pi != 0 )  pi->SetState(STATE_VISIBLE, bVisible);
		}

		if ( !bVisible )
		{
			for ( group=0 ; group<16 ; group++ )
			{
				pb = (CButton*)pw->SearchControl((EventMsg)(EVENT_LEVEL111+rChap*16+group));
				if ( pb == 0 )  continue;

				pb->SetState(STATE_VISIBLE, FALSE);
			}
		}
		else
		{
			for ( group=0 ; group<4 ; group++ )
			{
				for ( level=0 ; level<4 ; level++ )
				{
					pb = (CButton*)pw->SearchControl((EventMsg)(EVENT_LEVEL111+rChap*16+group*4+level));
					if ( pb == 0 )  continue;

					if ( level == 0 || m_main->RetShowAll() )
					{
						bVisible = TRUE;
					}
					else
					{
						KeyPuzzle(puzzle, aChap*16+group*4+(level-1));
						bVisible = m_gamerFile->RetPassed(puzzle);
					}
					pb->SetState(STATE_VISIBLE, bVisible);
					KeyPuzzle(puzzle, aChap*16+group*4+level);
					pb->SetState(STATE_TODO,  !m_gamerFile->RetPassed(puzzle));
					pb->SetState(STATE_PASSED, m_gamerFile->RetPassed(puzzle));

					if ( m_sel == aChap*16+group*4+level )
					{
						pb->SetState(STATE_FLASH, TRUE);
						pos = pb->RetPos();
						dim = pb->RetDim();
						if ( m_movePhase == 0 )
						{
							src.x = pos.x+dim.x;
							src.y = pos.y+dim.y/2.0f;
						}
						else
						{
							m_moveButton = pb;
							m_moveButtonPos = pos;
							m_moveButtonDim = dim;
						}
						KeyPuzzle(puzzle, aChap*16+group*4+level);
						bGreen = m_gamerFile->RetPassed(puzzle);

						if ( bInit )
						{
							if ( m_movePhase != 20 )
							{
								if ( group == 0 && level == 0 )  // nouvel étage ?
								{
									m_moveCenter.x =  0.5f;
//?									m_moveCenter.y =  1.5f;
									m_moveCenter.y = -0.5f;
									m_moveZoom = 1.0f;
									m_moveAngle = 0.0f;
									m_movePhase = 10;
								}
								else
								{
									m_moveCenter = pos;
									m_moveZoom = 3.0f;
									m_moveAngle = 0.0f;
								}
							}
						}
					}
					else
					{
						pb->SetState(STATE_FLASH, FALSE);
					}
				}
			}
		}
	}

	pb = (CButton*)pw->SearchControl(EVENT_INTERFACE_PLAY);
	if ( pb != 0 )
	{
		GetResource(RES_TEXT, bGreen?RT_DIALOG_REPLAY:RT_DIALOG_PLAY, text);
		pb->SetName(text);
	}

	pla = (CLabel*)pw->SearchControl(EVENT_LABEL1);
	if ( pla != 0 )
	{
#if _DEMO | _SE
#if _DEMO
		sprintf(text, "Demo %d.%d", m_sel/4+1, (m_sel%4)+1);
#endif
#if _SE
		sprintf(text, "Value Ware %d.%d", m_sel/4+1, (m_sel%4)+1);
#endif
#else
		GetResource(RES_TEXT, RT_UNIVERS0+m_sel/4, univers);
		sprintf(text, "%s %d", univers, (m_sel%4)+1);
#endif
		pla->SetName(text);
	}

	dst.x  = 440.0f/640.0f;
	dst.y  = 106.0f/480.0f;
	pl->SetPoints(src, dst, bGreen);
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

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SHADOW);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_SHADOW)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_DIRTY);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_DIRTY)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SUNBEAM);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_SUNBEAM)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_LENSFLARE);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_LENSFLARE)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_DECOR);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_DECOR)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_DETAIL);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_DETAIL)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_METEO);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_METEO)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_AMBIANCE);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_AMBIANCE)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EXPLOVIB);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_EXPLOVIB)!=0.0f);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_SPEEDSCH);
	if ( ps != 0 )
	{
		ps->SetVisibleValue(m_engine->RetSetup(ST_SPEEDSCH));
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_SPEEDSCV);
	if ( ps != 0 )
	{
		ps->SetVisibleValue(m_engine->RetSetup(ST_SPEEDSCV));
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_MOUSESCROLL);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_MOUSESCROLL)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_INVSCH);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_INVSCH)!=0.0f);
		pc->SetState(STATE_ENABLE, m_engine->RetSetup(ST_MOUSESCROLL)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_INVSCV);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_INVSCV)!=0.0f);
		pc->SetState(STATE_ENABLE, m_engine->RetSetup(ST_MOUSESCROLL)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_MOVIE);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_MOVIE)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_HELP);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_HELP)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_TOOLTIPS);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_TOOLTIPS)!=0.0f);
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_NICEMOUSE);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_NICEMOUSE)!=0.0f);
		pc->SetState(STATE_ENABLE, m_engine->RetNiceMouseCap());
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_ACCEL);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_ACCEL)!=0.0f);
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLBLUPI);
	if ( ps != 0 )
	{
		ps->SetVisibleValue(m_engine->RetSetup(ST_VOLBLUPI));
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLSOUND);
	if ( ps != 0 )
	{
		ps->SetVisibleValue(m_engine->RetSetup(ST_VOLSOUND));
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLAMBIANCE);
	if ( ps != 0 )
	{
		ps->SetVisibleValue(m_engine->RetSetup(ST_VOLAMBIANCE));
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_SOUND3D);
	if ( pc != 0 )
	{
		if ( m_sound->RetSound3DCap() )
		{
			pc->SetState(STATE_CHECK, m_engine->RetSetup(ST_SOUND3D)!=0.0f);
			pc->SetState(STATE_ENABLE, TRUE);
		}
		else
		{
			pc->SetState(STATE_CHECK, FALSE);
			pc->SetState(STATE_ENABLE, FALSE);
		}
	}
}

// Met à jour le moteur en fonction des boutons après la phase de setup.

void CMainDialog::ChangeSetupButtons()
{
	CWindow*	pw;
	CSlider*	ps;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW4);
	if ( pw == 0 )  return;

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_SPEEDSCH);
	if ( ps != 0 )
	{
		m_engine->SetSetup(ST_SPEEDSCH, ps->RetVisibleValue());
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_SPEEDSCV);
	if ( ps != 0 )
	{
		m_engine->SetSetup(ST_SPEEDSCV, ps->RetVisibleValue());
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLBLUPI);
	if ( ps != 0 )
	{
		m_engine->SetSetup(ST_VOLBLUPI, ps->RetVisibleValue());
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLSOUND);
	if ( ps != 0 )
	{
		m_engine->SetSetup(ST_VOLSOUND, ps->RetVisibleValue());
	}

	ps = (CSlider*)pw->SearchControl(EVENT_INTERFACE_VOLAMBIANCE);
	if ( ps != 0 )
	{
		m_engine->SetSetup(ST_VOLAMBIANCE, ps->RetVisibleValue());
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
	SetProfileString("Directory", "defi",     m_defiDir);
	SetProfileString("Directory", "public",   m_publicDir);
	SetProfileString("Directory", "files",    m_filesDir);

	for ( i=0 ; i<100 ; i++ )
	{
		if ( m_engine->RetSetupName((SetupType)i, key) )
		{
			fValue = m_engine->RetSetup((SetupType)i);
			SetProfileFloat("Setup", key, fValue);
		}
	}

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

	if ( GetProfileString("Directory", "defi", key, _MAX_FNAME) )
	{
		strcpy(m_defiDir, key);
	}

	if ( GetProfileString("Directory", "public", key, _MAX_FNAME) )
	{
		strcpy(m_publicDir, key);
	}

	if ( GetProfileString("Directory", "files", key, _MAX_FNAME) )
	{
		strcpy(m_filesDir, key);
	}

	for ( i=0 ; i<100 ; i++ )
	{
		if ( m_engine->RetSetupName((SetupType)i, key) )
		{
			if ( GetProfileFloat("Setup", key, fValue) )
			{
				m_engine->SetSetup((SetupType)i, fValue);
			}
		}
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
	float	fValue;

	fValue = (quality==0)?0.0f:1.0f;

	m_engine->SetSetup(ST_SHADOW,    fValue);
	m_engine->SetSetup(ST_DIRTY,     fValue);
	m_engine->SetSetup(ST_SUNBEAM,   fValue);
	m_engine->SetSetup(ST_LENSFLARE, fValue);
	m_engine->SetSetup(ST_DECOR,     fValue);
	m_engine->SetSetup(ST_DETAIL,    fValue);
	m_engine->SetSetup(ST_METEO,     fValue);
	m_engine->SetSetup(ST_AMBIANCE,  fValue);

	m_engine->FirstExecuteAdapt(FALSE);
}


// Touches redéfinissables :

static int key_table[KEY_TOTAL] =
{
	KEYRANK_LEFT,
	KEYRANK_RIGHT,
	KEYRANK_UP,
	KEYRANK_DOWN,
	KEYRANK_ROTCW,
	KEYRANK_ROTCCW,
	KEYRANK_STOP,
	KEYRANK_HELP,
};

static EventMsg key_event[KEY_TOTAL] =
{
	EVENT_INTERFACE_KLEFT,
	EVENT_INTERFACE_KRIGHT,
	EVENT_INTERFACE_KUP,
	EVENT_INTERFACE_KDOWN,
	EVENT_INTERFACE_KROTCW,
	EVENT_INTERFACE_KROTCCW,
	EVENT_INTERFACE_KSTOP,
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
	CLabel*		pl;
	FPOINT		pos, dim;
	char		name[100];

	m_main->StopDisplayInfo();

	if ( m_bEdit )
	{
		StartDialog(FPOINT(0.7f, 0.3f), TRUE, FALSE, FALSE);
		m_dialogType = DIALOG_NULL;

		pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
		if ( pw == 0 )  return;

		pos.x = 0.15f;
		pos.y = 0.35f;
		dim.x = 0.30f;
		dim.y = 0.30f;
		pw->CreateGroup(pos, dim, 5, EVENT_INTERFACE_GLINTl);  // coin orange
		pos.x = 0.55f;
		pos.y = 0.35f;
		dim.x = 0.30f;
		dim.y = 0.30f;
		pw->CreateGroup(pos, dim, 4, EVENT_INTERFACE_GLINTr);  // coin bleu

		pos.x = 0.00f;
		pos.y = 0.48f;
		dim.x = 1.00f;
		dim.y = 0.05f;
		GetResource(RES_TEXT, RT_DIALOG_QUITEDIT, name);
		pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
		pl->SetFontSize(13.0f);

		pos.x = 160.0f/640.0f;
		pos.y = 190.0f/480.0f;
		dim.x =  96.0f/640.0f;
		dim.y =  32.0f/480.0f;
		pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_OK);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
//?		pb->SetState(STATE_WARNING);
		pb->SetFocus(TRUE);
		pb->SetTabOrder(0);
		GetResource(RES_TEXT, RT_DIALOG_YESQUITEDIT, name);
		pb->SetName(name);

		pos.x = 272.0f/640.0f;
		pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_AGAIN);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(1);
		GetResource(RES_TEXT, RT_DIALOG_NOQUITEDIT, name);
		pb->SetName(name);

		pos.x = 384.0f/640.0f;
		pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(2);
		GetResource(RES_TEXT, RT_DIALOG_CANQUITEDIT, name);
		pb->SetName(name);
	}
	else
	{
		StartDialog(FPOINT(0.3f, 0.8f), TRUE, FALSE, FALSE);
		m_dialogType = DIALOG_NULL;

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
}

// Création d'un nouveau joueur.

void CMainDialog::StartCreateGamer()
{
	CWindow*	pw;
	CButton*	pb;
	CEdit*		pe;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.7f, 0.6f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_CREATEGAMER;

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
	pe->SetState(STATE_SHADOW);
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

void CMainDialog::StartDeleteGamer(char *gamer)
{
	CWindow*	pw;
	CButton*	pb;
	FPOINT		pos, dim;
	char		name[100];
	char		text[100];

	StartDialog(FPOINT(0.7f, 0.3f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_DELETEGAMER;

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

// Voulez-vous détruire le puzzle ?

void CMainDialog::StartDeletePuzzle()
{
	CWindow*	pw;
	CButton*	pb;
	CLabel*		pl;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.5f, 0.4f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_DELETEPUZZLE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.00f;
	pos.y = 0.50f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_TEXT, RT_DIALOG_DELPUZ, name);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
	pl->SetFontSize(12.0f);

	pos.x = 0.00f;
	pos.y = 0.46f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_TEXT, (m_phase==PHASE_DEFI)?RT_DIALOG_DELPUZd:RT_DIALOG_DELPUZu, name);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
	pl->SetFontSize(12.0f);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESDELPUZ, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NODELPUZ, name);
	pb->SetName(name);
}

// Voulez-vous créer un nouveau puzzle ?

void CMainDialog::StartNewPuzzle()
{
	CWindow*	pw;
	CScroll*	ps;
	CButton*	pb;
	CImage*		pi;
	CLabel*		pl;
	CEdit*		pe;
	FPOINT		pos, p1, dim;
	char		dir[200];
	char		name[200];
	char		text[100];
	char		univers[100];
	int			i, j;

	StartDialog(FPOINT(0.8f, 0.8f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_NEWPUZZLE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x =   0.0f/640.0f;
	pos.y = 358.0f/480.0f;
	dim.x = 640.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_NEWPUZZLE, name);
	GetResource(RES_TEXT, RT_UNIVERS0+m_environment, univers);
	sprintf(text, name, univers);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, text);
	pl->SetFontSize(12.0f);

	m_scrollOffset = m_environment/4-2;
	if ( m_scrollOffset < 0 )  m_scrollOffset = 0;

	pos.x = 468.0f/640.0f;
	pos.y = 150.0f/480.0f;
	dim.x =  16.0f/640.0f;
	dim.y = 208.0f/480.0f;
	ps = pw->CreateScroll(pos, dim, -1, EVENT_PUZZLE_SCROLL);
	ps->SetState(STATE_SHADOW);
	ps->SetVisibleRatio(0.6f);
	ps->SetVisibleValue(1.0f-m_scrollOffset/2.0f);
	ps->SetArrowStep(0.5f);

	pos.y = 150.0f/480.0f;
	for ( i=0 ; i<3 ; i++ )
	{
		pos.x = 180.0f/640.0f;

		p1.x = pos.x-40.0f/640.0f;
		p1.y = pos.y;
		dim.x = 40.0f/640.0f;
		dim.y = 16.0f/480.0f;
		pl = pw->CreateLabel(p1, dim, 0, (EventMsg)(EVENT_LABEL100+i), "");
		pl->SetJustif(1);
		pl->SetFontSize(30.0f);

		for ( j=0 ; j<4 ; j++ )
		{
			p1.x = pos.x;
			p1.y = pos.y;
			dim.x = 64.0f/640.0f;
			dim.y = 64.0f/480.0f;
			pb = pw->CreateButton(p1, dim, -1, (EventMsg)(EVENT_BUTTON0+i*4+j));
			pb->SetState(STATE_SHADOW);
			pb->SetName(" ");

			p1.x = pos.x+8.0f/640.0f;
			p1.y = pos.y+8.0f/480.0f;
			dim.x = 48.0f/640.0f;
			dim.y = 48.0f/480.0f;
			pi = pw->CreateImage(p1, dim, 0, (EventMsg)(EVENT_IMAGE110+i*4+j));

			pos.x += 72.0f/640.0f;
		}
		pos.y += 72.0f/480.0f;
	}

	pos.x =  70.0f/640.0f;
	pos.y = 108.0f/480.0f;
	dim.x = 100.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_FILE, text);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_NULL, text);
	pl->SetJustif(-1);

	pos.x = 180.0f/640.0f;
	pos.y = 115.0f/480.0f;
	dim.x = 280.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pe = pw->CreateEdit(pos, dim, -1, EVENT_DIALOG_EDIT1);
	pe->SetState(STATE_SHADOW);
	pe->SetMaxChar(MAXFILENAME);
	pe->SetFilenameCap(TRUE);

	sprintf(dir, "%s\\%s", m_savegameDir, m_main->RetGamerName());
	if ( SearchNewName(dir, m_main->RetGamerName(), name, m_newPuzzleFilename) )
	{
		pe->SetText(m_newPuzzleFilename);
		pe->SetCursor(strlen(m_newPuzzleFilename), 0);
		pe->SetFocus(TRUE);
	}

	UpdateNewPuzzle();

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESNEWP, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NONEWP, name);
	pb->SetName(name);
}

// Met à jour le dialogue pour créer un nouveau puzzle.

void CMainDialog::UpdateNewPuzzle()
{
	CWindow*	pw;
	CScroll*	ps;
	CButton*	pb;
	CImage*		pi;
	CLabel*		pl;
	int			rChap, aChap, group, i, passed;
	char		puzzle[MAXFILENAME+2];
	char		name[100];
	char		text[100];
	char		univers[100];
	BOOL		bVisible;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	ps = (CScroll*)pw->SearchControl(EVENT_PUZZLE_SCROLL);
	if ( ps == 0 )  return;

	m_scrollOffset = (int)((1.0f-(ps->RetVisibleValue()-0.25f))*2.0f);  // 2..0

	for ( rChap=0 ; rChap<3 ; rChap++ )
	{
		aChap = rChap+m_scrollOffset;

		pl = (CLabel*)pw->SearchControl((EventMsg)(EVENT_LABEL100+rChap));
		if ( pl != 0 )
		{
			sprintf(name, "%d", m_scrollOffset+rChap+1);
			pl->SetName(name);
		}

		for ( group=0 ; group<4 ; group++ )
		{
			passed = 0;
			for ( i=0 ; i<4 ; i++ )
			{
				KeyPuzzle(puzzle, aChap*16+group*4+i);
				if ( m_gamerFile->RetPassed(puzzle) )  passed ++;
			}
			bVisible = (passed == 4);
			if ( m_main->RetShowAll() )  bVisible = TRUE;

			if ( bVisible )
			{
				pb = (CButton*)pw->SearchControl((EventMsg)(EVENT_BUTTON0+rChap*4+group));
				if ( pb != 0 )
				{
					pb->SetState(STATE_ENABLE, TRUE);
					pb->SetState(STATE_PASSED, m_environment==m_scrollOffset*4+rChap*4+group);
				}

				pi = (CImage*)pw->SearchControl((EventMsg)(EVENT_IMAGE110+rChap*4+group));
				if ( pi != 0 )
				{
					pi->SetState(STATE_VISIBLE, TRUE);
					sprintf(name, "diagram\\%s%d%d0.bmp", "puzzle", m_scrollOffset+rChap+1, group+1);
					pi->SetFilenameImage(name);
				}
			}
			else
			{
				pb = (CButton*)pw->SearchControl((EventMsg)(EVENT_BUTTON0+rChap*4+group));
				if ( pb != 0 )
				{
					pb->SetState(STATE_ENABLE, FALSE);
					pb->SetState(STATE_PASSED, FALSE);
				}

				pi = (CImage*)pw->SearchControl((EventMsg)(EVENT_IMAGE110+rChap*4+group));
				if ( pi != 0 )
				{
					pi->SetState(STATE_VISIBLE, FALSE);
				}
			}
		}
	}

	pl = (CLabel*)pw->SearchControl(EVENT_DIALOG_LABEL);
	if ( pl != 0 )
	{
		GetResource(RES_TEXT, RT_DIALOG_NEWPUZZLE, name);
		GetResource(RES_TEXT, RT_UNIVERS0+m_environment, univers);
		sprintf(text, name, univers);
		pl->SetName(text);
	}
}

// Informations complémentaires sur le puzzle.

void CMainDialog::StartInfoPuzzle()
{
	CWindow*	pw;
	CButton*	pb;
	CLabel*		pl;
	CEdit*		pe;
	FPOINT		pos, dim;
	char		name[100];
	char		text[100];

	StartDialog(FPOINT(0.8f, 0.6f), FALSE, TRUE, FALSE);
	m_dialogType = DIALOG_INFOPUZZLE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x =  70.0f/640.0f;
	pos.y = 300.0f/480.0f;
	dim.x =  90.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_RESUMEINFO, text);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_NULL, text);
	pl->SetJustif(-1);

	pos.x = 170.0f/640.0f;
	pos.y = 270.0f/480.0f;
	dim.x = 390.0f/640.0f;
	dim.y =  60.0f/480.0f;
	pe = pw->CreateEdit(pos, dim, -1, EVENT_DIALOG_EDIT1);
	pe->SetState(STATE_SHADOW);
	pe->SetMaxChar(1000-50);
	pe->SetMultiFont(TRUE);
//?	pe->SetTabOrder(0);
	pe->SetText(m_main->RetResume());

	pos.x =  70.0f/640.0f;
	pos.y = 230.0f/480.0f;
	dim.x =  90.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_SIGNINFO, text);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_NULL, text);
	pl->SetJustif(-1);

	pos.x = 170.0f/640.0f;
	pos.y = 200.0f/480.0f;
	dim.x = 390.0f/640.0f;
	dim.y =  60.0f/480.0f;
	pe = pw->CreateEdit(pos, dim, -1, EVENT_DIALOG_EDIT2);
	pe->SetState(STATE_SHADOW);
	pe->SetMaxChar(1000-50);
	pe->SetMultiFont(TRUE);
//?	pe->SetTabOrder(1);
	pe->SetText(m_main->RetSign());

	pos.x =  70.0f/640.0f;
	pos.y = 163.0f/480.0f;
	dim.x =  90.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_AUTHORINFO, text);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_NULL, text);
	pl->SetJustif(-1);

	pos.x = 170.0f/640.0f;
	pos.y = 170.0f/480.0f;
	dim.x = 390.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pe = pw->CreateEdit(pos, dim, -1, EVENT_DIALOG_EDIT3);
	pe->SetState(STATE_SHADOW);
	pe->SetMaxChar(20);
//?	pe->SetTabOrder(2);
	pe->SetText(m_main->RetAuthor());

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESINFO, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);
}

// Renomme un puzzle.

void CMainDialog::StartRenamePuzzle()
{
	CWindow*	pw;
	CLabel*		pl;
	CEdit*		pe;
	CButton*	pb;
	FPOINT		pos, dim;
	char		name[100];
	char		text[100];

	StartDialog(FPOINT(0.7f, 0.4f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_RENAMEPUZZLE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x =  96.0f/640.0f;
	pos.y = 262.0f/480.0f;
	dim.x = 448.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_RENAMEGAME, name);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
	pl->SetFontSize(11.0f);

	pos.x =  70.0f/640.0f;
	pos.y = 232.0f/480.0f;
	dim.x = 130.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_RENAMEOLD, text);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_NULL, text);
	pl->SetJustif(-1);

	strcpy(name, m_listBuffer[m_list].filename);

	pos.x = 212.0f/640.0f;
	pos.y = 239.0f/480.0f;
	dim.x = 217.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pe = pw->CreateEdit(pos, dim, -1, EVENT_DIALOG_EDIT1);
	pe->SetState(STATE_SHADOW);
	pe->SetMaxChar(MAXFILENAME);
	pe->SetText(name);
	pe->SetEditCap(FALSE);
	pe->SetHiliteCap(FALSE);

	pos.x =  70.0f/640.0f;
	pos.y = 202.0f/480.0f;
	dim.x = 130.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_RENAMENEW, text);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_NULL, text);
	pl->SetJustif(-1);

	pos.x = 212.0f/640.0f;
	pos.y = 209.0f/480.0f;
	dim.x = 217.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pe = pw->CreateEdit(pos, dim, -1, EVENT_DIALOG_EDIT2);
	pe->SetState(STATE_SHADOW);
	pe->SetMaxChar(MAXFILENAME);
	pe->SetFilenameCap(TRUE);
	pe->SetText(name);
	pe->SetCursor(strlen(name), 0);
	pe->SetFocus(TRUE);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESRENAME, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NORENAME, name);
	pb->SetName(name);
}

// Exportation d'un puzzle.

void CMainDialog::StartExportPuzzle()
{
	CWindow*	pw;
	CLabel*		pl;
	CButton*	pb;
	CCheck*		pc;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.7f, 0.4f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_EXPORTPUZZLE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x =  96.0f/640.0f;
	pos.y = 256.0f/480.0f;
	dim.x = 448.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_EXPORTGAME, name);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
	pl->SetFontSize(12.0f);

	pos.x = 212.0f/640.0f;
	pos.y = 234.0f/480.0f;
	dim.x = 300.0f/640.0f;
	dim.y =  16.0f/480.0f;
	pc = pw->CreateCheck(pos, dim, -1, EVENT_INTERFACE_EXPORTdefi);
	pc->SetState(STATE_SHADOW);
	pc->SetState(STATE_CHECK, (m_exportType==0));

	pos.y -= 20.0f/480.0f;
	pc = pw->CreateCheck(pos, dim, -1, EVENT_INTERFACE_EXPORTdoc);
	pc->SetState(STATE_SHADOW);
	pc->SetState(STATE_CHECK, (m_exportType==1));

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESEXPORT, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NOEXPORT, name);
	pb->SetName(name);
}

// Met à jour le type d'exportation.

void CMainDialog::UpdateExportType()
{
	CWindow*	pw;
	CCheck*		pc;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EXPORTdefi);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, (m_exportType==0));
	}

	pc = (CCheck*)pw->SearchControl(EVENT_INTERFACE_EXPORTdoc);
	if ( pc != 0 )
	{
		pc->SetState(STATE_CHECK, (m_exportType==1));
	}
}

// Importation d'un puzzle.

void CMainDialog::StartImportPuzzle()
{
	CWindow*	pw;
	CLabel*		pl;
	CButton*	pb;
	CArray*		pa;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.8f, 0.8f), FALSE, TRUE, TRUE);
	m_dialogType = DIALOG_IMPORTPUZZLE;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x =  98.0f/640.0f;
	pos.y = 358.0f/480.0f;
	dim.x = 450.0f/640.0f;
	dim.y =  20.0f/480.0f;
	GetResource(RES_TEXT, RT_DIALOG_IMPORTGAME, name);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
	pl->SetJustif(1);

	pos.x =  94.0f/640.0f;
	pos.y = 110.0f/480.0f;
	dim.x = 450.0f/640.0f;
	dim.y = 250.0f/480.0f;
	pa = pw->CreateArray(pos, dim, 0, EVENT_INTERFACE_LIST);
	pa->SetState(STATE_DEFAULT);
	pa->SetState(STATE_SHADOW);
	pa->SetTabs(0, 110.0f/640.0f, 1, FONT_COLOBOT);  // filename
	pa->SetTabs(1,  70.0f/640.0f, 1, FONT_COLOBOT);  // univers
	pa->SetTabs(2,  70.0f/640.0f, 1, FONT_COLOBOT);  // auteur
	pa->SetTabs(3, 180.0f/640.0f, 1, FONT_COLOBOT);  // resumé
	pa->SetSelectCap(TRUE);
	pa->SetFontSize(9.0f);
	UpdateImportPuzzle(pa);
	
	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESIMPORT, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_CANCEL);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_NOIMPORT, name);
	pb->SetName(name);
}

// Met à jour la liste des puzzles à importer.

void CMainDialog::UpdateImportPuzzle(CArray *pa)
{
	LPITEMIDLIST ppidl;
	long		hFile;
	struct _finddata_t fileBuffer;
	char		dir[200];
	char		name[200];
	char		filename[100];
	char		univers[100];
	char		resume[1000];
	char		author[100];
	int			i, environment;
	BOOL		bSolved;

	pa->Flush();

	SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &ppidl);
	SHGetPathFromIDList(ppidl, dir);
	#if _EGAMES
	strcat(dir, "\\SpeedyEggbertMania\\");
	#else
	strcat(dir, "\\BlupiMania2\\");
	#endif

	strcpy(name, dir);
	strcat(name, "*.bm2");

	i = 0;
	hFile = _findfirst(name, &fileBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( (fileBuffer.attrib & _A_SUBDIR) == 0 )
			{
				strcpy(filename, dir);
				strcat(filename, fileBuffer.name);
				fnmcpy(m_importBuffer[i].filename, fileBuffer.name);
				i ++;
			}
		}
		while ( _findnext(hFile, &fileBuffer) == 0 && i < LISTMAX );
	}
	m_importTotal = i;

	GetResource(RES_TEXT, RT_IMPORT_HEADER, name);
	pa->SetName(-1, name);  // texte de la légende

	for ( i=0 ; i<m_importTotal ; i++ )
	{
		sprintf(filename, "%s%s.bm2", dir, m_importBuffer[i].filename);
		ReadScene(filename, univers, resume, author, bSolved, environment);
		m_importBuffer[i].bSolved = bSolved;

		strcpy(name, m_importBuffer[i].filename);

		strcat(name, "\t");
		strcat(name, univers);

		strcat(name, "\t");
		strcat(name, author);

		strcat(name, "\t");
		strncat(name, resume, 100);

		pa->SetName(i, name);
		pa->SetEnable(i, bSolved);
		pa->SetIndex(i, i);
		pa->SetSortValue(i, environment);
	}

	m_importSelect = -1;
	pa->SetSelect(-1);
	pa->Sort();
	pa->ShowSelect();  // montre
}

// Affiche une erreur.

void CMainDialog::StartError(int err)
{
	CWindow*	pw;
	CButton*	pb;
	CLabel*		pl;
	FPOINT		pos, dim;
	char		name[100];

	StartDialog(FPOINT(0.8f, 0.3f), FALSE, TRUE, FALSE);
	m_dialogType = DIALOG_ERROR;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw == 0 )  return;

	pos.x = 0.00f;
	pos.y = 0.48f;
	dim.x = 1.00f;
	dim.y = 0.05f;
	GetResource(RES_ERR, err, name);
	pl = pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);
	pl->SetFontSize(12.0f);

	pb = (CButton*)pw->SearchControl(EVENT_DIALOG_OK);
	if ( pb == 0 )  return;
	GetResource(RES_TEXT, RT_DIALOG_YESERROR, name);
	pb->SetName(name);
	pb->SetState(STATE_WARNING);
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
	pw->SetName(" ");

	m_dialogPos = pos;
	m_dialogDim = dim;
	m_dialogTime = 0.0f;
	m_dialogParti = 999.0f;

	if ( bOK )
	{
		if ( bCancel )  pos.x  = 0.50f-0.15f-0.02f;
		else            pos.x  = 0.50f-0.15f/2.0f;
		pos.y  = 0.50f-dim.y/2.0f+0.04f;
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
		pos.y  = 0.50f-dim.y/2.0f+0.04f;
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

	m_dialogParti += rTime;
	if ( m_dialogParti < 0.05f )  return;
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
	CArray*		pa;
	char		text[1000];

	m_dialogName[0] = 0;
	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW9);
	if ( pw != 0 )
	{
		pe = (CEdit*)pw->SearchControl(EVENT_INTERFACE_NEDIT);
		if ( pe != 0 )
		{
			pe->GetText(m_dialogName, 100);
		}

		if ( m_dialogType == DIALOG_INFOPUZZLE )
		{
			pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT1);
			if ( pe != 0 )
			{
				pe->GetText(text, 1000);
				m_main->SetResume(RetLanguageLetter(), text);
			}

			pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT2);
			if ( pe != 0 )
			{
				pe->GetText(text, 1000);
				m_main->SetSign(RetLanguageLetter(), text);
			}

			pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT3);
			if ( pe != 0 )
			{
				pe->GetText(text, 100);
				m_main->SetAuthor(text);
			}
		}

		if ( m_dialogType == DIALOG_NEWPUZZLE )
		{
			pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT1);
			if ( pe != 0 )
			{
				pe->GetText(m_newPuzzleFilename, 100);
			}
		}

		if ( m_dialogType == DIALOG_RENAMEPUZZLE )
		{
			pe = (CEdit*)pw->SearchControl(EVENT_DIALOG_EDIT2);
			if ( pe != 0 )
			{
				pe->GetText(m_newPuzzleFilename, 100);
			}
		}

		if ( m_dialogType == DIALOG_IMPORTPUZZLE )
		{
			pa = (CArray*)pw->SearchControl(EVENT_INTERFACE_LIST);
			if ( pa != 0 )
			{
				m_importSelect = pa->RetIndex(pa->RetSelect());
			}
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
//?	m_sound->MuteAll(TRUE);
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
//?	m_sound->MuteAll(FALSE);
	m_main->ClearInterface();
	if ( !m_bInitPause )  m_engine->SetPause(FALSE);
	m_engine->SetOverFront(TRUE);  // over plane devant
	m_main->StopSuspend();
	m_camera->SetType(m_initCamera);
}


// Indique si un dialogue est affiché.

BOOL CMainDialog::IsDialog()
{
	return m_bDialog;
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


// Indique si l'on édite un puzzle.

BOOL CMainDialog::RetEdit()
{
	return m_bEdit;
}

// Indique si l'on teste un puzzle.

BOOL CMainDialog::RetTest()
{
	return m_bTest;
}

// Indique si l'on édite un puzzle.

BOOL CMainDialog::RetProto()
{
	return m_bProto;
}

// Indique si l'on a utiliser le bouton "Recommencer" pour jouer.

BOOL CMainDialog::RetAgain()
{
	return m_bAgain;
}

void CMainDialog::SetAgain(BOOL bAgain)
{
	m_bAgain = bAgain;
}


// Lit le fichier des missions.

BOOL CMainDialog::ReadGamerMission()
{
	char	filename[100];
	int		i;

	sprintf(filename, "%s\\%s\\%s.gam", m_savegameDir, m_main->RetGamerName(), "info");
	if ( !m_gamerFile->Read(filename) )  return FALSE;

	for ( i=0 ; i<INDEX_MAX ; i++ )
	{
		m_gamerFile->RetSelect(i, m_selectFilename[i]);
	}

	return TRUE;
}

// Ecrit le fichier des missions.

BOOL CMainDialog::WriteGamerMission()
{
	int		i;

	for ( i=0 ; i<INDEX_MAX ; i++ )
	{
		m_gamerFile->SetSelect(i, m_selectFilename[i]);
	}

	return m_gamerFile->Write();
}


// Indique une tentative de plus.

void CMainDialog::IncGamerInfoTry()
{
	int		numTry;
	char	puzzle[MAXFILENAME+2];

	if ( strcmp(m_sceneName, "puzzle") == 0 )
	{
		KeyPuzzle(puzzle, m_sel);
		numTry = m_gamerFile->RetNumTry(puzzle);
		if ( numTry < 100 )
		{
			m_gamerFile->SetNumTry(puzzle, numTry+1);
		}
	}

	if ( strcmp(m_sceneName, "defi") == 0 )
	{
		strcpy(puzzle, m_listBuffer[m_list].filename);
		numTry = m_gamerFile->RetNumTry(puzzle);
		if ( numTry < 100 )
		{
			m_gamerFile->SetNumTry(puzzle, numTry+1);
		}
	}
}

// Mémorise le temps joué.

void CMainDialog::SetGamerTotalTime(float time)
{
	char	puzzle[MAXFILENAME+2];

	if ( strcmp(m_sceneName, "puzzle") == 0 )
	{
		KeyPuzzle(puzzle, m_sel);
		m_gamerFile->SetTotalTime(puzzle, time);
	}

	if ( strcmp(m_sceneName, "defi") == 0 )
	{
		strcpy(puzzle, m_listBuffer[m_list].filename);
		m_gamerFile->SetTotalTime(puzzle, time);
	}
}

// Retourne le temps total joué.

float CMainDialog::RetGamerTotalTime()
{
	char	puzzle[MAXFILENAME+2];

	if ( strcmp(m_sceneName, "puzzle") == 0 )
	{
		KeyPuzzle(puzzle, m_sel);
		return m_gamerFile->RetTotalTime(puzzle);
	}

	if ( strcmp(m_sceneName, "defi") == 0 )
	{
		strcpy(puzzle, m_listBuffer[m_list].filename);
		return m_gamerFile->RetTotalTime(puzzle);
	}

	return 0.0f;
}

// Signal la mission comme réussie.

void CMainDialog::SetGamerInfoPassed()
{
	char	puzzle[MAXFILENAME+2];

	if ( strcmp(m_sceneName, "puzzle") == 0 )
	{
		KeyPuzzle(puzzle, m_sel);
		m_gamerFile->SetPassed(puzzle, TRUE);
	}

	if ( strcmp(m_sceneName, "defi") == 0 )
	{
		if ( !m_main->RetCheatUsed() )
		{
			strcpy(puzzle, m_listBuffer[m_list].filename);
			m_gamerFile->SetPassed(puzzle, TRUE);
		}
	}

	if ( strcmp(m_sceneName, "user") == 0 )
	{
		if ( !m_main->RetCheatUsed() )
		{
			SolvedPuzzle(m_list, TRUE, m_main->RetTotalManip());
		}
	}
}

// Indique si la mission est réussie.

BOOL CMainDialog::RetGamerInfoPassed(int rank)
{
	char	puzzle[MAXFILENAME+2];

	if ( strcmp(m_sceneName, "puzzle") == 0 )
	{
		KeyPuzzle(puzzle, rank);
		return m_gamerFile->RetPassed(puzzle);
	}

	if ( strcmp(m_sceneName, "defi") == 0 )
	{
		strcpy(puzzle, m_listBuffer[m_list].filename);
		return m_gamerFile->RetPassed(puzzle);
	}

	return FALSE;
}


// Cherche la mission suivante par défaut à effectuer.

void CMainDialog::NextMission()
{
	int		max, first, last, i;

	if ( strcmp(m_sceneName, "puzzle") != 0 )  return;

	m_bMoveAnimation = TRUE;

#if _EGAMES
	max = 80-1;
#if _DEMO
	max = 8-1;
#endif
#if _SE
	max = 16-1;
#endif
#else
#if _DEMO
	max = 16-1;
#else
	max = 80-1;
#endif
#endif

	// Suivante pas encore faite ?
	last = (m_sel/16)*16 + (16-1);
	if ( m_sel < max &&
		 m_sel < last &&
		 !RetGamerInfoPassed(m_sel+1) )
	{
		m_sel ++;
		KeyPuzzle(m_selectFilename[m_index], m_sel);
		return;
	}

	// Cherche dans groupe de 4 une pas faite.
	first = (m_sel/4)*4;
	for ( i=first ; i<first+4 ; i++ )
	{
		if ( !RetGamerInfoPassed(i) )
		{
			m_sel = i;
			KeyPuzzle(m_selectFilename[m_index], m_sel);
			return;
		}
	}

	// Cherche dans tout l'étage de 16 une pas faite.
	first = (m_sel/16)*16;
	for ( i=first ; i<first+16 ; i++ )
	{
		if ( i > max )  break;
		if ( !RetGamerInfoPassed(i) )
		{
			m_sel = i;
			KeyPuzzle(m_selectFilename[m_index], m_sel);
			return;
		}
	}

#if !_DEMO & !_SE
	if ( first >= max-16+1 )  // tout fini !
	{
		if ( m_sel < max )
		{
			m_sel ++;  // suivante
		}
		KeyPuzzle(m_selectFilename[m_index], m_sel);
		return;
	}

	// Cherche dans l'étage suivant une pas faite.
	first = (m_sel/16)*16+16;
	for ( i=first ; i<first+16 ; i++ )
	{
		if ( !RetGamerInfoPassed(i) )
		{
			m_sel = i;
			KeyPuzzle(m_selectFilename[m_index], m_sel);
			return;
		}
	}
#endif

	// Suivante, bof !
	if ( m_sel < max )
	{
		m_sel ++;
		KeyPuzzle(m_selectFilename[m_index], m_sel);
		return;
	}
}

// Indique s'il faut écrire le fichier.

BOOL CMainDialog::RetWriteFile()
{
	return m_bWriteFile;
}


// Crée un nouveau puzzle sur la base d'un fichier modèle.

BOOL CMainDialog::CreateNewPuzzle(int environment, char *filename)
{
	CCryptFile	fileSrc;
	CCryptFile	fileDst;
	FILE*		file;
	char		filenameSrc[200];
	char		filenameDst[200];
	char		line[1000];

	sprintf(filenameSrc, "%s\\model%.3d.bm2", m_sceneDir, environment);
	if ( !fileSrc.Open(filenameSrc, "r") )  return FALSE;

	sprintf(filenameDst, "%s\\%s\\%s.bm2", m_savegameDir, m_main->RetGamerName(), filename);
	file = fopen(filenameDst, "r");
	if ( file != 0 )
	{
		fclose(file);
		fileSrc.Close();
		StartError(ERR_CREATE);
		return FALSE;
	}
	if ( !fileDst.Open(filenameDst, "w") )
	{
		fileSrc.Close();
		return FALSE;
	}

	// Copie le fichier.
	while ( fileSrc.GetLine(line, 1000) )
	{
		if ( Cmd(line, "Environment") )
		{
			fileDst.PutLine(line);

			sprintf(line, "Author name=\"%s\"\n", m_main->RetGamerName());
			fileDst.PutLine(line);
		}
		else if ( Cmd(line, "Author") )
		{
		}
		else
		{
			fileDst.PutLine(line);
		}
	}
	fileSrc.Close();
	fileDst.Close();

	strcpy(m_selectFilename[m_index], filename);
	return TRUE;
}

// Supprime le puzzle sélectionné dans la liste.

BOOL CMainDialog::DeletePuzzle(int i)
{
	char	filename[200];

	if ( m_phase == PHASE_DEFI )
	{
		sprintf(filename, "%s\\%s.bm2",
								m_defiDir, m_listBuffer[i].filename);
	}
	if ( m_phase == PHASE_USER )
	{
		sprintf(filename, "%s\\%s\\%s.bm2",
								m_savegameDir, m_main->RetGamerName(),
								m_listBuffer[i].filename);
	}
	remove(filename);

	if ( m_phase == PHASE_DEFI )
	{
		DeleteGamerFile(m_listBuffer[i].filename);
	}

	m_selectFilename[m_index][0] = 0;
	return TRUE;
}

// Supprime un fichier des défis pour tous les joueurs.

void CMainDialog::DeleteGamerFile(char *puzzle)
{
	long				hFile;
	struct _finddata_t	fBuffer;
	char				dir[_MAX_FNAME];
	char				filename[_MAX_FNAME];

	WriteGamerMission();

	sprintf(dir, "%s\\*", m_savegameDir);
	hFile = _findfirst(dir, &fBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( (fBuffer.attrib & _A_SUBDIR) && fBuffer.name[0] != '.' )
			{
				sprintf(filename, "%s\\%s\\%s.gam", m_savegameDir, fBuffer.name, "info");
				if ( m_gamerFile->Read(filename) )
				{
					m_gamerFile->Delete(puzzle);
					m_gamerFile->Write();
				}
			}
		}
		while ( _findnext(hFile, &fBuffer) == 0 );
	}
	_findclose(hFile);

	ReadGamerMission();
}

// Renomme le puzzle sélectionné dans la liste.

BOOL CMainDialog::RenamePuzzle(int i, char *newName)
{
	char	filenameOld[200];
	char	filenameNew[200];

	sprintf(filenameOld, "%s\\%s\\%s.bm2",
							m_savegameDir, m_main->RetGamerName(),
							m_listBuffer[i].filename);
	sprintf(filenameNew, "%s\\%s\\%s.bm2",
							m_savegameDir, m_main->RetGamerName(),
							newName);

	rename(filenameOld, filenameNew);
	strcpy(m_selectFilename[m_index], newName);
	return TRUE;
}

// Exporte le puzzle sélectionné dans la liste.

BOOL CMainDialog::ExportPuzzle(int i)
{
	LPITEMIDLIST ppidl;
	char		filenameSrc[200];
	char		filenameDst[MAX_PATH];

	sprintf(filenameSrc, "%s\\%s\\%s.bm2",
								m_savegameDir, m_main->RetGamerName(),
								m_listBuffer[i].filename);

	if ( m_exportType == 0 )  // exporte dans les défis ?
	{
		sprintf(filenameDst, "%s\\", m_defiDir);
	}
	if ( m_exportType == 1 )  // exporte dans "Mes documents" ?
	{
		SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &ppidl);
		SHGetPathFromIDList(ppidl, filenameDst);
		#if _EGAMES
		strcat(filenameDst, "\\SpeedyEggbertMania");
		#else
		strcat(filenameDst, "\\BlupiMania2");
		#endif
		CreateDirectory(filenameDst, NULL);
		strcat(filenameDst, "\\");
	}
	strcat(filenameDst, m_listBuffer[i].filename);
	strcat(filenameDst, ".bm2");

	if ( !CopyFile(filenameSrc, filenameDst) )  return FALSE;

	if ( m_exportType == 0 )  // exporte dans les défis ?
	{
		DeleteGamerFile(m_listBuffer[i].filename);
	}

	strcpy(m_selectFilename[INDEX_DEFI], m_listBuffer[i].filename);
	return TRUE;
}

// Importe un puzzle.

BOOL CMainDialog::ImportPuzzle(int i)
{
	LPITEMIDLIST ppidl;
	char		filenameSrc[MAX_PATH];
	char		filenameDst[200];

	if ( i == -1 )  return FALSE;

	SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &ppidl);
	SHGetPathFromIDList(ppidl, filenameSrc);
	#if _EGAMES
	strcat(filenameSrc, "\\SpeedyEggbertMania\\");
	#else
	strcat(filenameSrc, "\\BlupiMania2\\");
	#endif
	strcat(filenameSrc, m_importBuffer[i].filename);
	strcat(filenameSrc, ".bm2");

	sprintf(filenameDst, "%s\\%s.bm2",
								m_defiDir, m_importBuffer[i].filename);

	if ( !CopyFile(filenameSrc, filenameDst) )  return FALSE;

	DeleteGamerFile(m_listBuffer[i].filename);

	strcpy(m_selectFilename[m_index], m_importBuffer[i].filename);
	return TRUE;
}

// Cherche un nom inutilisé.

BOOL CMainDialog::SearchNewName(char *dir, char *base,
								char *filename, char *quick)
{
	FILE*		file;
	int			i;

	for ( i=0 ; i<1000 ; i++ )
	{
		sprintf(quick, "%s%.3d", base, i);
		sprintf(filename, "%s\\%s.bm2", dir, quick);
		file = fopen(filename, "r");
		if ( file == 0 )  break;
		fclose(file);
	}
	return ( i < 1000 );
}

// Copie un fichier.

BOOL CMainDialog::CopyFile(char *filenameSrc, char *filenameDst)
{
	CCryptFile	fileSrc;
	CCryptFile	fileDst;
	char		line[1000];

	if ( !fileSrc.Open(filenameSrc, "r") )  return FALSE;

	remove(filenameDst);
	if ( !fileDst.Open(filenameDst, "w") )
	{
		fileSrc.Close();
		return FALSE;
	}

	// Copie le fichier.
	while ( fileSrc.GetLine(line, 1000) )
	{
		fileDst.PutLine(line);
	}
	fileSrc.Close();
	fileDst.Close();

	return TRUE;
}

// Ajoute dans un puzzle la commande indiquant s'il a été solutionné.

BOOL CMainDialog::SolvedPuzzle(int i, BOOL bSolved, int totalManip)
{
	CCryptFile	fileSrc;
	CCryptFile	fileDst;
	char		filenameSrc[200];
	char		filenameDst[200];
	char		line[1000];

	if ( strcmp(m_sceneName, "user") != 0 )  return TRUE;

	sprintf(filenameSrc, "%s\\%s\\%s.bm2",
							m_savegameDir, m_main->RetGamerName(),
							m_listBuffer[i].filename);
	if ( !fileSrc.Open(filenameSrc, "r") )
	{
		return FALSE;
	}

	strcpy(filenameDst, filenameSrc);
	strcpy(filenameDst+strlen(filenameDst)-4, ".tmp");
	remove(filenameDst);
	if ( !fileDst.Open(filenameDst, "w") )
	{
		fileSrc.Close();
		return FALSE;
	}

	// Copie le fichier.
	while ( fileSrc.GetLine(line, 1000) )
	{
		if ( Cmd(line, "Environment") )
		{
			fileDst.PutLine(line);

			sprintf(line, "Solved state=%d manip=%d\n", bSolved, totalManip);
			fileDst.PutLine(line);
		}
		else if ( Cmd(line, "Solved") )
		{
		}
		else
		{
			fileDst.PutLine(line);
		}
	}
	fileSrc.Close();
	fileDst.Close();

	remove(filenameSrc);
	rename(filenameDst, filenameSrc);

	return TRUE;
}


// Changement de joueur.

void CMainDialog::GamerChanged()
{
	int		i;

	m_sel = 0;
	m_list = 0;

	for ( i=0 ; i<INDEX_MAX ; i++ )
	{
		m_selectFilename[i][0] = 0;
	}
}


