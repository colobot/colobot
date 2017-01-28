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


#define MAX_STATES		12

static int table_state[MAX_STATES] =
{
	D3DSTATENORMAL,
	D3DSTATEPART1,  // #1 flan roue
	D3DSTATEPART2,  // #2 profil pneu
	D3DSTATEPART3,
	D3DSTATEPART4,
	D3DSTATE2FACE,	// #5
	D3DSTATETTw,
	D3DSTATETTb,
	D3DSTATETTw|D3DSTATE2FACE,  // #8 (pales helico)
	D3DSTATETTb|D3DSTATE2FACE,  // #9
	D3DSTATETTb|D3DSTATE2FACE|D3DSTATEWRAP,  // #10 (vitres blanches)
	D3DSTATETTw|D3DSTATE2FACE|D3DSTATEWRAP,  // #11 (vitres noires)
};


#define MAX_NAMES		48




// Constructeur de l'objet.

CModel::CModel(CInstanceManager* iMan)
{
	m_iMan = iMan;

	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);

	m_modFile = new CModFile(m_iMan);
	m_triangleTable = m_modFile->RetTriangleList();

	m_textureRank = 0;
	strcpy(m_textureName, "car01.tga");
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
						if ( m_textureMode > 28 )  m_textureMode = 0;
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
					if ( m_secondTexNum > 10 )  m_secondTexNum = 1;
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
	int				i, used, objRank, state;

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
		state = D3DSTATENORMAL;

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
			sprintf(texName2, "dirty%.2d.tga", m_triangleTable[i].texNum2);
			m_engine->AddTriangle(objRank, &m_triangleTable[i].p1, 3,
								  *pMat, state|D3DSTATEDUALb,
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
	int			used, i, j, rank;
	D3DVERTEX2	vi, vj;
	D3DVECTOR	sum;

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
//?			if ( vj.x == vi.x &&
//?				 vj.y == vi.y &&
//?				 vj.z == vi.z )
			if ( Abs(vj.x-vi.x) < 0.0001f &&
				 Abs(vj.y-vi.y) < 0.0001f &&
				 Abs(vj.z-vi.z) < 0.0001f )
			{
				bDone[j] = TRUE;
				index[rank++] = j;
				if ( rank >= 100 )  break;
			}
		}

		sum.x = 0;
		sum.y = 0;
		sum.z = 0;
		for ( j=0 ; j<rank ; j++ )
		{
			GetVertex(index[j], vj);
			sum.x += vj.nx;
			sum.y += vj.ny;
			sum.z += vj.nz;
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

	if ( mode == 1 )
	{
		MappingSelectSpherical(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}
	if ( mode == 2 )
	{
		MappingSelectCylindrical(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}
	if ( mode == 3 )
	{
		MappingSelectFace(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}
	if ( mode >= 4 && mode <= 27 )
	{
		MappingSelectStretch(mode, rotate, bMirrorX, bMirrorY, ti, ts, texName);
		return;
	}

	BBoxCompute(min, max);

	bPlausible[0] = IsMappingSelectPlausible(D3DMAPPINGX);
	bPlausible[1] = IsMappingSelectPlausible(D3DMAPPINGY);
	bPlausible[2] = IsMappingSelectPlausible(D3DMAPPINGZ);

	for ( i=0 ; i<9 ; i++ )
	{
		if ( !bPlausible[i%3] )  continue;
		if ( rotate-- == 0 )  break;
	}
	if ( i%3 == 0 )  D3Dmode = D3DMAPPINGX;
	if ( i%3 == 1 )  D3Dmode = D3DMAPPINGY;
	if ( i%3 == 2 )  D3Dmode = D3DMAPPINGZ;

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

#if 0
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

//?		if ( u < PI )  u = u/PI;
//?		else           u = 2.0f-u/PI;
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

			u[j] = RotateAngle(p.x, p.z)/(PI*2.0f)+0.5f;
			if ( u[j] > 1.0f ) u[j] -= 1.0f;

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

	mode = (mode-4)*4;

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



static int tablePartC01[] =	// car01.tga
{
	128,   0, 144,  16,  // bleu
	144,   0, 160,  16,  // gris clair
	160,   0, 176,  16,  // gris foncé
	176,   0, 192,  16,  // doré
	192,  76, 208,  92,  // brun
	208,  76, 224,  92,  // brun
	  0,   0, 128,  92,  // aile
	  0, 124, 128, 216,  // arrière
	192,   0, 256,  76,  // avant
	192,  92, 240, 104,  // gardeboue
	213, 104, 229, 120,  // gardegoue int
	213, 120, 229, 136,  // gardegoue ext
	  0,  92, 107, 124,  // tableau de bord
	128, 104, 213, 168,  // intérieur bas
	128, 222, 164, 245,  // intérieur haut
	128,  16, 192,  60,  // radiateur grille
	128,  60, 192,  72,  // radiateur bouchon
	  0, 216, 128, 256,  // marche-pied
	128,  72, 160, 104,  // phare
	111,  92, 127, 100,  // phare profil
	160,  72, 192, 104,  // différentiel
	128, 168, 214, 222,  // coffre
	128, 208, 214, 220,  // coffre
	138, 183, 165, 222,  // coffre
	214, 168, 256, 238,  // support coffre
	224,  76, 240,  92,  // trou
	240,  76, 248,  92,  // air
	128, 245, 256, 256,  // longeron
	164, 222, 198, 245,  // télécommande
	107,  92, 111, 124,  // antenne
	-1
};

static int tablePartC02[] =	// car02.tga
{
	160,   0, 176,  16,  // bleu
	160,  16, 176,  32,  // gris clair
	176,  16, 192,  32,  // gris moyen
	144,  16, 160,  32,  // gris foncé
	176,   0, 192,  16,  // doré
	110,  23, 119,  32,  // intérieur coffre
	192,   0, 224,  32,  // phare av
	226,  64, 254, 107,  // phare ar
	200,  60, 216,  68,  // phare profil
	224,   0, 256,  32,  // différentiel
	144,   0, 160,  16,  // axe
	  0,  32, 189,  43,  // chassis

	  0, 178,  87, 256,  // aile arrière
	 82, 178, 185, 256,  // porte
	178, 185, 204, 256,  // avant
	 86, 154, 126, 181,  // montant toît
	 97, 139, 185, 162,  // toît
	180, 161, 191, 189,  // montant av

	  0,  43,   9,  65,  // arrière montant g
	119,  43, 128,  65,  // arrière montant d
	  7,  43, 121,  65,  // arrière fenêtre

	  0,  65, 128,  88,  // coffre ar
	  0,  65, 128,  69,  // coffre ar

	  0,  88, 128,  92,  // avant
	128,  43, 192, 139,  // radiateur
	119,  23, 135,  31,  // radiateur
	192,  32, 200, 112,  // intérieur
	  0,   0, 107,  32,  // tableau de bord
	213,  64, 256, 112,  // ventillo
	213, 112, 256, 128,  // carter
	208,  32, 216,  60,  // moteur
	216,  32, 224,  48,  // moteur
	204, 162, 256, 184,  // carbu
	204, 162, 218, 184,  // carbu
	204, 162, 256, 163,  // carbu
	224,  32, 256,  64,  // pot d'échappement
	107,   0, 110,  32,  // antenne
	110,   0, 144,  23,  // télécommande
	200,  32, 208,  60,  // parchoc
	-1
};

static int tablePartC03[] =	// car03.tga
{
	128,   0, 144,  16,  // bleu
	 34, 136,  50, 152,  // autre couleur
	144,   0, 160,  16,  // gris clair
	160,   0, 176,  16,  // gris foncé
	176,   0, 192,  16,  // doré
	  0,   0,  32,  64,  // montant rear up
	 32,   0,  64,  64,  // montant rear down
	 64,   0, 128,  64,  // porte
	130, 154, 256, 256,  // toît dessus
	136, 154, 142, 256,  // toît tranche
	 76,  96,  98, 160,  // montant front
	 76,  90,  98,  96,  // dessus avant
	  0,  64,  76, 128,  // radiateur
	128,  32, 213,  96,  // intérieur
	192,   0, 224,  32,  // phare av
	 76,  64, 128,  90,  // phare ar
	224,   0, 256,  32,  // différentiel
	213,  32, 256,  80,  // ventillo
	213,  80, 256,  96,  // carter
	128,  16, 144,  32,  // pot d'échappement
	144,  16, 152,  32,  // moteur
	248, 107, 256, 135,  // moteur
	128,  96, 256, 107,  // chassis
	128, 107, 235, 139,  // tableau de bord
	235, 107, 239, 139,  // antenne
	  0, 136,  34, 159,  // télécommande
	  0, 160,  96, 256,  // métal
	 96, 160, 114, 256,  // bois
	239, 107, 248, 135,  // ressort
	152,  16, 192,  32,  // entrée carbu
	240, 135, 256, 139,  // bouchon radiateur
	  0, 128,  76, 136,  // chassis
	-1
};

static int tablePartC04[] =	// car04.tga
{
	160,   0, 176,  16,  // rouge
	168,  65, 184,  81,  // autre couleur
	160,  16, 176,  32,  // gris clair
	176,  16, 192,  32,  // gris moyen
	144,  16, 160,  32,  // gris foncé
	176,   0, 192,  16,  // doré
	 76, 128, 196, 138,  // bi-color
	  0,  32,  76,  89,  // radiateur
	  0,  89,  76, 124,  // capot
	  0, 124,  76, 159,  // capot
	 76,  32, 136,  80,  // gardeboue
	 76,  80, 136, 128,  // gardeboue
	200,  97, 256, 121,  // phare ar
	200, 121, 256, 145,  // phare ar
	  0, 164, 154, 210,  // côté d
	154, 164, 256, 210,  // arrière
	  0, 210, 154, 256,  // côté g
	154, 210, 256, 256,  // avant
	136,  32, 256,  65,  // siège
	152,  97, 176, 121,  // siège d
	176,  97, 200, 121,  // siège g
	152,  97, 156, 101,  // siège (cuir uni)
	192,   0, 224,  32,  // phare av
	136,  65, 168,  97,  // phare av dos
	192,  65, 224,  97,  // phare av dos
	110,  23, 126,  31,  // phare profil
	126,  23, 142,  31,  // phare profil
	224,   0, 256,  32,  // différentiel
	144,   0, 160,  16,  // axe
	  0,   0, 107,  32,  // tableau de bord
	107,   0, 110,  32,  // antenne
	110,   0, 144,  23,  // télécommande
	224,  65, 256,  97,  // pot d'échappement
	184,  65, 192,  89,  // parchoc
	-1
};

static int tablePartC05[] =	// car05.tga
{
	160,   0, 176,  16,  // bleu
	160,  16, 176,  32,  // gris clair
	176,  16, 192,  32,  // gris moyen
	144,  16, 160,  32,  // gris foncé
	176,   0, 192,  16,  // doré
	192, 112, 208, 128,  // bleu/vert selon subModel
	192,   0, 224,  32,  // phare av
	192, 128, 256, 144,  // phare ar
	110,  23, 126,  31,  // phare profil
	224,   0, 256,  32,  // différentiel
	144,   0, 160,  16,  // axe
	  0,  32, 165,  43,  // chassis
	  0,  43, 100, 121,  // porte
	100,  43, 128, 121,  // carro av
	128, 139, 154, 256,  // flan arrière
	200,  60, 208, 112,  // bleu ombré
	128,  43, 192, 139,  // radiateur
	 64, 192,  96, 256,  // garde boue bleu
	 96, 192, 128, 256,  // garde boue bleu
	 64, 192, 128, 256,  // garde boue bleu
	  0, 192,  64, 256,  // garde boue bleu
	 64, 128,  96, 192,  // garde boue vert
	 96, 128, 128, 192,  // garde boue vert
	 64, 128, 128, 192,  // garde boue vert
	  0, 128,  64, 192,  // garde boue vert
	192,  32, 200, 112,  // intérieur
	192,  48, 200, 112,  // intérieur
	  0,   0, 107,  32,  // tableau de bord
	208,  32, 216,  60,  // moteur
	216,  32, 224,  48,  // moteur
	224,  32, 256,  64,  // pot d'échappement
	107,   0, 110,  32,  // antenne
	110,   0, 144,  23,  // télécommande
	200,  32, 208,  60,  // parchoc
	160, 160, 256, 256,  // métal
	-1
};

static int tablePartC06[] =	// car06.tga
{
	160,   0, 176,  16,  // bleu
	192,   0, 208,  16,  // bleu clair
	160,  16, 176,  32,  // gris clair
	176,  16, 192,  32,  // gris moyen
	144,  16, 160,  32,  // gris foncé
	192,  16, 208,  32,  // jaune "avant"
	176,   0, 192,  16,  // intérieur
	  0,  32,   8, 128,  // carro
	  0,  32,   8,  64,  // carro
	  0,  96,   8, 128,  // carro
	  8,  32, 142, 128,  // porte
	 27, 157, 142, 217,  // aile av
	 27, 128, 142, 157,  // capot av
	142,  96, 256, 217,  // toît
	 64, 217, 256, 256,  // parchoc av
	 64, 217,  66, 256,  // parchoc av
	112,   0, 120,  28,  // parchoc ar
	181,  32, 213,  80,  // phare av
	  0, 128,  27, 256,  // phare ar
	224,   0, 256,  32,  // différentiel
	144,   0, 160,  16,  // axe
	213,  32, 256,  80,  // ventillo
	213,  80, 256,  96,  // carter
	120,   0, 128,  28,  // moteur
	136,  16, 144,  32,  // carbu
	128,   0, 144,  16,  // échappement bout
	216,   0, 224,  32,  // échappement tuyau
	  0,   0, 107,  32,  // tableau de bord
	107,   0, 110,  32,  // antenne
	-1
};

static int tablePartC07[] =	// car07.tga
{
	160,   0, 176,  16,  // bleu
	160,  16, 176,  32,  // gris clair
	176,  16, 192,  32,  // gris moyen
	144,  16, 160,  32,  // gris foncé
	176,   0, 192,  16,  // intérieur uni
	248,  32, 256,  96,  // intérieur bas
	208,   0, 216,  32,  // intérieur toît
	224,  32, 240,  48,  // toît
	  0, 128, 157, 208,  // carro arrière
	157, 128, 230, 208,  // carro porte
	230, 128, 256, 208,  // carro avant
	  0, 128,   4, 208,  // carro montant
	 96, 208, 222, 243,  // carro arrière haut
	  0, 208,  96, 245,  // carro arrière bas
	 64,  96, 192, 100,  // avant
	  0,  32,  64, 128,  // radiateur
	 64,  32, 128,  96,  // marche pied
	128,  32, 160,  96,  // garde boue
	128,  32, 192,  96,  // garde boue
	224,   0, 256,  32,  // phare av
	192,  32, 224,  96,  // phare ar
	110,  24, 126,  32,  // côté phare
	200,   0, 208,  16,  // bouchon 1
	200,  16, 208,  32,  // bouchon 2
	216,   0, 224,  32,  // parchoc
	  0, 245, 256, 256,  // chassis
	144,   0, 160,  16,  // échappement bout
	224,  96, 256, 128,  // différentiel
	  0,   0, 107,  32,  // tableau de bord
	110,   0, 144,  23,  // télécommande
	107,   0, 110,  32,  // antenne
	-1
};

static int tablePartC08[] =	// car08.tga
{
	  0,   0,  16,  16,  // bleu
	 16,   0,  32,  16,  // gris clair
	 32,   0,  48,  16,  // gris moyen
	 48,   0,  64,  16,  // intérieur uni
	  0,  16,  34,  39,  // télécommande
	 64,   0, 160,  96,  // tableau de bord
	160,   0, 256,  64,  // intérieur
	  0,  40,  64, 152,  // radiateur face
	  0, 152,  64, 160,  // radiateur côté
	  0, 160,  64, 197,  // arrière horizontal
	 18, 190,  46, 197,  // arrière vertical
	  0, 160,   2, 179,  // arrière sur tranche flan
	  0, 177,   2, 197,  // arrière sur tranche flan
	 64,  96, 256, 187,  // côté
	  0, 197, 224, 245,  // capot dessus
	  0, 239, 224, 245,  // capot côté
	  0, 245, 256, 256,  // chassis
	224, 197, 228, 229,  // antenne
	 64, 187, 256, 197,  // frein à main
	-1
};

static int tablePartW01[] =	// wheel01.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 141,  // carter
	  0, 141,  64, 191,  // dessus
	 64,  96, 128, 148,  // profil
	128,  96, 192, 146,  // face
	192,  96, 256, 146,  // dos
	 32,  96,  40, 104,  // courroie
	 64, 148,  78, 180,  // carbu
	 78, 148, 102, 172,  // carbu
	-1
};

static int tablePartW02[] =	// wheel02.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 141,  // carter
	  0, 141,  64, 191,  // dessus
	 64,  96, 128, 148,  // profil
	128,  96, 192, 146,  // face
	192,  96, 256, 146,  // dos
	 32,  96,  40, 104,  // courroie
	 64, 148,  78, 180,  // carbu
	 78, 148, 102, 172,  // carbu
	102, 148, 150, 170,  // carbu'
	150, 148, 198, 176,  // carbu'
	198, 148, 230, 176,  // carbu'
	-1
};

static int tablePartW03[] =	// wheel03.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 141,  // carter
	  0, 141,  64, 191,  // dessus
	 64,  96, 128, 148,  // profil
	128,  96, 192, 146,  // face
	192,  96, 256, 146,  // dos
	 32,  96,  40, 104,  // courroie
	 64, 148,  78, 180,  // carbu
	 78, 148, 102, 172,  // carbu
	-1
};

static int tablePartW04[] =	// wheel04.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 149,  // carter
	 64,  96, 128, 148,  // profil
	128,  96, 192, 156,  // face
	192,  96, 256, 156,  // dos
	  0, 149,  14, 181,  // carbu
	  0, 149,  56, 181,  // carbu
	128, 156, 224, 180,  // carbu
	-1
};

