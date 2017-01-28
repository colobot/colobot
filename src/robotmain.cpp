// robotmain.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <d3d.h>

#include "cbot/cbotdll.h"
#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "cryptfile.h"
#include "language.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "filebuffer.h"
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "cloud.h"
#include "blitz.h"
#include "planet.h"
#include "object.h"
#include "motion.h"
#include "pyro.h"
#include "modfile.h"
#include "model.h"
#include "camera.h"
#include "tasklist.h"
#include "task.h"
#include "auto.h"
#include "text.h"
#include "interface.h"
#include "map.h"
#include "label.h"
#include "button.h"
#include "gauge.h"
#include "group.h"
#include "window.h"
#include "edit.h"
#include "array.h"
#include "sound.h"
#include "displaytext.h"
#include "cbottoken.h"
#include "cmdtoken.h"
#include "maindialog.h"
#include "mainedit.h"
#include "mainundo.h"
#include "mainmap.h"
#include "script.h"
#include "robotmain.h"



#define CBOT_STACK	TRUE	// enregistre le stack des programmes CBOT



// Variables globales.

long	g_id;				// identificateur unique
long	g_build;			// bâtiments constructibles
long	g_researchDone;		// recherches effectuées
long	g_researchEnable;	// recherches accessibles



#include "classfile.cpp"




// Compilation de la classe "point".

CBotTypResult cPoint(CBotVar* pThis, CBotVar* &var)
{
	if ( !pThis->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadNum);

	if ( var == NULL )  return CBotTypResult(0);  // ok si aucun paramètre

	// Premier paramètre (x) :
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	// Deuxième paramètre (y) :
	if ( var == NULL )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	// Troisième paramètre (z) :
	if ( var == NULL )  // seulement 2 paramètres ?
	{
		return CBotTypResult(0);  // cette fonction retourne void
	}

	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != NULL )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(0);  // cette fonction retourne void
}

// Exécution de la classe "point".

BOOL rPoint(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception)
{
	CBotVar		*pX, *pY, *pZ;

	if ( var == NULL )  return TRUE;  // constructeur sans paramètres est ok

	if ( var->GivType() > CBotTypDouble )
	{
		Exception = CBotErrBadNum;  return FALSE;
	}

	pX = pThis->GivItem("x");
	if ( pX == NULL )
	{
		Exception = CBotErrUndefItem;  return FALSE;
	}
	pX->SetValFloat( var->GivValFloat() );
	var = var->GivNext();

	if ( var == NULL ) 
	{
		Exception = CBotErrLowParam;  return FALSE;
	}

	if ( var->GivType() > CBotTypDouble )
	{
		Exception = CBotErrBadNum;  return FALSE;
	}

	pY = pThis->GivItem("y");
	if ( pY == NULL )
	{
		Exception = CBotErrUndefItem;  return FALSE;
	}
	pY->SetValFloat( var->GivValFloat() );
	var = var->GivNext();

	if ( var == NULL )
	{
		return TRUE;  // ok avec seulement 2 paramètres
	}

	pZ = pThis->GivItem("z");
	if ( pZ == NULL )
	{
		Exception = CBotErrUndefItem;  return FALSE;
	}
	pZ->SetValFloat( var->GivValFloat() );
	var = var->GivNext();

	if ( var != NULL )
	{
		Exception = CBotErrOverParam;  return FALSE;
	}

	return	TRUE;  // pas d'interruption
}




// Constructeur de l'application robot.

CRobotMain::CRobotMain(CInstanceManager* iMan)
{
	ObjectType	type;
	float		fValue;
	int			i;
	char*		token;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_MAIN, this);

	m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_cloud     = (CCloud*)m_iMan->SearchInstance(CLASS_CLOUD);
	m_blitz     = (CBlitz*)m_iMan->SearchInstance(CLASS_BLITZ);
	m_planet    = (CPlanet*)m_iMan->SearchInstance(CLASS_PLANET);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_interface   = new CInterface(m_iMan);
	m_terrain     = new CTerrain(m_iMan);
	m_model       = new CModel(m_iMan);
	m_camera      = new CCamera(m_iMan);
	m_displayText = new CDisplayText(m_iMan);
	m_dialog      = new CMainDialog(m_iMan);
	m_edit        = new CMainEdit(m_iMan);
	m_undo        = new CMainUndo(m_iMan);
	m_map         = new CMainMap(m_iMan);
	m_fileBuffer  = new CFileBuffer(m_iMan);

	m_engine->SetTerrain(m_terrain);
	m_filesDir = m_dialog->RetFilesDir();

	m_time = 0.0f;
	m_gameTime = 0.0f;
	m_checkEndTime = -10.0f;

	m_phase       = PHASE_NAME;
	m_lastPhase   = PHASE_NULL;
	m_audioTrack  = 0;
	m_bAudioRepeat = TRUE;
	m_selectObject = 0;
	m_defMouse = D3DMOUSENORM;

	m_bMovieLock    = FALSE;
	m_bMovieFinal   = FALSE;
	m_bPause        = FALSE;
	m_bHilite       = FALSE;
	m_bSelect       = FALSE;
	m_bScroll       = FALSE;
	m_bFreePhoto    = FALSE;
	m_bShowPos      = FALSE;
	m_bShowAll      = FALSE;
	m_scene         = SCENE_NULL;
	m_handleMode    = 1;
	m_adviseMode    = 0;
	m_bSuspend      = FALSE;
	m_bFriendAim    = FALSE;
	m_bSkipFrame    = FALSE;
	m_bDisplayInfo  = FALSE;
	m_bCheatUsed    = FALSE;
	m_firstInfoDelay = 0.0f;
	m_additionalTotal = 0;
	m_undoCounter   = 0;
	m_stopCounter   = 0;
	m_typeButton    = 0;
	m_totalBlupi    = 0;
	m_totalGoal     = 0;
	m_totalManip    = 0;

	m_engine->SetMovieLock(m_bMovieLock);

	m_tooltipPos = FPOINT(0.0f, 0.0f);
	m_tooltipName[0] = 0;
	m_tooltipTime = 0.0f;

	m_endingWinRank   = 0;
	m_endingLostRank  = 0;

	m_fontSize  = 9.0f;
	m_windowPos = FPOINT(0.15f, 0.17f);
	m_windowDim = FPOINT(0.70f, 0.66f);

	m_terrainSpritePos = D3DVECTOR(NAN, NAN, NAN);
	m_terrainSpriteChannel = -1;

	ZeroMemory(m_superShift, sizeof(BYTE)*256);
	m_superShift19 = 0;
	m_lastSuperShift19 = 0;

	if ( GetProfileFloat("Edit", "FontSize",    fValue) )  m_fontSize    = fValue;
	if ( GetProfileFloat("Edit", "WindowPos.x", fValue) )  m_windowPos.x = fValue;
	if ( GetProfileFloat("Edit", "WindowPos.y", fValue) )  m_windowPos.y = fValue;
	if ( GetProfileFloat("Edit", "WindowDim.x", fValue) )  m_windowDim.x = fValue;
	if ( GetProfileFloat("Edit", "WindowDim.y", fValue) )  m_windowDim.y = fValue;
	
	InitEye();

	g_id = 0;
	g_build = 0;
	g_researchDone = 0;  // aucune recherche effectuée
	g_researchEnable = 0;

	m_repeat = 0;
	m_lastFilename[0] = 0;

#if _DEMO | _SE
	strcpy(m_gamerName, "demo");
#else
	m_gamerName[0] = 0;
	GetProfileString("Gamer", "LastName", m_gamerName, 100);
