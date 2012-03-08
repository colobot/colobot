// autoflag.h

#ifndef _AUTOFLAG_H_
#define	_AUTOFLAG_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



class CAutoFlag : public CAuto
{
public:
	CAutoFlag(CInstanceManager* iMan, CObject* object);
	~CAutoFlag();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	Error		RetError();

protected:

protected:
	float		m_strong;
	int			m_param;
	float		m_progress;
};


#endif //_AUTOFLAG_H_
