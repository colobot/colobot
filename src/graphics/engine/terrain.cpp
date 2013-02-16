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


#include "graphics/engine/terrain.h"

#include "app/app.h"

#include "common/image.h"
#include "common/logger.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/water.h"

#include "math/geometry.h"

#include <sstream>

#include <SDL/SDL.h>


// Graphics module namespace
namespace Gfx {


CTerrain::CTerrain()
{
    m_engine = CEngine::GetInstancePointer();
    m_water  = m_engine->GetWater();

    m_mosaicCount     = 20;
    m_brickCount      = 1 << 4;
    m_brickSize       = 10.0f;
    m_vision          = 200.0f;
    m_textureScale    = 0.01f;
    m_scaleRelief     = 1.0f;
    m_textureSubdivCount   = 1;
    m_depth           = 2;
    m_maxMaterialID   = 0;
    m_wind            = Math::Vector(0.0f, 0.0f, 0.0f);
    m_defaultHardness = 0.5f;
    m_useMaterials    = false;

    FlushBuildingLevel();
    FlushFlyingLimit();
    FlushMaterials();
}

CTerrain::~CTerrain()
{
}

bool CTerrain::Generate(int mosaicCount, int brickCountPow2, float brickSize,
                        float vision, int depth, float hardness)
{
    m_mosaicCount   = mosaicCount;
    m_brickCount    = 1 << brickCountPow2;
    m_brickSize     = brickSize;
    m_vision        = vision;
    m_depth         = depth;
    m_defaultHardness   = hardness;

    m_engine->SetTerrainVision(vision);

    m_textureScale  = 1.0f / (m_brickCount*m_brickSize);
    m_textureSubdivCount = 1;

    m_useMaterials = false;

    int dim = 0;

    dim = (m_mosaicCount*m_brickCount+1)*(m_mosaicCount*m_brickCount+1);
    std::vector<float>(dim).swap(m_relief);

    dim = m_mosaicCount*m_textureSubdivCount*m_mosaicCount*m_textureSubdivCount;
    std::vector<int>(dim).swap(m_textures);

    dim = m_mosaicCount*m_mosaicCount;
    std::vector<int>(dim).swap(m_objRanks);

    return true;
}


int CTerrain::GetMosaicCount()
{
    return m_mosaicCount;
}

int CTerrain::GetBrickCount()
{
    return m_brickCount;
}

float CTerrain::GetBrickSize()
{
    return m_brickSize;
}

float CTerrain::GetReliefScale()
{
    return m_scaleRelief;
}

bool CTerrain::InitTextures(const std::string& baseName, int* table, int dx, int dy)
{
    m_useMaterials = false;

    m_texBaseName = baseName;
    size_t pos = baseName.find('.');

    if (pos == std::string::npos)
    {
        m_texBaseExt = ".png";
    }
    else
    {
        m_texBaseExt = m_texBaseName.substr(pos);
        m_texBaseName = m_texBaseName.substr(0, pos);
    }

    for (int y = 0; y < m_mosaicCount*m_textureSubdivCount; y++)
    {
        for (int x = 0; x < m_mosaicCount*m_textureSubdivCount; x++)
        {
            m_textures[x+y*m_mosaicCount] = table[(x%dx)+(y%dy)*dx];
        }
    }
    return true;
}


void CTerrain::FlushMaterials()
{
    m_materials.clear();
    m_maxMaterialID = 0;
    m_materialAutoID = 1000;
    FlushMaterialPoints();
}

void CTerrain::AddMaterial(int id, const std::string& texName, const Math::Point &uv,
                           int up, int right, int down, int left,
                           float hardness)
{
    InitMaterialPoints();

    if (id == 0)
        id = m_materialAutoID++;

    TerrainMaterial tm;
    tm.texName  = texName;
    tm.id       = id;
    tm.uv       = uv;
    tm.mat[0]   = up;
    tm.mat[1]   = right;
    tm.mat[2]   = down;
    tm.mat[3]   = left;
    tm.hardness = hardness;

    m_materials.push_back(tm);

    if (m_maxMaterialID < up+1   )  m_maxMaterialID = up+1;
    if (m_maxMaterialID < right+1)  m_maxMaterialID = right+1;
    if (m_maxMaterialID < down+1 )  m_maxMaterialID = down+1;
    if (m_maxMaterialID < left+1 )  m_maxMaterialID = left+1;

    m_useMaterials = true;
    m_textureSubdivCount = 4;
}


/**
 * The image must be 24 bits/pixel and grayscale and dx x dy in size
 * with dx = dy = (mosaic*brick)+1 */
bool CTerrain::LoadResources(const std::string& fileName)
{
    CImage img;
    std::string path = CApplication::GetInstance().GetDataFilePath(DIR_TEXTURE, fileName);
    if (! img.Load(path))
    {
        GetLogger()->Error("Cannot load resource file: '%s'\n", path.c_str());
        return false;
    }

    ImageData *data = img.GetData();

    int size = (m_mosaicCount*m_brickCount)+1;

    std::vector<unsigned char>(size*size).swap(m_resources);

    if ( (data->surface->w != size) || (data->surface->h != size) )
    {
        GetLogger()->Error("Invalid resource file\n");
        return false;
    }

    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            Gfx::IntColor pixel = img.GetPixelInt(Math::IntPoint(x, size - y - 1));
            TerrainRes res = TR_NULL;

            // values from original bitmap palette
            if      (pixel.r == 255 && pixel.g ==   0 && pixel.b == 0)
                res = TR_STONE;
            else if (pixel.r == 255 && pixel.g == 255 && pixel.b == 0)
                res = TR_URANIUM;
            else if (pixel.r ==   0 && pixel.g == 255 && pixel.b == 0)
                res = TR_POWER;
            else if (pixel.r ==   0 && pixel.g == 204 && pixel.b == 0)
                res = TR_KEY_A;
            else if (pixel.r ==  51 && pixel.g == 204 && pixel.b == 0)
                res = TR_KEY_B;
            else if (pixel.r == 102 && pixel.g == 204 && pixel.b == 0)
                res = TR_KEY_C;
            else if (pixel.r == 153 && pixel.g == 204 && pixel.b == 0)
                res = TR_KEY_D;

            m_resources[x+size*y] = static_cast<unsigned char>(res);
        }
    }

    return true;
}

TerrainRes CTerrain::GetResource(const Math::Vector &p)
{
    if (m_resources.empty())
        return TR_NULL;

    int x = static_cast<int>((p.x + (m_mosaicCount*m_brickCount*m_brickSize)/2.0f)/m_brickSize);
    int y = static_cast<int>((p.z + (m_mosaicCount*m_brickCount*m_brickSize)/2.0f)/m_brickSize);

    if ( x < 0 || x > m_mosaicCount*m_brickCount ||
         y < 0 || y > m_mosaicCount*m_brickCount )
        return TR_NULL;

    int size = (m_mosaicCount*m_brickCount)+1;

    return static_cast<TerrainRes>( m_resources[x+size*y] );
}

