// autoradar.h

#ifndef _AUTORADAR_H_
#define	_AUTORADAR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoRadarPhase
{
	ARAP_WAIT		= 1,	// attend
	ARAP_SEARCH		= 2,	// cherche
	ARAP_SHOW		= 3,	// montre
	ARAP_SINUS		= 4,	// oscille
};



class CAutoRadar : public CAuto
{
public:
	CAutoRadar(CInstanceManager* iMan, CObject* object);
	~CAutoRadar();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	BOOL		CreateInterface(BOOL bSelect);
	Error		RetError();

protected:
	void		UpdateInterface();
	BOOL		SearchEnemy(D3DVECTOR &pos);

protected:
	AutoRadarPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_aTime;
	float			m_timeVirus;
	float			m_lastParticule;
	float			m_angle;
	float			m_start;
	int				m_totalDetect;
};


#endif //_AUTORADAR_H_
