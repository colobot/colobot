// mainedit.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "profile.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "motion.h"
#include "interface.h"
#include "button.h"
#include "menu.h"
#include "list.h"
#include "label.h"
#include "window.h"
#include "text.h"
#include "array.h"
#include "camera.h"
#include "pyro.h"
#include "sound.h"
#include "cmdtoken.h"
#include "robotmain.h"
#include "maindialog.h"
#include "mainedit.h"





// Constructeur de l'application robot.

CMainEdit::CMainEdit(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_EDIT, this);

	m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_dialog    = (CMainDialog*)m_iMan->SearchInstance(CLASS_DIALOG);
	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_terrain   = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_environment = ENV_CASTEL;
	m_level = LEVEL_GROUND;
	m_tool[0] = TOOL_DELETE;
	m_tool[1] = TOOL_DELETE;
	m_tool[2] = TOOL_DELETE;
	m_time = 0.0f;
	m_nbTiles = 0;
	m_nbTiles2 = 0;
	m_dimTile = 0.0f;
	m_gridTable = 0;
	m_gridSelectTotal = 0;
	m_gridSelectP1 = D3DVECTOR(NAN, NAN, NAN);
	m_bGridSelectDown = FALSE;
	m_gridHeight = 0.2f;

	m_lastAngles = (float*)malloc(sizeof(float)*OBJECT_MAX);
	ZeroMemory(m_lastAngles, sizeof(float)*OBJECT_MAX);
	m_lastAngleSignMark = 0.0f;
}

// Destructeur de l'application robot.

CMainEdit::~CMainEdit()
{
	GridFlush();
	free(m_lastAngles);
}


// Ouvre l'éditeur.

void CMainEdit::Open()
{
	FPOINT		pos, dim;
	char		text[100];

	m_nbTiles = m_terrain->RetNbTiles();
	m_nbTiles2 = m_nbTiles/2;
	m_dimTile = m_terrain->RetDimTile();
	m_model = m_terrain->RetModel();

	InitEnvironment();

	pos.x =   0.0f/640.0f;
	pos.y =   0.0f/480.0f;
	dim.x =  56.0f/640.0f;
	dim.y = 480.0f/480.0f;
	m_interface->CreateGroup(pos, dim, 31, EVENT_INTERFACE_KGROUP);

	pos.x =  10.0f/640.0f;
	pos.y = 434.0f/480.0f;
	dim.x =  36.0f/640.0f;
	dim.y =  36.0f/480.0f;
	m_button[0] = m_interface->CreateButton(pos, dim, 66, EVENT_EDIT_OBJECT);
	pos.y -= 36.0f/480.0f;
	m_button[1] = m_interface->CreateButton(pos, dim, 65, EVENT_EDIT_GROUND);
	pos.y -= 36.0f/480.0f;
	m_button[2] = m_interface->CreateButton(pos, dim, 64, EVENT_EDIT_WATER);

	pos.y -= 46.0f/480.0f;
	m_button[3] = m_interface->CreateButton(pos, dim, 11, EVENT_EDIT_CLEAR);
	pos.y -= 36.0f/480.0f;
	m_menu[0] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU1);
	pos.y -= 36.0f/480.0f;
	m_menu[1] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU2);
	pos.y -= 36.0f/480.0f;
	m_menu[2] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU3);
	pos.y -= 36.0f/480.0f;
	m_menu[3] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU4);
	pos.y -= 36.0f/480.0f;
	m_menu[4] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU5);
	pos.y -= 36.0f/480.0f;
	m_menu[5] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU6);
	pos.y -= 36.0f/480.0f;
	m_menu[6] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU7);
	pos.y -= 36.0f/480.0f;
	m_menu[7] = m_interface->CreateMenu(pos, dim, 48, EVENT_EDIT_MENU8);

	pos.y = 10.0f/480.0f;
	m_button[9] = m_interface->CreateButton(pos, dim, 12, EVENT_EDIT_INFO);

	pos.x =   0.0f/640.0f;
	pos.y = 160.0f/480.0f;
	dim.x =  56.0f/640.0f;
	dim.y =  36.0f/480.0f;
	GetResource(RES_EVENT, EVENT_LABEL_SIGN, text);
	m_label = m_interface->CreateLabel(pos, dim, 0, EVENT_LABEL_SIGN, text);
	m_label->SetFontSize(8.0f);

	UpdateButtons();
	GridCreate();
}

// Ferme l'éditeur.

void CMainEdit::Close()
{
	m_interface->DeleteControl(EVENT_INTERFACE_KGROUP);
	m_interface->DeleteControl(EVENT_EDIT_OBJECT);
	m_interface->DeleteControl(EVENT_EDIT_GROUND);
	m_interface->DeleteControl(EVENT_EDIT_WATER);
	m_interface->DeleteControl(EVENT_EDIT_CLEAR);
	m_interface->DeleteControl(EVENT_EDIT_MENU1);
	m_interface->DeleteControl(EVENT_EDIT_MENU2);
	m_interface->DeleteControl(EVENT_EDIT_MENU3);
	m_interface->DeleteControl(EVENT_EDIT_MENU4);
	m_interface->DeleteControl(EVENT_EDIT_MENU5);
	m_interface->DeleteControl(EVENT_EDIT_MENU6);
	m_interface->DeleteControl(EVENT_EDIT_MENU7);
	m_interface->DeleteControl(EVENT_EDIT_MENU8);
	m_interface->DeleteControl(EVENT_EDIT_INFO);

	GridFlush();
}


// Choix de l'environnement.

void CMainEdit::SetEnvironment(int type)
{
	m_environment = type;

	if ( m_environment == ENV_CAVE )
	{
		m_gridHeight = 1.5f;
	}
	else if ( m_environment == ENV_INCA )
	{
		m_gridHeight = 1.0f;
	}
	else if ( m_environment == ENV_KID )
	{
		m_gridHeight = 0.8f;
	}
	else if ( m_environment == ENV_WOOD )
	{
		m_gridHeight = 0.8f;
	}
	else
	{
		m_gridHeight = 0.2f;
	}
}

int CMainEdit::RetEnvironment()
{
	return m_environment;
}

float CMainEdit::RetGridHeight()
{
	return m_gridHeight;
}


// Vide complètement un menu.

void FlushToolMenu(ToolMenu *menu)
{
	int		i;

	menu->total = 0;
	menu->select = 0;

	for ( i=0 ; i<20 ; i++ )
	{
		menu->tool[i].type = OBJECT_NULL;
		menu->tool[i].ground = 0;
		menu->tool[i].icon = -1;
		menu->tool[i].texture[0] = 0;
		menu->tool[i].uv1 = FPOINT(0.0f, 0.0f);
		menu->tool[i].uv2 = FPOINT(0.0f, 0.0f);
	}
}

// Ajoute une case à un menu.

void AddToolMenuIcon(ToolMenu *menu, ObjectType type, int icon)
{
	int		i;

	i = menu->total;
	menu->tool[i].type = type;
	menu->tool[i].icon = icon;

	menu->total ++;
}

// Ajoute une case à un menu.

void AddToolMenuIcon(ToolMenu *menu, int ground,
					 char *texture, float uvx, float uvy, float dim)
{
	int		i;

	i = menu->total;
	menu->tool[i].ground = ground;
	strcpy(menu->tool[i].texture, texture);
	menu->tool[i].uv1.x = uvx/256.0f;
	menu->tool[i].uv1.y = uvy/256.0f;
	menu->tool[i].uv2.x = menu->tool[i].uv1.x+dim/256.0f;
	menu->tool[i].uv2.y = menu->tool[i].uv1.y+dim/256.0f;

	menu->total ++;
}

// Initialise l'environnement global.

void CMainEdit::InitEnvironment()
{
	FlushToolMenu(&m_menuBox);
	FlushToolMenu(&m_menuBarrier);
	FlushToolMenu(&m_menuPlant);
	FlushToolMenu(&m_menuBig);
	FlushToolMenu(&m_menuBlupi);
	FlushToolMenu(&m_menuGround);
	FlushToolMenu(&m_menuDecorDown);
	FlushToolMenu(&m_menuSignMark);

	if ( m_environment == ENV_CASTEL )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER10, 74);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER11, 75);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER12, 76);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER46, 77);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,      78);

//?		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0,  88);
//?		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT5,  89);
//?		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT10, 90);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
//?		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,   92);
//?		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE4,   93);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "castel01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "castel01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "castel01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "castel01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "castel01.tga",  64.0f, 64.0f, 64.0f);
	
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND2, 73);
	}

	if ( m_environment == ENV_HOME )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER7,   99);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER8,  100);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER9,  101);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER4,   96);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER5,   97);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER6,   98);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER69,  72);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0, 88);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,  92);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 3, "vegetal02.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 2, "vegetal02.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 4, "vegetal02.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 0, "vegetal02.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "vegetal02.tga",  64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_DOCK )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER3, 127);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,      78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_DOCK, 135);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "metal01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "metal01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "metal01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "metal01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "metal01.tga",  64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_MECA )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER40, 128);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER41, 129);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER42, 130);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER43, 131);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_CATAPULT, 132);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "metal05.tga",  0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 5, "metal05.tga", 64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_DESERT )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER23, 104);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER24, 105);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER25, 106);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER26, 107);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER28, 109);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER27, 108);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE4,   93);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "desert01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "desert01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "desert01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "desert01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "desert01.tga",  64.0f, 64.0f, 64.0f);
	
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_BARRIER29, 102);
	}

	if ( m_environment == ENV_SNOW )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER30, 112);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER31, 113);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER35, 117);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER36, 118);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER33, 115);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER32, 114);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER34, 116);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER37, 119);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER38, 110);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "snow01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "snow01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "snow01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "snow01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "snow01.tga",  64.0f, 64.0f, 64.0f);

		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND3,   103);
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_BARRIER39, 111);
	}

	if ( m_environment == ENV_INCA )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX8, 95);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER13, 120);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER14, 121);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER15, 122);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER16, 123);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER17, 124);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE4,   93);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_TRAX, 138);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "inca01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "inca01.tga", 128.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 3, "inca01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "inca01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "inca01.tga",  64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_CRAZY )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_KEY1, 86);
		AddToolMenuIcon(&m_menuBox, OBJECT_KEY2, 87);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER44, 136);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER45, 137);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER47, 140);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER68, 149);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0, 88);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,  92);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_DOOR1, 133);
		AddToolMenuIcon(&m_menuBig, OBJECT_DOOR2, 134);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_CRAZY, 70);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "cathe01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "cathe01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "cathe01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "cathe01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "cathe01.tga",  64.0f, 64.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 6, "cathe01.tga", 128.0f, 64.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 7, "cathe01.tga", 192.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_JUNGLE )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE, 78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT5,  89);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "orga01.tga",   0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 2, "orga01.tga", 128.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 3, "orga01.tga", 192.0f,  0.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 4, "orga01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "orga01.tga",  64.0f, 64.0f, 64.0f);

		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND0, 125);
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND1, 126);
	}

	if ( m_environment == ENV_UNREAL )
	{
#if 1
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER0,  67);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER1,  68);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER2,  69);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_FIOLE,    141);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,      78);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "metal03.tga",   0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 2, "metal03.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "metal03.tga", 192.0f,  0.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 4, "metal03.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "metal03.tga",  64.0f, 64.0f, 32.0f);

		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND0, 125);
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND1, 126);
#else
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER0,  67);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER1,  68);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER2,  69);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_FIOLE,    141);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,      78);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
	
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0,  88);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT5,  89);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT10, 90);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);

		AddToolMenuIcon(&m_menuBig, OBJECT_PERFO, 143);
		AddToolMenuIcon(&m_menuBig, OBJECT_CATAPULT, 132);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "metal03.tga",   0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 2, "metal03.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "metal03.tga", 192.0f,  0.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 4, "metal03.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "metal03.tga",  64.0f, 64.0f, 32.0f);

		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND0, 125);
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND1, 126);
#endif
	}

	if ( m_environment == ENV_FUTURA )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX7, 94);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER61, 163);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER60, 162);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER59, 161);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER58, 160);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER62, 164);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER64, 166);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER63, 165);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER65, 167);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_GLASS1,    142);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "futur01.tga", 0.0f, 0.0f, 64.0f);

		AddToolMenuIcon(&m_menuDecorDown, OBJECT_BARRIER67, 159);
	}

	if ( m_environment == ENV_CAVE )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX7, 94);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER52, 154);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER53, 155);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER50, 152);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER51, 153);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER54, 156);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER57, 157);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER66, 158);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

