// taskgungoal.h

#ifndef _TASKGUNGOAL_H_
#define	_TASKGUNGOAL_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


class CTaskGunGoal : public CTask
{
public:
	CTaskGunGoal(CInstanceManager* iMan, CObject* object);
	~CTaskGunGoal();

	BOOL		EventProcess(const Event &event);

	Error		Start(float dirV, float dirH);
	Error		IsEnded();
	BOOL		Abort();

protected:

protected:
	float		m_progress;
	float		m_speed;
	float		m_initialDirV;	// direction initiale
	float		m_finalDirV;	// direction à atteindre
	float		m_initialDirH;	// direction initiale
	float		m_finalDirH;	// direction à atteindre
};


#endif //_TASKGUNGOAL_H_