void CTerrain::FlushRelief()
{
    m_relief.clear();
}

/**
 * The image must be 24 bits/pixel and dx x dy in size
 * with dx = dy = (mosaic*brick)+1 */
bool CTerrain::LoadRelief(const std::string &fileName, float scaleRelief,
                          bool adjustBorder)
{
    m_scaleRelief = scaleRelief;

    CImage img;
    std::string path = CApplication::GetInstance().GetDataFilePath(DIR_TEXTURE, fileName);
    if (! img.Load(path))
    {
        GetLogger()->Error("Could not load relief file: '%s'!\n", path.c_str());
        return false;
    }

    ImageData *data = img.GetData();

    int size = (m_mosaicCount*m_brickCount)+1;

    if ( (data->surface->w != size) || (data->surface->h != size) )
    {
        GetLogger()->Error("Invalid relief file!\n");
        return false;
    }

    float limit = 0.9f;
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            Gfx::IntColor color = img.GetPixelInt(Math::IntPoint(x, size - y - 1));

            float avg = (color.r + color.g + color.b) / 3.0f; // to be sure it is grayscale
            float level = (255.0f - avg) * scaleRelief;

            float dist = Math::Max(fabs(static_cast<float>(x-size/2)),
                                   fabs(static_cast<float>(y-size/2)));
            dist = dist/ static_cast<float>(size / 2);
            if (dist > limit && adjustBorder)
            {
                dist = (dist-limit)/(1.0f-limit);  // 0..1
                if (dist > 1.0f) dist = 1.0f;
                float border = 300.0f+Math::Rand()*20.0f;
                level = level+dist*(border-level);
            }

            m_relief[x+y*size] = level;
        }
    }

    return true;
}

bool CTerrain::AddReliefPoint(Math::Vector pos, float scaleRelief)
{
    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;
    int size = (m_mosaicCount*m_brickCount)+1;

    pos.x = (pos.x+dim)/m_brickSize;
    pos.z = (pos.z+dim)/m_brickSize;

    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.z);

    if ( x < 0 || x >= size ||
         y < 0 || y >= size )  return false;

    if (m_relief[x+y*size] < pos.y*scaleRelief)
        m_relief[x+y*size] = pos.y*scaleRelief;

    return true;
}

void CTerrain::AdjustRelief()
{
    if (m_depth == 1) return;

    int ii = m_mosaicCount*m_brickCount+1;
    int b = 1 << (m_depth-1);

    for (int y = 0; y < m_mosaicCount*m_brickCount; y += b)
    {
        for (int x = 0; x < m_mosaicCount*m_brickCount; x += b)
        {
            int xx = 0;
            int yy = 0;
            if ((y+yy)%m_brickCount == 0)
            {
                float level1 = m_relief[(x+0)+(y+yy)*ii];
                float level2 = m_relief[(x+b)+(y+yy)*ii];
                for (xx = 1; xx < b; xx++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*xx+level1;
                }
            }

            yy = b;
            if ((y+yy)%m_brickCount == 0)
            {
                float level1 = m_relief[(x+0)+(y+yy)*ii];
                float level2 = m_relief[(x+b)+(y+yy)*ii];
                for (xx = 1; xx < b; xx++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*xx+level1;
                }
            }

            xx = 0;
            if ((x+xx)%m_brickCount == 0)
            {
                float level1 = m_relief[(x+xx)+(y+0)*ii];
                float level2 = m_relief[(x+xx)+(y+b)*ii];
                for (yy = 1; yy < b; yy++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*yy+level1;
                }
            }

            xx = b;
            if ((x+xx)%m_brickCount == 0)
            {
                float level1 = m_relief[(x+xx)+(y+0)*ii];
                float level2 = m_relief[(x+xx)+(y+b)*ii];
                for (yy = 1; yy < b; yy++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*yy+level1;
                }
            }
        }
    }
}

Math::Vector CTerrain::GetVector(int x, int y)
{
    Math::Vector p;
    p.x = x*m_brickSize - (m_mosaicCount*m_brickCount*m_brickSize) / 2.0;
    p.z = y*m_brickSize - (m_mosaicCount*m_brickCount*m_brickSize) / 2.0;

    if ( !m_relief.empty()                         &&
         x >= 0 && x <= m_mosaicCount*m_brickCount &&
         y >= 0 && y <= m_mosaicCount*m_brickCount )
    {
        p.y = m_relief[x+y*(m_mosaicCount*m_brickCount+1)];
    }
    else
    {
        p.y = 0.0f;
    }

    return p;
}

/** Calculates an averaged normal, taking into account the six adjacent triangles:

\verbatim
  ^ y
  |
  b---c---+
  |\  |\  |
  |  \|  \|
  a---o---d
  |\  |\  |
  |  \|  \|
  +---f---e--> x
\endverbatim */
VertexTex2 CTerrain::GetVertex(int x, int y, int step)
{
    VertexTex2 v;

    Math::Vector o = GetVector(x, y);
    v.coord = o;

    Math::Vector a = GetVector(x-step, y     );
    Math::Vector b = GetVector(x-step, y+step);
    Math::Vector c = GetVector(x,      y+step);
    Math::Vector d = GetVector(x+step, y     );
    Math::Vector e = GetVector(x+step, y-step);
    Math::Vector f = GetVector(x,      y-step);

    Math::Vector s(0.0f, 0.0f, 0.0f);

    if (x-step >= 0 && y+step <= m_mosaicCount*m_brickCount+1)
    {
        s += Math::NormalToPlane(b,a,o);
        s += Math::NormalToPlane(c,b,o);
    }

    if (x+step <= m_mosaicCount*m_brickCount+1 && y+step <= m_mosaicCount*m_brickCount+1)
        s += Math::NormalToPlane(d,c,o);

    if (x+step <= m_mosaicCount*m_brickCount+1 && y-step >= 0)
    {
        s += Math::NormalToPlane(e,d,o);
        s += Math::NormalToPlane(f,e,o);
    }

    if (x-step >= 0 && y-step >= 0)
        s += Math::NormalToPlane(a,f,o);

    s = Normalize(s);
    v.normal = s;

    int brick = m_brickCount/m_textureSubdivCount;
    Math::Vector oo = GetVector((x/brick)*brick, (y/brick)*brick);
    o  = GetVector(x, y);
    v.texCoord.x =        (o.x-oo.x)*m_textureScale*m_textureSubdivCount;
    v.texCoord.y = 1.0f - (o.z-oo.z)*m_textureScale*m_textureSubdivCount;

    v.texCoord2 = v.texCoord;

    return v;
}

