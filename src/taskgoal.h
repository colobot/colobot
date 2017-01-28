// taskgoal.h

#ifndef _TASKGOAL_H_
#define	_TASKGOAL_H_


class CInstanceManager;
class CTerrain;
class CObject;



enum TaskGoalPhase
{
	TGO_GOAL	= 1,	// lève les bras
	TGO_FLY		= 2,	// s'envole
};



class CTaskGoal : public CTask
{
public:
	CTaskGoal(CInstanceManager* iMan, CObject* object);
	~CTaskGoal();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *goal);
	Error	IsEnded();

protected:
	void	StartAction(int action, float speed=0.2f);
	BOOL	IsLastGoal();

protected:
	TaskGoalPhase	m_phase;
	CObject*		m_goal;
	D3DVECTOR		m_startPos;
	float			m_time;
	float			m_progress;
	float			m_speed;
	BOOL			m_bError;
};


#endif //_TASKGOAL_H_
