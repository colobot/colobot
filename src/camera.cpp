// camera.cpp

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
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "physics.h"
#include "camera.h"




// Constructeur de l'objet.

CCamera::CCamera(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_CAMERA, this);

	m_engine  = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water   = (CWater*)m_iMan->SearchInstance(CLASS_WATER);

	m_type       = CAMERA_FREE;
	m_smooth     = CS_NORM;
	m_smoothNext = CS_NORM;
	m_cameraObj  = 0;

	m_eyeDistance = 10.0f;
	m_initDelay   =  0.0f;

	m_actualEye    = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_actualLookat = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_finalEye     = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_finalLookat  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_normEye      = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_normLookat   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_focus        = 1.0f;

	m_bRightDown     = FALSE;
	m_rightPosInit   = FPOINT(0.5f, 0.5f);
	m_rightPosCenter = FPOINT(0.5f, 0.5f);
	m_rightPosMove   = FPOINT(0.5f, 0.5f);

	m_eyePt        = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_directionH   =  0.0f;
	m_directionV   =  0.0f;
	m_heightEye    = 20.0f;
	m_heightShift  =  0.0f;
	m_heightLookat =  0.0f;
	m_speed        =  2.0f;

	m_backDist      = 0.0f;
	m_backMin       = 0.0f;
	m_backMotorSpeed= 0.0f;
	m_bBackLockRotate = FALSE;
	m_backSleepTime = 0.0f;
	m_backRotSpeed  = 0.0f;
	m_rotDirectionH = 0.0f;
	m_addDirectionH = 0.0f;
	m_addDirectionV = 0.0f;
	m_bTransparency = FALSE;

	m_fixDist       = 0.0f;
	m_fixDirectionH = 0.0f;
	m_fixDirectionV = 0.0f;

	m_visitGoal       = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_visitDist       = 0.0f;
	m_visitTime       = 0.0f;
	m_visitType       = CAMERA_NULL;
	m_visitDirectionH = 0.0f;
	m_visitDirectionV = 0.0f;

	m_editHeight = 40.0f;

	m_mouseDirH    = 0.0f;
	m_mouseDirV    = 0.0f;
	m_mouseMarging = 0.01f;

	m_centeringPhase    = CP_NULL;
	m_centeringAngleH   = 0.0f;
	m_centeringAngleV   = 0.0f;
	m_centeringDist     = 0.0f;
	m_centeringCurrentH = 0.0f;
	m_centeringCurrentV = 0.0f;
	m_centeringTime     = 0.0f;
	m_centeringProgress = 0.0f;

	m_effectType     = CE_NULL;
	m_effectPos      = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_effectForce    = 0.0f;
	m_effectProgress = 0.0f;
	m_effectOffset   = D3DVECTOR(0.0f, 0.0f, 0.0f);

	m_scriptEye    = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_scriptLookat = D3DVECTOR(0.0f, 0.0f, 0.0f);

	m_bEffect = TRUE;
	m_bFlash  = TRUE;
}

// Destructeur de l'objet.

CCamera::~CCamera()
{
}


void CCamera::SetEffect(BOOL bEnable)
{
	m_bEffect = bEnable;
}

void CCamera::SetFlash(BOOL bFlash)
{
	m_bFlash = bFlash;
}


// Indique la consigne de vitesse moteur avant/arrière.

void CCamera::SetMotorSpeed(float speed)
{
	m_backMotorSpeed = speed;
}



// Initialise la caméra.

void CCamera::Init(D3DVECTOR eye, D3DVECTOR lookat, float delay)
{
	D3DVECTOR	vUpVec;

	m_initDelay = delay;

//?	eye.y    += m_terrain->RetFloorLevel(eye,    TRUE);
//?	lookat.y += m_terrain->RetFloorLevel(lookat, TRUE);

	m_type = CAMERA_FREE;
	m_eyePt = eye;

	m_directionH = RotateAngle(eye.x-lookat.x, eye.z-lookat.z)+PI/2.0f;
	m_directionV = -RotateAngle(Length2d(eye, lookat), eye.y-lookat.y);

	m_eyeDistance = 10.0f;
	m_heightLookat = 10.0f;
	m_backDist = 30.0f;
	m_backMin  = 10.0f;
	m_rotDirectionH = 0.0f;
	m_addDirectionH = 0.0f;
	m_addDirectionV = -PI*0.05f;
	m_fixDist = 50.0f;
	m_fixDirectionH = PI*0.25f;
	m_fixDirectionV = -PI*0.10f;
	m_centeringPhase = CP_NULL;
	m_actualEye = m_eyePt;
	m_actualLookat = LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
	m_finalEye = m_actualEye;
	m_finalLookat = m_actualLookat;
	m_scriptEye = m_actualEye;
	m_scriptLookat = m_actualLookat;
	m_focus = 1.00f;

	FlushEffect();
	FlushOver();
	SetType(CAMERA_FREE);
}


// Donne l'objet pilotant la caméra.

void CCamera::SetObject(CObject* object)
{
	m_cameraObj = object;
}

CObject* CCamera::RetObject()
{
	return m_cameraObj;
}


// Modifie le niveau de transparence d'un objet et des objets
// transportés (pile & fret).

void SetTransparency(CObject* pObj, float value)
{
	CObject*	pFret;

	pObj->SetTransparency(value);

	pFret = pObj->RetFret();
	if ( pFret != 0 )
	{
		pFret->SetTransparency(value);
	}

	pFret = pObj->RetPower();
	if ( pFret != 0 )
	{
		pFret->SetTransparency(value);
	}
}

// Modifie le type de la caméra.

