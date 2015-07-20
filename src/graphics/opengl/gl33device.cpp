/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "graphics/opengl/gl33device.h"
#include "graphics/opengl/glframebuffer.h"
#include "graphics/engine/engine.h"

#include "common/config.h"
#include "common/image.h"
#include "common/logger.h"
#include "common/config_file.h"

#include "math/geometry.h"


#include <SDL.h>
#include <physfs.h>

#include <cassert>


// Graphics module namespace
namespace Gfx {

CGL33Device::CGL33Device(const DeviceConfig &config)
{
    m_config = config;
    m_lighting = false;
    m_lastVboId = 0;
    m_anisotropyAvailable = false;
    m_maxAnisotropy = 1;
    m_glMajor = 1;
    m_glMinor = 1;

    m_perPixelLighting = false;
}


CGL33Device::~CGL33Device()
{
}

void CGL33Device::DebugHook()
{
    /* This function is only called here, so it can be used
     * as a breakpoint when debugging using gDEBugger */
    glColor3i(0, 0, 0);
}

void CGL33Device::DebugLights()
{
    Gfx::ColorHSV color(0.0, 1.0, 1.0);

    glLineWidth(3.0f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    Math::Matrix saveWorldMat = m_worldMat;
    Math::Matrix identity;
    identity.LoadIdentity();
    SetTransform(TRANSFORM_WORLD, identity);

    for (int i = 0; i < static_cast<int>( m_lights.size() ); ++i)
    {
        color.h = static_cast<float>(i) / static_cast<float>(m_lights.size());
        if (m_lightsEnabled[i])
        {
            const Light& l = m_lights[i];
            if (l.type == LIGHT_DIRECTIONAL)
            {
                Gfx::VertexCol v[2];
                v[0].coord = -Math::Normalize(l.direction) * 100.0f + Math::Vector(0.0f, 0.0f, 1.0f) * i;
                v[0].color = HSV2RGB(color);
                v[1].coord =  Math::Normalize(l.direction) * 100.0f + Math::Vector(0.0f, 0.0f, 1.0f) * i;
                v[1].color = HSV2RGB(color);
                while (v[0].coord.y < 60.0f && v[0].coord.y < 60.0f)
                {
                    v[0].coord.y += 10.0f;
                    v[1].coord.y += 10.0f;
                }
                DrawPrimitive(PRIMITIVE_LINES, v, 2);

                v[0].coord = v[1].coord + Math::Normalize(v[0].coord - v[1].coord) * 50.0f;

                glLineWidth(10.0f);
                DrawPrimitive(PRIMITIVE_LINES, v, 2);
                glLineWidth(3.0f);
            }
            else  if (l.type == LIGHT_POINT)
            {
                Gfx::VertexCol v[8];
                for (int i = 0; i < 8; ++i)
                    v[i].color = HSV2RGB(color);

                v[0].coord = l.position + Math::Vector(-1.0f, -1.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector( 1.0f, -1.0f, -1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f,  1.0f, -1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector(-1.0f,  1.0f, -1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector(-1.0f, -1.0f, -1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINE_STRIP, v, 5);

                v[0].coord = l.position + Math::Vector(-1.0f, -1.0f,  1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector( 1.0f, -1.0f,  1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f,  1.0f,  1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector(-1.0f,  1.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector(-1.0f, -1.0f,  1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINE_STRIP, v, 5);

                v[0].coord = l.position + Math::Vector(-1.0f, -1.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector(-1.0f, -1.0f,  1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f, -1.0f, -1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector( 1.0f, -1.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector( 1.0f,  1.0f, -1.0f) * 4.0f;
                v[5].coord = l.position + Math::Vector( 1.0f,  1.0f,  1.0f) * 4.0f;
                v[6].coord = l.position + Math::Vector(-1.0f,  1.0f, -1.0f) * 4.0f;
                v[7].coord = l.position + Math::Vector(-1.0f,  1.0f,  1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINES, v, 8);
            }
            else if (l.type == LIGHT_SPOT)
            {
                Gfx::VertexCol v[5];
                for (int i = 0; i < 5; ++i)
                    v[i].color = HSV2RGB(color);

                v[0].coord = l.position + Math::Vector(-1.0f,  0.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector( 1.0f,  0.0f, -1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f,  0.0f,  1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector(-1.0f,  0.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector(-1.0f,  0.0f, -1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINE_STRIP, v, 5);

                v[0].coord = l.position;
                v[1].coord = l.position + Math::Normalize(l.direction) * 100.0f;
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(3.0, 0xFF);
                DrawPrimitive(PRIMITIVE_LINES, v, 2);
                glDisable(GL_LINE_STIPPLE);
            }
        }
    }

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);

    SetTransform(TRANSFORM_WORLD, saveWorldMat);
}

bool CGL33Device::Create()
{
    GetLogger()->Info("Creating CDevice - OpenGL 3.3\n");

    /*static*/ bool glewInited = false;

    if (!glewInited)
    {
        glewInited = true;

        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            GetLogger()->Error("GLEW initialization failed\n");
            return false;
        }

        // Extract OpenGL version
        const char *version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        sscanf(version, "%d.%d", &m_glMajor, &m_glMinor);

        int glVersion = 10 * m_glMajor + m_glMinor;
        if (glVersion < 30)
        {
            GetLogger()->Error("Your hardware does not support OpenGL 3.0+. Exiting.\n");
            return false;
        }
        else if (glVersion < 33)
        {
            GetLogger()->Warn("Full OpenGL 3.3 unavailable. Graphics might be bugged.\n");
        }
        else
        {
            GetLogger()->Info("OpenGL %d.%d\n", m_glMajor, m_glMinor);
        }

        // Detect support of anisotropic filtering
        m_anisotropyAvailable = glewIsSupported("GL_EXT_texture_filter_anisotropic");
        if(m_anisotropyAvailable)
        {
            // Obtain maximum anisotropy level available
            float level;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &level);
            m_maxAnisotropy = static_cast<int>(level);

            GetLogger()->Info("Anisotropic filtering available\n");
            GetLogger()->Info("Maximum anisotropy: %d\n", m_maxAnisotropy);
        }
        else
        {
            GetLogger()->Info("Anisotropic filtering not available\n");
        }
    }

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    int numLights = 8;

    m_lights        = std::vector<Light>(numLights, Light());
    m_lightsEnabled = std::vector<bool> (numLights, false);

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    GetLogger()->Info("Maximum texture image units: %d\n", maxTextures);

    m_currentTextures    = std::vector<Texture>           (maxTextures, Texture());
    m_texturesEnabled    = std::vector<bool>              (maxTextures, false);
    m_textureStageParams = std::vector<TextureStageParams>(maxTextures, TextureStageParams());

    // Create auxilliary vertex buffer
    m_vertex = CreateStaticBuffer(PRIMITIVE_POINTS, static_cast<Vertex*>(nullptr), 1);
    m_vertexTex2 = CreateStaticBuffer(PRIMITIVE_POINTS, static_cast<VertexTex2*>(nullptr), 1);
    m_vertexCol = CreateStaticBuffer(PRIMITIVE_POINTS, static_cast<VertexCol*>(nullptr), 1);

    int value;
    if (CConfigFile::GetInstance().GetIntProperty("Setup", "PerPixelLighting", value))
    {
        m_perPixelLighting = value > 0;
    }

    if (m_perPixelLighting)
        CLogger::GetInstance().Info("Using per-pixel lighting\n");
    else
        CLogger::GetInstance().Info("Using per-vertex lighting\n");

    // Create shader program
    GLint shaders[2];
    char filename[64];

    if (m_perPixelLighting)
        sprintf(filename, "shaders/vertex_shader_33_perpixel.glsl");
    else
        sprintf(filename, "shaders/vertex_shader_33_pervertex.glsl");

    shaders[0] = LoadShader(GL_VERTEX_SHADER, filename);
    if (shaders[0] == 0) return false;

    if (m_perPixelLighting)
        sprintf(filename, "shaders/fragment_shader_33_perpixel.glsl");
    else
        sprintf(filename, "shaders/fragment_shader_33_pervertex.glsl");

    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, filename);
    if (shaders[1] == 0) return false;

    m_shaderProgram = LinkProgram(2, shaders);
    if (m_shaderProgram == 0) return false;

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    glUseProgram(m_shaderProgram);

    // Obtain uniform locations
    uni_ProjectionMatrix = glGetUniformLocation(m_shaderProgram, "uni_ProjectionMatrix");
    uni_ViewMatrix = glGetUniformLocation(m_shaderProgram, "uni_ViewMatrix");
    uni_ModelMatrix = glGetUniformLocation(m_shaderProgram, "uni_ModelMatrix");
    uni_NormalMatrix = glGetUniformLocation(m_shaderProgram, "uni_NormalMatrix");
    uni_ShadowMatrix = glGetUniformLocation(m_shaderProgram, "uni_ShadowMatrix");

    uni_PrimaryTexture = glGetUniformLocation(m_shaderProgram, "uni_PrimaryTexture");
    uni_SecondaryTexture = glGetUniformLocation(m_shaderProgram, "uni_SecondaryTexture");
    uni_ShadowTexture = glGetUniformLocation(m_shaderProgram, "uni_ShadowTexture");

    uni_PrimaryTextureEnabled = glGetUniformLocation(m_shaderProgram, "uni_PrimaryTextureEnabled");
    uni_SecondaryTextureEnabled = glGetUniformLocation(m_shaderProgram, "uni_SecondaryTextureEnabled");
    uni_ShadowTextureEnabled = glGetUniformLocation(m_shaderProgram, "uni_ShadowTextureEnabled");

    uni_FogEnabled = glGetUniformLocation(m_shaderProgram, "uni_FogEnabled");
    uni_FogRange = glGetUniformLocation(m_shaderProgram, "uni_FogRange");
    uni_FogColor = glGetUniformLocation(m_shaderProgram, "uni_FogColor");

    uni_AlphaTestEnabled = glGetUniformLocation(m_shaderProgram, "uni_AlphaTestEnabled");
    uni_AlphaReference = glGetUniformLocation(m_shaderProgram, "uni_AlphaReference");

    uni_ShadowColor = glGetUniformLocation(m_shaderProgram, "uni_ShadowColor");

    uni_SmoothShading = glGetUniformLocation(m_shaderProgram, "uni_SmoothShading");
    uni_LightingEnabled = glGetUniformLocation(m_shaderProgram, "uni_LightingEnabled");

    uni_AmbientColor = glGetUniformLocation(m_shaderProgram, "uni_AmbientColor");
    uni_DiffuseColor = glGetUniformLocation(m_shaderProgram, "uni_DiffuseColor");
    uni_SpecularColor = glGetUniformLocation(m_shaderProgram, "uni_SpecularColor");

    GLchar name[64];
    for (int i = 0; i < 8; i++)
    {
        sprintf(name, "uni_Light[%d].Enabled", i);
        uni_Light[i].Enabled = glGetUniformLocation(m_shaderProgram, name);

        sprintf(name, "uni_Light[%d].Position", i);
        uni_Light[i].Position = glGetUniformLocation(m_shaderProgram, name);

        sprintf(name, "uni_Light[%d].Ambient", i);
        uni_Light[i].Ambient = glGetUniformLocation(m_shaderProgram, name);

        sprintf(name, "uni_Light[%d].Diffuse", i);
        uni_Light[i].Diffuse = glGetUniformLocation(m_shaderProgram, name);

        sprintf(name, "uni_Light[%d].Specular", i);
        uni_Light[i].Specular = glGetUniformLocation(m_shaderProgram, name);

        sprintf(name, "uni_Light[%d].Attenuation", i);
        uni_Light[i].Attenuation = glGetUniformLocation(m_shaderProgram, name);
    }

    // Set default uniform values
    Math::Matrix matrix;
    matrix.LoadIdentity();

    glUniformMatrix4fv(uni_ProjectionMatrix, 1, GL_FALSE, matrix.Array());
    glUniformMatrix4fv(uni_ViewMatrix, 1, GL_FALSE, matrix.Array());
    glUniformMatrix4fv(uni_ModelMatrix, 1, GL_FALSE, matrix.Array());
    glUniformMatrix4fv(uni_NormalMatrix, 1, GL_FALSE, matrix.Array());
    glUniformMatrix4fv(uni_ShadowMatrix, 1, GL_FALSE, matrix.Array());

    glUniform1i(uni_PrimaryTexture, 0);
    glUniform1i(uni_SecondaryTexture, 1);
    glUniform1i(uni_ShadowTexture, 2);

    glUniform1i(uni_PrimaryTextureEnabled, 0);
    glUniform1i(uni_SecondaryTextureEnabled, 0);
    glUniform1i(uni_ShadowTextureEnabled, 0);

    glUniform4f(uni_AmbientColor, 0.4f, 0.4f, 0.4f, 1.0f);
    glUniform4f(uni_DiffuseColor, 0.8f, 0.8f, 0.8f, 1.0f);
    glUniform4f(uni_SpecularColor, 0.3f, 0.3f, 0.3f, 1.0f);

    glUniform1i(uni_FogEnabled, 0);
    glUniform2f(uni_FogRange, 100.0f, 200.0f);
    glUniform4f(uni_FogColor, 0.8f, 0.8f, 0.8f, 1.0f);

    glUniform1f(uni_ShadowColor, 0.5f);

    glUniform1i(uni_AlphaTestEnabled, 0);
    glUniform1f(uni_AlphaReference, 1.0f);

    glUniform1i(uni_LightingEnabled, 0);

    for (int i = 0; i < 8; i++)
        glUniform1i(uni_Light[i].Enabled, 0);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = new CDefaultFramebuffer(framebufferParams);

    GetLogger()->Info("CDevice created successfully\n");

    return true;
}

void CGL33Device::Destroy()
{
    // delete shader program
    glUseProgram(0);
    glDeleteProgram(m_shaderProgram);

    // delete framebuffers
    for (std::map<std::string, CFramebuffer*>::iterator i = m_framebuffers.begin(); i != m_framebuffers.end(); i++)
    {
        i->second->Destroy();
        delete i->second;
    }

    m_framebuffers.clear();

    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    DestroyAllTextures();

    m_lights.clear();
    m_lightsEnabled.clear();

    m_currentTextures.clear();
    m_texturesEnabled.clear();
    m_textureStageParams.clear();
}

void CGL33Device::ConfigChanged(const DeviceConfig& newConfig)
{
    m_config = newConfig;

    // Reset state
    m_lighting = false;
    Destroy();
    Create();
}

void CGL33Device::BeginScene()
{
    Clear();

    glUniformMatrix4fv(uni_ProjectionMatrix, 1, GL_FALSE, m_projectionMat.Array());
    glUniformMatrix4fv(uni_ViewMatrix, 1, GL_FALSE, m_viewMat.Array());
    glUniformMatrix4fv(uni_ModelMatrix, 1, GL_FALSE, m_worldMat.Array());
}

void CGL33Device::EndScene()
{
}

void CGL33Device::Clear()
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CGL33Device::SetTransform(TransformType type, const Math::Matrix &matrix)
{
    if      (type == TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        glUniformMatrix4fv(uni_ModelMatrix, 1, GL_FALSE, m_worldMat.Array());

        // normal transform
        Math::Matrix normalMat = matrix;

        if (abs(normalMat.Det()) > 1e-6)
            normalMat = normalMat.Inverse();

        glUniformMatrix4fv(uni_NormalMatrix, 1, GL_TRUE, normalMat.Array());
    }
    else if (type == TRANSFORM_VIEW)
    {
        m_viewMat = matrix;
        Math::Matrix scale;
        Math::LoadScaleMatrix(scale, Math::Vector(1.0f, 1.0f, -1.0f));
        Math::Matrix temp = Math::MultiplyMatrices(scale, matrix);
        glUniformMatrix4fv(uni_ViewMatrix, 1, GL_FALSE, temp.Array());
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = matrix;
        glUniformMatrix4fv(uni_ProjectionMatrix, 1, GL_FALSE, m_projectionMat.Array());
    }
    else if (type == TRANSFORM_SHADOW)
    {
        Math::Matrix temp = matrix;
        glUniformMatrix4fv(uni_ShadowMatrix, 1, GL_FALSE, temp.Array());
    }
    else
    {
        assert(false);
    }
}

void CGL33Device::SetMaterial(const Material &material)
{
    m_material = material;

    glUniform4fv(uni_AmbientColor, 1, m_material.ambient.Array());
    glUniform4fv(uni_DiffuseColor, 1, m_material.diffuse.Array());
    glUniform4fv(uni_SpecularColor, 1, m_material.specular.Array());
}

int CGL33Device::GetMaxLightCount()
{
    return m_lights.size();
}

void CGL33Device::SetLight(int index, const Light &light)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lights[index] = light;

    glUniform4fv(uni_Light[index].Ambient, 1, light.ambient.Array());
    glUniform4fv(uni_Light[index].Diffuse, 1, light.diffuse.Array());
    glUniform4fv(uni_Light[index].Specular, 1, light.specular.Array());
    glUniform3f(uni_Light[index].Attenuation, light.attenuation0, light.attenuation1, light.attenuation2);

    if (light.type == LIGHT_DIRECTIONAL)
    {
        glUniform4f(uni_Light[index].Position, -light.direction.x, -light.direction.y, -light.direction.z, 0.0f);
    }
    else
    {
        glUniform4f(uni_Light[index].Position, light.position.x, light.position.y, light.position.z, 1.0f);
    }

    // TODO: add spotlight params
}

// probably makes no sense anymore
void CGL33Device::UpdateLightPosition(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    /*
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
    */
}

void CGL33Device::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lightsEnabled[index] = enabled;

    glUniform1i(uni_Light[index].Enabled, enabled ? 1 : 0);
}

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Texture CGL33Device::CreateTexture(CImage *image, const TextureCreateParams &params)
{
    ImageData *data = image->GetData();
    if (data == nullptr)
    {
        GetLogger()->Error("Invalid texture data\n");
        return Texture(); // invalid texture
    }

    Math::IntPoint originalSize = image->GetSize();

    if (params.padToNearestPowerOfTwo)
        image->PadToNearestPowerOfTwo();

    Texture tex = CreateTexture(data, params);
    tex.originalSize = originalSize;

    return tex;
}

Texture CGL33Device::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture result;

    result.size.x = data->surface->w;
    result.size.y = data->surface->h;

    result.originalSize = result.size;

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    // Set texture parameters
    GLint minF = GL_NEAREST, magF = GL_NEAREST;
    int mipmapLevel = 1;

    switch (params.filter)
    {
    case TEX_FILTER_NEAREST:
        minF = GL_NEAREST;
        magF = GL_NEAREST;
        break;
    case TEX_FILTER_BILINEAR:
        minF = GL_LINEAR;
        magF = GL_LINEAR;
        break;
    case TEX_FILTER_TRILINEAR:
        minF = GL_LINEAR_MIPMAP_LINEAR;
        magF = GL_LINEAR;
        mipmapLevel = CEngine::GetInstance().GetTextureMipmapLevel();
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);

    // Set mipmap level and automatic mipmap generation if neccesary
    if (params.mipmap)
    {
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapLevel - 1);
    }
    else
    {
	    // Has to be set to 0 because no mipmaps are generated
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }

    // Set anisotropy level if available
    if (m_anisotropyAvailable)
    {
	    float level = Math::Min(m_maxAnisotropy, CEngine::GetInstance().GetTextureAnisotropyLevel());

	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, level);
    }

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
        if (data->surface->format->BytesPerPixel == 4)
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
        else if (data->surface->format->BytesPerPixel == 3)
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
        else {
            GetLogger()->Error("Unknown data surface format");
            assert(false);
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, actualSurface->w, actualSurface->h,
                 0, sourceFormat, GL_UNSIGNED_BYTE, actualSurface->pixels);

    if (params.mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(convertedSurface);

    // Restore the previous state of 1st stage
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    return result;
}

Texture CGL33Device::CreateDepthTexture(int width, int height, int depth)
{
    Texture result;

    result.alpha = false;
    result.size.x = width;
    result.size.y = height;

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    GLuint format = GL_DEPTH_COMPONENT;

    switch (depth)
    {
    case 16:
        format = GL_DEPTH_COMPONENT16;
        break;
    case 24:
        format = GL_DEPTH_COMPONENT24;
        break;
    case 32:
        format = GL_DEPTH_COMPONENT32;
        break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    return result;
}

void CGL33Device::DestroyTexture(const Texture &texture)
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

void CGL33Device::DestroyAllTextures()
{
    // Unbind all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        SetTexture(index, Texture());

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();
}

int CGL33Device::GetMaxTextureStageCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void CGL33Device::SetTexture(int index, const Texture &texture)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_currentTextures[index].id == texture.id;

    m_currentTextures[index] = texture; // remember the new value

    if (same)
        return; // nothing to do

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
}

void CGL33Device::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_currentTextures[index].id == textureId)
        return; // nothing to do

    m_currentTextures[index].id = textureId;

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
}

void CGL33Device::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_texturesEnabled[index] == enabled;

    m_texturesEnabled[index] = enabled;

    if (same)
        return; // nothing to do

    UpdateRenderingMode();
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void CGL33Device::SetTextureStageParams(int index, const TextureStageParams &params)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index] = params;

    UpdateTextureParams(index);
}

void CGL33Device::SetTextureCoordGeneration(int index, TextureGenerationParams &params)
{
    // TODO: think about generalized way
    /*
    glActiveTexture(GL_TEXTURE0 + index);

    for (int i = 0; i < 4; i++)
    {
        GLuint texCoordGen = textureCoordGen[i];
        GLuint texCoord = textureCoordinates[i];

        if (params.coords[i].mode == TEX_GEN_NONE)
        {
            glDisable(texCoordGen);
        }
        else
        {
            glEnable(texCoordGen);

            switch (params.coords[i].mode)
            {
            case TEX_GEN_OBJECT_LINEAR:
                glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
                glTexGenfv(texCoord, GL_OBJECT_PLANE, params.coords[i].plane);
                break;
            case TEX_GEN_EYE_LINEAR:
                glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
                glTexGenfv(texCoord, GL_EYE_PLANE, params.coords[i].plane);
                break;
            case TEX_GEN_SPHERE_MAP:
                glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                break;
            case TEX_GEN_NORMAL_MAP:
                glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
                break;
            case TEX_GEN_REFLECTION_MAP:
                glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
                break;
            }
        }
    }
    // */
}

void CGL33Device::UpdateTextureParams(int index)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    const TextureStageParams &params = m_textureStageParams[index];

    glActiveTexture(GL_TEXTURE0 + index);

    if      (params.wrapS == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    else if (params.wrapS == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    else if (params.wrapS == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (params.wrapT == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    else if (params.wrapT == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    else if (params.wrapT == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);

    // TODO: this needs to be redone
    /*
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
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE1);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE2);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE3);
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
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE0);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE1);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE2);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE3);
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
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE1);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE2);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE3);
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
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE1);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE2);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE3);
    else if (params.alphaArg2 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg2 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg2 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
    else  assert(false);

after_tex_operations: ;
    */
}

void CGL33Device::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index].wrapS = wrapS;
    m_textureStageParams[index].wrapT = wrapT;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    glActiveTexture(GL_TEXTURE0 + index);

    if      (wrapS == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    else if (wrapS == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    else if (wrapS == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (wrapT == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    else if (wrapT == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    else if (wrapT == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);
}

void CGL33Device::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount, Color color)
{
    Vertex* vs = const_cast<Vertex*>(vertices);
    VertexBufferInfo &info = m_vboObjects[m_vertex];

    unsigned int size = vertexCount * sizeof(Vertex);

    BindVAO(info.vao);
    BindVBO(info.vbo);

    // If needed vertex data is too large, increase the size of buffer
    if (info.size >= size)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vs);
    }
    else
    {
        CLogger::GetInstance().Debug("Resizing dynamic buffer: %d->%d\n", info.size, size);
        glBufferData(GL_ARRAY_BUFFER, size, vs, GL_STREAM_DRAW);
        info.size = size;

        // Vertex coordinate
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, coord)));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        // Color
        glDisableVertexAttribArray(2);
        glVertexAttrib4fv(2, color.Array());

        // Texture coordinate 0
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

        // Texture coordinate 1
        glDisableVertexAttribArray(4);
        glVertexAttrib2f(4, 0.0f, 0.0f);
    }

    glVertexAttrib4fv(2, color.Array());

    UpdateRenderingMode();

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);
}

void CGL33Device::DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount, Color color)
{
    VertexTex2* vs = const_cast<VertexTex2*>(vertices);
    VertexBufferInfo &info = m_vboObjects[m_vertexTex2];

    unsigned int size = vertexCount * sizeof(VertexTex2);

    BindVAO(info.vao);
    BindVBO(info.vbo);

    // If needed vertex data is too large, increase the size of buffer
    if (info.size >= size)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vs);
    }
    else
    {
        CLogger::GetInstance().Debug("Resizing dynamic buffer: %d->%d\n", info.size, size);
        glBufferData(GL_ARRAY_BUFFER, size, vs, GL_STREAM_DRAW);
        info.size = size;

        // Vertex coordinate
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, coord)));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, normal)));

        // Color
        glDisableVertexAttribArray(2);
        glVertexAttrib4fv(2, color.Array());

        // Texture coordinate 0
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, texCoord)));

        // Texture coordinate 1
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, texCoord2)));
    }

    glVertexAttrib4fv(2, color.Array());

    UpdateRenderingMode();

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);
}

