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

// gldevice.cpp

#include "graphics/opengl/gldevice.h"

#include "common/config.h"
#include "common/image.h"
#include "math/geometry.h"


#if defined(USE_GLEW)

// When using GLEW, only glew.h is needed
#include <GL/glew.h>

#else

// Should define prototypes of used extensions as OpenGL functions
#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#endif // if defined(GLEW)

#include <SDL/SDL.h>

#include <cassert>



void Gfx::GLDeviceConfig::LoadDefault()
{
    Gfx::DeviceConfig::LoadDefault();

    hardwareAccel = true;

    redSize = 8;
    blueSize = 8;
    greenSize = 8;
    alphaSize = 8;
    depthSize = 24;
}




Gfx::CGLDevice::CGLDevice(const Gfx::GLDeviceConfig &config)
{
    m_config = config;
    m_lighting = false;
    m_texturing = false;
}


Gfx::CGLDevice::~CGLDevice()
{
}

std::string Gfx::CGLDevice::GetError()
{
    return m_error;
}

bool Gfx::CGLDevice::Create()
{
#if defined(USE_GLEW)
    static bool glewInited = false;

    if (!glewInited)
    {
        glewInited = true;

        if (glewInit() != GLEW_OK)
        {
            m_error = "GLEW initialization failed";
            return false;
        }

        if ( (! GLEW_ARB_multitexture) || (! GLEW_EXT_texture_env_combine) || (! GLEW_EXT_secondary_color) )
        {
            m_error = "GLEW reports required extensions not supported";
            return false;
        }
    }
#endif

    /* NOTE: when not using GLEW, extension testing is not performed, as it is assumed that
             glext.h is up-to-date and the OpenGL shared library has the required functions present. */

    // This is mostly done in all modern hardware by default
    // DirectX doesn't even allow the option to turn off perspective correction anymore
    // So turn it on permanently
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // To use separate specular color in drawing primitives
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    // To avoid problems with scaling & lighting
    glEnable(GL_RESCALE_NORMAL);

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, m_config.size.w, m_config.size.h);


    m_lights        = std::vector<Gfx::Light>(GL_MAX_LIGHTS, Gfx::Light());
    m_lightsEnabled = std::vector<bool>      (GL_MAX_LIGHTS, false);

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextures);

    m_currentTextures    = std::vector<Gfx::Texture>           (maxTextures, Gfx::Texture());
    m_texturesEnabled    = std::vector<bool>                   (maxTextures, false);
    m_textureStageParams = std::vector<Gfx::TextureStageParams>(maxTextures, Gfx::TextureStageParams());

    return true;
}

void Gfx::CGLDevice::Destroy()
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

void Gfx::CGLDevice::ConfigChanged(const Gfx::GLDeviceConfig& newConfig)
{
    m_config = newConfig;

    // Reset state
    m_lighting = false;
    m_texturing = false;
    Destroy();
    Create();
}

void Gfx::CGLDevice::BeginScene()
{
    Clear();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMat.Array());

    UpdateModelviewMatrix();
}

void Gfx::CGLDevice::EndScene()
{
    glFlush();
}

void Gfx::CGLDevice::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Gfx::CGLDevice::SetTransform(Gfx::TransformType type, const Math::Matrix &matrix)
{
    if      (type == Gfx::TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        UpdateModelviewMatrix();
    }
    else if (type == Gfx::TRANSFORM_VIEW)
    {
        m_viewMat = matrix;
        UpdateModelviewMatrix();
    }
    else if (type == Gfx::TRANSFORM_PROJECTION)
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

const Math::Matrix& Gfx::CGLDevice::GetTransform(Gfx::TransformType type)
{
    if      (type == Gfx::TRANSFORM_WORLD)
        return m_worldMat;
    else if (type == Gfx::TRANSFORM_VIEW)
        return m_viewMat;
    else if (type == Gfx::TRANSFORM_PROJECTION)
        return m_projectionMat;
    else
        assert(false);

    return m_worldMat; // to avoid warning
}

void Gfx::CGLDevice::MultiplyTransform(Gfx::TransformType type, const Math::Matrix &matrix)
{
    if      (type == Gfx::TRANSFORM_WORLD)
    {
        m_worldMat = Math::MultiplyMatrices(m_worldMat, matrix);
        UpdateModelviewMatrix();
    }
    else if (type == Gfx::TRANSFORM_VIEW)
    {
        m_viewMat = Math::MultiplyMatrices(m_viewMat, matrix);
        UpdateModelviewMatrix();
    }
    else if (type == Gfx::TRANSFORM_PROJECTION)
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

void Gfx::CGLDevice::UpdateModelviewMatrix()
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

void Gfx::CGLDevice::SetMaterial(const Gfx::Material &material)
{
    m_material = material;

    glMaterialfv(GL_FRONT, GL_AMBIENT,  m_material.ambient.Array());
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  m_material.diffuse.Array());
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_material.specular.Array());
}

