// physics.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "robotmain.h"
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "camera.h"
#include "object.h"
#include "auto.h"
#include "pyro.h"
#include "brain.h"
#include "motion.h"
#include "motionvehicle.h"
#include "motionhuman.h"
#include "motionbot.h"
#include "sound.h"
#include "task.h"
#include "cmdtoken.h"
#include "physics.h"



#define LANDING_SPEED	3.0f
#define LANDING_ACCEL	5.0f
#define LANDING_ACCELh	1.5f

#define STARTDELAY		0.6f	// temps de mise en marche du moteur





BOOL IsInSegment(FPOINT p1, FPOINT p2, FPOINT p)
{
	if ( p1.x > p2.x )  Swap(p1.x, p2.x);
	if ( p1.y > p2.y )  Swap(p1.y, p2.y);

	return ( p.x >= p1.x && p.x <= p2.x &&
			 p.y >= p1.y && p.y <= p2.y );
}



// Constructeur de l'objet.

CPhysics::CPhysics(CInstanceManager* iMan, CObject* object)
{
	int		i;

	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_PHYSICS, this, 100);

	m_object    = object;
	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_terrain   = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	m_brain     = 0;
	m_motion    = 0;

	m_type = TYPE_RACE;
	m_gravity = 9.81f;  // gravité terrestre par défaut
	m_time = 0.0f;
	m_timeUnderWater = 0.0f;
	m_timeWheelBurn = 0.0f;
	m_timeMotorBurn = 0.0f;
	m_timeLock = 0.0f;
	m_motorSpeed = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_motorSpeedW = 0.0f;
	m_motorSpeedWk = 0.0f;
	m_forceSpeed = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_imprecisionA = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_imprecisionH = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_imprecisionT = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_bMotor = FALSE;
	m_bSilent = FALSE;
	m_bForceSlow = FALSE;
	m_bLand = TRUE;  // au sol
	m_bSwim = FALSE;  // dans l'air
	m_bCollision = FALSE;
	m_bObstacle = FALSE;
	m_repeatCollision = 0;
	m_doorRank = 0;
	m_linVibrationFactor = 1.0f;
	m_cirVibrationFactor = 1.0f;
	m_inclinaisonFactor  = 1.0f;
	m_lastSlideParticule = 0.0f;
	m_lastMotorParticule = 0.0f;
	m_lastWaterParticule = 0.0f;
	m_lastUnderParticule = 0.0f;
	m_lastPloufParticule = 0.0f;
	m_lastFlameParticule = 0.0f;
	m_lastCrashParticule = 0.0f;
	m_lastWheelParticule = 0.0f;
	m_lastSuspParticule  = 0.0f;
	m_bWheelParticuleBrake = FALSE;
	m_absorbWater        = 0.0f;
	m_reactorTemperature = 0.0f;
	m_timeReactorFail    = 0.0f;
	m_lastSoundCollision = 0.0f;
	m_lastSoundCrash     = 0.0f;
	m_lastSoundWater     = 0.0f;
	m_lastSoundInsect    = 0.0f;
	m_restBreakParticule = 0.0f;
	m_floorHeight        = 0.0f;
	m_soundChannelMotor1 = -1;
	m_soundChannelMotor2 = -1;
	m_soundChannelSlide = -1;
	m_soundChannelBrake = -1;
	m_soundChannelBoost = -1;
	m_soundChannelHorn  = -1;
	m_soundChannelGlide = -1;
	m_soundTimeJostle = 0.0f;
	m_soundTimeBoum = 0.0f;
	m_soundTimePshhh = 0.0f;
	m_soundAmplitudeMotor1 = 0.0f;
	m_soundFrequencyMotor1 = 0.3f;
	m_soundAmplitudeMotor2 = 0.0f;
	m_soundFrequencyMotor2 = 0.3f;
	m_glideTime = 0.0f;
	m_glideVolume = 0.0f;
	m_glideAmplitude = 0.0f;
	m_bFreeze = FALSE;
	m_bBrake = FALSE;
	m_bHandbrake = FALSE;
	m_bForceUpdate = TRUE;
	m_bLowLevel = FALSE;
	m_bWater = FALSE;
	m_bOldWater = FALSE;
	m_bHornPress = FALSE;
	m_lastDoorCounter = 0;
	m_FFBamplitude = 0.0f;
	m_FFBspeed = 0.0f;
	m_FFBfrequency = 1.0f;
	m_FFBprogress = 0.0f;
	m_FFBforce = 0.0f;
	m_FFBbrake = 0.0f;

	ZeroMemory(&m_linMotion, sizeof(Motion));
	ZeroMemory(&m_cirMotion, sizeof(Motion));
	m_graviSpeed  = 0.0f;
	m_graviGlu    = 0.0f;
	m_centriSpeed = 0.0f;
	m_wheelSlide  = 0.0f;
	m_overTurn    = 0.0f;
	m_overTurnCur = 0.0f;
	m_overBrake   = 0.0f;
	m_chocSpin    = 0.0f;
	m_chocAngle   = 0.0f;
	m_motorAngle  = 0.0f;
	m_motorRPM    = 0.0f;
	m_motorState  = 0;

	m_suspTime[0]    = 0.0f;
	m_suspTime[1]    = 0.0f;
	m_suspHeight[0]  = 0.0f;
	m_suspHeight[1]  = 0.0f;
	m_suspDelayed[0] = 0.0f;
	m_suspDelayed[1] = 0.0f;

	for ( i=0 ; i<5 ; i++ )
	{
		m_suspEnergy[0][i]  = 0.0f;
		m_suspEnergy[1][i]  = 0.0f;
	}
}

// Destructeur de l'objet.

CPhysics::~CPhysics()
{
	m_iMan->DeleteInstance(CLASS_PHYSICS, this);
}


// Détruit l'objet.

void CPhysics::DeleteObject(BOOL bAll)
{
	if ( m_soundChannelMotor1 != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelMotor1);
		m_sound->AddEnvelope(m_soundChannelMotor1, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelMotor1 = -1;
	}
	if ( m_soundChannelMotor2 != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelMotor2);
		m_sound->AddEnvelope(m_soundChannelMotor2, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelMotor2 = -1;
	}
	if ( m_soundChannelSlide != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelSlide);
		m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelSlide = -1;
	}
	if ( m_soundChannelBrake != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelBrake);
		m_sound->AddEnvelope(m_soundChannelBrake, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelBrake = -1;
	}
	if ( m_soundChannelBoost != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelBoost);
		m_sound->AddEnvelope(m_soundChannelBoost, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelBoost = -1;
	}
	if ( m_soundChannelHorn != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelHorn);
		m_sound->AddEnvelope(m_soundChannelHorn, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelHorn = -1;
	}
	if ( m_soundChannelGlide != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannelGlide);
		m_sound->AddEnvelope(m_soundChannelGlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
		m_soundChannelGlide = -1;
	}
}



void CPhysics::SetBrain(CBrain* brain)
{
	m_brain = brain;
}

void CPhysics::SetMotion(CMotion* motion)
{
	m_motion = motion;
}

// Gestion du type.

void CPhysics::SetType(PhysicsType type)
{
	m_type = type;
}

PhysicsType CPhysics::RetType()
{
	return m_type;
}



// Gestion de la force de gravité.

void CPhysics::SetGravity(float value)
{
	m_gravity = value;
}

float CPhysics::RetGravity()
{
	return m_gravity;
}


// Retourne la hauteur au-dessus du sol.

float CPhysics::RetFloorHeight()
{
	return m_floorHeight;
}


// Gestion de l'état du moteur.

void CPhysics::SetMotor(BOOL bState)
{
	m_bMotor = bState;
}

BOOL CPhysics::RetMotor()
{
	return m_bMotor;
}

void CPhysics::SetSilent(BOOL bState)
{
	m_bSilent = bState;
}

BOOL CPhysics::RetSilent()
{
	return m_bSilent;
}

void CPhysics::SetForceSlow(BOOL bState)
{
	m_bForceSlow = bState;
	m_timeLock = 0.0f;
}

BOOL CPhysics::RetForceSlow()
{
	return m_bForceSlow;
}


// Gestion de l'état en vol/au sol.

void CPhysics::SetLand(BOOL bState)
{
	m_bLand = bState;
	SetMotor(!bState);  // allume le réacteur si on part en vol
}

BOOL CPhysics::RetLand()
{
	return m_bLand;
}


// Gestion de l'état dans l'air/l'eau.

void CPhysics::SetSwim(BOOL bState)
{
	if ( !m_bSwim && bState )  // entre dans l'eau ?
	{
		m_timeUnderWater = 0.0f;
	}
	m_bSwim = bState;
}

BOOL CPhysics::RetSwim()
{
	return m_bSwim;
}


// Indique s une collision a eu lieu.

void CPhysics::SetCollision(BOOL bCollision)
{
	m_bCollision = bCollision;
}

BOOL CPhysics::RetCollision()
{
	return m_bCollision;
}


// Indique si l'influence du sol est activée ou non.

void CPhysics::SetFreeze(BOOL bFreeze)
{
	m_bFreeze = bFreeze;
}

BOOL CPhysics::RetFreeze()
{
	return m_bFreeze;
}


// Indique si les phares sont allumés.

BOOL CPhysics::RetLight(int rank)
{
	if ( m_object->RetDead() )  return FALSE;
	if ( !m_motion->RetLight(rank) )  return FALSE;

	if ( rank == 0 || rank == 1 )  // phares avant ?
	{
		return TRUE;
	}
	if ( rank == 2 || rank == 3 )  // feu de stop ?
	{
		return m_bBrake;
	}
	if ( rank == 4 || rank == 5 )  // feu de recule ?
	{
		return (m_motorSpeed.x < 0.0f && !m_bBrake);
	}

	return FALSE;
}


// Spécifie la vitesse du moteur.
// x = avancer/reculer
// y = monter/descendre
// z = tourner

void CPhysics::SetMotorSpeed(D3DVECTOR speed)
{
	m_motorSpeed = speed;
}

// Spécifie la vitesse du moteur pour avancer/reculer.
// +1 = avancer
// -1 = reculer

void CPhysics::SetMotorSpeedX(float speed)
{
	m_motorSpeed.x = speed;
}

// Spécifie la vitesse du moteur pour monter/descendre.
// +1 = monter
// -1 = descendre

void CPhysics::SetMotorSpeedY(float speed)
{
	m_motorSpeed.y = speed;
}

// Spécifie la vitesse du moteur pour tourner.
// +1 = tourner à droite (CW)
// -1 = tourner à gauche (CCW)

void CPhysics::SetMotorSpeedZ(float speed)
{
	m_motorSpeed.z = speed;
}

// Spécifie la force du frain à main.
//  0 = relâché
// +1 = tiré au maximun

void CPhysics::SetMotorSpeedW(float speed)
{
	m_motorSpeedW = speed;
}

D3DVECTOR CPhysics::RetMotorSpeed()
{
	return m_motorSpeed;
}

float CPhysics::RetMotorSpeedX()
{
	return m_motorSpeed.x;
}

float CPhysics::RetMotorSpeedY()
{
	return m_motorSpeed.y;
}

float CPhysics::RetMotorSpeedZ()
{
	return m_motorSpeed.z;
}

float CPhysics::RetMotorSpeedW()
{
	return m_motorSpeedW;
}

void CPhysics::ForceMotorSpeedX(float speed)
{
	m_forceSpeed.x = speed;
}

void CPhysics::ForceMotorSpeedY(float speed)
{
	m_forceSpeed.y = speed;
}

void CPhysics::ForceMotorSpeedZ(float speed)
{
	m_forceSpeed.z = speed;
}


// Retourne la valeur du compte tour (0..2).

float CPhysics::RetMotorRPM()
{
	return m_motorRPM;
}

// Retourne le rapport de la boîte de vitesses.

int CPhysics::RetMotorState()
{
	return m_motorState;
}



// Gestion des vitesses linéaires et angulaires.
// Spécifie la vitesse parallèle au sens de marche.

void CPhysics::SetLinMotion(PhysicsMode mode, D3DVECTOR value)
{
	if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel  = value;
	if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel   = value;
	if ( mode == MO_STOACCEL )  m_linMotion.stopAccel     = value;
	if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed  = value;
	if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide  = value;
	if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel    = value;
	if ( mode == MO_TERFORCE )  m_linMotion.terrainForce  = value;
	if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed  = value;
	if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed   = value;
	if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed    = value;
	if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed  = value;
	if ( mode == MO_REASPEED )  m_linMotion.realSpeed     = value;
	if ( mode == MO_REAACCEL )  m_linMotion.realAccel     = value;
}

D3DVECTOR CPhysics::RetLinMotion(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel;
	if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel;
	if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel;
	if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed;
	if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide;
	if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel;
	if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce;
	if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed;
	if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed;
	if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed;
	if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed;
	if ( mode == MO_REASPEED )  return m_linMotion.realSpeed;
	if ( mode == MO_REAACCEL )  return m_linMotion.realAccel;
	return D3DVECTOR(0.0f, 0.0f, 0.0f);
}

void CPhysics::SetLinMotionX(PhysicsMode mode, float value)
{
	if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel.x  = value;
	if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel.x   = value;
	if ( mode == MO_STOACCEL )  m_linMotion.stopAccel.x     = value;
	if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed.x  = value;
	if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide.x  = value;
	if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel.x    = value;
	if ( mode == MO_TERFORCE )  m_linMotion.terrainForce.x  = value;
	if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed.x  = value;
	if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed.x   = value;
	if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed.x    = value;
	if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed.x  = value;
	if ( mode == MO_REASPEED )  m_linMotion.realSpeed.x     = value;
	if ( mode == MO_REAACCEL )  m_linMotion.realAccel.x     = value;
}

float CPhysics::RetLinMotionX(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel.x;
	if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel.x;
	if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel.x;
	if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed.x;
	if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide.x;
	if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel.x;
	if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce.x;
	if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed.x;
	if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed.x;
	if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed.x;
	if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed.x;
	if ( mode == MO_REASPEED )  return m_linMotion.realSpeed.x;
	if ( mode == MO_REAACCEL )  return m_linMotion.realAccel.x;
	return 0.0f;
}

// Spécifie la vitesse d'élévation.

void CPhysics::SetLinMotionY(PhysicsMode mode, float value)
{
	if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel.y  = value;
	if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel.y   = value;
	if ( mode == MO_STOACCEL )  m_linMotion.stopAccel.y     = value;
	if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed.y  = value;
	if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide.y  = value;
	if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel.y    = value;
	if ( mode == MO_TERFORCE )  m_linMotion.terrainForce.y  = value;
	if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed.y  = value;
	if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed.y   = value;
	if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed.y    = value;
	if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed.y  = value;
	if ( mode == MO_REASPEED )  m_linMotion.realSpeed.y     = value;
	if ( mode == MO_REAACCEL )  m_linMotion.realAccel.y     = value;
}

float CPhysics::RetLinMotionY(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel.y;
	if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel.y;
	if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel.y;
	if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed.y;
	if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide.y;
	if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel.y;
	if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce.y;
	if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed.y;
	if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed.y;
	if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed.y;
	if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed.y;
	if ( mode == MO_REASPEED )  return m_linMotion.realSpeed.y;
	if ( mode == MO_REAACCEL )  return m_linMotion.realAccel.y;
	return 0.0f;
}

// Spécifie la vitesse perpendiculaire au sens de marche.

void CPhysics::SetLinMotionZ(PhysicsMode mode, float value)
{
	if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel.z  = value;
	if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel.z   = value;
	if ( mode == MO_STOACCEL )  m_linMotion.stopAccel.z     = value;
	if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed.z  = value;
	if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide.z  = value;
	if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel.z    = value;
	if ( mode == MO_TERFORCE )  m_linMotion.terrainForce.z  = value;
	if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed.z  = value;
	if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed.z   = value;
	if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed.z    = value;
	if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed.z  = value;
	if ( mode == MO_REASPEED )  m_linMotion.realSpeed.z     = value;
	if ( mode == MO_REAACCEL )  m_linMotion.realAccel.z     = value;
}

float CPhysics::RetLinMotionZ(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel.z;
	if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel.z;
	if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel.z;
	if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed.z;
	if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide.z;
	if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel.z;
	if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce.z;
	if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed.z;
	if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed.z;
	if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed.z;
	if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed.z;
	if ( mode == MO_REASPEED )  return m_linMotion.realSpeed.z;
	if ( mode == MO_REAACCEL )  return m_linMotion.realAccel.z;
	return 0.0f;
}

// Spécifie la rotation autour de l'axe de marche.

void CPhysics::SetCirMotion(PhysicsMode mode, D3DVECTOR value)
{
	if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel  = value;
	if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel   = value;
	if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel     = value;
	if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed  = value;
	if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide  = value;
	if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel    = value;
	if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce  = value;
	if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed  = value;
	if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed   = value;
	if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed    = value;
	if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed  = value;
	if ( mode == MO_REASPEED )  m_cirMotion.realSpeed     = value;
	if ( mode == MO_REAACCEL )  m_cirMotion.realAccel     = value;
}

D3DVECTOR CPhysics::RetCirMotion(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel;
	if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel;
	if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel;
	if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed;
	if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide;
	if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel;
	if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce;
	if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed;
	if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed;
	if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed;
	if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed;
	if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed;
	if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel;
	return D3DVECTOR(0.0f, 0.0f, 0.0f);
}

void CPhysics::SetCirMotionX(PhysicsMode mode, float value)
{
	if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel.x  = value;
	if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel.x   = value;
	if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel.x     = value;
	if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed.x  = value;
	if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide.x  = value;
	if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel.x    = value;
	if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce.x  = value;
	if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed.x  = value;
	if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed.x   = value;
	if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed.x    = value;
	if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed.x  = value;
	if ( mode == MO_REASPEED )  m_cirMotion.realSpeed.x     = value;
	if ( mode == MO_REAACCEL )  m_cirMotion.realAccel.x     = value;
}

float CPhysics::RetCirMotionX(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel.x;
	if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel.x;
	if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel.x;
	if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed.x;
	if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide.x;
	if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel.x;
	if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce.x;
	if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed.x;
	if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed.x;
	if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed.x;
	if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed.x;
	if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed.x;
	if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel.x;
	return 0.0f;
}

// Spécifie la rotation de direction.

void CPhysics::SetCirMotionY(PhysicsMode mode, float value)
{
	if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel.y  = value;
	if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel.y   = value;
	if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel.y     = value;
	if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed.y  = value;
	if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide.y  = value;
	if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel.y    = value;
	if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce.y  = value;
	if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed.y  = value;
	if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed.y   = value;
	if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed.y    = value;
	if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed.y  = value;
	if ( mode == MO_REASPEED )  m_cirMotion.realSpeed.y     = value;
	if ( mode == MO_REAACCEL )  m_cirMotion.realAccel.y     = value;
}

float CPhysics::RetCirMotionY(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel.y;
	if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel.y;
	if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel.y;
	if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed.y;
	if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide.y;
	if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel.y;
	if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce.y;
	if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed.y;
	if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed.y;
	if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed.y;
	if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed.y;
	if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed.y;
	if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel.y;
	return 0.0f;
}

// Spécifie la rotation de montée/descente.

void CPhysics::SetCirMotionZ(PhysicsMode mode, float value)
{
	if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel.z  = value;
	if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel.z   = value;
	if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel.z     = value;
	if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed.z  = value;
	if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide.z  = value;
	if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel.z    = value;
	if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce.z  = value;
	if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed.z  = value;
	if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed.z   = value;
	if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed.z    = value;
	if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed.z  = value;
	if ( mode == MO_REASPEED )  m_cirMotion.realSpeed.z     = value;
	if ( mode == MO_REAACCEL )  m_cirMotion.realAccel.z     = value;
}

float CPhysics::RetCirMotionZ(PhysicsMode mode)
{
	if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel.z;
	if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel.z;
	if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel.z;
	if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed.z;
	if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide.z;
	if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel.z;
	if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce.z;
	if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed.z;
	if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed.z;
	if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed.z;
	if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed.z;
	if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed.z;
	if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel.z;
	return 0.0f;
}


// Retourne la distance linéaire de freinage.
//
//       v*v
//	d = -----
//	     2a

float CPhysics::RetLinStopLength(PhysicsMode sMode, PhysicsMode aMode)
{
	float		speed, accel;

	speed = RetLinMotionX(sMode);  // MO_ADVSPEED/MO_RECSPEED
	accel = RetLinMotionX(aMode);  // MO_ADVACCEL/MO_RECACCEL/MO_STOACCEL

	return (speed*speed) / (accel*2.0f);
}

// Retourne l'angle circulaire de freinage.

float CPhysics::RetCirStopLength()
{
	return m_cirMotion.advanceSpeed.y * m_cirMotion.advanceSpeed.y / 
		   m_cirMotion.stopAccel.y / 2.0f;
}

// Retourne la longueur avancée en une seconde, au sol, à vitesse maximale.

float CPhysics::RetLinMaxLength(float dir)
{
	float		dist;

	if ( dir > 0.0f )  dist = m_linMotion.advanceSpeed.x;
	else               dist = m_linMotion.recedeSpeed.x;

	return dist;
}

// Retourne le temps nécessaire pour parcourir une certaine distance.

float CPhysics::RetLinTimeLength(float dist, float dir)
{
	float		accel, decel, dps;

	if ( dir > 0.0f )
	{
		accel = RetLinStopLength(MO_ADVSPEED, MO_ADVACCEL);
		decel = RetLinStopLength(MO_ADVSPEED, MO_STOACCEL);
	}
	else
	{
		accel = RetLinStopLength(MO_RECSPEED, MO_RECACCEL);
		decel = RetLinStopLength(MO_RECSPEED, MO_STOACCEL);
	}

	dps = RetLinMaxLength(dir);

	return (dist+accel+decel)/dps;
}

// Retourne la longueur à avancer pour parcourir une certaine
// distance, en tenant compte des accélérations/décélérations.

float CPhysics::RetLinLength(float dist)
{
	float	accDist, desDist;

	if ( dist > 0.0f )
	{
		accDist = RetLinStopLength(MO_ADVSPEED, MO_ADVACCEL);
		desDist = RetLinStopLength(MO_ADVSPEED, MO_STOACCEL);

		if ( dist > accDist+desDist )
		{
			return dist-desDist;
		}

		return dist*m_linMotion.stopAccel.x /
			   (m_linMotion.advanceAccel.x+m_linMotion.stopAccel.x);
	}
	else
	{
		dist = -dist;
		accDist = RetLinStopLength(MO_RECSPEED, MO_RECACCEL);
		desDist = RetLinStopLength(MO_RECSPEED, MO_STOACCEL);

		if ( dist > accDist+desDist )
		{
			return dist-desDist;
		}

		return dist*m_linMotion.stopAccel.x /
			   (m_linMotion.recedeAccel.x+m_linMotion.stopAccel.x);
	}
}

// Retourne la vitesse de rotation des roues.

float CPhysics::RetWheelSpeed(BOOL bFront)
{
	float	speed;

	if ( m_bBrake && !(m_bHandbrake && bFront) && Abs(m_motorSpeed.x) > 0.5f )
	{
		speed = 0.0f;  // roues bloquées (pas d'ABS !)
	}
	else
	{
		speed = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x * 50.0f;

		if ( !bFront )  // roues arrières (motrices) ?
		{
			speed += m_wheelSlide*40.0f;  // ajoute le patinage à l'accélération
		}
	}

	return speed;
}


// Gestion d'un événement.
// Retourne FALSE si l'objet est détruit.