static int tablePartW05[] =	// wheel05.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 141,  // carter
	  0, 141,  64, 191,  // dessus
	 64,  96, 128, 148,  // profil
	128,  96, 192, 146,  // face
	192,  96, 256, 146,  // dos
	 32,  96,  40, 104,  // courroie
	 64, 148,  78, 180,  // carbu
	 78, 148, 102, 172,  // carbu
	-1
};

static int tablePartW06[] =	// wheel06.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 141,  // carter
	  0, 141,  64, 191,  // dessus
	 64,  96, 128, 148,  // profil
	128,  96, 192, 146,  // face
	192,  96, 256, 146,  // dos
	 32,  96,  40, 104,  // courroie
	 64, 148,  78, 180,  // carbu
	 78, 148, 102, 172,  // carbu
	-1
};

static int tablePartW07[] =	// wheel07.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	  0, 192, 128, 256,  // vitre
	  0, 108,  64, 141,  // carter
	  0, 141,  64, 191,  // dessus
	 64,  96, 128, 148,  // profil
	128,  96, 192, 146,  // face
	192,  96, 256, 146,  // dos
	 32,  96,  40, 104,  // courroie
	 64, 148,  78, 180,  // carbu
	 78, 148,  94, 168,  // carbu
	 78, 148, 126, 168,  // carbu
	128, 146, 192, 181,  // carbu
	-1
};

