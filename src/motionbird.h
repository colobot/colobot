// motionbird.h

#ifndef _MOTIONBIRD_H_
#define	_MOTIONBIRD_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



typedef struct
{
	float		speed;		// vitesse (~1.0)
	float		time;		// temps absolu
	D3DVECTOR	pos;		// position relative
}
BirdDesc;




class CMotionBird : public CMotion
{
public:
	CMotionBird(CInstanceManager* iMan, CObject* object);
	~CMotionBird();

	void		DeleteObject(BOOL bAll=FALSE);
	BOOL		Create(D3DVECTOR pos, float angle, ObjectType type);
	BOOL		EventProcess(const Event &event);
	Error		SetAction(int action, float time=0.2f);

	float		RetLinSpeed();
	float		RetCirSpeed();
	float		RetLinStopLength();

protected:
	BOOL		EventFrame(const Event &event);
	BOOL		CreateShadow(int i, float radius, float intensity, D3DShadowType type);
	void		MoveShadow(int i, float progress);

protected:
	float		m_progress;
	float		m_speed;
	D3DVECTOR	m_startPos;
	D3DVECTOR	m_goalPos;
	int			m_total;			// nb total d'oiseaux
	BirdDesc	m_birdTable[10];	// descripteurs des oiseaux
};


#endif //_MOTIONBIRD_H_
