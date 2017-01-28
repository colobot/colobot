// brain.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "cbot/cbotdll.h"
#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "robotmain.h"
#include "terrain.h"
#include "water.h"
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "motion.h"
#include "pyro.h"
#include "taskmanager.h"
#include "task.h"
#include "script.h"
#include "interface.h"
#include "button.h"
#include "color.h"
#include "edit.h"
#include "list.h"
#include "label.h"
#include "group.h"
#include "gauge.h"
#include "slider.h"
#include "window.h"
#include "sound.h"
#include "displaytext.h"
#include "text.h"
#include "particule.h"
#include "cmdtoken.h"
#include "brain.h"





// Constructeur de l'objet.

CBrain::CBrain(CInstanceManager* iMan, CObject* object)
{
	int			i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_BRAIN, this, 100);

	m_object      = object;
	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_interface   = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_physics     = 0;
	m_motion      = 0;
	m_primaryTask = 0;
	m_secondaryTask = 0;

	m_program = -1;
	m_bActivity = TRUE;
	m_bBurn = FALSE;
	m_time = 0.0f;
	m_burnTime = 0.0f;
	m_lastUpdateTime = 0.0f;
	m_lastHumanTime = 0.0f;
	m_lastWormTime = 0.0f;
	m_beeBullet = 0;
	m_lastAlarmTime = 0.0f;
	m_soundChannelAlarm = -1;
	m_flagColor = 0;
	m_lastMotorState = 999;

	m_buttonAxe = EVENT_NULL;

	for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
	{
		m_script[i] = 0;
		m_scriptName[i][0] = 0;
	}
	m_scriptRun = -1;
	m_soluceName[0] = 0;
	m_selScript = 0;
}

// Destructeur de l'objet.

CBrain::~CBrain()
{
	int		i;

	for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
	{
		delete m_script[i];
	}

	delete m_primaryTask;
	delete m_secondaryTask;
	m_iMan->DeleteInstance(CLASS_BRAIN, this);
}


// Détruit l'objet.

void CBrain::DeleteObject(BOOL bAll)
{
	if ( m_soundChannelAlarm != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelAlarm);
		m_sound->AddEnvelope(m_soundChannelAlarm, 0.0f, 0.5f, 0.5f, SOPER_STOP);
		m_soundChannelAlarm = -1;
	}

	if ( !bAll )
	{
		if ( m_beeBullet != 0 )
		{
			m_beeBullet->DeleteObject();
			delete m_beeBullet;
			m_beeBullet = 0;
		}
	}
}


void CBrain::SetPhysics(CPhysics* physics)
{
	m_physics = physics;
}

void CBrain::SetMotion(CMotion* motion)
{
	m_motion = motion;
}


// Gestion d'un événement.

