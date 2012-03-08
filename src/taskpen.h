// taskpen.h

#ifndef _TASKSPEN_H_
#define	_TASKSPEN_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskPenPhase
{
	TPP_UP		= 1,	// monte le crayon
	TPP_TURN	= 2,	// tourne le carrousel
	TPP_DOWN	= 3,	// descend le crayon
};



class CTaskPen : public CTask
{
public:
	CTaskPen(CInstanceManager* iMan, CObject* object);
	~CTaskPen();

	BOOL		EventProcess(const Event &event);

	Error		Start(BOOL bDown, int color);
	Error		IsEnded();
	BOOL		Abort();

protected:
	void		SoundManip(float time, float amplitude, float frequency);
	int			AngleToRank(float angle);
	float		ColorToAngle(int color);
	int			ColorToRank(int color);

protected:
	BOOL			m_bError;
	TaskPenPhase	m_phase;
	float			m_progress;
	float			m_delay;
	float			m_time;
	float			m_lastParticule;
	D3DVECTOR		m_supportPos;

	float			m_timeUp;
	float			m_oldAngle;
	float			m_newAngle;
	float			m_timeDown;
};


#endif //_TASKSPEN_H_