#endif
	SetGlobalGamerName(m_gamerName);
	m_dialog->SetupRecall();

	CBotProgram::SetTimer(100);
	CBotProgram::Init();

	for ( i=0 ; i<OBJECT_MAX ; i++ )
	{
		type = (ObjectType)i;
		token = RetObjectName(type);
		if ( token[0] != 0 )
		{
			CBotProgram::DefineNum(token, type);
		}
	}

	CBotProgram::DefineNum("DisplayError",   TT_ERROR);
	CBotProgram::DefineNum("DisplayWarning", TT_WARNING);
	CBotProgram::DefineNum("DisplayInfo",    TT_INFO);
	CBotProgram::DefineNum("DisplayMessage", TT_MESSAGE);

	CBotProgram::DefineNum("FilterNone",        FILTER_NONE);
	CBotProgram::DefineNum("FilterOnlyLanding", FILTER_ONLYLANDING);
	CBotProgram::DefineNum("FilterOnlyFliying", FILTER_ONLYFLYING);

	// Ajoute la classe Point.
	CBotClass* bc;
	bc = new CBotClass("point", NULL, TRUE);  // classe intrinsèque
	bc->AddItem("x", CBotTypFloat);
	bc->AddItem("y", CBotTypFloat);
	bc->AddItem("z", CBotTypFloat);
	bc->AddFunction("point", rPoint, cPoint);

	// Ajoute la classe Object.
	bc = new CBotClass("object", NULL);
	bc->AddItem("category",    CBotTypResult(CBotTypInt), PR_READ);
	bc->AddItem("position",    CBotTypResult(CBotTypClass, "point"), PR_READ);
	bc->AddItem("orientation", CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("pitch",       CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("roll",        CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("energyLevel", CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("shieldLevel", CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("temperature", CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("altitude",    CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("lifeTime",    CBotTypResult(CBotTypFloat), PR_READ);
	bc->AddItem("energyCell",  CBotTypResult(CBotTypPointer, "object"), PR_READ);
	bc->AddItem("load",        CBotTypResult(CBotTypPointer, "object"), PR_READ);

	// Initialise la classe FILE.
	InitClassFILE();

	CScript::InitFonctions();
}

// Destructeur de l'application robot.

CRobotMain::~CRobotMain()
{
	delete m_dialog;
	delete m_edit;
	delete m_undo;
	delete m_map;
	delete m_terrain;
	delete m_model;
}


// Crée le fichier blupimania.ini la première fois.

void CRobotMain::CreateIni()
{
	int		iValue;

	// blupimania.ini inexistant ?
	if ( !GetProfileInt("Setup", "Shadow", iValue) )
	{
		m_dialog->SetupMemorize();
	}
}


// Change de phase.

void CRobotMain::ChangePhase(Phase phase, Phase fadeIn)
{
	CEdit*			pe;
	CButton*		pb;
	CGroup*			pgr;
	D3DCOLORVALUE	color;
	FPOINT			pos, dim, ddim;
	float			ox, oy, sx, sy;
	Scene			oldScene;
	BOOL			bCreate;

	m_sound->SetWater(FALSE);  // en dehors de l'eau

	if ( phase == PHASE_FADEIN )
	{
		m_lastPhase = m_phase;
		m_phase = phase;
		m_dialog->ChangePhase(phase, fadeIn);
		return;
	}

	bCreate = TRUE;
	if ( phase == PHASE_SIMUL )
	{
		if ( m_phase == PHASE_SETUPds ||
			 m_phase == PHASE_SETUPgs ||
			 m_phase == PHASE_SETUPps ||
			 m_phase == PHASE_SETUPcs ||
			 m_phase == PHASE_SETUPss )
		{
			bCreate = FALSE;
		}
	}

	oldScene = m_scene;
	m_scene = SCENE_NULL;
	if ( phase == PHASE_NAME   ||
		 phase == PHASE_INIT   ||
		 phase == PHASE_PUZZLE ||
		 phase == PHASE_DEFI   ||
		 phase == PHASE_USER   ||
		 phase == PHASE_PROTO  ||
		 phase == PHASE_SETUPd ||
		 phase == PHASE_SETUPg ||
		 phase == PHASE_SETUPp ||
		 phase == PHASE_SETUPc ||
		 phase == PHASE_SETUPs ||
		 phase == PHASE_TERM   ||
		 phase == PHASE_FADEIN )
	{
		m_scene = SCENE_FIX;
	}
	if ( phase == PHASE_SIMUL   ||
		 phase == PHASE_MODELe  ||
		 phase == PHASE_MODELi  ||
		 phase == PHASE_SETUPds ||
		 phase == PHASE_SETUPgs ||
		 phase == PHASE_SETUPps ||
		 phase == PHASE_SETUPcs ||
		 phase == PHASE_SETUPss ||
		 phase == PHASE_WIN     ||
		 phase == PHASE_LOST    )
	{
		m_scene = SCENE_SIMUL;
	}

	if ( phase == PHASE_LOST )
	{
		m_phase = PHASE_LOST;

		DeselectAll();  // désélectionne la voiture
//?		m_camera->StartVisit(m_engine->RetLookatPt(), 200.0f);
		m_camera->StartVisit(D3DVECTOR(0.0f, 0.0f, 0.0f), 200.0f);
		m_particule->SetSunbeamIntensity(0.0f);

		m_interface->DeleteControl(EVENT_BUTTON_QUIT);
		m_interface->DeleteControl(EVENT_INTERFACE_UNDO);
		m_interface->DeleteControl(EVENT_INTERFACE_SIGN);

		pos.x  = 520.0f/640.0f;
		pos.y  =  62.0f/480.0f;
		ddim.x = 110.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = m_interface->CreateButton(pos, ddim, -1, EVENT_INTERFACE_AGAIN);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(0);
		pb->SetFocus(TRUE);

		pos.x  = 520.0f/640.0f;
		pos.y  =  14.0f/480.0f;
		ddim.x = 110.0f/640.0f;
		ddim.y =  32.0f/480.0f;
		pb = m_interface->CreateButton(pos, ddim, -1, EVENT_INTERFACE_TERM);
		pb->SetFontType(FONT_HILITE);
		pb->SetState(STATE_SHADOW);
		pb->SetTabOrder(1);

		return;  // conserve la scène en cours !
	}

	if ( m_lastPhase == PHASE_SIMUL )  // termine une simulation ?
	{
		m_lastPhase = PHASE_NULL;

		if ( RetEdit() )
		{
			if ( m_dialog->RetWriteFile() )
			{
				WriteScene();
			}
			m_fileBuffer->Close();
			m_edit->Close();
		}
		m_engine->SetJoyForces(0.0f, 0.0f);
		m_sound->StopMusic();
		m_camera->SetObject(0);

		m_dialog->IncGamerInfoTry();
		m_dialog->SetGamerTotalTime(m_gameTime);

		if ( phase == PHASE_WIN )
		{
			m_dialog->SetGamerInfoPassed();
			m_dialog->NextMission();
		}

		m_dialog->WriteGamerMission();
	}

	m_phase = phase;

	m_winDelay     = 0.0f;
	m_lostDelay    = 0.0f;
	m_bMovieLock   = FALSE;
	m_bMovieFinal  = FALSE;
	m_bFreePhoto   = FALSE;
	m_bDisplayInfo = FALSE;

	if ( m_scene != oldScene )
	{
		DeleteAllObjects();  // supprime toute la scène 3D actuelle

		ChangePause(FALSE);
		m_engine->SetRankView(0);
		m_engine->FlushObject();
		color.r = color.g = color.b = color.a = 0.0f;
		m_engine->SetWaterAddColor(color);
		m_engine->SetBackground("");
		m_engine->SetBackForce(FALSE);
		m_engine->LensFlareFlush();
//?		m_engine->SetOverColor();
		m_engine->GroundMarkDelete(0);
		SetSpeed(1.0f);
		m_terrain->SetWind(D3DVECTOR(0.0f, 0.0f, 0.0f));
		m_light->FlushLight();
		m_particule->FlushParticule();
		m_water->Flush();
		m_cloud->Flush();
		m_blitz->Flush();
		m_planet->Flush();
		m_iMan->Flush(CLASS_OBJECT);
		m_iMan->Flush(CLASS_PYRO);
		m_model->StopUserAction();
		m_sound->SetListener(D3DVECTOR(0.0f, 0.0f, 0.0f), D3DVECTOR(0.0f, 0.0f, 1.0f));
		m_camera->SetType(CAMERA_DIALOG);
	}

	m_engine->SetMovieLock(m_bMovieLock);
	m_interface->Flush();
	ClearInterface();
	m_defMouse = D3DMOUSENORM;
	m_terrainSpritePos = D3DVECTOR(NAN, NAN, NAN);
	m_terrainSpriteChannel = -1;

	// Crée et cache la console de commande.
	pos.x = 100.0f/640.0f;
	pos.y = 452.0f/480.0f;
	dim.x = 200.0f/640.0f;
	dim.y =  18.0f/480.0f;
	pe = m_interface->CreateEdit(pos, dim, 0, EVENT_CMD);
	if ( pe == 0 )  return;
	pe->ClearState(STATE_VISIBLE);
	m_bCmdEdit = FALSE;  // caché pour l'instant

	// Crée l'indicateur de vitesse.
	pos.x =   0.0f/640.0f;
	pos.y = 460.0f/480.0f;
	dim.x =  30.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_SPEED);
	if ( pb == 0 )  return;
	pb->SetState(STATE_SIMPLY);
	pb->ClearState(STATE_VISIBLE);

	// Crée l'indicateur de pause.
	pos.x =  20.0f/640.0f;
	pos.y = 420.0f/480.0f;
	dim.x =  40.0f/640.0f;
	dim.y =  40.0f/480.0f;
	pgr = m_interface->CreateGroup(pos, dim, 22, EVENT_OBJECT_EDITLOCK);
	if ( pgr == 0 )  return;
	pgr->ClearState(STATE_VISIBLE);

	m_dialog->ChangePhase(m_phase, fadeIn);

	dim.x = 32.0f/640.0f;
	dim.y = 32.0f/480.0f;
	ox = 3.0f/640.0f;
	oy = 3.0f/480.0f;
	sx = (32.0f+2.0f)/640.0f;
	sy = (32.0f+2.0f)/480.0f;

	m_engine->SetDrawWorld(TRUE);
	m_engine->SetDrawFront(FALSE);
	m_handleMode = 1;
	m_adviseMode = 0;

#if _DEMO | _SE
	if ( (m_phase >= PHASE_GENERIC1 &&
		  m_phase <= PHASE_GENERIC5 ) ||
		 m_phase == PHASE_INIT )
	{
		if ( m_phase != PHASE_GENERIC1 )
		{
			m_engine->FreeTexture("gener1a.tga");
			m_engine->FreeTexture("gener1b.tga");
			m_engine->FreeTexture("gener1c.tga");
			m_engine->FreeTexture("gener1d.tga");
		}
		if ( m_phase != PHASE_GENERIC2 )
		{
			m_engine->FreeTexture("gener2a.tga");
			m_engine->FreeTexture("gener2b.tga");
			m_engine->FreeTexture("gener2c.tga");
			m_engine->FreeTexture("gener2d.tga");
		}
		if ( m_phase != PHASE_GENERIC3 )
		{
			m_engine->FreeTexture("gener3a.tga");
			m_engine->FreeTexture("gener3b.tga");
			m_engine->FreeTexture("gener3c.tga");
			m_engine->FreeTexture("gener3d.tga");
		}
		if ( m_phase != PHASE_GENERIC4 )
		{
			m_engine->FreeTexture("gener4a.tga");
			m_engine->FreeTexture("gener4b.tga");
			m_engine->FreeTexture("gener4c.tga");
			m_engine->FreeTexture("gener4d.tga");
		}
		if ( m_phase != PHASE_GENERIC5 )
		{
			m_engine->FreeTexture("gener5a.tga");
			m_engine->FreeTexture("gener5b.tga");
			m_engine->FreeTexture("gener5c.tga");
			m_engine->FreeTexture("gener5d.tga");
		}
	}
#else
	if ( m_phase == PHASE_INIT )
	{
		m_engine->FreeTexture("generfa.tga");
		m_engine->FreeTexture("generfb.tga");
		m_engine->FreeTexture("generfc.tga");
		m_engine->FreeTexture("generfd.tga");
	}
#endif

	if ( m_phase == PHASE_SIMUL )
	{
		m_engine->FreeTexture("inter01a.tga");
		m_engine->FreeTexture("inter01b.tga");
		m_engine->FreeTexture("inter01c.tga");
		m_engine->FreeTexture("inter01d.tga");

		if ( bCreate )
		{
			CreateScene(FALSE);  // scène interractive
		}

		if ( RetEdit() )
		{
			m_edit->Open();
		}

		// Crée les boutons.
		pos.x = 620.0f/640.0f;
		pos.y = 460.0f/480.0f;
		dim.x =  20.0f/640.0f;
		dim.y =  20.0f/480.0f;
		pb = m_interface->CreateButton(pos, dim, 11, EVENT_BUTTON_QUIT);

		if ( !RetEdit() )
		{
			pos.x = 10.0f/640.0f;
			pos.y = 10.0f/480.0f;
			dim.x = 36.0f/640.0f;
			dim.y = 36.0f/480.0f;
			pb = m_interface->CreateButton(pos, dim, 19, EVENT_INTERFACE_UNDO);
			pb->SetState(STATE_VISIBLE, FALSE);

			pos.x = 10.0f/640.0f;
			pos.y = 46.0f/480.0f;
			dim.x = 36.0f/640.0f;
			dim.y = 36.0f/480.0f;
			pb = m_interface->CreateButton(pos, dim, 12, EVENT_INTERFACE_SIGN);
			pb->SetState(STATE_VISIBLE, FALSE);
		}

		m_engine->SetJoyForces(0.0f, 0.0f);
		m_sound->StopMusic();
		StartMusic();

		m_firstInfoDelay = 0.0f;
		if ( !RetAgain() && !RetEdit() && m_scene != oldScene )
		{
			m_firstInfoDelay = 0.2f;  // faudra afficher les consignes
		}

//?		m_engine->SetOverColor(RetColor(0.0f), D3DSTATETCw);
//?		m_engine->SetOverFront(TRUE);
	}

	if ( m_scene == SCENE_FIX && oldScene != SCENE_FIX )
	{
		CreateScene(TRUE);  // scène fixe
	}

	if ( m_phase == PHASE_WIN )
	{
		ChangePhase(PHASE_TERM);
		m_sound->StopAll();
		StartMusic();
	}

	if ( m_phase == PHASE_MODELe )
	{
		pos.x = ox+sx*0;  pos.y = oy+sy*0;
		m_interface->CreateButton(pos, dim, 11, EVENT_BUTTON_CANCEL);
		CreateModel(0);
	}
	if ( m_phase == PHASE_MODELi )
	{
		pos.x = ox+sx*0;  pos.y = oy+sy*0;
		m_interface->CreateButton(pos, dim, 11, EVENT_BUTTON_CANCEL);
		CreateModel(1);
	}

	if ( m_phase == PHASE_LOADING )
	{
		m_engine->SetMouseHide(TRUE);
	}
	else
	{
		m_engine->SetMouseHide(FALSE);
	}

	m_engine->LoadAllTexture();
}


// Traite un événement.

BOOL CRobotMain::EventProcess(const Event &event)
{
	CEdit*			pe;
	Event			newEvent;

	if ( m_phase == PHASE_SIMUL && !RetEdit() )
	{
		m_interface->SetDefMouse(m_defMouse);
	}
	else
	{
		m_interface->SetDefMouse(D3DMOUSENORM);
	}

	if ( event.event == EVENT_FRAME )
	{
		ReadSuperShift();

		if ( m_bSkipFrame )
		{
			m_bSkipFrame = FALSE;
			return TRUE;
		}

		if ( m_firstInfoDelay > 0.0f && !m_bMovieLock )
		{
			m_firstInfoDelay -= event.rTime;
			if ( m_firstInfoDelay < 0.0f )  m_firstInfoDelay = 0.0f;

			if ( m_firstInfoDelay == 0.0f )
			{
				StartDisplayInfo(RetResume());
			}
		}

		if ( m_phase == PHASE_SIMUL && !RetEdit() )
		{
			UpdateInterface();
		}

		m_dialog->EventProcess(event);
		m_displayText->EventProcess(event);

		m_interface->EventProcess(event);
		return EventFrame(event);
	}

	// Gestion de la console de commande.
#if 0
	if ( m_phase != PHASE_NAME &&
		 event.event == EVENT_KEYDOWN &&
		 event.param == VK_PAUSE &&
		 (event.keyState&KS_CONTROL) != 0 )
#else
	if ( m_phase != PHASE_NAME &&
		 event.event == EVENT_KEYDOWN &&
		 event.param == VK_CANCEL )  // Ctrl+Pause ?
#endif
	{
		pe = (CEdit*)m_interface->SearchControl(EVENT_CMD);
		if ( pe == 0 )  return FALSE;
		pe->SetState(STATE_VISIBLE);
		pe->SetFocus(TRUE);
		if ( m_phase == PHASE_SIMUL )  ChangePause(TRUE);
		m_bCmdEdit = TRUE;
		return FALSE;
	}
	if ( event.event == EVENT_KEYDOWN &&
		 event.param == VK_RETURN && m_bCmdEdit )
	{
		char	cmd[50];
		pe = (CEdit*)m_interface->SearchControl(EVENT_CMD);
		if ( pe == 0 )  return FALSE;
		pe->GetText(cmd, 50);
		pe->SetText("");
		pe->ClearState(STATE_VISIBLE);
		if ( m_phase == PHASE_SIMUL )  ChangePause(FALSE);
		ExecuteCmd(cmd);
		m_bCmdEdit = FALSE;
		return FALSE;
	}

	// Gestion du changement de vitesse.
	if ( event.event == EVENT_SPEED )
	{
		SetSpeed(1.0f);
	}

	if ( !m_dialog->EventProcess(event) )
	{
		if ( event.event == EVENT_MOUSEMOVE )
		{
			m_lastMousePos = event.pos;
			HiliteObject(event.pos);
		}
		return FALSE;
	}

	if ( !m_displayText->EventProcess(event) )
	{
		return FALSE;
	}

	if ( event.event == EVENT_MOUSEMOVE )
	{
		m_lastMousePos = event.pos;
		HiliteObject(event.pos);
	}

	if ( m_bDisplayInfo )  // info en cours ?
	{
		if ( event.event == EVENT_KEYDOWN )
		{
			if ( event.param == m_engine->RetKey(KEYRANK_HELP, 0) ||
				 event.param == m_engine->RetKey(KEYRANK_HELP, 1) ||
				 event.param == VK_ESCAPE )
			{
				StopDisplayInfo();
				return FALSE;
			}
		}
		if ( event.event == EVENT_INTERFACE_RESOK )
		{
			StopDisplayInfo();
			return FALSE;
		}
	}

	// Phase de simulation du jeu.
	if ( m_phase == PHASE_SIMUL )
	{
		if ( RetEdit() )
		{
			UpdateInfoText();
			m_camera->EventProcess(event);

			switch( event.event )
			{
				case EVENT_RBUTTONDOWN:
					m_engine->SetMouseCapture();
					ScrollBegin(event.pos);
					break;

				case EVENT_MOUSEMOVE:
					if ( m_bScroll )
					{
						ScrollMove(event.pos);
					}
					break;

				case EVENT_RBUTTONUP:
					m_engine->ReleaseMouseCapture();
					if ( m_bScroll )
					{
						if ( ScrollEnd(event.pos) )
						{
							m_edit->MouseDownFlush();
							return FALSE;
						}
					}
					break;
			}

			m_edit->EventProcess(event);

			switch( event.event )
			{
				case EVENT_KEYDOWN:
				case EVENT_BUTTON_QUIT:
//?					HiliteClear();
					if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
						 event.param == VK_BUTTON9 )
					{
						m_engine->SetMouseHide(FALSE);
						m_dialog->StartAbort();  // voulez-vous quitter ?
					}
					break;
				case EVENT_EDIT_INFO:
					m_dialog->StartInfoPuzzle();
					break;
			}
			return FALSE;
		}
		else
		{
			UpdateInfoText();
			m_camera->EventProcess(event);

			switch( event.event )
			{
				case EVENT_KEYDOWN:
//?					HiliteClear();
					if ( m_bMovieLock && !m_bMovieFinal )  // film en cours ?
					{
						if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
							 event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
							 event.param == VK_ESCAPE  ||
							 event.param == VK_BUTTON1 ||
							 event.param == VK_BUTTON2 )
						{
							AbortMovie();
						}
						return FALSE;
					}
					if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
						 event.param == VK_BUTTON9 )
					{
						m_engine->SetMouseHide(FALSE);
						if ( m_bPause )  ChangePause(FALSE);
						if ( m_winDelay > 0.0f )
						{
							ChangePhase(PHASE_FADEIN, PHASE_WIN);
						}
						else if ( m_lostDelay > 0.0f )
						{
							ChangePhase(PHASE_LOST);
						}
						else
						{
							m_dialog->StartAbort();  // voulez-vous quitter ?
						}
					}
					if ( event.param == VK_PAUSE )
					{
						if ( !m_bMovieLock && !m_bCmdEdit )
						{
							ChangePause(!m_engine->RetPause());
						}
					}
					if ( event.param == m_engine->RetKey(KEYRANK_HELP, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_HELP, 1) )
					{
						if ( m_bDisplayInfo )  // info en cours ?
						{
							StopDisplayInfo();
							return FALSE;
						}
						else
						{
							StartDisplayInfo(RetResume());
							return FALSE;
						}
					}
					if ( event.param == m_engine->RetKey(KEYRANK_STOP, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_STOP, 1) )
					{
						StopAction();
					}
#if _DEBUG
					if ( event.param == m_engine->RetKey(KEYRANK_SPEED10, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_SPEED10, 1) )
					{
						SetSpeed(1.0f);
					}
					if ( event.param == m_engine->RetKey(KEYRANK_SPEED15, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_SPEED15, 1) )
					{
						SetSpeed(1.5f);
					}
					if ( event.param == m_engine->RetKey(KEYRANK_SPEED20, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_SPEED20, 1) )
					{
						SetSpeed(2.0f);
					}
					if ( event.param == m_engine->RetKey(KEYRANK_SPEED30, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_SPEED30, 1) )
					{
						SetSpeed(3.0f);
					}
#endif
					break;

				case EVENT_KEYUP:
					break;

				case EVENT_LBUTTONDOWN:
					m_engine->SetMouseCapture();
					if ( !FilterMousePos(event.pos) )
					{
						SelectBegin(event.pos);
					}
					break;
				case EVENT_RBUTTONDOWN:
					m_engine->SetMouseCapture();
					if ( !FilterMousePos(event.pos) )
					{
						ScrollBegin(event.pos);
					}
					break;

				case EVENT_MOUSEMOVE:
					if ( !FilterMousePos(event.pos) )
					{
						if ( m_bSelect )
						{
							SelectMove(event.pos);
						}
						if ( m_bScroll )
						{
							ScrollMove(event.pos);
						}
					}
					break;

				case EVENT_LBUTTONUP:
				case EVENT_RBUTTONUP:
					m_engine->ReleaseMouseCapture();
					if ( !FilterMousePos(event.pos) )
					{
						if ( m_bSelect )
						{
							SelectEnd(event.pos);
						}
						if ( m_bScroll )
						{
							ScrollEnd(event.pos);
						}
					}
					break;

				case EVENT_BUTTON_QUIT:
					if ( m_bPause )  ChangePause(FALSE);
					if ( m_winDelay > 0.0f )
					{
						ChangePhase(PHASE_FADEIN, PHASE_WIN);
					}
					else if ( m_lostDelay > 0.0f )
					{
						ChangePhase(PHASE_LOST);
					}
					else
					{
						m_dialog->StartAbort();  // voulez-vous quitter ?
					}
					break;

				case EVENT_INTERFACE_UNDO:
					if ( m_typeButton == 0 )
					{
						if ( m_undo->Undo() )
						{
							m_sound->Play(SOUND_ENERGY);
						}
						else
						{
							m_sound->Play(SOUND_WARNING);
						}
					}
					else
					{
						StopAction();
					}
					break;

				case EVENT_INTERFACE_SIGN:
					if ( m_bDisplayInfo )  // info en cours ?
					{
						StopDisplayInfo();
					}
					else
					{
						StartDisplaySign();
					}
					break;

				case EVENT_OBJECT_MAPZOOM:
					m_map->ZoomMap();
					break;

				case EVENT_WIN:
					ChangePhase(PHASE_FADEIN, PHASE_WIN);
					break;

				case EVENT_LOST:
					ChangePhase(PHASE_LOST);
					break;
			}

			EventObject(event);
			return FALSE;
		}
	}

	if ( m_phase == PHASE_WIN  ||
		 m_phase == PHASE_LOST )
	{
		EventObject(event);

		switch( event.event )
		{
			case EVENT_KEYDOWN:
				if ( event.param == VK_UP    ||
					 event.param == VK_LEFT  ||
					 (event.param == VK_TAB && (event.keyState&KS_SHIFT) != 0) ||
					 event.param == VK_JUP   ||
					 event.param == VK_JLEFT )
				{
					ChangeTabOrder(-1, event.param);
				}
				if ( event.param == VK_DOWN   ||
					 event.param == VK_RIGHT  ||
					 (event.param == VK_TAB && (event.keyState&KS_SHIFT) == 0) ||
					 event.param == VK_JDOWN  ||
					 event.param == VK_JRIGHT )
				{
					ChangeTabOrder(1, event.param);
				}

				if ( event.param == VK_ESCAPE )
				{
					ChangePhase(PHASE_TERM);
				}
				break;

			case EVENT_INTERFACE_AGAIN:
				m_dialog->LaunchSimul(FALSE, FALSE);
				m_dialog->SetAgain(TRUE);
				break;

			case EVENT_INTERFACE_TERM:
				ChangePhase(PHASE_TERM);
				break;
		}
	}

	if ( m_phase == PHASE_MODELe ||
		 m_phase == PHASE_MODELi )
	{
		switch( event.event )
		{
			case EVENT_KEYDOWN:
				if ( event.param == VK_ESCAPE )
				{
					ChangePhase(PHASE_INIT);
				}
				if ( event.param == VK_HOME )
				{
					InitEye();
				}
				break;

			case EVENT_BUTTON_CANCEL:
				ChangePhase(PHASE_INIT);
				break;
		}

		m_model->EventProcess(event);
		return FALSE;
	}

	return TRUE;
}

// Lit l'état des touches super-shift.

void CRobotMain::ReadSuperShift()
{
	m_superShift19 = 0;

	if ( GetKeyboardState(m_superShift) )
	{
		if ( (m_superShift['1']&0x80) || (m_superShift[VK_NUMPAD1]&0x80) )  m_superShift19 = 1;
		if ( (m_superShift['2']&0x80) || (m_superShift[VK_NUMPAD2]&0x80) )  m_superShift19 = 2;
		if ( (m_superShift['3']&0x80) || (m_superShift[VK_NUMPAD3]&0x80) )  m_superShift19 = 3;
		if ( (m_superShift['4']&0x80) || (m_superShift[VK_NUMPAD4]&0x80) )  m_superShift19 = 4;
		if ( (m_superShift['5']&0x80) || (m_superShift[VK_NUMPAD5]&0x80) )  m_superShift19 = 5;
		if ( (m_superShift['6']&0x80) || (m_superShift[VK_NUMPAD6]&0x80) )  m_superShift19 = 6;
		if ( (m_superShift['7']&0x80) || (m_superShift[VK_NUMPAD7]&0x80) )  m_superShift19 = 7;
		if ( (m_superShift['8']&0x80) || (m_superShift[VK_NUMPAD8]&0x80) )  m_superShift19 = 8;
		if ( (m_superShift['9']&0x80) || (m_superShift[VK_NUMPAD9]&0x80) )  m_superShift19 = 9;
	}

	if ( m_superShift19 != m_lastSuperShift19 )
	{
		m_lastSuperShift19 = m_superShift19;
		HiliteClear();
		HiliteObject(m_lastMousePos);
	}
}

// Indique quelle est la touche super-shift 1..9 pressée.

int CRobotMain::RetSuperShift19()
{
	return m_superShift19;
}


// Change le bouton qui a le focus.

void CRobotMain::ChangeTabOrder(int dir, int param)
{
	CControl*	pc;
	CControl*	pn;
	int			rank, i;

	pc = m_interface->SearchControl();  // cherche bouton avec le focus
	if ( pc == 0 )  return;

	rank = pc->RetTabOrder();
	if ( rank == -1 )  return;

	for ( i=0 ; i<20 ; i++ )
	{
		rank += dir;
		if ( rank <  0 )  rank = 19;
		if ( rank > 19 )  rank =  0;
		pn = m_interface->SearchControl(rank);  // cherche le nouveau bouton
		if ( pn == 0 )  continue;
		if ( !pn->TestState(STATE_VISIBLE) )  continue;
		if ( !pn->TestState(STATE_ENABLE ) )  continue;
		break;
	}
	if ( pn == 0 ) return;

	pc->SetFocus(FALSE);
	pn->SetFocus(TRUE);
}


// Exécute une commande.

void CRobotMain::ExecuteCmd(char *cmd)
{
	if ( cmd[0] == 0 )  return;

	if ( m_phase == PHASE_SIMUL )
	{
		if ( strcmp(cmd, "bypass") == 0 )
		{
			Event		newEvent;
			m_event->MakeEvent(newEvent, EVENT_WIN);
			m_event->AddEvent(newEvent);
			m_bCheatUsed = TRUE;  // gagné en trichant
			return;
		}

		if ( strcmp(cmd, "looser") == 0 )
		{
			Event		newEvent;
			m_event->MakeEvent(newEvent, EVENT_LOST);
			m_event->AddEvent(newEvent);
			return;
		}

		if ( strcmp(cmd, "longtime") == 0 )
		{
			m_gameTime = 10.0f*60.0f;
			return;
		}
	}

#if _DEBUG
	if ( strcmp(cmd, "testgame") == 0 )
	{
		m_bShowAll = TRUE;
		m_dialog->AllMissionUpdate();
		return;
	}

	if ( strcmp(cmd, "all") == 0 )
	{
		m_bShowAll = !m_bShowAll;
		m_dialog->AllMissionUpdate();
		return;
	}
#endif

	if ( strcmp(cmd, "showstat") == 0 )
	{
		m_engine->SetShowStat(!m_engine->RetShowStat());
		return;
	}

	if ( strcmp(cmd, "showpos") == 0 )
	{
		m_bShowPos = !m_bShowPos;
		return;
	}

	if ( strcmp(cmd, "process=1") == 0 )
	{
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		return;
	}
	if ( strcmp(cmd, "process=2") == 0 )
	{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		return;
	}

	if ( strcmp(cmd, "thread=1") == 0 )
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		return;
	}
	if ( strcmp(cmd, "thread=2") == 0 )
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
		return;
	}
	if ( strcmp(cmd, "thread=3") == 0 )
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		return;
	}

