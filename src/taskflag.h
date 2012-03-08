// taskflag.h

#ifndef _TASKFLAG_H_
#define	_TASKFLAG_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskFlagOrder
{
	TFL_CREATE	= 0,	// met
	TFL_DELETE	= 1,	// enlève
};



class CTaskFlag : public CTask
{
public:
	CTaskFlag(CInstanceManager* iMan, CObject* object);
	~CTaskFlag();

	BOOL		EventProcess(const Event &event);

	Error		Start(TaskFlagOrder order, int rank);
	Error		IsEnded();
	BOOL		Abort();

protected:
	Error		CreateFlag(int rank);
	Error		DeleteFlag();
	CObject*	SearchNearest(D3DVECTOR pos, ObjectType type);
	int			CountObject(ObjectType type);

protected:
	TaskFlagOrder	m_order;
	float			m_time;
	BOOL			m_bError;
};


#endif //_TASKFLAG_H_
