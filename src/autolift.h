// autolift.h

#ifndef _AUTOLIFT_H_
#define	_AUTOLIFT_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



enum AutoLiftPhase
{
	ALI_NULL	= 1,	// rien à faire
	ALI_UP1		= 2,	// monte
	ALI_UP2		= 3,	// monte
	ALI_DOWN	= 4,	// redescend
};



class CAutoLift : public CAuto
{
public:
	CAutoLift(CInstanceManager* iMan, CObject* object);
	~CAutoLift();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int phase);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(D3DVECTOR center, float radius);
	void		StartAction(int action, float speed=0.2f);
	void		CameraStart1();
	void		CameraStart2();
	void		CameraProgress(float progress);
	void		CameraStop();

protected:
	D3DVECTOR		m_posGround;
	D3DVECTOR		m_posBlupi;
	D3DVECTOR		m_angleBlupi;
	D3DVECTOR		m_lastAngleBlupi;
	CObject*		m_blupi;
	AutoLiftPhase	m_phase;
	float			m_speed;
	float			m_progress;
	float			m_totalRot;
	BOOL			m_bSelect;
	BOOL			m_bCamera;
	D3DVECTOR		m_eyeStart;
	D3DVECTOR		m_lookatStart;
	D3DVECTOR		m_eyeGoal;
	D3DVECTOR		m_lookatGoal;
	D3DVECTOR		m_eyeFinal;
	D3DVECTOR		m_lookatFinal;
	float			m_dirFinal;
};


#endif //_AUTOLIFT_H_