const Gfx::Material& Gfx::CGLDevice::GetMaterial()
{
    return m_material;
}

int Gfx::CGLDevice::GetMaxLightCount()
{
    return m_lights.size();
}

void Gfx::CGLDevice::SetLight(int index, const Gfx::Light &light)
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

    if (light.type == Gfx::LIGHT_SPOT)
    {
        glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, light.spotAngle);
        glLightf(GL_LIGHT0 + index, GL_SPOT_EXPONENT, light.spotIntensity);
    }
    else
    {
        glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180.0f);
    }

    UpdateLightPosition(index);
}

void Gfx::CGLDevice::UpdateLightPosition(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    if ((! m_lighting) || (! m_lightsEnabled[index]))
        return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    glMultMatrixf(m_viewMat.Array());

    if (m_lights[index].type == LIGHT_DIRECTIONAL)
    {
        GLfloat position[4] = { m_lights[index].direction.x, m_lights[index].direction.y, m_lights[index].direction.z, 0.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }
    else
    {
        GLfloat position[4] = { m_lights[index].position.x, m_lights[index].position.y, m_lights[index].position.z, 1.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }

    if (m_lights[index].type == Gfx::LIGHT_SPOT)
    {
        GLfloat direction[4] = { m_lights[index].direction.x, m_lights[index].direction.y, m_lights[index].direction.z, 0.0f };
        glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction);
    }

    glPopMatrix();
}

const Gfx::Light& Gfx::CGLDevice::GetLight(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    return m_lights[index];
}

void Gfx::CGLDevice::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lightsEnabled[index] = enabled;

    glEnable(GL_LIGHT0 + index);
}

bool Gfx::CGLDevice::GetLightEnabled(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    return m_lightsEnabled[index];
}

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Gfx::Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Gfx::Texture Gfx::CGLDevice::CreateTexture(CImage *image, const Gfx::TextureCreateParams &params)
{
    ImageData *data = image->GetData();
    if (data == NULL)
    {
        m_error = "Invalid texture data";
        return Gfx::Texture(); // invalid texture
    }

    return CreateTexture(data, params);
}

Gfx::Texture Gfx::CGLDevice::CreateTexture(ImageData *data, const Gfx::TextureCreateParams &params)
{
    Gfx::Texture result;

    result.valid = true;
    result.size.w = data->surface->w;
    result.size.h = data->surface->h;

    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    // Set params

    GLint minF = 0;
    if      (params.minFilter == Gfx::TEX_MIN_FILTER_NEAREST)                minF = GL_NEAREST;
    else if (params.minFilter == Gfx::TEX_MIN_FILTER_LINEAR)                 minF = GL_LINEAR;
    else if (params.minFilter == Gfx::TEX_MIN_FILTER_NEAREST_MIPMAP_NEAREST) minF = GL_NEAREST_MIPMAP_NEAREST;
    else if (params.minFilter == Gfx::TEX_MIN_FILTER_LINEAR_MIPMAP_NEAREST)  minF = GL_LINEAR_MIPMAP_NEAREST;
    else if (params.minFilter == Gfx::TEX_MIN_FILTER_NEAREST_MIPMAP_LINEAR)  minF = GL_NEAREST_MIPMAP_LINEAR;
    else if (params.minFilter == Gfx::TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR)   minF = GL_LINEAR_MIPMAP_LINEAR;
    else  assert(false);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);

    GLint magF = 0;
    if      (params.magFilter == Gfx::TEX_MAG_FILTER_NEAREST) magF = GL_NEAREST;
    else if (params.magFilter == Gfx::TEX_MAG_FILTER_LINEAR)  magF = GL_LINEAR;
    else  assert(false);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);

    if (params.mipmap)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

    GLenum sourceFormat = 0;
    if (params.format == Gfx::TEX_IMG_RGB)
    {
        sourceFormat = GL_RGB;
        result.alpha = false;
    }
    else if (params.format == Gfx::TEX_IMG_BGR)
    {
        sourceFormat = GL_BGR;
        result.alpha = false;
    }
    else if (params.format == Gfx::TEX_IMG_RGBA)
    {
        sourceFormat = GL_RGBA;
        result.alpha = true;
    }
    else if (params.format == Gfx::TEX_IMG_BGRA)
    {
        sourceFormat = GL_BGRA;
        result.alpha = true;
    }
    else
        assert(false);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data->surface->w, data->surface->h,
                 0, sourceFormat, GL_UNSIGNED_BYTE, data->surface->pixels);


    // Restore the previous state of 1st stage
    if (m_currentTextures[0].valid)
        glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);
    else
        glBindTexture(GL_TEXTURE_2D, 0);

    if ( (! m_texturing) || (! m_texturesEnabled[0]) )
        glDisable(GL_TEXTURE_2D);

    return result;
}