static int tablePartW08[] =	// wheel08.tga
{
	  0,   0,  64,  64,  // roue
	  0,  64,  64,  96,  // profil pneu
	 10,  10,  54,  54,  // jante
	  0,  96,  32, 108,  // profil jante
	 32,  96,  40, 104,  // gris
	 32, 104,  48, 120,  // échappement
	  0, 128, 165, 192,  // moteur d
	  0, 192, 165, 256,  // moteur g
	  5, 128,  43, 192,  // moteur face
	  0, 172, 165, 192,  // moteur dessus
	165, 128, 221, 256,  // chaîne
	-1
};

static int tablePartT[] =	// trax.tga
{
	128,   0, 136,   8,  // jaune
	136,   0, 144,   8,  // gris
	192,   0, 256,  32,  // profil chenille
	  0,   0, 128,  53,  // chenille
	160,   0, 192,  32,  // pot d'échappement
	  0,  53, 128, 129,  // pelle profil
	  0, 129, 128, 183,  // pelle face
	  0, 183, 128, 256,  // carro profil
	176,  32, 256,  80,  // carro avant
	128,  32, 176,  80,  // bouton
	176, 128, 256, 178,  // tableau de bord
	128,   8, 136,  32,  // levier bas
	136,   8, 144,  32,  // levier haut
						 // ufo :
	144,   0, 152,   8,  // vert
	152,   0, 160,   8,  // vert
	128,  80, 160, 128,  // dessus
	128, 128, 160, 144,  // côté
	128, 144, 160, 240,  // dessous
	160,  80, 192, 112,  // réacteur
	160, 112, 192, 128,  // réacteur
	192,  80, 256, 123,  // vitre

	178, 178, 256, 256,  // gravier
	-1
};

