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

#ifndef _CAMERA_H_
#define	_CAMERA_H_


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CWater;
class CObject;
enum D3DMouse;


enum CameraType
{
	CAMERA_NULL		= 0,	// cam�ra ind�finie
	CAMERA_FREE		= 1,	// cam�ra libre (jamais en principe)
	CAMERA_EDIT		= 2,	// cam�ra pendant l'�dition d'un programme
	CAMERA_ONBOARD	= 3,	// cam�ra � bord d'un robot
	CAMERA_BACK		= 4,	// cam�ra derri�re un robot
	CAMERA_FIX		= 5,	// cam�ra fixe apr�s robot
	CAMERA_EXPLO	= 6,	// cam�ra immobile apr�s explosion
	CAMERA_SCRIPT	= 7,	// cam�ra pendant un film script�
	CAMERA_INFO		= 8,	// cam�ra pendant l'affichage des informations
	CAMERA_VISIT	= 9,	// visite du lieu d'une erreur
	CAMERA_DIALOG	= 10,	// cam�ra pendant dialogue
	CAMERA_PLANE	= 11,	// cam�ra fixe en hauteur
};

enum CameraSmooth
{
	CS_NONE			= 0,	// brusque
	CS_NORM			= 1,	// normal
	CS_HARD			= 2,	// dur
	CS_SPEC			= 3,	// sp�cial
};

enum CenteringPhase
{
	CP_NULL			= 0,
	CP_START		= 1,
	CP_WAIT			= 2,
	CP_STOP			= 3,
};

enum CameraEffect
{
	CE_NULL			= 0,	// pas d'effet
	CE_TERRAFORM	= 1,	// terrassement
	CE_CRASH		= 2,	// v�hicule volant pos� violemment
	CE_EXPLO		= 3,	// explosion
	CE_SHOT			= 4,	// coup non mortel
	CE_VIBRATION	= 5,	// vibration pendant construction
	CE_PET			= 6,	// rat� du r�acteur
};

enum OverEffect
{
	OE_NULL			= 0,	// pas d'effet
	OE_BLOOD		= 1,	// flash rouge
	OE_FADEINw		= 2,	// blanc -> rien
	OE_FADEOUTw		= 3,	// rien -> blanc
	OE_FADEOUTb		= 4,	// rien -> bleu
	OE_BLITZ		= 5,	// �clair
};



class CCamera
{
public:
	CCamera(CInstanceManager* iMan);
	~CCamera();

	BOOL		EventProcess(const Event &event);

	void		Init(D3DVECTOR eye, D3DVECTOR lookat, float delay);

	void		SetObject(CObject* object);
	CObject*	RetObject();

	void		SetType(CameraType type);
	CameraType	RetType();

	void		SetSmooth(CameraSmooth type);
	CameraSmooth RetSmoth();

	void		SetDist(float dist);
	float		RetDist();

	void		SetFixDirection(float angle);
	float		RetFixDirection();

	void		SetRemotePan(float value);
	float		RetRemotePan();

	void		SetRemoteZoom(float value);
	float		RetRemoteZoom();

	void		StartVisit(D3DVECTOR goal, float dist);
	void		StopVisit();

	void		RetCamera(D3DVECTOR &eye, D3DVECTOR &lookat);

	BOOL		StartCentering(CObject *object, float angleH, float angleV, float dist, float time);
	BOOL		StopCentering(CObject *object, float time);
	void		AbortCentering();

	void		FlushEffect();
	void		StartEffect(CameraEffect effect, D3DVECTOR pos, float force);

	void		FlushOver();
	void		SetOverBaseColor(D3DCOLORVALUE color);
	void		StartOver(OverEffect effect, D3DVECTOR pos, float force);

	void		FixCamera();
	void		SetScriptEye(D3DVECTOR eye);
	void		SetScriptLookat(D3DVECTOR lookat);

	void		SetEffect(BOOL bEnable);
	void		SetCameraScroll(BOOL bScroll);
	void		SetCameraInvertX(BOOL bInvert);
	void		SetCameraInvertY(BOOL bInvert);

	float		RetMotorTurn();
	D3DMouse	RetMouseDef(FPOINT pos);

protected:
	BOOL		EventMouseMove(const Event &event);
	void		EventMouseWheel(int dir);
	BOOL		EventFrame(const Event &event);
	BOOL		EventFrameFree(const Event &event);
	BOOL		EventFrameEdit(const Event &event);
	BOOL		EventFrameDialog(const Event &event);
	BOOL		EventFrameBack(const Event &event);
	BOOL		EventFrameFix(const Event &event);
	BOOL		EventFrameExplo(const Event &event);
	BOOL		EventFrameOnBoard(const Event &event);
	BOOL		EventFrameInfo(const Event &event);
	BOOL		EventFrameVisit(const Event &event);
	BOOL		EventFrameScript(const Event &event);