#if _DEBUG
	if ( m_phase == PHASE_SIMUL )
	{
		if ( strcmp(cmd, "photo1") == 0 )
		{
			m_bFreePhoto = !m_bFreePhoto;
			if ( m_bFreePhoto )
			{
				m_camera->SetType(CAMERA_FREE);
				ChangePause(TRUE);
			}
			else
			{
				m_camera->SetType(CAMERA_FREE);
				ChangePause(FALSE);
			}
			return;
		}

		if ( strcmp(cmd, "photo2") == 0 )
		{
			CObject*	object;
			CGroup*		pg;

			m_bFreePhoto = !m_bFreePhoto;
			if ( m_bFreePhoto )
			{
				m_camera->SetType(CAMERA_FREE);
				ChangePause(TRUE);
				object = RetSelect();
				m_map->ShowMap(FALSE);
				m_displayText->HideText(TRUE);

				pg = (CGroup*)m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
				if ( pg != 0 )
				{
					pg->ClearState(STATE_VISIBLE);
				}
			}
			else
			{
				m_camera->SetType(CAMERA_FREE);
				ChangePause(FALSE);
				object = RetSelect();
				m_map->ShowMap(TRUE);
				m_displayText->HideText(FALSE);
			}
			return;
		}

		if ( strcmp(cmd, "noclip") == 0 )
		{
			CObject*	object;

			object = RetSelect();
			if ( object != 0 )
			{
				object->SetClip(FALSE);
			}
			return;
		}

		if ( strcmp(cmd, "clip") == 0 )
		{
			CObject*	object;

			object = RetSelect();
			if ( object != 0 )
			{
				object->SetClip(TRUE);
			}
			return;
		}

		if ( strcmp(cmd, "addhusky") == 0 )
		{
			CObject*	object;

			object = RetSelect();
			if ( object != 0 )
			{
				object->SetMagnifyDamage(object->RetMagnifyDamage()*0.1f);
			}
			return;
		}

		if ( strcmp(cmd, "strong") == 0 )
		{
			CObject*	object;

			object = RetSelect();
			if ( object != 0 )
			{
				object->SetStrong(1.0f);
			}
			return;
		}

		if ( strcmp(cmd, "glu") == 0 )
		{
			CObject*	object;

			object = RetSelect();
			if ( object != 0 )
			{
				object->SetStrong(-1.0f);
			}
			return;
		}
	}

	if ( strcmp(cmd, "debugmode") == 0 )
	{
		m_engine->SetDebugMode(!m_engine->RetDebugMode());
		return;
	}

	if ( strcmp(cmd, "showstat") == 0 )
	{
		m_engine->SetShowStat(!m_engine->RetShowStat());
		return;
	}

//?	if ( strcmp(cmd, "showlockzone") == 0 )
	if ( strcmp(cmd, "lz") == 0 )
	{
		m_terrain->SetDebugLockZone(!m_terrain->RetDebugLockZone());
		return;
	}

	if ( strcmp(cmd, "crypt") == 0 )
	{
		Crypt("scene", TRUE);
		Crypt("defi", TRUE);
		m_displayText->DisplayText("Cryptage terminé");
	}
	if ( strcmp(cmd, "decrypt") == 0 )
	{
		Crypt("scene", FALSE);
		Crypt("defi", FALSE);
		m_displayText->DisplayText("Décryptage terminé");
	}

	if ( m_phase == PHASE_SIMUL )
	{
		m_displayText->DisplayError(ERR_CMD);
	}
#endif
}


// Crypte ou décrypte tous les fichiers.

void CRobotMain::Crypt(char *dir, BOOL bCrypt)
{
	long		hFile;
	struct _finddata_t fileBuffer;
	char		crit[200];
	char		filename[200];

	strcpy(crit, dir);
	strcat(crit, "\\*.bm2");
	hFile = _findfirst(crit, &fileBuffer);
	if ( hFile != -1 )
	{
		do
		{
			if ( (fileBuffer.attrib & _A_SUBDIR) == 0 )
			{
				strcpy(filename, dir);
				strcat(filename, "\\");
				strcat(filename, fileBuffer.name);
				CryptFile(filename, bCrypt);
			}
		}
		while ( _findnext(hFile, &fileBuffer) == 0 );
	}
}

// Crypte ou décrypte un fichier.

BOOL CRobotMain::CryptFile(char *filename, BOOL bCrypt)
{
	CCryptFile	fileSrc;
	CCryptFile	fileDst;
	char		filenameSrc[200];
	char		filenameDst[200];
	char		line[1000];

	sprintf(filenameSrc, "%s", filename);
	if ( !fileSrc.Open(filenameSrc, "r") )
	{
		return FALSE;
	}

	strcpy(filenameDst, filenameSrc);
	strcpy(filenameDst+strlen(filenameDst)-4, ".tmp");
	remove(filenameDst);
	if ( !fileDst.Open(filenameDst, "w", bCrypt) )
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

	remove(filenameSrc);
	rename(filenameDst, filenameSrc);
	return TRUE;
}


// Stoppe l'action en cours dès que possible.

void CRobotMain::StopAction()
{
	CTaskList*	pTaskList;

	if ( m_selectObject == 0 )  return;

	pTaskList = m_selectObject->RetTaskList();
	if ( pTaskList == 0 )  return;

	pTaskList->Stop();
}

// Met à jour l'état de tous les boutons de l'interface.

void CRobotMain::UpdateInterface()
{
	CButton*	pb;
	CTaskList*	pTaskList;
	BOOL		bQuit, bUndo, bStop, bStopable, bSign;

	m_typeButton = 0;

	bQuit = TRUE;

	bUndo = FALSE;
	if ( m_undo->IsUndoable() )
	{
		m_undoCounter ++;
		if ( m_undoCounter > 1 )  bUndo = TRUE;
	}
	else
	{
		m_undoCounter = 0;
	}
	if ( bUndo )  m_typeButton = 0;

	bStopable = FALSE;
	if ( m_selectObject != 0 )
	{
		pTaskList = m_selectObject->RetTaskList();
		if ( pTaskList != 0 )
		{
			bStopable = pTaskList->IsStopable();
		}
	}

	bStop = TRUE;
	if ( bStopable )
	{
		m_stopCounter = 0;
	}
	else
	{
		m_stopCounter ++;
		if ( m_stopCounter > 1 )  bStop = FALSE;
	}
	if ( bStop )  m_typeButton = 1;

	bSign = TRUE;
	if ( m_gameTime < 10.0f*60.0f &&  // 10 minutes de jeu ?
		 !RetTest() )
	{
		bSign = FALSE;
	}
	if ( m_engine->RetSetup(ST_HELP) == 0.0f )
	{
		bSign = FALSE;
	}

	if ( m_bMovieLock || m_bSuspend ||
		 m_phase == PHASE_FADEIN )
	{
		bQuit = FALSE;
		bUndo = FALSE;
		bStop = FALSE;
		bSign = FALSE;
	}

#if 0
	if ( m_bDisplayInfo )
	{
		bUndo = FALSE;
		bStop = FALSE;
		bSign = FALSE;
	}
#endif

	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
	if ( pb != 0 )
	{
		pb->SetState(STATE_VISIBLE, bQuit);
	}

	pb = (CButton*)m_interface->SearchControl(EVENT_INTERFACE_UNDO);
	if ( pb != 0 )
	{
		pb->SetIcon(m_typeButton==0?19:21);
		pb->SetState(STATE_VISIBLE, m_typeButton==0?bUndo:bStop);
	}

	pb = (CButton*)m_interface->SearchControl(EVENT_INTERFACE_SIGN);
	if ( pb != 0 )
	{
		pb->SetState(STATE_VISIBLE, bSign);
	}
}


// Filtre la position de la souris lorsqu'elle est sur
// les informations.

BOOL CRobotMain::FilterMousePos(FPOINT pos)
{
	if ( !m_bDisplayInfo )  return FALSE;

	if ( pos.x >= m_infoPos.x && pos.x <= m_infoPos.x+m_infoDim.x &&
		 pos.y >= m_infoPos.y && pos.y <= m_infoPos.y+m_infoDim.y )
	{
		return TRUE;
	}

	return FALSE;
}


// Début de l'affichage des indices.

void CRobotMain::StartDisplaySign()
{
	char*	pText;
	char	text[100];

	pText = RetSign();
	if ( pText == 0 || pText[0] == 0 )
	{
		GetResource(RES_TEXT, RT_ADVISE_NOSIGN, text);
		StartDisplayInfo(text);
	}
	else
	{
		StartDisplayInfo(pText);
	}

	m_terrain->SignMarkShow(TRUE);
}

// Début de l'affichage des instructions.

void CRobotMain::StartDisplayInfo(char *text)
{
	CEdit*		pe;
	CButton*	pb;
	D3DVECTOR	ppos;
	FPOINT		pos, dim;
	int			i;

	if ( m_bCmdEdit )  return;
	if ( text[0] == 0 )  return;
	if ( m_engine->RetSetup(ST_HELP) == 0.0f )  return;

	m_interface->DeleteControl(EVENT_INTERFACE_RESUME);
	m_interface->DeleteControl(EVENT_INTERFACE_RESOK);

	pos.x =  56.0f/640.0f;
	pos.y =  10.0f/480.0f;
	dim.x = 528.0f/640.0f;
	dim.y =  72.0f/480.0f;
	m_infoPos = pos;
	m_infoDim = dim;
	pe = m_interface->CreateEdit(pos, dim, 0, EVENT_INTERFACE_RESUME);
	if ( pe == 0 )  return;
	pe->SetMaxChar(1000-50);
	pe->SetMultiFont(TRUE);
	pe->SetEditCap(FALSE);  // juste pour voir
	pe->SetHiliteCap(FALSE);
	pe->SetFontSize(10.0f);
	pe->SetText(text);

	pos.x = 594.0f/640.0f;
	dim.x =  36.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 16, EVENT_INTERFACE_RESOK);
	if ( pb == 0 )  return;
	pb->SetState(STATE_FLASH);

	for ( i=0 ; i<40 ; i++ )
	{
		ppos.x = (56.0f+528.0f*Rand())/640.0f;
		ppos.y = 10.0f/480.0f;
		ppos.z = 0.0f;
		dim.x = 0.02f+Rand()*0.02f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(ppos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 Rand()*2.5f+2.5f, 0.0f, SH_INTERFACE);
	}
	for ( i=0 ; i<40 ; i++ )
	{
		ppos.x = (56.0f+528.0f*Rand())/640.0f;
		ppos.y = 82.0f/480.0f;
		ppos.z = 0.0f;
		dim.x = 0.02f+Rand()*0.02f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(ppos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 Rand()*2.5f+2.5f, 0.0f, SH_INTERFACE);
	}
	for ( i=0 ; i<10 ; i++ )
	{
		ppos.x = 56.0f/640.0f;
		ppos.y = (10.0f+72.0f*Rand())/480.0f;
		ppos.z = 0.0f;
		dim.x = 0.02f+Rand()*0.02f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(ppos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 Rand()*2.5f+2.5f, 0.0f, SH_INTERFACE);
	}
	for ( i=0 ; i<10 ; i++ )
	{
		ppos.x = 584.0f/640.0f;
		ppos.y = (10.0f+72.0f*Rand())/480.0f;
		ppos.z = 0.0f;
		dim.x = 0.02f+Rand()*0.02f;
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(ppos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim,
									 (ParticuleType)(PARTILENS1+rand()%3),
									 Rand()*2.5f+2.5f, 0.0f, SH_INTERFACE);
	}

	m_sound->Play(SOUND_WARNING);
	m_bDisplayInfo = TRUE;
}

