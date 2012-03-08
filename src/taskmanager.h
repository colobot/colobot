// taskmanager.h

#ifndef _TASKMANAGER_H_
#define	_TASKMANAGER_H_


class CInstanceManager;
class CTask;

enum TaskManipOrder;
enum TaskManipArm;
enum TaskFlagOrder;
enum TaskGotoGoal;
enum TaskGotoCrash;
enum TaskShieldMode;
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
	Error	StartTaskTake();
	Error	StartTaskManip(TaskManipOrder order, TaskManipArm arm);
	Error	StartTaskFlag(TaskFlagOrder order, int rank);
	Error	StartTaskBuild(ObjectType type);
	Error	StartTaskSearch();
	Error	StartTaskInfo(char *name, float value, float power, BOOL bSend);
	Error	StartTaskTerraform();
	Error	StartTaskPen(BOOL bDown, int color);
	Error	StartTaskRecover();
	Error	StartTaskShield(TaskShieldMode mode, float delay);
	Error	StartTaskFire(float delay);
	Error	StartTaskFireAnt(D3DVECTOR impact);
	Error	StartTaskGunGoal(float dirV, float dirH);
	Error	StartTaskSpiderExplo();
	Error	StartTaskReset(D3DVECTOR goal, D3DVECTOR angle);

	BOOL	EventProcess(const Event &event);
	Error	IsEnded();
	BOOL	IsBusy();
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
