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

// terrain.cpp

#include "graphics/engine/terrain.h"

#include "app/app.h"
#include "common/iman.h"
#include "common/image.h"
#include "common/logger.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/water.h"
#include "math/geometry.h"

#include <sstream>

#include <SDL/SDL.h>

const int LEVEL_MAT_PREALLOCATE_COUNT = 101;
const int FLYING_LIMIT_PREALLOCATE_COUNT = 10;
const int BUILDING_LEVEL_PREALLOCATE_COUNT = 101;


Gfx::CTerrain::CTerrain(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_TERRAIN, this);

    m_engine = static_cast<Gfx::CEngine*>( m_iMan->SearchInstance(CLASS_ENGINE) );
    m_water  = static_cast<Gfx::CWater*>( m_iMan->SearchInstance(CLASS_WATER) );

    m_mosaic        = 20;
    m_brick         = 1 << 4;
    m_size          = 10.0f;
    m_vision        = 200.0f;
    m_scaleMapping  = 0.01f;
    m_scaleRelief   = 1.0f;
    m_subdivMapping = 1;
    m_depth         = 2;
    m_levelMatMax   = 0;
    m_multiText     = true;
    m_levelText     = false;
    m_wind          = Math::Vector(0.0f, 0.0f, 0.0f);
    m_defHardness   = 0.5f;

    m_levelMats.reserve(LEVEL_MAT_PREALLOCATE_COUNT);
    m_flyingLimits.reserve(FLYING_LIMIT_PREALLOCATE_COUNT);
    m_buildingLevels.reserve(BUILDING_LEVEL_PREALLOCATE_COUNT);
}

Gfx::CTerrain::~CTerrain()
{
}

/**
  The terrain is composed of mosaics, themselves composed of bricks.
  Each brick is composed of two triangles.
  mosaic:       number of mosaics along the axes X and Z
  brick:        number of bricks (power of 2)
  size:         size of a brick along the axes X and Z
  vision:       vision before a change of resolution
  scaleMapping: scale textures for mapping

\verbatim
            ^ z
            |   <--->  brick*size
    +---+---+---+---+
    |   |   |   |_|_|  mosaic = 4
    |   |   |   | | |  brick  = 2 (brickP2=1)
    +---+---+---+---+
    |\ \|   |   |   |
    |\ \|   |   |   |
    +---+---o---+---+---> x
    |   |   |   |   |
    |   |   |   |   |
    +---+---+---+---+
    |   |   |   |   |  The land is viewed from above here.
    |   |   |   |   |
    +---+---+---+---+
        <--------------->  mosaic*brick*size
\endverbatim */
bool Gfx::CTerrain::Generate(int mosaic, int brickPow2, float size, float vision,
                             int depth, float hardness)
{
    m_mosaic        = mosaic;
    m_brick         = 1 << brickPow2;
    m_size          = size;
    m_vision        = vision;
    m_depth         = depth;
    m_defHardness   = hardness;

    m_engine->SetTerrainVision(vision);

    m_multiText    = true;
    m_levelText    = false;
    m_scaleMapping  = 1.0f / (m_brick*m_size);
    m_subdivMapping = 1;

    int dim = 0;

    dim = (m_mosaic*m_brick+1)*(m_mosaic*m_brick+1);
    std::vector<float>(dim).swap(m_relief);

    dim = m_mosaic*m_subdivMapping*m_mosaic*m_subdivMapping;
    std::vector<int>(dim).swap(m_texture);

    dim = m_mosaic*m_mosaic;
    std::vector<int>(dim).swap(m_objRank);

    return true;
}


int Gfx::CTerrain::GetMosaic()
{
    return m_mosaic;
}

int Gfx::CTerrain::GetBrick()
{
    return m_brick;
}

float Gfx::CTerrain::GetSize()
{
    return m_size;
}

float Gfx::CTerrain::GetScaleRelief()
{
    return m_scaleRelief;
}

bool Gfx::CTerrain::InitTextures(const std::string& baseName, int* table, int dx, int dy)
{
    m_levelText = false;
    m_texBaseName = baseName;
    size_t pos = baseName.find('.');
    if (pos == baseName.npos)
    {
        m_texBaseExt = ".png";
    }
    else
    {
        m_texBaseName = m_texBaseName.substr(0, pos);
        m_texBaseExt = m_texBaseName.substr(pos);
    }

    for (int y = 0; y < m_mosaic*m_subdivMapping; y++)
    {
        for (int x = 0; x < m_mosaic*m_subdivMapping; x++)
        {
            m_texture[x+y*m_mosaic] = table[(x%dx)+(y%dy)*dx];
        }
    }
    return true;
}


void Gfx::CTerrain::LevelFlush()
{
    m_levelMats.clear();
    m_levelMatMax = 0;
    m_levelID = 1000;
    LevelCloseTable();
}

void Gfx::CTerrain::LevelMaterial(int id, std::string& baseName, float u, float v,
                                 int up, int right, int down, int left,
                                 float hardness)
{
    LevelOpenTable();

    if (id == 0)
        id = m_levelID++;  // puts an ID internal standard

    Gfx::TerrainMaterial tm;
    tm.texName  = baseName;
    tm.id       = id;
    tm.u        = u;
    tm.v        = v;
    tm.mat[0]   = up;
    tm.mat[1]   = right;
    tm.mat[2]   = down;
    tm.mat[3]   = left;
    tm.hardness = hardness;

    m_levelMats.push_back(tm);

    if (m_levelMatMax < up+1   )  m_levelMatMax = up+1;
    if (m_levelMatMax < right+1)  m_levelMatMax = right+1;
    if (m_levelMatMax < down+1 )  m_levelMatMax = down+1;
    if (m_levelMatMax < left+1 )  m_levelMatMax = left+1;

    m_levelText = true;
    m_subdivMapping = 4;
}


