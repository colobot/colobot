// * This file is part of the COLOBOT source code
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


#include "graphics/opengl/gldevice.h"

#include "common/config.h"
#include "common/image.h"
#include "common/logger.h"

#include "math/geometry.h"


// Using GLEW so only glew.h is needed
#include <GL/glew.h>

#include <SDL.h>

#include <cassert>


// Graphics module namespace
namespace Gfx {

GLDeviceConfig::GLDeviceConfig()
{
    LoadDefault();
}

void GLDeviceConfig::LoadDefault()
{
    DeviceConfig::LoadDefault();

    hardwareAccel = true;

    redSize = 8;
    blueSize = 8;
    greenSize = 8;
    alphaSize = 8;
    depthSize = 24;

    vboMode = VBO_MODE_AUTO;
}




CGLDevice::CGLDevice(const GLDeviceConfig &config)
{
    m_config = config;
    m_lighting = false;
    m_lastVboId = 0;
    m_multitextureAvailable = false;
    m_vboAvailable = false;
}


CGLDevice::~CGLDevice()
{
}

void CGLDevice::DebugHook()
{
    /* This function is only called here, so it can be used
     * as a breakpoint when debugging using gDEBugger */
    glColor3i(0, 0, 0);
}

bool CGLDevice::Create()
{
    GetLogger()->Info("Creating CDevice\n");

    static bool glewInited = false;

    if (!glewInited)
    {
        glewInited = true;

        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            GetLogger()->Error("GLEW initialization failed\n");
            return false;
        }

        m_multitextureAvailable = glewIsSupported("GL_ARB_multitexture GL_ARB_texture_env_combine");
        if (!m_multitextureAvailable)
            GetLogger()->Warn("GLEW reports multitexturing not supported - graphics quality will be degraded!\n");

        if (m_config.vboMode == VBO_MODE_ENABLE)
        {
            GetLogger()->Info("VBO enabled by override - using VBOs\n");
            m_vboAvailable = true;
        }
        else if (m_config.vboMode == VBO_MODE_DISABLE)
        {
            GetLogger()->Info("VBO disabled by override - using display lists\n");
            m_vboAvailable = false;
        }
        else
        {
            GetLogger()->Info("Auto-detecting VBO support\n");
            m_vboAvailable = glewIsSupported("GL_ARB_vertex_buffer_object");

            if (m_vboAvailable)
                GetLogger()->Info("Detected ARB_vertex_buffer_object extension - using VBOs\n");
            else
                GetLogger()->Info("No ARB_vertex_buffer_object extension present - using display lists\n");
        }
    }

    // This is mostly done in all modern hardware by default
    // DirectX doesn't even allow the option to turn off perspective correction anymore
    // So turn it on permanently
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // To avoid problems with scaling & lighting
    glEnable(GL_RESCALE_NORMAL);

    // Minimal depth bias to avoid Z-fighting
    SetDepthBias(0.001f);

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    int numLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &numLights);

    m_lights        = std::vector<Light>(numLights, Light());
    m_lightsEnabled = std::vector<bool> (numLights, false);

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextures);

    m_currentTextures    = std::vector<Texture>           (maxTextures, Texture());
    m_texturesEnabled    = std::vector<bool>              (maxTextures, false);
    m_textureStageParams = std::vector<TextureStageParams>(maxTextures, TextureStageParams());

    GetLogger()->Info("CDevice created successfully\n");

    return true;
}

void CGLDevice::Destroy()
{
    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    DestroyAllTextures();

    m_lights.clear();
    m_lightsEnabled.clear();

    m_currentTextures.clear();
    m_texturesEnabled.clear();
    m_textureStageParams.clear();
}

void CGLDevice::ConfigChanged(const GLDeviceConfig& newConfig)
{
    m_config = newConfig;

    // Reset state
    m_lighting = false;
    Destroy();
    Create();
}

void CGLDevice::SetUseVbo(bool vboAvailable)
{
    m_vboAvailable = vboAvailable;
}

bool CGLDevice::GetUseVbo()
{
    return m_vboAvailable;
}

void CGLDevice::BeginScene()
{
    Clear();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMat.Array());

    UpdateModelviewMatrix();
}

void CGLDevice::EndScene()
{
}

void CGLDevice::Clear()
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CGLDevice::SetTransform(TransformType type, const Math::Matrix &matrix)
{
    if      (type == TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        UpdateModelviewMatrix();
    }
    else if (type == TRANSFORM_VIEW)
    {
        m_viewMat = matrix;
        UpdateModelviewMatrix();
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = matrix;
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_projectionMat.Array());
    }
    else
    {
        assert(false);
    }
}

