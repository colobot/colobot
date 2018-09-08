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


#include "graphics/engine/engine.h"

#include "app/app.h"
#include "app/input.h"

#include "common/image.h"
#include "common/key.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/profiler.h"
#include "common/stringutils.h"

#include "common/system/system.h"

#include "common/thread/resource_owning_thread.h"

#include "graphics/core/device.h"
#include "graphics/core/framebuffer.h"

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

#include "math/geometry.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"

#include <iomanip>
#include <SDL_surface.h>
#include <SDL_thread.h>

// Graphics module namespace
namespace Gfx
{

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
    EngineRenderState mode1;
    //! Mode to render 2nd image in
    EngineRenderState mode2;
    //! Hot point
    Math::IntPoint hotPoint;

    EngineMouse(int icon1 = -1,
                int icon2 = -1,
                int iconShadow = -1,
                EngineRenderState mode1 = ENG_RSTATE_NORMAL,
                EngineRenderState mode2 = ENG_RSTATE_NORMAL,
                Math::IntPoint hotPoint = Math::IntPoint())
        : icon1(icon1)
        , icon2(icon2)
        , iconShadow(iconShadow)
        , mode1(mode1)
        , mode2(mode2)
        , hotPoint(hotPoint)
    {}
};

const Math::IntPoint MOUSE_SIZE(32, 32);
const std::map<EngineMouseType, EngineMouse> MOUSE_TYPES = {
    {{ENG_MOUSE_NORM},    {EngineMouse( 0,  1, 32, ENG_RSTATE_TTEXTURE_WHITE, ENG_RSTATE_TTEXTURE_BLACK, Math::IntPoint( 1,  1))}},
    {{ENG_MOUSE_WAIT},    {EngineMouse( 2,  3, 33, ENG_RSTATE_TTEXTURE_WHITE, ENG_RSTATE_TTEXTURE_BLACK, Math::IntPoint( 8, 12))}},
    {{ENG_MOUSE_HAND},    {EngineMouse( 4,  5, 34, ENG_RSTATE_TTEXTURE_WHITE, ENG_RSTATE_TTEXTURE_BLACK, Math::IntPoint( 7,  2))}},
    {{ENG_MOUSE_NO},      {EngineMouse( 6,  7, 35, ENG_RSTATE_TTEXTURE_WHITE, ENG_RSTATE_TTEXTURE_BLACK, Math::IntPoint(10, 10))}},
    {{ENG_MOUSE_EDIT},    {EngineMouse( 8,  9, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 6, 10))}},
    {{ENG_MOUSE_CROSS},   {EngineMouse(10, 11, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint(10, 10))}},
    {{ENG_MOUSE_MOVEV},   {EngineMouse(12, 13, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 5, 11))}},
    {{ENG_MOUSE_MOVEH},   {EngineMouse(14, 15, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint(11,  5))}},
    {{ENG_MOUSE_MOVED},   {EngineMouse(16, 17, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 9,  9))}},
    {{ENG_MOUSE_MOVEI},   {EngineMouse(18, 19, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 9,  9))}},
    {{ENG_MOUSE_MOVE},    {EngineMouse(20, 21, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint(11, 11))}},
    {{ENG_MOUSE_TARGET},  {EngineMouse(22, 23, -1, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint(15, 15))}},
    {{ENG_MOUSE_SCROLLL}, {EngineMouse(24, 25, 43, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 2,  9))}},
    {{ENG_MOUSE_SCROLLR}, {EngineMouse(26, 27, 44, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint(17,  9))}},
    {{ENG_MOUSE_SCROLLU}, {EngineMouse(28, 29, 45, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 9,  2))}},
    {{ENG_MOUSE_SCROLLD}, {EngineMouse(30, 31, 46, ENG_RSTATE_TTEXTURE_BLACK, ENG_RSTATE_TTEXTURE_WHITE, Math::IntPoint( 9, 17))}},
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
    m_overMode  = ENG_RSTATE_TCOLOR_BLACK;
    m_highlight = false;
    std::fill_n(m_highlightRank, 100, -1);
    m_highlightTime = 0.0f;
    m_eyePt    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_lookatPt = Math::Vector(0.0f, 0.0f, 1.0f);
    m_drawWorld = true;
    m_drawFront = false;
    m_particleDensity = 1.0f;
    m_clippingDistance = 1.0f;
    m_terrainVision = 1000.0f;
    m_textureMipmapLevel = 1;
    m_textureAnisotropy = 1;
    m_shadowMapping = true;
    m_offscreenShadowRendering = true;
    m_offscreenShadowRenderingResolution = 1024;
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
    m_lastFrameTime = m_systemUtils->CreateTimeStamp();
    m_currentFrameTime = m_systemUtils->CreateTimeStamp();

    m_shadowColor = 0.5f;

    m_defaultTexParams.format = TEX_IMG_AUTO;
    m_defaultTexParams.filter = TEX_FILTER_BILINEAR;

    m_terrainTexParams.format = TEX_IMG_AUTO;
    m_terrainTexParams.filter = TEX_FILTER_BILINEAR;

    // Compute bias matrix for shadow mapping
    Math::Matrix temp1, temp2;
    Math::LoadScaleMatrix(temp1, Math::Vector(0.5f, 0.5f, 0.5f));
    Math::LoadTranslationMatrix(temp2, Math::Vector(1.0f, 1.0f, 1.0f));
    m_shadowBias = Math::MultiplyMatrices(temp1, temp2);

    m_lastState = -1;
    m_statisticTriangle = 0;
    m_fps = 0.0f;
    m_firstGroundSpot = false;
}

CEngine::~CEngine()
{
    m_systemUtils->DestroyTimeStamp(m_lastFrameTime);
    m_lastFrameTime = nullptr;
    m_systemUtils->DestroyTimeStamp(m_currentFrameTime);
    m_currentFrameTime = nullptr;
}

void CEngine::SetDevice(CDevice *device)
{
    m_device = device;
}

CDevice* CEngine::GetDevice()
{
    return m_device;
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

    m_modelManager = MakeUnique<COldModelManager>(this);
    m_pyroManager = MakeUnique<CPyroManager>();
    m_lightMan   = MakeUnique<CLightManager>(this);
    m_text       = MakeUnique<CText>(this);
    m_particle   = MakeUnique<CParticle>(this);
    m_water      = MakeUnique<CWater>(this);
    m_cloud      = MakeUnique<CCloud>(this);
    m_lightning  = MakeUnique<CLightning>(this);
    m_planet     = MakeUnique<CPlanet>(this);

    m_lightMan->SetDevice(m_device);
    m_particle->SetDevice(m_device);

    m_text->SetDevice(m_device);
    if (! m_text->Create())
    {
        std::string error = m_text->GetError();
        GetLogger()->Error("Error creating CText: %s\n", error.c_str());
        return false;
    }

    m_device->SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    m_device->SetShadeModel(SHADE_SMOOTH);
    m_device->SetFillMode(FILL_POLY);

    SetFocus(m_focus);

    m_matWorldInterface.LoadIdentity();
    m_matViewInterface.LoadIdentity();

    Math::LoadOrthoProjectionMatrix(m_matProjInterface, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    TextureCreateParams params;
    params.format = TEX_IMG_AUTO;
    params.filter = TEX_FILTER_NEAREST;
    params.mipmap = false;
    m_miceTexture = LoadTexture("textures/interface/mouse.png", params);

    m_systemUtils->GetCurrentTimeStamp(m_currentFrameTime);
    m_systemUtils->GetCurrentTimeStamp(m_lastFrameTime);

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

void CEngine::WriteScreenShot(const std::string& fileName)
{
    auto data = MakeUnique<WriteScreenShotData>();
    data->img = MakeUnique<CImage>(Math::IntPoint(m_size.x, m_size.y));

    auto pixels = m_device->GetFrameBufferPixels();
    data->img->SetDataPixels(pixels->GetPixelsData());
    data->img->FlipVertically();

    data->fileName = fileName;

    CResourceOwningThread<WriteScreenShotData> thread(CEngine::WriteScreenShotThread, std::move(data), "WriteScreenShot thread");
    thread.Start();
}

void CEngine::WriteScreenShotThread(std::unique_ptr<WriteScreenShotData> data)
{
    if ( data->img->SavePNG(data->fileName.c_str()) )
    {
       GetLogger()->Debug("Save screenshot saved successfully\n");
    }
    else
    {
       GetLogger()->Error("%s!\n", data->img->GetError().c_str());
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

Math::IntPoint CEngine::GetWindowSize()
{
    return m_size;
}

Math::Point CEngine::WindowToInterfaceCoords(Math::IntPoint pos)
{
    return Math::Point(        static_cast<float>(pos.x) / static_cast<float>(m_size.x),
                        1.0f - static_cast<float>(pos.y) / static_cast<float>(m_size.y)  );
}

Math::IntPoint CEngine::InterfaceToWindowCoords(Math::Point pos)
{
    return Math::IntPoint(static_cast<int>(pos.x * m_size.x),
                          static_cast<int>((1.0f - pos.y) * m_size.y));
}

Math::Point CEngine::WindowToInterfaceSize(Math::IntPoint size)
{
    return Math::Point(static_cast<float>(size.x) / static_cast<float>(m_size.x),
                       static_cast<float>(size.y) / static_cast<float>(m_size.y));
}

Math::IntPoint CEngine::InterfaceToWindowSize(Math::Point size)
{
    return Math::IntPoint(static_cast<int>(size.x * m_size.x),
                          static_cast<int>(size.y * m_size.y));
}

void CEngine::AddStatisticTriangle(int count)
{
    m_statisticTriangle += count;
}

int CEngine::GetStatisticTriangle()
{
    return m_statisticTriangle;
}

void CEngine::SetStatisticPos(Math::Vector pos)
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

EngineBaseObjTexTier& CEngine::AddLevel2(EngineBaseObject& p1, const std::string& tex1Name, const std::string& tex2Name)
{
    for (int i = 0; i < static_cast<int>( p1.next.size() ); i++)
    {
        if (p1.next[i].tex1Name == tex1Name && p1.next[i].tex2Name == tex2Name)
            return p1.next[i];
    }

    p1.next.push_back(EngineBaseObjTexTier(tex1Name, tex2Name));
    return p1.next.back();
}

EngineBaseObjDataTier& CEngine::AddLevel3(EngineBaseObjTexTier& p3, EngineTriangleType type,
                                          const Material& material, int state)
{
    for (int i = 0; i < static_cast<int>( p3.next.size() ); i++)
    {
        if ( (p3.next[i].type == type) && (p3.next[i].material == material) && (p3.next[i].state == state) )
            return p3.next[i];
    }

    p3.next.push_back(EngineBaseObjDataTier(type, material, state));
    return p3.next.back();
}

int CEngine::CreateBaseObject()
{
    int baseObjRank = 0;
    for ( ; baseObjRank < static_cast<int>( m_baseObjects.size() ); baseObjRank++)
    {
        if (! m_baseObjects[baseObjRank].used)
        {
            m_baseObjects[baseObjRank].LoadDefault();
            break;
        }
    }

    if (baseObjRank == static_cast<int>( m_baseObjects.size() ))
        m_baseObjects.push_back(EngineBaseObject());
    else
        m_baseObjects[baseObjRank].LoadDefault();


    m_baseObjects[baseObjRank].used = true;

    return baseObjRank;
}

void CEngine::DeleteBaseObject(int baseObjRank)
{
    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    if (! p1.used)
        return;

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        EngineBaseObjTexTier& p2 = p1.next[l2];

        for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
        {
            EngineBaseObjDataTier& p3 = p2.next[l3];
            m_device->DestroyStaticBuffer(p3.staticBufferId);
            p3.staticBufferId = 0;
        }
    }

    p1.next.clear();

    p1.used = false;
}

void CEngine::DeleteAllBaseObjects()
{
    for (int baseObjRank = 0; baseObjRank < static_cast<int>( m_baseObjects.size() ); baseObjRank++)
    {
        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];
                m_device->DestroyStaticBuffer(p3.staticBufferId);
                p3.staticBufferId = 0;
            }
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

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        EngineBaseObjTexTier& p2 = p1.next[l2];

        for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
        {
            EngineBaseObjDataTier& p3 = p2.next[l3];
            p3.staticBufferId = 0;
        }
    }
}

void CEngine::AddBaseObjTriangles(int baseObjRank, const std::vector<VertexTex2>& vertices,
                                  const Material& material, int state,
                                  std::string tex1Name, std::string tex2Name)
{
    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject&      p1 = m_baseObjects[baseObjRank];
    EngineBaseObjTexTier&  p2 = AddLevel2(p1, tex1Name, tex2Name);
    EngineBaseObjDataTier& p3 = AddLevel3(p2, ENG_TRIANGLE_TYPE_TRIANGLES, material, state);

    p3.vertices.insert(p3.vertices.end(), vertices.begin(), vertices.end());

    p3.updateStaticBuffer = true;
    m_updateStaticBuffers = true;

    for (int i = 0; i < static_cast<int>( vertices.size() ); i++)
    {
        p1.bboxMin.x = Math::Min(vertices[i].coord.x, p1.bboxMin.x);
        p1.bboxMin.y = Math::Min(vertices[i].coord.y, p1.bboxMin.y);
        p1.bboxMin.z = Math::Min(vertices[i].coord.z, p1.bboxMin.z);
        p1.bboxMax.x = Math::Max(vertices[i].coord.x, p1.bboxMax.x);
        p1.bboxMax.y = Math::Max(vertices[i].coord.y, p1.bboxMax.y);
        p1.bboxMax.z = Math::Max(vertices[i].coord.z, p1.bboxMax.z);
    }

    p1.boundingSphere = Math::BoundingSphereForBox(p1.bboxMin, p1.bboxMax);

    p1.totalTriangles += vertices.size() / 3;
}

void CEngine::AddBaseObjQuick(int baseObjRank, const EngineBaseObjDataTier& buffer,
                              std::string tex1Name, std::string tex2Name,
                              bool globalUpdate)
{
    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject&      p1 = m_baseObjects[baseObjRank];
    EngineBaseObjTexTier&  p2 = AddLevel2(p1, tex1Name, tex2Name);

    p2.next.push_back(buffer);

    EngineBaseObjDataTier& p3 = p2.next.back();

    UpdateStaticBuffer(p3);

    if (globalUpdate)
    {
        m_updateGeometry = true;
    }
    else
    {
        for (int i = 0; i < static_cast<int>( p3.vertices.size() ); i++)
        {
            p1.bboxMin.x = Math::Min(p3.vertices[i].coord.x, p1.bboxMin.x);
            p1.bboxMin.y = Math::Min(p3.vertices[i].coord.y, p1.bboxMin.y);
            p1.bboxMin.z = Math::Min(p3.vertices[i].coord.z, p1.bboxMin.z);
            p1.bboxMax.x = Math::Max(p3.vertices[i].coord.x, p1.bboxMax.x);
            p1.bboxMax.y = Math::Max(p3.vertices[i].coord.y, p1.bboxMax.y);
            p1.bboxMax.z = Math::Max(p3.vertices[i].coord.z, p1.bboxMax.z);
        }

        p1.boundingSphere = Math::BoundingSphereForBox(p1.bboxMin, p1.bboxMax);
    }

    if (p3.type == ENG_TRIANGLE_TYPE_TRIANGLES)
        p1.totalTriangles += p3.vertices.size() / 3;
    else if (p3.type == ENG_TRIANGLE_TYPE_SURFACE)
        p1.totalTriangles += p3.vertices.size() - 2;
}

void CEngine::DebugObject(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    CLogger* l = GetLogger();

    l->Debug("Debug object: %d\n", objRank);
    if (! m_objects[objRank].used)
    {
        l->Debug(" not used\n");
        return;
    }

    l->Debug(" baseObjRank = %d\n", m_objects[objRank].baseObjRank);
    l->Debug(" visible = %s\n", m_objects[objRank].visible ? "true" : "false");
    l->Debug(" drawWorld = %s\n", m_objects[objRank].drawWorld ? "true" : "false");
    l->Debug(" drawFront = %s\n", m_objects[objRank].drawFront ? "true" : "false");
    l->Debug(" type = %d\n", m_objects[objRank].type);
    l->Debug(" distance = %f\n", m_objects[objRank].distance);
    l->Debug(" shadowRank = %d\n", m_objects[objRank].shadowRank);
    l->Debug(" transparency = %f\n", m_objects[objRank].transparency);

    l->Debug(" baseObj:\n");
    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
    {
        l->Debug("  null\n");
        return;
    }

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];
    if (!p1.used)
    {
        l->Debug("  not used\n");
        return;
    }

    std::string vecStr;

    vecStr = p1.bboxMin.ToString();
    l->Debug("  bboxMin: %s\n", vecStr.c_str());
    vecStr = p1.bboxMax.ToString();
    l->Debug("  bboxMax: %s\n", vecStr.c_str());
    l->Debug("  totalTriangles: %d\n", p1.totalTriangles);
    l->Debug("  radius: %f\n", p1.boundingSphere.radius);

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        EngineBaseObjTexTier& p2 = p1.next[l2];
        l->Debug("  l2:\n");

        l->Debug("   tex1: %s (id: %u)\n", p2.tex1Name.c_str(), p2.tex1.id);
        l->Debug("   tex2: %s (id: %u)\n", p2.tex2Name.c_str(), p2.tex2.id);

        for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
        {
            EngineBaseObjDataTier& p3 = p2.next[l3];

            l->Debug("   l3:\n");
            l->Debug("    type: %d\n", p3.type);
            l->Debug("    state: %d\n", p3.state);
            l->Debug("    staticBufferId: %u\n", p3.staticBufferId);
            l->Debug("    updateStaticBuffer: %s\n", p3.updateStaticBuffer ? "true" : "false");
        }
    }
}

