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
#include "robotmain.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "camera.h"




// Constructeur de l'objet.

CCamera::CCamera(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_CAMERA, this);

	m_engine  = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water   = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_main    = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);

	m_type       = CAMERA_FREE;
	m_smooth     = CS_NORM;
	m_smoothNext = CS_NORM;
	m_cameraObj  = 0;

	m_eyeDistance = 10.0f;

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
	m_lookatPt     = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_directionH   =  0.0f;
	m_directionV   =  0.0f;
	m_rotH         =  0.0f;
	m_distance     = FREE_DIST;
	m_heightEye    = 50.0f;
	m_heightShift  =  0.0f;
	m_heightLookat =  0.0f;
	m_speed        =  2.0f;
	m_scrollMouse  = FPOINT(0.0f, 0.0f);
	m_bFinalScroll = FALSE;

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

	m_overType = OE_NULL;
}

// Destructeur de l'objet.

CCamera::~CCamera()
{
}


// Indique la consigne de vitesse moteur avant/arrière.

void CCamera::SetMotorSpeed(float speed)
{
	m_backMotorSpeed = speed;
}



// Initialise la caméra.

void CCamera::Init(D3DVECTOR lookat, float dirH, float dirV, float dist)
{
	D3DVECTOR	vUpVec;

	m_type = CAMERA_FREE;
	m_directionH = dirH;
	m_directionV = dirV;
	m_rotH = 0.0f;
	m_distance = dist;
	m_lookatPt = lookat;
	m_lookatPt.y = 0.0f;
	m_eyePt = RotateView(m_lookatPt, m_directionH+PI/2.0f, m_directionV-PI, m_distance);
	m_actualEye = m_eyePt;
	m_actualLookat = m_lookatPt;
	m_finalEye = m_actualEye;
	m_finalLookat = m_actualLookat;
	m_scriptEye = m_actualEye;
	m_scriptLookat = m_actualLookat;
	m_scrollMouse  = FPOINT(0.0f, 0.0f);
	m_bFinalScroll = FALSE;

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
	m_focus = 1.00f;

	FlushEffect();
	FlushOver();
	SetType(CAMERA_FREE);
}


// Spécifie le point visé.

void CCamera::SetLookat(D3DVECTOR lookat)
{
	m_lookatPt = lookat;
	m_lookatPt.y = 0.0f;
}

// Spécifie la direction horizontale.

void CCamera::SetDirH(float dirH)
{
	m_directionH = dirH;
}

// Spécifie la direction verticale.

void CCamera::SetDirV(float dirV)
{
	m_directionV = dirV;
}

// Spécifie l'éloignement de la caméra.

void CCamera::SetDistance(float dist)
{
	m_distance = dist;
}

// Retourne le point de vue de la caméra.