/**
  The size of the image must be dimension dx and dy with dx=dy=(mosaic*brick)+1.
  The image must be 24 bits/pixel

  Converts coordinated image (x;y) -> world (x;-;z) :
  Wx =   5*Ix-400
  Wz = -(5*Iy-400)

  Converts coordinated world (x;-;z) -> image (x;y) :
  Ix = (400+Wx)/5
  Iy = (400-Wz)/5 */
bool Gfx::CTerrain::ResFromPNG(const std::string& fileName)
{
    CImage img;
    if (! img.Load(CApplication::GetInstance().GetDataFilePath(m_engine->GetTextureDir(), fileName)))
        return false;

    ImageData *data = img.GetData();

    int size  = (m_mosaic*m_brick)+1;

    m_resources.clear();

    std::vector<unsigned char>(3*size*size).swap(m_resources);

    if ( (data->surface->w != size) || (data->surface->h != size) ||
         (data->surface->format->BytesPerPixel != 3) )
        return false;

    // Assuming the data format is compatible
    memcpy(&m_resources[0], data->surface->pixels, 3*size*size);

    return true;
}

Gfx::TerrainRes Gfx::CTerrain::GetResource(const Math::Vector &p)
{
    if (m_resources.empty())
        return Gfx::TR_NULL;

    int x = static_cast<int>((p.x + (m_mosaic*m_brick*m_size)/2.0f)/m_size);
    int y = static_cast<int>((p.z + (m_mosaic*m_brick*m_size)/2.0f)/m_size);

    if ( x < 0 || x > m_mosaic*m_brick ||
         y < 0 || y > m_mosaic*m_brick )
        return Gfx::TR_NULL;

    int size  = (m_mosaic*m_brick)+1;

    int resR = m_resources[3*x+3*size*(size-y-1)];
    int resG = m_resources[3*x+3*size*(size-y-1)+1];
    int resB = m_resources[3*x+3*size*(size-y-1)+2];

    if (resR == 255 && resG ==   0 && resB == 0) return Gfx::TR_STONE;
    if (resR == 255 && resG == 255 && resB == 0) return Gfx::TR_URANIUM;
    if (resR ==   0 && resG == 255 && resB == 0) return Gfx::TR_POWER;

    // TODO key res values
    //if (ress == 24) return Gfx::TR_KEY_A;     // ~green?
    //if (ress == 25) return Gfx::TR_KEY_B;     // ~green?
    //if (ress == 26) return Gfx::TR_KEY_C;     // ~green?
    //if (ress == 27) return Gfx::TR_KEY_D;     // ~green?

    return TR_NULL;
}

void Gfx::CTerrain::FlushRelief()
{
    m_relief.clear();
}

/**
  The size of the image must be dimension dx and dy with dx=dy=(mosaic*brick)+1.
  The image must be 24 bits/pixel, but gray scale:
  white = ground (y=0)
  black = mountain (y=255*scaleRelief)

  Converts coordinated image(x;y) -> world (x;-;z) :
  Wx =   5*Ix-400
  Wz = -(5*Iy-400)

  Converts coordinated world (x;-;z) -> image (x;y) :
  Ix = (400+Wx)/5
  Iy = (400-Wz)/5 */
bool Gfx::CTerrain::ReliefFromPNG(const std::string &fileName, float scaleRelief,
                                  bool adjustBorder)
{
    m_scaleRelief = scaleRelief;

    CImage img;
    if (! img.Load(CApplication::GetInstance().GetDataFilePath(m_engine->GetTextureDir(), fileName)))
        return false;

    ImageData *data = img.GetData();

    int size = (m_mosaic*m_brick)+1;

    if ( (data->surface->w != size) || (data->surface->h != size) ||
         (data->surface->format->BytesPerPixel != 3) )
        return false;

    unsigned char* pixels = static_cast<unsigned char*>(data->surface->pixels);

    float limit = 0.9f;
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            float level = (255 - pixels[3*x+3*size*(size-y-1)]) * scaleRelief;

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

bool Gfx::CTerrain::ReliefAddDot(Math::Vector pos, float scaleRelief)
{
    float dim = (m_mosaic*m_brick*m_size)/2.0f;
    int size = (m_mosaic*m_brick)+1;

    pos.x = (pos.x+dim)/m_size;
    pos.z = (pos.z+dim)/m_size;

    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.z);

    if ( x < 0 || x >= size ||
         y < 0 || y >= size )  return false;

    if (m_relief[x+y*size] < pos.y*scaleRelief)
        m_relief[x+y*size] = pos.y*scaleRelief;

    return true;
}

void Gfx::CTerrain::LimitPos(Math::Vector &pos)
{
// TODO: #if _TEEN
//    dim = (m_mosaic*m_brick*m_size)/2.0f*0.98f;

    float dim = (m_mosaic*m_brick*m_size)/2.0f*0.92f;

    if (pos.x < -dim) pos.x = -dim;
    if (pos.x >  dim) pos.x =  dim;
    if (pos.z < -dim) pos.z = -dim;
    if (pos.z >  dim) pos.z =  dim;
}

