// displayinfo.h

#ifndef _DISPLAYINFO_H_
#define	_DISPLAYINFO_H_


class CInstanceManager;
class CD3DEngine;
class CEvent;
class CRobotMain;
class CCamera;
class CInterface;
class CObject;
class CParticule;
class CLight;


class CDisplayInfo
{
public:
	CDisplayInfo(CInstanceManager* iMan);
	~CDisplayInfo();

	BOOL		EventProcess(const Event &event);

	void		StartDisplayInfo(char *filename, int index, BOOL bSoluce);
	void		StopDisplayInfo();

	void		SetPosition(int pos);
	int			RetPosition();

protected:
	BOOL		EventFrame(const Event &event);
	void		HyperUpdate();
	void		AdjustDisplayInfo(FPOINT wpos, FPOINT wdim);
	void		ChangeIndexButton(int index);
	void		UpdateIndexButton();
	void		UpdateCopyButton();
	void		ViewDisplayInfo();
	CObject*	SearchToto();
	void		CreateObjectsFile();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*	m_engine;
	CEvent*		m_event;
	CRobotMain*	m_main;
	CCamera*	m_camera;
	CInterface*	m_interface;
	CParticule*	m_particule;
	CLight*		m_light;

	BOOL		m_bInfoMaximized;
	BOOL		m_bInfoMinimized;

	int			m_index;
	CameraType	m_infoCamera;
	FPOINT		m_infoNormalPos;
	FPOINT		m_infoNormalDim;
	FPOINT		m_infoActualPos;
	FPOINT		m_infoActualDim;
	FPOINT		m_infoFinalPos;
	FPOINT		m_infoFinalDim;
	int			m_lightSuppl;
	BOOL		m_bEditLock;
	BOOL		m_bInitPause;
	BOOL		m_bSoluce;
	CObject*	m_toto;
};


#endif //_DISPLAYINFO_H_
