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

#pragma once

// config.h must be included first
#include "common/config.h"

#include "common/make_unique.h"

#include "graphics/core/device.h"

#include "math/intpoint.h"

#include <GL/glew.h>

#include <memory>


// Graphics module namespace
namespace Gfx
{

enum FramebufferSupport
{
    FBS_NONE,
    FBS_EXT,
    FBS_ARB,
};

FramebufferSupport DetectFramebufferSupport();

//! Creates OpenGL device
std::unique_ptr<CDevice> CreateDevice(const DeviceConfig &config, const std::string& name);

//! Returns OpenGL version as one number.
// First digit is major part, second digit is minor part.
int GetOpenGLVersion();

//! Translate Gfx primitive type to OpenGL primitive type
GLenum TranslateGfxPrimitive(PrimitiveType type);

CompFunc TranslateGLCompFunc(GLenum flag);

GLenum TranslateGfxCompFunc(CompFunc func);

BlendFunc TranslateGLBlendFunc(GLenum flag);

GLenum TranslateGfxBlendFunc(BlendFunc func);

bool InPlane(Math::Vector normal, float originPlane, Math::Vector center, float radius);

GLenum TranslateTextureCoordinate(int index);

GLenum TranslateTextureCoordinateGen(int index);

GLint LoadShader(GLint type, const char* filename);

GLint LinkProgram(int count, GLint shaders[]);

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

} // namespace Gfx