void Gfx::CTerrain::AdjustRelief()
{
    if (m_depth == 1)  return;

    int ii = m_mosaic*m_brick+1;
    int b = 1 << (m_depth-1);

    for (int y = 0; y < m_mosaic*m_brick; y += b)
    {
        for (int x = 0; x < m_mosaic*m_brick; x += b)
        {
            int xx = 0;
            int yy = 0;
            if ((y+yy)%m_brick == 0)
            {
                float level1 = m_relief[(x+0)+(y+yy)*ii];
                float level2 = m_relief[(x+b)+(y+yy)*ii];
                for (xx = 1; xx < b; xx++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*xx+level1;
                }
            }

            yy = b;
            if ((y+yy)%m_brick == 0)
            {
                float level1 = m_relief[(x+0)+(y+yy)*ii];
                float level2 = m_relief[(x+b)+(y+yy)*ii];
                for (xx = 1; xx < b; xx++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*xx+level1;
                }
            }

            xx = 0;
            if ((x+xx)%m_brick == 0)
            {
                float level1 = m_relief[(x+xx)+(y+0)*ii];
                float level2 = m_relief[(x+xx)+(y+b)*ii];
                for (yy = 1; yy < b; yy++)
                {
                    m_relief[(x+xx)+(y+yy)*ii] = ((level2-level1)/b)*yy+level1;
                }
            }

            xx = b;
            if ((x+xx)%m_brick == 0)
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

Math::Vector Gfx::CTerrain::GetVector(int x, int y)
{
    Math::Vector p;
    p.x = x*m_size - (m_mosaic*m_brick*m_size)/2;
    p.z = y*m_size - (m_mosaic*m_brick*m_size)/2;

    if ( !m_relief.empty()               &&
         x >= 0 && x <= m_mosaic*m_brick &&
         y >= 0 && y <= m_mosaic*m_brick )
    {
        p.y = m_relief[x+y*(m_mosaic*m_brick+1)];
    }
    else
    {
        p.y = 0.0f;
    }

    return p;
}

/** Calculates a normal soft, taking into account the six adjacent triangles:

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
Gfx::VertexTex2 Gfx::CTerrain::GetVertex(int x, int y, int step)
{
    Gfx::VertexTex2 v;

    Math::Vector o = GetVector(x, y);
    v.coord = o;

    Math::Vector a = GetVector(x-step, y     );
    Math::Vector b = GetVector(x-step, y+step);
    Math::Vector c = GetVector(x,      y+step);
    Math::Vector d = GetVector(x+step, y     );
    Math::Vector e = GetVector(x+step, y-step);
    Math::Vector f = GetVector(x,      y-step);

    Math::Vector s(0.0f, 0.0f, 0.0f);

    if (x-step >= 0 && y+step <= m_mosaic*m_brick+1)
    {
        s += Math::NormalToPlane(b,a,o);
        s += Math::NormalToPlane(c,b,o);
    }

    if (x+step <= m_mosaic*m_brick+1 && y+step <= m_mosaic*m_brick+1)
        s += Math::NormalToPlane(d,c,o);

    if (x+step <= m_mosaic*m_brick+1 && y-step >= 0)
    {
        s += Math::NormalToPlane(e,d,o);
        s += Math::NormalToPlane(f,e,o);
    }

    if (x-step >= 0 && y-step >= 0)
        s += Math::NormalToPlane(a,f,o);

    s = Normalize(s);
    v.normal = s;

    if (m_multiText)
    {
        int brick = m_brick/m_subdivMapping;
        Math::Vector oo = GetVector((x/brick)*brick, (y/brick)*brick);
        o  = GetVector(x, y);
        v.texCoord.x =        (o.x-oo.x)*m_scaleMapping*m_subdivMapping;
        v.texCoord.y = 1.0f - (o.z-oo.z)*m_scaleMapping*m_subdivMapping;
    }
    else
    {
        v.texCoord.x = o.x*m_scaleMapping;
        v.texCoord.y = o.z*m_scaleMapping;
    }

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
bool Gfx::CTerrain::CreateMosaic(int ox, int oy, int step, int objRank,
                                 const Gfx::Material &mat,
                                 float min, float max)
{
    std::string texName1;
    std::string texName2;

    if ( step == 1 && m_engine->GetGroundSpot() )
    {
        int i = (ox/5) + (oy/5)*(m_mosaic/5);
        std::stringstream s;
        s << "shadow";
        s.width(2);
        s.fill('0');
        s << i;
        s << ".png";
        texName2 = s.str();
    }

    int brick = m_brick/m_subdivMapping;

    Gfx::VertexTex2 o = GetVertex(ox*m_brick+m_brick/2, oy*m_brick+m_brick/2, step);
    int total = ((brick/step)+1)*2;

    float pixel = 1.0f/256.0f;  // 1 pixel cover (*)
    float dp = 1.0f/512.0f;

    Math::Point uv;

    for (int my = 0; my < m_subdivMapping; my++)
    {
        for (int mx = 0; mx < m_subdivMapping; mx++)
        {
            if (m_levelText)
            {
                int xx = ox*m_brick + mx*(m_brick/m_subdivMapping);
                int yy = oy*m_brick + my*(m_brick/m_subdivMapping);
                LevelTextureName(xx, yy, texName1, uv);
            }
            else
            {
                int i = (ox*m_subdivMapping+mx)+(oy*m_subdivMapping+my)*m_mosaic;
                std::stringstream s;
                s << m_texBaseName;
                s.width(3);
                s.fill('0');
                s << m_texture[i];
                s << m_texBaseExt;
                texName1 = s.str();
            }

            for (int y = 0; y < brick; y += step)
            {
                Gfx::EngineObjLevel4 buffer;
                buffer.vertices.reserve(total);

                buffer.type = Gfx::ENG_TRIANGLE_TYPE_SURFACE;
                buffer.material = mat;

                buffer.state = Gfx::ENG_RSTATE_WRAP;

                buffer.state |= Gfx::ENG_RSTATE_SECOND;
                if (step == 1)
                    buffer.state |= Gfx::ENG_RSTATE_DUAL_BLACK;

                for (int x = 0; x <= brick; x += step)
                {
                    Gfx::VertexTex2 p1 = GetVertex(ox*m_brick+mx*brick+x, oy*m_brick+my*brick+y+0   , step);
                    Gfx::VertexTex2 p2 = GetVertex(ox*m_brick+mx*brick+x, oy*m_brick+my*brick+y+step, step);
                    p1.coord.x -= o.coord.x;  p1.coord.z -= o.coord.z;
                    p2.coord.x -= o.coord.x;  p2.coord.z -= o.coord.z;

                    if (m_multiText)
                    {
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
                    }

                    if (m_levelText)
                    {
                        p1.texCoord.x /= m_subdivMapping;  // 0..1 -> 0..0.25
                        p1.texCoord.y /= m_subdivMapping;
                        p2.texCoord.x /= m_subdivMapping;
                        p2.texCoord.y /= m_subdivMapping;

                        if (x == 0)
                        {
                            p1.texCoord.x = 0.0f+dp;
                            p2.texCoord.x = 0.0f+dp;
                        }
                        if (x == brick)
                        {
                            p1.texCoord.x = (1.0f/m_subdivMapping)-dp;
                            p2.texCoord.x = (1.0f/m_subdivMapping)-dp;
                        }
                        if (y == 0)
                            p1.texCoord.y = (1.0f/m_subdivMapping)-dp;

                        if (y == brick - step)
                            p2.texCoord.y = 0.0f+dp;

                        p1.texCoord.x += uv.x;
                        p1.texCoord.y += uv.y;
                        p2.texCoord.x += uv.x;
                        p2.texCoord.y += uv.y;
                    }

                    int xx = mx*(m_brick/m_subdivMapping) + x;
                    int yy = my*(m_brick/m_subdivMapping) + y;
                    p1.texCoord2.x = (static_cast<float>(ox%5)*m_brick+xx+0.0f)/(m_brick*5);
                    p1.texCoord2.y = (static_cast<float>(oy%5)*m_brick+yy+0.0f)/(m_brick*5);
                    p2.texCoord2.x = (static_cast<float>(ox%5)*m_brick+xx+0.0f)/(m_brick*5);
                    p2.texCoord2.y = (static_cast<float>(oy%5)*m_brick+yy+1.0f)/(m_brick*5);

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
                m_engine->AddQuick(objRank, buffer, texName1, texName2, min, max, true);
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

Gfx::TerrainMaterial* Gfx::CTerrain::LevelSearchMat(int id)
{
    for (int i = 0; i < static_cast<int>( m_levelMats.size() ); i++)
    {
        if (id == m_levelMats[i].id)
            return &m_levelMats[i];
    }

    return nullptr;
}

void Gfx::CTerrain::LevelTextureName(int x, int y, std::string& name, Math::Point &uv)
{
    x /= m_brick/m_subdivMapping;
    y /= m_brick/m_subdivMapping;

    TerrainMaterial* tm = LevelSearchMat(m_levelDots[x+y*m_levelDotSize].id);
    if (tm == nullptr)
    {
        name = "xxx.png";
        uv.x = 0.0f;
        uv.y = 0.0f;
    }
    else
    {
        name = tm->texName;
        uv.x = tm->u;
        uv.y = tm->v;
    }
}

float Gfx::CTerrain::LevelGetHeight(int x, int y)
{
    int size = (m_mosaic*m_brick+1);

    if (x <  0   )  x = 0;
    if (x >= size)  x = size-1;
    if (y <  0   )  y = 0;
    if (y >= size)  y = size-1;

    return m_relief[x+y*size];
}

bool Gfx::CTerrain::LevelGetDot(int x, int y, float min, float max, float slope)
{
    float hc = LevelGetHeight(x, y);
    float h[4] =
    {
        LevelGetHeight(x+0, y+1),
        LevelGetHeight(x+1, y+0),
        LevelGetHeight(x+0, y-1),
        LevelGetHeight(x-1, y+0)
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


/** Returns the index within m_levelMats or -1 if there is not.
   m_levelMats[i].id gives the identifier. */
int Gfx::CTerrain::LevelTestMat(char *mat)
{
    for (int i = 0; i < static_cast<int>( m_levelMats.size() ); i++)
    {
        if ( m_levelMats[i].mat[0] == mat[0] &&
             m_levelMats[i].mat[1] == mat[1] &&
             m_levelMats[i].mat[2] == mat[2] &&
             m_levelMats[i].mat[3] == mat[3] )  return i;
    }

    return -1;
}

void Gfx::CTerrain::LevelSetDot(int x, int y, int id, char *mat)
{
    TerrainMaterial* tm = LevelSearchMat(id);
    if (tm == nullptr)  return;

    if ( tm->mat[0] != mat[0] ||
         tm->mat[1] != mat[1] ||
         tm->mat[2] != mat[2] ||
         tm->mat[3] != mat[3] )  // id incompatible with mat?
    {
        int ii = LevelTestMat(mat);
        if (ii == -1)  return;
        id = m_levelMats[ii].id;  // looking for a id compatible with mat
    }

    // Changes the point
    m_levelDots[x+y*m_levelDotSize].id     = id;
    m_levelDots[x+y*m_levelDotSize].mat[0] = mat[0];
    m_levelDots[x+y*m_levelDotSize].mat[1] = mat[1];
    m_levelDots[x+y*m_levelDotSize].mat[2] = mat[2];
    m_levelDots[x+y*m_levelDotSize].mat[3] = mat[3];

    // Changes the lower neighbor
    if ( (x+0) >= 0 && (x+0) < m_levelDotSize &&
         (y-1) >= 0 && (y-1) < m_levelDotSize )
    {
        int i = (x+0)+(y-1)*m_levelDotSize;
        if (m_levelDots[i].mat[0] != mat[2])
        {
            m_levelDots[i].mat[0] = mat[2];
            int ii = LevelTestMat(m_levelDots[i].mat);
            if (ii != -1)
                m_levelDots[i].id = m_levelMats[ii].id;
        }
    }

    // Modifies the left neighbor
    if ( (x-1) >= 0 && (x-1) < m_levelDotSize &&
         (y+0) >= 0 && (y+0) < m_levelDotSize )
    {
        int i = (x-1)+(y+0)*m_levelDotSize;
        if (m_levelDots[i].mat[1] != mat[3])
        {
            m_levelDots[i].mat[1] = mat[3];
            int ii = LevelTestMat(m_levelDots[i].mat);
            if (ii != -1)
                m_levelDots[i].id = m_levelMats[ii].id;
        }
    }

    // Changes the upper neighbor
    if ( (x+0) >= 0 && (x+0) < m_levelDotSize &&
         (y+1) >= 0 && (y+1) < m_levelDotSize )
    {
        int i = (x+0)+(y+1)*m_levelDotSize;
        if (m_levelDots[i].mat[2] != mat[0])
        {
            m_levelDots[i].mat[2] = mat[0];
            int ii = LevelTestMat(m_levelDots[i].mat);
            if (ii != -1)
                m_levelDots[i].id = m_levelMats[ii].id;
        }
    }

    // Changes the right neighbor
    if ( (x+1) >= 0 && (x+1) < m_levelDotSize &&
         (y+0) >= 0 && (y+0) < m_levelDotSize )
    {
        int i = (x+1)+(y+0)*m_levelDotSize;
        if ( m_levelDots[i].mat[3] != mat[1] )
        {
            m_levelDots[i].mat[3] = mat[1];
            int ii = LevelTestMat(m_levelDots[i].mat);
            if (ii != -1)
                m_levelDots[i].id = m_levelMats[ii].id;
        }
    }
}

bool Gfx::CTerrain::LevelIfDot(int x, int y, int id, char *mat)
{
    char test[4];

    // Compatible with lower neighbor?
    if ( x+0 >= 0 && x+0 < m_levelDotSize &&
         y-1 >= 0 && y-1 < m_levelDotSize )
    {
        test[0] = mat[2];
        test[1] = m_levelDots[(x+0)+(y-1)*m_levelDotSize].mat[1];
        test[2] = m_levelDots[(x+0)+(y-1)*m_levelDotSize].mat[2];
        test[3] = m_levelDots[(x+0)+(y-1)*m_levelDotSize].mat[3];

        if ( LevelTestMat(test) == -1 )  return false;
    }

    // Compatible with left neighbor?
    if ( x-1 >= 0 && x-1 < m_levelDotSize &&
         y+0 >= 0 && y+0 < m_levelDotSize )
    {
        test[0] = m_levelDots[(x-1)+(y+0)*m_levelDotSize].mat[0];
        test[1] = mat[3];
        test[2] = m_levelDots[(x-1)+(y+0)*m_levelDotSize].mat[2];
        test[3] = m_levelDots[(x-1)+(y+0)*m_levelDotSize].mat[3];

        if ( LevelTestMat(test) == -1 )  return false;
    }

    // Compatible with upper neighbor?
    if ( x+0 >= 0 && x+0 < m_levelDotSize &&
         y+1 >= 0 && y+1 < m_levelDotSize )
    {
        test[0] = m_levelDots[(x+0)+(y+1)*m_levelDotSize].mat[0];
        test[1] = m_levelDots[(x+0)+(y+1)*m_levelDotSize].mat[1];
        test[2] = mat[0];
        test[3] = m_levelDots[(x+0)+(y+1)*m_levelDotSize].mat[3];

        if ( LevelTestMat(test) == -1 )  return false;
    }

    // Compatible with right neighbor?
    if ( x+1 >= 0 && x+1 < m_levelDotSize &&
         y+0 >= 0 && y+0 < m_levelDotSize )
    {
        test[0] = m_levelDots[(x+1)+(y+0)*m_levelDotSize].mat[0];
        test[1] = m_levelDots[(x+1)+(y+0)*m_levelDotSize].mat[1];
        test[2] = m_levelDots[(x+1)+(y+0)*m_levelDotSize].mat[2];
        test[3] = mat[1];

        if ( LevelTestMat(test) == -1 )  return false;
    }

    LevelSetDot(x, y, id, mat);  // puts the point
    return true;
}

bool Gfx::CTerrain::LevelPutDot(int x, int y, int id)
{
    char mat[4];

    x /= m_brick/m_subdivMapping;
    y /= m_brick/m_subdivMapping;

    if ( x < 0 || x >= m_levelDotSize ||
         y < 0 || y >= m_levelDotSize )  return false;

    TerrainMaterial* tm = LevelSearchMat(id);
    if (tm == nullptr)  return false;

    // Tries without changing neighbors.
    if ( LevelIfDot(x, y, id, tm->mat) )  return true;

    // Tries changing a single neighbor (4x).
    for (int up = 0; up < m_levelMatMax; up++)
    {
        mat[0] = up;
        mat[1] = tm->mat[1];
        mat[2] = tm->mat[2];
        mat[3] = tm->mat[3];

        if (LevelIfDot(x, y, id, mat))  return true;
    }

    for (int right = 0; right < m_levelMatMax; right++)
    {
        mat[0] = tm->mat[0];
        mat[1] = right;
        mat[2] = tm->mat[2];
        mat[3] = tm->mat[3];

        if (LevelIfDot(x, y, id, mat)) return true;
    }

    for (int down = 0; down < m_levelMatMax; down++)
    {
        mat[0] = tm->mat[0];
        mat[1] = tm->mat[1];
        mat[2] = down;
        mat[3] = tm->mat[3];

        if (LevelIfDot(x, y, id, mat)) return true;
    }

    for (int left = 0; left < m_levelMatMax; left++)
    {
        mat[0] = tm->mat[0];
        mat[1] = tm->mat[1];
        mat[2] = tm->mat[2];
        mat[3] = left;

        if (LevelIfDot(x, y, id, mat)) return true;
    }

    // Tries changing two neighbors (6x).
    for (int up = 0; up < m_levelMatMax; up++)
    {
        for (int down = 0; down < m_levelMatMax; down++)
        {
            mat[0] = up;
            mat[1] = tm->mat[1];
            mat[2] = down;
            mat[3] = tm->mat[3];

            if (LevelIfDot(x, y, id, mat)) return true;
        }
    }

    for (int right = 0; right < m_levelMatMax; right++)
    {
        for (int left = 0; left < m_levelMatMax; left++)
        {
            mat[0] = tm->mat[0];
            mat[1] = right;
            mat[2] = tm->mat[2];
            mat[3] = left;

            if (LevelIfDot(x, y, id, mat)) return true;
        }
    }

    for (int up = 0; up < m_levelMatMax; up++)
    {
        for (int right = 0; right < m_levelMatMax; right++)
        {
            mat[0] = up;
            mat[1] = right;
            mat[2] = tm->mat[2];
            mat[3] = tm->mat[3];

            if (LevelIfDot(x, y, id, mat)) return true;
        }
    }

    for (int right = 0; right < m_levelMatMax; right++)
    {
        for (int down = 0; down < m_levelMatMax; down++)
        {
            mat[0] = tm->mat[0];
            mat[1] = right;
            mat[2] = down;
            mat[3] = tm->mat[3];

            if (LevelIfDot(x, y, id, mat)) return true;
        }
    }

    for (int down = 0; down < m_levelMatMax; down++)
    {
        for (int left = 0; left < m_levelMatMax; left++)
        {
            mat[0] = tm->mat[0];
            mat[1] = tm->mat[1];
            mat[2] = down;
            mat[3] = left;

            if (LevelIfDot(x, y, id, mat)) return true;
        }
    }

    for (int up = 0; up < m_levelMatMax; up++)
    {
        for (int left = 0; left < m_levelMatMax; left++)
        {
            mat[0] = up;
            mat[1] = tm->mat[1];
            mat[2] = tm->mat[2];
            mat[3] = left;

            if (LevelIfDot(x, y, id, mat)) return true;
        }
    }

    // Tries changing all the neighbors.
    for (int up = 0; up < m_levelMatMax; up++)
    {
        for (int right = 0; right < m_levelMatMax; right++)
        {
            for (int down = 0; down < m_levelMatMax; down++)
            {
                for (int left = 0; left < m_levelMatMax; left++)
                {
                    mat[0] = up;
                    mat[1] = right;
                    mat[2] = down;
                    mat[3] = left;

                    if (LevelIfDot(x, y, id, mat)) return true;
                }
            }
        }
    }

    GetLogger()->Error("LevelPutDot error\n");
    return false;
}

bool Gfx::CTerrain::LevelInit(int id)
{
    TerrainMaterial* tm = LevelSearchMat(id);
    if (tm == nullptr) return false;

    for (int i = 0; i < m_levelDotSize*m_levelDotSize; i++)
    {
        m_levelDots[i].id = id;

        for (int j = 0; j < 4; j++)
            m_levelDots[i].mat[j] = tm->mat[j];
    }

    return true;
}

bool Gfx::CTerrain::LevelGenerate(int *id, float min, float max,
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
    while ( id[i] != 0 )
    {
        tm = LevelSearchMat(id[i++]);
        if (tm == nullptr)  return false;
    }
    int numID = i;

    int group = m_brick / m_subdivMapping;

    if (radius > 0.0f && radius < 5.0f)  // just a square?
    {
        float dim = (m_mosaic*m_brick*m_size)/2.0f;

        int xx = static_cast<int>((center.x+dim)/m_size);
        int yy = static_cast<int>((center.z+dim)/m_size);

        int x = xx/group;
        int y = yy/group;

        tm = LevelSearchMat(id[0]);
        if (tm != nullptr)
            LevelSetDot(x, y, id[0], tm->mat);  // puts the point
    }
    else
    {
        for (int y = 0; y < m_levelDotSize; y++)
        {
            for (int x = 0; x < m_levelDotSize; x++)
            {
                if (radius != 0.0f)
                {
                    Math::Vector pos;
                    pos.x = (static_cast<float>(x)-m_levelDotSize/2.0f)*group*m_size;
                    pos.z = (static_cast<float>(y)-m_levelDotSize/2.0f)*group*m_size;
                    if (Math::DistanceProjected(pos, center) > radius) continue;
                }

                if (freq < 100.0f)
                {
                    int rnd = random[(x%10)+(y%10)*10];
                    if ( static_cast<float>(rnd) > freq )  continue;
                }

                int xx = x*group + group/2;
                int yy = y*group + group/2;

                if (LevelGetDot(xx, yy, min, max, slope))
                {
                    int rnd = random[(x%10)+(y%10)*10];
                    int ii = rnd % numID;
                    LevelPutDot(xx, yy, id[ii]);
                }
            }
        }
    }

    return true;
}

void Gfx::CTerrain::LevelOpenTable()
{
    if (! m_levelText)  return;
    if (! m_levelDots.empty())  return;  // already allocated

    m_levelDotSize = (m_mosaic*m_brick)/(m_brick/m_subdivMapping)+1;
    std::vector<Gfx::DotLevel>(m_levelDotSize*m_levelDotSize).swap(m_levelDots);

    for (int i = 0; i < m_levelDotSize * m_levelDotSize; i++)
    {
        for (int j = 0; j < 4; j++)
            m_levelDots[i].mat[j] = 0;
    }
}

void Gfx::CTerrain::LevelCloseTable()
{
    m_levelDots.clear();
}

bool Gfx::CTerrain::CreateSquare(bool multiRes, int x, int y)
{
    Gfx::Material mat;
    mat.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);
    mat.ambient = Gfx::Color(0.0f, 0.0f, 0.0f);

    int objRank = m_engine->CreateObject();
    m_engine->SetObjectType(objRank, Gfx::ENG_OBJTYPE_TERRAIN);  // it is a terrain

    m_objRank[x+y*m_mosaic] = objRank;

    if (multiRes)
    {
        float min = 0.0f;
        float max = m_vision;
        max *= m_engine->GetClippingDistance();
        for (int step = 0; step < m_depth; step++)
        {
            CreateMosaic(x, y, 1 << step, objRank, mat, min, max);
            min = max;
            max *= 2;
            if (step == m_depth-1) max = Math::HUGE_NUM;
        }
    }
    else
    {
        CreateMosaic(x, y, 1, objRank, mat, 0.0f, Math::HUGE_NUM);
    }

    return true;
}

bool Gfx::CTerrain::CreateObjects(bool multiRes)
{
    AdjustRelief();

    for (int y = 0; y < m_mosaic; y++)
    {
        for (int x = 0; x < m_mosaic; x++)
            CreateSquare(multiRes, x, y);
    }

    return true;
}

/** ATTENTION: ok only with m_depth = 2! */
bool Gfx::CTerrain::Terraform(const Math::Vector &p1, const Math::Vector &p2, float height)
{
    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    Math::IntPoint tp1, tp2;
    tp1.x = static_cast<int>((p1.x+dim+m_size/2.0f)/m_size);
    tp1.y = static_cast<int>((p1.z+dim+m_size/2.0f)/m_size);
    tp2.x = static_cast<int>((p2.x+dim+m_size/2.0f)/m_size);
    tp2.y = static_cast<int>((p2.z+dim+m_size/2.0f)/m_size);

    if (tp1.x > tp2.x)
    {
        int x     = tp1.x;
        tp1.x = tp2.x;
        tp2.x = x;
    }

    if (tp1.y > tp2.y)
    {
        int y     = tp1.y;
        tp1.y = tp2.y;
        tp2.y = y;
    }

    int size = (m_mosaic*m_brick)+1;

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

            if (x % m_brick == 0 && y % m_depth != 0)
            {
                m_relief[(x+0)+(y-1)*size] = avg+height;
                m_relief[(x+0)+(y+1)*size] = avg+height;
            }

            if (y % m_brick == 0 && x % m_depth != 0)
            {
                m_relief[(x-1)+(y+0)*size] = avg+height;
                m_relief[(x+1)+(y+0)*size] = avg+height;
            }
        }
    }
    AdjustRelief();

    Math::IntPoint pp1, pp2;
    pp1.x = (tp1.x-2)/m_brick;
    pp1.y = (tp1.y-2)/m_brick;
    pp2.x = (tp2.x+1)/m_brick;
    pp2.y = (tp2.y+1)/m_brick;

    if (pp1.x <  0       ) pp1.x = 0;
    if (pp1.x >= m_mosaic) pp1.x = m_mosaic-1;
    if (pp1.y <  0       ) pp1.y = 0;
    if (pp1.y >= m_mosaic) pp1.y = m_mosaic-1;

    for (int y = pp1.y; y <= pp2.y; y++)
    {
        for (int x = pp1.x; x <= pp2.x; x++)
        {
            m_engine->DeleteObject(m_objRank[x+y*m_mosaic]);
            CreateSquare(m_multiText, x, y);  // recreates the square
        }
    }
    m_engine->Update();

    return true;
}

