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

/**
 * \file graphics/engine/terrain.h
 * \brief Terrain rendering - Gfx::CTerrain class
 */

#pragma once

#include "graphics/engine/engine.h"


class CInstanceManager;


namespace Gfx {

class CEngine;
class CWater;


//! Limit of slope considered a flat piece of land
const short FLATLIMIT = (5.0f*Math::PI/180.0f);


/**
 * \enum TerrainRes
 * \brief Underground resource type
 */
enum TerrainRes
{
    //! No resource
    TR_NULL      = 0,
    //! Titanium
    TR_STONE     = 1,
    //! Uranium
    TR_URANIUM   = 2,
    //! Energy
    TR_POWER     = 3,
    //! Vault keys
    //@{
    TR_KEY_A     = 4,
    TR_KEY_B     = 5,
    TR_KEY_C     = 6,
    TR_KEY_D     = 7
    //@}
};

struct BuildingLevel
{
    Math::Vector center;
    float        factor;
    float        min;
    float        max;
    float        level;
    float        height;
    float        bboxMinX;
    float        bboxMaxX;
    float        bboxMinZ;
    float        bboxMaxZ;

    BuildingLevel()
    {
        factor = min = max = level = height = 0.0f;
        bboxMinX = bboxMaxX = bboxMinZ = bboxMaxZ = 0.0f;
    }
};

struct TerrainMaterial
{
    short       id;
    std::string texName;
    float       u,v;
    float       hardness;
    char        mat[4];     // up, right, down, left

    TerrainMaterial()
    {
        id = 0;
        u = v = 0.0f;
        hardness = 0.0f;
        mat[0] = mat[1] = mat[2] = mat[3] = 0;
    }
};

struct DotLevel
{
    short       id;
    char        mat[4];     // up, right, down, left

    DotLevel()
    {
        id = 0;
        mat[0] = mat[1] = mat[2] = mat[3] = 0;
    }
};

/**
 * \struct FlyingLimit
 * \brief Spherical limit of flight
 */
struct FlyingLimit
{
    Math::Vector center;
    float        extRadius;
    float        intRadius;
    float        maxHeight;

    FlyingLimit()
    {
        extRadius = intRadius = maxHeight = 0.0f;
    }
};


/**
 * \class CTerrain
 * \brief Terrain loader/generator and manager
 *
 * Terrain is created from relief textures specifying a XY plane with height
 * values which are then scaled and translated into XZ surface forming the
 * terrain of game level.
 *
 * The class also facilitates creating and searching for flat space expanses
 * for construction of buildings.
 *
 * The terrain also specifies underground resources loaded from texture
 * and flying limits for the player.
 *
 * ...
 */
class CTerrain
{
public:
    CTerrain(CInstanceManager* iMan);
    ~CTerrain();

    //! Generates a new flat terrain
    bool        Generate(int mosaic, int brickPow2, float size, float vision, int depth, float hardness);
    //! Initializes the names of textures to use for the land
    bool        InitTextures(const std::string& baseName, int* table, int dx, int dy);
    //! Empties level
    void        LevelFlush();
    //! Initializes the names of textures to use for the land
    void        LevelMaterial(int id, std::string& baseName, float u, float v, int up, int right, int down, int left, float hardness);
    //! Initializes all the ground with a material
    bool        LevelInit(int id);
    //! Generates a level in the terrain
    bool        LevelGenerate(int *id, float min, float max, float slope, float freq, Math::Vector center, float radius);
    //! Initializes a completely flat terrain
    void        FlushRelief();
    //! Load relief from a PNG file
    bool        ReliefFromPNG(const std::string& filename, float scaleRelief, bool adjustBorder);
    //! Load resources from a PNG file
    bool        ResFromPNG(const std::string& filename);
    //! Creates all objects of the terrain within the 3D engine
    bool        CreateObjects(bool multiRes);
    //! Modifies the terrain's relief
    bool        Terraform(const Math::Vector& p1, const Math::Vector& p2, float height);

    //@{
    //! Management of the wind
    void         SetWind(Math::Vector speed);
    Math::Vector GetWind();
    //@}

    //! Gives the exact slope of the terrain of a place given
    float       GetFineSlope(const Math::Vector& pos);
    //! Gives the approximate slope of the terrain of a specific location
    float       GetCoarseSlope(const Math::Vector& pos);
    //! Gives the normal vector at the position p (x,-,z) of the ground
    bool        GetNormal(Math::Vector& n, const Math::Vector &p);
    //! returns the height of the ground
    float       GetFloorLevel(const Math::Vector& p, bool brut=false, bool water=false);
    //! Returns the height to the ground
    float       GetFloorHeight(const Math::Vector& p, bool brut=false, bool water=false);
    //! Modifies the coordinate "y" of point "p" to rest on the ground floor
    bool        MoveOnFloor(Math::Vector& p, bool brut=false, bool water=false);
    //! Modifies a coordinate so that it is on the ground
    bool        ValidPosition(Math::Vector& p, float marging);
    //! Returns the resource type available underground
    Gfx::TerrainRes GetResource(const Math::Vector& p);
    //! Adjusts a position so that it does not exceed the boundaries
    void        LimitPos(Math::Vector &pos);

