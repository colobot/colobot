/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/core/device.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <vector>

struct SDL_Surface;


// Graphics module namespace
namespace Gfx
{

bool InitializeGLEW();

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

bool InPlane(glm::vec3 normal, float originPlane, glm::vec3 center, float radius);

GLenum TranslateType(Type type);

std::string GetLastShaderError();

std::string LoadSource(const std::string& path);

GLint CreateShader(GLint type, const std::vector<std::string>& sources);

GLint LoadShader(GLint type, const char* filename);

GLint LinkProgram(int count, const GLint* shaders);

GLint LinkProgram(const std::vector<GLint>& shaders);

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
        : m_pixels(std::make_unique<GLubyte[]>(size))
    {}

    void* GetPixelsData() override
    {
        return static_cast<void*>(m_pixels.get());
    }

private:
    std::unique_ptr<GLubyte[]> m_pixels;
};

std::unique_ptr<CGLFrameBufferPixels> GetGLFrameBufferPixels(const glm::ivec2& size);

} // namespace Gfx