/** The origin of mosaic is its center.
\verbatim
  ^ z
  |
  |  2---4---6--
  |  |\  |\  |\
  |  |  \|  \|
  |  1---3---5--- ...
  |
  +-------------------> x
\endverbatim */
bool CTerrain::CreateMosaic(int ox, int oy, int step, int objRank,
                            const Material &mat)
{
    int baseObjRank = m_engine->GetObjectBaseRank(objRank);
    if (baseObjRank == -1)
    {
        baseObjRank = m_engine->CreateBaseObject();
        m_engine->SetObjectBaseRank(objRank, baseObjRank);
    }

    std::string texName1;
    std::string texName2;

    if ( step == 1 && m_engine->GetGroundSpot() )
    {
        int i = (ox/5) + (oy/5)*(m_mosaicCount/5);
        std::stringstream s;
        s << "shadow";
        s.width(2);
        s.fill('0');
        s << i;
        s << ".png";
        texName2 = s.str();
    }

    int brick = m_brickCount/m_textureSubdivCount;

    VertexTex2 o = GetVertex(ox*m_brickCount+m_brickCount/2, oy*m_brickCount+m_brickCount/2, step);
    int total = ((brick/step)+1)*2;

    float pixel = 1.0f/256.0f;  // 1 pixel cover (*)
    float dp = 1.0f/512.0f;

    Math::Point uv;

    for (int my = 0; my < m_textureSubdivCount; my++)
    {
        for (int mx = 0; mx < m_textureSubdivCount; mx++)
        {
            if (m_useMaterials)
            {
                int xx = ox*m_brickCount + mx*(m_brickCount/m_textureSubdivCount);
                int yy = oy*m_brickCount + my*(m_brickCount/m_textureSubdivCount);
                GetTexture(xx, yy, texName1, uv);
            }
            else
            {
                int i = (ox*m_textureSubdivCount+mx)+(oy*m_textureSubdivCount+my)*m_mosaicCount;
                std::stringstream s;
                s << m_texBaseName;
                s.width(3);
                s.fill('0');
                s << m_textures[i];
                s << m_texBaseExt;
                texName1 = s.str();
            }

            for (int y = 0; y < brick; y += step)
            {
                EngineBaseObjDataTier buffer;
                buffer.vertices.reserve(total);

                buffer.type = ENG_TRIANGLE_TYPE_SURFACE;
                buffer.material = mat;

                buffer.state = ENG_RSTATE_WRAP;

                buffer.state |= ENG_RSTATE_SECOND;
                if (step == 1)
                    buffer.state |= ENG_RSTATE_DUAL_BLACK;

                for (int x = 0; x <= brick; x += step)
                {
                    VertexTex2 p1 = GetVertex(ox*m_brickCount+mx*brick+x, oy*m_brickCount+my*brick+y+0   , step);
                    VertexTex2 p2 = GetVertex(ox*m_brickCount+mx*brick+x, oy*m_brickCount+my*brick+y+step, step);
                    p1.coord.x -= o.coord.x;  p1.coord.z -= o.coord.z;
                    p2.coord.x -= o.coord.x;  p2.coord.z -= o.coord.z;

                    if (x == 0)
                    {
                        p1.texCoord.x = 0.0f+(0.5f/256.0f);
                        p2.texCoord.x = 0.0f+(0.5f/256.0f);
                    }
                    if (x == brick)
                    {
                        p1.texCoord.x = 1.0f-(0.5f/256.0f);
                        p2.texCoord.x = 1.0f-(0.5f/256.0f);
                    }
                    if (y == 0)
                        p1.texCoord.y = 1.0f-(0.5f/256.0f);

                    if (y == brick - step)
                        p2.texCoord.y = 0.0f+(0.5f/256.0f);

                    if (m_useMaterials)
                    {
                        p1.texCoord.x /= m_textureSubdivCount;  // 0..1 -> 0..0.25
                        p1.texCoord.y /= m_textureSubdivCount;
                        p2.texCoord.x /= m_textureSubdivCount;
                        p2.texCoord.y /= m_textureSubdivCount;

                        if (x == 0)
                        {
                            p1.texCoord.x = 0.0f+dp;
                            p2.texCoord.x = 0.0f+dp;
                        }
                        if (x == brick)
                        {
                            p1.texCoord.x = (1.0f/m_textureSubdivCount)-dp;
                            p2.texCoord.x = (1.0f/m_textureSubdivCount)-dp;
                        }
                        if (y == 0)
                            p1.texCoord.y = (1.0f/m_textureSubdivCount)-dp;

                        if (y == brick - step)
                            p2.texCoord.y = 0.0f+dp;

                        p1.texCoord.x += uv.x;
                        p1.texCoord.y += uv.y;
                        p2.texCoord.x += uv.x;
                        p2.texCoord.y += uv.y;
                    }

                    int xx = mx*(m_brickCount/m_textureSubdivCount) + x;
                    int yy = my*(m_brickCount/m_textureSubdivCount) + y;
                    p1.texCoord2.x = (static_cast<float>(ox%5)*m_brickCount+xx+0.0f)/(m_brickCount*5);
                    p1.texCoord2.y = (static_cast<float>(oy%5)*m_brickCount+yy+0.0f)/(m_brickCount*5);
                    p2.texCoord2.x = (static_cast<float>(ox%5)*m_brickCount+xx+0.0f)/(m_brickCount*5);
                    p2.texCoord2.y = (static_cast<float>(oy%5)*m_brickCount+yy+1.0f)/(m_brickCount*5);

// Correction for 1 pixel cover
// There is 1 pixel cover around each of the 16 surfaces:
//
//  |<--------------256-------------->|
//  |   |<----------254---------->|   |
//  |---|---|---|-- ... --|---|---|---|
//    |  0.0                   1.0  |
//    |   |                     |   |
//   0.0 min                   max 1.0
//
// The uv coordinates used for texturing are between min and max (instead of 0 and 1)
// This allows to exclude the pixels situated in a margin of a pixel around the surface

                    p1.texCoord2.x = (p1.texCoord2.x+pixel)*(1.0f-pixel)/(1.0f+pixel);
                    p1.texCoord2.y = (p1.texCoord2.y+pixel)*(1.0f-pixel)/(1.0f+pixel);
                    p2.texCoord2.x = (p2.texCoord2.x+pixel)*(1.0f-pixel)/(1.0f+pixel);
                    p2.texCoord2.y = (p2.texCoord2.y+pixel)*(1.0f-pixel)/(1.0f+pixel);


                    buffer.vertices.push_back(p1);
                    buffer.vertices.push_back(p2);
                }

                m_engine->AddBaseObjQuick(baseObjRank, buffer, texName1, texName2, LOD_Constant, true);
            }
        }
    }

    Math::Matrix transform;
    transform.LoadIdentity();
    transform.Set(1, 4, o.coord.x);
    transform.Set(3, 4, o.coord.z);
    m_engine->SetObjectTransform(objRank, transform);

    return true;
}

TerrainMaterial* CTerrain::FindMaterial(int id)
{
    for (int i = 0; i < static_cast<int>( m_materials.size() ); i++)
    {
        if (id == m_materials[i].id)
            return &m_materials[i];
    }

    return nullptr;
}

