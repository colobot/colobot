// autobase.h

#ifndef _AUTOBASE_H_
#define	_AUTOBASE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



#define PARAM_STOP				0		// run=0 -> stoppé et ouvert
#define PARAM_LANDING			1		// run=1 -> atterrissage
#define PARAM_PORTICO			2		// run=2 -> porté par le portique
#define PARAM_FIXSCENE			3		// run=3 -> stoppé et ouvert pour win/lost
#define PARAM_TRANSIT1			11		// run=11 -> transit dans l'espace
#define PARAM_TRANSIT2			12		// run=12 -> transit dans l'espace
#define PARAM_TRANSIT3			13		// run=13 -> transit dans l'espace


enum AutoBasePhase
{
	ABP_WAIT		= 1,	// attend
	ABP_START		= 2,	// démarrage

	ABP_LAND		= 3,	// atterissage
	ABP_OPENWAIT	= 4,	// attente avant ouverture
	ABP_OPEN		= 5,	// ouvre les portes
	ABP_OPEN2		= 6,	// ouvre les suppléments
	ABP_LDWAIT		= 7,	// attend

	ABP_CLOSE2		= 8,	// ferme les suppléments
	ABP_CLOSE		= 9,	// ferme les portes
	ABP_TOWAIT		= 10,	// attente avant décollage
	ABP_TAKEOFF		= 11,	// décollage

	ABP_PORTICO_MOVE = 12,	// portique avance
	ABP_PORTICO_WAIT1= 13,	// portique attend
	ABP_PORTICO_DOWN = 14,	// portique descend
	ABP_PORTICO_WAIT2= 15,	// portique attend
	ABP_PORTICO_OPEN = 16,	// portique s'ouvre

	ABP_TRANSIT_MOVE = 17,	// transit - déplacement
};



class CAutoBase : public CAuto
{
public:
	CAutoBase(CInstanceManager* iMan, CObject* object);
	~CAutoBase();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

protected:
	void		UpdateInterface();
	void		FreezeCargo(BOOL bFreeze);
	void		MoveCargo();
	Error		CheckCloseDoor();
	void		BeginTransit();
	void		EndTransit();

protected:
	AutoBasePhase	m_phase;
	BOOL			m_bOpen;
	float			m_progress;
	float			m_speed;
	float			m_lastParticule;
	float			m_lastMotorParticule;
	float			m_fogStart;
	float			m_deepView;
	D3DVECTOR		m_pos;
	D3DVECTOR		m_posSound;
	D3DVECTOR		m_finalPos;
	D3DVECTOR		m_lastPos;
	int				m_param;
	int				m_soundChannel;
	int				m_partiChannel[8];

	char			m_bgBack[100];
	char			m_bgName[100];
	D3DCOLOR		m_bgUp;
	D3DCOLOR		m_bgDown;
	D3DCOLOR		m_bgCloudUp;
	D3DCOLOR		m_bgCloudDown;
};


#endif //_AUTOBASE_H_