static int tablePartR[] =	// door.tga
{
	112,  54, 120,  62,  // gris
	120,  54, 128,  62,  // gris foncé
	128,  54, 136,  62,  // gris clair
	  0,   0, 256,  54,  // porte
	  0,  54, 112, 254,  // montant
	  0,  54,  10, 254,  // montant
	192,  54, 256,  86,  // tableau
	112, 227, 256, 256,  // barrier 2
	160, 169, 256, 227,  // barrier 3
	112, 115, 136, 179,  // tonneau 1
	136, 115, 160, 179,  // tonneau 1
	160,  54, 192,  86,  // tonneau 1
	250, 126, 256, 169,  // tonneau 2
	231, 150, 250, 169,  // tonneau 2
	202, 150, 221, 169,  // bombe
	136,  54, 152,  70,  // bombe
	136,  70, 152,  86,  // bombe
	152,  54, 160,  86,  // mèche
	112,  62, 128,  78,  // pierre
	112,  86, 256, 106,  // bois
	 48,  38, 208,  54,  // hachures barrière
	120,  78, 128,  86,  // orange cône
	128,  62, 136,  86,  // hachures cône
	136, 179, 160, 203,  // dessous cône
	 48,   0,  97,  54,  // mine2
						 // fire :
	160, 150, 168, 158,  // rouge
	160, 158, 168, 166,  // gris
	168, 150, 176, 158,  // tuyau
	112, 106, 160, 115,  // bombonne
	226, 126, 250, 150,  // bombonne
	160, 106, 168, 128,  // support
	160, 128, 168, 150,  // support
	168, 106, 176, 150,  // support
	176, 126, 202, 169,  // articulation
	176, 106, 256, 126,  // lance
	202, 126, 226, 150,  // trou
	 42,  38, 106,  54,  // hachures
	136, 203, 160, 227,  // arrow
						 // aquarium
	221, 150, 229, 158,  // verre
	221, 158, 229, 166,  // eau
	-1
};

static int tablePartR2[] =	// door2.tga
{
	  8, 184,  16, 192,  // gris 1
	  0, 176,   8, 184,  // gris 2
	  8, 176,  16, 184,  // gris 3
	  0, 184,   8, 192,  // gris 4
	  0,   0,  32, 144,  // bielle
	  0, 144,  24, 176,  // axe piston
	  0, 192,  64, 256,  // grosse roue
	 24, 144,  40, 160,  // petite roue
	 16, 176,  24, 192,  // engrenage
	 32,   0, 112,  80,  // machoire face
	112, 228, 256, 256,  // machoire côté
	 32,  80, 112,  88,  // machoire piston
	 64, 208, 112, 256,  // pillon
	248,   0, 256, 224,  // face
	 32,  88,  80, 144,  // aération
	112,   0, 120,  68,  // piston brillant
	 24, 160,  88, 192,  // tableau de bord
	120,   0, 184,  91,  // moteur

	120,  91, 184, 155,  // bâtiment fenêtre large
	120, 155, 184, 219,  // bâtiment uni
	184,   0, 248,  64,  // bâtiment fenêtre étroite
	184,  64, 248, 128,  // bâtiment fenêtre large
	184, 128, 248, 192,  // bâtiment uni
	120,  91, 184, 187,  // bâtiment fenêtre large rez
	184,  64, 248, 160,  // bâtiment fenêtre large rez
	184, 128, 248, 224,  // bâtiment uni rez
	-1
};