void Gfx::CGLDevice::DestroyTexture(const Gfx::Texture &texture)
{
    std::set<Gfx::Texture>::iterator it = m_allTextures.find(texture);
    if (it != m_allTextures.end())
        m_allTextures.erase(it);

    // Unbind the texture if in use anywhere
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
    {
        if (m_currentTextures[index] == texture)
            SetTexture(index, Gfx::Texture()); // set to invalid texture
    }

    glDeleteTextures(1, &texture.id);
}

void Gfx::CGLDevice::DestroyAllTextures()
{
    std::set<Gfx::Texture> allCopy = m_allTextures;
    std::set<Gfx::Texture>::iterator it;
    for (it = allCopy.begin(); it != allCopy.end(); ++it)
        DestroyTexture(*it);
}

int Gfx::CGLDevice::GetMaxTextureCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void Gfx::CGLDevice::SetTexture(int index, const Gfx::Texture &texture)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    // Enable the given texture stage
    glActiveTexture(GL_TEXTURE0 + index);
    glEnable(GL_TEXTURE_2D);

    m_currentTextures[index] = texture; // remember the change

    if (! texture.valid)
    {
        glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture.id);                  // bind the texture
        SetTextureStageParams(index, m_textureStageParams[index]); // texture stage params need to be re-set for the new texture
    }

    // Disable the stage if it is set so
    if ( (! m_texturing) || (! m_texturesEnabled[index]) )
        glDisable(GL_TEXTURE_2D);
}

void Gfx::CGLDevice::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    // Enable the given texture stage
    glActiveTexture(GL_TEXTURE0 + index);
    glEnable(GL_TEXTURE_2D);

    m_currentTextures[index].id = textureId;

    glBindTexture(GL_TEXTURE_2D, textureId);

    // Disable the stage if it is set so
    if ( (! m_texturing) || (! m_texturesEnabled[index]) )
        glDisable(GL_TEXTURE_2D);
}

/**
  Returns the previously assigned texture or invalid texture if the given stage is not enabled. */
Gfx::Texture Gfx::CGLDevice::GetTexture(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    return m_currentTextures[index];
}

void Gfx::CGLDevice::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    m_texturesEnabled[index] = enabled;

    glActiveTexture(GL_TEXTURE0 + index);
    if (enabled)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);
}