void CGL33Device::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
{
    VertexCol* vs = const_cast<VertexCol*>(vertices);
    VertexBufferInfo &info = m_vboObjects[m_vertexCol];

    unsigned int size = vertexCount * sizeof(VertexCol);

    BindVAO(info.vao);
    BindVBO(info.vbo);

    // If needed vertex data is too large, increase the size of buffer
    if (info.size >= size)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vs);
    }
    else
    {
        CLogger::GetInstance().Debug("Resizing dynamic buffer: %d->%d\n", info.size, size);
        glBufferData(GL_ARRAY_BUFFER, size, vs, GL_STREAM_DRAW);
        info.size = size;

        // Vertex coordinate
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCol), reinterpret_cast<void*>(offsetof(VertexCol, coord)));

        // Normal
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);

        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexCol), reinterpret_cast<void*>(offsetof(VertexCol, color)));

        // Texture coordinate 0
        glDisableVertexAttribArray(3);
        glVertexAttrib2f(3, 0.0f, 0.0f);

        // Texture coordinate 1
        glDisableVertexAttribArray(4);
        glVertexAttrib2f(4, 0.0f, 0.0f);
    }

    UpdateRenderingMode();

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);
}

unsigned int CGL33Device::CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    unsigned int id = 0;

    id = ++m_lastVboId;

    VertexBufferInfo info;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_NORMAL;
    info.vertexCount = vertexCount;
    info.size = vertexCount * sizeof(Vertex);

    glGenVertexArrays(1, &info.vao);
    BindVAO(info.vao);

    glGenBuffers(1, &info.vbo);
    BindVBO(info.vbo);

    glBufferData(GL_ARRAY_BUFFER, info.size, vertices, GL_STATIC_DRAW);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, coord)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // Color
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);

    // Texture coordinate 0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

    // Texture coordinate 1
    glDisableVertexAttribArray(4);
    glVertexAttrib2f(4, 0.0f, 0.0f);

    m_vboObjects[id] = info;

    return id;
}