// Fin de l'affichage d'instructions.

void CRobotMain::StopDisplayInfo()
{
	m_interface->DeleteControl(EVENT_INTERFACE_RESUME);
	m_interface->DeleteControl(EVENT_INTERFACE_RESOK);

	if ( !RetEdit() && !RetTest() )
	{
		m_terrain->SignMarkShow(FALSE);
	}
	m_bDisplayInfo = FALSE;
}


// Début d'un dialogue pendant le jeu,

void CRobotMain::StartSuspend()
{
	m_map->ShowMap(FALSE);
	m_infoObject = DeselectAll();  // enlève les boutons de commande
	m_displayText->HideText(TRUE);
	m_bSuspend = TRUE;
}

// Fin d'un dialogue pendant le jeu,

void CRobotMain::StopSuspend()
{
	SelectObject(m_infoObject, FALSE);  // remet les boutons de commande
	m_map->ShowMap(TRUE);
	m_displayText->HideText(FALSE);
	m_bSuspend = FALSE;
}


// Indique si le chrono tourne.

BOOL CRobotMain::IsGameTime()
{
	return ( !m_bMovieLock && !m_bSuspend &&
			 m_winDelay == 0.0f && m_lostDelay == 0.0f &&
			 !m_bPause );
}

// Retourne le temps absolu du jeu.

float CRobotMain::RetGameTime()
{
	return m_gameTime;
}


// Gestion de la taille des caractères par défaut.

void CRobotMain::SetFontSize(float size)
{
	m_fontSize = size;
	SetProfileFloat("Edit", "FontSize", m_fontSize);
}

float CRobotMain::RetFontSize()
{
	return m_fontSize;
}

// Gestion de la taille de la fenêtre par défaut.

void CRobotMain::SetWindowPos(FPOINT pos)
{
	m_windowPos = pos;
	SetProfileFloat("Edit", "WindowPos.x", m_windowPos.x);
	SetProfileFloat("Edit", "WindowPos.y", m_windowPos.y);
}

FPOINT CRobotMain::RetWindowPos()
{
	return m_windowPos;
}

void CRobotMain::SetWindowDim(FPOINT dim)
{
	m_windowDim = dim;
	SetProfileFloat("Edit", "WindowDim.x", m_windowDim.x);
	SetProfileFloat("Edit", "WindowDim.y", m_windowDim.y);
}

FPOINT CRobotMain::RetWindowDim()
{
	return m_windowDim;
}



// Début d'une sélection.

void CRobotMain::SelectBegin(FPOINT mouse)
{
	m_bSelect = TRUE;
	m_selectBegin = mouse;
}

// Déplacement pendant une sélection.

void CRobotMain::SelectMove(FPOINT mouse)
{
	m_selectEnd = mouse;
//?	m_engine->SetSelect(m_selectBegin, m_selectEnd, TRUE);
}

// Fin d'une sélection.

void CRobotMain::SelectEnd(FPOINT mouse)
{
	m_bSelect = FALSE;

	SelectMove(mouse);
	GroupOper(mouse);
}

// Début d'un scroll.

void CRobotMain::ScrollBegin(FPOINT mouse)
{
	m_bScroll = TRUE;
	m_scrollBegin = mouse;
	m_scrollQuantity = 0;
}

// Déplacement pendant un scroll.

void CRobotMain::ScrollMove(FPOINT mouse)
{
	FPOINT	scroll;

	if ( mouse.x >= 0.0f && mouse.x <= 1.0f &&
		 mouse.y >= 0.0f && mouse.y <= 1.0f )
	{
		m_scrollEnd = mouse;
		m_scrollQuantity ++;

		scroll.x = (m_scrollEnd.x-m_scrollBegin.x);
		scroll.y = (m_scrollEnd.y-m_scrollBegin.y);
		m_camera->SetScrollMouse(scroll, FALSE);
	}
}

// Fin d'un scroll.

BOOL CRobotMain::ScrollEnd(FPOINT mouse)
{
	FPOINT	scroll;

	m_bScroll = FALSE;

	if ( m_scrollQuantity < 2 )
	{
		m_camera->SetScrollMouse(FPOINT(0.0f, 0.0f), TRUE);
		GroupOper(mouse);
		return FALSE;
	}
	else
	{
		scroll.x = (m_scrollEnd.x-m_scrollBegin.x);
		scroll.y = (m_scrollEnd.y-m_scrollBegin.y);
		m_camera->SetScrollMouse(scroll, TRUE);
		return TRUE;
	}
}

// Opération sur le groupe d'objets sélectionnés.

void CRobotMain::GroupOper(FPOINT mouse)
{
	CObject*	pTarget;
	CObject*	pObj;
	CTaskList*	pTaskList;
	ObjectType	type;
	D3DVECTOR	pos, ppos;
	float		param;
	int			part, i, res, total, nb;
	char		text[500];

	if ( !MagicDetect(mouse, pos, pTarget, part) )  return;

	if ( pTarget != 0 && IsSelectable(pTarget) )
	{
		DeselectAll();
		SelectObject(pTarget);
		return;
	}

	if ( RetSelect() == 0 )  return;  // rien de sélectionné

	if ( pTarget == 0 )
	{
		ppos = pos;
		ppos.y = m_edit->RetGridHeight();
		m_particule->CreateGoto(ppos);  // montre la position d'arrivée
	}

	if ( pTarget != 0 && !pTarget->RetActif() )  return;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetSelect() )  continue;
		if ( pObj->RetLock() )  continue;  // par ex. soulevé par dock

		if ( pTarget == 0 )  type = OBJECT_NULL;
		else                 type = pTarget->RetType();

		if ( pTarget != 0 && part != 0 &&
			 ((type >= OBJECT_BOX1 && type <= OBJECT_BOX20) ||
			  (type >= OBJECT_KEY1 && type <= OBJECT_KEY5 ) ) )
		{
			pTarget->SetFlash(part);
			param = (float)RetSuperShift19();
			if ( param == 0.0f || part >= 5 )  param = 1.0f;
			pObj->StartTaskList(TO_GOTOPUSH, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, param);
		}

		if ( pTarget != 0 &&
			 type == OBJECT_DOCK )
		{
			pTarget->SetFlash(part);
			if ( part == 6 )
			{
				pObj->StartTaskList(TO_GOTODOCK, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
			}
			else
			{
				total = RetSuperShift19();
				if ( total == 0 )  total ++;
				for ( nb=0 ; nb<total ; nb++ )
				{
					pObj->StartTaskList(TO_GOTODOCK, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
				}
			}
		}

		if ( pTarget != 0 && part == 1 &&
			 type == OBJECT_CATAPULT )
		{
			pTarget->SetFlash(part);
			pObj->StartTaskList(TO_GOTOCATA, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
		}

		if ( pTarget != 0 && part >= 3 && part <= 5 &&
			 type == OBJECT_TRAX )
		{
			pTarget->SetFlash(part);
			total = RetSuperShift19();
			if ( total == 0 )  total ++;
			for ( nb=0 ; nb<total ; nb++ )
			{
				pObj->StartTaskList(TO_GOTOTRAX, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
			}
		}

		if ( pTarget != 0 && part >= 3 && part <= 5 &&
			 type == OBJECT_PERFO )
		{
			pTarget->SetFlash(part);
			total = RetSuperShift19();
			if ( total == 0 )  total ++;
			for ( nb=0 ; nb<total ; nb++ )
			{
				pObj->StartTaskList(TO_GOTOPERFO, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
			}
		}

		if ( pTarget != 0 && part >= 3 && part <= 5 &&
			 type == OBJECT_GUN )
		{
			pTarget->SetFlash(part);
			total = RetSuperShift19();
			if ( total == 0 )  total ++;
			for ( nb=0 ; nb<total ; nb++ )
			{
				pObj->StartTaskList(TO_GOTOGUN, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
			}
		}

		if ( pTarget != 0 &&
			 (type == OBJECT_FIOLE || type == OBJECT_GLU) )
		{
			pTarget->SetFlash(part);
			pObj->StartTaskList(TO_GOTODRINK, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
		}

		if ( pTarget != 0 &&
			 type == OBJECT_GOAL )
		{
			pTarget->SetFlash(part);
			pObj->StartTaskList(TO_GOTOGOAL, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
		}

		if ( pTarget != 0 &&
			 type == OBJECT_BARRIER69 )  // plongeoir ?
		{
			pTarget->SetFlash(part);
			pObj->StartTaskList(TO_GOTODIVE, D3DVECTOR(0.0f, 0.0f, 0.0f), pTarget, part, 0.0f);
		}

		if ( pTarget == 0 )
		{
			pObj->StartTaskList(TO_GOTO, pos, pTarget, part, 0.0f);
		}

		if ( m_adviseMode == 1 )
		{
			pTaskList = pObj->RetTaskList();
			if ( pTaskList != 0 )
			{
				res = pTaskList->RetStatisticAdvise();
				if ( res != 0 )
				{
					GetResource(RES_TEXT, res, text);
					StartDisplayInfo(text);
					pTaskList->FlushStatistic();
				}
			}
		}
	}
}


// Retourne l'objet sélectionné.

CObject* CRobotMain::RetSelect()
{
	if ( RetEdit() )  return 0;
	return m_selectObject;
}

// Sélectionne un objet donné.

void CRobotMain::SetSelect(CObject* pSel)
{
	CObject*	pObj;
	int			i;

	m_selectObject = 0;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == pSel )
		{
			pObj->SetSelect(TRUE);
			m_selectObject = pObj;
		}
		else
		{
			pObj->SetSelect(FALSE);
		}
	}
}

// Sélectionne un autre blupi.

void CRobotMain::SelectOther(CObject* pSel)
{
	CObject*	pObj;
	int			i;

	m_selectObject = 0;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		pObj->SetSelect(FALSE);

		if ( pObj->RetLock() )  continue;  // blupi qui s'envole ?

		if ( pObj->RetType() == OBJECT_BLUPI &&
			 m_selectObject == 0 &&
			 pObj != pSel )
		{
			pObj->SetSelect(TRUE);
			m_selectObject = pObj;
		}
	}
}

// Désélectionne tout, et retourne l'objet qui était sélectionné.

CObject* CRobotMain::DeselectAll()
{
	CObject*	pObj;
	CObject*	pPrev;
	int			i;

	m_selectObject = 0;

	pPrev = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetSelect() )  pPrev = pObj;
		pObj->SetSelect(FALSE);
	}
	return pPrev;
}

// Sélectionne l'objet visé par la souris.

BOOL CRobotMain::SelectObject(CObject* pObj, BOOL bDisplayError)
{
	CObject*	pPrev;

	if ( m_bMovieLock || m_bPause )  return FALSE;
	if ( pObj == 0 )  return FALSE;

	pPrev = DeselectAll();

	SelectOneObject(pObj, bDisplayError);
	return TRUE;
}

// Sélectionne un objet, sans s'occuper de désélectionner le reste.

void CRobotMain::SelectOneObject(CObject* pObj, BOOL bDisplayError)
{
	pObj->SetSelect(TRUE, bDisplayError);
	m_selectObject = pObj;
}

// Désélectionne l'objet sélectionné.

BOOL CRobotMain::DeselectObject()
{
	DeselectAll();
	return TRUE;
}

// Supprime rapidement tous les objets.

void CRobotMain::DeleteAllObjects()
{
	CPyro*		pyro;
	CObject*	pObj;

	// Supprime tous les effets pyrotechniques en cours.
	while ( TRUE )
	{
		pyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, 0);
		if ( pyro == 0 )  break;

		pyro->DeleteObject();
		delete pyro;
	}

	while ( TRUE )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, 0);
		if ( pObj == 0 )  break;

		pObj->DeleteObject(TRUE);  // détruit rapidement
		delete pObj;
	}
}


// Détecte une position au sol ou un objet.

BOOL CRobotMain::MagicDetect(FPOINT mouse, D3DVECTOR &pos,
							 CObject* &pObj, int &part)
{
	ObjectType	type;

	if ( DetectObject(mouse, pObj, part) )
	{
		pos = pObj->RetPosition(0);
		pos = Grid(pos, 8.0f);
		return TRUE;
	}

	if ( m_terrain->GroundDetect(mouse, pos) )
	{
		pObj = SearchObject(pos, 4.0f, FALSE);

		if ( pObj != 0 )
		{
			type = pObj->RetType();
			if ( type == OBJECT_LIFT      ||
				 type == OBJECT_MAX1X     ||
				 type == OBJECT_DOCK      ||
				 type == OBJECT_CATAPULT  ||
				 type == OBJECT_BARRIER24 ||  // tunnel ?
				 type == OBJECT_BARRIER57 ||  // tunnel ?
				 type == OBJECT_BARRIER65 )   // barrière ?
			{
				pObj = 0;
			}
		}
		part = 0;
		return TRUE;
	}

	pObj = 0;
	part = 0;
	return FALSE;
}

// Détecte l'objet visé par la souris.

BOOL CRobotMain::DetectObject(FPOINT pos, CObject* &pObj, int &part)
{
	CObject*	pObjBase;
	int			partBase;

	// Détecte tous les objets.
	if ( !DetectObject(pos, FALSE, pObj, part) )  return FALSE;

	pObjBase = pObj;
	partBase = part;
	// Détecte seulement les boutons-actions.
	if ( DetectObject(pos, TRUE, pObj, part) )  return TRUE;

	// Si pas trouvé un bouton-action, rend l'objet de base.
	pObj = pObjBase;
	part = partBase;
	return TRUE;
}

// Détecte l'objet visé par la souris.

BOOL CRobotMain::DetectObject(FPOINT pos, BOOL bAction, CObject* &pObj, int &part)
{
	ObjectType	type;
	int			objRank, i, j, rank;

	objRank = m_engine->DetectObject(pos, bAction);
	if ( objRank == -1 )
	{
		pObj = 0;
		part = 0;
		return FALSE;
	}

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetActif() )  continue;
		// Il faut détecter une caisse "lock" (poussée), sinon
		// on va tenter un "goto" à cet endroit !

		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();

		for ( j=0 ; j<OBJECTMAXPART ; j++ )
		{
			rank = pObj->RetObjectRank(j);
			if ( rank == -1 )  continue;
			if ( rank != objRank )  continue;
			part = j;
			return TRUE;
		}
	}

	pObj = 0;
	part = 0;
	return FALSE;
}

// Indique si un objet est sélectionnable.

BOOL CRobotMain::IsSelectable(CObject* pObj)
{
	ObjectType	type;

	if ( !pObj->RetSelectable() )  return FALSE;

	type = pObj->RetType();
	if ( type == OBJECT_BLUPI )
	{
		return TRUE;
	}

	return FALSE;
}


// Supprime l'objet sélectionné.

BOOL CRobotMain::DeleteObject()
{
	CObject*	pObj;
	CPyro*		pyro;

	pObj = RetSelect();
	if ( pObj == 0 )  return FALSE;

	pyro = new CPyro(m_iMan);
	pyro->Create(PT_FRAGT, pObj);

	pObj->SetSelect(FALSE);  // désélectionne l'objet
	DeselectAll();

	return TRUE;
}