void CTerrain::GetTexture(int x, int y, std::string& name, Math::Point &uv)
{
    x /= m_brickCount/m_textureSubdivCount;
    y /= m_brickCount/m_textureSubdivCount;

    TerrainMaterial* tm = FindMaterial(m_materialPoints[x+y*m_materialPointCount].id);
    if (tm == nullptr)
    {
        name = "";
        uv = Math::Point(0.0f, 0.0f);
    }
    else
    {
        name = tm->texName;
        uv = tm->uv;
    }
}

float CTerrain::GetHeight(int x, int y)
{
    int size = (m_mosaicCount*m_brickCount+1);

    if (x <  0   )  x = 0;
    if (x >= size)  x = size-1;
    if (y <  0   )  y = 0;
    if (y >= size)  y = size-1;

    return m_relief[x+y*size];
}

bool CTerrain::CheckMaterialPoint(int x, int y, float min, float max, float slope)
{
    float hc = GetHeight(x, y);
    float h[4] =
    {
        GetHeight(x+0, y+1),
        GetHeight(x+1, y+0),
        GetHeight(x+0, y-1),
        GetHeight(x-1, y+0)
    };

    if (hc < min || hc > max)
        return false;

    if (slope == 0.0f)
        return true;

    if (slope > 0.0f)
    {
        for (int i = 0; i < 4; i++)
        {
            if (fabs(hc - h[i]) >= slope)
                return false;
        }
        return true;
    }

    if (slope < 0.0f)
    {
        for (int i = 0; i < 4; i++)
        {
            if (fabs(hc - h[i]) < -slope)
                return false;
        }
        return true;
    }

    return false;
}

int CTerrain::FindMaterialByNeighbors(char *mat)
{
    for (int i = 0; i < static_cast<int>( m_materials.size() ); i++)
    {
        if ( m_materials[i].mat[0] == mat[0] &&
             m_materials[i].mat[1] == mat[1] &&
             m_materials[i].mat[2] == mat[2] &&
             m_materials[i].mat[3] == mat[3] )  return i;
    }

    return -1;
}

void CTerrain::SetMaterialPoint(int x, int y, int id, char *mat)
{
    TerrainMaterial* tm = FindMaterial(id);
    if (tm == nullptr)  return;

    if ( tm->mat[0] != mat[0] ||
         tm->mat[1] != mat[1] ||
         tm->mat[2] != mat[2] ||
         tm->mat[3] != mat[3] )  // id incompatible with mat?
    {
        int ii = FindMaterialByNeighbors(mat);
        if (ii == -1)  return;
        id = m_materials[ii].id;  // looking for a id compatible with mat
    }

    // Changes the point
    m_materialPoints[x+y*m_materialPointCount].id     = id;
    m_materialPoints[x+y*m_materialPointCount].mat[0] = mat[0];
    m_materialPoints[x+y*m_materialPointCount].mat[1] = mat[1];
    m_materialPoints[x+y*m_materialPointCount].mat[2] = mat[2];
    m_materialPoints[x+y*m_materialPointCount].mat[3] = mat[3];

    // Changes the lower neighbor
    if ( (x+0) >= 0 && (x+0) < m_materialPointCount &&
         (y-1) >= 0 && (y-1) < m_materialPointCount )
    {
        int i = (x+0)+(y-1)*m_materialPointCount;
        if (m_materialPoints[i].mat[0] != mat[2])
        {
            m_materialPoints[i].mat[0] = mat[2];
            int ii = FindMaterialByNeighbors(m_materialPoints[i].mat);
            if (ii != -1)
                m_materialPoints[i].id = m_materials[ii].id;
        }
    }

    // Modifies the left neighbor
    if ( (x-1) >= 0 && (x-1) < m_materialPointCount &&
         (y+0) >= 0 && (y+0) < m_materialPointCount )
    {
        int i = (x-1)+(y+0)*m_materialPointCount;
        if (m_materialPoints[i].mat[1] != mat[3])
        {
            m_materialPoints[i].mat[1] = mat[3];
            int ii = FindMaterialByNeighbors(m_materialPoints[i].mat);
            if (ii != -1)
                m_materialPoints[i].id = m_materials[ii].id;
        }
    }

    // Changes the upper neighbor
    if ( (x+0) >= 0 && (x+0) < m_materialPointCount &&
         (y+1) >= 0 && (y+1) < m_materialPointCount )
    {
        int i = (x+0)+(y+1)*m_materialPointCount;
        if (m_materialPoints[i].mat[2] != mat[0])
        {
            m_materialPoints[i].mat[2] = mat[0];
            int ii = FindMaterialByNeighbors(m_materialPoints[i].mat);
            if (ii != -1)
                m_materialPoints[i].id = m_materials[ii].id;
        }
    }

    // Changes the right neighbor
    if ( (x+1) >= 0 && (x+1) < m_materialPointCount &&
         (y+0) >= 0 && (y+0) < m_materialPointCount )
    {
        int i = (x+1)+(y+0)*m_materialPointCount;
        if ( m_materialPoints[i].mat[3] != mat[1] )
        {
            m_materialPoints[i].mat[3] = mat[1];
            int ii = FindMaterialByNeighbors(m_materialPoints[i].mat);
            if (ii != -1)
                m_materialPoints[i].id = m_materials[ii].id;
        }
    }
}

bool CTerrain::CondChangeMaterialPoint(int x, int y, int id, char *mat)
{
    char test[4];

    // Compatible with lower neighbor?
    if ( x+0 >= 0 && x+0 < m_materialPointCount &&
         y-1 >= 0 && y-1 < m_materialPointCount )
    {
        test[0] = mat[2];
        test[1] = m_materialPoints[(x+0)+(y-1)*m_materialPointCount].mat[1];
        test[2] = m_materialPoints[(x+0)+(y-1)*m_materialPointCount].mat[2];
        test[3] = m_materialPoints[(x+0)+(y-1)*m_materialPointCount].mat[3];

        if ( FindMaterialByNeighbors(test) == -1 )  return false;
    }

    // Compatible with left neighbor?
    if ( x-1 >= 0 && x-1 < m_materialPointCount &&
         y+0 >= 0 && y+0 < m_materialPointCount )
    {
        test[0] = m_materialPoints[(x-1)+(y+0)*m_materialPointCount].mat[0];
        test[1] = mat[3];
        test[2] = m_materialPoints[(x-1)+(y+0)*m_materialPointCount].mat[2];
        test[3] = m_materialPoints[(x-1)+(y+0)*m_materialPointCount].mat[3];

        if ( FindMaterialByNeighbors(test) == -1 )  return false;
    }

    // Compatible with upper neighbor?
    if ( x+0 >= 0 && x+0 < m_materialPointCount &&
         y+1 >= 0 && y+1 < m_materialPointCount )
    {
        test[0] = m_materialPoints[(x+0)+(y+1)*m_materialPointCount].mat[0];
        test[1] = m_materialPoints[(x+0)+(y+1)*m_materialPointCount].mat[1];
        test[2] = mat[0];
        test[3] = m_materialPoints[(x+0)+(y+1)*m_materialPointCount].mat[3];

        if ( FindMaterialByNeighbors(test) == -1 )  return false;
    }

    // Compatible with right neighbor?
    if ( x+1 >= 0 && x+1 < m_materialPointCount &&
         y+0 >= 0 && y+0 < m_materialPointCount )
    {
        test[0] = m_materialPoints[(x+1)+(y+0)*m_materialPointCount].mat[0];
        test[1] = m_materialPoints[(x+1)+(y+0)*m_materialPointCount].mat[1];
        test[2] = m_materialPoints[(x+1)+(y+0)*m_materialPointCount].mat[2];
        test[3] = mat[1];

        if ( FindMaterialByNeighbors(test) == -1 )  return false;
    }

    SetMaterialPoint(x, y, id, mat);  // puts the point
    return true;
}

