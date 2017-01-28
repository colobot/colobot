// model.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "water.h"
#include "robotmain.h"
#include "interface.h"
#include "edit.h"
#include "button.h"
#include "cmdtoken.h"
#include "modfile.h"
#include "model.h"



#define MAX_COLORS		9

static float table_color[MAX_COLORS*3] =
{
	1.0f,	1.0f,	1.0f,	// blanc
	1.0f,	0.0f,	0.0f,	// rouge
	0.0f,	1.0f,	0.0f,	// vert
	0.0f,	0.6f,	1.0f,	// bleu
	1.0f,	1.0f,	0.0f,	// jaune
	0.0f,	1.0f,	1.0f,	// cyan
	1.0f,	0.0f,	1.0f,	// magenta
	0.3f,	0.3f,	0.3f,	// gris
	0.0f,	0.0f,	0.0f,	// noir
};


#define MAX_STATES		13

static int table_state[MAX_STATES] =
{
	D3DSTATENORMAL,
	D3DSTATEPART1,  // #1
	D3DSTATEPART2,  // #2
	D3DSTATEPART3,
	D3DSTATEPART4,
	D3DSTATE2FACE,	// #5
	D3DSTATEACTION,	// #6 (bouton, flèche, etc.)
	D3DSTATETTw,
	D3DSTATETTb,
	D3DSTATETTw|D3DSTATE2FACE,  // #9
	D3DSTATETTb|D3DSTATE2FACE,  // #10 (vitres)
	D3DSTATETTb|D3DSTATE2FACE|D3DSTATEWRAP,  // #10 (vitres blanches)
	D3DSTATETTw|D3DSTATE2FACE|D3DSTATEWRAP,  // #11 (vitres noires)
};


#define MAX_NAMES		28




// Constructeur de l'objet.

CModel::CModel(CInstanceManager* iMan)
{
	m_iMan = iMan;

	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);

	m_modFile = new CModFile(m_iMan);
	m_triangleTable = m_modFile->RetTriangleList();

	m_textureRank = 0;
	strcpy(m_textureName, "box.tga");
	m_color = 0;
	m_state = 0;
	m_textureMode = 0;
	m_textureRotate = 0;
	m_bTextureMirrorX = FALSE;
	m_bTextureMirrorY = FALSE;
	m_texturePart = 0;
	m_textureAngle = D3DVECTOR(0.0f, 0.0f, 0.0f);
	TexturePartUpdate();

	m_bDisplayTransparent = FALSE;
	m_bDisplayOnlySelection = FALSE;
	InitView();

	m_triangleSel1 = 0;
	m_triangleSel2 = 0;

	m_mode = 1;
	m_oper = 'P';

	m_secondTexNum = 0;
	m_secondSubdiv = 7;
	m_secondOffsetU = 0;
	m_secondOffsetV = 0;

	m_modifPos  = 1.0f;
	m_modifRot  = 22.5f*PI/180.0f;
	m_modifZoom = 2.0f;

	m_min = 0.0f;
	m_max = 1000000.0f;
}

// Destructeur de l'objet.

CModel::~CModel()
{
	delete m_modFile;
}


// Il faut appeler cette procédure avant de modifier interactivement
// le modèle.

void CModel::StartUserAction()
{
	Event		event;
	FPOINT		pos, dim;
	CButton*	pb;

	dim.x = 105.0f/640.0f;
	dim.y =  18.0f/480.0f;
	pos.x =  10.0f/640.0f;
	pos.y = 450.0f/480.0f;
	m_interface->CreateEdit(pos, dim, 0, EVENT_EDIT1);

	dim.x =  50.0f/640.0f;
	pos.x = 125.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON1);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("Load");
	pos.x = 185.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON2);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("Script");
	pos.x = 245.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON3);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("Read");
	pos.x = 305.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON4);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("Add");
	pos.x = 365.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON5);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("Write");

	dim.x =  50.0f/640.0f;
	dim.y =  18.0f/480.0f;
	pos.x =  10.0f/640.0f;
	pos.y = 425.0f/480.0f;
	m_interface->CreateEdit(pos, dim, 0, EVENT_EDIT2);
	pos.x =  65.0f/640.0f;
	pos.y = 425.0f/480.0f;
	m_interface->CreateEdit(pos, dim, 0, EVENT_EDIT3);
	pos.x =  10.0f/640.0f;
	pos.y = 400.0f/480.0f;
	m_interface->CreateEdit(pos, dim, 0, EVENT_EDIT4);
	pos.x =  65.0f/640.0f;
	pos.y = 400.0f/480.0f;
	m_interface->CreateEdit(pos, dim, 0, EVENT_EDIT5);

	dim.x =  20.0f/640.0f;
	dim.y =  20.0f/480.0f;
	pos.y = 370.0f/480.0f;
	pos.x =  10.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON10);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("P");
	pos.x =  30.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON11);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("R");
	pos.x =  50.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON12);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("Z");
	pos.y = 350.0f/480.0f;
	pos.x =  10.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON13);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("+X");
	pos.x =  30.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON14);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("+Y");
	pos.x =  50.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON15);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("+Z");
	pos.y = 330.0f/480.0f;
	pos.x =  10.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON16);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("-X");
	pos.x =  30.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON17);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("-Y");
	pos.x =  50.0f/640.0f;
	pb = m_interface->CreateButton(pos, dim, 0, EVENT_BUTTON18);
	pb->SetState(STATE_SIMPLY);
	pb->SetName("-Z");

	dim.x =  80.0f/640.0f;
	dim.y =  18.0f/480.0f;
	pos.x =  10.0f/640.0f;
	pos.y = 300.0f/480.0f;
	m_interface->CreateEdit(pos, dim, 0, EVENT_EDIT6);

	PutModifValues();

//?	m_modFile->ReadModel("objects\\io.mod");
	DeselectAll();
	CurrentInit();

	ZeroMemory(&event, sizeof(Event));
	EventFrame(event);

	m_engine->LoadAllTexture();
	UpdateInfoText();
}

// Il faut appeler cette procédure après avoir modifié interactivement
// le modèle.

void CModel::StopUserAction()
{
	m_interface->DeleteControl(EVENT_EDIT1);
	m_interface->DeleteControl(EVENT_EDIT2);
	m_interface->DeleteControl(EVENT_EDIT3);
	m_interface->DeleteControl(EVENT_EDIT4);
	m_interface->DeleteControl(EVENT_EDIT5);
	m_interface->DeleteControl(EVENT_EDIT6);
	m_interface->DeleteControl(EVENT_BUTTON1);
	m_interface->DeleteControl(EVENT_BUTTON2);
	m_interface->DeleteControl(EVENT_BUTTON3);
	m_interface->DeleteControl(EVENT_BUTTON4);
	m_interface->DeleteControl(EVENT_BUTTON5);
	m_interface->DeleteControl(EVENT_BUTTON10);
	m_interface->DeleteControl(EVENT_BUTTON11);
	m_interface->DeleteControl(EVENT_BUTTON12);
	m_interface->DeleteControl(EVENT_BUTTON13);
	m_interface->DeleteControl(EVENT_BUTTON14);
	m_interface->DeleteControl(EVENT_BUTTON15);
	m_interface->DeleteControl(EVENT_BUTTON16);
	m_interface->DeleteControl(EVENT_BUTTON17);
	m_interface->DeleteControl(EVENT_BUTTON18);

	m_engine->SetInfoText(0, "");
	m_engine->SetInfoText(1, "");
}


// Met à jour les valeurs éditables pour le mapping des textures.

void CModel::PutTextureValues()
{
	CEdit*	pe;
	char	s[100];
	int		value;

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT2);
	if ( pe != 0 )
	{
		value = (int)(m_textureSup.x*256.0f+0.5f);
		sprintf(s, "%d", value);
		pe->SetText(s);
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT3);
	if ( pe != 0 )
	{
		value = (int)(m_textureSup.y*256.0f+0.5f);
		sprintf(s, "%d", value);
		pe->SetText(s);
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT4);
	if ( pe != 0 )
	{
		value = (int)(m_textureInf.x*256.0f-0.5f);
		sprintf(s, "%d", value);
		pe->SetText(s);
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT5);
	if ( pe != 0 )
	{
		value = (int)(m_textureInf.y*256.0f-0.5f);
		sprintf(s, "%d", value);
		pe->SetText(s);
	}
}

// Prend les valeurs éditables pour le mapping des textures.

void CModel::GetTextureValues()
{
	CEdit*	pe;
	char	s[100];
	int		value;

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT2);
	if ( pe != 0 )
	{
		pe->GetText(s, 100);
		sscanf(s, "%d", &value);
		m_textureSup.x = ((float)value-0.5f)/256.0f;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT3);
	if ( pe != 0 )
	{
		pe->GetText(s, 100);
		sscanf(s, "%d", &value);
		m_textureSup.y = ((float)value-0.5f)/256.0f;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT4);
	if ( pe != 0 )
	{
		pe->GetText(s, 100);
		sscanf(s, "%d", &value);
		m_textureInf.x = ((float)value+0.5f)/256.0f;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT5);
	if ( pe != 0 )
	{
		pe->GetText(s, 100);
		sscanf(s, "%d", &value);
		m_textureInf.y = ((float)value+0.5f)/256.0f;
	}
}


// Met à jour les valeurs éditables pour les modifications.

void CModel::PutModifValues()
{
	CEdit*	pe;
	char	s[100];
	float	value;

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT6);
	if ( pe != 0 )
	{
		if ( m_oper == 'P' )
		{
			value = m_modifPos;
			sprintf(s, "%.2f", value);
		}
		if ( m_oper == 'R' )
		{
			value = m_modifRot*180.0f/PI;
			sprintf(s, "%.2f", value);
		}
		if ( m_oper == 'Z' )
		{
			value = m_modifZoom*100.0f;
			sprintf(s, "%.2f", value);
		}
		pe->SetText(s);
	}
}

// Prend les valeurs éditables pour les modifications.

void CModel::GetModifValues()
{
	CEdit*	pe;
	char	s[100];
	float	value;

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT6);
	if ( pe != 0 )
	{
		pe->GetText(s, 100);

		if ( m_oper == 'P' )
		{
			sscanf(s, "%f", &value);
			m_modifPos = value;
		}
		if ( m_oper == 'R' )
		{
			sscanf(s, "%f", &value);
			m_modifRot = value*PI/180.0f;
		}
		if ( m_oper == 'Z' )
		{
			sscanf(s, "%f", &value);
			m_modifZoom = value/100.0f;
		}
	}
}


// Donne le nom du modèle.

void CModel::GetModelName(char *buffer)
{
	CEdit*	pe;
	char	s[100];

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT1);
	if ( pe == 0 )
	{
		strcpy(buffer, "objects\\io.mod");
	}
	else
	{
		pe->GetText(s, 100);
		if ( s[0] == 0 )
		{
			strcpy(buffer, "objects\\io.mod");
		}
		else
		{
			sprintf(buffer, "objects\\%s.mod", s);
		}
	}
}

// Donne le nom du modèle.

void CModel::GetDXFName(char *buffer)
{
	CEdit*	pe;
	char	s[100];

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT1);
	if ( pe == 0 )
	{
		strcpy(buffer, "models\\import.dxf");
	}
	else
	{
		pe->GetText(s, 100);
		sprintf(buffer, "models\\%s.dxf", s);
	}
}

// Donne le nom du modèle.

void CModel::GetScriptName(char *buffer)
{
	CEdit*	pe;
	char	s[100];

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT1);
	if ( pe == 0 )
	{
		strcpy(buffer, "objects\\script.txt");
	}
	else
	{
		pe->GetText(s, 100);
		sprintf(buffer, "objects\\%s.txt", s);
	}
}

// Indique si l'édition du nom a le focus.

BOOL CModel::IsEditFocus()
{
	CEdit*	pe;

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT1);
	if ( pe != 0 )
	{
		if ( pe->RetFocus() )  return TRUE;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT2);
	if ( pe != 0 )
	{
		if ( pe->RetFocus() )  return TRUE;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT3);
	if ( pe != 0 )
	{
		if ( pe->RetFocus() )  return TRUE;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT4);
	if ( pe != 0 )
	{
		if ( pe->RetFocus() )  return TRUE;
	}

	pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT5);
	if ( pe != 0 )
	{
		if ( pe->RetFocus() )  return TRUE;
	}

	return FALSE;
}


// Gestion d'un événement.

