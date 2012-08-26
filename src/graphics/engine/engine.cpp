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

// engine.cpp

#include "graphics/engine/engine.h"

#include "app/app.h"
#include "common/iman.h"
#include "common/image.h"
#include "common/key.h"
#include "common/logger.h"
#include "graphics/core/device.h"
#include "graphics/engine/camera.h"
#include "graphics/engine/cloud.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/planet.h"
#include "graphics/engine/pyro.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/text.h"
#include "graphics/engine/water.h"
#include "math/geometry.h"
#include "sound/sound.h"


// Initial size of various vectors
const int OBJECT_PREALLOCATE_COUNT       = 1200;
const int SHADOW_PREALLOCATE_COUNT       = 500;
const int GROUNDSPOT_PREALLOCATE_COUNT   = 100;

const int LEVEL1_PREALLOCATE_COUNT        = 50;
const int LEVEL2_PREALLOCATE_COUNT        = 100;
const int LEVEL3_PREALLOCATE_COUNT        = 5;
const int LEVEL4_PREALLOCATE_COUNT        = 100;
const int LEVEL4_VERTEX_PREALLOCATE_COUNT = 200;


Gfx::EngineObjLevel1::EngineObjLevel1(bool used, const std::string& tex1Name, const std::string& tex2Name)
{
    this->used = used;
    this->tex1Name = tex1Name;
    this->tex2Name = tex2Name;

    next.reserve(LEVEL2_PREALLOCATE_COUNT);
}

Gfx::EngineObjLevel2::EngineObjLevel2(bool used, int objRank)
{
    this->used = used;
    this->objRank = objRank;

    next.reserve(LEVEL3_PREALLOCATE_COUNT);
}

Gfx::EngineObjLevel3::EngineObjLevel3(bool used, float min, float max)
{
    this->used = used;
    this->min = min;
    this->max = max;

    next.reserve(LEVEL4_PREALLOCATE_COUNT);
}

Gfx::EngineObjLevel4::EngineObjLevel4(bool used, Gfx::EngineTriangleType type, const Gfx::Material& material, int state)
{
    this->used = used;
    this->type = type;
    this->material = material;
    this->state = state;

    vertices.reserve(LEVEL4_VERTEX_PREALLOCATE_COUNT);
}



// TODO: temporary stub for CInterface
class CInterface
{
public:
    void Draw() {}
};

Gfx::CEngine::CEngine(CInstanceManager *iMan, CApplication *app)
{
    m_iMan   = iMan;
    m_app    = app;
    m_device = nullptr;

    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_ENGINE, this);
    m_app = app;

    m_lightMan   = nullptr;
    m_text       = nullptr;
    m_particle   = nullptr;
    m_water      = nullptr;
    m_cloud      = nullptr;
    m_lightning  = nullptr;
    m_planet     = nullptr;
    m_sound      = nullptr;
    m_terrain    = nullptr;

    m_focus = 0.75f;

    m_rankView = 0;

    m_ambientColor[0] = Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f);
    m_ambientColor[1] = Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f);
    m_fogColor[0]     = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
    m_fogColor[1]     = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
    m_deepView[0]     = 1000.0f;
    m_deepView[1]     = 1000.0f;
    m_fogStart[0]     = 0.75f;
    m_fogStart[1]     = 0.75f;
    m_waterAddColor   = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);

    m_pause             = false;
    m_render            = true;
    m_movieLock         = false;
    m_shadowVisible     = true;
    m_groundSpotVisible = true;
    m_dirty             = true;
    m_fog               = true;
    m_speed             = 1.0f;
    m_secondTexNum      = 0;
    m_eyeDirH           = 0.0f;
    m_eyeDirV           = 0.0f;
    m_backgroundName    = "";  // no background image
    m_backgroundColorUp   = 0;
    m_backgroundColorDown = 0;
    m_backgroundCloudUp   = 0;
    m_backgroundCloudDown = 0;
    m_backgroundFull = false;
    m_backgroundQuarter = false;
    m_overFront = true;
    m_overColor = 0;
    m_overMode  = ENG_RSTATE_TCOLOR_BLACK;
    m_highlightRank[0] = -1;  // empty list
    m_highlightTime = 0.0f;
    m_eyePt    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_lookatPt = Math::Vector(0.0f, 0.0f, 1.0f);
    m_drawWorld = true;
    m_drawFront = false;
    m_limitLOD[0] = 100.0f;
    m_limitLOD[1] = 200.0f;
    m_particleDensity = 1.0f;
    m_clippingDistance = 1.0f;
    m_lastClippingDistance = m_clippingDistance;
    m_objectDetail = 1.0f;
    m_lastObjectDetail = m_objectDetail;
    m_terrainVision = 1000.0f;
    m_gadgetQuantity = 1.0f;
    m_textureQuality = 1;
    m_totoMode = true;
    m_lensMode = true;
    m_waterMode = true;
    m_skyMode = true;
    m_backForce = false; // TODO: change to true?
    m_planetMode = true;
    m_lightMode = true;
    m_editIndentMode = true;
    m_editIndentValue = 4;
    m_tracePrecision = 1.0f;

    m_alphaMode = 1;

    m_forceStateColor = true;
    m_stateColor = false;

    m_updateGeometry = false;

    m_mice[Gfx::ENG_MOUSE_NORM]    = Gfx::EngineMouse( 0,  1, 32, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Math::Point( 1.0f,  1.0f));
    m_mice[Gfx::ENG_MOUSE_WAIT]    = Gfx::EngineMouse( 2,  3, 33, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Math::Point( 8.0f, 12.0f));
    m_mice[Gfx::ENG_MOUSE_HAND]    = Gfx::EngineMouse( 4,  5, 34, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Math::Point( 7.0f,  2.0f));
    m_mice[Gfx::ENG_MOUSE_NO]      = Gfx::EngineMouse( 6,  7, 35, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Math::Point(10.0f, 10.0f));
    m_mice[Gfx::ENG_MOUSE_EDIT]    = Gfx::EngineMouse( 8,  9, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 6.0f, 10.0f));
    m_mice[Gfx::ENG_MOUSE_CROSS]   = Gfx::EngineMouse(10, 11, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point(10.0f, 10.0f));
    m_mice[Gfx::ENG_MOUSE_MOVEV]   = Gfx::EngineMouse(12, 13, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 5.0f, 11.0f));
    m_mice[Gfx::ENG_MOUSE_MOVEH]   = Gfx::EngineMouse(14, 15, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point(11.0f,  5.0f));
    m_mice[Gfx::ENG_MOUSE_MOVED]   = Gfx::EngineMouse(16, 17, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 9.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_MOVEI]   = Gfx::EngineMouse(18, 19, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 9.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_MOVE]    = Gfx::EngineMouse(20, 21, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point(11.0f, 11.0f));
    m_mice[Gfx::ENG_MOUSE_TARGET]  = Gfx::EngineMouse(22, 23, -1, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point(15.0f, 15.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLL] = Gfx::EngineMouse(24, 25, 43, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 2.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLR] = Gfx::EngineMouse(26, 27, 44, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point(17.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLU] = Gfx::EngineMouse(28, 29, 45, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 9.0f,  2.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLD] = Gfx::EngineMouse(30, 31, 46, Gfx::ENG_RSTATE_TTEXTURE_BLACK, Gfx::ENG_RSTATE_TTEXTURE_WHITE, Math::Point( 9.0f, 17.0f));

    m_mouseSize    = Math::Point(0.04f, 0.04f * (800.0f / 600.0f));
    m_mousePos     = Math::Point(0.5f, 0.5f);
    m_mouseType    = Gfx::ENG_MOUSE_NORM;
    m_mouseVisible = false;

    m_texPath = "textures/";
    m_defaultTexParams.format = Gfx::TEX_IMG_RGB;
    m_defaultTexParams.mipmap = true;
    m_defaultTexParams.minFilter = Gfx::TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR;
    m_defaultTexParams.magFilter = Gfx::TEX_MAG_FILTER_LINEAR;

    m_objectTree.reserve(LEVEL1_PREALLOCATE_COUNT);
    m_objects.reserve(OBJECT_PREALLOCATE_COUNT);
    m_shadows.reserve(SHADOW_PREALLOCATE_COUNT);
    m_groundSpots.reserve(GROUNDSPOT_PREALLOCATE_COUNT);
}

Gfx::CEngine::~CEngine()
{
    m_iMan    = nullptr;
    m_app     = nullptr;
    m_device  = nullptr;
    m_sound   = nullptr;
    m_terrain = nullptr;
}

void Gfx::CEngine::SetDevice(Gfx::CDevice *device)
{
    m_device = device;
}

Gfx::CDevice* Gfx::CEngine::GetDevice()
{
    return m_device;
}

void Gfx::CEngine::SetTerrain(Gfx::CTerrain* terrain)
{
    m_terrain = terrain;
}

Gfx::CText* Gfx::CEngine::GetText()
{
    return m_text;
}

bool Gfx::CEngine::Create()
{
    m_size = m_lastSize = m_app->GetVideoConfig().size;

    m_lightMan   = new Gfx::CLightManager(m_iMan, this);
    m_text       = new Gfx::CText(m_iMan, this);
    m_particle   = new Gfx::CParticle(m_iMan, this);
    m_water      = new Gfx::CWater(m_iMan, this);
    m_cloud      = new Gfx::CCloud(m_iMan, this);
    m_lightning  = new Gfx::CLightning(m_iMan, this);
    m_planet     = new Gfx::CPlanet(m_iMan, this);

    m_lightMan->SetDevice(m_device);

    m_text->SetDevice(m_device);
    if (! m_text->Create())
    {
        std::string error = m_text->GetError();
        GetLogger()->Error("Error creating CText: %s\n", error.c_str());
        return false;
    }

    m_device->SetClearColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));
    m_device->SetShadeModel(Gfx::SHADE_SMOOTH);
    m_device->SetFillMode(Gfx::FILL_FILL);

    SetFocus(m_focus);

    m_matWorldInterface.LoadIdentity();
    m_matViewInterface.LoadIdentity();

    Math::LoadOrthoProjectionMatrix(m_matProjInterface, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    Gfx::TextureCreateParams params;
    params.format = Gfx::TEX_IMG_RGB;
    params.minFilter = Gfx::TEX_MIN_FILTER_NEAREST;
    params.magFilter = Gfx::TEX_MAG_FILTER_NEAREST;
    params.mipmap = false;
    m_miceTexture = LoadTexture("mouse.png", params);

    return true;
}

void Gfx::CEngine::Destroy()
{
    m_text->Destroy();

    delete m_lightMan;
    m_lightMan = nullptr;

    delete m_text;
    m_text = nullptr;

    delete m_particle;
    m_particle = nullptr;

    delete m_water;
    m_water = nullptr;

    delete m_cloud;
    m_cloud = nullptr;

    delete m_lightning;
    m_lightning = nullptr;

    delete m_planet;
    m_planet = nullptr;
}

void Gfx::CEngine::ResetAfterDeviceChanged()
{
    // TODO reload textures, reset device state, etc.
}

bool Gfx::CEngine::ProcessEvent(const Event &event)
{
    if (event.type == EVENT_KEY_DOWN)
    {
        // !! Debug, to be removed later !!

        if (event.key.key == KEY(F1))
        {
            m_mouseVisible = !m_mouseVisible;
            m_app->SetSystemMouseVisible(! m_app->GetSystemMouseVisibile());
        }
        else if (event.key.key == KEY(F2))
        {
            int index = static_cast<int>(m_mouseType);
            m_mouseType = static_cast<Gfx::EngineMouseType>( (index + 1) % Gfx::ENG_MOUSE_COUNT );
        }
        else if (event.key.key == KEY(F3))
        {
            bool bq = !m_backgroundQuarter;
            SetBackground(bq ? "geneda.png" : "", Gfx::Color(), Gfx::Color(), Gfx::Color(), Gfx::Color(), true, bq);
        }
        else if (event.key.key == KEY(F4))
        {
            m_skyMode = !m_skyMode;
            if (! m_skyMode)
            {
                m_backgroundColorDown = Gfx::Color(0.2f, 0.2f, 0.2f);
                m_backgroundColorUp = Gfx::Color(0.8f, 0.8f, 0.8f);
            }
            else
            {
                m_backgroundColorDown = m_backgroundColorUp = Gfx::Color(0.0f, 0.0f, 0.0f);
            }
        }
    }
    else if (event.type == EVENT_FRAME)
    {
        m_highlightTime += event.rTime;
    }

    // By default, pass on all events
    return true;
}

