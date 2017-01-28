// autodock.h

#ifndef _AUTODOCK_H_
#define	_AUTODOCK_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoDockPhase
{
	ADKP_WAIT	= 1,	// attend ordre
	ADKP_MOVEZP	= 2,	// translation sur les rails en z+
	ADKP_MOVEZM	= 3,	// translation sur les rails en z-
	ADKP_MOVEXP	= 4,	// charriot en x+
	ADKP_MOVEXM	= 5,	// charriot en x-
	ADKP_TAKE	= 6,	// prend un objet
	ADKP_ERROR	= 7,	// ne reste rien
};



class CAutoDock : public CAuto
{
public:
	CAutoDock(CInstanceManager* iMan, CObject* object);
	~CAutoDock();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	BOOL		IsRunning();
	Error		RetError();

	void		WriteSituation();
	void		ReadSituation();

protected:
	void		UpdatePosition();
	BOOL		DecRest();
	void		UpdateRest(float rTime);
	BOOL		IsFlatGround();
	CObject*	SearchFret();
	CObject*	SearchObject(D3DVECTOR center, float radius);
	BOOL		IsFreePos(D3DVECTOR dir);
	D3DVECTOR	CalcPosPiston();
	D3DVECTOR	CalcPosPiston(D3DVECTOR piston);
	void		StartAction(CObject *pObj, int action, float speed=0.2f);
	void		SoundManip(float time, float amplitude, float frequency);
	void		StartBzzz();
	void		StopBzzz();
	void		PosBzzz();

protected:
	int				m_rest;
	AutoDockPhase	m_phase;
	float			m_progress;
	float			m_progressRest;
	float			m_speed;
	D3DVECTOR		m_currentPos;
	D3DVECTOR		m_initialPos;
	D3DVECTOR		m_posPiston;
	D3DVECTOR		m_posPistonGround;
	BOOL			m_bTake;
	float			m_lastParticule;
	int				m_channelSound;
};


#endif //_AUTODOCK_H_