BOOL CModel::EventProcess(const Event &event)
{
	char	s[100];
	int		first, last, dir;

	switch( event.event )
	{
		case EVENT_FRAME:
			EventFrame(event);
			break;

		case EVENT_KEYDOWN:
			if ( IsEditFocus() )
				break;

			if ( event.param == '1' )
			{
				m_mode = 1;
				UpdateInfoText();
			}
			if ( event.param == '2' )
			{
				m_mode = 2;
				UpdateInfoText();
			}
			if ( event.param == '3' )
			{
				m_mode = 3;
				UpdateInfoText();
			}
			if ( event.param == VK_ADD )  // plus pavé num ?
			{
				if ( event.keyState & KS_SHIFT )  CurrentSelect(TRUE);
				CurrentSearchNext(+1, (event.keyState & KS_CONTROL));
			}
			if ( event.param == VK_SUBTRACT )  // moins pavé num ?
			{
				if ( event.keyState & KS_SHIFT )  CurrentSelect(TRUE);
				CurrentSearchNext(-1, (event.keyState & KS_CONTROL));
			}
			if ( event.param == VK_NUMPAD0 )
			{
				CurrentSelect(FALSE);
			}
			if ( event.param == VK_DECIMAL )
			{
				CurrentSelect(TRUE);
			}
			if ( event.param == VK_END )
			{
				DeselectAll();
			}
			if ( event.param == VK_INSERT )
			{
				SelectAll();
			}
			if ( event.param == VK_BACK )  // Delete normal ?
			{
				SelectDelete();
			}
			if ( event.param == VK_SPACE )
			{
				m_bDisplayTransparent = !m_bDisplayTransparent;
				m_bDisplayOnlySelection = FALSE;
			}
			if ( event.param == 'H' )
			{
				m_bDisplayOnlySelection = !m_bDisplayOnlySelection;
				m_bDisplayTransparent = FALSE;
			}
			if ( m_mode == 1 )
			{
				if ( event.param == 'S' )
				{
					SmoothSelect();
				}
				if ( event.param == 'N' )
				{
					PlaneSelect();
				}
				if ( event.param == 'C' )
				{
					ColorSelect();
				}
				if ( event.param == 'V' )
				{
					if ( event.keyState & KS_SHIFT )
					{
						m_color --;
						if ( m_color < 0 )  m_color = MAX_COLORS-1;
					}
					else
					{
						m_color ++;
						if ( m_color >= MAX_COLORS )  m_color = 0;
					}
					UpdateInfoText();
					ColorSelect();
				}
				if ( event.param == 'J' )
				{
					StateSelect();
				}
				if ( event.param == 'K' )
				{
					if ( event.keyState & KS_SHIFT )
					{
						m_state --;
						if ( m_state < 0 )  m_state = MAX_STATES-1;
					}
					else
					{
						m_state ++;
						if ( m_state >= MAX_STATES )  m_state = 0;
					}
					UpdateInfoText();
					StateSelect();
				}
				if ( event.param == 'M' )
				{
					if ( event.keyState & KS_SHIFT )
					{
						m_textureMode --;
						if ( m_textureMode < 0 )  m_textureMode = 28-1;
					}
					else
					{
						m_textureMode ++;
						if ( m_textureMode > 31 )  m_textureMode = 0;
					}
					UpdateInfoText();
					GetTextureValues();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'Z' )
				{
					m_textureRotate ++;
					if ( m_textureRotate > 2 )  m_textureRotate = 0;
					UpdateInfoText();
					GetTextureValues();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'X' )
				{
					m_bTextureMirrorX = !m_bTextureMirrorX;
					UpdateInfoText();
					GetTextureValues();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'Y' )
				{
					m_bTextureMirrorY = !m_bTextureMirrorY;
					UpdateInfoText();
					GetTextureValues();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'O' )
				{
					if ( event.keyState & KS_SHIFT )  dir = -1;
					else                              dir =  1;
					TextureRankChange(dir);
					UpdateInfoText();
				}
				if ( event.param == 'P' )
				{
					if ( event.keyState & KS_SHIFT )  dir = -1;
					else                              dir =  1;
					TexturePartChange(dir);
					UpdateInfoText();
					GetTextureValues();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'T' )
				{
					GetTextureValues();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'E' )
				{
					FPOINT	ti, ts;
					ti.x =  0.00f;
					ti.y =  0.00f;
					ts.x =  0.00f;
					ts.y =  0.00f;
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  ti, ts, "");
				}
				if ( event.param == 'D' )
				{
					if ( event.keyState & KS_SHIFT )
					{
						m_textureAngle.x -= m_modifRot*180.0f/PI;
						if ( m_textureAngle.x < 0.0f )  m_textureAngle.x += 360.0f;
					}
					else
					{
						m_textureAngle.x += m_modifRot*180.0f/PI;
						if ( m_textureAngle.x >= 360.0f )  m_textureAngle.x -= 360.0f;
					}
					UpdateInfoText();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'F' )
				{
					if ( event.keyState & KS_SHIFT )
					{
						m_textureAngle.y -= m_modifRot*180.0f/PI;
						if ( m_textureAngle.y < 0.0f )  m_textureAngle.y += 360.0f;
					}
					else
					{
						m_textureAngle.y += m_modifRot*180.0f/PI;
						if ( m_textureAngle.y >= 360.0f )  m_textureAngle.y -= 360.0f;
					}
					UpdateInfoText();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
				if ( event.param == 'G' )
				{
					if ( event.keyState & KS_SHIFT )
					{
						m_textureAngle.z -= m_modifRot*180.0f/PI;
						if ( m_textureAngle.z < 0.0f )  m_textureAngle.z += 360.0f;
					}
					else
					{
						m_textureAngle.z += m_modifRot*180.0f/PI;
						if ( m_textureAngle.z >= 360.0f )  m_textureAngle.z -= 360.0f;
					}
					UpdateInfoText();
					MappingSelect(m_textureMode, m_textureRotate,
								  m_bTextureMirrorX, m_bTextureMirrorY,
								  m_textureInf, m_textureSup, m_textureName);
				}
			}
			if ( m_mode == 2 )
			{
				if ( event.param == 'E' )
				{
					m_secondTexNum = 0;
					UpdateInfoText();
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
				}
				if ( event.param == 'O' )
				{
					m_secondTexNum ++;
					if ( m_secondTexNum > 15 )  m_secondTexNum = 1;
					UpdateInfoText();
				}
				if ( event.param == 'T' )
				{
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
					m_engine->LoadAllTexture();
				}
				if ( event.param == 'U' )
				{
					m_secondOffsetU += 45;
					if ( m_secondOffsetU >= 360 )  m_secondOffsetU = 0;
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
					UpdateInfoText();
				}
				if ( event.param == 'V' )
				{
					m_secondOffsetV += 45;
					if ( m_secondOffsetV >= 360 )  m_secondOffsetV = 0;
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
					UpdateInfoText();
				}
				if ( event.param == 'X' )
				{
					m_bTextureMirrorX = !m_bTextureMirrorX;
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
					UpdateInfoText();
				}
				if ( event.param == 'Y' )
				{
					m_bTextureMirrorY = !m_bTextureMirrorY;
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
					UpdateInfoText();
				}
				if ( event.param == 'S' )
				{
					m_secondSubdiv ++;
					if ( m_secondSubdiv > 7 )  m_secondSubdiv = 1;
					MappingSelect2(m_secondTexNum, m_secondSubdiv, m_secondOffsetU, m_secondOffsetV, m_bTextureMirrorX, m_bTextureMirrorY);
					UpdateInfoText();
				}
			}
			if ( m_mode == 3 )
			{
				if ( event.param == 'M' )
				{
					if ( m_min == 0.0f && m_max == 1000000.0f )
					{
						m_min = 0.0f;  m_max = 100.0f;
					}
					else if ( m_min == 0.0f && m_max == 100.0f )
					{
						m_min = 100.0f;  m_max = 200.0f;
					}
					else if ( m_min == 100.0f && m_max == 200.0f )
					{
						m_min = 200.0f;  m_max = 1000000.0f;
					}
					else if ( m_min == 200.0f && m_max == 1000000.0f )
					{
						m_min = 0.0f;  m_max = 1000000.0f;
					}
					UpdateInfoText();
				}
				if ( event.param == 'C' )
				{
					MinMaxChange();
				}
			}
			break;

		case EVENT_BUTTON1:  // import ?
			GetDXFName(s);
			m_modFile->ReadDXF(s, m_min, m_max);
			DeselectAll();
			CurrentInit();
			EventFrame(event);
			m_engine->LoadAllTexture();
			break;

		case EVENT_BUTTON2:  // script ?
			GetScriptName(s);
			ReadScript(s);
			DeselectAll();
			CurrentInit();
			EventFrame(event);
			m_engine->LoadAllTexture();
			break;

		case EVENT_BUTTON3:  // read ?
			GetModelName(s);
			m_modFile->ReadModel(s, TRUE, FALSE);  // lit avec frontières standard
			Adjust();
			DeselectAll();
			CurrentInit();
			EventFrame(event);
			m_engine->LoadAllTexture();
			break;

		case EVENT_BUTTON4:  // add ?
			GetModelName(s);
//?strcpy(s, "objects\\io.mod");
			first = m_modFile->RetTriangleUsed();
			m_modFile->AddModel(s, first, TRUE, FALSE);  // lit avec frontières standard
			last = m_modFile->RetTriangleUsed();
			SelectZone(first, last);
			EventFrame(event);
			break;

		case EVENT_BUTTON5:  // write ?
			GetModelName(s);
			DeselectAll();
			m_modFile->WriteModel(s);
			break;

		case EVENT_BUTTON10:  // pos ?
			GetModifValues();
			m_oper = 'P';
			PutModifValues();
			break;
		case EVENT_BUTTON11:  // rotate ?
			GetModifValues();
			m_oper = 'R';
			PutModifValues();
			break;
		case EVENT_BUTTON12:  // zoom ?
			GetModifValues();
			m_oper = 'Z';
			PutModifValues();
			break;

		case EVENT_BUTTON13:  // +X ?
			MoveSelect(D3DVECTOR(1.0f, 0.0f, 0.0f));
			break;
		case EVENT_BUTTON16:  // -X ?
			MoveSelect(D3DVECTOR(-1.0f, 0.0f, 0.0f));
			break;
		case EVENT_BUTTON14:  // +Y ?
			MoveSelect(D3DVECTOR(0.0f, 1.0f, 0.0f));
			break;
		case EVENT_BUTTON17:  // -Y ?
			MoveSelect(D3DVECTOR(0.0f, -1.0f, 0.0f));
			break;
		case EVENT_BUTTON15:  // +Z ?
			MoveSelect(D3DVECTOR(0.0f, 0.0f, 1.0f));
			break;
		case EVENT_BUTTON18:  // -Z ?
			MoveSelect(D3DVECTOR(0.0f, 0.0f, -1.0f));
			break;
	}

	return 0;
}


// Anime le modèle.

BOOL CModel::EventFrame(const Event &event)
{
	D3DMATERIAL7	matCurrent, matCurrenti, matCurrents, matTrans;
	D3DMATERIAL7*	pMat;
	D3DVERTEX2		vertex[3];
	char			texName2[20];
	int				i, used, objRank, state, texNum;

	m_time += event.rTime;

	m_engine->FlushObject();
	objRank = m_engine->CreateObject();

	ZeroMemory(&matCurrent, sizeof(D3DMATERIAL7));
	matCurrent.diffuse.r = 1.0f;
	matCurrent.diffuse.g = 0.0f;
	matCurrent.diffuse.b = 0.0f;  // rouge
	matCurrent.ambient.r = 0.5f;
	matCurrent.ambient.g = 0.5f;
	matCurrent.ambient.b = 0.5f;

	ZeroMemory(&matCurrents, sizeof(D3DMATERIAL7));
	matCurrents.diffuse.r = 1.0f;
	matCurrents.diffuse.g = 1.0f;
	matCurrents.diffuse.b = 0.0f;  // jaune
	matCurrents.ambient.r = 0.5f;
	matCurrents.ambient.g = 0.5f;
	matCurrents.ambient.b = 0.5f;

	ZeroMemory(&matCurrenti, sizeof(D3DMATERIAL7));
	matCurrenti.diffuse.r = 0.0f;
	matCurrenti.diffuse.g = 0.0f;
	matCurrenti.diffuse.b = 1.0f;  // bleu
	matCurrenti.ambient.r = 0.5f;
	matCurrenti.ambient.g = 0.5f;
	matCurrenti.ambient.b = 0.5f;

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( !m_triangleTable[i].bUsed )  continue;

		if ( m_triangleTable[i].min != m_min ||
			 m_triangleTable[i].max != m_max )  continue;

		pMat = &m_triangleTable[i].material;
//?		state = D3DSTATENORMAL;
		state = m_triangleTable[i].state;

		if ( strcmp(m_triangleTable[i].texName, "plant.tga") == 0 )
		{
			state |= D3DSTATEALPHA;
		}

		if ( i >= m_triangleSel1 &&
			 i <= m_triangleSel2 &&
			 (int)(m_time*10.0f)%2 == 0 )
		{
			pMat = &matCurrent;
		}
		else if ( m_triangleTable[i].bSelect &&
				  (int)(m_time*10.0f)%2 == 0 )
		{
			pMat = &matCurrents;
		}
		else
		{
			if ( m_bDisplayOnlySelection )  continue;
			if ( m_bDisplayTransparent )
			{
				matTrans = m_triangleTable[i].material;
				matTrans.diffuse.a = 0.1f;  // très transparent
				pMat = &matTrans;
				state = D3DSTATETD;
			}
		}

		if ( m_triangleTable[i].texNum2 == 0 )
		{
			m_engine->AddTriangle(objRank, &m_triangleTable[i].p1, 3,
								  *pMat, state,
								  m_triangleTable[i].texName, "",
								  0.0f, 1000000.0f, FALSE);
		}
		else
		{
			texNum = m_triangleTable[i].texNum2;
			if ( texNum >= 1 && texNum <= 10 )
			{
				state |= D3DSTATEDUALb;
			}
			else
			{
				state |= D3DSTATEDUALw;
			}
			sprintf(texName2, "dirty%.2d.tga", texNum);
			m_engine->AddTriangle(objRank, &m_triangleTable[i].p1, 3,
								  *pMat, state,
								  m_triangleTable[i].texName, texName2,
								  0.0f, 1000000.0f, FALSE);
		}

		if ( m_bDisplayTransparent &&  // dessine l'intérieur ?
			 i >= m_triangleSel1   &&
			 i <= m_triangleSel2   )
		{
			vertex[0] = m_triangleTable[i].p3;
			vertex[1] = m_triangleTable[i].p2;
			vertex[2] = m_triangleTable[i].p1;

			m_engine->AddTriangle(objRank, vertex, 3,
								  matCurrenti, D3DSTATENORMAL,
								  m_triangleTable[i].texName, "",
								  0.0f, 1000000.0f, FALSE);
		}
	}

	return TRUE;
}


// Donne un vertex.

