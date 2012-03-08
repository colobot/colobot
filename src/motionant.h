// motionant.h

#ifndef _MOTIONANT_H_
#define	_MOTIONANT_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


#define MA_MARCH		0
#define MA_STOP			1
#define MA_SPEC			2

#define MAS_PREPARE		0
#define MAS_FIRE		1
#define MAS_TERMINATE	2
#define MAS_BURN		3
#define MAS_RUIN		4
#define MAS_BACK1		5
#define MAS_BACK2		6
#define MAS_BACK3		7


class CMotionAnt : public CMotion
{
public:
	CMotionAnt(CInstanceManager* iMan, CObject* object);
	~CMotionAnt();

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
	short		m_armAngles[3*3*3*3*3 + 3*3*3*8];
	int			m_armTimeIndex;
	int			m_armPartIndex;
	int			m_armMemberIndex;
	int			m_armLastAction;
	BOOL		m_bArmStop;
	float		m_lastParticule;
};


#endif //_MOTIONANT_H_