void CCamera::SetType(CameraType type)
{
	CObject*	pObj;
	ObjectType	oType;
	D3DVECTOR	vUpVec;
	int			i;

	if ( m_type == CAMERA_BACK && m_bTransparency )
	{
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;

			if ( pObj->RetTruck() )  continue;  // pile ou fret?

			SetTransparency(pObj, 0.0f);  // objet opaque
		}
	}
	m_bTransparency = FALSE;

	if ( type == CAMERA_INFO  ||
		 type == CAMERA_VISIT )  // xx -> info ?
	{
		m_normEye    = m_engine->RetEyePt();
		m_normLookat = m_engine->RetLookatPt();

		m_engine->SetFocus(1.00f);  // normal
		m_type = type;
		return;
	}

	if ( m_type == CAMERA_INFO  ||
		 m_type == CAMERA_VISIT )  // info -> xx ?
	{
		m_engine->SetFocus(m_focus);  // remet focus initial
		m_type = type;

		vUpVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
		SetViewParams(m_normEye, m_normLookat, vUpVec);
		return;
	}

	if ( m_type == CAMERA_BACK && type == CAMERA_FREE )  // back -> free ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);
	}

	if ( m_type == CAMERA_BACK && type == CAMERA_EDIT )  // back -> edit ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -1.0f);
	}

	if ( m_type == CAMERA_ONBOARD && type == CAMERA_FREE )  // onboard -> free ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);
	}

	if ( m_type == CAMERA_ONBOARD && type == CAMERA_EDIT )  // onboard -> edit ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);
	}

	if ( m_type == CAMERA_ONBOARD && type == CAMERA_EXPLO )  // onboard -> explo ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);
	}

	if ( m_type == CAMERA_BACK && type == CAMERA_EXPLO )  // back -> explo ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -20.0f);
	}

	if ( type == CAMERA_FIX )
	{
		AbortCentering();  // stoppe cadrage spécial
	}

	if ( type == CAMERA_BACK )
	{
		AbortCentering();  // stoppe cadrage spécial
		m_rotDirectionH = 0.0f;
		m_addDirectionH = 0.0f;
		m_addDirectionV = -PI*0.05f;

		if ( m_cameraObj == 0 )  oType = OBJECT_NULL;
		else                     oType = m_cameraObj->RetType();

		m_backDist = 20.0f;
		if ( oType == OBJECT_HUMAN    )  m_backDist =  15.0f;
		if ( oType == OBJECT_TECH     )  m_backDist =  20.0f;
		if ( oType == OBJECT_TOWER    )  m_backDist =  45.0f;
		if ( oType == OBJECT_NUCLEAR  )  m_backDist =  70.0f;
		if ( oType == OBJECT_PARA     )  m_backDist = 180.0f;
		if ( oType == OBJECT_DOCK     )  m_backDist =  50.0f;
		if ( oType == OBJECT_HOME1    )  m_backDist =  40.0f;
		if ( oType == OBJECT_HOME2    )  m_backDist =  40.0f;
		if ( oType == OBJECT_HOME3    )  m_backDist =  40.0f;
		if ( oType == OBJECT_HOME4    )  m_backDist =  40.0f;
		if ( oType == OBJECT_HOME5    )  m_backDist =  40.0f;
		if ( oType == OBJECT_STARTER  )  m_backDist =   5.0f;
		if ( oType == OBJECT_INCA7    )  m_backDist = 100.0f;

		m_backMin = m_backDist/3.0f;
		if ( oType == OBJECT_HUMAN    )  m_backMin =  10.0f;
		if ( oType == OBJECT_TECH     )  m_backMin =  10.0f;
		if ( oType == OBJECT_NUCLEAR  )  m_backMin =  32.0f;
		if ( oType == OBJECT_PARA     )  m_backMin =  40.0f;
		if ( oType == OBJECT_DOCK     )  m_backMin =  20.0f;

		if ( oType == OBJECT_HOME1 ||
			 oType == OBJECT_HOME3 ||
			 oType == OBJECT_HOME4 ||
			 oType == OBJECT_HOME5 )
		{
			m_addDirectionV = -PI*0.10f;
		}
		if ( oType == OBJECT_HOME2 )
		{
			m_addDirectionV = -PI*0.04f;
		}
		if ( oType == OBJECT_INCA7 )
		{
			m_addDirectionV = PI*0.02f;
			m_addDirectionH = PI*1.0f;
		}
	}

	if ( type == CAMERA_ONBOARD )
	{
		m_focus = 1.50f;  // grand-angle
	}
	else
	{
		m_focus = 1.00f;  // normal
	}
	m_engine->SetFocus(m_focus);

	m_type = type;

	SetSmooth(CS_NORM);
}

CameraType CCamera::RetType()
{
	return m_type;
}


// Gestion du mode de lissage.

void CCamera::SetSmoothOneTime(CameraSmooth type)
{
	if ( type == m_smooth )  return;
	m_smoothNext = m_smooth;  // revient ensuite au mode initial
	m_smooth = type;
}

void CCamera::SetSmooth(CameraSmooth type)
{
	m_smoothNext = type;
	m_smooth = type;
}

CameraSmooth CCamera::RetSmoth()
{
	return m_smooth;
}


// Gestion de la distance de recul.

void CCamera::SetBackDist(float dist)
{
	m_backDist = dist;
}

float CCamera::RetBackDist()
{
	return m_backDist;
}


// Gestion de l'angle vertical.

void CCamera::SetBackVerti(float angle)
{
	m_addDirectionV = angle;
}

float CCamera::RetBackVerti()
{
	return m_addDirectionV;
}


// Gestion de l'angle horizontal.

void CCamera::SetBackHoriz(float angle)
{
	m_addDirectionH = angle;
}

float CCamera::RetBackHoriz()
{
	return m_addDirectionH;
}


void CCamera::ResetLockRotate()
{
	m_backSleepTime = 0.0f;
	m_backRotSpeed  = 0.0f;
}

void CCamera::SetLockRotate(BOOL bLock)
{
	m_bBackLockRotate = bLock;
}

BOOL CCamera::RetLockRotate()
{
	return m_bBackLockRotate;
}


// Gestion de la distance de recul.

void CCamera::SetDist(float dist)
{
	m_fixDist = dist;
}

float CCamera::RetDist()
{
	return m_fixDist;
}


// Gestion de l'angle en mode CAMERA_FIX.

void CCamera::SetFixDirection(float angle)
{
	m_fixDirectionH = angle;
}

float CCamera::RetFixDirection()
{
	return m_fixDirectionH;
}


// Début d'une visite circulaire avec la caméra.

void CCamera::StartVisit(D3DVECTOR goal, float dist)
{
	m_visitType = m_type;
	SetType(CAMERA_VISIT);
	m_visitGoal = goal;
	m_visitDist = dist;
	m_visitTime = 0.0f;
	m_visitDirectionH = 0.0f;
	m_visitDirectionV = -PI*0.10f;
}

// Fin d'une visite circulaire avec la caméra.

void CCamera::StopVisit()
{
	SetType(m_visitType);  // remet le type initial
}


// Retourne le point de vue de la caméra.

void CCamera::RetCamera(D3DVECTOR &eye, D3DVECTOR &lookat)
{
	eye = m_eyePt;
	lookat = LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
}


// Spécifie un mouvement spécial de caméra pour cadrer une action.

BOOL CCamera::StartCentering(CObject *object, float angleH, float angleV,
							 float dist, float time)
{
	if ( m_type != CAMERA_BACK )  return FALSE;
	if ( object != m_cameraObj )  return FALSE;

	if ( m_centeringPhase != CP_NULL )  return FALSE;

	if ( m_addDirectionH > PI )
	{
		angleH = PI*2.0f-angleH;
	}

	m_centeringPhase    = CP_START;
	m_centeringAngleH   = angleH;
	m_centeringAngleV   = angleV;
	m_centeringDist     = dist;
	m_centeringCurrentH = 0.0f;
	m_centeringCurrentV = 0.0f;
	m_centeringTime     = time;
	m_centeringProgress = 0.0f;

	return TRUE;
}

// Termine un mouvement spécial de caméra pour cadrer une action.

BOOL CCamera::StopCentering(CObject *object, float time)
{
	if ( m_type != CAMERA_BACK )  return FALSE;
	if ( object != m_cameraObj )  return FALSE;

	if ( m_centeringPhase != CP_START &&
		 m_centeringPhase != CP_WAIT  )  return FALSE;

	m_centeringPhase = CP_STOP;

	if ( m_centeringAngleH != 99.9f )
	{
		m_centeringAngleH = m_centeringCurrentH;
	}
	if ( m_centeringAngleV != 99.9f )
	{
		m_centeringAngleV = m_centeringCurrentV;
	}

	m_centeringTime     = time;
	m_centeringProgress = 0.0f;

	m_backSleepTime = 0.0f;
	m_backRotSpeed  = 0.0f;

	return TRUE;
}