BOOL CModel::GetVertex(int rank, D3DVERTEX2 &vertex)
{
	if ( rank < 0 || rank/3 >= m_modFile->RetTriangleUsed() )  return FALSE;
	if ( !m_triangleTable[rank/3].bUsed )  return FALSE;

	if ( !m_triangleTable[rank/3].bSelect )  return FALSE;

	if ( rank%3 == 0 )
	{
		vertex = m_triangleTable[rank/3].p1;
		return TRUE;
	}
	if ( rank%3 == 1 )
	{
		vertex = m_triangleTable[rank/3].p2;
		return TRUE;
	}
	if ( rank%3 == 2 )
	{
		vertex = m_triangleTable[rank/3].p3;
		return TRUE;
	}
	return FALSE;
}

// Modifie un vertex.

BOOL CModel::SetVertex(int rank, D3DVERTEX2 &vertex)
{
	if ( rank < 0 || rank/3 >= m_modFile->RetTriangleUsed() )  return FALSE;
	if ( !m_triangleTable[rank/3].bUsed )  return FALSE;

	if ( !m_triangleTable[rank/3].bSelect )  return FALSE;

	if ( rank%3 == 0 )
	{
		m_triangleTable[rank/3].p1 = vertex;
		return TRUE;
	}
	if ( rank%3 == 1 )
	{
		m_triangleTable[rank/3].p2 = vertex;
		return TRUE;
	}
	if ( rank%3 == 2 )
	{
		m_triangleTable[rank/3].p3 = vertex;
		return TRUE;
	}
	return FALSE;
}

// Adouci les normales des triangles sélectionés.

void CModel::SmoothSelect()
{
	char*		bDone;
	int			index[100];
	int			used, i, j, k, rank, nb;
	D3DVERTEX2	vi, vj, vk;
	D3DVECTOR	sum;

	PlaneSelect();  // réinit les normales

	used = m_modFile->RetTriangleUsed();

	bDone = (char*)malloc(used*3*sizeof(char));
	for ( i=0 ; i<used*3 ; i++ )
	{
		bDone[i] = FALSE;
	}

	for ( i=0 ; i<used*3 ; i++ )
	{
		bDone[i] = TRUE;
		rank = 0;
		index[rank++] = i;
		if ( !GetVertex(i, vi) )  continue;

		for ( j=0 ; j<used*3 ; j++ )
		{
			if ( bDone[j] )  continue;
			if ( !GetVertex(j, vj) )  continue;
			if ( Abs(vj.x-vi.x) < 0.001f &&
				 Abs(vj.y-vi.y) < 0.001f &&
				 Abs(vj.z-vi.z) < 0.001f )
			{
				bDone[j] = TRUE;
				index[rank++] = j;
				if ( rank >= 100 )  break;
			}
		}

		sum.x = 0.0f;
		sum.y = 0.0f;
		sum.z = 0.0f;
		for ( j=0 ; j<rank ; j++ )
		{
			GetVertex(index[j], vj);

			// Si un vecteur n apparaît plusieurs fois, diminue
			// son importance. Chaque vecteur différent doit être
			// compté une fois !
			nb = 0;
			for ( k=0 ; k<rank ; k++ )
			{
				GetVertex(index[k], vk);
				if ( Abs(vk.nx-vj.nx) < 0.001f &&
					 Abs(vk.ny-vj.ny) < 0.001f &&
					 Abs(vk.nz-vj.nz) < 0.001f )  nb ++;
			}

			sum.x += vj.nx/nb;
			sum.y += vj.ny/nb;
			sum.z += vj.nz/nb;
		}
		sum = Normalize(sum);

		for ( j=0 ; j<rank ; j++ )
		{
			GetVertex(index[j], vj);
			vj.nx = sum.x;
			vj.ny = sum.y;
			vj.nz = sum.z;
			SetVertex(index[j], vj);
		}
	}

	free(bDone);

	SelectTerm();
}


// Durci les normales des triangles sélectionés.

void CModel::PlaneSelect()
{
	D3DVECTOR	p1, p2, p3, n;
	int			used, i;

	used = m_modFile->RetTriangleUsed();

	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].bSelect )
		{
			p1.x = m_triangleTable[i].p1.x;
			p1.y = m_triangleTable[i].p1.y;
			p1.z = m_triangleTable[i].p1.z;

			p2.x = m_triangleTable[i].p2.x;
			p2.y = m_triangleTable[i].p2.y;
			p2.z = m_triangleTable[i].p2.z;

			p3.x = m_triangleTable[i].p3.x;
			p3.y = m_triangleTable[i].p3.y;
			p3.z = m_triangleTable[i].p3.z;

			n = ComputeNormal(p3, p2, p1);

			m_triangleTable[i].p1.nx = n.x;
			m_triangleTable[i].p1.ny = n.y;
			m_triangleTable[i].p1.nz = n.z;

			m_triangleTable[i].p2.nx = n.x;
			m_triangleTable[i].p2.ny = n.y;
			m_triangleTable[i].p2.nz = n.z;

			m_triangleTable[i].p3.nx = n.x;
			m_triangleTable[i].p3.ny = n.y;
			m_triangleTable[i].p3.nz = n.z;
		}
	}
	SelectTerm();
}


// Change la couleur des triangles sélectionés.

void CModel::ColorSelect()
{
	int			used, i;

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].bSelect )
		{
			m_triangleTable[i].material.diffuse.r = table_color[m_color*3+0];
			m_triangleTable[i].material.diffuse.g = table_color[m_color*3+1];
			m_triangleTable[i].material.diffuse.b = table_color[m_color*3+2];
		}
	}
	SelectTerm();
}

// Change l'état des triangles sélectionés.

void CModel::StateSelect()
{
	int			used, i;

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].bSelect )
		{
			m_triangleTable[i].state = table_state[m_state];
		}
	}
	SelectTerm();
}

// Déplace la sélection.

void CModel::MoveSelect(D3DVECTOR move)
{
	GetModifValues();

	if ( m_oper == 'P' )
	{
		move.x *= m_modifPos;
		move.y *= m_modifPos;
		move.z *= m_modifPos;
	}
	if ( m_oper == 'R' )
	{
		     if ( move.x == +1 )  move.x =  m_modifRot;
		else if ( move.x == -1 )  move.x = -m_modifRot;
		     if ( move.y == +1 )  move.y =  m_modifRot;
		else if ( move.y == -1 )  move.y = -m_modifRot;
		     if ( move.z == +1 )  move.z =  m_modifRot;
		else if ( move.z == -1 )  move.z = -m_modifRot;
	}
	if ( m_oper == 'Z' )
	{
		     if ( move.x == +1 )  move.x = m_modifZoom;
		else if ( move.x == -1 )  move.x = 1.0f/m_modifZoom;
		else                      move.x = 1.0f;
		     if ( move.y == +1 )  move.y = m_modifZoom;
		else if ( move.y == -1 )  move.y = 1.0f/m_modifZoom;
		else                      move.y = 1.0f;
		     if ( move.z == +1 )  move.z = m_modifZoom;
		else if ( move.z == -1 )  move.z = 1.0f/m_modifZoom;
		else                      move.z = 1.0f;
	}

	OperSelect(move, m_oper);
}

// Déplace la sélection.

void CModel::OperSelect(D3DVECTOR move, char oper)
{
	FPOINT		rot;
	int			used, i;

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].bSelect )
		{
			if ( oper == 'P' )
			{
				m_triangleTable[i].p1.x += move.x;
				m_triangleTable[i].p1.y += move.y;
				m_triangleTable[i].p1.z += move.z;
				m_triangleTable[i].p2.x += move.x;
				m_triangleTable[i].p2.y += move.y;
				m_triangleTable[i].p2.z += move.z;
				m_triangleTable[i].p3.x += move.x;
				m_triangleTable[i].p3.y += move.y;
				m_triangleTable[i].p3.z += move.z;
			}
			if ( oper == 'Z' )
			{
				m_triangleTable[i].p1.x *= move.x;
				m_triangleTable[i].p1.y *= move.y;
				m_triangleTable[i].p1.z *= move.z;
				m_triangleTable[i].p2.x *= move.x;
				m_triangleTable[i].p2.y *= move.y;
				m_triangleTable[i].p2.z *= move.z;
				m_triangleTable[i].p3.x *= move.x;
				m_triangleTable[i].p3.y *= move.y;
				m_triangleTable[i].p3.z *= move.z;
			}
			if ( oper == 'R' )
			{
				if ( move.x != 0.0f )
				{
					rot.x = m_triangleTable[i].p1.z;
					rot.y = m_triangleTable[i].p1.y;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.x, rot);
					m_triangleTable[i].p1.z = rot.x;
					m_triangleTable[i].p1.y = rot.y;

					rot.x = m_triangleTable[i].p2.z;
					rot.y = m_triangleTable[i].p2.y;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.x, rot);
					m_triangleTable[i].p2.z = rot.x;
					m_triangleTable[i].p2.y = rot.y;

					rot.x = m_triangleTable[i].p3.z;
					rot.y = m_triangleTable[i].p3.y;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.x, rot);
					m_triangleTable[i].p3.z = rot.x;
					m_triangleTable[i].p3.y = rot.y;
				}
				if ( move.y != 0.0f )
				{
					rot.x = m_triangleTable[i].p1.x;
					rot.y = m_triangleTable[i].p1.z;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.y, rot);
					m_triangleTable[i].p1.x = rot.x;
					m_triangleTable[i].p1.z = rot.y;

					rot.x = m_triangleTable[i].p2.x;
					rot.y = m_triangleTable[i].p2.z;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.y, rot);
					m_triangleTable[i].p2.x = rot.x;
					m_triangleTable[i].p2.z = rot.y;

					rot.x = m_triangleTable[i].p3.x;
					rot.y = m_triangleTable[i].p3.z;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.y, rot);
					m_triangleTable[i].p3.x = rot.x;
					m_triangleTable[i].p3.z = rot.y;
				}
				if ( move.z != 0.0f )
				{
					rot.x = m_triangleTable[i].p1.x;
					rot.y = m_triangleTable[i].p1.y;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.z, rot);
					m_triangleTable[i].p1.x = rot.x;
					m_triangleTable[i].p1.y = rot.y;

					rot.x = m_triangleTable[i].p2.x;
					rot.y = m_triangleTable[i].p2.y;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.z, rot);
					m_triangleTable[i].p2.x = rot.x;
					m_triangleTable[i].p2.y = rot.y;

					rot.x = m_triangleTable[i].p3.x;
					rot.y = m_triangleTable[i].p3.y;
					rot = RotatePoint(FPOINT(0.0f, 0.0f), move.z, rot);
					m_triangleTable[i].p3.x = rot.x;
					m_triangleTable[i].p3.y = rot.y;
				}
			}
		}
	}
	SelectTerm();
}

// Effectue un script de construction.

void CModel::ReadScript(char *filename)
{
	FILE*		file = NULL;
	char		line[200];
	char		name[200];
	char		buffer[200];
	int			i, first, last;
	D3DVECTOR	move;
	BOOL		bFirst = TRUE;

	file = fopen(filename, "r");
	if ( file == NULL )  return;

	while ( fgets(line, 200, file) != NULL )
	{
		for ( i=0 ; i<200 ; i++ )
		{
			if ( line[i] == '\t' )  line[i] = ' ';  // remplace tab par space
			if ( line[i] == '/' && line[i+1] == '/' )
			{
				line[i] = 0;
				break;
			}
		}

		if ( Cmd(line, "Object") )
		{
			OpString(line, "name", name);
			sprintf(buffer, "objects\\%s.mod", name);

			if ( bFirst )
			{
				m_modFile->ReadModel(buffer, TRUE, TRUE);
				last = m_modFile->RetTriangleUsed();
				SelectZone(0, last);
			}
			else
			{
				first = m_modFile->RetTriangleUsed();
				m_modFile->AddModel(buffer, first, TRUE, FALSE);
				last = m_modFile->RetTriangleUsed();
				SelectZone(first, last);
			}
			bFirst = FALSE;

			move = OpDir(line, "zoom");
			OperSelect(move, 'Z');

			move = OpDir(line, "rot");
			move *= PI/180.0f;  // degrés -> radians
			OperSelect(move, 'R');

			move = OpDir(line, "pos");
			OperSelect(move, 'P');
		}
	}

	fclose(file);
}



// Calcule la bbox des triangles sélectionnés.

void CModel::BBoxCompute(D3DVECTOR &min, D3DVECTOR &max)
{
	D3DVERTEX2	vertex;
	int			used, i;

	min.x =  1000000.0f;
	min.y =  1000000.0f;
	min.z =  1000000.0f;
	max.x = -1000000.0f;
	max.y = -1000000.0f;
	max.z = -1000000.0f;

	used = m_modFile->RetTriangleUsed();

	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		if ( vertex.x < min.x )  min.x = vertex.x;
		if ( vertex.y < min.y )  min.y = vertex.y;
		if ( vertex.z < min.z )  min.z = vertex.z;

		if ( vertex.x > max.x )  max.x = vertex.x;
		if ( vertex.y > max.y )  max.y = vertex.y;
		if ( vertex.z > max.z )  max.z = vertex.z;
	}
}

// Retourne le centre de gravité de la sélection.

D3DVECTOR CModel::RetSelectCDG()
{
	D3DVECTOR	min, max, cdg;

	BBoxCompute(min, max);

	cdg.x = (min.x+max.x)/2.0f;
	cdg.y = (min.y+max.y)/2.0f;
	cdg.z = (min.z+max.z)/2.0f;

	return cdg;
}

// Retourne le vecteur normal de la sélection.

D3DVECTOR CModel::RetSelectNormal()
{
	D3DVECTOR	p1, p2, p3, n;

	p1.x = m_triangleTable[m_triangleSel1].p1.nx;
	p1.y = m_triangleTable[m_triangleSel1].p1.ny;
	p1.z = m_triangleTable[m_triangleSel1].p1.nz;

	p2.x = m_triangleTable[m_triangleSel1].p2.nx;
	p2.y = m_triangleTable[m_triangleSel1].p2.ny;
	p2.z = m_triangleTable[m_triangleSel1].p2.nz;

	p3.x = m_triangleTable[m_triangleSel1].p3.nx;
	p3.y = m_triangleTable[m_triangleSel1].p3.ny;
	p3.z = m_triangleTable[m_triangleSel1].p3.nz;

	n = Normalize(p1+p2+p3);

	return n;
}