BOOL CPhysics::EventProcess(const Event &event)
{
	if ( !m_object->RetEnable() )  return TRUE;

	if ( m_brain != 0 )
	{
		m_brain->EventProcess(event);
	}

	if ( event.event == EVENT_KEYDOWN )
	{
		if ( event.param == m_engine->RetKey(KEYRANK_HORN, 0) ||
			 event.param == m_engine->RetKey(KEYRANK_HORN, 1) )
		{
			HornEvent(TRUE);
		}
	}

	if ( event.event == EVENT_KEYUP )
	{
		if ( event.param == m_engine->RetKey(KEYRANK_HORN, 0) ||
			 event.param == m_engine->RetKey(KEYRANK_HORN, 1) )
		{
			HornEvent(FALSE);
		}
	}

	if ( event.event == EVENT_BUTTON_HORN )
	{
		HornEvent(TRUE);
	}
	if ( event.event == EVENT_BUTTON_BRAKE )
	{
		m_motorSpeedWk = 1.0f;
	}
	if ( event.event == EVENT_LBUTTONUP ||
		 event.event == EVENT_RBUTTONUP )
	{
		HornEvent(FALSE);
		m_motorSpeedWk = 0.0f;
	}

	if ( event.event == EVENT_FRAME )
	{
		if ( m_soundChannelHorn != -1 )
		{
			m_sound->Position(m_soundChannelHorn, m_object->RetPosition(0));
		}
		return EventFrame(event);
	}

	return TRUE;
}

// Action du claxon.

void CPhysics::HornEvent(BOOL bPress)
{
	if ( bPress != m_bHornPress )  // changement d'état ?
	{
		m_bHornPress = bPress;

		if ( !m_bHornPress )  // bouton relâché ?
		{
			HornAction();
		}
	}

	if ( bPress )
	{
		if ( m_soundChannelHorn == -1 )
		{
			m_soundChannelHorn = m_sound->Play(SOUND_HORN, m_object->RetPosition(0), 0.0f, 0.95f, TRUE);
			m_sound->AddEnvelope(m_soundChannelHorn, 1.0f, 1.0f, 0.05f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_soundChannelHorn, 1.0f, 1.0f, 1.00f, SOPER_LOOP);
		}
	}
	else
	{
		if ( m_soundChannelHorn != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelHorn);
			m_sound->AddEnvelope(m_soundChannelHorn, 0.0f, 1.0f, 0.1f, SOPER_STOP);
			m_soundChannelHorn = -1;
		}
	}
}


// Met à jour les consignes de vitesse du moteur.

void CPhysics::MotorUpdate(float aTime, float rTime)
{
	Character*	character;
	ObjectType	type;
	D3DVECTOR	pos, motorSpeed;
	float		speed, factor, terrain, h, booster;

	if ( m_type == TYPE_MASS )
	{
		m_motorSpeed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		m_motorSpeedW = 0.0f;
		m_motorSpeedWk = 0.0f;
		m_linMotion.motorAccel.x = m_linMotion.stopAccel.x;  // avancer/reculer
		m_linMotion.motorAccel.y = m_linMotion.stopAccel.y;  // monter/descendre
		m_linMotion.motorAccel.z = m_linMotion.stopAccel.z;  // translation latérale

		if ( m_floorHeight > 0.0f )  // en l'air ?
		{
			m_linMotion.motorSpeed.y = -200.0f;  // vitesse max de chute
		}
		else	// au sol ?
		{
			m_linMotion.motorSpeed.y = 0.0f;
		}
		return;
	}

	type = m_object->RetType();
	character = m_object->RetCharacter();

	if ( m_object->RetDead() || RetLock() || m_chocSpin != 0.0f )
	{
		m_motorSpeed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		m_motorSpeedW = 0.0f;
		m_motorSpeedWk = 0.0f;
	}

	// Les consignes forcées sont utiles lorsque le véhicule est SetLock.
	if ( m_forceSpeed.x != 0.0f )
	{
		m_motorSpeed.x = m_forceSpeed.x;
		m_motorSpeedW = 0.0f;
		m_motorSpeedWk = 0.0f;
	}
	if ( m_forceSpeed.y != 0.0f )
	{
		m_motorSpeed.y = m_forceSpeed.y;
	}
	if ( m_forceSpeed.z != 0.0f )
	{
		m_motorSpeed.z = m_forceSpeed.z;
	}

	m_bHandbrake = FALSE;

	if ( m_type == TYPE_RACE )
	{
		// Avance moins vite sur les zones lentes.
		factor = m_terrain->RetSlowerZone(m_object->RetPosition(0));
		if ( m_motorSpeed.x > 0.0f )
		{
			m_motorSpeed.x *= factor;
		}
		if ( m_motorSpeed.x < 0.0f )
		{
			m_motorSpeed.x *= Norm(factor*3.0f);
		}

		// Avance moins vite sur les sols mous.
		if ( m_terrainHard < 0.5f )
		{
			m_motorSpeed.x *= 0.5f+m_terrainHard;
		}

		// Gestion du frein à main.
		if ( m_motorSpeedW > 0.0f &&  // frein à main ?
			 m_motorSpeed.x >= 0.0f )
		{
			if ( m_linMotion.realSpeed.x > 0.0f )  // on avance ?
			{
				m_motorSpeed.x = -m_motorSpeedW*0.6f;  // met du frein normal
			}
			m_bHandbrake = TRUE;

			if ( Abs(m_linMotion.realSpeed.x) < 1.0f )  // presque arrêté ?
			{
				m_motorSpeed.x = 0.0f;  // ne met pas de gaz
			}
		}
		if ( m_motorSpeedWk > 0.0f &&  // frein à main ?
			 m_motorSpeed.x >= 0.0f )
		{
			if ( m_linMotion.realSpeed.x > 0.0f )  // on avance ?
			{
				m_motorSpeed.x = -m_motorSpeedWk*0.6f;  // met du frein normal
			}
			m_bHandbrake = TRUE;

			if ( Abs(m_linMotion.realSpeed.x) < 1.0f )  // presque arrêté ?
			{
				m_motorSpeed.x = 0.0f;  // ne met pas de gaz
			}
		}
	}

	motorSpeed = m_motorSpeed;

	booster = 1.0f;
	if ( m_object->RetStarting() )  // bloqué pendant 3,2,1,go ?
	{
		if ( m_main->IsStarter() &&   // starter existe ?
			 m_main->RetMotorBlast() )
		{
			if ( m_forceSpeed.x == 0.0f )
			{
				if ( motorSpeed.x > 0.0f )
				{
					m_timeMotorBurn += motorSpeed.x*rTime;
				}
				else
				{
					m_timeMotorBurn -= rTime;
					if ( m_timeMotorBurn < 0.0f )  m_timeMotorBurn = 0.0f;
				}
			}
		}
		motorSpeed.x = 0.0f;
	}
	else
	{
		if ( m_timeMotorBurn > 1.0f )
		{
			booster = m_timeMotorBurn*2.0f;  // super-méga accélération
			m_timeMotorBurn -= rTime;
		}
	}

	if ( m_object->RetDead() )  // homme mort ?
	{
		motorSpeed.x = 0.0f;
		motorSpeed.z = 0.0f;
		if ( m_motion->RetAction() == MHS_DEADw )  // mort noyé ?
		{
			motorSpeed.y = 0.0f;  // c'est MHS_DEADw qui remonte
		}
		else
		{
			motorSpeed.y = -1.0f;  // tombe
		}
		SetMotor(FALSE);
	}

	if ( m_type == TYPE_RACE )
	{
		// Avance moins vite si dérape.
		motorSpeed.x *= 1.0f-Norm(Abs(m_centriSpeed/character->gripSlide))*0.3f;

		// Tourne proportionnellement à la vitesse.
		speed = Norm(Abs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x));
		if ( motorSpeed.x >= 0.0f )  // avance ?
		{
			// Tourne moins sur un sol lisse.
			motorSpeed.z *= powf(speed, 0.5f)*(0.25f+0.5f*m_terrainHard);
			motorSpeed.z *= 1.0f-Norm(Abs(m_centriSpeed/character->gripSlide)*character->turnSlide);
		}
		else	// recule ?
		{
			motorSpeed.z *= speed*2.0f;  // tourne + en marche arrière
		}

		// On freine si on avance avec la marche arrière.
		m_bBrake = ( motorSpeed.x < 0.0f && m_linMotion.realSpeed.x > 0.0f );
		m_bBrake |= m_bHandbrake;

		if ( m_bBrake && !m_bHandbrake )
		{
			// Tourne moins bien lors d'un freinage.
			motorSpeed.z *= 1.0f-(1.0f-character->brakeDir)*Abs(motorSpeed.x);
		}
		if ( m_bHandbrake )  // frein à main ?
		{
			speed = Norm(Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
			motorSpeed.z *= 1.0f+powf(speed, 5.0f);  // tourne plus
		}

		if ( motorSpeed.x > 0.0f )  // en avant ?
		{
			speed = Norm(Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
//?			speed = Norm(speed*1.2f);
//?			m_wheelSlide = m_motorSpeed.x*(1.0f-speed);
			if ( m_motorSpeed.x <= 0.5f )
			{
				m_wheelSlide = 0.0f;
			}
			else
			{
				speed = Norm(speed*1.2f);
				m_wheelSlide = (m_motorSpeed.x-0.5f)/0.5f*(1.0f-speed);
			}
		}
		else
		{
			m_wheelSlide = 0.0f;
		}

		if ( m_wheelSlide > 0.0f )  // en avant ?
		{
			// Survire si gaz à fond et vitesse faible.
			speed = Norm(Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
			speed = Norm(speed*1.5f);
			motorSpeed.z *= 1.0f+m_wheelSlide*(1.0f-speed)*character->overProp;
		}

		if ( m_floorHeight > 0.0f )
		{
			h = m_floorHeight/character->suspDetect;
			if ( h > 1.0f )  h = 1.0f;
			motorSpeed.x *= 1.0f-h;  // on ne peut plus accélérer en l'air
			motorSpeed.z *= 1.0f-h;  // on ne peut plus tourner en l'air
		}

		// Ajoute une imprécision mécanique.
		if ( m_imprecisionT.x <= m_time )
		{
			m_imprecisionT.x = m_time+0.1f+Rand()*0.2f;
			m_imprecisionH.x = Rand()-0.5f;
		}
		m_imprecisionA.x = Smooth(m_imprecisionA.x, m_imprecisionH.x, rTime*20.0f);

		if ( m_imprecisionT.z <= m_time )
		{
			m_imprecisionT.z = m_time+0.1f+Rand()*0.2f;
			m_imprecisionH.z = Rand()-0.5f;
		}
		m_imprecisionA.z = Smooth(m_imprecisionA.z, m_imprecisionH.z, rTime*20.0f);

		factor = 2.0f-(m_terrainHard-0.2f)/0.6f;
		if ( motorSpeed.x != 0.0f )  motorSpeed.x += m_imprecisionA.x*factor*0.2f;
		motorSpeed.z += Abs(motorSpeed.x)*m_imprecisionA.z*factor*0.02f;
		motorSpeed.z *= 1.0f+m_imprecisionA.z*factor*0.5f;
	}

	// Avancer/reculer.
	if ( m_type == TYPE_RACE )
	{
		// Accélère moins plus on va vite.
		factor = Norm(Abs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x));
		factor = 1.0f-powf(1.0f-factor, character->accelSmooth);
		factor = character->accelLow - (character->accelLow-character->accelHigh)*factor;

		terrain = (0.6f+m_terrainHard*0.5f);
	}
	else
	{
		factor  = 1.0f;
		terrain = 1.0f;
	}

	if ( motorSpeed.x > 0.0f )
	{
		m_linMotion.motorAccel.x = m_linMotion.advanceAccel.x * terrain * factor * booster;
		m_linMotion.motorSpeed.x = m_linMotion.advanceSpeed.x * motorSpeed.x;
	}
	if ( motorSpeed.x < 0.0f )
	{
		m_linMotion.motorAccel.x = m_linMotion.recedeAccel.x * terrain;
		m_linMotion.motorSpeed.x = m_linMotion.recedeSpeed.x * motorSpeed.x;
	}
	if ( motorSpeed.x == 0.0f )
	{
		if ( m_type == TYPE_RACE && m_floorHeight > 0.0f )
		{
			if ( m_main->RetLevel() == 1 )  // niveau CN ?
			{
				m_linMotion.motorAccel.x = 5.0f;  // déccélération en l'air
				m_linMotion.motorSpeed.x = 10.0f * motorSpeed.x;
			}
			else
			{
				m_linMotion.motorAccel.x = 10.0f;  // déccélération en l'air
				m_linMotion.motorSpeed.x = 10.0f * motorSpeed.x;
			}
			m_centriSpeed = SmoothP(m_centriSpeed, 0.0f, rTime);
		}
		else
		{
			m_linMotion.motorAccel.x = m_linMotion.stopAccel.x;
			m_linMotion.motorSpeed.x = 0.0f;
		}
	}

	// Monter/descendre.
	if ( motorSpeed.y > 0.0f )
	{
		m_linMotion.motorAccel.y = m_linMotion.advanceAccel.y;
		m_linMotion.motorSpeed.y = m_linMotion.advanceSpeed.y * motorSpeed.y;
	}
	if ( motorSpeed.y < 0.0f )
	{
		m_linMotion.motorAccel.y = m_linMotion.recedeAccel.y;
		m_linMotion.motorSpeed.y = m_linMotion.recedeSpeed.y * motorSpeed.y;
	}
	if ( motorSpeed.y == 0.0f )
	{
		m_linMotion.motorAccel.y = m_linMotion.stopAccel.y;
		m_linMotion.motorSpeed.y = 0.0f;
	}

	// Tourner à gauche/droite.
	if ( m_type == TYPE_RACE )
	{
		speed = motorSpeed.z;

		// Inverse la direction si recule !
		if ( m_linMotion.realSpeed.x < 0.0f && motorSpeed.x <= 0.0f )
		{
			speed = -speed;
		}
	}
	else
	{
		speed = motorSpeed.z;
	}

	if ( motorSpeed.z > 0.0f )
	{
		m_cirMotion.motorAccel.y = m_cirMotion.advanceAccel.y;
		m_cirMotion.motorSpeed.y = m_cirMotion.advanceSpeed.y * speed;
	}
	if ( motorSpeed.z < 0.0f )
	{
		m_cirMotion.motorAccel.y = m_cirMotion.recedeAccel.y;
		m_cirMotion.motorSpeed.y = m_cirMotion.recedeSpeed.y * speed;
	}
	if ( motorSpeed.z == 0.0f )
	{
		m_cirMotion.motorAccel.y = m_cirMotion.stopAccel.y;
		m_cirMotion.motorSpeed.y = 0.0f;
	}

	if ( m_type == TYPE_RACE )
	{
		if ( m_motorSpeed.x == 0.0f )
		{
			SetMotor(FALSE);
		}
		else
		{
			SetMotor(TRUE);
		}

		if ( m_object->RetLock() || !m_object->RetSelect() )
		{
			SetSilent(!m_bForceSlow);
		}
		else
		{
			SetSilent(FALSE);
		}
	}

	if ( m_type == TYPE_TANK )
	{
		if ( motorSpeed.x == 0.0f &&
			 motorSpeed.z == 0.0f )
		{
			SetMotor(FALSE);
		}
		else
		{
			SetMotor(TRUE);
		}
	}
}


// Action éventuelle lorsque le claxon est actionné.

void CPhysics::HornAction()
{
	D3DVECTOR	zoom;

	if ( !m_object->RetSelect() )  return;
	if ( RetLock() )  return;
	if ( m_motion->RetAction() != -1 )  return;

	zoom = m_object->RetZoom(0);
	if ( zoom.x != 1.0f ||
		 zoom.y != 1.0f ||
		 zoom.z != 1.0f )  return;

	if ( m_main->RetHornAction() == 1 )
	{
		m_motion->SetAction(MV_LOADBOT, 2.5f);
	}
}


// Met à jour les effets de vibration et d'inclinaison.

void CPhysics::EffectUpdate(float aTime, float rTime)
{
	Character*	character;
	D3DVECTOR	vibLin, vibCir, incl, zoom;
	float		speedLin, speedCir, accel, factor, over;
	ObjectType	type;
	BOOL		bOnBoard;

	if ( m_type == TYPE_MASS )  return;
	if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return;

	type = m_object->RetType();
	character = m_object->RetCharacter();

	bOnBoard = FALSE;
	if ( m_object->RetSelect() &&
		 m_camera->RetType() == CAMERA_ONBOARD )
	{
		bOnBoard = TRUE;
	}

	vibLin = m_motion->RetLinVibration();
	vibCir = m_motion->RetCirVibration();
	incl   = m_motion->RetInclinaison();

	if ( type == OBJECT_HUMAN ||  // homme ?
		 type == OBJECT_TECH  )
	{
		if ( !m_bLand && !m_bSwim )  // en vol ?
		{
			vibLin.y = sinf(aTime*2.00f)*0.5f+
					   sinf(aTime*2.11f)*0.3f;

			vibCir.z = sinf(aTime*PI* 2.01f)*(PI/150.0f)+
					   sinf(aTime*PI* 2.51f)*(PI/200.0f)+
					   sinf(aTime*PI*19.01f)*(PI/400.0f);

			vibCir.x = sinf(aTime*PI* 2.03f)*(PI/150.0f)+
					   sinf(aTime*PI* 2.52f)*(PI/200.0f)+
					   sinf(aTime*PI*19.53f)*(PI/400.0f);

			speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
			speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
			incl.x = -speedLin*speedCir*0.5f;  // penche si virage

			speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.2f;
			speedLin /= m_linMotion.advanceSpeed.x;
			m_linMotion.finalInclin.z = speedLin*1.4f;
			if ( incl.z < m_linMotion.finalInclin.z )
			{
				incl.z += rTime*0.4f;
				if ( incl.z > m_linMotion.finalInclin.z )
				{
					incl.z = m_linMotion.finalInclin.z;
				}
			}
			else if ( incl.z > m_linMotion.finalInclin.z )
			{
				incl.z -= rTime*0.4f;
				if ( incl.z < m_linMotion.finalInclin.z )
				{
					incl.z = m_linMotion.finalInclin.z;
				}
			}

			vibLin *= m_linVibrationFactor;
			vibCir *= m_cirVibrationFactor;
			incl *= m_inclinaisonFactor;

			m_motion->SetLinVibration(vibLin);
			m_motion->SetCirVibration(vibCir);
			m_motion->SetInclinaison(incl);
		}
		else if ( m_bSwim )  // nage ?
		{
			vibLin.y = sinf(aTime*2.00f)*0.5f+
					   sinf(aTime*2.11f)*0.3f;

			vibCir.z = sinf(aTime*PI* 2.01f)*(PI/150.0f)+
					   sinf(aTime*PI* 2.51f)*(PI/200.0f)+
					   sinf(aTime*PI*19.01f)*(PI/400.0f);

			vibCir.x = sinf(aTime*PI* 2.03f)*(PI/150.0f)+
					   sinf(aTime*PI* 2.52f)*(PI/200.0f)+
					   sinf(aTime*PI*19.53f)*(PI/400.0f);

			speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
			speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
			incl.x = -speedLin*speedCir*5.0f;  // penche si virage

			speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.2f;
			speedLin /= m_linMotion.advanceSpeed.x;
			m_linMotion.finalInclin.z = speedLin*1.4f;
			if ( incl.z < m_linMotion.finalInclin.z )
			{
				incl.z += rTime*0.4f;
				if ( incl.z > m_linMotion.finalInclin.z )
				{
					incl.z = m_linMotion.finalInclin.z;
				}
			}
			else if ( incl.z > m_linMotion.finalInclin.z )
			{
				incl.z -= rTime*0.4f;
				if ( incl.z < m_linMotion.finalInclin.z )
				{
					incl.z = m_linMotion.finalInclin.z;
				}
			}

			if ( m_linMotion.realSpeed.y > 0.0f )  // monte ?
			{
				vibCir.z += m_linMotion.realSpeed.y*0.05f;
			}
			else	// descend ?
			{
				vibCir.z += m_linMotion.realSpeed.y*0.12f;
			}
			vibCir.z -= PI*0.4f;

			vibLin *= m_linVibrationFactor;
			vibCir *= m_cirVibrationFactor;
			incl *= m_inclinaisonFactor;

			m_motion->SetLinVibration(vibLin);
			m_motion->SetCirVibration(vibCir);
			m_motion->SetInclinaison(incl);
		}
		else
		{
			m_motion->SetLinVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
//?			m_motion->SetCirVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
//?			m_motion->SetInclinaison(D3DVECTOR(0.0f, 0.0f, 0.0f));
		}
	}

	if ( type == OBJECT_CAR )  // roues ?
	{
		zoom = m_object->RetZoom(0);

		speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
		speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
		incl.x = speedLin*speedCir*character->rolling;  // penche si virage
		if ( m_object->RetFret() != 0 )  incl.x *= 1.4f;
		if ( zoom.y < 1.0f )  incl.x = 0.0f;  // tout plat ?
		if ( zoom.z < 1.0f )  incl.x *= 3.0f;  // tout mince ?
		if ( incl.x >  0.9f )  incl.x =  0.9f;
		if ( incl.x < -0.9f )  incl.x = -0.9f;

		speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x;
		speedLin /= m_linMotion.advanceSpeed.x;
		if ( speedLin > 1.0f )  speedLin = 1.0f;
		factor = 1.0f;
		if ( m_object->RetFret() != 0 )  factor = 1.4f;
		m_linMotion.finalInclin.z = -speedLin*character->nicking*factor;
		accel = (0.40f-Abs(incl.z))*4.0f;
		if ( incl.z < m_linMotion.finalInclin.z )
		{
			incl.z += rTime*accel;
			if ( incl.z > m_linMotion.finalInclin.z )
			{
				incl.z = m_linMotion.finalInclin.z;
			}
		}
		else if ( incl.z > m_linMotion.finalInclin.z )
		{
			incl.z -= rTime*accel;
			if ( incl.z < m_linMotion.finalInclin.z )
			{
				incl.z = m_linMotion.finalInclin.z;
			}
		}

		// Tient compte de l'angle de survirage.
//?		incl.y = m_overTurn;
		if ( Abs(m_overTurn) > Abs(m_overTurnCur) )
		{
			m_overTurnCur = SmoothP(m_overTurnCur, m_overTurn, rTime*1.0f);
		}
		else
		{
			m_overTurnCur = m_overTurn;
		}
		incl.y = m_overTurnCur;
		if ( m_motion->RetWheelType() != WT_BURN   &&
			 m_camera->RetType() != CAMERA_ONBOARD )
		{
			incl.y *= character->overMul;
		}

		vibCir.x = 0.0f;
		vibCir.y = 0.0f;
		vibCir.z = 0.0f;

		vibLin.x = 0.0f;
		vibLin.z = 0.0f;
		vibLin.y = Abs(character->wheelFrontPos.x*sinf(incl.z))*0.8f +
				   Abs(character->wheelFrontPos.z*sinf(incl.x))*0.5f;
		if ( m_object->RetDead() )
		{
			vibLin.y -= character->wheelFrontDim*0.5f;
		}
		else
		{
			vibLin.y += (m_suspHeight[0]+m_suspHeight[1])/2.0f;
			vibCir.z += atanf((m_suspHeight[0]-vibLin.y)/character->wheelFrontPos.x);
		}

		// Fait vibrer la voiture à l'arrêt.
		factor = 1.0f-Norm(Abs(m_linMotion.realSpeed.x)/20.0f);
		if ( m_object->RetDead() || m_bSilent )  factor = 0.0f;
		if ( bOnBoard )  factor *= 0.2f;
		vibCir.x += sinf(aTime*30.0f)*0.02f*factor;
		if ( zoom.y < 1.0f )  vibCir.x = 0.0f;  // tout plat ?

		if ( incl.x+vibCir.x > character->maxRolling )
		{
			over = incl.x+vibCir.x - character->maxRolling;
			incl.x   -= over/2.0f;
			vibCir.x -= over/2.0f;
		}
		if ( incl.x+vibCir.x < -character->maxRolling )
		{
			over = incl.x+vibCir.x + character->maxRolling;
			incl.x   -= over/2.0f;
			vibCir.x -= over/2.0f;
		}
		if ( bOnBoard )  vibCir.x *= 0.5f;
		if ( bOnBoard )  incl.x   *= 0.5f;

		if ( incl.z+vibCir.z > character->maxNicking )
		{
			over = incl.z+vibCir.z - character->maxNicking;
			incl.z   -= over/2.0f;
			vibCir.z -= over/2.0f;
		}
		if ( incl.z+vibCir.z < -character->maxNicking )
		{
			over = incl.z+vibCir.z + character->maxNicking;
			incl.z   -= over/2.0f;
			vibCir.z -= over/2.0f;
		}
		if ( bOnBoard )  vibCir.z *= 0.5f;

		m_motion->SetInclinaison(incl);
		m_motion->SetLinVibration(vibLin);
		m_object->SetCirVibration(vibCir);

		// Fait vibrer le moteur.
//?		factor = 1.0f-Norm(Abs(m_linMotion.realSpeed.x)/50.0f);
//?		if ( m_object->RetDead() || m_bSilent )  factor = 0.0f;
//?		m_object->SetAngleX(7, sinf(aTime*41.0f)*0.03f*factor);
		factor = Norm(Abs(m_linMotion.realSpeed.x)/50.0f);
		m_motorAngle += rTime*37.0f*(1.0f+factor);
		if ( m_object->RetDead() || m_bSilent )  m_motorAngle = 0.0f;
		m_object->SetAngleX(7, sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f));
	}

	if ( type == OBJECT_UFO )
	{
		speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
		speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;

//?		vibLin.y = Abs(speedLin)*24.0f;
//?		if ( vibLin.y > 12.0f )  vibLin.y = 12.0f;
		vibLin.y = Abs(speedLin)*4.0f;

		vibCir.x = -speedCir*0.8f + sinf(aTime*8.0f)*0.12f + sinf(aTime*5.1f)*0.08f;
		vibCir.z = -speedLin*0.4f;

		m_motion->SetLinVibration(vibLin);
		m_motion->SetCirVibration(vibCir);
	}
}