// Stoppe le cadrage spécial dans la position actuelle.

void CCamera::AbortCentering()
{
	if ( m_type == CAMERA_INFO  ||
		 m_type == CAMERA_VISIT )  return;

	if ( m_centeringPhase == CP_NULL )  return;

	m_centeringPhase = CP_NULL;

	if ( m_centeringAngleH != 99.9f )
	{
		m_addDirectionH = m_centeringCurrentH;
	}
	if ( m_centeringAngleV != 99.9f )
	{
		m_addDirectionV = m_centeringCurrentV;
	}
}



// Supprime l'effet spécial avec la caméra.

void CCamera::FlushEffect()
{
	m_effectType     = CE_NULL;
	m_effectForce    = 0.0f;
	m_effectProgress = 0.0f;
	m_effectOffset   = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Démarre un effet spécial avec la caméra.

void CCamera::StartEffect(CameraEffect effect, D3DVECTOR pos, float force)
{
	if ( !m_bEffect )  return;

	m_effectType     = effect;
	m_effectPos      = pos;
	m_effectForce    = force;
	m_effectProgress = 0.0f;
}

// Fait progresser l'effet de la caméra.

void CCamera::EffectFrame(const Event &event)
{
	float		dist, force;

	if ( m_type == CAMERA_INFO  ||
		 m_type == CAMERA_VISIT )  return;

	if ( m_effectType == CE_NULL )  return;

	m_effectOffset = D3DVECTOR(0.0f, 0.0f, 0.0f);
	force = m_effectForce;

	if ( m_effectType == CE_TERRAFORM )
	{
		m_effectProgress += event.rTime*0.7f;
		m_effectOffset.x = (Rand()-0.5f)*10.0f;
		m_effectOffset.y = (Rand()-0.5f)*10.0f;
		m_effectOffset.z = (Rand()-0.5f)*10.0f;

		force *= 1.0f-m_effectProgress;
	}

	if ( m_effectType == CE_EXPLO )
	{
		m_effectProgress += event.rTime*1.0f;
		m_effectOffset.x = (Rand()-0.5f)*5.0f;
		m_effectOffset.y = (Rand()-0.5f)*5.0f;
		m_effectOffset.z = (Rand()-0.5f)*5.0f;

		force *= 1.0f-m_effectProgress;
	}

	if ( m_effectType == CE_SHOT )
	{
		m_effectProgress += event.rTime*1.0f;
		m_effectOffset.x = (Rand()-0.5f)*2.0f;
		m_effectOffset.y = (Rand()-0.5f)*2.0f;
		m_effectOffset.z = (Rand()-0.5f)*2.0f;

		force *= 1.0f-m_effectProgress;
	}

	if ( m_effectType == CE_CRASH )
	{
		m_effectProgress += event.rTime*5.0f;
		m_effectOffset.y = sinf(m_effectProgress*PI)*1.5f;
		m_effectOffset.x = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
		m_effectOffset.z = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
	}

	if ( m_effectType == CE_VIBRATION )
	{
		m_effectProgress += event.rTime*0.1f;
		m_effectOffset.y = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
		m_effectOffset.x = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
		m_effectOffset.z = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
	}

	if ( m_effectType == CE_PET )
	{
		m_effectProgress += event.rTime*5.0f;
		m_effectOffset.x = (Rand()-0.5f)*0.2f;
		m_effectOffset.y = (Rand()-0.5f)*2.0f;
		m_effectOffset.z = (Rand()-0.5f)*0.2f;
	}

	dist = Length(m_eyePt, m_effectPos);
	dist = (dist-100.f)/100.0f;
	if ( dist < 0.0f )  dist = 0.0f;
	if ( dist > 1.0f )  dist = 1.0f;

	force *= 1.0f-dist;
	m_effectOffset *= force;

	if ( m_effectProgress >= 1.0f )
	{
		FlushEffect();
	}
}


// Supprime l'effet de superposition au premier plan.

void CCamera::FlushOver()
{
	m_overType = OE_NULL;
	m_overColorBase.r = 0.0f;  // noir
	m_overColorBase.g = 0.0f;
	m_overColorBase.b = 0.0f;
	m_overColorBase.a = 0.0f;
	m_engine->SetOverColor();  // rien
}

// Spécifie la couleur de base.

void CCamera::SetOverBaseColor(D3DCOLORVALUE color)
{
	m_overColorBase = color;
}

// Démarre un effet de superposition au premier plan.

void CCamera::StartOver(OverEffect effect, D3DVECTOR pos, float force)
{
	D3DCOLOR	color;
	float		dist, decay;

	if ( !m_bFlash )  return;

	m_overType = effect;
	m_overTime = 0.0f;

	if ( m_overType == OE_BLITZ )  decay = 400.0f;
	else                           decay = 100.0f;
	dist = Length(m_eyePt, pos);
	dist = (dist-decay)/decay;
	if ( dist < 0.0f )  dist = 0.0f;
	if ( dist > 1.0f )  dist = 1.0f;

	m_overForce = force * (1.0f-dist);

	if ( m_overType == OE_BLOOD )
	{
		m_overColor.r = 0.8f;
		m_overColor.g = 0.1f;
		m_overColor.b = 0.1f;  // rouge
		m_overMode    = D3DSTATETCb;

		m_overFadeIn  = 0.4f;
		m_overFadeOut = 0.8f;
		m_overForce   = 1.0f;
	}

	if ( m_overType == OE_ORGA )
	{
		m_overColor.r = 0.1f;
		m_overColor.g = 0.8f;
		m_overColor.b = 0.1f;  // vert
		m_overMode    = D3DSTATETCb;

		m_overFadeIn  = 0.4f;
		m_overFadeOut = 0.8f;
		m_overForce   = 1.0f;
	}

	if ( m_overType == OE_CRASH )
	{
		m_overColor.r = 0.8f;
		m_overColor.g = 0.8f;
		m_overColor.b = 0.8f;  // blanc
		m_overMode    = D3DSTATETCb;

		m_overFadeIn  = 0.0f;
		m_overFadeOut = 0.2f;
		m_overForce   = 1.0f;
	}

	if ( m_overType == OE_FADEINw )
	{
		m_overColor.r = 1.0f;
		m_overColor.g = 1.0f;
		m_overColor.b = 1.0f;  // blanc
		m_overMode    = D3DSTATETCb;

		m_overFadeIn  = 0.0f;
		m_overFadeOut =20.0f;
		m_overForce   = 1.0f;
	}

	if ( m_overType == OE_FADEOUTw )
	{
		m_overColor.r = 1.0f;
		m_overColor.g = 1.0f;
		m_overColor.b = 1.0f;  // blanc
		m_overMode    = D3DSTATETCb;

		m_overFadeIn  = 6.0f;
		m_overFadeOut = 100000.0f;
		m_overForce   = 1.0f;
	}

	if ( m_overType == OE_FADEOUTb )
	{
		color = m_engine->RetFogColor(1);  // couleur brouillard sous-marin
		m_overColor = RetColor(color);
		m_overMode = D3DSTATETCw;

		m_overFadeIn  = 4.0f;
		m_overFadeOut = 100000.0f;
		m_overForce   = 1.0f;
	}

	if ( m_overType == OE_BLITZ )
	{
		m_overColor.r = 0.9f;
		m_overColor.g = 1.0f;
		m_overColor.b = 1.0f;  // blanc-cyan
		m_overMode    = D3DSTATETCb;

		m_overFadeIn  = 0.0f;
		m_overFadeOut = 1.0f;
	}
}

// Fait progresser l'effet de superposition au premier plan.

void CCamera::OverFrame(const Event &event)
{
	D3DCOLORVALUE	color;
	float			intensity;

	if ( m_type == CAMERA_INFO  ||
		 m_type == CAMERA_VISIT )  return;

	if ( m_overType == OE_NULL )
	{
		return;
	}

	m_overTime += event.rTime;

	if ( m_overType == OE_BLITZ )
	{
		if ( rand()%2 == 0 )
		{
			color.r = m_overColor.r*m_overForce;
			color.g = m_overColor.g*m_overForce;
			color.b = m_overColor.b*m_overForce;
		}
		else
		{
			color.r = 0.0f;
			color.g = 0.0f;
			color.b = 0.0f;
		}
		color.a = 0.0f;
		m_engine->SetOverColor(RetColor(color), m_overMode);
	}
	else
	{
		if ( m_overFadeIn > 0.0f && m_overTime < m_overFadeIn )
		{
			intensity = m_overTime/m_overFadeIn;
			intensity *= m_overForce;

			if ( m_overMode == D3DSTATETCw )
			{
				color.r = 1.0f-(1.0f-m_overColor.r)*intensity;
				color.g = 1.0f-(1.0f-m_overColor.g)*intensity;
				color.b = 1.0f-(1.0f-m_overColor.b)*intensity;
			}
			else
			{
				color.r = m_overColor.r*intensity;
				color.g = m_overColor.g*intensity;
				color.b = m_overColor.b*intensity;

				color.r = 1.0f-(1.0f-color.r)*(1.0f-m_overColorBase.r);
				color.g = 1.0f-(1.0f-color.g)*(1.0f-m_overColorBase.g);
				color.b = 1.0f-(1.0f-color.b)*(1.0f-m_overColorBase.b);
			}
			color.a = 0.0f;
			m_engine->SetOverColor(RetColor(color), m_overMode);
		}
		else if ( m_overFadeOut > 0.0f && m_overTime-m_overFadeIn < m_overFadeOut )
		{
			intensity = 1.0f-(m_overTime-m_overFadeIn)/m_overFadeOut;
			intensity *= m_overForce;

			if ( m_overMode == D3DSTATETCw )
			{
				color.r = 1.0f-(1.0f-m_overColor.r)*intensity;
				color.g = 1.0f-(1.0f-m_overColor.g)*intensity;
				color.b = 1.0f-(1.0f-m_overColor.b)*intensity;
			}
			else
			{
				color.r = m_overColor.r*intensity;
				color.g = m_overColor.g*intensity;
				color.b = m_overColor.b*intensity;

				color.r = 1.0f-(1.0f-color.r)*(1.0f-m_overColorBase.r);
				color.g = 1.0f-(1.0f-color.g)*(1.0f-m_overColorBase.g);
				color.b = 1.0f-(1.0f-color.b)*(1.0f-m_overColorBase.b);
			}
			color.a = 0.0f;
			m_engine->SetOverColor(RetColor(color), m_overMode);
		}
	}

	if ( m_overTime >= m_overFadeIn+m_overFadeOut )
	{
		FlushOver();
		return;
	}
}



// Fixe le mouvement mou de la caméra.

void CCamera::FixCamera()
{
#if 0
	m_initDelay     = 0.0f;
	m_backSleepTime = 0.0f;
	m_backRotSpeed  = 0.0f;
	m_actualEye    = m_finalEye    = m_scriptEye;
	m_actualLookat = m_finalLookat = m_scriptLookat;
	SetViewTime(m_scriptEye, m_scriptLookat, 0.0f);
#else
	Event	event;

	m_initDelay     = 0.0f;
	m_backSleepTime = 0.0f;
	m_backRotSpeed  = 0.0f;

	ZeroMemory(&event, sizeof(Event));
	event.event = EVENT_FRAME;
	EventFrame(event);

	SetViewTime(m_finalEye, m_finalLookat, 100.0f);
#endif
}

// Spécifie l'emplacement et la direction du point de vue au moteur 3D.

void CCamera::SetViewTime(const D3DVECTOR &vEyePt,
						  const D3DVECTOR &vLookatPt,
						  float rTime)
{
	D3DVECTOR	vUpVec, eye, lookat;
	float		prog, dEye, dLook, h, margin;

	if ( m_type == CAMERA_INFO )
	{
		eye    = vEyePt;
		lookat = vLookatPt;
	}
	else
	{
		if ( m_initDelay > 0.0f )
		{
			m_initDelay -= rTime;
			if ( m_initDelay < 0.0f )  m_initDelay = 0.0f;
			rTime /= 1.0f+m_initDelay;
		}

		eye    = vEyePt;
		lookat = vLookatPt;
		if ( !IsCollision(eye, lookat) )
		{
			m_finalEye    = eye;
			m_finalLookat = lookat;
		}

		dEye = Length(m_finalEye, m_actualEye);
		if ( m_smooth == CS_NONE )  prog = dEye;
//?		if ( m_smooth == CS_NORM )  prog = powf(dEye, 1.5f)*rTime*0.5f;
		if ( m_smooth == CS_NORM )  prog = powf(dEye, 1.5f)*rTime*1.0f;
//?		if ( m_smooth == CS_NORM )  prog = powf(dEye, 1.5f)*rTime*2.0f;
		if ( m_smooth == CS_SOFT )  prog = powf(dEye, 1.0f)*rTime*3.0f;
		if ( m_smooth == CS_HARD )  prog = powf(dEye, 1.0f)*rTime*4.0f;
		if ( m_smooth == CS_SPEC )  prog = powf(dEye, 1.0f)*rTime*0.05f;
		if ( dEye == 0.0f )
		{
			m_actualEye = m_finalEye;
		}
		else
		{
			if ( prog > dEye )  prog = dEye;
			m_actualEye = (m_finalEye-m_actualEye)/dEye*prog + m_actualEye;
		}

		dLook = Length(m_finalLookat, m_actualLookat);
		if ( m_smooth == CS_NONE )  prog = dLook;
//?		if ( m_smooth == CS_NORM )  prog = powf(dLook, 1.5f)*rTime*2.0f;
		if ( m_smooth == CS_NORM )  prog = powf(dLook, 1.5f)*rTime*3.0f;
//?		if ( m_smooth == CS_NORM )  prog = powf(dLook, 1.5f)*rTime*5.0f;
		if ( m_smooth == CS_SOFT )  prog = powf(dLook, 1.0f)*rTime*3.0f;
		if ( m_smooth == CS_HARD )  prog = powf(dLook, 1.0f)*rTime*4.0f;
		if ( m_smooth == CS_SPEC )  prog = powf(dLook, 1.0f)*rTime*4.0f;
		if ( dLook == 0.0f )
		{
			m_actualLookat = m_finalLookat;
		}
		else
		{
			if ( prog > dLook )  prog = dLook;
			m_actualLookat = (m_finalLookat-m_actualLookat)/dLook*prog + m_actualLookat;
		}

		eye = m_effectOffset+m_actualEye;
		m_water->AdjustEye(eye);

		h = m_terrain->RetFloorLevel(eye);
		margin = 4.0f;
		if ( m_type == CAMERA_SCRIPT )  margin = -100.0f;
		if ( m_type == CAMERA_FREE   )  margin = -100.0f;
		if ( eye.y < h+margin )
		{
			eye.y = h+margin;
		}

		lookat = m_effectOffset+m_actualLookat;

		if ( dEye < 5.0f && dLook < 5.0f )
		{
			m_smooth = m_smoothNext;
		}
	}

	vUpVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
	SetViewParams(eye, lookat, vUpVec);
}


// Evite les obstacles.

BOOL CCamera::IsCollision(D3DVECTOR &eye, D3DVECTOR lookat)
{
	if ( m_type == CAMERA_BACK )  return IsCollisionBack(eye, lookat);
	if ( m_type == CAMERA_FIX  )  return IsCollisionFix(eye, lookat);
	return FALSE;
}

// Evite les obstacles.

BOOL CCamera::IsCollisionBack(D3DVECTOR &eye, D3DVECTOR lookat)
{
	CObject		*pObj;
	D3DVECTOR	oPos, min, max, proj;
	ObjectType	oType;
	float		oRadius, dpp, del, len;
	int			i;

	min.x = Min(m_actualEye.x, m_actualLookat.x);
	min.y = Min(m_actualEye.y, m_actualLookat.y);
	min.z = Min(m_actualEye.z, m_actualLookat.z);

	max.x = Max(m_actualEye.x, m_actualLookat.x);
	max.y = Max(m_actualEye.y, m_actualLookat.y);
	max.z = Max(m_actualEye.z, m_actualLookat.z);

	m_bTransparency = FALSE;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetTruck() )  continue;  // pile ou fret?
		if ( pObj->RetGhost() )  continue;

		SetTransparency(pObj, 0.0f);  // objet opaque

		if ( pObj == m_cameraObj )  continue;

		oType = pObj->RetType();
		if ( oType == OBJECT_CAR    ||
			 oType == OBJECT_HUMAN  ||
			 oType == OBJECT_TECH   ||
			 oType == OBJECT_FIX    ||
			 oType == OBJECT_FRET   )  continue;

		pObj->GetGlobalSphere(oPos, oRadius);
		if ( oRadius <= 2.0f )  continue;  // ignore les petits objets

		if ( oPos.x+oRadius < min.x ||
			 oPos.y+oRadius < min.y ||
			 oPos.z+oRadius < min.z ||
			 oPos.x-oRadius > max.x ||
			 oPos.y-oRadius > max.y ||
			 oPos.z-oRadius > max.z )  continue;

		proj = Projection(m_actualEye, m_actualLookat, oPos);
		dpp = Length(proj, oPos);
		if ( dpp > oRadius )  continue;

		del = Length(m_actualEye, m_actualLookat);
		len = Length(m_actualEye, proj);
		if ( len > del )  continue;

		if ( !pObj->IsOccludeCamera(m_actualEye, m_actualLookat) )  continue;

		SetTransparency(pObj, 1.0f);  // objet transparent
		m_bTransparency = TRUE;
	}
	return FALSE;
}

