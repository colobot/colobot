// taskrecover.h

#ifndef _TASKSRECOVER_H_
#define	_TASKSRECOVER_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskRecoverPhase
{
	TRP_TURN	= 1,	// tourne
	TRP_MOVE	= 2,	// avance
	TRP_DOWN	= 3,	// descend
	TRP_OPER	= 4,	// opère
	TRP_UP		= 5,	// remonte
};



class CTaskRecover : public CTask
{
public:
	CTaskRecover(CInstanceManager* iMan, CObject* object);
	~CTaskRecover();

	BOOL		EventProcess(const Event &event);

	Error		Start();
	Error		IsEnded();
	BOOL		Abort();

protected:
	CObject*	SearchRuin();

protected:
	TaskRecoverPhase m_phase;
	float			m_progress;
	float			m_speed;
	float			m_time;
	float			m_angle;
	float			m_lastParticule;
	BOOL			m_bError;
	CObject*		m_ruin;
	CObject*		m_metal;
	D3DVECTOR		m_recoverPos;
	int				m_soundChannel;
};


#endif //_TASKSRECOVER_H_
