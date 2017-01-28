// task.h

#ifndef _TASK_H_
#define	_TASK_H_


class CInstanceManager;
class CD3DEngine;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CMotion;
class CObject;
class CRobotMain;
class CMainUndo;
class CDisplayText;
class CSound;



#define PUSH_DIST		8.0f		// distance d'une caisse pour la pousser



class CTask
{
public:
	CTask(CInstanceManager* iMan, CObject* object);
	virtual ~CTask();

	virtual BOOL	EventProcess(const Event &event);
	virtual	Error	IsEnded();
	virtual BOOL	Abort();
	virtual BOOL	IsUndoable();
	virtual BOOL	IsStopable();
	virtual BOOL	Stop();

	virtual void	WriteSituation();
	virtual void	ReadSituation();

protected:

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CLight*			m_light;
	CParticule*		m_particule;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CMotion*		m_motion;
	CObject*		m_object;
	CRobotMain*		m_main;
	CMainUndo*		m_undo;
	CDisplayText*	m_displayText;
	CSound*			m_sound;
};


#endif //_TASK_H_
