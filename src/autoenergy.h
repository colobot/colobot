// autoenergy.h

#ifndef _AUTOENERGY_H_
#define	_AUTOENERGY_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoEnergyPhase
{
	AENP_STOP		= 1,
	AENP_WAIT		= 2,
	AENP_BLITZ		= 3,
	AENP_CREATE		= 4,
	AENP_SMOKE		= 5,
};



class CAutoEnergy : public CAuto
{
public:
	CAutoEnergy(CInstanceManager* iMan, CObject* object);
	~CAutoEnergy();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		UpdateInterface(float rTime);

	CObject*	SearchMetal();
	BOOL		SearchVehicle();
	void		CreatePower();
	CObject*	SearchPower();

protected:
	AutoEnergyPhase		m_phase;
	float				m_progress;
	float				m_speed;
	float				m_timeVirus;
	float				m_lastUpdateTime;
	float				m_lastParticule;
	int					m_partiSphere;
};


#endif //_AUTOENERGY_H_