//?		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0,  88);
//?		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT10, 90);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_PERFO, 143);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "roc01.tga", 0.0f, 0.0f, 64.0f);
	}

	if ( m_environment == ENV_CINEMA )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX7, 94);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER18, 144);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0,  88);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,  92);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "cinema01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "cinema01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "cinema01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "cinema01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "cinema01.tga",  64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_CATA )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER40, 128);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER41, 129);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER42, 130);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER43, 131);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_CATAPULT, 132);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "metal05.tga",  0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 5, "metal05.tga", 64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_KID )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX9, 150);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER78, 176);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER79, 177);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER81, 179);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER80, 178);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,   92);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "wood01.tga",   0.0f,   0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "wood01.tga",   0.0f,  64.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "wood01.tga",  64.0f,  64.0f, 32.0f);
	}

	if ( m_environment == ENV_BETON )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX7, 94);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX10, 151);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER86, 184);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER87, 185);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER88, 186);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER89, 187);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER90, 188);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER91, 189);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "beton01.tga", 16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "beton01.tga", 192.0f, 0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "beton01.tga", 16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "beton01.tga", 64.0f, 64.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "beton01.tga", 128.0f, 0.0f, 64.0f);
	}

	if ( m_environment == ENV_LABO )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX7, 94);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER72, 170);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER73, 171);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER70, 168);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER74, 172);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER75, 173);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER76, 174);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER77, 175);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER71, 169);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_GLASS2,    142);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "labo01.tga", 0.0f, 0.0f, 64.0f);
	}

	if ( m_environment == ENV_GLU )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1, 80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2, 81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3, 82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4, 83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5, 84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6, 85);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER0,  67);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER1,  68);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER2,  69);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_FIOLE,    141);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_GLU,      180);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,    139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,      78);
	
		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "metal03.tga",   0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 2, "metal03.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "metal03.tga", 192.0f,  0.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 4, "metal03.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "metal03.tga",  64.0f, 64.0f, 32.0f);

		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND0, 125);
		AddToolMenuIcon(&m_menuDecorDown, OBJECT_GROUND1, 126);
	}

	if ( m_environment == ENV_WOOD )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1,   80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2,   81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3,   82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4,   83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5,   84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6,   85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX9,  150);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX11, 181);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_FIOLE, 141);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_GLU,   180);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X, 139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,   78);
	
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT0, 88);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,  92);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "wood02.tga",   0.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 2, "wood02.tga", 128.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 3, "wood02.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "wood02.tga",   0.0f, 64.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 5, "wood02.tga",  64.0f, 64.0f, 64.0f);
	}

	if ( m_environment == ENV_ZONE )
	{
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX1,   80);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX2,   81);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX3,   82);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX4,   83);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX5,   84);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX6,   85);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX9,  150);
		AddToolMenuIcon(&m_menuBox, OBJECT_BOX11, 181);

		AddToolMenuIcon(&m_menuBarrier, OBJECT_BARRIER92, 183);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_FIOLE,     141);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_GLU,       180);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MAX1X,     139);
		AddToolMenuIcon(&m_menuBarrier, OBJECT_MINE,       78);

		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT15, 91);
		AddToolMenuIcon(&m_menuPlant, OBJECT_PLANT5,  89);
		AddToolMenuIcon(&m_menuPlant, OBJECT_TREE0,   92);
	
		AddToolMenuIcon(&m_menuBig, OBJECT_GUN, 182);

		AddToolMenuIcon(&m_menuBlupi, OBJECT_BLUPI, 71);
		AddToolMenuIcon(&m_menuBlupi, OBJECT_GOAL,  79);
	
		AddToolMenuIcon(&m_menuGround, 0, "grass01.tga",  16.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 2, "grass01.tga", 144.0f, 16.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 3, "grass01.tga", 192.0f,  0.0f, 64.0f);
		AddToolMenuIcon(&m_menuGround, 4, "grass01.tga",  16.0f, 80.0f, 32.0f);
		AddToolMenuIcon(&m_menuGround, 5, "grass01.tga",  64.0f, 64.0f, 64.0f);
	}

	AddToolMenuIcon(&m_menuSignMark,  0, "effect02.tga",  64.0f, 128.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  1, "effect02.tga",  64.0f, 160.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  2, "effect02.tga",  96.0f, 128.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  3, "effect02.tga",  96.0f, 160.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  4, "effect02.tga", 128.0f, 128.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  5, "effect02.tga", 128.0f, 160.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  6, "effect02.tga", 160.0f, 128.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  7, "effect02.tga", 160.0f, 160.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  8, "effect02.tga", 192.0f, 128.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark,  9, "effect02.tga", 192.0f, 160.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark, 10, "effect02.tga", 224.0f, 128.0f, 32.0f);
	AddToolMenuIcon(&m_menuSignMark, 11, "effect02.tga", 224.0f, 160.0f, 32.0f);
}


// Traite un événement.
// Retourne FALSE si l'événement a été traîté complètement.

BOOL CMainEdit::EventProcess(const Event &event)
{
	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event.rTime);
	}

	switch( event.event )
	{
		case EVENT_LBUTTONDOWN:
			EventMouseDown(event.pos, FALSE);
			break;
		case EVENT_RBUTTONDOWN:
			EventMouseDown(event.pos, TRUE);
			break;

		case EVENT_MOUSEMOVE:
			EventMouseMove(event.pos);
			break;

		case EVENT_LBUTTONUP:
			EventMouseUp(event.pos, FALSE);
			break;
		case EVENT_RBUTTONUP:
			EventMouseUp(event.pos, TRUE);
			break;

		case EVENT_EDIT_WATER:
			m_level = LEVEL_WATER;
			UpdateButtons();
			GridCreate();
			break;
		case EVENT_EDIT_GROUND:
			m_level = LEVEL_GROUND;
			UpdateButtons();
			GridCreate();
			break;
		case EVENT_EDIT_OBJECT:
			m_level = LEVEL_OBJECT;
			UpdateButtons();
			GridCreate();
			break;

		case EVENT_EDIT_CLEAR:
			m_tool[m_level] = TOOL_DELETE;
			UpdateButtons();
			GridCreate();
			break;

		case EVENT_EDIT_MENU1:
			m_tool[m_level] = 1;
			if ( m_level == LEVEL_OBJECT )
			{
				UpdateMenu(m_menu[0], &m_menuBox);
				UpdateButtons();
				GridCreate();
			}
			if ( m_level == LEVEL_GROUND )
			{
				UpdateMenu(m_menu[0], &m_menuGround);
				UpdateButtons();
				GridCreate();
			}
			break;

		case EVENT_EDIT_MENU2:
			m_tool[m_level] = 2;
			if ( m_level == LEVEL_OBJECT )
			{
				UpdateMenu(m_menu[1], &m_menuBarrier);
				UpdateButtons();
				GridCreate();
			}
			if ( m_level == LEVEL_GROUND )
			{
				UpdateMenu(m_menu[1], &m_menuDecorDown);
				UpdateButtons();
				GridCreate();
			}
			break;

		case EVENT_EDIT_MENU3:
			m_tool[m_level] = 3;
			if ( m_level == LEVEL_OBJECT )
			{
				UpdateMenu(m_menu[2], &m_menuPlant);
				UpdateButtons();
				GridCreate();
			}
			break;

		case EVENT_EDIT_MENU4:
			m_tool[m_level] = 4;
			if ( m_level == LEVEL_OBJECT )
			{
				UpdateMenu(m_menu[3], &m_menuBig);
				UpdateButtons();
				GridCreate();
			}
			break;

		case EVENT_EDIT_MENU5:
			m_tool[m_level] = 5;
			if ( m_level == LEVEL_OBJECT )
			{
				UpdateMenu(m_menu[4], &m_menuBlupi);
				UpdateButtons();
				GridCreate();
			}
			if ( m_level == LEVEL_GROUND )
			{
				UpdateMenu(m_menu[4], &m_menuSignMark);
				UpdateButtons();
				GridCreate();
			}
			break;

		case EVENT_EDIT_MENU6:
			m_tool[m_level] = 6;
			break;

		case EVENT_EDIT_MENU7:
			m_tool[m_level] = 7;
			break;

		case EVENT_EDIT_MENU8:
			m_tool[m_level] = 8;
			break;
	}

	return TRUE;
}

// Temps écoulé.

BOOL CMainEdit::EventFrame(float rTime)
{
	m_time += rTime;
	return TRUE;
}


// Détecte la position visée par la souris.

void CMainEdit::EventMouseDetect(FPOINT mouse,
								 D3DVECTOR &pos, ObjectType &type)
{
	CObject*	pObj;
	D3DVECTOR	ppos;

	type = OBJECT_NULL;

	if ( m_level == LEVEL_OBJECT )  // objets ?
	{
		DetectObject(mouse, pObj, pos);
		if ( pObj == 0 )
		{
			if ( m_tool[m_level] == TOOL_BIG )
			{
				type = m_menuBig.tool[m_menuBig.select].type;
			}
		}
	}

	if ( m_level == LEVEL_GROUND )  // sol ?
	{
		pos = D3DVECTOR(NAN, NAN, NAN);
		if ( GridDetect(mouse, ppos) )
		{
			if ( GridCheckPos(ppos) )
			{
				pos = ppos;
			}
		}
	}

	if ( m_level == LEVEL_WATER )  // eau ?
	{
		pos = D3DVECTOR(NAN, NAN, NAN);
		if ( GridDetect(mouse, ppos) )
		{
			pos = ppos;
		}
	}
}

// Annule l'enfoncement du bouton de la souris.

void CMainEdit::MouseDownFlush()
{
	m_bGridSelectDown = FALSE;
}

// Bouton souris enfoncé.

void CMainEdit::EventMouseDown(FPOINT mouse, BOOL bRight)
{
	D3DVECTOR	pos;
	ObjectType	type;

	EventMouseDetect(mouse, pos, type);

	m_bClickRight = bRight;
	m_bGridSelectDown = TRUE;
	m_gridSelectP1 = pos;
	m_gridSelectP2 = pos;
	GridSelect(m_gridSelectP1, m_gridSelectP2, type);
}

// Souris bougée.

