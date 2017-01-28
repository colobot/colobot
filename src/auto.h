// auto.h

#ifndef _AUTO_H_
#define	_AUTO_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CLight;
class CTerrain;
class CCloud;
class CPlanet;
class CBlitz;
class CCamera;
class CObject;
class CInterface;
class CRobotMain;
class CDisplayText;
class CWindow;
class CSound;




class CAuto
{
public:
	CAuto(CInstanceManager* iMan, CObject* object);
	virtual ~CAuto();

	virtual void	DeleteObject(BOOL bAll=FALSE);

	virtual void	Init();
	virtual void	Start(int param);
	virtual BOOL	EventProcess(const Event &event);
	virtual Error	IsEnded();
	virtual BOOL	Abort();

	virtual BOOL	SetType(ObjectType type);
	virtual BOOL	SetValue(int rank, float value);
	virtual BOOL	SetString(char *string);

	virtual	Error	SetAction(int action, float time=0.2f);
	virtual int		RetAction();

	virtual Error	RetError();

	virtual BOOL	RetBusy();
	virtual void	SetBusy(BOOL bBuse);
	virtual void	InitProgressTotal(float total);
	virtual void	EventProgress(float rTime);

	virtual BOOL	RetMotor();
	virtual void	SetMotor(BOOL bMotor);

protected:
	void		UpdateInterface(float rTime);

protected:
	CInstanceManager* m_iMan;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CParticule*		m_particule;
	CLight*			m_light;
	CTerrain*		m_terrain;
	CCloud	*		m_cloud;
	CPlanet	*		m_planet;
	CBlitz*			m_blitz;
	CCamera*		m_camera;
	CInterface*		m_interface;
	CRobotMain*		m_main;
	CDisplayText*	m_displayText;
	CObject*		m_object;
	CSound*			m_sound;

	ObjectType		m_type;
	BOOL			m_bBusy;
	BOOL			m_bMotor;
	float			m_time;
	float			m_lastUpdateTime;
	float			m_progressTime;
	float			m_progressTotal;
	int				m_actionType;
	float			m_actionTime;
	float			m_actionProgress;
};


#endif //_AUTO_H_
