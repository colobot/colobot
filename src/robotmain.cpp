// robotmain.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "cbot/cbotdll.h"
#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "language.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "recorder.h"
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "cloud.h"
#include "blitz.h"
#include "planet.h"
#include "object.h"
#include "motion.h"
#include "motionhuman.h"
#include "physics.h"
#include "brain.h"
#include "pyro.h"
#include "modfile.h"
#include "model.h"
#include "camera.h"
#include "task.h"
#include "auto.h"
#include "text.h"
#include "interface.h"
#include "map.h"
#include "label.h"
#include "button.h"
#include "gauge.h"
#include "geiger.h"
#include "progress.h"
#include "group.h"
#include "window.h"
#include "edit.h"
#include "array.h"
#include "sound.h"
#include "displaytext.h"
#include "cbottoken.h"
#include "cmdtoken.h"
#include "mainmovie.h"
#include "maindialog.h"
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
	int			iValue, i;
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
	m_movie       = new CMainMovie(m_iMan);
	m_dialog      = new CMainDialog(m_iMan);
	m_map         = new CMainMap(m_iMan);

	m_engine->SetTerrain(m_terrain);
	m_filesDir = m_dialog->RetFilesDir();

	m_time = 0.0f;
	m_gameTime = 0.0f;
	m_recordTime = 0.0f;
	m_checkEndTime = -10.0f;

	m_phase       = PHASE_NAME;
	m_cameraRank  = -1;
	m_audioTrack  = 0;
	m_bAudioRepeat = TRUE;
	m_selectObject = 0;
	m_carState     = 0;
	m_geiger       = 0;

	m_movieTotal = 0;
	m_movieIndex = 0;

	m_bMovieLock    = FALSE;
	m_bPause        = FALSE;
	m_bFreePhoto    = FALSE;
	m_bShowPos      = FALSE;
	m_bSelectInsect = FALSE;
	m_bShowAll      = FALSE;
	m_bCheatRadar   = FALSE;
	m_bFixScene     = FALSE;
	m_bTrainerPilot = FALSE;
	m_bSuspend      = FALSE;
	m_bFriendAim    = FALSE;
	m_bSuperWin     = FALSE;
	m_bSuperLost    = FALSE;
	m_bLostLookat   = FALSE;
	m_bSkipFrame    = FALSE;
	m_bDisplayInfo  = FALSE;
	m_bStopwatch    = TRUE;
	m_bCheatUsed    = FALSE;
	m_movieAuto     = 0;
	m_decorStamp    = 0;

	m_engine->SetMovieLock(m_bMovieLock);

	m_movie->Flush();
	m_movieInfoIndex = -1;

	m_tooltipPos = FPOINT(0.0f, 0.0f);
	m_tooltipName[0] = 0;
	m_tooltipTime = 0.0f;

	m_endingWinRank   = 0;
	m_endingLostRank  = 0;
	m_bWinTerminate   = FALSE;
	m_messageTime     = 0.0f;

	m_fontSize  = 9.0f;
	m_windowPos = FPOINT(0.15f, 0.17f);
	m_windowDim = FPOINT(0.70f, 0.66f);

	if ( GetProfileFloat("Edit", "FontSize",    fValue) )  m_fontSize    = fValue;
	if ( GetProfileFloat("Edit", "WindowPos.x", fValue) )  m_windowPos.x = fValue;
	if ( GetProfileFloat("Edit", "WindowPos.y", fValue) )  m_windowPos.y = fValue;
	if ( GetProfileFloat("Edit", "WindowDim.x", fValue) )  m_windowDim.x = fValue;
	if ( GetProfileFloat("Edit", "WindowDim.y", fValue) )  m_windowDim.y = fValue;
	
	m_IOPublic = FALSE;
	m_IODim = FPOINT(320.0f/640.0f, (121.0f+18.0f*8)/480.0f);
	m_IOPos.x = (1.0f-m_IODim.x)/2.0f;  // au milieu
	m_IOPos.y = (1.0f-m_IODim.y)/2.0f;
	
	if ( GetProfileInt  ("Edit", "IOPublic", iValue) )  m_IOPublic    = iValue;
	if ( GetProfileFloat("Edit", "IOPos.x",  fValue) )  m_IOPos.x = fValue;
	if ( GetProfileFloat("Edit", "IOPos.y",  fValue) )  m_IOPos.y = fValue;
	if ( GetProfileFloat("Edit", "IODim.x",  fValue) )  m_IODim.x = fValue;
	if ( GetProfileFloat("Edit", "IODim.y",  fValue) )  m_IODim.y = fValue;
	
	InitEye();

	g_id = 0;
	g_build = 0;
	g_researchDone = 0;  // aucune recherche effectuée
	g_researchEnable = 0;

	m_starterTry  = 0;
	m_starterLast = 0;

	m_repeat = 0;
	m_lastRank = -1;

	m_gamerName[0] = 0;
	GetProfileString("Gamer", "LastName", m_gamerName, 100);
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

	m_recorderRecord = 0;
	m_recorderPlay   = 0;

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
	delete m_movie;
	delete m_dialog;
	delete m_map;
	delete m_terrain;
	delete m_model;
}


// Crée le fichier buzzingcars.ini la première fois.

void CRobotMain::CreateIni()
{
	int		iValue;

	// buzzingcars.ini inexistant ?
	if ( !GetProfileInt("Setup", "TotoMode", iValue) )
	{
		m_dialog->SetupMemorize();
	}
}


// Change de phase.

void CRobotMain::ChangePhase(Phase phase)
{
	CEdit*			pe;
	CButton*		pb;
	CLabel*			pl;
	CGauge*			pg;
	CGeiger*		pgg;
	CProgress*		pp;
	CGroup*			pgr;
	CArray*			pa;
	D3DCOLORVALUE	color;
	FPOINT			pos, dim, ddim;
	float			ox, oy, sx, sy, value1, value2, bonus;
	char			text[100];
	int				rank, numTry, points, pesetas, maxPesetas, icon;
	BOOL			bWorld;

	m_sound->SetWater(FALSE);  // en dehors de l'eau

	if ( phase == PHASE_LOST )
	{
		m_phase = PHASE_LOST;
		m_bWinTerminate = FALSE;

		DeselectAll();  // désélectionne la voiture
		if ( m_bLostLookat )
		{
			m_camera->StartVisit(m_lostLookat, 200.0f);
		}
		else
		{
			m_camera->StartVisit(m_engine->RetLookatPt(), 200.0f);
		}

		m_interface->DeleteControl(EVENT_BONUS);
		m_interface->DeleteControl(EVENT_TIME);
		m_interface->DeleteControl(EVENT_BUTTON_HELP);
		m_interface->DeleteControl(EVENT_BUTTON_CAMERA);
		m_interface->DeleteControl(EVENT_BUTTON_HORN);
		m_interface->DeleteControl(EVENT_BUTTON_BRAKE);
		m_interface->DeleteControl(EVENT_BUTTON_QUIT);
		m_interface->DeleteControl(EVENT_PROGRESS);
		m_interface->DeleteControl(EVENT_OBJECT_GGEIGER);

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

	if ( phase == PHASE_NAME )
	{
		FlushStarterType();  // recommence avec un starter lent
	}

	if ( m_phase == PHASE_SIMUL )  // termine une simulation ?
	{
		m_statEndTime    = m_gameTime;
		m_statEndVehicle = RetVehiclePart();
		m_statEndCone    = RetConeTotal();
		
		m_engine->SetJoyForces(0.0f, 0.0f);
		m_sound->StopMusic();
		m_camera->SetObject(0);

		if ( !m_dialog->RetDuel() )
		{
			rank = m_dialog->RetSceneRank();
			numTry = m_dialog->RetGamerInfoTry(rank);
			m_dialog->SetGamerInfoTry(rank, numTry+1);
			m_dialog->WriteGamerMission();
		}

		if ( phase == PHASE_WIN )  // gagné une simulation ?
		{
			bonus = RetBonusPoints();
			value1 = (float)m_statEndVehicle/(float)m_statStartVehicle*100.0f;
			
			if ( m_statStartCone > 0 )
			{
				value2 = (float)m_statEndCone/(float)m_statStartCone*100.0f;
			}
			else
			{
				value2 = 100.0f;
			}

			if ( bonus < 0.0f )  bonus = 0.0f;
			maxPesetas = m_maxPesetas;
			if ( maxPesetas == 0 )  maxPesetas = 100;
			points = (int)((value1+value2+bonus)/300.0f*maxPesetas);

			if ( m_maxPesetas > 0 && !m_dialog->RetDuel() )
			{
				pesetas = points + m_dialog->RetPesetas();
				m_dialog->SetPesetas(pesetas);  // crédite le joueur
				m_dialog->PesetasUnlock();  // débloque la suite si assez
			}
			
			MemorizeCarState();
			rank = m_dialog->RetSceneRank();
			if ( !m_dialog->RetDuel() )
			{
				m_dialog->SetGamerInfoPassed(rank, TRUE);
//?				m_dialog->NextMission();
				m_dialog->WriteGamerMission();
			}

			if ( !m_bCheatUsed )  // gagné sans tricher ?
			{
				bWorld = m_dialog->AddRecord(m_statEndTime-m_statStartTime, bonus, value1, value2, points, 0);
				m_dialog->WriteRecord(rank, 0);  // écrit les records globaux
				m_dialog->WriteRecord(rank, 1);  // écrit les records par tours

				if ( m_recorderRecord != 0 )
				{
					m_recorderRecord->SetChrono(m_statEndTime-m_statStartTime);
				}
				if ( bWorld && m_recorderRecord != 0 )  // record du monde ?
				{
					RecorderWrite(rank, m_dialog->RetModel());  // écrit sur disque
				}
			}

//?			if ( m_recorderRecord != 0 )
//?			{
//?				delete m_recorderRecord;
//?				m_recorderRecord = 0;
//?			}
		}
	}

	if ( phase != PHASE_WIN   && m_phase != PHASE_WRITE &&
		 phase != PHASE_WRITE && m_phase != PHASE_WIN   )
	{
		FlushCarState();
	}

	DeleteAllObjects();  // supprime toute la scène 3D actuelle

	m_phase        = phase;
	m_winDelay     = 0.0f;
	m_lostDelay    = 0.0f;
	m_bMovieLock   = FALSE;
	m_bFreePhoto   = FALSE;
	m_bSuperWin    = FALSE;
	m_bSuperLost   = FALSE;
	m_bLostLookat  = FALSE;
	m_movieAuto    = 0;
	m_decorStamp   = 0;
	m_geiger       = 0;
	m_startCounter = 0;
	m_raceType     = 0;
	m_hornAction   = 0;
	m_messageTime  = 0.0f;

	m_engine->SetMovieLock(m_bMovieLock);
	ChangePause(FALSE);
	m_engine->SetRankView(0);
	m_engine->FlushObject();
	color.r = color.g = color.b = color.a = 0.0f;
	m_engine->SetWaterAddColor(color);
	m_engine->SetBackground("");
	m_engine->SetBackForce(FALSE);
	m_engine->SetFrontsizeName("");
	m_engine->SetOverColor();
	m_engine->GroundMarkDelete(0);
	SetSpeed(1.0f);
	m_terrain->SetWind(D3DVECTOR(0.0f, 0.0f, 0.0f));
	m_terrain->FlushBuildingLevel();
	m_terrain->FlushSlowerZone();
	m_terrain->FlushFlyingLimit();
	m_terrain->FlushTraject();
	m_light->FlushLight();
	m_particule->FlushParticule();
	m_water->Flush();
	m_cloud->Flush();
	m_blitz->Flush();
	m_planet->Flush();
	m_iMan->Flush(CLASS_OBJECT);
	m_iMan->Flush(CLASS_PHYSICS);
	m_iMan->Flush(CLASS_BRAIN);
	m_iMan->Flush(CLASS_PYRO);
	m_model->StopUserAction();
	m_interface->Flush();
	ClearInterface();
	m_sound->SetListener(D3DVECTOR(0.0f, 0.0f, 0.0f), D3DVECTOR(0.0f, 0.0f, 1.0f));
	m_camera->SetType(CAMERA_DIALOG);
	m_movie->Flush();
	m_movieInfoIndex = -1;

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

	m_dialog->ChangePhase(m_phase);

	dim.x = 32.0f/640.0f;
	dim.y = 32.0f/480.0f;
	ox = 3.0f/640.0f;
	oy = 3.0f/480.0f;
	sx = (32.0f+2.0f)/640.0f;
	sy = (32.0f+2.0f)/480.0f;

	if ( m_phase != PHASE_CAR )
	{
		m_engine->SetDrawWorld(TRUE);
		m_engine->SetDrawFront(FALSE);
		m_bFixScene = FALSE;
	}

#if _DEMO|_SE
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

//?		m_map->CreateMap();
		CreateScene(FALSE);  // scène interractive

//?		pos.x = 620.0f/640.0f;
//?		pos.y = 460.0f/480.0f;
//?		ddim.x = 20.0f/640.0f;
//?		ddim.y = 20.0f/480.0f;
//?		m_interface->CreateButton(pos, ddim, 11, EVENT_BUTTON_QUIT);

		// Crée l'indicateur de temps.
		pos.x = 175.0f/640.0f;
		pos.y =  10.0f/480.0f;
		dim.x = 100.0f/640.0f;
		dim.y =  18.0f/480.0f;
		pe = m_interface->CreateEdit(pos, dim, 0, EVENT_TIME);
		if ( pe == 0 )  return;
		pe->SetEditCap(FALSE);  // juste pour voir
		pe->SetHiliteCap(FALSE);

		// Crée la jauge de temps.
		pos.x = 175.0f/640.0f;
		pos.y =  38.0f/480.0f;
		dim.x = 100.0f/640.0f;
		dim.y =  18.0f/480.0f;
		pg = m_interface->CreateGauge(pos, dim, 2, EVENT_BONUS);
		if ( pg == 0 )  return;
		if ( m_bonusLimit == 0.0f )
		{
			pg->ClearState(STATE_VISIBLE);
		}

		// Crée les boutons.
		pos.x = 285.0f/640.0f;
		pos.y =  10.0f/480.0f;
		dim.x =  32.0f/640.0f;
		dim.y =  18.0f/480.0f;
		pb = m_interface->CreateButton(pos, dim, 14, EVENT_BUTTON_HELP);
		pos.x += 37.0f/640.0f;
		pb = m_interface->CreateButton(pos, dim, 13, EVENT_BUTTON_CAMERA);
		pos.x += 37.0f/640.0f;
		pb = m_interface->CreateButton(pos, dim, 19, EVENT_BUTTON_HORN);
		pb->SetImmediat(TRUE);
		pos.x += 37.0f/640.0f;
		pb = m_interface->CreateButton(pos, dim, 12, EVENT_BUTTON_BRAKE);
		pb->SetImmediat(TRUE);
		pos.x += 37.0f/640.0f;
		pb = m_interface->CreateButton(pos, dim, 20, EVENT_BUTTON_QUIT);

		// Crée l'indicateur de progression.
		pos.x = 285.0f/640.0f;
		pos.y =  38.0f/480.0f;
		dim.x = 180.0f/640.0f;
		dim.y =  18.0f/480.0f;
		pp = m_interface->CreateProgress(pos, dim, 0, EVENT_PROGRESS);
		if ( pp == 0 )  return;
		pp->ClearState(STATE_VISIBLE);

		if ( m_geiger != 0 )
		{
			pos.x = 590.0f/640.0f;
			pos.y =  70.0f/480.0f;
			dim.x =  30.0f/640.0f;
			dim.y =  30.0f/480.0f;
			pgg = m_interface->CreateGeiger(pos, dim, m_geiger, EVENT_OBJECT_GGEIGER);
		}

		m_engine->SetJoyForces(0.0f, 0.0f);
		m_sound->StopMusic();
		StartMusic();

		m_statStartTime    = m_gameTime;
		m_statLapTime      = m_gameTime;
		m_statBestTime     = 0.0f;
		m_statStartVehicle = RetVehiclePart();
		m_statStartCone    = RetConeTotal();
		m_bStatRecordAll   = FALSE;
		m_bStatRecordOne   = FALSE;

		pos.x = 630.0f/640.0f;
		pos.y =  10.0f/480.0f;
		m_engine->MoveMousePos(pos);  // met la souris en bas à droite
	}

	if ( m_phase == PHASE_WIN )
	{
		if ( m_endingWinRank == -1 )
		{
			ChangePhase(PHASE_TERM);
		}
		else
		{
//?			m_bWinTerminate = (m_endingWinRank == 904);
			m_bWinTerminate = FALSE;
			strcpy(m_sceneName, m_dialog->RetSceneName());
			m_dialog->SetSceneName("win");
			m_dialog->SetSceneRank(m_endingWinRank);
			CreateScene(TRUE);  // scène fixe

			pos.x  =   0.0f/640.0f;
			pos.y  = 364.0f/480.0f;
			ddim.x = 640.0f/640.0f;
			ddim.y = 116.0f/480.0f;
			pgr = m_interface->CreateGroup(pos, ddim, 23, EVENT_LABEL1);
			pos.x  =   0.0f/640.0f;
			pos.y  =   0.0f/480.0f;
			ddim.x = 640.0f/640.0f;
			ddim.y = 116.0f/480.0f;
			pgr = m_interface->CreateGroup(pos, ddim, 23, EVENT_LABEL1);

			// Partie supérieure (pesetas)
			pos.x  =  30.0f/640.0f;
			pos.y  = 382.0f/480.0f;
			ddim.x =  80.0f/640.0f;
			ddim.y =  80.0f/480.0f;
			pgr = m_interface->CreateGroup(pos, ddim, 25, EVENT_LABEL1);  // pesetas

			PutTime(text, m_statEndTime-m_statStartTime);
			pos.x  = 140.0f/640.0f;
			pos.y  = 450.0f/480.0f;
			ddim.x =  90.0f/640.0f;
			ddim.y =  18.0f/480.0f;
			pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
			pe->SetState(STATE_SHADOW);
			pe->SetEditCap(FALSE);
			pe->SetHiliteCap(FALSE);
			pe->SetText(text);
			pos.x += 110.0f/640.0f;
			pos.y -=   9.0f/480.0f;
			ddim.x = 170.0f/640.0f;
			GetResource(RES_TEXT, RT_WIN_TIME, text);
			pl = m_interface->CreateLabel(pos, ddim, 0, EVENT_LABEL1, text);
			pl->SetJustif(1);

			bonus = RetBonusPoints();
			if ( bonus >= 0.0f && !m_dialog->RetDuel() )
			{
				points = (int)bonus;
				Put100(text, points);
				pos.x  = 140.0f/640.0f;
				pos.y  = 432.0f/480.0f;
				ddim.x =  90.0f/640.0f;
				ddim.y =  18.0f/480.0f;
				pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT2);
				pe->SetState(STATE_SHADOW);
				pe->SetEditCap(FALSE);
				pe->SetHiliteCap(FALSE);
				pe->SetText(text);
				pos.x += 110.0f/640.0f;
				pos.y -=   9.0f/480.0f;
				ddim.x = 170.0f/640.0f;
				GetResource(RES_TEXT, RT_WIN_BONUS, text);
				pl = m_interface->CreateLabel(pos, ddim, 0, EVENT_LABEL2, text);
				pl->SetJustif(1);
				pos.x += 180.0f/640.0f;
				pos.y +=   9.0f/480.0f;
				ddim.x = 120.0f/640.0f;
				pg = m_interface->CreateGauge(pos, ddim, 0, EVENT_LABEL12);
				pg->SetLevel(bonus/100.0f);
			}

			value1 = (float)m_statEndVehicle/(float)m_statStartVehicle*100.0f;
			Put100(text, (int)value1);
			pos.x  = 140.0f/640.0f;
			pos.y  = 414.0f/480.0f;
			ddim.x =  90.0f/640.0f;
			ddim.y =  18.0f/480.0f;
			pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT3);
			pe->SetState(STATE_SHADOW);
			pe->SetEditCap(FALSE);
			pe->SetHiliteCap(FALSE);
			pe->SetText(text);
			pos.x += 110.0f/640.0f;
			pos.y -=   9.0f/480.0f;
			ddim.x = 170.0f/640.0f;
			GetResource(RES_TEXT, RT_WIN_SOLID, text);
			pl = m_interface->CreateLabel(pos, ddim, 0, EVENT_LABEL3, text);
			pl->SetJustif(1);
			pos.x += 180.0f/640.0f;
			pos.y +=   9.0f/480.0f;
			ddim.x = 120.0f/640.0f;
			pg = m_interface->CreateGauge(pos, ddim, 0, EVENT_LABEL13);
			pg->SetLevel(value1/100.0f);

			if ( m_statStartCone > 0 )
			{
				value2 = (float)m_statEndCone/(float)m_statStartCone*100.0f;
				Put100(text, (int)value2);
				pos.x  = 140.0f/640.0f;
				pos.y  = 396.0f/480.0f;
				ddim.x =  90.0f/640.0f;
				ddim.y =  18.0f/480.0f;
				pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT4);
				pe->SetState(STATE_SHADOW);
				pe->SetEditCap(FALSE);
				pe->SetHiliteCap(FALSE);
				pe->SetText(text);
				pos.x += 110.0f/640.0f;
				pos.y -=   9.0f/480.0f;
				ddim.x = 170.0f/640.0f;
				GetResource(RES_TEXT, RT_WIN_CONE, text);
				pl = m_interface->CreateLabel(pos, ddim, 0, EVENT_LABEL4, text);
				pl->SetJustif(1);
				pos.x += 180.0f/640.0f;
				pos.y +=   9.0f/480.0f;
				ddim.x = 120.0f/640.0f;
				pg = m_interface->CreateGauge(pos, ddim, 0, EVENT_LABEL14);
				pg->SetLevel(value2/100.0f);
			}
			else
			{
				value2 = 100.0f;
			}

			if ( bonus < 0.0f )  bonus = 0.0f;
			maxPesetas = m_maxPesetas;
			if ( maxPesetas != 0 && !m_dialog->RetDuel() )
			{
				points = (int)((value1+value2+bonus)/300.0f*maxPesetas);
				PutPoints(text, points);
				pos.x  = 140.0f/640.0f;
				pos.y  = 378.0f/480.0f;
				ddim.x =  90.0f/640.0f;
				ddim.y =  18.0f/480.0f;
				pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT5);
				pe->SetState(STATE_SHADOW);
				pe->SetState(STATE_CHECK);  // sur fond jaune
				pe->SetEditCap(FALSE);
				pe->SetHiliteCap(FALSE);
				pe->SetFontType(FONT_COLOBOT);
				pe->SetText(text);
				pos.x += 110.0f/640.0f;
				pos.y -=   9.0f/480.0f;
				ddim.x = 170.0f/640.0f;
				GetResource(RES_TEXT, RT_WIN_POINTS, text);
				pl = m_interface->CreateLabel(pos, ddim, 0, EVENT_LABEL5, text);
				pl->SetJustif(1);
				pos.x += 180.0f/640.0f;
				pos.y +=   9.0f/480.0f;
				ddim.x = 120.0f/640.0f;
				pg = m_interface->CreateGauge(pos, ddim, 0, EVENT_LABEL15);
				pg->SetLevel((float)points/maxPesetas);
			}

			// Partie inférieure (records)
			pos.x  =  30.0f/640.0f;
			pos.y  =  14.0f/480.0f;
			ddim.x =  80.0f/640.0f;
			ddim.y =  80.0f/480.0f;
			icon = 24;  // chrono
			if ( m_bStatRecordAll )  icon = 26;  // vainqueur
			pgr = m_interface->CreateGroup(pos, ddim, icon, EVENT_LABEL1);

			if ( m_bStatRecordAll )
			{
				pos.x  =  10.0f/640.0f;
				pos.y  =   6.0f/480.0f;
				ddim.x = 120.0f/640.0f;
				ddim.y =  14.0f/480.0f;
				GetResource(RES_TEXT, RT_RECORD_QALL, text);
				pgr= m_interface->CreateGroup(pos, ddim, 6, EVENT_LABEL1);
				pos.y -= 6.0f/480.0f;
				pl = m_interface->CreateLabel(pos, ddim, -1, EVENT_LABEL1, text);
				pl->SetFontSize(9.0f);
			}
			if ( m_bStatRecordOne )
			{
				pos.x  =  10.0f/640.0f;
				pos.y  =  88.0f/480.0f;
				ddim.x = 120.0f/640.0f;
				ddim.y =  14.0f/480.0f;
				GetResource(RES_TEXT, RT_RECORD_QONE, text);
				pgr= m_interface->CreateGroup(pos, ddim, 6, EVENT_LABEL1);
				pos.y -= 6.0f/480.0f;
				pl = m_interface->CreateLabel(pos, ddim, -1, EVENT_LABEL1, text);
				pl->SetFontSize(9.0f);
			}

			pos.x  = 140.0f/640.0f;
			pos.y  =  90.0f/480.0f;
			ddim.x = 360.0f/640.0f;
			ddim.y =  14.0f/480.0f;
			GetResource(RES_TEXT, RT_WIN_ARRAY, text);
			pl = m_interface->CreateLabel(pos, ddim, -1, EVENT_LABEL1, text);
			pl->SetJustif(1);

			pos.x  = 140.0f/640.0f;
			pos.y  =  13.0f/480.0f;
			ddim.x = 360.0f/640.0f;
			ddim.y =  84.0f/480.0f;
			pa = m_interface->CreateArray(pos, ddim, 0, EVENT_ARRAY1);
			pa->SetState(STATE_SHADOW);
			pa->SetTabs(0, 20.0f/640.0f, -1, FONT_COLOBOT);  // rang
			pa->SetTabs(1,110.0f/640.0f,  1, FONT_COLOBOT);  // voiture
			pa->SetTabs(2, 70.0f/640.0f,  1, FONT_COLOBOT);  // nom du joueur
			pa->SetTabs(3, 60.0f/640.0f, -1, FONT_COLOBOT);  // chrono
			pa->SetTabs(4, 55.0f/640.0f, -1, FONT_COLOBOT);  // crédits gagnés
			pa->SetSelectCap(FALSE);
			pa->SetFontSize(9.0f);
			m_dialog->UpdateRecord(pa, 0);

