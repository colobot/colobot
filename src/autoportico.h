// autoportico.h

#ifndef _AUTOPORTICO_H_
#define	_AUTOPORTICO_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoPorticoPhase
{
	APOP_WAIT		= 1,	// attend
	APOP_START		= 2,	// départ de l'action
	APOP_MOVE		= 3,	// avance
	APOP_WAIT1		= 4,	// attend
	APOP_DOWN		= 5,	// descend
	APOP_WAIT2		= 6,	// attend
	APOP_OPEN		= 7,	// ouvre
};



class CAutoPortico : public CAuto
{
public:
	CAutoPortico(CInstanceManager* iMan, CObject* object);
	~CAutoPortico();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	void		UpdateTrackMapping(float left, float right);

protected:
	AutoPorticoPhase m_phase;
	float			m_progress;
	float			m_speed;
	float			m_cameraProgress;
	float			m_cameraSpeed;
	float			m_lastParticule;
	D3DVECTOR		m_finalPos;
	D3DVECTOR		m_startPos;
	float			m_posTrack;
	int				m_param;
	int				m_soundChannel;
};


#endif //_AUTOPORTICO_H_
