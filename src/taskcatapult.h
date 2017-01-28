// taskcatapult.h

#ifndef _TASKCATAPULT_H_
#define	_TASKCATAPULT_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskCatapult : public CTask
{
public:
	CTaskCatapult(CInstanceManager* iMan, CObject* object);
	~CTaskCatapult();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *dock, int part);
	Error	IsEnded();

protected:
	void	StartAction(int action, float speed=0.2f);

protected:
	CObject*	m_dock;
	int			m_part;
	float		m_time;
	float		m_progress;
	float		m_speed;
	BOOL		m_bAuto;
	BOOL		m_bError;
};


#endif //_TASKCATAPULT_H_
