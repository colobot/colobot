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


#include "graphics/engine/engine.h"

#include "app/app.h"
#include "app/input.h"

#include "common/image.h"
#include "common/key.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "common/stringutils.h"

#include "common/system/system.h"

#include "graphics/core/device.h"
#include "graphics/core/framebuffer.h"
#include "graphics/core/material.h"
#include "graphics/core/renderers.h"
#include "graphics/core/triangle.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/cloud.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/planet.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/text.h"
#include "graphics/engine/water.h"

#include "graphics/model/model_mesh.h"
#include "graphics/model/model_shadow_spot.h"

#include "level/robotmain.h"
#include "level/player_profile.h"

#include "math/geometry.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"

#include <iomanip>
#include <SDL_surface.h>
#include <SDL_thread.h>
#include <thread>

using TimeUtils::TimeUnit;

// Graphics module namespace
namespace Gfx
{

/**
 * \struct EngineBaseObjDataTier
 * \brief Tier 3 of object tree (data)
 */
struct EngineBaseObjDataTier
{
    EngineTriangleType      type = EngineTriangleType::TRIANGLES;
    Material                material = {};

    std::vector<Vertex3D>   vertices;
    CVertexBuffer* buffer = nullptr;
    bool                    updateStaticBuffer = false;

    Texture                 albedoTexture;
    Texture                 emissiveTexture;
    Texture                 materialTexture;
    Texture                 normalTexture;
    Texture                 detailTexture;

    glm::vec2               uvOffset = { 0.0f, 0.0f };
    glm::vec2               uvScale = { 1.0f, 1.0f };
};

/**
 * \struct BaseEngineObject
 * \brief Base (template) object - geometry for engine objects
 *
 * This is also the tier 1 of base object tree.
 */
struct EngineBaseObject
{
    //! If true, base object is valid in objects vector
    bool used = false;
    //! Number of triangles
    int                    totalTriangles = 0;
    //! Bounding box min (origin 0,0,0 always included)
    glm::vec3           bboxMin{ 0, 0, 0 };
    //! bounding box max (origin 0,0,0 always included)
    glm::vec3           bboxMax{ 0, 0, 0 };
    //! A bounding sphere that contains all the vertices in this EngineBaseObject
    Math::Sphere           boundingSphere;
    //! Next tier
    std::vector<EngineBaseObjDataTier> next;

    inline void LoadDefault()
    {
        *this = EngineBaseObject();
    }
};

/**
 * \struct EngineMouse
 * \brief Information about mouse cursor
 */
struct EngineMouse
{
    //! Index of texture element for 1st image
    int icon1;
    //! Index of texture element for 2nd image
    int icon2;
    //! Shadow texture part
    int iconShadow;
    //! Mode to render 1st image in
    TransparencyMode mode1;
    //! Mode to render 2nd image in
    TransparencyMode mode2;
    //! Hot point
    glm::ivec2 hotPoint;

    EngineMouse(int icon1 = -1,
                int icon2 = -1,
                int iconShadow = -1,
                TransparencyMode mode1 = TransparencyMode::NONE,
                TransparencyMode mode2 = TransparencyMode::NONE,
                glm::ivec2 hotPoint = { 0, 0 })
        : icon1(icon1)
        , icon2(icon2)
        , iconShadow(iconShadow)
        , mode1(mode1)
        , mode2(mode2)
        , hotPoint(hotPoint)
    {}
};

constexpr glm::ivec2 MOUSE_SIZE(32, 32);
const std::map<EngineMouseType, EngineMouse> MOUSE_TYPES = {
    {{ENG_MOUSE_NORM},    {EngineMouse( 0,  1, 32, TransparencyMode::WHITE, TransparencyMode::BLACK, glm::ivec2( 1,  1))}},
    {{ENG_MOUSE_WAIT},    {EngineMouse( 2,  3, 33, TransparencyMode::WHITE, TransparencyMode::BLACK, glm::ivec2( 8, 12))}},
    {{ENG_MOUSE_HAND},    {EngineMouse( 4,  5, 34, TransparencyMode::WHITE, TransparencyMode::BLACK, glm::ivec2( 7,  2))}},
    {{ENG_MOUSE_NO},      {EngineMouse( 6,  7, 35, TransparencyMode::WHITE, TransparencyMode::BLACK, glm::ivec2(10, 10))}},
    {{ENG_MOUSE_EDIT},    {EngineMouse( 8,  9, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 6, 10))}},
    {{ENG_MOUSE_CROSS},   {EngineMouse(10, 11, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2(10, 10))}},
    {{ENG_MOUSE_MOVEV},   {EngineMouse(12, 13, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 5, 11))}},
    {{ENG_MOUSE_MOVEH},   {EngineMouse(14, 15, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2(11,  5))}},
    {{ENG_MOUSE_MOVED},   {EngineMouse(16, 17, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 9,  9))}},
    {{ENG_MOUSE_MOVEI},   {EngineMouse(18, 19, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 9,  9))}},
    {{ENG_MOUSE_MOVE},    {EngineMouse(20, 21, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2(11, 11))}},
    {{ENG_MOUSE_TARGET},  {EngineMouse(22, 23, -1, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2(15, 15))}},
    {{ENG_MOUSE_SCROLLL}, {EngineMouse(24, 25, 43, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 2,  9))}},
    {{ENG_MOUSE_SCROLLR}, {EngineMouse(26, 27, 44, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2(17,  9))}},
    {{ENG_MOUSE_SCROLLU}, {EngineMouse(28, 29, 45, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 9,  2))}},
    {{ENG_MOUSE_SCROLLD}, {EngineMouse(30, 31, 46, TransparencyMode::BLACK, TransparencyMode::WHITE, glm::ivec2( 9, 17))}},
};

CEngine::CEngine(CApplication *app, CSystemUtils* systemUtils)
    : m_app(app),
      m_systemUtils(systemUtils),
      m_ambientColor(),
      m_fogColor(),
      m_deepView(),
      m_fogStart(),
      m_highlightRank()
{
    m_device = nullptr;

    m_lightMan   = nullptr;
    m_text       = nullptr;
    m_particle   = nullptr;
    m_water      = nullptr;
    m_cloud      = nullptr;
    m_lightning  = nullptr;
    m_planet     = nullptr;
    m_sound      = nullptr;
    m_terrain    = nullptr;

    m_showStats = false;

    m_focus = 0.75f;
    m_hfov = 2.0f * atan((640.f/480.f) * tan(m_focus / 2.0f));

    m_rankView = 0;

    m_ambientColor[0] = Color(0.5f, 0.5f, 0.5f, 0.5f);
    m_ambientColor[1] = Color(0.5f, 0.5f, 0.5f, 0.5f);
    m_fogColor[0]     = Color(1.0f, 1.0f, 1.0f, 1.0f);
    m_fogColor[1]     = Color(1.0f, 1.0f, 1.0f, 1.0f);
    m_deepView[0]     = 1000.0f;
    m_deepView[1]     = 1000.0f;
    m_fogStart[0]     = 0.75f;
    m_fogStart[1]     = 0.75f;
    m_waterAddColor   = Color(0.0f, 0.0f, 0.0f, 0.0f);

    m_render            = true;
    m_renderInterface   = true;
    m_screenshotMode    = false;
    m_triplanarMode     = false;
    m_triplanarScale    = 0.2f;
    m_dirty             = true;
    m_fog               = true;
    m_secondTex         = "";
    m_eyeDirH           = 0.0f;
    m_eyeDirV           = 0.0f;
    m_backgroundName    = "";  // no background image
    m_backgroundColorUp   = Color();
    m_backgroundColorDown = Color();
    m_backgroundCloudUp   = Color();
    m_backgroundCloudDown = Color();
    m_backgroundFull = false;
    m_backgroundScale = false;
    m_overFront = true;
    m_overColor = Color();
    m_overMode  = TransparencyMode::BLACK;
    m_highlight = false;
    std::fill_n(m_highlightRank, 100, -1);
    m_highlightTime = 0.0f;
    m_eyePt    = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lookatPt = glm::vec3(0.0f, 0.0f, 1.0f);
    m_drawWorld = true;
    m_drawFront = false;
    m_particleDensity = 1.0f;
    m_clippingDistance = 1.0f;
    m_terrainVision = 1000.0f;
    m_textureMipmapLevel = 1;
    m_textureAnisotropy = 1;
    m_shadowMapping = true;
    m_offscreenShadowRendering = true;
    m_offscreenShadowRenderingResolution = 2048;
    m_qualityShadows = true;
    m_terrainShadows = false;
    m_shadowRange = 0.0f;
    m_multisample = 2;
    m_vsync = 0;

    m_backForce = true;
    m_lightMode = true;
    m_editIndentMode = true;
    m_editIndentValue = 4;
    m_tracePrecision = 1.0f;
    m_pauseBlurEnabled = true;


    m_updateGeometry = false;
    m_updateStaticBuffers = false;

    m_interfaceMode = false;

    m_debugLights = false;
    m_debugDumpLights = false;

    m_mouseType    = ENG_MOUSE_NORM;

    m_fpsCounter = 0;

    m_shadowColor = 0.5f;

    m_defaultTexParams.format = TextureFormat::AUTO;
    m_defaultTexParams.filter = TextureFilter::BILINEAR;

    m_terrainTexParams.format = TextureFormat::AUTO;
    m_terrainTexParams.filter = TextureFilter::BILINEAR;

    // Compute bias matrix for shadow mapping
    glm::mat4 temp1, temp2;
    Math::LoadScaleMatrix(temp1, glm::vec3(0.5f, 0.5f, 0.5f));
    Math::LoadTranslationMatrix(temp2, glm::vec3(1.0f, 1.0f, 1.0f));
    m_shadowBias = temp1 * temp2;

    m_statisticTriangle = 0;
    m_fps = 0.0f;
    m_firstGroundSpot = false;
}

CEngine::~CEngine()
{
}

void CEngine::SetDevice(CDevice *device)
{
    m_device = device;
}

CDevice* CEngine::GetDevice()
{
    return m_device;
}

CUIRenderer* CEngine::GetUIRenderer()
{
    return m_device->GetUIRenderer();
}

CObjectRenderer* CEngine::GetObjectRenderer()
{
    return m_device->GetObjectRenderer();
}

COldModelManager* CEngine::GetModelManager()
{
    return m_modelManager.get();
}

CPyroManager* CEngine::GetPyroManager()
{
    return m_pyroManager.get();
}

CText* CEngine::GetText()
{
    return m_text.get();
}

CLightManager* CEngine::GetLightManager()
{
    return m_lightMan.get();
}

CParticle* CEngine::GetParticle()
{
    return m_particle.get();
}

CTerrain* CEngine::GetTerrain()
{
    return m_terrain;
}

CWater* CEngine::GetWater()
{
    return m_water.get();
}

CLightning* CEngine::GetLightning()
{
    return m_lightning.get();
}

CPlanet* CEngine::GetPlanet()
{
    return m_planet.get();
}

CCloud* CEngine::GetCloud()
{
    return m_cloud.get();
}

void CEngine::SetTerrain(CTerrain* terrain)
{
    m_terrain = terrain;
}

bool CEngine::Create()
{
    m_size = m_app->GetVideoConfig().size;

    // Use the setters to set defaults, because they automatically disable what is not supported
    SetShadowMapping(m_shadowMapping);
    SetShadowMappingQuality(m_qualityShadows);
    SetShadowMappingOffscreen(m_offscreenShadowRendering);
    SetShadowMappingOffscreenResolution(m_offscreenShadowRenderingResolution);
    SetMultiSample(m_multisample);
    SetVSync(m_vsync);

    m_modelManager = std::make_unique<COldModelManager>(this);
    m_pyroManager = std::make_unique<CPyroManager>();
    m_lightMan   = std::make_unique<CLightManager>(this);
    m_text       = std::make_unique<CText>(this);
    m_particle   = std::make_unique<CParticle>(this);
    m_water      = std::make_unique<CWater>(this);
    m_cloud      = std::make_unique<CCloud>(this);
    m_lightning  = std::make_unique<CLightning>(this);
    m_planet     = std::make_unique<CPlanet>(this);

    m_lightMan->SetDevice(m_device);
    m_particle->SetDevice(m_device);

    m_text->SetDevice(m_device);
    if (! m_text->Create())
    {
        std::string error = m_text->GetError();
        GetLogger()->Error("Error creating CText: %%", error);
        return false;
    }

    m_device->SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));

    SetFocus(m_focus);

    m_matWorldInterface = glm::mat4(1.0f);
    m_matViewInterface = glm::mat4(1.0f);

    auto renderer = m_device->GetUIRenderer();
    renderer->SetProjection(0.0f, 1.0f, 0.0f, 1.0f);

    Math::LoadOrthoProjectionMatrix(m_matProjInterface, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    TextureCreateParams params;
    params.format = TextureFormat::AUTO;
    params.filter = TextureFilter::NEAREST;
    params.mipmap = false;
    m_miceTexture = LoadTexture("textures/interface/mouse.png", params);

    m_currentFrameTime = m_systemUtils->GetCurrentTimeStamp();
    m_lastFrameTime = m_systemUtils->GetCurrentTimeStamp();

    return true;
}

void CEngine::Destroy()
{
    m_text->Destroy();

    if (m_shadowMap.id != 0)
    {
        if (m_offscreenShadowRendering)
            m_device->DeleteFramebuffer("shadow");
        else
            m_device->DestroyTexture(m_shadowMap);

        m_shadowMap = Texture();
    }

    m_lightMan.reset();
    m_text.reset();
    m_particle.reset();
    m_water.reset();
    m_cloud.reset();
    m_lightning.reset();
    m_planet.reset();
}

void CEngine::ResetAfterVideoConfigChanged()
{
    m_size = m_app->GetVideoConfig().size;

    // Update the camera projection matrix for new aspect ratio
    ApplyChange();

    // This needs to be recreated on resolution change
    m_device->DeleteFramebuffer("multisample");
}

void CEngine::ReloadAllTextures()
{
    FlushTextureCache();
    m_text->FlushCache();
    m_text->ReloadFonts();

    m_app->GetEventQueue()->AddEvent(Event(EVENT_RELOAD_TEXTURES));
    UpdateGroundSpotTextures();
    // LoadAllTextures() is called from CRobotMain on EVENT_RELOAD_TEXTURES
    // This is required because all dynamic textures need to be loaded first

    // recapture 3D scene
    if (m_worldCaptured)
    {
        m_captureWorld = true;
        m_worldCaptured = false;
    }
}

bool CEngine::ProcessEvent(const Event &event)
{
    if (event.type == EVENT_RESOLUTION_CHANGED)
    {
        ResetAfterVideoConfigChanged();
    }

    if (event.type == EVENT_KEY_DOWN)
    {
        auto data = event.GetData<KeyEventData>();

        if (data->key == KEY(F11) || data->key == KEY(F12))
        {
            m_showStats = !m_showStats;
            return false;
        }
    }

    // By default, pass on all events
    return true;
}

void CEngine::FrameUpdate()
{
    float rTime = m_app->GetRelTime();

    m_lightMan->UpdateProgression(rTime);

    CProfiler::StartPerformanceCounter(PCNT_UPDATE_PARTICLE);
    m_particle->FrameParticle(rTime);
    CProfiler::StopPerformanceCounter(PCNT_UPDATE_PARTICLE);

    ComputeDistance();
    UpdateGeometry();
    UpdateStaticBuffers();

    m_highlightTime = m_app->GetAbsTime();

    if (m_groundMark.draw)
    {
        if (m_groundMark.phase == ENG_GR_MARK_PHASE_INC)  // growing?
        {
            m_groundMark.intensity += rTime*(1.0f/m_groundMark.delay[0]);
            if (m_groundMark.intensity >= 1.0f)
            {
                m_groundMark.intensity = 1.0f;
                m_groundMark.fix = 0.0f;
                m_groundMark.phase = ENG_GR_MARK_PHASE_FIX;
            }
        }
        else if (m_groundMark.phase == ENG_GR_MARK_PHASE_FIX)  // fixed?
        {
            m_groundMark.fix += rTime*(1.0f/m_groundMark.delay[1]);
            if (m_groundMark.fix >= 1.0f)
                m_groundMark.phase = ENG_GR_MARK_PHASE_DEC;
        }
        else if (m_groundMark.phase == ENG_GR_MARK_PHASE_DEC)  // decay?
        {
            m_groundMark.intensity -= rTime*(1.0f/m_groundMark.delay[2]);
            if (m_groundMark.intensity < 0.0f)
            {
                m_groundMark.intensity = 0.0f;
                m_groundMark.phase     = ENG_GR_MARK_PHASE_NULL;
                m_groundMark.draw      = false;
            }
        }
    }
}

void CEngine::WriteScreenShot(const std::filesystem::path& fileName)
{
    auto data = std::make_unique<WriteScreenShotData>();
    data->img = std::make_unique<CImage>(glm::ivec2(m_size.x, m_size.y));

    auto pixels = m_device->GetFrameBufferPixels();
    data->img->SetDataPixels(pixels->GetPixelsData());
    data->img->FlipVertically();

    data->fileName = fileName;

    std::thread{&CEngine::WriteScreenShotThread, std::move(data)}.detach();
}

void CEngine::WriteScreenShotThread(std::unique_ptr<WriteScreenShotData> data)
{
    if ( data->img->SavePNG(data->fileName) )
    {
       GetLogger()->Debug("Save screenshot saved successfully");
    }
    else
    {
       GetLogger()->Error("%%!", data->img->GetError());
    }

    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_WRITE_SCENE_FINISHED));
}

void CEngine::SetPause(bool pause)
{
    m_pause = pause;
}

bool CEngine::GetPause()
{
    return m_pause;
}

void CEngine::SetShowStats(bool show)
{
    m_showStats = show;
}

bool CEngine::GetShowStats()
{
    return m_showStats;
}

void CEngine::SetRenderEnable(bool enable)
{
    m_render = enable;
}

void CEngine::SetRenderInterface(bool enable)
{
    m_renderInterface = enable;
}

bool CEngine::GetRenderInterface()
{
    return m_renderInterface;
}

void CEngine::SetScreenshotMode(bool screenshotMode)
{
    m_screenshotMode = screenshotMode;
}