void CMainEdit::EventMouseMove(FPOINT mouse)
{
	D3DVECTOR	pos;
	ObjectType	type;
	BOOL		bRect;

	EventMouseDetect(mouse, pos, type);

	bRect = FALSE;
	if ( m_level == LEVEL_GROUND )
	{
		if ( m_tool[m_level] != TOOL_DECORDOWN &&
			 m_tool[m_level] != TOOL_SIGNMARK  )
		{
			bRect = TRUE;
		}
	}
	if ( m_level == LEVEL_WATER )
	{
		bRect = TRUE;
	}
	if ( m_bClickRight )  bRect = FALSE;

	if ( m_bGridSelectDown && bRect )
	{
		if ( pos.x != m_gridSelectP2.x ||
			 pos.z != m_gridSelectP2.z )
		{
			m_gridSelectP2 = pos;
			GridSelect(m_gridSelectP1, m_gridSelectP2, type);
		}
	}
	else
	{
		if ( pos.x != m_gridSelectP1.x ||
			 pos.z != m_gridSelectP1.z )
		{
			m_gridSelectP1 = pos;
			m_gridSelectP2 = pos;
			GridSelect(m_gridSelectP1, m_gridSelectP2, type);
		}
	}
}

// Bouton souris relâché.

void CMainEdit::EventMouseUp(FPOINT mouse, BOOL bRight)
{
	CObject*	pObj;
	ObjectType	type;
	TerrainRes	res;
	D3DVECTOR	pos, p1, p2;
	int			nb, nbDelete, nbRes, sign;

	if ( !m_bGridSelectDown )  return;
	m_bGridSelectDown = FALSE;

	if ( m_level == LEVEL_OBJECT )  // objets ?
	{
		if ( m_tool[m_level] == TOOL_DELETE || bRight )  // détruit ?
		{
			DetectObject(mouse, pObj, pos);
			if ( pObj == 0 )
			{
				m_sound->Play(SOUND_TUTUTU);
			}
			else
			{
				DeleteObject(pObj);
			}
			m_gridSelectP1 = D3DVECTOR(NAN,NAN,NAN);
			GridSelect(m_gridSelectP1, m_gridSelectP2, OBJECT_NULL);
			return;
		}

		if ( !DetectObject(mouse, pObj, pos) )
		{
			m_sound->Play(SOUND_TUTUTU);
			return;
		}
		pos.y = 0.0f;

		if ( m_tool[m_level] == TOOL_BOX )
		{
			type = m_menuBox.tool[m_menuBox.select].type;
			CreateObject(pos, type);
		}

		if ( m_tool[m_level] == TOOL_BARRIER )
		{
			type = m_menuBarrier.tool[m_menuBarrier.select].type;
			CreateObject(pos, type);
		}

		if ( m_tool[m_level] == TOOL_PLANT )
		{
			type = m_menuPlant.tool[m_menuPlant.select].type;
			CreateObject(pos, type);
		}

		if ( m_tool[m_level] == TOOL_BIG )
		{
			if ( m_bGridSelectOK )
			{
				type = m_menuBig.tool[m_menuBig.select].type;
				CreateObject(pos, type);
			}
			else
			{
				m_sound->Play(SOUND_TUTUTU);
			}
		}

		if ( m_tool[m_level] == TOOL_BLUPI )
		{
			type = m_menuBlupi.tool[m_menuBlupi.select].type;
			CreateObject(pos, type);
		}
	}

	if ( m_level == LEVEL_GROUND  &&  // sol ?
		 m_gridSelectP1.x != NAN  &&
		 m_gridSelectP2.x != NAN  )
	{
		if ( m_tool[m_level] == TOOL_DELETE || bRight )  // détruit ?
		{
			p1.x = Min(m_gridSelectP1.x, m_gridSelectP2.x);
			p1.z = Min(m_gridSelectP1.z, m_gridSelectP2.z);
			p2.x = Max(m_gridSelectP1.x, m_gridSelectP2.x);
			p2.z = Max(m_gridSelectP1.z, m_gridSelectP2.z);

			nbDelete = 0;
			nbRes = 0;
			for ( pos.z=p1.z ; pos.z<=p2.z ; pos.z+=8.0f )
			{
				for ( pos.x=p1.x ; pos.x<=p2.x ; pos.x+=8.0f )
				{
					if ( !m_terrain->SignMarkDelete(pos) )
					{
						pObj = SearchObject(pos, m_level);
						if ( pObj != 0 )
						{
							DeleteObject(pObj, TRUE, FALSE);
							nbDelete ++;
						}
						else if ( m_terrain->IsSolid(pos) )
						{
							if ( SearchObject(pos, LEVEL_OBJECT) == 0 )
							{
								m_terrain->SetResource(pos, TR_HOLE, TRUE);
								nbRes ++;
							}
						}
					}
				}
			}

			if ( nbDelete > 0 )
			{
				DeleteObject(0, FALSE, TRUE);
			}
			if ( nbRes > 0 )
			{
				RebuildDecor(TRUE, FALSE, TRUE);
			}
		}
		else if ( m_tool[m_level] == TOOL_GROUND )
		{
			p1.x = Min(m_gridSelectP1.x, m_gridSelectP2.x);
			p1.z = Min(m_gridSelectP1.z, m_gridSelectP2.z);
			p2.x = Max(m_gridSelectP1.x, m_gridSelectP2.x);
			p2.z = Max(m_gridSelectP1.z, m_gridSelectP2.z);

			for ( pos.z=p1.z ; pos.z<=p2.z ; pos.z+=8.0f )
			{
				for ( pos.x=p1.x ; pos.x<=p2.x ; pos.x+=8.0f )
				{
					res = m_terrain->RetResource(pos);
					if ( res == TR_HOLE   ||
						 res == TR_SOLID0 ||
						 res == TR_SOLID2 ||
						 res == TR_SOLID3 ||
						 res == TR_SOLID4 ||
						 res == TR_SOLID5 )
					{
						res = (TerrainRes)(TR_SOLID0+m_menuGround.tool[m_menuGround.select].ground);
						m_terrain->SetResource(pos, res, TRUE);
					}
				}
			}
			RebuildDecor(TRUE, FALSE, TRUE);
		}
		else if ( m_tool[m_level] == TOOL_DECORDOWN )
		{
			type = m_menuDecorDown.tool[m_menuDecorDown.select].type;
			CreateObject(m_gridSelectP1, type);
		}
		else if ( m_tool[m_level] == TOOL_SIGNMARK )
		{
			sign = m_menuSignMark.tool[m_menuSignMark.select].ground;
			pos = m_gridSelectP1;
			pos.y = RetGridHeight();
			CreateSignMark(pos, sign);
		}
	}

	if ( m_level == LEVEL_WATER   &&  // eau ?
		 m_gridSelectP1.x != NAN  &&
		 m_gridSelectP2.x != NAN  )
	{
		p1.x = Min(m_gridSelectP1.x, m_gridSelectP2.x);
		p1.z = Min(m_gridSelectP1.z, m_gridSelectP2.z);
		p2.x = Max(m_gridSelectP1.x, m_gridSelectP2.x);
		p2.z = Max(m_gridSelectP1.z, m_gridSelectP2.z);

		nb = 0;
		for ( pos.z=p1.z ; pos.z<=p2.z ; pos.z+=8.0f )
		{
			for ( pos.x=p1.x ; pos.x<=p2.x ; pos.x+=8.0f )
			{
				if ( m_terrain->RetResource(pos) == TR_SPACE )  nb ++;
			}
		}

		for ( pos.z=p1.z ; pos.z<=p2.z ; pos.z+=8.0f )
		{
			for ( pos.x=p1.x ; pos.x<=p2.x ; pos.x+=8.0f )
			{
				if ( nb == 0 )
				{
					if ( m_terrain->RetResource(pos) == TR_HOLE )
					{
						m_terrain->SetResource(pos, TR_SPACE, TRUE);
					}
				}
				else
				{
					if ( m_terrain->RetResource(pos) == TR_SPACE )
					{
						m_terrain->SetResource(pos, TR_HOLE, TRUE);
					}
				}
			}
		}

		RebuildDecor(TRUE, TRUE, TRUE);
	}

	m_gridSelectP1 = D3DVECTOR(NAN,NAN,NAN);
	GridSelect(m_gridSelectP1, m_gridSelectP2, OBJECT_NULL);
}


// Détruit un objet.

void CMainEdit::DeleteObject(CObject *pObj, BOOL bAction, BOOL bFinish)
{
	D3DVECTOR	pos;
	ObjectType	type;
	BOOL		bTerrain;

	if ( bAction )
	{
		bTerrain = FALSE;
		type = pObj->RetType();
		pos = pObj->RetPosition(0);

		if ( type == OBJECT_BLUPI ||
			 type == OBJECT_GOAL  ||
			 type == OBJECT_MAX1X )
		{
			m_terrain->SetResource(pos, TR_SOLID0, TRUE);
			bTerrain = TRUE;
		}

		if ( type >= OBJECT_GROUND0  &&
			 type <= OBJECT_GROUND19 )
		{
			bTerrain = TRUE;
		}

		pObj->DeleteObject();
		delete pObj;

		if ( type == OBJECT_BLUPI )
		{
			pObj = SearchLift(pos);  // cherche OBJECT_LIFT
			if ( pObj != 0 )
			{
				pObj->DeleteObject();
				delete pObj;
			}
		}
	}

	if ( bFinish )
	{
		AdaptGround();
		AdaptBarrier();
		RebuildDecor(bTerrain, FALSE, TRUE);
	}
}


// Retourne la famille d'un objet.

ObjectType RetFamily(ObjectType type)
{
	if ( type >= OBJECT_TREE0 &&
		 type <= OBJECT_TREE3 )  return OBJECT_TREE0;

	if ( type >= OBJECT_TREE4 &&
		 type <= OBJECT_TREE7 )  return OBJECT_TREE4;

	if ( type >= OBJECT_PLANT0 &&
		 type <= OBJECT_PLANT4 )  return OBJECT_PLANT0;

	if ( type >= OBJECT_PLANT5 &&
		 type <= OBJECT_PLANT9 )  return OBJECT_PLANT5;

	if ( type >= OBJECT_PLANT10 &&
		 type <= OBJECT_PLANT14 )  return OBJECT_PLANT10;

	if ( type >= OBJECT_PLANT15 &&
		 type <= OBJECT_PLANT19 )  return OBJECT_PLANT15;

	if ( type >= OBJECT_BARRIER18 &&
		 type <= OBJECT_BARRIER22 )  return OBJECT_BARRIER18;

	if ( type >= OBJECT_BARRIER54 &&
		 type <= OBJECT_BARRIER55 )  return OBJECT_BARRIER54;

	if ( type == OBJECT_BARRIER78 ||
		 type == OBJECT_BARRIER82 )  return OBJECT_BARRIER78;
	if ( type == OBJECT_BARRIER79 ||
		 type == OBJECT_BARRIER83 )  return OBJECT_BARRIER79;
	if ( type == OBJECT_BARRIER80 ||
		 type == OBJECT_BARRIER84 )  return OBJECT_BARRIER80;
	if ( type == OBJECT_BARRIER81 ||
		 type == OBJECT_BARRIER85 )  return OBJECT_BARRIER81;

	if ( type >= OBJECT_BARRIER92 &&
		 type <= OBJECT_BARRIER99 )  return OBJECT_BARRIER92;

	return type;
}

// Vérifie si un objet peut cohabiter avec un autre.
// Par exemple, une caisse peut exister au même endroit d'un dock.

