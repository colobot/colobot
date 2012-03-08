// autoderrick.h

#ifndef _AUTODERRICK_H_
#define	_AUTODERRICK_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ObjectType;



enum AutoDerrickPhase
{
	ADP_WAIT		= 1,
	ADP_EXCAVATE	= 2,	// descend le foret
	ADP_ASCEND		= 3,	// remonte le foret
	ADP_EXPORT		= 4,	// exporte la matière
	ADP_ISFREE		= 5,	// attend disparition matière
};



class CAutoDerrick : public CAuto
{
public:
	CAutoDerrick(CInstanceManager* iMan, CObject* object);
	~CAutoDerrick();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	BOOL		EventProcess(const Event &event);
	Error		RetError();

	BOOL		CreateInterface(BOOL bSelect);

	BOOL		Write(char *line);
	BOOL		Read(char *line);

protected:
	CObject*	SearchFret();
	BOOL		SearchFree(D3DVECTOR pos);
	void		CreateFret(D3DVECTOR pos, float angle, ObjectType type, float height);
	BOOL		ExistKey();

protected:
	AutoDerrickPhase	m_phase;
	float				m_progress;
	float				m_speed;
	float				m_timeVirus;
	float				m_lastParticule;
	float				m_lastTrack;
	D3DVECTOR			m_fretPos;
	int					m_soundChannel;
	BOOL				m_bSoundFall;
};


#endif //_AUTODERRICK_H_
