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
	ADCP_WAIT		= 1,	// attend véhicule
	ADCP_STOPCHECK	= 2,	// attend véhicule immobile

	ADCP_OUTMOVE1	= 10,	// va vers stock
	ADCP_OUTDOWN1	= 11,	// 
	ADCP_OUTUP1		= 12,	// 
	ADCP_OUTMOVE2	= 13,	// va vers véhicule
	ADCP_OUTDOWN2	= 14,	// 
	ADCP_OUTUP2		= 15,	// 

	ADCP_INMOVE1	= 20,	// va vers véhicule
	ADCP_INDOWN1	= 21,	// 
	ADCP_INUP1		= 22,	// 
	ADCP_INMOVE2	= 23,	// va vers stock
	ADCP_INDOWN2	= 24,	// 
	ADCP_INUP2		= 25,	// 

	ADCP_START		= 30,	// attend le départ
};



class CAutoDock : public CAuto
{
public:
	CAutoDock(CInstanceManager* iMan, CObject* object);
	~CAutoDock();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	void		FireStopUpdate(BOOL bLightOn);
	void		MoveDock();
	CObject*	SearchEvil();
	CObject*	SearchVehicle();
	CObject*	SearchStockOut();
	float		RetObjectHeight(CObject *pObj);
	D3DVECTOR	RetVehiclePoint(CObject *pObj);
	BOOL		SearchFreePos(D3DVECTOR &os);
	void		ParticuleFrame(float rTime);
	void		SoundManip(float time, float amplitude=1.0f, float frequency=1.0f);
	void		StartBzzz();
	void		StopBzzz();
	void		StartVehicleAction(int action);
	void		TruckObject(CObject *pObj, BOOL bTake);
	void		ArmObject(CObject *pObj, BOOL bTake);
	void		CameraBegin();
	void		CameraEnd();

protected:
	AutoDockPhase	m_phase;
	float			m_progress;
	float			m_speed;
	int				m_partiStop[6];
	D3DVECTOR		m_center;
	D3DVECTOR		m_currentPos;
	D3DVECTOR		m_startPos;
	D3DVECTOR		m_goalPos;
	float			m_startAngle;
	float			m_goalAngle;
	float			m_heightFret;
	float			m_heightVehicle;
	CObject*		m_vehicle;
	CObject*		m_fret;
	D3DVECTOR		m_fretPos;
	D3DVECTOR		m_fretOffset;
	D3DVECTOR		m_vehiclePos;
	float			m_lastParticule;
	float			m_lastEffect;
	int				m_channelSound;
	CameraType		m_cameraType;
};


#endif //_AUTODOCK_H_
