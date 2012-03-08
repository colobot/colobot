// autoegg.h

#ifndef _AUTOEGG_H_
#define	_AUTOEGG_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



enum AutoEggPhase
{
	AEP_NULL	= 0,
	AEP_DELAY	= 1,
	AEP_INCUB	= 3,
	AEP_ZOOM	= 4,
	AEP_WAIT	= 5,
};



class CAutoEgg : public CAuto
{
public:
	CAutoEgg(CInstanceManager* iMan, CObject* object);
	~CAutoEgg();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	Error		IsEnded();
	Error		RetError();

	BOOL		SetType(ObjectType type);
	BOOL		SetValue(int rank, float value);
	BOOL		SetString(char *string);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchAlien();

protected:
	ObjectType		m_type;
	float			m_value;
	char			m_string[100];
	int				m_param;
	AutoEggPhase	m_phase;
	float			m_progress;
	float			m_speed;
};


#endif //_AUTOEGG_H_