bool CEngine::GetScreenshotMode()
{
    return m_screenshotMode;
}

glm::ivec2 CEngine::GetWindowSize()
{
    return m_size;
}

glm::vec2 CEngine::WindowToInterfaceCoords(const glm::ivec2& pos)
{
    return { static_cast<float>(pos.x) / static_cast<float>(m_size.x),
             1.0f - static_cast<float>(pos.y) / static_cast<float>(m_size.y) };
}

glm::ivec2 CEngine::InterfaceToWindowCoords(const glm::vec2& pos)
{
    return { static_cast<int>(pos.x * m_size.x),
             static_cast<int>((1.0f - pos.y) * m_size.y) };
}

glm::vec2 CEngine::WindowToInterfaceSize(const glm::ivec2& size)
{
    return { static_cast<float>(size.x) / static_cast<float>(m_size.x),
             static_cast<float>(size.y) / static_cast<float>(m_size.y) };
}

glm::ivec2 CEngine::InterfaceToWindowSize(const glm::vec2& size)
{
    return { static_cast<int>(size.x * m_size.x),
             static_cast<int>(size.y * m_size.y) };
}

void CEngine::AddStatisticTriangle(int count)
{
    m_statisticTriangle += count;
}

int CEngine::GetStatisticTriangle()
{
    return m_statisticTriangle;
}

void CEngine::SetStatisticPos(glm::vec3 pos)
{
    m_statisticPos = pos;
}

void CEngine::SetTimerDisplay(const std::string& text)
{
    m_timerText = text;
}



/*******************************************************
                   Object management
 *******************************************************/

EngineBaseObjDataTier& CEngine::AddLevel(EngineBaseObject& p3, EngineTriangleType type, const Material& material)
{
    for (size_t i = 0; i < p3.next.size(); i++)
    {
        if ( (p3.next[i].type == type) && (p3.next[i].material == material) )
            return p3.next[i];
    }

    p3.next.push_back(EngineBaseObjDataTier{ type, material });
    return p3.next.back();
}

int CEngine::CreateBaseObject()
{
    size_t baseObjRank = 0;

    for (; baseObjRank < m_baseObjects.size(); baseObjRank++)
    {
        if (!m_baseObjects[baseObjRank].used)
        {
            m_baseObjects[baseObjRank] = {};
            break;
        }
    }

    if (baseObjRank == m_baseObjects.size())
        m_baseObjects.push_back(EngineBaseObject());
    else
        m_baseObjects[baseObjRank] = {};


    m_baseObjects[baseObjRank].used = true;

    return static_cast<int>(baseObjRank);
}

void CEngine::DeleteBaseObject(int baseObjRank)
{
    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    if (! p1.used)
        return;

    for (auto& data : p1.next)
    {
        m_device->DestroyVertexBuffer(data.buffer);
        data.buffer = nullptr;
    }

    p1.next.clear();
    p1.used = false;
}

void CEngine::DeleteAllBaseObjects()
{
    for (auto& object : m_baseObjects)
    {
        if (!object.used)
            continue;

        for (auto& data : object.next)
        {
            m_device->DestroyVertexBuffer(data.buffer);
            data.buffer = nullptr;
        }
    }

    m_baseObjects.clear();
}

void CEngine::CopyBaseObject(int sourceBaseObjRank, int destBaseObjRank)
{
    assert(sourceBaseObjRank >= 0 && sourceBaseObjRank < static_cast<int>( m_baseObjects.size() ));
    assert(destBaseObjRank >= 0 && destBaseObjRank < static_cast<int>( m_baseObjects.size() ));

    m_baseObjects[destBaseObjRank] = m_baseObjects[sourceBaseObjRank];

    EngineBaseObject& p1 = m_baseObjects[destBaseObjRank];

    if (! p1.used)
        return;

    for (auto& data : p1.next)
    {
        data.buffer = nullptr;
        data.updateStaticBuffer = true;
    }

    m_updateStaticBuffers = true;
}

void CEngine::AddBaseObjTriangles(int baseObjRank, const std::vector<Vertex3D>& vertices,
    const Material& material, EngineTriangleType type)
{
    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject&      p1 = m_baseObjects[baseObjRank];
    EngineBaseObjDataTier& p3 = AddLevel(p1, type, material);

    p3.vertices.insert(p3.vertices.end(), vertices.begin(), vertices.end());

    if (p3.buffer)
        m_device->DestroyVertexBuffer(p3.buffer);

    p3.buffer = nullptr;
    p3.updateStaticBuffer = true;
    m_updateStaticBuffers = true;

    for (size_t i = 0; i < vertices.size(); i++)
    {
        p1.bboxMin.x = Math::Min(vertices[i].position.x, p1.bboxMin.x);
        p1.bboxMin.y = Math::Min(vertices[i].position.y, p1.bboxMin.y);
        p1.bboxMin.z = Math::Min(vertices[i].position.z, p1.bboxMin.z);
        p1.bboxMax.x = Math::Max(vertices[i].position.x, p1.bboxMax.x);
        p1.bboxMax.y = Math::Max(vertices[i].position.y, p1.bboxMax.y);
        p1.bboxMax.z = Math::Max(vertices[i].position.z, p1.bboxMax.z);
    }

    p1.boundingSphere = Math::BoundingSphereForBox(p1.bboxMin, p1.bboxMax);

    p1.totalTriangles += vertices.size() / 3;
}

void CEngine::DebugObject(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    CLogger* l = GetLogger();

    l->Debug("Debug object: %%", objRank);
    if (! m_objects[objRank].used)
    {
        l->Debug(" not used\n");
        return;
    }

    l->Debug(" baseObjRank = %%", m_objects[objRank].baseObjRank);
    l->Debug(" visible = %%", m_objects[objRank].visible);
    l->Debug(" drawWorld = %%", m_objects[objRank].drawWorld);
    l->Debug(" drawFront = %%", m_objects[objRank].drawFront);
    l->Debug(" type = %%", m_objects[objRank].type);
    l->Debug(" distance = %%", m_objects[objRank].distance);
    l->Debug(" shadowRank = %%", m_objects[objRank].shadowRank);
    l->Debug(" ghost = %%", m_objects[objRank].ghost);

    l->Debug(" baseObj:");
    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
    {
        l->Debug("  null");
        return;
    }

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];
    if (!p1.used)
    {
        l->Debug("  not used");
        return;
    }

    std::string vecStr;

    vecStr = Math::ToString(p1.bboxMin);
    l->Debug("  bboxMin: %%", vecStr);
    vecStr = Math::ToString(p1.bboxMax);
    l->Debug("  bboxMax: %%", vecStr);
    l->Debug("  totalTriangles: %%", p1.totalTriangles);
    l->Debug("  radius: %%", p1.boundingSphere.radius);

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        EngineBaseObjDataTier& p2 = p1.next[l2];
        l->Debug("  l2:");

        /*
        l->Debug("   tex1: %s (id: %u)\n", p2.tex1Name.c_str(), p2.tex1.id);
        l->Debug("   tex2: %s (id: %u)\n", p2.tex2Name.c_str(), p2.tex2.id);

        for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
        {
            EngineBaseObjDataTier& p3 = p2.next[l3];

            l->Debug("   l3:\n");
            l->Debug("    type: %d\n", p3.type);
            l->Debug("    state: %d\n", p3.state);
            l->Debug("    staticBufferId: %u\n", p3.buffer);
            l->Debug("    updateStaticBuffer: %s\n", p3.updateStaticBuffer ? "true" : "false");
        }
        // */
    }
}

int CEngine::CreateObject()
{
    int objRank = 0;
    for ( ; objRank < static_cast<int>( m_objects.size() ); objRank++)
    {
        if (! m_objects[objRank].used)
        {
            m_objects[objRank] = {};
            break;
        }
    }

    if (objRank == static_cast<int>( m_objects.size() ))
        m_objects.push_back(EngineObject());


    m_objects[objRank].used = true;

    glm::mat4 mat = glm::mat4(1.0f);
    SetObjectTransform(objRank, mat);

    m_objects[objRank].drawWorld = true;
    m_objects[objRank].distance = 0.0f;
    m_objects[objRank].baseObjRank = -1;
    m_objects[objRank].shadowRank = -1;

    return objRank;
}

void CEngine::DeleteAllObjects()
{
    m_objects.clear();
    m_shadowSpots.clear();

    DeleteAllGroundSpots();
}

void CEngine::DeleteObject(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    // Mark object as deleted
    m_objects[objRank].used = false;

    // Delete associated shadows
    DeleteShadowSpot(objRank);
}

void CEngine::SetObjectBaseRank(int objRank, int baseObjRank)
{
    assert(objRank == -1 || (objRank >= 0 && objRank < static_cast<int>( m_objects.size() )));

    m_objects[objRank].baseObjRank = baseObjRank;
}

int CEngine::GetObjectBaseRank(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    return m_objects[objRank].baseObjRank;
}

void CEngine::SetObjectType(int objRank, EngineObjectType type)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].type = type;
}

EngineObjectType CEngine::GetObjectType(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    return m_objects[objRank].type;
}


void CEngine::SetObjectTransform(int objRank, const glm::mat4& transform)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].transform = transform;
}

void CEngine::GetObjectTransform(int objRank, glm::mat4& transform)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    transform = m_objects[objRank].transform;
}

void CEngine::SetObjectDrawWorld(int objRank, bool draw)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].drawWorld = draw;
}

void CEngine::SetObjectTeam(int objRank, int team)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    m_objects[objRank].team = team;
}

void CEngine::SetObjectDrawFront(int objRank, bool draw)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].drawFront = draw;
}

void CEngine::SetObjectGhostMode(int objRank, bool enabled)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].ghost = enabled;
}

void CEngine::GetObjectBBox(int objRank, glm::vec3& min, glm::vec3& max)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

    min = m_baseObjects[baseObjRank].bboxMin;
    max = m_baseObjects[baseObjRank].bboxMax;
}


int CEngine::GetObjectTotalTriangles(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return 0;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    return m_baseObjects[baseObjRank].totalTriangles;
}

int CEngine::GetPartialTriangles(int objRank, float percent, int maxCount,
                                 std::vector<EngineTriangle>& triangles)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return 0;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    int total = p1.totalTriangles;
    int expectedCount = static_cast<int>(percent * total);
    triangles.reserve(Math::Min(maxCount, expectedCount));

    int actualCount = 0;

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        EngineBaseObjDataTier& p3 = p1.next[l2];

        if (p3.type == EngineTriangleType::TRIANGLES)
        {
            for (size_t i = 0; i < p3.vertices.size(); i += 3)
            {
                if (static_cast<float>(actualCount) / total >= percent)
                    break;

                if (actualCount >= maxCount)
                    break;

                EngineTriangle t;
                t.triangle[0] = p3.vertices[i];
                t.triangle[1] = p3.vertices[i + 1];
                t.triangle[2] = p3.vertices[i + 2];
                t.material = p3.material;

                triangles.push_back(t);

                ++actualCount;
            }
        }
        else if (p3.type == EngineTriangleType::SURFACE)
        {
            for (size_t i = 0; i < p3.vertices.size(); i += 1)
            {
                if (static_cast<float>(actualCount) / total >= percent)
                    break;

                if (actualCount >= maxCount)
                    break;

                EngineTriangle t;
                t.triangle[0] = p3.vertices[i];
                t.triangle[1] = p3.vertices[i + 1];
                t.triangle[2] = p3.vertices[i + 2];
                t.material = p3.material;

                triangles.push_back(t);

                ++actualCount;
            }
        }
    }

    return actualCount;
}

void CEngine::ChangeSecondTexture(int objRank, const std::string& tex2Name)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    for (auto& data : p1.next)
    {
        if (data.material.detailTexture == tex2Name)
            continue;  // already new

        data.material.detailTexture = tex2Name;

        data.detailTexture = LoadTexture("textures/" + tex2Name);
    }
}

void CEngine::SetUVTransform(int objRank, const std::string& tag, const glm::vec2& offset, const glm::vec2& scale)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    for (auto& data : p1.next)
    {
        if (data.material.tag == tag)
        {
            data.uvOffset = offset;
            data.uvScale = scale;
        }
    }
}

void CEngine::CreateShadowSpot(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    // Already allocated?
    if (m_objects[objRank].shadowRank != -1)
        return;

    int index = 0;
    for ( ; index < static_cast<int>( m_shadowSpots.size() ); index++)
    {
        if (! m_shadowSpots[index].used)
        {
            m_shadowSpots[index] = {};
            break;
        }
    }

    if (index == static_cast<int>( m_shadowSpots.size() ))
        m_shadowSpots.push_back(EngineShadow());

    m_shadowSpots[index].used = true;
    m_shadowSpots[index].objRank = objRank;
    m_shadowSpots[index].height = 0.0f;

    m_objects[objRank].shadowRank = index;
}

void CEngine::DeleteShadowSpot(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].used = false;
    m_shadowSpots[shadowRank].objRank = -1;

    m_objects[objRank].shadowRank = -1;
}

void CEngine::SetObjectShadowSpotHide(int objRank, bool hide)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].hide = hide;
}

void CEngine::SetObjectShadowSpotType(int objRank, EngineShadowType type)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].type = type;
}

void CEngine::SetObjectShadowSpotPos(int objRank, const glm::vec3& pos)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].pos = pos;
}

void CEngine::SetObjectShadowSpotAngle(int objRank, float angle)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].angle = angle;
}

void CEngine::SetObjectShadowSpotRadius(int objRank, float radius)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].radius = radius;
}

void CEngine::SetObjectShadowSpotIntensity(int objRank, float intensity)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].intensity = intensity;
}

void CEngine::SetObjectShadowSpotHeight(int objRank, float height)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    m_shadowSpots[shadowRank].height = height;
}

bool CEngine::GetHighlight(glm::vec2& p1, glm::vec2& p2)
{
    p1 = m_highlightP1;
    p2 = m_highlightP2;
    return m_highlight;
}

void CEngine::SetHighlightRank(int *rankList)
{
    int i = 0;
    while ( *rankList != -1 )
    {
        m_highlightRank[i++] = *rankList++;
    }
    m_highlightRank[i] = -1;  // terminator
}

bool CEngine::GetBBox2D(int objRank, glm::vec2& min, glm::vec2& max)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    min.x =  1000000.0f;
    min.y =  1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return false;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 p{ 0, 0, 0 };

        if ( i & (1<<0) )  p.x = p1.bboxMin.x;
        else               p.x = p1.bboxMax.x;
        if ( i & (1<<1) )  p.y = p1.bboxMin.y;
        else               p.y = p1.bboxMax.y;
        if ( i & (1<<2) )  p.z = p1.bboxMin.z;
        else               p.z = p1.bboxMax.z;

        glm::vec3 pp{ 0, 0, 0 };
        if (TransformPoint(pp, objRank, p))
        {
            if (pp.x < min.x) min.x = pp.x;
            if (pp.x > max.x) max.x = pp.x;
            if (pp.y < min.y) min.y = pp.y;
            if (pp.y > max.y) max.y = pp.y;
        }
    }

    if (min.x ==  1000000.0f ||
        min.y ==  1000000.0f ||
        max.x == -1000000.0f ||
        max.y == -1000000.0f)
        return false;

    return true;
}

void CEngine::DeleteAllGroundSpots()
{
    m_groundSpots.clear();
    m_firstGroundSpot = true;

    for (int s = 0; s < 16; s++)
    {
        CImage shadowImg(glm::ivec2(256, 256));
        shadowImg.Fill(Gfx::IntColor(255, 255, 255, 255));

        std::stringstream str;
        str << "shadow" << std::setfill('0') << std::setw(2) << s << ".png";
        std::string texName = str.str();

        CreateOrUpdateTexture(texName, &shadowImg);
    }
}

int CEngine::CreateGroundSpot()
{
    int index = 0;
    for ( ; index < static_cast<int>( m_groundSpots.size() ); index++)
    {
        if (! m_groundSpots[index].used)
        {
            m_groundSpots[index] = {};
            break;
        }
    }

    m_groundSpots.push_back(EngineGroundSpot());

    m_groundSpots[index].used = true;
    m_groundSpots[index].smooth = 1.0f;

    return index;
}

void CEngine::DeleteGroundSpot(int rank)
{
    assert(rank >= 0 && rank < static_cast<int>( m_groundSpots.size() ));

    m_groundSpots[rank].used = false;
    m_groundSpots[rank].pos = glm::vec3(0.0f, 0.0f, 0.0f);
}

void CEngine::SetObjectGroundSpotPos(int rank, const glm::vec3& pos)
{
    assert(rank >= 0 && rank < static_cast<int>( m_groundSpots.size() ));

    m_groundSpots[rank].pos = pos;
}

void CEngine::SetObjectGroundSpotRadius(int rank, float radius)
{
    assert(rank >= 0 && rank < static_cast<int>( m_groundSpots.size() ));

    m_groundSpots[rank].radius = radius;
}

void CEngine::SetObjectGroundSpotColor(int rank, const Color& color)
{
    assert(rank >= 0 && rank < static_cast<int>( m_groundSpots.size() ));

    m_groundSpots[rank].color = color;
}

void CEngine::SetObjectGroundSpotMinMax(int rank, float min, float max)
{
    assert(rank >= 0 && rank < static_cast<int>( m_groundSpots.size() ));

    m_groundSpots[rank].min = min;
    m_groundSpots[rank].max = max;
}

void CEngine::SetObjectGroundSpotSmooth(int rank, float smooth)
{
    assert(rank >= 0 && rank < static_cast<int>( m_groundSpots.size() ));

    m_groundSpots[rank].smooth = smooth;
}