BOOL CBrain::EventProcess(const Event &event)
{
	EventMsg	action;
	ObjectType	type;
	float		axeX, axeY, axeZ, axeW;

	type = m_object->RetType();

	if ( m_primaryTask != 0 )  // tâche en cours ?
	{
		m_primaryTask->EventProcess(event);
	}

	if ( m_secondaryTask != 0 )  // tâche en cours ?
	{
		m_secondaryTask->EventProcess(event);
	}

	action = event.event;

	if ( action == EVENT_NULL )  return TRUE;

	if ( action == EVENT_UPDINTERFACE )
	{
		if ( m_object->RetSelect() )  CreateInterface(TRUE);
	}

	if ( action == EVENT_FRAME )
	{
		EventFrame(event);
	}

	if ( !m_object->RetSelect() &&  // robot pas sélectionné ?
		 m_program == -1        &&
		 m_primaryTask == 0     )
	{
		axeX = 0.0f;
		axeY = 0.0f;
		axeZ = 0.0f;
		axeW = 0.0f;
		if ( m_object->RetBurn() )  // brûle ?
		{
			if ( !m_bBurn )  // début ?
			{
				m_bBurn = TRUE;
				m_burnTime = 0.0f;
			}

			axeZ = -1.0f;  // tombe
		}
		m_physics->SetMotorSpeedX(axeY);  // avancer/reculer
		m_physics->SetMotorSpeedY(axeZ);  // monter/descendre
		m_physics->SetMotorSpeedZ(axeX);  // tourner
		m_physics->SetMotorSpeedW(axeW);  // frain à main
		return TRUE;
	}

	if ( m_program != -1     &&
		 m_object->RetRuin() )
	{
		StopProgram();
		return TRUE;
	}

	if ( !m_object->RetSelect() )  // robot pas sélectionné ?
	{
		return TRUE;
	}

	if ( m_primaryTask != 0 ||  // tâche en cours ?
		 m_program != -1    )
	{
		if ( m_primaryTask == 0 || !m_primaryTask->IsPilot() )  return TRUE;
	}

	if ( action == EVENT_LBUTTONUP ||
		 action == EVENT_RBUTTONUP )
	{
		m_buttonAxe = EVENT_NULL;
	}

	axeX = event.axeX;
	axeY = event.axeY;
	axeZ = event.axeZ;
	axeW = event.axeW;

	if ( !m_main->RetTrainerPilot() &&
		 m_object->RetTrainer()     )  // véhicule d'entraînement ?
	{
		axeX = 0.0f;
		axeY = 0.0f;
		axeZ = 0.0f;
		axeW = 0.0f;  // télécommande impossible !
	}

	if ( (g_researchDone&RESEARCH_FLY) == 0 )
	{
		axeZ = -1.0f;  // tombe
	}

	m_physics->SetMotorSpeedX(axeY);  // avancer/reculer
	m_physics->SetMotorSpeedY(axeZ);  // monter/descendre
	m_physics->SetMotorSpeedZ(axeX);  // tourner
	m_physics->SetMotorSpeedW(axeW);  // frain à main

	return TRUE;
}


// Fait évoluer le cerveau selon le temps écoulé.

BOOL CBrain::EventFrame(const Event &event)
{
	m_time += event.rTime;
	if ( m_bBurn )  m_burnTime += event.rTime;

	if ( m_soundChannelAlarm != -1 )
	{
		m_sound->Position(m_soundChannelAlarm, m_object->RetPosition(0));
	}

	UpdateInterface(event.rTime);

	if ( m_engine->RetPause() )  return TRUE;
	if ( !m_bActivity )  return TRUE;  // attend si inactif
	if ( EndedTask() == ERR_CONTINUE )  return TRUE;  // attend si pas fini ...

	if ( m_program != -1 )  // programme en cours ?
	{
		if ( m_script[m_program]->Continue(event) )
		{
			StopProgram();
		}
	}

	return TRUE;
}


// Stoppe le programme en cours.

void CBrain::StopProgram()
{
	StopTask();

	if ( m_object->RetType() == OBJECT_HUMAN ||
		 m_object->RetType() == OBJECT_TECH  )  return;

	if ( m_program != -1 &&
		 m_script[m_program] != 0 )
	{
		m_script[m_program]->Stop();
	}

	m_program = -1;

	m_physics->SetMotorSpeedX(0.0f);
	m_physics->SetMotorSpeedY(0.0f);
	m_physics->SetMotorSpeedZ(0.0f);
	m_physics->SetMotorSpeedW(0.0f);
	m_physics->SetLinMotion(MO_MOTSPEED, D3DVECTOR(0.0f, 0.0f, 0.0f));
	m_physics->SetLinMotion(MO_CURSPEED, D3DVECTOR(0.0f, 0.0f, 0.0f));
	m_physics->SetLinMotion(MO_REASPEED, D3DVECTOR(0.0f, 0.0f, 0.0f));

	m_motion->SetAction(-1);

	UpdateInterface();
	m_object->CreateSelectParticule();
}

// Stoppe la tâche en cours.

void CBrain::StopTask()
{
	if ( m_primaryTask != 0 )
	{
		m_primaryTask->Abort();
		delete m_primaryTask;  // stoppe la tâche en cours
		m_primaryTask = 0;
	}
}


// Tire.

