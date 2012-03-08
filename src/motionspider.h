// motionspider.h

#ifndef _MOTIONSPIDER_H_
#define	_MOTIONSPIDER_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


#define MS_MARCH		0
#define MS_STOP			1
#define MS_SPEC			2

#define MSS_BURN		0
#define MSS_RUIN		1
#define MSS_EXPLO		2
#define MSS_BACK1		3
#define MSS_BACK2		4
#define MSS_BACK3		5


class CMotionSpider : public CMotion
{
public:
	CMotionSpider(CInstanceManager* iMan, CObject* object);
	~CMotionSpider();

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
	short		m_armAngles[3*4*4*3*3 + 3*4*4*6];
	int			m_armTimeIndex;
	int			m_armPartIndex;
	int			m_armMemberIndex;
	int			m_armLastAction;
	BOOL		m_bArmStop;
	float		m_lastParticule;
};


#endif //_MOTIONSPIDER_H_
