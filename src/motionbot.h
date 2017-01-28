// motionbot.h

#ifndef _MOTIONBOT_H_
#define	_MOTIONBOT_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;


#define MBOT_WAIT		0		// attend au repos
#define MBOT_YOUPIE		1		// content
#define MBOT_GOAL		2		// suspendu au ballon
#define MBOT_TURN		3		// tourne
#define MBOT_WALK1		4		// marche 1
#define MBOT_WALK2		5		// marche 2
#define MBOT_WALK3		6		// marche 3
#define MBOT_WALK4		7		// marche 4
#define MBOT_MAX		8



class CMotionBot : public CMotion
{
public:
	CMotionBot(CInstanceManager* iMan, CObject* object);
	~CMotionBot();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

	void		WriteSituation();
	void		ReadSituation();

protected:
	void		CreatePhysics();
	BOOL		EventFrame(const Event &event);
	void		BrainFrameCrazy();
	CObject*	SearchBlupi(D3DVECTOR center);
	void		CrazyParticule(BOOL bTurn);
	void		FireBot2();
	void		FireEvil1a();
	void		FireEvil1b();
	void		StepSound();

protected:
	int			m_option;
	float		m_aTime;
	short		m_armAngles[3*20*MBOT_MAX];
	int			m_armPartIndex;
	float		m_lastParticule;
	float		m_lastSound;
	float		m_walkTime;
	D3DVECTOR	m_cirVib;
	BOOL		m_bFirstSound;
};


#endif //_MOTIONBOT_H_