BOOL IsColoc(ObjectType type, ObjectType newLoc)
{
	ObjectType	temp;
	int			i;

	type = RetFamily(type);
	newLoc = RetFamily(newLoc);

	for ( i=0 ; i<2 ; i++ )
	{
		if ( type >= OBJECT_BOX1  &&
			 type <= OBJECT_BOX20 &&
			 newLoc == OBJECT_DOCK )  return TRUE;

		if ( type >= OBJECT_BOX1  &&
			 type <= OBJECT_BOX20 &&
			 newLoc == OBJECT_CATAPULT )  return TRUE;

		temp = type;
		type = newLoc;
		newLoc = temp;  // type <-> newLoc
	}

	return FALSE;
}

// Choix d'une variante pour un objet.

void ChoiceVariante(ObjectType &type, float &angle, float &zoom, BOOL bRot)
{
	if ( type == OBJECT_BARRIER1  ||
		 type == OBJECT_BARRIER4  ||
		 type == OBJECT_BARRIER5  ||
		 type == OBJECT_BARRIER6  ||
		 type == OBJECT_BARRIER7  ||
		 type == OBJECT_BARRIER8  ||
		 type == OBJECT_BARRIER9  ||
		 type == OBJECT_BARRIER10 ||
		 type == OBJECT_BARRIER11 ||
		 type == OBJECT_BARRIER12 ||
		 type == OBJECT_BARRIER13 ||
		 type == OBJECT_BARRIER14 ||
		 type == OBJECT_BARRIER15 ||
		 type == OBJECT_BARRIER16 ||
		 type == OBJECT_BARRIER17 ||
		 type == OBJECT_BARRIER17 ||
		 type == OBJECT_BARRIER23 ||
		 type == OBJECT_BARRIER24 ||
		 type == OBJECT_BARRIER25 ||
		 type == OBJECT_BARRIER28 ||
		 type == OBJECT_BARRIER29 ||
		 type == OBJECT_BARRIER31 ||
		 type == OBJECT_BARRIER32 ||
		 type == OBJECT_BARRIER33 ||
		 type == OBJECT_BARRIER34 ||
		 type == OBJECT_BARRIER35 ||
		 type == OBJECT_BARRIER36 ||
		 type == OBJECT_BARRIER37 ||
		 type == OBJECT_BARRIER38 ||
		 type == OBJECT_BARRIER39 ||
		 type == OBJECT_BARRIER40 ||
		 type == OBJECT_BARRIER41 ||
		 type == OBJECT_BARRIER42 ||
		 type == OBJECT_BARRIER43 ||
		 type == OBJECT_BARRIER44 ||
		 type == OBJECT_BARRIER45 ||
		 type == OBJECT_BARRIER46 ||
		 type == OBJECT_BARRIER57 ||
		 type == OBJECT_BARRIER58 ||
		 type == OBJECT_BARRIER59 ||
		 type == OBJECT_BARRIER60 ||
		 type == OBJECT_BARRIER61 ||
		 type == OBJECT_BARRIER62 ||
		 type == OBJECT_BARRIER63 ||
		 type == OBJECT_BARRIER64 ||
		 type == OBJECT_BARRIER65 ||
		 type == OBJECT_BARRIER67 ||
		 type == OBJECT_BARRIER68 ||
		 type == OBJECT_BARRIER69 ||
		 type == OBJECT_BARRIER70 ||
		 type == OBJECT_BARRIER71 ||
		 type == OBJECT_BARRIER74 ||
		 type == OBJECT_BARRIER76 ||
		 type == OBJECT_BARRIER78 ||
		 type == OBJECT_BARRIER79 ||
		 type == OBJECT_BARRIER80 ||
		 type == OBJECT_BARRIER81 ||
		 type == OBJECT_BARRIER82 ||
		 type == OBJECT_BARRIER83 ||
		 type == OBJECT_BARRIER84 ||
		 type == OBJECT_BARRIER85 ||
		 type == OBJECT_BARRIER86 ||
		 type == OBJECT_BARRIER87 ||
		 type == OBJECT_BARRIER88 ||
		 type == OBJECT_BARRIER89 ||
		 type == OBJECT_BARRIER90 ||
		 type == OBJECT_BARRIER91 )
	{
		if ( bRot )  angle = NormAngle(Grid(angle+PI/2.0f, PI/2.0f));
	}

	if ( type == OBJECT_BARRIER0  ||
		 type == OBJECT_BARRIER2  ||
		 type == OBJECT_BARRIER3  ||
		 type == OBJECT_BARRIER26 ||
		 type == OBJECT_BARRIER27 ||
		 type == OBJECT_BARRIER30 ||
		 type == OBJECT_BARRIER47 ||
		 type == OBJECT_BARRIER72 ||
		 type == OBJECT_BARRIER73 ||
		 type == OBJECT_BARRIER75 ||
		 type == OBJECT_BARRIER77 )
	{
		angle = 0.0f;
	}

	if ( type == OBJECT_BARRIER50 ||
		 type == OBJECT_BARRIER51 ||
		 type == OBJECT_BARRIER52 ||
		 type == OBJECT_BARRIER53 )
	{
		angle = Rand()*PI*2.0f;
	}

	if ( type >= OBJECT_BARRIER18 &&
		 type <= OBJECT_BARRIER22 )  // cinéma ?
	{
		type = (ObjectType)(OBJECT_BARRIER18+rand()%5);
		if ( bRot )  angle = NormAngle(Grid(angle+PI/2.0f, PI/2.0f));
	}

	if ( type >= OBJECT_BARRIER54 &&
		 type <= OBJECT_BARRIER55 )  // barrière ?
	{
		type = (ObjectType)(OBJECT_BARRIER54+rand()%2);
		if ( bRot )
		{
			angle = Grid(angle+PI/2.0f, PI/2.0f);
		}
		else
		{
			if ( rand()%2 )  angle = NormAngle(angle+PI);
		}
	}

	if ( type == OBJECT_BARRIER66 )  // trésor ?
	{
		if ( bRot )
		{
			angle = NormAngle(Grid(angle+PI/2.0f, PI/2.0f));
			angle += (Rand()-0.5f)*(20.0f*PI/180.0f);
		}
	}

	if ( type == OBJECT_BARRIER78 )  // table ?
	{
		if ( rand()%2 )  type = OBJECT_BARRIER82;
	}
	if ( type == OBJECT_BARRIER79 )  // commode ?
	{
		if ( rand()%2 )  type = OBJECT_BARRIER83;
	}
	if ( type == OBJECT_BARRIER80 )  // armoire ?
	{
		if ( rand()%2 )  type = OBJECT_BARRIER84;
	}
	if ( type == OBJECT_BARRIER81 )  // buffet ?
	{
		if ( rand()%2 )  type = OBJECT_BARRIER85;
	}

	if ( type >= OBJECT_BARRIER92 &&
		 type <= OBJECT_BARRIER99 )  // palissade ?
	{
		type = (ObjectType)(OBJECT_BARRIER92+rand()%8);
		if ( bRot )
		{
			angle = NormAngle(Grid(angle+PI/2.0f, PI/2.0f));
		}
		else
		{
			if ( rand()%2 )  angle = NormAngle(angle+PI);
		}
	}

	if ( type == OBJECT_DOCK )
	{
		angle = 0.0f;
	}

	if ( type == OBJECT_DOOR1    ||
		 type == OBJECT_DOOR2    ||
		 type == OBJECT_CATAPULT )
	{
		if ( bRot )  angle = NormAngle(Grid(angle+PI/2.0f, PI/2.0f));
	}

	if ( type == OBJECT_BLUPI  ||
		 type == OBJECT_CRAZY  ||
		 type == OBJECT_TRAX   ||
		 type == OBJECT_PERFO  ||
		 type == OBJECT_GUN    ||
		 type == OBJECT_MAX1X  ||
		 type == OBJECT_GLASS1 ||
		 type == OBJECT_GLASS2 )
	{
		if ( bRot )  angle = Grid(angle+PI/2.0f, PI/2.0f);
	}

	if ( type == OBJECT_TREE0 )  // arbre feuillu ?
	{
		type = (ObjectType)(OBJECT_TREE0+rand()%4);
		angle = Rand()*PI*2.0f;
	}

	if ( type == OBJECT_TREE4 )  // palmier ?
	{
		type = (ObjectType)(OBJECT_TREE4+rand()%4);
		angle = Rand()*PI*2.0f;
	}

	if ( type == OBJECT_PLANT0 )  // plante normale ?
	{
		type = (ObjectType)(OBJECT_PLANT0+rand()%5);
		angle = Rand()*PI*2.0f;
		zoom = 0.6f+Rand()*0.2f;
	}

	if ( type == OBJECT_PLANT5 )  // plante tombante ?
	{
		type = (ObjectType)(OBJECT_PLANT5+rand()%5);
	}

	if ( type == OBJECT_PLANT10 )  // plante grasse ?
	{
		type = (ObjectType)(OBJECT_PLANT10+rand()%5);
		angle = Rand()*PI*2.0f;
		zoom = 0.6f+Rand()*0.3f;
	}

	if ( type == OBJECT_PLANT15 )  // fougère ?
	{
		type = (ObjectType)(OBJECT_PLANT15+rand()%5);
		angle = Rand()*PI*2.0f;
		zoom = 0.6f+Rand()*0.3f;
	}

	if ( type >= OBJECT_BOX1  &&
		 type <= OBJECT_BOX20 )
	{
		angle = 0.0f;
	}

	if ( type == OBJECT_BOX11 )
	{
		 type = OBJECT_BOX12;
	}
	else if ( type == OBJECT_BOX12 )
	{
		 type = OBJECT_BOX11;
	}
}

// Retourne l'angle et le zoom initial pour un objet.

void CMainEdit::InitAngleZoom(ObjectType &type, float &angle, float &zoom)
{
	angle = m_lastAngles[type];
	zoom  = 1.0f;

	if ( type == OBJECT_PLANT0  ||
		 type == OBJECT_PLANT5  ||
		 type == OBJECT_PLANT10 ||
		 type == OBJECT_PLANT15 ||
		 type == OBJECT_TREE0   ||
		 type == OBJECT_TREE4   )
	{
		ChoiceVariante(type, angle, zoom, TRUE);
	}
	else
	{
		ChoiceVariante(type, angle, zoom, FALSE);
	}
}

// Crée un nouvel objet, ou modifie un objet existant.