#if _DEMO|_SE
#else
			if ( m_dialog->RetGhostExist() )
			{
				pos.x  = 520.0f/640.0f;
				pos.y  =  89.0f/480.0f;
				ddim.x = 110.0f/640.0f;
				ddim.y =  32.0f/480.0f;
				pb = m_interface->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SAVE);
				pb->SetFontType(FONT_HILITE);
				pb->SetState(STATE_SHADOW);
				pb->SetTabOrder(1);
			}
#endif

			pos.x  = 520.0f/640.0f;
			pos.y  =  51.0f/480.0f;
			ddim.x = 110.0f/640.0f;
			ddim.y =  32.0f/480.0f;
			pb = m_interface->CreateButton(pos, ddim, -1, EVENT_INTERFACE_AGAIN);
			pb->SetFontType(FONT_HILITE);
			pb->SetState(STATE_SHADOW);
			pb->SetTabOrder(2);

			pos.x  = 520.0f/640.0f;
			pos.y  =  13.0f/480.0f;
			ddim.x = 110.0f/640.0f;
			ddim.y =  32.0f/480.0f;
			pb = m_interface->CreateButton(pos, ddim, -1, EVENT_INTERFACE_TERM);
			pb->SetFontType(FONT_HILITE);
			pb->SetState(STATE_SHADOW);
			pb->SetTabOrder(0);
			pb->SetFocus(TRUE);

#if 0
			if ( m_bWinTerminate )
			{
				pos.x = ox+sx*3;
				pos.y = oy+sy*0.2f;
				ddim.x = dim.x*15;
				ddim.y = dim.y*3.0f;
				pe = m_interface->CreateEdit(pos, ddim, 0, EVENT_EDIT0);
				pe->SetGenericMode(TRUE);
				pe->SetFontType(FONT_COLOBOT);
				pe->SetEditCap(FALSE);
				pe->SetHiliteCap(FALSE);
				pe->ReadText("help\\win.txt");
			}
			else
			{
				m_displayText->DisplayError(INFO_WIN, 1000.0f);
			}
#endif
		}
		m_sound->StopAll();
		StartMusic();
	}

	if ( m_phase == PHASE_MODEL )
	{
		pos.x = ox+sx*0;  pos.y = oy+sy*0;
		m_interface->CreateButton(pos, dim, 11, EVENT_BUTTON_CANCEL);

		CreateModel();
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

Phase CRobotMain::RetPhase()
{
	return m_phase;
}


// Libère l'état du véhicule.

void CRobotMain::FlushCarState()
{
	if ( m_carState != 0 )
	{
		free(m_carState);
		m_carState = 0;
	}
}

// Mémorise l'état du véhicule.

void CRobotMain::MemorizeCarState()
{
	CObject*	vehicle;
	CMotion*	motion;
	int			len;

	FlushCarState();

	vehicle = SearchObject(OBJECT_CAR);
	if ( vehicle == 0 )  return;

	motion = vehicle->RetMotion();
	if ( motion == 0 )  return;

	len = motion->RetStateLength();
	m_carState = (char*)malloc(sizeof(char)*len);
	motion->GetStateBuffer(m_carState);
}

// Donne l'état du véhicule sauvegardé.

char* CRobotMain::GetCarState()
{
	return m_carState;
}


// Traite un événement.

BOOL CRobotMain::EventProcess(const Event &event)
{
	CObject*		pObj;
	CEdit*			pe;
	CButton*		pb;
	Sound			sound;
	Event			newEvent;
	char			text[100];
	char			res[100];

	if ( event.event == EVENT_FRAME )
	{
		if ( m_bSkipFrame )
		{
			m_bSkipFrame = FALSE;
			return TRUE;
		}

		if ( m_movieIndex < m_movieTotal-1 )
		{
			MovieFrame(event.rTime);
		}

		if ( m_phase == PHASE_SIMUL &&
			 !m_bPause && !m_bSuspend &&
			 m_startCounter > 0 )  // décompte 3,2,1,go ?
		{
			m_startDelay -= event.rTime;
			if ( m_startDelay <= 0.0f )
			{
				m_startCounter --;
				m_startDelay = 1.0f;
				if ( RetStarterType() == STARTER_321 )  // normal ?
				{
					if ( m_startCounter == 4 )
					{
						m_startDelay = 1.5f;
						GetResource(RES_TEXT, RT_START_READY, text);
						sound = SOUND_STARTREADY;
						if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
						m_displayText->DisplayText(text, 2.0f, 20.0f, TT_START, sound);
					}
					if ( m_startCounter >= 1 && m_startCounter <= 3 )
					{
						GetResource(RES_TEXT, RT_START_123, res);
						sprintf(text, res, m_startCounter);
						if ( m_startCounter == 3 )  sound = SOUND_START3;
						if ( m_startCounter == 2 )  sound = SOUND_START2;
						if ( m_startCounter == 1 )  sound = SOUND_START1;
						if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
						m_displayText->DisplayText(text, 2.0f, 20.0f, TT_START, sound);
					}
				}
				else	// quick ?
				{
					if ( m_startCounter == 1 )
					{
						m_startDelay = 1.0f;
						GetResource(RES_TEXT, RT_START_QUICK, text);
						sound = SOUND_STARTREADY;
						if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
						m_displayText->DisplayText(text, 2.0f, 20.0f, TT_START, sound);
					}
				}
				if ( m_startCounter == 0 )
				{
					GetResource(RES_TEXT, RT_START_GO, text);
					sound = SOUND_STARTGO;
					if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
					m_displayText->DisplayText(text, 2.0f, 20.0f, TT_START, sound);
					pObj = RetSelect();
					if ( pObj != 0 )
					{
						pObj->SetStarting(FALSE);  // on peut avancer
					}
					m_camera->SetLockRotate(FALSE);
					m_camera->ResetLockRotate();
					m_starterTry ++;
					m_starterLast = m_dialog->RetSceneRank();
				}
			}
		}

		if ( m_phase == PHASE_SIMUL && m_messageTime > 0.0f )
		{
			m_messageTime -= event.rTime;
			if ( m_messageTime <= 0.0f )
			{
				m_displayText->DisplayText(m_messageText, m_messageDelay, m_messageSize, TT_START, m_messageSound);
				m_messageTime = 0.0f;
			}
		}

		m_dialog->EventProcess(event);
		m_displayText->EventProcess(event);

		m_interface->EventProcess(event);
		return EventFrame(event);
	}

	// Gestion de la console de commande.
#if 0
	if ( m_phase != PHASE_NAME &&
		 !m_movie->IsExist()   &&
		 event.event == EVENT_KEYDOWN &&
		 event.param == VK_PAUSE &&
		 (event.keyState&KS_CONTROL) != 0 )
#else
	if ( m_phase != PHASE_NAME &&
		 !m_movie->IsExist()   &&
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
			}
		}
		if ( event.event == EVENT_BUTTON_HELP ||
			 event.event == EVENT_LBUTTONDOWN ||
			 event.event == EVENT_RBUTTONDOWN )
		{
			StopDisplayInfo();
		}
		return FALSE;
	}

	// Phase de simulation du jeu.
	if ( m_phase == PHASE_SIMUL )
	{
		UpdateInfoText();
		m_camera->EventProcess(event);

		switch( event.event )
		{
			case EVENT_KEYDOWN:
				HiliteClear();
				if ( m_movieIndex < m_movieTotal-1 )  // film caméra en cours ?
				{
					if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
						 event.param == VK_ESCAPE  ||
						 event.param == VK_BUTTON1 ||
						 event.param == VK_BUTTON2 )
					{
						MovieAbort();
					}
					return FALSE;
				}
				if ( m_movieAuto != 0 )  // film automat en cours ?
				{
					if ( event.param == m_engine->RetKey(KEYRANK_QUIT, 0) ||
						 event.param == m_engine->RetKey(KEYRANK_QUIT, 1) ||
						 event.param == VK_ESCAPE  ||
						 event.param == VK_BUTTON1 ||
						 event.param == VK_BUTTON2 )
					{
						m_movieAuto->Start(99);
					}
					return FALSE;
				}
				if ( m_bMovieLock )  // film en cours ?
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
					if ( m_bPause )  ChangePause(FALSE);
					if ( m_winDelay > 0.0f )
					{
						ChangePhase(PHASE_WIN);
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
					if ( !m_bMovieLock && !m_bCmdEdit &&
						 !m_movie->IsExist() )
					{
						ChangePause(!m_engine->RetPause());
					}
				}
				if ( event.param == m_engine->RetKey(KEYRANK_CAMERA, 0) ||
					 event.param == m_engine->RetKey(KEYRANK_CAMERA, 1) )
				{
					ChangeCamera();
				}
				if ( event.param == m_engine->RetKey(KEYRANK_HELP, 0) ||
					 event.param == m_engine->RetKey(KEYRANK_HELP, 1) )
				{
					StartDisplayInfo();
				}
				if ( event.param == m_engine->RetKey(KEYRANK_HORN, 0) ||
					 event.param == m_engine->RetKey(KEYRANK_HORN, 1) )
				{
					pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_HORN);
					if ( pb != 0 )  pb->SetState(STATE_CHECK);
				}
				if ( event.param == m_engine->RetKey(KEYRANK_BRAKE, 0) ||
					 event.param == m_engine->RetKey(KEYRANK_BRAKE, 1) )
				{
					pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_BRAKE);
					if ( pb != 0 )  pb->SetState(STATE_CHECK);
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
				if ( event.param == m_engine->RetKey(KEYRANK_HORN, 0) ||
					 event.param == m_engine->RetKey(KEYRANK_HORN, 1) )
				{
					pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_HORN);
					if ( pb != 0 )  pb->ClearState(STATE_CHECK);
				}
				if ( event.param == m_engine->RetKey(KEYRANK_BRAKE, 0) ||
					 event.param == m_engine->RetKey(KEYRANK_BRAKE, 1) )
				{
					pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_BRAKE);
					if ( pb != 0 )  pb->ClearState(STATE_CHECK);
				}
				break;

			case EVENT_BUTTON_QUIT:
				if ( m_movieIndex < m_movieTotal-1 )  break;
				if ( m_bPause )  ChangePause(FALSE);
				if ( m_winDelay > 0.0f )
				{
					ChangePhase(PHASE_WIN);
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

			case EVENT_BUTTON_HELP:
				StartDisplayInfo();
				break;

			case EVENT_BUTTON_HORN:
				break;

			case EVENT_BUTTON_BRAKE:
				break;

			case EVENT_BUTTON_CAMERA:
				if ( m_movieIndex >= m_movieTotal-1 )  // pas de film caméra en cours ?
				{
					ChangeCamera();
				}
				break;

			case EVENT_OBJECT_MAPZOOM:
				m_map->ZoomMap();
				break;

			case EVENT_WIN:
				ChangePhase(PHASE_WIN);
				break;

			case EVENT_LOST:
				ChangePhase(PHASE_LOST);
				break;
		}

		EventObject(event);
		return FALSE;
	}

	if ( m_phase == PHASE_CAR )
	{
		EventObject(event);
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
					if ( m_phase == PHASE_WIN )
					{
						if ( !m_dialog->RetDuel() )
						{
							m_dialog->SetSceneName(m_sceneName);
							m_dialog->NextMission();
							m_dialog->WriteGamerMission();
						}
					}
					if ( m_bWinTerminate )
					{
						ChangePhase(PHASE_INIT);
					}
					else
					{
						ChangePhase(PHASE_TERM);
					}
				}
				break;

			case EVENT_INTERFACE_SAVE:
				ChangePhase(PHASE_WRITE);
				break;

			case EVENT_INTERFACE_AGAIN:
				m_dialog->LaunchSimul();
				break;

			case EVENT_INTERFACE_TERM:
				if ( m_phase == PHASE_WIN )
				{
					if ( !m_dialog->RetDuel() )
					{
						m_dialog->SetSceneName(m_sceneName);
						m_dialog->NextMission();
						m_dialog->WriteGamerMission();
					}
				}
				if ( m_bWinTerminate )
				{
					ChangePhase(PHASE_INIT);
				}
				else
				{
					ChangePhase(PHASE_TERM);
				}
				break;
		}
	}

	if ( m_phase == PHASE_MODEL )
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