int CEngine::CreateObject()
{
    int objRank = 0;
    for ( ; objRank < static_cast<int>( m_objects.size() ); objRank++)
    {
        if (! m_objects[objRank].used)
        {
            m_objects[objRank].LoadDefault();
            break;
        }
    }

    if (objRank == static_cast<int>( m_objects.size() ))
        m_objects.push_back(EngineObject());


    m_objects[objRank].used = true;

    Math::Matrix mat;
    mat.LoadIdentity();
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


void CEngine::SetObjectTransform(int objRank, const Math::Matrix& transform)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].transform = transform;
}

void CEngine::GetObjectTransform(int objRank, Math::Matrix& transform)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    transform = m_objects[objRank].transform;
}

void CEngine::SetObjectDrawWorld(int objRank, bool draw)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].drawWorld = draw;
}

void CEngine::SetObjectDrawFront(int objRank, bool draw)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].drawFront = draw;
}

void CEngine::SetObjectTransparency(int objRank, float value)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    m_objects[objRank].transparency = value;
}

void CEngine::GetObjectBBox(int objRank, Math::Vector& min, Math::Vector& max)
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

EngineBaseObjDataTier* CEngine::FindTriangles(int objRank, const Material& material,
                                              int state, std::string tex1Name,
                                              std::string tex2Name)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return nullptr;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        EngineBaseObjTexTier& p2 = p1.next[l2];

        if (p2.tex1Name != tex1Name)
            continue;

        for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
        {
            EngineBaseObjDataTier& p3 = p2.next[l3];

            if ( (p3.state & (~(ENG_RSTATE_DUAL_BLACK|ENG_RSTATE_DUAL_WHITE))) != state ||
                  p3.material != material )
                continue;

            return &p3;
        }
    }

    return nullptr;
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
        EngineBaseObjTexTier& p2 = p1.next[l2];

        for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
        {
            EngineBaseObjDataTier& p3 = p2.next[l3];

            if (p3.type == ENG_TRIANGLE_TYPE_TRIANGLES)
            {
                for (int i = 0; i < static_cast<int>( p3.vertices.size() ); i += 3)
                {
                    if (static_cast<float>(actualCount) / total >= percent)
                        break;

                    if (actualCount >= maxCount)
                        break;

                    EngineTriangle t;
                    t.triangle[0] = p3.vertices[i];
                    t.triangle[1] = p3.vertices[i+1];
                    t.triangle[2] = p3.vertices[i+2];
                    t.material = p3.material;
                    t.state = p3.state;
                    t.tex1Name = p2.tex1Name;
                    t.tex2Name = p2.tex2Name;

                    triangles.push_back(t);

                    ++actualCount;
                }
            }
            else if (p3.type == ENG_TRIANGLE_TYPE_SURFACE)
            {
                for (int i = 0; i < static_cast<int>( p3.vertices.size() ); i += 1)
                {
                    if (static_cast<float>(actualCount) / total >= percent)
                        break;

                    if (actualCount >= maxCount)
                        break;

                    EngineTriangle t;
                    t.triangle[0] = p3.vertices[i];
                    t.triangle[1] = p3.vertices[i+1];
                    t.triangle[2] = p3.vertices[i+2];
                    t.material = p3.material;
                    t.state = p3.state;
                    t.tex1Name = p2.tex1Name;
                    t.tex2Name = p2.tex2Name;

                    triangles.push_back(t);

                    ++actualCount;
                }
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

    for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
    {
        if (p1.next[l2].tex2Name == tex2Name)
            continue;  // already new

        std::string tex1Name = p1.next[l2].tex1Name;
        EngineBaseObjTexTier& newP2 = AddLevel2(p1, tex1Name, tex2Name);
        newP2.next.insert(newP2.next.end(), p1.next[l2].next.begin(), p1.next[l2].next.end());
        p1.next[l2].next.clear();

        if (!newP2.tex1.Valid() && !newP2.tex1Name.empty())
            newP2.tex1 = LoadTexture("textures/"+newP2.tex1Name);

        if (!newP2.tex2.Valid() && !newP2.tex2Name.empty())
            newP2.tex2 = LoadTexture("textures/"+newP2.tex2Name);
    }
}

void CEngine::ChangeTextureMapping(int objRank, const Material& mat, int state,
                                   const std::string& tex1Name, const std::string& tex2Name,
                                   EngineTextureMapping mode,
                                   float au, float bu, float av, float bv)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    EngineBaseObjDataTier* p4 = FindTriangles(objRank, mat, state, tex1Name, tex2Name);
    if (p4 == nullptr)
        return;

    int nb = p4->vertices.size();

    if (mode == ENG_TEX_MAPPING_X)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.z * au + bu;
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.y * av + bv;
        }
    }
    else if (mode == ENG_TEX_MAPPING_Y)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.x * au + bu;
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.z * av + bv;
        }
    }
    else if (mode == ENG_TEX_MAPPING_Z)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.x * au + bu;
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.y * av + bv;
        }
    }
    else if (mode == ENG_TEX_MAPPING_1X)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.x * au + bu;
        }
    }
    else if (mode == ENG_TEX_MAPPING_1Y)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.y = p4->vertices[i].coord.y * au + bu;
        }
    }
    else if (mode == ENG_TEX_MAPPING_1Z)
    {
        for (int i = 0; i < nb; i++)
        {
            p4->vertices[i].texCoord.x = p4->vertices[i].coord.z * au + bu;
        }
    }

    UpdateStaticBuffer(*p4);
}

