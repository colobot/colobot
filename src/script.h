// script.h

#ifndef _SCRIPT_H_
#define	_SCRIPT_H_


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CDisplayText;
class CEdit;
class CList;
class CObject;
class CTaskManager;
class CBotProgram;
class CRobotMain;
class CTerrain;
class CWater;



class CScript
{
public:
	CScript(CInstanceManager* iMan, CObject* object, CTaskManager** secondaryTask);
	~CScript();

	static void	InitFonctions();

	BOOL		GetScript(CEdit* edit);
	BOOL		RetCompile();

	void		GetTitle(char* buffer);

	void		SetStepMode(BOOL bStep);
	BOOL		Run();
	BOOL		Continue(const Event &event);
	BOOL		Step(const Event &event);
	void		Stop();
	BOOL		IsRunning();
	BOOL		IsContinue();
	BOOL		GetCursor(int &cursor1, int &cursor2);
	void		UpdateList(CList* list);
	void		ColorizeScript(CEdit* edit);

	int			RetError();
	void		GetError(char* buffer);

	BOOL		ReadScript(char* filename);
	BOOL		WriteScript(char* filename);
	BOOL		ReadStack(FILE *file);
	BOOL		WriteStack(FILE *file);
	BOOL		Compare(CScript* other);

	void		SetFilename(char *filename);
	char*		RetFilename();

	CObject*	SearchBlupi(D3DVECTOR center);

protected:
	BOOL		IsEmpty();
	BOOL		Compile();

public:
	CInstanceManager* m_iMan;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CDisplayText*	m_displayText;
	CBotProgram*	m_botProg;
	CRobotMain*		m_main;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CTaskManager*	m_primaryTask;
	CTaskManager**	m_secondaryTask;
	CObject*		m_object;

	int				m_ipf;			// nb d'instructions / seconde
	int				m_errMode;		// que faire en cas d'erreur
	int				m_len;			// longueur du script (sans le <0>)
	char*			m_script;		// script terminé par <0>
	BOOL			m_bRun;			// programme en cours d'exécution ?
	BOOL			m_bStepMode;	// step by step
	BOOL			m_bContinue;	// fonction externe à continuer
	BOOL			m_bCompile;		// compilation ok ?
	char			m_title[50];	// titre du script
	char			m_filename[50];	// nom du fichier
	int				m_error;		// erreur (0=ok)
	int				m_cursor1;
	int				m_cursor2;
	Event			m_event;
};


#endif //_SCRIPT_H_
