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

#pragma once

// config.h must be included first
#include "common/config.h"

#include "common/make_unique.h"

#include "graphics/core/device.h"

#include "math/intpoint.h"
#include "math/vector.h"

#include <GL/glew.h>

#include <string>
#include <memory>

struct SDL_Surface;


// Graphics module namespace
namespace Gfx
{

enum FramebufferSupport
{
    FBS_NONE,
    FBS_EXT,
    FBS_ARB,
};

bool InitializeGLEW();

FramebufferSupport DetectFramebufferSupport();

//! Creates OpenGL device
std::unique_ptr<CDevice> CreateDevice(const DeviceConfig &config, const std::string& name);

//! Returns OpenGL version
// \return First digit is major part, second digit is minor part.
int GetOpenGLVersion();

//! Returns OpenGL version
// \return First digit is major part, second digit is minor part.
int GetOpenGLVersion(int &major, int &minor);

//! Checks if extensions in space-delimited list are supported
// \return true if all extensions are supported
bool AreExtensionsSupported(std::string list);

//! Returns information about graphics card
std::string GetHardwareInfo(bool full = false);

//! Clears OpenGL errors
int ClearGLErrors();

//! Checks for OpenGL errors
bool CheckGLErrors();

//! Translate Gfx primitive type to OpenGL primitive type
GLenum TranslateGfxPrimitive(PrimitiveType type);

CompFunc TranslateGLCompFunc(GLenum flag);

GLenum TranslateGfxCompFunc(CompFunc func);

BlendFunc TranslateGLBlendFunc(GLenum flag);

GLenum TranslateGfxBlendFunc(BlendFunc func);

bool InPlane(Math::Vector normal, float originPlane, Math::Vector center, float radius);

GLenum TranslateTextureCoordinate(int index);

GLenum TranslateTextureCoordinateGen(int index);

GLenum TranslateType(Type type);

std::string GetLastShaderError();

GLint LoadShader(GLint type, const char* filename);

GLint LinkProgram(int count, GLint shaders[]);

// TODO: Moved this here temporarily only to remove code duplication in CGLDeviceXX
struct PreparedTextureData
{
    SDL_Surface* actualSurface = nullptr;
    SDL_Surface* convertedSurface = nullptr;
    GLenum sourceFormat = 0;
    bool alpha = false;
};

PreparedTextureData PrepareTextureData(ImageData* imageData, TexImgFormat format);

class CGLFrameBufferPixels : public CFrameBufferPixels
{
public:
    CGLFrameBufferPixels(std::size_t size)
        : m_pixels(MakeUniqueArray<GLubyte>(size))
    {}

    void* GetPixelsData() override
    {
        return static_cast<void*>(m_pixels.get());
    }

private:
    std::unique_ptr<GLubyte[]> m_pixels;
};

std::unique_ptr<CGLFrameBufferPixels> GetGLFrameBufferPixels(Math::IntPoint size);

struct LightLocations
{
    //! true enables light
    GLint enabled = -1;
    //! Light type
    GLint type = -1;
    //! Position or direction vector
    GLint position = -1;
    //! Ambient color
    GLint ambient = -1;
    //! Diffuse color
    GLint diffuse = -1;
    //! Specular color
    GLint specular = -1;
    //! Attenuation
    GLint attenuation = -1;
    //! Spot light direction
    GLint spotDirection = -1;
    //! Spot light exponent
    GLint spotExponent = -1;
    //! Spot light cutoff
    GLint spotCutoff = -1;
};

struct UniformLocations
{
    // Uniforms
    //! Projection matrix
    GLint projectionMatrix = -1;
    //! View matrix
    GLint viewMatrix = -1;
    //! Model matrix
    GLint modelMatrix = -1;
    //! Shadow matrix
    GLint shadowMatrix = -1;
    //! Normal matrix
    GLint normalMatrix = -1;

    //! Primary texture sampler
    GLint primaryTexture = -1;
    //! Secondary texture sampler
    GLint secondaryTexture = -1;
    //! Shadow texture sampler
    GLint shadowTexture = -1;

    //! true enables texture
    GLint textureEnabled[3] = {};

    // Alpha test parameters
    //! true enables alpha test
    GLint alphaTestEnabled = -1;
    //! Alpha test reference value
    GLint alphaReference = -1;

    //! true enables fog
    GLint fogEnabled = -1;
    //! Fog range
    GLint fogRange = -1;
    //! Fog color
    GLint fogColor = -1;

    //! Shadow color
    GLint shadowColor = -1;

    // Number of enabled lights
    GLint lightCount = -1;
    //! Ambient color
    GLint ambientColor = -1;
    //! Diffuse color
    GLint diffuseColor = -1;
    //! Specular color
    GLint specularColor = -1;

    LightLocations lights[8] = {};
};

} // namespace Gfx
