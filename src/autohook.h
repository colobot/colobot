// autohook.h

#ifndef _AUTOHOOK_H_
#define	_AUTOHOOK_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoHookPhase
{
	AHKP_WAIT		= 1,	// attend cible
	AHKP_GOSTART	= 2,	// 
	AHKP_DOWN		= 3,	// 
	AHKP_UP			= 4,	// 
	AHKP_GOEND		= 5,	// 
};


class CAutoHook : public CAuto
{
public:
	CAutoHook(CInstanceManager* iMan, CObject* object);
	~CAutoHook();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float minRadius, float maxRadius);
	void		StartAction(int action, float delay);
	void		SoundManip(float time, float amplitude, float frequency);

protected:
	AutoHookPhase m_phase;
	float		m_progress;
	float		m_speed;

	float		m_lastParticule;
	D3DVECTOR	m_targetPos;
	CameraType	m_cameraType;
	float		m_startAngle;
	float		m_goalAngle;
	float		m_startDist;
	float		m_goalDist;
	float		m_startHeight;
	float		m_goalHeight;
	float		m_startRot;
	float		m_goalRot;
	BOOL		m_bGrab;
	CObject*	m_target;
	CObject*	m_load;
};


#endif //_AUTOHOOK_H_