// Enlève la mise en évidence de l'objet survolé par la souris.

void CRobotMain::HiliteClear()
{
	CObject*	pObj;
	int			i;

	ClearTooltip();
	m_tooltipName[0] = 0;  // enlève vraiment le tooltip

	if ( !m_bHilite )  return;

	i = -1;
	m_engine->SetHiliteRank(&i);  // plus rien de sélectionné

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		pObj->SetHilite(FALSE, FALSE);
	}

	m_bHilite = FALSE;
}

// Met en évidence l'objet survolé par la souris.

void CRobotMain::HiliteObject(FPOINT pos)
{
	CObject*	pObj;
	D3DVECTOR	tPos, speed, terrainSpritePos;
	FPOINT		dim;
	char		name[100];
	int			part;

	m_defMouse = D3DMOUSENORM;

	if ( m_bMovieLock )  return;
//?	if ( m_engine->RetPause() )  return;
	if ( m_engine->RetMouseHide() )  return;

	ClearInterface();  // enlève mise en évidence et tooltip

	if ( m_engine->RetSetup(ST_TOOLTIPS) != 0.0f &&
		 m_interface->GetTooltip(pos, name) )
	{
		m_tooltipPos = pos;
		strcpy(m_tooltipName, name);
		m_tooltipTime = 0.0f;
		return;
	}

	if ( m_phase != PHASE_SIMUL )  return;

	terrainSpritePos = D3DVECTOR(NAN, NAN, NAN);

	if ( MagicDetect(pos, tPos, pObj, part) )
	{
		terrainSpritePos = tPos;
		terrainSpritePos.y = m_edit->RetGridHeight();

		if ( RetSelect() != 0 )
		{
			if ( pObj == 0 )
			{
				m_defMouse = D3DMOUSEGOTO;
				m_tooltipPos = pos;
				GetResource(RES_TEXT, RT_ACTION_GOTO, m_tooltipName);
				m_tooltipTime = 0.0f;
			}
			else
			{
				if ( pObj->IsAction(part) && RetSelect() != 0 )
				{
					pObj->SetVarTex(part);
					m_defMouse = D3DMOUSEHAND;
				}
				else
				{
					pObj->SetVarTex(-1);
					if ( IsSelectable(pObj) )
					{
						m_defMouse = D3DMOUSESELECT;
					}
				}

				if ( (RetSelect() != 0 || IsSelectable(pObj)) &&
					 m_engine->RetSetup(ST_TOOLTIPS) != 0.0f &&
					 pObj->GetTooltipName(part, name) )
				{
					m_tooltipPos = pos;
					strcpy(m_tooltipName, name);
					m_tooltipTime = 0.0f;
				}

				pObj->SetHilite(TRUE, IsSelectable(pObj));
				m_bHilite = TRUE;
			}
		}
	}

	if ( terrainSpritePos.x != m_terrainSpritePos.x ||
		 terrainSpritePos.z != m_terrainSpritePos.z )
	{
		if ( m_terrainSpriteChannel != -1 )
		{
			m_particule->DeleteParticule(m_terrainSpriteChannel);
			m_terrainSpriteChannel = -1;
		}
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		dim.x = 4.0f;
		dim.y = dim.x;
		m_terrainSpriteChannel = m_particule->CreateParticule(terrainSpritePos, speed, dim, PARTITERSPTIRE);
		m_terrainSpritePos = terrainSpritePos;
	}
}

// Efface certaines mises en évidence lorsque la souris a bougé.

void CRobotMain::MouseMoveClear()
{
	if ( m_terrainSpriteChannel != -1 )
	{
		m_particule->DeleteParticule(m_terrainSpriteChannel);
		m_terrainSpriteChannel = -1;
	}
}

// Met en évidence l'objet survolé par la souris.

void CRobotMain::HiliteFrame(float rTime)
{
//?	if ( m_bMovieLock )  return;

	m_tooltipTime += rTime;

	ClearTooltip();

	if ( m_tooltipTime >= 0.2f &&
		 m_tooltipName[0] != 0 )
	{
		CreateTooltip(m_tooltipPos, m_tooltipName);
	}
}

// Crée un tooltip.

void CRobotMain::CreateTooltip(FPOINT pos, char* text)
{
	CWindow*	pw;
	FPOINT		start, end, dim, offset, corner;

	corner.x = pos.x+0.022f;
	corner.y = pos.y-0.052f;

	m_engine->RetText()->DimText(text, corner, 1,
								 SMALLFONT, NORMSTRETCH, FONT_COLOBOT,
								 start, end);
	start.x -= 0.010f;
	start.y -= 0.002f;
	end.x   += 0.010f;
	end.y   += 0.004f;  // ch'tite marge

	pos.x = start.x;
	pos.y = start.y;
	dim.x = end.x-start.x;
	dim.y = end.y-start.y;

	offset.x = 0.0f;
	offset.y = 0.0f;
	if ( pos.x+dim.x > 1.0f )  offset.x = 1.0f-(pos.x+dim.x);
	if ( pos.y       < 0.0f )  offset.y = -pos.y;

	corner.x += offset.x;
	corner.y += offset.y;
	pos.x += offset.x;
	pos.y += offset.y;

	m_interface->CreateWindows(pos, dim, 1, EVENT_TOOLTIP);

	pw = (CWindow*)m_interface->SearchControl(EVENT_TOOLTIP);
	if ( pw != 0 )
	{
		pw->SetState(STATE_SHADOW);
		pw->SetTrashEvent(FALSE);

		pos.y -= m_engine->RetText()->RetHeight(SMALLFONT, FONT_COLOBOT)/2.0f;
		pw->CreateLabel(pos, dim, -1, EVENT_LABEL2, text);
	}
}

// Efface le tooltip précédent.

void CRobotMain::ClearTooltip()
{
	m_interface->DeleteControl(EVENT_TOOLTIP);
}



// Annule le film en cours.

void CRobotMain::AbortMovie()
{
	CObject*	pObj;
	CAuto*		automat;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		automat = pObj->RetAuto();
		if ( automat != 0 )
		{
			automat->Abort();
		}
	}

	m_engine->SetMouseHide(FALSE);
}



// Met à jour le texte d'informations.

void CRobotMain::UpdateInfoText()
{
	CObject*	pObj;
	D3DVECTOR	pos;
	char		info[100];

	if ( m_bShowPos )
	{
		pObj = RetSelect();
		if ( pObj != 0 )
		{
			pos = pObj->RetPosition(0);
			sprintf(info, "             Pos = %.2f ; %.2f ; %.2f", pos.x/UNIT, pos.z/UNIT, pos.y/UNIT);
			m_engine->SetInfoText(4, info);
		}
	}
}


// Initialise le point de vue.

void CRobotMain::InitEye()
{
	D3DVECTOR	eye, lookat;
	float		dirH, dirV, dist;

	if ( m_scene == SCENE_FIX )
	{
		eye = D3DVECTOR(0.0f, -50.0f, -200.0f);
		lookat = D3DVECTOR(0.0f, -200.0f, -50.0f);
		m_camera->SetType(CAMERA_SCRIPT);
		m_camera->SetScriptEye(eye);
		m_camera->SetScriptLookat(lookat);
		m_camera->FixCamera();
		m_camera->ResetLockRotate();
		return;
	}

	if ( m_phase == PHASE_SIMUL )
	{
		if ( m_selectObject == 0 )
		{
			lookat = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dirH =  0.0f*PI/180.0f;
			dirV = 45.0f*PI/180.0f;
			dist = 60.0f;
		}
		else
		{
			lookat = m_selectObject->RetPosition(0);
			dirH = PI/2.0f-m_selectObject->RetAngleY(0);
			dirV = 45.0f*PI/180.0f;
			dist = 60.0f;
		}
		dirH += 10.0f*PI/180.0f;
		m_camera->Init(lookat, dirH, dirV, dist);
		m_camera->SetType(CAMERA_FREE);
		m_camera->FixCamera();
		m_camera->ResetLockRotate();
	}

	if ( m_phase == PHASE_MODELe ||
		 m_phase == PHASE_MODELi )
	{
		m_model->InitView();
	}
}


// Fait progresser toute la scène.

BOOL CRobotMain::EventFrame(const Event &event)
{
	ObjectType	type;
	CObject		*pObj;
	CPyro*		pPyro;
	CMap*		pm;
	int			i;

	m_time += event.rTime;
	if ( IsGameTime() )
	{
		m_gameTime += event.rTime;
#if !_DEBUG
		if ( m_phase == PHASE_SIMUL &&
			 m_engine->RetSetup(ST_ACCEL) == 1.0f )
		{
			float		speed;

			speed = 1.0f+Norm((m_gameTime-10.0f*60.0f)/(10.0f*60.0f));
			m_engine->SetSpeed(speed);
		}
		else
		{
			m_engine->SetSpeed(1.0f);
		}
#endif
	}

	m_water->EventProcess(event);
	m_cloud->EventProcess(event);
	m_blitz->EventProcess(event);
	m_planet->EventProcess(event);

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm != 0 )  pm->FlushObject();
	if ( !m_bFreePhoto )
	{
		// Fait progresser tous les robots.
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;
			if ( pm != 0 )  pm->UpdateObject(pObj);
			if ( pObj->RetTruck() != 0 )  continue;
			type = pObj->RetType();
			pObj->EventProcess(event);
		}
		// Fait progresser tous les objets transportés par les robots.
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;
			if ( pObj->RetTruck() == 0 )  continue;
			pObj->EventProcess(event);
		}

		// Fait progresser les effets pyrotechniques.
		for ( i=0 ; i<1000000 ; i++ )
		{
			pPyro = (CPyro*)m_iMan->SearchInstance(CLASS_PYRO, i);
			if ( pPyro == 0 )  break;

			pPyro->EventProcess(event);
			if ( pPyro->IsEnded() != ERR_CONTINUE )
			{
				pPyro->DeleteObject();
				delete pPyro;
			}
		}
	}

	// Fait bouger la caméra après les objets, car sa position peut
	// dépendre de l'objet sélectionné (CAMERA_ONBOARD ou CAMERA_BACK).
	if ( m_phase == PHASE_SIMUL )
	{
		m_camera->EventProcess(event);
		m_camera->SetOverBaseColor(m_particule->RetFogColor(m_engine->RetEyePt()));
	}
	if ( m_phase == PHASE_WIN  ||
		 m_phase == PHASE_LOST )
	{
		m_camera->EventProcess(event);
	}

	// Fait progresser le modèle.
	if ( m_phase == PHASE_MODELe ||
		 m_phase == PHASE_MODELi )
	{
		m_model->ViewMove(event, 2.0f);
		m_model->UpdateView();
		m_model->EventProcess(event);
	}

	HiliteFrame(event.rTime);

	// Fait bouger l'indicateur de pause.
	if ( m_bPause )  // édition en cours ?
	{
		CGroup*		pg;
		FPOINT		pos, dim;
		float		zoom;

		pg = (CGroup*)m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
		if ( pg != 0 )
		{
			pos.x =  40.0f/640.0f;
			pos.y = 440.0f/480.0f;
			dim.x =  40.0f/640.0f;
			dim.y =  40.0f/480.0f;

			zoom = 1.0f+sinf(m_time*6.0f)*0.1f;  // 0.9 .. 1.1
			dim.x *= zoom;
			dim.y *= zoom;
			pos.x -= dim.x/2.0f;
			pos.y -= dim.y/2.0f;

			pg->SetPos(pos);
			pg->SetDim(dim);
		}
	}

	if ( m_phase == PHASE_SIMUL )
	{
		if ( m_checkEndTime+1.0f < m_time )
		{
			m_checkEndTime = m_time;
			CheckEndMission(TRUE);
		}

		if ( m_winDelay > 0.0f )
		{
			m_winDelay -= event.rTime;
			if ( m_winDelay <= 0.0f )
			{
				Event		newEvent;
				m_event->MakeEvent(newEvent, EVENT_WIN);
				m_event->AddEvent(newEvent);
				m_bCheatUsed = FALSE;  // gagné sans tricher
			}
		}

		if ( m_lostDelay > 0.0f )
		{
			m_lostDelay -= event.rTime;
			if ( m_lostDelay <= 0.0f )
			{
				Event		newEvent;
				m_event->MakeEvent(newEvent, EVENT_LOST);
				m_event->AddEvent(newEvent);
			}
		}
	}

	return S_OK;
}

// Donne l'événement à tous les robots.

BOOL CRobotMain::EventObject(const Event &event)
{
	CObject*	pObj;
	int			i;

	if ( m_bFreePhoto )  return S_OK;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		pObj->EventProcess(event);
	}

	return S_OK;
}


// Crée toute la scène.

