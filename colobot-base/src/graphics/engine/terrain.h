/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

/**
 * \file graphics/engine/terrain.h
 * \brief Terrain rendering - CTerrain class
 */

#pragma once

#include "graphics/core/vertex.h"

#include "math/const.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <filesystem>


// Graphics module namespace
namespace Gfx
{

class CEngine;
class CWater;


//! Limit of slope considered a flat piece of land
const float TERRAIN_FLATLIMIT = (5.0f*Math::PI/180.0f);


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
//! Converts TerrainRes to color
Gfx::IntColor ResourceToColor(TerrainRes res);

/**
 * \class CTerrain
 * \brief Terrain loader/generator and manager
 *
 * \section Mapping Terrain mapping
 *
 * Terrain is created from relief textures specifying a XY plane with height
 * values which are then scaled and translated into XZ surface forming the
 * terrain of game level.
 *
 * The basic unit of terrain is called "brick", which is two triangles
 * forming a quad. Bricks have constant size (brick size)
 * in X and Z direction.
 * Points forming the bricks correspond one-to-one to relief data points
 * (pixels in relief image).
 *
 * Bricks are grouped into "mosaics". Mosaic is a square containing
 * brickCount x brickCount bricks where brickCount is an even power of 2.
 * Each mosaic corresponds to one created engine object.
 *
 * The whole terrain is also a square formed by mosaicCount * mosaicCount
 * of mosaics.
 *
 * Image coordinates are converted in the following way to world coordinates
 * of brick points (Wx, Wy, Wz - world coordinates, Ix, Iy - image coordinates,
 * Pxy - pixel value at Ix,Iy):
 *
 * Wx = (Ix - brickCount*mosaicCount / 2.0f) * brickSize \n
 * Wz = (Iy - brickCount*mosaicCount / 2.0f) * brickSize \n
 * Wy = (Pxy / 255.0f) * reliefScale
 *
 * To create and initialize a terrain, you must call Generate() as the first function,
 * setting the number of bricks, mosaics etc.
 *
 * \section Materials Materials and textures
 *
 * The terrain can be textured in two ways:
 *  - by applying texture index table
 *  - by specifying one or more "materials" that cover "material points"
 *
 * Textures are applied to subdivisions of mosaics (groups of bricks of size
 * brickCount / textureSubdivCount).
 *
 * \subsection TextureIndexes Texture indexes
 *
 * Texture indexes specify the texture for each textured point by concatenating
 * base name of texture, the index number and texture extension.
 *
 * Texture indexes are specified directly in InitTextures().
 *
 * \subsection TerrainMaterials Terrain materials
 *
 * Terrain materials are more sophisticated system. Each material is a texture,
 * applied to one area, but specifying also the textures to use on surrounding areas:
 * left, right, bottom and top.
 *
 * You specify one or more terrain materials in AddMaterial() function.
 * The function will add a material for given circle on the ground, with some
 * randomized matching of supplied materials and sophisticated logic for ensuring
 * that borders between neighboring materials follow the specified rules.
 *
 * \subsection BuildingLevels Other features
 *
 * Terrain can have specified building levels - flat space expanses,
 * where relief data is specifically adjusted to level space to allow
 * construction of buildings.
 *
 * Underground resources can be supplied by loading them from image like relief data.
 *
 * Terrain also specifies flying limits for player: one global level and possible
 * additional spherical restrictions.
 */
class CTerrain
{
public:
    CTerrain();
    ~CTerrain();

    //! Generates a new flat terrain
    bool        Generate(int mosaicCount, int brickCountPow2, float brickSize, float vision, int depth, float hardness);

    //! Initializes the names of textures to use for the land
    bool        InitTextures(const std::filesystem::path& baseName, int* table, int dx, int dy);

    //! Clears all terrain materials
    void        FlushMaterials();
    //! Adds a terrain material the names of textures to use for the land
    void        AddMaterial(int id, const std::filesystem::path& texName, const glm::vec2& uv,
                            int up, int right, int down, int left, float hardness);
    //! Initializes all the ground with one material
    bool        InitMaterials(int id);
    //! Generates a level in the terrain
    bool        GenerateMaterials(int *id, float min, float max, float slope, float freq, glm::vec3 center, float radius);

    //! Clears the relief, resources and all other associated data
    void        FlushRelief();
    //! Load relief from image
    bool        LoadRelief(const std::filesystem::path& fileName, float scaleRelief, bool adjustBorder);
    //! Load ramdomized relief
    bool        RandomizeRelief();

