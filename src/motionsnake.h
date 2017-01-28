// motionsnake.h

#ifndef _MOTIONSNAKE_H_
#define	_MOTIONSNAKE_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum MotionSnakePhase
{
	MSP_WAIT	= 0,	// attend
	MSP_JUMP	= 1,	// saute
};




class CMotionSnake : public CMotion
{
public:
	CMotionSnake(CInstanceManager* iMan, CObject* object);
	~CMotionSnake();

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
	MotionSnakePhase m_phase;
	float			m_progress;
	float			m_speed;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	float			m_level;
	float			m_height;
	BOOL			m_bPlouf1;
	BOOL			m_bPlouf2;
};


#endif //_MOTIONSNAKE_H_