// Evite les obstacles.

BOOL CCamera::IsCollisionFix(D3DVECTOR &eye, D3DVECTOR lookat)
{
	CObject		*pObj;
	D3DVECTOR	oPos, proj;
	ObjectType	type;
	float		oRadius, dist;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_cameraObj )  continue;

		type = pObj->RetType();
		if ( type == OBJECT_FRET    ||
			 type == OBJECT_STONE   ||
			 type == OBJECT_URANIUM ||
			 type == OBJECT_METAL   ||
			 type == OBJECT_BARREL  ||
			 type == OBJECT_BARRELa ||
			 type == OBJECT_ATOMIC  ||
			 type == OBJECT_BULLET  ||
			 type == OBJECT_BBOX    ||
			 type == OBJECT_KEYa    ||
			 type == OBJECT_KEYb    ||
			 type == OBJECT_KEYc    ||
			 type == OBJECT_KEYd    ||
			 type == OBJECT_BOT1    ||
			 type == OBJECT_BOT2    ||
			 type == OBJECT_BOT3    ||
			 type == OBJECT_BOT4    ||
			 type == OBJECT_BOT5    )  continue;

		pObj->GetGlobalSphere(oPos, oRadius);
		if ( oRadius == 0.0f )  continue;

		dist = Length(eye, oPos);
		if ( dist < oRadius )
		{
			dist = Length(eye, lookat);
			proj = Projection(eye, lookat, oPos);
			eye = (lookat-eye)*oRadius/dist + proj;
			return FALSE;
		}
	}
	return FALSE;
}


