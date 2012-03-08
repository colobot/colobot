// autoresearch.h

#ifndef _AUTORESEARCH_H_
#define	_AUTORESEARCH_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



enum AutoResearchPhase
{
	ALP_WAIT		= 1,
	ALP_SEARCH		= 2,	// recherche en cours
};



class CAutoResearch : public CAuto
{
public:
	CAutoResearch(CInstanceManager* iMan, CObject* object);
	~CAutoResearch();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		UpdateInterface();
	void		UpdateInterface(float rTime);
	void		OkayButton(CWindow *pw, EventMsg event);
	BOOL		TestResearch(EventMsg event);
	void		SetResearch(EventMsg event);
	void		FireStopUpdate(float progress, BOOL bLightOn);

protected:
	AutoResearchPhase	m_phase;
	float				m_progress;
	float				m_speed;
	float				m_timeVirus;
	float				m_lastUpdateTime;
	float				m_lastParticule;
	EventMsg			m_research;
	int					m_partiStop[6];
	int					m_channelSound;
};


#endif //_AUTORESEARCH_H_
