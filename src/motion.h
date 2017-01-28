// motion.h

#ifndef _MOTION_H_
#define	_MOTION_H_


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CObject;
class CRobotMain;
class CMainUndo;
class CSound;



enum TypePart
{
	TP_TOP			= 0,		// toît
};


enum WheelType
{
	WT_NORM			= 0,		// roues normales
	WT_BURN			= 1,		// roues cramées
	WT_SLIDE		= 2,		// roues glissantes
};



class CMotion
{
public:
	CMotion(CInstanceManager* iMan, CObject* object);
	virtual ~CMotion();

	virtual void	DeleteObject(BOOL bAll=FALSE);
	virtual BOOL	Create(D3DVECTOR pos, float angle, ObjectType type);
	virtual BOOL	EventProcess(const Event &event);
	virtual	Error	SetAction(int action, float speed=0.2f);
	virtual int		RetAction();
	virtual float	RetActionProgress();

	virtual BOOL	SetParam(int rank, float value);
	virtual float	RetParam(int rank);

	virtual void		SetLinVibration(D3DVECTOR dir);
	virtual D3DVECTOR	RetLinVibration();
	virtual void		SetCirVibration(D3DVECTOR dir);
	virtual D3DVECTOR	RetCirVibration();
	virtual void		SetInclinaison(D3DVECTOR dir);
	virtual D3DVECTOR	RetInclinaison();

	virtual float	RetLinSpeed();
	virtual float	RetCirSpeed();
	virtual float	RetLinStopLength();

	virtual void	SetActionProgress(float progress);
	virtual void	SetActionLinSpeed(float speed);
	virtual void	SetActionCirSpeed(float speed);

	virtual void		SetWheelType(WheelType type);
	virtual WheelType	RetWheelType();

	virtual int		RetStateLength();
	virtual void	GetStateBuffer(char *buffer);

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
	CObject*		m_object;
	CRobotMain*		m_main;
	CMainUndo*		m_undo;
	CSound*			m_sound;

	int				m_actionType;
	float			m_actionSpeed;
	float			m_actionTime;
	float			m_progress;

	D3DVECTOR		m_linVibration;		// vibration linéaire
	D3DVECTOR		m_cirVibration;		// vibration circulaire
	D3DVECTOR		m_inclinaison;		// inclinaison

	float			m_actionProgress;
	float			m_actionLinSpeed;
	float			m_actionCirSpeed;

	WheelType		m_wheelType;
};


#endif //_MOTION_H_