static int tablePartW[] =	// dock.tga
{
	  0, 136,   8, 144,  // gris clair
	  0, 144,   8, 152,  // gris
	  8, 144,  16, 152,  // gris foncé
	  0,   0, 152, 152,  // sol octogone
	194,   0, 256, 224,  // pillier 1
	194,   0, 210, 224,  // pillier 2
	  0, 224, 256, 256,  // portique
	  0, 248, 256, 256,  // barrière
	  0, 152,   8, 224,  // piston 1
	  0, 174,   8, 224,  // piston 2
	  0, 190,   8, 224,  // piston 3
	  0, 200,   8, 224,  // piston 4
	 80, 152, 126, 184,  // aimant
	 16, 152,  80, 184,  // tableau
	152,   0, 194,  26,  // flèche
	  8, 144,  16, 224,  // paratonnerre: haut
	 16, 184,  80, 208,  // paratonnerre: milieu
	 80, 184, 112, 216,  // paratonnerre: pic
	178,  26, 186, 107,  // paratonnerre: haut
	186,  26, 194, 206,  // paratonnerre: montant
	165,  26, 178, 224,  // crémaillère
	112, 184, 151, 216,  // crochet
	126, 152, 164, 175,  // crochet
	-1
};

static int tablePartStone[] =	// stone.tga
{
	  0,   0, 256, 256,  // pierre
	-1
};

static int tablePartA1[] =	// alien1.tga
{
	 80,   0,  88,   8,  // vert
	 80,   8,  88,  16,  // vert foncé
	 80,  16,  88,  24,  // vert clair
	 80,  24,  88,  32,  // gris
	 80,  32,  88,  40,  // gris foncé
	 80,  40,  88,  48,  // gris clair
	  0,   0,  64,  48,  // face 1
	  0,  48,  64,  96,  // face 2
	  0,  96,  64, 144,  // face 3
	  0, 144,  64, 192,  // face 4
	  0, 192,  64, 202,  // face 5 grise
	 64, 192, 128, 200,  // face 6
	 64,   0,  72, 192,  // profil vert 1
	112,   0, 120, 192,  // profil vert 1 ombré
	 72,   0,  80,  64,  // profil vert 2
	 72,  64,  80, 128,  // profil vert 3
	 72, 128,  80, 192,  // profil vert 4
	120,  64, 128, 192,  // profil vert 5
	 80,  64,  88, 128,  // profil gris 1
	 80, 128,  88, 192,  // profil gris 2
	 88,  64,  96, 128,  // profil gris 3
	 88, 128,  96, 192,  // profil gris 4
	 96,  64, 104, 128,  // profil gris 5
	 96, 128, 104, 192,  // profil gris 6
	104,  64, 112, 128,  // profil gris 7
	  0, 248, 256, 256,  // hachures //
	128, 230, 192, 256,  // électrocuteur
	192, 187, 256, 256,  // marteau
	 88,   0, 107,  19,  // électrocuteur
	 88,  19, 107,  38,  // écraseur
	120,   0, 200,  64,  // siège profil
	200,   0, 256,  64,  // siège face
	135,  47, 154,  64,  // siège avant
	192,  64, 256, 128,  // tapis roulant
	  0, 202, 192, 224,  // profil chenille
	208, 128, 256, 176,  // bouton rouge
	128, 128, 192, 197,  // panneau de commande
	  0, 224, 192, 230,  // montant torture
	 80,  48,  88,  64,  // poutre
	120,   0, 160,  40,  // dessous tire-bouchon
	  0, 230, 126, 238,  // tapis
	-1
};

static int tablePartA2[] =	// alien2.tga
{
	  0,   0, 256, 109,  // côté avec porte
	 60,  29, 172, 109,  // porte seule
	 40,  29,  58,  76,  // béton haut
	 65,  20, 167,  34,  // béton large
	247,  76, 256, 109,  // bas
	  0, 109, 122, 256,  // marteau
	  0, 109,  32, 256,  // marteau
	122, 109, 204, 256,  // écraseur
	204, 109, 256, 256,  // écraseur
	-1
};

static int tablePartI1[] =	// inca1.tga
{
	  0,   0, 256,  45,  // toît basrelief
	196,  86, 256, 131,  // toît long
	 32, 204, 256, 256,  // toît court
	  0,  45, 256,  86,  // socle
	  0,  45,  64,  86,  // socle court
	 64,  45, 128,  86,  // socle court
	128,  45, 192,  86,  // socle court
	192,  45, 256,  86,  // socle court
	183, 131, 256, 204,  // sol
	 87, 148, 183, 204,  // escaliers
	 87, 148, 183, 157,  // dessus
	 87, 148,  96, 204,  // tranche
	  0, 128,  16, 256,  // colonne 1
	 16, 128,  32, 256,  // colonne 2
	  0,  86, 192,  94,  // hachures //
	  0,  94,   8, 102,  // gris
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
	154, 214, 198, 256,  // couronne
	116, 218, 154, 256,  // couronne
	-1
};

static int tablePartB2[] =	// bot2.tga
{
	  0,   0,  80, 120,  // face bot3
	  0, 128,  64, 181,  // bouche heureux
	 64, 128, 128, 181,  // bouche triste
	  0, 120,   8, 128,  // jaune
	  8, 120,  16, 128,  // noir
						 // bot4:
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
						 // couronne:
	248,  24, 256,  32,  // or
	128, 134, 256, 256,  // perles
	 64, 192, 128, 256,  // diamant
	 64, 181,  96, 192,  // tranche
	 96, 181, 128, 192,  // tranche
	 56, 181,  64, 256,  // tranche
	-1
};

