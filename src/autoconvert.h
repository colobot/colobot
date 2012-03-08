// autoconvert.h

#ifndef _AUTOCONVERT_H_
#define	_AUTOCONVERT_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoConvertPhase
{
	ACP_STOP		= 1,
	ACP_WAIT		= 2,
	ACP_CLOSE		= 3,	// ferme le couvervle
	ACP_ROTATE		= 4,	// tourne le couvercle
	ACP_OPEN		= 5,	// ouvre le couvercle
};



class CAutoConvert : public CAuto
{
public:
	CAutoConvert(CInstanceManager* iMan, CObject* object);
	~CAutoConvert();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();
	BOOL		Abort();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchStone(ObjectType type);
	BOOL		SearchVehicle();
	void		CreateMetal();

protected:
	AutoConvertPhase	m_phase;
	float				m_progress;
	float				m_speed;
	float				m_timeVirus;
	float				m_lastParticule;
	BOOL				m_bResetDelete;
	BOOL				m_bSoundClose;
	int					m_soundChannel;
};


#endif //_AUTOCONVERT_H_
