// autobomb.h

#ifndef _AUTOBOMB_H_
#define	_AUTOBOMB_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



#define AB_STOP		0
#define AB_TRUCK	1
#define AB_BURNON	2
#define AB_BURNOFF	3
#define AB_BURNFLIP	4



class CAutoBomb : public CAuto
{
public:
	CAutoBomb(CInstanceManager* iMan, CObject* object);
	~CAutoBomb();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	Error		SetAction(int action, float time);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	void		SpeedAdapt(D3DVECTOR &angle, float rTime);

protected:
	float		m_progress;
	float		m_speed;
	D3DVECTOR	m_angle;
	BOOL		m_bBurn;
	float		m_fireDelay;
	float		m_fireProgress;
	float		m_lastParticule;
	int			m_partiSpark;
};


#endif //_AUTOBOMB_H_
