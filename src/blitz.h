// blitz.h

#ifndef _BLITZ_H_
#define	_BLITZ_H_


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CCamera;
class CSound;



#define BLITZPARA	200.0f		// rayon de protection du paratonnerre
#define BLITZMAX	50

enum BlitzPhase
{
	BPH_WAIT,
	BPH_BLITZ,
};



class CBlitz
{
public:
	CBlitz(CInstanceManager* iMan, CD3DEngine* engine);
	~CBlitz();

	void		Flush();
	BOOL		EventProcess(const Event &event);
	BOOL		Create(float sleep, float delay, float magnetic);
	BOOL		GetStatus(float &sleep, float &delay, float &magnetic, float &progress);
	BOOL		SetStatus(float sleep, float delay, float magnetic, float progress);
	void		Draw();

protected:
	BOOL		EventFrame(const Event &event);
	CObject*	SearchObject(D3DVECTOR pos);

protected:
	CInstanceManager*	m_iMan;
	CD3DEngine*			m_engine;
	CTerrain*			m_terrain;
	CCamera*			m_camera;
	CSound*				m_sound;

	BOOL		m_bBlitzExist;
	float		m_sleep;
	float		m_delay;
	float		m_magnetic;
	BlitzPhase	m_phase;
	float		m_time;
	float		m_speed;
	float		m_progress;
	D3DVECTOR	m_pos;
	FPOINT		m_shift[BLITZMAX];
	float		m_width[BLITZMAX];
};


#endif //_BLITZ_H_
