// motiontrax.h

#ifndef _MOTIONTRAX_H_
#define	_MOTIONTRAX_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;


#define MTRAX_WAIT		0		// attend au repos
#define MTRAX_ERROR		1		// opération impossible
#define MTRAX_ROLL		2		// pousse une sphère



class CMotionTrax : public CMotion
{
public:
	CMotionTrax(CInstanceManager* iMan, CObject* object);
	~CMotionTrax();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	BOOL		EventFrame(const Event &event);
	void		ParticuleFrame(float rTime, float smoke, float error);
	void		UpdateTrackMapping(float left, float right);
	void		StartMotor(float freq);
	void		StopMotor();

protected:
	float		m_time;
	float		m_lastParticule;
	float		m_leftTrack;
	float		m_rightTrack;
	int			m_channelSound;
	float		m_motorFreq;
};


#endif //_MOTIONTRAX_H_