void Gfx::CEngine::FrameMove(float rTime)
{
    m_lightMan->UpdateProgression(rTime);
    m_particle->FrameParticle(rTime);
    ComputeDistance();
    UpdateGeometry();

    if (m_groundMark.draw)
    {
        if (m_groundMark.phase == Gfx::ENG_GR_MARK_PHASE_INC)  // growing?
        {
            m_groundMark.intensity += rTime*(1.0f/m_groundMark.delay[0]);
            if (m_groundMark.intensity >= 1.0f)
            {
                m_groundMark.intensity = 1.0f;
                m_groundMark.fix = 0.0f;
                m_groundMark.phase = Gfx::ENG_GR_MARK_PHASE_FIX;
            }
        }
        else if (m_groundMark.phase == Gfx::ENG_GR_MARK_PHASE_FIX)  // fixed?
        {
            m_groundMark.fix += rTime*(1.0f/m_groundMark.delay[1]);
            if (m_groundMark.fix >= 1.0f)
                m_groundMark.phase = Gfx::ENG_GR_MARK_PHASE_DEC;
        }
        else if (m_groundMark.phase == Gfx::ENG_GR_MARK_PHASE_DEC)  // decay?
        {
            m_groundMark.intensity -= rTime*(1.0f/m_groundMark.delay[2]);
            if (m_groundMark.intensity < 0.0f)
            {
                m_groundMark.intensity = 0.0f;
                m_groundMark.phase     = Gfx::ENG_GR_MARK_PHASE_NULL;
                m_groundMark.draw     = false;
            }
        }
    }

    if (m_sound == nullptr)
        m_sound = static_cast<CSoundInterface*>( m_iMan->SearchInstance(CLASS_SOUND) );

    m_sound->FrameMove(rTime);
}

void Gfx::CEngine::StepSimulation(float rTime)
{
    m_app->StepSimulation(rTime);
}

bool Gfx::CEngine::WriteScreenShot(const std::string& fileName, int width, int height)
{
    // TODO write screenshot: not very important for now
    GetLogger()->Trace("CEngine::WriteSceenShot(): stub!\n");
    return true;
}

bool Gfx::CEngine::ReadSettings()
{
    // TODO: when INI reading is completed
    return true;
}

bool Gfx::CEngine::WriteSettings()
{
    // TODO: when INI writing is completed
    return true;
}

void Gfx::CEngine::SetPause(bool pause)
{
    m_pause = pause;
}

bool Gfx::CEngine::GetPause()
{
    return m_pause;
}

void Gfx::CEngine::SetMovieLock(bool lock)
{
    m_movieLock = lock;
}

bool Gfx::CEngine::GetMovieLock()
{
    return m_movieLock;
}

void Gfx::CEngine::SetShowStats(bool show)
{
    m_showStats = show;
}

bool Gfx::CEngine::GetShowStats()
{
    return m_showStats;
}

void Gfx::CEngine::SetRenderEnable(bool enable)
{
    m_render = enable;
}

Math::IntPoint Gfx::CEngine::GetWindowSize()
{
    return m_size;
}

Math::IntPoint Gfx::CEngine::GetLastWindowSize()
{
    return m_lastSize;
}

Math::Point Gfx::CEngine::WindowToInterfaceCoords(Math::IntPoint pos)
{
    return Math::Point(        static_cast<float>(pos.x) / static_cast<float>(m_size.x),
                        1.0f - static_cast<float>(pos.y) / static_cast<float>(m_size.y)  );
}

Math::IntPoint Gfx::CEngine::InterfaceToWindowCoords(Math::Point pos)
{
    return Math::IntPoint(static_cast<int>(pos.x * m_size.x),
                          static_cast<int>((1.0f - pos.y) * m_size.y));
}

Math::Point Gfx::CEngine::WindowToInterfaceSize(Math::IntPoint size)
{
    return Math::Point(static_cast<float>(size.x) / static_cast<float>(m_size.x),
                       static_cast<float>(size.y) / static_cast<float>(m_size.y));
}

Math::IntPoint Gfx::CEngine::InterfaceToWindowSize(Math::Point size)
{
    return Math::IntPoint(static_cast<int>(size.x * m_size.x),
                         static_cast<int>(size.y * m_size.y));
}

std::string Gfx::CEngine::GetTextureDir()
{
    return m_texPath;
}

void Gfx::CEngine::AddStatisticTriangle(int count)
{
    m_statisticTriangle += count;
}

int Gfx::CEngine::GetStatisticTriangle()
{
    return m_statisticTriangle;
}



/*******************************************************
                   Object management
 *******************************************************/



int Gfx::CEngine::CreateObject()
{
    int i = 0;
    for ( ; i < static_cast<int>( m_objects.size() ); i++)
    {
        if (! m_objects[i].used)
        {
            m_objects[i].LoadDefault();
            break;
        }
    }

    if (i == static_cast<int>( m_objects.size() ))
        m_objects.push_back(Gfx::EngineObject());


    m_objects[i].used = true;

    Math::Matrix mat;
    mat.LoadIdentity();
    SetObjectTransform(i, mat);

    m_objects[i].drawWorld = true;
    m_objects[i].distance = 0.0f;
    m_objects[i].bboxMin = Math::Vector(0.0f, 0.0f, 0.0f);
    m_objects[i].bboxMax = Math::Vector(0.0f, 0.0f, 0.0f);
    m_objects[i].shadowRank = -1;

    return i;
}

void Gfx::CEngine::FlushObject()
{
    m_objectTree.clear();
    m_objects.clear();

    m_shadows.clear();

    FlushGroundSpot();
}

bool Gfx::CEngine::DeleteObject(int objRank)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    // Delete object's triangles
    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            if (p2.objRank == objRank)
            {
                p2.used = false;
                p2.next.clear();
            }
        }
    }

    // Mark object as deleted
    m_objects[objRank].used = false;

    // Delete associated shadows
    DeleteShadow(objRank);

    return true;
}

bool Gfx::CEngine::SetObjectType(int objRank, Gfx::EngineObjectType type)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    m_objects[objRank].type = type;
    return true;
}

Gfx::EngineObjectType Gfx::CEngine::GetObjectType(int objRank)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return Gfx::ENG_OBJTYPE_NULL;

    return m_objects[objRank].type;
}


bool Gfx::CEngine::SetObjectTransform(int objRank, const Math::Matrix& transform)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    m_objects[objRank].transform = transform;
    return true;
}

bool Gfx::CEngine::GetObjectTransform(int objRank, Math::Matrix& transform)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    transform = m_objects[objRank].transform;
    return true;
}

bool Gfx::CEngine::SetObjectDrawWorld(int objRank, bool draw)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    m_objects[objRank].drawWorld = draw;
    return true;
}

bool Gfx::CEngine::SetObjectDrawFront(int objRank, bool draw)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    m_objects[objRank].drawFront = draw;
    return true;
}

bool Gfx::CEngine::SetObjectTransparency(int objRank, float value)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    m_objects[objRank].transparency = value;
    return true;
}

bool Gfx::CEngine::GetObjectBBox(int objRank, Math::Vector& min, Math::Vector& max)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return 0;

    min = m_objects[objRank].bboxMin;
    max = m_objects[objRank].bboxMax;
    return true;
}


int Gfx::CEngine::GetObjectTotalTriangles(int objRank)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return 0;

    return m_objects[objRank].totalTriangles;
}


Gfx::EngineObjLevel1& Gfx::CEngine::AddLevel1(const std::string& tex1Name, const std::string& tex2Name)
{
    bool unusedPresent = false;
    for (int i = 0; i < static_cast<int>( m_objectTree.size() ); i++)
    {
        if (! m_objectTree[i].used)
        {
            unusedPresent = true;
            continue;
        }

        if (m_objectTree[i].tex1Name == tex1Name && m_objectTree[i].tex2Name == tex2Name)
            return m_objectTree[i];
    }

    if (unusedPresent)
    {
        for (int i = 0; i < static_cast<int>( m_objectTree.size() ); i++)
        {
            if (! m_objectTree[i].used)
            {
                m_objectTree[i].used = true;
                m_objectTree[i].tex1Name = tex1Name;
                m_objectTree[i].tex2Name = tex2Name;
                return m_objectTree[i];
            }
        }
    }

    m_objectTree.push_back(Gfx::EngineObjLevel1(true, tex1Name, tex2Name));
    return m_objectTree.back();
}

Gfx::EngineObjLevel2& Gfx::CEngine::AddLevel2(Gfx::EngineObjLevel1& p1, int objRank)
{
    bool unusedPresent = false;
    for (int i = 0; i < static_cast<int>( p1.next.size() ); i++)
    {
        if (! p1.next[i].used)
        {
            unusedPresent = true;
            continue;
        }

        if (p1.next[i].objRank == objRank)
            return p1.next[i];
    }

    if (unusedPresent)
    {
        for (int i = 0; i < static_cast<int>( p1.next.size() ); i++)
        {
            if (! p1.next[i].used)
            {
                p1.next[i].used = true;
                p1.next[i].objRank = objRank;
                return p1.next[i];
            }
        }
    }

    p1.next.push_back(Gfx::EngineObjLevel2(true, objRank));
    return p1.next.back();
}

Gfx::EngineObjLevel3& Gfx::CEngine::AddLevel3(Gfx::EngineObjLevel2& p2, float min, float max)
{
    bool unusedPresent = false;
    for (int i = 0; i < static_cast<int>( p2.next.size() ); i++)
    {
        if (! p2.next[i].used)
        {
            unusedPresent = true;
            continue;
        }

        if ( (p2.next[i].min == min) && (p2.next[i].max == max) )
            return p2.next[i];
    }

    if (unusedPresent)
    {
        for (int i = 0; i < static_cast<int>( p2.next.size() ); i++)
        {
            if (! p2.next[i].used)
            {
                p2.next[i].used = true;
                p2.next[i].min = min;
                p2.next[i].max = max;
                return p2.next[i];
            }
        }
    }

    p2.next.push_back(Gfx::EngineObjLevel3(true, min, max));
    return p2.next.back();
}

Gfx::EngineObjLevel4& Gfx::CEngine::AddLevel4(Gfx::EngineObjLevel3& p3, Gfx::EngineTriangleType type,
                                              const Gfx::Material& material, int state)
{
    bool unusedPresent = false;
    for (int i = 0; i < static_cast<int>( p3.next.size() ); i++)
    {
        if (! p3.next[i].used)
        {
            unusedPresent = true;
            continue;
        }

        if ( (p3.next[i].type == type) && (p3.next[i].material == material) && (p3.next[i].state == state) )
            return p3.next[i];
    }

    if (unusedPresent)
    {
        for (int i = 0; i < static_cast<int>( p3.next.size() ); i++)
        {
            if (! p3.next[i].used)
            {
                p3.next[i].used = true;
                p3.next[i].type = type;
                p3.next[i].material = material;
                p3.next[i].state = state;
                return p3.next[i];
            }
        }
    }

    p3.next.push_back(Gfx::EngineObjLevel4(true, type, material, state));
    return p3.next.back();
}

bool Gfx::CEngine::AddTriangles(int objRank, const std::vector<Gfx::VertexTex2>& vertices,
                                const Gfx::Material& material, int state,
                                std::string tex1Name, std::string tex2Name,
                                float min, float max, bool globalUpdate)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
    {
        GetLogger()->Error("AddTriangle(): invalid object rank %d\n", objRank);
        return false;
    }

    m_lastSize = m_size;
    m_lastObjectDetail = m_objectDetail;
    m_lastClippingDistance = m_clippingDistance;

    Gfx::EngineObjLevel1& p1 = AddLevel1(tex1Name, tex2Name);
    Gfx::EngineObjLevel2& p2 = AddLevel2(p1, objRank);
    Gfx::EngineObjLevel3& p3 = AddLevel3(p2, min, max);
    Gfx::EngineObjLevel4& p4 = AddLevel4(p3, Gfx::ENG_TRIANGLE_TYPE_TRIANGLES, material, state);

    p4.vertices.insert(p4.vertices.end(), vertices.begin(), vertices.end());

    if (globalUpdate)
    {
        m_updateGeometry = true;
    }
    else
    {
        for (int i = 0; i < static_cast<int>( vertices.size() ); i++)
        {
            m_objects[objRank].bboxMin.x = Math::Min(vertices[i].coord.x, m_objects[objRank].bboxMin.x);
            m_objects[objRank].bboxMin.y = Math::Min(vertices[i].coord.y, m_objects[objRank].bboxMin.y);
            m_objects[objRank].bboxMin.z = Math::Min(vertices[i].coord.z, m_objects[objRank].bboxMin.z);
            m_objects[objRank].bboxMax.x = Math::Max(vertices[i].coord.x, m_objects[objRank].bboxMax.x);
            m_objects[objRank].bboxMax.y = Math::Max(vertices[i].coord.y, m_objects[objRank].bboxMax.y);
            m_objects[objRank].bboxMax.z = Math::Max(vertices[i].coord.z, m_objects[objRank].bboxMax.z);
        }

        m_objects[objRank].radius = Math::Max(m_objects[objRank].bboxMin.Length(),
                                              m_objects[objRank].bboxMax.Length());
    }

    m_objects[objRank].totalTriangles += vertices.size() / 3;

    return true;
}