const Math::Matrix& CGLDevice::GetTransform(TransformType type)
{
    if      (type == TRANSFORM_WORLD)
        return m_worldMat;
    else if (type == TRANSFORM_VIEW)
        return m_viewMat;
    else if (type == TRANSFORM_PROJECTION)
        return m_projectionMat;
    else
        assert(false);

    return m_worldMat; // to avoid warning
}

void CGLDevice::MultiplyTransform(TransformType type, const Math::Matrix &matrix)
{
    if      (type == TRANSFORM_WORLD)
    {
        m_worldMat = Math::MultiplyMatrices(m_worldMat, matrix);
        UpdateModelviewMatrix();
    }
    else if (type == TRANSFORM_VIEW)
    {
        m_viewMat = Math::MultiplyMatrices(m_viewMat, matrix);
        UpdateModelviewMatrix();
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = Math::MultiplyMatrices(m_projectionMat, matrix);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_projectionMat.Array());
    }
    else
    {
        assert(false);
    }
}

void CGLDevice::UpdateModelviewMatrix()
{
    m_modelviewMat = Math::MultiplyMatrices(m_viewMat, m_worldMat);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    glMultMatrixf(m_modelviewMat.Array());

    if (m_lighting)
    {
        for (int index = 0; index < static_cast<int>( m_lights.size() ); ++index)
            UpdateLightPosition(index);
    }
}

void CGLDevice::SetMaterial(const Material &material)
{
    m_material = material;

    glMaterialfv(GL_FRONT, GL_AMBIENT,  m_material.ambient.Array());
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  m_material.diffuse.Array());
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_material.specular.Array());
}

const Material& CGLDevice::GetMaterial()
{
    return m_material;
}

int CGLDevice::GetMaxLightCount()
{
    return m_lights.size();
}

void CGLDevice::SetLight(int index, const Light &light)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lights[index] = light;

    // Indexing from GL_LIGHT0 should always work
    glLightfv(GL_LIGHT0 + index, GL_AMBIENT,  const_cast<GLfloat*>(light.ambient.Array()));
    glLightfv(GL_LIGHT0 + index, GL_DIFFUSE,  const_cast<GLfloat*>(light.diffuse.Array()));
    glLightfv(GL_LIGHT0 + index, GL_SPECULAR, const_cast<GLfloat*>(light.specular.Array()));

    glLightf(GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION,  light.attenuation0);
    glLightf(GL_LIGHT0 + index, GL_LINEAR_ATTENUATION,    light.attenuation1);
    glLightf(GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, light.attenuation2);

    if (light.type == LIGHT_SPOT)
    {
        glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, light.spotAngle * Math::RAD_TO_DEG);
        glLightf(GL_LIGHT0 + index, GL_SPOT_EXPONENT, light.spotIntensity);
    }
    else
    {
        glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180.0f);
    }

    UpdateLightPosition(index);
}

void CGLDevice::UpdateLightPosition(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    Math::Matrix mat = m_viewMat;
    mat.Set(1, 4, 0.0f);
    mat.Set(2, 4, 0.0f);
    mat.Set(3, 4, 0.0f);
    glMultMatrixf(mat.Array());

    if (m_lights[index].type == LIGHT_SPOT)
    {
        GLfloat direction[4] = { -m_lights[index].direction.x, -m_lights[index].direction.y, -m_lights[index].direction.z, 1.0f };
        glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction);
    }

    if (m_lights[index].type == LIGHT_DIRECTIONAL)
    {
        GLfloat position[4] = { -m_lights[index].direction.x, -m_lights[index].direction.y, -m_lights[index].direction.z, 0.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }
    else
    {
        glLoadIdentity();
        glScalef(1.0f, 1.0f, -1.0f);
        glMultMatrixf(m_viewMat.Array());

        GLfloat position[4] = { m_lights[index].position.x, m_lights[index].position.y, m_lights[index].position.z, 1.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }

    glPopMatrix();
}

const Light& CGLDevice::GetLight(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    return m_lights[index];
}

void CGLDevice::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lightsEnabled[index] = enabled;

    if (enabled)
        glEnable(GL_LIGHT0 + index);
    else
        glDisable(GL_LIGHT0 + index);
}