void CRobotMain::CreateScene(BOOL bFixScene)
{
	CObject*		pSel;
	CMotion*		motion;
	CCryptFile		file;
	char			filename[100];
	char			line[1000];
	char			name[200];
	char			dir[100];
	char			op[100];
	D3DCOLORVALUE	color;
	D3DVECTOR		pos, vortex;
	int				obj, i, j, rankBlupi, chapter;

	if ( bFixScene )
	{
		strcpy(filename, "scene\\init001.bm2");
	}
	else
	{
		m_dialog->BuildSceneName(filename);
	}

	if ( !bFixScene && strcmp(filename, m_lastFilename) == 0 )
	{
		m_repeat = 1;
		strcpy(m_lastFilename, filename);
	}

	m_bSkipFrame = TRUE;  // saute 1er EventFrame (rTime trop grand)

	g_id = 0;
	g_build = 0;
	g_researchDone = 0;  // aucune recherche effectuée
	g_researchEnable = 0;
	m_selectObject = 0;

	m_engine->FlushReplaceTex();
	m_engine->InitLockZone();
	m_audioTrack = 0;
	m_bAudioRepeat = TRUE;
	m_bDisplayInfo = FALSE;
	m_displayText->SetDelay(1.0f);
	m_displayText->SetEnable(TRUE);
	m_endingWinRank   = 0;
	m_endingLostRank  = 0;
	m_additionalTotal = 0;
	m_totalBlupi = 0;
	m_totalGoal = 0;
	m_totalManip = 0;

	m_colorRefBot.r = 110.0f/256.0f;
	m_colorRefBot.g = 161.0f/256.0f;
	m_colorRefBot.b = 208.0f/256.0f;  // bleu
	m_colorRefBot.a = 0.0f;
	m_colorNewBot = m_colorRefBot;

	m_colorRefGreen.r = 135.0f/256.0f;
	m_colorRefGreen.g = 170.0f/256.0f;
	m_colorRefGreen.b =  13.0f/256.0f;  // vert
	m_colorRefGreen.a = 0.0f;
	m_colorNewGreen = m_colorRefGreen;

	m_colorRefWater.r =  25.0f/256.0f;
	m_colorRefWater.g = 255.0f/256.0f;
	m_colorRefWater.b = 240.0f/256.0f;  // cyan
	m_colorRefWater.a = 0.0f;
	m_colorNewWater = m_colorRefWater;

	for ( j=0 ; j<26 ; j++ )
	{
		m_resume[j][0] = 0;
		m_sign[j][0] = 0;
	}
	m_author[0] = 0;

	if ( !file.Open(filename, "r") )  return;

	if ( RetEdit() )
	{
		m_fileBuffer->Open();
	}

	m_undo->Flush();

	rankBlupi = 0;
	pSel = 0;

	while ( file.GetLine(line, 1000) )
	{
		chapter = -1;

		if ( Cmd(line, "Environment") )
		{
			chapter = -1;
			m_edit->SetEnvironment(OpInt(line, "type",  0));
		}

		for ( j=0 ; j<26 ; j++ )
		{
			char	resume[1000];

			sprintf(op, "Resume.%c", 'A'+j);
			if ( Cmd(line, op) )
			{
				OpString(line, "text", resume);
				RemoveEscape(m_resume[j], resume);
			}
		}

		for ( j=0 ; j<26 ; j++ )
		{
			char	sign[1000];

			sprintf(op, "Sign.%c", 'A'+j);
			if ( Cmd(line, op) )
			{
				OpString(line, "text", sign);
				RemoveEscape(m_sign[j], sign);
			}
		}

		if ( Cmd(line, "Author") )
		{
			OpString(line, "name", m_author);
		}

		if ( Cmd(line, "EndingFile") )
		{
			chapter = -1;
			m_endingWinRank  = OpInt(line, "win",  0);
			m_endingLostRank = OpInt(line, "lost", 0);
		}

		if ( Cmd(line, "TrainerMode") )
		{
			chapter = 0;
			m_handleMode = OpInt(line, "handle",  1);
			m_adviseMode = OpInt(line, "advise",  0);
		}

		if ( Cmd(line, "MessageDelay") )
		{
			chapter = 0;
			m_displayText->SetDelay(OpFloat(line, "factor", 1.0f));
		}

		if ( Cmd(line, "Audio") )
		{
			chapter = 0;
			m_audioTrack = OpInt(line, "track", 0);
			m_bAudioRepeat = OpInt(line, "repeat", 1);
		}

		if ( Cmd(line, "AmbiantColor") )
		{
			chapter = 0;
			m_engine->SetAmbiantColor(OpColor(line, "air",   0x88888888), 0);
			m_engine->SetAmbiantColor(OpColor(line, "water", 0x88888888), 1);
		}

		if ( Cmd(line, "FogColor") )
		{
			D3DCOLOR	air, light, dark;
			chapter = 0;
			air   = OpColor(line, "air",   0x88888888);
			light = OpColor(line, "light", 0x88888888);
			dark  = OpColor(line, "dark",  0x88888888);
			if ( light == 0x88888888 )
			{
				m_engine->SetFogColor(air, 0);
				m_engine->SetFogColor(air, 1);
			}
			else
			{
				m_engine->SetFogColor(light, 0);
				m_engine->SetFogColor(dark,  1);
			}
		}

		if ( Cmd(line, "BotColor") )
		{
			chapter = 0;
			m_colorNewBot = RetColor(OpColor(line, "color", 0x88888888));
		}

		if ( Cmd(line, "GreeneryColor") )
		{
			chapter = 0;
			m_colorNewGreen = RetColor(OpColor(line, "color", 0x88888888));
		}

		if ( Cmd(line, "DeepView") )
		{
			chapter = 0;
			m_engine->SetDeepView(OpFloat(line, "air",   500.0f)*UNIT, 0, TRUE);
			m_engine->SetDeepView(OpFloat(line, "water", 100.0f)*UNIT, 1, TRUE);
		}

		if ( Cmd(line, "FogStart") )
		{
			chapter = 0;
			m_engine->SetFogStart(OpFloat(line, "air",   0.5f), 0);
			m_engine->SetFogStart(OpFloat(line, "water", 0.5f), 1);
		}

		if ( Cmd(line, "SecondTexture") )
		{
			chapter = 0;
			m_engine->SetSecondTexture(OpInt(line, "rank", 1));
		}

		if ( Cmd(line, "Background") )
		{
			chapter = 0;
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_engine->SetBackground(dir,
									OpColor(line, "up",        0x00000000),
									OpColor(line, "down",      0x00000000),
									OpFloat(line, "sunFlash", 0.5f),
									OpInt(line, "full", 0),
									OpInt(line, "quarter", 0),
									OpInt(line, "panel", 0));
		}

		if ( Cmd(line, "Planet") )
		{
			D3DVECTOR	ppos, uv1, uv2;

			chapter = 0;
			ppos  = OpPos(line, "pos");
			uv1   = OpPos(line, "uv1");
			uv2   = OpPos(line, "uv2");
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_planet->Create(OpInt(line, "mode", 0),
							 FPOINT(ppos.x, ppos.z),
							 OpFloat(line, "dim", 0.2f),
							 OpFloat(line, "speed", 0.0f),
							 OpFloat(line, "dir", 0.0f),
							 dir,
							 FPOINT(uv1.x, uv1.z),
							 FPOINT(uv2.x, uv2.z));
		}

		if ( Cmd(line, "LensFlare") )
		{
			chapter = 0;
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_engine->LensFlareAdd(dir,
								   OpFloat(line, "dist", 0.0f),
								   OpFloat(line, "dim", 0.3f),
								   OpFloat(line, "intensity", 1.0f),
								   OpFloat(line, "offsetH", 0.0f),
								   OpFloat(line, "offsetV", 0.0f));
		}

		if ( Cmd(line, "TerrainGenerate") )
		{
			chapter = 0;
			m_terrain->Generate(OpInt(line, "total", 20),
								OpFloat(line, "size", 2.0f)*UNIT);
		}

		if ( Cmd(line, "TerrainTextures") )
		{
			int		wall1[50];
			int		wall2[50];
			int		wall3[50];
			int		flat[50];
			int		hole[50];
			int		ground[50];
			char*	op;

			chapter = 0;

			op = SearchOp(line, "wall1");
			i = 0;
			while ( TRUE )
			{
				wall1[i+1] = GetInt(op, i, NAN);
				if ( wall1[i+1] == NAN )  break;
				i ++;
			}
			wall1[0] = i;

			op = SearchOp(line, "wall2");
			i = 0;
			while ( TRUE )
			{
				wall2[i+1] = GetInt(op, i, NAN);
				if ( wall2[i+1] == NAN )  break;
				i ++;
			}
			wall2[0] = i;

			op = SearchOp(line, "wall3");
			i = 0;
			while ( TRUE )
			{
				wall3[i+1] = GetInt(op, i, NAN);
				if ( wall3[i+1] == NAN )  break;
				i ++;
			}
			wall3[0] = i;

			op = SearchOp(line, "flat");
			i = 0;
			while ( TRUE )
			{
				flat[i+1] = GetInt(op, i, NAN);
				if ( flat[i+1] == NAN )  break;
				i ++;
			}
			flat[0] = i;

			op = SearchOp(line, "hole");
			i = 0;
			while ( TRUE )
			{
				hole[i+1] = GetInt(op, i, NAN);
				if ( hole[i+1] == NAN )  break;
				i ++;
			}
			hole[0] = i;

			op = SearchOp(line, "ground");
			i = 0;
			while ( TRUE )
			{
				ground[i+1] = GetInt(op, i, NAN);
				if ( ground[i+1] == NAN )  break;
				i ++;
			}
			ground[0] = i;

			OpString(line, "primary", name);
			OpString(line, "secondary", dir);
			m_terrain->InitTextures(name, dir,
									wall1, wall2, wall3, flat, hole, ground,
									OpInt(line, "depth", 1),
									OpFloat(line, "slope", 0.0f),
									OpInt(line, "model", 0),
									OpFloat(line, "mrv", 0.0f),
									OpFloat(line, "mrh", 0.0f),
									OpFloat(line, "mbv", 0.0f),
									OpFloat(line, "mbh", 0.0f),
									OpInt(line, "smooth", 1));
		}

		if ( Cmd(line, "TerrainHardness") )
		{
			float	h[8];
			chapter = 0;
			h[0] = OpFloat(line, "id0", 0.5f);
			h[1] = OpFloat(line, "id1", 0.5f);
			h[2] = OpFloat(line, "id2", 0.5f);
			h[3] = OpFloat(line, "id3", 0.5f);
			h[4] = OpFloat(line, "id4", 0.5f);
			h[5] = OpFloat(line, "id5", 0.5f);
			h[6] = OpFloat(line, "id6", 0.5f);
			h[7] = OpFloat(line, "id7", 0.5f);
			m_terrain->SetHardness(h);
		}

		if ( Cmd(line, "TerrainWind") )
		{
			chapter = -1;
			m_terrain->SetWind(OpPos(line, "speed"));
		}

		if ( Cmd(line, "TerrainInit") )
		{
			chapter = 0;
			m_terrain->ResInit(RetProto());
		}

		if ( Cmd(line, "TerrainTile") )
		{
			chapter = -1;
			pos = OpPos(line, "pos");
			pos *= UNIT;
			m_terrain->SetResource(pos, (TerrainRes)OpInt(line, "type", 0), TRUE);
		}

		if ( Cmd(line, "SignMark") )
		{
			chapter = -1;
			pos = OpPos(line, "pos");
			pos *= UNIT;
			pos.y = m_edit->RetGridHeight();
			m_terrain->SignMarkCreate(pos,
									  OpFloat(line, "angle", 0.0f)*PI/180.0f,
									  (ParticuleType)(PARTISIGNMARK0+OpInt(line, "type", 0)));
		}

		if ( Cmd(line, "TerrainWater") )
		{
			chapter = 1;
			OpString(line, "image", name);
			UserDir(dir, name, "");
			pos.x = OpFloat(line, "moveX", 0.0f);
			pos.y = OpFloat(line, "moveY", 0.0f);
			pos.z = pos.x;
			vortex.x = OpFloat(line, "vortexX", 0.0f);
			vortex.z = OpFloat(line, "vortexY", 0.0f);
			vortex.y = OpFloat(line, "vortexZ", 0.0f);
			m_water->Init(OpTypeWater(line, "air",   WATER_TO),
						  OpTypeWater(line, "water", WATER_TO),
						  dir,
						  RetColor(OpColor(line, "diffuse", 0xffffffff)),
						  RetColor(OpColor(line, "ambiant", 0xffffffff)),
						  OpFloat(line, "level", -12.0f)*UNIT,
						  OpFloat(line, "glint", 1.0f),
						  pos, vortex,
						  OpFloat(line, "pick", 0.0f),
						  OpFloat(line, "tension", 0.0f),
						  OpFloat(line, "shadow", 1.0f),
						  OpColor(line, "farColorLight", 0xffffffff),
						  OpColor(line, "farColorDark",  0xffffffff),
						  OpFloat(line, "farStart", 0.4f),
						  OpFloat(line, "farEnd",   0.8f),
						  (Meteo)OpInt(line, "meteo", 0),
						  OpInt(line, "bold", 0));
			m_water->Create();
			m_colorNewWater = RetColor(OpColor(line, "color", RetColor(m_colorRefWater)));
			m_colorShiftWater = OpFloat(line, "brightness", 0.0f);
		}

		if ( Cmd(line, "TerrainCloud") )
		{
			chapter = 1;
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_cloud->Create(dir,
							RetColor(OpColor(line, "diffuse", 0xffffffff)),
							RetColor(OpColor(line, "ambiant", 0xffffffff)),
							OpFloat(line, "level", 500.0f)*UNIT);
		}

		if ( Cmd(line, "TerrainBlitz") )
		{
			chapter = 1;
			m_blitz->Create(OpFloat(line, "sleep", 0.0f),
							OpFloat(line, "delay", 3.0f),
							OpFloat(line, "magnetic", 50.0f)*UNIT);
		}

		if ( Cmd(line, "TerrainCreate") )
		{
			chapter = 1;
			m_terrain->CreateObjects();
		}

		if ( Cmd(line, "AdditionalBlupi") )
		{
			chapter = 1;
			i = m_additionalTotal;
			if ( i < 10 )
			{
				m_additional[i][ADD_HAT  ] = OpInt(line, "hat", 0);
				m_additional[i][ADD_GLASS] = OpInt(line, "glass", 0);
				m_additional[i][ADD_GLOVE] = OpInt(line, "glove", 0);
				m_additional[i][ADD_SHOE ] = OpInt(line, "shoe", 0);
				m_additional[i][ADD_BAG  ] = OpInt(line, "bag", 0);
				m_additionalTotal ++;
			}
		}

		if ( Cmd(line, "Sunbeam") )
		{
			chapter = 1;
			CreateSunbeam(OpInt(line, "total", 0),
						  OpFloat(line, "min", 1.0f),
						  OpFloat(line, "max", 8.0f),
						  OpFloat(line, "desym", 30.0f)*PI/180.0f,
						  (ParticuleType)(PARTISUNBEAM0+OpInt(line, "type", 0)));
		}

		if ( Cmd(line, "BeginObject") )
		{
			chapter = 1;
		}

		if ( Cmd(line, "CreateObject") )
		{
			CObject*	pObj;
			CAuto*		pAuto;
			CPyro*		pyro;
			ObjectType	type;
			PyroType	pType;
			CameraType	cType;
			float		dir;
			char		op[20];
			char*		p;
			int			gadget, level, repeat, index;
			int			additional[10];

			chapter = -1;

			type = OpTypeObject(line, "type", OBJECT_NULL);
			if ( type == OBJECT_NULL )
			{
				type = (ObjectType)OpInt(line, "type", 0);
			}

			gadget = OpInt(line, "gadget", -1);
			if ( gadget == -1 )
			{
				gadget = 0;
				if ( type == OBJECT_BIRD  ||
					 type == OBJECT_PTERO ||
					 type == OBJECT_FISH  ||
					 type == OBJECT_SNAKE ||
					 type == OBJECT_SUBM  ||
					 type == OBJECT_JET   ||
					 (type >= OBJECT_PLANT5 && type <= OBJECT_PLANT9) )
				{
					gadget = 1;
				}
			}
			if ( !RetEdit() && gadget != 0 )  // est-ce un gadget ?
			{
				if ( m_engine->RetSetup(ST_DECOR) == 0.0f )  continue;
			}

			level = OpInt(line, "level", 0);
			if ( level > 0 && level !=  RetLevel() )  continue;
			if ( level < 0 && level == -RetLevel() )  continue;

			repeat = OpInt(line, "repeat", -1);
			if ( repeat != -1 && repeat != m_repeat )  continue;

			pos = OpPos(line, "pos");
			if ( OpInt(line, "adjust", 0) == 1 )
			{
				pos.x = pos.x*1.4f-64.0f;
				pos.z = pos.z*1.4f-108.0f;
			}
			pos *= UNIT;
			dir = OpFloat(line, "dir", 0.0f)*PI;

			if ( !RetEdit() && m_engine->RetSetup(ST_DECOR) == 0.0f &&
				 ((type >= OBJECT_PLANT0 && type <= OBJECT_PLANT19) ||
				  (type >= OBJECT_TREE0  && type <= OBJECT_TREE9  ) ))
			{
				if ( !m_terrain->IsSolid(pos) )  continue;
			}

			if ( m_additionalTotal == 0 )
			{
				for ( j=0 ; j<10 ; j++ )
				{
					additional[j] = 0;
				}
			}
			else
			{
				index = rankBlupi%m_additionalTotal;
				for ( j=0 ; j<10 ; j++ )
				{
					additional[j] = m_additional[index][j];
				}
			}

			pObj = CreateObject(pos, dir,
								OpFloat(line, "z", 1.0f),
								OpFloat(line, "h", 0.0f),
								type,
								OpInt(line, "option", 0),
								additional[ADD_HAT],
								additional[ADD_GLASS],
								additional[ADD_GLOVE],
								additional[ADD_SHOE],
								additional[ADD_BAG]);

			if ( pObj != 0 )
			{
				if ( type == OBJECT_BLUPI ||
					 type == OBJECT_CRAZY )
				{
					m_totalBlupi ++;
				}
				if ( type == OBJECT_GOAL )
				{
					m_totalGoal ++;
				}

				cType = OpCamera(line, "camera");
				if ( cType != CAMERA_NULL )
				{
					pObj->SetCameraType(cType);
				}
				pObj->SetCameraDist(OpFloat(line, "cameraDist", 50.0f));
				pObj->SetCameraLock(OpInt(line, "cameraLock", 0));

				pType = OpPyro(line, "pyro");
				if ( pType != PT_NULL )
				{
					pyro = new CPyro(m_iMan);
					pyro->Create(pType, pObj);
				}

				// Met les paramètres de la ligne de commande.
				p = SearchOp(line, "cmdline");
				for ( i=0 ; i<OBJECTMAXCMDLINE ; i++ )
				{
					float	value;
					value = GetFloat(p, i, NAN);
					if ( value == NAN )  break;
					pObj->SetCmdLine(i, value);
				}

				if ( OpInt(line, "select", 0) == 1 )
				{
					pSel = pObj;
				}

				pObj->SetSelectable(OpInt(line, "selectable", 1));
				if ( (type >= OBJECT_GROUND0  &&
					  type <= OBJECT_GROUND19 ) ||
					 (type >= OBJECT_PLANT0  &&
					  type <= OBJECT_PLANT19 ) ||
					 type == OBJECT_COLUMN1 ||
					 type == OBJECT_COLUMN2 ||
					 type == OBJECT_COLUMN3 ||
					 type == OBJECT_COLUMN4 ||
					 type == OBJECT_BIRD    ||
					 type == OBJECT_PTERO   ||
					 type == OBJECT_FISH    ||
					 type == OBJECT_SNAKE   ||
					 type == OBJECT_SUBM    ||
					 type == OBJECT_JET     )
				{
					pObj->SetEnable(OpInt(line, "enable", 0));
				}
				else
				{
					pObj->SetEnable(OpInt(line, "enable", 1));
				}
				pObj->SetShield(OpFloat(line, "shield", 1.0f));
				pObj->SetMagnifyDamage(OpFloat(line, "magnifyDamage", 1.0f));
				pObj->SetClip(OpInt(line, "clip", 1));

				motion = pObj->RetMotion();
				if ( motion != 0 )
				{
					p = SearchOp(line, "param");
					for ( i=0 ; i<10 ; i++ )
					{
						float	value;
						value = GetFloat(p, i, NAN);
						if ( value == NAN )  break;
						motion->SetParam(i, value);
					}
				}

				pAuto = pObj->RetAuto();
				if ( pAuto != 0 )
				{
					type = OpTypeObject(line, "autoType", OBJECT_NULL);
					pAuto->SetType(type);
					for ( i=0 ; i<5 ; i++ )
					{
						sprintf(op, "autoValue%d", i+1);  // autoValue1..autoValue5
						pAuto->SetValue(i, OpFloat(line, op, 0.0f));
					}
					OpString(line, "autoString", name);
					pAuto->SetString(name);

					i = OpInt(line, "run", -1);
					if ( i != -1 )
					{
						pAuto->Start(i);  // démarre le film
					}
				}
			}

			if ( type == OBJECT_BLUPI )  rankBlupi ++;
		}

		if ( Cmd(line, "AutoGen") )
		{
			CObject*	pObj;
			ObjectType	type, oType;
			float		dir;
			int			total, max;

			chapter = 2;

			if ( !RetEdit() && m_engine->RetSetup(ST_DECOR) != 0.0f )
			{
				type = OpTypeObject(line, "type", OBJECT_NULL);
				total = OpInt(line, "total", 0);
				i = max = 0;
				do
				{
					if ( max++ > 100 )  break;

					if ( type == OBJECT_BIRD  ||
						 type == OBJECT_PTERO ||
						 type == OBJECT_FISH  ||
						 type == OBJECT_SNAKE ||
						 type == OBJECT_SUBM  ||
						 type == OBJECT_JET   )
					{
						pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
					}
					else
					{
						if ( !m_water->SearchArea(pos, 6.0f) )  continue;
						if ( SearchObject(pos, 8.0f, TRUE) != 0 )  continue;
					}

					oType = type;
					if ( oType == OBJECT_SCRAP0 )
					{
						oType = (ObjectType)(OBJECT_SCRAP0+rand()%6);
					}
					dir = Rand()*PI*2.0f;
					pObj = CreateObject(pos, dir,
										OpFloat(line, "z", 1.0f),
										OpFloat(line, "h", 0.0f),
										oType,
										OpInt(line, "option", 0));
					if ( pObj != 0 )  pObj->SetEnable(FALSE);
					i ++;
				}
				while ( i < total );
			}
		}

		if ( Cmd(line, "CreateLimit") )
		{
			chapter = 2;
			CreateLimit(OpTypeObject(line, "type", OBJECT_NULL),
						OpInt(line, "max", 1));
		}

		if ( Cmd(line, "CreateFog") )
		{
			ParticuleType	type;
			FPOINT			dim;
			float			height, ddim, delay;

			chapter = 2;
			type = (ParticuleType)(PARTIFOG0+OpInt(line, "type", 0));
			pos = OpPos(line, "pos")*UNIT;
			height = OpFloat(line, "height", 1.0f)*UNIT;
			ddim = OpFloat(line, "dim", 50.0f)*UNIT;
			delay = OpFloat(line, "delay", 2.0f);
			m_terrain->MoveOnFloor(pos);
			pos.y += height;
			dim.x = ddim;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, type, delay, 0.0f);
		}

		if ( Cmd(line, "CreateLight") )
		{
			D3DTypeObj	type;

			chapter = 2;
			color.r = 0.5f;
			color.g = 0.5f;
			color.b = 0.5f;
			color.a = 1.0f;
			obj = CreateLight(OpDir(line, "dir"),
							  OpColorValue(line, "color", color));

			type = OpTypeTerrain(line, "type", TYPENULL);
			if ( type == TYPETERRAIN )
			{
				m_light->SetLightIncluType(obj, TYPETERRAIN);
			}
			if ( type == TYPEQUARTZ )
			{
				m_light->SetLightIncluType(obj, TYPEQUARTZ);
			}
			if ( type == TYPEMETAL )
			{
				m_light->SetLightIncluType(obj, TYPEMETAL);
			}
			if ( type == TYPEBLUPI )
			{
				m_light->SetLightIncluType(obj, TYPEBLUPI);
			}
			if ( type == TYPEOBJECT )
			{
				m_light->SetLightExcluType(obj, TYPEBLUPI);
			}
		}
		if ( Cmd(line, "CreateSpot") )
		{
			D3DTypeObj	type;

			chapter = 2;
			color.r = 0.5f;
			color.g = 0.5f;
			color.b = 0.5f;
			color.a = 1.0f;
			obj = CreateSpot(OpDir(line, "pos")*UNIT,
							 OpColorValue(line, "color", color));

			type = OpTypeTerrain(line, "type", TYPENULL);
			if ( type == TYPETERRAIN )
			{
				m_light->SetLightIncluType(obj, TYPETERRAIN);
			}
			if ( type == TYPEQUARTZ )
			{
				m_light->SetLightIncluType(obj, TYPEQUARTZ);
			}
			if ( type == TYPEMETAL )
			{
				m_light->SetLightIncluType(obj, TYPEMETAL);
			}
			if ( type == TYPEBLUPI )
			{
				m_light->SetLightIncluType(obj, TYPEBLUPI);
			}
			if ( type == TYPEOBJECT )
			{
				m_light->SetLightExcluType(obj, TYPEBLUPI);
			}
		}

		if ( Cmd(line, "GroundSpot") )
		{
			int		rank;

			chapter = 2;
			rank = m_engine->GroundSpotCreate();
			if ( rank != -1 )
			{
				m_engine->SetObjectGroundSpotPos(rank, OpPos(line, "pos")*UNIT);
				m_engine->SetObjectGroundSpotRadius(rank, OpFloat(line, "radius", 10.0f)*UNIT);
				m_engine->SetObjectGroundSpotColor(rank, RetColor(OpColor(line, "color", 0x88888888)));
				m_engine->SetObjectGroundSpotSmooth(rank, OpFloat(line, "smooth", 1.0f));
				m_engine->SetObjectGroundSpotMinMax(rank, OpFloat(line, "min", 0.0f)*UNIT,
														  OpFloat(line, "max", 0.0f)*UNIT);
			}
		}

		if ( Cmd(line, "WaterColor") )
		{
			chapter = 2;
			color.r = 0.0f;
			color.g = 0.0f;
			color.b = 0.0f;
			color.a = 1.0f;
			m_engine->SetWaterAddColor(OpColorValue(line, "color", color));
		}

		if ( Cmd(line, "MapColor") )
		{
			chapter = 2;
			m_map->FloorColorMap(RetColor(OpColor(line, "floor", 0x88888888)),
								 RetColor(OpColor(line, "water", 0x88888888)));
		}
		if ( Cmd(line, "MapZoom") )
		{
			chapter = 2;
			m_map->ZoomMap(OpFloat(line, "factor", 2.0f));
			m_map->MapEnable(OpInt(line, "enable", 1));
		}

		if ( Cmd(line, "EnableBuild") )
		{
			chapter = 2;
			g_build |= OpBuild(line, "type");
		}

		if ( Cmd(line, "EnableResearch") )
		{
			chapter = 2;
			g_researchEnable |= OpResearch(line, "type");
		}
		if ( Cmd(line, "DoneResearch") )  // pas loading file ?
		{
			chapter = 2;
			g_researchDone |= OpResearch(line, "type");
		}

		if ( RetEdit() && chapter != -1 )
		{
			m_fileBuffer->PutLine(chapter, line);
		}
	}

	file.Close();

	if ( m_author[0] == 0 )  // pas d'auteur ?
	{
		strcpy(m_author, m_gamerName);
	}

	ChangeColor();  // change les couleurs des textures

	m_terrain->SignMarkShow(RetEdit()||RetTest());

	m_engine->TimeInit();
	m_engine->FlushPressKey();
	m_time = 0.0f;
	m_gameTime = m_dialog->RetGamerTotalTime();
	m_checkEndTime = -10.0f;

	if ( pSel != 0 && !RetEdit() )
	{
		SelectObject(pSel);
	}

	InitEye();
	StartLift();  // démarre les ascenseurs
}