unsigned int CGL33Device::CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    unsigned int id = 0;

    id = ++m_lastVboId;

    VertexBufferInfo info;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_TEX2;
    info.vertexCount = vertexCount;
    info.size = vertexCount * sizeof(VertexTex2);

    glGenVertexArrays(1, &info.vao);
    BindVAO(info.vao);

    glGenBuffers(1, &info.vbo);
    BindVBO(info.vbo);

    glBufferData(GL_ARRAY_BUFFER, info.size, vertices, GL_STATIC_DRAW);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, coord)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, normal)));

    // Color
    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);

    // Texture coordinate 0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, texCoord)));

    // Texture coordinate 1
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, texCoord2)));

    m_vboObjects[id] = info;

    return id;
}

unsigned int CGL33Device::CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    unsigned int id = ++m_lastVboId;

    VertexBufferInfo info;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_COL;
    info.vertexCount = vertexCount;
    info.size = vertexCount * sizeof(VertexCol);

    glGenVertexArrays(1, &info.vao);
    BindVAO(info.vao);

    glGenBuffers(1, &info.vbo);
    BindVBO(info.vbo);

    glBufferData(GL_ARRAY_BUFFER, info.size, vertices, GL_STATIC_DRAW);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCol), reinterpret_cast<void*>(offsetof(VertexCol, coord)));

    // Normal
    glDisableVertexAttribArray(1);
    glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);

    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexCol), reinterpret_cast<void*>(offsetof(VertexCol, color)));

    // Texture coordinate 0
    glDisableVertexAttribArray(3);
    glVertexAttrib2f(3, 0.0f, 0.0f);

    // Texture coordinate 1
    glDisableVertexAttribArray(4);
    glVertexAttrib2f(4, 0.0f, 0.0f);

    m_vboObjects[id] = info;

    return id;
}