// Met à jour une structure Motion.

void CPhysics::UpdateMotionStruct(float rTime, Motion &motion)
{
	float	speed, motor, init;

	// Gestion de la coordonnée x.
	init  = motion.realSpeed.x;
	speed = motion.currentSpeed.x;
	motor = motion.motorSpeed.x * m_inclinaisonFactor;
	if ( speed < motor )
	{
		speed += rTime*motion.motorAccel.x;  // accélère
		if ( speed > motor )
		{
			speed = motor;  // ne dépasse pas la vitesse
		}
	}
	if ( speed > motor )
	{
		speed -= rTime*motion.motorAccel.x;  // déccélère
		if ( speed < motor )
		{
			speed = motor;  // ne dépasse pas la vitesse
		}
	}
	motion.currentSpeed.x = speed;
	motion.realSpeed.x    = speed;

	if ( Abs(motion.terrainSpeed.x) > motion.terrainSlide.x )
	{
		if ( motion.terrainSpeed.x > 0 )
		{
			speed = motion.terrainSpeed.x - motion.terrainSlide.x;
		}
		else
		{
			speed = motion.terrainSpeed.x + motion.terrainSlide.x;
		}
		motion.realSpeed.x += speed;
	}
	motion.realAccel.x = (motion.realSpeed.x-init)/rTime;

	// Gestion de la coordonnée y.
	init  = motion.realSpeed.y;
	speed = motion.currentSpeed.y;
	motor = motion.motorSpeed.y;  // vitesse non limitée !
	if ( speed < motor )
	{
		speed += rTime*motion.motorAccel.y;  // accélère
		if ( speed > motor )
		{
			speed = motor;  // ne dépasse pas la vitesse
		}
	}
	if ( speed > motor )
	{
		speed -= rTime*motion.motorAccel.y;  // déccélère
		if ( speed < motor )
		{
			speed = motor;  // ne dépasse pas la vitesse
		}
	}
	motion.currentSpeed.y = speed;
	motion.realSpeed.y    = speed;

	if ( Abs(motion.terrainSpeed.y) > motion.terrainSlide.y )
	{
		if ( motion.terrainSpeed.y > 0 )
		{
			speed = motion.terrainSpeed.y - motion.terrainSlide.y;
		}
		else
		{
			speed = motion.terrainSpeed.y + motion.terrainSlide.y;
		}
		motion.realSpeed.y += speed;
	}
	motion.realAccel.y = (motion.realSpeed.y-init)/rTime;

	// Gestion de la coordonnée z.
	init  = motion.realSpeed.z;
	speed = motion.currentSpeed.z;
	motor = motion.motorSpeed.z * m_inclinaisonFactor;
	if ( speed < motor )
	{
		speed += rTime*motion.motorAccel.z;  // accélère
		if ( speed > motor )
		{
			speed = motor;  // ne dépasse pas la vitesse
		}
	}
	if ( speed > motor )
	{
		speed -= rTime*motion.motorAccel.z;  // déccélère
		if ( speed < motor )
		{
			speed = motor;  // ne dépasse pas la vitesse
		}
	}
	motion.currentSpeed.z = speed;
	motion.realSpeed.z    = speed;

	if ( Abs(motion.terrainSpeed.z) > motion.terrainSlide.z )
	{
		if ( motion.terrainSpeed.z > 0 )
		{
			speed = motion.terrainSpeed.z - motion.terrainSlide.z;
		}
		else
		{
			speed = motion.terrainSpeed.z + motion.terrainSlide.z;
		}
		motion.realSpeed.z += speed;
	}
	motion.realAccel.z = (motion.realSpeed.z-init)/rTime;
}


// Fait évoluer la physique selon le temps écoulé.
// Retourne FALSE si l'objet est détruit.
//
//	a:  accélération
//	v1: vitesse au temps t1
//	v2: vitesse au temps t2
//	dt: temps écoulé depuis t1, donc: dt=t2-t1
//	dd: différence de distance (avance)
//
//	v2 = v1 + a*dt
//	dd = v2*dt

BOOL CPhysics::EventFrame(const Event &event)
{
	Character*	character;
	ObjectType	type;
	D3DMATRIX	objRotate, matRotate, *mat;
	D3DVECTOR	iPos, iAngle, newpos, newangle, n, cirVib;
	float		h, factor, hope, current;
	int			i;

	if ( m_engine->RetPause() )
	{
		if ( m_object->RetType() == OBJECT_CAR )
		{
			m_engine->SetJoyForces(0.0f, 0.0f);
		}
		return TRUE;
	}

	m_time            += event.rTime;
	m_timeUnderWater  += event.rTime;
	m_soundTimeJostle += event.rTime;
	m_timeLock        += event.rTime;

	character = m_object->RetCharacter();
	type      = m_object->RetType();
	iPos      = m_object->RetPosition(0);
	iAngle    = m_object->RetAngle(0);

	if ( type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   )
	{
		if ( m_object->RetTruck() ||
			 m_object->RetLock()  )  return TRUE;
	}

	FFBForce(0.0f);

	m_terrainHard = m_terrain->RetHardness(iPos);
	m_bOldWater = m_bWater;
	m_bWater = ( iPos.y+1.0f < m_water->RetLevel() && !m_water->RetLava() );

	if ( type == OBJECT_CAR )
	{
		m_sound->SetWater(m_bWater);  // bruit feutré si sous l'eau
	}

	MotorUpdate (m_time, event.rTime);
	EffectUpdate(m_time, event.rTime);
	WaterFrame  (m_time, event.rTime);

	if ( m_timeMotorBurn >= 1.9f )  // moteur explose ?
	{
		CPyro*		pyro;
		m_main->FlushStarterType();  // recommence avec un starter lent
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_EXPLOT, m_object);  // destruction totale
		return FALSE;
	}

	// Accélère à la descente, freine à la montée.
	if ( m_bFreeze || m_object->RetDead() )
	{
		m_linMotion.terrainSpeed.x = 0.0f;
		m_linMotion.terrainSpeed.z = 0.0f;
		m_linMotion.terrainSpeed.y = 0.0f;
	}
	else
	{
		h = m_terrain->RetBuildingFactor(iPos);
		if ( iPos.y < m_water->RetLevel(m_object) )  // sous l'eau ?
		{
			h *= 0.5f;
		}
		if ( m_floorHeight == 0.0f )  // au sol ?
		{
			m_linMotion.terrainSpeed.x = -tanf(iAngle.z)*0.9f*m_linMotion.terrainForce.x*h;
			m_linMotion.terrainSpeed.z =  tanf(iAngle.x)*0.9f*m_linMotion.terrainForce.z*h;
			m_linMotion.terrainSpeed.y = 0.0f;

			// Si le terrain est très pentu, n'exagère pas !
			if ( m_linMotion.terrainSpeed.x >  50.0f )  m_linMotion.terrainSpeed.x =  50.0f;
			if ( m_linMotion.terrainSpeed.x < -50.0f )  m_linMotion.terrainSpeed.x = -50.0f;
			if ( m_linMotion.terrainSpeed.z >  50.0f )  m_linMotion.terrainSpeed.z =  50.0f;
			if ( m_linMotion.terrainSpeed.z < -50.0f )  m_linMotion.terrainSpeed.z = -50.0f;
		}
	}

	UpdateMotionStruct(event.rTime, m_linMotion);
	UpdateMotionStruct(event.rTime, m_cirMotion);
	m_linMotion.realSpeed.z += m_centriSpeed;

	if ( m_type == TYPE_MASS )
	{
		cirVib = m_object->RetCirVibration();
		cirVib += event.rTime*m_cirMotion.realSpeed;
		m_object->SetCirVibration(cirVib);
		newangle = iAngle;
		newpos = iPos + event.rTime*m_linMotion.realSpeed;
	}
	else
	{
		newangle = iAngle + event.rTime*m_cirMotion.realSpeed;
		MatRotateZXY(matRotate, newangle);
		newpos = event.rTime*m_linMotion.realSpeed;
		newpos = Transform(matRotate, newpos);
		newpos += iPos;
	}

	if ( m_type == TYPE_RACE &&
		 !m_bBrake &&
		 m_linMotion.realSpeed.x < 0.0f &&  // marche arrière ?
		 m_chocSpin == 0.0f )  // pas de rotation due à un choc ?
	{
		// Déplace l'axe de rotation sur l'essieu arrière (ouaaah !).
		mat = m_object->RetRotateMatrix(0);
		newpos += Transform(*mat, D3DVECTOR(character->wheelBackPos.x, 0.0f, 0.0f));
		newpos -= Transform(matRotate, D3DVECTOR(character->wheelBackPos.x, 0.0f, 0.0f));
	}

	// Transfère l'énergie de survirage en orientation réelle
	// et définitive.
	if ( m_type == TYPE_RACE && m_floorHeight < character->suspDetect )
	{
		factor = 1.0f-(m_floorHeight/character->suspDetect);
		factor *= m_overTurn*event.rTime*1.0f;
//?		if ( m_bBrake )  factor *= 0.0f;
//?		if ( m_bHandbrake )  factor *= 0.0f;

		if ( m_overTurn > 0.0f )
		{
			m_overTurn -= factor;
			if ( m_overTurn < 0.0f )  m_overTurn = 0.0f;
		}
		if ( m_overTurn < 0.0f )
		{
			m_overTurn -= factor;
			if ( m_overTurn > 0.0f )  m_overTurn = 0.0f;
		}

		newangle.y += factor;
	}

	m_terrain->LimitPos(newpos);
	FloorAdapt(m_time, event.rTime, newpos, newangle);
	m_glideAmplitude = 0.0f;

	if ( m_bForceUpdate         ||
		 newpos.x   != iPos.x   ||
		 newpos.y   != iPos.y   ||
		 newpos.z   != iPos.z   ||
		 newangle.x != iAngle.x ||
		 newangle.y != iAngle.y ||
		 newangle.z != iAngle.z )
	{
		i = ObjectAdapt(newpos, newangle, m_time, event.rTime);
		if ( i == 2 )  // objet détruit ?
		{
			return FALSE;
		}
		if ( i == 1 )  // objet immobile ?
		{
			newpos = iPos;  // garde la position initiale, mais accepte la rotation
		}
	}

	UpdateGlideSound(event.rTime);

	if ( newangle.x != iAngle.x ||
		 newangle.y != iAngle.y ||
		 newangle.z != iAngle.z )
	{
		m_object->SetAngle(0, newangle);
	}

	if ( newpos.x != iPos.x ||
		 newpos.y != iPos.y ||
		 newpos.z != iPos.z )
	{
		m_object->SetPosition(0, newpos);  // m_curCorner <- coins courants
	}

	MotorParticule(m_time, event.rTime);
	SoundMotor(event.rTime);

	if ( type == OBJECT_CAR )
	{
		FFBFrame(m_time, event.rTime);
	}

	if ( m_object->RetSelect() &&
		 m_type == TYPE_RACE   &&
		 m_camera->RetObject() == m_object )
	{
		m_camera->SetMotorSpeed(m_linMotion.realSpeed.x);

		if ( !RetLock() && !m_object->RetDead() )
		{
			// Eloigne la caméra en marche arrière.
			if ( m_motorSpeed.x < 0.0f && !m_bBrake )
			{
				hope = 50.0f;
				factor = 1.0f;
			}
			else
			{
				hope = 20.0f;
				factor = 0.6f;
			}
			current = m_camera->RetBackDist();
			m_camera->SetBackDist(Smooth(current, hope, event.rTime*factor));

			if ( m_motorSpeed.x < 0.0f && !m_bBrake )
			{
				hope = -PI*0.10f;
				factor = 1.0f;
			}
			else
			{
				hope = -PI*0.05f;
				factor = 0.6f;
			}
			current = m_camera->RetBackVerti();
			m_camera->SetBackVerti(Smooth(current, hope, event.rTime*factor));
		}
	}

	m_bForceUpdate = FALSE;

	return TRUE;
}


// Indique si la voiture est utilisable. Il y a un léger retard après
// que l'objet soit Unlock, pour simuler le démarrage du moteur.

BOOL CPhysics::RetLock()
{
	if ( m_type == TYPE_RACE &&
		 !m_main->IsStarter() )   // pas de starter ?
	{
		if ( m_object->RetLock() )
		{
			m_timeLock = 0.0f;
			return TRUE;
		}
		else
		{
			return (m_timeLock < STARTDELAY);
		}
	}
	else
	{
		return m_object->RetLock();
	}
}


// Démarre ou stoppe les bruits de moteur.

void CPhysics::SoundMotor(float rTime)
{
	ObjectType	type;

	if ( m_type == TYPE_MASS )  return;

	m_lastSoundInsect -= rTime;
	type = m_object->RetType();

	if ( m_type == TYPE_RACE )
	{
		if ( m_bMotor && !m_bSilent && !m_bBrake && m_object->RetActif() )
		{
			SoundMotorFull(rTime, type);  // plein régime
		}
		else
		{
			if ( (m_object->RetSelect() || m_bForceSlow) &&
				 !m_object->RetDead() &&
				 !m_bSilent )
			{
				SoundMotorSlow(rTime, type);  // au ralenti
			}
			else
			{
				SoundMotorStop(rTime, type);  // à l'arrêt
			}
		}
	}

	if ( type == OBJECT_TRAX )
	{
		if ( m_bMotor && m_object->RetActif() )
		{
			SoundMotorFull(rTime, type);  // plein régime
		}
		else
		{
			SoundMotorSlow(rTime, type);  // au ralenti
		}
	}

	if ( type == OBJECT_UFO )
	{
		if ( m_bMotor && !m_bSwim &&
			 m_object->RetActif() && !m_object->RetDead() )
		{
			SoundReactorFull(rTime, type);  // plein régime
		}
		else
		{
			SoundReactorStop(rTime, type);  // à l'arrêt
		}
	}
}

// Fait exploser l'objet s'il est sous l'eau.

void CPhysics::WaterFrame(float aTime, float rTime)
{
	D3DMATRIX*	mat;
	Character*	character;
	ObjectType	type;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		level, duration, height, factor;
	int			i;

	if ( m_type == TYPE_MASS )  return;

	level = m_water->RetLevel();
	if ( level == 0.0f )  return;  // pas d'eau ?
	if ( m_object->RetTruck() != 0 )  return;  // objet transporté ?

	type = m_object->RetType();

	// Gestion des flammes dans la lave.
	pos = m_object->RetPosition(0);
	height = pos.y-m_water->RetLevel(m_object);  // hauteur sur l'eau

	if ( m_water->RetLava() &&
		 m_lastFlameParticule+m_engine->ParticuleAdapt(0.05f) <= aTime )
	{
		m_lastFlameParticule = aTime;

		if ( height < 0.0f )  // sous la lave ?
		{
			pos = m_object->RetPosition(0);
			pos.x += (Rand()-0.5f)*3.0f;
			pos.z += (Rand()-0.5f)*3.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = Rand()*5.0f+3.0f;
			dim.x = Rand()*2.0f+1.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f);

			pos = m_object->RetPosition(0);
			pos.y -= 2.0f;
			pos.x += (Rand()-0.5f)*5.0f;
			pos.z += (Rand()-0.5f)*5.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 6.0f+Rand()*6.0f+6.0f;
			dim.x = Rand()*1.5f+1.0f+3.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f);
		}

		if ( type == OBJECT_CAR )  // voiture ?
		{
			character = m_object->RetCharacter();
			mat = m_object->RetWorldMatrix(0);

			if ( height < 0.0f &&
				 m_motion->RetWheelType() != WT_BURN )  // roues pas cramées ?
			{
				for ( i=0 ; i<4*5 ; i++ )
				{
					if ( i%4 < 2 )  pos = character->wheelFrontPos;
					else            pos = character->wheelBackPos;
					pos.z += 1.0f;
					if ( i%2 == 0 )  pos.z = -pos.z;
					pos = Transform(*mat, pos);
					speed.x = 0.0f;
					speed.z = 0.0f;
					speed.y = Rand()*6.0f+6.0f;
					dim.x = Rand()*1.0f+1.0f;
					dim.y = dim.x;
					duration = 0.5f+Rand()*0.5f;
					m_particule->CreateParticule(pos, speed, dim, PARTIWHEEL, duration);
				}
			}

			if ( m_timeWheelBurn > 0.0f )  // jantes crament encore ?
			{
				if ( m_timeWheelBurn > 0.0f )
				{
					m_timeWheelBurn -= rTime;
					if ( m_timeWheelBurn < 0.0f )  m_timeWheelBurn = 0.0f;
				}

				for ( i=0 ; i<4*2 ; i++ )
				{
					if ( i%4 < 2 )  pos = character->wheelFrontPos;
					else            pos = character->wheelBackPos;
					pos.z += 1.0f;
					if ( i%2 == 0 )  pos.z = -pos.z;
					pos = Transform(*mat, pos);
					speed.x = 0.0f;
					speed.z = 0.0f;
					speed.y = Rand()*3.0f+3.0f;
					dim.x = Rand()*0.6f+0.6f;
					dim.y = dim.x;
					duration = 1.5f+Rand()*1.5f;
					m_particule->CreateParticule(pos, speed, dim, PARTIWHEEL, duration);
				}
			}
		}
	}
	
	if ( height >= 0.0f )  return;  // hors de l'eau ?
	if ( type == OBJECT_NULL )  return;

	if ( !m_object->RetActif() )  return;

	if ( m_water->RetLava() )
	{
		if ( type == OBJECT_CAR   &&  // voiture ?
			 !m_object->RetDead() )
		{
			level = m_object->RetBurnShield();
			factor = 5.0f+height*4.0f/5.0f;  // si profond -> meurt vite
			if ( factor < 0.5f )  factor = 0.5f;
			level -= rTime/factor;
			if ( level < 0.0f )  level = 0.0f;
			m_object->SetBurnShield(level);

			if ( level < 0.5f )
			{
				if ( m_motion->RetWheelType() != WT_BURN )
				{
					m_motion->SetWheelType(WT_BURN);  // roule sur les jantes
					m_timeWheelBurn = 5.0f;  // crame encore qq temps
				}
			}

			if ( level == 0.0f )
			{
				m_object->ExploObject(EXPLO_BOUM, 10000.0f);  // démarre explosion
			}
		}

		if ( type == OBJECT_HUMAN   &&
			 m_object->RetOption() != 0 )  // homme sans casque ?
		{
			m_object->ExploObject(EXPLO_WATER, 1.0f);  // démarre explosion
		}
	}
}

// Choc avec vibration.

void CPhysics::FFBCrash(float amplitude, float speed, float frequency)
{
	m_FFBamplitude = amplitude;
	m_FFBspeed = speed;
	m_FFBfrequency = frequency;
	m_FFBprogress = 1.0f;
}

// Force constante.

void CPhysics::FFBForce(float amplitude)
{
	m_FFBforce = amplitude;
}

// Gestion de l'effet force feedback.

void CPhysics::FFBFrame(float aTime, float rTime)
{
	float	forceX, forceY, effect, speed;

	if ( m_main->RetPhase() != PHASE_SIMUL )  return;

	forceX = 0.0f;
	forceY = 0.0f;
	speed = Abs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x);

	if ( m_FFBprogress > 0.0f )
	{
		m_FFBprogress -= rTime*(1.0f/m_FFBspeed);
		if ( m_FFBprogress < 0.0f )  m_FFBprogress = 0.0f;
//?		effect = (Rand()-0.5f)*2.0f;
		effect = sinf(0.2f+m_FFBprogress* 7.0f*m_FFBfrequency)+
				 cosf(0.1f+m_FFBprogress*10.0f*m_FFBfrequency)+
				 sinf(0.7f+m_FFBprogress*19.0f*m_FFBfrequency);
		effect *= m_FFBprogress*m_FFBamplitude;
		forceX += effect;
		forceY += effect;
	}

	forceX += m_FFBforce;

	if ( m_terrainHard < 0.5f && !m_bWater )  // sol mou ?
	{
		effect = (Rand()-0.5f)*0.7f*speed;
		forceX += effect;
		forceY += effect;
	}

	if ( m_motion->RetWheelType() == WT_BURN )  // sur les jantes ?
	{
		effect = sinf(m_time*15.0f*speed)*0.6f*speed;
		forceX += effect;
		forceY += effect;
	}

	if ( forceX == 0.0f && m_engine->RetJoystick() == 1 )
	{
		if ( m_bWater )  // sous l'eau ?
		{
			forceX = -m_motorSpeed.z*speed;  // accentue
		}
		else
		{
			if ( m_bBrake )
			{
				m_FFBbrake += rTime*3.0f;
				m_FFBbrake = Norm(m_FFBbrake);
				forceX = -m_motorSpeed.z*speed*0.5f*m_FFBbrake;  // accentue
			}
			else
			{
				m_FFBbrake = 0.0f;
				forceX = m_motorSpeed.z*0.7f;  // rappel au centre
			}
		}
	}

	if ( m_floorHeight > 0.0f )
	{
		forceX = 0.0f;
	}

	m_engine->SetJoyForces(NormSign(forceX), NormSign(forceY));
}

// Fait entendre le moteur à plein régime.

void CPhysics::SoundMotorFull(float rTime, ObjectType type)
{
	Character*	character;
	Sound		sound;
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		freq, dir, factor, RPM;
	int			i, max;

	if ( type == OBJECT_HUMAN   ||
		 type == OBJECT_TECH    ||
		 type == OBJECT_UFO     ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   ||
		 type == OBJECT_EVIL1   ||
		 type == OBJECT_EVIL2   ||
		 type == OBJECT_EVIL3   ||
		 type == OBJECT_EVIL4   ||
		 type == OBJECT_EVIL5   )  return;

	character = m_object->RetCharacter();
	sound = SOUND_MOTOR1;
	if ( character->motorSound == 1 )  sound = SOUND_MOTOR1;
	if ( character->motorSound == 2 )  sound = SOUND_MOTOR2;
	if ( character->motorSound == 3 )  sound = SOUND_MOTOR3;
	if ( character->motorSound == 4 )  sound = SOUND_MOTOR4;
	if ( character->motorSound == 5 )  sound = SOUND_MOTOR5;
	if ( character->motorSound == 6 )  sound = SOUND_MOTOR6;

	freq = Abs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x);

	if ( freq < 0.5f )  // 1ère
	{
		freq = freq/0.5f;
		m_motorState = 1;
	}
	else if ( freq < 0.8f )  // 2ème
	{
		freq = (freq-0.5f)/0.3f;
		freq = 0.2f+freq*0.8f;
		m_motorState = 2;
	}
	else  // 3ème
	{
		freq = (freq-0.8f)/0.2f;
		freq = 0.2f+freq*0.8f;
		m_motorState = 3;
	}

	if ( m_motorSpeed.x < 0.0f && !m_bBrake )
	{
		m_motorState = -1;
	}

	freq = 0.5f+0.5f*freq;
	freq *= 1.3f;
	if ( m_floorHeight > 0.0f )  freq *= 1.3f;
	if ( freq > 2.0f )  freq = 2.0f;  // rupteur
