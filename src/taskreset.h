// taskreset.h

#ifndef _TASKRESET_H_
#define	_TASKRESET_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskResetPhase
{
	TRSP_ZOUT	= 1,	// disparaît
	TRSP_MOVE	= 2,	// déplace
	TRSP_ZIN	= 3,	// réapparaît
};



class CTaskReset : public CTask
{
public:
	CTaskReset(CInstanceManager* iMan, CObject* object);
	~CTaskReset();

	BOOL	EventProcess(const Event &event);

	Error	Start(D3DVECTOR goal, D3DVECTOR angle);
	Error	IsEnded();

protected:
	BOOL	SearchVehicle();

protected:
	D3DVECTOR		m_begin;
	D3DVECTOR		m_goal;
	D3DVECTOR		m_angle;

	TaskResetPhase	m_phase;
	BOOL			m_bError;
	float			m_time;
	float			m_speed;
	float			m_progress;
	float			m_lastParticule;  // temps génération dernière particule
	float			m_iAngle;
};


#endif //_TASKRESET_H_
