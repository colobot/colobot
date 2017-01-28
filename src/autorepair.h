// autorepair.h

#ifndef _AUTOREPAIR_H_
#define	_AUTOREPAIR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



enum AutoRepairPhase
{
	AREP_WAIT		= 1,	// attend robot
	AREP_DOWN		= 2,	// descend le capteur
	AREP_REPAIR1	= 3,	// répare
	AREP_REPAIR2	= 4,	// répare
	AREP_TERM		= 5,	// c'est fini
};



class CAutoRepair : public CAuto
{
public:
	CAutoRepair(CInstanceManager* iMan, CObject* object);
	~CAutoRepair();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float minRadius, float maxRadius);
	void		StartAction(int action, float delay);

protected:
	AutoRepairPhase m_phase;
	D3DVECTOR	m_pos;
	float		m_progress;
	float		m_speed;
	float		m_lastParticule;
	CObject*	m_bot;
};


#endif //_AUTOREPAIR_H_
