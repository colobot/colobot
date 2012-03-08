// taskfireant.h

#ifndef _TASKFIREANT_H_
#define	_TASKTIREANT_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskFireAnt
{
	TFA_NULL		= 0,	// rien à faire
	TFA_TURN		= 1,	// tourne
	TFA_PREPARE		= 2,	// prépare position de tir
	TFA_FIRE		= 3,	// tir
	TFA_TERMINATE	= 4,	// termine position de tir
};



class CTaskFireAnt : public CTask
{
public:
	CTaskFireAnt(CInstanceManager* iMan, CObject* object);
	~CTaskFireAnt();

	BOOL	EventProcess(const Event &event);

	Error	Start(D3DVECTOR impact);
	Error	IsEnded();
	BOOL	Abort();

protected:

protected:
	D3DVECTOR	m_impact;
	TaskFireAnt	m_phase;
	float		m_progress;
	float		m_speed;
	float		m_angle;
	BOOL		m_bError;
	BOOL		m_bFire;
	float		m_time;
	float		m_lastParticule;
};


#endif //_TASKFIREANT_H_
