// taskturn.h

#ifndef _TASKTURN_H_
#define	_TASKTURN_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


class CTaskTurn : public CTask
{
public:
	CTaskTurn(CInstanceManager* iMan, CObject* object);
	~CTaskTurn();

	BOOL	EventProcess(const Event &event);

	Error	Start(float angle);
	Error	IsEnded();

protected:

protected:
	float		m_angle;
	float		m_startAngle;
	float		m_finalAngle;
	BOOL		m_bLeft;
	BOOL		m_bError;
};


#endif //_TASKTURN_H_
