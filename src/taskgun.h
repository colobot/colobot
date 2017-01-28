// taskgun.h

#ifndef _TASKGUN_H_
#define	_TASKGUN_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskGun : public CTask
{
public:
	CTaskGun(CInstanceManager* iMan, CObject* object);
	~CTaskGun();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *gun, int part);
	Error	IsEnded();

protected:
	void		StartAction(int action, float speed=0.2f);

protected:
	CObject*	m_gun;
	int			m_part;
	float		m_time;
	float		m_progress;
	float		m_speed;
	BOOL		m_bAuto;
	BOOL		m_bError;
};


#endif //_TASKGUN_H_
