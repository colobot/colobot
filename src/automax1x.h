// automax1x.h

#ifndef _AUTOMAX1X_H_
#define	_AUTOMAX1X_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



enum AutoMax1xPhase
{
	AMP_WAITOBJ,	// attend l'arrivée d'un objet
	AMP_WAITNULL,	// attend le départ d'un objet
	AMP_OPEN,		// ouvre les portes
	AMP_UP,			// monte le clown
	AMP_TERM,		// fini, bloqué
};



class CAutoMax1x : public CAuto
{
public:
	CAutoMax1x(CInstanceManager* iMan, CObject* object);
	~CAutoMax1x();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

	void		WriteSituation();
	void		ReadSituation();

protected:
	void		SwingClown(float rTime);

protected:
	D3DVECTOR		m_pos;
	AutoMax1xPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_swingSpeed;
	float			m_swingTime;
	float			m_lastParticule;
};


#endif //_AUTOMAX1X_H_
