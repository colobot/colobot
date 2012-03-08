// motionmother.h

#ifndef _MOTIONMOTHER_H_
#define	_MOTIONMOTHER_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


class CMotionMother : public CMotion
{
public:
	CMotionMother(CInstanceManager* iMan, CObject* object);
	~CMotionMother();

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
	short		m_armAngles[3*3*3*3*10];
	int			m_armTimeIndex;
	int			m_armPartIndex;
	int			m_armMemberIndex;
	int			m_armLastAction;
	int			m_specAction;
	float		m_specTime;
	BOOL		m_bArmStop;
};


#endif //_MOTIONMOTHER_H_