void CMainEdit::CreateObject(D3DVECTOR pos, ObjectType type)
{
	CObject*	pObj;
	D3DVECTOR	ppos;
	float		angle, zoom;
	BOOL		bTerrain;
	int			err, rank, i, option;
	int			addHat, addGlass, addGlove, addShoe, addBag;

	err = CheckCreate(pos, type);  // trop d'objets de ce type ?
	if ( err != 0 )
	{
		m_dialog->StartError(err);
		return;
	}

	if ( type == OBJECT_PLANT5 )  // plante tombante ?
	{
		while ( TRUE )
		{
			pObj = SearchPlant5(pos);
			if ( pObj == 0 )  break;
			DeleteObject(pObj);
		}
		for ( i=0 ; i<4 ; i++ )
		{
			ppos = pos;
			if ( i == 0 )  ppos.x += 8.0f;
			if ( i == 1 )  ppos.x -= 8.0f;
			if ( i == 2 )  ppos.z += 8.0f;
			if ( i == 3 )  ppos.z -= 8.0f;

			if ( !m_terrain->IsSolid(ppos) )
			{
				type = OBJECT_PLANT5;
				ChoiceVariante(type, angle, zoom, TRUE);
				if ( i == 0 )  angle = PI*1.0f;
				if ( i == 1 )  angle = PI*0.0f;
				if ( i == 2 )  angle = PI*0.5f;
				if ( i == 3 )  angle = PI*1.5f;
				zoom = 1.0f;
				pObj = new CObject(m_iMan);
				pObj->CreateObject(pos, angle, zoom, 0.0f, type);
			}
		}
		m_engine->LoadAllTexture();
		return;
	}

	InitAngleZoom(type, angle, zoom);
	option = 0;

	pObj = SearchObject(pos, m_level, type);
	if ( pObj != 0 )
	{
		if ( RetFamily(type) == RetFamily(pObj->RetType()) )
		{
			type = RetFamily(type);
			ChoiceVariante(type, angle, zoom, TRUE);
		}

		if ( type == pObj->RetType() &&
			 type != OBJECT_DOCK     )
		{
			m_lastAngles[RetFamily(type)] = angle;
			pObj->SetAngleY(0, angle);
			pObj->SetZoomY(0, zoom);
			AdaptBarrier();
			return;
		}

		option = pObj->RetOption();
		DeleteObject(pObj);
	}

	m_lastAngles[RetFamily(type)] = angle;

	pObj = new CObject(m_iMan);

	bTerrain = FALSE;
	if ( type == OBJECT_BLUPI )
	{
		m_terrain->SetResource(pos, TR_LIFT, TRUE);

		rank = RetTotalBlupi();
		addHat   = m_main->RetAdditional(rank, ADD_HAT);
		addGlass = m_main->RetAdditional(rank, ADD_GLASS);
		addGlove = m_main->RetAdditional(rank, ADD_GLOVE);
		addShoe  = m_main->RetAdditional(rank, ADD_SHOE);
		addBag   = m_main->RetAdditional(rank, ADD_BAG);

		pObj->CreateObject(pos, angle, zoom, 0.0f, type, 1,
						   addHat, addGlass, addGlove, addShoe, addBag);
		bTerrain = TRUE;
	}
	else if ( type == OBJECT_GOAL )
	{
		m_terrain->SetResource(pos, TR_SOLID1, TRUE);
		pObj->CreateObject(pos, angle, zoom, 0.0f, type);
		bTerrain = TRUE;
	}
	else if ( type == OBJECT_MAX1X )
	{
		m_terrain->SetResource(pos, TR_HOLE, TRUE);
		pObj->CreateObject(pos, angle, zoom, 0.0f, type);
		bTerrain = TRUE;
	}
	else if ( type == OBJECT_DOCK )
	{
		if ( option < 8 )  option ++;
		else               option = 1;
		pObj->CreateObject(pos, angle, zoom, 0.0f, type, option);
	}
	else
	{
		pObj->CreateObject(pos, angle, zoom, 0.0f, type);
	}

	m_engine->LoadAllTexture();
	AdaptGround();
	AdaptBarrier();
	RebuildDecor(bTerrain, FALSE, TRUE);
}

// Retourne le nombre total de blupi existant.

int CMainEdit::RetTotalBlupi()
{
	CObject*	pObj;
	ObjectType	type;
	int			total, i;

	total = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type == OBJECT_BLUPI )  total ++;
	}

	return total;
}


// Crée une marque au sol.

void CMainEdit::CreateSignMark(D3DVECTOR pos, int sign)
{
	ParticuleType	type;
	float			angle;

	if ( m_terrain->SignMarkGet(pos, angle, type) &&
		 type == (ParticuleType)(PARTISIGNMARK0+sign) )
	{
		angle = NormAngle(angle+PI/2.0f);
		m_lastAngleSignMark = angle;
		m_terrain->SignMarkDelete(pos);
		m_terrain->SignMarkCreate(pos, angle, type);
	}
	else
	{
		m_terrain->SignMarkDelete(pos);
		m_terrain->SignMarkCreate(pos, m_lastAngleSignMark, (ParticuleType)(PARTISIGNMARK0+sign));
	}
}


// Vide la liste des visites.

void CMainEdit::FlushVisitList()
{
	m_visitListTotal = 0;
}

// Ajoute un objet visité.

BOOL CMainEdit::AddVisitList(CObject *pObj)
{
	if ( m_visitListTotal >= 200 )  return FALSE;
	m_visitListBuffer[m_visitListTotal++] = pObj;
	return TRUE;
}

// Indique si un objet à été visité.

BOOL CMainEdit::IsVisitList(CObject *pObj)
{
	int		i;

	for ( i=0 ; i<m_visitListTotal ; i++ )
	{
		if ( pObj == m_visitListBuffer[i] )  return TRUE;
	}
	return FALSE;
}


// Retourne la famille d'un objet de sol.

ObjectType RetFamilyGround(ObjectType type)
{
	if ( type == OBJECT_GROUND2 ||
		 type == OBJECT_GROUND4 )  return OBJECT_GROUND2;

	if ( type == OBJECT_BARRIER29 ||
		 type == OBJECT_BARRIER48 ||
		 type == OBJECT_BARRIER49 )  return OBJECT_BARRIER29;

	return type;
}

// Cherche un objet identique autour.

BOOL CMainEdit::SearchBrother(CObject *pObj, D3DVECTOR &dir)
{
	CObject*	pOther;
	ObjectType	type;
	D3DVECTOR	pos, sPos;

	type = RetFamilyGround(pObj->RetType());
	pos = pObj->RetPosition(0);

	sPos = pos;
	sPos.x += 8.0f;
	pOther = SearchObject(sPos, LEVEL_GROUND);
	if ( IsVisitList(pOther) )  pOther = 0;
	if ( pOther != 0 && type == RetFamilyGround(pOther->RetType()) )
	{
		dir = sPos-pos;
		return TRUE;
	}

	sPos = pos;
	sPos.x -= 8.0f;
	pOther = SearchObject(sPos, LEVEL_GROUND);
	if ( IsVisitList(pOther) )  pOther = 0;
	if ( pOther != 0 && type == RetFamilyGround(pOther->RetType()) )
	{
		dir = sPos-pos;
		return TRUE;
	}

	sPos = pos;
	sPos.z += 8.0f;
	pOther = SearchObject(sPos, LEVEL_GROUND);
	if ( IsVisitList(pOther) )  pOther = 0;
	if ( pOther != 0 && type == RetFamilyGround(pOther->RetType()) )
	{
		dir = sPos-pos;
		return TRUE;
	}

	sPos = pos;
	sPos.z -= 8.0f;
	pOther = SearchObject(sPos, LEVEL_GROUND);
	if ( IsVisitList(pOther) )  pOther = 0;
	if ( pOther != 0 && type == RetFamilyGround(pOther->RetType()) )
	{
		dir = sPos-pos;
		return TRUE;
	}

	dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
	return FALSE;
}

// Cherche le premier et le dernier objet d'une ligne.

BOOL CMainEdit::SearchLimit(CObject *pObj, D3DVECTOR &dir,
							D3DVECTOR &src, D3DVECTOR &dst)
{
	ObjectType	type;
	CObject*	pOther;
	D3DVECTOR	pos, sPos;

	type = RetFamilyGround(pObj->RetType());
	pos = pObj->RetPosition(0);

	if ( !SearchBrother(pObj, dir) )
	{
		src = dst = pos;

		sPos = pos;
		sPos.x += 8.0f;
		if ( m_terrain->IsSolid(sPos) )
		{
			dir = pos-sPos;
			return TRUE;
		}

		sPos = pos;
		sPos.x -= 8.0f;
		if ( m_terrain->IsSolid(sPos) )
		{
			dir = pos-sPos;
			return TRUE;
		}

		sPos = pos;
		sPos.z += 8.0f;
		if ( m_terrain->IsSolid(sPos) )
		{
			dir = pos-sPos;
			return TRUE;
		}

		sPos = pos;
		sPos.z -= 8.0f;
		if ( m_terrain->IsSolid(sPos) )
		{
			dir = pos-sPos;
			return TRUE;
		}

		return FALSE;
	}

	while ( TRUE )
	{
		src = pos;
		pos -= dir;
		pOther = SearchObject(pos, LEVEL_GROUND);
		if ( IsVisitList(pOther) )  pOther = 0;
		if ( pOther == 0 || type != RetFamilyGround(pOther->RetType()) )  break;
	}

	pos = pObj->RetPosition(0);
	while ( TRUE )
	{
		dst = pos;
		pos += dir;
		pOther = SearchObject(pos, LEVEL_GROUND);
		if ( IsVisitList(pOther) )  pOther = 0;
		if ( pOther == 0 || type != RetFamilyGround(pOther->RetType()) )  break;
	}

	return TRUE;
}

// Adapte un objet.

void CMainEdit::AdaptObject(CObject *pObj)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	CObject*	pOther;
	ObjectType	type, oType;
	int			nb, option;

	type = pObj->RetType();
	mat = pObj->RetWorldMatrix(0);

	if ( type == OBJECT_GROUND2 )  // pont-levis ?
	{
		nb = 0;

		pos = Transform(*mat, D3DVECTOR(8.0f, 0.0f, 8.0f));
		pOther = SearchObject(pos, LEVEL_OBJECT);
		oType = OBJECT_NULL;
		if ( pOther != 0 )  oType = pOther->RetType();
		if ( oType == OBJECT_BARRIER12 ||
			 oType == OBJECT_BARRIER46 )  nb ++;

		pos = Transform(*mat, D3DVECTOR(-8.0f, 0.0f, 8.0f));
		pOther = SearchObject(pos, LEVEL_OBJECT);
		oType = OBJECT_NULL;
		if ( pOther != 0 )  oType = pOther->RetType();
		if ( oType == OBJECT_BARRIER12 ||
			 oType == OBJECT_BARRIER46 )  nb ++;

		// Si 2 tours gauche+droite -> support visible.
		option = (nb<2)?0:1;
		pObj->SetOption(option);
		m_engine->SetObjectHide(pObj->RetObjectRank(1), option==0);
	}
}

// Adapte les objets d'une ligne.

BOOL CMainEdit::AdaptLimit(CObject *pObj, ObjectType family)
{
	ObjectType	type;
	D3DVECTOR	dir, src, dst, pos;
	float		dist, angle;
	int			total, i;

	if ( SearchLimit(pObj, dir, src, dst) )
	{
		dist = Length2d(src, dst);
		total = (int)(dist/8.0f);

		pos = src;
		angle = RotateAngle(dir.x, -dir.z)-PI/2.0f;
		if ( family == OBJECT_GROUND0   ||
			 family == OBJECT_GROUND3   ||
			 family == OBJECT_BARRIER39 )  angle += PI/2.0f;

		for ( i=0 ; i<=total ; i++ )
		{
			pObj = SearchObject(pos, LEVEL_GROUND);
			if ( pObj != 0 )
			{
				pObj->DeleteObject();
				delete pObj;

				if ( i == 0 )  // premier ?
				{
					type = family;
					if ( family == OBJECT_BARRIER29 )  // tuyau ?
					{
						type = OBJECT_BARRIER48;
					}
				}
				else if ( i < total )  // milieu ?
				{
					type = family;
					if ( family == OBJECT_GROUND2 )  // pont-levis ?
					{
						type = OBJECT_GROUND4;
					}
				}
				else	// dernier ?
				{
					type = family;
					if ( family == OBJECT_GROUND2 )
					{
						angle += PI;
					}
					if ( family == OBJECT_BARRIER29 )  // tuyau ?
					{
						type = OBJECT_BARRIER49;
					}
				}

				pObj = new CObject(m_iMan);
				pObj->CreateObject(pos, angle, 1.0f, 0.0f, type);
				AdaptObject(pObj);
				AddVisitList(pObj);
			}

			pos += dir;
		}
		return TRUE;
	}

	return FALSE;
}

