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
const int LEVEL4_PREALLOCATE_COUNT        = 10;
const int LEVEL5_PREALLOCATE_COUNT        = 100;
const int LEVEL5_VERTEX_PREALLOCATE_COUNT = 200;


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
    m_baseTime = 0;
    m_lastTime = 0;
    m_absTime = 0.0f;
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
    m_backForce = true;
    m_planetMode = true;
    m_lightMode = true;
    m_editIndentMode = true;
    m_editIndentValue = 4;
    m_tracePrecision = 1.0f;

    m_alphaMode = 1;

    m_forceStateColor = true;
    m_stateColor = false;

    m_blackSrcBlend[0]    = 0;
    m_blackDestBlend[0]   = 0;
    m_whiteSrcBlend[0]    = 0;
    m_whiteDestBlend[0]   = 0;
    m_diffuseSrcBlend[0]  = 0;
    m_diffuseDestBlend[0] = 0;
    m_alphaSrcBlend[0]    = 0;
    m_alphaDestBlend[0]   = 0;

    m_updateGeometry = false;

    m_mice[Gfx::ENG_MOUSE_NORM]    = Gfx::EngineMouse( 0,  1, 32, Gfx::ENG_RSTATE_TCOLOR_WHITE, Gfx::ENG_RSTATE_TCOLOR_BLACK, Math::Point( 1.0f,  1.0f));
    m_mice[Gfx::ENG_MOUSE_WAIT]    = Gfx::EngineMouse( 2,  3, 33, Gfx::ENG_RSTATE_TCOLOR_WHITE, Gfx::ENG_RSTATE_TCOLOR_BLACK, Math::Point( 8.0f, 12.0f));
    m_mice[Gfx::ENG_MOUSE_HAND]    = Gfx::EngineMouse( 4,  5, 34, Gfx::ENG_RSTATE_TCOLOR_WHITE, Gfx::ENG_RSTATE_TCOLOR_BLACK, Math::Point( 7.0f,  2.0f));
    m_mice[Gfx::ENG_MOUSE_NO]      = Gfx::EngineMouse( 6,  7, 35, Gfx::ENG_RSTATE_TCOLOR_WHITE, Gfx::ENG_RSTATE_TCOLOR_BLACK, Math::Point(10.0f, 10.0f));
    m_mice[Gfx::ENG_MOUSE_EDIT]    = Gfx::EngineMouse( 8,  9, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 6.0f, 10.0f));
    m_mice[Gfx::ENG_MOUSE_CROSS]   = Gfx::EngineMouse(10, 11, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point(10.0f, 10.0f));
    m_mice[Gfx::ENG_MOUSE_MOVEV]   = Gfx::EngineMouse(12, 13, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 5.0f, 11.0f));
    m_mice[Gfx::ENG_MOUSE_MOVEH]   = Gfx::EngineMouse(14, 15, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point(11.0f,  5.0f));
    m_mice[Gfx::ENG_MOUSE_MOVED]   = Gfx::EngineMouse(16, 17, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 9.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_MOVEI]   = Gfx::EngineMouse(18, 19, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 9.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_MOVE]    = Gfx::EngineMouse(20, 21, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point(11.0f, 11.0f));
    m_mice[Gfx::ENG_MOUSE_TARGET]  = Gfx::EngineMouse(22, 23, -1, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point(15.0f, 15.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLL] = Gfx::EngineMouse(24, 25, 43, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 2.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLR] = Gfx::EngineMouse(26, 27, 44, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point(17.0f,  9.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLU] = Gfx::EngineMouse(28, 29, 45, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 9.0f,  2.0f));
    m_mice[Gfx::ENG_MOUSE_SCROLLD] = Gfx::EngineMouse(30, 31, 46, Gfx::ENG_RSTATE_TCOLOR_BLACK, Gfx::ENG_RSTATE_TCOLOR_WHITE, Math::Point( 9.0f, 17.0f));

    m_mouseSize    = Math::Point(0.04f, 0.04f * (800.0f / 600.0f));
    m_mousePos     = Math::Point(0.5f, 0.5f);
    m_mouseType    = Gfx::ENG_MOUSE_NORM;
    m_mouseVisible = false;

    m_texPath = "textures/";
    m_defaultTexParams.format = Gfx::TEX_IMG_RGBA;
    m_defaultTexParams.mipmap = true;
    m_defaultTexParams.minFilter = Gfx::TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR;
    m_defaultTexParams.magFilter = Gfx::TEX_MAG_FILTER_LINEAR;

    m_objectTree.reserve(LEVEL1_PREALLOCATE_COUNT);
    m_objects.reserve(OBJECT_PREALLOCATE_COUNT);
    m_shadow.reserve(SHADOW_PREALLOCATE_COUNT);
    m_groundSpot.reserve(GROUNDSPOT_PREALLOCATE_COUNT);
}