//?	freq *= 1.0f+m_wheelSlide*1.5f;
	if ( m_object->RetStarting() )  // bloqué pendant 3,2,1,go ?
	{
		freq *= 3.0f;
		if ( m_timeMotorBurn > 1.0f )  // explosion moteur imminente ?
		{
			freq *= m_timeMotorBurn;  // augmente la fréquence
		}
		factor = 10.0f;
		m_motorState = 0;
	}
	else
	{
		factor = 10.0f;
	}
	m_soundFrequencyMotor1 = Smooth(m_soundFrequencyMotor1, freq, rTime*factor);
	m_soundAmplitudeMotor1 = Linear(m_soundAmplitudeMotor1, 1.0f, rTime*factor);

	m_soundFrequencyMotor2 = Smooth(m_soundFrequencyMotor2, freq, rTime*factor);
	m_soundAmplitudeMotor2 = Linear(m_soundAmplitudeMotor2, 1.0f, rTime*factor);

	RPM = (m_soundFrequencyMotor1-0.5f)/0.5f;
	RPM = 0.2f+RPM*0.8f;
	if ( m_object->RetStarting() )  // bloqué pendant 3,2,1,go ?
	{
		RPM = log10f((RPM+1.0f))*2.5f;
	}
	factor = Norm(Abs(m_linMotion.realSpeed.x)/50.0f);
	RPM += sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f);
	m_motorRPM = Smooth(m_motorRPM, RPM, rTime*10.0f);

	factor = 1.0f;

#if 1
	if ( m_soundAmplitudeMotor1 > 0.0f )
	{
		if ( m_soundChannelMotor1 == -1 )
		{
			if ( !m_bWater )  m_sound->Play(SOUND_MOTORs, m_object->RetPosition(0));
			m_soundChannelMotor1 = m_sound->Play(sound, m_object->RetPosition(0), 1.0f, 1.0f, TRUE);
		}
		m_sound->Position(m_soundChannelMotor1, m_object->RetPosition(0));
		m_sound->Frequency(m_soundChannelMotor1, m_soundFrequencyMotor1*factor);
		m_sound->Amplitude(m_soundChannelMotor1, m_soundAmplitudeMotor1*factor);
	}
	else
	{
		if ( m_soundChannelMotor1 != -1 )
		{
			m_sound->Stop(m_soundChannelMotor1);
			m_soundChannelMotor1 = -1;
		}
	}
#endif

#if 0
	if ( m_soundAmplitudeMotor2 > 0.0f )
	{
		if ( m_soundChannelMotor2 == -1 )
		{
			m_soundChannelMotor2 = m_sound->Play(SOUND_MOTOR2, m_object->RetPosition(0), 1.0f, 1.0f, TRUE);
		}
		m_sound->Position(m_soundChannelMotor2, m_object->RetPosition(0));
		m_sound->Frequency(m_soundChannelMotor2, m_soundFrequencyMotor2*factor);
		m_sound->Amplitude(m_soundChannelMotor2, m_soundAmplitudeMotor2*factor);
	}
	else
	{
		if ( m_soundChannelMotor2 != -1 )
		{
			m_sound->Stop(m_soundChannelMotor2);
			m_soundChannelMotor2 = -1;
		}
	}
#endif

	if ( type == OBJECT_TRAX )
	{
		m_soundTimePshhh -= rTime;

		if ( m_soundTimePshhh <= 0.0f )
		{
			m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0), 1.0f);
			m_soundTimePshhh = 4.0f+4.0f*Rand();

			max = (int)(10.0f*m_engine->RetParticuleDensity());
			mat = m_object->RetWorldMatrix(3);  // pelle
			for ( i=0 ; i<max ; i++ )
			{
				if ( Rand() < 0.5f )  dir =  1.0f;
				else                  dir = -1.0f;
				pos = D3DVECTOR(6.0f, 1.0f, 10.0f*dir);
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*2.0f;
				speed = pos;
				speed.x += (Rand()-0.5f)*6.0f;
				speed.z += Rand()*12.0f*dir;
				speed.y += Rand()*6.0f;
				pos   = Transform(*mat, pos);
				speed = Transform(*mat, speed)-pos;
				dim.x = Rand()*3.0f+3.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIMOTOR, 2.0f);
			}
		}
	}
}

// Fait entendre le moteur au ralenti.

void CPhysics::SoundMotorSlow(float rTime, ObjectType type)
{
	Character*	character;
	Sound		sound;
	float		factor, RPM;

	if ( type == OBJECT_HUMAN   ||
		 type == OBJECT_TECH    ||
		 type == OBJECT_UFO     ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   ||
		 type == OBJECT_EVIL1   ||
		 type == OBJECT_EVIL2   ||
		 type == OBJECT_EVIL3   ||
		 type == OBJECT_EVIL4   ||
		 type == OBJECT_EVIL5   )  return;

	character = m_object->RetCharacter();
	sound = SOUND_MOTOR1;
	if ( character->motorSound == 1 )  sound = SOUND_MOTOR1;
	if ( character->motorSound == 2 )  sound = SOUND_MOTOR2;
	if ( character->motorSound == 3 )  sound = SOUND_MOTOR3;
	if ( character->motorSound == 4 )  sound = SOUND_MOTOR4;
	if ( character->motorSound == 5 )  sound = SOUND_MOTOR5;
	if ( character->motorSound == 6 )  sound = SOUND_MOTOR6;

	if ( m_object->RetStarting() )  // bloqué pendant 3,2,1,go ?
	{
		factor = 1.5f;
	}
	else
	{
		factor = 0.5f;
	}

	m_soundFrequencyMotor1 = Linear(m_soundFrequencyMotor1, 0.5f, rTime*factor);
	m_soundAmplitudeMotor1 = Linear(m_soundAmplitudeMotor1, 0.8f, rTime*factor);

	m_soundFrequencyMotor2 = Linear(m_soundFrequencyMotor2, 0.5f, rTime*factor);
	m_soundAmplitudeMotor2 = Linear(m_soundAmplitudeMotor2, 0.8f, rTime*factor);

	m_motorState = 0;

	RPM = (m_soundFrequencyMotor1-0.5f)/0.5f;
	RPM = 0.2f+RPM*0.8f;
	if ( m_object->RetStarting() )  // bloqué pendant 3,2,1,go ?
	{
		RPM = log10f((RPM+1.0f))*2.5f;
	}
	factor = Norm(Abs(m_linMotion.realSpeed.x)/50.0f);
	RPM += sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f);
	m_motorRPM = Smooth(m_motorRPM, RPM, rTime*10.0f);

	factor = 1.0f;

#if 1
	if ( m_soundAmplitudeMotor1 > 0.0f )
	{
		if ( m_soundChannelMotor1 == -1 )
		{
			if ( !m_bWater )  m_sound->Play(SOUND_MOTORs, m_object->RetPosition(0));
			m_soundChannelMotor1 = m_sound->Play(sound, m_object->RetPosition(0), 1.0f, 1.0f, TRUE);
		}
		m_sound->Position(m_soundChannelMotor1, m_object->RetPosition(0));
		m_sound->Frequency(m_soundChannelMotor1, m_soundFrequencyMotor1*factor);
		m_sound->Amplitude(m_soundChannelMotor1, m_soundAmplitudeMotor1*factor);
	}
	else
	{
		if ( m_soundChannelMotor1 != -1 )
		{
			m_sound->Stop(m_soundChannelMotor1);
			m_soundChannelMotor1 = -1;
		}
	}
#endif

#if 0
	if ( m_soundAmplitudeMotor2 > 0.0f )
	{
		if ( m_soundChannelMotor2 == -1 )
		{
			m_soundChannelMotor2 = m_sound->Play(SOUND_MOTOR2, m_object->RetPosition(0), 1.0f, 1.0f, TRUE);
		}
		m_sound->Position(m_soundChannelMotor2, m_object->RetPosition(0));
		m_sound->Frequency(m_soundChannelMotor2, m_soundFrequencyMotor2*factor);
		m_sound->Amplitude(m_soundChannelMotor2, m_soundAmplitudeMotor2*factor);
	}
	else
	{
		if ( m_soundChannelMotor2 != -1 )
		{
			m_sound->Stop(m_soundChannelMotor2);
			m_soundChannelMotor2 = -1;
		}
	}
#endif
}

// Fait entendre le moteur à l'arrêt.

void CPhysics::SoundMotorStop(float rTime, ObjectType type)
{
	Character*	character;
	Sound		sound;
	float		factor, RPM;

	if ( type == OBJECT_HUMAN   ||
		 type == OBJECT_TECH    ||
		 type == OBJECT_UFO     ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   ||
		 type == OBJECT_EVIL1   ||
		 type == OBJECT_EVIL2   ||
		 type == OBJECT_EVIL3   ||
		 type == OBJECT_EVIL4   ||
		 type == OBJECT_EVIL5   )  return;

	character = m_object->RetCharacter();
	sound = SOUND_MOTOR1;
	if ( character->motorSound == 1 )  sound = SOUND_MOTOR1;
	if ( character->motorSound == 2 )  sound = SOUND_MOTOR2;
	if ( character->motorSound == 3 )  sound = SOUND_MOTOR3;
	if ( character->motorSound == 4 )  sound = SOUND_MOTOR4;
	if ( character->motorSound == 5 )  sound = SOUND_MOTOR5;
	if ( character->motorSound == 6 )  sound = SOUND_MOTOR6;

	m_soundFrequencyMotor1 = Linear(m_soundFrequencyMotor1, 0.3f, rTime*0.3f);
	m_soundAmplitudeMotor1 = Linear(m_soundAmplitudeMotor1, 0.0f, rTime*1.0f);

	m_soundFrequencyMotor2 = Linear(m_soundFrequencyMotor2, 0.5f, rTime*0.3f);
	m_soundAmplitudeMotor2 = Linear(m_soundAmplitudeMotor2, 0.0f, rTime*1.0f);

	m_motorState = 0;

	RPM = (m_soundFrequencyMotor1-0.5f)/0.5f;
	RPM = 0.2f+RPM*0.8f;
	factor = Norm(Abs(m_linMotion.realSpeed.x)/50.0f);
	RPM += sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f);
	m_motorRPM = Smooth(m_motorRPM, RPM, rTime*10.0f);

	factor = 1.0f;

#if 1
	if ( m_soundAmplitudeMotor1 > 0.0f )
	{
		if ( m_soundChannelMotor1 == -1 )
		{
			m_soundChannelMotor1 = m_sound->Play(sound, m_object->RetPosition(0), 1.0f, 1.0f, TRUE);
		}
		m_sound->Position(m_soundChannelMotor1, m_object->RetPosition(0));
		m_sound->Frequency(m_soundChannelMotor1, m_soundFrequencyMotor1*factor);
		m_sound->Amplitude(m_soundChannelMotor1, m_soundAmplitudeMotor1*factor);
	}
	else
	{
		if ( m_soundChannelMotor1 != -1 )
		{
			m_sound->Stop(m_soundChannelMotor1);
			m_soundChannelMotor1 = -1;
		}
	}
#endif

#if 0
	if ( m_soundAmplitudeMotor2 > 0.0f )
	{
		if ( m_soundChannelMotor2 == -1 )
		{
			m_soundChannelMotor2 = m_sound->Play(SOUND_MOTOR2, m_object->RetPosition(0), 1.0f, 1.0f, TRUE);
		}
		m_sound->Position(m_soundChannelMotor2, m_object->RetPosition(0));
		m_sound->Frequency(m_soundChannelMotor2, m_soundFrequencyMotor2*factor);
		m_sound->Amplitude(m_soundChannelMotor2, m_soundAmplitudeMotor2*factor);
	}
	else
	{
		if ( m_soundChannelMotor2 != -1 )
		{
			m_sound->Stop(m_soundChannelMotor2);
			m_soundChannelMotor2 = -1;
		}
	}
#endif
}

// Fait entendre le réacteur à plein régime.

void CPhysics::SoundReactorFull(float rTime, ObjectType type)
{
	Sound		sound;
	float		freq;

	if ( m_soundChannelMotor1 == -1 )
	{
		sound = SOUND_FLY;
		m_soundChannelMotor1 = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
		m_sound->AddEnvelope(m_soundChannelMotor1, 1.0f, 1.0f, 0.6f, SOPER_CONTINUE);
		m_sound->AddEnvelope(m_soundChannelMotor1, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
	}
	else
	{
		m_sound->Position(m_soundChannelMotor1, m_object->RetPosition(0));
	}

	freq = 1.0f + m_linMotion.realSpeed.y/100.0f;
	freq *= 1.0f + Abs(m_cirMotion.realSpeed.y/5.0f);
	m_sound->Frequency(m_soundChannelMotor1, freq);
}

// Fait entendre le réacteur à l'arrêt.

void CPhysics::SoundReactorStop(float rTime, ObjectType type)
{
	if ( m_soundChannelMotor1 != -1 )  // moteur tourne ?
	{
		m_sound->FlushEnvelope(m_soundChannelMotor1);
		m_sound->AddEnvelope(m_soundChannelMotor1, 0.0f, 1.0f, 1.0f, SOPER_STOP);
		m_soundChannelMotor1 = -1;
	}
}


// Adapte la physique de l'objet en fonction du terrain.

void CPhysics::FloorAdapt(float aTime, float rTime,
						  D3DVECTOR &pos, D3DVECTOR &angle)
{
	Character*	character;
	ObjectType	type;
	D3DVECTOR	norm, iPos, nAngle;
	D3DMATRIX	matRotate;
	FPOINT		v;
	float		level, h, f, volume, force, centri, iHeight, speed, accel, mass;
	int			i, j;
	BOOL		bOldSwim;

	type = m_object->RetType();
	character = m_object->RetCharacter();

	level = m_water->RetLevel(m_object);
	bOldSwim = m_bSwim;
	SetSwim( pos.y < level );

	m_floorLevel = m_terrain->RetFloorLevel(pos);  // hauteur au-dessus du sol
	m_floorLevel += character->height;
	h = pos.y-m_floorLevel;
	iHeight = m_floorHeight;
	m_floorHeight = h;

	WaterParticule(aTime, pos, type, m_floorLevel,
				   Abs(m_linMotion.realSpeed.x),
				   Abs(m_cirMotion.realSpeed.y*15.0f));

	if ( m_type == TYPE_RACE )
	{
		iPos = m_object->RetPosition(0);
		mass = character->mass;
		if ( m_bWater )  mass *= 0.1f;
//?		pos.y = iPos.y + (m_graviSpeed-mass/500.0f)*rTime;
		pos.y = iPos.y + (m_graviSpeed-mass/15.0f*rTime)*rTime;
		if ( pos.y < m_floorLevel )  pos.y = m_floorLevel;  // pas sous le sol
		m_floorHeight = h = pos.y-m_floorLevel;

		speed = (pos.y-iPos.y)/rTime;
		accel = (speed-m_graviSpeed)/rTime;  // positif si tombe
		m_graviSpeed = speed;
		if ( m_graviSpeed > 0.0f )  // monte ?
		{
			m_graviSpeed *= 1.0f-m_graviGlu;  // colle au sol si choc récent
		}

		m_graviGlu -= rTime*10.0f;
		if ( m_graviGlu < 0.0f )  m_graviGlu = 0.0f;

		if ( m_floorHeight == 0.0f )  // au sol ?
		{
			if ( accel > 100.0f )
			{
//?				force = Norm((accel-100.0f)/100.0f);
				force = Norm((accel-100.0f)/1000.0f);
				CrashParticule(force, TRUE);  // poussière si tombe
			}
			if ( accel > 200.0f && !m_bWater )
			{
				volume = Norm((accel-200.0f)/200.0f);
				m_sound->Play(SOUND_BOUMv, pos, volume);  // bruit si tombe

				if ( m_main->RetPhase() == PHASE_SIMUL )
				{
					if ( m_terrainHard >= 0.5f )  // sol dur ?
					{
						if ( accel > 2000.0f )
						{
							m_sound->Play(SOUND_FALLg, pos, 0.8f);
						}
					}
					else
					{
						volume = Norm((accel-200.0f)/2000.0f);
						if ( volume > 0.6f )  volume = 0.6f;
						m_sound->Play(SOUND_FALLg, pos, volume);
					}
				}
			}

			SuspForce(0, -accel, 0.0f, rTime);
		}
		else	// en l'air ?
		{
			SuspForce(0, 0.0f, 0.0f, rTime);
		}

		// Calcule la force centripète qui déporte le véhicule
		// vers l'extérieur lors d'un virage.
		if ( m_floorHeight == 0.0f )
		{
			if ( m_linMotion.realSpeed.x >= 0.0f )  // avance ?
			{
				speed = Norm(Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
				force = speed*m_cirMotion.realSpeed.y;
				f = 1.0f-Norm(speed);  // gripLimit -> 0 (si vitesse élevée)
				if ( Abs(force) > character->gripLimit*f )
				{
					if ( force > 0.0f )  force -= character->gripLimit*f;
					else                 force += character->gripLimit*f;
					force *= character->gripSlide;
					centri = force*(1.0f-m_terrainHard*0.5f);
#if 0
					if ( centri > 0.0f )
					{
						if ( centri > m_centriSpeed )  f = 2.0f;
						else                           f = 4.0f;
					}
					else
					{
						if ( centri < m_centriSpeed )  f = 2.0f;
						else                           f = 4.0f;
					}
#else
//?					f = 2.0f+2.0f*speed;
					f = 2.0f+m_terrainHard*speed;  // brusque si vitesse haute
#endif
					if ( m_bBrake )
					{
						f *= 1.0f-Norm(Abs(centri/character->gripSlide));
					}
				}
				else
				{
					centri = 0.0f;
					f = 4.0f;
				}
				if ( m_bHandbrake )  // frein à main?
				{
					centri *= 2.0f;
//?					centri = 0.0f;
//?					f = 3.0f;
				}
				if ( centri >  40.0f )  centri =  40.0f;
				if ( centri < -40.0f )  centri = -40.0f;
				m_centriSpeed = SmoothP(m_centriSpeed, centri, rTime*f);

				if ( speed < 0.1f )  // presque arrêté ?
				{
					m_centriSpeed *= speed/0.1f;  // plus de force centripète
				}
			}
			else	// recule ?
			{
				m_centriSpeed = 0.0f;
			}

			m_overTurn = m_centriSpeed*character->overFactor/character->gripSlide;
			if ( m_overTurn >  character->overAngle )  m_overTurn =  character->overAngle;
			if ( m_overTurn < -character->overAngle )  m_overTurn = -character->overAngle;
		}

		if ( m_floorHeight == 0.0f && force != 0.0f )
		{
			force = Norm(Abs(m_centriSpeed/20.0f));
			SlideParticule(aTime, rTime, force*(1.0f-m_terrainHard), force*m_terrainHard);
		}
		else
		{
			SlideParticule(aTime, rTime, 0.0f, 0.0f);
		}

		if ( m_floorHeight == 0.0f )  // plaqué au sol ?
		{
			if ( m_bBrake )
			{
				// traces de frein
//?				force = Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x;
				force = Abs(m_motorSpeed.x);
				SuspForce(1, -force, 1.0f, rTime);
				if ( force <= 0.5f )
				{
					WheelParticule(aTime, rTime, 0.0f, TRUE);
				}
				else
				{
					force = (force-0.5f)/0.5f;
					WheelParticule(aTime, rTime, -m_motorSpeed.x*force, TRUE);
				}
			}
			else if ( m_wheelSlide > 0.0f )
			{
				// patine au démarrage
//?				WheelParticule(aTime, rTime, 1.0f+m_wheelSlide*0.5f, FALSE);
				WheelParticule(aTime, rTime, m_wheelSlide*2.0f, FALSE);
			}
			else
			{
				WheelParticule(aTime, rTime, 0.0f, FALSE);
			}
		}
		else
		{
			WheelParticule(aTime, rTime, 0.0f, FALSE);
		}

		SuspParticule(aTime, rTime);

		// Calcule la suspension.
		for ( j=0 ; j<2 ; j++ )
		{
			force = 0.0f;
			for ( i=0 ; i<5 ; i++ )
			{
				force += m_suspEnergy[j][i];  // somme des forces
			}
			force = NormSign(force);

			if ( Abs(force) >= Abs(m_suspDelayed[j]) )
			{
				m_suspDelayed[j] = force;
				m_suspHeight[j] = m_suspDelayed[j];
				m_suspTime[j] = 0.0f;
			}
			else
			{
				m_suspDelayed[j] = Smooth(m_suspDelayed[j], force, rTime*character->suspAbsorber*2.0f);
				m_suspHeight[j] = cosf(m_suspTime[j]*character->suspFrequency)*NormSign(m_suspDelayed[j]);
				m_suspTime[j] += rTime;
			}

			for ( i=0 ; i<5 ; i++ )
			{
				if ( i == 0)  continue;  // force du terrain ?
				m_suspEnergy[j][i] = Smooth(m_suspEnergy[j][i], 0.0f, rTime*character->suspAbsorber);
			}

			m_suspHeight[j] *= character->suspHeight;
			if ( m_camera->RetType() == CAMERA_ONBOARD )  m_suspHeight[j] *= 0.5f;
			if ( m_object->RetZoomY(0) < 1.0f )  // tout plat ?
			{
				m_suspHeight[j] = 0.0f;
			}
		}

		// Calcule la rotation après un choc.
		if ( m_chocSpin != 0.0f )  // rotation en cours ?
		{
			m_chocAngle += m_chocSpin*rTime;
			m_object->SetCirChoc(D3DVECTOR(0.0f, m_chocAngle, 0.0f));

			if ( m_chocSpin > 0.0f )
			{
				m_chocSpin -= rTime*(8.0f+m_terrainHard*5.0f);
				if ( m_chocSpin < 0.0f )  m_chocSpin = 0.0f;
			}
			if ( m_chocSpin < 0.0f )
			{
				m_chocSpin += rTime*(8.0f+m_terrainHard*5.0f);
				if ( m_chocSpin > 0.0f )  m_chocSpin = 0.0f;
			}

			if ( m_chocSpin == 0.0f )  // rotation terminée ?
			{
				angle.y += m_chocAngle;
				m_object->SetCirChoc(D3DVECTOR(0.0f, 0.0f, 0.0f));
			}
		}
	}

	if ( m_type == TYPE_TANK )
	{
		if ( type == OBJECT_WALKER ||
			 type == OBJECT_CRAZY  )
		{
			if ( m_linMotion.realSpeed.x != 0.0f &&  // marche ?
				 m_object->RetTruck() == 0 )
			{
				pos.y -= h;  // plaque immédiatement au sol
				m_floorHeight = 0.0f;
			}
		}
		else
		{
			pos.y -= h;  // plaque immédiatement au sol
			m_floorHeight = 0.0f;
		}
	}

	if ( type == OBJECT_HUMAN ||
		 type == OBJECT_TECH  )  return;  // toujours droit

	if ( m_type == TYPE_RACE )
	{
		if ( h > 0.0f )  // en l'air ?
		{
			f = h/character->suspDetect;
			if ( f < 1.0f )  // presque au sol ?
			{
				nAngle = angle;
				FloorAngle(pos, nAngle);  // adapte l'angle au terrain
				angle.x = angle.x*f + nAngle.x*(1.0f-f);
				angle.z = angle.z*f + nAngle.z*(1.0f-f);
			}
			return;
		}
	}

	if ( m_type == TYPE_MASS )
	{
		iPos = m_object->RetPosition(0);
		if ( pos.y < m_floorLevel )  // collision avec le sol ?
		{
			pos.y = m_floorLevel;  // pas sous le sol
//?			m_linMotion.realSpeed.y = -m_linMotion.realSpeed.y;
		}
		m_floorHeight = pos.y-m_floorLevel;
	}

	FloorAngle(pos, angle);  // adapte l'angle au terrain
}

// Force actionnée sur la suspension. Une force négative effectue une
// pression vers le bas (écrasement).
//	rank=0 -> force du terrain
//	rank=1 -> force de freinage
//	rank=2 -> force des effets (objet posé dans véhicule)

void CPhysics::SuspForce(int rank, float force, float front, float rTime)
{
	float	factor;

	if ( rank == 0 )  // force du terrain ?
	{
		factor = NormSign(1.0f+front*2.0f);  // force sur l'avant
		m_suspEnergy[0][rank] = NormSign(Smooth(m_suspEnergy[0][rank], force*factor/20.0f, rTime));

		factor = NormSign(1.0f-front*2.0f);  // force sur l'arrière
		m_suspEnergy[1][rank] = NormSign(Smooth(m_suspEnergy[1][rank], force*factor/20.0f, rTime));
	}
	else	// autre force ?
	{
		factor = NormSign(1.0f+front*2.0f);  // force sur l'avant
		m_suspEnergy[0][rank] = NormSign(m_suspEnergy[0][rank]+force*factor);

		factor = NormSign(1.0f-front*2.0f);  // force sur l'arrière
		m_suspEnergy[1][rank] = NormSign(m_suspEnergy[1][rank]+force*factor);
	}
}

// Calcule l'angle d'un objet avec le terrain.

void CPhysics::FloorAngle(const D3DVECTOR &pos, D3DVECTOR &angle)
{
	Character*	character;
	D3DVECTOR	pw, norm, choc;
	FPOINT		nn;
	float		a, a1, a2;

	a = angle.y;

	choc = m_object->RetCirChoc();  // tient compte rotation choc
	a += choc.y;

	if ( m_type == TYPE_RACE ||
		 m_type == TYPE_TANK )
	{
		character = m_object->RetCharacter();

		pw.x = pos.x+character->wheelFrontPos.x*cosf(a+PI*0.0f);
		pw.y = pos.y;
		pw.z = pos.z-character->wheelFrontPos.x*sinf(a+PI*0.0f);
		a1 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelFrontPos.x);

		pw.x = pos.x-character->wheelBackPos.x*cosf(a+PI*1.0f);
		pw.y = pos.y;
		pw.z = pos.z+character->wheelBackPos.x*sinf(a+PI*1.0f);
		a2 = atanf(m_terrain->RetFloorHeight(pw)/-character->wheelBackPos.x);

		angle.z = (a2-a1)/2.0f;

		pw.x = pos.x+character->wheelBackPos.z*cosf(a+PI*0.5f)*cosf(angle.z);
		pw.y = pos.y;
		pw.z = pos.z-character->wheelBackPos.z*sinf(a+PI*0.5f)*cosf(angle.z);
		a1 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelBackPos.z);

		pw.x = pos.x+character->wheelBackPos.z*cosf(a+PI*1.5f)*cosf(angle.z);
		pw.y = pos.y;
		pw.z = pos.z-character->wheelBackPos.z*sinf(a+PI*1.5f)*cosf(angle.z);
		a2 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelBackPos.z);

		angle.x = (a2-a1)/2.0f;
	}
	
	if ( m_type == TYPE_MASS )
	{
		if ( m_floorHeight == 0.0f &&  // au sol ?
			 m_terrain->GetNormal(norm, pos) )
		{
			nn = RotatePoint(-a, FPOINT(norm.z, norm.x));
			angle.x =  sinf(nn.x);
			angle.z = -sinf(nn.y);
		}
	}
}


