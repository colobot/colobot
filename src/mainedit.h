// mainedit.h

#ifndef _MAINEDIT_H_
#define	_MAINEDIT_H_



class CInstanceManager;
class CRobotMain;
class CMainDialog;
class CEvent;
class CD3DEngine;
class CInterface;
class CWindow;
class CControl;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CSound;
class CButton;
class CMenu;

enum ObjectType;



#define ENV_CASTEL			0
#define ENV_HOME			1
#define ENV_DOCK			2
#define ENV_MECA			3
#define ENV_DESERT			4
#define ENV_SNOW			5
#define ENV_INCA			6
#define ENV_CRAZY			7
#define ENV_JUNGLE			8
#define ENV_UNREAL			9
#define ENV_FUTURA			10
#define ENV_CAVE			11
#define ENV_CINEMA			12
#define ENV_CATA			13
#define ENV_KID				14
#define ENV_BETON			15
#define ENV_LABO			16
#define ENV_GLU				17
#define ENV_WOOD			18
#define ENV_ZONE			19

#define LEVEL_OBJECT		0
#define LEVEL_GROUND		1
#define LEVEL_WATER			2

#define TOOL_DELETE			0
#define TOOL_BOX			1
#define TOOL_BARRIER		2
#define TOOL_PLANT			3
#define TOOL_BIG			4
#define TOOL_BLUPI			5
#define TOOL_GROUND			1
#define TOOL_DECORDOWN		2
#define TOOL_SIGNMARK		5


typedef struct
{
	int		channel;
	float	height;
}
GridItem;

typedef struct
{
	ObjectType	type;
	int			ground;
	int			icon;
	char		texture[50];
	FPOINT		uv1, uv2;
}
ToolObject;

typedef struct
{
	int			total;
	int			select;
	ToolObject	tool[20];
}
ToolMenu;




class CMainEdit
{
public:
	CMainEdit(CInstanceManager* iMan);
	~CMainEdit();

	BOOL		EventProcess(const Event &event);

	void		Open();
	void		Close();

	void		SetEnvironment(int type);
	int			RetEnvironment();
	float		RetGridHeight();

	void		MouseDownFlush();

	int			CheckPuzzle();

protected:
	void		InitEnvironment();
	BOOL		EventFrame(float rTime);
	void		EventMouseDetect(FPOINT mouse, D3DVECTOR &pos, ObjectType &type);
	void		EventMouseDown(FPOINT mouse, BOOL bRight);
	void		EventMouseMove(FPOINT mouse);
	void		EventMouseUp(FPOINT mouse, BOOL bRight);
	void		DeleteObject(CObject *pObj, BOOL bAction=TRUE, BOOL bFinish=TRUE);
	void		InitAngleZoom(ObjectType &type, float &angle, float &zoom);
	void		CreateObject(D3DVECTOR pos, ObjectType type);
	int			RetTotalBlupi();
	void		CreateSignMark(D3DVECTOR pos, int sign);
	void		FlushVisitList();
	BOOL		AddVisitList(CObject *pObj);
	BOOL		IsVisitList(CObject *pObj);
	BOOL		SearchBrother(CObject *pObj, D3DVECTOR &dir);
	BOOL		SearchLimit(CObject *pObj, D3DVECTOR &dir, D3DVECTOR &src, D3DVECTOR &dst);
	void		AdaptObject(CObject *pObj);
	BOOL		AdaptLimit(CObject *pObj, ObjectType family);
	void		AdaptGround();
	void		AdaptBarrier();
	void		AdaptPlant();
	void		ConfigMenu(CMenu *pm, ToolMenu *menu);
	void		UpdateMenu(CMenu *pm, ToolMenu *menu);
	void		UpdateButtons();
	void		RebuildDecor(BOOL bTerrain, BOOL bWater, BOOL bGrid);
	BOOL		IsDecorDown(D3DVECTOR pos);
	BOOL		GridCheckPos(D3DVECTOR pos);
	void		GridCreate();
	void		GridFlush();
	void		GridSelectPos(D3DVECTOR center, ObjectType type);
	void		GridSelect(D3DVECTOR p1, D3DVECTOR p2, ObjectType type);
	BOOL		GridDetect(FPOINT mouse, D3DVECTOR &pos);
	CObject*	SearchLift(D3DVECTOR center);
	CObject*	SearchPlant5(D3DVECTOR center);
	CObject*	SearchObject(D3DVECTOR center, int level, ObjectType newLoc=(ObjectType)0);
	CObject*	DetectObject(FPOINT mouse);
	BOOL		DetectObject(FPOINT mouse, CObject *&pObj, D3DVECTOR &pos);
	int			CheckCreate(D3DVECTOR pos, ObjectType type);

protected:
	CInstanceManager* m_iMan;
	CRobotMain*		m_main;
	CMainDialog*	m_dialog;
	CEvent*			m_event;
	CD3DEngine*		m_engine;
	CInterface*		m_interface;
	CParticule*		m_particule;
	CTerrain*		m_terrain;
	CWater*			m_water;
	CCamera*		m_camera;
	CSound*			m_sound;

	float			m_time;
	int				m_nbTiles;
	int				m_nbTiles2;
	float			m_dimTile;
	int				m_model;

	int				m_environment;
	int				m_level;
	int				m_tool[3];

	CButton*		m_button[10];
	CMenu*			m_menu[10];
	CLabel*			m_label;

	ToolMenu		m_menuBox;
	ToolMenu		m_menuBarrier;
	ToolMenu		m_menuPlant;
	ToolMenu		m_menuBig;
	ToolMenu		m_menuBlupi;
	ToolMenu		m_menuGround;
	ToolMenu		m_menuDecorDown;
	ToolMenu		m_menuSignMark;

	float*			m_lastAngles;
	float			m_lastAngleSignMark;

	int				m_gridTotal;
	GridItem*		m_gridTable;
	int				m_gridSelectTotal;
	int				m_gridSelectParti[20];
	ParticuleType	m_gridSelectType[20];
	D3DVECTOR		m_gridSelectPos[20];
	D3DVECTOR		m_gridSelectP1;
	D3DVECTOR		m_gridSelectP2;
	BOOL			m_bGridSelectDown;
	BOOL			m_bGridSelectOK;
	BOOL			m_bClickRight;
	float			m_gridHeight;

	int				m_visitListTotal;
	CObject*		m_visitListBuffer[200];
};


#endif //_MAINEDIT_H_
