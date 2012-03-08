// autonuclear.h

#ifndef _AUTONUCLEAR_H_
#define	_AUTONUCLEAR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoNuclearPhase
{
	ANUP_STOP		= 1,
	ANUP_WAIT		= 2,
	ANUP_CLOSE		= 3,
	ANUP_GENERATE	= 4,
	ANUP_OPEN		= 5,
};



class CAutoNuclear : public CAuto
{
public:
	CAutoNuclear(CInstanceManager* iMan, CObject* object);
	~CAutoNuclear();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchUranium();
	BOOL		SearchVehicle();
	void		CreatePower();

protected:
	AutoNuclearPhase	m_phase;
	float				m_progress;
	float				m_speed;
	float				m_timeVirus;
	float				m_lastParticule;
	D3DVECTOR			m_pos;
	int					m_channelSound;
};


#endif //_AUTONUCLEAR_H_