void CGL33Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VertexBufferInfo& info = (*it).second;

    unsigned int size = vertexCount * sizeof(Vertex);

    bool changed = (info.vertexType != VERTEX_TYPE_NORMAL) || (size > info.size);

    if (info.vertexType != VERTEX_TYPE_NORMAL) CLogger::GetInstance().Debug("Changing static buffer type\n");

    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_NORMAL;
    info.vertexCount = vertexCount;

    BindVBO(info.vbo);

    if (info.size < size)
    {
        CLogger::GetInstance().Debug("Resizing static buffer: %d->%d\n", info.size, size);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        info.size = size;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
    }

    if (changed)        // Update vertex array bindings
    {
        BindVAO(info.vao);

        // Vertex coordinate
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, coord)));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        // Color
        glDisableVertexAttribArray(2);
        glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);

        // Texture coordinate 0
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

        // Texture coordinate 1
        glDisableVertexAttribArray(4);
        glVertexAttrib2f(4, 0.0f, 0.0f);
    }
}

void CGL33Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VertexBufferInfo& info = (*it).second;

    unsigned int size = vertexCount * sizeof(VertexTex2);

    bool changed = (info.vertexType != VERTEX_TYPE_TEX2) || (size > info.size);

    if (info.vertexType != VERTEX_TYPE_TEX2) CLogger::GetInstance().Debug("Changing static buffer type\n");

    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_TEX2;
    info.vertexCount = vertexCount;

    BindVBO(info.vbo);

    if (info.size < size)
    {
        CLogger::GetInstance().Debug("Resizing static buffer: %d->%d\n", info.size, size);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        info.size = size;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
    }

    if (changed)        // Update vertex array bindings
    {
        BindVAO(info.vao);

        // Vertex coordinate
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, coord)));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, normal)));

        // Color
        glDisableVertexAttribArray(2);
        glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);

        // Texture coordinate 0
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, texCoord)));

        // Texture coordinate 1
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex2), reinterpret_cast<void*>(offsetof(VertexTex2, texCoord2)));
    }
}

