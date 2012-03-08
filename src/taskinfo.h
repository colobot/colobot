// taskinfo.h

#ifndef _TASKINFO_H_
#define	_TASKINFO_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



class CTaskInfo : public CTask
{
public:
	CTaskInfo(CInstanceManager* iMan, CObject* object);
	~CTaskInfo();

	BOOL		EventProcess(const Event &event);

	Error		Start(char *name, float value, float power, BOOL bSend);
	Error		IsEnded();
	BOOL		Abort();

protected:
	CObject*	SearchInfo(float power);

protected:
	float			m_progress;
	float			m_speed;
	float			m_time;
	BOOL			m_bError;
};


#endif //_TASKINFO_H_