Error CBrain::StartTaskFire(float delay)
{
	Error	err;

	m_primaryTask = new CTaskManager(m_iMan, m_object);
	err = m_primaryTask->StartTaskFire(delay);
	UpdateInterface();
	return err;
}

// Termine la tâche lorsque le moment est venu.

Error CBrain::EndedTask()
{
	Error	err;

	if ( m_secondaryTask != 0 )  // tâche en cours ?
	{
		err = m_secondaryTask->IsEnded();
		if ( err != ERR_CONTINUE )  // tâche terminée ?
		{
			delete m_secondaryTask;
			m_secondaryTask = 0;
			UpdateInterface();
		}
	}

	if ( m_primaryTask != 0 )  // tâche en cours ?
	{
		err = m_primaryTask->IsEnded();
		if ( err != ERR_CONTINUE )  // tâche terminée ?
		{
			delete m_primaryTask;
			m_primaryTask = 0;
			UpdateInterface();
		}
		return err;
	}
	return ERR_STOP;
}


// Choix de la couleur pour un indicateur de couleur.

void CBrain::ColorFlag(int color)
{
	m_flagColor = color;
	UpdateInterface();
}


// Crée toute l'interface lorsque l'objet est sélectionné.

BOOL CBrain::CreateInterface(BOOL bSelect)
{
	ObjectType	type;
	CWindow*	pw;
	CGauge*		pg;
	CLabel*		pl;
	FPOINT		pos, dim, ddim;
	float		ox, oy, sx, sy;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw != 0 )
	{
		pw->Flush();  // détruit les boutons de la fenêtre
		m_interface->DeleteControl(EVENT_WINDOW0);  // détruit la fenêtre
	}

	if ( !bSelect )  return TRUE;

	type = m_object->RetType();
	if ( type == OBJECT_HUMAN  )  return TRUE;

	pos.x = 0.0f;
	pos.y = 0.0f;
//?	dim.x = 540.0f/640.0f;
//?	dim.y =  71.0f/480.0f;
	dim.x = 0.0f;
	dim.y = 0.0f;
	m_interface->CreateWindows(pos, dim, 3, EVENT_WINDOW0);
	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw == 0 )  return FALSE;

	dim.x = 33.0f/640.0f;
	dim.y = 33.0f/480.0f;
	ox = 3.0f/640.0f;
	oy = 3.0f/480.0f;
	sx = 33.0f/640.0f;
	sy = 33.0f/480.0f;

	if ( type == OBJECT_CAR )
	{
		pos.x  =  10.0f/640.0f;
		pos.y  =  10.0f/480.0f;
		ddim.x =  90.0f/640.0f;
		ddim.y =  90.0f/480.0f;
		pg = pw->CreateGauge(pos, ddim, 1, EVENT_OBJECT_GSPEED);
		pg->SetMode(GM_SPEED);

		pos.x  = 100.0f/640.0f;
		pos.y  =  10.0f/480.0f;
		ddim.x =  70.0f/640.0f;
		ddim.y =  70.0f/480.0f;
		pg = pw->CreateGauge(pos, ddim, 1, EVENT_OBJECT_GRPM);
		pg->SetMode(GM_RPM);

//?		pos.x  = 126.0f/640.0f;
//?		pos.y  =  36.0f/480.0f;
//?		ddim.x =  18.0f/640.0f;
//?		ddim.y =  18.0f/480.0f;
		pos.x  = 124.0f/640.0f;
		pos.y  =  26.0f/480.0f;
		ddim.x =  22.0f/640.0f;
		ddim.y =  22.0f/480.0f;
		pl = pw->CreateLabel(pos, ddim, 1, EVENT_OBJECT_GMOTOR, "");
		pl->SetFontType(FONT_HILITE);
		pl->SetFontSize(8.0f);

		pos.x  = 470.0f/640.0f;
		pos.y  =  10.0f/480.0f;
		ddim.x =  50.0f/640.0f;
		ddim.y =  50.0f/480.0f;
		pg = pw->CreateGauge(pos, ddim, 1, EVENT_OBJECT_GCOMPASS);
		pg->SetMode(GM_COMPASS);

		pos.x  = 525.0f/640.0f;
		pos.y  =  10.0f/480.0f;
		ddim.x =  50.0f/640.0f;
		ddim.y =  50.0f/480.0f;
		pg = pw->CreateGauge(pos, ddim, 1, EVENT_OBJECT_GLEVEL1);
		pg->SetMode(GM_LEVEL1);

		pos.x  = 580.0f/640.0f;
		pos.y  =  10.0f/480.0f;
		ddim.x =  50.0f/640.0f;
		ddim.y =  50.0f/480.0f;
		pg = pw->CreateGauge(pos, ddim, 1, EVENT_OBJECT_GLEVEL2);
		pg->SetMode(GM_LEVEL2);
	}

	UpdateInterface();
	m_lastUpdateTime = 0.0f;
	UpdateInterface(0.0f);

	return TRUE;
}