void CGL33Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VertexBufferInfo& info = (*it).second;

    unsigned int size = vertexCount * sizeof(VertexCol);

    bool changed = (info.vertexType != VERTEX_TYPE_COL) || (size > info.size);

    if (info.vertexType != VERTEX_TYPE_NORMAL) CLogger::GetInstance().Debug("Changing static buffer type\n");

    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_COL;
    info.vertexCount = vertexCount;

    BindVBO(info.vbo);

    if (info.size < size)
    {
        CLogger::GetInstance().Debug("Resizing static buffer: %d->%d\n", info.size, size);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        info.size = size;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
    }

    if (changed)        // Update vertex array bindings
    {
        BindVAO(info.vao);

        // Vertex coordinate
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCol), reinterpret_cast<void*>(offsetof(VertexCol, coord)));

        // Normal
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);

        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexCol), reinterpret_cast<void*>(offsetof(VertexCol, color)));

        // Texture coordinate 0
        glDisableVertexAttribArray(3);
        glVertexAttrib2f(3, 0.0f, 0.0f);

        // Texture coordinate 1
        glDisableVertexAttribArray(4);
        glVertexAttrib2f(4, 0.0f, 0.0f);
    }
}

void CGL33Device::DrawStaticBuffer(unsigned int bufferId)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VertexBufferInfo &info = (*it).second;

    UpdateRenderingMode();

    BindVAO(info.vao);

    GLenum mode = TranslateGfxPrimitive(info.primitiveType);
    glDrawArrays(mode, 0, info.vertexCount);
}