// Change le bouton que a le focus.

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


// Début du film de caméra.

void CRobotMain::MovieStart()
{
	CObject*	pObj;

	m_movieIndex = 0;
	SetMovieLock(TRUE);

	m_movieType = m_camera->RetType();
	m_camera->SetType(CAMERA_SCRIPT);
	m_camera->SetSmooth(CS_SOFT);

	m_camera->SetScriptEye(m_movieTable[0].eye);
	m_camera->SetScriptLookat(m_movieTable[0].look);
	m_camera->FixCamera();

	if ( m_movieTable[0].sound != SOUND_CLICK )
	{
		m_sound->Play(m_movieTable[0].sound, m_movieTable[0].look);
	}

	pObj = RetSelect();
	if ( pObj != 0 )
	{
		pObj->SetStarting(TRUE);  // n'avance pas
	}
}

// Avance du film de caméra.

void CRobotMain::MovieFrame(float rTime)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	Sound		sound;
	float		progress;
	int			i;
	char		text[100];

	if ( m_bPause )  return;

	i = m_movieIndex;

	if ( m_movieTable[i].delay < 0.0f )  // transition instantanée brusque ?
	{
		pos = m_movieTable[i+1].eye;
		m_camera->SetScriptEye(pos);

		pos = m_movieTable[i+1].look;
		m_camera->SetScriptLookat(pos);

		m_camera->FixCamera();
		m_movieIndex ++;

		i = m_movieIndex;
		if ( m_movieTable[i].sound != SOUND_CLICK )
		{
			m_sound->Play(m_movieTable[i].sound, m_movieTable[i].look);
		}
		return;
	}

	m_movieTable[i].progress += rTime/m_movieTable[i].delay;
	progress = Norm(m_movieTable[i].progress);

	pos = m_movieTable[i].eye+(m_movieTable[i+1].eye-m_movieTable[i].eye)*progress;
	m_camera->SetScriptEye(pos);

	pos = m_movieTable[i].look+(m_movieTable[i+1].look-m_movieTable[i].look)*progress;
	m_camera->SetScriptLookat(pos);

	if ( m_movieTable[i].progress >= 1.0f )
	{
		m_movieIndex ++;

		i = m_movieIndex;
		if ( m_movieTable[i].sound != SOUND_CLICK )
		{
			m_sound->Play(m_movieTable[i].sound, m_movieTable[i].look);
		}

		if ( m_movieIndex >= m_movieTotal-1 )
		{
			m_camera->SetType(m_movieType);
			m_camera->SetSmooth(CS_NORM);
			SetMovieLock(FALSE);
			GetResource(RES_TEXT, RT_START_GO, text);
			sound = SOUND_STARTGO;
			if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
			m_displayText->DisplayText(text, 2.0f, 20.0f, TT_START, sound);

			pObj = RetSelect();
			if ( pObj != 0 )
			{
				pObj->SetStarting(FALSE);  // on peut avancer
			}
		}
	}
}

// Stoppe le film de caméra en cours.

void CRobotMain::MovieAbort()
{
	CObject*	pObj;

	m_movieIndex = m_movieTotal;

	m_camera->SetType(m_movieType);
	m_camera->SetSmooth(CS_NORM);
	m_camera->FixCamera();
	SetMovieLock(FALSE);

	pObj = RetSelect();
	if ( pObj != 0 )
	{
		pObj->SetStarting(FALSE);  // on peut avancer
	}
}


// Exécute une commande.

void CRobotMain::ExecuteCmd(char *cmd)
{
	if ( cmd[0] == 0 )  return;

	if ( m_phase == PHASE_SIMUL )
	{
		if ( strcmp(cmd, "winrace") == 0 )
		{
			Event		newEvent;
			m_event->MakeEvent(newEvent, EVENT_WIN);
			m_event->AddEvent(newEvent);
			m_bCheatUsed = TRUE;  // gagné en trichant
		}

		if ( strcmp(cmd, "lostrace") == 0 )
		{
			Event		newEvent;
			m_event->MakeEvent(newEvent, EVENT_LOST);
			m_event->AddEvent(newEvent);
		}
	}

	if ( strcmp(cmd, "testgame") == 0 )
	{
		m_bShowAll = TRUE;
		m_dialog->AllMissionUpdate();
		m_dialog->SetPesetas(99999);
		m_dialog->UpdateScenePesetasPerso();
		m_dialog->BuyAllPerso();
		return;
	}

	if ( strcmp(cmd, "allrace") == 0 )
	{
		m_bShowAll = !m_bShowAll;
		m_dialog->AllMissionUpdate();
		return;
	}

	if ( strcmp(cmd, "pesetas") == 0 )
	{
		int		i;
		i = m_dialog->RetPesetas();
		if ( i == 0 )  i = 1;
		else           i *= 5;
		m_dialog->SetPesetas(i);
		m_dialog->UpdateScenePesetasPerso();
		return;
	}

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
				m_camera->SetType(m_dialog->RetDefCamera());
				ChangePause(FALSE);
			}
			return;
		}

		if ( strcmp(cmd, "photo2") == 0 )
		{
			CObject*	object;
			CPhysics*	physics;
			CGroup*		pg;

			m_bFreePhoto = !m_bFreePhoto;
			if ( m_bFreePhoto )
			{
				m_camera->SetType(CAMERA_FREE);
				ChangePause(TRUE);
				object = RetSelect();
				if ( object != 0 )
				{
					physics= object->RetPhysics();
					if ( physics != 0 )
					{
						physics->CreateInterface(FALSE);
					}
				}
				m_map->ShowMap(FALSE);
				m_displayText->HideText(TRUE);
				ShowHideButtons(FALSE);

				pg = (CGroup*)m_interface->SearchControl(EVENT_OBJECT_EDITLOCK);
				if ( pg != 0 )
				{
					pg->ClearState(STATE_VISIBLE);
				}
			}
			else
			{
				m_camera->SetType(m_dialog->RetDefCamera());
				ChangePause(FALSE);
				object = RetSelect();
				if ( object != 0 )
				{
					physics= object->RetPhysics();
					if ( physics != 0 )
					{
						physics->CreateInterface(TRUE);
					}
				}
				m_map->ShowMap(TRUE);
				m_displayText->HideText(FALSE);
				ShowHideButtons(TRUE);
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

		if ( strcmp(cmd, "fullshield") == 0 )
		{
			CObject*	object;

			object = RetSelect();
			if ( object != 0 )
			{
				object->SetShield(1.0f);
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

	if ( strcmp(cmd, "invshadow") == 0 )
	{
		m_engine->SetShadow(!m_engine->RetShadow());
		return;
	}

	if ( strcmp(cmd, "invdirty") == 0 )
	{
		m_engine->SetDirty(!m_engine->RetDirty());
		return;
	}

	if ( strcmp(cmd, "invfog") == 0 )
	{
		m_engine->SetFog(!m_engine->RetFog());
		return;
	}

	if ( strcmp(cmd, "invlens") == 0 )
	{
		m_engine->SetLensMode(!m_engine->RetLensMode());
		return;
	}

	if ( strcmp(cmd, "invwater") == 0 )
	{
		m_engine->SetWaterMode(!m_engine->RetWaterMode());
		return;
	}

	if ( strcmp(cmd, "invsky") == 0 )
	{
		m_engine->SetSkyMode(!m_engine->RetSkyMode());
		return;
	}

	if ( strcmp(cmd, "invplanet") == 0 )
	{
		m_engine->SetPlanetMode(!m_engine->RetPlanetMode());
		return;
	}

	if ( strcmp(cmd, "selectinsect") == 0 )
	{
		m_bSelectInsect = !m_bSelectInsect;
		return;
	}

	if ( strcmp(cmd, "invradar") == 0 )
	{
		m_bCheatRadar = !m_bCheatRadar;
		return;
	}

	if ( m_phase == PHASE_SIMUL )
	{
		m_displayText->DisplayError(ERR_CMD);
	}
#endif
}



// Retourne le type de film en cours.

MainMovieType CRobotMain::RetMainMovie()
{
	return m_movie->RetType();
}


// Début de l'affichage des instructions.

void CRobotMain::StartDisplayInfo()
{
	CEdit*		pe;
	CButton*	pb;
	FPOINT		pos, dim;
	int			rank;

	if ( m_bCmdEdit )  return;

	pos.x = 175.0f/640.0f;
	pos.y =  66.0f/480.0f;
	dim.x = 290.0f/640.0f;
	dim.y =  90.0f/480.0f;
	pe = m_interface->CreateEdit(pos, dim, 0, EVENT_INTERFACE_RESUME);
	if ( pe == 0 )  return;
	pe->SetMaxChar(500);
	pe->SetMultiFont(TRUE);
	pe->SetEditCap(FALSE);  // juste pour voir
	pe->SetHiliteCap(FALSE);

	rank = m_dialog->RetSceneRank();
	m_dialog->UpdateSceneResume(rank, pe);

	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_HELP);
	if ( pb != 0 )  pb->SetState(STATE_CHECK);

	ChangePause(TRUE);
	m_bDisplayInfo = TRUE;
}

// Fin de l'affichage d'instructions.

void CRobotMain::StopDisplayInfo()
{
	CButton*	pb;

	m_interface->DeleteControl(EVENT_INTERFACE_RESUME);

	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_HELP);
	if ( pb != 0 )  pb->ClearState(STATE_CHECK);

	ChangePause(FALSE);
	m_bDisplayInfo = FALSE;
}


// Montre ou cache les boutons du tableau de bord.

void CRobotMain::ShowHideButtons(BOOL bShow)
{
	CGauge*		pg;
	CProgress*	pp;
	CEdit*		pe;
	CButton*	pb;
	CGeiger*	pgg;

	if ( m_bonusLimit == 0.0f )
	{
		pg = (CGauge*)m_interface->SearchControl(EVENT_BONUS);
		if ( pg != 0 )  pg->ClearState(STATE_VISIBLE);
	}
	else
	{
		pg = (CGauge*)m_interface->SearchControl(EVENT_BONUS);
		if ( pg != 0 )  pg->SetState(STATE_VISIBLE, bShow);
	}

	if ( m_progressTotal != 0 )
	{
		pp = (CProgress*)m_interface->SearchControl(EVENT_PROGRESS);
		if ( pp != 0 )  pp->SetState(STATE_VISIBLE, bShow);
	}

	if ( m_geiger != 0 )
	{
		pgg = (CGeiger*)m_interface->SearchControl(EVENT_OBJECT_GGEIGER);
		if ( pgg != 0 )  pgg->SetState(STATE_VISIBLE, bShow);
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_TIME);
	if ( pe != 0 )  pe->SetState(STATE_VISIBLE, bShow);

	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_CAMERA);
	if ( pb != 0 )  pb->SetState(STATE_VISIBLE, bShow);
	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_HORN);
	if ( pb != 0 )  pb->SetState(STATE_VISIBLE, bShow);
	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_BRAKE);
	if ( pb != 0 )  pb->SetState(STATE_VISIBLE, bShow);
	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_HELP);
	if ( pb != 0 )  pb->SetState(STATE_VISIBLE, bShow);
	pb = (CButton*)m_interface->SearchControl(EVENT_BUTTON_QUIT);
	if ( pb != 0 )  pb->SetState(STATE_VISIBLE, bShow);
}

// Début d'un dialogue pendant le jeu,

void CRobotMain::StartSuspend()
{
	m_map->ShowMap(FALSE);
	m_infoObject = DeselectAll();  // enlève les boutons de commande
	m_displayText->HideText(TRUE);
	ShowHideButtons(FALSE);
	m_bSuspend = TRUE;
}

// Fin d'un dialogue pendant le jeu,

void CRobotMain::StopSuspend()
{
	SelectObject(m_infoObject, FALSE);  // remet les boutons de commande
	m_map->ShowMap(TRUE);
	m_displayText->HideText(FALSE);
	ShowHideButtons(TRUE);
	m_bSuspend = FALSE;
}


// Enclanche ou déclanche le chrono.

void CRobotMain::SetStopwatch(BOOL bRun)
{
	m_bStopwatch = bRun;
}

// Stoppe le starter lorsque le moteur explose.

void CRobotMain::StopStartCounter()
{
	m_startCounter = -1;
}

// Indique si le starter est en fonction.

BOOL CRobotMain::IsStartCounter()
{
	return (m_startCounter != -1);
}

// Indique s'il existe un starter.

BOOL CRobotMain::IsStarter()
{
	return (m_startCounter != 0);
}

// Indique si le chrono tourne.

