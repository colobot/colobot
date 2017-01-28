// autohome.h

#ifndef _AUTOHOME_H_
#define	_AUTOHOME_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoHomePhase
{
	AHOP_WAIT		= 1,	// attend véhicule avec robot
	AHOP_STOPCHECK	= 2,	// attend véhicule immobile
	AHOP_OPEN		= 3,	// ouvre le toît
	AHOP_MOVE		= 4,	// déplace le robot
	AHOP_CLOSE		= 5,	// ferme le toît
	AHOP_LIVE1		= 6,	// maison définitivement habitée
	AHOP_LIVE2		= 7,	// 
	AHOP_LIVE3		= 8,	// 
	AHOP_LIVE4		= 9,	// 
	AHOP_LIVE5		= 10,	// 
	AHOP_BREAKDOWN	= 11,	// panne
};



class CAutoHome : public CAuto
{
public:
	CAutoHome(CInstanceManager* iMan, CObject* object);
	~CAutoHome();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	void		FireStopUpdate();
	void		StartingEffect();
	void		MoveBot(float progress, float rTime);
	void		EndingEffect();
	CObject*	SearchVehicle();
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float radius);
	BOOL		ProxiSelect(float dist);
	D3DVECTOR	RetVehiclePoint(CObject *pObj);
	void		StartVehicleAction(int action);
	void		StartBotAction(int action, float delay=2.0f);
	void		HappyBlupi();

protected:
	AutoHomePhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_lastParticule;
	int				m_partiStop[6];
	ObjectType		m_type;
	CObject*		m_vehicle;
	CObject*		m_bot;
	D3DVECTOR		m_vehiclePos;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	D3DVECTOR		m_dir;
	float			m_startAngle;
	float			m_goalAngle;
	float			m_beforeClose;
	CameraType		m_cameraType;
	BOOL			m_bZoomIn;
	BOOL			m_bBreakFinish;
	int				m_breakPhase[4];
	float			m_breakTimeWait[4];
	float			m_breakTimeBzzz[4];
	int				m_lastDir;
	int				m_channelSound;
};


#endif //_AUTOHOME_H_