void CEngine::TrackTextureMapping(int objRank, const Material& mat, int state,
                                  const std::string& tex1Name, const std::string& tex2Name,
                                  EngineTextureMapping mode,
                                  float pos, float factor, float tl, float ts, float tt)
{
    assert(objRank >= 0 && objRank < static_cast<int>( m_objects.size() ));

    EngineBaseObjDataTier* p4 = FindTriangles(objRank, mat, state, tex1Name, tex2Name);
    if (p4 == nullptr)
        return;

    int tNum = p4->vertices.size();
    if (tNum < 12 || tNum % 6 != 0)
        return;

    std::vector<Gfx::VertexTex2>& vs = p4->vertices;

    while (pos < 0.0f)
        pos += 1.0f;  // never negative!

    Math::Vector current;

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            if (Math::IsEqual(vs[i].coord.x, vs[j+6].coord.x) &&
                Math::IsEqual(vs[i].coord.y, vs[j+6].coord.y))
            {
                current.x = vs[i].coord.x;  // position end link
                current.y = vs[i].coord.y;
                break;
            }
        }
    }

    float ps = 0.0f;  // start position on the periphery
    float pe = 0.0f;
    int is[6] = { 0 }, ie[6] = { 0 };

    int tBase = 0;
    for (int ti = 0; ti < tNum / 6; ti++)
    {
        int s = 0;
        int e = 0;

        for (int i = 0; i < 6; i++)
        {
            if (Math::IsEqual(vs[tBase + i].coord.x, current.x, 0.0001f) &&
                Math::IsEqual(vs[tBase + i].coord.y, current.y, 0.0001f))
            {
                ie[e++] = i;
            }
            else
            {
                is[s++] = i;
            }
        }
        if (s == 3 && e == 3)
        {
            pe = ps + Math::Point(vs[tBase + is[0]].coord.x - vs[tBase + ie[0]].coord.x,
                                  vs[tBase + is[0]].coord.y - vs[tBase + ie[0]].coord.y).Length() / factor;  // end position on the periphery

            float pps = ps + pos;
            float ppe = pe + pos;
            int offset = static_cast<int>(pps);
            ppe -= offset;
            pps -= offset;

            for (int i = 0; i < 3; i++)
            {
                vs[tBase + is[i]].texCoord.x = ((pps * tl) + ts) / tt;
                vs[tBase + ie[i]].texCoord.x = ((ppe * tl) + ts) / tt;
            }
        }

        if (ti >= (tNum / 6) - 1)
            break;

        for (int i = 0; i < 6; i++)
        {
            if (!Math::IsEqual(vs[tBase + i+6].coord.x, current.x, 0.0001f) ||
                !Math::IsEqual(vs[tBase + i+6].coord.y, current.y, 0.0001f))
            {
                current.x = vs[tBase + i+6].coord.x;  // end next link
                current.y = vs[tBase + i+6].coord.y;
                break;
            }
        }
        ps = pe;  // following start position on the periphery
        tBase += 6;
    }

    UpdateStaticBuffer(*p4);
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
            m_shadowSpots[index].LoadDefault();
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

void CEngine::SetObjectShadowSpotPos(int objRank, const Math::Vector& pos)
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

bool CEngine::GetHighlight(Math::Point &p1, Math::Point &p2)
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

bool CEngine::GetBBox2D(int objRank, Math::Point &min, Math::Point &max)
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
        Math::Vector p;

        if ( i & (1<<0) )  p.x = p1.bboxMin.x;
        else               p.x = p1.bboxMax.x;
        if ( i & (1<<1) )  p.y = p1.bboxMin.y;
        else               p.y = p1.bboxMax.y;
        if ( i & (1<<2) )  p.z = p1.bboxMin.z;
        else               p.z = p1.bboxMax.z;

        Math::Vector pp;
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
        CImage shadowImg(Math::IntPoint(256, 256));
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
            m_groundSpots[index].LoadDefault();
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
    m_groundSpots[rank].pos = Math::Vector(0.0f, 0.0f, 0.0f);
}

void CEngine::SetObjectGroundSpotPos(int rank, const Math::Vector& pos)
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

void CEngine::CreateGroundMark(Math::Vector pos, float radius,
                                   float delay1, float delay2, float delay3,
                                   int dx, int dy, char* table)
{
    m_groundMark.LoadDefault();

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
    m_groundMark.LoadDefault();
}

void CEngine::ComputeDistance()
{
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

void CEngine::UpdateGeometry()
{
    if (! m_updateGeometry)
        return;

    for (int baseObjRank = 0; baseObjRank < static_cast<int>( m_baseObjects.size() ); baseObjRank++)
    {
        EngineBaseObject &p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        p1.bboxMin.LoadZero();
        p1.bboxMax.LoadZero();

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];

                for (int i = 0; i < static_cast<int>( p3.vertices.size() ); i++)
                {
                        p1.bboxMin.x = Math::Min(p3.vertices[i].coord.x, p1.bboxMin.x);
                        p1.bboxMin.y = Math::Min(p3.vertices[i].coord.y, p1.bboxMin.y);
                        p1.bboxMin.z = Math::Min(p3.vertices[i].coord.z, p1.bboxMin.z);
                        p1.bboxMax.x = Math::Max(p3.vertices[i].coord.x, p1.bboxMax.x);
                        p1.bboxMax.y = Math::Max(p3.vertices[i].coord.y, p1.bboxMax.y);
                        p1.bboxMax.z = Math::Max(p3.vertices[i].coord.z, p1.bboxMax.z);
                }
            }
        }

        p1.boundingSphere = Math::BoundingSphereForBox(p1.bboxMin, p1.bboxMax);
    }

    m_updateGeometry = false;
}

void CEngine::UpdateStaticBuffer(EngineBaseObjDataTier& p4)
{
    PrimitiveType type;
    if (p4.type == ENG_TRIANGLE_TYPE_TRIANGLES)
        type = PRIMITIVE_TRIANGLES;
    else
        type = PRIMITIVE_TRIANGLE_STRIP;

    if (p4.staticBufferId == 0)
        p4.staticBufferId = m_device->CreateStaticBuffer(type, &p4.vertices[0], p4.vertices.size());
    else
        m_device->UpdateStaticBuffer(p4.staticBufferId, type, &p4.vertices[0], p4.vertices.size());

    p4.updateStaticBuffer = false;
}

void CEngine::UpdateStaticBuffers()
{
    if (!m_updateStaticBuffers)
        return;

    m_updateStaticBuffers = false;

    for (int baseObjRank = 0; baseObjRank < static_cast<int>( m_baseObjects.size() ); baseObjRank++)
    {
        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];

                if (! p3.updateStaticBuffer)
                        continue;

                UpdateStaticBuffer(p3);
            }
        }
    }
}

void CEngine::Update()
{
    ComputeDistance();
    UpdateGeometry();
    UpdateStaticBuffers();
}

bool CEngine::DetectBBox(int objRank, Math::Point mouse)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return false;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

    EngineBaseObject& p1 = m_baseObjects[baseObjRank];

    Math::Point min, max;
    min.x =  1000000.0f;
    min.y =  1000000.0f;
    max.x = -1000000.0f;
    max.y = -1000000.0f;

    for (int i = 0; i < 8; i++)
    {
        Math::Vector p;

        if ( i & (1<<0) )  p.x = p1.bboxMin.x;
        else               p.x = p1.bboxMax.x;
        if ( i & (1<<1) )  p.y = p1.bboxMin.y;
        else               p.y = p1.bboxMax.y;
        if ( i & (1<<2) )  p.z = p1.bboxMin.z;
        else               p.z = p1.bboxMax.z;

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

int CEngine::DetectObject(Math::Point mouse, Math::Vector& targetPos, bool terrain)
{
    float min = 1000000.0f;
    int nearest = -1;
    Math::Vector pos;

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
            EngineBaseObjTexTier& p2 = p1.next[l2];

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];

                if (p3.type == ENG_TRIANGLE_TYPE_TRIANGLES)
                {
                    for (int i = 0; i < static_cast<int>( p3.vertices.size() ); i += 3)
                    {
                        float dist = 0.0f;
                        if (DetectTriangle(mouse, &p3.vertices[i], objRank, dist, pos) && dist < min)
                        {
                            min = dist;
                            nearest = objRank;
                            targetPos = pos;
                        }
                    }
                }
                else if (p3.type == ENG_TRIANGLE_TYPE_SURFACE)
                {
                    for (int i = 0; i < static_cast<int>( p3.vertices.size() ) - 2; i += 1)
                    {
                        float dist = 0.0f;
                        if (DetectTriangle(mouse, &p3.vertices[i], objRank, dist, pos) && dist < min)
                        {
                            min = dist;
                            nearest = objRank;
                            targetPos = pos;
                        }
                    }
                }
            }
        }
    }

    return nearest;
}

bool CEngine::DetectTriangle(Math::Point mouse, VertexTex2* triangle, int objRank, float& dist, Math::Vector& pos)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    Math::Vector p2D[3], p3D;

    for (int i = 0; i < 3; i++)
    {
        p3D.x = triangle[i].coord.x;
        p3D.y = triangle[i].coord.y;
        p3D.z = triangle[i].coord.z;

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

    Math::Point a, b, c;
    a.x = p2D[0].x;
    a.y = p2D[0].y;
    b.x = p2D[1].x;
    b.y = p2D[1].y;
    c.x = p2D[2].x;
    c.y = p2D[2].y;

    if (! Math::IsInsideTriangle(a, b, c, mouse))
        return false;

    Math::Vector a2 = Math::Transform(m_objects[objRank].transform, triangle[0].coord);
    Math::Vector b2 = Math::Transform(m_objects[objRank].transform, triangle[1].coord);
    Math::Vector c2 = Math::Transform(m_objects[objRank].transform, triangle[2].coord);
    Math::Vector e  = Math::Transform(m_matView.Inverse(), Math::Vector(0.0f, 0.0f, -1.0f));
    Math::Vector f  = Math::Transform(m_matView.Inverse(), Math::Vector(
        (mouse.x*2.0f-1.0f)*m_matProj.Inverse().Get(1,1),
        (mouse.y*2.0f-1.0f)*m_matProj.Inverse().Get(2,2),
        0.0f));
    Math::Intersect(a2, b2, c2, e, f, pos);

    dist = (p2D[0].z + p2D[1].z + p2D[2].z) / 3.0f;
    return true;
}

//! Use only after world transform already set
bool CEngine::IsVisible(int objRank)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    int baseObjRank = m_objects[objRank].baseObjRank;
    if (baseObjRank == -1)
        return false;

    assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

    const auto& sphere = m_baseObjects[baseObjRank].boundingSphere;
    if (m_device->ComputeSphereVisibility(sphere.pos, sphere.radius) == Gfx::FRUSTUM_PLANE_ALL)
    {
        m_objects[objRank].visible = true;
        return true;
    }

    m_objects[objRank].visible = false;
    return false;
}

