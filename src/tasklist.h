// tasklist.h

#ifndef _TASKLIST_H_
#define	_TASKLIST_H_



class CTaskManager;
class CTask;
class CMainUndo;
class CRobotMain;


enum TaskOrder
{
	TO_NULL,		// rien
	TO_MOVE,		// avance
	TO_TURN,		// tourne
	TO_GOTO,		// va
	TO_GOTOPUSH,	// va puis pousse
	TO_PUSH,		// pousse seulement
	TO_ROLL,		// roule une sphère
	TO_GOTODOCK,	// va puis manoeuvre
	TO_DOCK,		// manoeuvre seulement
	TO_GOTOCATA,	// va puis manoeuvre
	TO_CATA,		// manoeuvre seulement
	TO_GOTOTRAX,	// va puis manoeuvre
	TO_TRAX,		// manoeuvre seulement
	TO_GOTOPERFO,	// va puis manoeuvre
	TO_PERFO,		// manoeuvre seulement
	TO_GOTOGUN,		// va puis manoeuvre
	TO_GUN	,		// manoeuvre seulement
	TO_GOTODRINK,	// va puis boit
	TO_DRINK,		// boit seulement
	TO_GOTOGOAL,	// va puis s'envole
	TO_GOAL,		// s'envole seulement
	TO_GOTODIVE,	// va puis plonge
	TO_DIVE,		// plonge seulement
};

typedef struct
{
	TaskOrder	order;
	D3DVECTOR	pos;
	int			id;
	int			part;
	float		param;
}
TaskItem;

#define MAXTASKLIST	20



class CTaskList
{
public:
	CTaskList(CInstanceManager* iMan, CObject* object);
	~CTaskList();

	void		Flush();
	BOOL		AddHead(TaskOrder order, D3DVECTOR pos, int id, int part, float param);
	BOOL		AddTail(TaskOrder order, D3DVECTOR pos, int id, int part, float param);
	BOOL		SubHead(TaskOrder &order, D3DVECTOR &pos, int &id, int &part, float &param);
	BOOL		SubTail(TaskOrder &order, D3DVECTOR &pos, int &id, int &part, float &param);
	BOOL		EventFrame(const Event &event);
	BOOL		IsRunning();
	TaskOrder	RetRunningOrder();
	CTask*		RetRunningTask();
	BOOL		IsOtherTask();
	BOOL		IsUndoable();
	BOOL		IsStopable();
	BOOL		Stop();
	BOOL		Abort();
	void		FlushStatistic();
	int			RetStatisticAdvise();

	Error		StartTaskMove(float length, BOOL bNoError);
	Error		StartTaskTurn(float angle);
	Error		StartTaskGoto(D3DVECTOR pos, CObject *target, int part);
	Error		StartTaskPush(int part, int nbTiles);
	Error		StartTaskRoll(D3DVECTOR dir);
	Error		StartTaskDock(CObject *dock, int part);
	Error		StartTaskCatapult(CObject *catapult, int part);
	Error		StartTaskTrax(CObject *trax, int part);
	Error		StartTaskPerfo(CObject *perfo, int part);
	Error		StartTaskGun(CObject *gun, int part);
	Error		StartTaskDrink(CObject *drink);
	Error		StartTaskGoal(CObject *goal);
	Error		StartTaskDive(CObject *dive);

protected:
	void		UndoRecord(const D3DVECTOR &goal);
	void		UndoRecord();
	BOOL		IsTargetRunning(CObject *target);
	CObject*	IdSearch(int id);
	void		ShowError();
	void		AddStatistic(TaskOrder order, D3DVECTOR pos, int id, int part, float param);
	BOOL		IsTaskPossible(CObject *pObj, TaskOrder order, D3DVECTOR pos, int id, int part, float param);
	BOOL		SearchOtherBlupi();

protected:
	CInstanceManager* m_iMan;
	CMainUndo*		m_undo;
	CRobotMain*		m_main;
	CObject*		m_object;

	int				m_total;
	int				m_head;
	int				m_tail;
	TaskItem		m_fifo[MAXTASKLIST];

	TaskOrder		m_taskOrder;
	CTaskManager*	m_task;		// tâche en cours
	BOOL			m_bStopPending;

	TaskOrder		m_statisticOrder;
	D3DVECTOR		m_statisticPos;
	int				m_statisticLittleGoto;
	int				m_statisticGotoPush;
};


#endif //_TASKLIST_H_
