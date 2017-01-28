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
	Error	StartTaskMove(float length, BOOL bNoError);
	Error	StartTaskTurn(float angle);
	Error	StartTaskGoto(D3DVECTOR pos, CObject *target, int part);
	Error	StartTaskPush(int part, int nbTiles);
	Error	StartTaskRoll(D3DVECTOR dir);
	Error	StartTaskDock(CObject *dock, int part);
	Error	StartTaskCatapult(CObject *catapult, int part);
	Error	StartTaskTrax(CObject *trax, int part);
	Error	StartTaskPerfo(CObject *perfo, int part);
	Error	StartTaskGun(CObject *gun, int part);
	Error	StartTaskDrink(CObject *fiole);
	Error	StartTaskGoal(CObject *goal);
	Error	StartTaskDive(CObject *dive);
	Error	StartTaskFire(float delay);

	BOOL	EventProcess(const Event &event);
	Error	IsEnded();
	BOOL	IsPilot();
	BOOL	IsUndoable();
	BOOL	IsStopable();
	BOOL	Stop();
	BOOL	Abort();
	CTask*	RetRunningTask();

protected:

protected:
	CInstanceManager* m_iMan;
	CTask*			m_task;
	CObject*		m_object;
	BOOL			m_bPilot;
};


#endif //_TASKMANAGER_H_
