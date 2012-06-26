// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// model.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
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



#define MAX_COLORS      9

static float table_color[MAX_COLORS*3] =
{
    1.0f,   1.0f,   1.0f,   // white
    1.0f,   0.0f,   0.0f,   // red
    0.0f,   1.0f,   0.0f,   // green
    0.0f,   0.6f,   1.0f,   // blue
    1.0f,   1.0f,   0.0f,   // yellow
    0.0f,   1.0f,   1.0f,   // cyan
    1.0f,   0.0f,   1.0f,   // magenta
    0.3f,   0.3f,   0.3f,   // grey
    0.0f,   0.0f,   0.0f,   // black
};


#define MAX_STATES      10

static int table_state[MAX_STATES] =
{
    D3DSTATENORMAL,
    D3DSTATEPART1,
    D3DSTATEPART2,
    D3DSTATEPART3,
    D3DSTATEPART4,
    D3DSTATE2FACE,  // #5
    D3DSTATETTw,
    D3DSTATETTb,
    D3DSTATETTw|D3DSTATE2FACE,  // #8
    D3DSTATETTb|D3DSTATE2FACE,  // #9
};


#define MAX_NAMES       23




// Object's constructor.

CModel::CModel(CInstanceManager* iMan)
{
    m_iMan = iMan;

    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);

    m_modFile = new CModFile(m_iMan);
    m_triangleTable = m_modFile->RetTriangleList();

    m_textureRank = 0;
    strcpy(m_textureName, "lemt.tga");
    m_color = 0;
    m_state = 0;
    m_textureMode = 0;
    m_textureRotate = 0;
    m_bTextureMirrorX = FALSE;
    m_bTextureMirrorY = FALSE;
    m_texturePart = 0;
    TexturePartUpdate();

    m_bDisplayTransparent = FALSE;
    m_bDisplayOnlySelection = FALSE;
    InitView();

    m_triangleSel1 = 0;
    m_triangleSel2 = 0;

    m_mode = 1;
    m_oper = 'P';

    m_secondTexNum = 0;
    m_secondSubdiv = 1;
    m_secondOffsetU = 0;
    m_secondOffsetV = 0;

    m_min = 0.0f;
    m_max = 1000000.0f;
}

// Object's destructor.

CModel::~CModel()
{
    delete m_modFile;
}


// You must call this procedure before changing the model interactively.

void CModel::StartUserAction()
{
    Event       event;
    FPOINT      pos, dim;
    CButton*    pb;

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

//? m_modFile->ReadModel("objects\\io.mod");
    DeselectAll();
    CurrentInit();

    ZeroMemory(&event, sizeof(Event));
    EventFrame(event);

    m_engine->LoadAllTexture();
    UpdateInfoText();
}

// You must call this procedure after modifing the model interactively.