BOOL CRobotMain::IsGameTime()
{
	return ( !m_bMovieLock && m_startCounter == 0 && !m_bSuspend &&
			 m_bStopwatch && m_winDelay == 0.0f && m_lostDelay == 0.0f &&
			 !m_bSuperWin && !m_bPause );
}

// Indique si le chrono tourne pour enregistrer la course.

BOOL CRobotMain::IsRecordTime()
{
	return ( !m_bMovieLock && m_startCounter == 0 && !m_bSuspend &&
			 m_bStopwatch && !m_bSuperWin && !m_bPause );
}

// Indique si le chrono tourne.

BOOL CRobotMain::RetEndingGame()
{
	return ( m_winDelay != 0.0f || m_lostDelay != 0.0f );
}

// Retourne le temps absolu du jeu.

float CRobotMain::RetGameTime()
{
	return m_gameTime;
}

// Retourne le temps absolu du jeu.

float CRobotMain::RetRecordTime()
{
	return m_recordTime;
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


// Gestion des fenêtres ouvrir/enregistrer.

void CRobotMain::SetIOPublic(BOOL bMode)
{
	m_IOPublic = bMode;
	SetProfileInt("Edit", "IOPublic", m_IOPublic);
}

BOOL CRobotMain::RetIOPublic()
{
	return m_IOPublic;
}

void CRobotMain::SetIOPos(FPOINT pos)
{
	m_IOPos = pos;
	SetProfileFloat("Edit", "IOPos.x", m_IOPos.x);
	SetProfileFloat("Edit", "IOPos.y", m_IOPos.y);
}

FPOINT CRobotMain::RetIOPos()
{
	return m_IOPos;
}

void CRobotMain::SetIODim(FPOINT dim)
{
	m_IODim = dim;
	SetProfileFloat("Edit", "IODim.x", m_IODim.x);
	SetProfileFloat("Edit", "IODim.y", m_IODim.y);
}

FPOINT CRobotMain::RetIODim()
{
	return m_IODim;
}




// Retourne l'objet par défaut à sélectionner après la création d'une scène.

CObject* CRobotMain::RetSelectObject()
{
	if ( m_selectObject != 0 )  return m_selectObject;
	return SearchHuman();
}

// Désélectionne tout, et retourne l'objet qui était sélectionné.

CObject* CRobotMain::DeselectAll()
{
	CObject*	pObj;
	CObject*	pPrev;
	int			i;

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
	if ( m_movie->IsExist() )  return FALSE;
	if ( pObj == 0 )  return FALSE;

	pPrev = DeselectAll();

	SelectOneObject(pObj, bDisplayError);
	return TRUE;
}

// Sélectionne un objet, sans s'occuper de désélectionner le reste.

void CRobotMain::SelectOneObject(CObject* pObj, BOOL bDisplayError)
{
	pObj->SetSelect(TRUE, bDisplayError);
	m_camera->SetObject(pObj);
//?	m_camera->SetType(pObj->RetCameraType());
//?	m_camera->SetDist(pObj->RetCameraDist());
	m_camera->SetType(m_dialog->RetDefCamera());
}

// Désélectionne l'objet sélectionné.

BOOL CRobotMain::DeselectObject()
{
	CObject*	pObj;
	CObject*	pPrev;

	pPrev = DeselectAll();

	if ( pPrev == 0 )
	{
		pObj = SearchHuman();
	}
	if ( pObj == 0 )
	{
		pObj = SearchHuman();
	}

	if ( pObj != 0 )
	{
		SelectOneObject(pObj);
	}
	else
	{
		m_camera->SetType(CAMERA_FREE);
	}

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

// Retourne l'objet de l'homme.

CObject* CRobotMain::SearchHuman()
{
	ObjectType	type;
	CObject*	pObj;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type == OBJECT_HUMAN )
		{
			return pObj;
		}
	}
	return 0;
}

// Retourne l'objet sélectionné.

CObject* CRobotMain::RetSelect()
{
	CObject*	pObj;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetSelect() )
		{
			return pObj;
		}
	}
	return 0;
}

CObject* CRobotMain::SearchObject(ObjectType type)
{
	CObject*	pObj;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetType() == type )
		{
			return pObj;
		}
	}
	return 0;
}

// Retourne le nombre de pièces du véhicule.

int CRobotMain::RetVehiclePart()
{
	CObject*	pObj;
	CMotion*	motion;

	pObj = SearchObject(OBJECT_CAR);
	if ( pObj == 0 )  return 0;

	motion = pObj->RetMotion();
	if ( motion == 0 )  return 0;
	return motion->RetUsedPart();
}

// Retourne le nombre de cônes et d'autres objets destructibles.

int CRobotMain::RetConeTotal()
{
	CObject*	pObj;
	ObjectType	type;
	int			i, t, total;
	BOOL		bGoal;

	total = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetRuin() )  continue;
		if ( !pObj->RetEnable() )  continue;
		if ( pObj->RetGround() )  continue;  // objet renversé ?

		type = pObj->RetType();
		if ( type == OBJECT_CONE     ||
			 type == OBJECT_BARREL   ||
			 type == OBJECT_BARRELa  ||
			 type == OBJECT_STONE    ||
			 type == OBJECT_BARRIER4 ||
			 type == OBJECT_BARRIER5 ||
			 (type >= OBJECT_ROADSIGN1 && type <= OBJECT_ROADSIGN30) ||
			 (type >= OBJECT_BOX1 && type <= OBJECT_BOX10) )
		{
			bGoal = FALSE;
			for ( t=0 ; t<m_endTakeTotal ; t++ )
			{
				if ( type == m_endTake[t].type )  bGoal = TRUE;
			}

			if( !bGoal )  total ++;
		}
	}
	return total;
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
	m_camera->SetType(CAMERA_EXPLO);
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

	i = -1;
	m_engine->SetHiliteRank(&i);  // plus rien de sélectionné

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		pObj->SetHilite(FALSE);
		m_map->SetHilite(0);
	}
}

// Met en évidence l'objet survolé par la souris.

void CRobotMain::HiliteObject(FPOINT pos)
{
	char		name[100];

//?	if ( m_bFixScene && m_phase != PHASE_PERSO )  return;
//?	if ( m_bMovieLock )  return;
	if ( m_movie->IsExist() )  return;
	if ( m_engine->RetMouseHide() )  return;

	ClearInterface();  // enlève mise en évidence et tooltip

	if ( m_dialog->RetTooltip() && m_interface->GetTooltip(pos, name) )
	{
		m_tooltipPos = pos;
		strcpy(m_tooltipName, name);
		m_tooltipTime = 0.0f;
	}
}

// Met en évidence l'objet survolé par la souris.

void CRobotMain::HiliteFrame(float rTime)
{
//?	if ( m_bFixScene && m_phase != PHASE_PERSO )  return;
//?	if ( m_bMovieLock )  return;
	if ( m_movie->IsExist() )  return;

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


// Change le mode de la caméra.

void CRobotMain::ChangeCamera()
{
	CObject*	pObj;
	ObjectType	oType;
	CameraType	type;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetSelect() )
		{
			if ( pObj->RetCameraLock() )  return;
			if ( pObj->RetLock() )  return;
			if ( pObj != m_camera->RetObject() )  return;

			oType = pObj->RetType();
//?			type = pObj->RetCameraType();
			type = m_dialog->RetDefCamera();

			if ( oType != OBJECT_CAR      &&
				 oType != OBJECT_MOBILEtg &&
				 oType != OBJECT_MOBILEfb &&
				 oType != OBJECT_MOBILEob &&
				 oType != OBJECT_TRAX     &&
				 oType != OBJECT_UFO      &&
				 oType != OBJECT_CARROT   &&
				 oType != OBJECT_STARTER  &&
				 oType != OBJECT_WALKER   &&
				 oType != OBJECT_CRAZY    &&
				 oType != OBJECT_GUIDE    &&
				 oType != OBJECT_EVIL1    &&
				 oType != OBJECT_EVIL2    &&
				 oType != OBJECT_EVIL3    &&
				 oType != OBJECT_EVIL4    &&
				 oType != OBJECT_EVIL5    )  return;

#if _DEBUG
				 if ( type == CAMERA_ONBOARD )  type = CAMERA_FIX;
			else if ( type == CAMERA_FIX     )  type = CAMERA_BACK;
			else if ( type == CAMERA_BACK    )  type = CAMERA_ONBOARD;
#else
				 if ( type == CAMERA_ONBOARD )  type = CAMERA_BACK;
			else if ( type == CAMERA_BACK    )  type = CAMERA_ONBOARD;
#endif

			pObj->SetCameraType(type);
			m_camera->SetType(type);
			m_dialog->SetDefCamera(type);
		}
	}
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
	D3DVECTOR	eye, look;

	if ( m_phase == PHASE_SIMUL )
	{
		eye  = D3DVECTOR( 0.0f, 10.0f, 0.0f);
		look = D3DVECTOR(10.0f,  5.0f, 0.0f);
		m_camera->Init(eye, look, 0.0f);
		m_camera->SetType(m_dialog->RetDefCamera());
		m_camera->FixCamera();
		m_camera->ResetLockRotate();
	}

	if ( m_phase == PHASE_MODEL )
	{
		m_model->InitView();
	}
}

// Initialise le point de vue pour un objet donné.