void CGL33Device::DestroyStaticBuffer(unsigned int bufferId)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VertexBufferInfo &info = (*it).second;

    if (m_currentVAO == info.vao)
        BindVAO(0);
    if (m_currentVBO == info.vbo)
        BindVBO(0);

    glDeleteBuffers(1, &info.vbo);
    glDeleteVertexArrays(1, &info.vao);

    info.vbo = 0;
    info.vao = 0;

    m_vboObjects.erase(it);
}

/* Based on libwine's implementation */

int CGL33Device::ComputeSphereVisibility(const Math::Vector &center, float radius)
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

void CGL33Device::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void CGL33Device::SetRenderState(RenderState state, bool enabled)
{
    if (state == RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == RENDER_STATE_LIGHTING)
    {
        m_lighting = enabled;

        glUniform1i(uni_LightingEnabled, enabled ? 1 : 0);

        return;
    }
    else if (state == RENDER_STATE_FOG)
    {
        glUniform1i(uni_FogEnabled, enabled ? 1 : 0);

        return;
    }
    else if (state == RENDER_STATE_ALPHA_TEST)
    {
        glUniform1i(uni_AlphaTestEnabled, enabled ? 1 : 0);

        return;
    }

    GLenum flag = 0;

    switch (state)
    {
        case RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        case RENDER_STATE_DEPTH_BIAS:  flag = GL_POLYGON_OFFSET_FILL; break;
        default: assert(false); break;
    }

    if (enabled)
        glEnable(flag);
    else
        glDisable(flag);
}

