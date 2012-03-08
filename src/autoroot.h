// autoroot.h

#ifndef _AUTOROOT_H_
#define	_AUTOROOT_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



class CAutoRoot : public CAuto
{
public:
	CAutoRoot(CInstanceManager* iMan, CObject* object);
	~CAutoRoot();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

protected:

protected:
	float			m_lastParticule;
	D3DVECTOR		m_center;
};


#endif //_AUTOROOT_H_