// Adapte la physique de l'objet en fonction des autres objets (collisions).
// Retourne 0 -> objet mobile
// Retourne 1 -> objet immobile (à cause collision)
// Retourne 2 -> objet détruit

int CPhysics::ObjectAdapt(D3DVECTOR &pos, D3DVECTOR &angle,
						  float aTime, float rTime)
{
	CObject		*pObj, *maxObj;
	D3DMATRIX	matRotate;
	D3DVECTOR	iiPos, iPos, oPos, oAngle, oSpeed, repulse;
	FPOINT		adjust, inter, maxAdjust, maxInter;
	Sound		sound, maxSound;
	float		iRad, oRad, distance, force, volume, hardness, maxHardness;
	float		iMass, oMass, factor, chocAngle, len, maxLen, maxAngle;
	float		priority, maxPriority;
	int			i, j, colType;
	ObjectType	iType, oType;

	if ( m_object->RetRuin() )  return 0;  // brûle ou explose ?
	if ( !m_object->RetClip() )  return 0;
	if ( m_object->RetGhost() )  return 0;  // voiture fantome ?

	// iiPos = centre sphère à l'ancienne position.
	// iPos  = centre sphère à la nouvelle position.
	m_object->GetCrashSphere(0, iiPos, iRad);
	iPos = iiPos + (pos - m_object->RetPosition(0));
	iType = m_object->RetType();

	UpdateCorner(pos, angle);  // m_newCorner <- nouveaux coins

	maxLen = 0.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;  // soi-même ?
		if ( pObj->RetTruck() != 0 )  continue;  // objet transporté ?
		if ( !pObj->RetEnable() )  continue;  // inactif ?
		if ( pObj->RetRuin() )  continue;  // brûle ou explose ?
		if ( pObj->RetDead() )  continue;  // homme mort ?
		if ( pObj->RetExplo() )  continue;  // en cours de destruction ?
		if ( pObj->RetGhost() )  continue;  // voiture fantome ?

		oType = pObj->RetType();
		if ( oType == OBJECT_NULL                          )  continue;
		if ( iType == OBJECT_UFO   && oType != OBJECT_CAR  )  continue;
		if ( iType == OBJECT_PIECE && oType != OBJECT_CAR  )  continue;

		pObj->GetJotlerSphere(oPos, oRad);
		if ( oRad > 0.0f )
		{
			JostleObject(pObj, iPos, iRad, oPos, oRad);
		}

		if ( (oType == OBJECT_WAYPOINT ||
			  oType == OBJECT_TARGET   ) &&
			 pObj->RetEnable()   &&
			 iType == OBJECT_CAR )
		{
			oPos = pObj->RetPosition(0);
			distance = Length2d(oPos, iPos);
			if ( distance < 40.0f )
			{
				DoorCounter(pObj, oType);
			}
		}

		if ( iType == OBJECT_CAR  ||
			 iType == OBJECT_TRAX )
		{
			if ( pObj->RetCrashLineTotal() > 1 )
			{
				if ( CrashCornerRect(pObj, pos, angle, adjust, inter, chocAngle, priority, hardness, sound, aTime, rTime) != 0 )
				{
					len = Length(adjust.x, adjust.y);
					if ( len > maxLen )
					{
						maxLen = len;
						maxObj = pObj;
						maxAdjust = adjust;
						maxInter = inter;
						maxAngle = chocAngle;
						maxPriority = priority;
						maxHardness = hardness;
						maxSound = sound;
					}
				}
			}

			if ( pObj->RetCrashSphereTotal() >= 1 )
			{
				if ( CrashCornerCircle(pObj, pos, angle, adjust, inter, chocAngle, priority, hardness, sound, aTime, rTime) != 0 )
				{
					len = Length(adjust.x, adjust.y);
					if ( len > maxLen )
					{
						maxLen = len;
						maxObj = pObj;
						maxAdjust = adjust;
						maxInter = inter;
						maxAngle = chocAngle;
						maxPriority = priority;
						maxHardness = hardness;
						maxSound = sound;
					}
				}
			}

			continue;
		}

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRad) )
		{
			distance = Length(oPos, iPos);
			if ( distance < iRad+oRad )  // collision ?
			{
				distance = Length(oPos, iiPos);
				if ( distance >= iRad+oRad )  // voir (*)
				{
					m_bCollision = TRUE;
					m_bObstacle = TRUE;

					colType = 0;
					sound = pObj->RetCrashSphereSound(j-1);
					if ( sound != SOUND_CLICK )
					{
						force = Abs(m_linMotion.realSpeed.x);
						if ( iType == OBJECT_TRAX )  force *= 10.0f;
						hardness = pObj->RetCrashSphereHardness(j-1);
						force *= hardness*2.0f;
						if ( ExploOther(iType, pObj, oType, force, hardness) )  continue;
						colType = ExploHimself(iType, oType, force, hardness, D3DVECTOR(NAN,NAN,NAN));
						if ( colType == 2 )  return 2;  // détruit ?
						if ( colType == 0 )  continue;  // passe outre ?
					}

					force = Length(m_linMotion.realSpeed);
					force *= pObj->RetCrashSphereHardness(j-1);
					volume = Norm(Abs(force/20.0f));
					if ( sound != SOUND_CLICK &&
						 iType != OBJECT_HUMAN &&
						 oType != OBJECT_BOT3  &&
						 oType != OBJECT_EVIL1 &&
						 oType != OBJECT_EVIL3 &&
						 aTime-m_lastSoundCollision >= 0.1f )
					{
						m_lastSoundCollision = aTime;
						m_sound->Play(sound, m_object->RetPosition(0), volume);
					}

//?					m_repeatCollision += 2;
//?					if ( m_repeatCollision > 10 )
//?					{
//?						m_repeatCollision = 10;
//?					}

					if ( colType == 3 )  // passe au travers ?
					{
						iMass = m_object->RetCharacter()->mass;
						oMass = pObj->RetCharacter()->mass;
						if ( iMass == 0.0f || oMass == 0.0f )
						{
							factor = 0.6f;
						}
						else
						{
							factor = Norm(0.25f+(iMass/oMass)*0.25f);
						}
						m_linMotion.currentSpeed *= factor;  // on est freiné
						continue;  // passe outre
					}

					m_centriSpeed = 0.0f;
					m_overTurn = 0.0f;
					m_overBrake = 0.0f;

					if ( m_chocSpin == 0.0f )
					{
						m_chocSpin = ChocSpin(oPos, force);
					}

//?					repulse = Normalize(iPos-oPos)*force;
					repulse = ChocRepulse(m_object, pObj, iPos, oPos);
					repulse *= 0.5f;  // amortissement
//?					repulse.x *= 1.0f+(m_repeatCollision-2)*0.1f;
					ChocObject(m_object, -repulse);
					ChocObject(pObj, repulse);
					return 1;
				}
			}
		}
	}

	if ( maxLen > 0.0f )
	{
		colType = CrashCornerDo(maxObj, pos, maxAdjust, maxInter, maxAngle,
								maxPriority, maxHardness, maxSound,
								aTime, rTime);
		if ( colType == 2 )  return 2;  // détruit ?
		if ( colType != 0 )  return 0;
	}

	if ( m_repeatCollision > 0 )
	{
		m_repeatCollision --;
	}
	return 0;
}

// Calcule la force de répulsion à appliquer après une collision.

D3DVECTOR CPhysics::ChocRepulse(CObject *pObj1, CObject *pObj2,
								D3DVECTOR p1, D3DVECTOR p2)
{
	CPhysics*	physics;
	D3DMATRIX	matRotate;
	D3DVECTOR	v1, v2, a1, a2, c;

	c = D3DVECTOR(0.0f, 0.0f, 0.0f);

	physics = pObj1->RetPhysics();
	if ( physics == 0 )  return c;
	v1 = physics->RetLinMotion(MO_CURSPEED);
	MatRotateXZY(matRotate, pObj1->RetAngle(0));
	v1 = Transform(matRotate, v1);

	physics = pObj2->RetPhysics();
	if ( physics == 0 )
	{
		a1 = Projection(p1, p2, p1+v1);
//?		c = (a1-p1)*2.0f;
		c = (a1-p1)*3.0f;
	}
	else
	{
		v2 = physics->RetLinMotion(MO_CURSPEED);
		MatRotateXZY(matRotate, pObj2->RetAngle(0));
		v2 = Transform(matRotate, v2);

		a1 = Projection(p1, p2, p1+v1);
		a2 = Projection(p1, p2, p2+v2);
		c = (a1-p1) - (a2-p2);
	}

	if ( c.x == 0.0f && c.y == 0.0f && c.z == 0.0f )
	{
		c = Normalize(p1-p2);
	}

	return c;
}

// Effectue un choc sur un objet suite à une collision.

void CPhysics::ChocObject(CObject *pObj, D3DVECTOR repulse)
{
	CPhysics*	physics;
	PhysicsType	type;
	D3DMATRIX	matRotate;
	D3DVECTOR	angle, speed, current;

	physics = pObj->RetPhysics();
	if ( physics == 0 )  return;
	type = physics->RetType();

	angle = pObj->RetAngle(0);
	MatRotateXZY(matRotate, -angle);
	speed = Transform(matRotate, repulse);

	if ( type == TYPE_RACE ||
		 type == TYPE_TANK )
	{
		speed.y = 0.0f;  // reste toujours au sol
		speed.z *= 0.4f;  // glisse peu latéralement
	}
	if ( type == TYPE_MASS )
	{
//?		speed.y = 50.0f;  //????
		physics->SetCirMotionY(MO_CURSPEED, 3.0f);
	}

	current = physics->RetLinMotion(MO_CURSPEED);
	physics->SetLinMotion(MO_CURSPEED, current+speed);
}

// Calcule l'angle de rotation suite à un choc.

float CPhysics::ChocSpin(D3DVECTOR obstacle, float force)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, p;
	FPOINT		center, p1, p2;
	float		a;

	if ( m_type != TYPE_RACE )  return 0.0f;
	if ( force < 40.0f )  return 0.0f;  // petit/moyen choc ?

	pos = m_object->RetPosition(0);
	center.x = pos.x;
	center.y = pos.z;
	mat = m_object->RetWorldMatrix(0);
	p = Transform(*mat, D3DVECTOR(2.0f, 0.0f, 0.0f));
	p1.x = p.x;
	p1.y = p.z;
	p2.x = obstacle.x;
	p2.y = obstacle.z;
	a = RotateAngle(center, p1, p2);

	if ( force > 80.0f )  force = 80.0f;
	force *= 0.3f;

	a /= PI*0.25f;  // 0..8 (octan)
	if ( a >= 2.0f && a <= 4.0f )  a -= 2.0f;  // 0..2
	if ( a >= 4.0f && a <= 6.0f )  a += 2.0f;  // 6..8

	if ( a <= 1.0f )  return -force*a;         // choc côté droite, devant
	if ( a <= 2.0f )  return -force*(2.0f-a);  // choc côté droite, derrière
	if ( a <= 7.0f )  return  force*(a-6.0f);  // choc côté gauche, derrière
	                  return  force*(8.0f-a);  // choc côté gauche, devant
}

// Bouscule un objet.

BOOL CPhysics::JostleObject(CObject* pObj, D3DVECTOR iPos, float iRad,
							D3DVECTOR oPos, float oRad)
{
	D3DVECTOR	speed;
	float		distance, force, d, f;

	distance = Length(oPos, iPos);
	if ( distance >= iRad+oRad )  return FALSE;

	d = (iRad+oRad)/2.0f;
	f = (distance-d)/d;  // 0=loin, 1=proche
	if ( f < 0.0f )  f = 0.0f;
	if ( f > 1.0f )  f = 1.0f;

	speed = m_linMotion.realSpeed;
	speed.y = 0.0f;
	force = Length(speed)*f*0.05f;
	if ( force > 1.0f )  force = 1.0f;

	if ( m_soundTimeJostle >= 0.20f )
	{
		m_soundTimeJostle = 0.0f;
		m_sound->Play(SOUND_JOSTLE, iPos, force);
	}

	return pObj->JostleObject(force);
}

// Bouscule forcément un objet.

BOOL CPhysics::JostleObject(CObject* pObj, float force)
{
	D3DVECTOR	oPos;
	float		oRad;

	pObj->GetJotlerSphere(oPos, oRad);
	if ( oRad <= 0.0f )  return FALSE;

	if ( m_soundTimeJostle >= 0.20f )
	{
		m_soundTimeJostle = 0.0f;
		m_sound->Play(SOUND_JOSTLE, pObj->RetPosition(0), force);
	}

	return pObj->JostleObject(force);
}

// Action de l'explosion sur l'objet tamponné.
// Retourne TRUE s'il faut ignorer cet obstacle.

BOOL CPhysics::ExploOther(ObjectType iType,
						  CObject *pObj, ObjectType oType,
						  float force, float hardness)
{
	CPyro*		pyro;
	CAuto*		automatic;
	float		f;

	if ( !pObj->RetEnable() )  return TRUE;

	JostleObject(pObj, 1.0f);  // bouscule l'objet

	f = Norm(force/40.0f);

	if ( force > 0.0f &&
		 (oType == OBJECT_DOOR2 ||
		  oType == OBJECT_DOOR3 ) &&
		 hardness == 0.44f )
	{
		automatic = pObj->RetAuto();
		if ( automatic != 0 )
		{
			automatic->Start(2);  // casse le mécanisme
		}
	}
	if ( force > 0.0f &&
		 oType == OBJECT_ALIEN8 &&
		 hardness == 0.44f )
	{
		automatic = pObj->RetAuto();
		if ( automatic != 0 )
		{
			automatic->Start(2);  // casse le mécanisme
		}
	}
	if ( force > 0.0f &&
		 oType == OBJECT_COMPUTER &&
		 hardness >= 0.42f &&
		 hardness <= 0.44f )
	{
		automatic = pObj->RetAuto();
		if ( automatic != 0 )
		{
			if ( hardness == 0.44f )
			{
				automatic->Start(2);  // casse le mécanisme
			}
			if ( hardness == 0.43f )
			{
				automatic->Start(3);  // casse le mécanisme
			}
			if ( hardness == 0.42f )
			{
				automatic->Start(4);  // casse le mécanisme
			}
		}
	}
	if ( force > 0.0f &&
		 oType == OBJECT_INCA7 &&
		 hardness == 0.44f )
	{
		automatic = pObj->RetAuto();
		if ( automatic != 0 )
		{
			automatic->Start(2);  // casse le mécanisme
		}
	}
		 
	if ( force > 50.0f &&
		 (oType == OBJECT_FRET  ||
		  oType == OBJECT_METAL ) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_EXPLOT, pObj);  // destruction totale
	}

	if ( force > 0.0f &&
		 oType == OBJECT_BARREL )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGT, pObj);  // destruction totale
	}
	if ( force > 0.0f &&
		 (oType == OBJECT_BARRELa ||
		  oType == OBJECT_ATOMIC  ) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGA, pObj);  // destruction totale
	}

	if ( force > 25.0f &&
		 (oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10) )
	{
		pyro = new CPyro(m_iMan);
//-		pyro->Create(PT_EJECT, pObj);  // destruction totale
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 25.0f &&
		 (oType == OBJECT_STONE   ||
		  oType == OBJECT_URANIUM ) )
	{
		pyro = new CPyro(m_iMan);
//-		pyro->Create(PT_FRAGT, pObj);  // destruction totale
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 25.0f &&
		 ((oType >= OBJECT_BARRIER4 && oType <= OBJECT_BARRIER5) ||
		  oType == OBJECT_BARRIER19 ) )
	{
		pyro = new CPyro(m_iMan);
//-		pyro->Create(PT_EJECT, pObj);  // destruction totale
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 25.0f &&
		 ((oType >= OBJECT_ROADSIGN1  && oType <= OBJECT_ROADSIGN21) ||
		  (oType >= OBJECT_ROADSIGN26 && oType <= OBJECT_ROADSIGN30) ) )
	{
		pyro = new CPyro(m_iMan);
//-		pyro->Create(PT_EJECT, pObj);  // destruction totale
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 25.0f &&
//?		 (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10) )
		 (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX3) )
	{
		pyro = new CPyro(m_iMan);
//?		pyro->Create(PT_EJECT, pObj);  // destruction totale
		pyro->Create(PT_FRAGT, pObj);  // destruction totale
	}

	if ( force > 25.0f &&
		 (oType >= OBJECT_BOX5 && oType <= OBJECT_BOX6) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 25.0f &&
		 (oType >= OBJECT_TOYS1 && oType <= OBJECT_TOYS5) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( oType == OBJECT_PIECE ||
		 oType == OBJECT_CONE  )
	{
		pyro = new CPyro(m_iMan);
//-		pyro->Create(PT_EJECT, pObj);  // destruction totale
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 0.0f &&
		 (oType == OBJECT_EVIL1 ||
		  oType == OBJECT_EVIL3 ) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_EXPLOO, pObj);  // destruction totale
	}

	if ( force > 10.0f &&
		 (oType == OBJECT_BOT1    ||
		  oType == OBJECT_BOT2    ||
		  oType == OBJECT_BOT3    ||
		  oType == OBJECT_BOT4    ||
		  oType == OBJECT_BOT5    ||
		  oType == OBJECT_CARROT  ||
		  oType == OBJECT_STARTER ||
		  oType == OBJECT_WALKER  ||
		  oType == OBJECT_CRAZY   ||
		  oType == OBJECT_GUIDE   ) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
	}

	if ( force > 25.0f &&
		 oType == OBJECT_CAR )  // véhicule ?
	{
		pObj->ExploObject(EXPLO_BOUM, force/200.0f);
	}

	if ( force > 25.0f &&
		 oType == OBJECT_UFO )  // soucoupe ?
	{
		pObj->ExploObject(EXPLO_BOUM, force/200.0f);
	}

	if ( force > 25.0f &&
		 iType != OBJECT_TRAX &&
		 oType == OBJECT_TRAX )  // trax ?
	{
		if ( hardness > 0.5f )  // bouton arrière touché ?
		{
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_EXPLOT, pObj);  // destruction totale
		}
	}

	if ( force > 10.0f &&
		 (oType == OBJECT_MOBILEtg ||
		  oType == OBJECT_TNT      ) )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGT, pObj);  // destruction totale
	}

	if ( force > 0.0f &&
		 oType == OBJECT_MINE )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGT, pObj);  // destruction totale
	}

	return FALSE;
}

// Action de l'explosion sur l'objet lui-même.
// Retourne 0 -> objet mobile
// Retourne 1 -> objet immobile
// Retourne 2 -> objet détruit
// Retourne 3 -> passe au travers

int CPhysics::ExploHimself(ObjectType iType, ObjectType oType,
						   float force, float hardness, D3DVECTOR impact)
{
	PyroType	type;
	ObjectType	fType;
	CObject*	fret;
	CMotion*	motion;
	CPyro*		pyro;
	int			colli;

	m_main->InfoCollision(oType);

	FFBCrash(force/50.0f, 0.3f, 2.0f);

	fret = m_object->RetFret();
	if ( fret != 0 )
	{
		fType = fret->RetType();
		if ( fType == OBJECT_BOT1   ||
			 fType == OBJECT_BOT2   ||
			 fType == OBJECT_BOT3   ||
			 fType == OBJECT_BOT4   ||
			 fType == OBJECT_BOT5   ||
			 fType == OBJECT_WALKER ||
			 fType == OBJECT_CRAZY  )
		{
			motion = fret->RetMotion();
			if ( motion != 0 )
			{
				motion->SetAction(MB_FEAR, 10.0f);  // le robot a peur !
			}
		}
	}

	if ( force > 0.0f &&
		 (oType == OBJECT_BOT3  ||
		  oType == OBJECT_EVIL1 ||
		  oType == OBJECT_EVIL3 ) )
	{
		return 3;  // passe au travers sans aucun dégat
	}

	if ( force > 10.0f &&
		 (oType == OBJECT_TNT      ||
		  oType == OBJECT_MOBILEtg ) )
	{
		if ( iType == OBJECT_HUMAN )  type = PT_DEADG;
		else                          type = PT_EXPLOT;
		pyro = new CPyro(m_iMan);
		pyro->Create(type, m_object);  // destruction totale
		return 2;
	}

	if ( force > 0.0f &&
		 oType == OBJECT_MINE )
	{
		if ( iType == OBJECT_HUMAN )
		{
			type = PT_DEADG;
		}
		else
		{
			type = PT_EXPLOT;
		}
		pyro = new CPyro(m_iMan);
		pyro->Create(type, m_object);  // destruction totale
		return 2;
	}

	if ( force > 0.0f &&
		 iType != OBJECT_TRAX &&
		 oType == OBJECT_TRAX &&
		 hardness < 0.5f )  // pas le bouton arrière ?
	{
		force /= 10.0f;
		if ( m_object->ExploObject(EXPLO_BOUM, force) )  return 2;
		return 1;
	}

	if ( oType == OBJECT_CAR &&
		 (iType == OBJECT_BOT1    ||
		  iType == OBJECT_BOT2    ||
		  iType == OBJECT_BOT3    ||
		  iType == OBJECT_BOT4    ||
		  iType == OBJECT_BOT5    ||
		  iType == OBJECT_CARROT  ||
		  iType == OBJECT_STARTER ||
		  iType == OBJECT_WALKER  ||
		  iType == OBJECT_CRAZY   ||
		  iType == OBJECT_GUIDE   ) )
	{
//?		pyro = new CPyro(m_iMan);
//?		pyro->Create(PT_ACROBATIC, m_object, f);  // voltige
		return 3;  // passe au travers sans aucun dégat
	}

	colli = 1;  // immobile

	if ( force > 25.0f &&
		 (iType == OBJECT_HUMAN ||
		  iType == OBJECT_CAR   ||
		  iType == OBJECT_UFO   ) )  // véhicule ?
	{
		if ( oType == OBJECT_TOWER    ||
			 oType == OBJECT_NUCLEAR  ||
			 oType == OBJECT_PARA     ||
			 oType == OBJECT_COMPUTER ||
			 oType == OBJECT_DOCK     ||
			 oType == OBJECT_REMOTE   ||
			 oType == OBJECT_STAND    )  // bâtiment ?
		{
			force /= 200.0f;
		}
		else
		if ( (oType == OBJECT_DOOR2 ||
			  oType == OBJECT_DOOR3 ) &&
			 hardness == 0.44f )  // casse le mécanisme ?
		{
			return 1;
		}
		if ( oType == OBJECT_ALIEN8 &&
			 hardness == 0.44f )  // casse le mécanisme ?
		{
			return 1;
		}
		else
		if ( oType == OBJECT_FRET     ||
			 oType == OBJECT_STONE    ||
			 oType == OBJECT_METAL    ||
			 (oType >= OBJECT_BARRIER4 && oType <= OBJECT_BARRIER5) ||
			 oType == OBJECT_BARRIER19 ||
			 (oType >= OBJECT_ROADSIGN1  && oType <= OBJECT_ROADSIGN21) ||
			 (oType >= OBJECT_ROADSIGN26 && oType <= OBJECT_ROADSIGN30) ||
//?			 (oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10) ||
//?			 (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10) )
			 (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX3) )
		{
			force /= 500.0f;
			colli = 3;  // passe au travers
		}
		else
		if ( oType == OBJECT_BOX4 )
		{
			return 1;
		}
		else
		if ( oType == OBJECT_PIECE ||
			 oType == OBJECT_CONE  )
		{
			return 3;  // passe au travers sans aucun dégat
		}
		else
		if ( oType == OBJECT_URANIUM ||
			 oType == OBJECT_BARREL  ||
			 oType == OBJECT_BARRELa ||
			 oType == OBJECT_ATOMIC  )
		{
			force /= 500.0f;
			colli = 3;  // passe au travers
		}
		else
		{
			force /= 200.0f;
		}

		if ( m_object->ExploObject(EXPLO_BOUM, force, impact) )  return 2;
	}

	return colli;
}


