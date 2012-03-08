// motionworm.h

#ifndef _MOTIONWORM_H_
#define	_MOTIONWORM_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


class CMotionWorm : public CMotion
{
public:
	CMotionWorm(CInstanceManager* iMan, CObject* object);
	~CMotionWorm();

	void	DeleteObject(BOOL bAll=FALSE);
	BOOL	Create(D3DVECTOR pos, float angle, ObjectType type, float power);
	BOOL	EventProcess(const Event &event);

	BOOL	SetParam(int rank, float value);
	float	RetParam(int rank);

protected:
	void	CreatePhysics();
	BOOL	EventFrame(const Event &event);

protected:
	float		m_timeUp;
	float		m_timeDown;
	float		m_armMember;
	float		m_armTimeAbs;
	float		m_armTimeMarch;
	float		m_armTimeAction;
	short		m_armAngles[3*3*3*3*10];
	int			m_armTimeIndex;
	int			m_armPartIndex;
	int			m_armMemberIndex;
	int			m_armLastAction;
	float		m_armLinSpeed;
	float		m_armCirSpeed;
	int			m_specAction;
	float		m_specTime;
	BOOL		m_bArmStop;
	float		m_lastParticule;
};


#endif //_MOTIONWORM_H_