// Mappe une texture sur les triangles sélectionnés.

BOOL CModel::IsMappingSelectPlausible(D3DMaping D3Dmode)
{
	D3DVERTEX2	vertex[3];
	D3DVECTOR	min, max;
	FPOINT		a, b, ti, ts;
	float		au, bu, av, bv;
	int			used, i, j;

	ti.x = 0.0f;
	ti.y = 0.0f;
	ts.x = 1.0f;
	ts.y = 1.0f;

	BBoxCompute(min, max);

	if ( D3Dmode == D3DMAPPINGX )
	{
		a.x = min.z;
		a.y = min.y;
		b.x = max.z;
		b.y = max.y;
	}
	if ( D3Dmode == D3DMAPPINGY )
	{
		a.x = min.x;
		a.y = min.z;
		b.x = max.x;
		b.y = max.z;
	}
	if ( D3Dmode == D3DMAPPINGZ )
	{
		a.x = min.x;
		a.y = min.y;
		b.x = max.x;
		b.y = max.y;
	}

	au = (ts.x-ti.x)/(b.x-a.x);
	bu = ts.x-b.x*(ts.x-ti.x)/(b.x-a.x);

	av = (ts.y-ti.y)/(b.y-a.y);
	bv = ts.y-b.y*(ts.y-ti.y)/(b.y-a.y);

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( !GetVertex(i*3+0, vertex[0]) )  continue;
		if ( !GetVertex(i*3+1, vertex[1]) )  continue;
		if ( !GetVertex(i*3+2, vertex[2]) )  continue;

		for ( j=0 ; j<3 ; j++ )
		{
			if ( D3Dmode == D3DMAPPINGX )
			{
				vertex[j].tu = vertex[j].z*au+bu;
				vertex[j].tv = vertex[j].y*av+bv;
			}
			if ( D3Dmode == D3DMAPPINGY )
			{
				vertex[j].tu = vertex[j].x*au+bu;
				vertex[j].tv = vertex[j].z*av+bv;
			}
			if ( D3Dmode == D3DMAPPINGZ )
			{
				vertex[j].tu = vertex[j].x*au+bu;
				vertex[j].tv = vertex[j].y*av+bv;
			}
		}

		if ( vertex[0].tu == vertex[1].tu &&
			 vertex[0].tu == vertex[2].tu )  return FALSE;

		if ( vertex[0].tv == vertex[1].tv &&
			 vertex[0].tv == vertex[2].tv )  return FALSE;
	}

	return TRUE;
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelect(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
						   FPOINT ti, FPOINT ts, char *texName)
{
	D3DVECTOR	rot;

	rot = m_textureAngle*PI/180.0f;

	if ( rot.x != 0.0f )  OperSelect(D3DVECTOR(rot.x, 0.0f, 0.0f), 'R');
	if ( rot.y != 0.0f )  OperSelect(D3DVECTOR(0.0f, rot.y, 0.0f), 'R');
	if ( rot.z != 0.0f )  OperSelect(D3DVECTOR(0.0f, 0.0f, rot.z), 'R');

	MappingSelectBase(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);

	if ( rot.z != 0.0f )  OperSelect(D3DVECTOR(0.0f, 0.0f, -rot.z), 'R');
	if ( rot.y != 0.0f )  OperSelect(D3DVECTOR(0.0f, -rot.y, 0.0f), 'R');
	if ( rot.x != 0.0f )  OperSelect(D3DVECTOR(-rot.x, 0.0f, 0.0f), 'R');
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectBase(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
							   FPOINT ti, FPOINT ts, char *texName)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	min, max;
	FPOINT		a, b;
	D3DMaping	D3Dmode;
	float		au, bu, av, bv;
	int			used, i;
	BOOL		bPlausible[3];

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( !m_triangleTable[i].bUsed   )  continue;
		if ( !m_triangleTable[i].bSelect )  continue;

		strcpy(m_triangleTable[i].texName, texName);
	}

	if ( mode == 4 )
	{
		MappingSelectSpherical(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}
	if ( mode == 5 )
	{
		MappingSelectCylindrical(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}
	if ( mode == 6 )
	{
		MappingSelectFace(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}
	if ( mode >= 7 && mode <= 30 )
	{
		MappingSelectStretch(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}

	BBoxCompute(min, max);

	bPlausible[0] = IsMappingSelectPlausible(D3DMAPPINGX);
	bPlausible[1] = IsMappingSelectPlausible(D3DMAPPINGY);
	bPlausible[2] = IsMappingSelectPlausible(D3DMAPPINGZ);

	if ( mode == 0 )
	{
		for ( i=0 ; i<9 ; i++ )
		{
			if ( !bPlausible[i%3] )  continue;
			if ( rotate-- == 0 )  break;
		}
		if ( i%3 == 0 )  D3Dmode = D3DMAPPINGX;
		if ( i%3 == 1 )  D3Dmode = D3DMAPPINGY;
		if ( i%3 == 2 )  D3Dmode = D3DMAPPINGZ;
	}
	if ( mode == 1 )
	{
		D3Dmode = D3DMAPPINGX;
	}
	if ( mode == 2 )
	{
		D3Dmode = D3DMAPPINGY;
	}
	if ( mode == 3 )
	{
		D3Dmode = D3DMAPPINGZ;
	}

	if ( D3Dmode == D3DMAPPINGX )
	{
		a.x = min.z;
		a.y = min.y;
		b.x = max.z;
		b.y = max.y;
	}
	if ( D3Dmode == D3DMAPPINGY )
	{
		a.x = min.x;
		a.y = min.z;
		b.x = max.x;
		b.y = max.z;
	}
	if ( D3Dmode == D3DMAPPINGZ )
	{
		a.x = min.x;
		a.y = min.y;
		b.x = max.x;
		b.y = max.y;
	}

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	au = (ts.x-ti.x)/(b.x-a.x);
	bu = ts.x-b.x*(ts.x-ti.x)/(b.x-a.x);

	av = (ts.y-ti.y)/(b.y-a.y);
	bv = ts.y-b.y*(ts.y-ti.y)/(b.y-a.y);

	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		if ( D3Dmode == D3DMAPPINGX )
		{
			vertex.tu = vertex.z*au+bu;
			vertex.tv = vertex.y*av+bv;
		}
		if ( D3Dmode == D3DMAPPINGY )
		{
			vertex.tu = vertex.x*au+bu;
			vertex.tv = vertex.z*av+bv;
		}
		if ( D3Dmode == D3DMAPPINGZ )
		{
			vertex.tu = vertex.x*au+bu;
			vertex.tv = vertex.y*av+bv;
		}

		SetVertex(i, vertex);
	}

	SelectTerm();
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectSpherical(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
									FPOINT ti, FPOINT ts, char *texName)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	min, max, center, dim, p;
	float		radius, k, u, v;
	int			used, i;

	BBoxCompute(min, max);
	center = (min+max)/2.0f;
	dim    = (max-min)/2.0f;
	radius = Min(dim.x, dim.y, dim.z);

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		p.x = vertex.x-center.x;
		p.y = vertex.y-center.y;
		p.z = vertex.z-center.z;

		k = radius/Length(p);
		u = k*p.x;
		v = k*p.z;
		u = (u/dim.x*2.0f+1.0f)/2.0f;  // 0..1
		v = (v/dim.z*2.0f+1.0f)/2.0f;

		vertex.tu = ti.x+(ts.x-ti.x)*u;
		vertex.tv = ti.y+(ts.y-ti.y)*v;

		SetVertex(i, vertex);
	}

	SelectTerm();
}

// Cherche le centre d'un groupe de points.

D3DVECTOR CModel::RetMappingCenter(D3DVECTOR pos, D3DVECTOR min)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	center, p;
	int			used, i, nb;

	center.x = 0.0f;
	center.y = 0.0f;
	center.z = 0.0f;

	nb = 0;
	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		p.x = vertex.x;
		p.y = vertex.y;
		p.z = vertex.z;

		if ( Abs(p.x-pos.x) <= min.x &&
			 Abs(p.y-pos.y) <= min.y &&
			 Abs(p.z-pos.z) <= min.z )
		{
			center.x += p.x;
			center.y += p.y;
			center.z += p.z;
			nb ++;
		}
	}

	if ( nb == 0 )  return pos;

	center.x /= (float)nb;
	center.y /= (float)nb;
	center.z /= (float)nb;

	return center;
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectCylindrical(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
									  FPOINT ti, FPOINT ts, char *texName)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	min, max, center, local, dim, p, pp, box;
	float		radius, u, v;
	int			used, i;

	BBoxCompute(min, max);
	center = (min+max)/2.0f;
	dim    = (max-min)/2.0f;
	radius = Min(dim.x, dim.y, dim.z);

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	if ( rotate == 0 )
	{
		box.x =  2.0f;
		box.y = 10.0f;
		box.z = 10.0f;
	}
	if ( rotate == 1 )
	{
		box.x = 10.0f;
		box.y =  2.0f;
		box.z = 10.0f;
	}
	if ( rotate == 2 )
	{
		box.x = 10.0f;
		box.y = 10.0f;
		box.z =  2.0f;
	}

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		p.x = vertex.x;
		p.y = vertex.y;
		p.z = vertex.z;

#if 1
		p.x -= center.x;
		p.y -= center.y;
		p.z -= center.z;

		pp = p;
#else
		local = RetMappingCenter(p, box);

		pp = p;
		pp.x -= local.x;
		pp.y -= local.y;
		pp.z -= local.z;

		p.x -= center.x;
		p.y -= center.y;
		p.z -= center.z;
#endif

		if ( rotate == 0 )
		{
			u = RotateAngle(pp.y, pp.z);
			v = p.x/dim.x/2.0f + 0.5f;
		}
		if ( rotate == 1 )
		{
			u = RotateAngle(pp.x, pp.z);
			v = p.y/dim.y/2.0f + 0.5f;
		}
		if ( rotate == 2 )
		{
			u = RotateAngle(pp.x, pp.y);
			v = p.z/dim.z/2.0f + 0.5f;
		}

#if 0
		if ( u < PI )  u = u/PI;
		else           u = 2.0f-u/PI;
#endif
		u = u/(PI*2.0f);

		vertex.tu = ti.x+(ts.x-ti.x)*u;
		vertex.tv = ti.y+(ts.y-ti.y)*v;

		SetVertex(i, vertex);
	}

	SelectTerm();
}


// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectFace(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
							   FPOINT ti, FPOINT ts, char *texName)
{
	D3DVERTEX2	vertex[3];
	D3DVECTOR	min, max, center, local, dim, p;
	float		radius, u[3], v[3], m[3], avg;
	int			used, i, j;

	BBoxCompute(min, max);
	center = (min+max)/2.0f;
	dim    = (max-min)/2.0f;
	radius = Min(dim.x, dim.y, dim.z);

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		for ( j=0 ; j<3 ; j++ )
		{
			if ( !GetVertex(i*3+j, vertex[j]) )  continue;

			p.x = vertex[j].x - center.x;
			p.y = vertex[j].y - center.y;
			p.z = vertex[j].z - center.z;

#if 1
			u[j] = RotateAngle(p.x, p.z)/(PI*2.0f)+0.5f;
			if ( u[j] > 1.0f ) u[j] -= 1.0f;
#else
			u[j] = RotateAngle(p.x, p.z)/PI;
			if ( u[j] > 1.0f )  u[j] = 2.0f-u[j];
//?			if ( u[j] > 1.0f )  u[j] -= 1.0f;
#endif

			v[j] = p.y/dim.y/2.0f + 0.5f;

			if ( u[j] < 0.5f )  m[j] = u[j];
			else                m[j] = u[j]-1.0f;
		}

		avg = (m[0]+m[1]+m[2])/3.0f;

		for ( j=0 ; j<3 ; j++ )
		{
			if ( u[j] < 0.05f || u[j] > 0.95f )
			{
				if ( avg > 0.0f )  u[j] = 0.0f;
				else               u[j] = 1.0f;
			}

			vertex[j].tu = ti.x+(ts.x-ti.x)*u[j];
			vertex[j].tv = ti.y+(ts.y-ti.y)*v[j];

			SetVertex(i*3+j, vertex[j]);
		}
	}

	SelectTerm();
}

int StretchIndex(D3DVECTOR *list, int total, D3DVECTOR p)
{
	int		i;

	for ( i=0 ; i<total ; i++ )
	{
		if ( p.x == list[i].x &&
			 p.y == list[i].y &&
			 p.z == list[i].z )  return i;
	}
	return -1;
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectStretch(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
							   FPOINT ti, FPOINT ts, char *texName)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	p, list[1000];
	float		u, v;
	int			used, i, total, index;

	float		uv[4*2] =
	{
		0.0f,	0.0f,
		0.0f,	1.0f,
		1.0f,	0.0f,
		1.0f,	1.0f,
	};

	int			table[24*4] =
	{
		0,1,2,3,
		0,1,3,2,
		0,2,1,3,
		0,2,3,1,
		0,3,1,2,
		0,3,2,1,
		1,0,2,3,
		1,0,3,2,
		1,2,0,3,
		1,2,3,0,
		1,3,0,2,
		1,3,2,0,
		2,0,1,3,
		2,0,3,1,
		2,1,0,3,
		2,1,3,0,
		2,3,0,1,
		2,3,1,0,
		3,0,1,2,
		3,0,2,1,
		3,1,0,2,
		3,1,2,0,
		3,2,0,1,
		3,2,1,0,
	};

	mode = (mode-7)*4;

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	total = 0;
	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		p.x = vertex.x;
		p.y = vertex.y;
		p.z = vertex.z;

		index = StretchIndex(list, total, p);
		if ( index == -1 )
		{
			index = total;
			list[total++] = p;
		}

		u = uv[table[mode+index%4]*2+0];
		v = uv[table[mode+index%4]*2+1];

		vertex.tu = ti.x+(ts.x-ti.x)*u;
		vertex.tv = ti.y+(ts.y-ti.y)*v;

		SetVertex(i, vertex);
	}

	SelectTerm();
}


