// autoflash.h

#ifndef _AUTOFLASH_H_
#define	_AUTOFLASH_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoFlash : public CAuto
{
public:
	CAutoFlash(CInstanceManager* iMan, CObject* object);
	~CAutoFlash();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:

protected:
	ObjectType	m_type;
	int			m_parti[10];
};


#endif //_AUTOFLASH_H_
