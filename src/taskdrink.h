// taskdrink.h

#ifndef _TASKDRINK_H_
#define	_TASKDRINK_H_


class CInstanceManager;
class CTerrain;
class CObject;



enum TaskDrinkPhase
{
	TDR_TAKE	= 1,	// prend la fiole
	TDR_DRINK	= 2,	// boit
};



class CTaskDrink : public CTask
{
public:
	CTaskDrink(CInstanceManager* iMan, CObject* object);
	~CTaskDrink();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *fiole);
	Error	IsEnded();

protected:
	void	StartAction(int action, float speed=0.2f);

protected:
	TaskDrinkPhase	m_phase;
	CObject*		m_fiole;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	float			m_time;
	float			m_progress;
	float			m_speed;
	BOOL			m_bError;
};


#endif //_TASKDRINK_H_
