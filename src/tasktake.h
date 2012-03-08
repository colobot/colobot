// tasktake.h

#ifndef _TASKTAKE_H_
#define	_TASKTAKE_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskTakeOrder
{
	TTO_TAKE	= 1,	// prend un objet
	TTO_DEPOSE	= 2,	// dépose l'objet
};

enum TaskTakeArm
{
	TTA_NEUTRAL	= 1,	// bras vide au repos
	TTA_FFRONT	= 2,	// bras au sol
	TTA_FRIEND	= 3,	// bras derrière un robot amis
};



class CTaskTake : public CTask
{
public:
	CTaskTake(CInstanceManager* iMan, CObject* object);
	~CTaskTake();

	BOOL		EventProcess(const Event &event);

	Error		Start();
	Error		IsEnded();
	BOOL		Abort();

protected:
	CObject*	SearchTakeObject(float &angle, float dLimit, float aLimit);
	CObject*	SearchFriendObject(float &angle, float dLimit, float aLimit);
	BOOL		TruckTakeObject();
	BOOL		TruckDeposeObject();
	BOOL		IsFreeDeposeObject(D3DVECTOR pos);

protected:
	CTerrain*		m_terrain;

	TaskTakeOrder	m_order;
	TaskTakeArm		m_arm;
	int				m_step;
	float			m_speed;
	float			m_progress;
	float			m_height;
	BOOL			m_bError;
	BOOL			m_bTurn;
	float			m_angle;
	ObjectType		m_fretType;
};


#endif //_TASKTAKE_H_
