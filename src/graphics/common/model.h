// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

#pragma once

#include "engine.h"
#include "common/event.h"
#include "modfile.h"
#include "vertex.h"
#include "math/point.h"


class CInstanceManager;
class CModFile;
class CInterface;


namespace Gfx {

class CEngine;


class CModel {
    public:
    CModel(CInstanceManager* iMan);
    ~CModel();

    void        StartUserAction();
    void        StopUserAction();

    bool        EventProcess(const Event &event);

    void        InitView();
    void        InitViewFromSelect();
    void        UpdateView();
    void        ViewMove(const Event &event, float speed);

    protected:
    bool        EventFrame(const Event &event);
    bool        GetVertex(int rank, Gfx::VertexTex2 &vertex);
    bool        SetVertex(int rank, Gfx::VertexTex2 &vertex);
    Math::Vector    RetSelectCDG();
    Math::Vector    RetSelectNormal();
    void        SmoothSelect();
    void        PlaneSelect();
    void        ColorSelect();
    void        StateSelect();
    void        MoveSelect(Math::Vector move);
    void        OperSelect(Math::Vector move, char oper);
    void        ReadScript(char *filename);
    void        BBoxCompute(Math::Vector &min, Math::Vector &max);
    bool        IsMappingSelectPlausible(Gfx::Mapping D3Dmode);
    void        MappingSelect(int mode, int rotate, bool bMirrorX, bool bMirrorY, Math::Point ti, Math::Point ts, char *texName);
    void        MappingSelectSpherical(int mode, int rotate, bool bMirrorX, bool bMirrorY, Math::Point ti, Math::Point ts, char *texName);
    Math::Vector    RetMappingCenter(Math::Vector pos, Math::Vector min);
    void        MappingSelectCylindrical(int mode, int rotate, bool bMirrorX, bool bMirrorY, Math::Point ti, Math::Point ts, char *texName);
    void        MappingSelectFace(int mode, int rotate, bool bMirrorX, bool bMirrorY, Math::Point ti, Math::Point ts, char *texName);
    void        MappingSelect2(int texNum2, int subdiv, int offsetU, int offsetV, bool bMirrorX, bool bMirrorY);
    void        MappingSelectPlane2(int mode, bool bMirrorX, bool bMirrorY);
    void        MappingSelectSpherical2(bool bMirrorX, bool bMirrorY);
    void        MappingSelectMagic2(bool bMirrorX, bool bMirrorY);
    int         SearchNext(int rank, int step);
    int         SearchSamePlane(int first, int step);
    void        CurrentSearchNext(int step, bool bControl);
    void        CurrentInit();
    void        CurrentSelect(bool bSelect);
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
    bool        IsEditFocus();

    protected:
    CInstanceManager* m_iMan;
    Gfx::CEngine*   m_engine;
    CModFile*       m_modFile;
    CInterface*     m_interface;

    float           m_time;
    ModelTriangle*  m_triangleTable;
    int             m_triangleSel1;
    int             m_triangleSel2;
    int             m_mode;
    int             m_textureMode;
    int             m_textureRotate;
    bool            m_bTextureMirrorX;
    bool            m_bTextureMirrorY;
    Math::Point         m_textureInf;
    Math::Point         m_textureSup;
    int             m_texturePart;
    int             m_textureRank;
    char            m_textureName[20];
    bool            m_bDisplayTransparent;
    bool            m_bDisplayOnlySelection;
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

}; // namespace Gfx
