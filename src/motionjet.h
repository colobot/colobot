// motionjet.h

#ifndef _MOTIONJET_H_
#define	_MOTIONJET_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CMotionJet : public CMotion
{
public:
	CMotionJet(CInstanceManager* iMan, CObject* object);
	~CMotionJet();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		CreateShadow(float radius, float intensity, D3DShadowType type);
	void		MoveShadow(float progress);

protected:
	float		m_progress;
	float		m_speed;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	int			m_channelSound;
};


#endif //_MOTIONJET_H_
