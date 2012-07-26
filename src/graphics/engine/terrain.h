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

// terrain.h

#pragma once

#include "graphics/engine/engine.h"


class CInstanceManager;


namespace Gfx {

class CEngine;
class CWater;


const short FLATLIMIT = (5.0f*Math::PI/180.0f);


enum TerrainRes
{
    TR_NULL     = 0,
    TR_STONE    = 1,
    TR_URANIUM  = 2,
    TR_POWER    = 3,
    TR_KEYa     = 4,
    TR_KEYb     = 5,
    TR_KEYc     = 6,
    TR_KEYd     = 7,
};


const short MAXBUILDINGLEVEL = 100;

struct BuildingLevel
{
    Math::Vector    center;
    float       factor;
    float       min;
    float       max;
    float       level;
    float       height;
    float       bboxMinX;
    float       bboxMaxX;
    float       bboxMinZ;
    float       bboxMaxZ;
};


const short MAXMATTERRAIN = 100;

struct TerrainMaterial
{
    short       id;
    char        texName[20];
    float       u,v;
    float       hardness;
    char        mat[4];     // up, right, down, left
};

struct DotLevel
{
    short       id;
    char        mat[4];     // up, right, down, left
};


const short MAXFLYINGLIMIT = 10;

struct FlyingLimit
{
    Math::Vector    center;
    float       extRadius;
    float       intRadius;
    float       maxHeight;
};



class CTerrain
{
public:
    CTerrain(CInstanceManager* iMan);
    ~CTerrain();

    bool        Generate(int mosaic, int brickP2, float size, float vision, int depth, float hardness);
    bool        InitTextures(char* baseName, int* table, int dx, int dy);
    void        LevelFlush();
    bool        LevelMaterial(int id, char* baseName, float u, float v, int up, int right, int down, int left, float hardness);
    bool        LevelInit(int id);
    bool        LevelGenerate(int *id, float min, float max, float slope, float freq, Math::Vector center, float radius);
    void        FlushRelief();
    bool        ReliefFromBMP(const char* filename, float scaleRelief, bool adjustBorder);
    bool        ReliefFromDXF(const char* filename, float scaleRelief);
    bool        ResFromBMP(const char* filename);
    bool        CreateObjects(bool bMultiRes);
    bool        Terraform(const Math::Vector &p1, const Math::Vector &p2, float height);

    void        SetWind(Math::Vector speed);
    Math::Vector    RetWind();

    float       RetFineSlope(const Math::Vector &pos);
    float       RetCoarseSlope(const Math::Vector &pos);
    bool        GetNormal(Math::Vector &n, const Math::Vector &p);
    float       RetFloorLevel(const Math::Vector &p, bool bBrut=false, bool bWater=false);
    float       RetFloorHeight(const Math::Vector &p, bool bBrut=false, bool bWater=false);
    bool        MoveOnFloor(Math::Vector &p, bool bBrut=false, bool bWater=false);
    bool        ValidPosition(Math::Vector &p, float marging);
    TerrainRes  RetResource(const Math::Vector &p);
    void        LimitPos(Math::Vector &pos);

    void        FlushBuildingLevel();
    bool        AddBuildingLevel(Math::Vector center, float min, float max, float height, float factor);
    bool        UpdateBuildingLevel(Math::Vector center);
    bool        DeleteBuildingLevel(Math::Vector center);
    float       RetBuildingFactor(const Math::Vector &p);
    float       RetHardness(const Math::Vector &p);

    int         RetMosaic();
    int         RetBrick();
    float       RetSize();
    float       RetScaleRelief();

    void        GroundFlat(Math::Vector pos);
    float       RetFlatZoneRadius(Math::Vector center, float max);

    void        SetFlyingMaxHeight(float height);
    float       RetFlyingMaxHeight();
    void        FlushFlyingLimit();
    bool        AddFlyingLimit(Math::Vector center, float extRadius, float intRadius, float maxHeight);
    float       RetFlyingLimit(Math::Vector pos, bool bNoLimit);

protected:
    bool        ReliefAddDot(Math::Vector pos, float scaleRelief);
    void        AdjustRelief();
    Math::Vector    RetVector(int x, int y);
    Gfx::VertexTex2 RetVertex(int x, int y, int step);
    bool        CreateMosaic(int ox, int oy, int step, int objRank, const Gfx::Material &mat, float min, float max);
    bool        CreateSquare(bool bMultiRes, int x, int y);

    TerrainMaterial* LevelSearchMat(int id);
    void        LevelTextureName(int x, int y, char *name, Math::Point &uv);
    float       LevelRetHeight(int x, int y);
    bool        LevelGetDot(int x, int y, float min, float max, float slope);
    int         LevelTestMat(char *mat);
    void        LevelSetDot(int x, int y, int id, char *mat);
    bool        LevelIfDot(int x, int y, int id, char *mat);
    bool        LevelPutDot(int x, int y, int id);
    void        LevelOpenTable();
    void        LevelCloseTable();

    void        AdjustBuildingLevel(Math::Vector &p);

protected:
    CInstanceManager* m_iMan;
    CEngine*        m_engine;
    CWater*         m_water;

    int         m_mosaic;       // number of mosaics
    int         m_brick;        // number of bricks per mosaics
    float           m_size;         // size of an item in an brick
    float           m_vision;       // vision before a change of resolution
    float*          m_relief;       // table of the relief
    int*            m_texture;      // table of textures
    int*            m_objRank;      // table of rows of objects
    bool            m_bMultiText;
    bool            m_bLevelText;
    float           m_scaleMapping;     // scale of the mapping
    float           m_scaleRelief;
    int         m_subdivMapping;
    int         m_depth;        // number of different resolutions (1,2,3,4)
    char            m_texBaseName[20];
    char            m_texBaseExt[10];
    float           m_defHardness;

    TerrainMaterial m_levelMat[MAXMATTERRAIN+1];
    int             m_levelMatTotal;
    int             m_levelMatMax;
    int             m_levelDotSize;
    DotLevel*       m_levelDot;
    int             m_levelID;

    int             m_buildingUsed;
    BuildingLevel   m_buildingTable[MAXBUILDINGLEVEL];

    unsigned char*  m_resources;
    Math::Vector        m_wind;         // wind speed

    float           m_flyingMaxHeight;
    int             m_flyingLimitTotal;
    FlyingLimit     m_flyingLimit[MAXFLYINGLIMIT];
};

}; // namespace Gfx