bool CGLDevice::GetLightEnabled(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    return m_lightsEnabled[index];
}

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Texture CGLDevice::CreateTexture(CImage *image, const TextureCreateParams &params)
{
    ImageData *data = image->GetData();
    if (data == NULL)
    {
        GetLogger()->Error("Invalid texture data\n");
        return Texture(); // invalid texture
    }

    return CreateTexture(data, params);
}

Texture CGLDevice::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture result;

    result.size.x = data->surface->w;
    result.size.y = data->surface->h;

    // Use & enable 1st texture stage
    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    // Set params

    GLint minF = 0;
    if      (params.minFilter == TEX_MIN_FILTER_NEAREST)                minF = GL_NEAREST;
    else if (params.minFilter == TEX_MIN_FILTER_LINEAR)                 minF = GL_LINEAR;
    else if (params.minFilter == TEX_MIN_FILTER_NEAREST_MIPMAP_NEAREST) minF = GL_NEAREST_MIPMAP_NEAREST;
    else if (params.minFilter == TEX_MIN_FILTER_LINEAR_MIPMAP_NEAREST)  minF = GL_LINEAR_MIPMAP_NEAREST;
    else if (params.minFilter == TEX_MIN_FILTER_NEAREST_MIPMAP_LINEAR)  minF = GL_NEAREST_MIPMAP_LINEAR;
    else if (params.minFilter == TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR)   minF = GL_LINEAR_MIPMAP_LINEAR;
    else  assert(false);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);

    GLint magF = 0;
    if      (params.magFilter == TEX_MAG_FILTER_NEAREST) magF = GL_NEAREST;
    else if (params.magFilter == TEX_MAG_FILTER_LINEAR)  magF = GL_LINEAR;
    else  assert(false);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);

    if (params.mipmap)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);


    bool convert = false;
    GLenum sourceFormat = 0;

    if (params.format == TEX_IMG_RGB)
    {
        sourceFormat = GL_RGB;
        result.alpha = false;
    }
    else if (params.format == TEX_IMG_BGR)
    {
        sourceFormat = GL_BGR;
        result.alpha = false;
    }
    else if (params.format == TEX_IMG_RGBA)
    {
        sourceFormat = GL_RGBA;
        result.alpha = true;
    }
    else if (params.format == TEX_IMG_BGRA)
    {
        sourceFormat = GL_BGRA;
        result.alpha = true;
    }
    else if (params.format == TEX_IMG_AUTO)
    {
        if (data->surface->format->Amask != 0)
        {
            if ((data->surface->format->Amask == 0xFF000000) &&
                (data->surface->format->Rmask == 0x00FF0000) &&
                (data->surface->format->Gmask == 0x0000FF00) &&
                (data->surface->format->Bmask == 0x000000FF))
            {
                sourceFormat = GL_BGRA;
                result.alpha = true;
            }
            else if ((data->surface->format->Amask == 0xFF000000) &&
                     (data->surface->format->Bmask == 0x00FF0000) &&
                     (data->surface->format->Gmask == 0x0000FF00) &&
                     (data->surface->format->Rmask == 0x000000FF))
            {
                sourceFormat = GL_RGBA;
                result.alpha = true;
            }
            else
            {
                sourceFormat = GL_RGBA;
                convert = true;
            }
        }
        else
        {
            if ((data->surface->format->Rmask == 0xFF0000) &&
                (data->surface->format->Gmask == 0x00FF00) &&
                (data->surface->format->Bmask == 0x0000FF))
            {
                sourceFormat = GL_BGR;
                result.alpha = false;
            }
            else if ((data->surface->format->Bmask == 0xFF0000) &&
                     (data->surface->format->Gmask == 0x00FF00) &&
                     (data->surface->format->Rmask == 0x0000FF))
            {
                sourceFormat = GL_RGB;
                result.alpha = false;
            }
            else
            {
                sourceFormat = GL_RGBA;
                convert = true;
            }
        }
    }
    else
        assert(false);

    SDL_Surface* actualSurface = data->surface;
    SDL_Surface* convertedSurface = nullptr;

    if (convert)
    {
        SDL_PixelFormat format;
        format.BytesPerPixel = 4;
        format.BitsPerPixel = 32;
        format.alpha = 0;
        format.colorkey = 0;
        format.Aloss = format.Bloss = format.Gloss = format.Rloss = 0;
        format.Amask = 0xFF000000;
        format.Ashift = 24;
        format.Bmask = 0x00FF0000;
        format.Bshift = 16;
        format.Gmask = 0x0000FF00;
        format.Gshift = 8;
        format.Rmask = 0x000000FF;
        format.Rshift = 0;
        format.palette = nullptr;
        convertedSurface = SDL_ConvertSurface(data->surface, &format, SDL_SWSURFACE);
        if (convertedSurface != nullptr)
            actualSurface = convertedSurface;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, actualSurface->w, actualSurface->h,
                 0, sourceFormat, GL_UNSIGNED_BYTE, actualSurface->pixels);

    SDL_FreeSurface(convertedSurface);


    // Restore the previous state of 1st stage
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    if (! m_texturesEnabled[0])
        glDisable(GL_TEXTURE_2D);

    return result;
}