// Gestion d'un événement.

BOOL CCamera::EventProcess(const Event &event)
{
	switch( event.event )
	{
		case EVENT_FRAME:
			EventFrame(event);
			break;

#if 0
		case EVENT_RBUTTONDOWN:
			m_bRightDown = TRUE;
			m_rightPosInit = event.pos;
			m_rightPosCenter = FPOINT(0.5f, 0.5f);
			m_engine->MoveMousePos(m_rightPosCenter);
//?			m_engine->SetMouseHide(TRUE);  // cache la souris
			break;

		case EVENT_RBUTTONUP:
			m_bRightDown = FALSE;
			m_engine->MoveMousePos(m_rightPosInit);
//?			m_engine->SetMouseHide(FALSE);  // remontre la souris
			m_addDirectionH = 0.0f;
			m_addDirectionV = -PI*0.05f;
			break;
#endif

		case EVENT_MOUSEMOVE:
			EventMouseMove(event);
			break;

		case EVENT_KEYDOWN:
			if ( event.param == VK_WHEELUP   )  EventMouseWheel(+1);
			if ( event.param == VK_WHEELDOWN )  EventMouseWheel(-1);
			break;
	}
	return TRUE;
}

// Fait évoluer la caméra selon la souris déplacée.

BOOL CCamera::EventMouseMove(const Event &event)
{
	m_mousePos = event.pos;
	return TRUE;
}

// Molette souris actionnée.

void CCamera::EventMouseWheel(int dir)
{
	if ( m_type == CAMERA_BACK )
	{
//?		if ( dir > 0 )
//?		{
//?			m_backDist -= 8.0f;
//?			if ( m_backDist < m_backMin )  m_backDist = m_backMin;
//?		}
//?		if ( dir < 0 )
//?		{
//?			m_backDist += 8.0f;
//?			if ( m_backDist > 200.0f )  m_backDist = 200.0f;
//?		}
	}

	if ( m_type == CAMERA_FIX )
	{
		if ( dir > 0 )
		{
			m_fixDist -= 8.0f;
			if ( m_fixDist < 10.0f )  m_fixDist = 10.0f;
		}
		if ( dir < 0 )
		{
			m_fixDist += 8.0f;
			if ( m_fixDist > 200.0f )  m_fixDist = 200.0f;
		}
	}

	if ( m_type == CAMERA_VISIT )
	{
		if ( dir > 0 )
		{
			m_visitDist -= 8.0f;
			if ( m_visitDist < 20.0f )  m_visitDist = 20.0f;
		}
		if ( dir < 0 )
		{
			m_visitDist += 8.0f;
			if ( m_visitDist > 200.0f )  m_visitDist = 200.0f;
		}
	}
}

