// taskmanip.h

#ifndef _TASKMANIP_H_
#define	_TASKMANIP_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskManipOrder
{
	TMO_AUTO	= 0,	// prend ou dépose automatique
	TMO_GRAB	= 1,	// prend un objet
	TMO_DROP	= 2,	// dépose l'objet
};

enum TaskManipArm
{
	TMA_NEUTRAL	= 1,	// bras vide au repos
	TMA_STOCK	= 2,	// bras plein au repos
	TMA_FFRONT	= 3,	// bras au sol
	TMA_FBACK	= 4,	// bras derrière le robot
	TMA_POWER	= 5,	// bras derrière le robot
	TMA_OTHER	= 6,	// bras derrière un robot amis
	TMA_GRAB	= 7,	// prend immédiatement
};

enum TaskManipHand
{
	TMH_OPEN	= 1,	// pince ouverte
	TMH_CLOSE	= 2,	// pince fermée
};



class CTaskManip : public CTask
{
public:
	CTaskManip(CInstanceManager* iMan, CObject* object);
	~CTaskManip();

	BOOL		EventProcess(const Event &event);

	Error		Start(TaskManipOrder order, TaskManipArm arm);
	Error		IsEnded();
	BOOL		Abort();

protected:
	void		InitAngle();
	CObject*	SearchTakeUnderObject(D3DVECTOR &pos, float dLimit);
	CObject*	SearchTakeFrontObject(BOOL bAdvance, D3DVECTOR &pos, float &distance, float &angle);
	CObject*	SearchTakeBackObject(BOOL bAdvance, D3DVECTOR &pos, float &distance, float &angle);
	CObject*	SearchOtherObject(BOOL bAdvance, D3DVECTOR &pos, float &distance, float &angle, float &height);
	BOOL		TruckTakeObject();
	BOOL		TruckDeposeObject();
	BOOL		IsFreeDeposeObject(D3DVECTOR pos);
	void		SoundManip(float time, float amplitude=1.0f, float frequency=1.0f);

protected:
	TaskManipOrder	m_order;
	TaskManipArm	m_arm;
	TaskManipHand	m_hand;
	int				m_step;
	float			m_speed;
	float			m_progress;
	float			m_initialAngle[5];
	float			m_finalAngle[5];
	float			m_height;
	float			m_advanceLength;
	float			m_energy;
	BOOL			m_bError;
	BOOL			m_bTurn;
	BOOL			m_bSubm;
	BOOL			m_bBee;
	float			m_angle;
	float			m_move;
	D3DVECTOR		m_targetPos;
	float			m_timeLimit;
	ObjectType		m_fretType;
};


#endif //_TASKMANIP_H_
