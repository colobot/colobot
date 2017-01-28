// brain.h

#ifndef _BRAIN_H_
#define	_BRAIN_H_


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CWater;
class CCamera;
class CObject;
class CPhysics;
class CMotion;
class CTaskManager;
class CInterface;
class CWindow;
class CDisplayText;
class CScript;
class CRobotMain;
class CSound;
class CParticule;

enum ObjectType;


#define BRAINMAXSCRIPT		10



class CBrain
{
public:
	CBrain(CInstanceManager* iMan, CObject* object);
	~CBrain();

	void		DeleteObject(BOOL bAll=FALSE);

	void		SetPhysics(CPhysics* physics);
	void		SetMotion(CMotion* motion);

	BOOL		EventProcess(const Event &event);
	BOOL		CreateInterface(BOOL bSelect);

	void		SetActivity(BOOL bMode);
	BOOL		RetActivity();
	BOOL		IsProgram();
	BOOL		ProgramExist(int rank);
	void		RunProgram(int rank);
	int			FreeProgram();
	int			RetProgram();
	void		StopProgram();
	void		StopTask();

	void		SetScriptRun(int rank);
	int			RetScriptRun();
	void		SetScriptName(int rank, char *name);
	char*		RetScriptName(int rank);
	void		SetSoluceName(char *name);
	char*		RetSoluceName();

	BOOL		ReadSoluce(char* filename);
	BOOL		ReadProgram(int rank, char* filename);
	BOOL		RetCompile(int rank);
	BOOL		WriteProgram(int rank, char* filename);
	BOOL		ReadStack(FILE *file);
	BOOL		WriteStack(FILE *file);

	Error		StartTaskFire(float delay);

	void		UpdateInterface(float rTime);
	void		UpdateInterface();

protected:
	BOOL		EventFrame(const Event &event);

	Error		EndedTask();
	void		ColorFlag(int color);

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CObject*		m_object;
	CPhysics*		m_physics;
	CMotion*		m_motion;
	CInterface*		m_interface;
	CDisplayText*	m_displayText;
	CRobotMain*		m_main;
	CSound*			m_sound;
	CParticule*		m_particule;
	CTaskManager*	m_primaryTask;
	CTaskManager*	m_secondaryTask;

	CScript*	m_script[BRAINMAXSCRIPT];
	int			m_selScript;		// rang du script sélectionné
	int			m_program;			// rang du programme exécuté / -1
	BOOL		m_bActivity;
	BOOL		m_bBurn;

	int			m_scriptRun;
	char		m_scriptName[BRAINMAXSCRIPT][50];
	char		m_soluceName[50];

	EventMsg	m_buttonAxe;
	EventMsg	m_interfaceEvent[100];

	CObject*	m_beeBullet;
	float		m_beeBulletSpeed;
	D3DVECTOR	m_startPos;
	float		m_time;
	float		m_burnTime;
	float		m_lastUpdateTime;
	float		m_lastHumanTime;
	float		m_lastSpiderTime;
	float		m_lastWormTime;
	float		m_lastBulletTime;
	float		m_lastAlarmTime;
	int			m_soundChannelAlarm;
	int			m_flagColor;
	int			m_lastMotorState;
};


#endif //_BRAIN_H_