bool CEngine::TransformPoint(Math::Vector& p2D, int objRank, Math::Vector p3D)
{
    assert(objRank >= 0 && objRank < static_cast<int>(m_objects.size()));

    p3D = Math::Transform(m_objects[objRank].transform, p3D);
    p3D = Math::Transform(m_matView, p3D);

    if (p3D.z < 2.0f)
        return false;  // behind?

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



void CEngine::SetState(int state, const Color& color)
{
    if (state == m_lastState && color == m_lastColor)
        return;

    m_lastState = state;
    m_lastColor = color;

    if (state & ENG_RSTATE_TTEXTURE_BLACK)  // transparent black texture?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_ONE, BLEND_INV_SRC_COLOR);

        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_MODULATE;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = TEX_MIX_ARG_FACTOR;
        params.alphaOperation = TEX_MIX_OPER_DEFAULT;
        params.factor = color;

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & ENG_RSTATE_TTEXTURE_WHITE)  // transparent white texture?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_DST_COLOR, BLEND_ZERO);

        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_ADD;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = TEX_MIX_ARG_FACTOR;
        params.alphaOperation = TEX_MIX_OPER_DEFAULT;
        params.factor = color.Inverse();

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & ENG_RSTATE_TCOLOR_BLACK)  // transparent black color?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_ONE, BLEND_INV_SRC_COLOR);

        m_device->SetTextureEnabled(0, false);
    }
    else if (state & ENG_RSTATE_TCOLOR_WHITE)  // transparent white color?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_DST_COLOR, BLEND_ZERO);

        m_device->SetTextureEnabled(0, false);
    }
    else if (state & ENG_RSTATE_TDIFFUSE)  // diffuse color as transparent?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_SRC_ALPHA, BLEND_DST_ALPHA);

        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_REPLACE;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.alphaOperation = TEX_MIX_OPER_DEFAULT;

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & ENG_RSTATE_OPAQUE_TEXTURE) // opaque texture ?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(RENDER_STATE_BLENDING,    false);

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, TextureStageParams()); // default operation
    }
    else if (state & ENG_RSTATE_OPAQUE_COLOR) // opaque color ?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(RENDER_STATE_BLENDING,    false);

        m_device->SetTextureEnabled(0, false);
    }
    else if (state & ENG_RSTATE_TEXT)  // font rendering?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, TextureStageParams()); // default operation
    }
    else if (state & ENG_RSTATE_ALPHA)  // image with alpha channel?
    {
        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

        m_device->SetRenderState(RENDER_STATE_FOG,         true);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, true);

        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  true);

        m_device->SetAlphaTestFunc(COMP_FUNC_GREATER, 0.5f);

        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_MODULATE;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = TEX_MIX_ARG_SRC_COLOR;
        params.alphaOperation = TEX_MIX_OPER_REPLACE;
        params.alphaArg1 = TEX_MIX_ARG_TEXTURE;
        params.factor = color;

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & ENG_RSTATE_TTEXTURE_ALPHA)  // texture with alpha channel?
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);

        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  true);
        m_device->SetAlphaTestFunc(COMP_FUNC_GREATER, 0.0f);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_DST_COLOR, BLEND_INV_SRC_ALPHA);

        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_MODULATE;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = TEX_MIX_ARG_SRC_COLOR;
        params.alphaOperation = TEX_MIX_OPER_MODULATE;
        params.alphaArg1 = TEX_MIX_ARG_TEXTURE;
        params.alphaArg2 = TEX_MIX_ARG_FACTOR;
        params.factor = color;

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }
    else if (state & ENG_RSTATE_TCOLOR_ALPHA)
    {
        m_device->SetRenderState(RENDER_STATE_FOG,         false);
        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);

        m_device->SetRenderState(RENDER_STATE_BLENDING,    true);
        m_device->SetBlendFunc(BLEND_DST_COLOR, BLEND_INV_SRC_ALPHA);

        m_device->SetTextureEnabled(0, false);
    }
    else    // normal ?
    {
        m_device->SetRenderState(RENDER_STATE_ALPHA_TEST,  false);
        m_device->SetRenderState(RENDER_STATE_BLENDING,    false);

        m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, true);
        m_device->SetRenderState(RENDER_STATE_FOG,         true);

        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_DEFAULT; // default modulate
        params.alphaOperation = TEX_MIX_OPER_DEFAULT;

        m_device->SetTextureEnabled(0, true);
        m_device->SetTextureStageParams(0, params);
    }

    if (state & ENG_RSTATE_FOG)
        m_device->SetRenderState(RENDER_STATE_FOG, true);


    bool second = m_dirty;

    // TODO: I'm pretty sure this is reversed and should be m_dirty instead of !m_dirty ~krzys_h
    if ( !m_dirty && (state & ENG_RSTATE_SECOND) == 0 ) second = false;

    if ((state & ENG_RSTATE_DUAL_BLACK) && second)
    {
        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_MODULATE;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = TEX_MIX_ARG_COMPUTED_COLOR;
        params.alphaOperation = TEX_MIX_OPER_DEFAULT;
        m_device->SetTextureEnabled(1, true);
        m_device->SetTextureStageParams(1, params);
    }
    else if ((state & ENG_RSTATE_DUAL_WHITE) && second)
    {
        TextureStageParams params;
        params.colorOperation = TEX_MIX_OPER_ADD;
        params.colorArg1 = TEX_MIX_ARG_TEXTURE;
        params.colorArg2 = TEX_MIX_ARG_COMPUTED_COLOR;
        params.alphaOperation = TEX_MIX_OPER_DEFAULT;
        m_device->SetTextureEnabled(1, true);
        m_device->SetTextureStageParams(1, params);
    }
    else
    {
        m_device->SetTextureEnabled(1, false);
    }

    if (state & ENG_RSTATE_WRAP)
    {
        m_device->SetTextureStageWrap(0, TEX_WRAP_REPEAT, TEX_WRAP_REPEAT);
        m_device->SetTextureStageWrap(1, TEX_WRAP_REPEAT, TEX_WRAP_REPEAT);
    }
    else if (state & ENG_RSTATE_CLAMP)
    {
        m_device->SetTextureStageWrap(0, TEX_WRAP_CLAMP, TEX_WRAP_CLAMP);
        m_device->SetTextureStageWrap(1, TEX_WRAP_CLAMP, TEX_WRAP_CLAMP);
    }

    if (state & ENG_RSTATE_2FACE)
    {
        m_device->SetRenderState(RENDER_STATE_CULLING, false);
    }
    else
    {
        m_device->SetRenderState(RENDER_STATE_CULLING, true);
        m_device->SetCullMode(CULL_CCW);
    }

    if (state & ENG_RSTATE_LIGHT)
        m_device->SetGlobalAmbient(Color(1.0f, 1.0f, 1.0f, 1.0f));
    else
        m_device->SetGlobalAmbient(m_ambientColor[m_rankView]);


    // In interface mode, disable lighting
    if (m_interfaceMode)
        m_device->SetRenderState(RENDER_STATE_LIGHTING, false);
}

void CEngine::SetMaterial(const Material& mat)
{
    m_lastMaterial = mat;
    m_device->SetMaterial(mat);
}

void CEngine::SetViewParams(const Math::Vector &eyePt, const Math::Vector &lookatPt, const Math::Vector &upVec)
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
            GetLogger()->Error("Couldn't load texture '%s': %s, blacklisting\n", texName.c_str(), error.c_str());
            m_texBlacklist.insert(texName);
            return Texture(); // invalid texture
        }

        image = &img;
    }

    tex = m_device->CreateTexture(image, params);

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

    if (! m_foregroundName.empty())
        m_foregroundTex = LoadTexture(m_foregroundName);
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

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            if (! p2.tex1Name.empty())
            {
                if (terrain)
                    p2.tex1 = LoadTexture("textures/"+p2.tex1Name, m_terrainTexParams);
                else
                    p2.tex1 = LoadTexture("textures/"+p2.tex1Name);

                if (! p2.tex1.Valid())
                    ok = false;
            }

            if (! p2.tex2Name.empty())
            {
                if (terrain)
                    p2.tex2 = LoadTexture("textures/"+p2.tex2Name, m_terrainTexParams);
                else
                    p2.tex2 = LoadTexture("textures/"+p2.tex2Name);

                if (! p2.tex2.Valid())
                    ok = false;
            }
        }
    }

    return ok;
}

static bool IsExcludeColor(Math::Point *exclude, int x, int y)
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


bool CEngine::ChangeTextureColor(const std::string& texName,
                                 const std::string& srcName,
                                 Color colorRef1, Color colorNew1,
                                 Color colorRef2, Color colorNew2,
                                 float tolerance1, float tolerance2,
                                 Math::Point ts, Math::Point ti,
                                 Math::Point *exclude, float shift, bool hsv)
{
    CImage img;
    if (!img.Load(srcName))
    {
        std::string error = img.GetError();
        GetLogger()->Error("Couldn't load texture '%s': %s, blacklisting\n", srcName.c_str(), error.c_str());
        m_texBlacklist.insert(srcName);
        return false;
    }

    bool changeColorsNeeded = true;

    if (colorRef1.r == colorNew1.r &&
        colorRef1.g == colorNew1.g &&
        colorRef1.b == colorNew1.b &&
        colorRef2.r == colorNew2.r &&
        colorRef2.g == colorNew2.g &&
        colorRef2.b == colorNew2.b)
    {
        changeColorsNeeded = false;
    }


    int dx = img.GetSize().x;
    int dy = img.GetSize().y;

    int sx = static_cast<int>(Math::Max(ts.x*dx, 0));
    int sy = static_cast<int>(Math::Max(ts.y*dy, 0));

    int ex = static_cast<int>(Math::Min(ti.x*dx, dx));
    int ey = static_cast<int>(Math::Min(ti.y*dy, dy));


    ColorHSV cr1 = RGB2HSV(colorRef1);
    ColorHSV cn1 = RGB2HSV(colorNew1);
    ColorHSV cr2 = RGB2HSV(colorRef2);
    ColorHSV cn2 = RGB2HSV(colorNew2);

    if (changeColorsNeeded)
    {
        for (int y = sy; y < ey; y++)
        {
            for (int x = sx; x < ex; x++)
            {
                if (exclude != nullptr && IsExcludeColor(exclude, x,y) )
                    continue;

                Color color = img.GetPixel(Math::IntPoint(x, y));

                if (hsv)
                {
                    ColorHSV c = RGB2HSV(color);
                    if (c.s > 0.01f && fabs(c.h - cr1.h) < tolerance1)
                    {
                        c.h += cn1.h - cr1.h;
                        c.s += cn1.s - cr1.s;
                        c.v += cn1.v - cr1.v;
                        if (c.h < 0.0f) c.h -= 1.0f;
                        if (c.h > 1.0f) c.h += 1.0f;
                        color = HSV2RGB(c);
                        color.r = Math::Norm(color.r + shift);
                        color.g = Math::Norm(color.g + shift);
                        color.b = Math::Norm(color.b + shift);
                        img.SetPixel(Math::IntPoint(x, y), color);
                    }
                    else if (tolerance2 != -1.0f &&
                            c.s > 0.01f && fabs(c.h - cr2.h) < tolerance2)
                    {
                        c.h += cn2.h - cr2.h;
                        c.s += cn2.s - cr2.s;
                        c.v += cn2.v - cr2.v;
                        if (c.h < 0.0f) c.h -= 1.0f;
                        if (c.h > 1.0f) c.h += 1.0f;
                        color = HSV2RGB(c);
                        color.r = Math::Norm(color.r + shift);
                        color.g = Math::Norm(color.g + shift);
                        color.b = Math::Norm(color.b + shift);
                        img.SetPixel(Math::IntPoint(x, y), color);
                    }
                }
                else
                {
                    if ( fabs(color.r - colorRef1.r) +
                        fabs(color.g - colorRef1.g) +
                        fabs(color.b - colorRef1.b) < tolerance1 * 3.0f)
                    {
                        color.r = Math::Norm(colorNew1.r + color.r - colorRef1.r + shift);
                        color.g = Math::Norm(colorNew1.g + color.g - colorRef1.g + shift);
                        color.b = Math::Norm(colorNew1.b + color.b - colorRef1.b + shift);
                        img.SetPixel(Math::IntPoint(x, y), color);
                    }
                    else if (tolerance2 != -1 &&
                            fabs(color.r - colorRef2.r) +
                            fabs(color.g - colorRef2.g) +
                            fabs(color.b - colorRef2.b) < tolerance2 * 3.0f)
                    {
                        color.r = Math::Norm(colorNew2.r + color.r - colorRef2.r + shift);
                        color.g = Math::Norm(colorNew2.g + color.g - colorRef2.g + shift);
                        color.b = Math::Norm(colorNew2.b + color.b - colorRef2.b + shift);
                        img.SetPixel(Math::IntPoint(x, y), color);
                    }
                }
            }
        }
    }

    CreateOrUpdateTexture(texName, &img);

    return true;
}