// Crée un objet du décor mobile ou fixe.

CObject* CRobotMain::CreateObject(D3DVECTOR pos, float angle, float zoom,
								  float height, ObjectType type, int option,
								  int addHat, int addGlass,
								  int addGlove, int addShoe, int addBag)
{
	CObject*	pObject;

	if ( type == OBJECT_NULL )  return 0;

	pObject = new CObject(m_iMan);
	pObject->CreateObject(pos, angle, zoom, height, type, option,
						  addHat, addGlass, addGlove, addShoe, addBag);

	return pObject;
}

// Limite la quantité d'un objet quelconque en détruisant qq objets.

void CRobotMain::CreateLimit(ObjectType type, int max)
{
	CObject*	list[100];
	CObject*	pObj;
	int			i, j, index;

	index = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetType() != type )  continue;

		list[index++] = pObj;
		if ( index >= 100 )  break;
	}
	if ( index <= max )  return;

	for ( i=0 ; i<index-max ; i++ )
	{
		do
		{
			j = rand()%index;
		}
		while ( list[j] == 0 );

		list[j]->DeleteObject(TRUE);  // détruit rapidement
		delete list[j];
		list[j] = 0;
	}
}


// Retourne un paramètre additionnel pour blupi.

int CRobotMain::RetAdditional(int rank, int type)
{
	int		i;

	if ( m_additionalTotal == 0 )  return 0;

	i = rank%m_additionalTotal;
	return m_additional[i][type];
}


// Vérifie si le puzzle est plausible.

int CRobotMain::CheckPuzzle()
{
	return m_edit->CheckPuzzle();
}

// Ecrit la scene.

BOOL CRobotMain::WriteScene()
{
	CCryptFile		file;
	CObject*		pObj;
	D3DVECTOR		pos;
	TerrainRes		res;
	ObjectType		type;
	ParticuleType	pType;
	float			dimTile, angle, zoom;
	int				i, x, y, env, nbTiles, option, pass, hope;
	char			filename[100];
	char			resume[1000];
	char			sign[1000];
	char			line[1000];
	char			cmd1[100];
	char			cmd2[100];
	char			cmd3[100];
	BOOL			bSelect;

	// Ouvre le fichier.
	m_dialog->BuildSceneName(filename);
	if ( !file.Open(filename, "w") )  return FALSE;

	// Ecrit la préface.
	env = m_edit->RetEnvironment();
	sprintf(line, "Environment type=%d\n", env);
	file.PutLine(line);

	for ( i=0 ; i<26 ; i++ )
	{
		if ( m_resume[i][0] == 0 )  continue;
		InsertEscape(resume, m_resume[i]);
		sprintf(line, "Resume.%c text=\"%s\"\n", 'A'+i, resume);
		file.PutLine(line);
	}
	for ( i=0 ; i<26 ; i++ )
	{
		if ( m_sign[i][0] == 0 )  continue;
		InsertEscape(sign, m_sign[i]);
		sprintf(line, "Sign.%c text=\"%s\"\n", 'A'+i, sign);
		file.PutLine(line);
	}
	if ( m_author[0] != 0 )
	{
		sprintf(line, "Author name=\"%s\"\n", m_author);
		file.PutLine(line);
	}
	file.PutLine("\n");

	// Ecrit les tokens de l'en-tête.
	for ( i=0 ; i<100 ; i++ )
	{
		if ( !m_fileBuffer->GetLine(0, i, line) )  break;
		file.PutLine(line);
	}
	file.PutLine("\n");

	// Ecrit le terrain.
	m_terrain->RestoreResourceCopy();
	dimTile = m_terrain->RetDimTile();
	nbTiles = m_terrain->RetNbTiles();
	for ( y=0 ; y<nbTiles ; y++ )
	{
		for ( x=0 ; x<nbTiles ; x++ )
		{
			res = m_terrain->RetResource(x,y);
			if ( res == TR_SPACE )  continue;

			if ( x == 0 || x == nbTiles-1 ||
				 y == 0 || y == nbTiles-1 )  // sur un bord ?
			{
				if ( res == TR_HOLE )  continue;
			}				 

			pos.x = dimTile*(x-nbTiles/2);
			pos.z = dimTile*(y-nbTiles/2);

			sprintf(line, "TerrainTile pos=%d;%d type=%d\n",
							(int)(pos.x/UNIT), (int)(pos.z/UNIT), res);
			file.PutLine(line);
		}
	}
	file.PutLine("\n");

	// Ecrit les marques des indices.
	for ( i=0 ; i<MAXSIGNMARK ; i++ )
	{
		if ( m_terrain->SignMarkGet(i, pos, angle, pType) )
		{
			sprintf(line, "SignMark pos=%d;%d angle=%.2f type=%d\n",
							(int)(pos.x/UNIT), (int)(pos.z/UNIT),
							angle*180.0f/PI, pType-PARTISIGNMARK0);
			file.PutLine(line);
		}
	}
	file.PutLine("\n");

	// Ecrit les tokens intermédiaires.
	for ( i=0 ; i<100 ; i++ )
	{
		if ( !m_fileBuffer->GetLine(1, i, line) )  break;
		file.PutLine(line);
	}
	file.PutLine("\n");

	// Ecrit les objets.
	bSelect = TRUE;
	for ( pass=0 ; pass<5 ; pass++ )
	{
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;

			pos    = pObj->RetPosition(0);
//?			pos = Grid(pos, 8.0f);
			angle  = pObj->RetAngleY(0);
			zoom   = pObj->RetZoomY(0);
			type   = pObj->RetType();
			option = pObj->RetOption();

			if ( type == OBJECT_LIFT )  continue;

			hope = 4;

			if ( type >= OBJECT_COLUMN1 &&
				 type <= OBJECT_COLUMN4 )
			{
				hope = 0;
			}
			if ( type >= OBJECT_GROUND0  &&
				 type <= OBJECT_GROUND19 )
			{
				hope = 1;
			}
			if ( type == OBJECT_BLUPI ||
				 type == OBJECT_CRAZY )
			{
				hope = 2;
			}
			if ( type >= OBJECT_BOX1  &&
				 type <= OBJECT_BOX20 )
			{
				hope = 3;
			}
			if ( type >= OBJECT_KEY1 &&
				 type <= OBJECT_KEY5 )
			{
				hope = 3;
			}

			if ( hope != pass )  continue;


			cmd1[0] = 0;
			cmd2[0] = 0;
			cmd3[0] = 0;

			if ( zoom != 1.0f )
			{
				sprintf(cmd1, " z=%.2f", zoom);
			}

			if ( option != 0 )
			{
				sprintf(cmd2, " option=%d", option);
			}

			if ( type == OBJECT_BLUPI && bSelect )
			{
				strcpy(cmd3, " select=1");
				bSelect = FALSE;
			}

			sprintf(line, "CreateObject pos=%d;%d dir=%.2f type=%d%s%s%s\n",
							(int)(pos.x/UNIT), (int)(pos.z/UNIT),
							angle/PI, type, cmd1, cmd2, cmd3);
			file.PutLine(line);
		}
	}
	file.PutLine("\n");

	// Ecrit les tokens finaux.
	for ( i=0 ; i<100 ; i++ )
	{
		if ( !m_fileBuffer->GetLine(2, i, line) )  break;
		file.PutLine(line);
	}

	file.Close();
	return TRUE;
}


// Crée le modèle éditable.