bool Gfx::CEngine::AddSurface(int objRank, const std::vector<Gfx::VertexTex2>& vertices,
                              const Gfx::Material& material, int state,
                              std::string tex1Name, std::string tex2Name,
                              float min, float max, bool globalUpdate)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
    {
        GetLogger()->Error("AddSurface(): invalid object rank %d\n", objRank);
        return false;
    }

    m_lastSize = m_size;
    m_lastObjectDetail = m_objectDetail;
    m_lastClippingDistance = m_clippingDistance;

    Gfx::EngineObjLevel1& p1 = AddLevel1(tex1Name, tex2Name);
    Gfx::EngineObjLevel2& p2 = AddLevel2(p1, objRank);
    Gfx::EngineObjLevel3& p3 = AddLevel3(p2, min, max);
    Gfx::EngineObjLevel4& p4 = AddLevel4(p3, Gfx::ENG_TRIANGLE_TYPE_SURFACE, material, state);

    p4.vertices.insert(p4.vertices.end(), vertices.begin(), vertices.end());

    if (globalUpdate)
    {
        m_updateGeometry = true;
    }
    else
    {
        for (int i = 0; i < static_cast<int>( vertices.size() ); i++)
        {
            m_objects[objRank].bboxMin.x = Math::Min(vertices[i].coord.x, m_objects[objRank].bboxMin.x);
            m_objects[objRank].bboxMin.y = Math::Min(vertices[i].coord.y, m_objects[objRank].bboxMin.y);
            m_objects[objRank].bboxMin.z = Math::Min(vertices[i].coord.z, m_objects[objRank].bboxMin.z);
            m_objects[objRank].bboxMax.x = Math::Max(vertices[i].coord.x, m_objects[objRank].bboxMax.x);
            m_objects[objRank].bboxMax.y = Math::Max(vertices[i].coord.y, m_objects[objRank].bboxMax.y);
            m_objects[objRank].bboxMax.z = Math::Max(vertices[i].coord.z, m_objects[objRank].bboxMax.z);
        }

        m_objects[objRank].radius = Math::Max(m_objects[objRank].bboxMin.Length(),
                                              m_objects[objRank].bboxMax.Length());
    }

    m_objects[objRank].totalTriangles += vertices.size() - 2;

    return true;
}

bool Gfx::CEngine::AddQuick(int objRank, const Gfx::EngineObjLevel4& buffer,
                            std::string tex1Name, std::string tex2Name,
                            float min, float max, bool globalUpdate)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
    {
        GetLogger()->Error("AddQuick(): invalid object rank %d\n", objRank);
        return false;
    }

    Gfx::EngineObjLevel1& p1 = AddLevel1(tex1Name, tex2Name);
    Gfx::EngineObjLevel2& p2 = AddLevel2(p1, objRank);
    Gfx::EngineObjLevel3& p3 = AddLevel3(p2, min, max);

    p3.next.push_back(buffer);
    p3.next.back().used = true; // ensure that it is used

    if (globalUpdate)
    {
        m_updateGeometry = true;
    }
    else
    {
        for (int i = 0; i < static_cast<int>( buffer.vertices.size() ); i++)
        {
            m_objects[objRank].bboxMin.x = Math::Min(buffer.vertices[i].coord.x, m_objects[objRank].bboxMin.x);
            m_objects[objRank].bboxMin.y = Math::Min(buffer.vertices[i].coord.y, m_objects[objRank].bboxMin.y);
            m_objects[objRank].bboxMin.z = Math::Min(buffer.vertices[i].coord.z, m_objects[objRank].bboxMin.z);
            m_objects[objRank].bboxMax.x = Math::Max(buffer.vertices[i].coord.x, m_objects[objRank].bboxMax.x);
            m_objects[objRank].bboxMax.y = Math::Max(buffer.vertices[i].coord.y, m_objects[objRank].bboxMax.y);
            m_objects[objRank].bboxMax.z = Math::Max(buffer.vertices[i].coord.z, m_objects[objRank].bboxMax.z);
        }

        m_objects[objRank].radius = Math::Max(m_objects[objRank].bboxMin.Length(),
                                              m_objects[objRank].bboxMax.Length());
    }

    if (buffer.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
        m_objects[objRank].totalTriangles += buffer.vertices.size() / 3;
    else if (buffer.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
        m_objects[objRank].totalTriangles += buffer.vertices.size() - 2;

    return true;
}

Gfx::EngineObjLevel4* Gfx::CEngine::FindTriangles(int objRank, const Gfx::Material& material,
                                                  int state, std::string tex1Name,
                                                  std::string tex2Name, float min, float max)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
    {
        GetLogger()->Error("FindTriangles(): invalid object rank %d\n", objRank);
        return nullptr;
    }

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        if (p1.tex1Name != tex1Name) continue;
        // TODO: tex2Name compare?

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            if (p2.objRank != objRank) continue;

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                Gfx::EngineObjLevel3& p3 = p2.next[l3];
                if (! p3.used) continue;

                if (p3.min != min || p3.max != max) continue;

                for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                {
                    Gfx::EngineObjLevel4& p4 = p3.next[l4];
                    if (! p4.used) continue;

                    if ( (p4.state & (~(Gfx::ENG_RSTATE_DUAL_BLACK|Gfx::ENG_RSTATE_DUAL_WHITE))) != state ||
                         p4.material != material )
                        continue;

                    return &p4;
                }
            }
        }
    }

    return nullptr;
}

int Gfx::CEngine::GetPartialTriangles(int objRank, float min, float max, float percent, int maxCount,
                                      std::vector<Gfx::EngineTriangle>& triangles)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
    {
        GetLogger()->Error("GetPartialTriangles(): invalid object rank %d\n", objRank);
        return 0;
    }

    int total = m_objects[objRank].totalTriangles;
    int expectedCount = static_cast<int>(percent * total);
    triangles.reserve(Math::Min(maxCount, expectedCount));

    int actualCount = 0;

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            if (p2.objRank != objRank) continue;

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                Gfx::EngineObjLevel3& p3 = p2.next[l3];
                if (! p3.used) continue;

                if (p3.min != min || p3.max != max) continue;

                for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                {
                    Gfx::EngineObjLevel4& p4 = p3.next[l4];
                    if (! p4.used) continue;

                    if (p4.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
                    {
                        for (int i = 0; i < static_cast<int>( p4.vertices.size() ); i += 3)
                        {
                            if (static_cast<float>(actualCount) / total >= percent)
                                break;

                            if (actualCount >= maxCount)
                                break;

                            Gfx::EngineTriangle t;
                            t.triangle[0] = p4.vertices[i];
                            t.triangle[1] = p4.vertices[i+1];
                            t.triangle[2] = p4.vertices[i+2];
                            t.material = p4.material;
                            t.state = p4.state;
                            t.tex1Name = p1.tex1Name;
                            t.tex2Name = p1.tex2Name;

                            triangles.push_back(t);

                            ++actualCount;
                        }
                    }
                    else if (p4.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
                    {
                        for (int i = 0; i < static_cast<int>( p4.vertices.size() ); i += 1)
                        {
                            if (static_cast<float>(actualCount) / total >= percent)
                                break;

                            if (actualCount >= maxCount)
                                break;

                            Gfx::EngineTriangle t;
                            t.triangle[0] = p4.vertices[i];
                            t.triangle[1] = p4.vertices[i+1];
                            t.triangle[2] = p4.vertices[i+2];
                            t.material = p4.material;
                            t.state = p4.state;
                            t.tex1Name = p1.tex1Name;
                            t.tex2Name = p1.tex2Name;

                            triangles.push_back(t);

                            ++actualCount;
                        }
                    }
                }
            }
        }
    }

    return actualCount;
}

void Gfx::CEngine::ChangeLOD()
{
    float oldLimit[2] =
    {
        GetLimitLOD(0, true),
        GetLimitLOD(1, true)
    };

    float newLimit[2] =
    {
        GetLimitLOD(0, false),
        GetLimitLOD(1, false)
    };

    float oldTerrain = m_terrainVision * m_lastClippingDistance;
    float newTerrain = m_terrainVision * m_clippingDistance;

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                Gfx::EngineObjLevel3& p3 = p2.next[l3];
                if (! p3.used) continue;

                if ( Math::IsEqual(p3.min, 0.0f       ) &&
                     Math::IsEqual(p3.max, oldLimit[0]) )
                {
                    p3.max = newLimit[0];
                }
                else if ( Math::IsEqual(p3.min, oldLimit[0]) &&
                          Math::IsEqual(p3.max, oldLimit[1]) )
                {
                    p3.min = newLimit[0];
                    p3.max = newLimit[1];
                }
                else if ( Math::IsEqual(p3.min, oldLimit[1]) &&
                          Math::IsEqual(p3.max, 1000000.0f ) )
                {
                    p3.min = newLimit[1];
                }
                else if ( Math::IsEqual(p3.min, 0.0f      ) &&
                          Math::IsEqual(p3.max, oldTerrain) )
                {
                    p3.max = newTerrain;
                }
            }
        }
    }

    m_lastSize = m_size;
    m_lastObjectDetail = m_objectDetail;
    m_lastClippingDistance = m_clippingDistance;
}

bool Gfx::CEngine::ChangeSecondTexture(int objRank, const std::string& tex2Name)
{
    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        if (p1.tex2Name == tex2Name) continue;  // already new

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            if (p2.objRank != objRank) continue;

            Gfx::EngineObjLevel1& newP1 = AddLevel1(p1.tex1Name, tex2Name);

            newP1.next.push_back(Gfx::EngineObjLevel2(true, objRank));

            Gfx::EngineObjLevel2& newP2 = newP1.next.back();
            newP2.next.swap(p2.next);

            p2.used = false;
        }
    }
    return true;
}

bool Gfx::CEngine::ChangeTextureMapping(int objRank, const Gfx::Material& mat, int state,
                                         const std::string& tex1Name, const std::string& tex2Name,
                                         float min, float max, Gfx::EngineTextureMapping mode,
                                         float au, float bu, float av, float bv)
{
    Gfx::EngineObjLevel4* p4 = FindTriangles(objRank, mat, state, tex1Name, tex2Name, min, max);
    if (p4 == nullptr)
        return false;

    int nb = p4->vertices.size();

    if (mode == Gfx::ENG_TEX_MAPPING_X)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.z * au + bu;
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.y * av + bv;
        }
    }
    else if (mode == Gfx::ENG_TEX_MAPPING_Y)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.x * au + bu;
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.z * av + bv;
        }
    }
    else if (mode == Gfx::ENG_TEX_MAPPING_Z)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.x * au + bu;
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.y * av + bv;
        }
    }
    else if (mode == Gfx::ENG_TEX_MAPPING_1X)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.x * au + bu;
        }
    }
    else if (mode == Gfx::ENG_TEX_MAPPING_1Y)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.y * au + bu;
        }
    }
    else if (mode == Gfx::ENG_TEX_MAPPING_1Z)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.z * au + bu;
        }
    }

    return true;
}

bool Gfx::CEngine::TrackTextureMapping(int objRank, const Gfx::Material& mat, int state,
                                        const std::string& tex1Name, const std::string& tex2Name,
                                        float min, float max, Gfx::EngineTextureMapping mode,
                                        float pos, float factor, float tl, float ts, float tt)
{
    // TODO track texture mapping: pretty complex code, so leaving it for now
    GetLogger()->Trace("CEngine::TrackTextureMapping(): stub!\n");
    return true;
}


bool Gfx::CEngine::CreateShadow(int objRank)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    // Already allocated?
    if (m_objects[objRank].shadowRank != -1) return true;

    int index = 0;
    for ( ; index < static_cast<int>( m_shadows.size() ); index++)
    {
        if (! m_shadows[index].used)
        {
            m_shadows[index].LoadDefault();
            break;
        }
    }

    m_shadows.push_back(Gfx::EngineShadow());

    m_shadows[index].used = true;
    m_shadows[index].objRank = objRank;
    m_shadows[index].height = 0.0f;

    m_objects[objRank].shadowRank = index;

    return true;
}

void Gfx::CEngine::DeleteShadow(int objRank)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return;

    m_shadows[i].used = false;
    m_shadows[i].objRank = -1;

    m_objects[objRank].shadowRank = -1;
}

bool Gfx::CEngine::SetObjectShadowHide(int objRank, bool hide)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].hide = hide;
    return true;
}

bool Gfx::CEngine::SetObjectShadowType(int objRank, Gfx::EngineShadowType type)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].type = type;
    return true;
}

bool Gfx::CEngine::SetObjectShadowPos(int objRank, const Math::Vector& pos)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].pos = pos;
    return true;
}

bool Gfx::CEngine::SetObjectShadowNormal(int objRank, const Math::Vector& normal)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].normal = normal;
    return true;
}

bool Gfx::CEngine::SetObjectShadowAngle(int objRank, float angle)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].angle = angle;
    return true;
}

bool Gfx::CEngine::SetObjectShadowRadius(int objRank, float radius)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].radius = radius;
    return true;
}

bool Gfx::CEngine::SetObjectShadowIntensity(int objRank, float intensity)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].intensity = intensity;
    return true;
}

bool Gfx::CEngine::SetObjectShadowHeight(int objRank, float height)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return false;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return false;

    m_shadows[i].height = height;
    return true;
}

float Gfx::CEngine::GetObjectShadowRadius(int objRank)
{
    if ( objRank < 0 || objRank >= static_cast<int>( m_objects.size() ) )
        return 0.0f;

    int i = m_objects[objRank].shadowRank;
    if (i == -1)
        return 0.0f;

    return m_shadows[i].radius;
}