void Gfx::CTerrain::SetWind(Math::Vector speed)
{
    m_wind = speed;
}

Math::Vector Gfx::CTerrain::GetWind()
{
    return m_wind;
}

float Gfx::CTerrain::GetFineSlope(const Math::Vector &pos)
{
    Math::Vector n;
    if (! GetNormal(n, pos)) return 0.0f;
    return fabs(Math::RotateAngle(Math::Point(n.x, n.z).Length(), n.y) - Math::PI/2.0f);
}

float Gfx::CTerrain::GetCoarseSlope(const Math::Vector &pos)
{
    if (m_relief.empty()) return 0.0f;

    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    int x = static_cast<int>((pos.x+dim)/m_size);
    int y = static_cast<int>((pos.z+dim)/m_size);

    if ( x < 0 || x >= m_mosaic*m_brick ||
         y < 0 || y >= m_mosaic*m_brick ) return 0.0f;

    float level[4] =
    {
        m_relief[(x+0)+(y+0)*(m_mosaic*m_brick+1)],
        m_relief[(x+1)+(y+0)*(m_mosaic*m_brick+1)],
        m_relief[(x+0)+(y+1)*(m_mosaic*m_brick+1)],
        m_relief[(x+1)+(y+1)*(m_mosaic*m_brick+1)],
    };

    float min = Math::Min(level[0], level[1], level[2], level[3]);
    float max = Math::Max(level[0], level[1], level[2], level[3]);

    return atanf((max-min)/m_size);
}

