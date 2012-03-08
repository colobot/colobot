// motionbee.h

#ifndef _MOTIONBEE_H_
#define	_MOTIONBEE_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


#define MB_MARCH		0
#define MB_SPEC			1

#define MBS_HOLD		0
#define MBS_BURN		1
#define MBS_RUIN		2


class CMotionBee : public CMotion
{
public:
	CMotionBee(CInstanceManager* iMan, CObject* object);
	~CMotionBee();

	void	DeleteObject(BOOL bAll=FALSE);
	BOOL	Create(D3DVECTOR pos, float angle, ObjectType type, float power);
	BOOL	EventProcess(const Event &event);

protected:
	void	CreatePhysics();
	BOOL	EventFrame(const Event &event);

protected:
	float		m_armMember;
	float		m_armTimeAbs;
	float		m_armTimeMarch;
	float		m_armTimeAction;
	short		m_armAngles[3*3*3*3*2];
	int			m_armTimeIndex;
	int			m_armPartIndex;
	int			m_armMemberIndex;
	int			m_armLastAction;
	BOOL		m_bArmStop;
};


#endif //_MOTIONBEE_H_