void CEngine::CreateGroundMark(glm::vec3 pos, float radius,
                                   float delay1, float delay2, float delay3,
                                   int dx, int dy, char* table)
{
    m_groundMark = {};

    m_groundMark.draw      = true;
    m_groundMark.phase     = ENG_GR_MARK_PHASE_INC;
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

void CEngine::DeleteGroundMark(int rank)
{
    m_groundMark = {};
}

void CEngine::ComputeDistance()
{
    for (int i = 0; i < static_cast<int>( m_objects.size() ); i++)
    {
        if (! m_objects[i].used)
            continue;

        glm::vec3 v{};
        v.x = m_eyePt.x - m_objects[i].transform[3][0];
        v.y = m_eyePt.y - m_objects[i].transform[3][1];
        v.z = m_eyePt.z - m_objects[i].transform[3][2];
        m_objects[i].distance = glm::length(v);
    }
}

void CEngine::UpdateGeometry()
{
    if (! m_updateGeometry)
        return;

    for (auto& object : m_baseObjects)
    {
        if (!object.used)
            continue;

        object.bboxMin = { 0, 0, 0 };
        object.bboxMax = { 0, 0, 0 };

        for (auto& data : object.next)
        {
            for (const auto& vertex : data.vertices)
            {
                object.bboxMin.x = Math::Min(vertex.position.x, object.bboxMin.x);
                object.bboxMin.y = Math::Min(vertex.position.y, object.bboxMin.y);
                object.bboxMin.z = Math::Min(vertex.position.z, object.bboxMin.z);
                object.bboxMax.x = Math::Max(vertex.position.x, object.bboxMax.x);
                object.bboxMax.y = Math::Max(vertex.position.y, object.bboxMax.y);
                object.bboxMax.z = Math::Max(vertex.position.z, object.bboxMax.z);
            }
        }

        object.boundingSphere = Math::BoundingSphereForBox(object.bboxMin, object.bboxMax);
    }

    m_updateGeometry = false;
}

void CEngine::UpdateStaticBuffer(EngineBaseObjDataTier& p4)
{
    PrimitiveType type;
    if (p4.type == EngineTriangleType::TRIANGLES)
        type = PrimitiveType::TRIANGLES;
    else
        type = PrimitiveType::TRIANGLE_STRIP;

    if (p4.buffer == nullptr)
    {
        p4.buffer = m_device->CreateVertexBuffer(type, p4.vertices.data(), p4.vertices.size());
    }
    else
    {
        p4.buffer->SetType(type);

        if (p4.buffer->Size() != p4.vertices.size())
            p4.buffer->Resize(p4.vertices.size());

        p4.buffer->SetData(p4.vertices.data(), 0, p4.vertices.size());
        p4.buffer->Update();
    }

    p4.updateStaticBuffer = false;
}

void CEngine::UpdateStaticBuffers()
{
    if (!m_updateStaticBuffers)
        return;

    m_updateStaticBuffers = false;

    for (auto& object : m_baseObjects)
    {
        if (!object.used)
            continue;

        for (auto& data : object.next)
        {
            if (data.updateStaticBuffer)
                UpdateStaticBuffer(data);
        }
    }
}

void CEngine::Update()
{
    ComputeDistance();
    UpdateGeometry();
    UpdateStaticBuffers();
}

bool CEngine::DetectBBox(int objRank, const glm::vec2& mouse)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return false;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    glm::vec2 min, max;
    min.x =  1000000.0f;
    min.y =  1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 p{ 0, 0, 0 };

        if ( i & (1<<0) )  p.x = p1.bboxMin.x;
        else               p.x = p1.bboxMax.x;
        if ( i & (1<<1) )  p.y = p1.bboxMin.y;
        else               p.y = p1.bboxMax.y;
        if ( i & (1<<2) )  p.z = p1.bboxMin.z;
        else               p.z = p1.bboxMax.z;

        glm::vec3 pp{ 0, 0, 0 };
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

int CEngine::DetectObject(const glm::vec2& mouse, glm::vec3& targetPos, bool terrain)
{
    float min = 1000000.0f;
    int nearest = -1;
    glm::vec3 pos{ 0, 0, 0 };

    for (int objRank = 0; objRank < static_cast<int>( m_objects.size() ); objRank++)
    {
        if (! m_objects[objRank].used)
            continue;

        if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN && !terrain)
            continue;

        if (! DetectBBox(objRank, mouse))
            continue;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjDataTier& data = p1.next[l2];

            if (data.type == EngineTriangleType::TRIANGLES)
            {
                for (int i = 0; i < static_cast<int>(data.vertices.size()); i += 3)
                {
                    float dist = 0.0f;
                    if (DetectTriangle(mouse, &data.vertices[i], objRank, dist, pos) && dist < min)
                    {
                        min = dist;
                        nearest = objRank;
                        targetPos = pos;
                    }
                }
            }
            else if (data.type == EngineTriangleType::SURFACE)
            {
                for (int i = 0; i < static_cast<int>(data.vertices.size()) - 2; i += 1)
                {
                    float dist = 0.0f;
                    if (DetectTriangle(mouse, &data.vertices[i], objRank, dist, pos) && dist < min)
                    {
                        min = dist;
                        nearest = objRank;
                        targetPos = pos;
                    }
                }
            }
        }
    }

    return nearest;
}

bool CEngine::DetectTriangle(const glm::vec2& mouse, Vertex3D* triangle, int objRank, float& dist, glm::vec3& pos)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    glm::vec3 p2D[3], p3D{ 0, 0, 0 };

    for (int i = 0; i < 3; i++)
    {
        p3D.x = triangle[i].position.x;
        p3D.y = triangle[i].position.y;
        p3D.z = triangle[i].position.z;

        if (! TransformPoint(p2D[i], objRank, p3D))
            return false;
    }

    if (mouse.x < p2D[0].x &&
        mouse.x < p2D[1].x &&
        mouse.x < p2D[2].x)
        return false;

    if (mouse.x > p2D[0].x &&
        mouse.x > p2D[1].x &&
        mouse.x > p2D[2].x)
        return false;

    if (mouse.y < p2D[0].y &&
        mouse.y < p2D[1].y &&
        mouse.y < p2D[2].y)
        return false;

    if (mouse.y > p2D[0].y &&
        mouse.y > p2D[1].y &&
        mouse.y > p2D[2].y)
        return false;

    glm::vec2 a, b, c;
    a.x = p2D[0].x;
    a.y = p2D[0].y;
    b.x = p2D[1].x;
    b.y = p2D[1].y;
    c.x = p2D[2].x;
    c.y = p2D[2].y;

    if (! Math::IsInsideTriangle(a, b, c, mouse))
        return false;

    auto matViewInverse = glm::inverse(m_matView);
    auto matProjInverse = glm::inverse(m_matProj);

    glm::vec3 a2 = Math::Transform(m_objects[objRank].transform, triangle[0].position);
    glm::vec3 b2 = Math::Transform(m_objects[objRank].transform, triangle[1].position);
    glm::vec3 c2 = Math::Transform(m_objects[objRank].transform, triangle[2].position);
    glm::vec3 e  = Math::Transform(matViewInverse, glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 f  = Math::Transform(matViewInverse, glm::vec3(
        (mouse.x*2.0f-1.0f) * matProjInverse[0][0],
        (mouse.y*2.0f-1.0f) * matProjInverse[1][1],
        0.0f));
    Math::Intersect(a2, b2, c2, e, f, pos);

    dist = (p2D[0].z + p2D[1].z + p2D[2].z) / 3.0f;
    return true;
}

//! Use only after world transform already set
bool CEngine::IsVisible(const glm::mat4& matrix, int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return false;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

    const auto& sphere = m_baseObjects[baseObjRank].boundingSphere;
    if (ComputeSphereVisibility(matrix, sphere.pos, sphere.radius) == Gfx::FRUSTUM_PLANE_ALL)
    {
        m_objects[objRank].visible = true;
        return true;
    }

    m_objects[objRank].visible = false;
    return false;
}

int CEngine::ComputeSphereVisibility(const glm::mat4& m, const glm::vec3& center, float radius)
{
    glm::vec3 vec[6];
    float originPlane[6];

    // Left plane
    vec[0].x = m[0][3] + m[0][0];
    vec[0].y = m[1][3] + m[1][0];
    vec[0].z = m[2][3] + m[2][0];
    float l1 = glm::length(vec[0]);
    vec[0] = glm::normalize(vec[0]);
    originPlane[0] = (m[3][3] + m[3][0]) / l1;

    // Right plane
    vec[1].x = m[0][3] - m[0][0];
    vec[1].y = m[1][3] - m[1][0];
    vec[1].z = m[2][3] - m[2][0];
    float l2 = glm::length(vec[1]);
    vec[1] = glm::normalize(vec[1]);
    originPlane[1] = (m[3][3] - m[3][0]) / l2;

    // Bottom plane
    vec[2].x = m[0][3] + m[1][0];
    vec[2].y = m[1][3] + m[1][1];
    vec[2].z = m[3][2] + m[1][2];
    float l3 = glm::length(vec[2]);
    vec[2] = glm::normalize(vec[2]);
    originPlane[2] = (m[3][3] + m[3][1]) / l3;

    // Top plane
    vec[3].x = m[0][3] - m[0][1];
    vec[3].y = m[1][3] - m[1][1];
    vec[3].z = m[2][3] - m[2][1];
    float l4 = glm::length(vec[3]);
    vec[3] = glm::normalize(vec[3]);
    originPlane[3] = (m[3][3] - m[3][1]) / l4;

    // Front plane
    vec[4].x = m[0][3] + m[0][2];
    vec[4].y = m[1][3] + m[1][2];
    vec[4].z = m[2][3] + m[2][2];
    float l5 = glm::length(vec[4]);
    vec[4] = glm::normalize(vec[4]);
    originPlane[4] = (m[3][3] + m[3][2]) / l5;

    // Back plane
    vec[5].x = m[0][3] - m[0][2];
    vec[5].y = m[1][3] - m[1][2];
    vec[5].z = m[2][3] - m[2][2];
    float l6 = glm::length(vec[5]);
    vec[5] = glm::normalize(vec[5]);
    originPlane[5] = (m[3][3] - m[3][2]) / l6;

    int result = 0;

    if (InPlane(vec[0], originPlane[0], center, radius))
        result |= FRUSTUM_PLANE_LEFT;
    if (InPlane(vec[1], originPlane[1], center, radius))
        result |= FRUSTUM_PLANE_RIGHT;
    if (InPlane(vec[2], originPlane[2], center, radius))
        result |= FRUSTUM_PLANE_BOTTOM;
    if (InPlane(vec[3], originPlane[3], center, radius))
        result |= FRUSTUM_PLANE_TOP;
    if (InPlane(vec[4], originPlane[4], center, radius))
        result |= FRUSTUM_PLANE_FRONT;
    if (InPlane(vec[5], originPlane[5], center, radius))
        result |= FRUSTUM_PLANE_BACK;

    return result;
}

bool CEngine::InPlane(glm::vec3 normal, float originPlane, glm::vec3 center, float radius)
{
    float distance = originPlane + glm::dot(normal, center);

    if (distance < -radius)
        return false;

    return true;
}

bool CEngine::TransformPoint(glm::vec3& p2D, int objRank, glm::vec3 p3D)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    p3D = Math::Transform(m_objects[objRank].transform, p3D);
    p3D = Math::Transform(m_matView, p3D);

    if (p3D.z < 2.0f)
        return false;  // behind?

    p2D.x = (p3D.x/p3D.z)*m_matProj[0][0];
    p2D.y = (p3D.y/p3D.z)*m_matProj[1][1];
    p2D.z = p3D.z;

    p2D.x = (p2D.x+1.0f)/2.0f;  // [-1..1] -> [0..1]
    p2D.y = (p2D.y+1.0f)/2.0f;

    return true;
}



/*******************************************************
                    Mode setting
 *******************************************************/


void CEngine::SetViewParams(const glm::vec3 &eyePt, const glm::vec3 &lookatPt, const glm::vec3 &upVec)
{
    m_eyePt = eyePt;
    m_lookatPt = lookatPt;
    m_eyeDirH = Math::RotateAngle(eyePt.x - lookatPt.x, eyePt.z - lookatPt.z);
    m_eyeDirV = Math::RotateAngle(Math::DistanceProjected(eyePt, lookatPt), eyePt.y - lookatPt.y);

    Math::LoadViewMatrix(m_matView, eyePt, lookatPt, upVec);

    if (m_sound == nullptr)
        m_sound = m_app->GetSound();

    if (m_sound != nullptr)
        m_sound->SetListener(eyePt, lookatPt);
}

Texture CEngine::CreateTexture(const std::string& texName, const TextureCreateParams& params, CImage* image)
{
    if (texName.empty())
        return Texture(); // invalid texture

    if (m_texBlacklist.find(texName) != m_texBlacklist.end())
        return Texture(); // invalid texture

    Texture tex;
    CImage img;

    if (image == nullptr)
    {
        if (!img.Load(texName))
        {
            std::string error = img.GetError();
            GetLogger()->Error("Couldn't load texture '%%': %%, blacklisting", texName, error);
            m_texBlacklist.insert(texName);
            return Texture(); // invalid texture
        }

        image = &img;
    }

    tex = m_device->CreateTexture(image, params);

    if (! tex.Valid())
    {
        GetLogger()->Error("Couldn't load texture '%%', blacklisting", texName);
        m_texBlacklist.insert(texName);
        return tex;
    }

    m_texNameMap[texName] = tex;
    m_revTexNameMap[tex] = texName;

    return tex;
}

Texture CEngine::LoadTexture(const std::string& name)
{
    return LoadTexture(name, m_defaultTexParams);
}

Texture CEngine::LoadTexture(const std::string& name, CImage* image)
{
    Texture tex = CreateTexture(name, m_defaultTexParams, image);
    return tex;
}

Texture CEngine::LoadTexture(const std::string& name, const TextureCreateParams& params)
{
    if (m_texBlacklist.find(name) != m_texBlacklist.end())
        return Texture();

    std::map<std::string, Texture>::iterator it = m_texNameMap.find(name);
    if (it != m_texNameMap.end())
        return (*it).second;

    return CreateTexture(name, params);
}

bool CEngine::LoadAllTextures()
{
    m_miceTexture = LoadTexture("textures/interface/mouse.png");
    LoadTexture("textures/interface/button1.png");
    LoadTexture("textures/interface/button2.png");
    LoadTexture("textures/interface/button3.png");
    LoadTexture("textures/interface/button4.png");
    LoadTexture("textures/effect00.png");
    LoadTexture("textures/effect01.png");
    LoadTexture("textures/effect02.png");
    LoadTexture("textures/effect03.png");

    if (! m_backgroundName.empty())
    {
        TextureCreateParams params = m_defaultTexParams;
        params.padToNearestPowerOfTwo = true;
        m_backgroundTex = LoadTexture(m_backgroundName, params);
    }
    else
        m_backgroundTex.SetInvalid();

    if (!m_foregroundName.empty())
    {
        TextureCreateParams params = m_defaultTexParams;
        params.wrap = TextureWrapMode::CLAMP;
        params.filter = TextureFilter::BILINEAR;
        params.mipmap = false;
        m_foregroundTex = LoadTexture(m_foregroundName, params);
    }
    else
        m_foregroundTex.SetInvalid();

    m_planet->LoadTexture();

    bool ok = true;

    for (int objRank = 0; objRank < static_cast<int>( m_objects.size() ); objRank++)
    {
        if (! m_objects[objRank].used)
            continue;

        bool terrain = false;
        if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
            terrain = true;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        for (auto& data : p1.next)
        {
            if (!data.material.albedoTexture.empty())
            {
                if (terrain)
                    data.albedoTexture = LoadTexture("textures/" + data.material.albedoTexture, m_terrainTexParams);
                else
                    data.albedoTexture = LoadTexture("textures/" + data.material.albedoTexture);

                if (!data.albedoTexture.Valid())
                    ok = false;
            }

            if (!data.material.detailTexture.empty())
            {
                if (terrain)
                    data.detailTexture = LoadTexture("textures/" + data.material.detailTexture, m_terrainTexParams);
                else
                    data.detailTexture = LoadTexture("textures/" + data.material.detailTexture);

                if (!data.detailTexture.Valid())
                    ok = false;
            }

            if (!data.material.materialTexture.empty())
            {
                if (terrain)
                    data.materialTexture = LoadTexture("textures/" + data.material.materialTexture, m_terrainTexParams);
                else
                    data.materialTexture = LoadTexture("textures/" + data.material.materialTexture);

                if (!data.materialTexture.Valid())
                    ok = false;
            }

            if (!data.material.emissiveTexture.empty())
            {
                if (terrain)
                    data.emissiveTexture = LoadTexture("textures/" + data.material.emissiveTexture, m_terrainTexParams);
                else
                    data.emissiveTexture = LoadTexture("textures/" + data.material.emissiveTexture);

                if (!data.emissiveTexture.Valid())
                    ok = false;
            }
        }
    }

    return ok;
}

static bool IsExcludeColor(glm::vec2* exclude, int x, int y)
{
    int i = 0;
    while ( exclude[i+0].x != 0.0f || exclude[i+0].y != 0.0f ||
            exclude[i+1].y != 0.0f || exclude[i+1].y != 0.0f )
    {
        if ( x >= static_cast<int>(exclude[i+0].x*256.0f) &&
             x <  static_cast<int>(exclude[i+1].x*256.0f) &&
             y >= static_cast<int>(exclude[i+0].y*256.0f) &&
             y <  static_cast<int>(exclude[i+1].y*256.0f) )
            return true;  // exclude

        i += 2;
    }

    return false;  // point to include
}

void CEngine::DeleteTexture(const std::string& texName)
{
    auto it = m_texNameMap.find(texName);
    if (it == m_texNameMap.end())
        return;

    auto revIt = m_revTexNameMap.find((*it).second);

    m_device->DestroyTexture((*it).second);

    m_revTexNameMap.erase(revIt);
    m_texNameMap.erase(it);
}

void CEngine::DeleteTexture(const Texture& tex)
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

void CEngine::CreateOrUpdateTexture(const std::string& texName, CImage* img)
{
    auto it = m_texNameMap.find(texName);
    if (it == m_texNameMap.end())
    {
        LoadTexture(texName, img);
    }
    else
    {
        m_device->UpdateTexture((*it).second, { 0, 0 }, img->GetData(), m_defaultTexParams.format);
    }
}

void CEngine::FlushTextureCache()
{
    m_device->DestroyAllTextures();

    m_backgroundTex.SetInvalid();
    m_foregroundTex.SetInvalid();

    m_texNameMap.clear();
    m_revTexNameMap.clear();
    m_texBlacklist.clear();

    m_firstGroundSpot = true;
}

void CEngine::SetTerrainVision(float vision)
{
    m_terrainVision = vision;
}

