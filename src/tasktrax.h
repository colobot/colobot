// taskdtrax.h

#ifndef _TASKTRAX_H_
#define	_TASKTRAX_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskTrax : public CTask
{
public:
	CTaskTrax(CInstanceManager* iMan, CObject* object);
	~CTaskTrax();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *trax, int part);
	Error	IsEnded();

protected:
	CObject*	SearchBox(D3DVECTOR center, float radius);
	void		StartAction(int action, float speed=0.2f);

protected:
	CObject*	m_trax;
	int			m_part;
	float		m_time;
	float		m_progress;
	float		m_speed;
	BOOL		m_bAuto;
	BOOL		m_bError;
};


#endif //_TASKTRAX_H_