// Mappe une texture secondaire sur les triangles sélectionnés.

void CModel::MappingSelect2(int texNum2, int subdiv,
							int offsetU, int offsetV,
							BOOL bMirrorX, BOOL bMirrorY)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	min, max, center, p;
	float		u ,v;
	int			used, i;

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( !m_triangleTable[i].bUsed   )  continue;
		if ( !m_triangleTable[i].bSelect )  continue;

		m_triangleTable[i].texNum2 = texNum2;
	}

	if ( subdiv == 6 )
	{
		MappingSelectSpherical2(bMirrorX, bMirrorY);
		return;
	}
	if ( subdiv == 7 )
	{
		MappingSelectMagic2(bMirrorX, bMirrorY);
		return;
	}
	if ( subdiv > 2 )
	{
		MappingSelectPlane2(subdiv-3, bMirrorX, bMirrorY);
		return;
	}

	BBoxCompute(min, max);
	center = (min+max)/2.0f;

	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		p.x = vertex.x-center.x;
		p.y = vertex.y-center.y;
		p.z = vertex.z-center.z;

		u = RotateAngle(p.x, p.z);
		v = RotateAngle(Length(p.x, p.z), p.y);
		if ( p.x < 0.0f )  v += PI;

		u = NormAngle(u+(float)offsetU*PI/180.0f);
		v = NormAngle(v+(float)offsetV*PI/180.0f);

		if ( subdiv == 1 )
		{
			u = u/(PI*2.0f);
			v = v/(PI*2.0f);
		}
		if ( subdiv == 2 )
		{
			if ( u < PI )  u = u/PI;
			else           u = (PI*2.0f-u)/PI;
			if ( v < PI )  v = v/PI;
			else           v = (PI*2.0f-v)/PI;
		}

		vertex.tu2 = u;
		vertex.tv2 = v;

		SetVertex(i, vertex);
	}

	SelectTerm();
}

// Mappe une texture secondaire à plat.

void CModel::MappingSelectPlane2(int mode, BOOL bMirrorX, BOOL bMirrorY)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	min, max;
	FPOINT		ti, ts, a, b;
	float		au, bu, av, bv;
	int			used, i;

	ti = FPOINT(0.0f, 0.0f);
	ts = FPOINT(1.0f, 1.0f);

	BBoxCompute(min, max);

	if ( mode == 0 )
	{
		a.x = min.z;
		a.y = min.y;
		b.x = max.z;
		b.y = max.y;
	}
	if ( mode == 1 )
	{
		a.x = min.x;
		a.y = min.z;
		b.x = max.x;
		b.y = max.z;
	}
	if ( mode == 2 )
	{
		a.x = min.x;
		a.y = min.y;
		b.x = max.x;
		b.y = max.y;
	}

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	au = (ts.x-ti.x)/(b.x-a.x);
	bu = ts.x-b.x*(ts.x-ti.x)/(b.x-a.x);

	av = (ts.y-ti.y)/(b.y-a.y);
	bv = ts.y-b.y*(ts.y-ti.y)/(b.y-a.y);

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		if ( mode == 0 )
		{
			vertex.tu2 = vertex.z*au+bu;
			vertex.tv2 = vertex.y*av+bv;
		}
		if ( mode == 1 )
		{
			vertex.tu2 = vertex.x*au+bu;
			vertex.tv2 = vertex.z*av+bv;
		}
		if ( mode == 2 )
		{
			vertex.tu2 = vertex.x*au+bu;
			vertex.tv2 = vertex.y*av+bv;
		}

		SetVertex(i, vertex);
	}

	SelectTerm();
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectSpherical2(BOOL bMirrorX, BOOL bMirrorY)
{
	D3DVERTEX2	vertex;
	D3DVECTOR	min, max, center, dim, p;
	FPOINT		ti, ts;
	float		radius, k, u, v;
	int			used, i;

	BBoxCompute(min, max);
	center = (min+max)/2.0f;
	dim    = (max-min)/2.0f;
	radius = Min(dim.x, dim.y, dim.z);

	ti = FPOINT(0.0f, 0.0f);
	ts = FPOINT(1.0f, 1.0f);

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( !GetVertex(i, vertex) )  continue;

		p.x = vertex.x-center.x;
		p.y = vertex.y-center.y;
		p.z = vertex.z-center.z;

		k = radius/Length(p);
		u = k*p.x;
		v = k*p.z;
		u = (u/dim.x*2.0f+1.0f)/2.0f;  // 0..1
		v = (v/dim.z*2.0f+1.0f)/2.0f;

		vertex.tu2 = ti.x+(ts.x-ti.x)*u;
		vertex.tv2 = ti.y+(ts.y-ti.y)*v;

		SetVertex(i, vertex);
	}

	SelectTerm();
}

// Mappe une texture sur les triangles sélectionnés.

void CModel::MappingSelectMagic2(BOOL bMirrorX, BOOL bMirrorY)
{
	D3DVERTEX2	vertex, v[3];
	D3DVECTOR	min, max, au, bu, av, bv, n;
	FPOINT		ti, ts;
	int			used, i, mode;

	ti = FPOINT(0.0f, 0.0f);
	ts = FPOINT(1.0f, 1.0f);

	BBoxCompute(min, max);

	if ( bMirrorX )
	{
		Swap(ti.x, ts.x);
	}

	if ( !bMirrorY )  // test inversé !
	{
		Swap(ti.y, ts.y);
	}

	au.x = (ts.x-ti.x)/(max.x-min.x);
	bu.x = ts.x-max.x*(ts.x-ti.x)/(max.x-min.x);
	au.y = (ts.x-ti.x)/(max.y-min.y);
	bu.y = ts.x-max.y*(ts.x-ti.x)/(max.y-min.y);
	au.z = (ts.x-ti.x)/(max.z-min.z);
	bu.z = ts.x-max.z*(ts.x-ti.x)/(max.z-min.z);

	av.x = (ts.y-ti.y)/(max.x-min.x);
	bv.x = ts.y-max.x*(ts.y-ti.y)/(max.x-min.x);
	av.y = (ts.y-ti.y)/(max.y-min.y);
	bv.y = ts.y-max.y*(ts.y-ti.y)/(max.y-min.y);
	av.z = (ts.y-ti.y)/(max.z-min.z);
	bv.z = ts.y-max.z*(ts.y-ti.y)/(max.z-min.z);

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used*3 ; i++ )
	{
		if ( i%3 == 0 )
		{
			if ( !GetVertex(i+0, v[0]) )  continue;
			if ( !GetVertex(i+1, v[1]) )  continue;
			if ( !GetVertex(i+2, v[2]) )  continue;

			n = ComputeNormal(D3DVECTOR(v[0].x, v[0].y, v[0].z),
							  D3DVECTOR(v[1].x, v[1].y, v[1].z),
							  D3DVECTOR(v[2].x, v[2].y, v[2].z));

			n.x = Abs(n.x);
			n.y = Abs(n.y);
			n.z = Abs(n.z);

			if ( n.x >= Max(n.y, n.z) )  mode = 0;
			if ( n.y >= Max(n.x, n.z) )  mode = 1;
			if ( n.z >= Max(n.x, n.y) )  mode = 2;
		}

		if ( !GetVertex(i, vertex) )  continue;

		if ( mode == 0 )
		{
			vertex.tu2 = vertex.z*au.z+bu.z;
			vertex.tv2 = vertex.y*av.y+bv.y;
		}
		if ( mode == 1 )
		{
			vertex.tu2 = vertex.x*au.x+bu.x;
			vertex.tv2 = vertex.z*av.z+bv.z;
		}
		if ( mode == 2 )
		{
			vertex.tu2 = vertex.x*au.x+bu.x;
			vertex.tv2 = vertex.y*av.y+bv.y;
		}

		SetVertex(i, vertex);
	}

	SelectTerm();
}


// Cherche le triangle suivant.

int CModel::SearchNext(int rank, int step)
{
	int		max, i;

	max = m_modFile->RetTriangleUsed();

	for ( i=0 ; i<max ; i++ )
	{
		rank += step;
		if ( rank <  0   )  rank = max-1;
		if ( rank >= max )  rank = 0;

		if ( m_triangleTable[rank].min != m_min ||
			 m_triangleTable[rank].max != m_max )  continue;

		if ( m_triangleTable[rank].bUsed )  break;
	}
	return rank;
}

// Cherche tous les triangles faisant partie du même plan.

int CModel::SearchSamePlane(int first, int step)
{
	D3DVECTOR	vFirst[3], vNext[3];
	int			last, i;

	vFirst[0].x = m_triangleTable[first].p1.x;
	vFirst[0].y = m_triangleTable[first].p1.y;
	vFirst[0].z = m_triangleTable[first].p1.z;
	vFirst[1].x = m_triangleTable[first].p2.x;
	vFirst[1].y = m_triangleTable[first].p2.y;
	vFirst[1].z = m_triangleTable[first].p2.z;
	vFirst[2].x = m_triangleTable[first].p3.x;
	vFirst[2].y = m_triangleTable[first].p3.y;
	vFirst[2].z = m_triangleTable[first].p3.z;

	for ( i=0 ; i<1000 ; i++ )
	{
		last = first;
		first = SearchNext(first, step);

		vNext[0].x = m_triangleTable[first].p1.x;
		vNext[0].y = m_triangleTable[first].p1.y;
		vNext[0].z = m_triangleTable[first].p1.z;
		vNext[1].x = m_triangleTable[first].p2.x;
		vNext[1].y = m_triangleTable[first].p2.y;
		vNext[1].z = m_triangleTable[first].p2.z;
		vNext[2].x = m_triangleTable[first].p3.x;
		vNext[2].y = m_triangleTable[first].p3.y;
		vNext[2].z = m_triangleTable[first].p3.z;

		if ( !IsSamePlane(vFirst, vNext) )  // autre plan ?
		{
			return last;
		}
	}
	return first;
}

// Cherche le triangle suivant.

void CModel::CurrentSearchNext(int step, BOOL bControl)
{
	if ( step > 0 )  // en avant ?
	{
		m_triangleSel1 = SearchNext(m_triangleSel2, step);
		if ( bControl )
		{
			m_triangleSel2 = m_triangleSel1;
		}
		else
		{
			m_triangleSel2 = SearchSamePlane(m_triangleSel1, step);
		}
	}
	if ( step < 0 )  // en arrière ?
	{
		m_triangleSel2 = SearchNext(m_triangleSel1, step);
		if ( bControl )
		{
			m_triangleSel1 = m_triangleSel2;
		}
		else
		{
			m_triangleSel1 = SearchSamePlane(m_triangleSel2, step);
		}
	}

#if 0
	char s[100];
	sprintf(s, "(%.2f;%.2f;%.2f) (%.2f;%.2f;%.2f) (%.2f;%.2f;%.2f)",
			m_triangleTable[m_triangleSel1].p1.x,
			m_triangleTable[m_triangleSel1].p1.y,
			m_triangleTable[m_triangleSel1].p1.z,
			m_triangleTable[m_triangleSel1].p2.x,
			m_triangleTable[m_triangleSel1].p2.y,
			m_triangleTable[m_triangleSel1].p2.z,
			m_triangleTable[m_triangleSel1].p3.x,
			m_triangleTable[m_triangleSel1].p3.y,
			m_triangleTable[m_triangleSel1].p3.z);
	m_engine->SetInfoText(2, s);
	sprintf(s, "(%.2f;%.2f) (%.2f;%.2f) (%.2f;%.2f)",
			m_triangleTable[m_triangleSel1].p1.tu2,
			m_triangleTable[m_triangleSel1].p1.tv2,
			m_triangleTable[m_triangleSel1].p2.tu2,
			m_triangleTable[m_triangleSel1].p2.tv2,
			m_triangleTable[m_triangleSel1].p3.tu2,
			m_triangleTable[m_triangleSel1].p3.tv2);
	m_engine->SetInfoText(3, s);
#endif

	InitViewFromSelect();
	UpdateInfoText();
}

// Initialise les triangles courants initiaux.

void CModel::CurrentInit()
{
	m_triangleSel1 = 0;
	m_triangleSel2 = SearchSamePlane(m_triangleSel1, +1);

	InitViewFromSelect();
	UpdateInfoText();
}

// Sélectionne les triangles courants.

void CModel::CurrentSelect(BOOL bSelect)
{
	int		i;

	for ( i=m_triangleSel1 ; i<=m_triangleSel2 ; i++ )
	{
		m_triangleTable[i].bSelect = bSelect;
	}
}


// Désélectionne tous les triangles.

void CModel::DeselectAll()
{
	int		used, i;

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		m_triangleTable[i].bSelect = FALSE;
	}
}

// Sélectionne une zone.

void CModel::SelectZone(int first, int last)
{
	int		used, i;

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		m_triangleTable[i].bSelect = FALSE;
		if ( i >= first && i < last )
		{
			m_triangleTable[i].bSelect = TRUE;
		}
	}
	m_triangleSel1 = first;
	m_triangleSel2 = last-1;
}

// Sélectionne tous les triangles.

void CModel::SelectAll()
{
	int		used, i;

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].min == m_min &&
			 m_triangleTable[i].max == m_max )
		{
			m_triangleTable[i].bSelect = TRUE;
		}
	}
}

