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

#include "graphics/core/device.h"

// config.h must be included before glew.h
#include "common/config.h"

#include <GL/glew.h>


// Graphics module namespace
namespace Gfx
{

/**
\enum VBOMode
\brief VBO autodetect/override
*/
enum VBOMode
{
    VBO_MODE_ENABLE,  //! < override: enable
    VBO_MODE_DISABLE, //! < override: disable
    VBO_MODE_AUTO     //! < autodetect
};

/**
\struct GLDeviceConfig
\brief Additional config with OpenGL-specific settings */
struct GLDeviceConfig : public DeviceConfig
{
    //! VBO override/autodetect
    VBOMode vboMode;

    //! Constructor calls LoadDefaults()
    GLDeviceConfig();

    //! Loads the default values
    void LoadDefault();
};

//! Creates OpenGL device
CDevice* CreateDevice(const GLDeviceConfig &config, const char *name);

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

} // namespace Gfx