static int tablePartD[] =	// remote.tga
{
	  0, 192,   8, 200,  // gris clair
	  8, 192,  16, 200,  // gris foncé
	 16, 192,  24, 200,  // rouge
	 24, 192,  32, 200,  // rouge
	  0,   0, 256, 128,  // grand écran
	  0, 128, 128, 192,  // haut
	128, 128, 256, 192,  // bas
	  0, 200,   4, 232,  // antenne
	  0, 232,   4, 248,  // support
	  8, 200,  16, 256,  // dessous
	  8, 222,  16, 256,  // dessous
	-1
};

static int tablePartC[] =	// home1.tga
{
	108,  60, 116,  68,  // gris clair
	116,  60, 124,  68,  // gris foncé
	  0,   0, 128,  42,  // synthé dessus
	  0,  42, 128,  60,  // synthé arrière
	  0,  60,  72, 156,  // hp devant
	 72,  60,  96,  92,  // hp derrière
	 96,  60, 108,  92,  // hp côté
	 72,  92,  96, 108,  // hp dessous
	 72, 108,  80, 128,  // pied synthé
	 72, 108,  80, 156,  // pied hp
	128,   0, 256, 128,  // sol home1
	 80, 108,  88, 156,  // côté home1
	108,  68, 116,  76,  // bois
	116,  68, 124,  76,  // orange
	108,  76, 116,  84,  // bleu
	116,  76, 124,  84,  // vert
	108,  84, 116,  92,  // magenta
	116,  84, 124,  92,  // violet
	  0, 160,  32, 192,  // 
	 32, 160,  64, 192,  // 
	 64, 160,  96, 192,  // 
	 96, 160, 128, 192,  // 
	  0, 192,  32, 224,  // 
	 32, 192,  64, 224,  // 
	 64, 192,  96, 224,  // 
	 96, 192, 128, 224,  // 
	  0, 224,  32, 256,  // 
	 32, 224,  64, 256,  // 
	128, 128, 256, 256,  // cible
	-1
};

static int tablePartF1[] =	// factory1.tga
{
	108, 200, 116, 208,  // gris foncé
	108, 208, 116, 216,  // gris clair
	108, 216, 116, 224,  // rouge
	  0,   0, 256, 160,  // face 3
	  0, 160, 128, 192,  // face 3
	  0, 192, 128, 200,  // face 3
	240, 160, 248, 256,  // face 1
	248, 160, 256, 256,  // face 1
	168, 160, 172, 224,  // face 8
	128, 160, 168, 224,  // face 8
	  0, 200, 100, 240,  // face 9
	  0, 200, 100, 202,  // face 9
	100, 200, 108, 240,  // face 9
	  0, 240, 240, 256,  // barrière
	224, 160, 232, 240,  // tuyau 1
	232, 160, 240, 240,  // tuyau 2
	130, 160, 166, 192,  // tuyau bout
	172, 160, 220, 240,  // moteur
	-1
};

static int tablePartF2[] =	// factory2.tga
{
	  0,   0, 256,  32,  // face 2
	  0,  32, 256, 128,  // face 2
	  0, 128, 120, 224,  // face 5
	120, 128, 168, 200,  // face 5
	168, 128, 176, 192,  // face 5
	176, 128, 184, 192,  // face 5
	168, 192, 176, 200,  // face 5
	184, 128, 192, 192,  // face 4
	192, 128, 200, 192,  // face 4
	120, 200, 256, 238,  // face 4
	216, 128, 254, 166,  // face 4
	120, 238, 256, 246,  // face 6
	200, 128, 216, 192,  // face 3
	176, 192, 192, 200,  // pillier
	192, 192, 208, 200,  // pillier
	  0, 246, 256, 256,  // barrière rouge-blanc
	  0, 246,   8, 256,  // barrière rouge
	240, 246, 256, 256,  // barrière blanc
	216, 166, 256, 186,  // panneau vanne
	-1
};

static int tablePartF3[] =	// factory3.tga
{
	  0,   0,  85,  64,  // 
	 85,   0, 170,  64,  // 
	170,   0, 255,  64,  // 
	  0,  64,  85, 128,  // 
	 85,  64, 170, 128,  // 
	170,  64, 255, 128,  // 
	  0, 128,  85, 192,  // 
	 85, 128, 170, 192,  // 
	170, 128, 255, 192,  // 
	  0, 192,  85, 256,  // 
	 85, 192, 170, 256,  // 
	170, 192, 255, 256,  // 
	-1
};

static int tablePartBox[] =	// box.tga
{
	  0,   0,  56,  56,  // caisse 1
	  0,  56,  56, 112,  // caisse 1
	 56,   0, 112,  56,  // caisse 2
	 56,  56, 112, 112,  // caisse 2
	112,   0, 168,  56,  // caisse 3
	112,  56, 168, 112,  // caisse 3
	  0, 112,  56, 168,  // caisse 4
	  0, 168,  56, 224,  // caisse 4
	 56, 112, 112, 168,  // caisse 5
	 56, 168, 112, 224,  // caisse 5
	112, 112, 172, 176,  // caisse 6
	112, 176, 172, 240,  // caisse 6
	168,   0, 224,  39,  // palette 1
	168,   0, 224,  67,  // palette 2
	168,   8, 224,  12,  // palette dessus
	-1
};

