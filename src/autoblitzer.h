// autoblitzer.h

#ifndef _AUTOBLITZER_H_
#define	_AUTOBLITZER_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoBlitzerPhase
{
	ABLP_WAIT		= 1,	// attend cible
	ABLP_BLITZ		= 2,	// électrocute
	ABLP_WAITOUT	= 3,	// attend avant de recommencer
};


class CAutoBlitzer : public CAuto
{
public:
	CAutoBlitzer(CInstanceManager* iMan, CObject* object);
	~CAutoBlitzer();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(int total, ObjectType *type, float *radius, D3DVECTOR center);
	void		BreakDownFactory();
	void		BreakDownEvil1();
	void		BreakDownEvil3();

protected:
	AutoBlitzerPhase m_phase;
	float		m_progress;
	float		m_speed;

	float		m_lastParticule;
	D3DVECTOR	m_targetPos;
	CameraType	m_cameraType;
	ObjectType	m_targetType;
	float		m_delayBlitz;
	float		m_delayTotal;
	int			m_channelSphere;
};


#endif //_AUTOBLITZER_H_
