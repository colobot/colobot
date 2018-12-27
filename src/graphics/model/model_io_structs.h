/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "graphics/core/material.h"

#include "graphics/model/model_triangle.h"

namespace Gfx
{

/*******************************************************
                      Deprecated enums/magic values
 *******************************************************/

/**
 * \enum ModelLODLevel
 * \brief Old level-of-detail enum
 *
 * @deprecated
 */
enum class ModelLODLevel
{
    Constant = -1, //!< triangle is always visible, no matter at what distance
    Low      =  1, //!< triangle is visible at farthest distance (lowest quality)
    Medium   =  2, //!< triangle is visible at medium distance (medium quality)
    High     =  4  //!< triangle is visible at closest distance (highest quality)
};

/**
 * \enum ModelRenderState
 * \brief Old render state enum (values copied from EngineRenderState)
 *
 * @deprecated
 */
enum class ModelRenderState
{
    TTextureBlack = 1,    //!< old ENG_RSTATE_TTEXTURE_BLACK
    TTextureWhite = 2,    //!< old ENG_RSTATE_TTEXTURE_WHITE
    Part1         = 256,  //!< old ENG_RSTATE_PART1
    Part2         = 512,  //!< old ENG_RSTATE_PART2
    Part3         = 1024, //!< old ENG_RSTATE_PART3
    TwoFace       = 4096, //!< old ENG_RSTATE_2FACE
    Alpha         = 8192  //!< old ENG_RSTATE_ALPHA
};

/*******************************************************
                     New model formats
 *******************************************************/

/**
 * \struct ModelHeaderV1AndV2
 * \brief Header for new model file version 1 and 2
 */
struct ModelHeaderV1AndV2
{
    //! File version (1, 2, ...)
    int version = 0;
    //! Total number of triangles
    int totalTriangles = 0;
};

/**
 * \struct ModelTriangleV1AndV2
 * \brief Triangle of new model file version 1 and 2
 */
struct ModelTriangleV1AndV2
{
    //! 1st vertex
    VertexTex2  p1;
    //! 2nd vertex
    VertexTex2  p2;
    //! 3rd vertex
    VertexTex2  p3;
    //! Material
    Material material;
    //! Name of 1st texture
    std::string tex1Name;
    //! Name of 2nd texture
    std::string tex2Name;
    //! If true, 2nd texture will be taken from current engine setting
    bool variableTex2 = true;
    //! LOD level (only version 1)
    ModelLODLevel lodLevel = ModelLODLevel::Constant;
    //! Rendering state to be set
    int state = 0;
};

/**
 * \struct ModelHeaderV3
 * \brief Header for new model file version 3
 */
struct ModelHeaderV3
{
    //! File version (1, 2, ...)
    int version = 0;
    //! Total number of crash spheres
    int totalCrashSpheres = 0;
    //! Whether model has shadow spot
    bool hasShadowSpot = false;
    //! Whether model has camera collision sphere
    bool hasCameraCollisionSphere = false;
    //! Total number of meshes
    int totalMeshes = 0;
};

/**
 * \struct ModelMeshHeaderV3
 * \brief Header for mesh saved in new model file version 3
 */
struct ModelMeshHeaderV3
{
    //! Total number of triangles
    int totalTriangles = 0;
    //! Mesh name
    std::string name;
    //! Parent mesh name
    std::string parentName;
    //! Mesh position
    Math::Vector position;
    //! Mesh rotation
    Math::Vector rotation;
    //! Mesh scale
    Math::Vector scale;
};

/**
 * \struct ModelTriangleV3
 * \brief Mesh triangle saved in new model file version 3
 *
 * NOTE: this is newest version, always same as ModelTriangle struct.
 */
struct ModelTriangleV3 : ModelTriangle {};



/*******************************************************
                      Deprecated formats
 *******************************************************/

/**
 * \struct OldModelHeader
 * \brief Old Colobot binary model header info
 *
 * @deprecated
 */
struct OldModelHeader
{
    //! Revision number
    int revision = 0;
    //! Version number
    int version = 0;
    //! Total number of triangles
    int totalTriangles = 0;
    //! Reserved area
    int reserved[10] = {};
};


/**
 * \struct OldModelTriangleV1
 * \brief Old Colobot binary model file version 1
 *
 * @deprecated
 */
struct OldModelTriangleV1
{
    char used = 0;
    char selected = 0;
    Vertex p1;
    Vertex p2;
    Vertex p3;
    Material material;
    char texName[21] = {'\0'};
    float min = 0;
    float max = 0;
};

/**
 * \struct OldModelTriangleV2
 * \brief Old Colobot binary model file version 2
 *
 * @deprecated
 */
struct OldModelTriangleV2
{
    char used = 0;
    char selected = 0;
    Vertex p1;
    Vertex p2;
    Vertex p3;
    Material material;
    char texName[21] = {'\0'};
    float min = 0.0f;
    float max = 0.0f;
    long state = 0;
    short reserved1 = 0;
    short reserved2 = 0;
    short reserved3 = 0;
    short reserved4 = 0;
};

/**
 * \struct OldModelTriangleV3
 * \brief Old Colobot binary model file version 3
 *
 * @deprecated
 */
struct OldModelTriangleV3
{
    char used = 0;
    char selected = 0;
    VertexTex2 p1;
    VertexTex2 p2;
    VertexTex2 p3;
    Material   material;
    char texName[21] = {'\0'};
    float min = 0.0f;
    float max = 0.0f;
    long state = 0;
    short texNum2 = 0;
    short reserved2 = 0;
    short reserved3 = 0;
    short reserved4 = 0;
};

} // namespace Gfx
