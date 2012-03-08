// studio.h

#ifndef _STUDIO_H_
#define	_STUDIO_H_


class CInstanceManager;
class CD3DEngine;
class CEvent;
class CRobotMain;
class CCamera;
class CSound;
class CInterface;
class CScript;
class CList;
class CEdit;



enum StudioDialog
{
	SD_NULL,
	SD_OPEN,
	SD_SAVE,
	SD_FIND,
	SD_REPLACE,
};



class CStudio
{
public:
	CStudio(CInstanceManager* iMan);
	~CStudio();

	BOOL		EventProcess(const Event &event);

	void		StartEditScript(CScript *script, char* name, int rank);
	BOOL		StopEditScript(BOOL bCancel);

protected:
	BOOL		EventFrame(const Event &event);
	void		SearchToken(CEdit* edit);
	void		ColorizeScript(CEdit* edit);
	void		AdjustEditScript();
	void		SetInfoText(char *text, BOOL bClickable);
	void		ViewEditScript();
	void		UpdateFlux();
	void		UpdateButtons();

	void		StartDialog(StudioDialog type);
	void		StopDialog();
	void		AdjustDialog();
	BOOL		EventDialog(const Event &event);
	void		UpdateChangeList();
	void		UpdateChangeEdit();
	void		UpdateDialogAction();
	void		UpdateDialogPublic();
	void		UpdateDialogList();
	void		SearchDirectory(char *dir, BOOL bCreate);
	BOOL		ReadProgram();
	BOOL		WriteProgram();

protected:
	CInstanceManager* m_iMan;
	CD3DEngine*	m_engine;
	CEvent*		m_event;
	CRobotMain*	m_main;
	CCamera*	m_camera;
	CSound*		m_sound;
	CInterface*	m_interface;

	int			m_rank;
	CScript*	m_script;

	BOOL		m_bEditMaximized;
	BOOL		m_bEditMinimized;

	CameraType	m_editCamera;
	FPOINT		m_editActualPos;
	FPOINT		m_editActualDim;
	FPOINT		m_editFinalPos;
	FPOINT		m_editFinalDim;

	float		m_time;
	float		m_fixInfoTextTime;
	BOOL		m_bRunning;
	BOOL		m_bRealTime;
	BOOL		m_bInitPause;
	char		m_helpFilename[100];

	StudioDialog m_dialog;
};


#endif //_STUDIO_H_