void CEngine::SetFocus(float focus)
{
    m_focus = focus;
    m_size = m_app->GetVideoConfig().size;

    float farPlane = m_deepView[0] * m_clippingDistance;

    float aspect = static_cast<float>(m_size.x) / static_cast<float>(m_size.y);

    // Compute H-FoV from V-FoV and aspect ratio.
    m_hfov = 2.0f * atan(aspect * tan(focus / 2.0f));

    Math::LoadProjectionMatrix(m_matProj, m_focus, aspect, 0.5f, farPlane);
}

float CEngine::GetFocus()
{
    return m_focus;
}

float CEngine::GetVFovAngle()
{
    return m_focus;
}

float CEngine::GetHFovAngle()
{
    return m_hfov;
}

void CEngine::SetShadowColor(float value)
{
    m_shadowColor = value;
}

float CEngine::GetShadowColor()
{
    return m_shadowColor;
}

void CEngine::SetShadowRange(float value)
{
    m_shadowRange = value;
}

float CEngine::GetShadowRange()
{
    return m_shadowRange;
}

void CEngine::SetMultiSample(int value)
{
    if(value == m_multisample) return;
    m_multisample = value;
    m_device->DeleteFramebuffer("multisample");
}

int CEngine::GetMultiSample()
{
    return m_multisample;
}

void CEngine::SetTriplanarMode(bool enabled)
{
    m_triplanarMode = enabled;
}

bool CEngine::GetTriplanarMode()
{
    return m_triplanarMode;
}

void CEngine::SetTriplanarScale(float scale)
{
    m_triplanarScale = scale;
}

float CEngine::GetTriplanarScale()
{
    return m_triplanarScale;
}

void CEngine::SetDirty(bool mode)
{
    m_dirty = mode;
}

bool CEngine::GetDirty()
{
    return m_dirty;
}

void CEngine::SetFog(bool mode)
{
    m_fog = mode;
}

bool CEngine::GetFog()
{
    return m_fog;
}

void CEngine::SetSecondTexture(const std::string& texNum)
{
    m_secondTex = texNum;
}

const std::string& CEngine::GetSecondTexture()
{
    return m_secondTex;
}

void CEngine::SetRankView(int rank)
{
    if (rank < 0) rank = 0;
    if (rank > 1) rank = 1;

    if (m_rankView == 0 && rank == 1)  // enters the water?
        m_lightMan->AdaptLightColor(m_waterAddColor, +1.0f);

    if (m_rankView == 1 && rank == 0)  // out of the water?
        m_lightMan->AdaptLightColor(m_waterAddColor, -1.0f);

    m_rankView = rank;
}

int CEngine::GetRankView()
{
    return m_rankView;
}

void CEngine::SetDrawWorld(bool draw)
{
    m_drawWorld = draw;
}

void CEngine::SetDrawFront(bool draw)
{
    m_drawFront = draw;
}

void CEngine::SetAmbientColor(const Color& color, int rank)
{
    m_ambientColor[rank] = color;
}

Color CEngine::GetAmbientColor(int rank)
{
    return m_ambientColor[rank];
}

void CEngine::SetWaterAddColor(const Color& color)
{
    m_waterAddColor = color;
}

Color CEngine::GetWaterAddColor()
{
    return m_waterAddColor;
}

void CEngine::SetFogColor(const Color& color, int rank)
{
    m_fogColor[rank] = color;
}

Color CEngine::GetFogColor(int rank)
{
    return m_fogColor[rank];
}

void CEngine::SetDeepView(float length, int rank, bool ref)
{
    if (ref)
        length *= m_clippingDistance;

    m_deepView[rank] = length;
}

float CEngine::GetDeepView(int rank)
{
    return m_deepView[rank];
}

void CEngine::SetFogStart(float start, int rank)
{
    if (start < 0.0f)
        m_fogStart[rank] = 0.0f;
    else
        m_fogStart[rank] = start;
}

float CEngine::GetFogStart(int rank)
{
    return m_fogStart[rank];
}

void CEngine::SetBackground(const std::string& name, Color up, Color down,
                            Color cloudUp, Color cloudDown, bool full, bool scale)
{
    if (m_backgroundTex.Valid() && name != m_backgroundName)
    {
        DeleteTexture(m_backgroundTex);
        m_backgroundTex.SetInvalid();
    }

    m_backgroundName      = name;
    m_backgroundColorUp   = up;
    m_backgroundColorDown = down;
    m_backgroundCloudUp   = cloudUp;
    m_backgroundCloudDown = cloudDown;
    m_backgroundFull      = full;
    m_backgroundScale     = scale;

    if (! m_backgroundName.empty() && !m_backgroundTex.Valid())
    {
        TextureCreateParams params = m_defaultTexParams;
        params.padToNearestPowerOfTwo = true;
        m_backgroundTex = LoadTexture(m_backgroundName, params);
    }
}

void CEngine::GetBackground(std::string& name, Color& up, Color& down,
                            Color& cloudUp, Color& cloudDown, bool &full, bool &scale)
{
    name      = m_backgroundName;
    up        = m_backgroundColorUp;
    down      = m_backgroundColorDown;
    cloudUp   = m_backgroundCloudUp;
    cloudDown = m_backgroundCloudDown;
    full      = m_backgroundFull;
    scale     = m_backgroundScale;
}

void CEngine::SetForegroundName(const std::string& name)
{
    if (m_foregroundTex.Valid() && name != m_foregroundName)
    {
        DeleteTexture(m_foregroundTex);
        m_foregroundTex.SetInvalid();
    }

    m_foregroundName = name;

    if (!m_foregroundName.empty() && !m_foregroundTex.Valid())
    {
        TextureCreateParams params;
        params.wrap = TextureWrapMode::CLAMP;
        params.filter = TextureFilter::BILINEAR;
        params.mipmap = false;
        m_foregroundTex = LoadTexture(m_foregroundName, params);
    }
}

void CEngine::SetOverFront(bool front)
{
    m_overFront = front;
}

void CEngine::SetOverColor(const Color& color, TransparencyMode mode)
{
    m_overColor = color;
    m_overMode  = mode;
}

void CEngine::SetParticleDensity(float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 2.0f) value = 2.0f;
    m_particleDensity = value;
}

float CEngine::GetParticleDensity()
{
    return m_particleDensity;
}

float CEngine::ParticleAdapt(float factor)
{
    if (m_particleDensity == 0.0f)
        return 1000000.0f;

    return factor / m_particleDensity;
}

void CEngine::SetClippingDistance(float value)
{
    if (value < 0.5f) value = 0.5f;
    if (value > 2.0f) value = 2.0f;
    m_clippingDistance = value;
}

float CEngine::GetClippingDistance()
{
    return m_clippingDistance;
}

void CEngine::SetTextureFilterMode(TextureFilter value)
{
    if(m_defaultTexParams.filter == value && m_terrainTexParams.filter == value) return;

    m_defaultTexParams.filter = m_terrainTexParams.filter = value;
    m_defaultTexParams.mipmap = m_terrainTexParams.mipmap = (value == TextureFilter::TRILINEAR);
    ReloadAllTextures();
}

TextureFilter CEngine::GetTextureFilterMode()
{
    return m_terrainTexParams.filter;
}

void CEngine::SetTextureMipmapLevel(int value)
{
    if (value < 1) value = 1;
    if (value > 16) value = 16;
    if(m_textureMipmapLevel == value) return;

    m_textureMipmapLevel = value;
    ReloadAllTextures();
}

int CEngine::GetTextureMipmapLevel()
{
    return m_textureMipmapLevel;
}

void CEngine::SetTextureAnisotropyLevel(int value)
{
    if (value < 1) value = 1;
    if (value > 16) value = 16;

    if(m_textureAnisotropy == value) return;

    m_textureAnisotropy = value;
    ReloadAllTextures();
}

int CEngine::GetTextureAnisotropyLevel()
{
    return m_textureAnisotropy;
}

bool CEngine::IsShadowMappingSupported()
{
    return true;
}

void CEngine::SetShadowMapping(bool value)
{
    if(!IsShadowMappingSupported()) value = false;
    if(value == m_shadowMapping) return;
    m_shadowMapping = value;
    if(!value)
    {
        m_device->DeleteFramebuffer("shadow");
        m_device->DestroyTexture(m_shadowMap);
        m_shadowMap.id = 0;
    }
}

bool CEngine::GetShadowMapping()
{
    return m_shadowMapping;
}

void CEngine::SetShadowMappingOffscreen(bool value)
{
    if(!m_device->IsFramebufferSupported()) value = false;
    if(value == m_offscreenShadowRendering) return;
    m_offscreenShadowRendering = value;
    if(value)
    {
        m_device->DestroyTexture(m_shadowMap);
        m_shadowMap.id = 0;
    }
    else
    {
        m_device->DeleteFramebuffer("shadow");
        m_shadowMap.id = 0;
    }
}

bool CEngine::GetShadowMappingOffscreen()
{
    return m_offscreenShadowRendering;
}

void CEngine::SetShadowMappingOffscreenResolution(int resolution)
{
    resolution = Math::Min(resolution, m_device->GetMaxTextureSize());
    if(resolution == m_offscreenShadowRenderingResolution) return;
    m_offscreenShadowRenderingResolution = resolution;
    m_device->DeleteFramebuffer("shadow");
    m_shadowMap.id = 0;
}

int CEngine::GetShadowMappingOffscreenResolution()
{
    return m_offscreenShadowRenderingResolution;
}

bool CEngine::IsShadowMappingQualitySupported()
{
    return true;
}

void CEngine::SetShadowMappingQuality(bool value)
{
    if(!IsShadowMappingQualitySupported()) value = false;
    m_qualityShadows = value;
}

bool CEngine::GetShadowMappingQuality()
{
    return m_qualityShadows;
}

void CEngine::SetTerrainShadows(bool value)
{
    m_terrainShadows = value;
}

bool CEngine::GetTerrainShadows()
{
    return m_terrainShadows;
}

void CEngine::SetVSync(int value)
{
    if (value < -1) value = -1;
    if (value > 1) value = 1;
    if(m_vsync == value) return;
    m_vsync = value;
}

int CEngine::GetVSync()
{
    return m_vsync;
}

void CEngine::SetBackForce(bool present)
{
    m_backForce = present;
}

bool CEngine::GetBackForce()
{
    return m_backForce;
}

void CEngine::SetLightMode(bool present)
{
    m_lightMode = present;
}

bool CEngine::GetLightMode()
{
    return m_lightMode;
}

void CEngine::SetEditIndentMode(bool autoIndent)
{
    m_editIndentMode = autoIndent;
}

bool CEngine::GetEditIndentMode()
{
    return m_editIndentMode;
}

void CEngine::SetEditIndentValue(int value)
{
    m_editIndentValue = value;
}

int CEngine::GetEditIndentValue()
{
    return m_editIndentValue;
}

void CEngine::SetTracePrecision(float factor)
{
    m_tracePrecision = factor;
}

float CEngine::GetTracePrecision()
{
    return m_tracePrecision;
}

void CEngine::SetMouseType(EngineMouseType type)
{
    m_mouseType = type;
}

EngineMouseType CEngine::GetMouseType()
{
    return m_mouseType;
}

void CEngine::SetPauseBlurEnabled(bool enable)
{
    m_pauseBlurEnabled = enable;
}

bool CEngine::GetPauseBlurEnabled()
{
    return m_pauseBlurEnabled;
}

const glm::mat4& CEngine::GetMatView()
{
    return m_matView;
}

const glm::mat4& CEngine::GetMatProj()
{
    return m_matProj;
}

glm::vec3 CEngine::GetEyePt()
{
    return m_eyePt;
}

glm::vec3 CEngine::GetLookatPt()
{
    return m_lookatPt;
}

float CEngine::GetEyeDirH()
{
    return m_eyeDirH;
}

float CEngine::GetEyeDirV()
{
    return m_eyeDirV;
}

bool CEngine::IsVisiblePoint(const glm::vec3 &pos)
{
    return glm::distance(m_eyePt, pos) <= (m_deepView[0] * m_clippingDistance);
}

Color CEngine::GetObjectColor(int object, const std::string& name)
{
    if (name == "team")
    {
        return CRobotMain::GetInstance().GetTeamColor(m_objects[object].team);
    }
    else if (name == "vehicle")
    {
        return CRobotMain::GetInstance().GetVehicleColor();
    }
    else if (name == "plant")
    {
        return CRobotMain::GetInstance().GetGreeneryColor();
    }
    else if (name == "alien")
    {
        return CRobotMain::GetInstance().GetAlienColor();
    }
    else if (name == "hair")
    {
        const auto& appearance = CRobotMain::GetInstance().GetPlayerProfile()->GetAppearance();

        return appearance.colorHair;
    }
    else if (name == "suit")
    {
        const auto& appearance = CRobotMain::GetInstance().GetPlayerProfile()->GetAppearance();

        return appearance.colorCombi;
    }
    else if (name == "band")
    {
        const auto& appearance = CRobotMain::GetInstance().GetPlayerProfile()->GetAppearance();

        return appearance.colorBand;
    }
    else
    {
        return Color(1.0, 1.0, 1.0, 1.0);
    }
}

void CEngine::ApplyChange()
{
    SetFocus(m_focus);

    // recapture 3D scene
    if (m_worldCaptured)
    {
        m_captureWorld = true;
        m_worldCaptured = false;
    }
}

/*******************************************************
                      Rendering
 *******************************************************/



/**
  This function sets up render states, clears the
  viewport, and renders the scene. */
void CEngine::Render()
{
    m_fpsCounter++;

    m_currentFrameTime = m_systemUtils->GetCurrentTimeStamp();
    float diff = TimeUtils::Diff(m_lastFrameTime, m_currentFrameTime, TimeUnit::SECONDS);
    if (diff > 1.0f)
    {
        m_lastFrameTime = m_currentFrameTime;

        m_fps = m_fpsCounter / diff;
        m_fpsCounter = 0;
    }

    if (! m_render)
        return;

    m_statisticTriangle = 0;

    m_lightMan->UpdateLights();

    Color color;
    if (m_cloud->GetLevel() != 0.0f)  // clouds?
        color = m_backgroundCloudDown;
    else
        color = m_backgroundColorDown;

    m_device->SetClearColor(color);

    // Begin the scene
    m_device->SetDepthMask(true);

    m_device->BeginScene();

    // use currently captured scene for world
    if (m_worldCaptured && !m_captureWorld)
    {
        DrawCaptured3DScene();
    }
    else
    {
        // Render shadow map
        if (m_drawWorld && m_shadowMapping)
            RenderShadowMap();

        UseMSAA(true);

        DrawBackground();                // draws the background

        if (m_drawWorld)
            Draw3DScene();

        UseMSAA(false);

        // marked to capture currently rendered world
        if (m_captureWorld)
        {
            Capture3DScene();
            m_device->Clear();
            DrawCaptured3DScene();
        }
    }

    CProfiler::StartPerformanceCounter(PCNT_RENDER_INTERFACE);
    DrawInterface();
    CProfiler::StopPerformanceCounter(PCNT_RENDER_INTERFACE);

    // End the scene
    m_device->EndScene();
}