// Gestion du passage multiple sur un objet, dans les circuits
// à plusieurs tours.

void CPhysics::DoorCounter(CObject *pObj, ObjectType oType)
{
	CPyro*		pyro;
	float		freq;
	int			rank, counter, lap;
	BOOL		bSound;

	if ( pObj == m_lastDoorCounter )  return;
	m_lastDoorCounter = pObj;

	rank = pObj->RetRankCounter();
	lap = m_main->RetLapProgress();
	if ( lap == 0 )  lap = 1000;
	if ( rank != -1 )
	{
		if ( m_doorRank%lap != rank )
		{
			m_sound->Play(SOUND_ERROR, m_object->RetPosition(0), 1.0f, 1.0f);
			return;
		}
		m_doorRank ++;
	}

	counter = pObj->RetPassCounter();
	if ( counter == 0 )
	{
		m_main->InfoCollision(oType);

		if ( oType == OBJECT_TARGET )  freq = 1.0f;
		else                           freq = 1.5f;
		m_sound->Play(SOUND_WAYPOINT, m_object->RetPosition(0), 1.0f, freq);

		pObj->SetLock(TRUE);
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_WPCHECK, pObj);
	}
	else
	{
		bSound = m_main->IncProgress();
		counter --;
		pObj->SetPassCounter(counter);

		m_main->InfoCollision(oType);

		if ( !bSound )
		{
			if ( oType == OBJECT_TARGET )  freq = 1.0f;
			else                           freq = 1.5f;
			if ( counter == 0 )            freq *= 1.5f;
			m_sound->Play(SOUND_WAYPOINT, m_object->RetPosition(0), 1.0f, freq);
		}

		if ( counter == 0 )
		{
			pObj->SetLock(TRUE);
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_WPCHECK, pObj);
		}
		else
		{
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_WPVIBRA, pObj);
		}
	}
}


// Gestion de la collision d'un véhicule rectangulaire avec un obstacle
// rectangulaire.

int CPhysics::CrashCornerRect(CObject *pObj, const D3DVECTOR &pos, const D3DVECTOR &angle,
							  FPOINT &adjust, FPOINT &inter,
							  float &chocAngle, float &priority,
							  float &hardness, Sound &sound,
							  float aTime, float rTime)
{
	D3DVECTOR	oPos;
	FPOINT		bbMin, bbMax;
	FPOINT		oP1,oP2, t1,t2, p, pp, oneP, oneT1,oneT2;
	FPOINT		maxAdjust, minAdjust, maxImpact, minImpact;
	FPOINT		maxP1,minP1, maxP2,minP2;
	Sound		maxSound, minSound, oneSound;
	float		maxHardness, minHardness, oneHardness;
	float		maxPriority, minPriority, onePriority;
	float		maxLen, minLen;
	float		len, a, h;
	int			i, j, max, nbInter, onePart;
	BOOL		bNew;

	max = pObj->RetCrashLineTotal();
	if ( max < 2 )  return 0;

	h = pObj->RetCrashLineHeight();
	if ( h != NAN )
	{
		oPos = pObj->RetPosition(0);
		if ( pos.y > oPos.y+h )  return 0;
	}

	pObj->RetCrashLineBBox(bbMin, bbMax);
	if ( m_newCorner.min.x > bbMax.x )  return 0;
	if ( m_newCorner.max.x < bbMin.x )  return 0;
	if ( m_newCorner.min.y > bbMax.y )  return 0;
	if ( m_newCorner.max.y < bbMin.y )  return 0;

	minLen = 100000.0f;
	nbInter = 0;  // nb d'intersections
	onePart = 0;  // unknow segment

	pObj->GetCrashLine(0, oP2, bNew);
	for ( j=1 ; j<max ; j++ )
	{
		oP1 = oP2;
		pObj->GetCrashLine(j, oP2, bNew);
		if ( bNew )  continue;

		// En cas d'intersection du segment de l'obstacle avec un
		// segment du véhicule, il faut vérifier si tous les segments
		// d'obstacle interrompent le même segment de véhicule.
		// Si oui, on est dans le cas particulier "one" où une pointe
		// d'obstacle entre dans un flan du véhicule.

		for ( i=0 ; i<4 ; i++ )
		{
//?			if ( m_linMotion.realSpeed.x > 0.0f && i == 2 )  continue;
//?			if ( m_linMotion.realSpeed.x < 0.0f && i == 0 )  continue;

			t1.x = m_newCorner.p[i].x;
			t1.y = m_newCorner.p[i].z;
			t2.x = m_newCorner.p[(i+1)%4].x;
			t2.y = m_newCorner.p[(i+1)%4].z;
			if ( IntersectSegment(oP1,oP2, t1,t2, pp) )
			{
				nbInter ++;
				if ( onePart == 0 )  // unknow segment ?
				{
					onePart = i+1;  // 1..4 = quel segment du véhicule
				}
				if ( onePart >= 1 && onePart <= 4 && onePart != i+1 )
				{
					onePart = -1;  // plusieurs segments
				}
				if ( onePart >= 1 && onePart <= 4 )
				{
					oneP = pp;
					if ( IsInside(t1,t2, oP1) )  oneP = oP1;
					if ( IsInside(t1,t2, oP2) )  oneP = oP2;
				}
				oneT1 = t1;
				oneT2 = t2;
				oneHardness = pObj->RetCrashLineHardness(j);
				oneSound = pObj->RetCrashLineSound(j);
				onePriority = 1.0f;
				if ( m_linMotion.realSpeed.x > 0.0f && i == 2 )  onePriority = 0.0f;
				if ( m_linMotion.realSpeed.x < 0.0f && i == 0 )  onePriority = 0.0f;
			}
		}

		// Cherche pour les 4 coins du véhicule celui qui nécessite le
		// plus grand déplacement (maxAdjust) pour éviter l'obstacle par
		// un déplacement perpendiculaire au segment de l'obstacle.

		maxLen = -1.0f;
		for ( i=0 ; i<4 ; i++ )
		{
//?			if ( m_linMotion.realSpeed.x > 0.0f && i/2 != 0 )  continue;
//?			if ( m_linMotion.realSpeed.x < 0.0f && i/2 == 0 )  continue;

			p.x = m_newCorner.p[i].x;
			p.y = m_newCorner.p[i].z;

			if ( IsInside(oP1,oP2, p) )
			{
				pp = Projection(oP1,oP2, p);
				len = Length(p, pp);
				if ( len > maxLen )
				{
					adjust.x = pp.x-p.x;
					adjust.y = pp.y-p.y;
					if ( CrashValidity(pObj, adjust) )
					{
						maxLen = len;
						maxAdjust = adjust;
						maxImpact = pp;
						maxP1 = oP1;
						maxP2 = oP2;
						maxHardness = pObj->RetCrashLineHardness(j);
						maxSound = pObj->RetCrashLineSound(j);
						maxPriority = 1.0f;
						if ( m_linMotion.realSpeed.x > 0.0f && i/2 != 0 )  maxPriority = 0.0f;
						if ( m_linMotion.realSpeed.x < 0.0f && i/2 == 0 )  maxPriority = 0.0f;
					}
				}
			}
		}

		// Le plus petit déplacement pour chaque segment d'obstacle
		// sera le meilleur.

		if ( maxLen > -1.0f )
		{
			if ( maxLen < minLen )
			{
				minLen = maxLen;
				minAdjust = maxAdjust;
				minImpact = maxImpact;
				minP1 = maxP1;
				minP2 = maxP2;
				minHardness = maxHardness;
				minSound = maxSound;
				minPriority = maxPriority;
			}
		}
	}

	if ( nbInter > 0 && minLen < 100000.0f )
	{
		if ( nbInter == 2 && onePart >= 1 && onePart <= 4 )  // cas particulier "one" ?
		{
			pp = Projection(oneT1,oneT2, oneP);
			minAdjust.x = oneP.x-pp.x;
			minAdjust.y = oneP.y-pp.y;
			minImpact = oneP;
			minP1 = oneT1;
			minP2 = oneT2;
			minHardness = oneHardness;
			minSound = oneSound;
			minPriority = onePriority;

			if ( onePart%2 == 0 )  return 0;  //?

//?			if ( onePart%2 == 0 )  a = PI/8.0f;  // flan ?
//?			else                   a = PI/2.0f;  // av/ar ?
			if ( onePart == 1 )  a = PI*0.5f;  // avant ?
			if ( onePart == 2 )  a = PI*0.4f;  // flan gauche ?
			if ( onePart == 3 )  a = PI*0.5f;  // arrière ?
			if ( onePart == 4 )  a = PI*0.6f;  // flan droite ?
		}
		else
		{
			t1.x = m_newCorner.p[1].x;
			t1.y = m_newCorner.p[1].z;  // fl
			t2.x = m_newCorner.p[2].x;
			t2.y = m_newCorner.p[2].z;  // rl
			if ( Intersect(t1,t2, minP1,minP2, p) )
			{
				a = RotateAngle(p, minP2, t1);
				if ( a > PI )  a -= PI;
//?				if ( a > PI/2.0f )  a = PI-a;
			}
			else
			{
				a = PI/2.0f;
			}
		}

		len = Length(minAdjust.x, minAdjust.y);
//?		char s[100];
//?		sprintf(s, "CrashCornerRect: i=%d op=%d adj=%.2f;%.2f a=%.2f len=%.2f\n", nbInter, onePart, minAdjust.x,minAdjust.y, a*180.0f/PI, len);
//?		OutputDebugString(s);
		if ( len < 0.1f )  return 0;

		adjust    = minAdjust;
		inter     = minImpact;
		chocAngle = a;
		hardness  = minHardness;
		sound     = minSound;
		priority  = minPriority;
		return 1;
	}

	return 0;
}

// Valide si un ajustement produit une position du véhicule qui
// n'est pas dans l'obstacle.

BOOL CPhysics::CrashValidity(CObject *pObj, FPOINT adjust)
{
	FPOINT	t1,t2, oP1,oP2;
	int		i, j, max;
	BOOL	bNew, bOut;

	max = pObj->RetCrashLineTotal();

	if ( adjust.x > 0.0f )  adjust.x += 0.001f;
	if ( adjust.x < 0.0f )  adjust.x -= 0.001f;
	if ( adjust.y > 0.0f )  adjust.y += 0.001f;
	if ( adjust.y < 0.0f )  adjust.y -= 0.001f;

	for ( i=0 ; i<4 ; i++ )
	{
		t1.x = adjust.x+m_newCorner.p[i].x;
		t1.y = adjust.y+m_newCorner.p[i].z;

		bOut = FALSE;
		pObj->GetCrashLine(0, oP2, bNew);
		for ( j=1 ; j<max ; j++ )
		{
			oP1 = oP2;
			pObj->GetCrashLine(j, oP2, bNew);
			if ( bNew )  continue;

			if ( !IsInside(oP1,oP2, t1) )  bOut = TRUE;
		}
		if ( !bOut )
		{
//?			OutputDebugString("CrashValidity exclure A\n");
			return FALSE;
		}
	}

	for ( j=1 ; j<max ; j++ )
	{
		pObj->GetCrashLine(j, oP1, bNew);
		if ( bNew )  continue;

		bOut = FALSE;
		for ( i=0 ; i<4 ; i++ )
		{
			t1.x = adjust.x+m_newCorner.p[i].x;
			t1.y = adjust.y+m_newCorner.p[i].z;
			t2.x = adjust.x+m_newCorner.p[(i+1)%4].x;
			t2.y = adjust.y+m_newCorner.p[(i+1)%4].z;

			if ( !IsInside(t1,t2, oP1) )  bOut = TRUE;
		}
		if ( !bOut )
		{
//?			OutputDebugString("CrashValidity exclure B\n");
			return FALSE;
		}
	}

	return TRUE;
}

// Gestion de la collision d'un véhicule rectangulaire avec un obstacle
// circulaire.

int CPhysics::CrashCornerCircle(CObject *pObj, const D3DVECTOR &pos, const D3DVECTOR &angle,
								FPOINT &adjust, FPOINT &inter,
								float &chocAngle, float &priority,
								float &hardness, Sound &sound,
								float aTime, float rTime)
{
	Character*	character;
	D3DVECTOR	oPos;
	FPOINT		cc, t1,t2, c, pp;
	float		oRad, iRad, distance, min, a, max;
	int			i, j;

	character = m_object->RetCharacter();

	cc.x = (m_newCorner.p[0].x+m_newCorner.p[1].x+m_newCorner.p[2].x+m_newCorner.p[3].x)/4.0f;
	cc.y = (m_newCorner.p[0].z+m_newCorner.p[1].z+m_newCorner.p[2].z+m_newCorner.p[3].z)/4.0f;
	iRad = Length(cc.x-m_newCorner.p[0].x, cc.y-m_newCorner.p[0].z);

	j = 0;
	while ( pObj->GetCrashSphere(j++, oPos, oRad) )
	{
		if ( pos.y+5.0f < oPos.y-oRad )  continue;
		if ( pos.y+1.0f > oPos.y+oRad )  continue;

		c.x = oPos.x;
		c.y = oPos.z;

		distance = Length(c, cc);
		if ( distance >= iRad+oRad )  continue;  // pas de collision ?

		min = 10000.0f;
		for ( i=0 ; i<4 ; i++ )
		{
			t1.x = m_newCorner.p[i].x;
			t1.y = m_newCorner.p[i].z;
			t2.x = m_newCorner.p[(i+1)%4].x;
			t2.y = m_newCorner.p[(i+1)%4].z;

			pp = Projection(t1,t2, c);
			if ( !IsInSegment(t1,t2, pp) )  continue;

			distance = Length(pp, c);
			if ( IsInside(t1,t2, c) )
			{
				distance += oRad;
				if ( i%2 == 0 )  max = character->crashFront-character->crashBack;
				else             max = character->crashWidth*2.0f;
				if ( distance >= oRad+max )  continue;
				if ( distance >= min )  continue;
				min = distance;
				inter.x = pp.x+(c.x-pp.x)*(oRad*2.0f/min);
				inter.y = pp.y+(c.y-pp.y)*(oRad*2.0f/min);
				adjust.x = inter.x-pp.x;
				adjust.y = inter.y-pp.y;
				if ( i%2 == 0 )  a = PI/2.0f;  // collision frontale
				else             a = 0.0f;     // collision rasante
			}
			else
			{
				if ( distance >= oRad )  continue;
				if ( distance >= min )  continue;
				min = distance;
				inter.x = c.x+(pp.x-c.x)*(oRad/min);
				inter.y = c.y+(pp.y-c.y)*(oRad/min);
				adjust.x = inter.x-pp.x;
				adjust.y = inter.y-pp.y;
				if ( i%2 == 0 )  a = PI/2.0f;  // collision frontale
				else             a = 0.0f;     // collision rasante
			}
		}

		for ( i=0 ; i<4 ; i++ )
		{
			t1.x = m_newCorner.p[i].x;
			t1.y = m_newCorner.p[i].z;

			distance = Length(t1, c);
			if ( distance >= oRad )  continue;
			if ( distance >= min )  continue;
			min = distance;
			inter.x = c.x+(t1.x-c.x)*(oRad/min);
			inter.y = c.y+(t1.y-c.y)*(oRad/min);
			adjust.x = inter.x-t1.x;
			adjust.y = inter.y-t1.y;

			t2.x = m_newCorner.p[i^1].x;
			t2.y = m_newCorner.p[i^1].z;
			a = RotateAngle(t1, t2, c);
			if ( a > PI )  a -= PI;
		}

		if ( min < 10000.0f )
		{
//?			char s[100];
//?			sprintf(s, "CrashCornerCircle: a=%.2f\n", a*180.0f/PI);
//?			OutputDebugString(s);

			chocAngle = a;
			priority = 1.0f;
			hardness = pObj->RetCrashSphereHardness(j-1);
			sound = pObj->RetCrashSphereSound(j-1);
			return 1;
		}
	}

	return 0;
}

// Effectue la collision.

int CPhysics::CrashCornerDo(CObject *pObj, D3DVECTOR &pos,
							FPOINT adjust, FPOINT inter,
							float angle, float priority,
							float hardness, Sound sound,
							float aTime, float rTime)
{
	D3DVECTOR	impact, ppos, speed, iSpeed;
	FPOINT		dim;
	ObjectType	iType, oType;
	float		force, volume, mass, iMass, oMass, factor, amplitude, iAngle;
	int			colType;

	iType = m_object->RetType();
	oType = pObj->RetType();

	impact.x = inter.x;
	impact.z = inter.y;
	impact.y = pos.y+1.0f;

	// Ajuste la position.
	pos.x += adjust.x;
	pos.z += adjust.y;

	force = Abs(m_linMotion.realSpeed.x);
	iSpeed = m_linMotion.currentSpeed;
	iAngle = angle;

#if 0
	// Vz' = -Vx*sin(a)*cos(a)*2
	// Vx' = -Vz'*tan(PI/2-2a)
	if ( Abs(angle-PI/2.0f) < 0.01f )
	{
		m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.x;
		m_linMotion.currentSpeed.z = 0.0f;
	}
	else
	{
		m_linMotion.currentSpeed.z = -m_linMotion.currentSpeed.x*sinf(angle)*cosf(angle)*2.0f;
		m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.z*tanf(PI/2.0f-2.0f*angle);
	}

	// Plus le choc est rasant (angle -> 0 ou 180), moins la vitesse
	// est amortie (par exemple pour glisser le long d'une barrière).
	m_linMotion.currentSpeed.x *= (1.0f-sinf(angle)*0.8f);
	m_linMotion.currentSpeed.z *= (1.0f-sinf(angle)*0.8f)*0.7f;

	m_linMotion.realSpeed = m_linMotion.currentSpeed;
#endif
#if 0
//?	m_linMotion.currentSpeed.x = 0.0f;
//?	m_linMotion.realSpeed = m_linMotion.currentSpeed;
	m_linMotion.currentSpeed.x *= 0.8f;
	m_linMotion.realSpeed = m_linMotion.currentSpeed;
#endif
#if 0
	if ( angle < PI/4.0f )  // choc rasant ?
	{
		m_linMotion.currentSpeed.x *= cosf(angle);
	}
	else	// choc frontal ?
	{
		m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.x*0.5f;
	}

	m_linMotion.realSpeed = m_linMotion.currentSpeed;
#endif
#if 1
	if ( priority == 0.0f )
	{
		force = 0.0f;
	}
	else
	{
		if ( angle < PI*0.25f || angle > PI*0.75f )  // choc rasant ?
		{
			if ( angle > PI/2.0f )  angle = PI-angle;
			m_linMotion.currentSpeed.x *= cosf(angle);
			m_linMotion.currentSpeed.z = 0.0f;
			force *= powf(angle/(PI*0.25f), 2.0f);
		}
		else	// choc frontal ?
		{
			// Vz' = -Vx*sin(a)*cos(a)*2
			// Vx' = -Vz'*tan(PI/2-2a)
			if ( Abs(angle-PI/2.0f) < 0.01f )
			{
				m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.x;
				m_linMotion.currentSpeed.z = 0.0f;
			}
			else
			{
				m_linMotion.currentSpeed.z = -m_linMotion.currentSpeed.x*sinf(angle)*cosf(angle)*2.0f;
				m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.z*tanf(PI/2.0f-2.0f*angle);
			}
//?			m_linMotion.currentSpeed.x *= 0.5f;
//?			m_linMotion.currentSpeed.z *= 0.3f;
			m_linMotion.currentSpeed.x *= 0.2f;
			m_linMotion.currentSpeed.z *= 0.1f;
			m_centriSpeed *= 0.1f;
		}
	}

	m_linMotion.realSpeed = m_linMotion.currentSpeed;
	m_graviGlu = 1.0f;
#endif

	if ( iType == OBJECT_TRAX )  force *= 10.0f;
	force *= hardness*2.0f;
	ExploOther(iType, pObj, oType, force, hardness);
	colType = ExploHimself(iType, oType, force, hardness, impact);
	if ( colType == 2 )  return 2;  // détruit ?

	volume = Norm(Abs(force/40.0f));
	if ( sound != SOUND_CLICK &&
		 iType != OBJECT_HUMAN &&
		 oType != OBJECT_BOT3  &&
		 oType != OBJECT_EVIL1 &&
		 oType != OBJECT_EVIL3 &&
		 aTime-m_lastSoundCrash >= 0.1f )
	{
		m_lastSoundCrash = aTime;
		m_sound->Play(sound, impact, volume);

		if ( sound != SOUND_BOUMm &&
			 oType != OBJECT_CONE )
		{
			m_sound->Play(SOUND_BOUMm, impact, volume*0.8f);
		}
	}

	if ( colType == 3 )  // passe au travers ?
	{
		iMass = m_object->RetCharacter()->mass;
		oMass = pObj->RetCharacter()->mass;
		if ( iMass == 0.0f || oMass == 0.0f )
		{
			factor = 0.6f;
		}
		else
		{
			factor = Norm(0.25f+(iMass/oMass)*0.25f);
		}
		m_linMotion.currentSpeed = iSpeed*factor;  // on est freiné
		m_linMotion.realSpeed = m_linMotion.currentSpeed;
		return 1;  // passe outre
	}
	
	if ( angle < PI*0.25f || angle > PI*0.75f )  // choc rasant ?
	{
//?		m_glideAmplitude = Norm(Abs(force/5.0f))*0.6f;
		m_glideAmplitude = Norm(Abs(force/3.0f));
		if ( m_glideAmplitude < 0.8f )  m_glideAmplitude = 0.8f;
		m_glideImpact = impact;

		ppos = impact;
		ppos.y += Rand()*3.0f;
		speed.x = (Rand()-0.5f)*5.0f;
		speed.z = (Rand()-0.5f)*5.0f;
		speed.y = 5.0f+Rand()*5.0f;
		mass    = 20.0f+Rand()*20.0f;
		dim.x = 0.4f+Rand()*0.4f;
		dim.y = dim.x;
		m_particule->CreateParticule(ppos, speed, dim, PARTIGLINT, 0.5f, mass);

		amplitude = Abs(iSpeed.x/m_linMotion.advanceSpeed.x)*5.0f;
		if ( iAngle < PI*0.25f )
		{
			amplitude *= iAngle/(PI*0.25f);  // tire à gauche contre l'obstacle
		}
		else
		{
			amplitude *= -(PI-iAngle)/(PI*0.25f);  // tire à droite contre l'obstacle
		}
		FFBForce(amplitude*0.5f);
	}
	else
	{
		amplitude = Abs(iSpeed.x/m_linMotion.advanceSpeed.x)*5.0f;
		FFBCrash(amplitude, 0.3f, 1.0f);
	}

	return 1;
}