bool Gfx::CEngine::GetHighlight(Math::Point &p1, Math::Point &p2)
{
    p1 = m_highlightP1;
    p2 = m_highlightP2;
    return m_highlight;
}

void Gfx::CEngine::SetHighlightRank(int *rankList)
{
    int i = 0;
    while ( *rankList != -1 )
    {
        m_highlightRank[i++] = *rankList++;
    }
    m_highlightRank[i] = -1;  // terminator
}

bool Gfx::CEngine::GetBBox2D(int objRank, Math::Point &min, Math::Point &max)
{
    min.x =  1000000.0f;
    min.y =  1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    for (int i = 0; i < 8; i++)
    {
        Math::Vector p;

        if ( i & (1<<0) )  p.x = m_objects[objRank].bboxMin.x;
        else               p.x = m_objects[objRank].bboxMax.x;
        if ( i & (1<<1) )  p.y = m_objects[objRank].bboxMin.y;
        else               p.y = m_objects[objRank].bboxMax.y;
        if ( i & (1<<2) )  p.z = m_objects[objRank].bboxMin.z;
        else               p.z = m_objects[objRank].bboxMax.z;

        Math::Vector pp;
        if (TransformPoint(pp, objRank, p))
        {
            if (pp.x < min.x) min.x = pp.x;
            if (pp.x > max.x) max.x = pp.x;
            if (pp.y < min.y) min.y = pp.y;
            if (pp.y > max.y) max.y = pp.y;
        }
    }

    if ( min.x ==  1000000.0f ||
         min.y ==  1000000.0f ||
         max.x == -1000000.0f ||
         max.y == -1000000.0f )  return false;

    return true;
}

void Gfx::CEngine::FlushGroundSpot()
{
    m_groundSpots.clear();
    m_firstGroundSpot = true;

    // TODO: blank all shadow textures
}

int Gfx::CEngine::CreateGroundSpot()
{
    int index = 0;
    for ( ; index < static_cast<int>( m_groundSpots.size() ); index++)
    {
        if (! m_groundSpots[index].used)
        {
            m_groundSpots[index].LoadDefault();
            break;
        }
    }

    m_groundSpots.push_back(Gfx::EngineGroundSpot());

    m_groundSpots[index].used = true;
    m_groundSpots[index].smooth = 1.0f;

    return index;
}

void Gfx::CEngine::DeleteGroundSpot(int rank)
{
    m_groundSpots[rank].used = false;
    m_groundSpots[rank].pos = Math::Vector(0.0f, 0.0f, 0.0f);
}

bool Gfx::CEngine::SetObjectGroundSpotPos(int rank, const Math::Vector& pos)
{
    if ( rank < 0 || rank >= static_cast<int>( m_groundSpots.size() ) )
        return 0.0f;

    m_groundSpots[rank].pos = pos;
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotRadius(int rank, float radius)
{
    if ( rank < 0 || rank >= static_cast<int>( m_groundSpots.size() ) )
        return 0.0f;

    m_groundSpots[rank].radius = radius;
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotColor(int rank, const Gfx::Color& color)
{
    if ( rank < 0 || rank >= static_cast<int>( m_groundSpots.size() ) )
        return 0.0f;

    m_groundSpots[rank].color = color;
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotMinMax(int rank, float min, float max)
{
    if ( rank < 0 || rank >= static_cast<int>( m_groundSpots.size() ) )
        return 0.0f;

    m_groundSpots[rank].min = min;
    m_groundSpots[rank].max = max;
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotSmooth(int rank, float smooth)
{
    if ( rank < 0 || rank >= static_cast<int>( m_groundSpots.size() ) )
        return 0.0f;

    m_groundSpots[rank].smooth = smooth;
    return true;
}

void Gfx::CEngine::CreateGroundMark(Math::Vector pos, float radius,
                                   float delay1, float delay2, float delay3,
                                   int dx, int dy, char* table)
{
    m_groundMark.LoadDefault();

    m_groundMark.phase     = Gfx::ENG_GR_MARK_PHASE_INC;
    m_groundMark.delay[0]  = delay1;
    m_groundMark.delay[1]  = delay2;
    m_groundMark.delay[2]  = delay3;
    m_groundMark.pos       = pos;
    m_groundMark.radius    = radius;
    m_groundMark.intensity = 0.0f;
    m_groundMark.dx        = dx;
    m_groundMark.dy        = dy;
    m_groundMark.table     = table;
}

void Gfx::CEngine::DeleteGroundMark(int rank)
{
    m_groundMark.LoadDefault();
}

void Gfx::CEngine::ComputeDistance()
{
    // TODO: s_resol???

    for (int i = 0; i < static_cast<int>( m_objects.size() ); i++)
    {
        if (! m_objects[i].used)
            continue;

        Math::Vector v;
        v.x = m_eyePt.x - m_objects[i].transform.Get(1, 4);
        v.y = m_eyePt.y - m_objects[i].transform.Get(2, 4);
        v.z = m_eyePt.z - m_objects[i].transform.Get(3, 4);
        m_objects[i].distance = v.Length();
    }
}

void Gfx::CEngine::UpdateGeometry()
{
    if (! m_updateGeometry)
        return;

    for (int i = 0; i < static_cast<int>( m_objects.size() ); i++)
    {
        m_objects[i].bboxMin.x = 0;
        m_objects[i].bboxMin.y = 0;
        m_objects[i].bboxMin.z = 0;
        m_objects[i].bboxMax.x = 0;
        m_objects[i].bboxMax.y = 0;
        m_objects[i].bboxMax.z = 0;
        m_objects[i].radius = 0;
    }

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                Gfx::EngineObjLevel3& p3 = p2.next[l3];
                if (! p3.used) continue;

                for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                {
                    Gfx::EngineObjLevel4& p4 = p3.next[l4];
                    if (! p4.used) continue;

                    int objRank = p2.objRank;

                    for (int i = 0; i < static_cast<int>( p4.vertices.size() ); i++)
                    {
                            m_objects[objRank].bboxMin.x = Math::Min(p4.vertices[i].coord.x, m_objects[objRank].bboxMin.x);
                            m_objects[objRank].bboxMin.y = Math::Min(p4.vertices[i].coord.y, m_objects[objRank].bboxMin.y);
                            m_objects[objRank].bboxMin.z = Math::Min(p4.vertices[i].coord.z, m_objects[objRank].bboxMin.z);
                            m_objects[objRank].bboxMax.x = Math::Max(p4.vertices[i].coord.x, m_objects[objRank].bboxMax.x);
                            m_objects[objRank].bboxMax.y = Math::Max(p4.vertices[i].coord.y, m_objects[objRank].bboxMax.y);
                            m_objects[objRank].bboxMax.z = Math::Max(p4.vertices[i].coord.z, m_objects[objRank].bboxMax.z);
                    }

                    m_objects[objRank].radius = Math::Max(m_objects[objRank].bboxMin.Length(),
                                                          m_objects[objRank].bboxMax.Length());
                }
            }
        }
    }

    m_updateGeometry = false;
}

void Gfx::CEngine::Update()
{
    ComputeDistance();
    UpdateGeometry();
}

bool Gfx::CEngine::DetectBBox(int objRank, Math::Point mouse)
{
    Math::Point min, max;
    min.x =  1000000.0f;
    min.y =  1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    for (int i = 0; i < 8; i++)
    {
        Math::Vector p;

        if ( i & (1<<0) )  p.x = m_objects[objRank].bboxMin.x;
        else               p.x = m_objects[objRank].bboxMax.x;
        if ( i & (1<<1) )  p.y = m_objects[objRank].bboxMin.y;
        else               p.y = m_objects[objRank].bboxMax.y;
        if ( i & (1<<2) )  p.z = m_objects[objRank].bboxMin.z;
        else               p.z = m_objects[objRank].bboxMax.z;

        Math::Vector pp;
        if ( TransformPoint(pp, objRank, p) )
        {
            if (pp.x < min.x) min.x = pp.x;
            if (pp.x > max.x) max.x = pp.x;
            if (pp.y < min.y) min.y = pp.y;
            if (pp.y > max.y) max.y = pp.y;
        }
    }

    return ( mouse.x >= min.x &&
             mouse.x <= max.x &&
             mouse.y >= min.y &&
             mouse.y <= max.y );
}

int Gfx::CEngine::DetectObject(Math::Point mouse)
{
    float min = 1000000.0f;
    int nearest = -1;

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            if (m_objects[p2.objRank].type == Gfx::ENG_OBJTYPE_TERRAIN) continue;

            if (! DetectBBox(p2.objRank, mouse)) continue;

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                Gfx::EngineObjLevel3& p3 = p2.next[l3];
                if (! p3.used) continue;

                if (p3.min != 0.0f) continue;  // LOD B or C?

                for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                {
                    Gfx::EngineObjLevel4& p4 = p3.next[l4];
                    if (! p4.used) continue;

                    if (p4.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
                    {
                        for (int i = 0; i < static_cast<int>( p4.vertices.size() ); i += 3)
                        {
                            float dist = 0.0f;
                            if (DetectTriangle(mouse, &p4.vertices[i], p2.objRank, dist) && dist < min)
                            {
                                min = dist;
                                nearest = p2.objRank;
                            }
                        }
                    }
                    else if (p4.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
                    {
                        for (int i = 0; i < static_cast<int>( p4.vertices.size() ) - 2; i += 1)
                        {
                            float dist = 0.0f;
                            if (DetectTriangle(mouse, &p4.vertices[i], p2.objRank, dist) && dist < min)
                            {
                                min = dist;
                                nearest = p2.objRank;
                            }
                        }
                    }
                }
            }
        }
    }

    return nearest;
}

bool Gfx::CEngine::DetectTriangle(Math::Point mouse, Gfx::VertexTex2* triangle, int objRank, float& dist)
{
    Math::Vector p2D[3], p3D;

    for (int i = 0; i < 3; i++)
    {
        p3D.x = triangle[i].coord.x;
        p3D.y = triangle[i].coord.y;
        p3D.z = triangle[i].coord.z;

        if (! TransformPoint(p2D[i], objRank, p3D))
            return false;
    }

    if ( mouse.x < p2D[0].x &&
         mouse.x < p2D[1].x &&
         mouse.x < p2D[2].x )  return false;
    if ( mouse.x > p2D[0].x &&
         mouse.x > p2D[1].x &&
         mouse.x > p2D[2].x )  return false;
    if ( mouse.y < p2D[0].y &&
         mouse.y < p2D[1].y &&
         mouse.y < p2D[2].y )  return false;
    if ( mouse.y > p2D[0].y &&
         mouse.y > p2D[1].y &&
         mouse.y > p2D[2].y )  return false;

    Math::Point a, b, c;
    a.x = p2D[0].x;
    a.y = p2D[0].y;
    b.x = p2D[1].x;
    b.y = p2D[1].y;
    c.x = p2D[2].x;
    c.y = p2D[2].y;

    if (! Math::IsInsideTriangle(a, b, c, mouse))
        return false;

    dist = (p2D[0].z + p2D[1].z + p2D[2].z) / 3.0f;
    return true;
}

bool Gfx::CEngine::IsVisible(int objRank)
{
    // TODO: use ComputeSphereVisiblity() after tested OK
    return true;
}

bool Gfx::CEngine::TransformPoint(Math::Vector& p2D, int objRank, Math::Vector p3D)
{
    p3D = Math::Transform(m_objects[objRank].transform, p3D);
    p3D = Math::Transform(m_matView, p3D);

    if (p3D.z < 2.0f)  return false;  // behind?

    p2D.x = (p3D.x/p3D.z)*m_matProj.Get(1,1);
    p2D.y = (p3D.y/p3D.z)*m_matProj.Get(2,2);
    p2D.z = p3D.z;

    p2D.x = (p2D.x+1.0f)/2.0f;  // [-1..1] -> [0..1]
    p2D.y = (p2D.y+1.0f)/2.0f;

    return true;
}



/*******************************************************
                    Mode setting
 *******************************************************/