void CGLDevice::DestroyTexture(const Texture &texture)
{
    // Unbind the texture if in use anywhere
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
    {
        if (m_currentTextures[index] == texture)
            SetTexture(index, Texture()); // set to invalid texture
    }

    glDeleteTextures(1, &texture.id);

    auto it = m_allTextures.find(texture);
    if (it != m_allTextures.end())
        m_allTextures.erase(it);
}

void CGLDevice::DestroyAllTextures()
{
    // Unbind all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        SetTexture(index, Texture());

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();
}

int CGLDevice::GetMaxTextureStageCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void CGLDevice::SetTexture(int index, const Texture &texture)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_currentTextures[index].id == texture.id;

    m_currentTextures[index] = texture; // remember the new value

    if (!m_multitextureAvailable && index != 0)
        return;

    if (same)
        return; // nothing to do

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Params need to be updated for the new bound texture
    SetTextureStageParams(index, m_textureStageParams[index]);
}

void CGLDevice::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_currentTextures[index].id == textureId)
        return; // nothing to do

    m_currentTextures[index].id = textureId;

    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, textureId);

    // Params need to be updated for the new bound texture
    SetTextureStageParams(index, m_textureStageParams[index]);
}

/**
  Returns the previously assigned texture or invalid texture if the given stage is not enabled. */
Texture CGLDevice::GetTexture(int index)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    return m_currentTextures[index];
}

void CGLDevice::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_texturesEnabled[index] == enabled;

    m_texturesEnabled[index] = enabled;

    if (same)
        return; // nothing to do

    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    if (enabled)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);
}

bool CGLDevice::GetTextureEnabled(int index)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    return m_texturesEnabled[index];
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void CGLDevice::SetTextureStageParams(int index, const TextureStageParams &params)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index] = params;

    if (!m_multitextureAvailable && index != 0)
        return;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    if      (params.wrapS == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    else if (params.wrapS == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (params.wrapT == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    else if (params.wrapT == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);

    // Texture env setting is silly without multitexturing
    if (!m_multitextureAvailable)
        return;

    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, params.factor.Array());

    // To save some trouble
    if ( (params.colorOperation == TEX_MIX_OPER_DEFAULT) &&
         (params.alphaOperation == TEX_MIX_OPER_DEFAULT) )
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        goto after_tex_operations;
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

    // Only these modes of getting color & alpha are used
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

    // Color operation

    if (params.colorOperation == TEX_MIX_OPER_DEFAULT)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
        goto after_tex_color;
    }
    else if (params.colorOperation == TEX_MIX_OPER_REPLACE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    else if (params.colorOperation == TEX_MIX_OPER_MODULATE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    else if (params.colorOperation == TEX_MIX_OPER_ADD)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
    else if (params.colorOperation == TEX_MIX_OPER_SUBTRACT)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_SUBTRACT);
    else  assert(false);

    // Color arg1
    if (params.colorArg1 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    else if (params.colorArg1 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    else if (params.colorArg1 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    else if (params.colorArg1 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
    else  assert(false);

    // Color arg2
    if (params.colorArg2 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    else if (params.colorArg2 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
    else if (params.colorArg2 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    else if (params.colorArg2 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
    else  assert(false);


after_tex_color:

    // Alpha operation
    if (params.alphaOperation == TEX_MIX_OPER_DEFAULT)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
        goto after_tex_operations;
    }
    else if (params.alphaOperation == TEX_MIX_OPER_REPLACE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    else if (params.alphaOperation == TEX_MIX_OPER_MODULATE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    else if (params.alphaOperation == TEX_MIX_OPER_ADD)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
    else if (params.alphaOperation == TEX_MIX_OPER_SUBTRACT)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_SUBTRACT);
    else  assert(false);

    // Alpha arg1
    if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
    else if (params.alphaArg1 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg1 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg1 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
    else  assert(false);

    // Alpha arg2
    if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
    else if (params.alphaArg2 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg2 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg2 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
    else  assert(false);

after_tex_operations: ;
}

void CGLDevice::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index].wrapS = wrapS;
    m_textureStageParams[index].wrapT = wrapT;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    if      (wrapS == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    else if (wrapS == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (wrapT == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    else if (wrapT == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);
}

TextureStageParams CGLDevice::GetTextureStageParams(int index)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    return m_textureStageParams[index];
}

GLenum TranslateGfxPrimitive(PrimitiveType type)
{
    GLenum flag = 0;
    switch (type)
    {
        case PRIMITIVE_POINTS:         flag = GL_POINTS; break;
        case PRIMITIVE_LINES:          flag = GL_LINES; break;
        case PRIMITIVE_LINE_STRIP:     flag = GL_LINE_STRIP; break;
        case PRIMITIVE_TRIANGLES:      flag = GL_TRIANGLES; break;
        case PRIMITIVE_TRIANGLE_STRIP: flag = GL_TRIANGLE_STRIP; break;
        default: assert(false); break;
    }
    return flag;
}

void CGLDevice::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount,
                              Color color)
{
    Vertex* vs = const_cast<Vertex*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].normal));

    if (m_multitextureAvailable)
        glClientActiveTexture(GL_TEXTURE0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
}

void CGLDevice::DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                              Color color)
{
    VertexTex2* vs = const_cast<VertexTex2*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].normal));

    if (m_multitextureAvailable)
        glClientActiveTexture(GL_TEXTURE0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    if (m_multitextureAvailable)
    {
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord2));
    }

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1
    if (m_multitextureAvailable)
    {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void CGLDevice::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
{
    VertexCol* vs = const_cast<VertexCol*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].color));

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

unsigned int CGLDevice::CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vboAvailable)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_NORMAL;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        glGenBuffers(1, &info.bufferId);
        glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vboObjects[id] = info;
    }
    else
    {
        id = glGenLists(1);

        glNewList(id, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }

    return id;
}

unsigned int CGLDevice::CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vboAvailable)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_TEX2;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        glGenBuffers(1, &info.bufferId);
        glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vboObjects[id] = info;
    }
    else
    {
        id = glGenLists(1);

        glNewList(id, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }

    return id;
}

