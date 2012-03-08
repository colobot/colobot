// autorepair.h

#ifndef _AUTOREPAIR_H_
#define	_AUTOREPAIR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoRepairPhase
{
	ARP_WAIT		= 1,	// attend métal
	ARP_DOWN		= 2,	// descend le couvercle
	ARP_REPAIR		= 3,	// construit le véhicule
	ARP_UP			= 4,	// remonte le couvercle
};



class CAutoRepair : public CAuto
{
public:
	CAutoRepair(CInstanceManager* iMan, CObject* object);
	~CAutoRepair();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchVehicle();

protected:
	AutoRepairPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastParticule;
};


#endif //_AUTOREPAIR_H_
