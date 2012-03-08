// autofactory.h

#ifndef _AUTOFACTORY_H_
#define	_AUTOFACTORY_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoFactoryPhase
{
	AFP_WAIT		= 1,	// attend métal
	AFP_CLOSE_S		= 2,	// ferme les portes (shift)
	AFP_CLOSE_T		= 3,	// ferme les portes (turn)
	AFP_BUILD		= 4,	// construit le véhicule
	AFP_OPEN_T		= 5,	// ouvre les portes (turn)
	AFP_OPEN_S		= 6,	// ouvre les portes (shift)
	AFP_ADVANCE		= 7,	// avance devant la porte
};



class CAutoFactory : public CAuto
{
public:
	CAutoFactory(CInstanceManager* iMan, CObject* object);
	~CAutoFactory();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		UpdateInterface();
	void		UpdateButton(CWindow *pw, EventMsg event, BOOL bBusy);

	CObject*	SearchFret();
	BOOL		NearestVehicle();
	BOOL		CreateVehicle();
	CObject*	SearchVehicle();

	void		SoundManip(float time, float amplitude, float frequency);

protected:
	AutoFactoryPhase	m_phase;
	float				m_progress;
	float				m_speed;
	float				m_lastParticule;
	D3DVECTOR			m_fretPos;
	int					m_channelSound;
};


#endif //_AUTOFACTORY_H_
