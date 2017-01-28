// autoinca.h

#ifndef _AUTOINCA_H_
#define	_AUTOINCA_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoIncaPhase
{
	AINP_WAIT		= 1,	// attend qu'on presse le bouton
	AINP_OPEN		= 2,	// ouvre les 2 portes
	AINP_DOWN		= 3,	// descend la colonne
	AINP_BLITZ		= 4,	// démarre les éclairs
	AINP_TERM		= 5,	// attente finale
};


class CAutoInca : public CAuto
{
public:
	CAutoInca(CInstanceManager* iMan, CObject* object);
	~CAutoInca();

	void		DeleteObject(BOOL bAll=FALSE);

	void		Init();
	void		Start(int param);
	BOOL		EventProcess(const Event &event);
	BOOL		Abort();
	Error		RetError();

protected:
	CObject*	SearchObject(ObjectType type, D3DVECTOR center, float radius);
	void		CameraBegin();
	void		CameraEnd();
	void		CreateBlitz(float delay);
	void		ProgressBlitz(float progress);
	void		BreakDownEvil3();
	void		SoundManip(float time, float amplitude, float frequency);
	void		SoundBlitz(float time);

protected:
	AutoIncaPhase m_phase;
	float		m_progress;
	float		m_speed;

	CObject*	m_vehicle;
	CObject*	m_king;
	float		m_lastParticule;
	D3DVECTOR	m_targetPos;
	CameraType	m_cameraType;
	int			m_channelRay;
	BOOL		m_bBlitz;
};


#endif //_AUTOINCA_H_
