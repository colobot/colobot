// autoinfo.h

#ifndef _AUTOINFO_H_
#define	_AUTOINFO_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoInfoPhase
{
	AIP_WAIT		= 1,
	AIP_EMETTE		= 2,
	AIP_RECEIVE		= 3,
	AIP_ERROR		= 4,
};



class CAutoInfo : public CAuto
{
public:
	CAutoInfo(CInstanceManager* iMan, CObject* object);
	~CAutoInfo();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		UpdateInterface(float rTime);
	void		UpdateList();
	void		UpdateListVirus();

protected:
	AutoInfoPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastParticule;
	D3DVECTOR		m_goal;
	BOOL			m_bLastVirus;
};


#endif //_AUTOINFO_H_