void Gfx::CEngine::SetState(int state, const Gfx::Color& color)
{
    if (state == m_lastState && color == m_lastColor)
        return;

    m_lastState = state;
    m_lastColor = color;

    if (m_alphaMode != 1 && (state & Gfx::ENG_RSTATE_ALPHA))
    {
        state &= ~Gfx::ENG_RSTATE_ALPHA;

        if (m_alphaMode == 2)
            state |= Gfx::ENG_RSTATE_TTEXTURE_BLACK;
    }


    if (state & Gfx::ENG_RSTATE_TTEXTURE_BLACK)  // transparent black texture?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(Gfx::BLEND_ONE, Gfx::BLEND_INV_SRC_COLOR);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        m_device->SetTextureFactor(color);

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_FACTOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // TODO: replace with src color ?

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & Gfx::ENG_RSTATE_TTEXTURE_WHITE)  // transparent white texture?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(Gfx::BLEND_DST_COLOR, Gfx::BLEND_ZERO);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        m_device->SetTextureFactor(color.Inverse());

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_ADD;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_FACTOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // TODO: replace with src color ?

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & Gfx::ENG_RSTATE_TCOLOR_BLACK)  // transparent black color?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   false);

        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(Gfx::BLEND_ONE, Gfx::BLEND_INV_SRC_COLOR);
    }
    else if (state & Gfx::ENG_RSTATE_TCOLOR_WHITE)  // transparent white color?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   false);

        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(Gfx::BLEND_DST_COLOR, Gfx::BLEND_ZERO);
    }
    else if (state & Gfx::ENG_RSTATE_TDIFFUSE)  // diffuse color as transparent?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(Gfx::BLEND_SRC_ALPHA, Gfx::BLEND_DST_ALPHA);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_REPLACE;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // TODO: replace with src color ?

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & Gfx::ENG_RSTATE_OPAQUE_TEXTURE) // opaque texture ?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    false);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, Gfx::TextureStageParams()); // default operation
    }
    else if (state & Gfx::ENG_RSTATE_OPAQUE_COLOR) // opaque color ?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   false);
    }
    else if (state & Gfx::ENG_RSTATE_TEXT)  // font rendering?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(Gfx::BLEND_SRC_ALPHA, Gfx::BLEND_INV_SRC_ALPHA);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_DEFAULT; // default modulate operation
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // default modulate operation

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & Gfx::ENG_RSTATE_ALPHA)  // image with alpha channel?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    false);

        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         true);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, true);

        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  true);

        m_device->SetAlphaTestFunc(Gfx::COMP_FUNC_GREATER, 0.5f);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        m_device->SetTextureFactor(color);

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_SRC_COLOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_REPLACE;
        params.alphaArg1 = Gfx::TEX_MIX_ARG_TEXTURE;

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else    // normal ?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    false);

        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, true);
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         true);

        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, true);

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_DEFAULT; // default modulate
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // TODO: replace with src color ?

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }

    if (state & Gfx::ENG_RSTATE_FOG)
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG, true);


    bool second = m_groundSpotVisible || m_dirty;

    if ( !m_groundSpotVisible && (state & Gfx::ENG_RSTATE_SECOND) != 0 ) second = false;
    if ( !m_dirty             && (state & Gfx::ENG_RSTATE_SECOND) == 0 ) second = false;

    if ((state & ENG_RSTATE_DUAL_BLACK) && second)
    {
        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_COMPUTED_COLOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // TODO: ???
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(1, params);
    }
    else if ((state & ENG_RSTATE_DUAL_WHITE) && second)
    {
        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_ADD;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_COMPUTED_COLOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_DEFAULT; // TODO: ???
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(1, params);
    }
    else
    {
        m_device->SetTextureEnabled(1, false);
    }

    if (state & Gfx::ENG_RSTATE_WRAP)
    {
        // TODO: separate function for setting wrap mode?

        Gfx::TextureStageParams p1 = m_device->GetTextureStageParams(0);
        p1.wrapS = p1.wrapT = Gfx::TEX_WRAP_REPEAT;
        m_device->SetTextureStageParams(0, p1);

        Gfx::TextureStageParams p2 = m_device->GetTextureStageParams(1);
        p2.wrapS = p2.wrapT = Gfx::TEX_WRAP_REPEAT;
        m_device->SetTextureStageParams(1, p2);
    }
    else // if (state & Gfx::ENG_RSTATE_CLAMP) or otherwise
    {
        Gfx::TextureStageParams p1 = m_device->GetTextureStageParams(0);
        p1.wrapS = p1.wrapT = Gfx::TEX_WRAP_CLAMP;
        m_device->SetTextureStageParams(0, p1);

        Gfx::TextureStageParams p2 = m_device->GetTextureStageParams(1);
        p2.wrapS = p2.wrapT = Gfx::TEX_WRAP_CLAMP;
        m_device->SetTextureStageParams(1, p2);
    }

    if (state & Gfx::ENG_RSTATE_2FACE)
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_CULLING, false);
    }
    else
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_CULLING, true);
        m_device->SetCullMode(Gfx::CULL_CCW);
    }

    if (state & Gfx::ENG_RSTATE_LIGHT)
        m_device->SetGlobalAmbient(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    else
        m_device->SetGlobalAmbient(m_ambientColor[m_rankView]);
}

void Gfx::CEngine::SetMaterial(const Gfx::Material& mat)
{
    m_lastMaterial = mat;
    m_device->SetMaterial(mat);
}

void Gfx::CEngine::SetViewParams(const Math::Vector& eyePt, const Math::Vector& lookatPt,
                                 const Math::Vector& upVec, float eyeDistance)
{
    m_eyePt = eyePt;
    m_lookatPt = lookatPt;
    m_eyeDirH = Math::RotateAngle(eyePt.x - lookatPt.x, eyePt.z - lookatPt.z);
    m_eyeDirV = Math::RotateAngle(Math::DistanceProjected(eyePt, lookatPt), eyePt.y - lookatPt.y);

    Math::LoadViewMatrix(m_matView, eyePt, lookatPt, upVec);

    if (m_sound == nullptr)
        m_sound = static_cast<CSoundInterface*>( m_iMan->SearchInstance(CLASS_SOUND) );

    m_sound->SetListener(eyePt, lookatPt);
}

Gfx::Texture Gfx::CEngine::CreateTexture(const std::string& texName, const Gfx::TextureCreateParams& params)
{
    if (m_texBlacklist.find(texName) != m_texBlacklist.end())
        return Gfx::Texture(); // invalid texture

    // TODO: detect alpha channel?

    CImage img;
    if (! img.Load(m_app->GetDataFilePath(m_texPath, texName)))
    {
        std::string error = img.GetError();
        GetLogger()->Error("Couldn't load texture '%s': %s, blacklisting\n", texName.c_str(), error.c_str());
        m_texBlacklist.insert(texName);
        return Gfx::Texture(); // invalid texture
    }

    Gfx::Texture tex = m_device->CreateTexture(&img, params);

    if (! tex.Valid())
    {
        GetLogger()->Error("Couldn't load texture '%s', blacklisting\n", texName.c_str());
        m_texBlacklist.insert(texName);
        return tex;
    }

    m_texNameMap[texName] = tex;
    m_revTexNameMap[tex] = texName;

    return tex;
}

Gfx::Texture Gfx::CEngine::LoadTexture(const std::string& name)
{
    return LoadTexture(name, m_defaultTexParams);
}

Gfx::Texture Gfx::CEngine::LoadTexture(const std::string& name, const Gfx::TextureCreateParams& params)
{
    if (m_texBlacklist.find(name) != m_texBlacklist.end())
        return Gfx::Texture();

    std::map<std::string, Gfx::Texture>::iterator it = m_texNameMap.find(name);
    if (it != m_texNameMap.end())
        return (*it).second;

    Gfx::Texture tex = CreateTexture(name, params);
    return tex;
}

bool Gfx::CEngine::LoadAllTextures()
{
    LoadTexture("text.png");
    m_miceTexture = LoadTexture("mouse.png");
    LoadTexture("button1.png");
    LoadTexture("button2.png");
    LoadTexture("button3.png");
    LoadTexture("effect00.png");
    LoadTexture("effect01.png");
    LoadTexture("effect02.png");
    LoadTexture("map.png");

    if (m_backgroundQuarter)  // image into 4 pieces?
    {
        if (! m_backgroundName.empty())
        {
            for (int i = 0; i < 4; i++)
                m_backgroundQuarterTexs[i] = LoadTexture(m_backgroundQuarterNames[i]);
        }
        else
        {
            for (int i = 0; i < 4; i++)
                m_backgroundQuarterTexs[i].SetInvalid();
        }
    }
    else
    {
        if (! m_backgroundName.empty())
            m_backgroundFullTex = LoadTexture(m_backgroundName);
        else
            m_backgroundFullTex.SetInvalid();
    }

    if (! m_foregroundName.empty())
        m_foregroundTex = LoadTexture(m_foregroundName);
    else
        m_foregroundTex.SetInvalid();

    m_planet->LoadTexture();

    bool ok = true;

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        if (! p1.tex1Name.empty())
        {
            p1.tex1 = LoadTexture(p1.tex1Name);
            if (! p1.tex1.Valid())
                ok = false;
        }

        if (! p1.tex2Name.empty())
        {
            p1.tex2 = LoadTexture(p1.tex2Name);
            if (! p1.tex2.Valid())
                ok = false;
        }
    }

    return ok;
}

void Gfx::CEngine::DeleteTexture(const std::string& texName)
{
    auto it = m_texNameMap.find(texName);
    if (it == m_texNameMap.end())
        return;

    auto revIt = m_revTexNameMap.find((*it).second);

    m_device->DestroyTexture((*it).second);

    m_revTexNameMap.erase(revIt);
    m_texNameMap.erase(it);
}

void Gfx::CEngine::DeleteTexture(const Gfx::Texture& tex)
{
    if (! tex.Valid())
        return;

    auto revIt = m_revTexNameMap.find(tex);
    if (revIt == m_revTexNameMap.end())
        return;

    m_device->DestroyTexture(tex);

    auto it = m_texNameMap.find((*revIt).second);

    m_revTexNameMap.erase(revIt);
    m_texNameMap.erase(it);
}

bool Gfx::CEngine::SetTexture(const std::string& name, int stage)
{
    auto it = m_texNameMap.find(name);
    if (it != m_texNameMap.end())
    {
        m_device->SetTexture(stage, (*it).second);
        return true;
    }

    if (! LoadTexture(name).Valid())
    {
        m_device->SetTexture(stage, 0); // invalid texture
        return false;
    }

    it = m_texNameMap.find(name);
    if (it != m_texNameMap.end())
    {
        m_device->SetTexture(stage, (*it).second);
        return true;
    }

    m_device->SetTexture(stage, 0); // invalid texture
    return false; // should not happen normally
}

void Gfx::CEngine::SetTexture(const Gfx::Texture& tex, int stage)
{
    m_device->SetTexture(stage, tex);
}

void Gfx::CEngine::SetLimitLOD(int rank, float limit)
{
    m_limitLOD[rank] = limit;
}

float Gfx::CEngine::GetLimitLOD(int rank, bool last)
{
    float limit = 0.0f;

    if (last)
    {
        limit = m_limitLOD[rank];
        limit *= m_lastSize.x/640.0f;  // limit further if large window!
        limit += m_limitLOD[0]*(m_lastObjectDetail*2.0f);
    }
    else
    {
        limit = m_limitLOD[rank];
        limit *= m_size.x/640.0f;  // limit further if large window!
        limit += m_limitLOD[0]*(m_objectDetail*2.0f);
    }

    if (limit < 0.0f) limit = 0.0f;

    return limit;
}

void Gfx::CEngine::SetTerrainVision(float vision)
{
    m_terrainVision = vision;
}

void Gfx::CEngine::SetFocus(float focus)
{
    m_focus = focus;
    m_size = m_app->GetVideoConfig().size;

    float aspect = (static_cast<float>(m_size.x)) / m_size.y;
    Math::LoadProjectionMatrix(m_matProj, m_focus, aspect, 0.5f, m_deepView[0]);
}

float Gfx::CEngine::GetFocus()
{
    return m_focus;
}

void Gfx::CEngine::SetGroundSpot(bool mode)
{
    m_groundSpotVisible = mode;
}

bool Gfx::CEngine::GetGroundSpot()
{
    return m_groundSpotVisible;
}

void Gfx::CEngine::SetShadow(bool mode)
{
    m_shadowVisible = mode;
}

bool Gfx::CEngine::GetShadow()
{
    return m_shadowVisible;
}

void Gfx::CEngine::SetDirty(bool mode)
{
    m_dirty = mode;
}

bool Gfx::CEngine::GetDirty()
{
    return m_dirty;
}

void Gfx::CEngine::SetFog(bool mode)
{
    m_fog = mode;
}

bool Gfx::CEngine::GetFog()
{
    return m_fog;
}

bool Gfx::CEngine::GetStateColor()
{
    return m_stateColor;
}

void Gfx::CEngine::SetSecondTexture(int texNum)
{
    m_secondTexNum = texNum;
}

int Gfx::CEngine::GetSecondTexture()
{
    return m_secondTexNum;
}

void Gfx::CEngine::SetRankView(int rank)
{
    if (rank < 0) rank = 0;
    if (rank > 1) rank = 1;

    if (m_rankView == 0 && rank == 1)  // enters the water?
        m_lightMan->AdaptLightColor(m_waterAddColor, +1.0f);

    if (m_rankView == 1 && rank == 0)  // out of the water?
        m_lightMan->AdaptLightColor(m_waterAddColor, -1.0f);

    m_rankView = rank;
}

int Gfx::CEngine::GetRankView()
{
    return m_rankView;
}

void Gfx::CEngine::SetDrawWorld(bool draw)
{
    m_drawWorld = draw;
}

void Gfx::CEngine::SetDrawFront(bool draw)
{
    m_drawFront = draw;
}

void Gfx::CEngine::SetAmbientColor(const Gfx::Color& color, int rank)
{
    m_ambientColor[rank] = color;
}

