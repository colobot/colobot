// taskdive.h

#ifndef _TASKDIVE_H_
#define	_TASKDIVE_H_


class CInstanceManager;
class CTerrain;
class CObject;



enum TaskDivePhase
{
	TDI_MARCH,	// va presque au bout
	TDI_TRY,	// essaie de plonger
	TDI_NONO,	// non-non
	TDI_TURN,	// demi-tour

	TDI_END,	// va au bout-bout
	TDI_OUPS,	// saute
	TDI_DIVE,	// plonge
	TDI_WAIT,	// attend sous l'eau
	TDI_UP,		// remonte
	TDI_OSCIL,	// balance
	TDI_JUMP,	// saut looping
	TDI_OH,		// oh !

	TDI_BACK,	// revient
};



class CTaskDive : public CTask
{
public:
	CTaskDive(CInstanceManager* iMan, CObject* object);
	~CTaskDive();

	BOOL	EventProcess(const Event &event);

	Error	Start(CObject *dive);
	Error	IsEnded();

protected:
	void	StartAction(int action, float speed=0.2f);
	void	SetLinSpeed(float speed);

protected:
	TaskDivePhase	m_phase;
	CObject*		m_dive;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	D3DVECTOR		m_oupsPos;
	D3DVECTOR		m_divePos;
	float			m_startAngle;
	float			m_time;
	float			m_progress;
	float			m_speed;
	float			m_level;
	BOOL			m_bError;
	BOOL			m_bPlouf;
};


#endif //_TASKDIVE_H_
