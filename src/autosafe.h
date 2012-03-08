// autosafe.h

#ifndef _AUTOSAFE_H_
#define	_AUTOSAFE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoSafePhase
{
	ASAP_WAIT		= 1,
	ASAP_OPEN		= 2,
	ASAP_FINISH		= 3,
};



class CAutoSafe : public CAuto
{
public:
	CAutoSafe(CInstanceManager* iMan, CObject* object);
	~CAutoSafe();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	int			CountKeys();
	void		LockKeys();
	void		DownKeys(float progress);
	void		DeleteKeys();
	CObject*	SearchVehicle();

protected:
	AutoSafePhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastParticule;
	int				m_channelSound;
	BOOL			m_bLock;
	int				m_countKeys;
	float			m_actualAngle;
	float			m_finalAngle;
	BOOL			m_bKey[4];
	D3DVECTOR		m_keyPos[4];
	int				m_keyParti[4];
};


#endif //_AUTOSAFE_H_