// Désélectionne tous les triangles.

void CModel::SelectTerm()
{
	int		used, i;

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( i >= m_triangleSel1 && i <= m_triangleSel2 )
		{
			if ( !m_triangleTable[i].bSelect )  return;
		}
		else
		{
			if ( m_triangleTable[i].bSelect )  return;
		}
	}

	DeselectAll();
}

// Sélectionne les triangles courants.

void CModel::DefaultSelect()
{
	int		used, i;

	used = m_modFile->RetTriangleUsed();
	for ( i=m_triangleSel1 ; i<=m_triangleSel2 ; i++ )
	{
		m_triangleTable[i].bSelect = TRUE;
	}
}



// Supprime tous les triangles sélectionnés.

void CModel::SelectDelete()
{
	int		used ,i;

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].bSelect )
		{
			m_triangleTable[i].bUsed = FALSE;
		}
	}

	i = m_triangleSel1;
	Compress();

	m_triangleSel1 = i;
	m_triangleSel2 = SearchSamePlane(m_triangleSel1, +1);
	InitViewFromSelect();
	UpdateInfoText();
}

// Supprime tous les triangles inutiles.

void CModel::Adjust()
{
	int		used ,i;

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].p1.x == m_triangleTable[i].p2.x &&
			 m_triangleTable[i].p1.y == m_triangleTable[i].p2.y &&
			 m_triangleTable[i].p1.z == m_triangleTable[i].p2.z &&
			 m_triangleTable[i].p1.x == m_triangleTable[i].p3.x &&
			 m_triangleTable[i].p1.y == m_triangleTable[i].p3.y &&
			 m_triangleTable[i].p1.z == m_triangleTable[i].p3.z )
		{
			m_triangleTable[i].bUsed = FALSE;
		}
	}
	Compress();
}

// Compresse tous les triangles.

void CModel::Compress()
{
	int		used, i, j;

	j = 0;
	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( m_triangleTable[i].bUsed )
		{
			m_triangleTable[j++] = m_triangleTable[i];
		}
	}
	m_modFile->SetTriangleUsed(j);
	CurrentInit();
}


// Change le min/max de tous les triangles sélectionnés.

void CModel::MinMaxChange()
{
	int		used, i;

	DefaultSelect();

	used = m_modFile->RetTriangleUsed();
	for ( i=0 ; i<used ; i++ )
	{
		if ( !m_triangleTable[i].bSelect )  continue;

		m_triangleTable[i].min = m_min;
		m_triangleTable[i].max = m_max;
	}
}


// Initialise le point de vue.

void CModel::InitView()
{
	m_viewHeight =  5.0f;
	m_viewDist   = 50.0f;
	m_viewAngleH =  0.0f;
	m_viewAngleV =  0.0f;
}

// Initialise le point de vue pour voir les triangles sélectionnés.

void CModel::InitViewFromSelect()
{
#if 0
	D3DVECTOR	n;
	float		h,v;

	n = RetSelectNormal();

	m_viewAngleH = RotateAngle(n.x, n.z)+PI;
	m_viewAngleV = RotateAngle(sqrtf(n.x*n.x+n.z*n.z), n.y)+PI;
	h = m_viewAngleH;
	v = m_viewAngleV;

	while ( m_viewAngleV <= -PI )
	{
		m_viewAngleV += PI;
		m_viewAngleH += PI;
	}
	while ( m_viewAngleV >=  PI )
	{
		m_viewAngleV -= PI;
		m_viewAngleH -= PI;
	}
	m_viewAngleV *= 0.75f;

	char s[100];
	sprintf(s, "angle=%f %f -> %f %f\n", h,v, m_viewAngleH, m_viewAngleV);
	OutputDebugString(s);
#endif
}

// Met à jour les paramètres pour le point de vue.

void CModel::UpdateView()
{
	D3DVECTOR	eye, lookat, vUpVec;

//?	lookat = RetSelectCDG();
	lookat = D3DVECTOR(0.0f, m_viewHeight, 0.0f);
	eye = RotateView(lookat, m_viewAngleH, m_viewAngleV, m_viewDist);

	vUpVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
	m_engine->SetViewParams(eye, lookat, vUpVec, 10.0f);
	m_engine->SetRankView(0);
}

// Déplace le point de vue.

void CModel::ViewMove(const Event &event, float speed)
{
	if ( IsEditFocus() )  return;

	// Up/Down.
	if ( event.axeY > 0.5f )
	{
		if ( event.keyState & KS_CONTROL )
		{
			m_viewHeight += event.rTime*10.0f*speed;
			if ( m_viewHeight > 100.0f )  m_viewHeight = 100.0f;
		}
		else
		{
			m_viewAngleV -= event.rTime*1.0f*speed;
			if ( m_viewAngleV < -PI*0.49f )  m_viewAngleV = -PI*0.49f;
		}
	}
	if ( event.axeY < -0.5f )
	{
		if ( event.keyState & KS_CONTROL )
		{
			m_viewHeight -= event.rTime*10.0f*speed;
			if ( m_viewHeight < -100.0f )  m_viewHeight = -100.0f;
		}
		else
		{
			m_viewAngleV += event.rTime*1.0f*speed;
			if ( m_viewAngleV > PI*0.49f )  m_viewAngleV = PI*0.49f;
		}
	}

	// Left/Right.
	if ( event.axeX < -0.5f )
	{
		m_viewAngleH -= event.rTime*1.0f*speed;
	}
	if ( event.axeX > 0.5f )
	{
		m_viewAngleH += event.rTime*1.0f*speed;
	}

	// PageUp/PageDown.
	if ( event.keyState & KS_PAGEUP )
	{
		m_viewDist -= event.rTime*30.0f*speed;
		if ( m_viewDist < 1.0f )  m_viewDist = 1.0f;
	}
	if ( event.keyState & KS_PAGEDOWN )
	{
		m_viewDist += event.rTime*30.0f*speed;
		if ( m_viewDist > 200.0f )  m_viewDist = 200.0f;
	}
}



// Met à jour le texte d'informations.

void CModel::UpdateInfoText()
{
	char		info[100];

	if ( m_mode == 1 )
	{
		sprintf(info, "[1] V:color=%d K:state=%d  DFG:angle=%d;%d;%d  Sel=%d..%d (T=%d)",
				m_color, m_state,
				(int)m_textureAngle.x,
				(int)m_textureAngle.y,
				(int)m_textureAngle.z,
				m_triangleSel1, m_triangleSel2,
				m_triangleSel2-m_triangleSel1+1);
		m_engine->SetInfoText(0, info);

		sprintf(info, "M:mode=%d Z:rot=%d XY:mir=%d;%d P:part=%d O:name=%s",
				m_textureMode, m_textureRotate,
				m_bTextureMirrorX, m_bTextureMirrorY,
				m_texturePart, m_textureName);
		m_engine->SetInfoText(1, info);
	}

	if ( m_mode == 2 )
	{
		sprintf(info, "[2]  Sel=%d..%d (T=%d)",
				m_triangleSel1, m_triangleSel2,
				m_triangleSel2-m_triangleSel1+1);
		m_engine->SetInfoText(0, info);

		sprintf(info, "O:dirty=%d UV:offset=%d;%d XY:mir=%d;%d S:subdiv=%d",
				m_secondTexNum,
				m_secondOffsetU, m_secondOffsetV,
				m_bTextureMirrorX, m_bTextureMirrorY,
				m_secondSubdiv);
		m_engine->SetInfoText(1, info);
	}

	if ( m_mode == 3 )
	{
		sprintf(info, "[3]  LOD Min/max=%d..%d  Sel=%d..%d (T=%d)",
				(int)m_min, (int)m_max,
				m_triangleSel1, m_triangleSel2,
				m_triangleSel2-m_triangleSel1+1);
		m_engine->SetInfoText(0, info);

		sprintf(info, "[Change]");
		m_engine->SetInfoText(1, info);
	}
}



static int tablePartW[] =	// dock.tga
{
	 24,   0,  32,   8,  // gris clair
	 24,   8,  32,  16,  // gris
	 32,   0,  40,   8,  // gris foncé
	 32,   8,  40,  16,  // orange
	  0,   0,  24, 192,  // jauge 1..8
	 64,   0, 256,  46,  // tableau boutons
	 64,  78, 256, 142,  // dessus complet
	 64,  84, 256, 117,  // dessus partiel
	 64, 142, 256, 182,  // côté
	 64, 182, 128, 221,  // face
	165,  46, 211,  78,  // piston
	 24,  16,  32, 192,  // pas de vis
	211,  46, 224,  78,  // rail sup.
	224,  46, 256,  78,  // butoir
	128, 182, 224, 214,  // charriot
	128, 214, 224, 246,  // charriot
	128, 220, 224, 226,  // charriot
	 42,   0,  64, 192,  // pilier
	  0, 192,  64, 256,  // béton
	 64, 221, 128, 256,  // béton
	 32,  16,  42,  24,  // rail
	 32,  24,  42,  32,  // rail
	-1
};

static int tablePartBlupi[] =	// blupi.tga
{
	  0, 120,   8, 128,  // jaune
	  8, 120,  16, 128,  // noir
	  0, 128,  64, 181,  // bouche heureux
	 64, 128, 128, 181,  // bouche triste
	128, 128, 192, 181,  // bouche fâché
	  0, 181,  64, 236,  // yeux ouverts
	 64, 181, 128, 236,  // yeux fermés
	  0,   0,  16,  64,  // bonnet
	 16,   0,  64,  36,  // casquette
	 80,  64, 112, 128,  // chapeau magicien
	 24, 120,  32, 128,  // visière
	112,  64, 144,  96,  // pluie
	144,  64, 160, 128,  // gandalf
	160,  64, 176, 128,  // gandalf
	 16, 120,  24, 128,  // lunettes 1
	 16,  36,  48,  58,  // verre 1
	 48,  36,  80,  58,  // verre 2
	  0,  64,  34,  98,  // verre 3
	192, 181, 256, 237,  // verre 4
	165,  51, 197,  59,  // verre 4
	197,  51, 205,  59,  // verre 4
	192, 253, 256, 256,  // verre 4
	 32, 120,  40, 128,  // bleu
	 34,  58,  80,  78,  // chaussures 1
	 34,  78,  80,  84,  // chaussures 1
	 34,  84,  80, 104,  // chaussures 2
	 34, 104,  80, 110,  // chaussures 2
	 34, 110,  80, 116,  // chaussures 3
	192,  96, 256, 132,  // chaussures 4
	192, 132, 256, 181,  // chaussures 4
	192, 237, 256, 253,  // chaussures 4
	 80,   0, 131,  64,  // sac 1
	131,   0, 164,  64,  // sac 1
	164,   0, 197,  51,  // sac 1
	197,   0, 205,  51,  // sac 1
	 64,   0,  80,  16,  // sac 1
	 64,  16,  80,  32,  // sac 1
	205,   0, 256,  61,  // sac 2
	176,  64, 180, 128,  // sac 3
	180,  64, 184, 128,  // sac 3
	184,  64, 188, 128,  // sac 3
	188,  64, 192, 128,  // sac 3
	192,  64, 224,  96,  // sac 3
	224,  64, 256,  96,  // sac 3
	-1
};

static int tablePartB1[] =	// bot1.tga
{
	170,  42, 178,  50,  // clair
	178,  42, 186,  50,  // foncé
	  0,   0,  58,  64,  // face torax 1
	  0, 186,  58, 250,  // face torax 2
	 58, 218,  92, 256,  // face torax 3
	 58,   0, 112,  64,  // profil torax
	112,   0, 170,  64,  // dos torax
	170,   0, 234,  42,  // visage
	214, 214, 256, 256,  // oeil face
	198, 214, 214, 256,  // oeil tour
	170,  50, 184,  64,  // articulation
	184,  50, 198,  64,  // oreille
	198,  50, 234,  64,  // attention
	242,   0, 251,  24,  // pied
	234,   0, 242,  64,  // ressort
	 58, 186, 154, 202,  // bras 1
	 58, 202, 154, 218,  // bras 2
	210,  42, 218,  50,  // orange
	242,  24, 250,  64,  // baton guide
						 // bot2:
	186,  42, 194,  50,  // foncé
	194,  42, 202,  50,  // orange
	112,  64, 176, 116,  // avant torax
	112, 116, 176, 168,  // arrière torax
	 48,  86, 112, 126,  // côté haut
	 48, 125, 112, 148,  // côté bas
	112, 168, 176, 186,  // côté pied
	  0, 129,  48, 158,  // face pied
	 54,  64, 101,  87,  // côté tête
	 48, 147, 112, 175,  // arrière tête
	  0,  64,  48, 130,  // articulation bras/pied
	  0, 158,  13, 175,  // petit
	176,  64, 256,  91,  // visage
	 48, 175, 112, 186,  // mitrailleuse
	 24, 158,  48, 186,  // arrière mitrailleuse
						 // bot3:
	176,  91, 184, 214,  // vert
	176,  91, 256, 214,  // face
	-1
};

static int tablePartB2[] =	// bot2.tga
{
	248,   0, 256,   8,  // gris clair
	248,   8, 256,  16,  // gris moyen
	248,  16, 256,  24,  // gris foncé
	 80,   0, 121,  50,  // tête: face
	121,   0, 151,  50,  // tête: oreille
	 80,   0,  84,  50,  // tête: arrière
	 80,  50, 144, 118,  // torax: avant
	144,  50, 208, 118,  // torax: arrière
	 89,  54, 136,  58,  // torax: dessus
	 80,  50,  84, 118,  // torax: côté
	232,  32, 256,  56,  // articulation
	156,  32, 176,  36,  // articulation
	156,  36, 172,  40,  // articulation
	152,  32, 156,  50,  // coup
	152,   0, 248,  16,  // membre 1
	152,   5, 248,  14,  // membre 1
	152,  16, 248,  32,  // membre 2
	152,  21, 248,  26,  // membre 2
	  0, 128,  89, 256,  // statue face
	 89, 128, 178, 256,  // statue dos
	-1
};

