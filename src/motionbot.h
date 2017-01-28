// motionbot.h

#ifndef _MOTIONBOT_H_
#define	_MOTIONBOT_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CBrain;
class CPhysics;
class CObject;


#define MB_WAIT		0		// attend au repos
#define MB_TRUCK	1		// soulevé par la grue
#define MB_FEAR		2		// peur
#define MB_GOHOME	3		// va à la maison
#define MB_HOME1	4		// à la maison 1
#define MB_HOME2	5		// à la maison 2
#define MB_FLIC		6		// gendarme (BOT1)
#define MB_FIRE		6		// tir (EVIL1)
#define MB_WALK1	7		// marche 1 (WALKER)
#define MB_WALK2	8		// marche 2 (WALKER)
#define MB_WALK3	9		// marche 3 (WALKER)
#define MB_WALK4	10		// marche 4 (WALKER)
#define MB_MAX		11

#define MB_HAPPY	100
#define MB_ANGRY	101
#define MB_BREAK	102
#define MB_REPAIR	103



class CMotionBot : public CMotion
{
public:
	CMotionBot(CInstanceManager* iMan, CObject* object);
	~CMotionBot();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type, BOOL bPlumb);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

protected:
	void		CreatePhysics();
	BOOL		EventFrame(const Event &event);
	void		SpeedAdapt(float effect[], D3DVECTOR &linVib, D3DVECTOR &cirVib, float rTime);
	void		FireBot2();
	void		FireEvil1a();
	void		FireEvil1b();
	void		BubbleBot1();
	void		UpdateFaceMapping(int face);

protected:
	int			m_option;
	int			m_face;
	float		m_aTime;
	short		m_armAngles[3*20*MB_MAX];
	int			m_armPartIndex;
	float		m_lastParticule;
	float		m_lastSound;
	float		m_walkTime;
	float		m_starterTime;
	int			m_starterPhase;
	int			m_partiGuide;
	CameraType	m_cameraType;
	CObject*	m_cameraObj;
	D3DVECTOR	m_cirVib;
	BOOL		m_bBreak;
};


#endif //_MOTIONBOT_H_