unsigned int CGLDevice::CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vboAvailable)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_COL;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        glGenBuffers(1, &info.bufferId);
        glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vboObjects[id] = info;
    }
    else
    {
        id = glGenLists(1);

        glNewList(id, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }

    return id;
}

void CGLDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_NORMAL;
        info.vertexCount = vertexCount;

        glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGLDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_TEX2;
        info.vertexCount = vertexCount;

        glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGLDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_COL;
        info.vertexCount = vertexCount;

        glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGLDevice::DrawStaticBuffer(unsigned int bufferId)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        glEnable(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, (*it).second.bufferId);

        if ((*it).second.vertexType == VERTEX_TYPE_NORMAL)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, coord));

            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, normal));

            if (m_multitextureAvailable)
                glClientActiveTexture(GL_TEXTURE0);

            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, texCoord));
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_TEX2)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, coord));

            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, normal));

            if (m_multitextureAvailable)
                glClientActiveTexture(GL_TEXTURE0);

            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord));

            if (m_multitextureAvailable)
            {
                glClientActiveTexture(GL_TEXTURE1);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord2));
            }
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_COL)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), static_cast<char*>(nullptr) + offsetof(VertexCol, coord));

            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_FLOAT, sizeof(VertexCol), static_cast<char*>(nullptr) + offsetof(VertexCol, color));
        }

        GLenum mode = TranslateGfxPrimitive((*it).second.primitiveType);
        glDrawArrays(mode, 0, (*it).second.vertexCount);

        if ((*it).second.vertexType == VERTEX_TYPE_NORMAL)
        {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_TEX2)
        {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1
            if (m_multitextureAvailable)
            {
                glClientActiveTexture(GL_TEXTURE0);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_COL)
        {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisable(GL_VERTEX_ARRAY);
    }
    else
    {
        glCallList(bufferId);
    }
}

void CGLDevice::DestroyStaticBuffer(unsigned int bufferId)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        glDeleteBuffers(1, &(*it).second.bufferId);

        m_vboObjects.erase(it);
    }
    else
    {
        glDeleteLists(bufferId, 1);
    }
}

