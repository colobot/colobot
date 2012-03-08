// taskshield.h

#ifndef _TASKSHIELD_H_
#define	_TASKSHIELD_H_


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


#define RADIUS_SHIELD_MIN	 40.0f		// rayon min de la zone protégée
#define RADIUS_SHIELD_MAX	100.0f		// rayon max de la zone protégée


enum TaskShieldPhase
{
	TS_UP1		= 1,	// monte
	TS_UP2		= 2,	// monte
	TS_SHIELD	= 3,	// bouclier déployé
	TS_SMOKE	= 4,	// fume
	TS_DOWN1	= 5,	// descend
	TS_DOWN2	= 6,	// descend
};

enum TaskShieldMode
{
	TSM_UP		= 1,	// déploie le bouclier
	TSM_DOWN	= 2,	// rentre le bouclier
	TSM_UPDATE	= 3,	// changement de rayon
};



class CTaskShield : public CTask
{
public:
	CTaskShield(CInstanceManager* iMan, CObject* object);
	~CTaskShield();

	BOOL		EventProcess(const Event &event);

	Error		Start(TaskShieldMode mode, float delay);
	Error		IsEnded();
	BOOL		IsBusy();
	BOOL		Abort();

protected:
	Error		Stop();
	BOOL		CreateLight(D3DVECTOR pos);
	void		IncreaseShield();
	float		RetRadius();

protected:
	TaskShieldPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_time;
	float			m_delay;
	float			m_lastParticule;
	float			m_lastRay;
	float			m_lastIncrease;
	float			m_energyUsed;
	BOOL			m_bError;
	D3DVECTOR		m_shieldPos;
	int				m_rankSphere;
	int				m_soundChannel;
	int				m_effectLight;
};


#endif //_TASKSHIELD_H_