    //! Load resources from image
    bool        LoadResources(const std::filesystem::path& fileName);

    //! Creates all objects of the terrain within the 3D engine
    bool        CreateObjects();

    //! Modifies the terrain's relief
    bool        Terraform(const glm::vec3& p1, const glm::vec3& p2, float height);

    //@{
    //! Management of the wind
    void        SetWind(glm::vec3 speed);
    glm::vec3   GetWind();
    //@}

    //! Gives the exact slope of the terrain at 2D (XZ) position
    float       GetFineSlope(const glm::vec3& pos);
    //! Gives the approximate slope of the terrain at 2D (XZ) position
    float       GetCoarseSlope(const glm::vec3& pos);
    //! Gives the normal vector at 2D (XZ) position
    bool        GetNormal(glm::vec3& n, const glm::vec3 &p);
    //! Returns the height of the ground level at 2D (XZ) position
    float       GetFloorLevel(const glm::vec3& pos, bool brut=false, bool water=false);
    //! Returns the distance to the ground level from 3D position
    float       GetHeightToFloor(const glm::vec3& pos, bool brut=false, bool water=false);
    //! Modifies the Y coordinate of 3D position to rest on the ground floor
    bool        AdjustToFloor(glm::vec3& pos, bool brut=false, bool water=false);
    //! Adjusts 3D position so that it is within standard terrain boundaries
    bool        AdjustToStandardBounds(glm::vec3 &pos);
    //! Adjusts 3D position so that it is within terrain boundaries and the given margin
    bool        AdjustToBounds(glm::vec3& pos, float margin);
    //! Returns the resource type available underground at 2D (XZ) position
    TerrainRes GetResource(const glm::vec3& pos);

    //! Empty the table of elevations
    void        FlushBuildingLevel();
    //! Adds a new elevation for a building
    bool        AddBuildingLevel(glm::vec3 center, float min, float max, float height, float factor);
    //! Updates the elevation for a building when it was moved up (after a terraforming)
    bool        UpdateBuildingLevel(glm::vec3 center);
    //! Removes the elevation for a building when it was destroyed
    bool        DeleteBuildingLevel(glm::vec3 center);
    //! Returns the influence factor whether a position is on a possible rise
    float       GetBuildingFactor(const glm::vec3& pos);
    //! Returns the hardness of the ground in a given place
    float       GetHardness(const glm::vec3& pos);

    //! Returns number of mosaics
    int         GetMosaicCount();
    //! Returns number of bricks in mosaic
    int         GetBrickCount();
    //! Returns brick size
    float       GetBrickSize();
    //! Returns the vertical scale of relief
    float       GetReliefScale();

    //! Shows the flat areas on the ground
    void        ShowFlatGround(glm::vec3 pos);
    //! Calculates the radius of the largest flat area available
    float       GetFlatZoneRadius(glm::vec3 center, float max);

    //@{
    //! Management of the global max flying height
    void        SetFlyingMaxHeight(float height);
    float       GetFlyingMaxHeight();
    //@}
    //! Empty the table of flying limits
    void        FlushFlyingLimit();
    //! Adds a new flying limit
    void        AddFlyingLimit(glm::vec3 center, float extRadius, float intRadius, float maxHeight);
    //! Returns the maximum height of flight
    float       GetFlyingLimit(glm::vec3 pos, bool noLimit);

protected:
    //! Adds a point of elevation in the buffer of relief
    bool        AddReliefPoint(glm::vec3 pos, float scaleRelief);
    //! Adjust the edges of each mosaic to be compatible with all lower resolutions
    void        AdjustRelief();
    //! Calculates a vector of the terrain
    glm::vec3   GetVector(int x, int y);
    //! Calculates a vertex of the terrain
    Vertex3D    GetVertex(int x, int y, int step);
    //! Creates all objects of a mosaic
    bool        CreateMosaic(int ox, int oy, int step, int objRank);
    //! Creates all objects in a mesh square ground
    bool        CreateSquare(int x, int y);