bool InPlane(Math::Vector normal, float originPlane, Math::Vector center, float radius)
{
    float distance = originPlane + Math::DotProduct(normal, center);

    if (distance < -radius)
        return false;

    return true;
}

/* Based on libwine's implementation */

int CGLDevice::ComputeSphereVisibility(const Math::Vector &center, float radius)
{
    Math::Matrix m;
    m = Math::MultiplyMatrices(m_worldMat, m);
    m = Math::MultiplyMatrices(m_viewMat, m);
    Math::Matrix sc;
    Math::LoadScaleMatrix(sc, Math::Vector(1.0f, 1.0f, -1.0f));
    m = Math::MultiplyMatrices(sc, m);
    m = Math::MultiplyMatrices(m_projectionMat, m);

    Math::Vector vec[6];
    float originPlane[6];

    // Left plane
    vec[0].x = m.Get(4, 1) + m.Get(1, 1);
    vec[0].y = m.Get(4, 2) + m.Get(1, 2);
    vec[0].z = m.Get(4, 3) + m.Get(1, 3);
    float l1 = vec[0].Length();
    vec[0].Normalize();
    originPlane[0] = (m.Get(4, 4) + m.Get(1, 4)) / l1;

    // Right plane
    vec[1].x = m.Get(4, 1) - m.Get(1, 1);
    vec[1].y = m.Get(4, 2) - m.Get(1, 2);
    vec[1].z = m.Get(4, 3) - m.Get(1, 3);
    float l2 = vec[1].Length();
    vec[1].Normalize();
    originPlane[1] = (m.Get(4, 4) - m.Get(1, 4)) / l2;

    // Bottom plane
    vec[2].x = m.Get(4, 1) + m.Get(2, 1);
    vec[2].y = m.Get(4, 2) + m.Get(2, 2);
    vec[2].z = m.Get(4, 3) + m.Get(2, 3);
    float l3 = vec[2].Length();
    vec[2].Normalize();
    originPlane[2] = (m.Get(4, 4) + m.Get(2, 4)) / l3;

    // Top plane
    vec[3].x = m.Get(4, 1) - m.Get(2, 1);
    vec[3].y = m.Get(4, 2) - m.Get(2, 2);
    vec[3].z = m.Get(4, 3) - m.Get(2, 3);
    float l4 = vec[3].Length();
    vec[3].Normalize();
    originPlane[3] = (m.Get(4, 4) - m.Get(2, 4)) / l4;

    // Front plane
    vec[4].x = m.Get(4, 1) + m.Get(3, 1);
    vec[4].y = m.Get(4, 2) + m.Get(3, 2);
    vec[4].z = m.Get(4, 3) + m.Get(3, 3);
    float l5 = vec[4].Length();
    vec[4].Normalize();
    originPlane[4] = (m.Get(4, 4) + m.Get(3, 4)) / l5;

    // Back plane
    vec[5].x = m.Get(4, 1) - m.Get(3, 1);
    vec[5].y = m.Get(4, 2) - m.Get(3, 2);
    vec[5].z = m.Get(4, 3) - m.Get(3, 3);
    float l6 = vec[5].Length();
    vec[5].Normalize();
    originPlane[5] = (m.Get(4, 4) - m.Get(3, 4)) / l6;

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

void CGLDevice::SetRenderState(RenderState state, bool enabled)
{
    if (state == RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == RENDER_STATE_LIGHTING)
    {
        m_lighting = enabled;

        if (enabled)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);

        if (enabled)
        {
            for (int index = 0; index < static_cast<int>( m_lights.size() ); ++index)
                UpdateLightPosition(index);
        }

        return;
    }

    GLenum flag = 0;

    switch (state)
    {
        case RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case RENDER_STATE_FOG:         flag = GL_FOG; break;
        case RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
        case RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        default: assert(false); break;
    }

    if (enabled)
        glEnable(flag);
    else
        glDisable(flag);
}

bool CGLDevice::GetRenderState(RenderState state)
{
    if (state == RENDER_STATE_LIGHTING)
        return m_lighting;

    GLenum flag = 0;

    switch (state)
    {
        case RENDER_STATE_DEPTH_WRITE: flag = GL_DEPTH_WRITEMASK; break;
        case RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case RENDER_STATE_FOG:         flag = GL_FOG; break;
        case RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
        case RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        default: assert(false); break;
    }

    GLboolean result = GL_FALSE;
    glGetBooleanv(flag, &result);

    return result == GL_TRUE;
}

