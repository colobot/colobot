// autocatapult.h

#ifndef _AUTOCATAPULT_H_
#define	_AUTOCATAPULT_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoCatapultPhase
{
	ACAP_WAIT	= 1,	// attend ordre
	ACAP_UP		= 2,	// marteau prend de l'élan
	ACAP_DOWN	= 3,	// marteau descend
	ACAP_FLY	= 4,	// caisse vole
	ACAP_RETURN	= 5,	// marteur au départ
	ACAP_FALL	= 6,	// caisse tombe dans trou
};



class CAutoCatapult : public CAuto
{
public:
	CAutoCatapult(CInstanceManager* iMan, CObject* object);
	~CAutoCatapult();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	BOOL		IsRunning();
	Error		RetError();

protected:
	void		LockZone(BOOL bLock);
	float		CalcDistFactor();
	D3DVECTOR	CalcPosition(float dist);
	CObject*	SearchObject(D3DVECTOR center, float radius);

protected:
	AutoCatapultPhase m_phase;
	float			m_progress;
	float			m_speed;
	ObjectType		m_type;
	CObject*		m_pBox;
	float			m_distFactor;
	float			m_hammerAngle[5];
	BOOL			m_bFall;
	float			m_lastParticule;
	int				m_channelSound;
};


#endif //_AUTOCATAPULT_H_