void CModel::StopUserAction()
{
    m_interface->DeleteControl(EVENT_EDIT1);
    m_interface->DeleteControl(EVENT_EDIT2);
    m_interface->DeleteControl(EVENT_EDIT3);
    m_interface->DeleteControl(EVENT_EDIT4);
    m_interface->DeleteControl(EVENT_EDIT5);
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


// Updates the​editable values for mapping textures.

void CModel::PutTextureValues()
{
    CEdit*  pe;
    char    s[100];
    int     value;

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

// Takes the editable values for mapping textures.

void CModel::GetTextureValues()
{
    CEdit*  pe;
    char    s[100];
    int     value;

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


// Gives the model name.

void CModel::GetModelName(char *buffer)
{
    CEdit*  pe;
    char    s[100];

    pe = (CEdit*)m_interface->SearchControl(EVENT_EDIT1);
    if ( pe == 0 )
    {
        strcpy(buffer, "objects\\io.mod");
    }
    else
    {
        pe->GetText(s, 100);
        sprintf(buffer, "objects\\%s.mod", s);
    }
}

// Gives the model name.

void CModel::GetDXFName(char *buffer)
{
    CEdit*  pe;
    char    s[100];

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

// Gives the model name.

void CModel::GetScriptName(char *buffer)
{
    CEdit*  pe;
    char    s[100];

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

// Indicates whether the edition name has focus.

BOOL CModel::IsEditFocus()
{
    CEdit*  pe;

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


// Management of an event.

BOOL CModel::EventProcess(const Event &event)
{
    char    s[100];
    int     first, last;

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
            if ( event.param == VK_ADD )  // numpad?
            {
                if ( event.keyState & KS_SHIFT )  CurrentSelect(TRUE);
                CurrentSearchNext(+1, (event.keyState & KS_CONTROL));
            }
            if ( event.param == VK_SUBTRACT )  // least numpad?
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
                    m_color ++;
                    if ( m_color >= MAX_COLORS )  m_color = 0;
                    UpdateInfoText();
                    ColorSelect();
                }
                if ( event.param == 'J' )
                {
                    StateSelect();
                }
                if ( event.param == 'K' )
                {
                    m_state ++;
                    if ( m_state >= MAX_STATES )  m_state = 0;
                    UpdateInfoText();
                    StateSelect();
                }
                if ( event.param == 'M' )
                {
                    m_textureMode ++;
                    if ( m_textureMode > 3 )  m_textureMode = 0;
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
                    TextureRankChange(+1);
                    UpdateInfoText();
                }
                if ( event.param == 'P' )
                {
                    TexturePartChange(+1);
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
                    FPOINT  ti, ts;
                    ti.x =  0.00f;
                    ti.y =  0.00f;
                    ts.x =  0.00f;
                    ts.y =  0.00f;
                    MappingSelect(m_textureMode, m_textureRotate,
                                  m_bTextureMirrorX, m_bTextureMirrorY,
                                  ti, ts, "");
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
            m_modFile->ReadModel(s, TRUE, FALSE);  // standard read with borders
            DeselectAll();
            CurrentInit();
            EventFrame(event);
            m_engine->LoadAllTexture();
            break;

        case EVENT_BUTTON4:  // add ?
            GetModelName(s);
            first = m_modFile->RetTriangleUsed();
            m_modFile->AddModel(s, first, TRUE, FALSE);  // standard read with borders
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
            m_oper = 'P';
            break;
        case EVENT_BUTTON11:  // rotate ?
            m_oper = 'R';
            break;
        case EVENT_BUTTON12:  // zoom ?
            m_oper = 'Z';
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


// Drives the model.

BOOL CModel::EventFrame(const Event &event)
{
    D3DMATERIAL7    matCurrent, matCurrenti, matCurrents, matTrans;
    D3DMATERIAL7*   pMat;
    D3DVERTEX2      vertex[3];
    char            texName2[20];
    int             i, used, objRank, state;

    m_time += event.rTime;

    m_engine->FlushObject();
    objRank = m_engine->CreateObject();

    ZeroMemory(&matCurrent, sizeof(D3DMATERIAL7));
    matCurrent.diffuse.r = 1.0f;
    matCurrent.diffuse.g = 0.0f;
    matCurrent.diffuse.b = 0.0f;  // red
    matCurrent.ambient.r = 0.5f;
    matCurrent.ambient.g = 0.5f;
    matCurrent.ambient.b = 0.5f;

    ZeroMemory(&matCurrents, sizeof(D3DMATERIAL7));
    matCurrents.diffuse.r = 1.0f;
    matCurrents.diffuse.g = 1.0f;
    matCurrents.diffuse.b = 0.0f;  // yellow
    matCurrents.ambient.r = 0.5f;
    matCurrents.ambient.g = 0.5f;
    matCurrents.ambient.b = 0.5f;

    ZeroMemory(&matCurrenti, sizeof(D3DMATERIAL7));
    matCurrenti.diffuse.r = 0.0f;
    matCurrenti.diffuse.g = 0.0f;
    matCurrenti.diffuse.b = 1.0f;  // blue
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
                matTrans.diffuse.a = 0.1f;  // very transparent
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

        if ( m_bDisplayTransparent &&  // draws inside?
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


// Gives a vertex.

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

// Modifies a vertex.

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

// Smoothed normals selected triangles.

void CModel::SmoothSelect()
{
    char*       bDone;
    int         index[100];
    int         used, i, j, rank;
    D3DVERTEX2  vi, vj;
    D3DVECTOR   sum;

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
            if ( vj.x == vi.x &&
                 vj.y == vi.y &&
                 vj.z == vi.z )
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


// Cast normals selected triangles.

void CModel::PlaneSelect()
{
    D3DVECTOR   p1, p2, p3, n;
    int         used, i;

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


// Change the color of the selected triangles.

void CModel::ColorSelect()
{
    int         used, i;

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

// Change the status of selected triangles.

void CModel::StateSelect()
{
    int         used, i;

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

// Moves the selection.

void CModel::MoveSelect(D3DVECTOR move)
{
    if ( m_oper == 'Z' )
    {
             if ( move.x == +1 )  move.x = 1.1f;
        else if ( move.x == -1 )  move.x = 1.0f/1.1f;
        else                      move.x = 1.0f;
             if ( move.y == +1 )  move.y = 1.1f;
        else if ( move.y == -1 )  move.y = 1.0f/1.1f;
        else                      move.y = 1.0f;
             if ( move.z == +1 )  move.z = 1.1f;
        else if ( move.z == -1 )  move.z = 1.0f/1.1f;
        else                      move.z = 1.0f;
    }
    if ( m_oper == 'R' )
    {
#if 0
             if ( move.x == +1 )  move.x =  5.0f*PI/180.0f;
        else if ( move.x == -1 )  move.x = -5.0f*PI/180.0f;
             if ( move.y == +1 )  move.y =  5.0f*PI/180.0f;
        else if ( move.y == -1 )  move.y = -5.0f*PI/180.0f;
             if ( move.z == +1 )  move.z =  5.0f*PI/180.0f;
        else if ( move.z == -1 )  move.z = -5.0f*PI/180.0f;
#else
             if ( move.x == +1 )  move.x =  45.0f*PI/180.0f;
        else if ( move.x == -1 )  move.x = -45.0f*PI/180.0f;
             if ( move.y == +1 )  move.y =  45.0f*PI/180.0f;
        else if ( move.y == -1 )  move.y = -45.0f*PI/180.0f;
             if ( move.z == +1 )  move.z =  45.0f*PI/180.0f;
        else if ( move.z == -1 )  move.z = -45.0f*PI/180.0f;
#endif
    }

    OperSelect(move, m_oper);
}

// Moves the selection.

void CModel::OperSelect(D3DVECTOR move, char oper)
{
    FPOINT      rot;
    int         used, i;

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
                if ( move.x != 0 )
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
                if ( move.y != 0 )
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
                if ( move.z != 0 )
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

// Performs a build script.

void CModel::ReadScript(char *filename)
{
    FILE*       file = NULL;
    char        line[200];
    char        name[200];
    char        buffer[200];
    int         i, first, last;
    D3DVECTOR   move;
    BOOL        bFirst = TRUE;

    file = fopen(filename, "r");
    if ( file == NULL )  return;

    while ( fgets(line, 200, file) != NULL )
    {
        for ( i=0 ; i<200 ; i++ )
        {
            if ( line[i] == '\t' )  line[i] = ' ';  // replace tab by space
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
            move *= PI/180.0f;  // degrees -> radians
            OperSelect(move, 'R');

            move = OpDir(line, "pos");
            OperSelect(move, 'P');
        }
    }

    fclose(file);
}



// Computes the bbox of selected triangles.

void CModel::BBoxCompute(D3DVECTOR &min, D3DVECTOR &max)
{
    D3DVERTEX2  vertex;
    int         used, i;

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

// Returns the gravity center of the selection.

D3DVECTOR CModel::RetSelectCDG()
{
    D3DVECTOR   min, max, cdg;

    BBoxCompute(min, max);

    cdg.x = (min.x+max.x)/2.0f;
    cdg.y = (min.y+max.y)/2.0f;
    cdg.z = (min.z+max.z)/2.0f;

    return cdg;
}

// Returns the normal vector of the selection.

D3DVECTOR CModel::RetSelectNormal()
{
    D3DVECTOR   p1, p2, p3, n;

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

// Maps a texture onto the selected triangles.

BOOL CModel::IsMappingSelectPlausible(D3DMaping D3Dmode)
{
    D3DVERTEX2  vertex[3];
    D3DVECTOR   min, max;
    FPOINT      a, b, ti, ts;
    float       au, bu, av, bv;
    int         used, i, j;

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

// Maps a texture onto the selected triangles.

void CModel::MappingSelect(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
                           FPOINT ti, FPOINT ts, char *texName)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   min, max;
    FPOINT      a, b;
    D3DMaping   D3Dmode;
    float       au, bu, av, bv;
    int         used, i;
    BOOL        bPlausible[3];

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

    if ( !bMirrorY )  // reverse test!
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

// Maps a texture onto the selected triangles.

void CModel::MappingSelectSpherical(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
                                    FPOINT ti, FPOINT ts, char *texName)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   min, max, center, dim, p;
    float       radius, k, u, v;
    int         used, i;

    BBoxCompute(min, max);
    center = (min+max)/2.0f;
    dim    = (max-min)/2.0f;
    radius = Min(dim.x, dim.y, dim.z);

    if ( bMirrorX )
    {
        Swap(ti.x, ts.x);
    }

    if ( !bMirrorY )  // reverse test!
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

// Seeking the center of a group of points.

D3DVECTOR CModel::RetMappingCenter(D3DVECTOR pos, D3DVECTOR min)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   center, p;
    int         used, i, nb;

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

// Maps a texture onto the selected triangles.

void CModel::MappingSelectCylindrical(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
                                      FPOINT ti, FPOINT ts, char *texName)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   min, max, center, local, dim, p, pp, box;
    float       radius, u, v;
    int         used, i;

    BBoxCompute(min, max);
    center = (min+max)/2.0f;
    dim    = (max-min)/2.0f;
    radius = Min(dim.x, dim.y, dim.z);

    if ( bMirrorX )
    {
        Swap(ti.x, ts.x);
    }

    if ( !bMirrorY )  // reverse test!
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

//?     if ( u < PI )  u = u/PI;
//?     else           u = 2.0f-u/PI;
        u = u/(PI*2.0f);

        vertex.tu = ti.x+(ts.x-ti.x)*u;
        vertex.tv = ti.y+(ts.y-ti.y)*v;

        SetVertex(i, vertex);
    }

    SelectTerm();
}


// Maps a texture onto the selected triangles.

void CModel::MappingSelectFace(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY,
                               FPOINT ti, FPOINT ts, char *texName)
{
    D3DVERTEX2  vertex[3];
    D3DVECTOR   min, max, center, local, dim, p;
    float       radius, u[3], v[3], m[3], avg;
    int         used, i, j;

    BBoxCompute(min, max);
    center = (min+max)/2.0f;
    dim    = (max-min)/2.0f;
    radius = Min(dim.x, dim.y, dim.z);

    if ( bMirrorX )
    {
        Swap(ti.x, ts.x);
    }

    if ( !bMirrorY )  // reverse test!
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

#if 0
            u[j] = RotateAngle(p.x, p.z)/(PI*2.0f)+0.5f;
            if ( u[j] > 1.0f ) u[j] -= 1.0f;
#else
            u[j] = RotateAngle(p.x, p.z)/PI;
//?         if ( u[j] > 1.0f )  u[j] = 2.0f-u[j];
            if ( u[j] > 1.0f )  u[j] -= 1.0f;
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


// Maps a secondary texture on selected triangles.

void CModel::MappingSelect2(int texNum2, int subdiv,
                            int offsetU, int offsetV,
                            BOOL bMirrorX, BOOL bMirrorY)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   min, max, center, p;
    float       u ,v;
    int         used, i;

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

// Maps a secondary texture on flat.

void CModel::MappingSelectPlane2(int mode, BOOL bMirrorX, BOOL bMirrorY)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   min, max;
    FPOINT      ti, ts, a, b;
    float       au, bu, av, bv;
    int         used, i;

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

    if ( !bMirrorY )  // reverse test!
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

// Maps a texture onto the selected triangles.

void CModel::MappingSelectSpherical2(BOOL bMirrorX, BOOL bMirrorY)
{
    D3DVERTEX2  vertex;
    D3DVECTOR   min, max, center, dim, p;
    FPOINT      ti, ts;
    float       radius, k, u, v;
    int         used, i;

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

    if ( !bMirrorY )  // reverse test!
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

// Maps a texture onto the selected triangles.

void CModel::MappingSelectMagic2(BOOL bMirrorX, BOOL bMirrorY)
{
    D3DVERTEX2  vertex, v[3];
    D3DVECTOR   min, max, au, bu, av, bv, n;
    FPOINT      ti, ts;
    int         used, i, mode;

    ti = FPOINT(0.0f, 0.0f);
    ts = FPOINT(1.0f, 1.0f);

    BBoxCompute(min, max);

    if ( bMirrorX )
    {
        Swap(ti.x, ts.x);
    }

    if ( !bMirrorY )  // reverse test!
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


// Seeks the next triangle.

int CModel::SearchNext(int rank, int step)
{
    int     max, i;

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

// Seeks all the triangles belonging to the same plane.

int CModel::SearchSamePlane(int first, int step)
{
    D3DVECTOR   vFirst[3], vNext[3];
    int         last, i;

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

        if ( !IsSamePlane(vFirst, vNext) )  // other plan?
        {
            return last;
        }
    }
    return first;
}

// Seeks the next triangle.

void CModel::CurrentSearchNext(int step, BOOL bControl)
{
    if ( step > 0 )  // forward?
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
    if ( step < 0 )  // back?
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

// Initializes the current triangles.

void CModel::CurrentInit()
{
    m_triangleSel1 = 0;
    m_triangleSel2 = SearchSamePlane(m_triangleSel1, +1);

    InitViewFromSelect();
    UpdateInfoText();
}

// Selects the current triangles.

void CModel::CurrentSelect(BOOL bSelect)
{
    int     i;

    for ( i=m_triangleSel1 ; i<=m_triangleSel2 ; i++ )
    {
        m_triangleTable[i].bSelect = bSelect;
    }
}


// Deselects all triangles.

void CModel::DeselectAll()
{
    int     used, i;

    used = m_modFile->RetTriangleUsed();
    for ( i=0 ; i<used ; i++ )
    {
        m_triangleTable[i].bSelect = FALSE;
    }
}

// Selects an area.

void CModel::SelectZone(int first, int last)
{
    int     used, i;

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

// Selects all triangles.

void CModel::SelectAll()
{
    int     used, i;

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

// Deselects all triangles.

void CModel::SelectTerm()
{
    int     used, i;

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

// Selects the triangles currents.

void CModel::DefaultSelect()
{
    int     used, i;

    used = m_modFile->RetTriangleUsed();
    for ( i=m_triangleSel1 ; i<=m_triangleSel2 ; i++ )
    {
        m_triangleTable[i].bSelect = TRUE;
    }
}



// Removes all selected triangles.

void CModel::SelectDelete()
{
    int     used ,i;

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

// Compresses all triangles.

void CModel::Compress()
{
    int     used, i, j;

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


// Change the min / max of all selected triangles.

void CModel::MinMaxChange()
{
    int     used, i;

    DefaultSelect();

    used = m_modFile->RetTriangleUsed();
    for ( i=0 ; i<used ; i++ )
    {
        if ( !m_triangleTable[i].bSelect )  continue;

        m_triangleTable[i].min = m_min;
        m_triangleTable[i].max = m_max;
    }
}


// Initializes the point of view.

void CModel::InitView()
{
    m_viewHeight =  5.0f;
    m_viewDist   = 50.0f;
    m_viewAngleH =  0.0f;
    m_viewAngleV =  0.0f;
}

// Initializes the point of view to see the selected triangles.

void CModel::InitViewFromSelect()
{
#if 0
    D3DVECTOR   n;
    float       h,v;

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

// Updates the parameters for the point of view.

void CModel::UpdateView()
{
    D3DVECTOR   eye, lookat, vUpVec;

//? lookat = RetSelectCDG();
    lookat = D3DVECTOR(0.0f, m_viewHeight, 0.0f);
    eye = RotateView(lookat, m_viewAngleH, m_viewAngleV, m_viewDist);

    vUpVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
    m_engine->SetViewParams(eye, lookat, vUpVec, 10.0f);
    m_engine->SetRankView(0);
}

// Moves the point of view.

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
        if ( m_viewDist > 300.0f )  m_viewDist = 300.0f;
    }
}



// Updates the text information.

void CModel::UpdateInfoText()
{
    char        info[100];

    if ( m_mode == 1 )
    {
        sprintf(info, "[1] V:color=%d K:state=%d  Sel=%d..%d (T=%d)",
                m_color, m_state,
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



static int tablePartT[] =   // lemt.tga
{
    192,   0, 256,  32,  // track profile
      0,  64, 128, 128,  // wheels for track
      0,   0, 128,  64,  // profile
     90,   0, 128,  28,  // pivot trainer
    128,   0, 192,  44,  // chest front
    128,  44, 192,  58,  // shell
    128,  58, 192,  87,  // back chest
    128,  87, 192, 128,  // back shell
    128, 128, 192, 144,  // sub back shell
      0, 128,  32, 152,  // rear fender
      0, 152,  32, 182,  // fender middle
      0, 182,  32, 256,  // front fender
     32, 128, 112, 176,  // wing
    224,  48, 232,  64,  // thrust tunnel
    192,  32, 224,  64,  // fire under reactor
    224,  32, 256,  48,  // foot
    192,  64, 256, 128,  // sensor
    192, 128, 224, 176,  // battery holder
    192, 216, 248, 248,  // cannon board
    220, 216, 222, 245,  // cannon board
     64, 176, 128, 224,  // top cannon
    128, 152, 192, 160,  // external cannon
    128, 144, 192, 152,  // interior cannon
    192, 176, 224, 192,  // small cannon
    128, 236, 192, 256,  // cannon organic
    214, 192, 224, 216,  // crosshair
    224, 128, 248, 152,  // articulation
    128, 192, 192, 214,  // piston board
    128, 214, 192, 236,  // piston front
    192, 192, 214, 214,  // piston edge
    128, 192, 161, 214,  // small piston board
     32, 176,  64, 198,  // radar piston
    128, 160, 160, 192,  // wheel
    232,  48, 255,  56,  // tire profile
    240, 152, 248, 216,  // vertical hatching
    248, 192, 256, 256,  // battery
    224, 152, 240, 168,  // rock
    144,  80, 176, 112,  // nuclear
    140,  76, 180, 116,  // large nuclear
    144,  80, 152,  88,  // yellow nuclear
    224, 168, 240, 192,  // cap resolution C
    224, 192, 240, 210,  // back resolution C
     32, 224,  96, 235,  // arm resolution C
     32, 235,  96, 246,  // arm resolution C
    161,   1, 164,   4,  // blank
    168,   1, 171,   4,  // medium gray
    154,   1, 157,   4,  // dark gray uniform
    147,   1, 150,   4,  // blue unifrom
    114, 130, 118, 134,  // red unifrom
    121, 130, 125, 134,  // green uniform
    114, 137, 118, 141,  // yellow uniform
    121, 137, 125, 141,  // violet uniform
    -1
};

static int tablePartR[] =   // roller.tga
{
      0,   0, 128,  52,  // wheels for track
     48, 137, 128, 201,  // catalytic radiator
      0,  52,  32,  84,  // front radiator
     32,  52,  43,  84,  // back radiator
      0,  84,  96, 137,  // large catalytic
    128,   0, 192,  85,  // front
    128, 173, 192, 256,  // back
    192,   0, 256,  42,  // over
    128,  85, 192, 109,  // catalytic pillon
    128, 109, 192, 173,  // top pillon
    192,  85, 240, 109,  // catalytic gate pillon
      0, 137,  24, 256,  // catalytic verrin
     24, 137,  48, 256,  // catalytic verrin
     48, 201, 128, 233,  // medium cannon
    192, 109, 256, 173,  // bottom cannon
    192, 173, 240, 205,  // cannon 1
    192, 173, 240, 177,  // cannon 2
     43,  52,  75,  84,  // front cannon
     48, 233, 128, 247,  // piston
     96, 105, 128, 137,  // front phazer
     96,  97, 128, 105,  // phazer cannon
     75,  52, 107,  84,  // exhaust pipe
    192, 205, 243, 256,  // nuclear power plant instruction
    192,  42, 256,  85,  // reflection glass
    -1
};

static int tablePartW[] =   // subm.tga
{
      0,   0, 128,  26,  // chenilles
      0,  26,  22, 114,  // portique 1
      0, 114,  22, 202,  // portique 2
     22,  26,  82,  56,  // c�t� hublot
     22,  56,  82,  86,  // c�t� ligne rouge
     22,  86,  82, 116,  // c�t� simple
     22, 116,  82, 146,  // avant/arri�re
     22, 146,  82, 176,  // avant/arri�re + phare
    132,  82, 196, 166,  // capot trainer
    132, 166, 196, 177,  // capot trainer
    132, 177, 196, 188,  // capot trainer
      0, 224,  96, 256,  // c�t� trainer
     30, 224,  48, 256,  // arri�re trainer
    136, 240, 216, 256,  // barri�re courte
     96, 240, 256, 256,  // barri�re longue
    128,   0, 160,  32,  // black-box 1
    160,   0, 192,  32,  // black-box 2
    192,   0, 224,  32,  // black-box 3
    224, 105, 256, 137,  // TNT 1
    224, 137, 256, 169,  // TNT 2
     82,  32, 146,  82,  // factory r�solution C
    146,  32, 210,  82,  // factory r�solution C
    224,   0, 256, 105,  // tower r�solution C
     82,  82, 132, 150,  // research r�solution C
    199, 169, 256, 233,  // sac r�solution C
    106, 150, 130, 214,  // cl� A
     82, 150, 106, 214,  // cl� B
    132, 188, 196, 212,  // cl� C
    132, 212, 196, 236,  // cl� D
    210,  32, 224,  46,  // gris
     56, 176,  82, 224,  // sol coffre-fort
    -1
};

static int tablePartDr[] =  // drawer.tga
{
    128,   0, 134,   6,  // bleu
    128,   6, 134,  12,  // gris fonc�
    128,  12, 134,  18,  // gris clair
      0,   0, 128,  32,  // roues chenille
    192,   0, 256,  32,  // profil chenille
    140,   0, 160,   8,  // profil phare
    160,   0, 192,  32,  // face phare
      0,  32, 160,  48,  // hachure
    160,  32, 192,  48,  // c�t�
      0,  48,  96,  96,  // tableau de bord
     96,  48, 192, 112,  // radiateur
    192,  32, 256, 112,  // grille lat�rale
    192, 112, 256, 128,  // capot
      0,  96,   8, 160,  // chassis
      8,  96,  96, 104,  // axe chenilles
      8, 104,  16, 160,  // axe carrousel
     16, 128,  24, 160,  // flan support
    224, 128, 256, 160,  // rotule
     24, 104,  32, 160,  // bocal (18)
     32, 104,  40, 160,  // bocal
     40, 104,  48, 160,  // bocal
     24, 152,  48, 160,  // bocal fond
      0, 240,  32, 256,  // crayon 1: couleur (22)
      0, 160,  32, 192,  // crayon 1: dessus
      0, 192,  32, 256,  // crayon 1: pointe
     32, 240,  64, 256,  // crayon 2: couleur
     32, 160,  64, 192,  // crayon 2: dessus
     32, 192,  64, 256,  // crayon 2: pointe
     64, 240,  96, 256,  // crayon 3: couleur
     64, 160,  96, 192,  // crayon 3: dessus
     64, 192,  96, 256,  // crayon 3: pointe
     96, 240, 128, 256,  // crayon 4: couleur
     96, 160, 128, 192,  // crayon 4: dessus
     96, 192, 128, 256,  // crayon 4: pointe
    128, 240, 160, 256,  // crayon 5: couleur
    128, 160, 160, 192,  // crayon 5: dessus
    128, 192, 160, 256,  // crayon 5: pointe
    160, 240, 192, 256,  // crayon 6: couleur
    160, 160, 192, 192,  // crayon 6: dessus
    160, 192, 192, 256,  // crayon 6: pointe
    192, 240, 224, 256,  // crayon 7: couleur
    192, 160, 224, 192,  // crayon 7: dessus
    192, 192, 224, 256,  // crayon 7: pointe
    224, 240, 256, 256,  // crayon 8: couleur
    224, 160, 256, 192,  // crayon 8: dessus
    224, 192, 256, 256,  // crayon 8: pointe
    -1
};

static int tablePartKi[] =  // kid.tga
{
      0,   0, 128,  53,  // ciseaux
    128,   0, 256, 128,  // CD
      0,   0,   8,   8,  // livre 1: fond
      8,   0,  16,   8,  // livre 2: fond
     16,   0,  24,   8,  // livre: fond
     24,   0,  32,   8,  // livre: fond
     32,   0,  40,   8,  // livre: fond
     40,   0,  48,   8,  // livre: fond
      0,  53,  22, 138,  // livre 1: tranche
     22,  53,  86, 138,  // livre 1: face
      0, 138,  22, 224,  // livre 2: tranche
     22, 138,  86, 224,  // livre 2: face
     86,  53,  94,  85,  // livre: pages
     94,  53, 110, 139,  // livre: tranche
    110,  53, 126, 139,  // livre: tranche
     86, 139, 102, 225,  // livre: tranche
    102, 139, 118, 225,  // livre: tranche
    118, 139, 134, 225,  // livre: tranche
     64,   0,  72,   8,  // fauille: fond
    155, 155, 256, 256,  // feuille: carreaux
     72,   0,  80,   8,  // lampe
     80,   0,  88,   8,  // lampe
     80,   8,  88,  16,  // ampoule
     72,   8,  80,  16,  // rayons (23)
     86,  85,  94, 139,  // lampe
      0, 224,  32, 256,  // lampe rotule
     64,   8,  72,  16,  // arrosoir: fond
    134, 128, 142, 256,  // arrosoir: corps
    142, 128, 150, 256,  // arrosoir: tuyau
    128, 225, 134, 256,  // arrosoir: int�rieur
     32, 224,  64, 256,  // arrosoir: ponneau
     56,   8,  64,  16,  // skate: roues (31)
     48,   8,  56,  16,  // skate: axes
     40,   8,  48,  16,  // skate: grip
     32,   8,  40,  16,  // skate: tranche
     24,   8,  32,  16,  // skate: dessous
    150, 128, 200, 256,  // skate: motif 1
    200, 128, 250, 256,  // skate: motif 2
     64, 224,  96, 256,  // skate: roue (38)
     96, 225, 104, 256,  // skate: amortisseur
    -1
};

static int tablePartKi2[] = // kid2.tga
{
      2,   2,  62,  62,  // coca: dessus
      0,  64,   8, 192,  // coca: flan
      8,  64,  96, 192,  // coca: logo
    128,   0, 256,  85,  // carton
    128,  85, 256,  91,  // carton tranche
    128, 128, 256, 256,  // roue
    192,  96, 256, 128,  // pneu
    184,  96, 192, 128,  // jante
    128,  96, 160, 128,  // int�rieur
    160,  96, 168, 104,  // porte bois
    160, 104, 168, 112,  // porte m�tal
    160, 112, 184, 128,  // vitre
     96,   0, 128, 256,  // bouteille: corps (12)
     64,   0,  96,  32,  // bouteille: bouchon
    168,  96, 176, 104,  // bouteille: vert
      0, 192,  96, 224,  // bois clair
      0, 224,  96, 256,  // bois fonc�
     64,  32,  96,  64,  // bateau
    168, 104, 176, 112,  // ballon (18)
    176, 104, 184, 112,  // ballon
    176,  96, 184, 104,  // int�rieur caisse
    -1
};

static int tablePartKi3[] = // kid3.tga
{
      0,   0,  32,  28,  // �crou: flan
      0,  28,  32,  44,  // �crou: profil
      0,  44,  32,  60,  // �crou: pas de vis
      0,  60,  32,  64,  // tuyau
      0,  64,  32,  68,  // tuyau
      0,  68,   8,  76,  // tuyau
      0,  76,  32, 108,  // plastic
      8,  68,  16,  76,  // saut: gris clair (7)
     16,  68,  24,  76,  // saut: gris fonc�
     24,  68,  32,  76,  // saut: gris bois
      0, 108,  32, 140,  // saut: rotule
      0, 140,  32, 144,  // saut: axe
    128,   0, 256, 128,  // saut: flan
      0, 144,   8, 152,  // basket: gris fonc� (13)
      8, 144,  16, 152,  // basket: gris clair
     16, 144,  24, 152,  // basket: gris lacets
     24, 144,  32, 152,  // basket: gris semelle
      0, 152,   8, 181,  // basket: int�rieur
      0, 181, 192, 256,  // basket: c�t�
    192, 181, 226, 256,  // basket: arri�re
     32, 135,  96, 181,  // basket: dessus (20)
     96, 168, 128, 181,  // basket: avant
      8, 152,  16, 160,  // chaise: plastique
     16, 152,  24, 160,  // chaise: m�tal
     32,   0,  64,  32,  // chaise: roue
      8, 177,  24, 181,  // chaise: roue
    226, 181, 234, 256,  // chaise: piston (26)
     64,  96, 128, 128,  // chaise: relief
     96, 135, 128, 167,  // chaise: dessous
     32, 128, 250, 135,  // paille 1
     38, 128, 256, 135,  // paille 2
    234, 181, 242, 256,  // allumette
      8, 160,  16, 168,  // allumette (dessus)
    128, 135, 224, 181,  // panneau
    242, 135, 256, 256,  // poteau (34)
     24, 152,  32, 160,  // clou
     16, 160,  24, 168,  // tuyau m�talique
    112, 181, 192, 185,  // tuyau int�rieur
     32,  32,  48,  80,  // pas de vis
     24, 160,  32, 168,  // ventillateur: plastique (39)
     40,  80,  56,  96,  // ventillateur: plastique d�grad�
      8, 168,  16, 176,  // ventillateur: m�tal
     32,  80,  40, 112,  // ventillateur: socle 1
     64,   0,  96,  16,  // ventillateur: socle 2
     48,  32,  56,  80,  // ventillateur: socle 3
     64,  16,  96,  32,  // ventillateur: moteur flan
     96,   0, 128,  32,  // ventillateur: moteur face
    102,   6, 122,  26,  // ventillateur: socle dessus
     16, 168,  24, 176,  // pot: uni (48)
     56,  32,  64,  64,  // pot: haut
     56,  64,  64,  96,  // pot: bas
     64,  32, 128,  96,  // pot: terre
    -1
};

static int tablePartF[] =   // factory.tga
{
      0,   0, 152, 152,  // plancher octogonal fabrique
     50,  50, 102, 102,  // dessus pile
      0, 152, 128, 252,  // avant
    128, 152, 256, 252,  // arri�re
    152,  28, 225, 128,  // c�t�
    152,  28, 176, 128,  // c�t� partiel
    152,   0, 216,  16,  // hachures
    236,   0, 256,  40,  // axe
    152, 128, 224, 152,  // support cible
    -1
};

static int tablePartD[] =   // derrick.tga
{
      0,   0,  64,  32,  // grand c�t�
     64,   0,  96,  24,  // petit c�t�
     96,   0, 136,  24,  // attention
      0,  32,   8, 160,  // tube 1
      8,  32,  16,  96,  // tube 2
     16,  32,  24, 160,  // pilier
     24,  32,  32, 160,  // tige foret
     32,  32,  40, 160,  // tige destructeur
      8,  96,  16, 128,  // foret
    136,   0, 256, 120,  // plancher octogonal station de recharge
     40,  32,  64,  56,  // cube m�tal
     64,  24, 128,  48,  // c�t� tour haut
     64,  48, 128, 229,  // c�t� tour bas
    136, 120, 256, 240,  // int�rieur usine
      0, 160,  64, 224,  // to�t usine
    -1
};

static int tablePartC[] =   // convert.tga
{
      0,   0, 120, 120,  // plancher octogonal convertisseur
      0, 120, 128, 176,  // grand c�t�
    128, 120, 192, 176,  // petit c�t�
    192, 120, 256, 184,  // couvercle convertisseur
    120,   0, 216,  64,  // face trianble
    216,   0, 248,  64,  // c�t� triangle
    120,  64, 160,  84,  // axe
      0, 141, 128, 176,  // recherche: base
      0, 176, 128, 214,  // recherche: haut
      0, 214, 128, 252,  // recherche: haut (!)
    174,  64, 190, 120,  // recherche: montant
    190,  64, 206, 120,  // recherche: montant
    206,  64, 254,  85,  // radar
    192, 168, 256, 232,  // hachures carr�es
    248,   0, 256,  64,  // c�ne fabrique de piles
    128, 176, 192, 240,  // dessus centrale nucl�aire
    120,  85, 174, 120,  // technicien, visage
    206, 106, 256, 120,  // technicien, casquette
    160,  64, 174,  78,  // technicien, visi�re
    -1
};

static int tablePartS[] =   // search.tga
{
      0,   0, 128, 128,  // usine 1
    128,   0, 256, 128,  // usine 2
      0, 128, 128, 256,  // pile
    128, 128, 228, 240,  // support pile
    228, 128, 256, 184,  // antenne
    128, 128, 192, 160,  // contr�le 1
    128, 160, 192, 192,  // contr�le 2
    128, 192, 192, 224,  // contr�le 3
    128, 224, 192, 256,  // contr�le 4
    -1
};

static int tablePartP[] =   // plant.tga
{
      0, 160,  48, 256,  // feuille 1
      0,   0,  94, 100,  // feuille 2
     48, 156, 108, 256,  // feuille 3
     94,   0, 104, 100,  // tige 1
    185,   0, 195, 100,  // tige 2
    108, 100, 182, 256,  // foug�re
    104,   0, 144, 100,  // courge
    203,   0, 256,  83,  // armature derrick r�solution C
    -1
};

static int tablePartV[] =   // vegetal.tga
{
      0,   0,  94, 100,  // racine
    186,   0, 256, 256,  // tronc
    162,   0, 168, 128,  // mat drapeau bleu
    168,   0, 174, 128,  // mat drapeau rouge
    174,   0, 180, 128,  // mat drapeau vert
    180,   0, 186, 128,  // mat drapeau jaune
    180, 128, 186, 256,  // mat drapeau violet
     94,   0, 107,  32,  // drapeau bleu
    107,   0, 120,  32,  // drapeau rouge
    120,   0, 133,  32,  // drapeau vert
    133,   0, 146,  32,  // drapeau jaune
    146,   0, 159,  32,  // drapeau violet
     94,  64, 126,  96,  // verre 1
    126,  64, 158,  86,  // verre 2
    128, 128, 180, 144,  // verre 3a
    128, 144, 180, 160,  // verre 3b
    128,  94, 162, 128,  // verre 4
      0, 100,  32, 228,  // champignon 1
     32, 100,  48, 228,  // champignon 1
     48, 100, 112, 228,  // champignon 2
    112, 100, 128, 228,  // champignon 2
    128, 160, 180, 212,  // tronc (21)
    -1
};

static int tablePartM[] =   // mother.tga
{
      0,   0, 128, 128,  // corps arri�re
    128,   0, 192, 128,  // corps avant
      0, 128,  64, 192,  // t�te
     64, 128, 192, 160,  // pince ext.
     64, 160, 192, 192,  // pince int.
      0, 192,  64, 256,  // mire
    -1
};

static int tablePartA[] =   // ant.tga
{
      0,   0,  64,  64,  // queue
      0,  96, 128, 160,  // queue abeille
     64,   0, 128,  64,  // corps
    128,   0, 192,  64,  // t�te
      0,  64,  64,  72,  // patte
      0,  72,  64,  80,  // antenne
     64,  64, 150,  96,  // queue ver
    150,  64, 182,  96,  // corps ver
    182,  64, 256,  96,  // t�te ver
    224,  32, 256,  64,  // articulation ver
    128,  96, 220, 160,  // aile
      0,  80,  16,  96,  // oeil
    200,   0, 208,   8,  // vert clair
    200,   8, 208,  16,  // vert fonc�
      0, 160,  64, 224,  // corps araign�e
     64, 160, 128, 192,  // t�te araign�e
    208,   0, 216,  64,  // patte araign�e
    216,   0, 224,  32,  // patte araign�e
    224,   0, 256,   8,  // antenne araign�e
    192,   0, 200,   8,  // brun clair
    192,   8, 200,  16,  // brun fonc�
    128, 160, 256, 256,  // SatCom
    -1
};

static int tablePartH[] =   // human.tga
{
      0,   0,  64,  64,  // vissi�re
     64,   0,  96,  64,  // cuisse
     96,   0, 128,  64,  // jambe
    128,   0, 192,  32,  // bras
    128,  32, 192,  64,  // avant-bras
      0,  64, 128, 224,  // ventre
    128,  64, 256, 224,  // dos
     64, 224, 112, 256,  // dessus pied
    144, 224, 168, 240,  // dessous pied
    112, 224, 144, 240,  // c�t� pied
    112, 224, 128, 240,  // c�t� pied
      0, 224,  64, 256,  // gant
    168, 224, 200, 256,  // oreille
    112, 240, 144, 256,  // ligne casque
    200, 224, 208, 256,  // int�rieur coup
    240,   0, 244,  64,  // bombone orange
    244,   0, 248,  64,  // bombone orange (reflet)
    248,   0, 252,  64,  // bombone bleu
    252,   0, 256,  64,  // bombone bleu (reflet)
    144, 240, 156, 256,  // gris habit
    156, 240, 168, 256,  // gris articulation
//? 208, 224, 256, 256,  // SatCom
    192,   0, 240,  64,  // quartz
    -1
};

static int tablePartG[] =   // apollo.tga
{
      0,   0,  64,  64,  // rev�tement LEM
     64,   0, 128,  64,  // rev�tement LEM
    128,   8, 136, 128,  // pied
      0,  64,  64, 128,  // roue
    136,  24, 152,  44,  // profil pneu
    136,   8, 160,  24,  // garde boue
     64,  64, 128, 128,  // si�ge
     64, 128, 128, 192,  // si�ge
     64, 192, 128, 212,  // si�ge
    128, 128, 240, 192,  // moteur
      0, 192,  28, 256,  // moteur
     32, 128,  60, 256,  // moteur
    224,   0, 256, 128,  // avant
    206,   0, 224, 128,  // avant
    136,  44, 168,  62,  // avant
     64, 212, 108, 256,  // panneau de commande
    198,   0, 206, 128,  // mat
    190,  64, 198, 128,  // mat
    160,   8, 176,  24,  // cam�ra
    176,   8, 192,  24,  // moyeu
    136,  64, 168,  96,  // module
    168,  64, 190,  96,  // module
    136,  96, 168, 128,  // module
    128, 192, 230, 252,  // drapeau
      0, 128,  32, 192,  // antenne
    128,   0, 136,   8,  // jaune
    136,   0, 144,   8,  // beige
    144,   0, 152,   8,  // brun
    168,   0, 176,   8,  // gris tr�s clair
    152,   0, 160,   8,  // gris clair
    160,   0, 168,   8,  // gris fonc�
    -1
};

static int tablePartB[] =   // base1.tga
{
      0,   0,  80, 256,  // int�rieur porte
     80,   0,  88, 256,  // tranche porte
    116,   0, 180,  64,  // coiffe 1
    116,  64, 180, 102,  // coiffe 2
    180,   0, 244,  37,  // base
    180,  37, 196, 101,  // support
     88,   0, 116, 256,  // colonne
    212,  37, 256, 128,  // suppl�ment
    128, 128, 256, 256,  // 1/4 du sol
    196,  37, 212,  53,  // gris fonc�
    196,  53, 212,  69,  // gris clair
    -1
};

static int tablePartCe[] =  // cellar01.tga
{
      0, 128,  64, 192,  // briques
     64, 128, 128, 192,  // briques
    128, 128, 192, 192,  // briques
    192, 128, 256, 192,  // briques
    -1
};

static int tablePartFa[] =  // face01.tga
{
      0,   0, 256, 256,  // visage
    -1
};

// Retourne le pointeur la table.

int* CModel::RetTextureTable()
{
    if ( m_textureRank ==  0 )  return tablePartT;
    if ( m_textureRank ==  1 )  return tablePartR;
    if ( m_textureRank ==  2 )  return tablePartW;
    if ( m_textureRank ==  3 )  return tablePartDr;
    if ( m_textureRank ==  4 )  return tablePartKi;
    if ( m_textureRank ==  5 )  return tablePartKi2;
    if ( m_textureRank ==  6 )  return tablePartKi3;
    if ( m_textureRank ==  7 )  return tablePartF;
    if ( m_textureRank ==  8 )  return tablePartD;
    if ( m_textureRank ==  9 )  return tablePartC;
    if ( m_textureRank == 10 )  return tablePartS;
    if ( m_textureRank == 11 )  return tablePartP;
    if ( m_textureRank == 12 )  return tablePartV;
    if ( m_textureRank == 13 )  return tablePartM;
    if ( m_textureRank == 14 )  return tablePartA;
    if ( m_textureRank == 15 )  return tablePartH;
    if ( m_textureRank == 16 )  return tablePartG;
    if ( m_textureRank == 17 )  return tablePartB;
    if ( m_textureRank == 18 )  return tablePartCe;
    if ( m_textureRank == 19 )  return tablePartFa;
    if ( m_textureRank == 20 )  return tablePartFa;
    if ( m_textureRank == 21 )  return tablePartFa;
    if ( m_textureRank == 22 )  return tablePartFa;
    return 0;
}

// Updates the part of texture.

void CModel::TexturePartUpdate()
{
    int     *table;

    table = RetTextureTable();
    if ( table == 0 )  return;

    m_textureInf.x = (table[m_texturePart*4+0]+0.5f)/256.0f;
    m_textureInf.y = (table[m_texturePart*4+1]+0.5f)/256.0f;
    m_textureSup.x = (table[m_texturePart*4+2]-0.5f)/256.0f;
    m_textureSup.y = (table[m_texturePart*4+3]-0.5f)/256.0f;

    PutTextureValues();
}

// Changes the texture.

void CModel::TextureRankChange(int step)
{
    m_textureRank += step;

    if ( m_textureRank >= MAX_NAMES )  m_textureRank = 0;
    if ( m_textureRank <  0         )  m_textureRank = MAX_NAMES-1;

    if ( m_textureRank ==  0 )  strcpy(m_textureName, "lemt.tga");
    if ( m_textureRank ==  1 )  strcpy(m_textureName, "roller.tga");
    if ( m_textureRank ==  2 )  strcpy(m_textureName, "subm.tga");
    if ( m_textureRank ==  3 )  strcpy(m_textureName, "drawer.tga");
    if ( m_textureRank ==  4 )  strcpy(m_textureName, "kid.tga");
    if ( m_textureRank ==  5 )  strcpy(m_textureName, "kid2.tga");
    if ( m_textureRank ==  6 )  strcpy(m_textureName, "kid3.tga");
    if ( m_textureRank ==  7 )  strcpy(m_textureName, "factory.tga");
    if ( m_textureRank ==  8 )  strcpy(m_textureName, "derrick.tga");
    if ( m_textureRank ==  9 )  strcpy(m_textureName, "convert.tga");
    if ( m_textureRank == 10 )  strcpy(m_textureName, "search.tga");
    if ( m_textureRank == 11 )  strcpy(m_textureName, "plant.tga");
    if ( m_textureRank == 12 )  strcpy(m_textureName, "vegetal.tga");
    if ( m_textureRank == 13 )  strcpy(m_textureName, "mother.tga");
    if ( m_textureRank == 14 )  strcpy(m_textureName, "ant.tga");
    if ( m_textureRank == 15 )  strcpy(m_textureName, "human.tga");
    if ( m_textureRank == 16 )  strcpy(m_textureName, "apollo.tga");
    if ( m_textureRank == 17 )  strcpy(m_textureName, "base1.tga");
    if ( m_textureRank == 18 )  strcpy(m_textureName, "cellar01.tga");
    if ( m_textureRank == 19 )  strcpy(m_textureName, "face01.tga");
    if ( m_textureRank == 20 )  strcpy(m_textureName, "face02.tga");
    if ( m_textureRank == 21 )  strcpy(m_textureName, "face03.tga");
    if ( m_textureRank == 22 )  strcpy(m_textureName, "face04.tga");

    m_texturePart = 0;
}

// Changes the part of texture.

void CModel::TexturePartChange(int step)
{
    int     *table;

    table = RetTextureTable();
    if ( table == 0 )  return;

    m_texturePart ++;

    if ( table[m_texturePart*4] == -1 )
    {
        m_texturePart = 0;
    }

    TexturePartUpdate();
}


