// motionvehicle.h

#ifndef _MOTIONVEHICLE_H_
#define	_MOTIONVEHICLE_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


class CMotionVehicle : public CMotion
{
public:
	CMotionVehicle(CInstanceManager* iMan, CObject* object);
	~CMotionVehicle();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type, float power);
	BOOL		EventProcess(const Event &event);

	BOOL		RetTraceDown();
	void		SetTraceDown(BOOL bDown);
	int			RetTraceColor();
	void		SetTraceColor(int color);
	float		RetTraceWidth();
	void		SetTraceWidth(float width);

protected:
	void		CreatePhysics(ObjectType type);
	BOOL		EventFrame(const Event &event);
	BOOL		EventFrameFly(const Event &event);
	BOOL		EventFrameInsect(const Event &event);
	BOOL		EventFrameCanoni(const Event &event);
	void		UpdateTrackMapping(float left, float right, ObjectType type);

protected:
	float		m_wheelTurn[4];
	float		m_flyPaw[3];
	float		m_posTrackLeft;
	float		m_posTrackRight;
	int			m_partiReactor;
	float		m_armTimeAbs;
	float		m_armMember;
	float		m_canonTime;
	float		m_lastTimeCanon;
	D3DVECTOR	m_wheelLastPos;
	D3DVECTOR	m_wheelLastAngle;
	D3DVECTOR	m_posKey;
	BOOL		m_bFlyFix;
	BOOL		m_bTraceDown;
	int			m_traceColor;
	float		m_traceWidth;
};


#endif //_MOTIONVEHICLE_H_