    //! Empty the table of elevations
    void        FlushBuildingLevel();
    //! Adds a new elevation for a building
    bool        AddBuildingLevel(Math::Vector center, float min, float max, float height, float factor);
    //! Updates the elevation for a building when it was moved up (after a terraforming)
    bool        UpdateBuildingLevel(Math::Vector center);
    //! Removes the elevation for a building when it was destroyed
    bool        DeleteBuildingLevel(Math::Vector center);
    //! Returns the influence factor whether a position is on a possible rise
    float       GetBuildingFactor(const Math::Vector& p);
    float       GetHardness(const Math::Vector& p);

    int         GetMosaic();
    int         GetBrick();
    float       GetSize();
    float       GetScaleRelief();

    //! Shows the flat areas on the ground
    void        GroundFlat(Math::Vector pos);
    //! Calculates the radius of the largest flat area available
    float       GetFlatZoneRadius(Math::Vector center, float max);

    //@{
    //! Management of the global max flying height
    void        SetFlyingMaxHeight(float height);
    float       GetFlyingMaxHeight();
    //@}
    //! Empty the table of flying limits
    void        FlushFlyingLimit();
    //! Adds a new flying limit
    void        AddFlyingLimit(Math::Vector center, float extRadius, float intRadius, float maxHeight);
    //! Returns the maximum height of flight
    float       GetFlyingLimit(Math::Vector pos, bool noLimit);

protected:
    //! Adds a point of elevation in the buffer of relief
    bool        ReliefAddDot(Math::Vector pos, float scaleRelief);
    //! Adjust the edges of each mosaic to be compatible with all lower resolutions
    void        AdjustRelief();
    //! Calculates a vector of the terrain
    Math::Vector GetVector(int x, int y);
    //! Calculates a vertex of the terrain
    Gfx::VertexTex2 GetVertex(int x, int y, int step);
    //! Creates all objects of a mosaic
    bool        CreateMosaic(int ox, int oy, int step, int objRank, const Gfx::Material& mat, float min, float max);
    //! Creates all objects in a mesh square ground
    bool        CreateSquare(bool multiRes, int x, int y);

    //! Seeks a materials based on theirs identifier
    Gfx::TerrainMaterial* LevelSearchMat(int id);
    //! Chooses texture to use for a given square
    void        LevelTextureName(int x, int y, std::string& name, Math::Point &uv);
    //! Returns the height of the terrain
    float       LevelGetHeight(int x, int y);
    //! Decide whether a point is using the materials
    bool        LevelGetDot(int x, int y, float min, float max, float slope);
    //! Seeks if material exists
    int         LevelTestMat(char *mat);
    //! Modifies the state of a point and its four neighbors, without testing if possible
    void        LevelSetDot(int x, int y, int id, char *mat);
    //! Tests if a material can give a place, according to its four neighbors. If yes, puts the point.
    bool        LevelIfDot(int x, int y, int id, char *mat);
    //! Modifies the state of a point
    bool        LevelPutDot(int x, int y, int id);
    //! Initializes a table with empty levels
    void        LevelOpenTable();
    //! Closes the level table
    void        LevelCloseTable();

    //! Adjusts a position according to a possible rise
    void        AdjustBuildingLevel(Math::Vector &p);

protected:
    CInstanceManager* m_iMan;
    CEngine*        m_engine;
    CWater*         m_water;

    //! Number of mosaics
    int             m_mosaic;
    //! Number of bricks per mosaics
    int             m_brick;
    int             m_levelDotSize;
    //! Size of an item in a brick
    float           m_size;
    //! Vision before a change of resolution
    float           m_vision;
    //! Table of the relief
    std::vector<float> m_relief;
    //! Table of textures
    std::vector<int> m_texture;
    //! Table of rows of objects
    std::vector<int> m_objRank;
    //! Table of resources
    std::vector<unsigned char> m_resources;
    bool            m_multiText;
    bool            m_levelText;
    //! Scale of the mapping
    float           m_scaleMapping;
    float           m_scaleRelief;
    int             m_subdivMapping;
    //! Number of different resolutions (1,2,3,4)
    int             m_depth;
    std::string     m_texBaseName;
    std::string     m_texBaseExt;
    float           m_defHardness;

    std::vector<TerrainMaterial> m_levelMats;
    std::vector<Gfx::DotLevel>  m_levelDots;
    int             m_levelMatMax;
    int             m_levelID;

    std::vector<Gfx::BuildingLevel> m_buildingLevels;

    //! Wind speed
    Math::Vector    m_wind;

    //! Global flying height limit
    float           m_flyingMaxHeight;
    //! List of local flight limits
    std::vector<Gfx::FlyingLimit> m_flyingLimits;
};

}; // namespace Gfx