void CCamera::RetCamera(D3DVECTOR &eye, D3DVECTOR &lookat,
						float &dirH, float &dirV, float &dist)
{
	eye = m_eyePt;
	lookat = m_lookatPt;
	dirH = m_directionH;
	dirV = m_directionV;
	dist = m_distance;
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

	if ( type == CAMERA_VISIT )  // xx -> info ?
	{
		m_normEye    = m_engine->RetEyePt();
		m_normLookat = m_engine->RetLookatPt();

		m_engine->SetFocus(1.00f);  // normal
		m_type = type;
		return;
	}

	if ( m_type == CAMERA_VISIT )  // info -> xx ?
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

	if ( m_type == CAMERA_ONBOARD && type == CAMERA_FREE )  // onboard -> free ?
	{
		m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);
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
		m_backMin = m_backDist/3.0f;
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


// Gestion du scroll avec le bouton droite de la souris.

void CCamera::SetScrollMouse(FPOINT mouse, BOOL bFinal)
{
	if ( m_engine->RetSetup(ST_MOUSESCROLL) == 0.0f )
	{
		mouse = FPOINT(0.0f, 0.0f);
	}
	else
	{
		mouse.x *= 200.0f;
		mouse.y *= 200.0f;
		mouse.x *= 0.5f+m_engine->RetSetup(ST_SPEEDSCH);
		mouse.y *= 0.5f+m_engine->RetSetup(ST_SPEEDSCV);
		if ( m_engine->RetSetup(ST_INVSCH) != 0.0f )  mouse.x *= -1.0f;
		if ( m_engine->RetSetup(ST_INVSCV) != 0.0f )  mouse.y *= -1.0f;
	}
	m_scrollMouse = mouse;
	m_bFinalScroll = bFinal;
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
	if ( m_type == CAMERA_VISIT )  return;

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
	if ( m_engine->RetSetup(ST_EXPLOVIB) == 0.0f )  return;

	m_effectType     = effect;
	m_effectPos      = pos;
	m_effectForce    = force;
	m_effectProgress = 0.0f;
}

// Fait progresser l'effet de la caméra.

void CCamera::EffectFrame(const Event &event)
{
	float		dist, force;

	if ( m_type == CAMERA_VISIT )  return;

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
	if ( m_overType == OE_NULL )  return;

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

	if ( m_type == CAMERA_VISIT )  return;

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
#if 1
	m_backSleepTime = 0.0f;
	m_backRotSpeed  = 0.0f;
	m_actualEye    = m_finalEye    = m_scriptEye;
	m_actualLookat = m_finalLookat = m_scriptLookat;
	SetViewTime(m_scriptEye, m_scriptLookat, 0.0f);
#else
	Event	event;

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

	if ( m_type == CAMERA_FREE )
	{
		eye    = vEyePt;
		lookat = vLookatPt;
		if ( !IsCollision(eye, lookat) )
		{
			m_finalEye    = eye;
			m_finalLookat = lookat;
		}

		dEye = Length(m_finalEye, m_actualEye);
		prog = powf(dEye, 1.5f)*rTime*3.0f;
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
		prog = powf(dLook, 1.5f)*rTime*3.0f;
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
//?		m_water->AdjustEye(eye);

		lookat = m_effectOffset+m_actualLookat;

		if ( dEye < 5.0f && dLook < 5.0f )
		{
			m_smooth = m_smoothNext;
		}
	}
	else
	{
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
//?		m_water->AdjustEye(eye);

		h = m_terrain->RetFloorLevel(eye);
		margin = 4.0f;
		if ( m_type == CAMERA_SCRIPT )  margin = -100.0f;
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
	return FALSE;
}

// Evite les obstacles.

BOOL CCamera::IsCollisionFix(D3DVECTOR &eye, D3DVECTOR lookat)
{
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

		case EVENT_MOUSEMOVE:
			EventMouseMove(event);
			break;

		case EVENT_KEYDOWN:
			if ( event.param == m_engine->RetKey(KEYRANK_ROTCW, 0) ||
				 event.param == m_engine->RetKey(KEYRANK_ROTCW, 1) )
			{
				EventMouseWheel(+1, event.param, TRUE);
			}
			if ( event.param == m_engine->RetKey(KEYRANK_ROTCCW, 0) ||
				 event.param == m_engine->RetKey(KEYRANK_ROTCCW, 1) )
			{
				EventMouseWheel(-1, event.param, TRUE);
			}
			break;

		case EVENT_KEYUP:
			if ( event.param == m_engine->RetKey(KEYRANK_ROTCW, 0) ||
				 event.param == m_engine->RetKey(KEYRANK_ROTCW, 1) )
			{
				EventMouseWheel(+1, event.param, FALSE);
			}
			if ( event.param == m_engine->RetKey(KEYRANK_ROTCCW, 0) ||
				 event.param == m_engine->RetKey(KEYRANK_ROTCCW, 1) )
			{
				EventMouseWheel(-1, event.param, FALSE);
			}
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

void CCamera::EventMouseWheel(int dir, long param, BOOL bDown)
{
	if ( m_type == CAMERA_FREE )
	{
		if ( param == VK_WHEELUP   ||
			 param == VK_WHEELDOWN )
		{
			m_directionH += 15.0f*PI/180.0f*dir;
		}
		else
		{
			m_rotH = bDown?(float)dir:0.0f;
		}

		m_main->MouseMoveClear();
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
	if ( m_type == CAMERA_ONBOARD )
	{
		return EventFrameOnBoard(event);
	}
	if ( m_type == CAMERA_SCRIPT )
	{
		return EventFrameScript(event);
	}
	if ( m_type == CAMERA_VISIT )
	{
		return EventFrameVisit(event);
	}

	return TRUE;
}


// Retourne le sprite par défaut à utiliser pour la souris.

D3DMouse CCamera::RetMouseDef(FPOINT pos, D3DMouse def)
{
	D3DMouse	type;

	type = def;
	m_mousePos = pos;

	if ( m_engine->RetSetup(ST_MOUSESCROLL) == 0.0f )  return type;

	m_mouseDirH = 0.0f;
	m_mouseDirV = 0.0f;

	if ( pos.x < m_mouseMarging && pos.x >= 0.0f )
	{
		m_mouseDirH = pos.x/m_mouseMarging - 1.0f;
	}

	if ( pos.x > 1.0f-m_mouseMarging && pos.x <= 1.0f )
	{
		m_mouseDirH = 1.0f - (1.0f-pos.x)/m_mouseMarging;
	}

	if ( pos.y < m_mouseMarging && pos.y >= 0.0f )
	{
		m_mouseDirV = pos.y/m_mouseMarging - 1.0f;
	}

	if ( pos.y > 1.0f-m_mouseMarging && pos.y <= 1.0f )
	{
		m_mouseDirV = 1.0f - (1.0f-pos.y)/m_mouseMarging;
	}

	if ( m_type == CAMERA_FREE )
	{
		if ( m_mouseDirH > 0.0f )
		{
			type = D3DMOUSESCROLLR;
		}
		if ( m_mouseDirH < 0.0f )
		{
			type = D3DMOUSESCROLLL;
		}
		if ( m_mouseDirV > 0.0f )
		{
			type = D3DMOUSESCROLLU;
		}
		if ( m_mouseDirV < 0.0f )
		{
			type = D3DMOUSESCROLLD;
		}
	}

	if ( m_engine->RetSetup(ST_INVSCH) != 0.0f )  m_mouseDirH *= -1.0f;
	if ( m_engine->RetSetup(ST_INVSCV) != 0.0f )  m_mouseDirV *= -1.0f;

	return type;
}


// Déplace le point de vue.

BOOL CCamera::EventFrameFree(const Event &event)
{
	D3DVECTOR	pos, eye, lookat;
	FPOINT		rot;
	float		factor, level, speed;

	factor = 30.0f;

	m_directionH += m_rotH*event.rTime*2.0f;

	// Up/Down.
	if ( event.axeY != 0.0f )
	{
		if ( event.keyState & KS_CONTROL )
		{
			m_directionV += event.axeY*event.rTime*m_speed*0.7f;
#if 0
			if ( m_directionV <  0.0f*PI/180.0f )  m_directionV =  0.0f*PI/180.0f;
			if ( m_directionV > 85.0f*PI/180.0f )  m_directionV = 85.0f*PI/180.0f;
#else
			if ( m_directionV < 30.0f*PI/180.0f )  m_directionV = 30.0f*PI/180.0f;
			if ( m_directionV > 60.0f*PI/180.0f )  m_directionV = 60.0f*PI/180.0f;
#endif
		}
		else
		{
			speed = m_engine->RetSetup(ST_SPEEDSCV)*2.0f;
			m_lookatPt = LookatPoint(m_lookatPt, m_directionH, 0.0f, event.axeY*event.rTime*factor*m_speed*speed);
			m_terrain->ValidPosition(m_lookatPt, -8.0f*10);
			m_lookatPt.y = 0.0f;
		}
	}

	if ( m_mouseDirV != 0.0f )
	{
		speed = m_engine->RetSetup(ST_SPEEDSCV)*2.0f;
		m_lookatPt = LookatPoint(m_lookatPt, m_directionH, 0.0f, m_mouseDirV*speed);
		m_terrain->ValidPosition(m_lookatPt, -8.0f*10);
		m_lookatPt.y = 0.0f;
	}

	// Left/Right.
	if ( event.axeX != 0.0f )
	{
		if ( event.keyState & KS_CONTROL )
		{
			m_directionH -= event.axeX*event.rTime*m_speed*0.7f;
		}
		else
		{
			speed = m_engine->RetSetup(ST_SPEEDSCH)*3.0f;
			if ( event.axeX < 0.0f )
			{
				m_lookatPt = LookatPoint(m_lookatPt, m_directionH+PI/2.0f, 0.0f, -event.axeX*event.rTime*factor*m_speed*speed);
			}
			if ( event.axeX > 0.0f )
			{
				m_lookatPt = LookatPoint(m_lookatPt, m_directionH-PI/2.0f, 0.0f, event.axeX*event.rTime*factor*m_speed*speed);
			}
			m_terrain->ValidPosition(m_lookatPt, -8.0f*10);
			m_lookatPt.y = 0.0f;
		}
	}

	if ( m_mouseDirH != 0.0f )
	{
		speed = m_engine->RetSetup(ST_SPEEDSCH)*2.0f;
		m_lookatPt = LookatPoint(m_lookatPt, m_directionH+PI/2.0f, 0.0f, -m_mouseDirH*speed);
		m_terrain->ValidPosition(m_lookatPt, -8.0f*10);
		m_lookatPt.y = 0.0f;
	}

	m_eyePt = RotateView(m_lookatPt, m_directionH+PI/2.0f, m_directionV-PI, m_distance);

	level = m_terrain->RetFloorLevel(m_eyePt)+2.0f;
	if ( m_eyePt.y < level )  m_eyePt.y = level;

	eye = m_eyePt;
	lookat = m_lookatPt;

	if ( m_scrollMouse.x != 0.0f ||
		 m_scrollMouse.y != 0.0f )
	{
		rot = RotatePoint(m_directionH, m_scrollMouse);

		if ( m_bFinalScroll )
		{
			m_eyePt.x += rot.x;
			m_eyePt.z += rot.y;
			m_lookatPt.x += rot.x;
			m_lookatPt.z += rot.y;
			m_scrollMouse  = FPOINT(0.0f, 0.0f);
			m_bFinalScroll = FALSE;
		}

		eye.x += rot.x;
		eye.z += rot.y;
		lookat.x += rot.x;
		lookat.z += rot.y;
		m_terrain->ValidPosition(lookat, -8.0f*10);
	}

	SetViewTime(eye, lookat, event.rTime);
	
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
	ObjectType	type;
	D3DVECTOR	pos, vLookatPt;
	FPOINT		mouse;
	float		centeringH, centeringV, centeringD, h, v, d;

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
	if ( type != OBJECT_BLUPI  &&
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
		vLookatPt.y +=  4.0f;

		h = -m_cameraObj->RetAngleY(0);  // angle véhicule/batiment

		if ( type == OBJECT_BUILDING1 ||
			 type == OBJECT_BUILDING2 ||
			 type == OBJECT_BUILDING3 ||
			 type == OBJECT_BUILDING4 ||
			 type == OBJECT_BUILDING5 ||
			 type == OBJECT_BUILDING6 ||
			 type == OBJECT_BUILDING7 ||
			 type == OBJECT_BUILDING8 ||
			 type == OBJECT_BUILDING9 ||
			 type == OBJECT_BUILDING10||
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
//?	BOOL		bUnder;

	m_engine->SetViewParams(eye, lookat, up, m_eyeDistance);

//?	bUnder = (eye.y < m_water->RetLevel());  // est-on sous l'eau ?
//?	m_engine->SetRankView(bUnder?1:0);
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
	return eye;
}


