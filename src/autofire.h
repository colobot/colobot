// autofire.h

#ifndef _AUTOFIRE_H_
#define	_AUTOFIRE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoFirePhase
{
	AFIP_WAIT		= 1,	// attend cible
	AFIP_TURNIN		= 2,	// tourne vers cible
	AFIP_SPRINKLE	= 3,	// gicle
	AFIP_TURNOUT	= 4,	// tourne vers repos
	AFIP_WAITOUT	= 5,	// attend avant de recommencer
};



class CAutoFire : public CAuto
{
public:
	CAutoFire(CInstanceManager* iMan, CObject* object);
	~CAutoFire();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float radius);
	void		BreakDown();

protected:
	AutoFirePhase m_phase;
	float		m_progress;
	float		m_speed;

	float		m_startAngleH;
	float		m_startAngleV;
	float		m_goalAngleH;
	float		m_goalAngleV;
	float		m_trackMass;
	float		m_lastParticule;
	float		m_lastPlouf;
	float		m_lastSound;
	float		m_lastBreakdown;
	D3DVECTOR	m_targetPos;
	CameraType	m_cameraType;
};


#endif //_AUTOFIRE_H_
