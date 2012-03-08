// autonest.h

#ifndef _AUTONEST_H_
#define	_AUTONEST_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



enum AutoNestPhase
{
	ANP_WAIT		= 1,
	ANP_BIRTH		= 2,	// apparition d'un boulet
};



class CAutoNest : public CAuto
{
public:
	CAutoNest(CInstanceManager* iMan, CObject* object);
	~CAutoNest();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	BOOL		SearchFree(D3DVECTOR pos);
	void		CreateFret(D3DVECTOR pos, float angle, ObjectType type);
	CObject*	SearchFret();

protected:
	AutoNestPhase	m_phase;
	float			m_progress;
	float			m_speed;
	float			m_lastParticule;
	D3DVECTOR		m_fretPos;
};


#endif //_AUTONEST_H_