    struct TerrainMaterial;
    //! Seeks a material based on its ID
    TerrainMaterial* FindMaterial(int id);
    //! Seeks a material based on neighbor values
    int         FindMaterialByNeighbors(char *mat);
    //! Returns the texture name and UV coords to use for a given square
    void        GetTexture(int x, int y, std::filesystem::path& name, glm::vec2& uv);
    //! Returns the height of the terrain
    float       GetHeight(int x, int y);
    //! Decide whether a point is using the materials
    bool        CheckMaterialPoint(int x, int y, float min, float max, float slope);
    //! Modifies the state of a point and its four neighbors, without testing if possible
    void        SetMaterialPoint(int x, int y, int id, char *mat);
    //! Modifies the state of a point
    bool        ChangeMaterialPoint(int x, int y, int id);
    //! Tests if a material can give a place, according to its four neighbors. If yes, puts the point.
    bool        CondChangeMaterialPoint(int x, int y, int id, char *mat);
    //! Initializes material points array
    void        InitMaterialPoints();
    //! Clears the material points
    void        FlushMaterialPoints();

    //! Adjusts a position according to a possible rise
    void        AdjustBuildingLevel(glm::vec3 &p);

protected:
    CEngine*        m_engine;
    CWater*         m_water;

    //! Relief data points
    std::vector<float> m_relief;
    //! Resources data
    std::vector<unsigned char> m_resources;
    //! Texture indices
    std::vector<int> m_textures;
    //! Object ranks for mosaic objects
    std::vector<int> m_objRanks;

    //! Number of mosaics (along one dimension)
    int             m_mosaicCount;
    //! Number of bricks per mosaic (along one dimension)
    int             m_brickCount;
    //! Number of terrain material dots (along one dimension)
    int             m_materialPointCount;
    //! Size of single brick (along X and Z axis)
    float           m_brickSize;
    //! Vertical (relief) scale
    float           m_scaleRelief;
    //! Subdivision of material points in mosaic
    int             m_textureSubdivCount;
    //! Number of different resolutions (1,2,3,4)
    int             m_depth;
    //! Scale of texture mapping
    float           m_textureScale;
    //! Vision before a change of resolution
    float           m_vision;

    //! Base name for single texture
    std::filesystem::path m_texBaseName;
    //! Extension for single texture
    std::filesystem::path m_texBaseExt;
    //! Default hardness for level material
    float           m_defaultHardness;
    /**
     * \struct TerrainMaterial
     * \brief Material for ground surface
     */
    struct TerrainMaterial
    {
        //! Unique ID
        short       id = 0;
        //! Texture
        std::string texName;
        //! UV texture coordinates
        glm::vec2   uv;
        //! Terrain hardness (defines e.g. sound of walking)
        float       hardness = 0.0f;
        //! IDs of neighbor materials: up, right, down, left
        char        mat[4] = {};
    };
    //! Terrain materials
    std::vector<TerrainMaterial> m_materials;

    /**
     * \struct TerrainMaterialPoint
     * \brief Material used for terrain point
     */
    struct TerrainMaterialPoint
    {
        //! ID of material
        short       id = 0;
        //! IDs of neighbor materials: up, right, down, left
        char        mat[4] = {};
    };


    //! Material for terrain points
    std::vector<TerrainMaterialPoint>  m_materialPoints;

    //! True if using terrain material mapping
    bool m_useMaterials;
    //! Maximum level ID (no ID is >= to this)
    int             m_maxMaterialID;
    //! Internal counter for auto generation of material IDs
    int             m_materialAutoID;

    /**
     * \struct BuildingLevel
     * \brief Flat level for building
     */
    struct BuildingLevel
    {
        glm::vec3    center{ 0, 0, 0 };
        float        factor = 0.0f;
        float        min = 0.0f;
        float        max = 0.0f;
        float        level = 0.0f;
        float        height = 0.0f;
        float        bboxMinX = 0.0f;
        float        bboxMaxX = 0.0f;
        float        bboxMinZ = 0.0f;
        float        bboxMaxZ = 0.0f;
    };
    std::vector<BuildingLevel> m_buildingLevels;

    //! Wind speed
    glm::vec3    m_wind{ 0, 0, 0 };

    //! Global flying height limit
    float           m_flyingMaxHeight;

    /**
     * \struct FlyingLimit
     * \brief Spherical limit of flight
     */
    struct FlyingLimit
    {
        glm::vec3    center{ 0, 0, 0 };
        float        extRadius = 0.0f;
        float        intRadius = 0.0f;
        float        maxHeight = 0.0f;
    };
    //! List of local flight limits
    std::vector<FlyingLimit> m_flyingLimits;
};


} // namespace Gfx

