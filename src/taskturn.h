// taskturn.h

#ifndef _TASKTURN_H_
#define	_TASKTURN_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskTurn : public CTask
{
public:
	CTaskTurn(CInstanceManager* iMan, CObject* object);
	~CTaskTurn();

	BOOL	EventProcess(const Event &event);

	Error	Start(float angle);
	Error	IsEnded();
	BOOL	IsUndoable();

	void	WriteSituation();
	void	ReadSituation();

protected:
	void	ProgressLinSpeed(float speed);
	void	ProgressCirSpeed(float speed);
	void	StartAction(CObject* pObj, int action);

protected:
	ObjectType	m_type;
	float		m_initialWait;
	float		m_startAngle;
	float		m_finalAngle;
	BOOL		m_bError;
};


#endif //_TASKTURN_H_
