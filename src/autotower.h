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
	ATP_ZERO	= 0,
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

protected:
	void		FireStopUpdate(float progress, BOOL bLightOn);

protected:
	AutoTowerPhase	m_phase;
	float			m_progress;
	float			m_speed;
	int				m_partiStop[4];
};


#endif //_AUTOTOWER_H_