bool CTerrain::ChangeMaterialPoint(int x, int y, int id)
{
    char mat[4];

    x /= m_brickCount/m_textureSubdivCount;
    y /= m_brickCount/m_textureSubdivCount;

    if ( x < 0 || x >= m_materialPointCount ||
         y < 0 || y >= m_materialPointCount )  return false;

    TerrainMaterial* tm = FindMaterial(id);
    if (tm == nullptr)  return false;

    // Tries without changing neighbors.
    if ( CondChangeMaterialPoint(x, y, id, tm->mat) )  return true;

    // Tries changing a single neighbor (4x).
    for (int up = 0; up < m_maxMaterialID; up++)
    {
        mat[0] = up;
        mat[1] = tm->mat[1];
        mat[2] = tm->mat[2];
        mat[3] = tm->mat[3];

        if (CondChangeMaterialPoint(x, y, id, mat))  return true;
    }

    for (int right = 0; right < m_maxMaterialID; right++)
    {
        mat[0] = tm->mat[0];
        mat[1] = right;
        mat[2] = tm->mat[2];
        mat[3] = tm->mat[3];

        if (CondChangeMaterialPoint(x, y, id, mat)) return true;
    }

    for (int down = 0; down < m_maxMaterialID; down++)
    {
        mat[0] = tm->mat[0];
        mat[1] = tm->mat[1];
        mat[2] = down;
        mat[3] = tm->mat[3];

        if (CondChangeMaterialPoint(x, y, id, mat)) return true;
    }

    for (int left = 0; left < m_maxMaterialID; left++)
    {
        mat[0] = tm->mat[0];
        mat[1] = tm->mat[1];
        mat[2] = tm->mat[2];
        mat[3] = left;

        if (CondChangeMaterialPoint(x, y, id, mat)) return true;
    }

    // Tries changing two neighbors (6x).
    for (int up = 0; up < m_maxMaterialID; up++)
    {
        for (int down = 0; down < m_maxMaterialID; down++)
        {
            mat[0] = up;
            mat[1] = tm->mat[1];
            mat[2] = down;
            mat[3] = tm->mat[3];

            if (CondChangeMaterialPoint(x, y, id, mat)) return true;
        }
    }

    for (int right = 0; right < m_maxMaterialID; right++)
    {
        for (int left = 0; left < m_maxMaterialID; left++)
        {
            mat[0] = tm->mat[0];
            mat[1] = right;
            mat[2] = tm->mat[2];
            mat[3] = left;

            if (CondChangeMaterialPoint(x, y, id, mat)) return true;
        }
    }

    for (int up = 0; up < m_maxMaterialID; up++)
    {
        for (int right = 0; right < m_maxMaterialID; right++)
        {
            mat[0] = up;
            mat[1] = right;
            mat[2] = tm->mat[2];
            mat[3] = tm->mat[3];

            if (CondChangeMaterialPoint(x, y, id, mat)) return true;
        }
    }

    for (int right = 0; right < m_maxMaterialID; right++)
    {
        for (int down = 0; down < m_maxMaterialID; down++)
        {
            mat[0] = tm->mat[0];
            mat[1] = right;
            mat[2] = down;
            mat[3] = tm->mat[3];

            if (CondChangeMaterialPoint(x, y, id, mat)) return true;
        }
    }

    for (int down = 0; down < m_maxMaterialID; down++)
    {
        for (int left = 0; left < m_maxMaterialID; left++)
        {
            mat[0] = tm->mat[0];
            mat[1] = tm->mat[1];
            mat[2] = down;
            mat[3] = left;

            if (CondChangeMaterialPoint(x, y, id, mat)) return true;
        }
    }

    for (int up = 0; up < m_maxMaterialID; up++)
    {
        for (int left = 0; left < m_maxMaterialID; left++)
        {
            mat[0] = up;
            mat[1] = tm->mat[1];
            mat[2] = tm->mat[2];
            mat[3] = left;

            if (CondChangeMaterialPoint(x, y, id, mat)) return true;
        }
    }

    // Tries changing all the neighbors.
    for (int up = 0; up < m_maxMaterialID; up++)
    {
        for (int right = 0; right < m_maxMaterialID; right++)
        {
            for (int down = 0; down < m_maxMaterialID; down++)
            {
                for (int left = 0; left < m_maxMaterialID; left++)
                {
                    mat[0] = up;
                    mat[1] = right;
                    mat[2] = down;
                    mat[3] = left;

                    if (CondChangeMaterialPoint(x, y, id, mat)) return true;
                }
            }
        }
    }

    GetLogger()->Error("AddMaterialPoint error\n");
    return false;
}

bool CTerrain::InitMaterials(int id)
{
    TerrainMaterial* tm = FindMaterial(id);
    if (tm == nullptr) return false;

    for (int i = 0; i < m_materialPointCount*m_materialPointCount; i++)
    {
        m_materialPoints[i].id = id;

        for (int j = 0; j < 4; j++)
            m_materialPoints[i].mat[j] = tm->mat[j];
    }

    return true;
}

bool CTerrain::GenerateMaterials(int *id, float min, float max,
                                 float slope, float freq,
                                 Math::Vector center, float radius)
{
    static char random[100] =
    {
        84,25,12, 6,34,52,85,38,97,16,
        21,31,65,19,62,40,72,22,48,61,
        56,47, 8,53,73,77, 4,91,26,88,
        76, 1,44,93,39,11,71,17,98,95,
        88,83,18,30, 3,57,28,49,74, 9,
        32,13,96,66,15,70,36,10,59,94,
        45,86, 2,29,63,42,51, 0,79,27,
        54, 7,20,69,89,23,64,43,81,92,
        90,33,46,14,67,35,50, 5,87,60,
        68,55,24,78,41,75,58,80,37,82,
    };

    TerrainMaterial* tm = nullptr;

    int i = 0;
    while (id[i] != 0)
    {
        tm = FindMaterial(id[i++]);
        if (tm == nullptr) return false;
    }
    int numID = i;

    int group = m_brickCount / m_textureSubdivCount;

    if (radius > 0.0f && radius < 5.0f)  // just a square?
    {
        float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

        int xx = static_cast<int>((center.x+dim)/m_brickSize);
        int yy = static_cast<int>((center.z+dim)/m_brickSize);

        int x = xx/group;
        int y = yy/group;

        tm = FindMaterial(id[0]);
        if (tm != nullptr)
            SetMaterialPoint(x, y, id[0], tm->mat);  // puts the point
    }
    else
    {
        for (int y = 0; y < m_materialPointCount; y++)
        {
            for (int x = 0; x < m_materialPointCount; x++)
            {
                if (radius != 0.0f)
                {
                    Math::Vector pos;
                    pos.x = (static_cast<float>(x)-m_materialPointCount/2.0f)*group*m_brickSize;
                    pos.z = (static_cast<float>(y)-m_materialPointCount/2.0f)*group*m_brickSize;
                    if (Math::DistanceProjected(pos, center) > radius) continue;
                }

                if (freq < 100.0f)
                {
                    int rnd = random[(x%10)+(y%10)*10];
                    if ( static_cast<float>(rnd) > freq )  continue;
                }

                int xx = x*group + group/2;
                int yy = y*group + group/2;

                if (CheckMaterialPoint(xx, yy, min, max, slope))
                {
                    int rnd = random[(x%10)+(y%10)*10];
                    int ii = rnd % numID;
                    ChangeMaterialPoint(xx, yy, id[ii]);
                }
            }
        }
    }

    return true;
}