bool Gfx::CTerrain::GetNormal(Math::Vector &n, const Math::Vector &p)
{
    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    int x = static_cast<int>((p.x+dim)/m_size);
    int y = static_cast<int>((p.z+dim)/m_size);

    if ( x < 0 || x > m_mosaic*m_brick ||
         y < 0 || y > m_mosaic*m_brick )  return false;

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

float Gfx::CTerrain::GetFloorLevel(const Math::Vector &p, bool brut, bool water)
{
    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    int x = static_cast<int>((p.x+dim)/m_size);
    int y = static_cast<int>((p.z+dim)/m_size);

    if ( x < 0 || x > m_mosaic*m_brick ||
         y < 0 || y > m_mosaic*m_brick )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    Math::Vector ps = p;
    if ( fabs(p.z-p2.z) < fabs(p.x-p2.x) )
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


/** This height is positive when you are above the ground */
float Gfx::CTerrain::GetFloorHeight(const Math::Vector &p, bool brut, bool water)
{
    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    int x = static_cast<int>((p.x+dim)/m_size);
    int y = static_cast<int>((p.z+dim)/m_size);

    if ( x < 0 || x > m_mosaic*m_brick ||
         y < 0 || y > m_mosaic*m_brick )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    Math::Vector ps = p;
    if ( fabs(p.z-p2.z) < fabs(p.x-p2.x) )
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
        if ( ps.y < level )  ps.y = level;  // not under water
    }

    return p.y-ps.y;
}

bool Gfx::CTerrain::MoveOnFloor(Math::Vector &p, bool brut, bool water)
{
    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    int x = static_cast<int>((p.x + dim) / m_size);
    int y = static_cast<int>((p.z + dim) / m_size);

    if ( x < 0 || x > m_mosaic*m_brick ||
         y < 0 || y > m_mosaic*m_brick )  return false;

    Math::Vector p1 = GetVector(x+0, y+0);
    Math::Vector p2 = GetVector(x+1, y+0);
    Math::Vector p3 = GetVector(x+0, y+1);
    Math::Vector p4 = GetVector(x+1, y+1);

    if (fabs(p.z - p2.z) < fabs(p.x - p2.x))
    {
        if (! Math::IntersectY(p1, p2, p3, p)) return false;
    }
    else
    {
        if (! Math::IntersectY(p2, p4, p3, p)) return false;
    }

    if (! brut) AdjustBuildingLevel(p);

    if (water)  // not going underwater?
    {
        float level = m_water->GetLevel();
        if (p.y < level) p.y = level;  // not under water
    }

    return true;
}


/** Returns false if the initial coordinate was too far */
bool Gfx::CTerrain::ValidPosition(Math::Vector &p, float marging)
{
    bool ok = true;

    float limit = m_mosaic*m_brick*m_size/2.0f - marging;

    if (p.x < -limit)
    {
        p.x = -limit;
        ok = false;
    }

    if (p.z < -limit)
    {
        p.z = -limit;
        ok = false;
    }

    if (p.x > limit)
    {
        p.x = limit;
        ok = false;
    }

    if (p.z > limit)
    {
        p.z = limit;
        ok = false;
    }

    return ok;
}

void Gfx::CTerrain::FlushBuildingLevel()
{
    m_buildingLevels.clear();
}

bool Gfx::CTerrain::AddBuildingLevel(Math::Vector center, float min, float max,
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
        m_buildingLevels.push_back(Gfx::BuildingLevel());

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

bool Gfx::CTerrain::UpdateBuildingLevel(Math::Vector center)
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

bool Gfx::CTerrain::DeleteBuildingLevel(Math::Vector center)
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

float Gfx::CTerrain::GetBuildingFactor(const Math::Vector &p)
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

void Gfx::CTerrain::AdjustBuildingLevel(Math::Vector &p)
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


// returns the hardness of the ground in a given place.
// The hardness determines the noise (SOUND_STEP and SOUND_BOUM).

float Gfx::CTerrain::GetHardness(const Math::Vector &p)
{
    float factor = GetBuildingFactor(p);
    if (factor != 1.0f) return 1.0f;  // on building

    if (m_levelDots.empty()) return m_defHardness;

    float dim = (m_mosaic*m_brick*m_size)/2.0f;

    int x, y;

    x = static_cast<int>((p.x+dim)/m_size);
    y = static_cast<int>((p.z+dim)/m_size);

    if ( x < 0 || x > m_mosaic*m_brick ||
         y < 0 || y > m_mosaic*m_brick )  return m_defHardness;

    x /= m_brick/m_subdivMapping;
    y /= m_brick/m_subdivMapping;

    if ( x < 0 || x >= m_levelDotSize ||
         y < 0 || y >= m_levelDotSize )  return m_defHardness;

    int id = m_levelDots[x+y*m_levelDotSize].id;
    TerrainMaterial* tm = LevelSearchMat(id);
    if (tm == nullptr) return m_defHardness;

    return tm->hardness;
}

void Gfx::CTerrain::GroundFlat(Math::Vector pos)
{
    static char table[41*41];


    float rapport = 3200.0f/1024.0f;

    for (int y = 0; y <= 40; y++)
    {
        for (int x = 0; x <= 40; x++)
        {
            int i = x + y*41;
            table[i] = 0;

            Math::Vector p;
            p.x = (x-20)*rapport;
            p.z = (y-20)*rapport;
            p.y = 0.0f;

            if (Math::Point(p.x, p.y).Length() > 20.0f*rapport)
                continue;

            float angle = GetFineSlope(pos+p);

            if (angle < FLATLIMIT)
                table[i] = 1;
            else
                table[i] = 2;
        }
    }

    m_engine->CreateGroundMark(pos, 40.0f, 0.001f, 15.0f, 0.001f, 41, 41, table);
}

float Gfx::CTerrain::GetFlatZoneRadius(Math::Vector center, float max)
{
    float angle = GetFineSlope(center);
    if (angle >= Gfx::FLATLIMIT)
        return 0.0f;

    float ref = GetFloorLevel(center, true);

    float radius = 1.0f;
    while (radius <= max)
    {
        angle = 0.0f;
        int nb = static_cast<int>(2.0f*Math::PI*radius);
        if (nb < 8) nb = 8;

        for (int i = 0; i < nb; i++)
        {
            Math::Point c(center.x, center.z);
            Math::Point p (center.x+radius, center.z);
            p = Math::RotatePoint(c, angle, p);
            Math::Vector pos;
            pos.x = p.x;
            pos.z = p.y;
            float h = GetFloorLevel(pos, true);
            if ( fabs(h-ref) > 1.0f )  return radius;

            angle += Math::PI*2.0f/8.0f;
        }
        radius += 1.0f;
    }
    return max;
}

void Gfx::CTerrain::SetFlyingMaxHeight(float height)
{
    m_flyingMaxHeight = height;
}

float Gfx::CTerrain::GetFlyingMaxHeight()
{
    return m_flyingMaxHeight;
}

void Gfx::CTerrain::FlushFlyingLimit()
{
    m_flyingMaxHeight = 280.0f;
    m_flyingLimits.clear();
}

void Gfx::CTerrain::AddFlyingLimit(Math::Vector center,
                                   float extRadius, float intRadius,
                                   float maxHeight)
{
    Gfx::FlyingLimit fl;
    fl.center    = center;
    fl.extRadius = extRadius;
    fl.intRadius = intRadius;
    fl.maxHeight = maxHeight;
    m_flyingLimits.push_back(fl);
}

float Gfx::CTerrain::GetFlyingLimit(Math::Vector pos, bool noLimit)
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