// Met à jour l'état de tous les boutons de l'interface,
// suite au temps qui s'écoule ...

void CBrain::UpdateInterface(float rTime)
{
	CWindow*	pw;
	CGauge*		pg;
	CLabel*		pl;
	CMotion*	motion;
	D3DVECTOR	pos, hPos;
	FPOINT		ppos;
	float		angle, range;
	int			state, i;
	char		res[10];

	m_lastAlarmTime += rTime;
	if ( m_time < m_lastUpdateTime+0.1f )  return;
	m_lastUpdateTime = m_time;

	if ( !m_object->RetSelect() )
	{
		if ( m_soundChannelAlarm != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelAlarm);
			m_sound->AddEnvelope(m_soundChannelAlarm, 0.0f, 1.0f, 0.1f, SOPER_STOP);
			m_soundChannelAlarm = -1;
		}
		return;
	}

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw == 0 )  return;

	pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GSPEED);
	if ( pg != 0 )
	{
		range = m_physics->RetLinMotionX(MO_REASPEED);
		pg->SetLevel(Abs(range));
	}

	pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GRPM);
	if ( pg != 0 )
	{
		range = m_physics->RetMotorRPM();
		pg->SetLevel(range);
	}

	pl = (CLabel*)pw->SearchControl(EVENT_OBJECT_GMOTOR);
	if ( pl != 0 )
	{
		state = m_physics->RetMotorState();
		if ( state != m_lastMotorState )
		{
			m_lastMotorState = state;
			i = -1;
			if ( state == -1 )  i = RT_MOTOR_REAR;
			if ( state ==  0 )  i = RT_MOTOR_NEUTRAL;
			if ( state ==  1 )  i = RT_MOTOR_S1;
			if ( state ==  2 )  i = RT_MOTOR_S2;
			if ( state ==  3 )  i = RT_MOTOR_S3;
			if ( i != -1 )
			{
				GetResource(RES_TEXT, i, res);
				pl->SetName(res);
			}
		}
	}

	pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GCOMPASS);
	if ( pg != 0 )
	{
		angle = -(m_object->RetAngleY(0)+PI/2.0f);
		pg->SetLevel(angle);
	}

	pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GLEVEL1);
	if ( pg != 0 )
	{
		motion = m_object->RetMotion();
		if ( motion != 0 )
		{
			range = (float)motion->RetUsedPart()/motion->RetTotalPart();
			pg->SetLevel(Abs(range));
		}
	}

	pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GLEVEL2);
	if ( pg != 0 )
	{
		range = m_object->RetBurnShield();
		pg->SetLevel(Abs(range));
	}
}

// Met à jour l'état de tous les boutons de l'interface.

void CBrain::UpdateInterface()
{
}


// Gestion de l'activité d'un objet.

void CBrain::SetActivity(BOOL bMode)
{
	m_bActivity = bMode;
}

BOOL CBrain::RetActivity()
{
	return m_bActivity;
}

// Indique si un programme est en cours d'exécution.

BOOL CBrain::IsProgram()
{
	return ( m_program != -1 );
}

// Indique si un programme existe.

BOOL CBrain::ProgramExist(int rank)
{
	return ( m_script[rank] != 0 );
}

// Démarre un programme donné.