static int tablePartBox[] =	// box.tga
{
	  0,   0,  56,  56,  // caisse 1
	  0,  56,  56, 112,  // caisse 2
	  0, 246,   8, 254,  // champfrein 2
	  0, 112,  56, 168,  // caisse 3
	  0, 224,  22, 246,  // coin inf. 3
	 22, 224,  44, 246,  // coin sup. 3
	  0, 168,  56, 224,  // caisse 4
	 56, 112, 112, 168,  // caisse 5
	112, 112, 172, 176,  // caisse 6
	112, 176, 172, 240,  // caisse 6
	118, 112, 166, 118,  // caisse 5/6 vis
	112, 118, 118, 170,  // caisse 5/6 vis
	172, 214, 214, 256,  // sphère 7
	 56,   0, 112,  56,  // caisse 8 dessus (13)
	 56,  56, 112,  67,  // caisse 8 plat
	 56,  67, 112,  87,  // caisse 8 pierre 1
	 56,  87, 112, 107,  // caisse 8 pierre 2
	112,   0, 168,  20,  // caisse 8 pierre 3
	112,  20, 168,  40,  // caisse 8 pierre 4
	112,  40, 168,  60,  // caisse 8 frise
	 56, 168,  69, 224,  // caisse 8 colonne 1
	 63, 168,  78, 224,  // caisse 8 colonne 2
	  0,   0,  56,   8,  // caisse 9
	  0,   0,   8,   8,  // caisse 9
	  8,   0,  48,   8,  // caisse 9
	216, 214, 226, 256,  // boxa chevron (25)
	172,   0, 214,  10,  // boxb chevron
	236, 214, 246, 256,  // boxc chevron
	172,  10, 214,  20,  // boxd chevron
	184,  56, 203,  88,  // boxe chevron
	192,  20, 224,  39,  // boxf chevron
	221,  56, 238,  88,  // boxg chevron
	192,  39, 224,  56,  // boxh chevron
	172,  88, 256, 151,  // clé 1
	172, 151, 256, 214,  // clé 2
	112,  60, 168,  77,  // porte //
	 78, 169, 112, 202,  // porte générique
	 78, 202,  92, 216,  // clé orange
	 92, 202, 106, 216,  // clé bleu
	-1
};

static int tablePartBox2[] =	// box2.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // pneu
	 64,   0, 192,  72,  // box11
	 64,  72, 192, 144,  // box11
	 64, 144, 192, 224,  // box11
	  0,  96,   8, 104,  // chassis
	-1
};

static int tablePartO1[] =	// object1.tga
{
	 56,   0,  64,   8,  // colonne 1
	 56,   8,  64,  16,  // colonne 2
	128,  46, 136,  62,  // colonne 2
	 56,  16,  64,  24,  // colonne 3
	 56,  24,  64,  32,  // colonne 4
	 48,  24,  56,  32,  // colonne 4
	 32,  16,  40,  24,  // orange
	 32,  24,  40,  32,  // ficelle
	  0,   0,  32,  64,  // fiole
	 32, 192,  64, 256,  // glu
	 32,   0,  48,  16,  // bouchon
	  0, 128,  32, 192,  // ballon
	 32,  64,  64,  96,  // rouge reflet
	 48,   0,  56,  16,  // pointe rouge
	 64,   0, 128,  64,  // lift
	 32,  96,  64, 136,  // vitre
	 64,  64, 192,  96,  // oiseau: corps
	 64,  96,  81, 128,  // oiseau: tête
	134,   0, 192,  62,  // oiseau: aile 1
	192,   0, 256,  86,  // oiseau: aile 2
	128, 174, 256, 203,  // ptero: corps
	 82, 128, 118, 158,  // ptero: aile 1
	 68, 158, 122, 186,  // ptero: aile 2
	 68, 186, 128, 256,  // ptero: aile 3
	 81,  96, 131, 128,  // ptero: patte
	128,  96, 256, 174,  // poisson
	128, 203, 181, 256,  // serpent
	 40,  16,  48,  24,  // ground0: structure
	  0, 192,  32, 256,  // ground1: piston
	 32, 136,  64, 168,  // ground1: piston
	181, 203, 234, 256,  // sous-marin
	 40,  24,  48,  32,  // sous-marin
	 48,  16,  56,  24,  // sous-marin
	230, 166, 256, 174,  // sous-marin
	-1
};

static int tablePartO2[] =	// object2.tga
{
	  0,   0,  64, 128,  // porte dessus
	 56,   0,  64, 128,  // porte bord
	 64,   0, 128,  20,  // intérieur côté
	 64,  20, 128,  84,  // intérieur fond
	  0, 192, 128, 256,  // trésor
	  0, 192, 128, 205,  // trésor
	192,  64, 256, 117,  // trésor
	  0, 128,   8, 136,  // clown: peau
	  8, 128,  16, 136,  // clown: chapeau
	 16, 128,  24, 136,  // clown: habit
	  0, 136,  64, 179,  // clown: habit
	 64, 134,  72, 179,  // clown: tête
	 72, 134,  80, 179,  // clown: support
	 24, 128,  32, 136,  // désert
	128,   0, 256,  64,  // jet
	128,  64, 192, 128,  // jet
	224,   0,  32, 256,  // jet
	240,  32, 256,  40,  // jet
	232,  32, 240,  40,  // jet
	128, 192, 160, 208,  // fiole/glu/goal: ^ (19)
	128, 208, 160, 224,  // fiole/glu/goal: v
	128, 224, 144, 256,  // fiole/glu/goal: <
	160, 224, 176, 256,  // fiole/glu/goal: >
	192, 224, 224, 256,  // orange reflet
	 96, 128, 128, 144,  // dock: bouton 4 (24)
	 96, 144, 128, 160,  // dock: bouton 5
	160, 128, 197, 160,  // dock: bouton 6
	 96, 160, 112, 192,  // dock: bouton 7
	128, 160, 144, 192,  // dock: bouton 8
	-1
};

static int tablePartCi1[] =	// cinema1.tga
{
	  0,   0, 128, 128,  // bois
	  0,  32, 128,  49,  // planche 1
	  0,  70, 128,  89,  // planche 2
	  0,  89, 128, 109,  // planche 3
	  0, 109, 128, 128,  // planche 4
	  0, 128, 128, 160,  // toit
	128,   0, 256, 128,  // porte 1
	128, 128, 256, 256,  // porte 2
	-1
};

static int tablePartCi2[] =	// cinema2.tga
{
	  0,   0, 128, 128,  // fenêtre 1
	  0, 128, 128, 256,  // fenêtre 2
	128,   0, 256, 128,  // fenêtre 3
	128, 128, 256, 256,  // porte 3
	-1
};

static int tablePartTrax[] =	// trax.tga
{
	  0,   0,   8,   8,  // gris clair
	  8,   0,  16,   8,  // gris moyen
	 16,   0,  24,   8,  // gris foncé
	 24,   0,  32,   8,  // rouge
	 32,   0,  40,   8,  // gris très clair
	 64,   0, 160,  59,  // arrière
	 64,  59, 160, 129,  // côté
	128, 129, 256, 193,  // avant
	160, 193, 256, 253,  // dessus
	  0, 172,  64, 204,  // bouton avance
	  0, 204,  64, 254,  // bouton tourne
	  0, 129, 128, 172,  // chenille côté
	192,   0, 256,  32,  // chenille bandes
	192,   0, 200,  32,  // chenille bande
	160,  32, 224,  96,  // buttoir
	160,   0, 176,  32,  // buttoir
	160,  96, 256, 112,  // hachures 6x
	160, 112, 240, 128,  // hachures 5x
	  0,  90,  64,  97,  // canon ext.
	  0,  97,  64, 129,  // canon int.
	-1
};

static int tablePartCata[] =	// catapult.tga
{
	 96,   0, 104,   8,  // gris clair
	104,   0, 112,   8,  // gris moyen
	112,   0, 120,   8,  // gris gris
	  0,   0,  32,  32,  // bouton
	128,   0, 192,  64,  // engrenage 1
	192,   0, 256,  64,  // engrenage 2
	128,  64, 192, 128,  // marteau
	144,  64, 176, 128,  // marteau
	192,  64, 256, 128,  // marteau
	  0,  45,  64,  53,  // support lisse
	  0,  45,  64,  69,  // support vis
	  0, 248, 256, 256,  // grand axe
	  0,  69,  69, 138,  // sup avec axe
	  0, 138,  69, 207,  // carré 1
	 69,  69, 104, 138,  // triangle
	 69, 138, 138, 207,  // L
	 69, 207, 138, 225,  // panneau 1
	138, 128, 207, 176,  // panneau 2
	 69, 225, 138, 243,  // panneau 3
	138, 176, 207, 245,  // carré 2
	  0, 207,  69, 242,  // moteur
	207, 128, 256, 224,  // tableau 1
	112,  10, 128, 138,  // tableau 2
	-1
};

static int tablePartBar1[] =	// barrier1.tga
{
	  0,  24,   8,  32,  // clair
	  0,   0,   8,   8,  // moyen-clair
	  0,   8,   8,  16,  // moyen
	  0,  16,   8,  24,  // foncé
	  0,   0,  64, 128,  // barrier0
	 64,   0,  68, 128,  // barrier0
	  0, 158,  64, 182,  // barrier0
	  0, 182,  64, 246,  // barrier0
	  0, 128,  64, 158,  // barrier1
	-1
};

static int tablePartBar2[] =	// barrier2.tga
{
	  0,   0, 128,  87,  // tuiles 2x2
	  0,   0, 128,  44,  // tuiles 2x1
	  0,   0,  64,  87,  // tuiles 1x2
	  0,   0,  64,  44,  // tuiles 1x1
	  0,  87, 128, 256,  // mur
	128,   0, 256,  16,  // toît
	128,  16, 256,  32,  // toît
	128,  87, 256, 256,  // fenêtre
	128,  87, 192, 256,  // fenêtre
	128,  87, 192, 200,  // fenêtre
	192,  87, 256, 256,  // fenêtre
	192,  87, 256, 160,  // fenêtre
	192, 160, 256, 256,  // fenêtre
	128,  87, 256, 173,  // fenêtre
	128, 173, 256, 256,  // fenêtre
	  0,  87,  20, 120,  // cheminée
	128,  67, 148,  87,  // cheminée
	-1
};

static int tablePartBar3[] =	// barrier3.tga
{
	  0,  43, 256, 256,  // mur h=10
	  0,  86, 256, 256,  // mur h=8
	  0, 128, 256, 256,  // mur h=6
	  0, 170, 256, 256,  // mur h=4
	  0, 213, 256, 256,  // mur h=2
	  0, 170, 256, 213,  // mur h=2
	 85, 127, 172, 172,  // pierre
	170, 212, 256, 256,  // pierre
	  0,   0,   8,   8,  // foncé
	  8,   0,  16,   8,  // clair
	 76,   0,  84,  43,  // mat
	 84,   0, 127,  43,  // drapeau 1
	127,   0, 170,  43,  // drapeau 2
	170,   0, 213,  43,  // drapeau 3
	213,   0, 256,  43,  // drapeau 4 <
	-1
};

static int tablePartBar4[] =	// barrier4.tga
{
	  0,   0, 128, 128,  // mur
	  0, 128,  32, 160,  // gris moyen
	  0, 160,  32, 192,  // gris foncé
	 64, 128, 128, 192,  // dôme
	128, 128, 256, 160,  // hachures
	128, 160, 256, 192,  // antenne
	 56, 128,  64, 136,  // support gris
	 32, 128,  64, 160,  // support joint
	251,  32, 256,  37,  // gris
	128,   0, 256,  16,  // panneau
	128,  16, 256,  32,  // panneau
	128,  32, 208,  48,  // panneau
	128,  48, 208,  67,  // panneau
	128,  67, 208,  85,  // panneau
	128,  84, 217, 128,  // clavier
	208,  32, 230,  84,  // côté gauche
	229,  32, 251,  84,  // côté droite
	218,  84, 246, 128,  // alarme
	246,  84, 256, 109,  // floppy
	 32, 160,  64, 192,  // phare
	  0, 192,  16, 224,  // phare
	-1
};

static int tablePartBar5[] =	// barrier5.tga
{
	 96,  32, 104,  40,  // gris
	104,  32, 112,  40,  // gris
	112,  32, 120,  40,  // gris
	120,  32, 128,  40,  // gris
	 96,  40, 104,  48,  // gris
	104,  40, 112,  48,  // gris
	112,  40, 120,  48,  // orange
	120,  40, 128,  48,  // blanc
	128,  32, 160,  64,  // gris =
	  0,   0,  64,  64,  // sol
	128,   0, 256,  32,  // côté
	 64,   0, 128,  32,  // tuyau
	192,  32, 256,  64,  // tuyau
	 64,  32,  96,  64,  // tuyau
	 64,  64, 128, 128,  // raccord
	  0,  64,  64, 128,  // face carrée
	  0, 128,  64, 192,  // face carrée + ombre o
	 64, 128, 128, 256,  // face rectangulaire
	160,  32, 192,  64,  // face carrée claire
	-1
};

static int tablePartBar6[] =	// barrier6.tga
{
	  0,   0, 128, 224,  // grand mur
	 80,   0, 128,  32,  // pierre 3x2
	  0, 194,  48, 224,  // pierre 3x2
	  0,  64,  64,  96,  // pierre 4x2
	 64,  64, 128,  96,  // pierre 4x2
	192, 176, 256, 256,  // mur fond
	128,   0, 256,  48,  // voute
	128,  48, 160, 178,  // colonne
	160,  48, 256, 135,  // toît
	160, 144, 256, 160,  // bord
	160, 160, 256, 176,  // bord
	  3,  98,  45, 125,  // statue
	  0, 224, 128, 234,  // épéé
	160, 135, 168, 143,  // épéé
	-1
};

