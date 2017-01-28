// automeca.h

#ifndef _AUTOMECA_H_
#define	_AUTOMECA_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoMeca : public CAuto
{
public:
	CAutoMeca(CInstanceManager* iMan, CObject* object);
	~CAutoMeca();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		Start(int part);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:

protected:
	ObjectType	m_type;
	float		m_rand;
	float		m_lastParticule;
	int			m_param;
};


#endif //_AUTOMECA_H_
