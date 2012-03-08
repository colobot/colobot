// autolabo.h

#ifndef _AUTOLABO_H_
#define	_AUTOLABO_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoLaboPhase
{
	ALAP_WAIT		= 1,
	ALAP_OPEN1		= 2,
	ALAP_OPEN2		= 3,
	ALAP_OPEN3		= 4,
	ALAP_ANALYSE	= 5,
	ALAP_CLOSE1		= 6,
	ALAP_CLOSE2		= 7,
	ALAP_CLOSE3		= 8,
};



class CAutoLabo : public CAuto
{
public:
	CAutoLabo(CInstanceManager* iMan, CObject* object);
	~CAutoLabo();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		UpdateInterface();
	void		OkayButton(CWindow *pw, EventMsg event);
	BOOL		TestResearch(EventMsg event);
	void		SetResearch(EventMsg event);
	void		SoundManip(float time, float amplitude, float frequency);

protected:
	AutoLaboPhase		m_phase;
	float				m_progress;
	float				m_speed;
	float				m_timeVirus;
	float				m_lastParticule;
	EventMsg			m_research;
	int					m_partiRank[3];
	int					m_partiSphere;
	int					m_soundChannel;
};


#endif //_AUTOLABO_H_