// Calcule les coins courants du véhicule.

void CPhysics::UpdateCorner()
{
	Character*	character;
	D3DMATRIX*	mat;
	ObjectType	type;

	type = m_object->RetType();
	if ( type != OBJECT_CAR  &&
		 type != OBJECT_TRAX )  return;

	character = m_object->RetCharacter();
	mat = m_object->RetWorldMatrix(0);
	m_curCorner.p[0] = Transform(*mat, D3DVECTOR(character->crashFront, 0.0f, -character->crashWidth));  // fr
	m_curCorner.p[1] = Transform(*mat, D3DVECTOR(character->crashFront, 0.0f,  character->crashWidth));  // fl
	m_curCorner.p[2] = Transform(*mat, D3DVECTOR(character->crashBack,  0.0f,  character->crashWidth));  // rl
	m_curCorner.p[3] = Transform(*mat, D3DVECTOR(character->crashBack,  0.0f, -character->crashWidth));  // rr

	m_curCorner.min.x = Min(m_curCorner.p[0].x, m_curCorner.p[1].x, m_curCorner.p[2].x, m_curCorner.p[3].x);
	m_curCorner.min.y = Min(m_curCorner.p[0].z, m_curCorner.p[1].z, m_curCorner.p[2].z, m_curCorner.p[3].z);

	m_curCorner.max.x = Max(m_curCorner.p[0].x, m_curCorner.p[1].x, m_curCorner.p[2].x, m_curCorner.p[3].x);
	m_curCorner.max.y = Max(m_curCorner.p[0].z, m_curCorner.p[1].z, m_curCorner.p[2].z, m_curCorner.p[3].z);
}

// Calcule les nouveaux coins du véhicule.

void CPhysics::UpdateCorner(const D3DVECTOR &pos, const D3DVECTOR &angle)
{
	Character*	character;
	D3DMATRIX	matRotate;
	ObjectType	type;

	type = m_object->RetType();
	if ( type != OBJECT_CAR  &&
		 type != OBJECT_TRAX )  return;

	character = m_object->RetCharacter();
	MatRotateZXY(matRotate, angle);  // calcule la nouvelle matrice de rotation
	m_newCorner.p[0] = Transform(matRotate, D3DVECTOR(character->crashFront, 0.0f, -character->crashWidth))+pos;  // fr
	m_newCorner.p[1] = Transform(matRotate, D3DVECTOR(character->crashFront, 0.0f,  character->crashWidth))+pos;  // fl
	m_newCorner.p[2] = Transform(matRotate, D3DVECTOR(character->crashBack,  0.0f,  character->crashWidth))+pos;  // rl
	m_newCorner.p[3] = Transform(matRotate, D3DVECTOR(character->crashBack,  0.0f, -character->crashWidth))+pos;  // rr

	m_newCorner.min.x = Min(m_newCorner.p[0].x, m_newCorner.p[1].x, m_newCorner.p[2].x, m_newCorner.p[3].x);
	m_newCorner.min.y = Min(m_newCorner.p[0].z, m_newCorner.p[1].z, m_newCorner.p[2].z, m_newCorner.p[3].z);

	m_newCorner.max.x = Max(m_newCorner.p[0].x, m_newCorner.p[1].x, m_newCorner.p[2].x, m_newCorner.p[3].x);
	m_newCorner.max.y = Max(m_newCorner.p[0].z, m_newCorner.p[1].z, m_newCorner.p[2].z, m_newCorner.p[3].z);
}

// Met à jour le grincement lorsque la voiture glisse le long d'un obstacle.

void CPhysics::UpdateGlideSound(float rTime)
{
	if ( m_glideAmplitude > 0.0f )
	{
		m_glideVolume = m_glideAmplitude;

		if ( m_soundChannelGlide == -1 )
		{
			m_soundChannelGlide = m_sound->Play(SOUND_WHEELb, m_glideImpact, m_glideVolume, 1.2f, TRUE);
			m_sound->AddEnvelope(m_soundChannelGlide, m_glideVolume, 1.2f, 1.0f, SOPER_LOOP);
		}
		if ( m_soundChannelGlide != -1 )
		{
			m_sound->Position(m_soundChannelGlide, m_glideImpact);
			m_sound->Amplitude(m_soundChannelGlide, m_glideVolume);
			m_sound->Frequency(m_soundChannelGlide, 1.0f);
		}
		m_glideTime = 0.2f;
	}
	else
	{
		m_glideTime -= rTime;
		if ( m_glideTime < 0.0f )  m_glideTime = 0.0f;

		if ( m_glideTime == 0.0f )
		{
			if ( m_soundChannelGlide != -1 )
			{
				m_sound->FlushEnvelope(m_soundChannelGlide);
				m_sound->AddEnvelope(m_soundChannelGlide, 0.0f, 1.2f, 0.1f, SOPER_STOP);
				m_soundChannelGlide = -1;
			}
		}
		else
		{
			if ( m_soundChannelGlide != -1 )
			{
				m_glideVolume -= rTime/0.4f;
				if ( m_glideVolume < 0.0f )  m_glideVolume = 0.0f;
				m_sound->Position(m_soundChannelGlide, m_glideImpact);
				m_sound->Amplitude(m_soundChannelGlide, m_glideVolume);
				m_sound->Frequency(m_soundChannelGlide, 1.0f);
			}
		}
	}
}


// Génère qq particules suite à une chute.
// crash: 0=super soft, 1=big crash

void CPhysics::CrashParticule(float crash, BOOL bCrash)
{
	D3DVECTOR	pos, ppos, speed;
	FPOINT		dim;
	float		len;
	int			i, max;

	if ( crash < 0.2f || m_bWater )  return;

	pos = m_object->RetPosition(0);

	if ( bCrash )
	{
		m_camera->StartEffect(CE_CRASH, pos, crash);
		FFBCrash(crash, 0.4f, 5.0f);
//?char s[100];
//?sprintf(s, "crash=%.2f\n", crash);
//?OutputDebugString(s);
	}

//?	max = (int)(crash*50.0f);
	max = (int)(crash*10.0f*m_engine->RetParticuleDensity());

	for ( i=0 ; i<max ; i++ )
	{
		ppos.x = pos.x + (Rand()-0.5f)*10.0f*crash;
		ppos.z = pos.z + (Rand()-0.5f)*10.0f*crash;
		ppos.y = pos.y + Rand()*4.0f;
		len = 1.0f-(Length(ppos, pos)/(15.0f+5.0f));
		if ( len <= 0.0f )  continue;
		speed.x = (ppos.x-pos.x)*0.1f;
		speed.z = (ppos.z-pos.z)*0.1f;
		speed.y = -2.0f;
		dim.x = 2.0f+crash*5.0f*len;
		dim.y = dim.x;
		m_particule->CreateParticule(ppos, speed, dim, PARTICRASH, 2.0f);
	}
}

// Génère qq particules suite à un dérappage.

void CPhysics::SlideParticule(float aTime, float rTime,
							  float fDusty, float fSound)
{
	Sound		sound;
	WheelType	wType;

	wType = m_motion->RetWheelType();

	if ( wType != WT_BURN &&
		 m_lastCrashParticule+m_engine->ParticuleAdapt(0.05f) <= aTime )
	{
		m_lastCrashParticule = aTime;
		CrashParticule(fDusty*1.5f, FALSE);
	}

	if ( fSound <= 0.4f )
	{
		if ( m_soundChannelSlide != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelSlide);
			m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.1f, SOPER_STOP);
			m_soundChannelSlide = -1;
		}
		return;
	}

	if ( m_soundChannelSlide == -1 && !m_bWater )
	{
		sound = SOUND_WHEEL;
		if ( wType == WT_BURN )  sound = SOUND_WHEELb;
		m_soundChannelSlide = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
		m_sound->AddEnvelope(m_soundChannelSlide, 0.7f, 1.0f, 0.1f, SOPER_CONTINUE);
		m_sound->AddEnvelope(m_soundChannelSlide, 0.7f, 1.0f, 1.0f, SOPER_LOOP);
	}
	if ( m_soundChannelSlide != -1 )
	{
		if ( wType == WT_BURN )  fSound *= 2.0f;
		m_sound->Position(m_soundChannelSlide, m_object->RetPosition(0));
		m_sound->Amplitude(m_soundChannelSlide, Norm(fSound*1.0f));
		m_sound->Frequency(m_soundChannelSlide, 1.0f+fSound*0.3f);
	}
}

// Génère qq particules à l'emplacement des roues, soit lors
// d'une force accélération, soit lors d'un freinage.

void CPhysics::WheelParticule(float aTime, float rTime, float force, BOOL bBrake)
{
	Sound			sound;
	Character*		character;
	D3DMATRIX*		mat;
	D3DVECTOR		pos, speed, goal1, goal2, wheel1, wheel2;
	FPOINT			dim;
	ParticuleType	parti;
	WheelType		wType;
	float			delay, factor, volume, mass, dist1, dist2, step;
	BOOL			bSilent;

	character = m_object->RetCharacter();
	mat = m_object->RetWorldMatrix(0);
	wType = m_motion->RetWheelType();

	if ( force <= 0.01f )
	{
		if ( m_soundChannelBoost != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelBoost);
			m_sound->AddEnvelope(m_soundChannelBoost, 0.0f, 1.0f, 0.3f, SOPER_STOP);
			m_soundChannelBoost = -1;
		}
		if ( m_soundChannelBrake != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelBrake);
			m_sound->AddEnvelope(m_soundChannelBrake, 0.0f, 1.0f, 0.3f, SOPER_STOP);
			m_soundChannelBrake = -1;
		}
		if ( wType != WT_BURN )
		{
			m_bWheelParticuleBrake = FALSE;
			return;
		}
		bSilent = TRUE;  // seulement pour dessiner les traces au sol
	}
	else
	{
		bSilent = FALSE;
	}

	if ( bBrake )
	{
		if ( m_soundChannelBoost != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelBoost);
			m_sound->AddEnvelope(m_soundChannelBoost, 0.0f, 1.0f, 0.3f, SOPER_STOP);
			m_soundChannelBoost = -1;
		}
	}
	else
	{
		if ( m_soundChannelBrake != -1 )
		{
			m_sound->FlushEnvelope(m_soundChannelBrake);
			m_sound->AddEnvelope(m_soundChannelBrake, 0.0f, 1.0f, 0.3f, SOPER_STOP);
			m_soundChannelBrake = -1;
		}
	}

	if ( m_terrainHard >= 0.5f )  // sol dur ?
	{
		sound = SOUND_WHEEL;
		if ( wType == WT_BURN )  sound = SOUND_WHEELb;
		volume = 0.5f+Norm(force)*0.5f;
		if ( m_bHandbrake )  volume = Norm(volume*2.0f);
	}
	else	// sol mou ?
	{
		sound = SOUND_WHEELg;
		if ( bBrake )
		{
			force = Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x;
			volume = Norm(force*4.0f);
			if ( m_bHandbrake )  volume = Norm(volume*2.0f);
		}
		else
		{
			volume = 0.0f;
		}
	}

	if ( m_soundChannelBoost == -1 && !bBrake && !m_bWater && !bSilent )
	{
		m_soundChannelBoost = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 1.5f, TRUE);
		m_sound->AddEnvelope(m_soundChannelBoost, volume, 1.5f, 0.3f, SOPER_CONTINUE);
		m_sound->AddEnvelope(m_soundChannelBoost, volume, 1.5f, 4.0f, SOPER_LOOP);
	}
	if ( m_soundChannelBoost != -1 )
	{
		if ( wType == WT_BURN )
		{
			volume *= 2.0f;
		}
		else
		{
			volume *= Abs(m_cirMotion.realSpeed.y)/m_cirMotion.advanceSpeed.y*2.0f;
		}
		m_sound->Position(m_soundChannelBoost, m_object->RetPosition(0));
		m_sound->Amplitude(m_soundChannelBoost, volume);
		m_sound->Frequency(m_soundChannelBoost, 1.0f);
	}

	if ( m_soundChannelBrake == -1 && bBrake && !m_bWater && !bSilent )
	{
		m_soundChannelBrake = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
		if ( sound == SOUND_WHEELg )
		{
			m_sound->AddEnvelope(m_soundChannelBrake, volume, 1.0f, 0.3f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_soundChannelBrake, volume, 1.0f, 4.0f, SOPER_LOOP);
		}
		else
		{
			m_sound->AddEnvelope(m_soundChannelBrake, volume, 1.0f, 0.3f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_soundChannelBrake, volume, 0.5f, 4.0f, SOPER_LOOP);
		}
	}
	if ( m_soundChannelBrake != -1 )
	{
		m_sound->Position(m_soundChannelBrake, m_object->RetPosition(0));
		m_sound->Amplitude(m_soundChannelBrake, volume);
		m_sound->Frequency(m_soundChannelBrake, 1.0f);
	}

	// Dessine les traces de frein sur le sol.
	if ( bBrake || force > 0.5f || wType == WT_BURN )
	{
		if ( wType == WT_BURN )
		{
			parti = PARTITRACE7;
			step = 2.0f;
		}
		else
		{
			if ( m_terrainHard < 0.5f )  // sol mou ?
			{
				parti = PARTITRACE6;
			}
			else	// sol dur (route) ?
			{
				if ( bBrake )  parti = PARTITRACE1;
				else           parti = PARTITRACE2;
			}
			step = 8.0f;
		}

		goal1.x = character->wheelBackPos.x+step/2.0f;
		goal1.y = 0.0f;
		goal1.z = character->wheelBackPos.z;
		goal1 = Transform(*mat, goal1);

		goal2.x = character->wheelBackPos.x+step/2.0f;
		goal2.y = 0.0f;
		goal2.z = character->wheelBackPos.z+character->wheelBackWidth;
		goal2 = Transform(*mat, goal2);

		if ( !m_bWheelParticuleBrake )
		{
			m_wheelParticulePos[0] = goal1;
			m_wheelParticulePos[1] = goal2;
		}

		while ( TRUE )
		{
			dist1 = Length(m_wheelParticulePos[0], goal1);
			if ( dist1 < step )  break;
			dist2 = Length(m_wheelParticulePos[1], goal2);
			wheel1 = SegmentDist(m_wheelParticulePos[0], goal1, step);
			wheel2 = SegmentDist(m_wheelParticulePos[1], goal2, step*dist2/dist1);
			if ( m_linMotion.realSpeed.x >= 0.0f )
			{
				m_particule->CreateWheelTrace(m_wheelParticulePos[0], m_wheelParticulePos[1], wheel1, wheel2, parti);
			}
			else
			{
				m_particule->CreateWheelTrace(m_wheelParticulePos[1], m_wheelParticulePos[0], wheel2, wheel1, parti);
			}
			m_wheelParticulePos[0] = wheel1;
			m_wheelParticulePos[1] = wheel2;
		}

		goal1.x = character->wheelBackPos.x+step/2.0f;
		goal1.y = 0.0f;
		goal1.z = -character->wheelBackPos.z;
		goal1 = Transform(*mat, goal1);

		goal2.x = character->wheelBackPos.x+step/2.0f;
		goal2.y = 0.0f;
		goal2.z = -(character->wheelBackPos.z+character->wheelBackWidth);
		goal2 = Transform(*mat, goal2);

		if ( !m_bWheelParticuleBrake )
		{
			m_wheelParticulePos[2] = goal1;
			m_wheelParticulePos[3] = goal2;
		}

		while ( TRUE )
		{
			dist1 = Length(m_wheelParticulePos[2], goal1);
			if ( dist1 < step )  break;
			dist2 = Length(m_wheelParticulePos[3], goal2);
			wheel1 = SegmentDist(m_wheelParticulePos[2], goal1, step);
			wheel2 = SegmentDist(m_wheelParticulePos[3], goal2, step*dist2/dist1);
			if ( m_linMotion.realSpeed.x >= 0.0f )
			{
				m_particule->CreateWheelTrace(m_wheelParticulePos[3], m_wheelParticulePos[2], wheel2, wheel1, parti);
			}
			else
			{
				m_particule->CreateWheelTrace(m_wheelParticulePos[2], m_wheelParticulePos[3], wheel1, wheel2, parti);
			}
			m_wheelParticulePos[2] = wheel1;
			m_wheelParticulePos[3] = wheel2;
		}

		m_bWheelParticuleBrake = TRUE;
	}
	else
	{
		m_bWheelParticuleBrake = FALSE;
	}

	if ( m_lastWheelParticule+m_engine->ParticuleAdapt(0.05f) > aTime )  return;
	m_lastWheelParticule = aTime;

	if ( wType == WT_BURN )
	{
		if ( force == 0.0f )  return;
		force += 1.0f;
	}

	if ( m_terrainHard == 0.0f )  // neige ?
	{
		parti  = PARTICRASH;
		delay  = 2.0f;
		factor = 1.0f;
		speed  = D3DVECTOR(0.0f, 0.0f, 0.0f);
		mass   = 0.0f;
	}
	else if ( m_terrainHard < 0.6f )  // sol mou ?
	{
		if ( bBrake )  parti = PARTIDUST1;
		else           parti = PARTIDUST2;
		delay  = 2.0f;
		factor = 2.0f;
		speed  = D3DVECTOR(0.0f, 0.0f, 0.0f);
		mass   = 0.0f;
	}
	else	// sol dur ?
	{
		parti  = PARTIWHEEL;
		delay  = 5.0f;
		factor = 1.0f;
		speed  = D3DVECTOR(0.0f, 0.0f, 0.0f);
		mass   = 0.0f;
	}
	if ( wType == WT_BURN )  // roules sur les jantes ?
	{
		parti   = PARTIGLINT;  // étincelles
		delay   = 0.5f;
		factor  = 0.2f;
		speed.x = (Rand()-0.5f)*5.0f;
		speed.z = (Rand()-0.5f)*5.0f;
		speed.y = 5.0f+Rand()*5.0f;
		mass    = 20.0f+Rand()*20.0f;
	}
	if ( m_bWater )
	{
		parti   = PARTIBUBBLE;
		delay   = 2.0f;
		factor  = 0.1f;
		speed.x = (Rand()-0.5f)*5.0f;
		speed.z = (Rand()-0.5f)*5.0f;
		speed.y = 5.0f+Rand()*5.0f;
		mass    = 0.0f;
	}

	if ( bBrake )
	{
		pos.x = character->wheelFrontPos.x+(Rand()-0.5f)*2.0f*character->wheelFrontDim;
		pos.y = 0.0f;
		pos.z = character->wheelFrontPos.z+Rand()*character->wheelFrontDim;
		pos = Transform(*mat, pos);
		dim.x = (0.5f+force*0.8f+Rand()*1.0f)*factor;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, parti, delay, mass);

		pos.x = character->wheelFrontPos.x+(Rand()-0.5f)*2.0f*character->wheelFrontDim;
		pos.y = 0.0f;
		pos.z = -character->wheelFrontPos.z-Rand()*character->wheelFrontDim;
		pos = Transform(*mat, pos);
		dim.x = (0.5f+force*0.8f+Rand()*1.0f)*factor;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, parti, delay, mass);
	}

	pos.x = character->wheelBackPos.x+(Rand()-0.5f)*2.0f*character->wheelBackDim;
	pos.y = 0.0f;
	pos.z = character->wheelBackPos.z+Rand()*character->wheelBackDim;
	pos = Transform(*mat, pos);
	dim.x = (0.5f+force*0.8f+Rand()*1.0f)*factor;
	dim.y = dim.x;
	m_particule->CreateParticule(pos, speed, dim, parti, delay, mass);

	pos.x = character->wheelBackPos.x+(Rand()-0.5f)*2.0f*character->wheelBackDim;
	pos.y = 0.0f;
	pos.z = -character->wheelBackPos.z-Rand()*character->wheelBackDim;
	pos = Transform(*mat, pos);
	dim.x = (0.5f+force*0.8f+Rand()*1.0f)*factor;
	dim.y = dim.x;
	m_particule->CreateParticule(pos, speed, dim, parti, delay, mass);

#if 0
	// Gommes qui brûlent tant le démarrage est rapide.
	if ( !bBrake               &&  // dragster ?
		 m_terrainHard >= 0.6f &&  // sol dur ?
		 wType != WT_BURN      &&  // roues pas cramées ?
		 !m_bWater             )   // pas sous l'eau ?
	{
		pos.x = character->wheelBackPos.x+(Rand()-0.5f)*2.0f*character->wheelBackDim;
		pos.y = 0.0f;
		pos.z = character->wheelBackPos.z+Rand()*character->wheelBackDim;
		pos = Transform(*mat, pos);
		speed = Transform(*mat, D3DVECTOR(m_linMotion.realSpeed.x/2.0f, 0.0f, 0.0f));
		speed.y += 4.0f+Rand()*4.0f;
		speed -= m_object->RetPosition(0);
		dim.x = (0.5f+force*0.8f+Rand()*1.0f)*factor;
		dim.y = dim.x;
		delay = 1.0f+Rand()*2.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTIWHEEL, delay, 0.0f);

		pos.x = character->wheelBackPos.x+(Rand()-0.5f)*2.0f*character->wheelBackDim;
		pos.y = 0.0f;
		pos.z = -character->wheelBackPos.z-Rand()*character->wheelBackDim;
		pos = Transform(*mat, pos);
		speed = Transform(*mat, D3DVECTOR(m_linMotion.realSpeed.x/2.0f, 0.0f, 0.0f));
		speed.y += 4.0f+Rand()*4.0f;
		speed -= m_object->RetPosition(0);
		dim.x = (0.5f+force*0.8f+Rand()*1.0f)*factor;
		dim.y = dim.x;
		delay = 1.0f+Rand()*2.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTIWHEEL, delay, 0.0f);
	}
#endif

	// Ejection de petites pierres.
	if ( !bBrake              &&  // dragster ?
		 m_terrainHard < 0.6f &&  // sol mou ?
		 wType != WT_BURN     )   // roues pas cramées ?
	{
		pos.x = character->wheelBackPos.x;
		pos.y = 0.0f;
		pos.z = character->wheelBackPos.z+Rand()*character->wheelBackDim;
		pos = Transform(*mat, pos);
		speed.x = (Rand()-0.5f)*15.0f;
		speed.z = (Rand()-0.5f)*15.0f;
		speed.y = 12.0f+Rand()*12.0f;
		dim.x = (0.5f+force*0.8f+Rand()*1.0f)*0.05f;
		dim.y = dim.x;
		mass = 20.0f+Rand()*20.0f;
		parti = (ParticuleType)(PARTISTONE1+rand()%4);
		if ( m_bWater )  parti = PARTIBUBBLE;
		m_particule->CreateParticule(pos, speed, dim, parti, 3.0f, mass);

		pos.x = character->wheelBackPos.x;
		pos.y = 0.0f;
		pos.z = -character->wheelBackPos.z-Rand()*character->wheelBackDim;
		pos = Transform(*mat, pos);
		speed.x = (Rand()-0.5f)*15.0f;
		speed.z = (Rand()-0.5f)*15.0f;
		speed.y = 12.0f+Rand()*12.0f;
		dim.x = (0.5f+force*0.8f+Rand()*1.0f)*0.05f;
		dim.y = dim.x;
		mass = 20.0f+Rand()*20.0f;
		parti = (ParticuleType)(PARTISTONE1+rand()%4);
		if ( m_bWater )  parti = PARTIBUBBLE;
		m_particule->CreateParticule(pos, speed, dim, parti, 3.0f, mass);
	}
}

// Génère qq particules selon la suspension.

void CPhysics::SuspParticule(float aTime, float rTime)
{
	D3DMATRIX*	mat;
	Character*	character;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		factor, speedx, duration;
	int			i;

return;
	if ( m_floorHeight > 0.0f )  return;
	if ( m_terrainHard < 0.5f )  return;

	character = m_object->RetCharacter();

	factor = m_linMotion.realSpeed.z;
	factor = Abs(factor/20.0f);
	if ( factor <= 0.0f )  return;

	speedx = Abs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x);
	speedx = (speedx-0.5f)/0.5f;