void CEngine::Draw3DScene()
{
    if (!m_worldCaptured)
    {
        if (m_capturedWorldTexture.Valid())
        {
            m_device->DestroyTexture(m_capturedWorldTexture);
            m_capturedWorldTexture = Texture();
        }
    }

    m_device->SetDepthTest(false);

    UpdateGroundSpotTextures();

    DrawPlanet();  // draws the planets
    m_cloud->Draw();  // draws the clouds

    // Display the objects

    //m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, true);
    //m_device->SetRenderState(RENDER_STATE_LIGHTING, true);
    //m_device->SetRenderState(RENDER_STATE_FOG, true);

    float fogStart = m_deepView[m_rankView] * m_fogStart[m_rankView] * m_clippingDistance;
    float fogEnd = m_deepView[m_rankView] * m_clippingDistance;

    // TODO: This causes a rendering artifact and I can't see anything that breaks if you just comment it out
    // So I'll just leave it like that for now ~krzys_h
    //m_water->DrawBack();  // draws water background

    CProfiler::StartPerformanceCounter(PCNT_RENDER_TERRAIN);

    // Draw terrain

    //m_lightMan->UpdateDeviceLights(ENG_OBJTYPE_TERRAIN);

    Gfx::ShadowParam shadowParams[4];
    for (int i = 0; i < m_shadowRegions; i++)
    {
        shadowParams[i].matrix = m_shadowParams[i].transform;
        shadowParams[i].uv_offset = m_shadowParams[i].offset;
        shadowParams[i].uv_scale = m_shadowParams[i].scale;
    }

    auto terrainRenderer = m_device->GetTerrainRenderer();
    terrainRenderer->Begin();

    terrainRenderer->SetProjectionMatrix(m_matProj);
    terrainRenderer->SetViewMatrix(m_matView);
    terrainRenderer->SetShadowMap(m_shadowMap);
    terrainRenderer->SetLight(glm::vec4(1.0, 1.0, -1.0, 0.0), 1.0f, glm::vec3(1.0));
    terrainRenderer->SetSky(Color(1.0, 1.0, 1.0), 0.2f);
    
    if (m_shadowMapping)
        terrainRenderer->SetShadowParams(m_shadowRegions, shadowParams);
    else
        terrainRenderer->SetShadowParams(0, nullptr);

    Color fogColor = m_fogColor[m_rankView];

    terrainRenderer->SetFog(fogStart, fogEnd, { fogColor.r, fogColor.g, fogColor.b });

    glm::mat4 scale = glm::mat4(1.0f);
    scale[2][2] = -1.0f;
    auto projectionViewMatrix = m_matProj * scale;
    projectionViewMatrix = projectionViewMatrix * m_matView;

    for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
    {
        if (! m_objects[objRank].used)
            continue;

        if (m_objects[objRank].type != ENG_OBJTYPE_TERRAIN)
            continue;

        if (! m_objects[objRank].drawWorld)
            continue;

        auto combinedMatrix = projectionViewMatrix * m_objects[objRank].transform;

        if (! IsVisible(combinedMatrix, objRank))
            continue;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        for (auto& data : p1.next)
        {
            terrainRenderer->SetAlbedoColor(data.material.albedoColor);
            terrainRenderer->SetAlbedoTexture(data.albedoTexture);
            terrainRenderer->SetDetailTexture(data.detailTexture);

            terrainRenderer->SetEmissiveColor(data.material.emissiveColor);
            terrainRenderer->SetEmissiveTexture(data.emissiveTexture);

            terrainRenderer->SetMaterialParams(data.material.roughness, data.material.metalness, data.material.aoStrength);
            terrainRenderer->SetMaterialTexture(data.materialTexture);

            terrainRenderer->DrawObject(m_objects[objRank].transform, data.buffer);
        }
    }

    terrainRenderer->End();

    // Draws the old-style shadow spots, if shadow mapping disabled
    if (!m_shadowMapping)
        DrawShadowSpots();

    CProfiler::StopPerformanceCounter(PCNT_RENDER_TERRAIN);

    // Draw other objects

    CProfiler::StartPerformanceCounter(PCNT_RENDER_OBJECTS);

    auto objectRenderer = m_device->GetObjectRenderer();
    objectRenderer->Begin();

    objectRenderer->SetProjectionMatrix(m_matProj);
    objectRenderer->SetViewMatrix(m_matView);
    objectRenderer->SetShadowMap(m_shadowMap);
    objectRenderer->SetLighting(true);
    objectRenderer->SetLight(glm::vec4(1.0, 1.0, -1.0, 0.0), 0.8f, glm::vec3(1.0));
    objectRenderer->SetSky(Color(1.0, 1.0, 1.0), 0.2f);
    objectRenderer->SetTransparency(TransparencyMode::NONE);

    objectRenderer->SetFog(fogStart, fogEnd, { fogColor.r, fogColor.g, fogColor.b });
    objectRenderer->SetAlphaScissor(0.0f);

    if (m_shadowMapping)
        objectRenderer->SetShadowParams(m_shadowRegions, shadowParams);
    else
        objectRenderer->SetShadowParams(0, nullptr);

    objectRenderer->SetTriplanarMode(m_triplanarMode);
    objectRenderer->SetTriplanarScale(m_triplanarScale);

    bool transparent = false;

    for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
    {
        if (! m_objects[objRank].used)
            continue;

        if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
            continue;

        if (! m_objects[objRank].drawWorld)
            continue;

        auto combinedMatrix = projectionViewMatrix * m_objects[objRank].transform;

        if (! IsVisible(combinedMatrix, objRank))
            continue;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        objectRenderer->SetModelMatrix(m_objects[objRank].transform);

        //m_lightMan->UpdateDeviceLights(m_objects[objRank].type);

        for (auto& data : p1.next)
        {
            if (m_objects[objRank].ghost)  // transparent ?
            {
                transparent = true;
                continue;
            }

            if (data.material.alphaMode != AlphaMode::NONE)
            {
                objectRenderer->SetAlphaScissor(data.material.alphaThreshold);
            }
            else
            {
                objectRenderer->SetAlphaScissor(0.0f);
            }

            Color color = data.material.albedoColor;

            if (!data.material.tag.empty())
            {
                Color c = GetObjectColor(objRank, data.material.tag);

                if (c != Color(1.0, 1.0, 1.0, 1.0))
                {
                    color = c;
                }
            }

            if (data.material.recolor.empty())
            {
                objectRenderer->SetRecolor(false);
            }
            else
            {
                Color recolorFrom = data.material.recolorReference;
                Color recolorTo = GetObjectColor(objRank, data.material.recolor);
                float recolorThreshold = 0.1;

                objectRenderer->SetRecolor(true, recolorFrom, recolorTo, recolorThreshold);
            }

            objectRenderer->SetAlbedoColor(color);
            objectRenderer->SetAlbedoTexture(data.albedoTexture);
            objectRenderer->SetDetailTexture(data.detailTexture);

            objectRenderer->SetEmissiveColor(data.material.emissiveColor);
            objectRenderer->SetEmissiveTexture(data.emissiveTexture);

            objectRenderer->SetMaterialParams(data.material.roughness, data.material.metalness, data.material.aoStrength);
            objectRenderer->SetMaterialTexture(data.materialTexture);

            objectRenderer->SetCullFace(data.material.cullFace);
            objectRenderer->SetUVTransform(data.uvOffset, data.uvScale);
            objectRenderer->DrawObject(data.buffer);
        }
    }

    objectRenderer->End();

    objectRenderer->Begin();
    objectRenderer->SetLighting(false);
    objectRenderer->SetDepthMask(false);
    objectRenderer->SetTransparency(TransparencyMode::BLACK);
    objectRenderer->SetAlphaScissor(0.0f);
    objectRenderer->SetCullFace(CullFace::NONE);

    // Draw transparent objects

    if (transparent)
    {
        Color tColor = Color(68.0f / 255.0f, 68.0f / 255.0f, 68.0f / 255.0f, 255.0f);

        for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
        {
            if (! m_objects[objRank].used)
                continue;

            if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
                continue;

            if (! m_objects[objRank].drawWorld)
                continue;

            if (!m_objects[objRank].ghost)
                continue;

            auto combinedMatrix = projectionViewMatrix * m_objects[objRank].transform;

            if (! IsVisible(combinedMatrix, objRank))
                continue;

            int baseObjRank = m_objects[objRank].baseObjRank;
            if (baseObjRank == -1)
                continue;

            assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

            EngineBaseObject& p1 = m_baseObjects[baseObjRank];
            if (! p1.used)
                continue;

            objectRenderer->SetModelMatrix(m_objects[objRank].transform);

            for (auto& data : p1.next)
            {
                objectRenderer->SetAlbedoColor(tColor);
                objectRenderer->SetAlbedoTexture(data.albedoTexture);
                objectRenderer->SetDetailTexture(data.detailTexture);
                objectRenderer->SetUVTransform(data.uvOffset, data.uvScale);
                objectRenderer->DrawObject(data.buffer);
            }
        }
    }

    objectRenderer->End();

    CProfiler::StopPerformanceCounter(PCNT_RENDER_OBJECTS);

    m_lightMan->UpdateDeviceLights(ENG_OBJTYPE_TERRAIN);

    if (m_debugDumpLights)
    {
        m_debugDumpLights = false;
        m_lightMan->DebugDumpLights();
    }

    CProfiler::StartPerformanceCounter(PCNT_RENDER_WATER);

    objectRenderer->Begin();

    objectRenderer->SetProjectionMatrix(m_matProj);
    objectRenderer->SetViewMatrix(m_matView);
    objectRenderer->SetShadowMap(m_shadowMap);
    objectRenderer->SetLighting(true);
    objectRenderer->SetLight(glm::vec4(1.0, 1.0, -1.0, 0.0), 1.0f, glm::vec3(1.0));
    objectRenderer->SetTransparency(TransparencyMode::NONE);

    objectRenderer->SetFog(fogStart, fogEnd, { fogColor.r, fogColor.g, fogColor.b });
    objectRenderer->SetAlphaScissor(0.0f);
    objectRenderer->SetShadowParams(m_shadowRegions, shadowParams);

    m_water->DrawSurf(); // draws water surface

    CProfiler::StopPerformanceCounter(PCNT_RENDER_WATER);

    RenderPendingDebugDraws();

    if (m_debugGoto)
    {
        glm::mat4 worldMatrix = glm::mat4(1.0f);
        objectRenderer->SetTransparency(TransparencyMode::NONE);
        objectRenderer->SetLighting(false);
        objectRenderer->SetModelMatrix(glm::mat4(1.0f));

        for (const auto& line : m_displayGoto)
        {
            objectRenderer->DrawPrimitive(PrimitiveType::LINE_STRIP, line.size(), line.data());
        }
    }
    m_displayGoto.clear();

    objectRenderer->End();

    auto particleRenderer = m_device->GetParticleRenderer();
    particleRenderer->Begin();
    particleRenderer->SetProjectionMatrix(m_matProj);
    particleRenderer->SetViewMatrix(m_matView);

    CProfiler::StartPerformanceCounter(PCNT_RENDER_PARTICLE_WORLD);
    m_particle->DrawParticle(SH_WORLD); // draws the particles of the 3D world
    CProfiler::StopPerformanceCounter(PCNT_RENDER_PARTICLE_WORLD);

    m_lightning->Draw();                     // draws lightning

    particleRenderer->End();

    DrawForegroundImage();   // draws the foreground

    if (! m_overFront) DrawOverColor();      // draws the foreground color
}

void CEngine::Capture3DScene()
{
    // destroy existing texture
    if (m_capturedWorldTexture.Valid())
    {
        m_device->DestroyTexture(m_capturedWorldTexture);
        m_capturedWorldTexture = Texture();
    }

    // obtain pixels from screen
    int width = m_size.x;
    int height = m_size.y;

    auto pixels = m_device->GetFrameBufferPixels();
    unsigned char* data = reinterpret_cast<unsigned char*>(pixels->GetPixelsData());

    // calculate 2nd mipmap
    int newWidth = width / 4;
    int newHeight = height / 4;
    std::unique_ptr<unsigned char[]> mipmap = std::make_unique<unsigned char[]>(4 * newWidth * newHeight);

    for (int x = 0; x < newWidth; x++)
    {
        for (int y = 0; y < newHeight; y++)
        {
            float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    int index = 4 * ((4 * x + i) + width * (4 * y + j));

                    for (int k = 0; k < 4; k++)
                        color[k] += data[index + k];
                }
            }

            int index = 4 * (x + newWidth * y);

            for (int k = 0; k < 4; k++)
            {
                mipmap[index + k] = static_cast<unsigned char>(color[k] * (1.0f / 16.0f));
            }
        }
    }

    // calculate Gaussian blur
    std::unique_ptr<unsigned char[]> blured = std::make_unique<unsigned char[]>(4 * newWidth * newHeight);

    float matrix[7][7] =
        {
            { 0.00000067f, 0.00002292f, 0.00019117f, 0.00038771f, 0.00019117f, 0.00002292f, 0.00000067f },
            { 0.00002292f, 0.00078634f, 0.00655965f, 0.01330373f, 0.00655965f, 0.00078633f, 0.00002292f },
            { 0.00019117f, 0.00655965f, 0.05472157f, 0.11098164f, 0.05472157f, 0.00655965f, 0.00019117f },
            { 0.00038771f, 0.01330373f, 0.11098164f, 0.22508352f, 0.11098164f, 0.01330373f, 0.00038771f },
            { 0.00019117f, 0.00655965f, 0.05472157f, 0.11098164f, 0.05472157f, 0.00655965f, 0.00019117f },
            { 0.00002292f, 0.00078633f, 0.00655965f, 0.01330373f, 0.00655965f, 0.00078633f, 0.00002292f },
            { 0.00000067f, 0.00002292f, 0.00019117f, 0.00038771f, 0.00019117f, 0.00002292f, 0.00000067f }
        };

    for (int x = 0; x < newWidth; x++)
    {
        for (int y = 0; y < newHeight; y++)
        {
            float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

            for (int i = -3; i <= 3; i++)
            {
                for (int j = -3; j <= 3; j++)
                {
                    int xp = glm::clamp(x + i, 0, newWidth - 1);
                    int yp = glm::clamp(y + j, 0, newHeight - 1);

                    float weight = matrix[i + 3][j + 3];

                    int index = 4 * (newWidth * yp + xp);

                    for (int k = 0; k < 4; k++)
                        color[k] += weight * mipmap[index + k];
                }
            }

            int index = 4 * (newWidth * y + x);

            for (int k = 0; k < 4; k++)
            {
                float value = glm::clamp(color[k], 0.0f, 255.0f);
                blured[index + k] = static_cast<unsigned char>(value);
            }
        }
    }

    // create SDL surface and final texture
    ImageData image;
    image.surface = SDL_CreateRGBSurfaceFrom(blured.get(), newWidth, newHeight,
        32, 4 * newWidth, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    TextureCreateParams params;
    params.filter = TextureFilter::BILINEAR;
    params.format = TextureFormat::RGBA;
    params.mipmap = false;

    m_capturedWorldTexture = m_device->CreateTexture(&image, params);

    SDL_FreeSurface(image.surface);

    m_captureWorld = false;
    m_worldCaptured = true;
}

void CEngine::DrawCaptured3DScene()
{
    m_device->SetDepthTest(false);

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTexture(m_capturedWorldTexture);
    renderer->SetTransparency(TransparencyMode::NONE);
    auto vertices = renderer->BeginPrimitive(Gfx::PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
    vertices[1] = { { 1.0f, 0.0f }, { 1.0f, 0.0f } };
    vertices[2] = { { 0.0f, 1.0f }, { 0.0f, 1.0f } };
    vertices[3] = { { 1.0f, 1.0f }, { 1.0f, 1.0f } };

    renderer->EndPrimitive();
}

void CEngine::RenderDebugSphere(const Math::Sphere& sphere, const glm::mat4& transform, const Gfx::Color& color)
{
    static constexpr int LONGITUDE_DIVISIONS = 16;
    static constexpr int LATITUDE_DIVISIONS = 8;
    static constexpr int NUM_LINE_STRIPS = 2 + LONGITUDE_DIVISIONS + LATITUDE_DIVISIONS;
    static constexpr int VERTS_IN_LINE_STRIP = 32;

    static std::array<glm::vec3, NUM_LINE_STRIPS * VERTS_IN_LINE_STRIP> verticesTemplate = []
    {
        std::array<glm::vec3, NUM_LINE_STRIPS * VERTS_IN_LINE_STRIP> vertices;

        auto SpherePoint = [&](float latitude, float longitude)
        {
            float latitudeAngle = (latitude - 0.5f) * 2.0f * Math::PI;
            float longitudeAngle = longitude * 2.0f * Math::PI;
            return glm::vec3(sinf(latitudeAngle) * cosf(longitudeAngle),
                                cosf(latitudeAngle),
                                sinf(latitudeAngle) * sinf(longitudeAngle));
        };

        auto vert = vertices.begin();

        for (int longitudeDivision = 0; longitudeDivision <= LONGITUDE_DIVISIONS; ++longitudeDivision)
        {
            for (int segment = 0; segment < VERTS_IN_LINE_STRIP; ++segment)
            {
                float latitude = static_cast<float>(segment) / VERTS_IN_LINE_STRIP;
                float longitude = static_cast<float>(longitudeDivision) / (LONGITUDE_DIVISIONS);
                *vert++ = SpherePoint(latitude, longitude);
            }
        }

        for (int latitudeDivision = 0; latitudeDivision <= LATITUDE_DIVISIONS; ++latitudeDivision)
        {
            for (int segment = 0; segment < VERTS_IN_LINE_STRIP; ++segment)
            {
                float latitude = static_cast<float>(latitudeDivision + 1) / (LATITUDE_DIVISIONS + 2);
                float longitude = static_cast<float>(segment) / VERTS_IN_LINE_STRIP;
                *vert++ = SpherePoint(latitude, longitude);
            }
        }
        return vertices;
    }();


    const int firstDraw = m_pendingDebugDraws.counts.size();
    const int firstVert = m_pendingDebugDraws.vertices.size();

    m_pendingDebugDraws.counts.resize(m_pendingDebugDraws.counts.size() + NUM_LINE_STRIPS);
    m_pendingDebugDraws.vertices.resize(m_pendingDebugDraws.vertices.size() + verticesTemplate.size());

    for (int i = 0; i < NUM_LINE_STRIPS; ++i)
    {
        m_pendingDebugDraws.counts[i + firstDraw] = VERTS_IN_LINE_STRIP;
    }

    for (std::size_t i = 0; i < verticesTemplate.size(); ++i)
    {
        auto pos = Math::Transform(transform, sphere.pos + verticesTemplate[i] * sphere.radius);
        m_pendingDebugDraws.vertices[i + firstVert] = Vertex3D{ pos, {}, color };
    }
}

void CEngine::RenderDebugBox(const glm::vec3& mins, const glm::vec3& maxs, const glm::mat4& transform, const Gfx::Color& color)
{
    static constexpr int NUM_LINE_STRIPS = 4;
    static constexpr int VERTS_IN_LINE_STRIP = 4;

    const int firstDraw = m_pendingDebugDraws.counts.size();
    const int firstVert = m_pendingDebugDraws.vertices.size();

    m_pendingDebugDraws.counts.resize(m_pendingDebugDraws.counts.size() + NUM_LINE_STRIPS);
    m_pendingDebugDraws.vertices.resize(m_pendingDebugDraws.vertices.size() + NUM_LINE_STRIPS * VERTS_IN_LINE_STRIP);

    for (int i = 0; i < NUM_LINE_STRIPS; ++i)
    {
        m_pendingDebugDraws.counts[i + firstDraw] = NUM_LINE_STRIPS;
    }

    auto vert = m_pendingDebugDraws.vertices.begin() + firstVert;

    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, mins.y, mins.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, mins.y, mins.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, maxs.y, mins.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, maxs.y, maxs.z}), {}, color };

    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, mins.y, maxs.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, mins.y, mins.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, maxs.y, mins.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, maxs.y, mins.z}), {}, color };

    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, mins.y, maxs.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, mins.y, maxs.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, maxs.y, maxs.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, maxs.y, mins.z}), {}, color };

    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, mins.y, mins.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, mins.y, maxs.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{maxs.x, maxs.y, maxs.z}), {}, color };
    *vert++ = Vertex3D{ Math::Transform(transform, glm::vec3{mins.x, maxs.y, maxs.z}), {}, color };
}

void CEngine::RenderPendingDebugDraws()
{
    if (m_pendingDebugDraws.counts.empty()) return;

    auto renderer = m_device->GetObjectRenderer();
    renderer->SetTransparency(TransparencyMode::NONE);
    renderer->SetLighting(false);
    renderer->SetModelMatrix(glm::mat4(1.0f));
    renderer->SetAlbedoTexture(Texture{});
    renderer->SetDetailTexture(Texture{});

    renderer->DrawPrimitives(PrimitiveType::LINE_STRIP,
        m_pendingDebugDraws.counts.size(),
        m_pendingDebugDraws.counts.data(),
        m_pendingDebugDraws.vertices.data());

    m_pendingDebugDraws.counts.clear();
    m_pendingDebugDraws.vertices.clear();
}

