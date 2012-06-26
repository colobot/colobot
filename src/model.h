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

// model.h

#ifndef _MODEL_H_
#define _MODEL_H_


#include "struct.h"


class CInstanceManager;
class CD3DEngine;
class CModFile;
class CInterface;



class CModel
{
public:
    CModel(CInstanceManager* iMan);
    ~CModel();

    void        StartUserAction();
    void        StopUserAction();

    BOOL        EventProcess(const Event &event);

    void        InitView();
    void        InitViewFromSelect();
    void        UpdateView();
    void        ViewMove(const Event &event, float speed);

protected:
    BOOL        EventFrame(const Event &event);
    BOOL        GetVertex(int rank, D3DVERTEX2 &vertex);
    BOOL        SetVertex(int rank, D3DVERTEX2 &vertex);
    D3DVECTOR   RetSelectCDG();
    D3DVECTOR   RetSelectNormal();
    void        SmoothSelect();
    void        PlaneSelect();
    void        ColorSelect();
    void        StateSelect();
    void        MoveSelect(D3DVECTOR move);
    void        OperSelect(D3DVECTOR move, char oper);
    void        ReadScript(char *filename);
    void        BBoxCompute(D3DVECTOR &min, D3DVECTOR &max);
    BOOL        IsMappingSelectPlausible(D3DMaping D3Dmode);
    void        MappingSelect(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY, FPOINT ti, FPOINT ts, char *texName);
    void        MappingSelectSpherical(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY, FPOINT ti, FPOINT ts, char *texName);
    D3DVECTOR   RetMappingCenter(D3DVECTOR pos, D3DVECTOR min);
    void        MappingSelectCylindrical(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY, FPOINT ti, FPOINT ts, char *texName);
    void        MappingSelectFace(int mode, int rotate, BOOL bMirrorX, BOOL bMirrorY, FPOINT ti, FPOINT ts, char *texName);
    void        MappingSelect2(int texNum2, int subdiv, int offsetU, int offsetV, BOOL bMirrorX, BOOL bMirrorY);
    void        MappingSelectPlane2(int mode, BOOL bMirrorX, BOOL bMirrorY);
    void        MappingSelectSpherical2(BOOL bMirrorX, BOOL bMirrorY);
    void        MappingSelectMagic2(BOOL bMirrorX, BOOL bMirrorY);
    int         SearchNext(int rank, int step);
    int         SearchSamePlane(int first, int step);
    void        CurrentSearchNext(int step, BOOL bControl);
    void        CurrentInit();
    void        CurrentSelect(BOOL bSelect);
    void        DeselectAll();
    void        SelectAll();
    void        SelectZone(int first, int last);
    void        SelectTerm();
    void        DefaultSelect();
    void        SelectDelete();
    void        Compress();
    void        MinMaxSelect();
    void        MinMaxChange();
    void        UpdateInfoText();
    int*        RetTextureTable();
    void        TexturePartUpdate();
    void        TextureRankChange(int step);
    void        TexturePartChange(int step);
    void        PutTextureValues();
    void        GetTextureValues();
    void        GetModelName(char *buffer);
    void        GetDXFName(char *buffer);
    void        GetScriptName(char *buffer);
    BOOL        IsEditFocus();

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CModFile*       m_modFile;
    CInterface*     m_interface;

    float           m_time;
    ModelTriangle*  m_triangleTable;
    int             m_triangleSel1;
    int             m_triangleSel2;
    int             m_mode;
    int             m_textureMode;
    int             m_textureRotate;
    BOOL            m_bTextureMirrorX;
    BOOL            m_bTextureMirrorY;
    FPOINT          m_textureInf;
    FPOINT          m_textureSup;
    int             m_texturePart;
    int             m_textureRank;
    char            m_textureName[20];
    BOOL            m_bDisplayTransparent;
    BOOL            m_bDisplayOnlySelection;
    float           m_viewHeight;
    float           m_viewDist;
    float           m_viewAngleH;
    float           m_viewAngleV;
    int             m_color;
    int             m_state;
    int             m_secondTexNum;
    int             m_secondSubdiv;
    int             m_secondOffsetU;
    int             m_secondOffsetV;
    char            m_oper;
    float           m_min;
    float           m_max;
};


#endif //_MODEL_H_
