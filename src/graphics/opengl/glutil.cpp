/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/opengl/glutil.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "graphics/opengl/gl21device.h"
#include "graphics/opengl/gl33device.h"
#include "graphics/opengl/gldevice.h"

#include <physfs.h>
#include <cstring>
#include <sstream>

// Graphics module namespace
namespace Gfx
{

GLuint textureCoordinates[] = { GL_S, GL_T, GL_R, GL_Q };
GLuint textureCoordGen[] = { GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_TEXTURE_GEN_R, GL_TEXTURE_GEN_Q };

bool InitializeGLEW()
{
    static bool glewInited = false;

    if (!glewInited)
    {
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            GetLogger()->Error("GLEW initialization failed\n");
            return false;
        }

        glewInited = true;
    }

    return true;
}

FramebufferSupport DetectFramebufferSupport()
{
    if (GetOpenGLVersion() >= 30) return FBS_ARB;
    if (glewIsSupported("GL_ARB_framebuffer_object")) return FBS_ARB;
    if (glewIsSupported("GL_EXT_framebuffer_object")) return FBS_EXT;
    return FBS_NONE;
}

std::unique_ptr<CDevice> CreateDevice(const DeviceConfig &config, const std::string& name)
{
    if      (name == "default") return MakeUnique<CGLDevice>(config);
    else if (name == "opengl")  return MakeUnique<CGLDevice>(config);
    else if (name == "gl14")    return MakeUnique<CGLDevice>(config);
    else if (name == "gl21")    return MakeUnique<CGL21Device>(config);
    else if (name == "gl33")    return MakeUnique<CGL33Device>(config);
    else if (name == "auto")
    {
        int version = GetOpenGLVersion();

             if (version >= 33) return MakeUnique<CGL33Device>(config);
        else if (version >= 21) return MakeUnique<CGL21Device>(config);
        else                    return MakeUnique<CGLDevice>(config);
    }

    return nullptr;
}

int GetOpenGLVersion()
{
    int major, minor;

    return GetOpenGLVersion(major, minor);
}

int GetOpenGLVersion(int &major, int &minor)
{
    const char *version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    sscanf(version, "%d.%d", &major, &minor);

    return 10 * major + minor;
}

std::string GetHardwareInfo(bool full)
{
    int glversion = GetOpenGLVersion();

    std::stringstream result;

    // basic hardware information
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    result << "Hardware information:\n\n";
    result << "OpenGL Version:\t\t" << version << '\n';
    result << "Hardware Vendor:\t\t" << vendor << '\n';
    result << "Renderer:\t\t\t" << renderer << '\n';

    // GLSL version if available
    if (glversion >= 20)
    {
        const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        result << "Shading Language Version:\t" << glslVersion << '\n';
    }

    if (!full) return result.str();

    // extended hardware information
    int value = 0;

    result << "\nCapabilities:\n\n";

    // texture size
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    result << "Max Texture Size:\t\t" << value << '\n';

    if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
    {
        result << "Anisotropic filtering:\t\tsupported\n";

        float level;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &level);

        result << "    Max Level:\t\t" << static_cast<int>(level) << '\n';
    }
    else
    {
        result << "Anisotropic filtering:\t\tunsupported\n";
    }

    // multitexturing
    if (glversion >= 13)
    {
        result << "Multitexturing:\t\tsupported\n";
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &value);
        result << "    Max Texture Units:\t\t" << value << '\n';

        if (glversion >= 20)
        {
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &value);
            result << "    Max Texture Image Units:\t" << value << '\n';
        }
    }
    else
    {
        result << "Multitexturing:\t\tunsupported\n";
    }

    // FBO support
    FramebufferSupport framebuffer = DetectFramebufferSupport();

    if (framebuffer == FBS_ARB)
    {
        result << "Framebuffer Object:\t\tsupported\n";
        result << "    Type:\t\t\tCore/ARB\n";

        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &value);
        result << "    Max Renderbuffer Size:\t" << value << '\n';

        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
        result << "    Max Color Attachments:\t" << value << '\n';

        result << "Multisampling:\t\tsupported\n";

        glGetIntegerv(GL_MAX_SAMPLES, &value);
        result << "    Max Framebuffer Samples:\t" << value << '\n';
    }
    else if (framebuffer == FBS_EXT)
    {
        result << "Framebuffer Object:\tsupported\n";
        result << "    Type:\tEXT\n";

        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &value);
        result << "    Max Renderbuffer Size:\t" << value << '\n';

        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &value);
        result << "    Max Color Attachments:\t" << value << '\n';

        if (glewIsSupported("GL_EXT_framebuffer_multisample"))
        {
            result << "Multisampling:\tsupported\n";

            glGetIntegerv(GL_MAX_SAMPLES_EXT, &value);
            result << "    Max Framebuffer Samples:\t" << value << '\n';
        }
    }
    else
    {
        result << "Framebuffer Object:\tunsupported\n";
    }

    // VBO support
    if (glversion >= 15)
    {
        result << "VBO:\t\t\tsupported (core)\n";
    }
    else if (glewIsSupported("GL_ARB_vertex_buffer_object"))
    {
        result << "VBO:\t\t\tsupported (ARB)\n";
    }
    else
    {
        result << "VBO:\t\t\tunsupported\n";
    }

    return result.str();
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

std::string lastShaderError;

std::string GetLastShaderError()
{
    return lastShaderError;
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

        auto message = MakeUniqueArray<GLchar>(len + 1);
        glGetShaderInfoLog(shader, len + 1, nullptr, message.get());

        GetLogger()->Error("Shader compilation error occured!\n%s\n", message.get());
        lastShaderError = std::string("Shader compilation error occured!\n\n") + std::string(message.get());

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

        auto message = MakeUniqueArray<GLchar>(len + 1);
        glGetProgramInfoLog(program, len + 1, nullptr, message.get());

        GetLogger()->Error("Shader program linking error occured!\n%s\n", message.get());
        lastShaderError = std::string("Shader program linking error occured!\n\n") + std::string(message.get());

        glDeleteProgram(program);

        return 0;
    }

    return program;
}

std::unique_ptr<CGLFrameBufferPixels> GetGLFrameBufferPixels(Math::IntPoint size)
{
    auto pixels = MakeUnique<CGLFrameBufferPixels>(4 * size.x * size.y);

    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels->GetPixelsData());

    GLuint* p = static_cast<GLuint*> ( pixels->GetPixelsData() );

    for (int i = 0; i < size.x * size.y; ++i)
        p[i] |= 0xFF000000;

    return pixels;
}


} // namespace Gfx