void CEngine::RenderShadowMap()
{
    m_shadowMapping = m_shadowMapping && m_device->IsShadowMappingSupported();
    m_offscreenShadowRendering = m_offscreenShadowRendering && m_device->IsFramebufferSupported();
    m_offscreenShadowRenderingResolution = Math::Min(m_offscreenShadowRenderingResolution, m_device->GetMaxTextureSize());

    if (!m_shadowMapping) return;

    CProfiler::StartPerformanceCounter(PCNT_RENDER_SHADOW_MAP);

    if (m_qualityShadows)
    {
        m_shadowRegions = 4;

        m_shadowParams[0].range = 16.0;
        m_shadowParams[0].offset = { 0.0, 0.0 };
        m_shadowParams[0].scale = { 0.5, 0.5 };

        m_shadowParams[1].range = 64.0;
        m_shadowParams[1].offset = { 0.5, 0.0 };
        m_shadowParams[1].scale = { 0.5, 0.5 };

        m_shadowParams[2].range = 256.0;
        m_shadowParams[2].offset = { 0.0, 0.5 };
        m_shadowParams[2].scale = { 0.5, 0.5 };

        m_shadowParams[3].range = 1024.0;
        m_shadowParams[3].offset = { 0.5, 0.5 };
        m_shadowParams[3].scale = { 0.5, 0.5 };
    }
    else
    {
        m_shadowRegions = 1;

        m_shadowParams[0].range = 256.0;
        m_shadowParams[0].offset = { 0.0, 0.0 };
        m_shadowParams[0].scale = { 1.0, 1.0 };
    }

    // If no shadow map texture exists, create it
    if (m_shadowMap.id == 0)
    {
        m_shadowMap = m_device->CreateDepthTexture(
            m_offscreenShadowRenderingResolution,
            m_offscreenShadowRenderingResolution,
            32);

        GetLogger()->Info("Created shadow map texture: %%x%%, depth %%\n",
            m_shadowMap.size.x, m_shadowMap.size.y, 32);
    }

    auto renderer = m_device->GetShadowRenderer();
    renderer->Begin();
    renderer->SetShadowMap(m_shadowMap);
    renderer->SetShadowRegion({ 0.0, 0.0 }, { 1.0, 1.0 });

    m_device->Clear();

    for (int region = 0; region < m_shadowRegions; region++)
    {
        renderer->SetShadowRegion(
            m_shadowParams[region].offset,
            m_shadowParams[region].scale);

        // recompute matrices
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        glm::vec3 lightDir = glm::vec3(1.0f, 2.0f, -1.0f);
        glm::vec3 dir = m_lookatPt - m_eyePt;
        dir.y = 0.0f;
        dir = glm::normalize(dir);

        float range = m_shadowParams[region].range;

        float dist = range;
        float depth = 200.0f;

        if (dist < 0.5f)
        {
            float scale = log(m_shadowMap.size.x) / log(2.0f) - 6.5f;
            dist = 75.0f * scale;
        }

        glm::vec3 pos = m_lookatPt + 0.25f * dist * dir;

        {
            // To prevent 'shadow shimmering', we ensure that the position only moves in texel-sized
            // increments. To do this we transform the position to a space where the light's forward/right/up
            // axes are aligned with the x/y/z axes (not necessarily in that order, and +/- signs don't matter).
            glm::mat4 lightRotation;
            Math::LoadViewMatrix(lightRotation, glm::vec3{0, 0, 0}, lightDir, worldUp);
            pos = Math::Transform(lightRotation, pos);
            // ...then we round to the nearest worldUnitsPerTexel:
            const float worldUnitsPerTexel = (dist * 2.0f) / m_shadowMap.size.x;
            pos /= worldUnitsPerTexel;
            pos.x = round(pos.x);
            pos.y = round(pos.y);
            pos.z = round(pos.z);
            pos *= worldUnitsPerTexel;
            // ...and convert back to world space.
            pos = Math::Transform(glm::inverse(lightRotation), pos);
        }

        glm::vec3 lookAt = pos - lightDir;

        Math::LoadOrthoProjectionMatrix(m_shadowProjMat, -dist, dist, -dist, dist, -depth, depth);
        Math::LoadViewMatrix(m_shadowViewMat, pos, lookAt, worldUp);

        glm::mat4 scaleMat;
        Math::LoadScaleMatrix(scaleMat, glm::vec3(1.0f, 1.0f, -1.0f));
        m_shadowViewMat = scaleMat * m_shadowViewMat;

        glm::mat4 temporary = m_shadowProjMat * m_shadowViewMat;
        m_shadowTextureMat = m_shadowBias * temporary;

        m_shadowViewMat = scaleMat * m_shadowViewMat;

        auto projectionViewMatrix = m_shadowProjMat * m_shadowViewMat;

        m_shadowParams[region].transform = m_shadowTextureMat;

        renderer->SetProjectionMatrix(m_shadowProjMat);
        renderer->SetViewMatrix(m_shadowViewMat);

        // render objects into shadow map
        for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
        {
            if (!m_objects[objRank].used)
                continue;

            bool terrain = (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN);

            if (terrain && !m_terrainShadows) continue;

            auto combinedMatrix = projectionViewMatrix * m_objects[objRank].transform;

            if (!IsVisible(combinedMatrix, objRank))
                continue;

            int baseObjRank = m_objects[objRank].baseObjRank;
            if (baseObjRank == -1)
                continue;

            assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

            EngineBaseObject& p1 = m_baseObjects[baseObjRank];
            if (!p1.used)
                continue;

            renderer->SetModelMatrix(m_objects[objRank].transform);

            for (auto& data : p1.next)
            {
                renderer->SetTexture(data.albedoTexture);

                renderer->DrawObject(data.buffer, true);
            }
        }
    }

    renderer->End();

    // restore default state
    m_device->SetViewport(0, 0, m_size.x, m_size.y);

    m_device->SetColorMask(true, true, true, true);
    m_device->Clear();

    CProfiler::StopPerformanceCounter(PCNT_RENDER_SHADOW_MAP);

    m_device->SetDepthTest(false);
}

void CEngine::UseMSAA(bool enable)
{
    m_multisample = Math::Min(m_device->GetMaxSamples(), m_multisample);
    if (m_multisample < 2) return;

    if (enable)
    {
        if (m_multisample > 1)
        {
            CFramebuffer* framebuffer = m_device->GetFramebuffer("multisample");

            if (framebuffer == nullptr)
            {
                CFramebuffer* screen = m_device->GetFramebuffer("default");

                FramebufferParams params;
                params.width = screen->GetWidth();
                params.height = screen->GetHeight();
                params.depth = 24;
                params.samples = m_multisample;

                framebuffer = m_device->CreateFramebuffer("multisample", params);

                if (framebuffer == nullptr)
                {
                    GetLogger()->Error("Could not create MSAA framebuffer, disabling MSAA");
                    m_multisample = 1;
                }
            }

            if (framebuffer != nullptr)
            {
                framebuffer->Bind();
            }

            m_device->SetDepthTest(true);
            m_device->SetDepthMask(true);

            m_device->Clear();
        }
    }
    else
    {
        if (m_multisample > 1)
        {
            CFramebuffer* framebuffer = m_device->GetFramebuffer("multisample");
            framebuffer->Unbind();

            CFramebuffer* screen = m_device->GetFramebuffer("default");

            int width = screen->GetWidth();
            int height = screen->GetHeight();

            framebuffer->CopyToScreen(0, 0, width, height, 0, 0, width, height);
        }
    }
}

void CEngine::DrawInterface()
{
    m_device->SetDepthTest(false);
    m_device->SetTransparency(TransparencyMode::NONE);

    SetInterfaceCoordinates();

    // Force new state to disable lighting
    m_interfaceMode = true;

    // Draw the entire interface
    Ui::CInterface* interface = CRobotMain::GetInstancePointer()->GetInterface();
    if (interface != nullptr && m_renderInterface)
    {
        interface->Draw();
    }

    m_interfaceMode = false;

    if (!m_screenshotMode && m_renderInterface)
    {
        auto particleRenderer = m_device->GetParticleRenderer();
        particleRenderer->Begin();
        particleRenderer->SetProjectionMatrix(m_matProjInterface);
        particleRenderer->SetViewMatrix(m_matViewInterface);
        particleRenderer->SetModelMatrix(m_matWorldInterface);

        CProfiler::StartPerformanceCounter(PCNT_RENDER_PARTICLE_IFACE);
        m_particle->DrawParticle(SH_INTERFACE);  // draws the particles of the interface
        CProfiler::StopPerformanceCounter(PCNT_RENDER_PARTICLE_IFACE);

        particleRenderer->End();
    }

    // 3D objects drawn in front of interface
    if (m_drawFront)
    {
        float fogStart = m_deepView[m_rankView] * m_fogStart[m_rankView] * m_clippingDistance;
        float fogEnd = m_deepView[m_rankView] * m_clippingDistance;
        Color fogColor = m_fogColor[m_rankView];

        auto renderer = m_device->GetObjectRenderer();
        renderer->Begin();
        renderer->SetProjectionMatrix(m_matProj);
        renderer->SetViewMatrix(m_matView);
        renderer->SetFog(fogStart, fogEnd, { fogColor.r, fogColor.g, fogColor.b });
        renderer->SetLighting(true);
        renderer->SetLight(glm::vec4(1.0, 1.0, -1.0, 0.0), 0.8f, glm::vec3(1.0));
        renderer->SetSky(Color(1.0, 1.0, 1.0), 0.2f);
        renderer->SetTransparency(TransparencyMode::NONE);
        renderer->SetAlphaScissor(0.0f);
        renderer->SetShadowParams(0, nullptr);
        renderer->SetAlbedoColor(Color{ 1.0f, 1.0f, 1.0f, 1.0f });
        renderer->SetCullFace(CullFace::BACK);

        renderer->SetTriplanarMode(m_triplanarMode);
        renderer->SetTriplanarScale(m_triplanarScale);

        auto projectionViewMatrix = m_matProj * m_matView;

        for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
        {
            if (! m_objects[objRank].used)
                continue;

            if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
                continue;

            if (! m_objects[objRank].drawFront)
                continue;

            auto combinedMatrix = projectionViewMatrix * m_objects[objRank].transform;

            //if (! IsVisible(combinedMatrix, objRank))
            //    continue;

            int baseObjRank = m_objects[objRank].baseObjRank;
            if (baseObjRank == -1)
                continue;

            assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

            EngineBaseObject& p1 = m_baseObjects[baseObjRank];
            if (! p1.used)
                continue;

            renderer->SetModelMatrix(m_objects[objRank].transform);

            //m_lightMan->UpdateDeviceLights(m_objects[objRank].type);

            for (auto& data : p1.next)
            {
                Color color = data.material.albedoColor;

                if (!data.material.tag.empty())
                {
                    Color c = GetObjectColor(objRank, data.material.tag);

                    if (c != Color(1.0, 1.0, 1.0, 1.0))
                    {
                        color = c;
                    }
                }

                if (data.material.recolor.empty())
                {
                    renderer->SetRecolor(false);
                }
                else
                {
                    Color recolorFrom = data.material.recolorReference;
                    Color recolorTo = GetObjectColor(objRank, data.material.recolor);
                    float recolorThreshold = 0.3;

                    renderer->SetRecolor(true, recolorFrom, recolorTo, recolorThreshold);
                }

                renderer->SetAlbedoColor(color);
                renderer->SetAlbedoTexture(data.albedoTexture);
                renderer->SetDetailTexture(data.detailTexture);

                renderer->DrawObject(data.buffer);
            }
        }

        renderer->End();

        auto particleRenderer = m_device->GetParticleRenderer();
        particleRenderer->Begin();
        particleRenderer->SetProjectionMatrix(m_matProj);
        particleRenderer->SetViewMatrix(m_matView);

        m_particle->DrawParticle(SH_FRONT);  // draws the particles of the 3D world

        particleRenderer->End();

        m_device->SetDepthTest(false);
    }

    SetInterfaceCoordinates();

    // Draw foreground color
    if (m_overFront)
        DrawOverColor();

    // At the end to not overlap
    if (m_renderInterface)
        DrawHighlight();

    DrawTimer();
    DrawStats();

    if (m_renderInterface)
        DrawMouse();
}

void CEngine::UpdateGroundSpotTextures()
{
    if (!m_firstGroundSpot                                   &&
        m_groundMark.drawPos.x     == m_groundMark.pos.x     &&
        m_groundMark.drawPos.z     == m_groundMark.pos.z     &&
        m_groundMark.drawRadius    == m_groundMark.radius    &&
        m_groundMark.drawIntensity == m_groundMark.intensity)
        return;

    for (int s = 0; s < 16; s++)
    {
        glm::vec2 min, max;
        min.x = (s%4) * 254.0f - 1.0f;  // 1 pixel cover
        min.y = (s/4) * 254.0f - 1.0f;
        max.x = min.x + 254.0f + 2.0f;
        max.y = min.y + 254.0f + 2.0f;

        bool clear = false;
        bool set   = false;

        // Calculate the area to be erased.
        int dot = static_cast<int>(m_groundMark.drawRadius/2.0f);

        float tu, tv;
        float cx, cy;

        tu = (m_groundMark.drawPos.x+1600.0f)/3200.0f;
        tv = (m_groundMark.drawPos.z+1600.0f)/3200.0f;  // 0..1

        cx = (tu*254.0f*4.0f)-0.5f;
        cy = (tv*254.0f*4.0f)-0.5f;

        if (dot == 0)
        {
            cx += 0.5f;
            cy += 0.5f;
        }

        float px = cx-Math::Mod(cx, 1.0f);
        float py = cy-Math::Mod(cy, 1.0f);  // multiple of 1

        if (m_firstGroundSpot ||
            (m_groundMark.drawRadius != 0.0f    &&
             px+dot >= min.x && py+dot >= min.y &&
             px-dot <= max.x && py-dot <= max.y))
        {
            clear = true;
        }

        // Calculate the area to draw.
        dot = static_cast<int>(m_groundMark.radius/2.0f);

        tu = (m_groundMark.pos.x+1600.0f)/3200.0f;
        tv = (m_groundMark.pos.z+1600.0f)/3200.0f;  // 0..1

        cx = (tu*254.0f*4.0f)-0.5f;
        cy = (tv*254.0f*4.0f)-0.5f;

        if ( dot == 0 )
        {
            cx += 0.5f;
            cy += 0.5f;
        }

        px = cx - Math::Mod(cx, 1.0f);
        py = cy - Math::Mod(cy, 1.0f);  // multiple of 1

        if (m_groundMark.draw &&
            px+dot >= min.x && py+dot >= min.y &&
            px-dot <= max.x && py-dot <= max.y)
        {
            set = true;
        }

        if (clear || set || m_debugResources || m_displayGotoImage != nullptr)
        {
            CImage shadowImg(glm::ivec2(256, 256));
            shadowImg.Fill(Gfx::IntColor(255, 255, 255, 255));

            // Draw the new shadows.
            for (int i = 0; i < static_cast<int>( m_groundSpots.size() ); i++)
            {
                if (m_groundSpots[i].used == false ||
                    m_groundSpots[i].radius == 0.0f)
                    continue;

                if (m_groundSpots[i].min == 0.0f &&
                    m_groundSpots[i].max == 0.0f)
                {
                    dot = static_cast<int>(m_groundSpots[i].radius/2.0f);

                    tu = (m_groundSpots[i].pos.x+1600.0f)/3200.0f;
                    tv = (m_groundSpots[i].pos.z+1600.0f)/3200.0f;  // 0..1

                    cx = (tu*254.0f*4.0f) - 0.5f;
                    cy = (tv*254.0f*4.0f) - 0.5f;

                    if (dot == 0)
                    {
                        cx += 0.5f;
                        cy += 0.5f;
                    }

                    px = cx-Math::Mod(cx, 1.0f);
                    py = cy-Math::Mod(cy, 1.0f);  // multiple of 1

                    if (px+dot < min.x || py+dot < min.y ||
                        px-dot > max.x || py-dot > max.y)
                        continue;

                    for (int iy = -dot; iy <= dot; iy++)
                    {
                        for (int ix =- dot; ix <= dot; ix++)
                        {
                            float ppx = px+ix;
                            float ppy = py+iy;

                            if (ppx <  min.x || ppy <  min.y ||
                                ppx >= max.x || ppy >= max.y)
                                continue;

                            float intensity;
                            if (dot == 0)
                                intensity = 0.0f;
                            else
                                intensity = glm::length(glm::vec2(ppx-cx, ppy-cy)) / dot;

                            ppx -= min.x;  // on the texture
                            ppy -= min.y;
                            glm::ivec2 pp(ppx, ppy);

                            Gfx::Color color = shadowImg.GetPixel(pp);
                            color.r *= Math::Norm(m_groundSpots[i].color.r+intensity);
                            color.g *= Math::Norm(m_groundSpots[i].color.g+intensity);
                            color.b *= Math::Norm(m_groundSpots[i].color.b+intensity);
                            shadowImg.SetPixel(pp, color);
                        }
                    }
                }
                else
                {
                    for (int iy = 0; iy < 256; iy++)
                    {
                        for (int ix = 0; ix < 256; ix++)
                        {
                            glm::vec3 pos{};
                            pos.x = (256.0f * (s%4) + ix) * 3200.0f/1024.0f - 1600.0f;
                            pos.z = (256.0f * (s/4) + iy) * 3200.0f/1024.0f - 1600.0f;
                            pos.y = 0.0f;

                            float level = m_terrain->GetFloorLevel(pos, true);
                            if (level < m_groundSpots[i].min ||
                                level > m_groundSpots[i].max)
                                continue;

                            float intensity;
                            if (level > (m_groundSpots[i].max+m_groundSpots[i].min)/2.0f)
                                intensity = 1.0f - (m_groundSpots[i].max-level) / m_groundSpots[i].smooth;
                            else
                                intensity = 1.0f - (level-m_groundSpots[i].min) / m_groundSpots[i].smooth;

                            if (intensity < 0.0f) intensity = 0.0f;

                            glm::ivec2 pp(ix, iy);

                            Gfx::Color color = shadowImg.GetPixel(pp);
                            color.r *= Math::Norm(m_groundSpots[i].color.r+intensity);
                            color.g *= Math::Norm(m_groundSpots[i].color.g+intensity);
                            color.b *= Math::Norm(m_groundSpots[i].color.b+intensity);
                            shadowImg.SetPixel(pp, color);
                        }
                    }
                }
            }

            if (set)
            {
                dot = static_cast<int>(m_groundMark.radius/2.0f);

                tu = (m_groundMark.pos.x + 1600.0f) / 3200.0f;
                tv = (m_groundMark.pos.z + 1600.0f) / 3200.0f;  // 0..1

                cx = (tu*254.0f*4.0f)-0.5f;
                cy = (tv*254.0f*4.0f)-0.5f;

                if (dot == 0)
                {
                    cx += 0.5f;
                    cy += 0.5f;
                }

                px = cx-Math::Mod(cx, 1.0f);
                py = cy-Math::Mod(cy, 1.0f);  // multiple of 1

                for (int iy = -dot; iy <= dot; iy++)
                {
                    for (int ix = -dot; ix <= dot; ix++)
                    {
                        float ppx = px+ix;
                        float ppy = py+iy;

                        if (ppx <  min.x || ppy <  min.y ||
                            ppx >= max.x || ppy >= max.y)
                            continue;

                        ppx -= min.x;  // on the texture
                        ppy -= min.y;

                        float intensity = 1.0f - glm::length(glm::vec2(ix, iy)) / dot;
                        if (intensity <= 0.0f)
                            continue;

                        intensity *= m_groundMark.intensity;

                        int j = (ix+dot) + (iy+dot) * m_groundMark.dx;
                        if (m_groundMark.table[j] == 1)  // green ?
                        {
                            glm::ivec2 pp(ppx, ppy);
                            Gfx::Color color = shadowImg.GetPixel(pp);
                            color.r *= Math::Norm(1.0f-intensity);
                            color.g *= 1.0f;
                            color.b *= Math::Norm(1.0f-intensity);
                            shadowImg.SetPixel(pp, color);
                        }
                        if (m_groundMark.table[j] == 2)  // red ?
                        {
                            glm::ivec2 pp(ppx, ppy);
                            Gfx::Color color = shadowImg.GetPixel(pp);
                            color.r *= 1.0f;
                            color.g *= Math::Norm(1.0f-intensity);
                            color.b *= Math::Norm(1.0f-intensity);
                            shadowImg.SetPixel(pp, color);
                        }
                    }
                }
            }

            if (m_debugResources)
            {
                for (float x = min.x; x < max.x; x += 1.0f)
                {
                    for (float y = min.y; y < max.y; y += 1.0f)
                    {
                        glm::vec3 pos(
                            x / 4.0f / 254.0f * 3200.0f - 1600.0f,
                            0.0f,
                            y / 4.0f / 254.0f * 3200.0f - 1600.0f
                        );
                        TerrainRes res = m_terrain->GetResource(pos);
                        glm::ivec2 p(x-min.x, y-min.y);
                        if (res == TR_NULL)
                        {
                            shadowImg.SetPixel(p, Gfx::Color(0.5f, 0.5f, 0.5f));
                            continue;
                        }
                        shadowImg.SetPixelInt(p, ResourceToColor(res));
                    }
                }
            }

            if (m_displayGotoImage != nullptr)
            {
                glm::ivec2 size = m_displayGotoImage->GetSize();
                for (float x = min.x; x < max.x; x += 1.0f)
                {
                    for (float y = min.y; y < max.y; y += 1.0f)
                    {
                        int px = x / 4.0f / 254.0f * size.x;
                        int py = y / 4.0f / 254.0f * size.y;
                        // This can happen because the shadow??.png textures have a 1 pixel margin around them
                        if (px < 0 || px >= size.x || py < 0 || py >= size.y)
                            continue;
                        auto color = m_displayGotoImage->GetPixelInt({ px, py });
                        shadowImg.SetPixelInt({ x - min.x, y - min.y }, color);
                    }
                }
            }

            std::stringstream str;
            str << "textures/shadow" << std::setfill('0') << std::setw(2) << s << ".png";
            std::string texName = str.str();

            CreateOrUpdateTexture(texName, &shadowImg);
        }
    }

    for (int i = 0; i < static_cast<int>( m_groundSpots.size() ); i++)
    {
        if (m_groundSpots[i].used == false ||
            m_groundSpots[i].radius == 0.0f)
        {
            m_groundSpots[i].drawRadius = 0.0f;
        }
        else
        {
            m_groundSpots[i].drawPos    = m_groundSpots[i].pos;
            m_groundSpots[i].drawRadius = m_groundSpots[i].radius;
        }
    }

    m_groundMark.drawPos       = m_groundMark.pos;
    m_groundMark.drawRadius    = m_groundMark.radius;
    m_groundMark.drawIntensity = m_groundMark.intensity;

    m_firstGroundSpot = false;
}

