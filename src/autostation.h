// autostation.h

#ifndef _AUTOSTATION_H_
#define	_AUTOSTATION_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CAutoStation : public CAuto
{
public:
	CAutoStation(CInstanceManager* iMan, CObject* object);
	~CAutoStation();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

protected:
	void		UpdateInterface(float rTime);

	CObject*	SearchVehicle();

protected:
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastUpdateTime;
	float			m_lastParticule;
	int				m_soundChannel;
	D3DVECTOR		m_fretPos;
	BOOL			m_bLastVirus;
	float			m_energyVirus;
};


#endif //_AUTOSTATION_H_