CompFunc TranslateGLCompFunc(GLenum flag)
{
    switch (flag)
    {
        case GL_NEVER:    return COMP_FUNC_NEVER;
        case GL_LESS:     return COMP_FUNC_LESS;
        case GL_EQUAL:    return COMP_FUNC_EQUAL;
        case GL_NOTEQUAL: return COMP_FUNC_NOTEQUAL;
        case GL_LEQUAL:   return COMP_FUNC_LEQUAL;
        case GL_GREATER:  return COMP_FUNC_GREATER;
        case GL_GEQUAL:   return COMP_FUNC_GEQUAL;
        case GL_ALWAYS:   return COMP_FUNC_ALWAYS;
        default: assert(false); break;
    }
    return COMP_FUNC_NEVER;
}

GLenum TranslateGfxCompFunc(CompFunc func)
{
    switch (func)
    {
        case COMP_FUNC_NEVER:    return GL_NEVER;
        case COMP_FUNC_LESS:     return GL_LESS;
        case COMP_FUNC_EQUAL:    return GL_EQUAL;
        case COMP_FUNC_NOTEQUAL: return GL_NOTEQUAL;
        case COMP_FUNC_LEQUAL:   return GL_LEQUAL;
        case COMP_FUNC_GREATER:  return GL_GREATER;
        case COMP_FUNC_GEQUAL:   return GL_GEQUAL;
        case COMP_FUNC_ALWAYS:   return GL_ALWAYS;
        default: assert(false); break;
    }
    return 0;
}

void CGLDevice::SetDepthTestFunc(CompFunc func)
{
    glDepthFunc(TranslateGfxCompFunc(func));
}

CompFunc CGLDevice::GetDepthTestFunc()
{
    GLint flag = 0;
    glGetIntegerv(GL_DEPTH_FUNC, &flag);
    return TranslateGLCompFunc(static_cast<GLenum>(flag));
}

void CGLDevice::SetDepthBias(float factor)
{
    glPolygonOffset(factor, 0.0f);
}

float CGLDevice::GetDepthBias()
{
    GLfloat result = 0.0f;
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &result);
    return result;
}

void CGLDevice::SetAlphaTestFunc(CompFunc func, float refValue)
{
    glAlphaFunc(TranslateGfxCompFunc(func), refValue);
}

void CGLDevice::GetAlphaTestFunc(CompFunc &func, float &refValue)
{
    GLint flag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &flag);
    func = TranslateGLCompFunc(static_cast<GLenum>(flag));

    glGetFloatv(GL_ALPHA_TEST_REF, static_cast<GLfloat*>(&refValue));
}

BlendFunc TranslateGLBlendFunc(GLenum flag)
{
    switch (flag)
    {
        case GL_ZERO:                return BLEND_ZERO;
        case GL_ONE:                 return BLEND_ONE;
        case GL_SRC_COLOR:           return BLEND_SRC_COLOR;
        case GL_ONE_MINUS_SRC_COLOR: return BLEND_INV_SRC_COLOR;
        case GL_DST_COLOR:           return BLEND_DST_COLOR;
        case GL_ONE_MINUS_DST_COLOR: return BLEND_INV_DST_COLOR;
        case GL_SRC_ALPHA:           return BLEND_SRC_ALPHA;
        case GL_ONE_MINUS_SRC_ALPHA: return BLEND_INV_SRC_ALPHA;
        case GL_DST_ALPHA:           return BLEND_DST_ALPHA;
        case GL_ONE_MINUS_DST_ALPHA: return BLEND_INV_DST_ALPHA;
        case GL_SRC_ALPHA_SATURATE:  return BLEND_SRC_ALPHA_SATURATE;
        default: assert(false); break;
    }

    return BLEND_ZERO;
}

GLenum TranslateGfxBlendFunc(BlendFunc func)
{
    switch (func)
    {
        case BLEND_ZERO:               return GL_ZERO;
        case BLEND_ONE:                return GL_ONE;
        case BLEND_SRC_COLOR:          return GL_SRC_COLOR;
        case BLEND_INV_SRC_COLOR:      return GL_ONE_MINUS_SRC_COLOR;
        case BLEND_DST_COLOR:          return GL_DST_COLOR;
        case BLEND_INV_DST_COLOR:      return GL_ONE_MINUS_DST_COLOR;
        case BLEND_SRC_ALPHA:          return GL_SRC_ALPHA;
        case BLEND_INV_SRC_ALPHA:      return GL_ONE_MINUS_SRC_ALPHA;
        case BLEND_DST_ALPHA:          return GL_DST_ALPHA;
        case BLEND_INV_DST_ALPHA:      return GL_ONE_MINUS_DST_ALPHA;
        case BLEND_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
        default: assert(false); break;
    }
    return 0;
}