void CEngine::DrawShadowSpots()
{
    m_device->SetDepthMask(false);

    glm::mat4 matrix = glm::mat4(1.0f);
    //m_device->SetTransform(TRANSFORM_WORLD, matrix);

    // TODO: create a separate texture
    //SetTexture("textures/effect03.png");

    glm::vec2 ts, ti;

    float dp = 0.5f/256.0f;
    ts.y = 192.0f/256.0f;
    ti.y = 224.0f/256.0f;
    ts.y += dp;
    ti.y -= dp;

    glm::vec3 n(0.0f, 1.0f, 0.0f);

    float startDeepView = m_deepView[m_rankView] * m_fogStart[m_rankView] * m_clippingDistance;
    float endDeepView = m_deepView[m_rankView] * m_clippingDistance;

    float lastIntensity = -1.0f;
    for (int i = 0; i < static_cast<int>( m_shadowSpots.size() ); i++)
    {
        if (m_shadowSpots[i].hide || !m_shadowSpots[i].used)
            continue;

        glm::vec3 pos = m_shadowSpots[i].pos;  // pos = center of the shadow on the ground

        if (m_eyePt.y == pos.y)
            continue;  // camera at the same level?

        float d = 0.0f;
        float D = 0.0f;

        // h is the height above the ground to which the shadow
        // will be drawn.
        if (m_eyePt.y > pos.y)  // camera on?
        {
            float height = m_eyePt.y-pos.y;
            float h = m_shadowSpots[i].radius;
            float max = height*0.5f;
            if ( h > max  )  h = max;
            if ( h > 4.0f )  h = 4.0f;

            D = glm::distance(m_eyePt, pos);
            if (D >= endDeepView)
                continue;

            d = D*h/height;

            pos.x += (m_eyePt.x-pos.x)*d/D;
            pos.z += (m_eyePt.z-pos.z)*d/D;
            pos.y += h;
        }
        else    // camera underneath?
        {
            float height = pos.y-m_eyePt.y;
            float h = m_shadowSpots[i].radius;
            float max = height*0.1f;
            if ( h > max  )  h = max;
            if ( h > 4.0f )  h = 4.0f;

            D = glm::distance(m_eyePt, pos);
            if (D >= endDeepView)
                continue;

            d = D*h/height;

            pos.x += (m_eyePt.x-pos.x)*d/D;
            pos.z += (m_eyePt.z-pos.z)*d/D;
            pos.y -= h;
        }

        // The hFactor decreases the intensity and size increases more
        // the object is high relative to the ground.
        float hFactor = m_shadowSpots[i].height/20.0f;
        if ( hFactor < 0.0f )  hFactor = 0.0f;
        if ( hFactor > 1.0f )  hFactor = 1.0f;
        hFactor = powf(1.0f-hFactor, 2.0f);
        if ( hFactor < 0.2f )  hFactor = 0.2f;

        float radius = m_shadowSpots[i].radius*1.5f;
        radius *= 2.0f-hFactor;  // greater if high
        radius *= 1.0f-d/D;  // smaller if close


        glm::vec3 corner[4];

        if (m_shadowSpots[i].type == EngineShadowType::NORMAL)
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
            glm::vec2 rot;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, { radius, radius });
            corner[0].x = rot.x;
            corner[0].z = rot.y;
            corner[0].y = 0.0f;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, { -radius, radius });
            corner[1].x = rot.x;
            corner[1].z = rot.y;
            corner[1].y = 0.0f;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, { radius, -radius });
            corner[2].x = rot.x;
            corner[2].z = rot.y;
            corner[2].y = 0.0f;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, { -radius, -radius });
            corner[3].x = rot.x;
            corner[3].z = rot.y;
            corner[3].y = 0.0f;

            if (m_shadowSpots[i].type == EngineShadowType::WORM)
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

        corner[0] = glm::cross(corner[0], m_shadowSpots[i].normal);
        corner[1] = glm::cross(corner[1], m_shadowSpots[i].normal);
        corner[2] = glm::cross(corner[2], m_shadowSpots[i].normal);
        corner[3] = glm::cross(corner[3], m_shadowSpots[i].normal);

        corner[0] += pos;
        corner[1] += pos;
        corner[2] += pos;
        corner[3] += pos;

        ts.x += dp;
        ti.x -= dp;

        IntColor white(255, 255, 255, 255);

        Vertex3D vertex[4] =
        {
            { corner[1], white, { ts.x, ts.y } },
            { corner[0], white, { ti.x, ts.y } },
            { corner[3], white, { ts.x, ti.y } },
            { corner[2], white, { ti.x, ti.y } }
        };

        float intensity = (0.5f+m_shadowSpots[i].intensity*0.5f)*hFactor;

        // Decreases the intensity of the shade if you're in the area
        // between the beginning and the end of the fog.
        if ( D > startDeepView )
            intensity *= 1.0f-(D-startDeepView)/(endDeepView-startDeepView);

        if (intensity == 0.0f)
            continue;

        if (lastIntensity != intensity)  // intensity changed?
        {
            lastIntensity = intensity;
            //SetState(ENG_RSTATE_TTEXTURE_WHITE, Color(intensity, intensity, intensity, intensity));
        }

        //m_device->DrawPrimitive(PrimitiveType::TRIANGLE_STRIP, vertex, 4);
        AddStatisticTriangle(2);
    }

    m_device->SetDepthMask(true);
}

void CEngine::DrawBackground()
{
    if (m_cloud->GetLevel() != 0.0f)  // clouds ?
    {
        if (m_backgroundCloudUp != m_backgroundCloudDown)  // degraded?
            DrawBackgroundGradient(m_backgroundCloudUp, m_backgroundCloudDown);
    }
    else
    {
        if (m_backgroundColorUp != m_backgroundColorDown)  // degraded?
            DrawBackgroundGradient(m_backgroundColorUp, m_backgroundColorDown);
    }

    if (m_backForce || !m_backgroundName.empty() )
    {
        DrawBackgroundImage();  // image
    }
}

void CEngine::DrawBackgroundGradient(const Color& up, const Color& down)
{
    glm::vec2 p1(0.0f, 0.0f);
    glm::vec2 p2(1.0f, 1.0f);

    auto up_int = ColorToIntColor(up);
    auto down_int = ColorToIntColor(down);

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTexture(Texture{});
    renderer->SetColor({ 1, 1, 1, 1 });
    renderer->SetTransparency(TransparencyMode::NONE);
    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { p1.x, p1.y }, {}, down_int };
    vertices[1] = { { p1.x, p2.y }, {}, up_int };
    vertices[2] = { { p2.x, p1.y }, {}, down_int };
    vertices[3] = { { p2.x, p2.y }, {}, up_int };

    renderer->EndPrimitive();
    AddStatisticTriangle(2);
}

void CEngine::DrawBackgroundImage()
{
    glm::vec2 p1, p2;
    p1.x = 0.0f;
    p1.y = 0.0f;
    p2.x = 1.0f;
    p2.y = 1.0f;

    glm::vec3 n = glm::vec3(0.0f, 0.0f, -1.0f);  // normal

    float u1, u2, v1, v2;
    if (m_backgroundFull)
    {
        u1 = 0.0f;
        v1 = 0.0f;
        u2 = 1.0f;
        v2 = 1.0f;
    }
    else
    {
        float h = 0.5f;  // visible area vertically (1=all)
        float a = m_eyeDirV-Math::PI*0.15f;
        if (a >  Math::PI     )  a -= Math::PI*2.0f;  // a = -Math::PI..Math::PI
        if (a >  Math::PI/4.0f)  a =  Math::PI/4.0f;
        if (a < -Math::PI/4.0f)  a = -Math::PI/4.0f;

        // Note the background covers Math::PI radians, i.e. it repeats twice per rotation!
        u1 = (-m_eyeDirH - GetHFovAngle()/2.0f) / Math::PI;
        u2 = u1 + (GetHFovAngle() / Math::PI);

        v1 = (1.0f-h)*(0.5f+a/(2.0f*Math::PI/4.0f))+0.1f;
        v2 = v1+h;
    }

    glm::vec2 backgroundScale;
    backgroundScale.x = static_cast<float>(m_backgroundTex.originalSize.x) / static_cast<float>(m_backgroundTex.size.x);
    backgroundScale.y = static_cast<float>(m_backgroundTex.originalSize.y) / static_cast<float>(m_backgroundTex.size.y);

    u2 *= backgroundScale.x;
    v2 *= backgroundScale.y;

    if (m_backgroundScale)
    {
        glm::vec2 scale;
        scale.x = static_cast<float>(m_size.x) / static_cast<float>(m_backgroundTex.originalSize.x);
        scale.y = static_cast<float>(m_size.y) / static_cast<float>(m_backgroundTex.originalSize.y);
        if (scale.x > scale.y)
        {
            scale.y /= scale.x;
            scale.x = 1;
        }
        else
        {
            scale.x /= scale.y;
            scale.y = 1;
        }
        float margin_u = (1-scale.x)/2;
        float margin_v = (1-scale.y)/2;
        margin_u *= backgroundScale.x;
        margin_v *= backgroundScale.y;
        u1 += margin_u;
        v1 += margin_v;
        u2 -= margin_u;
        v2 -= margin_v;
    }

    auto renderer = m_device->GetUIRenderer();
    renderer->SetColor({ 1, 1, 1, 1 });
    renderer->SetTexture(m_backgroundTex);
    renderer->SetTransparency(TransparencyMode::NONE);
    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { p1.x, p1.y }, { u1, v2 } };
    vertices[1] = { { p1.x, p2.y }, { u1, v1 } };
    vertices[2] = { { p2.x, p1.y }, { u2, v2 } };
    vertices[3] = { { p2.x, p2.y }, { u2, v1 } };

    renderer->EndPrimitive();
    AddStatisticTriangle(2);
}

void CEngine::DrawPlanet()
{
    if (! m_planet->PlanetExist())
        return;

    auto renderer = m_device->GetObjectRenderer();
    renderer->Begin();
    renderer->SetProjectionMatrix(m_matProjInterface);
    renderer->SetViewMatrix(m_matViewInterface);
    renderer->SetModelMatrix(m_matWorldInterface);
    renderer->SetFog(1e+6, 1e+6, {});
    renderer->SetLighting(false);
    renderer->SetDepthTest(false);
    renderer->SetDepthMask(false);
    renderer->SetCullFace(CullFace::NONE);

    m_planet->Draw();  // draws the planets

    renderer->End();
}

void CEngine::DrawForegroundImage()
{
    if (m_foregroundName.empty())
        return;

    glm::vec3 n = glm::vec3(0.0f, 0.0f, -1.0f);  // normal

    glm::vec2 p1(0.0f, 0.0f);
    glm::vec2 p2(1.0f, 1.0f);

    float u1 = -m_eyeDirH/(Math::PI*0.6f)+Math::PI*0.5f;
    float u2 = u1+0.50f;

    float v1 = 0.2f;
    float v2 = 1.0f;

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTexture(m_foregroundTex);
    renderer->SetTransparency(TransparencyMode::BLACK);
    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { p1.x, p1.y }, { u1, v2 } };
    vertices[1] = { { p1.x, p2.y }, { u1, v1 } };
    vertices[2] = { { p2.x, p1.y }, { u2, v2 } };
    vertices[3] = { { p2.x, p2.y }, { u2, v1 } };

    renderer->EndPrimitive();
    AddStatisticTriangle(2);
}

void CEngine::DrawOverColor()
{
    if ((m_overColor == Color(0.0f, 0.0f, 0.0f, 0.0f) && m_overMode == TransparencyMode::BLACK) ||
        (m_overColor == Color(1.0f, 1.0f, 1.0f, 1.0f) && m_overMode == TransparencyMode::WHITE))
        return;

    glm::vec2 p1(0.0f, 0.0f);
    glm::vec2 p2(1.0f, 1.0f);

    auto color = Gfx::ColorToIntColor(m_overColor);

    glm::u8vec4 colors[3] =
    {
        { color.r, color.g, color.b, color.a },
        { color.r, color.g, color.b, color.a },
        { 0, 0, 0, 0 }
    };

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTexture(Texture{});
    renderer->SetTransparency(m_overMode);
    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { p1.x, p1.y }, {}, colors[1] };
    vertices[1] = { { p1.x, p2.y }, {}, colors[0] };
    vertices[2] = { { p2.x, p1.y }, {}, colors[1] };
    vertices[3] = { { p2.x, p2.y }, {}, colors[0] };

    renderer->EndPrimitive();
    AddStatisticTriangle(2);
}

