// taskspiderexplo.h

#ifndef _TASKSPIDEREXPLO_H_
#define	_TASKSPIDEREXPLO_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


class CTaskSpiderExplo : public CTask
{
public:
	CTaskSpiderExplo(CInstanceManager* iMan, CObject* object);
	~CTaskSpiderExplo();

	BOOL		EventProcess(const Event &event);

	Error		Start();
	Error		IsEnded();
	BOOL		Abort();

protected:

protected:
	float		m_time;
	BOOL		m_bError;
};


#endif //_TASKSPIDEREXPLO_H_