bool CEngine::ChangeTextureColor(const std::string& texName,
                                 Color colorRef1, Color colorNew1,
                                 Color colorRef2, Color colorNew2,
                                 float tolerance1, float tolerance2,
                                 Math::Point ts, Math::Point ti,
                                 Math::Point *exclude, float shift, bool hsv)
{
    return ChangeTextureColor(texName, texName, colorRef1, colorNew1, colorRef2, colorNew2, tolerance1, tolerance2, ts, ti, exclude, shift, hsv);
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
        m_device->UpdateTexture((*it).second, Math::IntPoint(0, 0), img->GetData(), m_defaultTexParams.format);
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

bool CEngine::SetTexture(const std::string& name, int stage)
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

void CEngine::SetTexture(const Texture& tex, int stage)
{
    m_device->SetTexture(stage, tex);
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

    if (! m_foregroundName.empty() && !m_foregroundTex.Valid())
        m_foregroundTex = LoadTexture(m_foregroundName);
}

void CEngine::SetOverFront(bool front)
{
    m_overFront = front;
}

void CEngine::SetOverColor(const Color& color, int mode)
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

void CEngine::SetTextureFilterMode(TexFilter value)
{
    if(m_defaultTexParams.filter == value && m_terrainTexParams.filter == value) return;

    m_defaultTexParams.filter = m_terrainTexParams.filter = value;
    m_defaultTexParams.mipmap = m_terrainTexParams.mipmap = (value == TEX_FILTER_TRILINEAR);
    ReloadAllTextures();
}

TexFilter CEngine::GetTextureFilterMode()
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
    return m_device->IsShadowMappingSupported() && m_device->GetMaxTextureStageCount() >= 3;
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
    return IsShadowMappingSupported() && m_device->GetMaxTextureStageCount() >= 3;
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

const Math::Matrix& CEngine::GetMatView()
{
    return m_matView;
}

const Math::Matrix& CEngine::GetMatProj()
{
    return m_matProj;
}

Math::Vector CEngine::GetEyePt()
{
    return m_eyePt;
}

Math::Vector CEngine::GetLookatPt()
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

bool CEngine::IsVisiblePoint(const Math::Vector &pos)
{
    return Math::Distance(m_eyePt, pos) <= (m_deepView[0] * m_clippingDistance);
}

void CEngine::UpdateMatProj()
{
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProj);
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

    m_systemUtils->GetCurrentTimeStamp(m_currentFrameTime);
    float diff = m_systemUtils->TimeStampDiff(m_lastFrameTime, m_currentFrameTime, STU_SEC);
    if (diff > 1.0f)
    {
        m_systemUtils->CopyTimeStamp(m_lastFrameTime, m_currentFrameTime);

        m_fps = m_fpsCounter / diff;
        m_fpsCounter = 0;
    }

    if (! m_render)
        return;

    m_statisticTriangle = 0;
    m_lastState = -1;
    m_lastColor = Color(-1.0f);
    m_lastMaterial = Material();

    m_lightMan->UpdateLights();

    Color color;
    if (m_cloud->GetLevel() != 0.0f)  // clouds?
        color = m_backgroundCloudDown;
    else
        color = m_backgroundColorDown;

    m_device->SetClearColor(color);

    // Begin the scene
    m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, true);

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

    m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, false);

    UpdateGroundSpotTextures();

    DrawPlanet();  // draws the planets
    m_cloud->Draw();  // draws the clouds

    // Display the objects

    m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, true);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, true);
    m_device->SetRenderState(RENDER_STATE_FOG, true);

    float fogStart = m_deepView[m_rankView] * m_fogStart[m_rankView] * m_clippingDistance;
    float fogEnd = m_deepView[m_rankView] * m_clippingDistance;
    m_device->SetFogParams(FOG_LINEAR, m_fogColor[m_rankView], fogStart, fogEnd, 1.0f);

    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProj);
    m_device->SetTransform(TRANSFORM_VIEW, m_matView);

    // TODO: This causes a rendering artifact and I can't see anything that breaks if you just comment it out
    // So I'll just leave it like that for now ~krzys_h
    //m_water->DrawBack();  // draws water background

    CProfiler::StartPerformanceCounter(PCNT_RENDER_TERRAIN);

    // Draw terrain

    m_lightMan->UpdateDeviceLights(ENG_OBJTYPE_TERRAIN);

    UseShadowMapping(true);

    for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
    {
        if (! m_objects[objRank].used)
            continue;

        if (m_objects[objRank].type != ENG_OBJTYPE_TERRAIN)
            continue;

        if (! m_objects[objRank].drawWorld)
            continue;

        m_device->SetTransform(TRANSFORM_WORLD, m_objects[objRank].transform);

        if (! IsVisible(objRank))
            continue;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            SetTexture(p2.tex1, 0);
            SetTexture(p2.tex2, 1);

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];

                SetMaterial(p3.material);
                SetState(p3.state);

                DrawObject(p3);
            }
        }
    }

    if (!m_qualityShadows)
        UseShadowMapping(false);

    // Draws the old-style shadow spots, if shadow mapping disabled
    if (!m_shadowMapping)
        DrawShadowSpots();

    CProfiler::StopPerformanceCounter(PCNT_RENDER_TERRAIN);

    // Draw other objects

    CProfiler::StartPerformanceCounter(PCNT_RENDER_OBJECTS);

    bool transparent = false;

    for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
    {
        if (! m_objects[objRank].used)
            continue;

        if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
            continue;

        if (! m_objects[objRank].drawWorld)
            continue;

        m_device->SetTransform(TRANSFORM_WORLD, m_objects[objRank].transform);

        if (! IsVisible(objRank))
            continue;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (! p1.used)
            continue;

        m_lightMan->UpdateDeviceLights(m_objects[objRank].type);

        for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            SetTexture(p2.tex1, 0);
            SetTexture(p2.tex2, 1);

            for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];

                if (m_objects[objRank].transparency != 0.0f)  // transparent ?
                {
                    transparent = true;
                    continue;
                }

                SetMaterial(p3.material);
                SetState(p3.state);

                DrawObject(p3);
            }
        }
    }

    UseShadowMapping(false);

    // Draw transparent objects

    if (transparent)
    {
        int tState = ENG_RSTATE_TTEXTURE_BLACK | ENG_RSTATE_2FACE;
        Color tColor = Color(68.0f / 255.0f, 68.0f / 255.0f, 68.0f / 255.0f, 68.0f / 255.0f);

        for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
        {
            if (! m_objects[objRank].used)
                continue;

            if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
                continue;

            if (! m_objects[objRank].drawWorld)
                continue;

            m_device->SetTransform(TRANSFORM_WORLD, m_objects[objRank].transform);

            if (! IsVisible(objRank))
                continue;

            int baseObjRank = m_objects[objRank].baseObjRank;
            if (baseObjRank == -1)
                continue;

            assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

            EngineBaseObject& p1 = m_baseObjects[baseObjRank];
            if (! p1.used)
                continue;

            m_lightMan->UpdateDeviceLights(m_objects[objRank].type);

            for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
            {
                EngineBaseObjTexTier& p2 = p1.next[l2];

                SetTexture(p2.tex1, 0);
                SetTexture(p2.tex2, 1);

                for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
                {
                    EngineBaseObjDataTier& p3 = p2.next[l3];

                    if (m_objects[objRank].transparency == 0.0f)
                        continue;

                    SetMaterial(p3.material);
                    SetState(tState, tColor);

                    DrawObject(p3);
                }
            }
        }
    }

    CProfiler::StopPerformanceCounter(PCNT_RENDER_OBJECTS);

    m_lightMan->UpdateDeviceLights(ENG_OBJTYPE_TERRAIN);

    if (m_debugLights)
        m_device->DebugLights();

    if (m_debugDumpLights)
    {
        m_debugDumpLights = false;
        m_lightMan->DebugDumpLights();
    }

    CProfiler::StartPerformanceCounter(PCNT_RENDER_WATER);
    m_water->DrawSurf(); // draws water surface
    CProfiler::StopPerformanceCounter(PCNT_RENDER_WATER);

    m_device->SetRenderState(RENDER_STATE_LIGHTING, false);

    RenderPendingDebugDraws();

    if (m_debugGoto)
    {
        Math::Matrix worldMatrix;
        worldMatrix.LoadIdentity();
        m_device->SetTransform(TRANSFORM_WORLD, worldMatrix);

        SetState(ENG_RSTATE_OPAQUE_COLOR);

        for (const auto& line : m_displayGoto)
        {
            m_device->DrawPrimitive(PRIMITIVE_LINE_STRIP, line.data(), line.size());
        }
    }
    m_displayGoto.clear();

    CProfiler::StartPerformanceCounter(PCNT_RENDER_PARTICLE_WORLD);
    m_particle->DrawParticle(SH_WORLD); // draws the particles of the 3D world
    CProfiler::StopPerformanceCounter(PCNT_RENDER_PARTICLE_WORLD);

    m_device->SetRenderState(RENDER_STATE_LIGHTING, true);

    m_lightning->Draw();                     // draws lightning

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
    std::unique_ptr<unsigned char[]> mipmap = MakeUniqueArray<unsigned char>(4 * newWidth * newHeight);

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
    std::unique_ptr<unsigned char[]> blured = MakeUniqueArray<unsigned char>(4 * newWidth * newHeight);

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
                    int xp = Math::Clamp(x + i, 0, newWidth - 1);
                    int yp = Math::Clamp(y + j, 0, newHeight - 1);

                    float weight = matrix[i + 3][j + 3];

                    int index = 4 * (newWidth * yp + xp);

                    for (int k = 0; k < 4; k++)
                        color[k] += weight * mipmap[index + k];
                }
            }

            int index = 4 * (newWidth * y + x);

            for (int k = 0; k < 4; k++)
            {
                float value = Math::Clamp(color[k], 0.0f, 255.0f);
                blured[index + k] = static_cast<unsigned char>(value);
            }
        }
    }

    // create SDL surface and final texture
    ImageData image;
    image.surface = SDL_CreateRGBSurfaceFrom(blured.get(), newWidth, newHeight, 32, 0, 0, 0, 0, 0xFF000000);

    TextureCreateParams params;
    params.filter = TEX_FILTER_BILINEAR;
    params.format = TEX_IMG_RGBA;
    params.mipmap = false;

    m_capturedWorldTexture = m_device->CreateTexture(&image, params);

    SDL_FreeSurface(image.surface);

    m_captureWorld = false;
    m_worldCaptured = true;
}

void CEngine::DrawCaptured3DScene()
{
    Math::Matrix identity;

    m_device->SetTransform(TRANSFORM_PROJECTION, identity);
    m_device->SetTransform(TRANSFORM_VIEW, identity);
    m_device->SetTransform(TRANSFORM_WORLD, identity);

    m_device->SetRenderState(RENDER_STATE_BLENDING, false);
    m_device->SetRenderState(RENDER_STATE_CULLING, false);

    Vertex vertices[4];

    vertices[0] = Vertex(Math::Vector(-1.0f, -1.0f, 0.0f), Math::Vector(0.0f, 1.0f, 0.0f), Math::Point(0.0f, 0.0f));
    vertices[1] = Vertex(Math::Vector(1.0f, -1.0f, 0.0f), Math::Vector(0.0f, 1.0f, 0.0f), Math::Point(1.0f, 0.0f));
    vertices[2] = Vertex(Math::Vector(-1.0f, 1.0f, 0.0f), Math::Vector(0.0f, 1.0f, 0.0f), Math::Point(0.0f, 1.0f));
    vertices[3] = Vertex(Math::Vector(1.0f, 1.0f, 0.0f), Math::Vector(0.0f, 1.0f, 0.0f), Math::Point(1.0f, 1.0f));

    m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, false);

    m_device->SetTexture(TEXTURE_PRIMARY, m_capturedWorldTexture);
    m_device->SetTextureEnabled(TEXTURE_PRIMARY, true);
    m_device->SetTextureEnabled(TEXTURE_SECONDARY, false);

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertices, 4);
}

void CEngine::RenderDebugSphere(const Math::Sphere& sphere, const Math::Matrix& transform, const Gfx::Color& color)
{
    static constexpr int LONGITUDE_DIVISIONS = 16;
    static constexpr int LATITUDE_DIVISIONS = 8;
    static constexpr int NUM_LINE_STRIPS = 2 + LONGITUDE_DIVISIONS + LATITUDE_DIVISIONS;
    static constexpr int VERTS_IN_LINE_STRIP = 32;

    static std::array<Math::Vector, NUM_LINE_STRIPS * VERTS_IN_LINE_STRIP> verticesTemplate = []
    {
        std::array<Math::Vector, NUM_LINE_STRIPS * VERTS_IN_LINE_STRIP> vertices;

        auto SpherePoint = [&](float latitude, float longitude)
        {
            float latitudeAngle = (latitude - 0.5f) * 2.0f * Math::PI;
            float longitudeAngle = longitude * 2.0f * Math::PI;
            return Math::Vector(sinf(latitudeAngle) * cosf(longitudeAngle),
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


    const int firstDraw = m_pendingDebugDraws.firsts.size();
    const int firstVert = m_pendingDebugDraws.vertices.size();

    m_pendingDebugDraws.firsts.resize(m_pendingDebugDraws.firsts.size() + NUM_LINE_STRIPS);
    m_pendingDebugDraws.counts.resize(m_pendingDebugDraws.counts.size() + NUM_LINE_STRIPS);
    m_pendingDebugDraws.vertices.resize(m_pendingDebugDraws.vertices.size() + verticesTemplate.size());

    for (int i = 0; i < NUM_LINE_STRIPS; ++i)
    {
        m_pendingDebugDraws.firsts[i + firstDraw] = firstVert + i * VERTS_IN_LINE_STRIP;
    }

    for (int i = 0; i < NUM_LINE_STRIPS; ++i)
    {
        m_pendingDebugDraws.counts[i + firstDraw] = VERTS_IN_LINE_STRIP;
    }

    for (std::size_t i = 0; i < verticesTemplate.size(); ++i)
    {
        auto pos = Math::MatrixVectorMultiply(transform, sphere.pos + verticesTemplate[i] * sphere.radius);
        m_pendingDebugDraws.vertices[i + firstVert] = VertexCol{pos, color};
    }
}

void CEngine::RenderDebugBox(const Math::Vector& mins, const Math::Vector& maxs, const Math::Matrix& transform, const Gfx::Color& color)
{
    static constexpr int NUM_LINE_STRIPS = 4;
    static constexpr int VERTS_IN_LINE_STRIP = 4;

    const int firstDraw = m_pendingDebugDraws.firsts.size();
    const int firstVert = m_pendingDebugDraws.vertices.size();

    m_pendingDebugDraws.firsts.resize(m_pendingDebugDraws.firsts.size() + NUM_LINE_STRIPS);
    m_pendingDebugDraws.counts.resize(m_pendingDebugDraws.counts.size() + NUM_LINE_STRIPS);
    m_pendingDebugDraws.vertices.resize(m_pendingDebugDraws.vertices.size() + NUM_LINE_STRIPS * VERTS_IN_LINE_STRIP);

    for (int i = 0; i < NUM_LINE_STRIPS; ++i)
    {
        m_pendingDebugDraws.firsts[i + firstDraw] = firstVert + (i * VERTS_IN_LINE_STRIP);
    }

    for (int i = 0; i < NUM_LINE_STRIPS; ++i)
    {
        m_pendingDebugDraws.counts[i + firstDraw] = NUM_LINE_STRIPS;
    }

    auto vert = m_pendingDebugDraws.vertices.begin() + firstVert;

    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, mins.y, mins.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, mins.y, mins.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, maxs.y, mins.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, maxs.y, maxs.z}), color};

    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, mins.y, maxs.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, mins.y, mins.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, maxs.y, mins.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, maxs.y, mins.z}), color};

    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, mins.y, maxs.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, mins.y, maxs.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, maxs.y, maxs.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, maxs.y, mins.z}), color};

    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, mins.y, mins.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, mins.y, maxs.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{maxs.x, maxs.y, maxs.z}), color};
    *vert++ = VertexCol{Math::MatrixVectorMultiply(transform, Math::Vector{mins.x, maxs.y, maxs.z}), color};
}