bool Gfx::CGLDevice::GetTextureEnabled(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    return m_texturesEnabled[index];
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void Gfx::CGLDevice::SetTextureStageParams(int index, const Gfx::TextureStageParams &params)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index] = params;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].valid)
        return;

    // Enable the given stage
    glActiveTexture(GL_TEXTURE0 + index);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_currentTextures[index].id);

    // To save some trouble
    if ( (params.colorOperation == Gfx::TEX_MIX_OPER_DEFAULT) &&
         (params.alphaOperation == Gfx::TEX_MIX_OPER_DEFAULT) )
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

    if (params.colorOperation == Gfx::TEX_MIX_OPER_DEFAULT)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
        goto after_tex_color;
    }
    else if (params.colorOperation == Gfx::TEX_MIX_OPER_REPLACE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    else if (params.colorOperation == Gfx::TEX_MIX_OPER_MODULATE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    else if (params.colorOperation == Gfx::TEX_MIX_OPER_ADD)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
    else if (params.colorOperation == Gfx::TEX_MIX_OPER_SUBTRACT)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_SUBTRACT);
    else  assert(false);

    // Color arg1
    if (params.colorArg1 == Gfx::TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    else if (params.colorArg1 == Gfx::TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    else if (params.colorArg1 == Gfx::TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    else if (params.colorArg1 == Gfx::TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
    else  assert(false);

    // Color arg2
    if (params.colorArg2 == Gfx::TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    else if (params.colorArg2 == Gfx::TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
    else if (params.colorArg2 == Gfx::TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    else if (params.colorArg2 == Gfx::TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
    else  assert(false);


after_tex_color:

    // Alpha operation
    if (params.alphaOperation == Gfx::TEX_MIX_OPER_DEFAULT)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
        goto after_tex_operations;
    }
    else if (params.colorOperation == Gfx::TEX_MIX_OPER_REPLACE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    else if (params.alphaOperation == Gfx::TEX_MIX_OPER_MODULATE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    else if (params.alphaOperation == Gfx::TEX_MIX_OPER_ADD)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
    else if (params.alphaOperation == Gfx::TEX_MIX_OPER_SUBTRACT)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_SUBTRACT);
    else  assert(false);

    // Alpha arg1
    if (params.alphaArg1 == Gfx::TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
    else if (params.alphaArg1 == Gfx::TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg1 == Gfx::TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg1 == Gfx::TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
    else  assert(false);

    // Alpha arg2
    if (params.alphaArg2 == Gfx::TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
    else if (params.alphaArg2 == Gfx::TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg2 == Gfx::TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg2 == Gfx::TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
    else  assert(false);


after_tex_operations:

    if      (params.wrapS == Gfx::TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    else if (params.wrapS == Gfx::TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (params.wrapT == Gfx::TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    else if (params.wrapT == Gfx::TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);

    // Disable the stage if it is set so
    if ( (! m_texturing) || (! m_texturesEnabled[index]) )
        glDisable(GL_TEXTURE_2D);
}

Gfx::TextureStageParams Gfx::CGLDevice::GetTextureStageParams(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_currentTextures.size() ));

    return m_textureStageParams[index];
}

void Gfx::CGLDevice::SetTextureFactor(const Gfx::Color &color)
{
    // Needs to be set for all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
    {
        // Activate stage
        glActiveTexture(GL_TEXTURE0 + index);
        glEnable(GL_TEXTURE_2D);

        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color.Array());

        // Disable the stage if it is set so
        if ( (! m_texturing) || (! m_texturesEnabled[index]) )
            glDisable(GL_TEXTURE_2D);
    }
}

Gfx::Color Gfx::CGLDevice::GetTextureFactor()
{
    // Get from 1st stage (should be the same for all stages)
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    GLfloat color[4] = { 0.0f };
    glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);

    // Disable the 1st stage if it is set so
    if ( (! m_texturing) || (! m_texturesEnabled[0]) )
        glDisable(GL_TEXTURE_2D);

    return Gfx::Color(color[0], color[1], color[2], color[3]);
}

GLenum TranslateGfxPrimitive(Gfx::PrimitiveType type)
{
    GLenum flag = 0;
    switch (type)
    {
        case Gfx::PRIMITIVE_POINTS:         flag = GL_POINTS; break;
        case Gfx::PRIMITIVE_LINES:          flag = GL_LINES; break;
        case Gfx::PRIMITIVE_LINE_STRIP:     flag = GL_LINE_STRIP; break;
        case Gfx::PRIMITIVE_TRIANGLES:      flag = GL_TRIANGLES; break;
        case Gfx::PRIMITIVE_TRIANGLE_STRIP: flag = GL_TRIANGLE_STRIP; break;
        default: assert(false); break;
    }
    return flag;
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType type, const Vertex *vertices, int vertexCount)
{
    glBegin(TranslateGfxPrimitive(type));

    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < vertexCount; ++i)
    {
        glNormal3fv(const_cast<GLfloat*>(vertices[i].normal.Array()));
        glMultiTexCoord2fv(GL_TEXTURE0, const_cast<GLfloat*>(vertices[i].texCoord.Array()));
        glVertex3fv(const_cast<GLfloat*>(vertices[i].coord.Array()));
    }

    glEnd();
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType type, const Gfx::VertexCol *vertices, int vertexCount)
{
    glBegin(TranslateGfxPrimitive(type));

    for (int i = 0; i < vertexCount; ++i)
    {
        glColor4fv(const_cast<GLfloat*>(vertices[i].color.Array()));
        glSecondaryColor3fv(const_cast<GLfloat*>(vertices[i].specular.Array()));
        glMultiTexCoord2fv(GL_TEXTURE0, const_cast<GLfloat*>(vertices[i].texCoord.Array()));
        glVertex3fv(const_cast<GLfloat*>(vertices[i].coord.Array()));
    }

    glEnd();
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType type, const VertexTex2 *vertices, int vertexCount)
{
    glBegin(TranslateGfxPrimitive(type));

    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < vertexCount; ++i)
    {
        glNormal3fv(const_cast<GLfloat*>(vertices[i].normal.Array()));
        glMultiTexCoord2fv(GL_TEXTURE0, const_cast<GLfloat*>(vertices[i].texCoord.Array()));
        glMultiTexCoord2fv(GL_TEXTURE1, const_cast<GLfloat*>(vertices[i].texCoord.Array()));
        glVertex3fv(const_cast<GLfloat*>(vertices[i].coord.Array()));
    }

    glEnd();
}

bool InPlane(Math::Vector normal, float originPlane, Math::Vector center, float radius)
{
    float distance = (originPlane + Math::DotProduct(normal, center)) / normal.Length();

    if (distance < -radius)
        return true;

    return false;
}

/*
   The implementation of ComputeSphereVisibility is taken from libwine's device.c
   Copyright of the WINE team, licensed under GNU LGPL v 2.1
 */

// TODO: testing
int Gfx::CGLDevice::ComputeSphereVisibility(const Math::Vector &center, float radius)
{
    Math::Matrix m;
    m.LoadIdentity();
    m = Math::MultiplyMatrices(m, m_worldMat);
    m = Math::MultiplyMatrices(m, m_viewMat);
    m = Math::MultiplyMatrices(m, m_projectionMat);

    Math::Vector vec[6];
    float originPlane[6];

    // Left plane
    vec[0].x = m.Get(4, 1) + m.Get(1, 1);
    vec[0].y = m.Get(4, 2) + m.Get(1, 2);
    vec[0].z = m.Get(4, 3) + m.Get(1, 3);
    originPlane[0] = m.Get(4, 4) + m.Get(1, 4);

    // Right plane
    vec[1].x = m.Get(4, 1) - m.Get(1, 1);
    vec[1].y = m.Get(4, 2) - m.Get(1, 2);
    vec[1].z = m.Get(4, 3) - m.Get(1, 3);
    originPlane[1] = m.Get(4, 4) - m.Get(1, 4);

    // Top plane
    vec[2].x = m.Get(4, 1) - m.Get(2, 1);
    vec[2].y = m.Get(4, 2) - m.Get(2, 2);
    vec[2].z = m.Get(4, 3) - m.Get(2, 3);
    originPlane[2] = m.Get(4, 4) - m.Get(2, 4);

    // Bottom plane
    vec[3].x = m.Get(4, 1) + m.Get(2, 1);
    vec[3].y = m.Get(4, 2) + m.Get(2, 2);
    vec[3].z = m.Get(4, 3) + m.Get(2, 3);
    originPlane[3] = m.Get(4, 4) + m.Get(2, 4);

    // Front plane
    vec[4].x = m.Get(3, 1);
    vec[4].y = m.Get(3, 2);
    vec[4].z = m.Get(3, 3);
    originPlane[4] = m.Get(3, 4);

    // Back plane
    vec[5].x = m.Get(4, 1) - m.Get(3, 1);
    vec[5].y = m.Get(4, 2) - m.Get(3, 2);
    vec[5].z = m.Get(4, 3) - m.Get(3, 3);
    originPlane[5] = m.Get(4, 4) - m.Get(3, 4);

    int result = 0;

    if (InPlane(vec[0], originPlane[0], center, radius))
        result |= Gfx::INTERSECT_PLANE_LEFT;
    if (InPlane(vec[1], originPlane[1], center, radius))
        result |= Gfx::INTERSECT_PLANE_RIGHT;
    if (InPlane(vec[2], originPlane[2], center, radius))
        result |= Gfx::INTERSECT_PLANE_TOP;
    if (InPlane(vec[3], originPlane[3], center, radius))
        result |= Gfx::INTERSECT_PLANE_BOTTOM;
    if (InPlane(vec[4], originPlane[4], center, radius))
        result |= Gfx::INTERSECT_PLANE_FRONT;
    if (InPlane(vec[5], originPlane[5], center, radius))
        result |= Gfx::INTERSECT_PLANE_BACK;

    return result;
}

void Gfx::CGLDevice::SetRenderState(Gfx::RenderState state, bool enabled)
{
    if (state == Gfx::RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == Gfx::RENDER_STATE_LIGHTING)
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
    else if (state == Gfx::RENDER_STATE_TEXTURING)
    {
        m_texturing = enabled;

        // Enable/disable stages with new setting
        for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        {
            glActiveTexture(GL_TEXTURE0 + index);
            if (m_texturing && m_texturesEnabled[index])
                glEnable(GL_TEXTURE_2D);
            else
                glDisable(GL_TEXTURE_2D);
        }

        return;
    }

    GLenum flag = 0;

    switch (state)
    {
        case Gfx::RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case Gfx::RENDER_STATE_FOG:         flag = GL_FOG; break;
        case Gfx::RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case Gfx::RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
        case Gfx::RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        case Gfx::RENDER_STATE_DITHERING:   flag = GL_DITHER; break;
        default: assert(false); break;
    }

    if (enabled)
        glEnable(flag);
    else
        glDisable(flag);
}

bool Gfx::CGLDevice::GetRenderState(Gfx::RenderState state)
{
    if (state == Gfx::RENDER_STATE_LIGHTING)
        return m_lighting;

    if (state == Gfx::RENDER_STATE_TEXTURING)
        return m_texturing;

    GLenum flag = 0;

    switch (state)
    {
        case Gfx::RENDER_STATE_DEPTH_WRITE: flag = GL_DEPTH_WRITEMASK; break;
        case Gfx::RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case Gfx::RENDER_STATE_FOG:         flag = GL_FOG; break;
        case Gfx::RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case Gfx::RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
        case Gfx::RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        case Gfx::RENDER_STATE_DITHERING:   flag = GL_DITHER; break;
        default: assert(false); break;
    }

    GLboolean result = GL_FALSE;
    glGetBooleanv(flag, &result);

    return result == GL_TRUE;
}

Gfx::CompFunc TranslateGLCompFunc(GLenum flag)
{
    switch (flag)
    {
        case GL_NEVER:    return Gfx::COMP_FUNC_NEVER;
        case GL_LESS:     return Gfx::COMP_FUNC_LESS;
        case GL_EQUAL:    return Gfx::COMP_FUNC_EQUAL;
        case GL_NOTEQUAL: return Gfx::COMP_FUNC_NOTEQUAL;
        case GL_LEQUAL:   return Gfx::COMP_FUNC_LEQUAL;
        case GL_GREATER:  return Gfx::COMP_FUNC_GREATER;
        case GL_GEQUAL:   return Gfx::COMP_FUNC_GEQUAL;
        case GL_ALWAYS:   return Gfx::COMP_FUNC_ALWAYS;
        default: assert(false); break;
    }
    return Gfx::COMP_FUNC_NEVER;
}

GLenum TranslateGfxCompFunc(Gfx::CompFunc func)
{
    switch (func)
    {
        case Gfx::COMP_FUNC_NEVER:    return GL_NEVER;
        case Gfx::COMP_FUNC_LESS:     return GL_LESS;
        case Gfx::COMP_FUNC_EQUAL:    return GL_EQUAL;
        case Gfx::COMP_FUNC_NOTEQUAL: return GL_NOTEQUAL;
        case Gfx::COMP_FUNC_LEQUAL:   return GL_LEQUAL;
        case Gfx::COMP_FUNC_GREATER:  return GL_GREATER;
        case Gfx::COMP_FUNC_GEQUAL:   return GL_GEQUAL;
        case Gfx::COMP_FUNC_ALWAYS:   return GL_ALWAYS;
        default: assert(false); break;
    }
    return 0;
}

void Gfx::CGLDevice::SetDepthTestFunc(Gfx::CompFunc func)
{
    glDepthFunc(TranslateGfxCompFunc(func));
}

Gfx::CompFunc Gfx::CGLDevice::GetDepthTestFunc()
{
    GLint flag = 0;
    glGetIntegerv(GL_DEPTH_FUNC, &flag);
    return TranslateGLCompFunc(static_cast<GLenum>(flag));
}

void Gfx::CGLDevice::SetDepthBias(float factor)
{
    glPolygonOffset(factor, 0.0f);
}

float Gfx::CGLDevice::GetDepthBias()
{
    GLfloat result = 0.0f;
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &result);
    return result;
}

void Gfx::CGLDevice::SetAlphaTestFunc(Gfx::CompFunc func, float refValue)
{
    glAlphaFunc(TranslateGfxCompFunc(func), refValue);
}

void Gfx::CGLDevice::GetAlphaTestFunc(Gfx::CompFunc &func, float &refValue)
{
    GLint flag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &flag);
    func = TranslateGLCompFunc(static_cast<GLenum>(flag));

    glGetFloatv(GL_ALPHA_TEST_REF, static_cast<GLfloat*>(&refValue));
}

Gfx::BlendFunc TranslateGLBlendFunc(GLenum flag)
{
    switch (flag)
    {
        case GL_ZERO:                return Gfx::BLEND_ZERO;
        case GL_ONE:                 return Gfx::BLEND_ONE;
        case GL_SRC_COLOR:           return Gfx::BLEND_SRC_COLOR;
        case GL_ONE_MINUS_SRC_COLOR: return Gfx::BLEND_INV_SRC_COLOR;
        case GL_DST_COLOR:           return Gfx::BLEND_DST_COLOR;
        case GL_ONE_MINUS_DST_COLOR: return Gfx::BLEND_INV_DST_COLOR;
        case GL_SRC_ALPHA:           return Gfx::BLEND_SRC_ALPHA;
        case GL_ONE_MINUS_SRC_ALPHA: return Gfx::BLEND_INV_SRC_ALPHA;
        case GL_DST_ALPHA:           return Gfx::BLEND_DST_ALPHA;
        case GL_ONE_MINUS_DST_ALPHA: return Gfx::BLEND_INV_DST_ALPHA;
        case GL_SRC_ALPHA_SATURATE:  return Gfx::BLEND_SRC_ALPHA_SATURATE;
        default: assert(false); break;
    }

    return Gfx::BLEND_ZERO;
}

GLenum TranslateGfxBlendFunc(Gfx::BlendFunc func)
{
    switch (func)
    {
        case Gfx::BLEND_ZERO:               return GL_ZERO;
        case Gfx::BLEND_ONE:                return GL_ONE;
        case Gfx::BLEND_SRC_COLOR:          return GL_SRC_COLOR;
        case Gfx::BLEND_INV_SRC_COLOR:      return GL_ONE_MINUS_SRC_COLOR;
        case Gfx::BLEND_DST_COLOR:          return GL_DST_COLOR;
        case Gfx::BLEND_INV_DST_COLOR:      return GL_ONE_MINUS_DST_COLOR;
        case Gfx::BLEND_SRC_ALPHA:          return GL_SRC_ALPHA;
        case Gfx::BLEND_INV_SRC_ALPHA:      return GL_ONE_MINUS_SRC_ALPHA;
        case Gfx::BLEND_DST_ALPHA:          return GL_DST_ALPHA;
        case Gfx::BLEND_INV_DST_ALPHA:      return GL_ONE_MINUS_DST_ALPHA;
        case Gfx::BLEND_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
        default: assert(false); break;
    }
    return 0;
}

void Gfx::CGLDevice::SetBlendFunc(Gfx::BlendFunc srcBlend, Gfx::BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void Gfx::CGLDevice::GetBlendFunc(Gfx::BlendFunc &srcBlend, Gfx::BlendFunc &dstBlend)
{
    GLint srcFlag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &srcFlag);
    srcBlend = TranslateGLBlendFunc(static_cast<GLenum>(srcFlag));

    GLint dstFlag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &dstFlag);
    dstBlend = TranslateGLBlendFunc(static_cast<GLenum>(dstFlag));
}

void Gfx::CGLDevice::SetClearColor(const Gfx::Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

Gfx::Color Gfx::CGLDevice::GetClearColor()
{
    GLfloat color[4] = { 0.0f };
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    return Gfx::Color(color[0], color[1], color[2], color[3]);
}

void Gfx::CGLDevice::SetGlobalAmbient(const Gfx::Color &color)
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

Gfx::Color Gfx::CGLDevice::GetGlobalAmbient()
{
    GLfloat color[4] = { 0.0f };
    glGetFloatv(GL_LIGHT_MODEL_AMBIENT, color);
    return Gfx::Color(color[0], color[1], color[2], color[3]);
}

void Gfx::CGLDevice::SetFogParams(Gfx::FogMode mode, const Gfx::Color &color, float start, float end, float density)
{
    if      (mode == Gfx::FOG_LINEAR) glFogi(GL_FOG_MODE, GL_LINEAR);
    else if (mode == Gfx::FOG_EXP)    glFogi(GL_FOG_MODE, GL_EXP);
    else if (mode == Gfx::FOG_EXP2)   glFogi(GL_FOG_MODE, GL_EXP2);
    else assert(false);

    glFogf(GL_FOG_START,   start);
    glFogf(GL_FOG_END,     end);
    glFogf(GL_FOG_DENSITY, density);
}

void Gfx::CGLDevice::GetFogParams(Gfx::FogMode &mode, Gfx::Color &color, float &start, float &end, float &density)
{
    GLint flag = 0;
    glGetIntegerv(GL_FOG_MODE, &flag);
    if      (flag == GL_LINEAR) mode = Gfx::FOG_LINEAR;
    else if (flag == GL_EXP)    mode = Gfx::FOG_EXP;
    else if (flag == GL_EXP2)   mode = Gfx::FOG_EXP2;
    else assert(false);

    glGetFloatv(GL_FOG_START,   static_cast<GLfloat*>(&start));
    glGetFloatv(GL_FOG_END,     static_cast<GLfloat*>(&end));
    glGetFloatv(GL_FOG_DENSITY, static_cast<GLfloat*>(&density));
}

void Gfx::CGLDevice::SetCullMode(Gfx::CullMode mode)
{
    if      (mode == Gfx::CULL_CW)  glCullFace(GL_CW);
    else if (mode == Gfx::CULL_CCW) glCullFace(GL_CCW);
    else assert(false);
}

Gfx::CullMode Gfx::CGLDevice::GetCullMode()
{
    GLint flag = 0;
    glGetIntegerv(GL_CULL_FACE, &flag);
    if      (flag == GL_CW)  return Gfx::CULL_CW;
    else if (flag == GL_CCW) return Gfx::CULL_CCW;
    else assert(false);
    return Gfx::CULL_CW;
}

void Gfx::CGLDevice::SetShadeModel(Gfx::ShadeModel model)
{
    if      (model == Gfx::SHADE_FLAT)   glShadeModel(GL_FLAT);
    else if (model == Gfx::SHADE_SMOOTH) glShadeModel(GL_SMOOTH);
    else  assert(false);
}

Gfx::ShadeModel Gfx::CGLDevice::GetShadeModel()
{
    GLint flag = 0;
    glGetIntegerv(GL_SHADE_MODEL, &flag);
    if      (flag == GL_FLAT)    return Gfx::SHADE_FLAT;
    else if (flag == GL_SMOOTH)  return Gfx::SHADE_SMOOTH;
    else  assert(false);
    return Gfx::SHADE_FLAT;
}

void Gfx::CGLDevice::SetFillMode(Gfx::FillMode mode)
{
    if      (mode == Gfx::FILL_POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == Gfx::FILL_LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == Gfx::FILL_FILL)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

Gfx::FillMode Gfx::CGLDevice::GetFillMode()
{
    GLint flag = 0;
    glGetIntegerv(GL_POLYGON_MODE, &flag);
    if      (flag == GL_POINT) return Gfx::FILL_POINT;
    else if (flag == GL_LINE)  return Gfx::FILL_LINES;
    else if (flag == GL_FILL)  return Gfx::FILL_FILL;
    else  assert(false);
    return Gfx::FILL_POINT;
}