// Fait évoluer la caméra selon le temps écoulé.

BOOL CCamera::EventFrame(const Event &event)
{
	if ( m_type != CAMERA_FREE && m_engine->RetPause() )  return TRUE;

	EffectFrame(event);
	OverFrame(event);

	if ( m_type == CAMERA_FREE )
	{
		return EventFrameFree(event);
	}
	if ( m_type == CAMERA_EDIT )
	{
		return EventFrameEdit(event);
	}
	if ( m_type == CAMERA_DIALOG )
	{
		return EventFrameDialog(event);
	}
	if ( m_type == CAMERA_BACK )
	{
		return EventFrameBack(event);
	}
	if ( m_type == CAMERA_FIX )
	{
		return EventFrameFix(event);
	}
	if ( m_type == CAMERA_EXPLO )
	{
		return EventFrameExplo(event);
	}
	if ( m_type == CAMERA_ONBOARD )
	{
		return EventFrameOnBoard(event);
	}
	if ( m_type == CAMERA_SCRIPT )
	{
		return EventFrameScript(event);
	}
	if ( m_type == CAMERA_INFO )
	{
		return EventFrameInfo(event);
	}
	if ( m_type == CAMERA_VISIT )
	{
		return EventFrameVisit(event);
	}

	return TRUE;
}



// Déplace le point de vue.

