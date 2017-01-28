// taskperfo.h

#ifndef _TASKPERFO_H_
#define	_TASKPERFO_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskPerfo : public CTask
{
public:
	CTaskPerfo(CInstanceManager* iMan, CObject* object);
	~CTaskPerfo();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *perfo, int part);
	Error	IsEnded();

protected:
	void		StartAction(int action, float speed=0.2f);

protected:
	CObject*	m_perfo;
	int			m_part;
	float		m_time;
	float		m_progress;
	float		m_speed;
	BOOL		m_bAuto;
	BOOL		m_bError;
};


#endif //_TASKPERFO_H_