Gfx::Color Gfx::CEngine::GetAmbientColor(int rank)
{
    return m_ambientColor[rank];
}

void Gfx::CEngine::SetWaterAddColor(const Gfx::Color& color)
{
    m_waterAddColor = color;
}

Gfx::Color Gfx::CEngine::GetWaterAddColor()
{
    return m_waterAddColor;
}

void Gfx::CEngine::SetFogColor(const Gfx::Color& color, int rank)
{
    m_fogColor[rank] = color;
}

Gfx::Color Gfx::CEngine::GetFogColor(int rank)
{
    return m_fogColor[rank];
}

void Gfx::CEngine::SetDeepView(float length, int rank, bool ref)
{
    if (ref)
        length *= m_clippingDistance;

    m_deepView[rank] = length;
}

float Gfx::CEngine::GetDeepView(int rank)
{
    return m_deepView[rank];
}

void Gfx::CEngine::SetFogStart(float start, int rank)
{
    m_fogStart[rank] = start;
}

float Gfx::CEngine::GetFogStart(int rank)
{
    return m_fogStart[rank];
}

std::string QuarterName(const std::string& name, int quarter)
{
    size_t pos = name.find('.');
    if (pos == std::string::npos)
        return name;

    return name.substr(0, pos) + std::string(1, static_cast<char>('a' + quarter)) + name.substr(pos);
}

void Gfx::CEngine::SetBackground(const std::string& name, Gfx::Color up, Gfx::Color down,
                                 Gfx::Color cloudUp, Gfx::Color cloudDown,
                                 bool full, bool quarter)
{
    if (m_backgroundFullTex.Valid())
    {
        DeleteTexture(m_backgroundFullTex);
        m_backgroundFullTex.SetInvalid();
    }

    for (int i = 0; i < 4; i++)
    {
        DeleteTexture(m_backgroundQuarterTexs[i]);
        m_backgroundQuarterTexs[i].SetInvalid();
    }

    m_backgroundName      = name;
    m_backgroundColorUp   = up;
    m_backgroundColorDown = down;
    m_backgroundCloudUp   = cloudUp;
    m_backgroundCloudDown = cloudDown;
    m_backgroundFull      = full;
    m_backgroundQuarter   = quarter;

    if (! m_backgroundName.empty())
    {
        if (m_backgroundQuarter)
        {
            for (int i = 0; i < 4; i++)
            {
                m_backgroundQuarterNames[i] = QuarterName(name, i);
                m_backgroundQuarterTexs[i] = LoadTexture(m_backgroundQuarterNames[i]);
            }
        }
        else
        {
            m_backgroundFullTex = LoadTexture(m_backgroundName);
        }
    }
}

void Gfx::CEngine::GetBackground(std::string& name, Gfx::Color& up, Gfx::Color& down,
                                 Gfx::Color& cloudUp, Gfx::Color& cloudDown,
                                 bool &full, bool &quarter)
{
    name      = m_backgroundName;
    up        = m_backgroundColorUp;
    down      = m_backgroundColorDown;
    cloudUp   = m_backgroundCloudUp;
    cloudDown = m_backgroundCloudDown;
    full      = m_backgroundFull;
    quarter   = m_backgroundQuarter;
}

void Gfx::CEngine::SetForegroundName(const std::string& name)
{
    if (m_foregroundTex.Valid())
    {
        DeleteTexture(m_foregroundTex);
        m_foregroundTex.SetInvalid();
    }

    m_foregroundName = name;

    if (! m_foregroundName.empty())
        m_foregroundTex = LoadTexture(m_foregroundName);
}

void Gfx::CEngine::SetOverFront(bool front)
{
    m_overFront = front;
}

void Gfx::CEngine::SetOverColor(const Gfx::Color& color, int mode)
{
    m_overColor = color;
    m_overMode  = mode;
}

void Gfx::CEngine::SetParticleDensity(float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 2.0f) value = 2.0f;
    m_particleDensity = value;
}

float Gfx::CEngine::GetParticleDensity()
{
    return m_particleDensity;
}

float Gfx::CEngine::ParticleAdapt(float factor)
{
    if (m_particleDensity == 0.0f)
        return 1000000.0f;

    return factor / m_particleDensity;
}

void Gfx::CEngine::SetClippingDistance(float value)
{
    if (value < 0.5f) value = 0.5f;
    if (value > 2.0f) value = 2.0f;
    m_clippingDistance = value;
}

float Gfx::CEngine::GetClippingDistance()
{
    return m_clippingDistance;
}

void Gfx::CEngine::SetObjectDetail(float value)
{
    if ( value < 0.0f )  value = 0.0f;
    if ( value > 2.0f )  value = 2.0f;
    m_objectDetail = value;
}

float Gfx::CEngine::GetObjectDetail()
{
    return m_objectDetail;
}

void Gfx::CEngine::SetGadgetQuantity(float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    m_gadgetQuantity = value;
}

float Gfx::CEngine::GetGadgetQuantity()
{
    return m_gadgetQuantity;
}

void Gfx::CEngine::SetTextureQuality(int value)
{
    if (value < 0) value = 0;
    if (value > 2) value = 2;

    if (value != m_textureQuality)
    {
        m_textureQuality = value;
        LoadAllTextures();
    }
}

int Gfx::CEngine::GetTextureQuality()
{
    return m_textureQuality;
}

void Gfx::CEngine::SetTotoMode(bool present)
{
    m_totoMode = present;
}

bool Gfx::CEngine::GetTotoMode()
{
    return m_totoMode;
}

void Gfx::CEngine::SetLensMode(bool present)
{
    m_lensMode = present;
}

bool Gfx::CEngine::GetLensMode()
{
    return m_lensMode;
}

void Gfx::CEngine::SetWaterMode(bool present)
{
    m_waterMode = present;
}

bool Gfx::CEngine::GetWaterMode()
{
    return m_waterMode;
}

void Gfx::CEngine::SetLightingMode(bool present)
{
    m_lightMode = present;
}

bool Gfx::CEngine::GetLightingMode()
{
    return m_lightMode;
}

void Gfx::CEngine::SetSkyMode(bool present)
{
    m_skyMode = present;
}

bool Gfx::CEngine::GetSkyMode()
{
    return m_skyMode;
}

void Gfx::CEngine::SetBackForce(bool present)
{
    m_backForce = present;
}

bool Gfx::CEngine::GetBackForce()
{
    return m_backForce;
}

void Gfx::CEngine::SetPlanetMode(bool present)
{
    m_planetMode = present;
}

bool Gfx::CEngine::GetPlanetMode()
{
    return m_planetMode;
}

void Gfx::CEngine::SetLightMode(bool present)
{
    m_lightMode = present;
}

bool Gfx::CEngine::GetLightMode()
{
    return m_lightMode;
}

void Gfx::CEngine::SetEditIndentMode(bool autoIndent)
{
    m_editIndentMode = autoIndent;
}

bool Gfx::CEngine::GetEditIndentMode()
{
    return m_editIndentMode;
}

void Gfx::CEngine::SetEditIndentValue(int value)
{
    m_editIndentValue = value;
}

int Gfx::CEngine::GetEditIndentValue()
{
    return m_editIndentValue;
}

void Gfx::CEngine::SetSpeed(float speed)
{
    m_speed = speed;
}

float Gfx::CEngine::GetSpeed()
{
    return m_speed;
}

void Gfx::CEngine::SetTracePrecision(float factor)
{
    m_tracePrecision = factor;
}

float Gfx::CEngine::GetTracePrecision()
{
    return m_tracePrecision;
}

void Gfx::CEngine::SetMouseVisible(bool visible)
{
    m_mouseVisible = visible;
}

bool Gfx::CEngine::GetMouseVisible()
{
    return m_mouseVisible;
}

void Gfx::CEngine::SetMousePos(Math::Point pos)
{
    m_mousePos = pos;
}

Math::Point Gfx::CEngine::GetMousePos()
{
    return m_mousePos;
}

void Gfx::CEngine::SetMouseType(Gfx::EngineMouseType type)
{
    m_mouseType = type;
}

Gfx::EngineMouseType Gfx::CEngine::GetMouseType()
{
    return m_mouseType;
}

const Math::Matrix& Gfx::CEngine::GetMatView()
{
    return m_matView;
}

Math::Vector Gfx::CEngine::GetEyePt()
{
    return m_eyePt;
}

Math::Vector Gfx::CEngine::GetLookatPt()
{
    return m_lookatPt;
}

float Gfx::CEngine::GetEyeDirH()
{
    return m_eyeDirH;
}

float Gfx::CEngine::GetEyeDirV()
{
    return m_eyeDirV;
}

bool Gfx::CEngine::IsVisiblePoint(const Math::Vector &pos)
{
    return Math::Distance(m_eyePt, pos) <= m_deepView[0];
}

void Gfx::CEngine::UpdateMatProj()
{
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProj);
}

void Gfx::CEngine::ApplyChange()
{
    m_deepView[0] /= m_lastClippingDistance;
    m_deepView[1] /= m_lastClippingDistance;

    SetFocus(m_focus);
    ChangeLOD();

    m_deepView[0] *= m_clippingDistance;
    m_deepView[1] *= m_clippingDistance;
}



/*******************************************************
                      Rendering
 *******************************************************/



/**
  This function sets up render states, clears the
  viewport, and renders the scene. */
void Gfx::CEngine::Render()
{
    if (! m_render) return;

    m_statisticTriangle = 0;
    m_lastState = -1;
    m_lastColor = 999;
    m_lastMaterial = Gfx::Material();

    m_lightMan->UpdateLights();

    Gfx::Color color;
    if (m_skyMode && m_cloud->GetLevel() != 0.0f)  // clouds?
        color = m_backgroundCloudDown;
    else
        color = m_backgroundColorDown;

    m_device->SetClearColor(color);

    // Begin the scene
    m_device->BeginScene();

    if (m_drawWorld)
        Draw3DScene();

    DrawInterface();

    // End the scene
    m_device->EndScene();
}

