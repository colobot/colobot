// autotower.h

#ifndef _AUTOTOWER_H_
#define	_AUTOTOWER_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



enum AutoTowerPhase
{
	ATP_WAIT		= 1,
	ATP_ZERO		= 2,	// plus d'énergie
	ATP_SEARCH		= 3,	// cherche une cible
	ATP_TURN		= 4,	// tourne vers la cible
	ATP_FIRE		= 5,	// tire sur la cible
};



class CAutoTower : public CAuto
{
public:
	CAutoTower(CInstanceManager* iMan, CObject* object);
	~CAutoTower();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		UpdateInterface(float rTime);

	CObject*	SearchTarget(D3DVECTOR &impact);
	void		FireStopUpdate(float progress, BOOL bLightOn);

protected:
	AutoTowerPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastUpdateTime;
	float			m_lastParticule;
	D3DVECTOR		m_targetPos;
	float			m_angleYactual;
	float			m_angleZactual;
	float			m_angleYfinal;
	float			m_angleZfinal;
	int				m_partiStop[4];
};


#endif //_AUTOTOWER_H_