void CTerrain::InitMaterialPoints()
{
    if (! m_useMaterials) return;
    if (! m_materialPoints.empty()) return; // already allocated

    m_materialPointCount = (m_mosaicCount*m_brickCount)/(m_brickCount/m_textureSubdivCount)+1;
    std::vector<TerrainMaterialPoint>(m_materialPointCount*m_materialPointCount).swap(m_materialPoints);

    for (int i = 0; i < m_materialPointCount * m_materialPointCount; i++)
    {
        for (int j = 0; j < 4; j++)
            m_materialPoints[i].mat[j] = 0;
    }
}

void CTerrain::FlushMaterialPoints()
{
    m_materialPoints.clear();
}

bool CTerrain::CreateSquare(int x, int y)
{
    Material mat;
    mat.diffuse = Color(1.0f, 1.0f, 1.0f);
    mat.ambient = Color(0.0f, 0.0f, 0.0f);

    int objRank = m_engine->CreateObject();
    m_engine->SetObjectType(objRank, ENG_OBJTYPE_TERRAIN);

    m_objRanks[x+y*m_mosaicCount] = objRank;

    for (int step = 0; step < m_depth; step++)
    {
        CreateMosaic(x, y, 1 << step, objRank, mat);
    }

    return true;
}

bool CTerrain::CreateObjects()
{
    AdjustRelief();

    for (int y = 0; y < m_mosaicCount; y++)
    {
        for (int x = 0; x < m_mosaicCount; x++)
            CreateSquare(x, y);
    }

    return true;
}

/** ATTENTION: ok only with m_depth = 2! */
bool CTerrain::Terraform(const Math::Vector &p1, const Math::Vector &p2, float height)
{
    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    Math::IntPoint tp1, tp2;
    tp1.x = static_cast<int>((p1.x+dim+m_brickSize/2.0f)/m_brickSize);
    tp1.y = static_cast<int>((p1.z+dim+m_brickSize/2.0f)/m_brickSize);
    tp2.x = static_cast<int>((p2.x+dim+m_brickSize/2.0f)/m_brickSize);
    tp2.y = static_cast<int>((p2.z+dim+m_brickSize/2.0f)/m_brickSize);

    if (tp1.x > tp2.x)
    {
        int x = tp1.x;
        tp1.x = tp2.x;
        tp2.x = x;
    }

    if (tp1.y > tp2.y)
    {
        int y = tp1.y;
        tp1.y = tp2.y;
        tp2.y = y;
    }

    int size = (m_mosaicCount*m_brickCount)+1;

    // Calculates the current average height
    float avg = 0.0f;
    int nb = 0;
    for (int y = tp1.y; y <= tp2.y; y++)
    {
        for (int x = tp1.x; x <= tp2.x; x++)
        {
            avg += m_relief[x+y*size];
            nb ++;
        }
    }
    avg /= static_cast<float>(nb);

    // Changes the description of the relief
    for (int y = tp1.y; y <= tp2.y; y++)
    {
        for (int x = tp1.x; x <= tp2.x; x++)
        {
            m_relief[x+y*size] = avg+height;

            if (x % m_brickCount == 0 && y % m_depth != 0)
            {
                m_relief[(x+0)+(y-1)*size] = avg+height;
                m_relief[(x+0)+(y+1)*size] = avg+height;
            }

            if (y % m_brickCount == 0 && x % m_depth != 0)
            {
                m_relief[(x-1)+(y+0)*size] = avg+height;
                m_relief[(x+1)+(y+0)*size] = avg+height;
            }
        }
    }
    AdjustRelief();

    Math::IntPoint pp1, pp2;
    pp1.x = (tp1.x-2)/m_brickCount;
    pp1.y = (tp1.y-2)/m_brickCount;
    pp2.x = (tp2.x+1)/m_brickCount;
    pp2.y = (tp2.y+1)/m_brickCount;

    if (pp1.x <  0            ) pp1.x = 0;
    if (pp1.x >= m_mosaicCount) pp1.x = m_mosaicCount-1;
    if (pp1.y <  0            ) pp1.y = 0;
    if (pp1.y >= m_mosaicCount) pp1.y = m_mosaicCount-1;

    for (int y = pp1.y; y <= pp2.y; y++)
    {
        for (int x = pp1.x; x <= pp2.x; x++)
        {
            int objRank = m_objRanks[x+y*m_mosaicCount];
            int baseObjRank = m_engine->GetObjectBaseRank(objRank);
            m_engine->DeleteBaseObject(baseObjRank);
            m_engine->DeleteObject(objRank);
            CreateSquare(x, y);  // recreates the square
        }
    }
    m_engine->Update();

    return true;
}

void CTerrain::SetWind(Math::Vector speed)
{
    m_wind = speed;
}

Math::Vector CTerrain::GetWind()
{
    return m_wind;
}

float CTerrain::GetFineSlope(const Math::Vector &pos)
{
    Math::Vector n;
    if (! GetNormal(n, pos)) return 0.0f;
    return fabs(Math::RotateAngle(Math::Point(n.x, n.z).Length(), n.y) - Math::PI/2.0f);
}

float CTerrain::GetCoarseSlope(const Math::Vector &pos)
{
    if (m_relief.empty()) return 0.0f;

    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    int x = static_cast<int>((pos.x+dim)/m_brickSize);
    int y = static_cast<int>((pos.z+dim)/m_brickSize);

    if ( x < 0 || x >= m_mosaicCount*m_brickCount ||
         y < 0 || y >= m_mosaicCount*m_brickCount ) return 0.0f;

    float level[4] =
    {
        m_relief[(x+0)+(y+0)*(m_mosaicCount*m_brickCount+1)],
        m_relief[(x+1)+(y+0)*(m_mosaicCount*m_brickCount+1)],
        m_relief[(x+0)+(y+1)*(m_mosaicCount*m_brickCount+1)],
        m_relief[(x+1)+(y+1)*(m_mosaicCount*m_brickCount+1)],
    };

    float min = Math::Min(level[0], level[1], level[2], level[3]);
    float max = Math::Max(level[0], level[1], level[2], level[3]);

    return atanf((max-min)/m_brickSize);
}