void CEngine::RenderPendingDebugDraws()
{
    if (m_pendingDebugDraws.firsts.empty()) return;

    m_device->SetTransform(TRANSFORM_WORLD, Math::Matrix{});

    SetState(ENG_RSTATE_OPAQUE_COLOR);

    m_device->DrawPrimitives(PRIMITIVE_LINE_STRIP,
                             m_pendingDebugDraws.vertices.data(),
                             m_pendingDebugDraws.firsts.data(),
                             m_pendingDebugDraws.counts.data(),
                             m_pendingDebugDraws.firsts.size());

    m_pendingDebugDraws.firsts.clear();
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

    // If no shadow map texture exists, create it
    if (m_shadowMap.id == 0)
    {
        int width = 256, height = 256;
        int depth;

        if (m_offscreenShadowRendering)
        {
            width = height = m_offscreenShadowRenderingResolution;

            FramebufferParams params;
            params.width = params.height = width;
            params.depth = depth = 32;
            params.colorAttachment = FramebufferParams::AttachmentType::None;
            params.depthAttachment = FramebufferParams::AttachmentType::Texture;

            CFramebuffer *framebuffer = m_device->CreateFramebuffer("shadow", params);
            if (framebuffer == nullptr)
            {
                GetLogger()->Error("Could not create shadow mapping framebuffer, disabling dynamic shadows\n");
                m_shadowMapping = false;
                m_offscreenShadowRendering = false;
                m_qualityShadows = false;
                CProfiler::StopPerformanceCounter(PCNT_RENDER_SHADOW_MAP);
                return;
            }

            m_shadowMap.id = framebuffer->GetDepthTexture();
            m_shadowMap.size = Math::IntPoint(width, height);
        }
        else
        {
            int min = Math::Min(m_size.x, m_size.y);

            for (int i = 0; i < 16; i++)
            {
                if (min < (1 << i)) break;

                width = height = 1 << i;
            }

            depth = m_app->GetInstance().GetVideoConfig().depthSize;

            m_shadowMap = m_device->CreateDepthTexture(width, height, depth);
        }

        GetLogger()->Info("Created shadow map texture: %dx%d, depth %d\n", width, height, depth);
    }

    if (m_offscreenShadowRendering)
    {
        m_device->GetFramebuffer("shadow")->Bind();
    }

    m_device->SetRenderMode(RENDER_MODE_SHADOW);
    m_device->Clear();

    // change state to rendering shadow maps
    m_device->SetColorMask(false, false, false, false);
    m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, true);
    m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, true);
    m_device->SetRenderState(RENDER_STATE_BLENDING, false);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(RENDER_STATE_FOG, false);
    m_device->SetRenderState(RENDER_STATE_CULLING, false);
    m_device->SetRenderState(RENDER_STATE_ALPHA_TEST, true);
    m_device->SetAlphaTestFunc(COMP_FUNC_GREATER, 0.5f);
    m_device->SetRenderState(RENDER_STATE_DEPTH_BIAS, true);
    m_device->SetDepthBias(2.0f, 8.0f);
    m_device->SetViewport(0, 0, m_shadowMap.size.x, m_shadowMap.size.y);

    // recompute matrices
    Math::Vector worldUp(0.0f, 1.0f, 0.0f);
    Math::Vector lightDir = Math::Vector(1.0f, 2.0f, -1.0f);
    Math::Vector dir = m_lookatPt - m_eyePt;
    dir.y = 0.0f;
    dir.Normalize();

    float dist = m_shadowRange;
    float depth = 200.0f;

    if (dist < 0.5f)
    {
        float scale = log(m_shadowMap.size.x) / log(2.0f) - 6.5f;
        dist = 75.0f * scale;
    }

    Math::Vector pos = m_lookatPt + 0.25f * dist * dir;

    {
        // To prevent 'shadow shimmering', we ensure that the position only moves in texel-sized
        // increments. To do this we transform the position to a space where the light's forward/right/up
        // axes are aligned with the x/y/z axes (not necessarily in that order, and +/- signs don't matter).
        Math::Matrix lightRotation;
        Math::LoadViewMatrix(lightRotation, Math::Vector{}, lightDir, worldUp);
        pos = Math::MatrixVectorMultiply(lightRotation, pos);
        // ...then we round to the nearest worldUnitsPerTexel:
        const float worldUnitsPerTexel = (dist * 2.0f) / m_shadowMap.size.x;
        pos /= worldUnitsPerTexel;
        pos.x = round(pos.x);
        pos.y = round(pos.y);
        pos.z = round(pos.z);
        pos *= worldUnitsPerTexel;
        // ...and convert back to world space.
        pos = Math::MatrixVectorMultiply(lightRotation.Inverse(), pos);
    }

    Math::Vector lookAt = pos - lightDir;

    Math::LoadOrthoProjectionMatrix(m_shadowProjMat, -dist, dist, -dist, dist, -depth, depth);
    Math::LoadViewMatrix(m_shadowViewMat, pos, lookAt, worldUp);

    Math::Matrix scaleMat;
    Math::LoadScaleMatrix(scaleMat, Math::Vector(1.0f, 1.0f, -1.0f));
    m_shadowViewMat = Math::MultiplyMatrices(scaleMat, m_shadowViewMat);

    Math::Matrix temporary = Math::MultiplyMatrices(m_shadowProjMat, m_shadowViewMat);
    m_shadowTextureMat = Math::MultiplyMatrices(m_shadowBias, temporary);

    m_shadowViewMat = Math::MultiplyMatrices(scaleMat, m_shadowViewMat);

    m_device->SetTransform(TRANSFORM_PROJECTION, m_shadowProjMat);
    m_device->SetTransform(TRANSFORM_VIEW, m_shadowViewMat);

    m_device->SetTexture(0, 0);
    m_device->SetTexture(1, 0);
    m_device->SetTexture(2, 0);

    // render objects into shadow map
    for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
    {
        if (!m_objects[objRank].used)
            continue;

        bool terrain = (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN);

        if (terrain)
        {
            if (m_terrainShadows)
            {
                m_device->SetRenderState(RENDER_STATE_ALPHA_TEST, false);
                m_device->SetRenderState(RENDER_STATE_CULLING, true);
                m_device->SetCullMode(CULL_CCW);
            }
            else
                continue;
        }
        else
        {
            m_device->SetRenderState(RENDER_STATE_ALPHA_TEST, true);
            m_device->SetRenderState(RENDER_STATE_CULLING, false);
        }

        m_device->SetTransform(TRANSFORM_WORLD, m_objects[objRank].transform);

        if (!IsVisible(objRank))
            continue;

        int baseObjRank = m_objects[objRank].baseObjRank;
        if (baseObjRank == -1)
            continue;

        assert(baseObjRank >= 0 && baseObjRank < static_cast<int>(m_baseObjects.size()));

        EngineBaseObject& p1 = m_baseObjects[baseObjRank];
        if (!p1.used)
            continue;

        for (int l2 = 0; l2 < static_cast<int>(p1.next.size()); l2++)
        {
            EngineBaseObjTexTier& p2 = p1.next[l2];

            SetTexture(p2.tex1, 0);

            for (int l3 = 0; l3 < static_cast<int>(p2.next.size()); l3++)
            {
                EngineBaseObjDataTier& p3 = p2.next[l3];

                DrawObject(p3);
            }
        }
    }

    m_device->SetRenderState(RENDER_STATE_DEPTH_BIAS, false);
    m_device->SetDepthBias(0.0f, 0.0f);
    m_device->SetRenderState(RENDER_STATE_ALPHA_TEST, false);
    m_device->SetRenderState(RENDER_STATE_CULLING, false);
    m_device->SetCullMode(CULL_CW);

    if (m_offscreenShadowRendering)     // shadow map texture already have depth information, just unbind it
    {
        m_device->GetFramebuffer("shadow")->Unbind();
    }
    else    // copy depth buffer to shadow map
    {
        m_device->CopyFramebufferToTexture(m_shadowMap, 0, 0, 0, 0, m_shadowMap.size.x, m_shadowMap.size.y);
    }

    // restore default state
    m_device->SetViewport(0, 0, m_size.x, m_size.y);

    m_device->SetColorMask(true, true, true, true);
    m_device->Clear();

    CProfiler::StopPerformanceCounter(PCNT_RENDER_SHADOW_MAP);

    m_device->SetRenderMode(RENDER_MODE_NORMAL);
    m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, false);
}

void CEngine::UseShadowMapping(bool enable)
{
    if (!m_shadowMapping) return;
    if (m_shadowMap.id == 0) return;

    if (enable)
    {
        m_device->SetShadowColor(m_shadowColor);
        m_device->SetTransform(TRANSFORM_SHADOW, m_shadowTextureMat);
        m_device->SetTexture(TEXTURE_SHADOW, m_shadowMap);
        m_device->SetTextureStageWrap(TEXTURE_SHADOW, TEX_WRAP_CLAMP_TO_BORDER, TEX_WRAP_CLAMP_TO_BORDER);
        m_device->SetRenderState(RENDER_STATE_SHADOW_MAPPING, true);
    }
    else
    {
        m_device->SetRenderState(RENDER_STATE_SHADOW_MAPPING, false);
        m_device->SetTexture(TEXTURE_SHADOW, 0);
    }
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
                    GetLogger()->Error("Could not create MSAA framebuffer, disabling MSAA\n");
                    m_multisample = 1;
                }
            }

            framebuffer->Bind();

            m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, true);
            m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, true);

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

void CEngine::DrawObject(const EngineBaseObjDataTier& p4)
{
    if (p4.staticBufferId != 0)
    {
        m_device->DrawStaticBuffer(p4.staticBufferId);

        if (p4.type == ENG_TRIANGLE_TYPE_TRIANGLES)
            m_statisticTriangle += p4.vertices.size() / 3;
        else
            m_statisticTriangle += p4.vertices.size() - 2;
    }
    else
    {
        if (p4.type == ENG_TRIANGLE_TYPE_TRIANGLES)
        {
            m_device->DrawPrimitive(PRIMITIVE_TRIANGLES, &p4.vertices[0], p4.vertices.size());
            m_statisticTriangle += p4.vertices.size() / 3;
        }
        else
        {
            m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, &p4.vertices[0], p4.vertices.size() );
            m_statisticTriangle += p4.vertices.size() - 2;
        }
    }
}