void CEngine::DrawHighlight()
{
    glm::vec2 min, max;
    min.x = 1000000.0f;
    min.y = 1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    int i = 0;
    while (m_highlightRank[i] != -1)
    {
        glm::vec2 omin, omax;
        if (GetBBox2D(m_highlightRank[i++], omin, omax))
        {
            min.x = Math::Min(min.x, omin.x);
            min.y = Math::Min(min.y, omin.y);
            max.x = Math::Max(max.x, omax.x);
            max.y = Math::Max(max.y, omax.y);
        }
    }

    if (min.x == 1000000.0f ||
        min.y == 1000000.0f ||
        max.x == -1000000.0f ||
        max.y == -1000000.0f)
    {
        m_highlight = false;  // not highlighted
    }
    else
    {
        m_highlightP1 = min;
        m_highlightP2 = max;
        m_highlight = true;
    }

    if (!m_highlight)
        return;

    glm::vec2 p1 = m_highlightP1;
    glm::vec2 p2 = m_highlightP2;

    int nbOut = 0;
    if (p1.x < 0.0f || p1.x > 1.0f) nbOut++;
    if (p1.y < 0.0f || p1.y > 1.0f) nbOut++;
    if (p2.x < 0.0f || p2.x > 1.0f) nbOut++;
    if (p2.y < 0.0f || p2.y > 1.0f) nbOut++;
    if (nbOut > 2)
        return;

    float d = 0.5f + sinf(m_highlightTime * 6.0f) * 0.5f;
    d *= (p2.x - p1.x) * 0.1f;
    p1.x += d;
    p1.y += d;
    p2.x -= d;
    p2.y -= d;

    glm::u8vec4 color(255, 255, 0, 255);  // yellow

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTransparency(TransparencyMode::NONE);
    renderer->SetTexture(Texture{});

    float dx = (p2.x - p1.x) / 5.0f;
    float dy = (p2.y - p1.y) / 5.0f;

    auto line = renderer->BeginPrimitive(PrimitiveType::LINE_STRIP, 3);
    line[0] = { { p1.x, p1.y + dy }, {}, color };
    line[1] = { { p1.x, p1.y }, {}, color };
    line[2] = { { p1.x + dx, p1.y }, {}, color };
    renderer->EndPrimitive();

    line = renderer->BeginPrimitive(PrimitiveType::LINE_STRIP, 3);
    line[0] = { { p2.x - dx, p1.y }, {}, color };
    line[1] = { { p2.x, p1.y }, {}, color };
    line[2] = { { p2.x, p1.y + dy }, {}, color };
    renderer->EndPrimitive();

    line = renderer->BeginPrimitive(PrimitiveType::LINE_STRIP, 3);
    line[0] = { { p2.x, p2.y - dy }, {}, color };
    line[1] = { { p2.x, p2.y }, {}, color };
    line[2] = { { p2.x - dx, p2.y }, {}, color };
    renderer->EndPrimitive();

    line = renderer->BeginPrimitive(PrimitiveType::LINE_STRIP, 3);
    line[0] = { { p1.x + dx, p2.y }, {}, color };
    line[1] = { { p1.x, p2.y }, {}, color };
    line[2] = { { p1.x, p2.y - dy }, {}, color };
    renderer->EndPrimitive();
}

void CEngine::DrawMouse()
{
    MouseMode mode = m_app->GetMouseMode();
    if (mode != MOUSE_ENGINE && mode != MOUSE_BOTH)
        return;

    SetWindowCoordinates();

    glm::vec2 mousePos = CInput::GetInstancePointer()->GetMousePos();
    glm::ivec2 pos(mousePos.x * m_size.x, m_size.y - mousePos.y * m_size.y);
    pos.x -= MOUSE_TYPES.at(m_mouseType).hotPoint.x;
    pos.y -= MOUSE_TYPES.at(m_mouseType).hotPoint.y;

    glm::ivec2 shadowPos = { pos.x + 4, pos.y - 3 };

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTexture(m_miceTexture);

    DrawMouseSprite(shadowPos, MOUSE_SIZE, MOUSE_TYPES.at(m_mouseType).iconShadow, TransparencyMode::WHITE);
    DrawMouseSprite(pos, MOUSE_SIZE, MOUSE_TYPES.at(m_mouseType).icon1, MOUSE_TYPES.at(m_mouseType).mode1);
    DrawMouseSprite(pos, MOUSE_SIZE, MOUSE_TYPES.at(m_mouseType).icon2, MOUSE_TYPES.at(m_mouseType).mode2);

    SetInterfaceCoordinates();
}

void CEngine::DrawMouseSprite(const glm::ivec2& pos, const glm::ivec2& size, int icon, TransparencyMode mode)
{
    if (icon == -1)
        return;

    glm::ivec2 p1 = pos;
    glm::ivec2 p2 = p1 + size;

    float u1 = (32.0f / 256.0f) * (icon % 8);
    float v1 = (32.0f / 256.0f) * (icon / 8);
    float u2 = u1 + (32.0f / 256.0f);
    float v2 = v1 + (32.0f / 256.0f);

    float dp = 0.5f / 256.0f;
    u1 += dp;
    v1 += dp;
    u2 -= dp;
    v2 -= dp;

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTransparency(mode);
    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { p1.x, p2.y }, { u1, v2 } };
    vertices[1] = { { p1.x, p1.y }, { u1, v1 } };
    vertices[2] = { { p2.x, p2.y }, { u2, v2 } };
    vertices[3] = { { p2.x, p1.y }, { u2, v1 } };

    renderer->EndPrimitive();
    AddStatisticTriangle(2);
}

void CEngine::DrawStats()
{
    if (!m_showStats)
        return;

    float height = m_text->GetAscent(FONT_COMMON, 13.0f);
    float width = 0.4f;
    const int TOTAL_LINES = 22;

    glm::vec2 pos(0.05f * m_size.x/m_size.y, 0.05f + TOTAL_LINES * height);

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTransparency(TransparencyMode::ALPHA);
    renderer->SetTexture(Texture{});

    glm::u8vec4 black = { 0, 0, 0, 192 };

    glm::vec2 margin = { 5.f / m_size.x, 5.f / m_size.y };

    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { pos.x - margin.x, pos.y - (TOTAL_LINES + 1) * height - margin.y }, {}, black };
    vertices[1] = { { pos.x - margin.x, pos.y + height + margin.y }, {}, black };
    vertices[2] = { { pos.x + width + margin.x, pos.y - (TOTAL_LINES + 1) * height - margin.y }, {}, black };
    vertices[3] = { { pos.x + width + margin.x, pos.y + height + margin.y }, {}, black };

    renderer->EndPrimitive();
    renderer->SetTransparency(TransparencyMode::ALPHA);

    auto drawStatsLine = [&](const std::string& name, const std::string& value, const std::string& value2)
    {
        if (!name.empty())
            m_text->DrawText(name+":", FONT_COMMON, 12.0f, pos, 1.0f, TEXT_ALIGN_LEFT, 0, Color(1.0f, 1.0f, 1.0f, 1.0f));
        pos.x += 0.25f;
        if (!value.empty())
            m_text->DrawText(value, FONT_COMMON, 12.0f, pos, 1.0f, TEXT_ALIGN_LEFT, 0, Color(1.0f, 1.0f, 1.0f, 1.0f));
        pos.x += 0.15f;
        if (!value2.empty())
            m_text->DrawText(value2, FONT_COMMON, 12.0f, pos, 1.0f, TEXT_ALIGN_RIGHT, 0, Color(1.0f, 1.0f, 1.0f, 1.0f));
        pos.x -= 0.4f;
        pos.y -= height;
    };

    auto drawStatsValue = [&](const std::string& name, long long time)
    {
        float value = static_cast<float>(time)/CProfiler::GetPerformanceCounterTime(PCNT_ALL);
        drawStatsLine(name, StrUtils::Format("%.2f", value), StrUtils::Format("%.2f ms", time/1e6f));
    };

    auto drawStatsCounter = [&](const std::string& name, PerformanceCounter counter)
    {
        drawStatsValue(name, CProfiler::GetPerformanceCounterTime(counter));
    };

    // TODO: Find a more generic way to calculate these in CProfiler

    long long engineUpdate = CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_ENGINE) -
                             CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_PARTICLE);

    long long gameUpdate = CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_GAME) -
                           CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_CBOT);

    long long otherUpdate = CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_ALL) -
                            CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_ENGINE) -
                            CProfiler::GetPerformanceCounterTime(PCNT_UPDATE_GAME);

    long long otherRender = CProfiler::GetPerformanceCounterTime(PCNT_RENDER_ALL) -
                            CProfiler::GetPerformanceCounterTime(PCNT_RENDER_PARTICLE_WORLD) -
                            CProfiler::GetPerformanceCounterTime(PCNT_RENDER_WATER) -
                            CProfiler::GetPerformanceCounterTime(PCNT_RENDER_TERRAIN) -
                            CProfiler::GetPerformanceCounterTime(PCNT_RENDER_OBJECTS) -
                            CProfiler::GetPerformanceCounterTime(PCNT_RENDER_INTERFACE) -
                            CProfiler::GetPerformanceCounterTime(PCNT_RENDER_SHADOW_MAP);

    drawStatsCounter("Event processing", PCNT_EVENT_PROCESSING);
    drawStatsLine(   "", "", "");
    drawStatsCounter("Frame update",      PCNT_UPDATE_ALL);
    drawStatsValue  ("    Engine update",     engineUpdate);
    drawStatsCounter("    Particle update",   PCNT_UPDATE_PARTICLE);
    drawStatsValue  ("    Game update",       gameUpdate);
    drawStatsCounter("    CBot programs",     PCNT_UPDATE_CBOT);
    drawStatsValue(  "    Other update",      otherUpdate);
    drawStatsLine(   "", "", "");
    drawStatsCounter("Frame render",      PCNT_RENDER_ALL);
    drawStatsCounter("    Particle render",   PCNT_RENDER_PARTICLE_WORLD);
    drawStatsCounter("    Water render",      PCNT_RENDER_WATER);
    drawStatsCounter("    Terrain render",    PCNT_RENDER_TERRAIN);
    drawStatsCounter("    Objects render",    PCNT_RENDER_OBJECTS);
    drawStatsCounter("    UI render",         PCNT_RENDER_INTERFACE);
    drawStatsCounter("        particles",     PCNT_RENDER_PARTICLE_IFACE);
    drawStatsCounter("    Shadow map render", PCNT_RENDER_SHADOW_MAP);
    drawStatsValue(  "    Other render",      otherRender);
    drawStatsCounter("Swap buffers & VSync",  PCNT_SWAP_BUFFERS);
    drawStatsLine(   "", "", "");
    drawStatsLine(   "Triangles",         StrUtils::ToString<int>(m_statisticTriangle), "");
    drawStatsLine(   "FPS",               StrUtils::Format("%.3f", m_fps), "");
    drawStatsLine(   "", "", "");
    std::stringstream str;
    str << std::fixed << std::setprecision(2) << m_statisticPos.x << "; " << m_statisticPos.z;
    drawStatsLine(   "Position",          str.str(), "");
}

void CEngine::DrawTimer()
{
    glm::vec2 pos(0.98f, 0.98f-m_text->GetAscent(FONT_COMMON, 15.0f));
    m_text->DrawText(m_timerText, FONT_COMMON, 15.0f, pos, 1.0f, TEXT_ALIGN_RIGHT, 0, Color(1.0f, 1.0f, 1.0f, 1.0f));
}

void CEngine::AddBaseObjTriangles(int baseObjRank, const std::vector<Gfx::ModelTriangle>& triangles)
{
    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    std::array<Vertex3D, 3> vertices;

    for (const auto& triangle : triangles)
    {
        vertices[0] = triangle.p1;
        vertices[1] = triangle.p2;
        vertices[2] = triangle.p3;

        Material material = triangle.material;

        if (!material.albedoTexture.empty())
            material.albedoTexture = "objects/" + material.albedoTexture;

        if (!material.materialTexture.empty())
            material.materialTexture = "objects/" + material.materialTexture;

        if (!material.emissiveTexture.empty())
            material.emissiveTexture = "objects/" + material.emissiveTexture;

        if (material.variableDetail)
            material.detailTexture = GetSecondTexture();

        EngineBaseObjDataTier& data = AddLevel(p1, EngineTriangleType::TRIANGLES, material);

        data.vertices.insert(data.vertices.end(), vertices.begin(), vertices.end());

        data.updateStaticBuffer = true;

        for (size_t i = 0; i < vertices.size(); i++)
        {
            p1.bboxMin.x = Math::Min(vertices[i].position.x, p1.bboxMin.x);
            p1.bboxMin.y = Math::Min(vertices[i].position.y, p1.bboxMin.y);
            p1.bboxMin.z = Math::Min(vertices[i].position.z, p1.bboxMin.z);
            p1.bboxMax.x = Math::Max(vertices[i].position.x, p1.bboxMax.x);
            p1.bboxMax.y = Math::Max(vertices[i].position.y, p1.bboxMax.y);
            p1.bboxMax.z = Math::Max(vertices[i].position.z, p1.bboxMax.z);
        }

        p1.boundingSphere = Math::BoundingSphereForBox(p1.bboxMin, p1.bboxMax);

        p1.totalTriangles += vertices.size() / 3;
    }

    m_updateStaticBuffers = true;
}

void CEngine::UpdateObjectShadowSpotNormal(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int shadowRank = m_objects[objRank].shadowRank;
    if (shadowRank == -1)
        return;

    assert(shadowRank >= 0 && shadowRank < static_cast<int>( m_shadowSpots.size() ));

    // Calculating the normal to the ground in nine strategic locations,
    // then perform a weighted average (the dots in the center are more important).

    glm::vec3 pos = m_shadowSpots[shadowRank].pos;
    float radius = m_shadowSpots[shadowRank].radius;

    glm::vec3 n[20];
    glm::vec3 norm = { 0, 0, 0 };
    int i = 0;

    m_terrain->GetNormal(norm, pos);
    n[i++] = norm;
    n[i++] = norm;
    n[i++] = norm;

    glm::vec3 shPos = pos;
    shPos.x += radius*0.6f;
    shPos.z += radius*0.6f;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;
    n[i++] = norm;

    shPos = pos;
    shPos.x -= radius*0.6f;
    shPos.z += radius*0.6f;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;
    n[i++] = norm;

    shPos = pos;
    shPos.x += radius*0.6f;
    shPos.z -= radius*0.6f;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;
    n[i++] = norm;

    shPos = pos;
    shPos.x -= radius*0.6f;
    shPos.z -= radius*0.6f;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;
    n[i++] = norm;

    shPos = pos;
    shPos.x += radius;
    shPos.z += radius;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;

    shPos = pos;
    shPos.x -= radius;
    shPos.z += radius;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;

    shPos = pos;
    shPos.x += radius;
    shPos.z -= radius;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;

    shPos = pos;
    shPos.x -= radius;
    shPos.z -= radius;
    m_terrain->GetNormal(norm, shPos);
    n[i++] = norm;

    norm = { 0, 0, 0 };
    for (int j = 0; j < i; j++)
    {
        norm += n[j];
    }
    norm /= static_cast<float>(i);  // average vector

    m_shadowSpots[shadowRank].normal = norm;
}

int CEngine::AddStaticMesh(const std::string& key, const CModelMesh* mesh, const glm::mat4& worldMatrix)
{
    int baseObjRank = -1;

    auto it = m_staticMeshBaseObjects.find(key);
    if (it == m_staticMeshBaseObjects.end())
    {
        baseObjRank = CreateBaseObject();

        for (size_t i = 0; i < mesh->GetPartCount(); i++)
        {
            const auto& part = mesh->GetPart(i);

            std::vector<Gfx::ModelTriangle> triangles;
            part->GetTriangles(triangles);

            AddBaseObjTriangles(baseObjRank, triangles);
        }
        m_staticMeshBaseObjects[key] = baseObjRank;
    }
    else
    {
        baseObjRank = it->second;
    }

    int objRank = CreateObject();
    SetObjectBaseRank(objRank, baseObjRank);
    SetObjectTransform(objRank, worldMatrix);
    SetObjectType(objRank, ENG_OBJTYPE_FIX);

    return objRank;
}

void CEngine::AddStaticMeshShadowSpot(int meshHandle, const ModelShadowSpot& shadowSpot)
{
    int objRank = meshHandle;

    CreateShadowSpot(objRank);
    SetObjectShadowSpotRadius(objRank, shadowSpot.radius);
    SetObjectShadowSpotIntensity(objRank, shadowSpot.intensity);
    SetObjectShadowSpotType(objRank, EngineShadowType::NORMAL);
    SetObjectShadowSpotHeight(objRank, 0.0f);
    SetObjectShadowSpotAngle(objRank, 0.0f);
    UpdateObjectShadowSpotNormal(objRank);
}

void CEngine::DeleteStaticMesh(int meshHandle)
{
    int objRank = meshHandle;

    DeleteShadowSpot(objRank);
    DeleteObject(objRank);
}

const glm::mat4& CEngine::GetStaticMeshWorldMatrix(int meshHandle)
{
    int objRank = meshHandle;
    return m_objects[objRank].transform;
}

void CEngine::SetStaticMeshGhostMode(int meshHandle, bool enabled)
{
    int objRank = meshHandle;
    SetObjectGhostMode(objRank, enabled);
}

void CEngine::SetDebugLights(bool debugLights)
{
    m_debugLights = debugLights;
}

bool CEngine::GetDebugLights()
{
    return m_debugLights;
}

void CEngine::DebugDumpLights()
{
    m_debugDumpLights = true;
}

void CEngine::SetDebugResources(bool debugResources)
{
    m_debugResources = debugResources;
    m_firstGroundSpot = true; // Force a refresh of ground spot textures
    UpdateGroundSpotTextures();
}

bool CEngine::GetDebugResources()
{
    return m_debugResources;
}

void CEngine::SetDebugGoto(bool debugGoto)
{
    m_debugGoto = debugGoto;
    if (!m_debugGoto)
    {
        m_displayGotoImage.reset();
    }
}

bool CEngine::GetDebugGoto()
{
    return m_debugGoto;
}

void CEngine::AddDebugGotoLine(std::vector<Gfx::Vertex3D> line)
{
    m_displayGoto.push_back(line);
}

void CEngine::SetDebugGotoBitmap(std::unique_ptr<CImage> debugImage)
{
    m_displayGotoImage = std::move(debugImage);
    m_firstGroundSpot = true; // Force ground spot texture reload
    UpdateGroundSpotTextures();
}

void CEngine::SetInterfaceCoordinates()
{
    auto renderer = m_device->GetUIRenderer();
    renderer->SetProjection(0.0f, 1.0f, 0.0f, 1.0f);
}

void CEngine::EnablePauseBlur()
{
    if (!m_pauseBlurEnabled) return;

    m_captureWorld = true;
    m_worldCaptured = false;
}

void CEngine::DisablePauseBlur()
{
    m_captureWorld = false;
    m_worldCaptured = false;
}

void CEngine::SetWindowCoordinates()
{
    auto renderer = m_device->GetUIRenderer();
    renderer->SetProjection(0.0f, m_size.x, m_size.y, 0.0f);
}

} // namespace Gfx