void CGL33Device::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGL33Device::SetDepthTestFunc(CompFunc func)
{
    glDepthFunc(TranslateGfxCompFunc(func));
}

void CGL33Device::SetDepthBias(float factor, float units)
{
    glPolygonOffset(factor, units);
}

void CGL33Device::SetAlphaTestFunc(CompFunc func, float refValue)
{
    glUniform1f(uni_AlphaReference, refValue);
}

void CGL33Device::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void CGL33Device::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void CGL33Device::SetGlobalAmbient(const Color &color)
{
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

void CGL33Device::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
    // TODO: reimplement

    glUniform2f(uni_FogRange, start, end);
    glUniform4f(uni_FogColor, color.r, color.g, color.b, color.a);

    /*
    if      (mode == FOG_LINEAR) glFogi(GL_FOG_MODE, GL_LINEAR);
    else if (mode == FOG_EXP)    glFogi(GL_FOG_MODE, GL_EXP);
    else if (mode == FOG_EXP2)   glFogi(GL_FOG_MODE, GL_EXP2);
    else assert(false);

    glFogf(GL_FOG_START,   start);
    glFogf(GL_FOG_END,     end);
    glFogf(GL_FOG_DENSITY, density);
    glFogfv(GL_FOG_COLOR,  color.Array());
    // */
}