Gfx::CEngine::~CEngine()
{
    m_iMan    = nullptr;
    m_app     = nullptr;
    m_device  = nullptr;
    m_sound   = nullptr;
    m_terrain = nullptr;
}

std::string Gfx::CEngine::GetError()
{
    return m_error;
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

    m_text->SetDevice(m_device);
    if (! m_text->Create())
    {
        m_error = std::string("Error creating CText: ") + m_text->GetError();
        return false;
    }

    m_device->SetClearColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));
    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, false);
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
    m_miceTexture = CreateTexture("mouse.png", params);

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
    if (event.type == EVENT_MOUSE_MOVE)
    {
        m_mousePos = event.mouseMove.pos;
    }
    else if (event.type == EVENT_KEY_DOWN)
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

void Gfx::CEngine::TimeInit()
{
    /* TODO!
    m_baseTime = timeGetTime();
    m_lastTime = 0;
    m_absTime = 0.0f;*/
}

void Gfx::CEngine::TimeEnterGel()
{
    /* TODO!
    m_stopTime = timeGetTime();*/
}

void Gfx::CEngine::TimeExitGel()
{
    /* TODO!
    m_baseTime += timeGetTime() - m_stopTime;*/
}

float Gfx::CEngine::TimeGet()
{
    /* TODO!
    float aTime = (timeGetTime()-m_baseTime)*0.001f;  // in ms
    float rTime = (aTime - m_lastTime)*m_speed;
    m_absTime += rTime;
    m_lastTime = aTime;

    return rTime;*/
    return 0.0f;
}

bool Gfx::CEngine::WriteScreenShot(const std::string& fileName, int width, int height)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::ReadSettings()
{
    // TODO!
    return true;
}

