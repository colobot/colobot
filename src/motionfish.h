// motionfish.h

#ifndef _MOTIONFISH_H_
#define	_MOTIONFISH_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum MotionFishPhase
{
	MFP_WAIT	= 0,	// attend
	MFP_JUMP	= 1,	// saute
};




class CMotionFish : public CMotion
{
public:
	CMotionFish(CInstanceManager* iMan, CObject* object);
	~CMotionFish();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		SearchPosition();

protected:
	MotionFishPhase	m_phase;
	float			m_progress;
	float			m_speed;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	float			m_level;
	float			m_height;
	BOOL			m_bPlouf1;
	BOOL			m_bPlouf2;
};


#endif //_MOTIONFISH_H_
