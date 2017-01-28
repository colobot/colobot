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
class CBrain;
class CPhysics;
class CMotion;
class CObject;
class CRobotMain;
class CDisplayText;
class CSound;


#define TAKE_DIST		6.0f		// distance d'un objet pour le prendre
#define TAKE_DIST_OTHER	1.5f		// distance supplémentaire si sur ami

//?#define ARM_NEUTRAL_ANGLE1	 155.0f*PI/180.0f
//?#define ARM_NEUTRAL_ANGLE2	-125.0f*PI/180.0f
//?#define ARM_NEUTRAL_ANGLE3	 -45.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE1	 110.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE2	-130.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE3	 -50.0f*PI/180.0f

#define ARM_STOCK_ANGLE1	 110.0f*PI/180.0f
#define ARM_STOCK_ANGLE2	-100.0f*PI/180.0f
#define ARM_STOCK_ANGLE3	 -70.0f*PI/180.0f


class CTask
{
public:
	CTask(CInstanceManager* iMan, CObject* object);
	virtual ~CTask();

	virtual BOOL	EventProcess(const Event &event);
	virtual	Error	IsEnded();
	virtual BOOL	Abort();

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
	CBrain*			m_brain;
	CPhysics*		m_physics;
	CObject*		m_object;
	CRobotMain*		m_main;
	CDisplayText*	m_displayText;
	CSound*			m_sound;
};


#endif //_TASK_H_