void CEngine::DrawInterface()
{
    m_device->SetRenderMode(RENDER_MODE_INTERFACE);

    m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, false);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(RENDER_STATE_FOG, false);

    SetInterfaceCoordinates();

    // Force new state to disable lighting
    m_interfaceMode = true;
    m_lastState = -1;
    SetState(Gfx::ENG_RSTATE_NORMAL);

    // Draw the entire interface
    Ui::CInterface* interface = CRobotMain::GetInstancePointer()->GetInterface();
    if (interface != nullptr && m_renderInterface)
    {
        interface->Draw();
    }

    m_interfaceMode = false;
    m_lastState = -1;
    SetState(Gfx::ENG_RSTATE_NORMAL);

    if (!m_screenshotMode && m_renderInterface)
    {
        CProfiler::StartPerformanceCounter(PCNT_RENDER_PARTICLE_IFACE);
        m_particle->DrawParticle(SH_INTERFACE);  // draws the particles of the interface
        CProfiler::StopPerformanceCounter(PCNT_RENDER_PARTICLE_IFACE);
    }

    // 3D objects drawn in front of interface
    if (m_drawFront)
    {
        m_device->SetRenderMode(RENDER_MODE_NORMAL);

        // Display the objects
        m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, true);

        m_device->SetTransform(TRANSFORM_PROJECTION, m_matProj);

        m_device->SetGlobalAmbient(m_ambientColor[m_rankView]);
        m_device->SetRenderState(RENDER_STATE_LIGHTING, true);

        m_device->SetRenderState(RENDER_STATE_FOG, true);

        float fogStart = m_deepView[m_rankView] * m_fogStart[m_rankView] * m_clippingDistance;
        float fogEnd = m_deepView[m_rankView] * m_clippingDistance;
        m_device->SetFogParams(FOG_LINEAR, m_fogColor[m_rankView], fogStart, fogEnd, 1.0f);

        m_device->SetTransform(TRANSFORM_VIEW, m_matView);

        for (int objRank = 0; objRank < static_cast<int>(m_objects.size()); objRank++)
        {
            if (! m_objects[objRank].used)
                continue;

            if (m_objects[objRank].type == ENG_OBJTYPE_TERRAIN)
                continue;

            if (! m_objects[objRank].drawFront)
                continue;

            m_device->SetTransform(TRANSFORM_WORLD, m_objects[objRank].transform);

            if (! IsVisible(objRank))
                continue;

            int baseObjRank = m_objects[objRank].baseObjRank;
            if (baseObjRank == -1)
                continue;

            assert(baseObjRank >= 0 && baseObjRank < static_cast<int>( m_baseObjects.size() ));

            EngineBaseObject& p1 = m_baseObjects[baseObjRank];
            if (! p1.used)
                continue;

            m_lightMan->UpdateDeviceLights(m_objects[objRank].type);

            for (int l2 = 0; l2 < static_cast<int>( p1.next.size() ); l2++)
            {
                EngineBaseObjTexTier& p2 = p1.next[l2];

                SetTexture(p2.tex1, 0);
                SetTexture(p2.tex2, 1);

                for (int l3 = 0; l3 < static_cast<int>( p2.next.size() ); l3++)
                {
                    EngineBaseObjDataTier& p3 = p2.next[l3];

                    SetMaterial(p3.material);
                    SetState(p3.state);

                    DrawObject(p3);
                }
            }
        }

        m_particle->DrawParticle(SH_FRONT);  // draws the particles of the 3D world

        m_device->SetRenderState(RENDER_STATE_DEPTH_TEST, false);
        m_device->SetRenderState(RENDER_STATE_LIGHTING, false);
        m_device->SetRenderState(RENDER_STATE_FOG, false);

        m_device->SetRenderMode(RENDER_MODE_INTERFACE);

        SetInterfaceCoordinates();
    }

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

    m_device->SetRenderMode(RENDER_MODE_NORMAL);
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
        Math::Point min, max;
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
            CImage shadowImg(Math::IntPoint(256, 256));
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
                                intensity = Math::Point(ppx-cx, ppy-cy).Length()/dot;

                            ppx -= min.x;  // on the texture
                            ppy -= min.y;
                            Math::IntPoint pp(ppx, ppy);

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
                            Math::Vector pos;
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

                            Math::IntPoint pp(ix, iy);

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

                        float intensity = 1.0f - Math::Point(ix, iy).Length() / dot;
                        if (intensity <= 0.0f)
                            continue;

                        intensity *= m_groundMark.intensity;

                        int j = (ix+dot) + (iy+dot) * m_groundMark.dx;
                        if (m_groundMark.table[j] == 1)  // green ?
                        {
                            Math::IntPoint pp(ppx, ppy);
                            Gfx::Color color = shadowImg.GetPixel(pp);
                            color.r *= Math::Norm(1.0f-intensity);
                            color.g *= 1.0f;
                            color.b *= Math::Norm(1.0f-intensity);
                            shadowImg.SetPixel(pp, color);
                        }
                        if (m_groundMark.table[j] == 2)  // red ?
                        {
                            Math::IntPoint pp(ppx, ppy);
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
                        Math::Vector pos(
                            x / 4.0f / 254.0f * 3200.0f - 1600.0f,
                            0.0f,
                            y / 4.0f / 254.0f * 3200.0f - 1600.0f
                        );
                        TerrainRes res = m_terrain->GetResource(pos);
                        Math::IntPoint p(x-min.x, y-min.y);
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
                Math::IntPoint size = m_displayGotoImage->GetSize();
                for (float x = min.x; x < max.x; x += 1.0f)
                {
                    for (float y = min.y; y < max.y; y += 1.0f)
                    {
                        int px = x / 4.0f / 254.0f * size.x;
                        int py = y / 4.0f / 254.0f * size.y;
                        // This can happen because the shadow??.png textures have a 1 pixel margin around them
                        if (px < 0 || px >= size.x || py < 0 || py >= size.y)
                            continue;
                        shadowImg.SetPixelInt(Math::IntPoint(x-min.x, y-min.y), m_displayGotoImage->GetPixelInt(Math::IntPoint(px, py)));
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
    m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, false);

    Math::Matrix matrix;
    matrix.LoadIdentity();
    m_device->SetTransform(TRANSFORM_WORLD, matrix);


    Material material;
    material.diffuse = Color(1.0f, 1.0f, 1.0f);
    material.ambient = Color(0.5f, 0.5f, 0.5f);
    SetMaterial(material);

    // TODO: create a separate texture
    SetTexture("textures/effect03.png");

    Math::Point ts, ti;

    float dp = 0.5f/256.0f;
    ts.y = 192.0f/256.0f;
    ti.y = 224.0f/256.0f;
    ts.y += dp;
    ti.y -= dp;

    Math::Vector n(0.0f, 1.0f, 0.0f);

    float startDeepView = m_deepView[m_rankView] * m_fogStart[m_rankView] * m_clippingDistance;
    float endDeepView = m_deepView[m_rankView] * m_clippingDistance;

    float lastIntensity = -1.0f;
    for (int i = 0; i < static_cast<int>( m_shadowSpots.size() ); i++)
    {
        if (m_shadowSpots[i].hide || !m_shadowSpots[i].used)
            continue;

        Math::Vector pos = m_shadowSpots[i].pos;  // pos = center of the shadow on the ground

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

            D = Math::Distance(m_eyePt, pos);
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

            D = Math::Distance(m_eyePt, pos);
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


        Math::Vector corner[4];

        if (m_shadowSpots[i].type == ENG_SHADOW_NORM)
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

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, Math::Point(radius, radius));
            corner[0].x = rot.x;
            corner[0].z = rot.y;
            corner[0].y = 0.0f;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, Math::Point(-radius, radius));
            corner[1].x = rot.x;
            corner[1].z = rot.y;
            corner[1].y = 0.0f;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, Math::Point(radius, -radius));
            corner[2].x = rot.x;
            corner[2].z = rot.y;
            corner[2].y = 0.0f;

            rot = Math::RotatePoint(-m_shadowSpots[i].angle, Math::Point(-radius, -radius));
            corner[3].x = rot.x;
            corner[3].z = rot.y;
            corner[3].y = 0.0f;

            if (m_shadowSpots[i].type == ENG_SHADOW_WORM)
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

        corner[0] = Math::CrossProduct(corner[0], m_shadowSpots[i].normal);
        corner[1] = Math::CrossProduct(corner[1], m_shadowSpots[i].normal);
        corner[2] = Math::CrossProduct(corner[2], m_shadowSpots[i].normal);
        corner[3] = Math::CrossProduct(corner[3], m_shadowSpots[i].normal);

        corner[0] += pos;
        corner[1] += pos;
        corner[2] += pos;
        corner[3] += pos;

        ts.x += dp;
        ti.x -= dp;

        Vertex vertex[4] =
        {
            Vertex(corner[1], n, Math::Point(ts.x, ts.y)),
            Vertex(corner[0], n, Math::Point(ti.x, ts.y)),
            Vertex(corner[3], n, Math::Point(ts.x, ti.y)),
            Vertex(corner[2], n, Math::Point(ti.x, ti.y))
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
            SetState(ENG_RSTATE_TTEXTURE_WHITE, Color(intensity, intensity, intensity, intensity));
        }

        m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
        AddStatisticTriangle(2);
    }

    m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, true);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, true);
}

void CEngine::DrawBackground()
{
    m_device->SetRenderMode(RENDER_MODE_INTERFACE);

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

    m_device->SetRenderMode(RENDER_MODE_NORMAL);
}

void CEngine::DrawBackgroundGradient(const Color& up, const Color& down)
{
    Math::Point p1(0.0f, 0.5f);
    Math::Point p2(1.0f, 1.0f);

    Color color[3] =
    {
        up,
        down,
        Color(0.0f, 0.0f, 0.0f, 0.0f)
    };

    SetState(ENG_RSTATE_OPAQUE_COLOR);

    m_device->SetTransform(TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(TRANSFORM_WORLD, m_matWorldInterface);

    VertexCol vertex[4] =
    {
        VertexCol(Math::Vector(p1.x, p1.y, 0.0f), color[1]),
        VertexCol(Math::Vector(p1.x, p2.y, 0.0f), color[0]),
        VertexCol(Math::Vector(p2.x, p1.y, 0.0f), color[1]),
        VertexCol(Math::Vector(p2.x, p2.y, 0.0f), color[0])
    };

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

void CEngine::DrawBackgroundImage()
{
    Math::Point p1, p2;
    p1.x = 0.0f;
    p1.y = 0.0f;
    p2.x = 1.0f;
    p2.y = 1.0f;

    Math::Vector n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

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

    Math::Point backgroundScale;
    backgroundScale.x = static_cast<float>(m_backgroundTex.originalSize.x) / static_cast<float>(m_backgroundTex.size.x);
    backgroundScale.y = static_cast<float>(m_backgroundTex.originalSize.y) / static_cast<float>(m_backgroundTex.size.y);

    u2 *= backgroundScale.x;
    v2 *= backgroundScale.y;

    if (m_backgroundScale)
    {
        Math::Point scale;
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

    SetTexture(m_backgroundTex);
    SetState(ENG_RSTATE_OPAQUE_TEXTURE | ENG_RSTATE_WRAP);

    m_device->SetTransform(TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(TRANSFORM_WORLD, m_matWorldInterface);

    Vertex vertex[4] =
    {
        Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2)),
        Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1)),
        Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2)),
        Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1))
    };

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

void CEngine::DrawPlanet()
{
    if (! m_planet->PlanetExist())
        return;

    m_device->SetRenderState(RENDER_STATE_DEPTH_WRITE, false);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, false);
    m_device->SetRenderState(RENDER_STATE_FOG, false);

    m_device->SetTransform(TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(TRANSFORM_WORLD, m_matWorldInterface);

    m_planet->Draw();  // draws the planets
}

void CEngine::DrawForegroundImage()
{
    if (m_foregroundName.empty())
        return;

    Math::Vector n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    Math::Point p1(0.0f, 0.0f);
    Math::Point p2(1.0f, 1.0f);

    float u1 = -m_eyeDirH/(Math::PI*0.6f)+Math::PI*0.5f;
    float u2 = u1+0.50f;

    float v1 = 0.2f;
    float v2 = 1.0f;


    Vertex vertex[4] =
    {
        Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2)),
        Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1)),
        Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2)),
        Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1))
    };

    SetTexture(m_foregroundTex);
    SetState(ENG_RSTATE_CLAMP | ENG_RSTATE_TTEXTURE_BLACK);

    m_device->SetRenderMode(RENDER_MODE_INTERFACE);

    m_device->SetTransform(TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(TRANSFORM_WORLD, m_matWorldInterface);

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);

    m_device->SetRenderMode(RENDER_MODE_NORMAL);
}