void CRobotMain::CreateModel(int type)
{
	D3DVECTOR		direction;
	D3DCOLORVALUE	color;

	if ( type == 0 )  // édite ?
	{
		m_engine->SetAmbiantColor(RetColor(100,100,50));  // gris
		m_engine->SetBackground("", 0x80808080, 0x80808080);
		m_engine->SetFogColor(0x80808080);
		m_engine->SetDeepView(500.0f, 0);
		m_engine->SetDeepView(100.0f, 1);
		m_engine->SetFogStart(0.5f);

		m_model->StartUserAction();

		direction = D3DVECTOR(1.0f, -1.0f, 1.0f);
		color.r = 0.7f;
		color.g = 0.7f;
		color.b = 0.7f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(-1.0f, -1.0f, 1.0f);
		color.r = 0.7f;
		color.g = 0.7f;
		color.b = 0.7f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(1.0f, -1.0f, -1.0f);
		color.r = 0.7f;
		color.g = 0.7f;
		color.b = 0.7f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(-1.0f, -1.0f, -1.0f);
		color.r = 0.7f;
		color.g = 0.7f;
		color.b = 0.7f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(0.0f, 1.0f, 0.0f);
		color.r = 0.7f;
		color.g = 0.7f;
		color.b = 0.7f;  // blanc
		CreateLight(direction, color);
	}

	if ( type == 1 )  // icône ?
	{
		m_engine->SetAmbiantColor(RetColor(100,100,50));  // gris
		m_engine->SetBackground("", 0xffffffff, 0xffffffff);
		m_engine->SetFogColor(0x80808080);
		m_engine->SetDeepView(500.0f, 0);
		m_engine->SetDeepView(100.0f, 1);
		m_engine->SetFogStart(0.5f);

		m_model->StartUserAction();

		direction = D3DVECTOR(-1.0f, -1.0f, 0.0f);
		color.r = 0.8f;
		color.g = 0.8f;
		color.b = 0.8f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(0.0f, -1.0f, 1.0f);
		color.r = 0.6f;
		color.g = 0.6f;
		color.b = 0.6f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(1.0f, -1.0f, 0.0f);
		color.r = 0.2f;
		color.g = 0.2f;
		color.b = 0.2f;  // blanc
		CreateLight(direction, color);

		direction = D3DVECTOR(0.0f, -1.0f, -1.0f);
		color.r = 0.4f;
		color.g = 0.4f;
		color.b = 0.4f;  // blanc
		CreateLight(direction, color);
	}

	InitEye();

	m_engine->TimeInit();
	m_time = 0.0f;
	m_gameTime = 0.0f;
	m_checkEndTime = -10.0f;
}


// Crée les rayons de soleil.

void CRobotMain::CreateSunbeam(int total, float min, float max,
							   float desym, ParticuleType type)
{
	D3DVECTOR	pos, corner;
	float		length, dim, angle;
	int			i, create;

	length = m_terrain->RetDim();

	create = 0;
	for ( i=0 ; i<1000 ; i++ )
	{
		pos.x = Rand()*length-length/2.0f;
		pos.y = 0.0f;
		pos.z = Rand()*length-length/2.0f;
		if ( m_terrain->RetFloorLevel(pos) >= 0.0f )  continue;

		dim = min+Rand()*(max-min);

		corner.x = pos.x+dim;
		corner.z = pos.z+dim;
		if ( m_terrain->RetFloorLevel(pos) >= 0.0f )  continue;

		corner.x = pos.x+dim;
		corner.z = pos.z-dim;
		if ( m_terrain->RetFloorLevel(pos) >= 0.0f )  continue;

		corner.x = pos.x-dim;
		corner.z = pos.z+dim;
		if ( m_terrain->RetFloorLevel(pos) >= 0.0f )  continue;

		corner.x = pos.x-dim;
		corner.z = pos.z-dim;
		if ( m_terrain->RetFloorLevel(pos) >= 0.0f )  continue;

		angle = (Rand()-0.5f)*2.0f*desym;
		if ( !m_particule->CreateSunbeam(pos, dim, angle, type) )  break;
		create ++;

		if ( create >= total )  break;
	}
}

// Crée une lumière directionnelle.

int CRobotMain::CreateLight(D3DVECTOR direction, D3DCOLORVALUE color)
{
	D3DLIGHT7	light;
	int			obj;

	if ( direction.x == 0.0f &&
		 direction.y == 0.0f &&
		 direction.z == 0.0f )
	{
		direction.y = -1.0f;
	}

	ZeroMemory(&light, sizeof(D3DLIGHT7));
	light.dltType      = D3DLIGHT_DIRECTIONAL;
	light.dcvDiffuse.r = color.r;
	light.dcvDiffuse.g = color.g;
	light.dcvDiffuse.b = color.b;
	light.dvDirection  = direction;
	obj = m_light->CreateLight();
	m_light->SetLight(obj, light);

	return obj;
}

// Crée une lumière spot.

int CRobotMain::CreateSpot(D3DVECTOR pos, D3DCOLORVALUE color)
{
	D3DLIGHT7	light;
	int			obj;

	pos.y += m_terrain->RetFloorLevel(pos);

	ZeroMemory(&light, sizeof(D3DLIGHT7));
	light.dltType        = D3DLIGHT_SPOT;
	light.dcvDiffuse.r   = color.r;
	light.dcvDiffuse.g   = color.g;
	light.dcvDiffuse.b   = color.b;
	light.dvPosition     = pos;
	light.dvDirection    = D3DVECTOR(0.0f, -1.0f, 0.0f);
	light.dvRange        = D3DLIGHT_RANGE_MAX;
	light.dvFalloff      = 1.0f;
	light.dvTheta        = 10.0f*PI/180.0f;
	light.dvPhi          = 90.0f*PI/180.0f;
	light.dvAttenuation0 = 2.0f;
	light.dvAttenuation1 = 0.0f;
	light.dvAttenuation2 = 0.0f;
	obj = m_light->CreateLight();
	m_light->SetLight(obj, light);

	return obj;
}


// Change les couleurs des textures.

void CRobotMain::ChangeColor()
{
	D3DCOLORVALUE	colorRef1, colorNew1, colorRef2, colorNew2;
	FPOINT			ts, ti;
	FPOINT			exclu[6];

	colorRef1.a = 0.0f;
	colorRef2.a = 0.0f;
	colorNew1.a = 0.0f;
	colorNew2.a = 0.0f;

	ts = FPOINT(0.0f, 0.0f);
	ti = FPOINT(1.0f, 1.0f);  // toute l'image

	colorRef2.r = 0.0f;
	colorRef2.g = 0.0f;
	colorRef2.b = 0.0f;
	colorNew2.r = 0.0f;
	colorNew2.g = 0.0f;
	colorNew2.b = 0.0f;

	colorRef2.r = 0.0f;
	colorRef2.g = 0.0f;
	colorRef2.b = 0.0f;
	colorNew2.r = 0.0f;
	colorNew2.g = 0.0f;
	colorNew2.b = 0.0f;

	m_engine->ChangeColor("bot1.tga", m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, TRUE);

	exclu[0] = FPOINT( 95.0f/256.0f,   0.0f/256.0f);
	exclu[1] = FPOINT(195.0f/256.0f, 100.0f/256.0f);  // grillage
	exclu[2] = FPOINT(0.0f, 0.0f);
	exclu[3] = FPOINT(0.0f, 0.0f);  // terminateur
	m_engine->ChangeColor("plant.tga", m_colorRefGreen, m_colorNewGreen, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti, exclu);

	// PARTIFLIC, PARTIPLOUF0, PARTIDROP et PARTIRAIN:
	ts = FPOINT(0.250f, 0.500f);
	ti = FPOINT(0.875f, 0.750f);
	m_engine->ChangeColor("effect00.tga", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, TRUE);
}


// Démarre tous les ascenseurs. Le premier ascenseur démarré va
// s'occuper des mouvements de la caméra pour le film initial.

void CRobotMain::StartLift()
{
	CObject*	pObj;
	CAuto*		pAuto;
	int			i, rank;

	rank = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetType() == OBJECT_LIFT )
		{
			pAuto = pObj->RetAuto();
			if ( pAuto != 0 )
			{
				pAuto->Start(rank++);
			}
		}
	}
}

// Cherche un objet quelconque.

CObject* CRobotMain::SearchObject(D3DVECTOR center, float radius, BOOL bAll)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?
		if ( !bAll && !pObj->RetEnable() )  continue;

		type = pObj->RetType();

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Vérifie si la mission est terminée.

Error CRobotMain::CheckEndMission(BOOL bFrame)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	ObjectType	type;
	int			i, nbBlupi, nbGoal, nbWin;

	if ( RetEdit()  )  return ERR_MISSION_NOTERM;
	if ( RetProto() )  return ERR_MISSION_NOTERM;

	nbBlupi = 0;
	nbWin   = 0;
	nbGoal  = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();

		if ( type == OBJECT_CRAZY )
		{
			if ( !pObj->RetEnable() )  continue;
		}

		if ( type == OBJECT_BLUPI ||
			 type == OBJECT_CRAZY )
		{
			nbBlupi ++;
			pos = pObj->RetPosition(0);
			if ( pos.y >= 10000.0f )  nbWin ++;
		}
		if ( type == OBJECT_GOAL )
		{
			nbGoal ++;
		}
	}

	nbGoal += nbWin;  // car les ballons ont été détruits !

	if ( nbBlupi > nbGoal       ||  // plus assez de ballons ?
		 nbBlupi < m_totalBlupi )   // plus assez de blupi ?
	{
		m_displayText->DisplayError(INFO_LOST, 1000.0f);
		m_lostDelay = 0.1f;  // perdu dans 0.1 seconde
		m_winDelay  = 0.0f;
		m_displayText->SetEnable(FALSE);
		return INFO_LOST;
	}

	if ( nbWin >= m_totalBlupi )  // tous les blupi ont trouvé les ballons ?
	{
		m_winDelay  = 0.1f;  // gagné dans 0.1 seconde
		m_lostDelay = 0.0f;
		m_displayText->SetEnable(FALSE);
		return ERR_OK;  // mission terminée
	}

	return ERR_MISSION_NOTERM;
}



// Indique si la scène est fixe, sans interraction.

BOOL CRobotMain::RetFixScene()
{
	return (m_scene == SCENE_FIX);
}

// Indique s'il faut utiliser toutes les poignées-actions.

int CRobotMain::RetHandleMove()
{
	return m_handleMode;
}


// Retourne le résumé pour une langue donnée.

void CRobotMain::SetResume(int language, char *text)
{
	strcpy(m_resume[language-'A'], text);
}

// Retourne le résumé dans une langue donnée.

char* CRobotMain::RetResume(int language)
{
	return m_resume[language-'A'];
}

// Retourne le résumé dans la langue courante, ou dans une autre
// langue existante.

char* CRobotMain::RetResume()
{
	int		i;
	char	language;

	language = RetLanguageLetter();
	if ( language >= 'A' && language <= 'Z' )
	{
		if ( m_resume[language-'A'][0] != 0 )  // existe dans langue courante ?
		{
			return m_resume[language-'A'];
		}
	}

	for ( i=0 ; i<26 ; i++ )
	{
		if ( m_resume[i][0] != 0 )  // existe dans autre langue ?
		{
			return m_resume[i];
		}
	}

	return m_resume[0];
}

// Retourne le résumé pour une langue donnée.

void CRobotMain::SetSign(int language, char *text)
{
	strcpy(m_sign[language-'A'], text);
}

// Retourne le résumé dans une langue donnée.

char* CRobotMain::RetSign(int language)
{
	return m_sign[language-'A'];
}

// Retourne le résumé dans la langue courante, ou dans une autre
// langue existante.

char* CRobotMain::RetSign()
{
	int		i;
	char	language;

	language = RetLanguageLetter();
	if ( language >= 'A' && language <= 'Z' )
	{
		if ( m_sign[language-'A'][0] != 0 )  // existe dans langue courante ?
		{
			return m_sign[language-'A'];
		}
	}

	for ( i=0 ; i<26 ; i++ )
	{
		if ( m_sign[i][0] != 0 )  // existe dans autre langue ?
		{
			return m_sign[i];
		}
	}

	return m_sign[0];
}

// Gestion de l'auteur.

void CRobotMain::SetAuthor(char *text)
{
	strcpy(m_author, text);
}

char* CRobotMain::RetAuthor()
{
	return m_author;
}


BOOL CRobotMain::RetEdit()
{
	return m_dialog->RetEdit();
}

BOOL CRobotMain::RetTest()
{
	return m_dialog->RetTest();
}

BOOL CRobotMain::RetProto()
{
	return m_dialog->RetProto();
}

BOOL CRobotMain::RetAgain()
{
	return m_dialog->RetAgain();
}

BOOL CRobotMain::RetShowAll()
{
	return m_bShowAll;
}

char* CRobotMain::RetSavegameDir()
{
	return m_dialog->RetSavegameDir();
}

char* CRobotMain::RetPublicDir()
{
	return m_dialog->RetPublicDir();
}

char* CRobotMain::RetFilesDir()
{
	return m_dialog->RetFilesDir();
}

int CRobotMain::RetLevel()
{
	return m_dialog->RetLevel();
}

int CRobotMain::RetTotalBlupi()
{
	return m_totalBlupi;
}

int CRobotMain::RetTotalGoal()
{
	return m_totalGoal;
}

BOOL CRobotMain::RetCheatUsed()
{
	return m_bCheatUsed;
}


// Change le nom du joueur.

void CRobotMain::SetGamerName(char *name)
{
	strcpy(m_gamerName, name);
	SetGlobalGamerName(m_gamerName);
}

// Donne le nom du joueur.

char* CRobotMain::RetGamerName()
{
	return m_gamerName;
}


// Indique une manipulation de plus.

void CRobotMain::IncTotalManip(int manip)
{
	m_totalManip += manip;
}

// Spécifie le nombre de manipulations.

void CRobotMain::SetTotalManip(int manip)
{
	m_totalManip = manip;
}

// Retourne le nombre de manipulations.

int CRobotMain::RetTotalManip()
{
	return m_totalManip;
}



// Change le mode de pause.

void CRobotMain::ChangePause(BOOL bPause)
{
	CGroup*		pg;

	m_bPause = bPause;
	m_engine->SetPause(m_bPause);

	m_sound->MuteAll(m_bPause);
	if ( m_bPause )  HiliteClear();

	pg = (CGroup*)m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
	if ( pg != 0 )
	{
		pg->SetState(STATE_VISIBLE, m_bPause);
	}
}


// Change la vitesse du jeu.

void CRobotMain::SetSpeed(float speed)
{
	CButton*	pb;
	char		text[10];

	m_engine->SetSpeed(speed);

	pb = (CButton*)m_interface->SearchControl(EVENT_SPEED);
	if ( pb != 0 )
	{
		if ( speed == 1.0f )
		{
			pb->ClearState(STATE_VISIBLE);
		}
		else
		{
			sprintf(text, "x%.1f", speed);
			pb->SetName(text);
			pb->SetState(STATE_VISIBLE);
		}
	}
}

float CRobotMain::RetSpeed()
{
	return m_engine->RetSpeed();
}


// Met à jour la carte.

void CRobotMain::UpdateMap()
{
	m_map->UpdateMap();
}

// Indique si la mini-carte est visible.

BOOL CRobotMain::RetShowMap()
{
	return m_map->RetShowMap();
}


// Gestion du mode de blocage pendant les films.

void CRobotMain::SetMovieLock(BOOL bLock, BOOL bFinal)
{
	m_bMovieLock = bLock;
	m_bMovieFinal = bFinal;
	m_engine->SetMovieLock(m_bMovieLock);

	m_map->ShowMap(!m_bMovieLock);
	if ( m_bMovieLock )  HiliteClear();
	m_engine->SetMouseHide(m_bMovieLock);
}

BOOL CRobotMain::RetMovieLock()
{
	return m_bMovieLock;
}

BOOL CRobotMain::RetFreePhoto()
{
	return m_bFreePhoto;
}


// Indique si la souris vise un objet ami, sur lequel il ne faut
// pas tirer.

void CRobotMain::SetFriendAim(BOOL bFriend)
{
	m_bFriendAim = bFriend;
}

BOOL CRobotMain::RetFriendAim()
{
	return m_bFriendAim;
}


// Débute la musique d'une mission.

void CRobotMain::StartMusic()
{
	if ( m_audioTrack != 0 )
	{
		m_sound->StopMusic();
		m_sound->PlayMusic(m_audioTrack, m_bAudioRepeat);
	}
}

// Enlève tooltip.

void CRobotMain::ClearInterface()
{
	HiliteClear();  // enlève la mise en évidence
	m_tooltipName[0] = 0;  // enlève vraiment le tooltip
}