void CRobotMain::InitEye(CObject *pObj)
{
	D3DMATRIX*	mat;
	D3DVECTOR	eye, look;

	mat  = pObj->RetWorldMatrix(0);
	look = Transform(*mat, D3DVECTOR(  0.0f,  1.0f, 0.0f));
	eye  = Transform(*mat, D3DVECTOR(-30.0f, 10.0f, 0.0f));

	m_camera->Init(eye, look, 0.0f);
	
	if ( pObj->RetType() == OBJECT_CAR )
	{
		m_camera->SetType(m_dialog->RetDefCamera());
	}
	else
	{
		m_camera->SetType(pObj->RetCameraType());
	}
	
	m_camera->FixCamera();
	m_camera->ResetLockRotate();
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
	}
	if ( IsRecordTime() )
	{
		m_recordTime += event.rTime;
	}
	UpdateTime();

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

		if ( m_engine->RetFog() )
		{
			m_camera->SetOverBaseColor(m_particule->RetFogColor(m_engine->RetEyePt()));
		}
	}
	if ( m_phase == PHASE_CAR  ||
		 m_phase == PHASE_WIN  ||
		 m_phase == PHASE_LOST )
	{
		m_camera->EventProcess(event);
	}

	// Fait progresser le modèle.
	if ( m_phase == PHASE_MODEL )
	{
		m_model->ViewMove(event, 2.0f);
		m_model->UpdateView();
		m_model->EventProcess(event);
	}

	HiliteFrame(event.rTime);

	// Fait bouger l'indicateur de pause.
	if ( m_bPause &&  // édition en cours ?
		 m_camera->RetType() != CAMERA_FREE )
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
			UpdateProgress();
		}

		if ( m_winDelay > 0.0f )
		{
			m_winDelay -= event.rTime;
			if ( m_winDelay <= 0.0f )
			{
				if ( m_bMovieLock )
				{
					m_winDelay = 1.0f;
				}
				else
				{
					Event		newEvent;
					m_event->MakeEvent(newEvent, EVENT_WIN);
					m_event->AddEvent(newEvent);
					m_bCheatUsed = FALSE;  // gagné sans tricher
				}
			}
		}

		if ( m_lostDelay > 0.0f )
		{
			m_lostDelay -= event.rTime;
			if ( m_lostDelay <= 0.0f )
			{
				if ( m_bMovieLock )
				{
					m_winDelay = 1.0f;
				}
				else
				{
					Event		newEvent;
					m_event->MakeEvent(newEvent, EVENT_LOST);
					m_event->AddEvent(newEvent);
				}
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


// Calcule le point d'arrivée de la caméra.

D3DVECTOR CRobotMain::LookatPoint(D3DVECTOR eye, float angleH, float angleV,
								  float length)
{
	D3DVECTOR	lookat;

	lookat = eye;
	lookat.z += length;

	RotatePoint(eye, angleH, angleV, lookat);
	return lookat;
}



char* SkipNum(char *p)
{
	while ( *p == ' ' || *p == '.' || *p == '-' || (*p >= '0' && *p <= '9') )
	{
		p++;
	}
	return p;
}

// Conversion des unités.

void CRobotMain::Convert()
{
	FILE*			file = NULL;
	FILE*			fileNew = NULL;
	char			line[500];
	char			lineNew[500];
	char			s[200];
	char*			base;
	char*			p;
	int				rank;
	D3DVECTOR		pos;
	float			value;

	base = m_dialog->RetSceneName();
	rank = m_dialog->RetSceneRank();

	m_dialog->BuildSceneName(line, base, rank);
	file = fopen(line, "r");
	if ( file == NULL )  return;

	strcpy(line+strlen(line)-4, ".new");
	fileNew = fopen(line, "w");
	if ( fileNew == NULL )  return;

	while ( fgets(line, 500, file) != NULL )
	{
		strcpy(lineNew, line);

		if ( Cmd(line, "DeepView") )
		{
			p = strstr(line, "air=");
			if ( p != 0 )
			{
				value = OpFloat(line, "air", 500.0f);
				value /= UNIT;
				p[0] = 0;
				p = SkipNum(p+4);
				strcpy(lineNew, line);
				strcat(lineNew, "air=");
				sprintf(s, "%.2f", value);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
			strcpy(line, lineNew);

			p = strstr(line, "water=");
			if ( p != 0 )
			{
				value = OpFloat(line, "water", 100.0f);
				value /= UNIT;
				p[0] = 0;
				p = SkipNum(p+6);
				strcpy(lineNew, line);
				strcat(lineNew, "water=");
				sprintf(s, "%.2f", value);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
			strcpy(line, lineNew);
		}

		if ( Cmd(line, "TerrainGenerate") )
		{
			p = strstr(line, "vision=");
			if ( p != 0 )
			{
				value = OpFloat(line, "vision", 500.0f);
				value /= UNIT;
				p[0] = 0;
				p = SkipNum(p+7);
				strcpy(lineNew, line);
				strcat(lineNew, "vision=");
				sprintf(s, "%.2f", value);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
		}

		if ( Cmd(line, "CreateObject") ||
			 Cmd(line, "CreateSpot")   )
		{
			p = strstr(line, "pos=");
			if ( p != 0 )
			{
				pos = OpPos(line, "pos");
				pos.x /= UNIT;
				pos.y /= UNIT;
				pos.z /= UNIT;
				p[0] = 0;
				p = SkipNum(p+4);
				p = SkipNum(p+1);
				strcpy(lineNew, line);
				strcat(lineNew, "pos=");
				sprintf(s, "%.2f", pos.x);
				strcat(lineNew, s);
				strcat(lineNew, ";");
				sprintf(s, "%.2f", pos.z);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
		}

		if ( Cmd(line, "EndMissionTake") )
		{
			p = strstr(line, "pos=");
			if ( p != 0 )
			{
				pos = OpPos(line, "pos");
				pos.x /= UNIT;
				pos.y /= UNIT;
				pos.z /= UNIT;
				p[0] = 0;
				p = SkipNum(p+4);
				p = SkipNum(p+1);
				strcpy(lineNew, line);
				strcat(lineNew, "pos=");
				sprintf(s, "%.2f", pos.x);
				strcat(lineNew, s);
				strcat(lineNew, ";");
				sprintf(s, "%.2f", pos.z);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
			strcpy(line, lineNew);

			p = strstr(line, "dist=");
			if ( p != 0 )
			{
				value = OpFloat(line, "dist", 32.0f);
				value /= UNIT;
				p[0] = 0;
				p = SkipNum(p+5);
				strcpy(lineNew, line);
				strcat(lineNew, "dist=");
				sprintf(s, "%.2f", value);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
			strcpy(line, lineNew);
		}

		if ( Cmd(line, "Camera") )
		{
			p = strstr(line, "pos=");
			if ( p != 0 )
			{
				pos = OpPos(line, "pos");
				pos.x /= UNIT;
				pos.y /= UNIT;
				pos.z /= UNIT;
				p[0] = 0;
				p = SkipNum(p+4);
				p = SkipNum(p+1);
				strcpy(lineNew, line);
				strcat(lineNew, "pos=");
				sprintf(s, "%.2f", pos.x);
				strcat(lineNew, s);
				strcat(lineNew, ";");
				sprintf(s, "%.2f", pos.z);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
			strcpy(line, lineNew);

			p = strstr(line, "h=");
			if ( p != 0 )
			{
				value = OpFloat(line, "h", 32.0f);
				value /= UNIT;
				p[0] = 0;
				p = SkipNum(p+2);
				strcpy(lineNew, line);
				strcat(lineNew, "h=");
				sprintf(s, "%.2f", value);
				strcat(lineNew, s);
				strcat(lineNew, " ");
				strcat(lineNew, p);
			}
			strcpy(line, lineNew);
		}

		fputs(lineNew, fileNew);
	}

	fclose(fileNew);
	fclose(file);
}

// Charge la scène pour le personnage.

void CRobotMain::ScenePerso()
{
	CObject*	pObj;
	int			i;

	DeleteAllObjects();  // supprime toute la scène 3D actuelle
	m_engine->FlushObject();
	m_terrain->FlushRelief();  // tout plat
	m_terrain->FlushBuildingLevel();
	m_terrain->FlushSlowerZone();
	m_terrain->FlushFlyingLimit();
	m_terrain->FlushTraject();
	m_light->FlushLight();
	m_particule->FlushParticule();
	m_iMan->Flush(CLASS_OBJECT);
	m_iMan->Flush(CLASS_PHYSICS);
	m_iMan->Flush(CLASS_BRAIN);
	m_iMan->Flush(CLASS_PYRO);

	m_dialog->SetSceneName("car");  // car000.txt
	m_dialog->SetSceneRank(0);
	CreateScene(TRUE);  // scène fixe

//?	m_engine->SetDrawWorld(FALSE);  // ne dessine rien sous l'interface
//?	m_engine->SetDrawFront(TRUE);  // dessine human sur l'interface
	pObj = SearchObject(OBJECT_CAR);
	if ( pObj != 0 )
	{
		pObj->SetSelect(TRUE);
	}

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

//?		pObj->SetDrawFront(TRUE);  // dessine sur l'interface
	}
}

// Crée toute la scène.

void CRobotMain::CreateScene(BOOL bFixScene)
{
	CObject*		pObj;
	CObject*		pSel;
	CMotion*		motion;
	CPhysics*		physics;
	FILE*			file = NULL;
	char			line[500];
	char			name[200];
	char			dir[100];
	char			op[100];
	char*			base;
	D3DCOLORVALUE	color;
	D3DVECTOR		pos;
	int				rank, obj, i, rankObj, rankGadget, sabotage;
	float			value;

//?	Convert();

	base = m_dialog->RetSceneName();
	rank = m_dialog->RetSceneRank();

	m_repeat = 0;
	if ( !bFixScene && strcmp(base, "scene") == 0 )
	{
		if ( rank == m_lastRank )  m_repeat = 1;
		m_lastRank = rank;
	}

	m_bFixScene = bFixScene;
	m_bSkipFrame = TRUE;  // saute 1er EventFrame (rTime trop grand)

	g_id = 0;
	g_build = 0;
	g_researchDone = 0;  // aucune recherche effectuée
	g_researchEnable = 0;
	m_startCounter = 0;
	m_selectObject = 0;

	m_engine->FlushReplaceTex();
	m_terrain->LevelFlush();
	m_audioTrack = 0;
	m_bAudioRepeat = TRUE;
	m_displayText->SetDelay(1.0f);
	m_displayText->SetEnable(TRUE);
	m_endingWinRank   = 0;
	m_endingLostRank  = 0;
	m_endTakeTotal = 0;
	m_endTakeResearch = 0;
	m_endTakeWinDelay = 6.0f;
	m_endTakeLostDelay = 6.0f;
	m_movieTotal = 0;
	m_movieIndex = 0;
	m_progressTotal = 0;
	m_progressLap   = 0;
	m_progressLevel = 0;
	m_progressType = OBJECT_NULL;
	m_bStopwatch = TRUE;

	if ( !m_bFixScene )
	{
		m_bonusLimit = 0.0f;
		m_bonusRecord = 0.0f;
		m_maxPesetas = 100;
		m_dialog->ReadRecord(rank, 0);  // lit les records globaux
		m_dialog->ReadRecord(rank, 1);  // lit les records par tours

		if ( m_dialog->RetGhost() )
		{
			RecorderRead(rank, m_dialog->RetModel());  // lit sur disque le fichier d'enregistrement
		}
		else
		{
			if ( m_recorderPlay != 0 )
			{
				delete m_recorderPlay;
				m_recorderPlay = 0;
			}
		}
	}

	m_colorRefVeh.r =  10.0f/256.0f;
	m_colorRefVeh.g = 166.0f/256.0f;
	m_colorRefVeh.b = 254.0f/256.0f;  // bleu
	m_colorRefVeh.a = 0.0f;
	m_colorNewVeh = m_colorRefVeh;

	m_colorRefBot.r = 110.0f/256.0f;
	m_colorRefBot.g = 161.0f/256.0f;
	m_colorRefBot.b = 208.0f/256.0f;  // bleu
	m_colorRefBot.a = 0.0f;
	m_colorNewBot = m_colorRefBot;

	m_colorRefAlien.r = 135.0f/256.0f;
	m_colorRefAlien.g = 170.0f/256.0f;
	m_colorRefAlien.b =  13.0f/256.0f;  // vert
	m_colorRefAlien.a = 0.0f;
	m_colorNewAlien = m_colorRefAlien;

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

	m_dialog->BuildResumeName(m_title, base, rank);
	m_dialog->BuildResumeName(m_resume, base, rank);

	m_dialog->BuildSceneName(line, base, rank);
	file = fopen(line, "r");
	if ( file == NULL )  return;

	rankObj = 0;
	rankGadget = 0;
	pSel = 0;

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
			OpString(line, "text", m_title);
		}

		sprintf(op, "Resume.%c", RetLanguageLetter());
		if ( Cmd(line, op) )
		{
			OpString(line, "text", m_resume);
		}

		if ( Cmd(line, "EndingFile") )
		{
			m_endingWinRank  = OpInt(line, "win",  0);
			m_endingLostRank = OpInt(line, "lost", 0);
		}

		if ( Cmd(line, "MessageDelay") )
		{
			m_displayText->SetDelay(OpFloat(line, "factor", 1.0f));
		}

		if ( Cmd(line, "Audio") )
		{
			m_audioTrack = OpInt(line, "track", 0);
			m_bAudioRepeat = OpInt(line, "repeat", 1);
		}

		if ( Cmd(line, "AmbiantColor") )
		{
			m_engine->SetAmbiantColor(OpColor(line, "air",   0x88888888), 0);
			m_engine->SetAmbiantColor(OpColor(line, "water", 0x88888888), 1);
		}

		if ( Cmd(line, "FogColor") )
		{
			m_engine->SetFogColor(OpColor(line, "air",   0x88888888), 0);
			m_engine->SetFogColor(OpColor(line, "water", 0x88888888), 1);
		}

		if ( Cmd(line, "VehicleColor") )
		{
			m_colorNewVeh = RetColor(OpColor(line, "color", 0x88888888));
		}

		if ( Cmd(line, "BotColor") )
		{
			m_colorNewBot = RetColor(OpColor(line, "color", 0x88888888));
		}

		if ( Cmd(line, "InsectColor") )
		{
			m_colorNewAlien = RetColor(OpColor(line, "color", 0x88888888));
		}

		if ( Cmd(line, "GreeneryColor") )
		{
			m_colorNewGreen = RetColor(OpColor(line, "color", 0x88888888));
		}

		if ( Cmd(line, "DeepView") )
		{
			m_engine->SetDeepView(OpFloat(line, "air",   500.0f)*UNIT, 0, TRUE);
			m_engine->SetDeepView(OpFloat(line, "water", 100.0f)*UNIT, 1, TRUE);
		}

		if ( Cmd(line, "FogStart") )
		{
			m_engine->SetFogStart(OpFloat(line, "air",   0.5f), 0);
			m_engine->SetFogStart(OpFloat(line, "water", 0.5f), 1);
		}

		if ( Cmd(line, "SecondTexture") )
		{
			m_engine->SetSecondTexture(OpInt(line, "rank", 1));
		}

		if ( Cmd(line, "Background") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_engine->SetBackground(dir,
									OpColor(line, "up",        0x00000000),
									OpColor(line, "down",      0x00000000),
									OpColor(line, "cloudUp",   0x00000000),
									OpColor(line, "cloudDown", 0x00000000),
									OpInt(line, "full", 0),
									OpInt(line, "quarter", 0),
									OpInt(line, "panel", 0));
		}

		if ( Cmd(line, "Planet") )
		{
			D3DVECTOR	ppos, uv1, uv2;

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

		if ( Cmd(line, "FrontsizeName") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_engine->SetFrontsizeName(dir);
		}

		if ( Cmd(line, "TerrainGenerate") )
		{
			m_terrain->Generate(OpInt(line, "mosaic", 10),
								OpInt(line, "brick", 3),
								OpFloat(line, "size", 10.0f)*UNIT,
								OpFloat(line, "vision", 500.0f)*UNIT,
								OpInt(line, "depth", 1),
								OpFloat(line, "hard", 0.5f));
		}

		if ( Cmd(line, "TerrainWind") )
		{
			m_terrain->SetWind(OpPos(line, "speed"));
		}

		if ( Cmd(line, "TerrainRelief") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "textures");
			m_terrain->ReliefFromBMP(dir, OpFloat(line, "factor", 1.0f));
		}

		if ( Cmd(line, "TerrainReliefDXF") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "textures");
			m_terrain->ReliefFromDXF(dir, OpFloat(line, "factor", 1.0f));
		}

		if ( Cmd(line, "TerrainWater") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "");
			pos.x = OpFloat(line, "moveX", 0.0f);
			pos.y = OpFloat(line, "moveY", 0.0f);
			pos.z = pos.x;
			m_water->Create(OpTypeWater(line, "air",   WATER_TT),
							OpTypeWater(line, "water", WATER_TT),
							dir,
							RetColor(OpColor(line, "diffuse", 0xffffffff)),
							RetColor(OpColor(line, "ambiant", 0xffffffff)),
							OpFloat(line, "level", 100.0f)*UNIT,
							OpFloat(line, "glint", 1.0f),
							pos);
			m_colorNewWater = RetColor(OpColor(line, "color", RetColor(m_colorRefWater)));
			m_colorShiftWater = OpFloat(line, "brightness", 0.0f);
		}

		if ( Cmd(line, "TerrainLava") )
		{
			m_water->SetLava(OpInt(line, "mode", 0));
		}

		if ( Cmd(line, "TerrainCloud") )
		{
			OpString(line, "image", name);
			UserDir(dir, name, "");
			m_cloud->Create(dir,
							RetColor(OpColor(line, "diffuse", 0xffffffff)),
							RetColor(OpColor(line, "ambiant", 0xffffffff)),
							OpFloat(line, "level", 500.0f)*UNIT);
		}

		if ( Cmd(line, "TerrainBlitz") )
		{
			m_blitz->Create(OpFloat(line, "sleep", 0.0f),
							OpFloat(line, "delay", 3.0f),
							OpFloat(line, "magnetic", 50.0f)*UNIT);
		}

		if ( Cmd(line, "TerrainInitTextures") )
		{
			int		dx, dy, tt[100];
			char*	op;

			OpString(line, "image", name);
			AddExt(name, ".tga");
			dx = OpInt(line, "dx", 1);
			dy = OpInt(line, "dy", 1);
			op = SearchOp(line, "table");
			for ( i=0 ; i<dx*dy ; i++ )
			{
				tt[i] = GetInt(op, i, 0);
			}

			if ( strstr(name, "%user%") != 0 )
			{
				CopyFileListToTemp(name, tt, dx*dy);
			}

			m_terrain->InitTextures(name, tt, dx, dy);
		}

		if ( Cmd(line, "TerrainInit") )
		{
			m_terrain->LevelInit(OpInt(line, "id", 1));
		}

		if ( Cmd(line, "TerrainMaterial") )
		{
			OpString(line, "image", name);
			AddExt(name, ".tga");
			if ( strstr(name, "%user%") != 0 )
			{
				CopyFileToTemp(name);
			}

			m_terrain->LevelMaterial(OpInt(line, "id", 0),
									 name,
									 OpFloat(line, "u", 0.0f),
									 OpFloat(line, "v", 0.0f),
									 OpInt(line, "up",    1),
									 OpInt(line, "right", 1),
									 OpInt(line, "down",  1),
									 OpInt(line, "left",  1),
									 OpFloat(line, "hard", 0.5f));
		}

		if ( Cmd(line, "TerrainLevel") )
		{
			int		id[50];
			char*	op;

			op = SearchOp(line, "id");
			i = 0;
			while ( TRUE )
			{
				id[i] = GetInt(op, i, 0);
				if ( id[i++] == 0 )  break;
			}

			m_terrain->LevelGenerate(id,
									 OpFloat(line, "min", 0.0f)*UNIT,
									 OpFloat(line, "max", 100.0f)*UNIT,
									 OpFloat(line, "slope", 5.0f),
									 OpFloat(line, "freq", 100.0f),
									 OpPos(line, "center")*UNIT,
									 OpFloat(line, "radius", 0.0f)*UNIT);
		}

		if ( Cmd(line, "TerrainRoute") )
		{
			D3DVECTOR	p1, p2;
			int			id[50];
			char*		op;

			op = SearchOp(line, "id");
			i = 0;
			while ( TRUE )
			{
				id[i] = GetInt(op, i, 0);
				if ( id[i++] == 0 )  break;
			}

			p1 = OpPos(line, "p1")*UNIT;
			op = SearchOp(line, "p2");
			if ( *op == 0 )  p2 = p1;
			else             p2 = OpPos(line, "p2")*UNIT;

			if ( p1.z == p2.z )  // horizontal ?
			{
				if ( p1.x > p2.x )  Swap(p1.x, p2.x);
				while ( p1.x <= p2.x )
				{
					m_terrain->LevelGenerate(id, 0.0f, 100.0f*UNIT, 5.0f,
											  100.0f, p1, 1.0f*UNIT);
					p1.x += 10.0f*UNIT;
				}
			}
			else if ( p1.x == p2.x )  // vertical ?
			{
				if ( p1.z > p2.z )  Swap(p1.z, p2.z);
				while ( p1.z <= p2.z )
				{
					m_terrain->LevelGenerate(id, 0.0f, 100.0f*UNIT, 5.0f,
											  100.0f, p1, 1.0f*UNIT);
					p1.z += 10.0f*UNIT;
				}
			}
			else if ( Abs(p1.x-p2.x) == Abs(p1.z-p2.z) )  // diagonal ?
			{
				while ( TRUE )
				{
					m_terrain->LevelGenerate(id, 0.0f, 100.0f*UNIT, 5.0f,
											  100.0f, p1, 1.0f*UNIT);
					if ( p1.x == p2.x )  break;
					if ( p1.x < p2.x )  p1.x += 20.0f*UNIT;
					else                p1.x -= 20.0f*UNIT;
					if ( p1.z < p2.z )  p1.z += 20.0f*UNIT;
					else                p1.z -= 20.0f*UNIT;
				}
			}
		}

		if ( Cmd(line, "TerrainJoin") )
		{
			m_terrain->LevelRoadAdapt(OpInt(line, "f1", 0));
		}

		if ( Cmd(line, "TerrainCreate") )
		{
			m_terrain->CreateObjects(TRUE);
		}

		if ( Cmd(line, "BeginObject") )
		{
		}

		if ( Cmd(line, "CreateObject") )
		{
			CObject*	pObj;
			CBrain*		pBrain;
			CAuto*		pAuto;
			CPyro*		pyro;
			ObjectType	type;
			PyroType	pType;
			CameraType	cType;
			float		dir;
			char		op[20];
			char*		p;
			int			run, gadget, level, repeat;

			type = OpTypeObject(line, "type", OBJECT_NULL);

			gadget = OpInt(line, "gadget", -1);
			if ( gadget == -1 )
			{
				gadget = 0;
				if ( type == OBJECT_TECH ||
					 (type >= OBJECT_PLANT0  &&
					  type <= OBJECT_PLANT19 ) ||
					 (type >= OBJECT_TREE0   &&
					  type <= OBJECT_TREE9   ) ||
					 (type >= OBJECT_QUARTZ0 &&
					  type <= OBJECT_QUARTZ9 ) ||
					 (type >= OBJECT_ROOT0   &&
					  type <= OBJECT_ROOT4   ) )  // pas ROOT5 !
				{
					gadget = 1;
				}
			}
			if ( gadget != 0 )  // est-ce un gadget ?
			{
				if ( !TestGadgetQuantity(rankGadget++) )  continue;
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
			pObj = CreateObject(pos, dir,
								OpFloat(line, "z", 1.0f),
								OpFloat(line, "h", 0.0f),
								type,
								OpInt(line, "plumb", -1),
								OpInt(line, "trainer", 0),
								OpInt(line, "option", 0));

			if ( pObj != 0 )
			{
				pObj->SetDefRank(rankObj);

				if ( type == OBJECT_CAR && !m_bFixScene )
				{
					int	check[10];
					m_dialog->ComputeCheck(m_dialog->RetSceneRank(), check);
					if ( m_recorderRecord != 0 )
					{
						delete m_recorderRecord;
					}
					m_recorderRecord = new CRecorder(m_iMan);
					m_recorderRecord->SetModel(m_dialog->RetModel());
					m_recorderRecord->SetSubModel(m_dialog->RetSubModel());
					m_recorderRecord->SetColor(m_dialog->RetGamerColorCar());
					m_recorderRecord->SetType(*m_dialog->RetSceneName());
					m_recorderRecord->SetMission(m_dialog->RetSceneRank());
					for ( i=0 ; i<10 ; i++ )
					{
						m_recorderRecord->SetCheck(i, check[i]);
					}
					m_recorderRecord->SetLevel(RetLevel());
					m_recorderRecord->SetGamer(m_gamerName);
					pObj->SetRecorder(m_recorderRecord);
					pObj->SetRecorderRecordMode(TRUE);
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
				pObj->SetEnable(OpInt(line, "enable", 1));
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

				run = -1;
				pBrain = pObj->RetBrain();
				if ( pBrain != 0 )
				{
					for ( i=0 ; i<10 ; i++ )
					{
						sprintf(op, "script%d", i+1);  // script1..script10
						OpString(line, op, name);
						if ( name[0] != 0 )
						{
							pBrain->SetScriptName(i, name);
						}
					}

					i = OpInt(line, "run", 0);
					if ( i != 0 )
					{
						run = i-1;
						pBrain->SetScriptRun(run);
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
						if ( !m_dialog->RetMovies() )  i = 0;
						pAuto->Start(i);  // démarre le film
					}
				}
			}

			rankObj ++;
		}

		if ( Cmd(line, "CreateLimit") )
		{
			CreateLimit(OpTypeObject(line, "type", OBJECT_NULL),
						OpInt(line, "max", 1));
		}

		if ( Cmd(line, "CreateTraject") )
		{
			pos = OpPos(line, "pos")*UNIT;
			m_terrain->AddTraject(pos);

			if ( OpInt(line, "show", 0) == 1 )
			{
				CreateObject(pos, 0.0f, 1.0f, 0.0f, OBJECT_WAYPOINT, -1, 0, 0);
			}
		}

		if ( Cmd(line, "CreateFog") )
		{
			ParticuleType	type;
			FPOINT			dim;
			float			height, ddim, delay;

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
			if ( type == TYPEFIX )
			{
				m_light->SetLightExcluType(obj, TYPETERRAIN);
			}
		}
		if ( Cmd(line, "CreateSpot") )
		{
			D3DTypeObj	type;

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
			if ( type == TYPEFIX )
			{
				m_light->SetLightExcluType(obj, TYPETERRAIN);
			}
		}

		if ( Cmd(line, "GroundSpot") )
		{
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

		if ( Cmd(line, "SlowerZone") )
		{
			m_terrain->AddSlowerZone(OpPos(line, "pos")*UNIT,
									 OpFloat(line, "min", 10.0f)*UNIT,
									 OpFloat(line, "max", 20.0f)*UNIT,
									 OpFloat(line, "factor", 0.5f));
		}

		if ( Cmd(line, "WaterColor") )
		{
			color.r = 0.0f;
			color.g = 0.0f;
			color.b = 0.0f;
			color.a = 1.0f;
			m_engine->SetWaterAddColor(OpColorValue(line, "color", color));
		}

		if ( Cmd(line, "MapColor") )
		{
			m_map->FloorColorMap(RetColor(OpColor(line, "floor", 0x88888888)),
								 RetColor(OpColor(line, "water", 0x88888888)));
		}
		if ( Cmd(line, "MapZoom") )
		{
			m_map->ZoomMap(OpFloat(line, "factor", 2.0f));
			m_map->MapEnable(OpInt(line, "enable", 1));
		}

		if ( Cmd(line, "MaxFlyingHeight") )
		{
			m_terrain->SetFlyingMaxHeight(OpFloat(line, "max", 280.0f)*UNIT);
		}
		if ( Cmd(line, "AddFlyingHeight") )
		{
			m_terrain->AddFlyingLimit(OpPos(line, "center")*UNIT,
									  OpFloat(line, "extRadius", 20.0f)*UNIT,
									  OpFloat(line, "intRadius", 10.0f)*UNIT,
									  OpFloat(line, "maxHeight", 200.0f));
		}

		if ( Cmd(line, "Movie") )
		{
			i = m_movieTotal;
			if ( i < 20 )
			{
				pos = OpDir(line, "eye")*UNIT;
				pos.y += m_terrain->RetFloorLevel(pos);
				m_movieTable[i].eye = pos;
				pos = OpDir(line, "look")*UNIT;
				pos.y += m_terrain->RetFloorLevel(pos);
				m_movieTable[i].look = pos;
				m_movieTable[i].delay = OpFloat(line, "delay", 1.0f);
				m_movieTable[i].sound = (Sound)OpInt(line, "sound", 0);
				m_movieTable[i].progress = 0.0f;
				m_movieTotal ++;
			}
		}

		if ( Cmd(line, "StartDelay") )
		{
			if ( RetStarterType() == STARTER_321 )
			{
				m_startCounter = 5;
				m_startDelay = 2.5f;
			}
			else
			{
				m_startCounter = 2;
				m_startDelay = 1.5f;
			}
			m_camera->SetLockRotate(TRUE);
		}

		if ( Cmd(line, "RaceType") )
		{
			m_raceType = OpInt(line, "chrono", 0);
			sabotage = OpInt(line, "sabotage", 0);
			m_hornAction = OpInt(line, "horn", 0);
		}

		if ( Cmd(line, "EndMissionTake") )
		{
			i = m_endTakeTotal;
			if ( i < 10 )
			{
				m_endTake[i].pos  = OpPos(line, "pos")*UNIT;
				m_endTake[i].dist = OpFloat(line, "dist", 8.0f)*UNIT;
				m_endTake[i].type = OpTypeObject(line, "type", OBJECT_NULL);
				m_endTake[i].min  = OpInt(line, "min", 1);
				m_endTake[i].max  = OpInt(line, "max", 9999);
				m_endTake[i].lost = OpInt(line, "lost", -1);
				m_endTake[i].show = OpInt(line, "show", 0);
				m_endTake[i].last = -1;
				m_endTake[i].bImmediat = OpInt(line, "immediat", 0);
				m_endTakeTotal ++;
			}
		}
		if ( Cmd(line, "EndMissionDelay") )
		{
			m_endTakeWinDelay  = OpFloat(line, "win",  6.0f);
			m_endTakeLostDelay = OpFloat(line, "lost", 6.0f);
		}
		if ( Cmd(line, "EndMissionResearch") )
		{
			m_endTakeResearch |= OpResearch(line, "type");
		}
		if ( Cmd(line, "ProgressInfo") )
		{
			m_progressTotal = OpInt(line, "total", 0);
			m_progressLap   = OpInt(line, "lap", 0);
			m_progressAdd   = OpInt(line, "add", 0);
			m_progressType  = OpTypeObject(line, "deleteType", OBJECT_NULL);
		}
		if ( Cmd(line, "BonusTime") )
		{
			m_bonusLimit  = OpFloat(line, "limit",  0.0f);
			m_bonusRecord = OpFloat(line, "record", 0.0f);

			if ( RetLevel() == 1 )
			{
				m_bonusLimit  *= 2.5f;
//?				m_bonusRecord *= 2.5f;
			}

			if ( RetLevel() == 2 )
			{
				m_bonusLimit  *= 1.5f;
//?				m_bonusRecord *= 1.5f;
			}

			if ( RetLevel() == 3 )
			{
				m_bonusLimit  *= 1.2f;
//?				m_bonusRecord *= 1.2f;
			}

			if ( RetLevel() == 1 )
			{
				value = OpFloat(line, "limit1",  0.0f);
				if ( value != 0.0f )  m_bonusLimit = value;
				value = OpFloat(line, "record1", 0.0f);
				if ( value != 0.0f )  m_bonusRecord = value;
			}

			if ( RetLevel() == 2 )
			{
				value = OpFloat(line, "limit2",  0.0f);
				if ( value != 0.0f )  m_bonusLimit = value;
				value = OpFloat(line, "record2", 0.0f);
				if ( value != 0.0f )  m_bonusRecord = value;
			}

			if ( RetLevel() == 3 )
			{
				value = OpFloat(line, "limit3",  0.0f);
				if ( value != 0.0f )  m_bonusLimit = value;
				value = OpFloat(line, "record3", 0.0f);
				if ( value != 0.0f )  m_bonusRecord = value;
			}

			if ( RetLevel() == 4 )
			{
				value = OpFloat(line, "limit4",  0.0f);
				if ( value != 0.0f )  m_bonusLimit = value;
				value = OpFloat(line, "record4", 0.0f);
				if ( value != 0.0f )  m_bonusRecord = value;
			}
		}
		if ( Cmd(line, "Pesetas") )
		{
			m_maxPesetas = OpInt(line, "max",  100);
		}
		if ( Cmd(line, "Geiger") )
		{
			m_geiger = OpInt(line, "type", 0);
		}

		if ( Cmd(line, "EnableBuild") )
		{
			g_build |= OpBuild(line, "type");
		}

		if ( Cmd(line, "EnableResearch") )
		{
			g_researchEnable |= OpResearch(line, "type");
		}
		if ( Cmd(line, "DoneResearch") )  // pas loading file ?
		{
			g_researchDone |= OpResearch(line, "type");
		}
	}

	fclose(file);

	CompileScript();  // compile tous les scripts

	if ( strcmp(base, "free") == 0 )  // jeu libre ?
	{
		g_researchDone = m_freeResearch;

		g_build = m_freeBuild;
		g_build &= ~BUILD_RESEARCH;
		g_build &= ~BUILD_LABO;
		g_build |= BUILD_FACTORY;
		g_build |= BUILD_GFLAT;
		g_build |= BUILD_FLAG;
	}

	ChangeColor();  // change les couleurs des textures

	m_map->UpdateMap();
	m_engine->TimeInit();
	m_engine->FlushPressKey();
	m_time = 0.0f;
	m_gameTime = 0.0f;
	m_recordTime = 0.0f;
	m_checkEndTime = -10.0f;

	m_selectObject = pSel;

	if ( !m_bFixScene )   // scène interractive ?
	{
		if ( pSel == 0 )
		{
			pObj = SearchHuman();
		}
		else
		{
			pObj = pSel;
		}
		if ( pObj != 0 )
		{
			SelectObject(pObj);
			m_camera->SetObject(pObj);
			m_camera->SetType(m_dialog->RetDefCamera());

			if ( sabotage == 1 )
			{
				physics = pObj->RetPhysics();
				if ( physics != 0 )
				{
					physics->ForceMotorSpeedX(1.0f);
				}
			}

			InitEye(pObj);
		}

		if ( m_recorderPlay != 0 )
		{
			pObj = new CObject(m_iMan);
			pObj->CreateVehicle(D3DVECTOR(0.0f, 0.0f, 0.0f), 0.0f, 1.0f,
								OBJECT_CAR,
								m_recorderPlay->RetModel(),
								m_dialog->RetSubModel(),
								FALSE, FALSE);
			pObj->SetRecorder(m_recorderPlay);
			pObj->SetRecorderPlayMode(TRUE);
			pObj->SetGhost(TRUE);
			pObj->SetTransparency(1.0f);
		}
	}
	if ( m_bFixScene )
	{
		m_camera->SetType(CAMERA_SCRIPT);
	}
	if ( !m_bFixScene && m_startCounter > 0 && pSel != 0 )
	{
		pSel->SetStarting(TRUE);  // n'avance pas
	}
	if ( m_movieTotal > 0 )  // commence par un "film" ?
	{
		MovieStart();
	}
}

// Crée un objet du décor mobile ou fixe.

CObject* CRobotMain::CreateObject(D3DVECTOR pos, float angle, float zoom,
								  float height, ObjectType type,
								  int bPlumb, BOOL bTrainer,
								  int option)
{
	CObject*	pObject = 0;
	CAuto*		automat;
	int			model, subModel;

	if ( type == OBJECT_NULL )  return 0;

	if ( type == OBJECT_HUMAN ||
		 type == OBJECT_TECH  )
	{
		bTrainer = FALSE;  // forcément
	}

	if ( (type >= OBJECT_FACTORY1  && type <= OBJECT_FACTORY20 ) ||
		 (type >= OBJECT_ALIEN1    && type <= OBJECT_ALIEN10   ) ||
		 (type >= OBJECT_INCA1     && type <= OBJECT_INCA10    ) ||
		 (type >= OBJECT_BUILDING1 && type <= OBJECT_BUILDING10) ||
		 (type >= OBJECT_CARCASS1  && type <= OBJECT_CARCASS10 ) ||
		 (type >= OBJECT_ORGA1     && type <= OBJECT_ORGA10    ) ||
		 type == OBJECT_TOWER     ||
		 type == OBJECT_NUCLEAR   ||
		 type == OBJECT_PARA      ||
		 type == OBJECT_COMPUTER  ||
		 type == OBJECT_REPAIR    ||
		 type == OBJECT_SWEET     ||
		 type == OBJECT_DOOR1     ||
		 type == OBJECT_DOOR2     ||
		 type == OBJECT_DOOR3     ||
		 type == OBJECT_DOOR4     ||
		 type == OBJECT_DOOR5     ||
		 type == OBJECT_DOCK      ||
		 type == OBJECT_REMOTE    ||
		 type == OBJECT_STAND     ||
		 type == OBJECT_GENERATOR ||
		 type == OBJECT_START     ||
		 type == OBJECT_END       ||
		 type == OBJECT_SUPPORT   ||
		 type == OBJECT_GRAVEL    ||
		 type == OBJECT_TUB       )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreateBuilding(pos, angle, zoom, height, type, bPlumb);

		automat = pObject->RetAuto();
		if ( automat != 0 )
		{
			automat->Init();
		}
	}
	else
	if ( type == OBJECT_FRET        ||
		 type == OBJECT_STONE       ||
		 type == OBJECT_URANIUM     ||
		 type == OBJECT_METAL       ||
		 type == OBJECT_BARREL      ||
		 type == OBJECT_BARRELa     ||
		 type == OBJECT_ATOMIC      ||
		 type == OBJECT_BULLET      ||
		 type == OBJECT_BBOX        ||
		 type == OBJECT_KEYa        ||
		 type == OBJECT_KEYb        ||
		 type == OBJECT_KEYc        ||
		 type == OBJECT_KEYd        ||
		 type == OBJECT_TNT         ||
		 type == OBJECT_MINE        ||
		 type == OBJECT_POLE        ||
		 type == OBJECT_BOMB        ||
		 type == OBJECT_CONE        ||
		 type == OBJECT_PIPES       ||
		 type == OBJECT_WAYPOINT    ||
		 type == OBJECT_TRAJECT     ||
		 type == OBJECT_SHOW        ||
		 type == OBJECT_WINFIRE     ||
		 type == OBJECT_BAG         ||
		 type == OBJECT_TRAXf       ||
		 type == OBJECT_MARK        ||
		 type == OBJECT_CROWN       ||
		 (type >= OBJECT_ROADSIGN1 && type <= OBJECT_ROADSIGN30) ||
		 (type >= OBJECT_PUB11     && type <= OBJECT_PUB94     ) )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreateResource(pos, angle, zoom, type, bPlumb);
	}
	else
	if ( type == OBJECT_FIRE    ||
		 type == OBJECT_HELICO  ||
		 type == OBJECT_COMPASS ||
		 type == OBJECT_BLITZER ||
		 type == OBJECT_HOOK    ||
		 type == OBJECT_AQUA    )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->SetOption(option);
		pObject->CreateAdditionnal(pos, angle, zoom, type, bPlumb);
	}
	else
	if ( type == OBJECT_BOT1    ||
		 type == OBJECT_BOT2    ||
		 type == OBJECT_BOT3    ||
		 type == OBJECT_BOT4    ||
		 type == OBJECT_BOT5    ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   ||
		 type == OBJECT_EVIL1   ||
		 type == OBJECT_EVIL2   ||
		 type == OBJECT_EVIL3   ||
		 type == OBJECT_EVIL4   ||
		 type == OBJECT_EVIL5   )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->SetOption(option);
		pObject->CreateBot(pos, angle, zoom, type, bPlumb);
	}
	else
	if ( type >= OBJECT_BARRIER0  &&
		 type <= OBJECT_BARRIER19 )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreateBarrier(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( (type >= OBJECT_BOX1  &&
		  type <= OBJECT_BOX10 ) ||
		 (type >= OBJECT_TOYS1  &&
		  type <= OBJECT_TOYS5 ) )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreateBox(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_STONE1  ||
		 type == OBJECT_STONE2  ||
		 type == OBJECT_STONE3  ||
		 type == OBJECT_STONE4  ||
		 type == OBJECT_STONE5  ||
		 type == OBJECT_STONE6  ||
		 type == OBJECT_STONE7  ||
		 type == OBJECT_STONE8  ||
		 type == OBJECT_STONE9  ||
		 type == OBJECT_STONE10 ||
		 type == OBJECT_CROSS1  )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreateStone(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_PIECE0 ||
		 type == OBJECT_PIECE1 ||
		 type == OBJECT_PIECE2 ||
		 type == OBJECT_PIECE3 ||
		 type == OBJECT_PIECE4 ||
		 type == OBJECT_PIECE5 ||
		 type == OBJECT_PIECE6 ||
		 type == OBJECT_PIECE7 ||
		 type == OBJECT_PIECE8 ||
		 type == OBJECT_PIECE9 )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreatePiece(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_PLANT0  ||
		 type == OBJECT_PLANT1  ||
		 type == OBJECT_PLANT2  ||
		 type == OBJECT_PLANT3  ||
		 type == OBJECT_PLANT4  ||
		 type == OBJECT_PLANT5  ||
		 type == OBJECT_PLANT6  ||
		 type == OBJECT_PLANT7  ||
		 type == OBJECT_PLANT8  ||
		 type == OBJECT_PLANT9  ||
		 type == OBJECT_PLANT10 ||
		 type == OBJECT_PLANT11 ||
		 type == OBJECT_PLANT12 ||
		 type == OBJECT_PLANT13 ||
		 type == OBJECT_PLANT14 ||
		 type == OBJECT_PLANT15 ||
		 type == OBJECT_PLANT16 ||
		 type == OBJECT_PLANT17 ||
		 type == OBJECT_PLANT18 ||
		 type == OBJECT_PLANT19 ||
		 type == OBJECT_TREE0   ||
		 type == OBJECT_TREE1   ||
		 type == OBJECT_TREE2   ||
		 type == OBJECT_TREE3   ||
		 type == OBJECT_TREE4   ||
		 type == OBJECT_TREE5   ||
		 type == OBJECT_TREE6   ||
		 type == OBJECT_TREE7   ||
		 type == OBJECT_TREE8   ||
		 type == OBJECT_TREE9   )
	{
		if ( bPlumb == -1 )  bPlumb = TRUE;
		pObject = new CObject(m_iMan);
		pObject->CreatePlant(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_MUSHROOM0 ||
		 type == OBJECT_MUSHROOM1 ||
		 type == OBJECT_MUSHROOM2 ||
		 type == OBJECT_MUSHROOM3 ||
		 type == OBJECT_MUSHROOM4 ||
		 type == OBJECT_MUSHROOM5 ||
		 type == OBJECT_MUSHROOM6 ||
		 type == OBJECT_MUSHROOM7 ||
		 type == OBJECT_MUSHROOM8 ||
		 type == OBJECT_MUSHROOM9 )
	{
		if ( bPlumb == -1 )  bPlumb = TRUE;
		pObject = new CObject(m_iMan);
		pObject->CreateMushroom(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_QUARTZ0 ||
		 type == OBJECT_QUARTZ1 ||
		 type == OBJECT_QUARTZ2 ||
		 type == OBJECT_QUARTZ3 ||
		 type == OBJECT_QUARTZ4 ||
		 type == OBJECT_QUARTZ5 ||
		 type == OBJECT_QUARTZ6 ||
		 type == OBJECT_QUARTZ7 ||
		 type == OBJECT_QUARTZ8 ||
		 type == OBJECT_QUARTZ9 )
	{
		if ( bPlumb == -1 )  bPlumb = TRUE;
		pObject = new CObject(m_iMan);
		pObject->CreateQuartz(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_ROOT0 ||
		 type == OBJECT_ROOT1 ||
		 type == OBJECT_ROOT2 ||
		 type == OBJECT_ROOT3 ||
		 type == OBJECT_ROOT4 ||
		 type == OBJECT_ROOT5 ||
		 type == OBJECT_ROOT6 ||
		 type == OBJECT_ROOT7 ||
		 type == OBJECT_ROOT8 ||
		 type == OBJECT_ROOT9 )
	{
		if ( bPlumb == -1 )  bPlumb = TRUE;
		pObject = new CObject(m_iMan);
		pObject->CreateRoot(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_HOME1 ||
		 type == OBJECT_HOME2 ||
		 type == OBJECT_HOME3 ||
		 type == OBJECT_HOME4 ||
		 type == OBJECT_HOME5 )
	{
		if ( bPlumb == -1 )  bPlumb = TRUE;
		pObject = new CObject(m_iMan);
		pObject->CreateHome(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_RUINmobilew1 ||
		 type == OBJECT_RUINmobilew2 ||
		 type == OBJECT_RUINmobilet1 ||
		 type == OBJECT_RUINmobilet2 ||
		 type == OBJECT_RUINmobiler1 ||
		 type == OBJECT_RUINmobiler2 ||
		 type == OBJECT_RUINfactory  ||
		 type == OBJECT_RUINdoor     ||
		 type == OBJECT_RUINsupport  ||
		 type == OBJECT_RUINradar    ||
		 type == OBJECT_RUINconvert  ||
		 type == OBJECT_RUINbase     ||
		 type == OBJECT_RUINhead     )
	{
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->CreateRuin(pos, angle, zoom, height, type, bPlumb);
	}
	else
	if ( type == OBJECT_HUMAN    ||
		 type == OBJECT_TECH     ||
		 type == OBJECT_CAR      ||
		 type == OBJECT_MOBILEtg ||
		 type == OBJECT_MOBILEfb ||
		 type == OBJECT_MOBILEob ||
		 type == OBJECT_TRAX     ||
		 type == OBJECT_UFO      )
	{
		model = m_dialog->RetModel();
		subModel = m_dialog->RetSubModel();
		if ( bPlumb == -1 )  bPlumb = FALSE;
		pObject = new CObject(m_iMan);
		pObject->SetOption(option);
		pObject->CreateVehicle(pos, angle, zoom, type, model, subModel, bPlumb, bTrainer);
	}

	if ( m_bFixScene && type == OBJECT_HUMAN )
	{
		CMotion*	motion;

		motion = pObject->RetMotion();
		if ( m_phase == PHASE_WIN  )  motion->SetAction(MHS_WIN, 0.4f);
		if ( m_phase == PHASE_LOST )  motion->SetAction(MHS_DEADg, 0.5f);
	}

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


// Crée le modèle éditable.

void CRobotMain::CreateModel()
{
	D3DVECTOR		direction;
	D3DCOLORVALUE	color;

	m_engine->SetAmbiantColor(0xC0C0C0C0);  // gris
	m_engine->SetBackground("", 0x80808080, 0x80808080, 0x80808080, 0x80808080);
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

	InitEye();

	m_engine->TimeInit();
	m_time = 0.0f;
	m_gameTime = 0.0f;
	m_recordTime = 0.0f;
	m_checkEndTime = -10.0f;
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

	if ( !m_engine->RetLightMode() )  return -1;

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
	CObject*		vehicle;
	D3DCOLORVALUE	colorRef1, colorNew1, colorRef2, colorNew2;
	FPOINT			ts, ti;
	FPOINT			exclu[6];
	int				model, subModel;

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

	vehicle = SearchObject(OBJECT_CAR);
	if ( vehicle != 0 )
	{
		model = vehicle->RetModel();
		subModel = vehicle->RetSubModel();
		ChangeColorCar(model, subModel, m_dialog->RetGamerColorCar());
	}

	colorRef2.r = 0.0f;
	colorRef2.g = 0.0f;
	colorRef2.b = 0.0f;
	colorNew2.r = 0.0f;
	colorNew2.g = 0.0f;
	colorNew2.b = 0.0f;

	m_engine->ChangeColor("bot1.tga",    m_colorRefBot, m_colorNewBot, colorRef2, colorNew2, 0.10f, -1.0f, ts, ti, 0, 0, TRUE);

	exclu[0] = FPOINT(196.0f/256.0f,   0.0f/256.0f);
	exclu[1] = FPOINT(256.0f/256.0f, 256.0f/256.0f);  // structure
	exclu[2] = FPOINT(0.0f, 0.0f);
	exclu[3] = FPOINT(0.0f, 0.0f);  // terminateur
	m_engine->ChangeColor("plant.tga",   m_colorRefGreen, m_colorNewGreen, colorRef2, colorNew2, 0.50f, -1.0f, ts, ti, exclu);

	// PARTIPLOUF0 et PARTIDROP :
	ts = FPOINT(0.500f, 0.500f);
	ti = FPOINT(0.875f, 0.750f);
	m_engine->ChangeColor("effect00.tga", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, TRUE);

	// PARTIFLIC :
	ts = FPOINT(0.00f, 0.75f);
	ti = FPOINT(0.25f, 1.00f);
	m_engine->ChangeColor("effect02.tga", m_colorRefWater, m_colorNewWater, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, m_colorShiftWater, TRUE);
}

// Change la couleur d'un modèle de voiture.

void CRobotMain::ChangeColorCar(int model, int subModel, D3DCOLORVALUE color)
{
	D3DCOLORVALUE	colorRef1, colorNew1, colorRef2, colorNew2;
	FPOINT			ts, ti;
	FPOINT			exclu[6];
	char			texName[20];

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

	if ( subModel == 0 )
	{
		sprintf(texName, "car%.2d.tga", model);
	}
	if ( subModel == 1 )
	{
		sprintf(texName, "car%.2db.tga", model);
	}
	if ( subModel == 2 )
	{
		sprintf(texName, "car%.2dc.tga", model);
	}
	if ( subModel == 3 )
	{
		sprintf(texName, "car%.2dd.tga", model);
	}

	if ( model == 1 )  // tijuana taxi ?
	{
		colorRef1.r =  71.0f/256.0f;
		colorRef1.g =  67.0f/256.0f;
		colorRef1.b = 255.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, 0, TRUE);
	}
	if ( model == 2 )  // ford 32 ?
	{
		colorRef1.r = 143.0f/256.0f;
		colorRef1.g =  88.0f/256.0f;
		colorRef1.b = 224.0f/256.0f;  // violet
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.08f, -1.0f, ts, ti, 0, 0, TRUE);
	}
	if ( model == 3 )  // pickup ?
	{
		exclu[0] = FPOINT(  0.0f/256.0f, 160.0f/256.0f);
		exclu[1] = FPOINT( 96.0f/256.0f, 256.0f/256.0f);  // métal
		exclu[2] = FPOINT(0.0f, 0.0f);
		exclu[3] = FPOINT(0.0f, 0.0f);  // terminateur
		colorRef1.r =  72.0f/256.0f;
		colorRef1.g = 153.0f/256.0f;
		colorRef1.b = 236.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, exclu, 0, TRUE);
	}
	if ( model == 4 )  // firecraker ?
	{
		colorRef1.r =  56.0f/256.0f;
		colorRef1.g =  66.0f/256.0f;
		colorRef1.b = 196.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, 0, TRUE);
	}
	if ( model == 5 )  // hooligan ?
	{
		colorRef1.r =   0.0f/256.0f;
		colorRef1.g =   0.0f/256.0f;
		colorRef1.b = 255.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, 0, TRUE);
	}
	if ( model == 6 )  // chevy ?
	{
		colorRef1.r =   0.0f/256.0f;
		colorRef1.g =   0.0f/256.0f;
		colorRef1.b = 255.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.08f, -1.0f, ts, ti, 0, 0, TRUE);
	}
	if ( model == 7 )  // reo ?
	{
		colorRef1.r =   0.0f/256.0f;
		colorRef1.g =   0.0f/256.0f;
		colorRef1.b = 255.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.08f, -1.0f, ts, ti, 0, 0, TRUE);
	}
	if ( model == 8 )  // torpedo ?
	{
		colorRef1.r =  71.0f/256.0f;
		colorRef1.g =  67.0f/256.0f;
		colorRef1.b = 255.0f/256.0f;  // bleu
		colorNew1 = color;
		m_engine->ChangeColor(texName, colorRef1, colorNew1, colorRef2, colorNew2, 0.20f, -1.0f, ts, ti, 0, 0, TRUE);
	}
}

// Met à jour le nombre d'objets non indispansables.

BOOL CRobotMain::TestGadgetQuantity(int rank)
{
	float		percent;
	int			*table;

	static int table10[10] = {0,1,0,0,0,0,0,0,0,0};
	static int table20[10] = {0,1,0,0,0,1,0,0,0,0};
	static int table30[10] = {0,1,0,1,0,1,0,0,0,0};
	static int table40[10] = {0,1,0,1,0,1,0,1,0,0};
	static int table50[10] = {0,1,0,1,0,1,0,1,0,1};
	static int table60[10] = {0,1,0,1,1,1,0,1,0,1};
	static int table70[10] = {0,1,0,1,1,1,0,1,1,1};
	static int table80[10] = {0,1,1,1,1,1,0,1,1,1};
	static int table90[10] = {0,1,1,1,1,1,1,1,1,1};

	percent = m_engine->RetGadgetQuantity();
	if ( percent == 0.0f )  return FALSE;
	if ( percent == 1.0f )  return TRUE;

	     if ( percent <= 0.15f )  table = table10;
	else if ( percent <= 0.25f )  table = table20;
	else if ( percent <= 0.35f )  table = table30;
	else if ( percent <= 0.45f )  table = table40;
	else if ( percent <= 0.55f )  table = table50;
	else if ( percent <= 0.65f )  table = table60;
	else if ( percent <= 0.75f )  table = table70;
	else if ( percent <= 0.85f )  table = table80;
	else                          table = table90;

	return table[rank%10];
}



// Calcule la distance jusqu'à l'objet le plus proche.

float CRobotMain::SearchNearestObject(D3DVECTOR center, CObject *exclu)
{
	CObject*	pObj;
	ObjectType	type;
	D3DVECTOR	oPos;
	float		min, dist, oRadius;
	int			i, j;

	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetActif() )  continue;  // inactif ?
		if ( pObj->RetTruck() != 0 )  continue;  // objet porté ?
		if ( pObj == exclu )  continue;

		type = pObj->RetType();

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			dist = Length(center, oPos)-oRadius;
			if ( dist < 0.0f )  dist = 0.0f;
			min = Min(min, dist);
		}
	}
	return min;
}

