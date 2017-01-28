// taskfire.h

#ifndef _TASKFIRE_H_
#define	_TASKTIRE_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskFire : public CTask
{
public:
	CTaskFire(CInstanceManager* iMan, CObject* object);
	~CTaskFire();

	BOOL		EventProcess(const Event &event);

	Error		Start(float delay);
	Error		IsEnded();
	BOOL		Abort();

protected:

protected:
	float		m_delay;
	float		m_progress;
	BOOL		m_bError;
	BOOL		m_bRay;
	BOOL		m_bOrganic;
	float		m_time;
	float		m_speed;
	float		m_lastParticule;
	float		m_lastSound;
	int			m_soundChannel;
};


#endif //_TASKFIRE_H_