void CGLDevice::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void CGLDevice::GetBlendFunc(BlendFunc &srcBlend, BlendFunc &dstBlend)
{
    GLint srcFlag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &srcFlag);
    srcBlend = TranslateGLBlendFunc(static_cast<GLenum>(srcFlag));

    GLint dstFlag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &dstFlag);
    dstBlend = TranslateGLBlendFunc(static_cast<GLenum>(dstFlag));
}

void CGLDevice::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

Color CGLDevice::GetClearColor()
{
    GLfloat color[4] = { 0.0f };
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    return Color(color[0], color[1], color[2], color[3]);
}

void CGLDevice::SetGlobalAmbient(const Color &color)
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

Color CGLDevice::GetGlobalAmbient()
{
    GLfloat color[4] = { 0.0f };
    glGetFloatv(GL_LIGHT_MODEL_AMBIENT, color);
    return Color(color[0], color[1], color[2], color[3]);
}

void CGLDevice::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
    if      (mode == FOG_LINEAR) glFogi(GL_FOG_MODE, GL_LINEAR);
    else if (mode == FOG_EXP)    glFogi(GL_FOG_MODE, GL_EXP);
    else if (mode == FOG_EXP2)   glFogi(GL_FOG_MODE, GL_EXP2);
    else assert(false);

    glFogf(GL_FOG_START,   start);
    glFogf(GL_FOG_END,     end);
    glFogf(GL_FOG_DENSITY, density);
    glFogfv(GL_FOG_COLOR,  color.Array());
}

void CGLDevice::GetFogParams(FogMode &mode, Color &color, float &start, float &end, float &density)
{
    GLint flag = 0;
    glGetIntegerv(GL_FOG_MODE, &flag);
    if      (flag == GL_LINEAR) mode = FOG_LINEAR;
    else if (flag == GL_EXP)    mode = FOG_EXP;
    else if (flag == GL_EXP2)   mode = FOG_EXP2;
    else assert(false);

    glGetFloatv(GL_FOG_START,   static_cast<GLfloat*>(&start));
    glGetFloatv(GL_FOG_END,     static_cast<GLfloat*>(&end));
    glGetFloatv(GL_FOG_DENSITY, static_cast<GLfloat*>(&density));
    GLfloat col[4] = { 0.0f };
    glGetFloatv(GL_FOG_COLOR,  col);
    color = Color(col[0], col[1], col[2], col[3]);
}

void CGLDevice::SetCullMode(CullMode mode)
{
    // Cull clockwise back faces, so front face is the opposite
    // (assuming GL_CULL_FACE is GL_BACK)
    if      (mode == CULL_CW ) glFrontFace(GL_CCW);
    else if (mode == CULL_CCW) glFrontFace(GL_CW);
    else assert(false);
}

CullMode CGLDevice::GetCullMode()
{
    GLint flag = 0;
    glGetIntegerv(GL_FRONT_FACE, &flag);
    if      (flag == GL_CW)  return CULL_CCW;
    else if (flag == GL_CCW) return CULL_CW;
    else assert(false);
    return CULL_CW;
}

void CGLDevice::SetShadeModel(ShadeModel model)
{
    if      (model == SHADE_FLAT)   glShadeModel(GL_FLAT);
    else if (model == SHADE_SMOOTH) glShadeModel(GL_SMOOTH);
    else  assert(false);
}

ShadeModel CGLDevice::GetShadeModel()
{
    GLint flag = 0;
    glGetIntegerv(GL_SHADE_MODEL, &flag);
    if      (flag == GL_FLAT)    return SHADE_FLAT;
    else if (flag == GL_SMOOTH)  return SHADE_SMOOTH;
    else  assert(false);
    return SHADE_FLAT;
}

void CGLDevice::SetFillMode(FillMode mode)
{
    if      (mode == FILL_POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == FILL_LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == FILL_POLY)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

FillMode CGLDevice::GetFillMode()
{
    GLint flag = 0;
    glGetIntegerv(GL_POLYGON_MODE, &flag);
    if      (flag == GL_POINT) return FILL_POINT;
    else if (flag == GL_LINE)  return FILL_LINES;
    else if (flag == GL_FILL)  return FILL_POLY;
    else  assert(false);
    return FILL_POINT;
}


} // namespace Gfx