void Gfx::CEngine::Draw3DScene()
{
    if (m_groundSpotVisible)
        UpdateGroundSpotTextures();

    DrawBackground();                // draws the background
    if (m_planetMode) DrawPlanet();  // draws the planets
    if (m_skyMode) m_cloud->Draw();  // draws the clouds


    // Display the objects

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, true);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, true);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, true);

    float fogStart = m_deepView[m_rankView]*m_fogStart[m_rankView];
    float fogEnd = m_deepView[m_rankView];
    m_device->SetFogParams(Gfx::FOG_LINEAR, m_fogColor[m_rankView], fogStart, fogEnd, 1.0f);

    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProj);
    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matView);

    if (m_waterMode) m_water->DrawBack();  // draws water background

    if (m_shadowVisible)
    {
        m_lightMan->UpdateLightsEnableState(Gfx::ENG_OBJTYPE_TERRAIN);

        // Draw the terrain

        for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
        {
            Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
            if (! p1.used) continue;

            // Should be loaded by now
            SetTexture(p1.tex1, 0);
            SetTexture(p1.tex2, 1);

            for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
            {
                Gfx::EngineObjLevel2& p2 = p1.next[l2];
                if (! p2.used) continue;

                int objRank = p2.objRank;
                if (m_objects[objRank].type != Gfx::ENG_OBJTYPE_TERRAIN)
                    continue;
                if (! m_objects[objRank].drawWorld)
                    continue;

                m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_objects[objRank].transform);

                if (! IsVisible(objRank))
                    continue;

                for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
                {
                    Gfx::EngineObjLevel3& p3 = p2.next[l3];
                    if (! p3.used) continue;

                    if ( m_objects[objRank].distance <  p3.min ||
                         m_objects[objRank].distance >= p3.max )
                        continue;

                    for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                    {
                        Gfx::EngineObjLevel4& p4 = p3.next[l4];
                        if (! p4.used) continue;

                        SetMaterial(p4.material);
                        SetState(p4.state);

                        if (p4.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
                        {
                            m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLES,
                                                     &p4.vertices[0],
                                                     p4.vertices.size() );
                            m_statisticTriangle += p4.vertices.size() / 3;
                        }
                        if (p4.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
                        {
                            m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLE_STRIP,
                                                     &p4.vertices[0],
                                                     p4.vertices.size() );
                            m_statisticTriangle += p4.vertices.size() - 2;
                        }
                    }
                }
            }
        }

        // Draws the shadows
        DrawShadow();
    }

    // Draw objects (non-terrain)

    bool transparent = false;

    for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
    {
        Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
        if (! p1.used) continue;

        // Should be loaded by now
        SetTexture(p1.tex1, 0);
        SetTexture(p1.tex2, 1);

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            Gfx::EngineObjLevel2& p2 = p1.next[l2];
            if (! p2.used) continue;

            int objRank = p2.objRank;

            if (m_shadowVisible && m_objects[objRank].type == Gfx::ENG_OBJTYPE_TERRAIN)
                continue;

            if (! m_objects[objRank].drawWorld)
                continue;

            m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_objects[objRank].transform);

            if (! IsVisible(objRank))
                continue;

            m_lightMan->UpdateLightsEnableState(m_objects[objRank].type);

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                Gfx::EngineObjLevel3& p3 = p2.next[l3];
                if (! p3.used) continue;

                if ( m_objects[objRank].distance <  p3.min ||
                     m_objects[objRank].distance >= p3.max )  continue;

                for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                {
                    Gfx::EngineObjLevel4& p4 = p3.next[l4];
                    if (! p4.used) continue;

                    if (m_objects[objRank].transparency != 0.0f)  // transparent ?
                    {
                        transparent = true;
                        continue;
                    }

                    SetMaterial(p4.material);
                    SetState(p4.state);

                    if (p4.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
                    {
                        m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLES,
                                                 &p4.vertices[0],
                                                 p4.vertices.size() );

                        m_statisticTriangle += p4.vertices.size() / 3;
                    }
                    else if (p4.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
                    {
                        m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLE_STRIP,
                                                 &p4.vertices[0],
                                                 p4.vertices.size() );

                        m_statisticTriangle += p4.vertices.size() - 2;
                    }
                }
            }
        }
    }

    // Draw transparent objects

    if (transparent)
    {
        int tState = 0;
        Gfx::Color tColor;
        if (m_stateColor)
        {
            tState = Gfx::ENG_RSTATE_TTEXTURE_BLACK | Gfx::ENG_RSTATE_2FACE;
            tColor = Gfx::Color(68.0f / 255.0f, 68.0f / 255.0f, 68.0f / 255.0f, 68.0f / 255.0f);
        }
        else
        {
            tState = Gfx::ENG_RSTATE_TCOLOR_BLACK;
            tColor = Gfx::Color(136.0f / 255.0f, 136.0f / 255.0f, 136.0f / 255.0f, 136.0f / 255.0f);
        }

        for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
        {
            Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
            if (! p1.used) continue;

            // Should be loaded by now
            SetTexture(p1.tex1, 0);
            SetTexture(p1.tex2, 1);

            for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
            {
                Gfx::EngineObjLevel2& p2 = p1.next[l2];
                if (! p2.used) continue;

                int objRank = p2.objRank;

                if (m_shadowVisible && m_objects[objRank].type == Gfx::ENG_OBJTYPE_TERRAIN)
                    continue;

                if (! m_objects[objRank].drawWorld)
                    continue;

                m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_objects[objRank].transform);

                if (! IsVisible(objRank))
                    continue;

                m_lightMan->UpdateLightsEnableState(m_objects[objRank].type);

                for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
                {
                    Gfx::EngineObjLevel3& p3 = p2.next[l3];
                    if (! p3.used) continue;

                    if ( m_objects[objRank].distance <  p3.min ||
                        m_objects[objRank].distance >= p3.max )  continue;

                    for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                    {
                        Gfx::EngineObjLevel4& p4 = p3.next[l4];
                        if (! p4.used) continue;

                        if (m_objects[objRank].transparency == 0.0f)
                            continue;

                        SetMaterial(p4.material);
                        SetState(tState, tColor);

                        if (p4.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
                        {
                            m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLES,
                                                     &p4.vertices[0],
                                                     p4.vertices.size() );

                            m_statisticTriangle += p4.vertices.size() / 3;
                        }
                        else if (p4.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
                        {
                            m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLE_STRIP,
                                                     &p4.vertices[0],
                                                     p4.vertices.size() );
                            m_statisticTriangle += p4.vertices.size() - 2;
                        }
                    }
                }
            }
        }
    }

    m_lightMan->UpdateLightsEnableState(Gfx::ENG_OBJTYPE_TERRAIN);

    if (m_waterMode) m_water->DrawSurf();    // draws water surface

    m_particle->DrawParticle(Gfx::SH_WORLD); // draws the particles of the 3D world
    m_lightning->Draw();                     // draws lightning
    if (m_lensMode) DrawForegroundImage();   // draws the foreground
    if (! m_overFront) DrawOverColor();      // draws the foreground color
}

void Gfx::CEngine::DrawInterface()
{
    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW,       m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD,      m_matWorldInterface);

    // Draw the entire interface
    CInterface* interface = static_cast<CInterface*>( m_iMan->SearchInstance(CLASS_INTERFACE) );
    if (interface != nullptr)
        interface->Draw();

    m_particle->DrawParticle(Gfx::SH_INTERFACE);  // draws the particles of the interface

    // 3D objects drawn in front of interface
    if (m_drawFront)
    {
        // Display the objects
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, true);

        m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProj);

        m_device->SetGlobalAmbient(m_ambientColor[m_rankView]);
        m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, true);

        m_device->SetRenderState(Gfx::RENDER_STATE_FOG, true);

        float fogStart = m_deepView[m_rankView]*m_fogStart[m_rankView];
        float fogEnd = m_deepView[m_rankView];
        m_device->SetFogParams(Gfx::FOG_LINEAR, m_fogColor[m_rankView], fogStart, fogEnd, 1.0f);

        m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matView);

        for (int l1 = 0; l1 < static_cast<int>( m_objectTree.size() ); l1++)
        {
            Gfx::EngineObjLevel1& p1 = m_objectTree[l1];
            if (! p1.used) continue;

            // Should be loaded by now
            SetTexture(p1.tex1, 0);
            SetTexture(p1.tex2, 1);

            for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
            {
                Gfx::EngineObjLevel2& p2 = p1.next[l2];
                if (! p2.used) continue;

                int objRank = p2.objRank;

                if (m_shadowVisible && m_objects[objRank].type == Gfx::ENG_OBJTYPE_TERRAIN)
                    continue;

                if (! m_objects[objRank].drawFront)
                    continue;

                m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_objects[objRank].transform);

                if (! IsVisible(objRank))
                    continue;

                m_lightMan->UpdateLightsEnableState(m_objects[objRank].type);

                for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
                {
                    Gfx::EngineObjLevel3& p3 = p2.next[l3];
                    if (! p3.used) continue;

                    if ( m_objects[objRank].distance <  p3.min ||
                         m_objects[objRank].distance >= p3.max )  continue;

                    for (int l4 = 0; l4 < static_cast<int>( p3.next.size() ); l4++)
                    {
                        Gfx::EngineObjLevel4& p4 = p3.next[l4];
                        if (! p4.used) continue;

                        SetMaterial(p4.material);
                        SetState(p4.state);

                        if (p4.type == Gfx::ENG_TRIANGLE_TYPE_TRIANGLES)
                        {
                            m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLES,
                                                     &p4.vertices[0],
                                                     p4.vertices.size() );

                            m_statisticTriangle += p4.vertices.size() / 3;
                        }
                        else if (p4.type == Gfx::ENG_TRIANGLE_TYPE_SURFACE)
                        {
                            m_device->DrawPrimitive( Gfx::PRIMITIVE_TRIANGLE_STRIP,
                                                     &p4.vertices[0],
                                                     p4.vertices.size() );
                            m_statisticTriangle += p4.vertices.size() - 2;
                        }
                    }
                }
            }
        }

        m_particle->DrawParticle(Gfx::SH_FRONT);  // draws the particles of the 3D world

        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);

        m_device->SetTransform(Gfx::TRANSFORM_VIEW,       m_matViewInterface);
        m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
        m_device->SetTransform(Gfx::TRANSFORM_WORLD,      m_matWorldInterface);
    }

    // Draw foreground color
    if (m_overFront)
        DrawOverColor();

    // Mouse & highlight at the end
    DrawMouse();
    DrawHighlight();
}

void Gfx::CEngine::UpdateGroundSpotTextures()
{
    // TODO the original code modifying the textures is very complex, so stub for now
    GetLogger()->Trace("CEngine::UpdateGroundSpotTextures(): stub!\n");
}

void Gfx::CEngine::DrawShadow()
{
    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);

    Math::Matrix matrix;
    matrix.LoadIdentity();
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, matrix);


    Gfx::Material material;
    material.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);
    material.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);
    SetMaterial(material);

    // TODO: create a separate texture
    SetTexture("text.png");

    Math::Point ts, ti;

    float dp = 0.5f/256.0f;
    ts.y = 192.0f/256.0f;
    ti.y = 224.0f/256.0f;
    ts.y += dp;
    ti.y -= dp;

    Math::Vector n(0.0f, 1.0f, 0.0f);

    float startDeepView = m_deepView[m_rankView]*m_fogStart[m_rankView];
    float endDeepView = m_deepView[m_rankView];

    float lastIntensity = -1.0f;
    for (int i = 0; i < static_cast<int>( m_shadows.size() ); i++)
    {
        if (m_shadows[i].hide) continue;

        Math::Vector pos = m_shadows[i].pos;  // pos = center of the shadow on the ground

        if (m_eyePt.y == pos.y)  continue;  // camera at the same level?

        float d = 0.0f;
        float D = 0.0f;

        // h is the height above the ground to which the shadow
        // will be drawn.
        if (m_eyePt.y > pos.y)  // camera on?
        {
            float height = m_eyePt.y-pos.y;
            float h = m_shadows[i].radius;
            float max = height*0.5f;
            if ( h > max  )  h = max;
            if ( h > 4.0f )  h = 4.0f;

            D = Math::Distance(m_eyePt, pos);
            if ( D >= endDeepView )  continue;
            d = D*h/height;

            pos.x += (m_eyePt.x-pos.x)*d/D;
            pos.z += (m_eyePt.z-pos.z)*d/D;
            pos.y += h;
        }
        else    // camera underneath?
        {
            float height = pos.y-m_eyePt.y;
            float h = m_shadows[i].radius;
            float max = height*0.1f;
            if ( h > max  )  h = max;
            if ( h > 4.0f )  h = 4.0f;

            D = Math::Distance(m_eyePt, pos);
            if ( D >= endDeepView )  continue;
            d = D*h/height;

            pos.x += (m_eyePt.x-pos.x)*d/D;
            pos.z += (m_eyePt.z-pos.z)*d/D;
            pos.y -= h;
        }

        // The hFactor decreases the intensity and size increases more
        // the object is high relative to the ground.
        float hFactor = m_shadows[i].height/20.0f;
        if ( hFactor < 0.0f )  hFactor = 0.0f;
        if ( hFactor > 1.0f )  hFactor = 1.0f;
        hFactor = powf(1.0f-hFactor, 2.0f);
        if ( hFactor < 0.2f )  hFactor = 0.2f;

        float radius = m_shadows[i].radius*1.5f;
        radius *= 2.0f-hFactor;  // greater if high
        radius *= 1.0f-d/D;  // smaller if close


        Math::Vector corner[4];

        if (m_shadows[i].type == Gfx::ENG_SHADOW_NORM)
        {
            corner[0].x = +radius;
            corner[0].z = +radius;
            corner[0].y = 0.0f;

            corner[1].x = -radius;
            corner[1].z = +radius;
            corner[1].y = 0.0f;

            corner[2].x = +radius;
            corner[2].z = -radius;
            corner[2].y = 0.0f;

            corner[3].x = -radius;
            corner[3].z = -radius;
            corner[3].y = 0.0f;

            ts.x =  64.0f/256.0f;
            ti.x =  96.0f/256.0f;
        }
        else
        {
            Math::Point rot;

            rot = Math::RotatePoint(-m_shadows[i].angle, Math::Point(radius, radius));
            corner[0].x = rot.x;
            corner[0].z = rot.y;
            corner[0].y = 0.0f;

            rot = Math::RotatePoint(-m_shadows[i].angle, Math::Point(-radius, radius));
            corner[1].x = rot.x;
            corner[1].z = rot.y;
            corner[1].y = 0.0f;

            rot = Math::RotatePoint(-m_shadows[i].angle, Math::Point(radius, -radius));
            corner[2].x = rot.x;
            corner[2].z = rot.y;
            corner[2].y = 0.0f;

            rot = Math::RotatePoint(-m_shadows[i].angle, Math::Point(-radius, -radius));
            corner[3].x = rot.x;
            corner[3].z = rot.y;
            corner[3].y = 0.0f;

            if (m_shadows[i].type == Gfx::ENG_SHADOW_WORM)
            {
                ts.x =  96.0f/256.0f;
                ti.x = 128.0f/256.0f;
            }
            else
            {
                ts.x =  64.0f/256.0f;
                ti.x =  96.0f/256.0f;
            }
        }

        corner[0] = Math::CrossProduct(corner[0], m_shadows[i].normal);
        corner[1] = Math::CrossProduct(corner[1], m_shadows[i].normal);
        corner[2] = Math::CrossProduct(corner[2], m_shadows[i].normal);
        corner[3] = Math::CrossProduct(corner[3], m_shadows[i].normal);

        corner[0] += pos;
        corner[1] += pos;
        corner[2] += pos;
        corner[3] += pos;

        ts.x += dp;
        ti.x -= dp;

        Gfx::Vertex vertex[4] =
        {
            Gfx::Vertex(corner[1], n, Math::Point(ts.x, ts.y)),
            Gfx::Vertex(corner[0], n, Math::Point(ti.x, ts.y)),
            Gfx::Vertex(corner[3], n, Math::Point(ts.x, ti.y)),
            Gfx::Vertex(corner[2], n, Math::Point(ti.x, ti.y))
        };

        float intensity = (0.5f+m_shadows[i].intensity*0.5f)*hFactor;

        // Decreases the intensity of the shade if you're in the area
        // between the beginning and the end of the fog.
        if ( D > startDeepView )
            intensity *= 1.0f-(D-startDeepView)/(endDeepView-startDeepView);

        if (intensity == 0.0f)  continue;

        if (lastIntensity != intensity)  // intensity changed?
        {
            lastIntensity = intensity;
            SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE, Gfx::Color(intensity, intensity, intensity, intensity));
        }

        m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
        AddStatisticTriangle(2);
    }

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, true);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, true);
}