// Adapte les sols.

void CMainEdit::AdaptGround()
{
	CObject*	pObj;
	ObjectType	family;
	BOOL		bDo;
	int			i;

	FlushVisitList();
	do
	{
		bDo = FALSE;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;
			if ( IsVisitList(pObj) )  continue;

			family = RetFamilyGround(pObj->RetType());

			if ( family == OBJECT_GROUND0   ||
				 family == OBJECT_GROUND2   ||
				 family == OBJECT_GROUND3   ||
				 family == OBJECT_BARRIER29 ||
				 family == OBJECT_BARRIER39 ||
				 family == OBJECT_BARRIER67 )
			{
				if ( AdaptLimit(pObj, family) )
				{
					bDo = TRUE;
					break;
				}
			}
		}
	}
	while ( bDo );
}

// Adapte les barrières.

void CMainEdit::AdaptBarrier()
{
	D3DVECTOR	pos;
	CObject*	pObj;
	CObject*	pOther;
	ObjectType	type, oType[2];
	BOOL		bDo, bVert;
	float		angle;
	int			i;

	do
	{
		bDo = FALSE;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;

			type = pObj->RetType();

			if ( type == OBJECT_BARRIER54 ||
				 type == OBJECT_BARRIER55 ||
				 type == OBJECT_BARRIER56 )   // barrière grotte ?
			{
				angle = NormAngle(pObj->RetAngleY(0));
				bVert = ( angle > PI*0.25f && angle < PI*0.75f ) ||
						( angle > PI*1.25f && angle < PI*1.75f );

				pos = pObj->RetPosition(0);
				if ( bVert )  pos.z += 8.0f;
				else          pos.x += 8.0f;
				pOther = SearchObject(pos, LEVEL_OBJECT);
				oType[0] = OBJECT_NULL;
				if ( pOther != 0 )  oType[0] = pOther->RetType();

				pos = pObj->RetPosition(0);
				if ( bVert )  pos.z -= 8.0f;
				else          pos.x -= 8.0f;
				pOther = SearchObject(pos, LEVEL_OBJECT);
				oType[1] = OBJECT_NULL;
				if ( pOther != 0 )  oType[1] = pOther->RetType();

				pos = pObj->RetPosition(0);

				angle = NAN;
				if ( oType[0] == OBJECT_BARRIER50 ||
					 oType[0] == OBJECT_BARRIER51 )
				{
					if ( bVert )  angle = PI*0.5f;
					else          angle = PI*1.0f;
				}
				if ( oType[1] == OBJECT_BARRIER50 ||
					 oType[1] == OBJECT_BARRIER51 )
				{
					if ( bVert )  angle = PI*1.5f;
					else          angle = PI*0.0f;
				}

				if ( type == OBJECT_BARRIER56 )  // barrière longue ?
				{
					if ( angle != NAN )
					{
						pObj->SetAngleY(0, angle);
					}
					else
					{
						angle = pObj->RetAngleY(0);

						pObj->DeleteObject();
						delete pObj;

						pObj = new CObject(m_iMan);
						pObj->CreateObject(pos, angle, 1.0f, 0.0f, (ObjectType)(OBJECT_BARRIER54+rand()%2));

						bDo = TRUE;
						break;
					}
				}
				else
				{
					if ( angle != NAN )
					{
						pObj->DeleteObject();
						delete pObj;

						pObj = new CObject(m_iMan);
						pObj->CreateObject(pos, angle, 1.0f, 0.0f, OBJECT_BARRIER56);

						bDo = TRUE;
						break;
					}
				}
			}
		}
	}
	while ( bDo );
}


// Adapte les plantes tombantes en fonction des sols.

void CMainEdit::AdaptPlant()
{
	CObject*	pObj;
	ObjectType	type;
	D3DVECTOR	pos;
	BOOL		bDo;
	float		angle;
	int			i;

	FlushVisitList();
	do
	{
		bDo = FALSE;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;

			type = pObj->RetType();
			if ( type < OBJECT_PLANT5 ||
				 type > OBJECT_PLANT9 )  continue;

			pos = pObj->RetPosition(0);
			angle = pObj->RetAngleY(0);

			if ( !m_terrain->IsSolid(pos) )
			{
				pObj->DeleteObject();
				delete pObj;
				bDo = TRUE;
				break;
			}

			if ( angle == PI*0.0f )
			{
				pos.x -= 8.0f;
				if ( m_terrain->IsSolid(pos) )
				{
					pObj->DeleteObject();
					delete pObj;
					bDo = TRUE;
					break;
				}
			}

			if ( angle == PI*0.5f )
			{
				pos.z += 8.0f;
				if ( m_terrain->IsSolid(pos) )
				{
					pObj->DeleteObject();
					delete pObj;
					bDo = TRUE;
					break;
				}
			}

			if ( angle == PI*1.0f )
			{
				pos.x += 8.0f;
				if ( m_terrain->IsSolid(pos) )
				{
					pObj->DeleteObject();
					delete pObj;
					bDo = TRUE;
					break;
				}
			}

			if ( angle == PI*1.5f )
			{
				pos.z -= 8.0f;
				if ( m_terrain->IsSolid(pos) )
				{
					pObj->DeleteObject();
					delete pObj;
					bDo = TRUE;
					break;
				}
			}

			AddVisitList(pObj);
		}
	}
	while ( bDo );
}


// Configure un menu.

void CMainEdit::ConfigMenu(CMenu *pm, ToolMenu *menu)
{
	int		i;

	pm->SetSubTotal(menu->total);
	pm->SetSelect(menu->select);

	if ( menu->total == 0 )
	{
		pm->SetState(STATE_VISIBLE, FALSE);
	}
	else
	{
		pm->SetState(STATE_VISIBLE, TRUE);

		for ( i=0 ; i<menu->total ; i++ )
		{
			if ( i == menu->select )
			{
				pm->SetIcon(menu->tool[i].icon);

				if ( menu->tool[i].icon == -1 )
				{
					pm->SetTexture(menu->tool[i].texture, menu->tool[i].uv1, menu->tool[i].uv2);
				}
				else
				{
					pm->SetTexture("", FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f));
				}
			}

			pm->SetSubIcon(i, menu->tool[i].icon);

			if ( menu->tool[i].icon == -1 )
			{
				pm->SetSubTexture(i, menu->tool[i].texture, menu->tool[i].uv1, menu->tool[i].uv2);
			}
			else
			{
				pm->SetSubTexture(i, "", FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f));
			}
		}
	}
}

// Met à jour un menu après qu'il ait été cliqué.

void CMainEdit::UpdateMenu(CMenu *pm, ToolMenu *menu)
{
	menu->select = pm->RetSelect();
}

// Met à jour les boutons.

void CMainEdit::UpdateButtons()
{
	m_button[0]->SetState(STATE_CHECK, m_level==LEVEL_OBJECT);
	m_button[1]->SetState(STATE_CHECK, m_level==LEVEL_GROUND);
	m_button[2]->SetState(STATE_CHECK, m_level==LEVEL_WATER);

	if ( m_level == LEVEL_OBJECT )
	{
		m_button[3]->SetIcon(11);
		m_button[3]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_DELETE);
		m_button[3]->SetState(STATE_VISIBLE, TRUE);

		m_menu[0]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_BOX);
		ConfigMenu(m_menu[0], &m_menuBox);

		m_menu[1]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_BARRIER);
		ConfigMenu(m_menu[1], &m_menuBarrier);

		m_menu[2]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_PLANT);
		ConfigMenu(m_menu[2], &m_menuPlant);

		m_menu[3]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_BIG);
		ConfigMenu(m_menu[3], &m_menuBig);

		m_menu[4]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_BLUPI);
		ConfigMenu(m_menu[4], &m_menuBlupi);

		m_menu[5]->SetState(STATE_CHECK, FALSE);
		m_menu[5]->SetState(STATE_VISIBLE, FALSE);

		m_menu[6]->SetState(STATE_CHECK, FALSE);
		m_menu[6]->SetState(STATE_VISIBLE, FALSE);

		m_menu[7]->SetState(STATE_CHECK, FALSE);
		m_menu[7]->SetState(STATE_VISIBLE, FALSE);

		m_label->SetState(STATE_VISIBLE, FALSE);
	}

	if ( m_level == LEVEL_GROUND )
	{
		m_button[3]->SetIcon(11);
		m_button[3]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_DELETE);
		m_button[3]->SetState(STATE_VISIBLE, TRUE);

		m_menu[0]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_GROUND);
		ConfigMenu(m_menu[0], &m_menuGround);

		m_menu[1]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_DECORDOWN);
		ConfigMenu(m_menu[1], &m_menuDecorDown);

		m_menu[2]->SetState(STATE_CHECK, FALSE);
		m_menu[2]->SetState(STATE_VISIBLE, FALSE);

		m_menu[3]->SetState(STATE_CHECK, FALSE);
		m_menu[3]->SetState(STATE_VISIBLE, FALSE);

		m_menu[4]->SetState(STATE_CHECK, m_tool[m_level]==TOOL_SIGNMARK);
		ConfigMenu(m_menu[4], &m_menuSignMark);

		m_menu[5]->SetState(STATE_CHECK, FALSE);
		m_menu[5]->SetState(STATE_VISIBLE, FALSE);

		m_menu[6]->SetState(STATE_CHECK, FALSE);
		m_menu[6]->SetState(STATE_VISIBLE, FALSE);

		m_menu[7]->SetState(STATE_CHECK, FALSE);
		m_menu[7]->SetState(STATE_VISIBLE, FALSE);

		m_label->SetState(STATE_VISIBLE, TRUE);
	}

	if ( m_level == LEVEL_WATER )
	{
		m_button[3]->SetState(STATE_CHECK, FALSE);
		m_button[3]->SetState(STATE_VISIBLE, FALSE);

		m_menu[0]->SetState(STATE_CHECK, FALSE);
		m_menu[0]->SetState(STATE_VISIBLE, FALSE);

		m_menu[1]->SetState(STATE_CHECK, FALSE);
		m_menu[1]->SetState(STATE_VISIBLE, FALSE);

		m_menu[2]->SetState(STATE_CHECK, FALSE);
		m_menu[2]->SetState(STATE_VISIBLE, FALSE);

		m_menu[3]->SetState(STATE_CHECK, FALSE);
		m_menu[3]->SetState(STATE_VISIBLE, FALSE);

		m_menu[4]->SetState(STATE_CHECK, FALSE);
		m_menu[4]->SetState(STATE_VISIBLE, FALSE);

		m_menu[5]->SetState(STATE_CHECK, FALSE);
		m_menu[5]->SetState(STATE_VISIBLE, FALSE);

		m_menu[6]->SetState(STATE_CHECK, FALSE);
		m_menu[6]->SetState(STATE_VISIBLE, FALSE);

		m_menu[7]->SetState(STATE_CHECK, FALSE);
		m_menu[7]->SetState(STATE_VISIBLE, FALSE);

		m_label->SetState(STATE_VISIBLE, FALSE);
	}
}


// Refait le décor suite à une modification.