BOOL CCamera::EventFrameFree(const Event &event)
{
	D3DVECTOR	pos, vLookatPt;
	float		factor;

	factor = m_heightEye*0.5f+30.0f;

	if ( m_mouseDirH != 0.0f )
	{
		m_directionH -= m_mouseDirH*event.rTime*0.7f*m_speed;
	}
	if ( m_mouseDirV != 0.0f )
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, m_mouseDirV*event.rTime*factor*m_speed);
	}

	// Up/Down.
	m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, event.axeY*event.rTime*factor*m_speed*1.0f);

	// Left/Right.
	if ( event.keyState & KS_CONTROL )
	{
		if ( event.axeX < 0.0f )
		{
			m_eyePt = LookatPoint(m_eyePt, m_directionH+PI/2.0f, m_directionV, -event.axeX*event.rTime*factor*m_speed);
		}
		if ( event.axeX > 0.0f )
		{
			m_eyePt = LookatPoint(m_eyePt, m_directionH-PI/2.0f, m_directionV, event.axeX*event.rTime*factor*m_speed);
		}
	}
	else
	{
		m_directionH -= event.axeX*event.rTime*0.7f*m_speed;
	}

	// +/-.
	if ( event.keyState & KS_NUMMINUS )
	{
		if ( m_heightEye < 500.0f )
		{
			m_heightEye += event.rTime*factor*m_speed*0.1f;
		}
	}
	if ( event.keyState & KS_NUMPLUS )
	{
		if ( m_heightEye > -20.0f )
		{
			m_heightEye -= event.rTime*factor*m_speed*0.1f;
		}
	}

	// PageUp/PageDown.
	if ( event.keyState & KS_PAGEUP )
	{
		if ( m_heightShift < 50.0f )
		{
			m_heightShift += event.rTime*m_speed*5.0f;
		}
	}
	if ( event.keyState & KS_PAGEDOWN )
	{
		if ( m_heightShift > -50.0f )
		{
			m_heightShift -= event.rTime*m_speed*5.0f;
		}
	}

	m_terrain->ValidPosition(m_eyePt, 10.0f);

	m_eyePt.y += 20.0f;
	if ( m_terrain->MoveOnFloor(m_eyePt, TRUE) )
	{
		m_eyePt.y += m_heightEye;

		pos = m_eyePt;
		if ( m_terrain->MoveOnFloor(pos, TRUE) )
		{
			pos.y -= 2.0f;
			if ( m_eyePt.y < pos.y )
			{
				m_eyePt.y = pos.y;
			}
		}
	}
	m_eyePt.y -= 20.0f;

	vLookatPt = LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

	if ( m_terrain->MoveOnFloor(vLookatPt, TRUE) )
	{
		vLookatPt.y += m_heightLookat;
	}

	vLookatPt.y += m_heightShift;

	SetViewTime(m_eyePt, vLookatPt, event.rTime);
	
	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameEdit(const Event &event)
{
	D3DVECTOR	pos, vLookatPt;
	float		factor;

	factor = m_editHeight*0.5f+30.0f;

	if ( m_mouseDirH != 0.0f )
	{
		m_directionH -= m_mouseDirH*event.rTime*0.7f*m_speed;
	}
	if ( m_mouseDirV != 0.0f )
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, m_mouseDirV*event.rTime*factor*m_speed);
	}

	m_terrain->ValidPosition(m_eyePt, 10.0f);
	
	if ( m_terrain->MoveOnFloor(m_eyePt, FALSE) )
	{
		m_eyePt.y += m_editHeight;

		pos = m_eyePt;
		if ( m_terrain->MoveOnFloor(pos, FALSE) )
		{
			pos.y += 2.0f;
			if ( m_eyePt.y < pos.y )
			{
				m_eyePt.y = pos.y;
			}
		}

	}

	vLookatPt = LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

	if ( m_terrain->MoveOnFloor(vLookatPt, TRUE) )
	{
		vLookatPt.y += m_heightLookat;
	}

	SetViewTime(m_eyePt, vLookatPt, event.rTime);
	
	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameDialog(const Event &event)
{
	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameBack(const Event &event)
{
	D3DMATRIX*	mat;
	CPhysics*	physics;
	ObjectType	type;
	D3DVECTOR	pos, vLookatPt;
	FPOINT		mouse;
	float		centeringH, centeringV, centeringD, h, v, d, floor;

	if ( m_cameraObj == 0 )
	{
		type = OBJECT_NULL;
	}
	else
	{
		type = m_cameraObj->RetType();
	}

	// +/-.
//?	if ( event.keyState & KS_NUMPLUS )
//?	{
//?		m_backDist -= event.rTime*30.0f*m_speed;
//?		if ( m_backDist < m_backMin )  m_backDist = m_backMin;
//?	}
//?	if ( event.keyState & KS_NUMMINUS )
//?	{
//?		m_backDist += event.rTime*30.0f*m_speed;
//?		if ( m_backDist > 200.0f )  m_backDist = 200.0f;
//?	}

	if ( Abs(m_backMotorSpeed) <= 0.1f &&  // véhicule arrêté ?
		 !m_bBackLockRotate )
	{
		m_backSleepTime += event.rTime;
	}
	else
	{
		m_backSleepTime = 0.0f;
		m_backRotSpeed  = 0.0f;
	}
	if ( type != OBJECT_CAR    &&
		 type != OBJECT_HUMAN  &&
		 type != OBJECT_BOT1   &&
		 type != OBJECT_BOT2   &&
		 type != OBJECT_BOT3   &&
		 type != OBJECT_BOT4   &&
		 type != OBJECT_BOT5   &&
		 type != OBJECT_WALKER &&
		 type != OBJECT_CRAZY  )
	{
		m_backSleepTime = 0.0f;
		m_backRotSpeed  = 0.0f;
	}
	if ( m_centeringPhase != CP_NULL )
	{
		m_backSleepTime = 0.0f;
		m_backRotSpeed  = 0.0f;
	}

	// Fait tourner la caméra après un certain temps d'arrêt.
	if ( m_backSleepTime < 5.0f )
	{
		m_rotDirectionH = Smooth(m_rotDirectionH, 0.0f, event.rTime*5.0f);
	}
	else
	{
		m_backRotSpeed = Smooth(m_backRotSpeed, 1.0f, event.rTime);
		m_rotDirectionH -= event.rTime*m_backRotSpeed*0.5f;
		m_rotDirectionH = NormAngle(m_rotDirectionH);
		if ( m_rotDirectionH > PI )  m_rotDirectionH -= PI*2.0f;
	}

	if ( m_mouseDirH != 0 || m_mouseDirV != 0 )
	{
		AbortCentering();  // stoppe cadrage spécial
	}

	// Progression du cadrage spécial.
	centeringH = 0.0f;
	centeringV = 0.0f;
	centeringD = 0.0f;

	if ( m_centeringPhase == CP_START )
	{
		m_centeringProgress += event.rTime/m_centeringTime;
		if ( m_centeringProgress > 1.0f )  m_centeringProgress = 1.0f;
		centeringH = m_centeringProgress;
		centeringV = m_centeringProgress;
		centeringD = m_centeringProgress;
		if ( m_centeringProgress >= 1.0f )
		{
			m_centeringPhase = CP_WAIT;
		}
	}

	if ( m_centeringPhase == CP_WAIT )
	{
		centeringH = 1.0f;
		centeringV = 1.0f;
		centeringD = 1.0f;
	}

	if ( m_centeringPhase == CP_STOP )
	{
		m_centeringProgress += event.rTime/m_centeringTime;
		if ( m_centeringProgress > 1.0f )  m_centeringProgress = 1.0f;
		centeringH = 1.0f-m_centeringProgress;
		centeringV = 1.0f-m_centeringProgress;
		centeringD = 1.0f-m_centeringProgress;
		if ( m_centeringProgress >= 1.0f )
		{
			m_centeringPhase = CP_NULL;
		}
	}

	if ( m_centeringAngleH == 99.9f )  centeringH = 0.0f;
	if ( m_centeringAngleV == 99.9f )  centeringV = 0.0f;
	if ( m_centeringDist   ==  0.0f )  centeringD = 0.0f;

	if ( m_cameraObj != 0 )
	{
		vLookatPt = m_cameraObj->RetPosition(0);
		     if ( type == OBJECT_HUMAN   )  vLookatPt.y +=  1.0f;
		else if ( type == OBJECT_TECH    )  vLookatPt.y +=  1.0f;
		else if ( type == OBJECT_STARTER )  vLookatPt.y +=  1.5f;
		else                                vLookatPt.y +=  4.0f;

		if ( type == OBJECT_DOCK )
		{
			mat = m_cameraObj->RetWorldMatrix(0);
			vLookatPt = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 27.0f));
		}

		h = -m_cameraObj->RetAngleY(0);  // angle véhicule/batiment

		if ( type == OBJECT_FACTORY1  ||
			 type == OBJECT_FACTORY2  ||
			 type == OBJECT_FACTORY3  ||
			 type == OBJECT_FACTORY4  ||
			 type == OBJECT_FACTORY5  ||
			 type == OBJECT_FACTORY6  ||
			 type == OBJECT_FACTORY7  ||
			 type == OBJECT_FACTORY8  ||
			 type == OBJECT_FACTORY9  ||
			 type == OBJECT_FACTORY10 ||
			 type == OBJECT_ALIEN1    ||
			 type == OBJECT_ALIEN2    ||
			 type == OBJECT_ALIEN3    ||
			 type == OBJECT_ALIEN4    ||
			 type == OBJECT_ALIEN5    ||
			 type == OBJECT_ALIEN6    ||
			 type == OBJECT_ALIEN7    ||
			 type == OBJECT_ALIEN8    ||
			 type == OBJECT_ALIEN9    ||
			 type == OBJECT_ALIEN10   ||
			 type == OBJECT_INCA1     ||
			 type == OBJECT_INCA2     ||
			 type == OBJECT_INCA3     ||
			 type == OBJECT_INCA4     ||
			 type == OBJECT_INCA5     ||
			 type == OBJECT_INCA6     ||
			 type == OBJECT_INCA7     ||
			 type == OBJECT_INCA8     ||
			 type == OBJECT_INCA9     ||
			 type == OBJECT_INCA10    ||
			 type == OBJECT_BUILDING1 ||
			 type == OBJECT_BUILDING2 ||
			 type == OBJECT_BUILDING3 ||
			 type == OBJECT_BUILDING4 ||
			 type == OBJECT_BUILDING5 ||
			 type == OBJECT_BUILDING6 ||
			 type == OBJECT_BUILDING7 ||
			 type == OBJECT_BUILDING8 ||
			 type == OBJECT_BUILDING9 ||
			 type == OBJECT_BUILDING10||
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
			 type == OBJECT_START     ||
			 type == OBJECT_END       )  // batiment ?
		{
			h += PI*0.20f;  // presque de face
		}
		else if ( type == OBJECT_DOCK )
		{
			h += -PI*0.45f;  // zouli
		}
		else if ( type == OBJECT_STARTER )
		{
			h += 0.0f;  // de face
		}
		else	// véhicule ?
		{
			h += PI;  // de dos
		}
		h = NormAngle(h);
		v = 0.0f;  //?

		h += m_centeringCurrentH;
		h += m_addDirectionH*(1.0f-centeringH);
		h += m_rotDirectionH*(1.0f-centeringH);
		h = NormAngle(h);

		v += m_centeringCurrentV;
		v += m_addDirectionV*(1.0f-centeringV);

		d = m_backDist;
		d += m_centeringDist*centeringD;

		m_centeringCurrentH = m_centeringAngleH*centeringH;
		m_centeringCurrentV = m_centeringAngleV*centeringV;

		m_eyePt = RotateView(vLookatPt, h, v, d);

		physics = m_cameraObj->RetPhysics();
		if ( physics != 0 && physics->RetLand() )  // au sol ?
		{
			pos = vLookatPt+(vLookatPt-m_eyePt);
			floor = m_terrain->RetFloorHeight(pos)-4.0f;
			if ( floor > 0.0f )
			{
				if ( floor > 10.0f )  floor = 10.0f;
				m_eyePt.y += floor;  // montre la descente devant
			}
		}

		m_eyePt = ExcludeTerrain(m_eyePt, vLookatPt, h, v);
		m_eyePt = ExcludeObject(m_eyePt, vLookatPt, h, v);

		SetViewTime(m_eyePt, vLookatPt, event.rTime);

		m_directionH = h+PI/2.0f;
		m_directionV = v;
	}

	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameFix(const Event &event)
{
	D3DVECTOR	pos, vLookatPt;
	float		h, v;

	// +/-.
	if ( event.keyState & KS_NUMPLUS )
	{
		m_fixDist -= event.rTime*30.0f*m_speed;
		if ( m_fixDist < 10.0f )  m_fixDist = 10.0f;
	}
	if ( event.keyState & KS_NUMMINUS )
	{
		m_fixDist += event.rTime*30.0f*m_speed;
		if ( m_fixDist > 200.0f )  m_fixDist = 200.0f;
	}

	if ( m_mouseDirH != 0 || m_mouseDirV != 0 )
	{
		AbortCentering();  // stoppe cadrage spécial
	}

	if ( m_cameraObj != 0 )
	{
		vLookatPt = m_cameraObj->RetPosition(0);

		h = m_fixDirectionH;
		v = m_fixDirectionV;

		m_eyePt = RotateView(vLookatPt, h, v, m_fixDist);
		m_eyePt = ExcludeTerrain(m_eyePt, vLookatPt, h, v);
		m_eyePt = ExcludeObject(m_eyePt, vLookatPt, h, v);

		SetViewTime(m_eyePt, vLookatPt, event.rTime);

		m_directionH = h+PI/2.0f;
		m_directionV = v;
	}

	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameExplo(const Event &event)
{
	D3DVECTOR	pos, vLookatPt;
	float		factor;

	factor = m_heightEye*0.5f+30.0f;

	if ( m_mouseDirH != 0.0f )
	{
		m_directionH -= m_mouseDirH*event.rTime*0.7f*m_speed;
	}

	m_terrain->ValidPosition(m_eyePt, 10.0f);
	
	if ( m_terrain->MoveOnFloor(m_eyePt, FALSE) )
	{
		m_eyePt.y += m_heightEye;

		pos = m_eyePt;
		if ( m_terrain->MoveOnFloor(pos, FALSE) )
		{
			pos.y += 2.0f;
			if ( m_eyePt.y < pos.y )
			{
				m_eyePt.y = pos.y;
			}
		}

	}

	vLookatPt = LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

	if ( m_terrain->MoveOnFloor(vLookatPt, TRUE) )
	{
		vLookatPt.y += m_heightLookat;
	}

	SetViewTime(m_eyePt, vLookatPt, event.rTime);
	
	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameOnBoard(const Event &event)
{
	D3DVECTOR	vLookatPt, vUpVec, eye, lookat, pos;

	if ( m_cameraObj != 0 )
	{
		m_cameraObj->SetViewFromHere(m_eyePt, m_directionH, m_directionV,
									 vLookatPt, vUpVec, m_type);
		eye    = m_effectOffset*0.1f+m_eyePt;
		lookat = m_effectOffset*0.1f+vLookatPt;

		SetViewParams(eye, lookat, vUpVec);
		m_actualEye    = eye;
		m_actualLookat = lookat;
	}
	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameInfo(const Event &event)
{
	SetViewTime(D3DVECTOR(0.0f, 0.0f, 0.0f),
				D3DVECTOR(0.0f, 0.0f, 1.0f),
				event.rTime);
	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameVisit(const Event &event)
{
	D3DVECTOR	eye;
	float		angleH, angleV;

	m_visitTime += event.rTime;

	// +/-.
	if ( event.keyState & KS_NUMPLUS )
	{
		m_visitDist -= event.rTime*50.0f*m_speed;
		if ( m_visitDist < 20.0f )  m_visitDist = 20.0f;
	}
	if ( event.keyState & KS_NUMMINUS )
	{
		m_visitDist += event.rTime*50.0f*m_speed;
		if ( m_visitDist > 200.0f )  m_visitDist = 200.0f;
	}

	// PageUp/Down.
	if ( event.keyState & KS_PAGEUP )
	{
		m_visitDirectionV -= event.rTime*1.0f*m_speed;
		if ( m_visitDirectionV < -PI*0.40f )  m_visitDirectionV = -PI*0.40f;
	}
	if ( event.keyState & KS_PAGEDOWN )
	{
		m_visitDirectionV += event.rTime*1.0f*m_speed;
		if ( m_visitDirectionV > 0.0f )  m_visitDirectionV = 0.0f;
	}

	angleH = -(m_visitTime/10.0f)*(PI*2.0f);
	angleV = m_visitDirectionV;
	eye = RotateView(m_visitGoal, angleH, angleV, m_visitDist);
	eye = ExcludeTerrain(eye, m_visitGoal, angleH, angleV);
	eye = ExcludeObject(eye, m_visitGoal, angleH, angleV);
	SetViewTime(eye, m_visitGoal, event.rTime);

	return TRUE;
}

// Déplace le point de vue.

BOOL CCamera::EventFrameScript(const Event &event)
{
	SetViewTime(m_scriptEye+m_effectOffset,
				m_scriptLookat+m_effectOffset, event.rTime);
	return TRUE;
}

void CCamera::SetScriptEye(D3DVECTOR eye)
{
	m_scriptEye = eye;
}

void CCamera::SetScriptLookat(D3DVECTOR lookat)
{
	m_scriptLookat = lookat;
}


// Spécifie l'emplacement et la direction du point de vue.

void CCamera::SetViewParams(const D3DVECTOR &eye, const D3DVECTOR &lookat,
							const D3DVECTOR &up)
{
	BOOL		bUnder;

	m_engine->SetViewParams(eye, lookat, up, m_eyeDistance);

	bUnder = (eye.y < m_water->RetLevel());  // est-on sous l'eau ?
	if ( m_type == CAMERA_INFO )  bUnder = FALSE;
	m_engine->SetRankView(bUnder?1:0);
}


// Adapte la caméra pour ne pas entrer dans le terrain.

D3DVECTOR CCamera::ExcludeTerrain(D3DVECTOR eye, D3DVECTOR lookat,
								  float &angleH, float &angleV)
{
	D3DVECTOR	pos;
	float		dist;

	pos = eye;
	if ( m_terrain->MoveOnFloor(pos) )
	{
		dist = Length2d(lookat, pos);
		pos.y += 2.0f+dist*0.1f;
		if ( pos.y > eye.y )
		{
			angleV = -RotateAngle(dist, pos.y-lookat.y);
			eye = RotateView(lookat, angleH, angleV, dist);
		}
	}
	return eye;
}

// Adapte la caméra pour ne pas pénétrer dans un objet.

D3DVECTOR CCamera::ExcludeObject(D3DVECTOR eye, D3DVECTOR lookat,
								 float &angleH, float &angleV)
{
	CObject*	pObj;
	D3DVECTOR	oPos;
	float		oRad, dist;
	int			i, j;

return eye;
//?
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRad) )
		{
			dist = Length(oPos, eye);
			if ( dist < oRad+2.0f )
			{
				eye.y = oPos.y+oRad+2.0f;
			}
		}
	}

	return eye;
}