// Calcule un emplacement libre.

BOOL CRobotMain::FreeSpace(D3DVECTOR &center, float minRadius, float maxRadius,
						   float space, CObject *exclu)
{
	D3DVECTOR	pos;
	FPOINT		p;
	float		radius, ia, angle, dist, flat;

	if ( minRadius < maxRadius )  // de l'intérieur vers l'extérieur ?
	{
		for ( radius=minRadius ; radius<=maxRadius ; radius+=space )
		{
			ia = space/radius;
			for ( angle=0.0f ; angle<PI*2.0f ; angle+=ia )
			{
				p.x = center.x+radius;
				p.y = center.z;
				p = RotatePoint(FPOINT(center.x, center.z), angle, p);
				pos.x = p.x;
				pos.z = p.y;
				pos.y = 0.0f;
				m_terrain->MoveOnFloor(pos, TRUE);
				dist = SearchNearestObject(pos, exclu);
				if ( dist >= space )
				{
					flat = m_terrain->RetFlatZoneRadius(pos, dist/2.0f);
					if ( flat >= dist/2.0f )
					{
						center = pos;
						return TRUE;
					}
				}
			}
		}
	}
	else	// de l'extérieur vers l'intérieur ?
	{
		for ( radius=maxRadius ; radius>=minRadius ; radius-=space )
		{
			ia = space/radius;
			for ( angle=0.0f ; angle<PI*2.0f ; angle+=ia )
			{
				p.x = center.x+radius;
				p.y = center.z;
				p = RotatePoint(FPOINT(center.x, center.z), angle, p);
				pos.x = p.x;
				pos.z = p.y;
				pos.y = 0.0f;
				m_terrain->MoveOnFloor(pos, TRUE);
				dist = SearchNearestObject(pos, exclu);
				if ( dist >= space )
				{
					flat = m_terrain->RetFlatZoneRadius(pos, dist/2.0f);
					if ( flat >= dist/2.0f )
					{
						center = pos;
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

// Calcule le rayon maximal d'un emplacement libre.

float CRobotMain::RetFlatZoneRadius(D3DVECTOR center, float maxRadius,
									CObject *exclu)
{
	float	dist;

	dist = SearchNearestObject(center, exclu);
	if ( dist == 0.0f )  return 0.0f;
	if ( dist < maxRadius )
	{
		maxRadius = dist;
	}
	return m_terrain->RetFlatZoneRadius(center, maxRadius);
}


// Retourne un pointeur sur le dernier backslash d'un nom de fichier.

char* SearchLastDir(char *filename)
{
	char*	p = filename;

	while ( *p++ != 0 );
	p --;  // ^sur le zéro terminateur

	while ( p != filename )
	{
		if ( *(--p) == '\\' )  return p;
	}
	return 0;
}


// Compile tous les scripts des robots.

void CRobotMain::CompileScript()
{
	CObject*	pObj;
	CBrain*		brain;
	int			i, j, nbError, lastError, run;
	char*		name;

	nbError = 0;
	do
	{
		lastError = nbError;
		nbError = 0;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;
			if ( pObj->RetTruck() != 0 )  continue;

			brain = pObj->RetBrain();
			if ( brain == 0 )  continue;

			for ( j=0 ; j<10 ; j++ )
			{
				if ( brain->RetCompile(j) )  continue;

				name = brain->RetScriptName(j);
				if ( name[0] != 0 )
				{
					brain->ReadProgram(j, name);
					if ( !brain->RetCompile(j) )  nbError++;
				}
			}
		}
	}
	while ( nbError > 0 && nbError != lastError );

	// Démarre tous les programmes selon la commande "run".
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj->RetTruck() != 0 )  continue;

		brain = pObj->RetBrain();
		if ( brain == 0 )  continue;

		run = brain->RetScriptRun();
		if ( run != -1 )
		{
			brain->RunProgram(run);  // démarre le programme
		}
	}
}


// Recommencer avec un starter lent.

void CRobotMain::FlushStarterType()
{
	m_starterLast = 0;
	m_starterTry  = 0;
}

// Retourne le type d'action pour le droïde starter.

StarterType CRobotMain::RetStarterType()
{
	if ( m_starterLast == m_dialog->RetSceneRank() ||
		 m_starterTry >= 2 )
	{
		return STARTER_QUICK;
	}
	return STARTER_321;
}

// Retourne le type de course.

int CRobotMain::RetRaceType()
{
	return m_raceType;
}

// Retourne l'action pour le claxon.

int CRobotMain::RetHornAction()
{
	return m_hornAction;
}


// Informe de l'objet tamponné, pour éventuellement exécuter
// très rapidement CheckEndMission.

void CRobotMain::InfoCollision(ObjectType type)
{
	int		i;

	for ( i=0 ; i<m_endTakeTotal ; i++ )
	{
		if ( type == m_endTake[i].type )
		{
			m_checkEndTime = -10.0f;  // faudra faire CheckEndMission vite
		}
	}
}

// Vérifie si la mission est terminée.

Error CRobotMain::CheckEndMission(BOOL bFrame)
{
	CObject*	pObj;
	D3DVECTOR	bPos, oPos;
	ObjectType	type;
	Sound		sound;
	float		record, yourTime, recordTime;
	int			t, i, nb;
	char		res[100];
	char		text[100];

	if ( m_bSuperWin )
	{
		goto win;
	}

	if ( m_bSuperLost )
	{
		if ( m_lostDelay == 0.0f )
		{
			m_displayText->DisplayError(INFO_LOST, 1000.0f);
			m_lostDelay = m_endTakeLostDelay;  // perdu dans 6 secondes
			m_winDelay  = 0.0f;
		}
		m_displayText->SetEnable(FALSE);
		return INFO_LOST;
	}

	if ( m_raceType == 1 &&  // course contre la montre ?
		 m_gameTime*(1.0f/m_bonusLimit) > 1.0f )
	{
		if ( m_lostDelay == 0.0f )
		{
			m_displayText->DisplayError(INFO_LOST, 1000.0f);
			m_lostDelay = m_endTakeLostDelay;  // perdu dans 6 secondes
			m_winDelay  = 0.0f;
		}
		m_displayText->SetEnable(FALSE);
		return INFO_LOST;
	}

	for ( t=0 ; t<m_endTakeTotal ; t++ )
	{
		bPos = m_endTake[t].pos;
		bPos.y = 0.0f;

		nb = 0;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;

			// Ne pas utiliser RetActif(), car un ver invisible (sous terre)
			// doit être considéré comme existant ici !
//?			if ( pObj->RetLock() )  continue;
			if ( pObj->RetRuin() )  continue;
			if ( !pObj->RetEnable() )  continue;
			if ( pObj->RetGhost() )  continue;

			type = pObj->RetType();
			if ( type != m_endTake[t].type )  continue;

			if ( type == OBJECT_CAR )  // voiture ?
			{
				if ( pObj->RetDead() )  continue;  // ignore si cassé
			}

			if ( type != OBJECT_WALKER &&
				 type != OBJECT_CRAZY  &&
				 type != OBJECT_GUIDE  )
			{
				if ( pObj->RetExplo() )  continue;
			}

			if ( type == OBJECT_ALIEN8   ||  // générateur ?
				 type == OBJECT_ALIEN9   ||  // torture ?
				 type == OBJECT_COMPUTER ||  // ordinateur ?
				 type == OBJECT_REPAIR   )   // centre de réparation ?
			{
				if ( pObj->RetLock() )  continue;  // ignore si cassé
			}

			if ( pObj->RetTruck() == 0 )
			{
				oPos = pObj->RetPosition(0);
			}
			else
			{
				oPos = pObj->RetTruck()->RetPosition(0);
			}
			oPos.y = 0.0f;
			if ( Length2d(oPos, bPos) <= m_endTake[t].dist )
			{
				nb ++;
			}
		}

		if ( m_endTake[t].show != 0 )  // affiche le reste ?
		{
			if ( m_endTake[t].last != nb && m_endTake[t].last != -1 )
			{
				m_progressLevel += (nb-m_endTake[t].last)*m_endTake[t].show;
			}
			m_endTake[t].last = nb;
		}

		if ( nb <= m_endTake[t].lost )
		{
			if ( m_endTake[t].type == OBJECT_HUMAN )
			{
				if ( m_lostDelay == 0.0f )
				{
					m_lostDelay = 0.1f;  // perdu immédiatement
					m_winDelay  = 0.0f;
				}
				m_displayText->SetEnable(FALSE);
				return INFO_LOSTq;
			}
			else
			{
				if ( m_lostDelay == 0.0f )
				{
					m_displayText->DisplayError(INFO_LOST, 1000.0f);
					m_lostDelay = m_endTakeLostDelay;  // perdu dans 6 secondes
					m_winDelay  = 0.0f;
				}
				m_displayText->SetEnable(FALSE);
				return INFO_LOST;
			}
		}
		if ( nb < m_endTake[t].min ||
			 nb > m_endTake[t].max )
		{
			m_displayText->SetEnable(TRUE);
			return ERR_MISSION_NOTERM;
		}
		if ( m_endTake[t].bImmediat )
		{
			goto win;
		}
	}

	if ( m_endTakeResearch != 0 )
	{
		if ( m_endTakeResearch != (m_endTakeResearch&g_researchDone) )
		{
			m_displayText->SetEnable(TRUE);
			return ERR_MISSION_NOTERM;
		}
	}

	if ( m_endTakeWinDelay == -1.0f )
	{
		m_winDelay  = 1.0f;  // gagné dans 1 seconde
		m_lostDelay = 0.0f;
		m_displayText->SetEnable(FALSE);
		return ERR_OK;  // mission terminée
	}

win:
	if ( m_winDelay == 0.0f )
	{
		if ( m_dialog->RetDuel()   &&  // duel ?
			 m_recorderPlay != 0 )
		{
			yourTime = m_gameTime-m_statStartTime;
			recordTime = m_recorderPlay->RetChrono();
			if ( yourTime <= recordTime )
			{
				GetResource(RES_TEXT, RT_DUEL_WIN, res);
				sprintf(text, res, recordTime-yourTime);
				sound = SOUND_RECORDgall;
				if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				m_displayText->DisplayText(text, 10.0f, FONTSIZE, TT_START, sound);
			}
			else
			{
				GetResource(RES_TEXT, RT_DUEL_LOST, res);
				sprintf(text, res, yourTime-recordTime);
				sound = SOUND_RECORDgall;
				if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				m_displayText->DisplayText(text, 10.0f, FONTSIZE, TT_ERROR, sound);
			}
			m_endTakeWinDelay = 3.0f;
		}
		else
		{
			record = m_dialog->TimeRecord(m_gameTime-m_statStartTime, 0);
			if ( record > 0.0f && !m_dialog->FirstRecord(0) )
			{
				GetResource(RES_TEXT, RT_RECORD_GALL, res);
				sprintf(text, res, record);
				sound = SOUND_RECORDgall;
				if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				m_displayText->DisplayText(text, 10.0f, FONTSIZE, TT_START, sound);
				m_bStatRecordAll = TRUE;
			}
			else
			{
				m_displayText->DisplayError(INFO_WIN);
			}
		}
		m_winDelay  = m_endTakeWinDelay;  // gagné dans 6 secondes
		m_lostDelay = 0.0f;
	}
	m_displayText->SetEnable(FALSE);
	return ERR_OK;  // mission terminée
}

// Retourne le nombre de portes par tour.

int CRobotMain::RetLapProgress()
{
	return m_progressLap;
}

// Retourne le type d'objet détruit qui doit être signalé dans la progression.

ObjectType CRobotMain::RetTypeProgress()
{
	return m_progressType;
}

// Avance la progression.
// Retourne TRUE si un son a été généré.

BOOL CRobotMain::IncProgress()
{
	float	time, record;
	char	res[100];
	char	text[100];
	BOOL	bSound = FALSE;
	Sound	sound;
	int		rank;

	if ( m_progressLap > 0 && m_progressLevel%m_progressLap == 0 )
	{
		time = m_gameTime-m_statLapTime;  // temps pour faire un tour
		m_statLapTime = m_gameTime;

		if ( m_progressLevel+m_progressAdd > 0 )
		{
			record = m_dialog->TimeRecord(time, 1);
			if ( record > 0.0f && !m_dialog->FirstRecord(0) )
			{
				GetResource(RES_TEXT, RT_RECORD_GONE, res);
				sprintf(text, res, record);
				sound = SOUND_RECORDgone;
				if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				m_displayText->DisplayText(text, 10.0f, FONTSIZE, TT_START, sound);
				bSound = TRUE;
				m_bStatRecordOne = TRUE;
			}
			else
			{
				record = m_statBestTime-time;
				if ( m_statBestTime != 0.0f && record > 0.0f )
				{
					GetResource(RES_TEXT, RT_RECORD_LONE, res);
					sprintf(text, res, record);
					sound = SOUND_RECORDlone;
					if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
					m_displayText->DisplayText(text, 10.0f, FONTSIZE*0.75f, TT_START, sound);
					bSound = TRUE;
				}
			}

			m_statBestTime = time;
			m_dialog->AddRecord(time, 0.0f, 0.0f, 0.0f, 0, 1);

			rank = (m_progressTotal-m_progressLevel-m_progressAdd)/m_progressLap;
			if ( rank >= 1 && rank <= 4 )
			{
				if ( rank == 4 )
				{
					GetResource(RES_TEXT, RT_RACE_LAP4, res);
					sound = SOUND_LAP4;
					if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				}
				if ( rank == 3 )
				{
					GetResource(RES_TEXT, RT_RACE_LAP3, res);
					sound = SOUND_LAP3;
					if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				}
				if ( rank == 2 )
				{
					GetResource(RES_TEXT, RT_RACE_LAP2, res);
					sound = SOUND_LAP2;
					if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				}
				if ( rank == 1 )
				{
					GetResource(RES_TEXT, RT_RACE_LAP1, res);
					sound = SOUND_LAP1;
					if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
				}

				if ( bSound )
				{
					strcpy(m_messageText, res);
					m_messageDelay = 5.0f;
					m_messageSize  = FONTSIZE*0.75f;
					m_messageSound = sound;
					m_messageTime  = 1.0f;  // sera affiché dans 1 seconde
				}
				else
				{
					m_displayText->DisplayText(res, 5.0f, FONTSIZE*0.75f, TT_START, sound);
					bSound = TRUE;
				}
			}
		}
	}

	m_progressLevel ++;

	return bSound;
}

// Met à jour la progression sur le but à atteindre.

void CRobotMain::UpdateProgress()
{
	CProgress*	pp;

	if ( m_progressTotal == 0 ||
		 m_bSuspend           ||
		 m_bPause             )  return;

	pp = (CProgress*)m_interface->SearchControl(EVENT_PROGRESS);
	if ( pp == 0 )  return;

	pp->SetState(STATE_VISIBLE);
	pp->SetTotal(m_progressTotal);
	pp->SetProgress(m_progressLevel);
}

// Met à jour le temps écoulé.

void CRobotMain::UpdateTime()
{
	CEdit*	pe;
	CGauge*	pg;
	float	time;
	char	text[50];

	pe = (CEdit*)m_interface->SearchControl(EVENT_TIME);
	if ( pe == 0 )  return;

	if ( m_phase == PHASE_SIMUL )
	{
		if ( m_raceType == 1 )  // course contre la montre ?
		{
			time = m_bonusLimit-m_gameTime;  // décompte
			if ( time < 0.0f )  time = 0.0f;
		}
		else
		{
			time = m_gameTime;  // compte
		}
		PutTime(text, time);
		pe->SetText(text);
	}
	else
	{
		pe->ClearState(STATE_VISIBLE);
	}

	if ( m_bonusLimit != 0.0f )
	{
		pg = (CGauge*)m_interface->SearchControl(EVENT_BONUS);
		if ( pg == 0 )  return;
		pg->SetLevel(m_gameTime*(1.0f/m_bonusLimit));
	}
}

// Retourne le nombre de points gagnés grace au bonus de temps.

float CRobotMain::RetBonusPoints()
{
	float	pts;

	if ( m_bonusLimit == 0.0f )  return -1.0f;

	pts = ((m_statEndTime-m_statStartTime)-m_bonusRecord)/(m_bonusLimit-m_bonusRecord);
	pts = Norm(1.0f-pts)*100.0f;
	return pts;
}



// Indique s'il est possible de télécommander un robot d'entraînement.

BOOL CRobotMain::RetTrainerPilot()
{
	return m_bTrainerPilot;
}

// Indique si la scène est fixe, sans interraction.

BOOL CRobotMain::RetFixScene()
{
	return m_bFixScene;
}


char* CRobotMain::RetTitle()
{
	return m_title;
}

char* CRobotMain::RetResume()
{
	return m_resume;
}


BOOL CRobotMain::RetGlint()
{
	return m_dialog->RetGlint();
}

BOOL CRobotMain::RetMovies()
{
	return m_dialog->RetMovies();
}

BOOL CRobotMain::RetNiceReset()
{
	return m_dialog->RetNiceReset();
}

BOOL CRobotMain::RetHimselfDamage()
{
	return m_dialog->RetHimselfDamage();
}

BOOL CRobotMain::RetMotorBlast()
{
	return m_dialog->RetMotorBlast();
}

BOOL CRobotMain::RetShowAll()
{
	return m_bShowAll;
}

BOOL CRobotMain::RetCheatRadar()
{
	return m_bCheatRadar;
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

void CRobotMain::SetMovieLock(BOOL bLock)
{
	m_bMovieLock = bLock;
	m_engine->SetMovieLock(m_bMovieLock);

	m_map->ShowMap(!m_bMovieLock);
	if ( m_bMovieLock )  HiliteClear();
	m_engine->SetMouseHide(m_bMovieLock);
}

BOOL CRobotMain::RetMovieLock()
{
	return m_bMovieLock;
}

// Indique un film déroulé par un automat.

void CRobotMain::SetMovieAuto(CAuto *automat)
{
	m_movieAuto = automat;
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


// Gestion des modes gagné/perdu prioritaires.

void CRobotMain::SetSuperWin()
{
	m_bSuperWin = TRUE;
}

void CRobotMain::SetSuperLost()
{
	m_bSuperLost = TRUE;
}

void CRobotMain::SetSuperLost(D3DVECTOR lookat)
{
	m_bSuperLost = TRUE;
	m_bLostLookat = TRUE;
	m_lostLookat = lookat;
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


// Ecrit sur disque le fichier d'enregistrement.

void CRobotMain::RecorderWrite(int rank, int model)
{
	char	filename[100];

	sprintf(filename, "%s\\%s%.2d%.1d-%.2d.rec",
						m_dialog->RetSavegameDir(),
						m_dialog->RetSceneName(), rank/10, rank%10, model);

	m_recorderRecord->Write(filename);
}

// Lit sur disque le fichier d'enregistrement.

void CRobotMain::RecorderRead(int rank, int model)
{
	char	filename[100];
	char*	ghost;

	if ( m_recorderPlay != 0 )
	{
		delete m_recorderPlay;
		m_recorderPlay = 0;
	}

	ghost = m_dialog->RetGhostRead();
	if ( ghost[0] == 0 )
	{
		sprintf(filename, "%s\\%s%.2d%.1d-%.2d.rec",
							m_dialog->RetSavegameDir(),
							m_dialog->RetSceneName(), rank/10, rank%10, model);
	}
	else
	{
		strcpy(filename, ghost);
	}

	m_recorderPlay = new CRecorder(m_iMan);
	if ( !m_recorderPlay->Read(filename) )
	{
		delete m_recorderPlay;
		m_recorderPlay = 0;
	}
}

// Indique si un fichier d'enregistrement existe.

BOOL CRobotMain::RecorderExist(int rank, int model)
{
	FILE*	file;
	char	filename[100];

	sprintf(filename, "%s\\%s%.2d%.1d-%.2d.rec",
						m_dialog->RetSavegameDir(),
						m_dialog->RetSceneBase(), rank/10, rank%10, model);

	file = fopen(filename, "rb");
	if ( file == 0 )  return FALSE;
	fclose(file);
	return TRUE;
}

// Retourne l'enregistrement en cours.

CRecorder* CRobotMain::RetRecorderRecord()
{
	return m_recorderRecord;
}


// Indique un changement de l'état des décors.

void CRobotMain::IncDecorStamp()
{
	m_decorStamp ++;
}

// Retourne l'état des décors.

int CRobotMain::RetDecorStamp()
{
	return m_decorStamp;
}

