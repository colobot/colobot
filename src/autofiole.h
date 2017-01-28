// autofiole.h

#ifndef _AUTOFIOLE_H_
#define	_AUTOFIOLE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


class CAutoFiole : public CAuto
{
public:
	CAutoFiole(CInstanceManager* iMan, CObject* object);
	~CAutoFiole();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:

protected:
	D3DVECTOR	m_pos;
	ObjectType	m_type;
	float		m_lastParticule;
};


#endif //_AUTOFIOLE_H_