static int tablePartBar7[] =	// barrier7.tga
{
	  0,   0, 256, 256,  // roc
	-1
};

static int tablePartBar8[] =	// barrier8.tga
{
	  0,   0,   8,   8,  // 
	  8,   0,  16,   8,  // 
	 16,   0,  24,   8,  // 
	 24,   0,  32,   8,  // 
	 34,   2,  94,  62,  // carré (4)
	 32,   0,  96,  64,  //
	 96,   0, 160,  32,  // 
	 96,  32, 160,  64,  // 
	 34,  66,  94, 126,  // carré (8)
	 32,  64,  96, 128,  //
	 96,  64, 160,  96,  // 
	 96,  96, 160, 128,  // 
	 34, 130,  94, 190,  // carré (12)
	 32, 128,  96, 192,  //
	 96, 128, 160, 160,  // 
	 96, 160, 160, 192,  // 
	 34, 194,  94, 254,  // carré (16)
	 32, 192,  96, 256,  //
	 96, 192, 160, 224,  // 
	 96, 224, 160, 256,  // 
	160, 192, 224, 256,  // octogone
	  0,   8,   8,  16,  // vert
	  8,   8,  16,  16,  // orange
	 16,   8,  24,  16,  // violet
	  0,  32,  32,  64,  // creuset
	160,   0, 181, 128,  // 2 lignes vertes
	  0,  64,  32, 128,  // axe
	  0, 128,  32, 160,  // canon
	160, 128, 224, 192,  // base axe
	  0, 160,  32, 224,  // buse
	224, 128, 256, 256,  // isolateur
	245,   0, 256,  64,  // canon
	245,  64, 256, 128,  // canon
	 24,   8,  32,  16,  // blupi
	181,   0, 245,  55,  // blupi
	181,  55, 245, 108,  // blupi
	-1
};

static int tablePartBar9[] =	// barrier9.tga
{
	  0,   0,   8,   8,  // 
	  8,   0,  16,   8,  // 
	 16,   0,  24,   8,  // 
	 24,   0,  32,   8,  // 
	 32,   0,  96,  32,  // base //
	 96,   0, 199,  32,  // base
	  0,  32,  32,  64,  // base
	 32,  32,  64,  64,  // base
	 64,  32, 128,  64,  // base
	  0,  64,  32,  96,  // foncé 2x2
	  0,  96,  32, 192,  // foncé 2x6
	 32,  64, 160, 192,  // foncé 8x8
	 32, 192, 160, 240,  // foncé 8x3
	 32, 240, 160, 256,  // foncé 8x1
	160,  32, 256, 128,  // dessus parabole
	217,   0, 256,  32,  // articulation parabole
	160, 128, 256, 144,  // antenne parabole
	160, 144, 256, 240,  // parabole
	160, 240, 256, 256,  // isolateur
	-1
};

static int tablePartBar10[] =	// barrier10.tga
{
	 64,  16,  96,  32,  // commode
	 64,   0,  96,  32,  // commode
	  0,   0,  64,  32,  // commode

	160,  16, 192,  32,  // commode
	160,   0, 192,  32,  // commode
	 96,   0, 160,  32,  // commode

	 64,  96,  96, 128,  // armoire
	 64,  32,  96, 128,  // armoire
	  0,  32,  64, 128,  // armoire

	160,  96, 192, 128,  // armoire
	160,  32, 192, 128,  // armoire
	 96,  32, 160, 128,  // armoire

	 64, 160,  96, 176,  // buffer
	 64, 128,  96, 176,  // buffer
	  0, 128,  64, 176,  // buffer

	160, 160, 192, 176,  // buffer
	160, 128, 192, 176,  // buffer
	 96, 128, 160, 176,  // buffer
	-1
};

static int tablePartBar11[] =	// barrier11.tga
{
	  0,   0,  64,  64,  // 
	 64,   0, 128,  64,  // 
	128,   0, 192,  64,  // 
	192,   0, 256,  64,  // 
	  0,  64,  64, 128,  // 
	 64,  64, 128, 128,  // 
	128,  64, 192, 128,  // 
	192,  64, 256, 128,  // 
	  0, 128,  64, 192,  // 
	 64, 128, 128, 192,  // 
	128, 128, 192, 192,  // 
	192, 128, 256, 192,  // 
	  0, 192,  64, 256,  // 
	 64, 192, 128, 256,  // 
	128, 192, 192, 256,  // 
	192, 192, 256, 256,  // 

	  0,   0,   8,  64,  // 
	-1
};

static int tablePartP[] =	// plant.tga
{
#if 1
	 48, 100,  78, 139,  // feuille palmier 1
	 48, 139,  78, 178,  // feuille palmier 2
	 48, 178,  78, 217,  // feuille palmier 3
	 48, 217,  78, 256,  // feuille palmier 4
	 78, 100, 108, 139,  // feuille palmier 1
	 78, 139, 108, 178,  // feuille palmier 2
	 78, 178, 108, 217,  // feuille palmier 3
	 78, 217, 108, 256,  // feuille palmier 4
#endif

	195,   0, 225, 128,  // feuille tombante 1
	195, 128, 225, 170,  // feuille tombante 2
	195, 170, 225, 210,  // feuille tombante 3
	195, 210, 225, 256,  // feuille tombante 4
	225,   0, 256, 128,  // feuille tombante 1
	225, 128, 256, 170,  // feuille tombante 2
	225, 170, 256, 210,  // feuille tombante 3
	225, 210, 256, 256,  // feuille tombante 4

#if 1
	  0, 160,  48, 256,  // feuille plante de base
	  0,   0,  94, 100,  // feuille trèfle
	108, 100, 182, 256,  // feuille fougère
	 94,   0, 104, 100,  // tige 1
	185,   0, 195, 100,  // tige 2
	 95,   0, 195, 100,  // grillage

#endif
	-1
};

static int tablePartV[] =	// vegetal.tga
{
	186,   0, 256, 256,  // tronc arbre
	136,   0, 186, 256,  // tronc palmier
	-1
};

static int tablePartM2[] =	// metal02.tga
{
	  0,   0,  64,  64,  // 
	 64,   0, 128,  64,  // 
	128,   0, 192,  64,  // 
	192,   0, 256,  64,  // 
	  0,  64,  64, 128,  // 
	 64,  64, 128, 128,  // 
	128,  64, 192, 128,  // 
	192,  64, 256, 128,  // 
	  0, 128,  64, 192,  // 
	 64, 128, 128, 192,  // 
	128, 128, 192, 192,  // 
	192, 128, 256, 192,  // 
	  0, 192,  64, 256,  // 
	 64, 192, 128, 256,  // 
	128, 192, 192, 256,  // 
	192, 192, 256, 256,  // 
	-1
};

static int tablePartC1[] =	// castel01.tga
{
	  0,   0,  64,  64,  // 
	 64,   0, 128,  64,  // 
	128,   0, 192,  64,  // 
	192,   0, 256,  64,  // 
	  0,  64,  64, 128,  // 
	 64,  64, 128, 128,  // 
	128,  64, 192, 128,  // 
	192,  64, 256, 128,  // 
	  0, 128,  64, 192,  // 
	 64, 128, 128, 192,  // 
	128, 128, 192, 192,  // 
	192, 128, 256, 192,  // 
	  0, 192,  64, 256,  // 
	 64, 192, 128, 256,  // 
	128, 192, 192, 256,  // 
	192, 192, 256, 256,  // 
	-1
};

static int tablePartBet1[] =	// beton01.tga
{
	  0,   0,  64,  64,  // 
	  0, 128,  64, 192,  // 
	 64, 128, 128, 192,  // 
	128, 128, 192, 192,  // 
	192, 128, 256, 192,  // 
	  0, 192,  64, 256,  // 
	 64, 192, 128, 256,  // 
	128, 192, 192, 256,  // 
	192, 192, 256, 256,  // 
	-1
};

// Retourne le pointeur la table.

int* CModel::RetTextureTable()
{
	if ( m_textureRank ==  0 )  return tablePartW;
	if ( m_textureRank ==  1 )  return tablePartBlupi;
	if ( m_textureRank ==  2 )  return tablePartB1;
	if ( m_textureRank ==  3 )  return tablePartB2;
	if ( m_textureRank ==  4 )  return tablePartBox;
	if ( m_textureRank ==  5 )  return tablePartBox2;
	if ( m_textureRank ==  6 )  return tablePartO1;
	if ( m_textureRank ==  7 )  return tablePartO2;
	if ( m_textureRank ==  8 )  return tablePartCi1;
	if ( m_textureRank ==  9 )  return tablePartCi2;
	if ( m_textureRank == 10 )  return tablePartTrax;
	if ( m_textureRank == 11 )  return tablePartCata;
	if ( m_textureRank == 12 )  return tablePartBar1;
	if ( m_textureRank == 13 )  return tablePartBar2;
	if ( m_textureRank == 14 )  return tablePartBar3;
	if ( m_textureRank == 15 )  return tablePartBar4;
	if ( m_textureRank == 16 )  return tablePartBar5;
	if ( m_textureRank == 17 )  return tablePartBar6;
	if ( m_textureRank == 18 )  return tablePartBar7;
	if ( m_textureRank == 19 )  return tablePartBar8;
	if ( m_textureRank == 20 )  return tablePartBar9;
	if ( m_textureRank == 21 )  return tablePartBar10;
	if ( m_textureRank == 22 )  return tablePartBar11;
	if ( m_textureRank == 23 )  return tablePartP;
	if ( m_textureRank == 24 )  return tablePartV;
	if ( m_textureRank == 25 )  return tablePartM2;
	if ( m_textureRank == 26 )  return tablePartC1;
	if ( m_textureRank == 27 )  return tablePartBet1;
	return 0;
}

// Met à jour la partie de texture.

void CModel::TexturePartUpdate()
{
	int		*table;

	table = RetTextureTable();
	if ( table == 0 )  return;

	m_textureInf.x = (table[m_texturePart*4+0]+0.5f)/256.0f;
	m_textureInf.y = (table[m_texturePart*4+1]+0.5f)/256.0f;
	m_textureSup.x = (table[m_texturePart*4+2]-0.5f)/256.0f;
	m_textureSup.y = (table[m_texturePart*4+3]-0.5f)/256.0f;

	PutTextureValues();
}

// Change la texture.

void CModel::TextureRankChange(int step)
{
	m_textureRank += step;

	if ( m_textureRank >= MAX_NAMES )  m_textureRank = 0;
	if ( m_textureRank <  0         )  m_textureRank = MAX_NAMES-1;

	if ( m_textureRank ==  0 )  strcpy(m_textureName, "dock.tga");
	if ( m_textureRank ==  1 )  strcpy(m_textureName, "blupi.tga");
	if ( m_textureRank ==  2 )  strcpy(m_textureName, "bot1.tga");
	if ( m_textureRank ==  3 )  strcpy(m_textureName, "bot2.tga");
	if ( m_textureRank ==  4 )  strcpy(m_textureName, "box.tga");
	if ( m_textureRank ==  5 )  strcpy(m_textureName, "box2.tga");
	if ( m_textureRank ==  6 )  strcpy(m_textureName, "object1.tga");
	if ( m_textureRank ==  7 )  strcpy(m_textureName, "object2.tga");
	if ( m_textureRank ==  8 )  strcpy(m_textureName, "cinema1.tga");
	if ( m_textureRank ==  9 )  strcpy(m_textureName, "cinema2.tga");
	if ( m_textureRank == 10 )  strcpy(m_textureName, "trax.tga");
	if ( m_textureRank == 11 )  strcpy(m_textureName, "catapult.tga");
	if ( m_textureRank == 12 )  strcpy(m_textureName, "barrier1.tga");
	if ( m_textureRank == 13 )  strcpy(m_textureName, "barrier2.tga");
	if ( m_textureRank == 14 )  strcpy(m_textureName, "barrier3.tga");
	if ( m_textureRank == 15 )  strcpy(m_textureName, "barrier4.tga");
	if ( m_textureRank == 16 )  strcpy(m_textureName, "barrier5.tga");
	if ( m_textureRank == 17 )  strcpy(m_textureName, "barrier6.tga");
	if ( m_textureRank == 18 )  strcpy(m_textureName, "barrier7.tga");
	if ( m_textureRank == 19 )  strcpy(m_textureName, "barrier8.tga");
	if ( m_textureRank == 20 )  strcpy(m_textureName, "barrier9.tga");
	if ( m_textureRank == 21 )  strcpy(m_textureName, "barrier10.tga");
	if ( m_textureRank == 22 )  strcpy(m_textureName, "barrier11.tga");
	if ( m_textureRank == 23 )  strcpy(m_textureName, "plant.tga");
	if ( m_textureRank == 24 )  strcpy(m_textureName, "vegetal.tga");
	if ( m_textureRank == 25 )  strcpy(m_textureName, "metal02.tga");
	if ( m_textureRank == 26 )  strcpy(m_textureName, "castel01.tga");
	if ( m_textureRank == 27 )  strcpy(m_textureName, "beton01.tga");

	m_texturePart = 0;
}

// Change la partie de texture.

void CModel::TexturePartChange(int step)
{
	int		*table;

	table = RetTextureTable();
	if ( table == 0 )  return;

	if ( step > 0 )
	{
		m_texturePart ++;

		if ( table[m_texturePart*4] == -1 )
		{
			m_texturePart = 0;
		}
	}

	if ( step < 0 )
	{
		m_texturePart --;

		if ( m_texturePart < 0 )
		{
			m_texturePart = 0;
			while ( table[m_texturePart*4] != -1 )
			{
				m_texturePart ++;
			}
			m_texturePart --;
		}
	}

	TexturePartUpdate();
}


