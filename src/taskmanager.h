// taskmanager.h

#ifndef _TASKMANAGER_H_
#define	_TASKMANAGER_H_


class CInstanceManager;
class CTask;

enum TaskGotoGoal;
enum TaskGotoCrash;
enum ObjectType;



class CTaskManager
{
public:
	CTaskManager(CInstanceManager* iMan, CObject* object);
	~CTaskManager();

	Error	StartTaskWait(float time);
	Error	StartTaskAdvance(float length);
	Error	StartTaskTurn(float angle);
	Error	StartTaskGoto(D3DVECTOR pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
	Error	StartTaskFire(float delay);

	BOOL	EventProcess(const Event &event);
	Error	IsEnded();
	BOOL	IsPilot();
	BOOL	Abort();

protected:

protected:
	CInstanceManager* m_iMan;
	CTask*			m_task;
	CObject*		m_object;
	BOOL			m_bPilot;
};


#endif //_TASKMANAGER_H_
