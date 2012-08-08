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
#include "graphics/engine/lightman.h"
#include "graphics/engine/text.h"
#include "math/geometry.h"

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


Gfx::CEngine::CEngine(CInstanceManager *iMan, CApplication *app)
{
    m_iMan   = iMan;
    m_app    = app;
    m_device = nullptr;

    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_ENGINE, this);
    m_app = app;

    m_lightMan      = nullptr;
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
    m_frontsizeName = "";  // no front image
    m_hiliteRank[0] = -1;  // empty list
    m_eyePt    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_lookatPt = Math::Vector(0.0f, 0.0f, 1.0f);
    m_drawWorld = true;
    m_drawFront = false;
    m_limitLOD[0] = 100.0f;
    m_limitLOD[1] = 200.0f;
    m_particuleDensity = 1.0f;
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
    m_iMan   = nullptr;
    m_app    = nullptr;
    m_device = nullptr;

    m_sound = nullptr;
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

bool Gfx::CEngine::Create()
{
    m_size = m_lastSize = m_app->GetVideoConfig().size;

    m_lightMan   = new Gfx::CLightManager(m_iMan, this);
    m_text       = new Gfx::CText(m_iMan, this);
    /* TODO:
    m_particle   = new Gfx::CParticle(m_iMan, this);
    m_water      = new Gfx::CWater(m_iMan, this);
    m_cloud      = new Gfx::CCloud(m_iMan, this);
    m_lightning  = new Gfx::CLightning(m_iMan, this);
    m_planet     = new Gfx::CPlanet(m_iMan, this);*/

    m_text->SetDevice(m_device);
    if (! m_text->Create())
    {
        m_error = std::string("Error creating CText: ") + m_text->GetError();
        return false;
    }


    m_matWorldInterface.LoadIdentity();
    m_matViewInterface.LoadIdentity();
    Math::LoadOrthoProjectionMatrix(m_matProjInterface, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    m_device->SetClearColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));

    m_device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, false);

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

    /* TODO:
    delete m_particle;
    m_particle = nullptr;

    delete m_water;
    m_water = nullptr;

    delete m_cloud;
    m_cloud = nullptr;

    delete m_lightning;
    m_lightning = nullptr;

    delete m_planet;
    m_planet = nullptr;*/
}

void Gfx::CEngine::ResetAfterDeviceChanged()
{
    // TODO
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

void Gfx::CEngine::SetTexture(const std::string &name, int stage)
{
    std::map<std::string, Gfx::Texture>::iterator it = m_texNameMap.find(name);
    if (it != m_texNameMap.end())
        m_device->SetTexture(stage, (*it).second);

    // TODO if not present...
}

void Gfx::CEngine::SetMaterial(const Gfx::Material &mat)
{
    m_device->SetMaterial(mat);
}

void Gfx::CEngine::SetState(int state, Gfx::Color color)
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

bool Gfx::CEngine::Render()
{
    m_statisticTriangle = 0;

    m_lastState = -1;
    SetState(Gfx::ENG_RSTATE_NORMAL);

    m_device->BeginScene();

    SetUp3DView();

    if (! Draw3DScene() )
        return false;

    SetUpInterfaceView();

    if (! DrawInterface() )
        return false;

    m_device->EndScene();

    return true;
}

void Gfx::CEngine::SetUp3DView()
{
    // TODO
}

bool Gfx::CEngine::Draw3DScene()
{
    // TODO
    return true;
}

void Gfx::CEngine::SetUpInterfaceView()
{
    m_device->SetTransform(Gfx::TRANSFORM_WORLD,      m_matWorldInterface);
    m_device->SetTransform(Gfx::TRANSFORM_VIEW,       m_matViewInterface);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, m_matProjInterface);
}

bool Gfx::CEngine::DrawInterface()
{
    Gfx::VertexCol vertices[3] =
    {
        Gfx::VertexCol(Math::Vector( 0.25f,  0.25f, 0.0f), Gfx::Color(1.0f, 0.0f, 0.0f)),
        Gfx::VertexCol(Math::Vector( 0.75f,  0.25f, 0.0f), Gfx::Color(0.0f, 1.0f, 0.0f)),
        Gfx::VertexCol(Math::Vector( 0.5f,   0.75f, 0.0f), Gfx::Color(0.0f, 0.0f, 1.0f))
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, vertices, 3);

    DrawMouse();

    std::vector<Gfx::FontMetaChar> format;
    for (int i = 0; i < 10; ++i)
        format.push_back(Gfx::FONT_COLOBOT_BOLD | Gfx::FONT_HIGHLIGHT_CONST);
    for (int i = 0; i < 10; ++i)
        format.push_back(Gfx::FONT_COLOBOT_ITALIC | Gfx::FONT_HIGHLIGHT_KEY);
    for (int i = 0; i < 10; ++i)
        format.push_back(Gfx::FONT_COURIER | Gfx::FONT_HIGHLIGHT_LINK);
    for (int i = 0; i < 5; ++i)
        format.push_back(Gfx::FONT_COURIER_BOLD | Gfx::FONT_HIGHLIGHT_REM);

    m_text->DrawText("abcdefghijklmnopqrstuvwxyz ąęśćółńż", Gfx::FONT_COLOBOT, 15.0f, Math::Point(0.25f, 0.2f), 1.0f, Gfx::TEXT_ALIGN_LEFT, 0);
    float h = m_text->GetHeight(Gfx::FONT_COLOBOT, 15.0f);
    m_text->DrawText("abcdefghijklmnopqrstuvwxyz ąęśćółńż", format, 13.0f, Math::Point(0.25f, 0.2f - h), 1.0f, Gfx::TEXT_ALIGN_LEFT, 0);

    return true;
}

Math::IntSize Gfx::CEngine::GetWindowSize()
{
    return m_size;
}

Math::IntSize Gfx::CEngine::GetLastWindowSize()
{
    return m_lastSize;
}

/** Conversion of the position of the mouse from window coords to interface coords:
     - x: 0=left, 1=right
     - y: 0=down, 1=up */
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

bool Gfx::CEngine::GetPause()
{
    return m_pause;
}

Math::Vector Gfx::CEngine::GetLookatPt()
{
    return m_lookatPt;
}

Math::Vector Gfx::CEngine::GetEyePt()
{
    return m_eyePt;
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

void Gfx::CEngine::AddStatisticTriangle(int count)
{
    m_statisticTriangle += count;
}

void Gfx::CEngine::SetShowStat(bool show)
{
    m_showStats = show;
}

bool Gfx::CEngine::GetShowStat()
{
    return m_showStats;
}

