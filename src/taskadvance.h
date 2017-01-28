// taskadvance.h

#ifndef _TASKADVANCE_H_
#define	_TASKADVANCE_H_


class CInstanceManager;
class CTerrain;
class CObject;


class CTaskAdvance : public CTask
{
public:
	CTaskAdvance(CInstanceManager* iMan, CObject* object);
	~CTaskAdvance();

	BOOL	EventProcess(const Event &event);

	Error	Start(float length, BOOL bNoError);
	Error	IsEnded();
	BOOL	IsUndoable();

	void	WriteSituation();
	void	ReadSituation();

protected:
	CObject*	SearchObject(D3DVECTOR center, float radius);
	BOOL		IsPosFree(D3DVECTOR center);
	void		ProgressLinSpeed(float speed);
	void		ProgressCirSpeed(float speed);
	void		StartAction(CObject* pObj, int action);

protected:
	ObjectType	m_type;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	float		m_moveAbs;
	float		m_moveDist;
	BOOL		m_bMine;
	CObject*	m_pMine;
	BOOL		m_bGoal;
	BOOL		m_bLostGoal;
	CObject*	m_pGoal;
	BOOL		m_bPerfo;
	CObject*	m_pPerfo;
	BOOL		m_bFall;
	BOOL		m_bError;
};


#endif //_TASKADVANCE_H_