void CEngine::DrawOverColor()
{
    if ((m_overColor == Color(0.0f, 0.0f, 0.0f, 0.0f) && m_overMode == ENG_RSTATE_TCOLOR_BLACK) ||
        (m_overColor == Color(1.0f, 1.0f, 1.0f, 1.0f) && m_overMode == ENG_RSTATE_TCOLOR_WHITE))
        return;

    Math::Point p1(0.0f, 0.0f);
    Math::Point p2(1.0f, 1.0f);

    Color color[3] =
    {
        m_overColor,
        m_overColor,
        Color(0.0f, 0.0f, 0.0f, 0.0f)
    };

    m_device->SetRenderMode(RENDER_MODE_INTERFACE);

    SetState(m_overMode);

    m_device->SetTransform(TRANSFORM_VIEW, m_matViewInterface);
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(TRANSFORM_WORLD, m_matWorldInterface);
    m_device->SetRenderState(RENDER_STATE_LIGHTING, false);

    VertexCol vertex[4] =
    {
        VertexCol(Math::Vector(p1.x, p1.y, 0.0f), color[1]),
        VertexCol(Math::Vector(p1.x, p2.y, 0.0f), color[0]),
        VertexCol(Math::Vector(p2.x, p1.y, 0.0f), color[1]),
        VertexCol(Math::Vector(p2.x, p2.y, 0.0f), color[0])
    };

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);

    m_device->SetRenderMode(RENDER_MODE_NORMAL);
}

void CEngine::DrawHighlight()
{
    Math::Point min, max;
    min.x = 1000000.0f;
    min.y = 1000000.0f;
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

    Math::Point p1 = m_highlightP1;
    Math::Point p2 = m_highlightP2;

    int nbOut = 0;
    if (p1.x < 0.0f || p1.x > 1.0f) nbOut++;
    if (p1.y < 0.0f || p1.y > 1.0f) nbOut++;
    if (p2.x < 0.0f || p2.x > 1.0f) nbOut++;
    if (p2.y < 0.0f || p2.y > 1.0f) nbOut++;
    if (nbOut > 2)
        return;

    SetState(ENG_RSTATE_OPAQUE_COLOR);

    float d = 0.5f + sinf(m_highlightTime * 6.0f) * 0.5f;
    d *= (p2.x - p1.x) * 0.1f;
    p1.x += d;
    p1.y += d;
    p2.x -= d;
    p2.y -= d;

    Color color(1.0f, 1.0f, 0.0f);  // yellow

    VertexCol line[3] =
        {
            VertexCol(Math::Vector(), color),
            VertexCol(Math::Vector(), color),
            VertexCol(Math::Vector(), color)
        };

    float dx = (p2.x - p1.x) / 5.0f;
    float dy = (p2.y - p1.y) / 5.0f;

    line[0].coord = Math::Vector(p1.x, p1.y + dy, 0.0f);
    line[1].coord = Math::Vector(p1.x, p1.y, 0.0f);
    line[2].coord = Math::Vector(p1.x + dx, p1.y, 0.0f);
    m_device->DrawPrimitive(PRIMITIVE_LINE_STRIP, line, 3);

    line[0].coord = Math::Vector(p2.x - dx, p1.y, 0.0f);
    line[1].coord = Math::Vector(p2.x, p1.y, 0.0f);
    line[2].coord = Math::Vector(p2.x, p1.y + dy, 0.0f);
    m_device->DrawPrimitive(PRIMITIVE_LINE_STRIP, line, 3);

    line[0].coord = Math::Vector(p2.x, p2.y - dy, 0.0f);
    line[1].coord = Math::Vector(p2.x, p2.y, 0.0f);
    line[2].coord = Math::Vector(p2.x - dx, p2.y, 0.0f);
    m_device->DrawPrimitive(PRIMITIVE_LINE_STRIP, line, 3);

    line[0].coord = Math::Vector(p1.x + dx, p2.y, 0.0f);
    line[1].coord = Math::Vector(p1.x, p2.y, 0.0f);
    line[2].coord = Math::Vector(p1.x, p2.y - dy, 0.0f);
    m_device->DrawPrimitive(PRIMITIVE_LINE_STRIP, line, 3);
}

void CEngine::DrawMouse()
{
    MouseMode mode = m_app->GetMouseMode();
    if (mode != MOUSE_ENGINE && mode != MOUSE_BOTH)
        return;

    SetWindowCoordinates();

    Material material;
    material.diffuse = Color(1.0f, 1.0f, 1.0f);
    material.ambient = Color(0.5f, 0.5f, 0.5f);

    m_device->SetMaterial(material);
    m_device->SetTexture(0, m_miceTexture);

    Math::Point mousePos = CInput::GetInstancePointer()->GetMousePos();
    Math::IntPoint pos(mousePos.x * m_size.x, m_size.y - mousePos.y * m_size.y);
    pos.x -= MOUSE_TYPES.at(m_mouseType).hotPoint.x;
    pos.y -= MOUSE_TYPES.at(m_mouseType).hotPoint.y;

    Math::IntPoint shadowPos;
    shadowPos.x = pos.x + 4;
    shadowPos.y = pos.y - 3;

    SetState(ENG_RSTATE_TCOLOR_WHITE);
    DrawMouseSprite(shadowPos, MOUSE_SIZE, MOUSE_TYPES.at(m_mouseType).iconShadow);

    SetState(MOUSE_TYPES.at(m_mouseType).mode1);
    DrawMouseSprite(pos, MOUSE_SIZE, MOUSE_TYPES.at(m_mouseType).icon1);

    SetState(MOUSE_TYPES.at(m_mouseType).mode2);
    DrawMouseSprite(pos, MOUSE_SIZE, MOUSE_TYPES.at(m_mouseType).icon2);

    SetInterfaceCoordinates();
}

void CEngine::DrawMouseSprite(Math::IntPoint pos, Math::IntPoint size, int icon)
{
    if (icon == -1)
        return;

    Math::IntPoint p1 = pos;
    Math::IntPoint p2 = p1 + size;

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

    Vertex vertex[4] =
    {
        Vertex(Math::Vector(p1.x, p2.y, 0.0f), normal, Math::Point(u1, v2)),
        Vertex(Math::Vector(p1.x, p1.y, 0.0f), normal, Math::Point(u1, v1)),
        Vertex(Math::Vector(p2.x, p2.y, 0.0f), normal, Math::Point(u2, v2)),
        Vertex(Math::Vector(p2.x, p1.y, 0.0f), normal, Math::Point(u2, v1))
    };

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    AddStatisticTriangle(2);
}

void CEngine::DrawStats()
{
    if (!m_showStats)
        return;

    float height = m_text->GetAscent(FONT_COMMON, 13.0f);
    float width = 0.4f;
    const int TOTAL_LINES = 22;

    Math::Point pos(0.05f * m_size.x/m_size.y, 0.05f + TOTAL_LINES * height);

    SetState(ENG_RSTATE_TCOLOR_ALPHA);

    Gfx::Color black(0.0f, 0.0f, 0.0f, 0.75f);

    Math::Point margin = Math::Point(5.f / m_size.x, 5.f / m_size.y);

    VertexCol vertex[4] =
    {
        VertexCol(Math::Vector(pos.x         - margin.x, pos.y - (TOTAL_LINES + 1) * height - margin.y, 0.0f), black),
        VertexCol(Math::Vector(pos.x         - margin.x, pos.y + height                     + margin.y, 0.0f), black),
        VertexCol(Math::Vector(pos.x + width + margin.x, pos.y - (TOTAL_LINES + 1) * height - margin.y, 0.0f), black),
        VertexCol(Math::Vector(pos.x + width + margin.x, pos.y + height                     + margin.y, 0.0f), black)
    };

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertex, 4);

    SetState(ENG_RSTATE_TEXT);

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
    SetState(ENG_RSTATE_TEXT);

    Math::Point pos(0.98f, 0.98f-m_text->GetAscent(FONT_COMMON, 15.0f));
    m_text->DrawText(m_timerText, FONT_COMMON, 15.0f, pos, 1.0f, TEXT_ALIGN_RIGHT, 0, Color(1.0f, 1.0f, 1.0f, 1.0f));
}

void CEngine::AddBaseObjTriangles(int baseObjRank, const std::vector<Gfx::ModelTriangle>& triangles)
{
    std::vector<VertexTex2> vs(3, VertexTex2());

    for (const auto& triangle : triangles)
    {
        vs[0] = triangle.p1;
        vs[1] = triangle.p2;
        vs[2] = triangle.p3;

        Material material;
        material.ambient = triangle.ambient;
        material.diffuse = triangle.diffuse;
        material.specular = triangle.specular;

        int state = GetEngineState(triangle);

        std::string tex1Name;
        if (!triangle.tex1Name.empty())
            tex1Name = "objects/" + triangle.tex1Name;

        std::string tex2Name;
        if (triangle.variableTex2)
            tex2Name = GetSecondTexture();
        else
            tex2Name = triangle.tex2Name;

        AddBaseObjTriangles(baseObjRank, vs, material, state, tex1Name, tex2Name);
    }
}

int CEngine::GetEngineState(const ModelTriangle& triangle)
{
    int state = 0;

    if (!triangle.tex2Name.empty() || triangle.variableTex2)
        state |= ENG_RSTATE_DUAL_BLACK;

    switch (triangle.transparentMode)
    {
        case ModelTransparentMode::None:
            break;

        case ModelTransparentMode::AlphaChannel:
            state |= ENG_RSTATE_ALPHA;
            break;

        case ModelTransparentMode::MapBlackToAlpha:
            state |= ENG_RSTATE_TTEXTURE_BLACK;
            break;

        case ModelTransparentMode::MapWhiteToAlpha:
            state |= ENG_RSTATE_TTEXTURE_WHITE;
            break;
    }

    switch (triangle.specialMark)
    {
        case ModelSpecialMark::None:
            break;

        case ModelSpecialMark::Part1:
            state |= ENG_RSTATE_PART1;
            break;

        case ModelSpecialMark::Part2:
            state |= ENG_RSTATE_PART2;
            break;

        case ModelSpecialMark::Part3:
            state |= ENG_RSTATE_PART3;
            break;
    }

    if (triangle.doubleSided)
        state |= ENG_RSTATE_2FACE;

    return state;
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

    Math::Vector pos = m_shadowSpots[shadowRank].pos;
    float radius = m_shadowSpots[shadowRank].radius;

    Math::Vector n[20];
    Math::Vector norm;
    int i = 0;

    m_terrain->GetNormal(norm, pos);
    n[i++] = norm;
    n[i++] = norm;
    n[i++] = norm;

    Math::Vector shPos = pos;
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

    norm.LoadZero();
    for (int j = 0; j < i; j++)
    {
        norm += n[j];
    }
    norm /= static_cast<float>(i);  // average vector

    m_shadowSpots[shadowRank].normal = norm;
}

int CEngine::AddStaticMesh(const std::string& key, const CModelMesh* mesh, const Math::Matrix& worldMatrix)
{
    int baseObjRank = -1;

    auto it = m_staticMeshBaseObjects.find(key);
    if (it == m_staticMeshBaseObjects.end())
    {
        baseObjRank = CreateBaseObject();
        AddBaseObjTriangles(baseObjRank, mesh->GetTriangles());
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
    SetObjectShadowSpotType(objRank, ENG_SHADOW_NORM);
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

const Math::Matrix& CEngine::GetStaticMeshWorldMatrix(int meshHandle)
{
    int objRank = meshHandle;
    return m_objects[objRank].transform;
}

void CEngine::SetStaticMeshTransparency(int meshHandle, float value)
{
    int objRank = meshHandle;
    SetObjectTransparency(objRank, value);
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

void CEngine::AddDebugGotoLine(std::vector<Gfx::VertexCol> line)
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
    m_device->SetTransform(TRANSFORM_VIEW,       m_matViewInterface);
    m_device->SetTransform(TRANSFORM_PROJECTION, m_matProjInterface);
    m_device->SetTransform(TRANSFORM_WORLD,      m_matWorldInterface);
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
    Math::Matrix matWorldWindow;
    matWorldWindow.LoadIdentity();

    Math::Matrix matViewWindow;
    matViewWindow.LoadIdentity();

    Math::Matrix matProjWindow;
    Math::LoadOrthoProjectionMatrix(matProjWindow, 0.0f, m_size.x, m_size.y, 0.0f, -1.0f, 1.0f);

    m_device->SetTransform(TRANSFORM_VIEW,       matViewWindow);
    m_device->SetTransform(TRANSFORM_PROJECTION, matProjWindow);
    m_device->SetTransform(TRANSFORM_WORLD,      matWorldWindow);
}

} // namespace Gfx
