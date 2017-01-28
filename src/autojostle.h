// autojostle.h

#ifndef _AUTOJOSTLE_H_
#define	_AUTOJOSTLE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CAutoJostle : public CAuto
{
public:
	CAutoJostle(CInstanceManager* iMan, CObject* object);
	~CAutoJostle();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int param, float force);
	BOOL		EventProcess(const Event &event);
	Error		IsEnded();

protected:

protected:
	D3DVECTOR	m_zoom;
	float		m_force;
	float		m_freq;
	float		m_progress;
	float		m_speed;
	float		m_lastParticule;
	Error		m_error;
};


#endif //_AUTOJOSTLE_H_
