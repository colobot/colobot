// autopara.h

#ifndef _AUTOPARA_H_
#define	_AUTOPARA_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoParaPhase
{
	APAP_WAIT		= 1,
	APAP_BLITZ		= 2,
	APAP_CHARGE		= 3,
};



class CAutoPara : public CAuto
{
public:
	CAutoPara(CInstanceManager* iMan, CObject* object);
	~CAutoPara();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();
	void		StartBlitz();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	void		ChargeObject(float rTime);

protected:
	AutoParaPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_timeVirus;
	float			m_lastParticule;
	D3DVECTOR		m_pos;
	int				m_channelSound;
};


#endif //_AUTOPARA_H_
