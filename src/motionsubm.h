// motionsubm.h

#ifndef _MOTIONSUBM_H_
#define	_MOTIONSUBM_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum MotionSubmPhase
{
	MSBP_WAIT	= 0,	// attend
	MSBP_UP		= 1,	// sort de l'eau
	MSBP_LOOK	= 2,	// observe
	MSBP_MOVE	= 3,	// avance
	MSBP_DOWN	= 4,	// retourne sous l'eau
};




class CMotionSubm : public CMotion
{
public:
	CMotionSubm(CInstanceManager* iMan, CObject* object);
	~CMotionSubm();

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
	float			m_time;
	MotionSubmPhase	m_phase;
	float			m_progress;
	float			m_speed;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	float			m_level;
	float			m_angle;
	float			m_lastParticule;
};


#endif //_MOTIONSUBM_H_
