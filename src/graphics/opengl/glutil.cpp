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

#include "graphics/opengl/glutil.h"
#include "graphics/opengl/gldevice.h"
#include "graphics/opengl/gl21device.h"
#include "graphics/opengl/gl33device.h"
#include "common/logger.h"
#include <physfs.h>
#include <cstring>

// Graphics module namespace
namespace Gfx {

GLuint textureCoordinates[] = { GL_S, GL_T, GL_R, GL_Q };
GLuint textureCoordGen[] = { GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_TEXTURE_GEN_R, GL_TEXTURE_GEN_Q };

FramebufferSupport DetectFramebufferSupport()
{
    if (GetOpenGLVersion() >= 30) return FBS_ARB;
    if (glewIsSupported("GL_ARB_framebuffer_object")) return FBS_ARB;
    if (glewIsSupported("GL_EXT_framebuffer_object")) return FBS_EXT;
    return FBS_NONE;
}

std::unique_ptr<CDevice> CreateDevice(const DeviceConfig &config, const std::string& name)
{
    if      (name == "default") return std::unique_ptr<CDevice>{new CGLDevice(config)};
    else if (name == "opengl")  return std::unique_ptr<CDevice>{new CGLDevice(config)};
    else if (name == "gl14")    return std::unique_ptr<CDevice>{new CGLDevice(config)};
    else if (name == "gl21")    return std::unique_ptr<CDevice>{new CGL21Device(config)};
    else if (name == "gl33")    return std::unique_ptr<CDevice>{new CGL33Device(config)};
    else if (name == "auto")
    {
        int version = GetOpenGLVersion();

             if (version >= 33) return std::unique_ptr<CDevice>{new CGL33Device(config)};
        else if (version >= 21) return std::unique_ptr<CDevice>{new CGL21Device(config)};
        else                    return std::unique_ptr<CDevice>{new CGLDevice(config)};
    }

    return nullptr;
}

int GetOpenGLVersion()
{
    const char *version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    int major = 0, minor = 0;

    sscanf(version, "%d.%d", &major, &minor);

    return 10 * major + minor;
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

bool InPlane(Math::Vector normal, float originPlane, Math::Vector center, float radius)
{
    float distance = originPlane + Math::DotProduct(normal, center);

    if (distance < -radius)
        return false;

    return true;
}

GLenum TranslateTextureCoordinate(int index)
{
    assert(index >= 0 && index < 4);

    return textureCoordinates[index];
}

GLenum TranslateTextureCoordinateGen(int index)
{
    assert(index >= 0 && index < 4);

    return textureCoordGen[index];
}

GLint LoadShader(GLint type, const char* filename)
{
    PHYSFS_file *file = PHYSFS_openRead(filename);
    if (file == nullptr)
    {
        CLogger::GetInstance().Error("Cannot read shader source file\n");
        CLogger::GetInstance().Error("Missing file \"%s\"\n", filename);
        return 0;
    }

    GLchar source[65536];
    GLchar *sources[] = { source };
    int length = PHYSFS_read(file, source, 1, 65536);
    source[length] = '\0';

    PHYSFS_close(file);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, const_cast<const GLchar**>(sources), nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
    {
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        GLchar *message = new GLchar[len + 1];
        glGetShaderInfoLog(shader, len + 1, nullptr, message);

        GetLogger()->Error("Shader compilation error occured!\n%s\n", message);

        delete[] message;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLint LinkProgram(int count, GLint shaders[])
{
    GLint program = glCreateProgram();

    for (int i = 0; i < count; i++)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);

    for (int i = 0; i < count; i++)
        glDetachShader(program, shaders[i]);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

        GLchar *message = new GLchar[len + 1];
        glGetProgramInfoLog(program, len + 1, nullptr, message);

        GetLogger()->Error("Shader program linking error occured!\n%s\n", message);

        delete[] message;
        glDeleteProgram(program);

        return 0;
    }

    return program;
}

}