static int tablePartS[] =	// road1.tga
{
	  0,   0,   8,   8,  // gris foncé
	  0,  76,   8,  84,  // gris moyen
	  0,   0,  64,  38,  // start
	 64,   0, 128,  38,  // goal
	128,   0, 192,  38,  // bot1
	192,   0, 256,  38,  // bot3
	  0,  38,  64,  76,  // bot2
	 64,  38, 128,  76,  // evil1
	  0, 192,  64, 256,  // left
	 64, 192, 128, 256,  // boss
	  0, 131,  64, 186,  // !
	  0,  76,  64, 131,  // atomic
	 64,  76, 128, 131,  // fire
	 64, 131, 128, 186,  // load
	128,  76, 192, 131,  // 10%
	128, 131, 192, 186,  // stone
	192, 128, 256, 192,  // arrow up
	128, 192, 192, 256,  // arrow left
	192, 192, 256, 256,  // arrow right
	192,  64, 256, 128,  // engrenage cassé
	128,  38, 166,  76,  // hachures blanc-noir
	166,  38, 174,  76,  // support
	174,  38, 182,  76,  // support
	224,  48, 232,  64,  // support
	192,  38, 256,  48,  // néon
	240,  48, 248,  56,  // néon
	232,  48, 240,  56,  // néon
	248,  48, 256,  56,  // gris clair
	240,  56, 248,  64,  // gris moyen
	248,  56, 256,  64,  // gris foncé
	-1
};

static int tablePartP01[] =	// pub01.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP02[] =	// pub02.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP03[] =	// pub03.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP04[] =	// pub04.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP05[] =	// pub05.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP06[] =	// pub06.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP07[] =	// pub07.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP08[] =	// pub08.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP09[] =	// pub09.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartP10[] =	// pub10.tga
{
	  0,   0, 128, 128,  // 
	128,   0, 256, 128,  // 
	  0, 128, 128, 256,  // 
	128, 128, 256, 256,  // 
	-1
};

static int tablePartSu[] =	// support.tga
{
	  0,   0, 224, 224,  // dessus
	  0, 224,  64, 256,  // biseau
	 64, 224, 128, 256,  // biseau
	224,   0, 255, 224,  // côté
	-1
};

static int tablePartSe[] =	// search.tga
{
	245,   0, 253,   8,  // gris foncé
	245,   8, 253,  16,  // gris clair
	245,  16, 253,  24,  // rouge
	245,  24, 253,  32,  // bleu
	  0,   0,  71, 128,  // search neutre
	 71,   0, 142, 128,  // search CTRL
	142,   0, 213, 128,  // search DEL
	  0, 128,  71, 256,  // search ALT
	213,   0, 229,  56,  // montant
	229,   0, 245,  56,  // montant
	 71, 128, 199, 166,  // supp
	213,  56, 237,  80,  // bouton rouge
	 71, 166, 199, 182,  // pale
	213,  80, 229, 128,  // rotor
	199, 128, 247, 145,  // base
	199, 145, 256, 230,  // support
	247,  73, 256, 145,  // support
	119, 230, 145, 256,  // 3 pales en mouvement
						 // boussole :
	237,  56, 245, 128,  // rouge
	229,  80, 237, 104,  // bleu
	213,  80, 229, 104,  // rotor
						 // électrocuteur :
	230, 230, 256, 256,  // sphère
	 71, 182,  87, 246,  // pile
	 87, 182, 119, 246,  // pile danger
	119, 182, 151, 214,  // pile dessus
	-1
};

static int tablePartP[] =	// plant.tga
{
	  0, 160,  48, 256,  // feuille 1
	  0,   0,  94, 100,  // feuille 2
	 48, 156, 108, 256,  // feuille 3
	 94,   0, 104, 100,  // tige 1
	185,   0, 195, 100,  // tige 2
	108, 100, 182, 256,  // fougère
	104,   0, 144, 100,  // courge
	196,   0, 256, 256,  // armature
	-1
};

static int tablePartV[] =	// vegetal.tga
{
	  0,   0,  94, 100,  // racine
	186,   0, 256, 256,  // tronc
	 94,   0, 186, 128,  // mur
	  0, 100,   8, 108,  // intérieur
	  0, 128, 128, 256,  // organique
	  0, 166, 112, 216,  // organique
	-1
};

static int tablePartH[] =	// human.tga
{
	 64,   0,  96,  64,  // cuisse
	 96,   0, 128,  64,  // jambe
	128,   0, 192,  32,  // bras
	128,  32, 192,  64,  // avant-bras
	192,  32, 256,  64,  // avant-bras poilu
	  0,  64, 128, 224,  // ventre
	128,  64, 256, 224,  // dos
	 64, 224, 112, 256,  // dessus pied
	144, 224, 168, 240,  // dessous pied
	112, 224, 144, 240,  // côté pied
	112, 224, 128, 240,  // côté pied
	  0, 224,  64, 256,  // gant
	128, 240, 144, 256,  // tissus 1
	224, 224, 256, 256,  // tissus 2
	112, 240, 128, 256,  // peau
	192, 224, 224, 246,  // verres lunettes
	144, 240, 160, 256,  // branches lunettes
	-1
};

static int tablePartFa[] =	// face01.tga
{
	  0,   0, 256, 256,  // visage
	-1
};

// Retourne le pointeur la table.