bool Gfx::CEngine::WriteSettings()
{
    // TODO!
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

Math::IntSize Gfx::CEngine::GetWindowSize()
{
    return m_size;
}

Math::IntSize Gfx::CEngine::GetLastWindowSize()
{
    return m_lastSize;
}

Math::Point Gfx::CEngine::WindowToInterfaceCoords(Math::IntPoint pos)
{
    return Math::Point(        static_cast<float>(pos.x) / static_cast<float>(m_size.w),
                        1.0f - static_cast<float>(pos.y) / static_cast<float>(m_size.h)  );
}

Math::IntPoint Gfx::CEngine::InterfaceToWindowCoords(Math::Point pos)
{
    return Math::IntPoint(static_cast<int>(pos.x * m_size.w),
                          static_cast<int>((1.0f - pos.y) * m_size.h));
}

Math::Size Gfx::CEngine::WindowToInterfaceSize(Math::IntSize size)
{
    return Math::Size( static_cast<float>(size.w) / static_cast<float>(m_size.w),
                       static_cast<float>(size.h) / static_cast<float>(m_size.h)  );
}

Math::IntSize Gfx::CEngine::InterfaceToWindowSize(Math::Size size)
{
    return Math::IntSize(static_cast<int>(size.w * m_size.w),
                         static_cast<int>(size.h * m_size.h));
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
    // TODO!
    return 0;
}

void Gfx::CEngine::FlushObject()
{
    // TODO!
}

bool Gfx::CEngine::DeleteObject(int objRank)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetDrawWorld(int objRank, bool draw)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetDrawFront(int objRank, bool draw)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::AddTriangle(int objRank, Gfx::VertexTex2* vertex, int nb,
                               const Gfx::Material& mat, int state,
                               std::string texName1, std::string texName2,
                               float min, float max, bool globalUpdate)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::AddSurface(int objRank, Gfx::VertexTex2* vertex, int nb,
                              const Gfx::Material& mat, int state,
                              std::string texName1, std::string texName2,
                              float min, float max, bool globalUpdate)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::AddQuick(int objRank, const Gfx::EngineObjLevel5& buffer,
                            std::string texName1, std::string texName2,
                            float min, float max, bool globalUpdate)
{
    // TODO!
    return true;
}

Gfx::EngineObjLevel5* Gfx::CEngine::SearchTriangle(int objRank, const Gfx::Material& mat,
                                                   int state, std::string texName1,
                                                   std::string texName2, float min, float max)
{
    // TODO!
    return nullptr;
}

void Gfx::CEngine::ChangeLOD()
{
    // TODO!
}

bool Gfx::CEngine::ChangeSecondTexture(int objRank, const std::string& texName2)
{
    // TODO!
    return true;
}

int Gfx::CEngine::GetTotalTriangles(int objRank)
{
    // TODO!
    return 0;
}

int Gfx::CEngine::GetTriangles(int objRank, float min, float max, Gfx::EngineTriangle* buffer, int size, float percent)
{
    // TODO!
    return 0;
}

bool Gfx::CEngine::GetBBox(int objRank, Math::Vector& min, Math::Vector& max)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::ChangeTextureMapping(int objRank, const Gfx::Material& mat, int state,
                                         const std::string& texName1, const std::string& texName2,
                                         float min, float max, Gfx::EngineTextureMapping mode,
                                         float au, float bu, float av, float bv)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::TrackTextureMapping(int objRank, const Gfx::Material& mat, int state,
                                        const std::string& texName1, const std::string& texName2,
                                        float min, float max, Gfx::EngineTextureMapping mode,
                                        float pos, float factor, float tl, float ts, float tt)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectTransform(int objRank, const Math::Matrix& transform)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::GetObjectTransform(int objRank, Math::Matrix& transform)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectType(int objRank, Gfx::EngineObjectType type)
{
    // TODO!
    return true;
}

Gfx::EngineObjectType Gfx::CEngine::GetObjectType(int objRank)
{
    // TODO!
    return Gfx::ENG_OBJTYPE_FIX;
}

bool Gfx::CEngine::SetObjectTransparency(int objRank, float value)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::ShadowCreate(int objRank)
{
    // TODO!
    return true;
}

void Gfx::CEngine::ShadowDelete(int objRank)
{
    // TODO!
}

bool Gfx::CEngine::SetObjectShadowHide(int objRank, bool hide)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowType(int objRank, Gfx::EngineShadowType type)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowPos(int objRank, const Math::Vector& pos)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowNormal(int objRank, const Math::Vector& n)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowAngle(int objRank, float angle)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowRadius(int objRank, float radius)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowIntensity(int objRank, float intensity)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectShadowHeight(int objRank, float h)
{
    // TODO!
    return true;
}

float Gfx::CEngine::GetObjectShadowRadius(int objRank)
{
    // TODO!
    return 0.0f;
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

void Gfx::CEngine::GroundSpotFlush()
{
    // TODO
}

int Gfx::CEngine::GroundSpotCreate()
{
    // TODO!
    return 0;
}

void Gfx::CEngine::GroundSpotDelete(int rank)
{
    // TODO!
}

bool Gfx::CEngine::SetObjectGroundSpotPos(int rank, const Math::Vector& pos)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotRadius(int rank, float radius)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotColor(int rank, const Gfx::Color& color)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotMinMax(int rank, float min, float max)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetObjectGroundSpotSmooth(int rank, float smooth)
{
    // TODO!
    return true;
}

int Gfx::CEngine::GroundMarkCreate(Math::Vector pos, float radius,
                                   float delay1, float delay2, float delay3,
                                   int dx, int dy, char* table)
{
    // TODO!
    return 0;
}

bool Gfx::CEngine::GroundMarkDelete(int rank)
{
    // TODO!
    return true;
}

void Gfx::CEngine::ComputeDistance()
{
    // TODO!
}

void Gfx::CEngine::UpdateGeometry()
{
    // TODO!
}

void Gfx::CEngine::Update()
{
    ComputeDistance();
    UpdateGeometry();
}

bool Gfx::CEngine::DetectBBox(int objRank, Math::Point mouse)
{
    // TODO!
    return true;
}

int Gfx::CEngine::DetectObject(Math::Point mouse)
{
    // TODO!
    return 0;
}

bool Gfx::CEngine::DetectTriangle(Math::Point mouse, Gfx::VertexTex2* triangle, int objRank, float& dist)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::IsVisible(int objRank)
{
    // TODO!
    return true;
}

bool Gfx::CEngine::TransformPoint(Math::Vector& p2D, int objRank, Math::Vector p3D)
{
    // TODO!
    return true;
}



/*******************************************************
                    Mode setting
 *******************************************************/



void Gfx::CEngine::SetState(int state, const Gfx::Color& color)
{
    if ( state == m_lastState && color == m_lastColor )
        return;

    m_lastState = state;
    m_lastColor = color;

    if ( m_alphaMode != 1 && (state & Gfx::ENG_RSTATE_ALPHA) )
    {
        state &= ~Gfx::ENG_RSTATE_ALPHA;

        if (m_alphaMode == 2)
            state |= Gfx::ENG_RSTATE_TTEXTURE_BLACK;
    }

    // TODO other modes & thorough testing

    if (state & Gfx::ENG_RSTATE_TTEXTURE_BLACK)  // The transparent black texture?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        m_device->SetBlendFunc(Gfx::BLEND_ONE, Gfx::BLEND_INV_SRC_COLOR);
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureFactor(color);

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_FACTOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_MODULATE;
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & Gfx::ENG_RSTATE_TTEXTURE_WHITE)  // The transparent white texture?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        m_device->SetBlendFunc(Gfx::BLEND_DST_COLOR, Gfx::BLEND_ZERO);
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureFactor(color.Inverse());

        Gfx::TextureStageParams params;
        params.colorOperation = Gfx::TEX_MIX_OPER_ADD;
        params.colorArg1 = Gfx::TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = Gfx::TEX_MIX_ARG_FACTOR;
        params.alphaOperation = Gfx::TEX_MIX_OPER_MODULATE;
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & Gfx::ENG_RSTATE_TCOLOR_BLACK)  // The transparent black color?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING, true);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, true);

        m_device->SetBlendFunc(Gfx::BLEND_ONE, Gfx::BLEND_INV_SRC_COLOR);

        m_device->SetTextureFactor(color);
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, Gfx::TextureStageParams());
    }
    else if (state & Gfx::ENG_RSTATE_TCOLOR_WHITE)  // The transparent white color?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING, true);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, true);

        m_device->SetBlendFunc(Gfx::BLEND_DST_COLOR, Gfx::BLEND_ZERO);

        m_device->SetTextureFactor(color.Inverse());
        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, Gfx::TextureStageParams());
    }
    else if (state & Gfx::ENG_RSTATE_TDIFFUSE)  // diffuse color as transparent?
    {
        /*m_device->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);
        m_device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
        m_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
        m_device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, false);
        m_device->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_diffuseSrcBlend[1]);
        m_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_diffuseDestBlend[1]);

        m_device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);*/
    }
    else if (state & Gfx::ENG_RSTATE_ALPHA)  // image with alpha channel?
    {
        /*m_device->SetRenderState(D3DRENDERSTATE_FOGENABLE, true);
        m_device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, true);
        m_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
        m_device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, true);
        m_device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
        m_device->SetRenderState(D3DRENDERSTATE_ALPHAREF,  (DWORD)(128));
        m_device->SetRenderState(D3DRENDERSTATE_SRCBLEND,  m_alphaSrcBlend[1]);
        m_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, m_alphaSrcBlend[1]);

        m_device->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, color);
        m_device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
        m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);*/
    }
    else if (state & Gfx::ENG_RSTATE_TEXT)
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    true);

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, Gfx::TextureStageParams());

        m_device->SetBlendFunc(Gfx::BLEND_SRC_ALPHA, Gfx::BLEND_INV_SRC_ALPHA);
    }
    else    // normal ?
    {
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG,         true);
        m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, true);
        m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING,    false);
        m_device->SetRenderState(Gfx::RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING,   true);

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, Gfx::TextureStageParams());

        /*m_device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);*/
    }

    if (state & Gfx::ENG_RSTATE_FOG)
        m_device->SetRenderState(Gfx::RENDER_STATE_FOG, true);


    bool second = m_groundSpotVisible || m_dirty;

    if ( !m_groundSpotVisible && (state & Gfx::ENG_RSTATE_SECOND) != 0 ) second = false;
    if ( !m_dirty             && (state & Gfx::ENG_RSTATE_SECOND) == 0 ) second = false;

    if ( (state & ENG_RSTATE_DUAL_BLACK) && second )
    {
        /*m_device->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        m_device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_device->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
        m_device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);*/
    }
    else if ( (state & ENG_RSTATE_DUAL_WHITE) && second )
    {
        /*m_device->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_ADD);
        m_device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_device->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
        m_device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);*/
    }
    else
    {
        m_device->SetTextureEnabled(1, false);
    }

    if (state & Gfx::ENG_RSTATE_WRAP)
    {
        /*m_device->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
        m_device->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);*/
    }
    else if (state & Gfx::ENG_RSTATE_CLAMP)
    {
        /*m_device->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
        m_device->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);*/
    }
    else
    {
        /*m_device->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
        m_device->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);*/
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

void Gfx::CEngine::SetMaterial(const Gfx::Material &mat)
{
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

Gfx::Texture Gfx::CEngine::CreateTexture(const std::string &texName, const Gfx::TextureCreateParams &params)
{
    CImage img;
    if (! img.Load(m_app->GetDataFilePath(m_texPath, texName)))
    {
        std::stringstream str;
        str << "Couldn't load texture '" << texName << "': " << img.GetError();
        m_error = str.str();
        return Gfx::Texture(); // invalid texture
    }

    Gfx::Texture result = m_device->CreateTexture(&img, params);

    if (! result.valid)
    {
        std::stringstream str;
        str << "Couldn't load texture '" << texName << "': " << m_device->GetError();
        m_error = str.str();
        return result;
    }

    m_texNameMap[texName] = result;
    m_revTexNameMap[result] = texName;

    return result;
}

Gfx::Texture Gfx::CEngine::CreateTexture(const std::string &texName)
{
    return CreateTexture(texName, m_defaultTexParams);
}

void Gfx::CEngine::DestroyTexture(const std::string &texName)
{
    std::map<std::string, Gfx::Texture>::iterator it = m_texNameMap.find(texName);
    if (it == m_texNameMap.end())
        return;

    std::map<Gfx::Texture, std::string>::iterator revIt = m_revTexNameMap.find((*it).second);

    m_device->DestroyTexture((*it).second);

    m_revTexNameMap.erase(revIt);
    m_texNameMap.erase(it);
}

bool Gfx::CEngine::LoadTexture(const std::string& name, int stage)
{
    std::map<std::string, Gfx::Texture>::iterator it = m_texNameMap.find(name);
    if (it != m_texNameMap.end())
    {
        m_device->SetTexture(stage, (*it).second);
        return true;
    }

    // TODO if not present...
    return false;
}

bool Gfx::CEngine::LoadAllTextures()
{
    // TODO!
    return true;
}

bool Gfx::CEngine::SetTexture(const std::string& name, int stage)
{
    // TODO!
    return true;
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
        limit *= m_lastSize.w/640.0f;  // limit further if large window!
        limit += m_limitLOD[0]*(m_lastObjectDetail*2.0f);
    }
    else
    {
        limit = m_limitLOD[rank];
        limit *= m_size.w/640.0f;  // limit further if large window!
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

    float aspect = (static_cast<float>(m_size.h)) / m_size.w;
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


void Gfx::CEngine::SetBackground(const std::string& name, Gfx::Color up, Gfx::Color down,
                                 Gfx::Color cloudUp, Gfx::Color cloudDown,
                                 bool full, bool quarter)
{
    m_backgroundName      = name;
    m_backgroundColorUp   = up;
    m_backgroundColorDown = down;
    m_backgroundCloudUp   = cloudUp;
    m_backgroundCloudDown = cloudDown;
    m_backgroundFull      = full;
    m_backgroundQuarter   = quarter;
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

void Gfx::CEngine::SetForegroundImageName(const std::string& name)
{
    if (! m_foregroundImageName.empty())
        DestroyTexture(m_foregroundImageName);

    m_foregroundImageName = name;
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
    /* TODO!
    D3DObjLevel1*   p1;
    D3DObjLevel2*   p2;
    D3DObjLevel3*   p3;
    D3DObjLevel4*   p4;
    D3DObjLevel5*   p5;
    D3DVERTEX2*     pv;
    int             l1, l2, l3, l4, l5, objRank;*/

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
    {
        Draw3DScene();
    }


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
        // Draw the field
        // TODO!
        /*
        p1 = m_objectPointer;
        for ( l1=0 ; l1<p1->totalUsed ; l1++ )
        {
            p2 = p1->table[l1];
            if ( p2 == 0 )  continue;
            SetTexture(p2->texName1, 0);
            SetTexture(p2->texName2, 1);
            for ( l2=0 ; l2<p2->totalUsed ; l2++ )
            {
                p3 = p2->table[l2];
                if ( p3 == 0 )  continue;
                objRank = p3->objRank;
                if ( m_objectParam[objRank].type != TYPETERRAIN )  continue;
                if ( !m_objectParam[objRank].bDrawWorld )  continue;

                {
                    D3DMATRIX mat = MAT_TO_D3DMAT(m_objectParam[objRank].transform);
                    m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
                }

                if ( !IsVisible(objRank) )  continue;
                m_light->LightUpdate(m_objectParam[objRank].type);
                for ( l3=0 ; l3<p3->totalUsed ; l3++ )
                {
                    p4 = p3->table[l3];
                    if ( p4 == 0 )  continue;
                    if ( m_objectParam[objRank].distance <  p4->min ||
                            m_objectParam[objRank].distance >= p4->max )  continue;
                    for ( l4=0 ; l4<p4->totalUsed ; l4++ )
                    {
                        p5 = p4->table[l4];
                        if ( p5 == 0 )  continue;
                        for ( l5=0 ; l5<p5->totalUsed ; l5++ )
                        {
                            p6 = p5->table[l5];
                            if ( p6 == 0 )  continue;
                            SetMaterial(p6->material);
                            SetState(p6->state);
                            if ( p6->type == D3DTYPE6T )
                            {
                                pv = &p6->vertex[0];
                                m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
                                                            D3DFVF_VERTEX2,
                                                            pv, p6->totalUsed,
                                                            NULL);
                                m_statisticTriangle += p6->totalUsed/3;
                            }
                            if ( p6->type == D3DTYPE6S )
                            {
                                pv = &p6->vertex[0];
                                m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                                            D3DFVF_VERTEX2,
                                                            pv, p6->totalUsed,
                                                            NULL);
                                m_statisticTriangle += p6->totalUsed-2;
                            }
                        }
                    }
                }
            }
        }*/

        // Draws the shadows
        DrawShadow();
    }

    // Draw objects
    bool transparent = false;
    /* TODO!
    p1 = m_objectPointer;
    for ( l1=0 ; l1<p1->totalUsed ; l1++ )
    {
        p2 = p1->table[l1];
        if ( p2 == 0 )  continue;
        SetTexture(p2->texName1, 0);
        SetTexture(p2->texName2, 1);
        for ( l2=0 ; l2<p2->totalUsed ; l2++ )
        {
            p3 = p2->table[l2];
            if ( p3 == 0 )  continue;
            objRank = p3->objRank;
            if ( m_bShadow && m_objectParam[objRank].type == TYPETERRAIN )  continue;
            if ( !m_objectParam[objRank].bDrawWorld )  continue;

            {
                D3DMATRIX mat = MAT_TO_D3DMAT(m_objectParam[objRank].transform);
                m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
            }

            if ( !IsVisible(objRank) )  continue;
            m_light->LightUpdate(m_objectParam[objRank].type);
            for ( l3=0 ; l3<p3->totalUsed ; l3++ )
            {
                p4 = p3->table[l3];
                if ( p4 == 0 )  continue;
                if ( m_objectParam[objRank].distance <  p4->min ||
                        m_objectParam[objRank].distance >= p4->max )  continue;
                for ( l4=0 ; l4<p4->totalUsed ; l4++ )
                {
                    p5 = p4->table[l4];
                    if ( p5 == 0 )  continue;
                    for ( l5=0 ; l5<p5->totalUsed ; l5++ )
                    {
                        p6 = p5->table[l5];
                        if ( p6 == 0 )  continue;
                        SetMaterial(p6->material);
                        if ( m_objectParam[objRank].transparency != 0.0f )  // transparent ?
                        {
                            transparent = true;
                            continue;
                        }
                        SetState(p6->state);
                        if ( p6->type == D3DTYPE6T )
                        {
                            pv = &p6->vertex[0];
                            m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
                                                        D3DFVF_VERTEX2,
                                                        pv, p6->totalUsed,
                                                        NULL);
                            m_statisticTriangle += p6->totalUsed/3;
                        }
                        if ( p6->type == D3DTYPE6S )
                        {
                            pv = &p6->vertex[0];
                            m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                                        D3DFVF_VERTEX2,
                                                        pv, p6->totalUsed,
                                                        NULL);
                            m_statisticTriangle += p6->totalUsed-2;
                        }
                    }
                }
            }
        }
    }*/

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

        // Draw transparent objects.
        /* TODO!
        p1 = m_objectPointer;
        for ( l1=0 ; l1<p1->totalUsed ; l1++ )
        {
            p2 = p1->table[l1];
            if ( p2 == 0 )  continue;
            SetTexture(p2->texName1, 0);
            SetTexture(p2->texName2, 1);
            for ( l2=0 ; l2<p2->totalUsed ; l2++ )
            {
                p3 = p2->table[l2];
                if ( p3 == 0 )  continue;
                objRank = p3->objRank;
                if ( m_bShadow && m_objectParam[objRank].type == TYPETERRAIN )  continue;
                if ( !m_objectParam[objRank].bDrawWorld )  continue;

                {
                    D3DMATRIX mat = MAT_TO_D3DMAT(m_objectParam[objRank].transform);
                    m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
                }

                if ( !IsVisible(objRank) )  continue;
                m_light->LightUpdate(m_objectParam[objRank].type);
                for ( l3=0 ; l3<p3->totalUsed ; l3++ )
                {
                    p4 = p3->table[l3];
                    if ( p4 == 0 )  continue;
                    if ( m_objectParam[objRank].distance <  p4->min ||
                            m_objectParam[objRank].distance >= p4->max )  continue;
                    for ( l4=0 ; l4<p4->totalUsed ; l4++ )
                    {
                        p5 = p4->table[l4];
                        if ( p5 == 0 )  continue;
                        for ( l5=0 ; l5<p5->totalUsed ; l5++ )
                        {
                            p6 = p5->table[l5];
                            if ( p6 == 0 )  continue;
                            SetMaterial(p6->material);
                            if ( m_objectParam[objRank].transparency == 0.0f )  continue;
                            SetState(tState, tColor);
                            if ( p6->type == D3DTYPE6T )
                            {
                                pv = &p6->vertex[0];
                                m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
                                                            D3DFVF_VERTEX2,
                                                            pv, p6->totalUsed,
                                                            NULL);
                                m_statisticTriangle += p6->totalUsed/3;
                            }
                            if ( p6->type == D3DTYPE6S )
                            {
                                pv = &p6->vertex[0];
                                m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                                            D3DFVF_VERTEX2,
                                                            pv, p6->totalUsed,
                                                            NULL);
                                m_statisticTriangle += p6->totalUsed-2;
                            }
                        }
                    }
                }
            }
        } */
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

        // TODO!
        /*
        for (int l1 = 0; l1 < m_objectTree.size(); l1++)
        {
            Gfx::EngineObjLevel1* p1 = &m_objectTree[l1];
            p2 = p1->table[l1];
            if ( p2 == 0 )  continue;
            SetTexture(p2->texName1, 0);
            SetTexture(p2->texName2, 1);
            for ( l2=0 ; l2<p2->totalUsed ; l2++ )
            {
                p3 = p2->table[l2];
                if ( p3 == 0 )  continue;
                objRank = p3->objRank;
                if ( !m_objectParam[objRank].bDrawFront )  continue;

                {
                    D3DMATRIX mat = MAT_TO_D3DMAT(m_objectParam[objRank].transform);
                    m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
                }

                if ( !IsVisible(objRank) )  continue;
                m_light->LightUpdate(m_objectParam[objRank].type);
                for ( l3=0 ; l3<p3->totalUsed ; l3++ )
                {
                    p4 = p3->table[l3];
                    if ( p4 == 0 )  continue;
                    if ( m_objectParam[objRank].distance <  p4->min ||
                         m_objectParam[objRank].distance >= p4->max )  continue;
                    for ( l4=0 ; l4<p4->totalUsed ; l4++ )
                    {
                        p5 = p4->table[l4];
                        if ( p5 == 0 )  continue;
                        for ( l5=0 ; l5<p5->totalUsed ; l5++ )
                        {
                            p6 = p5->table[l5];
                            if ( p6 == 0 )  continue;
                            SetMaterial(p6->material);
                            SetState(p6->state);
                            if ( p6->type == D3DTYPE6T )
                            {
                                pv = &p6->vertex[0];
                                m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
                                                            D3DFVF_VERTEX2,
                                                            pv, p6->totalUsed,
                                                            NULL);
                                m_statisticTriangle += p6->totalUsed/3;
                            }
                            if ( p6->type == D3DTYPE6S )
                            {
                                pv = &p6->vertex[0];
                                m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                                            D3DFVF_VERTEX2,
                                                            pv, p6->totalUsed,
                                                            NULL);
                                m_statisticTriangle += p6->totalUsed-2;
                            }
                        }
                    }
                }
            }
        }*/

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
    // TODO!
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

    // TODO: wtf?
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
    for (int i = 0; i < static_cast<int>( m_shadow.size() ); i++)
    {
        if (m_shadow[i].hide) continue;

        Math::Vector pos = m_shadow[i].pos;  // pos = center of the shadow on the ground

        if (m_eyePt.y == pos.y)  continue;  // camera at the same level?

        float d = 0.0f;
        float D = 0.0f;

        // h is the height above the ground to which the shadow
        // will be drawn.
        if (m_eyePt.y > pos.y)  // camera on?
        {
            float height = m_eyePt.y-pos.y;
            float h = m_shadow[i].radius;
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
            float h = m_shadow[i].radius;
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
        float hFactor = m_shadow[i].height/20.0f;
        if ( hFactor < 0.0f )  hFactor = 0.0f;
        if ( hFactor > 1.0f )  hFactor = 1.0f;
        hFactor = powf(1.0f-hFactor, 2.0f);
        if ( hFactor < 0.2f )  hFactor = 0.2f;

        float radius = m_shadow[i].radius*1.5f;
        radius *= 2.0f-hFactor;  // greater if high
        radius *= 1.0f-d/D;  // smaller if close


        Math::Vector corner[4];

        if (m_shadow[i].type == Gfx::ENG_SHADOW_NORM)
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

            rot = Math::RotatePoint(-m_shadow[i].angle, Math::Point(radius, radius));
            corner[0].x = rot.x;
            corner[0].z = rot.y;
            corner[0].y = 0.0f;

            rot = Math::RotatePoint(-m_shadow[i].angle, Math::Point(-radius, radius));
            corner[1].x = rot.x;
            corner[1].z = rot.y;
            corner[1].y = 0.0f;

            rot = Math::RotatePoint(-m_shadow[i].angle, Math::Point(radius, -radius));
            corner[2].x = rot.x;
            corner[2].z = rot.y;
            corner[2].y = 0.0f;

            rot = Math::RotatePoint(-m_shadow[i].angle, Math::Point(-radius, -radius));
            corner[3].x = rot.x;
            corner[3].z = rot.y;
            corner[3].y = 0.0f;

            if (m_shadow[i].type == Gfx::ENG_SHADOW_WORM)
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

        corner[0] = Math::CrossProduct(corner[0], m_shadow[i].normal);
        corner[1] = Math::CrossProduct(corner[1], m_shadow[i].normal);
        corner[2] = Math::CrossProduct(corner[2], m_shadow[i].normal);
        corner[3] = Math::CrossProduct(corner[3], m_shadow[i].normal);

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

        float intensity = (0.5f+m_shadow[i].intensity*0.5f)*hFactor;

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

    if (m_backForce || (m_skyMode && m_backgroundName[0] != 0) )
    {
        DrawBackgroundImage();  // image
    }
}

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

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, false);

    SetState(Gfx::ENG_RSTATE_NORMAL);

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

void Gfx::CEngine::DrawBackgroundImageQuarter(Math::Point p1, Math::Point p2, const std::string& name)
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

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);

    SetTexture(name);
    SetState(Gfx::ENG_RSTATE_WRAP);

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

void QuarterName(std::string& buffer, const std::string& name, int quarter)
{
    size_t pos = name.find('.');
    if (pos == std::string::npos)
    {
        buffer = name;
        return;
    }

    buffer = name.substr(0, pos) + std::string(1, static_cast<char>('a' + quarter)) + name.substr(pos);
}

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
        QuarterName(name, m_backgroundName, 0);
        DrawBackgroundImageQuarter(p1, p2, name);

        p1.x = 0.5f;
        p1.y = 0.5f;
        p2.x = 1.0f;
        p2.y = 1.0f;
        QuarterName(name, m_backgroundName, 1);
        DrawBackgroundImageQuarter(p1, p2, name);

        p1.x = 0.0f;
        p1.y = 0.0f;
        p2.x = 0.5f;
        p2.y = 0.5f;
        QuarterName(name, m_backgroundName, 2);
        DrawBackgroundImageQuarter(p1, p2, name);

        p1.x = 0.5f;
        p1.y = 0.0f;
        p2.x = 1.0f;
        p2.y = 0.5f;
        QuarterName(name, m_backgroundName, 3);
        DrawBackgroundImageQuarter(p1, p2, name);
    }
    else
    {
        p1.x = 0.0f;
        p1.y = 0.0f;
        p2.x = 1.0f;
        p2.y = 1.0f;
        DrawBackgroundImageQuarter(p1, p2, m_backgroundName);
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

void Gfx::CEngine::DrawForegroundImage()
{
    if (m_foregroundImageName.empty()) return;

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

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false );
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);

    SetTexture(m_foregroundImageName);
    SetState(Gfx::ENG_RSTATE_CLAMP | Gfx::ENG_RSTATE_TTEXTURE_BLACK);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

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

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_FOG, false);
    m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, false);

    SetState(m_overMode);

    m_device->SetTransform(Gfx::TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, m_matWorldInterface);

    Gfx::VertexCol vertex[4] =
    {
        Gfx::VertexCol(Math::Vector(p1.x, p1.y, 0.0f), color[1],color[2]),
        Gfx::VertexCol(Math::Vector(p1.x, p2.y, 0.0f), color[0],color[2]),
        Gfx::VertexCol(Math::Vector(p2.x, p1.y, 0.0f), color[1],color[2]),
        Gfx::VertexCol(Math::Vector(p2.x, p2.y, 0.0f), color[0],color[2])
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

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

    // TODO: draw highlight!
}

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
        Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), normal, Math::Point(u2, v2)),
        Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), normal, Math::Point(u1, v1)),
        Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), normal, Math::Point(u2, v1))
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}