void CMainEdit::RebuildDecor(BOOL bTerrain, BOOL bWater, BOOL bGrid)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	ObjectType	type;
	int			i;

	if ( bTerrain )
	{
		m_terrain->DeleteObjects();
		m_terrain->RestoreResourceCopy();
		m_terrain->CreateObjects();

		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;

			type = pObj->RetType();

			if ( type == OBJECT_BLUPI )
			{
				pos = pObj->RetPosition(0);
				m_terrain->SetResource(pos, TR_BOX);
			}

			if ( type >= OBJECT_GROUND0  &&
				 type <= OBJECT_GROUND19 )
			{
				pos = pObj->RetPosition(0);
				m_terrain->SetResource(pos, TR_BOX);
			}

			if ( type == OBJECT_MAX1X )
			{
				pos = pObj->RetPosition(0);
				m_terrain->SetResource(pos, TR_BOX);
			}
		}

		AdaptPlant();
	}

	if ( bWater )
	{
		m_water->Create();
	}

	if ( bGrid )
	{
		GridCreate();
	}
}


// Indique si un objet est un décor bas.

BOOL CMainEdit::IsDecorDown(D3DVECTOR pos)
{
	CObject*	pObj;
	ObjectType	type;

	if ( m_terrain->IsSolid(pos) )  return TRUE;

	pObj = SearchObject(pos, LEVEL_GROUND);
	if ( pObj == 0 )  return FALSE;

	type = pObj->RetType();

	if ( type >= OBJECT_GROUND0  &&
		 type <= OBJECT_GROUND19 )  return TRUE;

	if ( type == OBJECT_BARRIER29 ||
		 type == OBJECT_BARRIER39 ||
		 type == OBJECT_BARRIER48 ||
		 type == OBJECT_BARRIER49 ||
		 type == OBJECT_BARRIER67 )  return TRUE;

	return FALSE;
}

// Vérifie si une position est ok pour la grille.

BOOL CMainEdit::GridCheckPos(D3DVECTOR pos)
{
	CObject*	pObj;
	float		dim;

	if ( m_level == LEVEL_OBJECT )  // objets ?
	{
		if ( m_tool[m_level] == TOOL_DELETE )
		{
			if ( SearchObject(pos, m_level) == 0 )  return FALSE;
		}
		else if ( m_tool[m_level] == TOOL_BIG )
		{
			if ( !m_terrain->IsSolid(pos) )  return FALSE;
		}
		else if ( m_tool[m_level] == TOOL_BLUPI )
		{
			pObj = SearchObject(pos, m_level);
			if ( pObj != 0 )
			{
				if ( pObj->RetType() == OBJECT_BLUPI ||
					 pObj->RetType() == OBJECT_CRAZY )  return TRUE;
				else                                    return FALSE;
			}
			if ( m_terrain->RetResource(pos) == TR_BOX )  return FALSE;
			if ( m_terrain->RetResource(pos) == TR_HOLE )  return FALSE;
			if ( m_terrain->RetResource(pos) == TR_SPACE )  return FALSE;
		}
		else
		{
			pObj = SearchObject(pos, m_level);
			if ( pObj != 0 )
			{
				if ( pObj->RetType() == OBJECT_BLUPI ||
					 pObj->RetType() == OBJECT_CRAZY )  return FALSE;
			}
			if ( m_terrain->RetResource(pos) == TR_SPACE )  return FALSE;
		}
		return TRUE;
	}

	if ( m_level == LEVEL_GROUND )  // sol ?
	{
		if ( m_tool[m_level] == TOOL_DELETE )
		{
			if ( SearchObject(pos, LEVEL_OBJECT) != 0 )  return FALSE;
			if ( SearchObject(pos, LEVEL_GROUND) != 0 )  return TRUE;
//?			if ( m_terrain->IsLockZone(pos) )  return FALSE;
			if ( !m_terrain->IsSolid(pos) )  return FALSE;
		}
		else if ( m_tool[m_level] == TOOL_DECORDOWN )
		{
			if ( m_terrain->IsSolid(pos) )  return FALSE;
			if ( !IsDecorDown(D3DVECTOR(pos.x+8.0f, pos.y, pos.z)) &&
				 !IsDecorDown(D3DVECTOR(pos.x-8.0f, pos.y, pos.z)) &&
				 !IsDecorDown(D3DVECTOR(pos.x, pos.y, pos.z+8.0f)) &&
				 !IsDecorDown(D3DVECTOR(pos.x, pos.y, pos.z-8.0f)) )  return FALSE;
		}
		else if ( m_tool[m_level] == TOOL_SIGNMARK )
		{
			if ( !m_terrain->IsSolid(pos) )  return FALSE;
		}
		else
		{
			if ( m_terrain->RetResource(pos) == TR_SPACE )  return FALSE;
//?			if ( SearchObject(pos, LEVEL_WATER) != 0 )  return FALSE;
			pObj = SearchObject(pos, LEVEL_WATER);
			if ( pObj != 0 )
			{
				if ( pObj->RetType() == OBJECT_DOCK     ||
					 pObj->RetType() == OBJECT_CATAPULT )  return TRUE;
				return FALSE;
			}
		}
		return TRUE;
	}

	if ( m_level == LEVEL_WATER )  // eau ?
	{
		dim = m_dimTile*(m_nbTiles2-1);
		if ( pos.x < -dim || pos.x > dim ||
			 pos.z < -dim || pos.z > dim )  return FALSE;
		if ( m_terrain->IsSolid(pos) )  return FALSE;
		if ( SearchObject(pos, m_level) != 0 )  return FALSE;

		return TRUE;
	}

	return FALSE;
}

// Crée la grille.

void CMainEdit::GridCreate()
{
	ParticuleType	pType;
	D3DVECTOR		pos, speed;
	FPOINT			dim;
	float			height[2];
	int				x, y, channel;

	GridFlush();  // détruit la grille existante

	height[0] = 0.0f;
	height[1] = (m_model==0)?-8.0f:-5.0f;

	m_gridTotal = m_nbTiles*m_nbTiles;
	m_gridTable = (GridItem*)malloc(sizeof(GridItem)*m_gridTotal);
	if ( m_gridTable == 0 )  return;

	speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
	dim.x = m_dimTile*0.5f;
	dim.y = dim.x;

	if ( m_level == LEVEL_OBJECT )  // objets ?
	{
		if ( m_tool[m_level] == TOOL_DELETE )
		{
			dim.x *= 1.2f;
			dim.y *= 1.2f;
		}
	}

	for ( y=0 ; y<m_nbTiles ; y++ )
	{
		for ( x=0 ; x<m_nbTiles ; x++ )
		{
			m_gridTable[x+y*m_nbTiles].channel = -1;

			pos.x = m_dimTile*(x-m_nbTiles2);
			pos.z = m_dimTile*(y-m_nbTiles2);

			if ( !GridCheckPos(pos) )  continue;

			if ( m_level == LEVEL_OBJECT )  // objets ?
			{
				pos.y = height[0];
				pType = PARTIGRIDt;
				if ( !m_terrain->IsSolid(x,y) )
				{
					pos.y = height[1];
				}
				pos.y += m_gridHeight;
				channel = m_particule->CreateParticule(pos, speed, dim, pType);
				m_gridTable[x+y*m_nbTiles].channel = channel;
			}

			if ( m_level == LEVEL_GROUND )  // sol ?
			{
				if ( m_tool[m_level] == TOOL_DELETE )
				{
					pos.y = height[0];
					pType = PARTIGRIDt;
				}
				else
				{
					pos.y = height[0];
					pType = PARTIGRIDt;
					if ( m_terrain->IsSolid(x,y) )
					{
						pType = PARTIGRIDb;
					}
				}
				pos.y += m_gridHeight;
				channel = m_particule->CreateParticule(pos, speed, dim, pType);
				m_gridTable[x+y*m_nbTiles].channel = channel;
			}

			if ( m_level == LEVEL_WATER )  // eau ?
			{
				pos.y = height[1];
				pType = PARTIGRIDt;
				if ( m_terrain->RetResource(x,y) == TR_SPACE )
				{
					pType = PARTIGRIDw;
				}
				pos.y += m_gridHeight;
				channel = m_particule->CreateParticule(pos, speed, dim, pType);
				m_gridTable[x+y*m_nbTiles].channel = channel;
			}

			m_gridTable[x+y*m_nbTiles].height = pos.y-m_gridHeight;
		}
	}
}

// Détruit la grille.

void CMainEdit::GridFlush()
{
	int		i;

	if ( m_gridTable == 0 )  return;

	for ( i=0 ; i<m_gridTotal ; i++ )
	{
		m_particule->DeleteParticule(m_gridTable[i].channel);
	}

	delete m_gridTable;
	m_gridTable = 0;
}

// Cherche toutes les positions occupées pour un objet.

