// autoalien.h

#ifndef _AUTOALIEN_H_
#define	_AUTOALIEN_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoAlien : public CAuto
{
public:
	CAutoAlien(CInstanceManager* iMan, CObject* object);
	~CAutoAlien();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	void		ArmPosition(D3DVECTOR &pos, float &dir, float angle, float lin);
	void		BotAction(CObject *bot, int action, float delay);
	void		SynchroHammer(int action);
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float radius);
	void		FireStopUpdate();
	void		UpdateTrackMapping(float progress);

protected:
	ObjectType	m_type;
	float		m_progress;
	float		m_speed;
	int			m_phase;
	float		m_lastParticule;
	float		m_trackProgress;
	float		m_rotAngle;
	float		m_rotSpeed;
	CObject*	m_bot;
	ObjectType	m_lastType;
	BOOL		m_bSynchro;
	BOOL		m_bStop;
	BOOL		m_bFear;
	int			m_partiStop[12];
};


#endif //_AUTOALIEN_H_