	void		SetViewTime(const D3DVECTOR &vEyePt, const D3DVECTOR &vLookatPt, float rTime);
	BOOL		IsCollision(D3DVECTOR &eye, D3DVECTOR lookat);
	BOOL		IsCollisionBack(D3DVECTOR &eye, D3DVECTOR lookat);
	BOOL		IsCollisionFix(D3DVECTOR &eye, D3DVECTOR lookat);

	D3DVECTOR	ExcludeTerrain(D3DVECTOR eye, D3DVECTOR lookat, float &angleH, float &angleV);
	D3DVECTOR	ExcludeObject(D3DVECTOR eye, D3DVECTOR lookat, float &angleH, float &angleV);

	void		SetViewParams(const D3DVECTOR &eye, const D3DVECTOR &lookat, const D3DVECTOR &up);
	void		EffectFrame(const Event &event);
	void		OverFrame(const Event &event);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*	m_engine;
	CTerrain*	m_terrain;
	CWater*		m_water;

	CameraType	m_type;				// type de la cam�ra (CAMERA_*)
	CameraSmooth m_smooth;			// type de lissage
	CObject*	m_cameraObj;		// objet li� � la cam�ra

	float		m_eyeDistance;		// distance entre les yeux
	float		m_initDelay;		// d�lai du centrage initial

	D3DVECTOR	m_actualEye;		// oeil actuel
	D3DVECTOR	m_actualLookat;		// vis�e actuelle
	D3DVECTOR	m_finalEye;			// oeil final
	D3DVECTOR	m_finalLookat;		// vis�e finale
	D3DVECTOR	m_normEye;			// oeil normal
	D3DVECTOR	m_normLookat;		// vis�e normale
	float		m_focus;

	BOOL		m_bRightDown;
	FPOINT		m_rightPosInit;
	FPOINT		m_rightPosCenter;
	FPOINT		m_rightPosMove;

	D3DVECTOR	m_eyePt;			// CAMERA_FREE: oeil
	float		m_directionH;		// CAMERA_FREE: direction horizontale
	float		m_directionV;		// CAMERA_FREE: direction verticale
	float		m_heightEye;		// CAMERA_FREE: hauteur au-dessus du sol
	float		m_heightLookat;		// CAMERA_FREE: hauteur au-dessus du sol
	float		m_speed;			// CAMERA_FREE: vitesse de d�placement

	float		m_backDist;			// CAMERA_BACK: �loignement
	float		m_backMin;			// CAMERA_BACK: �loignement minimal
	float		m_addDirectionH;	// CAMERA_BACK: direction suppl�mentaire
	float		m_addDirectionV;	// CAMERA_BACK: direction suppl�mentaire
	BOOL		m_bTransparency;

	float		m_fixDist;			// CAMERA_FIX: �loignement
	float		m_fixDirectionH;	// CAMERA_FIX: direction
	float		m_fixDirectionV;	// CAMERA_FIX: direction

	D3DVECTOR	m_visitGoal;		// CAMERA_VISIT: position vis�e
	float		m_visitDist;		// CAMERA_VISIT: �loignement
	float		m_visitTime;		// CAMERA_VISIT: temps relatif
	CameraType	m_visitType;		// CAMERA_VISIT: type initial
	float		m_visitDirectionH;	// CAMERA_VISIT: direction
	float		m_visitDirectionV;	// CAMERA_VISIT: direction

	float		m_editHeight;		// CAMERA_EDIT: hauteur

	float		m_remotePan;
	float		m_remoteZoom;

	FPOINT		m_mousePos;
	float		m_mouseDirH;
	float		m_mouseDirV;
	float		m_mouseMarging;

	float		m_motorTurn;

	CenteringPhase m_centeringPhase;
	float		m_centeringAngleH;
	float		m_centeringAngleV;
	float		m_centeringDist;
	float		m_centeringCurrentH;
	float		m_centeringCurrentV;
	float		m_centeringTime;
	float		m_centeringProgress;

	CameraEffect m_effectType;
	D3DVECTOR	m_effectPos;
	float		m_effectForce;
	float		m_effectProgress;
	D3DVECTOR	m_effectOffset;

	OverEffect	m_overType;
	float		m_overForce;
	float		m_overTime;
	D3DCOLORVALUE m_overColorBase;
	D3DCOLORVALUE m_overColor;
	int			m_overMode;
	float		m_overFadeIn;
	float		m_overFadeOut;

	D3DVECTOR	m_scriptEye;
	D3DVECTOR	m_scriptLookat;

	BOOL		m_bEffect;			// secousses si explosion ?
	BOOL		m_bCameraScroll;	// scroll dans les bords ?
	BOOL		m_bCameraInvertX;	// inversion X dans les bords ?
	BOOL		m_bCameraInvertY;	// inversion Y dans les bords ?
};


#endif //_CAMERA_H_