int* CModel::RetTextureTable()
{
	if ( m_textureRank ==  0 )  return tablePartC01;
	if ( m_textureRank ==  1 )  return tablePartC02;
	if ( m_textureRank ==  2 )  return tablePartC03;
	if ( m_textureRank ==  3 )  return tablePartC04;
	if ( m_textureRank ==  4 )  return tablePartC05;
	if ( m_textureRank ==  5 )  return tablePartC06;
	if ( m_textureRank ==  6 )  return tablePartC07;
	if ( m_textureRank ==  7 )  return tablePartC08;
	if ( m_textureRank ==  8 )  return tablePartW01;
	if ( m_textureRank ==  9 )  return tablePartW02;
	if ( m_textureRank == 10 )  return tablePartW03;
	if ( m_textureRank == 11 )  return tablePartW04;
	if ( m_textureRank == 12 )  return tablePartW05;
	if ( m_textureRank == 13 )  return tablePartW06;
	if ( m_textureRank == 14 )  return tablePartW07;
	if ( m_textureRank == 15 )  return tablePartW08;
	if ( m_textureRank == 16 )  return tablePartT;
	if ( m_textureRank == 17 )  return tablePartR;
	if ( m_textureRank == 18 )  return tablePartR2;
	if ( m_textureRank == 19 )  return tablePartW;
	if ( m_textureRank == 20 )  return tablePartStone;
	if ( m_textureRank == 21 )  return tablePartA1;
	if ( m_textureRank == 22 )  return tablePartA2;
	if ( m_textureRank == 23 )  return tablePartI1;
	if ( m_textureRank == 24 )  return tablePartB1;
	if ( m_textureRank == 25 )  return tablePartB2;
	if ( m_textureRank == 26 )  return tablePartD;
	if ( m_textureRank == 27 )  return tablePartC;
	if ( m_textureRank == 28 )  return tablePartF1;
	if ( m_textureRank == 29 )  return tablePartF2;
	if ( m_textureRank == 30 )  return tablePartF3;
	if ( m_textureRank == 31 )  return tablePartBox;
	if ( m_textureRank == 32 )  return tablePartS;
	if ( m_textureRank == 33 )  return tablePartP01;
	if ( m_textureRank == 34 )  return tablePartP02;
	if ( m_textureRank == 35 )  return tablePartP03;
	if ( m_textureRank == 36 )  return tablePartP04;
	if ( m_textureRank == 37 )  return tablePartP05;
	if ( m_textureRank == 38 )  return tablePartP06;
	if ( m_textureRank == 39 )  return tablePartP07;
	if ( m_textureRank == 40 )  return tablePartP08;
	if ( m_textureRank == 41 )  return tablePartP09;
	if ( m_textureRank == 42 )  return tablePartP10;
	if ( m_textureRank == 43 )  return tablePartSu;
	if ( m_textureRank == 44 )  return tablePartSe;
	if ( m_textureRank == 45 )  return tablePartP;
	if ( m_textureRank == 46 )  return tablePartV;
	if ( m_textureRank == 47 )  return tablePartH;
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

	if ( m_textureRank ==  0 )  strcpy(m_textureName, "car01.tga");
	if ( m_textureRank ==  1 )  strcpy(m_textureName, "car02.tga");
	if ( m_textureRank ==  2 )  strcpy(m_textureName, "car03.tga");
	if ( m_textureRank ==  3 )  strcpy(m_textureName, "car04.tga");
	if ( m_textureRank ==  4 )  strcpy(m_textureName, "car05.tga");
	if ( m_textureRank ==  5 )  strcpy(m_textureName, "car06.tga");
	if ( m_textureRank ==  6 )  strcpy(m_textureName, "car07.tga");
	if ( m_textureRank ==  7 )  strcpy(m_textureName, "car08.tga");
	if ( m_textureRank ==  8 )  strcpy(m_textureName, "wheel01.tga");
	if ( m_textureRank ==  9 )  strcpy(m_textureName, "wheel02.tga");
	if ( m_textureRank == 10 )  strcpy(m_textureName, "wheel03.tga");
	if ( m_textureRank == 11 )  strcpy(m_textureName, "wheel04.tga");
	if ( m_textureRank == 12 )  strcpy(m_textureName, "wheel05.tga");
	if ( m_textureRank == 13 )  strcpy(m_textureName, "wheel06.tga");
	if ( m_textureRank == 14 )  strcpy(m_textureName, "wheel07.tga");
	if ( m_textureRank == 15 )  strcpy(m_textureName, "wheel08.tga");
	if ( m_textureRank == 16 )  strcpy(m_textureName, "trax.tga");
	if ( m_textureRank == 17 )  strcpy(m_textureName, "door.tga");
	if ( m_textureRank == 18 )  strcpy(m_textureName, "door2.tga");
	if ( m_textureRank == 19 )  strcpy(m_textureName, "dock.tga");
	if ( m_textureRank == 20 )  strcpy(m_textureName, "stone.tga");
	if ( m_textureRank == 21 )  strcpy(m_textureName, "alien1.tga");
	if ( m_textureRank == 22 )  strcpy(m_textureName, "alien2.tga");
	if ( m_textureRank == 23 )  strcpy(m_textureName, "inca1.tga");
	if ( m_textureRank == 24 )  strcpy(m_textureName, "bot1.tga");
	if ( m_textureRank == 25 )  strcpy(m_textureName, "bot2.tga");
	if ( m_textureRank == 26 )  strcpy(m_textureName, "remote.tga");
	if ( m_textureRank == 27 )  strcpy(m_textureName, "home1.tga");
	if ( m_textureRank == 28 )  strcpy(m_textureName, "factory1.tga");
	if ( m_textureRank == 29 )  strcpy(m_textureName, "factory2.tga");
	if ( m_textureRank == 30 )  strcpy(m_textureName, "factory3.tga");
	if ( m_textureRank == 31 )  strcpy(m_textureName, "box.tga");
	if ( m_textureRank == 32 )  strcpy(m_textureName, "road1.tga");
	if ( m_textureRank == 33 )  strcpy(m_textureName, "pub01.tga");
	if ( m_textureRank == 34 )  strcpy(m_textureName, "pub02.tga");
	if ( m_textureRank == 35 )  strcpy(m_textureName, "pub03.tga");
	if ( m_textureRank == 36 )  strcpy(m_textureName, "pub04.tga");
	if ( m_textureRank == 37 )  strcpy(m_textureName, "pub05.tga");
	if ( m_textureRank == 38 )  strcpy(m_textureName, "pub06.tga");
	if ( m_textureRank == 39 )  strcpy(m_textureName, "pub07.tga");
	if ( m_textureRank == 40 )  strcpy(m_textureName, "pub08.tga");
	if ( m_textureRank == 41 )  strcpy(m_textureName, "pub09.tga");
	if ( m_textureRank == 42 )  strcpy(m_textureName, "pub10.tga");
	if ( m_textureRank == 43 )  strcpy(m_textureName, "support.tga");
	if ( m_textureRank == 44 )  strcpy(m_textureName, "search.tga");
	if ( m_textureRank == 45 )  strcpy(m_textureName, "plant.tga");
	if ( m_textureRank == 46 )  strcpy(m_textureName, "vegetal.tga");
	if ( m_textureRank == 47 )  strcpy(m_textureName, "human.tga");

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


