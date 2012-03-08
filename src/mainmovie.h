// mainmovie.h

#ifndef _MAINMOVIE_H_
#define	_MAINMOVIE_H_


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CInterface;
class CRobotMain;
class CCamera;
class CSound;
class CObject;




enum MainMovieType
{
	MM_NONE,
	MM_SATCOMopen,
	MM_SATCOMclose,
};



class CMainMovie
{
public:
	CMainMovie(CInstanceManager* iMan);
	~CMainMovie();

	void			Flush();
	BOOL			Start(MainMovieType type, float time);
	BOOL			Stop();
	BOOL			IsExist();
	BOOL			EventProcess(const Event &event);
	MainMovieType	RetType();
	MainMovieType	RetStopType();

protected:

protected:
	CInstanceManager* m_iMan;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CRobotMain*		m_main;
	CCamera*		m_camera;
	CSound*			m_sound;

	MainMovieType	m_type;
	MainMovieType	m_stopType;
	float			m_speed;
	float			m_progress;
	D3DVECTOR		m_initialEye;
	D3DVECTOR		m_initialLookat;
	D3DVECTOR		m_finalEye[2];
	D3DVECTOR		m_finalLookat[2];
};


#endif //_MAINMOVIE_H_