bool CTerrain::GetNormal(Math::Vector &n, const Math::Vector &p)
{
    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    int x = static_cast<int>((p.x+dim)/m_brickSize);
    int y = static_cast<int>((p.z+dim)/m_brickSize);

    if ( x < 0 || x > m_mosaicCount*m_brickCount ||
         y < 0 || y > m_mosaicCount*m_brickCount )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    if ( fabs(p.z - p2.z) < fabs(p.x - p2.x) )
        n = Math::NormalToPlane(p1,p2,p3);
    else
        n = Math::NormalToPlane(p2,p4,p3);

    return true;
}

float CTerrain::GetFloorLevel(const Math::Vector &pos, bool brut, bool water)
{
    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    int x = static_cast<int>((pos.x+dim)/m_brickSize);
    int y = static_cast<int>((pos.z+dim)/m_brickSize);

    if ( x < 0 || x > m_mosaicCount*m_brickCount ||
         y < 0 || y > m_mosaicCount*m_brickCount )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    Math::Vector ps = pos;
    if ( fabs(pos.z-p2.z) < fabs(pos.x-p2.x) )
    {
        if ( !IntersectY(p1, p2, p3, ps) )  return 0.0f;
    }
    else
    {
        if ( !IntersectY(p2, p4, p3, ps) )  return 0.0f;
    }

    if (! brut) AdjustBuildingLevel(ps);

    if (water)  // not going underwater?
    {
        float level = m_water->GetLevel();
        if (ps.y < level) ps.y = level;  // not under water
    }

    return ps.y;
}

float CTerrain::GetHeightToFloor(const Math::Vector &pos, bool brut, bool water)
{
    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    int x = static_cast<int>((pos.x+dim)/m_brickSize);
    int y = static_cast<int>((pos.z+dim)/m_brickSize);

    if ( x < 0 || x > m_mosaicCount*m_brickCount ||
         y < 0 || y > m_mosaicCount*m_brickCount )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    Math::Vector ps = pos;
    if ( fabs(pos.z-p2.z) < fabs(pos.x-p2.x) )
    {
        if ( !IntersectY(p1, p2, p3, ps) )  return 0.0f;
    }
    else
    {
        if ( !IntersectY(p2, p4, p3, ps) )  return 0.0f;
    }

    if (! brut) AdjustBuildingLevel(ps);

    if (water)  // not going underwater?
    {
        float level = m_water->GetLevel();
        if (ps.y < level ) ps.y = level;  // not under water
    }

    return pos.y-ps.y;
}

bool CTerrain::AdjustToFloor(Math::Vector &pos, bool brut, bool water)
{
    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    int x = static_cast<int>((pos.x + dim) / m_brickSize);
    int y = static_cast<int>((pos.z + dim) / m_brickSize);

    if ( x < 0 || x > m_mosaicCount*m_brickCount ||
         y < 0 || y > m_mosaicCount*m_brickCount )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    if (fabs(pos.z - p2.z) < fabs(pos.x - p2.x))
    {
        if (! Math::IntersectY(p1, p2, p3, pos)) return false;
    }
    else
    {
        if (! Math::IntersectY(p2, p4, p3, pos)) return false;
    }

    if (! brut) AdjustBuildingLevel(pos);

    if (water)  // not going underwater?
    {
        float level = m_water->GetLevel();
        if (pos.y < level) pos.y = level;  // not under water
    }

    return true;
}

/**
 * \param pos position to adjust
 * \returns \c false if the initial coordinate was outside terrain area; \c true otherwise
 */
bool CTerrain::AdjustToStandardBounds(Math::Vector& pos)
{
    bool ok = true;

    // In _TEEN there used to be a limit of 0.98f
    float limit = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f*0.92f;

    if (pos.x < -limit)
    {
        pos.x = -limit;
        ok = false;
    }

    if (pos.z < -limit)
    {
        pos.z = -limit;
        ok = false;
    }

    if (pos.x > limit)
    {
        pos.x = limit;
        ok = false;
    }

    if (pos.z > limit)
    {
        pos.z = limit;
        ok = false;
    }

    return ok;
}

/**
 * \param pos position to adjust
 * \param margin margin to the terrain border
 * \returns \c false if the initial coordinate was outside terrain area; \c true otherwise
 */
bool CTerrain::AdjustToBounds(Math::Vector& pos, float margin)
{
    bool ok = true;
    float limit = m_mosaicCount*m_brickCount*m_brickSize/2.0f - margin;

    if (pos.x < -limit)
    {
        pos.x = -limit;
        ok = false;
    }

    if (pos.z < -limit)
    {
        pos.z = -limit;
        ok = false;
    }

    if (pos.x > limit)
    {
        pos.x = limit;
        ok = false;
    }

    if (pos.z > limit)
    {
        pos.z = limit;
        ok = false;
    }

    return ok;
}

void CTerrain::FlushBuildingLevel()
{
    m_buildingLevels.clear();
}

bool CTerrain::AddBuildingLevel(Math::Vector center, float min, float max,
                                     float height, float factor)
{
    int i = 0;
    for ( ; i < static_cast<int>( m_buildingLevels.size() ); i++)
    {
        if ( center.x == m_buildingLevels[i].center.x &&
             center.z == m_buildingLevels[i].center.z )
        {
            break;
        }
    }

    if (i == static_cast<int>( m_buildingLevels.size() ))
        m_buildingLevels.push_back(BuildingLevel());

    m_buildingLevels[i].center   = center;
    m_buildingLevels[i].min      = min;
    m_buildingLevels[i].max      = max;
    m_buildingLevels[i].level    = GetFloorLevel(center, true);
    m_buildingLevels[i].height   = height;
    m_buildingLevels[i].factor   = factor;
    m_buildingLevels[i].bboxMinX = center.x-max;
    m_buildingLevels[i].bboxMaxX = center.x+max;
    m_buildingLevels[i].bboxMinZ = center.z-max;
    m_buildingLevels[i].bboxMaxZ = center.z+max;

    return true;
}

bool CTerrain::UpdateBuildingLevel(Math::Vector center)
{
    for (int i = 0; i < static_cast<int>( m_buildingLevels.size() ); i++)
    {
        if ( center.x == m_buildingLevels[i].center.x &&
             center.z == m_buildingLevels[i].center.z )
        {
            m_buildingLevels[i].center = center;
            m_buildingLevels[i].level  = GetFloorLevel(center, true);
            return true;
        }
    }
    return false;
}