//?	if ( speedx <= 0.0f )  return;
//?	factor *= speedx;

	if ( m_lastSuspParticule+m_engine->ParticuleAdapt(0.05f) <= aTime )
	{
		m_lastSuspParticule = aTime;

		mat = m_object->RetWorldMatrix(0);
		for ( i=0 ; i<(int)(factor*10.0f) ; i++ )
		{
			pos.x = character->wheelBackPos.x*Rand();
			pos.y = 0.0f;
			pos.z = character->wheelBackPos.z*(Rand()-0.5f)*1.5f;
			pos = Transform(*mat, pos);
			speed.x = -m_linMotion.realSpeed.x/5.0f;
			speed.y =  m_linMotion.realSpeed.x/20.0f;
			speed.z = (Rand()-0.5f)*6.0f;
			speed = Transform(*mat, speed);
			speed -= m_object->RetPosition(0);
			dim.x = 0.2f+Rand()*0.2f;
			dim.y = dim.x;
			duration = 1.0f+Rand()*1.0f;
//?			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, duration, 20.0f);
			m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
									 2.0f, 20.0f, 0.5f, 1.0f);
		}
	}
}

// Calcule position et vitesse pour les gaz d'échappement.

void CPhysics::GazCompute(int model, D3DMATRIX *mat, int i, float dir,
						  float factor,
						  D3DVECTOR &pos, D3DVECTOR &speed)
{
	if ( model == 1 )  // tijuana taxi ?
	{
		pos = D3DVECTOR(2.7f+0.5f*i, 3.1f, 3.1f*dir);
		speed = pos;
		speed.x += ((Rand()-0.5f)*2.0f)*factor;
		speed.y += (1.0f+Rand()*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 2 )  // ford 32?
	{
		pos = D3DVECTOR(1.8f+0.5f*i, 2.2f, 3.3f*dir);
		speed = pos;
		speed.x += ((Rand()-0.5f)*2.0f)*factor;
		speed.y += (1.0f+Rand()*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 3 )  // pickup ?
	{
		if ( i == 0 )  pos = D3DVECTOR(2.10f, 1.2f, 3.3f*dir*1.1f);
		if ( i == 1 )  pos = D3DVECTOR(2.65f, 1.4f, 3.3f*dir*1.1f);
		if ( i == 2 )  pos = D3DVECTOR(3.20f, 1.6f, 3.3f*dir*1.1f);
		speed = pos;
		speed.x += ((Rand()-0.5f)*2.0f)*factor;
		speed.y += ((Rand()-0.5f)*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 4 )  // firecraker ?
	{
		pos = D3DVECTOR(2.6f, 2.0f, 2.7f*dir);
		speed = pos;
		speed.x += ((Rand()-0.8f)*2.0f)*factor;
		speed.y += ((Rand()-0.2f)*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 5 )  // hooligan ?
	{
		pos = D3DVECTOR(2.9f, 1.5f, 3.4f*dir);
		speed = pos;
		speed.x += ((Rand()-0.8f)*2.0f)*factor;
		speed.y += ((Rand()-0.8f)*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 6 )  // chevy ?
	{
		pos = D3DVECTOR(-1.8f, 1.0f, 3.4f*dir);
		speed = pos;
		speed.x += ((Rand()-0.8f)*2.0f)*factor;
		speed.y += ((Rand()-0.5f)*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 7 )  // reo ?
	{
		pos = D3DVECTOR(2.2f, 2.3f, 3.0f*dir);
		speed = pos;
		speed.x += ((Rand()-0.8f)*2.0f)*factor;
		speed.y += ((Rand()-0.2f)*2.0f)*factor;
		speed.z += (1.0f+Rand()*2.0f)*factor*dir;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}

	if ( model == 8 )  // torpedo ?
	{
		pos = D3DVECTOR(-6.0f, 2.85f, -2.15f)*1.1f;
		speed = pos;
		speed.x += -(1.0f+Rand()*4.0f)*factor;
		speed.y += ((Rand()-0.5f)*2.0f)*factor;
		speed.z += ((Rand()-0.5f)*2.0f)*factor;
		pos   = Transform(*mat, pos);
		speed = Transform(*mat, speed)-pos;
	}
}

// Génère qq particules de gaz d'échappement.

void CPhysics::MotorParticule(float aTime, float rTime)
{
	ParticuleType	parti;
	D3DMATRIX*		mat;
	D3DVECTOR		pos, speed;
	FPOINT			dim;
	ObjectType		type;
	FPOINT			c, p;
	float			h, a, delay, level, factor, boost, duration, size;
	int				i, nb, model;

	if ( m_type == TYPE_MASS )  return;

	type  = m_object->RetType();
	model = m_object->RetModel();

	if ( type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_WALKER  ||
		 type == OBJECT_CRAZY   ||
		 type == OBJECT_GUIDE   ||
		 type == OBJECT_EVIL1   ||
		 type == OBJECT_EVIL2   ||
		 type == OBJECT_EVIL3   ||
		 type == OBJECT_EVIL4   ||
		 type == OBJECT_EVIL5   )  return;

	if ( type == OBJECT_HUMAN )  delay = 3.0f;
	else                         delay = 8.0f;
	if ( m_bSwim && m_timeUnderWater < delay )  // bulles en entrant dans l'eau ?
	{
		if ( aTime-m_lastUnderParticule >= m_engine->ParticuleAdapt(0.05f) )
		{
			m_lastUnderParticule = aTime;

			nb = (int)(20.0f-(20.0f/delay)*m_timeUnderWater);
			for ( i=0 ; i<nb ; i++ )
			{
				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*4.0f;
				pos.y += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				speed.y = (Rand()-0.5f)*8.0f+8.0f;
				speed.x = (Rand()-0.5f)*0.2f;
				speed.z = (Rand()-0.5f)*0.2f;
				dim.x = 0.06f+Rand()*0.10f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f);
			}
		}
	}

	level = m_water->RetLevel();
	pos = m_object->RetPosition(0);
	if ( type == OBJECT_HUMAN )  pos.y -= 2.0f;
	if ( pos.y < level )  // sous l'eau ?
	{
		m_absorbWater += rTime*(1.0f/2.0f);  // se mouille
		if ( m_absorbWater > 1.0f )  m_absorbWater = 1.0f;
	}
	else	// hors de l'eau ?
	{
		m_absorbWater -= rTime*(1.0f/3.0f);  // se sèche
		if ( m_absorbWater < 0.0f )  m_absorbWater = 0.0f;
	}

	if ( pos.y >= level       &&
		 m_absorbWater > 0.0f &&
		 !m_water->RetLava()  )  // gouttes en sortant de l'eau ?
	{
		if ( aTime-m_lastUnderParticule >= m_engine->ParticuleAdapt(0.05f) )
		{
			m_lastUnderParticule = aTime;

			nb = (int)(8.0f*m_absorbWater);
			for ( i=0 ; i<nb ; i++ )
			{
				pos = m_object->RetPosition(0);
				if ( type == OBJECT_HUMAN )  pos.y -= Rand()*2.0f;
				else                         pos.y += Rand()*2.0f;
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				speed.y = -((Rand()-0.5f)*8.0f+8.0f);
				speed.x = 0.0f;
				speed.z = 0.0f;
				dim.x = 0.2f;
				dim.y = 0.2f;
				m_particule->CreateParticule(pos, speed, dim, PARTIWATER, 2.0f, 0.0f);
			}
		}
	}

	if ( type == OBJECT_HUMAN ||  // homme ?
		 type == OBJECT_TECH  )
	{
		if ( m_bLand &&
			 aTime-m_lastSlideParticule >= m_engine->ParticuleAdapt(0.05f) )
		{
			h = Max(Abs(m_linMotion.terrainSpeed.x),
					Abs(m_linMotion.terrainSpeed.z));
			if ( h > m_linMotion.terrainSlide.x+0.5f &&
				 m_linMotion.motorSpeed.x == 0.0f )  // glisse à l'arrêt ?
			{
				m_lastSlideParticule = aTime;

				mat = m_object->RetWorldMatrix(0);
				pos.x = (Rand()-0.5f)*1.0f;
				pos.y = -m_object->RetCharacter()->height;
				pos.z = Rand()*0.4f+1.0f;
				if ( rand()%2 == 0 )  pos.z = -pos.z;
				pos = Transform(*mat, pos);
				speed = D3DVECTOR(0.0f, 1.0f, 0.0f);
				dim.x = Rand()*(h-5.0f)/2.0f+1.0f;
				if ( dim.x > 2.5f )  dim.x = 2.5f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f, 0.0f);
			}
		}
	}

	if ( type == OBJECT_MOBILEfb ||
		 type == OBJECT_MOBILEob )  // chenilles ?
	{
		if ( aTime-m_lastSlideParticule >= m_engine->ParticuleAdapt(0.05f) )
		{
			h = Abs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
			if ( h > 5.0f )
			{
				m_lastSlideParticule = aTime;

				mat = m_object->RetWorldMatrix(0);
				pos.x = (Rand()-0.5f)*8.0f;
				pos.y = 0.0f;
				pos.z = Rand()*2.0f+3.0f;
				if ( rand()%2 == 0 )  pos.z = -pos.z;
				pos = Transform(*mat, pos);
				speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
				dim.x = Rand()*(h-5.0f)/2.0f+1.0f;
				if ( dim.x > 3.0f )  dim.x = 3.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f, 0.0f);
			}
		}
	}

	if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && !m_bSwim )
	{
		if ( m_bLand )  // au sol ?
		{
			if ( m_reactorTemperature > 0.0f )
			{
				m_reactorTemperature -= rTime*(1.0f/10.0f);  // ça refroidi
				if ( m_reactorTemperature < 0.0f )
				{
					m_reactorTemperature = 0.0f;
				}
			}

			if ( m_reactorTemperature == 0.0f ||
				 aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.05f) )  return;
			m_lastMotorParticule = aTime;

			pos = D3DVECTOR(-1.6f, -0.5f, 0.0f);
			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, pos);

			speed.x = (Rand()-0.5f)*0.6f;
			speed.z = (Rand()-0.5f)*0.6f;
			speed.y = -(0.5f+Rand()*0.3f)*(1.0f-m_reactorTemperature);

			dim.x = (1.0f+Rand()*0.5f)*(0.2f+m_reactorTemperature*0.8f);
			dim.y = dim.x;

			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE2, 3.0f, 0.0f);
		}
		else	// en vol ?
		{
			if ( !m_bMotor )  return;

			if ( m_reactorTemperature < 1.0f )  // pas trop chaud ?
			{
				m_reactorTemperature += rTime*(1.0f/4.0f);  // ça chauffe
				if ( m_reactorTemperature > 1.0f )
				{
					m_reactorTemperature = 1.0f;  // mais pas trop
				}
			}

			if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.02f) )  return;
			m_lastMotorParticule = aTime;

			pos = D3DVECTOR(-1.6f, -1.0f, 0.0f);
			pos.x += (Rand()-0.5f)*3.0f;
			pos.y += (Rand()-0.5f)*1.5f;
			pos.z += (Rand()-0.5f)*3.0f;
			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, pos);

			h = m_floorHeight;
			if ( h > 10.0f )  // assez haut ?
			{
				speed = D3DVECTOR(0.0f, -10.0f, 0.0f);  // contre le bas
			}
			else
			{
				speed.y = 10.0f-2.0f*h - Rand()*(10.0f-h);  // contre le haut
				speed.x = (Rand()-0.5f)*(5.0f-h)*1.0f;  // horizontal (xz)
				speed.z = (Rand()-0.5f)*(5.0f-h)*1.0f;
			}

			dim.x = 0.12f;
			dim.y = 0.12f;

			m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS, 2.0f, 10.0f);

#if 1
			pos = D3DVECTOR(-1.6f, -0.5f, 0.0f);
			pos = Transform(*mat, pos);

			speed.x = (Rand()-0.5f)*1.0f;
			speed.z = (Rand()-0.5f)*1.0f;
			speed.y = -(4.0f+Rand()*3.0f);
			speed.x += m_linMotion.realSpeed.x*0.8f;
			speed.z -= m_linMotion.realSpeed.x*m_cirMotion.realSpeed.y*0.05f;
			if ( m_linMotion.realSpeed.y > 0.0f )
			{
				speed.y += m_linMotion.realSpeed.y*0.5f;
			}
			else
			{
				speed.y += m_linMotion.realSpeed.y*1.2f;
			}
			a = m_object->RetAngleY(0);
			p.x = speed.x;
			p.y = speed.z;
			p = RotatePoint(-a, p);
			speed.x = p.x;
			speed.z = p.y;

			dim.x = 0.4f+Rand()*0.2f;
			dim.y = dim.x;

			m_particule->CreateParticule(pos, speed, dim, PARTIEJECT, 0.3f, 10.0f);
#endif
		}
	}

	if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && m_bSwim )
	{
		m_reactorTemperature = 0.0f;  // réacteur froid
	}

	if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && m_bSwim )
	{
		if ( !m_object->RetDead() )
		{
			h = Mod(aTime, 5.0f);
			if ( h < 3.5f && ( h < 1.5f || h > 1.6f ) )  return;
		}
		if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.06f) )  return;
		m_lastMotorParticule = aTime;

		pos = D3DVECTOR(0.0f, 3.0f, 0.0f);
		mat = m_object->RetWorldMatrix(0);
		pos = Transform(*mat, pos);
		pos.x += (Rand()-0.5f)*1.0f;
		pos.z += (Rand()-0.5f)*1.0f;
		speed.y = (Rand()-0.5f)*8.0f+8.0f;
		speed.x = (Rand()-0.5f)*0.2f;
		speed.z = (Rand()-0.5f)*0.2f;
		dim.x = 0.2f;
		dim.y = 0.2f;
		m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f);

		if ( aTime-m_lastSoundWater > 1.5f )
		{
			m_lastSoundWater = aTime;
			m_sound->Play(SOUND_BLUP, m_object->RetPosition(0), 0.5f+Rand()*0.5f);
		}
	}

	if ( m_type == TYPE_RACE ||
		 m_type == TYPE_TANK )
	{
		if ( type == OBJECT_MOBILEfb ||
			 type == OBJECT_MOBILEob )
		{
			if ( !m_bMotor )  return;

			if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.1f) )  return;
			m_lastMotorParticule = aTime;

			pos = D3DVECTOR(-2.5f, 10.3f, -1.3f);
			pos.x += (Rand()-0.5f)*1.0f;
			pos.z += (Rand()-0.5f)*1.0f;
			mat = m_object->RetWorldMatrix(0);
			pos   = Transform(*mat, pos);

			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = 1.5f+Rand()*1.0f;

			dim.x = Rand()*0.6f+0.4f;
			dim.y = dim.x;

			m_particule->CreateParticule(pos, speed, dim, PARTIMOTOR, 2.0f);
		}
		else if ( type == OBJECT_TRAX )
		{
//?			if ( !m_bMotor )  return;
//?			if ( m_linMotion.motorSpeed.x == 0.0f )  return;

			if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.1f) )  return;
			m_lastMotorParticule = aTime;

			mat = m_object->RetWorldMatrix(0);
			for ( i=0 ; i<5 ; i++ )
			{
				pos = D3DVECTOR(2.0f, 16.0f, -2.0f);
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*10.0f;
				speed.z = (Rand()-0.5f)*10.0f;
				speed.y = 10.0f+Rand()*10.0f;
				dim.x = Rand()*2.0f+1.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 2.0f);
			}
		}
		else if ( type == OBJECT_UFO )
		{
			if ( !m_bMotor )  return;
			if ( m_linMotion.motorSpeed.x == 0.0f )  return;

			if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.05f) )  return;
			m_lastMotorParticule = aTime;

			mat = m_object->RetWorldMatrix(0);
			for ( i=0 ; i<10 ; i++ )
			{
				pos = D3DVECTOR(-6.0f, 3.0f, 0.0f);
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*3.0f;
				speed.y = (Rand()-0.5f)*3.0f;
				speed.z = (Rand()-0.5f)*3.0f;
				dim.x = (Rand()*1.5f+1.5f);
				dim.y = dim.x;
				duration = 0.6f+Rand()*0.6f;
				m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, duration);

				pos.x = (Rand()-0.5f)*5.0f;
				pos.z = (Rand()-0.5f)*5.0f;
				pos.y = -1.0f;
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*5.0f;
				speed.z = (Rand()-0.5f)*5.0f;
				speed.y = -(15.0f+Rand()*15.0f);
				dim.x = (Rand()*0.2f+0.2f);
				dim.y = dim.x;
				duration = 0.6f+Rand()*0.6f;
				m_particule->CreateParticule(pos, speed, dim, PARTIEJECT, duration, 20.0f);
			}

			// Fume si soucoupe abimée.
			factor = m_object->RetShield();
			if ( factor < 0.7f )
			{
				pos = D3DVECTOR(-6.0f, 3.0f, 0.0f);
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*5.0f;
				speed.z = (Rand()-0.5f)*5.0f;
				speed.y = (12.0f+Rand()*12.0f);
				dim.x = (Rand()*8.0f+8.0f)*(1.0f-factor);
				dim.y = dim.x;
				duration = 1.0f+Rand()*1.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, duration);
			}
		}
		else
		{
			if ( m_object->RetDead() || RetLock() || m_bSilent )  return;

			if ( m_bWater && aTime-m_lastSoundWater > 0.0f )
			{
				m_lastSoundWater = aTime+Rand()*1.0f+1.0f;
				m_sound->Play(Rand()>0.5f?SOUND_BLUP:SOUND_SWIM, m_object->RetPosition(0), 0.5f+Rand()*0.5f);
			}

			if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.05f) )  return;
			m_lastMotorParticule = aTime;

			mat = m_object->RetWorldMatrix(0);
			factor = Abs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x;
			factor = 1.0f-2.0f*factor;
			if ( m_timeLock < STARTDELAY+0.5f )  // démarrage moteur ?
			{
				size = 4.0f-((m_timeLock-STARTDELAY)/0.5f)*3.0f;
			}
			else
			{
				size = 1.0f;
			}
			if ( factor > 0.2f && m_linMotion.realSpeed.x < 80.0f )
			{
				parti = m_bWater?PARTIBUBBLE:PARTISMOKE3;
				for ( i=0 ; i<3 ; i++ )
				{
					GazCompute(model, mat, i, 1.0f, factor, pos, speed);
					dim.x = (Rand()*0.4f+0.3f)*factor*size;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, 2.0f);

					GazCompute(model, mat, i, -1.0f, factor, pos, speed);
					dim.x = (Rand()*0.4f+0.3f)*factor*size;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, 2.0f);
				}
			}
			if ( m_wheelSlide > 0.0f && m_linMotion.realSpeed.x < 80.0f )
			{
				boost = 1.0f+m_wheelSlide*2.0f;
				parti = m_bWater?PARTIBUBBLE:PARTIFLAME;
				for ( i=0 ; i<3 ; i++ )
				{
					GazCompute(model, mat, i, 1.0f, boost, pos, speed);
					dim.x = (Rand()*0.4f+0.3f)*boost;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, 0.3f, 20.0f);

					GazCompute(model, mat, i, -1.0f, boost, pos, speed);
					dim.x = (Rand()*0.4f+0.3f)*boost;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, 0.3f, 20.0f);
				}
			}
			if ( factor <= 0.2f && m_wheelSlide <= 0.0f && m_linMotion.realSpeed.x < 80.0f )
			{
				parti = m_bWater?PARTIBUBBLE:PARTIMOTOR;
				for ( i=0 ; i<3 ; i++ )
				{
					GazCompute(model, mat, i, 1.0f, 2.0f, pos, speed);
					dim.x = (Rand()*0.4f+0.3f);
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, 0.7f, 10.0f+Rand()*20.0f);

					GazCompute(model, mat, i, -1.0f, 2.0f, pos, speed);
					dim.x = (Rand()*0.4f+0.3f);
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, 0.7f, 10.0f+Rand()*20.0f);
				}
			}
			if ( m_linMotion.realSpeed.x < 50.0f )
			{
				if ( model == 8 )  // torpedo ?
				{
					parti = m_bWater?PARTIBUBBLE:PARTICRASH;
					pos.x = (1.5f+(Rand()-0.5f)*5.0f)*1.1f;
					pos.y = (2.5f+(Rand()-0.5f)*2.0f)*1.1f;
					pos.z = (0.0f+(Rand()-0.5f)*3.0f)*1.1f;
					speed.x = (Rand()-0.5f)*2.0f;
					speed.y = Rand()*4.0f+4.0f;
					speed.z = Rand()*3.0f+3.0f;
					if ( Rand() > 0.5f )  speed.z = -speed.z;
					pos   = Transform(*mat, pos);
					speed = Transform(*mat, speed)-pos;
					dim.x = (Rand()*1.0f+1.0f);
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, parti, Rand()*0.7f+0.5f);
				}
			}
		}
	}
}

// Génère qq particules suite à une chute dans l'eau.

void CPhysics::WaterParticule(float aTime, D3DVECTOR pos, ObjectType type,
							  float floor, float advance, float turn)
{
	D3DVECTOR	ppos, speed;
	FPOINT		dim;
	float		delay, level, min, max, force, volume, diam, speedy;
	int			i, nb;

	level = m_water->RetLevel();
	if ( floor >= level )  return;

	if ( type == OBJECT_HUMAN ||
		 type == OBJECT_TECH  )
	{
		min = 3.0f;
		max = 3.0f;
	}
	else
	{
		min = 0.0f;
		max = 9.0f;
	}

	if ( pos.y+max < level || pos.y-min > level )  return;

//?	speedy = m_linMotion.realSpeed.y;
	speedy = m_graviSpeed;

	// Gestion de la particule "plouf".
	if ( speedy < -5.0f &&
		 aTime-m_lastPloufParticule >= 1.0f )
	{
		m_lastPloufParticule = aTime;

		force = -speedy/20.0f;  // force selon vitesse chute
		if ( type == OBJECT_HUMAN ||
			 type == OBJECT_TECH  )
		{
			diam = 2.5f;
		}
		else
		{
			diam = 5.0f;
			force *= 1.3f;  // un robot est plus lourd
		}

		pos = m_object->RetPosition(0);
		pos.y = m_water->RetLevel()-1.0f;
		dim.x = 2.0f*force;  // hauteur
		dim.y = diam;  // diamètre
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);

		force = (0.5f+force*0.5f);
		nb = (int)(force*50.0f*m_engine->RetParticuleDensity());
		for ( i=0 ; i<nb ; i++ )
		{
			ppos = pos;
			ppos.x += (Rand()-0.5f)*4.0f;
			ppos.z += (Rand()-0.5f)*4.0f;
			ppos.y += 0.6f;
			speed.x = (Rand()-0.5f)*12.0f*force;
			speed.z = (Rand()-0.5f)*12.0f*force;
			speed.y = 6.0f+Rand()*6.0f*force;
			dim.x = 0.5f;
			dim.y = dim.x;
			m_particule->CreateParticule(ppos, speed, dim, PARTIDROP, 2.0f, 20.0f);
		}

		volume = Abs(speedy*0.02f);
		if ( volume > 1.0f )  volume = 1.0f;
		m_sound->Play(SOUND_PLOUF, pos, volume);
	}

	// Gestion des particules "flic".
	if ( m_water->RetLava() )  return;

	if ( advance == 0.0f && turn == 0.0f )
	{
		turn = 10.0f;
		delay = 0.50f;
	}
	else if ( advance == 0.0f )
	{
		delay = 0.24f;
	}
	else
	{
		delay = 0.06f;
	}
	m_engine->ParticuleAdapt(delay);

	if ( aTime-m_lastWaterParticule < delay )  return;
	m_lastWaterParticule = aTime;

	force = (advance+turn)*0.16f;
	if ( force < 0.001f )  return;

	pos = m_object->RetPosition(0);
	pos.y = level+0.1f;
	if ( advance == 0 )
	{
		pos.x += (Rand()-0.5f)*10.0f;
		pos.z += (Rand()-0.5f)*10.0f;
	}
	else
	{
		pos.x += (Rand()-0.5f)*4.0f;
		pos.z += (Rand()-0.5f)*4.0f;
	}
	speed.y = 0.0f;
	speed.x = 0.0f;
	speed.z = 0.0f;
	dim.x = Min(Rand()*force+force+1.0f, 10.0f);
	dim.y = dim.x;
	m_particule->CreateParticule(pos, speed, dim, PARTIFLIC, 3.0f, 0.0f);
}


// Crée l'interface.

void CPhysics::CreateInterface(BOOL bSelect)
{
	if ( m_brain != 0 )
	{
		m_brain->CreateInterface(bSelect);
	}
}


// Retourne une erreur liée à l'état général.

Error CPhysics::RetError()
{
	return ERR_OK;
}

