// autokid.h

#ifndef _AUTOKID_H_
#define	_AUTOKID_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



class CAutoKid : public CAuto
{
public:
	CAutoKid(CInstanceManager* iMan, CObject* object);
	~CAutoKid();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

protected:

protected:
	float		m_speed;
	float		m_progress;
	float		m_lastParticule;
	int			m_soundChannel;
	BOOL		m_bSilent;
};


#endif //_AUTOKID_H_