void CBrain::RunProgram(int rank)
{
	if ( rank < 0 )  return;

	if ( m_script[rank] != 0 &&
		 m_script[rank]->Run() )
	{
		m_program = rank;  // démarre nouveau programme
		m_object->CreateSelectParticule();
	}
}

// Retourne le premier programme libre.

int CBrain::FreeProgram()
{
	int		i;

	for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
	{
		if ( m_script[i] == 0 )  return i;
	}
	return -1;
}


// Retourne le programme en cours.

int CBrain::RetProgram()
{
	return m_program;
}


// Gestion du nom des scripts à charger.

void CBrain::SetScriptRun(int rank)
{
	m_scriptRun = rank;
}

int CBrain::RetScriptRun()
{
	return m_scriptRun;
}

void CBrain::SetScriptName(int rank, char *name)
{
	strcpy(m_scriptName[rank], name);
}

char* CBrain::RetScriptName(int rank)
{
	return m_scriptName[rank];
}

void CBrain::SetSoluceName(char *name)
{
	strcpy(m_soluceName, name);
}

char* CBrain::RetSoluceName()
{
	return m_soluceName;
}


// Charge un script solution, dans le premier script libre.
// S'il existe déjà un script identique, rien n'est chargé.

BOOL CBrain::ReadSoluce(char* filename)
{
	int		rank, i;

	rank = FreeProgram();
	if ( rank == -1 )  return FALSE;

	if ( !ReadProgram(rank, filename) )  return FALSE;  // charge solution

	for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
	{
		if ( i == rank || m_script[i] == 0 )  continue;

		if ( m_script[i]->Compare(m_script[rank]) )  // déjà un même ?
		{
			delete m_script[rank];
			m_script[rank] = 0;
			return FALSE;
		}
	}

	return TRUE;
}

// Charge un script avec un fichier texte.

BOOL CBrain::ReadProgram(int rank, char* filename)
{
	if ( m_script[rank] == 0 )
	{
		m_script[rank] = new CScript(m_iMan, m_object, &m_secondaryTask);
	}

	if ( m_script[rank]->ReadScript(filename) )  return TRUE;

	delete m_script[rank];
	m_script[rank] = 0;

	return FALSE;
}

// Indique si un programme est correctement compilé.

BOOL CBrain::RetCompile(int rank)
{
	if ( m_script[rank] == 0 )  return FALSE;
	return m_script[rank]->RetCompile();
}

// Sauve un script dans un fichier texte.

BOOL CBrain::WriteProgram(int rank, char* filename)
{
	if ( m_script[rank] == 0 )
	{
		m_script[rank] = new CScript(m_iMan, m_object, &m_secondaryTask);
	}

	if ( m_script[rank]->WriteScript(filename) )  return TRUE;

	delete m_script[rank];
	m_script[rank] = 0;

	return FALSE;
}


// Charge un stack de script en exécution avec un fichier.

BOOL CBrain::ReadStack(FILE *file)
{
	short		op;

	fRead(&op, sizeof(short), 1, file);
	if ( op == 1 )  // run ?
	{
		fRead(&op, sizeof(short), 1, file);  // program rank
		if ( op >= 0 && op < BRAINMAXSCRIPT )
		{
			m_program = op;  // redémarre programme
			m_selScript = op;

			if ( m_script[op] == 0 )
			{
				m_script[op] = new CScript(m_iMan, m_object, &m_secondaryTask);
			}
			if ( !m_script[op]->ReadStack(file) )  return FALSE;
		}
	}

	return TRUE;
}

// Sauve le stack du script en exécution dans un fichier.

BOOL CBrain::WriteStack(FILE *file)
{
	short		op;

	if ( m_program != -1 &&  // programme en cours ?
		 m_script[m_program]->IsRunning() )
	{
		op = 1;  // run
		fWrite(&op, sizeof(short), 1, file);

		op = m_program;
		fWrite(&op, sizeof(short), 1, file);

		return m_script[m_program]->WriteStack(file);
	}

	op = 0;  // stop
	fWrite(&op, sizeof(short), 1, file);
	return TRUE;
}


