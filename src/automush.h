// automush.h

#ifndef _AUTOMUSH_H_
#define	_AUTOMUSH_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



enum AutoMushPhase
{
	AMP_WAIT		= 1,
	AMP_SNIF		= 2,
	AMP_ZOOM		= 3,
	AMP_FIRE		= 4,
	AMP_SMOKE		= 5,
};



class CAutoMush : public CAuto
{
public:
	CAutoMush(CInstanceManager* iMan, CObject* object);
	~CAutoMush();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	BOOL		SearchTarget();

protected:
	AutoMushPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_lastParticule;
};


#endif //_AUTOMUSH_H_
