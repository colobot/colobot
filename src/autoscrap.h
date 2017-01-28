// autoscrap.h

#ifndef _AUTOSCRAP_H_
#define	_AUTOSCRAP_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoScrap : public CAuto
{
public:
	CAutoScrap(CInstanceManager* iMan, CObject* object);
	~CAutoScrap();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:

protected:
	float		m_lastParticule;
};


#endif //_AUTOSCRAP_H_
