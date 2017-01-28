// taskwait.h

#ifndef _TASKWAIT_H_
#define	_TASKWAIT_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskWait : public CTask
{
public:
	CTaskWait(CInstanceManager* iMan, CObject* object);
	~CTaskWait();

	BOOL	EventProcess(const Event &event);

	Error	Start(float time);
	Error	IsEnded();

protected:

protected:
	float		m_waitTime;
	float		m_passTime;
	BOOL		m_bEnded;
};


#endif //_TASKWAIT_H_