bool CTerrain::DeleteBuildingLevel(Math::Vector center)
{
    for (int i = 0; i < static_cast<int>( m_buildingLevels.size() ); i++)
    {
        if ( center.x == m_buildingLevels[i].center.x &&
             center.z == m_buildingLevels[i].center.z )
        {
            for (int j = i+1; j < static_cast<int>( m_buildingLevels.size() ); j++)
                m_buildingLevels[j-1] = m_buildingLevels[j];

            m_buildingLevels.pop_back();
            return true;
        }
    }
    return false;
}

float CTerrain::GetBuildingFactor(const Math::Vector &p)
{
    for (int i = 0; i < static_cast<int>( m_buildingLevels.size() ); i++)
    {
        if ( p.x < m_buildingLevels[i].bboxMinX ||
             p.x > m_buildingLevels[i].bboxMaxX ||
             p.z < m_buildingLevels[i].bboxMinZ ||
             p.z > m_buildingLevels[i].bboxMaxZ )  continue;

        float dist = Math::DistanceProjected(p, m_buildingLevels[i].center);

        if (dist <= m_buildingLevels[i].max)
            return m_buildingLevels[i].factor;
    }
    return 1.0f;  // it is normal on the ground
}

void CTerrain::AdjustBuildingLevel(Math::Vector &p)
{
    for (int i = 0; i < static_cast<int>( m_buildingLevels.size() ); i++)
    {
        if ( p.x < m_buildingLevels[i].bboxMinX ||
             p.x > m_buildingLevels[i].bboxMaxX ||
             p.z < m_buildingLevels[i].bboxMinZ ||
             p.z > m_buildingLevels[i].bboxMaxZ ) continue;

        float dist = Math::DistanceProjected(p, m_buildingLevels[i].center);

        if (dist > m_buildingLevels[i].max) continue;

        if (dist < m_buildingLevels[i].min)
        {
            p.y = m_buildingLevels[i].level + m_buildingLevels[i].height;
            return;
        }

        Math::Vector border;
        border.x = ((p.x - m_buildingLevels[i].center.x) * m_buildingLevels[i].max) /
                   dist + m_buildingLevels[i].center.x;
        border.z = ((p.z - m_buildingLevels[i].center.z) * m_buildingLevels[i].max) /
                   dist + m_buildingLevels[i].center.z;

        float base = GetFloorLevel(border, true);

        p.y = (m_buildingLevels[i].max - dist) /
              (m_buildingLevels[i].max - m_buildingLevels[i].min) *
              (m_buildingLevels[i].level + m_buildingLevels[i].height-base) +
              base;

        return;
    }
}

float CTerrain::GetHardness(const Math::Vector &p)
{
    float factor = GetBuildingFactor(p);
    if (factor != 1.0f) return 1.0f;  // on building level

    if (m_materialPoints.empty()) return m_defaultHardness;

    float dim = (m_mosaicCount*m_brickCount*m_brickSize)/2.0f;

    int x, y;

    x = static_cast<int>((p.x+dim)/m_brickSize);
    y = static_cast<int>((p.z+dim)/m_brickSize);

    if ( x < 0 || x > m_mosaicCount*m_brickCount ||
         y < 0 || y > m_mosaicCount*m_brickCount )  return m_defaultHardness;

    x /= m_brickCount/m_textureSubdivCount;
    y /= m_brickCount/m_textureSubdivCount;

    if ( x < 0 || x >= m_materialPointCount ||
         y < 0 || y >= m_materialPointCount )  return m_defaultHardness;

    int id = m_materialPoints[x+y*m_materialPointCount].id;
    TerrainMaterial* tm = FindMaterial(id);
    if (tm == nullptr) return m_defaultHardness;

    return tm->hardness;
}

void CTerrain::ShowFlatGround(Math::Vector pos)
{
    static char table[41*41] = { 1 };

    float radius = 3200.0f/1024.0f;

    for (int y = 0; y <= 40; y++)
    {
        for (int x = 0; x <= 40; x++)
        {
            int i = x + y*41;
            table[i] = 0;

            Math::Vector p;
            p.x = (x-20)*radius;
            p.z = (y-20)*radius;
            p.y = 0.0f;

            if (Math::Point(p.x, p.y).Length() > 20.0f*radius)
                continue;

            float angle = GetFineSlope(pos+p);

            if (angle < TERRAIN_FLATLIMIT)
                table[i] = 1;
            else
                table[i] = 2;
        }
    }

    m_engine->CreateGroundMark(pos, 40.0f, 0.001f, 15.0f, 0.001f, 41, 41, table);
}

float CTerrain::GetFlatZoneRadius(Math::Vector center, float max)
{
    float angle = GetFineSlope(center);
    if (angle >= TERRAIN_FLATLIMIT)
        return 0.0f;

    float ref = GetFloorLevel(center, true);
    Math::Point c(center.x, center.z);
    float radius = 1.0f;

    while (radius <= max)
    {
        angle = 0.0f;
        int nb = static_cast<int>(2.0f*Math::PI*radius);
        if (nb < 8) nb = 8;

        Math::Point p (center.x+radius, center.z);
        for (int i = 0; i < nb; i++)
        {
            Math::Point result = Math::RotatePoint(c, angle, p);
            Math::Vector pos;
            pos.x = result.x;
            pos.z = result.y;
            float h = GetFloorLevel(pos, true);
            if ( fabs(h-ref) > 1.0f )  return radius;

            angle += Math::PI*2.0f/8.0f;
        }
        radius += 1.0f;
    }
    return max;
}

void CTerrain::SetFlyingMaxHeight(float height)
{
    m_flyingMaxHeight = height;
}

float CTerrain::GetFlyingMaxHeight()
{
    return m_flyingMaxHeight;
}

void CTerrain::FlushFlyingLimit()
{
    m_flyingMaxHeight = 280.0f;
    m_flyingLimits.clear();
}

void CTerrain::AddFlyingLimit(Math::Vector center,
                                   float extRadius, float intRadius,
                                   float maxHeight)
{
    FlyingLimit fl;
    fl.center    = center;
    fl.extRadius = extRadius;
    fl.intRadius = intRadius;
    fl.maxHeight = maxHeight;
    m_flyingLimits.push_back(fl);
}

float CTerrain::GetFlyingLimit(Math::Vector pos, bool noLimit)
{
    if (noLimit)
        return 280.0f;

    if (m_flyingLimits.empty())
        return m_flyingMaxHeight;

    for (int i = 0; i < static_cast<int>( m_flyingLimits.size() ); i++)
    {
        float dist = Math::DistanceProjected(pos, m_flyingLimits[i].center);

        if (dist >= m_flyingLimits[i].extRadius)
            continue;

        if (dist <= m_flyingLimits[i].intRadius)
            return m_flyingLimits[i].maxHeight;

        dist -= m_flyingLimits[i].intRadius;

        float h = dist * (m_flyingMaxHeight - m_flyingLimits[i].maxHeight) /
                  (m_flyingLimits[i].extRadius - m_flyingLimits[i].intRadius);

        return h + m_flyingLimits[i].maxHeight;
    }

    return m_flyingMaxHeight;
}


} // namespace Gfx
