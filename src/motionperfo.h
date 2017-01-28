// motionperfo.h

#ifndef _MOTIONPERFO_H_
#define	_MOTIONPERFO_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;


#define MPERFO_WAIT		0		// attend au repos
#define MPERFO_ERROR	1		// opération impossible
#define MPERFO_ACTION	2		// perfore



class CMotionPerfo : public CMotion
{
public:
	CMotionPerfo(CInstanceManager* iMan, CObject* object);
	~CMotionPerfo();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

	void		SetActionLinSpeed(float speed);
	void		SetActionCirSpeed(float speed);

protected:
	BOOL		EventFrame(const Event &event);
	void		PerfoFrame(float rTime);
	void		ParticuleFrame(float rTime, float smoke, float error);
	void		UpdateTrackMapping(float left, float right);
	void		StartMotor(float freq);
	void		StopMotor();
	CObject*	SearchBox(D3DVECTOR center, float radius);

protected:
	float		m_time;
	float		m_lastParticule;
	float		m_lastParticulePerfo;
	float		m_leftTrack;
	float		m_rightTrack;
	int			m_channelSound;
	float		m_motorFreq;
};


#endif //_MOTIONPERFO_H_