void CGL33Device::SetCullMode(CullMode mode)
{
    // Cull clockwise back faces, so front face is the opposite
    // (assuming GL_CULL_FACE is GL_BACK)
    if      (mode == CULL_CW ) glFrontFace(GL_CCW);
    else if (mode == CULL_CCW) glFrontFace(GL_CW);
    else assert(false);
}

void CGL33Device::SetShadeModel(ShadeModel model)
{
    glUniform1i(uni_SmoothShading, (model == SHADE_SMOOTH ? 1 : 0));
}

void CGL33Device::SetShadowColor(float value)
{
    glUniform1f(uni_ShadowColor, value);
}

void CGL33Device::SetFillMode(FillMode mode)
{
    if      (mode == FILL_POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == FILL_LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == FILL_POLY)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

void CGL33Device::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
    if (texture.id == 0) return;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, x, y, width, height);

    // Restore previous texture
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);
}

void* CGL33Device::GetFrameBufferPixels() const
{
    GLubyte* pixels = new GLubyte[4 * m_config.size.x * m_config.size.y];

    glReadPixels(0, 0, m_config.size.x, m_config.size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    unsigned int* p = static_cast<unsigned int*> ( static_cast<void*>(pixels) );

    for (int i = 0; i < m_config.size.x * m_config.size.y; ++i)
        p[i] |= 0xFF000000;

    return static_cast<void*>(p);
}

CFramebuffer* CGL33Device::GetFramebuffer(std::string name)
{
    if (m_framebuffers.find(name) != m_framebuffers.end())
        return m_framebuffers[name];
    else
        return nullptr;
}

CFramebuffer* CGL33Device::CreateFramebuffer(std::string name, const FramebufferParams& params)
{
    // existing framebuffer was found
    if (m_framebuffers.find(name) != m_framebuffers.end())
    {
        return nullptr;
    }
    else
    {
        CGLFramebuffer* framebuffer = new CGLFramebuffer(params);
        framebuffer->Create();

        m_framebuffers[name] = framebuffer;
        return framebuffer;
    }
}

void CGL33Device::DeleteFramebuffer(std::string name)
{
    // can't delete default framebuffer
    if (name == "default") return;

    auto position = m_framebuffers.find(name);

    if (position != m_framebuffers.end())
    {
        position->second->Destroy();
        delete position->second;

        m_framebuffers.erase(position);
    }
}

void CGL33Device::UpdateRenderingMode()
{
    bool enabled = m_texturesEnabled[0] && m_currentTextures[0].id != 0;
    glUniform1i(uni_PrimaryTextureEnabled, enabled ? 1 : 0);

    enabled = m_texturesEnabled[1] && m_currentTextures[1].id != 0;
    glUniform1i(uni_SecondaryTextureEnabled, enabled ? 1 : 0);

    enabled = m_texturesEnabled[2] && m_currentTextures[2].id != 0;
    glUniform1i(uni_ShadowTextureEnabled, enabled ? 1 : 0);
}

inline void CGL33Device::BindVBO(GLuint vbo)
{
    if (m_currentVBO == vbo) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    m_currentVBO = vbo;
}

inline void CGL33Device::BindVAO(GLuint vao)
{
    if (m_currentVAO == vao) return;

    glBindVertexArray(vao);
    m_currentVAO = vao;
}

bool CGL33Device::IsAnisotropySupported()
{
    return m_anisotropyAvailable;
}

int CGL33Device::GetMaxAnisotropyLevel()
{
    return m_maxAnisotropy;
}

} // namespace Gfx
