// autodestroyer.h

#ifndef _AUTODESTROYER_H_
#define	_AUTODESTROYER_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoDestroyerPhase
{
	ADEP_WAIT		= 1,	// attend métal
	ADEP_DOWN		= 2,	// descend le couvercle
	ADEP_REPAIR		= 3,	// construit le véhicule
	ADEP_UP			= 4,	// remonte le couvercle
};



class CAutoDestroyer : public CAuto
{
public:
	CAutoDestroyer(CInstanceManager* iMan, CObject* object);
	~CAutoDestroyer();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchPlastic();
	BOOL		SearchVehicle();

protected:
	AutoDestroyerPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastParticule;
	BOOL			m_bExplo;
};


#endif //_AUTODESTROYER_H_
