// autodoor.h

#ifndef _AUTODOOR_H_
#define	_AUTODOOR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoDoorPhase
{
	ADOP_WAIT	= 1,	// attend porte fermée
	ADOP_OPEN	= 2,	// ouvre la porte
	ADOP_STOP	= 3,	// attend porte ouverte
	ADOP_CLOSE	= 4,	// ferme la porte
};



class CAutoDoor : public CAuto
{
public:
	CAutoDoor(CInstanceManager* iMan, CObject* object);
	~CAutoDoor();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

	void		WriteSituation();
	void		ReadSituation();

protected:
	void		MoveDoor(float progress);
	void		UpdateLockZone(BOOL bOpen);
	void		FireStopUpdate();
	CObject*	SearchKey();
	CObject*	SearchObject(D3DVECTOR center, float radius);
	void		OpenParticule();

protected:
	ObjectType		m_type;		// OBJECT_DOORn
	AutoDoorPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_lastParticuleFire;
	float			m_lastParticuleRay;
	int				m_partiStop;
	D3DVECTOR		m_posKey;
};


#endif //_AUTODOOR_H_