void CMainEdit::GridSelectPos(D3DVECTOR center, ObjectType type)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	FPOINT		rot;
	float		angle, zoom;
	int			i;

	if ( m_level != LEVEL_OBJECT     ||
		 m_tool[m_level] != TOOL_BIG )
	{
		m_gridSelectPos[0] = center;
		m_gridSelectType[0] = PARTIGRIDs;
		m_gridSelectTotal = 1;
		m_bGridSelectOK = TRUE;
		return;
	}

	InitAngleZoom(type, angle, zoom);

	pObj = SearchObject(center, m_level);
	if ( pObj != 0 )
	{
		type   = pObj->RetType();
		angle  = pObj->RetAngleY(0);
		zoom   = pObj->RetZoomY(0);
		ChoiceVariante(type, angle, zoom, FALSE);
	}

	i = 0;
	if ( type == OBJECT_DOOR1 ||
		 type == OBJECT_DOOR2 )
	{
		m_gridSelectPos[i++] = center;

		rot = RotatePoint(angle, FPOINT(-8.0f, 0.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(8.0f, 0.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;
	}
	else
	if ( type == OBJECT_CATAPULT )
	{
		m_gridSelectPos[i++] = center;

		rot = RotatePoint(angle, FPOINT(0.0f, -8.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(0.0f, 8.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;
	}
	else
	if ( type == OBJECT_DOCK )
	{
		m_gridSelectPos[i++] = center;

		rot = RotatePoint(angle, FPOINT(-32.0f, -24.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(-32.0f, -16.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(-32.0f, -8.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(-32.0f, 0.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(-32.0f, 8.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(-32.0f, 16.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(-32.0f, 24.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, -24.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, -16.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, -8.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, 0.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, 8.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, 16.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;

		rot = RotatePoint(angle, FPOINT(32.0f, 24.0f));
		pos.x = center.x+rot.x;
		pos.z = center.z+rot.y;
		pos.y = center.y;
		m_gridSelectPos[i++] = pos;
	}
	else
	{
		m_gridSelectPos[i++] = center;
	}
	m_gridSelectTotal = i;

	m_bGridSelectOK = TRUE;
	for ( i=0 ; i<m_gridSelectTotal ; i++ )
	{
		m_gridSelectType[i] = PARTIGRIDs;

		pObj = SearchObject(m_gridSelectPos[i], LEVEL_OBJECT);
		if ( i == 0 )
		{
//?			if ( pObj != 0 && pObj->RetType() != type )
//?			{
//?				m_gridSelectType[i] = PARTIGRIDe;
//?				m_bGridSelectOK = FALSE;
//?			}
		}
		else
		{
			if ( pObj != 0 )
			{
				m_gridSelectType[i] = PARTIGRIDe;
				m_bGridSelectOK = FALSE;
			}
		}

		if ( !m_terrain->IsSolid(m_gridSelectPos[i]) )
		{
			m_gridSelectType[i] = PARTIGRIDe;
			m_bGridSelectOK = FALSE;
		}
	}
}

// Sélectionne une case de la grille.

void CMainEdit::GridSelect(D3DVECTOR p1, D3DVECTOR p2, ObjectType type)
{
	D3DVECTOR		pos, speed;
	FPOINT			dim;
	float			height;
	int				i, x, y;

	// Supprime la sélection précédente.
	for ( i=0 ; i<m_gridSelectTotal ; i++ )
	{
		if ( m_gridSelectParti[i] != -1 )
		{
			m_particule->DeleteParticule(m_gridSelectParti[i]);
			m_gridSelectParti[i] = -1;
		}
	}

	if ( p1.x == NAN || p2.x == NAN )  return;

	x = (int)(p1.x/m_dimTile) + m_nbTiles2;
	y = (int)(p1.z/m_dimTile) + m_nbTiles2;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return;

	if ( p1.x == p2.x &&
		 p1.z == p2.z )  // sélectionne une seule case ?
	{
		GridSelectPos(p1, type);

		height = m_gridTable[x+y*m_nbTiles].height;
		height += m_gridHeight;
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		dim.x = m_dimTile*0.5f;
		dim.y = dim.x;

		if ( m_level == LEVEL_OBJECT )  // objets ?
		{
			if ( m_tool[m_level] == TOOL_DELETE )
			{
				dim.x *= 1.2f;
				dim.y *= 1.2f;
			}
		}

		for ( i=0 ; i<m_gridSelectTotal ; i++ )
		{
			pos = m_gridSelectPos[i];
			pos.y = height;
			m_gridSelectParti[i] = m_particule->CreateParticule(pos, speed, dim, m_gridSelectType[i]);
		}
	}
	else	// sélectionne un rectangle ?
	{
		pos.x = (p1.x+p2.x)/2.0f;
		pos.z = (p1.z+p2.z)/2.0f;
		pos.y = m_gridTable[x+y*m_nbTiles].height;
		pos.y += m_gridHeight;

		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);

		dim.x = (Abs(p1.x-p2.x)/8.0f+1.0f)*m_dimTile*0.5f;
		dim.y = (Abs(p1.z-p2.z)/8.0f+1.0f)*m_dimTile*0.5f;

		m_gridSelectPos[0] = pos;
		m_gridSelectType[0] = PARTIGRIDs;
		m_gridSelectParti[0] = m_particule->CreateParticule(pos, speed, dim, m_gridSelectType[0]);
		m_gridSelectTotal = 1;
	}
}

// Détecte où est la souris sur la grille du terrain.

BOOL CMainEdit::GridDetect(FPOINT mouse, D3DVECTOR &pos)
{
	D3DVERTEX2	vertex[3];
	D3DVECTOR	center;
	float		min, dist;
	int			x, y, objRank;

	objRank = m_terrain->RetObjRank();

	min = 1000000.0f;
	for ( y=0 ; y<m_nbTiles ; y++ )
	{
		for ( x=0 ; x<m_nbTiles ; x++ )
		{
			center.x = m_dimTile*(x-m_nbTiles2);
			center.z = m_dimTile*(y-m_nbTiles2);
			center.y = m_gridTable[x+y*m_nbTiles].height;

			vertex[0].x = center.x-m_dimTile/2.0f;
			vertex[0].z = center.z-m_dimTile/2.0f;
			vertex[1].x = center.x-m_dimTile/2.0f;
			vertex[1].z = center.z+m_dimTile/2.0f;
			vertex[2].x = center.x+m_dimTile/2.0f;
			vertex[2].z = center.z-m_dimTile/2.0f;
			vertex[0].y = center.y;
			vertex[1].y = center.y;
			vertex[2].y = center.y;

			if ( m_engine->DetectTriangle(mouse, vertex, objRank, dist) &&
				 dist < min )
			{
				min = dist;
				pos = center;
			}

			vertex[0].x = center.x-m_dimTile/2.0f;
			vertex[0].z = center.z+m_dimTile/2.0f;
			vertex[1].x = center.x+m_dimTile/2.0f;
			vertex[1].z = center.z-m_dimTile/2.0f;
			vertex[2].x = center.x+m_dimTile/2.0f;
			vertex[2].z = center.z+m_dimTile/2.0f;

			if ( m_engine->DetectTriangle(mouse, vertex, objRank, dist) &&
				 dist < min )
			{
				min = dist;
				pos = center;
			}
		}
	}

	return (min < 1000000.0f);
}


// Cherche un lift à une position donnée.

CObject* CMainEdit::SearchLift(D3DVECTOR center)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type != OBJECT_LIFT )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= 4.0f && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Cherche une plante tombante à une position donnée.

CObject* CMainEdit::SearchPlant5(D3DVECTOR center)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type < OBJECT_PLANT5 ||
			 type > OBJECT_PLANT9 )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= 4.0f && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Cherche un objet à une position donnée.
// Accepte un colocataire au même endroit (qui ne sera donc
// pas détecté).

CObject* CMainEdit::SearchObject(D3DVECTOR center, int level,
								 ObjectType newLoc)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();

		if ( type == OBJECT_COLUMN1 ||
			 type == OBJECT_COLUMN2 ||
			 type == OBJECT_COLUMN3 ||
			 type == OBJECT_COLUMN4 ||
			 type == OBJECT_LIFT    )  continue;

		if ( type >= OBJECT_PLANT5 &&
			 type <= OBJECT_PLANT9 )  continue;

		if ( level == LEVEL_OBJECT )  // objets ?
		{
			if ( (type >= OBJECT_GROUND0  &&
				  type <= OBJECT_GROUND19 ) ||
				 type == OBJECT_BARRIER29   ||
				 type == OBJECT_BARRIER39   ||
				 type == OBJECT_BARRIER48   ||
				 type == OBJECT_BARRIER49   ||
				 type == OBJECT_BARRIER67   )  continue;
		}

		if ( level == LEVEL_GROUND )  // sol ?
		{
			if ( (type < OBJECT_GROUND0  ||
				  type > OBJECT_GROUND19 ) &&
				 type != OBJECT_BARRIER29  &&
				 type != OBJECT_BARRIER39  &&
				 type != OBJECT_BARRIER48  &&
				 type != OBJECT_BARRIER49  &&
				 type != OBJECT_BARRIER67  )  continue;
		}

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= 4.0f && dist < min &&
			 !IsColoc(type, newLoc) )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Détecte l'objet directement visé par la souris.

CObject* CMainEdit::DetectObject(FPOINT mouse)
{
	CObject*	pObj;
	ObjectType	type;
	int			objRank, i, j, rank;

//?	objRank = m_engine->DetectObject(mouse, FALSE, TRUE);
	objRank = m_engine->DetectObject(mouse, FALSE);
	if ( objRank == -1 )  return 0;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type == OBJECT_COLUMN1 ||
			 type == OBJECT_COLUMN2 ||
			 type == OBJECT_COLUMN3 ||
			 type == OBJECT_COLUMN4 ||
			 type == OBJECT_LIFT    )  continue;

		for ( j=0 ; j<OBJECTMAXPART ; j++ )
		{
			rank = pObj->RetObjectRank(j);
			if ( rank == -1 )  continue;
			if ( rank != objRank )  continue;
			return pObj;
		}
	}
	return 0;
}

// Détecte l'objet visé par la souris, ou l'objet posé sur la
// tuile visée.

BOOL CMainEdit::DetectObject(FPOINT mouse, CObject *&pObj, D3DVECTOR &pos)
{
	pObj = DetectObject(mouse);
	if ( pObj != 0 )
	{
		pos = pObj->RetPosition(0);
		pos = Grid(pos, 8.0f);

		if ( !GridCheckPos(pos) )
		{
			pos = D3DVECTOR(NAN, NAN, NAN);
			return FALSE;
		}
		return TRUE;
	}

	if ( !GridDetect(mouse, pos) )
	{
		pos = D3DVECTOR(NAN, NAN, NAN);
		return FALSE;
	}

	if ( !GridCheckPos(pos) )
	{
		pos = D3DVECTOR(NAN, NAN, NAN);
		return FALSE;
	}

	pObj = SearchObject(pos, m_level);
	return TRUE;
}


// Vérifie si le puzzle est plausible.

int CMainEdit::CheckPuzzle()
{
	CObject*	pObj;
	ObjectType	type;
	int			i, nbBlupi, nbCrazy, nbGoal;

	nbBlupi = 0;
	nbCrazy = 0;
	nbGoal  = 0;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();

		if ( type == OBJECT_BLUPI )
		{
			nbBlupi ++;
		}

		if ( type == OBJECT_CRAZY )
		{
			nbCrazy ++;
		}

		if ( type == OBJECT_GOAL )
		{
			nbGoal ++;
		}
	}

	if ( nbBlupi == 0 )  return ERR_PUZZLE_ZEROBLUPI;
	if ( nbGoal == 0 )  return ERR_PUZZLE_ZEROGOAL;
	if ( nbGoal < nbBlupi+nbCrazy )  return ERR_PUZZLE_LOGOAL;
//?	if ( nbGoal > nbBlupi+nbCrazy )  return ERR_PUZZLE_HIGOAL;

	return 0;
}

// Retourne le groupe d'un objet.

ObjectType RetGroup(ObjectType type)
{
	if ( type == OBJECT_BLUPI ||
		 type == OBJECT_CRAZY )  return OBJECT_BLUPI;

	if ( type >= OBJECT_BOX1  &&
		 type <= OBJECT_BOX20 )  return OBJECT_BOX1;

	if ( type >= OBJECT_KEY1 &&
		 type <= OBJECT_KEY5 )  return OBJECT_KEY1;

	if ( type >= OBJECT_DOOR1 &&
		 type <= OBJECT_DOOR5 )  return OBJECT_DOOR1;

	if ( type >= OBJECT_TREE0 &&
		 type <= OBJECT_TREE9 )  return OBJECT_PLANT0;

	if ( type >= OBJECT_PLANT0  &&
		 type <= OBJECT_PLANT19 )  return OBJECT_PLANT0;

	if ( type >= OBJECT_BARRIER0  &&
		 type <= OBJECT_BARRIER99 )  return OBJECT_BARRIER0;

	return type;
}

// Vérifie s'il est possible de créer un nouvel objet.

int CMainEdit::CheckCreate(D3DVECTOR pos, ObjectType type)
{
	CObject*	pObj;
	ObjectType	oType;
	int			max, nb, i;

	pObj = SearchObject(pos, m_level, type);
	if ( pObj != 0 && RetFamily(pObj->RetType()) == type )  return 0;

	type = RetGroup(type);
	max = 50;
	if ( type == OBJECT_BLUPI    )  max =   4;
	if ( type == OBJECT_GOAL     )  max =   8;
	if ( type == OBJECT_BOX1     )  max =  30;
	if ( type == OBJECT_KEY1     )  max =   4;
	if ( type == OBJECT_DOOR1    )  max =   4;
	if ( type == OBJECT_DOCK     )  max =   1;
	if ( type == OBJECT_TRAX     )  max =   5;
	if ( type == OBJECT_PERFO    )  max =   5;
	if ( type == OBJECT_GUN      )  max =   5;
	if ( type == OBJECT_MINE     )  max =  20;
	if ( type == OBJECT_FIOLE    )  max =  10;
	if ( type == OBJECT_GLU      )  max =  10;
	if ( type == OBJECT_CATAPULT )  max =   2;
	if ( type == OBJECT_PLANT0   )  max = 100;

	nb = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		oType = RetGroup(pObj->RetType());
		if ( oType == type )  nb ++;
	}

	if ( nb < max )  return 0;

	if ( type == OBJECT_BLUPI )  return ERR_PUZZLE_MAXBLUPI;
	if ( type == OBJECT_GOAL  )  return ERR_PUZZLE_MAXGOAL;

	return ERR_PUZZLE_MAX;
}