// STATUS: TESTED, VERIFIED
void Gfx::CEngine::DrawBackground()
{
    if (m_skyMode && m_cloud->GetLevel() != 0.0f)  // clouds ?
    {
        if (m_backgroundCloudUp != m_backgroundCloudDown)  // degraded?
            DrawBackgroundGradient(m_backgroundCloudUp, m_backgroundCloudDown);
    }
    else
    {
        if (m_backgroundColorUp != m_backgroundColorDown)  // degraded?
            DrawBackgroundGradient(m_backgroundColorUp, m_backgroundColorDown);
    }

    if (m_backForce || (m_skyMode && !m_backgroundName.empty()) )
    {
        DrawBackgroundImage();  // image
    }
}

// STATUS: TESTED
void Gfx::CEngine::DrawBackgroundGradient(const Gfx::Color& up, const Gfx::Color& down)
{
    Math::Point p1(0.0f, 0.5f);
    Math::Point p2(1.0f, 1.0f);

    Gfx::Color color[3] =
    {
        up,
        down,
        Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)
    };

    SetState(Gfx::ENG_RSTATE_OPAQUE_COLOR);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    Gfx::VertexCol vertex[4] =
    {
        Gfx::VertexCol(Math::Vector(p1.x, p1.y, 0.0f), color[1], color[2]),
        Gfx::VertexCol(Math::Vector(p1.x, p2.y, 0.0f), color[0], color[2]),
        Gfx::VertexCol(Math::Vector(p2.x, p1.y, 0.0f), color[1], color[2]),
        Gfx::VertexCol(Math::Vector(p2.x, p2.y, 0.0f), color[0], color[2])
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

// Status: PART_TESTED
void Gfx::CEngine::DrawBackgroundImageQuarter(Math::Point p1, Math::Point p2, const Gfx::Texture &tex)
{
    Math::Vector n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    float u1, u2, v1, v2;
    if (m_backgroundFull)
    {
        u1 = 0.0f;
        v1 = 0.0f;
        u2 = 1.0f;
        v2 = 1.0f;

        if (m_backgroundQuarter)
        {
            u1 += 0.5f/512.0f;
            v1 += 0.5f/384.0f;
            u2 -= 0.5f/512.0f;
            v2 -= 0.5f/384.0f;
        }
    }
    else
    {
        float h = 0.5f;  // visible area vertically (1=all)
        float a = m_eyeDirV-Math::PI*0.15f;
        if (a >  Math::PI     )  a -= Math::PI*2.0f;  // a = -Math::PI..Math::PI
        if (a >  Math::PI/4.0f)  a =  Math::PI/4.0f;
        if (a < -Math::PI/4.0f)  a = -Math::PI/4.0f;

        u1 = -m_eyeDirH/Math::PI;
        u2 = u1+1.0f/Math::PI;

        v1 = (1.0f-h)*(0.5f+a/(2.0f*Math::PI/4.0f))+0.1f;
        v2 = v1+h;
    }

    SetTexture(tex);
    SetState(Gfx::ENG_RSTATE_OPAQUE_TEXTURE | Gfx::ENG_RSTATE_WRAP);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    Gfx::Vertex vertex[4] =
    {
        Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2)),
        Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1)),
        Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2)),
        Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1))
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

// Status: TESTED, VERIFIED
void Gfx::CEngine::DrawBackgroundImage()
{
    Math::Point p1, p2;
    std::string name;

    if (m_backgroundQuarter)
    {
        p1.x = 0.0f;
        p1.y = 0.5f;
        p2.x = 0.5f;
        p2.y = 1.0f;
        DrawBackgroundImageQuarter(p1, p2, m_backgroundQuarterTexs[0]);

        p1.x = 0.5f;
        p1.y = 0.5f;
        p2.x = 1.0f;
        p2.y = 1.0f;
        DrawBackgroundImageQuarter(p1, p2, m_backgroundQuarterTexs[1]);

        p1.x = 0.0f;
        p1.y = 0.0f;
        p2.x = 0.5f;
        p2.y = 0.5f;
        DrawBackgroundImageQuarter(p1, p2, m_backgroundQuarterTexs[2]);

        p1.x = 0.5f;
        p1.y = 0.0f;
        p2.x = 1.0f;
        p2.y = 0.5f;
        DrawBackgroundImageQuarter(p1, p2, m_backgroundQuarterTexs[3]);
    }
    else
    {
        p1.x = 0.0f;
        p1.y = 0.0f;
        p2.x = 1.0f;
        p2.y = 1.0f;
        DrawBackgroundImageQuarter(p1, p2, m_backgroundFullTex);
    }
}

void Gfx::CEngine::DrawPlanet()
{
    if (! m_planet->PlanetExist()) return;

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    m_planet->Draw();  // draws the planets
}

// Status: PART_TESTED
void Gfx::CEngine::DrawForegroundImage()
{
    if (m_foregroundName.empty()) return;

    Math::Vector n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    Math::Point p1(0.0f, 0.0f);
    Math::Point p2(1.0f, 1.0f);

    float u1 = -m_eyeDirH/(Math::PI*0.6f)+Math::PI*0.5f;
    float u2 = u1+0.50f;

    float v1 = 0.2f;
    float v2 = 1.0f;


    Gfx::Vertex vertex[4] =
    {
        Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2)),
        Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1)),
        Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2)),
        Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1))
    };

    SetTexture(m_foregroundTex);
    SetState(Gfx::ENG_RSTATE_CLAMP | Gfx::ENG_RSTATE_TTEXTURE_BLACK);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

// Status: PART_TESTED
void Gfx::CEngine::DrawOverColor()
{
    if (! m_stateColor) return;

    // TODO: fuzzy compare?
    if ( (m_overColor == Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f) && m_overMode == Gfx::ENG_RSTATE_TCOLOR_BLACK) ||
         (m_overColor == Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f) && m_overMode == Gfx::ENG_RSTATE_TCOLOR_WHITE) )  return;

    Math::Point p1(0.0f, 0.0f);
    Math::Point p2(1.0f, 1.0f);

    Gfx::Color color[3] =
    {
        m_overColor,
        m_overColor,
        Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)
    };

    SetState(m_overMode);

    // TODO: set also with m_overMode ?
    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, false);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    Gfx::VertexCol vertex[4] =
    {
        Gfx::VertexCol(Math::Vector(p1.x, p1.y, 0.0f), color[1], color[2]),
        Gfx::VertexCol(Math::Vector(p1.x, p2.y, 0.0f), color[0], color[2]),
        Gfx::VertexCol(Math::Vector(p2.x, p1.y, 0.0f), color[1], color[2]),
        Gfx::VertexCol(Math::Vector(p2.x, p2.y, 0.0f), color[0], color[2])
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

// Status: TESTED, VERIFIED
void Gfx::CEngine::DrawHighlight()
{
    Math::Point min, max;
    min.x =  1000000.0f;
    min.y =  1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    int i = 0;
    while (m_highlightRank[i] != -1)
    {
        Math::Point omin, omax;
        if (GetBBox2D(m_highlightRank[i++], omin, omax))
        {
            min.x = Math::Min(min.x, omin.x);
            min.y = Math::Min(min.y, omin.y);
            max.x = Math::Max(max.x, omax.x);
            max.y = Math::Max(max.y, omax.y);
        }
    }

    if ( min.x ==  1000000.0f ||
         min.y ==  1000000.0f ||
         max.x == -1000000.0f ||
         max.y == -1000000.0f )
    {
        m_highlight = false;  // not highlighted
    }
    else
    {
        m_highlightP1 = min;
        m_highlightP2 = max;
        m_highlight = true;
    }

    if (! m_highlight)
        return;

    Math::Point p1 = m_highlightP1;
    Math::Point p2 = m_highlightP2;

    int nbOut = 0;
    if (p1.x < 0.0f || p1.x > 1.0f) nbOut++;
    if (p1.y < 0.0f || p1.y > 1.0f) nbOut++;
    if (p2.x < 0.0f || p2.x > 1.0f) nbOut++;
    if (p2.y < 0.0f || p2.y > 1.0f) nbOut++;
    if (nbOut > 2)
        return;

    SetState(Gfx::ENG_RSTATE_OPAQUE_COLOR);

    float d = 0.5f+sinf(m_highlightTime*6.0f)*0.5f;
    d *= (p2.x-p1.x)*0.1f;
    p1.x += d;
    p1.y += d;
    p2.x -= d;
    p2.y -= d;

    Gfx::Color color(1.0f, 1.0f, 0.0f);  // yellow

    Gfx::VertexCol line[3] =
    {
        Gfx::VertexCol(Math::Vector(), color),
        Gfx::VertexCol(Math::Vector(), color),
        Gfx::VertexCol(Math::Vector(), color)
    };

    float dx = (p2.x - p1.x) / 5.0f;
    float dy = (p2.y - p1.y) / 5.0f;

    line[0].coord = Math::Vector(p1.x, p1.y + dy, 0.0f);
    line[1].coord = Math::Vector(p1.x, p1.y, 0.0f);
    line[2].coord = Math::Vector(p1.x + dx, p1.y, 0.0f);
    m_device->DrawPrimitive(Gfx::PRIMITIVE_LINE_STRIP, line, 3);

    line[0].coord = Math::Vector(p2.x - dx, p1.y, 0.0f);
    line[1].coord = Math::Vector(p2.x, p1.y, 0.0f);
    line[2].coord = Math::Vector(p2.x, p1.y + dy, 0.0f);
    m_device->DrawPrimitive(Gfx::PRIMITIVE_LINE_STRIP, line, 3);

    line[0].coord = Math::Vector(p2.x, p2.y - dy, 0.0f);
    line[1].coord = Math::Vector(p2.x, p2.y, 0.0f);
    line[2].coord = Math::Vector(p2.x - dx, p2.y, 0.0f);
    m_device->DrawPrimitive(Gfx::PRIMITIVE_LINE_STRIP, line, 3);

    line[0].coord = Math::Vector(p1.x + dx, p2.y, 0.0f);
    line[1].coord = Math::Vector(p1.x, p2.y, 0.0f);
    line[2].coord = Math::Vector(p1.x, p2.y - dy, 0.0f);
    m_device->DrawPrimitive(Gfx::PRIMITIVE_LINE_STRIP, line, 3);
}

// Status: TESTED, VERIFIED
void Gfx::CEngine::DrawMouse()
{
    if (! m_mouseVisible)
        return;

    if (m_app->GetSystemMouseVisibile())
        return;

    Gfx::Material material;
    material.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);
    material.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);

    m_device->SetMaterial(material);
    m_device->SetTexture(0, m_miceTexture);

    int index = static_cast<int>(m_mouseType);

    Math::Point pos = m_mousePos;
    pos.x = m_mousePos.x - (m_mice[index].hotPoint.x * m_mouseSize.x) / 32.0f;
    pos.y = m_mousePos.y - ((32.0f - m_mice[index].hotPoint.y) * m_mouseSize.y) / 32.0f;

    Math::Point shadowPos;
    shadowPos.x = pos.x + (4.0f/800.0f);
    shadowPos.y = pos.y - (3.0f/600.0f);

    SetState(Gfx::ENG_RSTATE_TCOLOR_WHITE);
    DrawMouseSprite(shadowPos, m_mouseSize, m_mice[index].iconShadow);

    SetState(m_mice[index].mode1);
    DrawMouseSprite(pos, m_mouseSize, m_mice[index].icon1);

    SetState(m_mice[index].mode2);
    DrawMouseSprite(pos, m_mouseSize, m_mice[index].icon2);
}

// Status: TESTED, VERIFIED
void Gfx::CEngine::DrawMouseSprite(Math::Point pos, Math::Point size, int icon)
{
    if (icon == -1)
        return;

    Math::Point p1 = pos;
    Math::Point p2 = p1 + size;

    float u1 = (32.0f / 256.0f) * (icon % 8);
    float v1 = (32.0f / 256.0f) * (icon / 8);
    float u2 = u1 + (32.0f / 256.0f);
    float v2 = v1 + (32.0f / 256.0f);

    float dp = 0.5f / 256.0f;
    u1 += dp;
    v1 += dp;
    u2 -= dp;
    v2 -= dp;

    Math::Vector normal(0.0f, 0.0f, -1.0f);

    Gfx::Vertex vertex[4] =
    {
        Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), normal, Math::Point(u1, v2)),
        Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), normal, Math::Point(u1, v1)),
        Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), normal, Math::Point(u2, v2)),
        Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), normal, Math::Point(u2, v1))
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}
