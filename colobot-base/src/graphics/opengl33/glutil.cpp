/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/opengl33/glutil.h"

#include "graphics/core/renderers.h"

#include "graphics/opengl33/gl33_device.h"

#include "common/image.h"
#include "common/logger.h"

#include <SDL.h>
#include <physfs.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>

// Graphics module namespace
namespace Gfx
{

bool InitializeGLEW()
{
    static bool glewInited = false;

    if (!glewInited)
    {
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            GetLogger()->Error("GLEW initialization failed");
            return false;
        }

        glewInited = true;
    }

    return true;
}

std::unique_ptr<CDevice> CreateDevice(const DeviceConfig &config, const std::string& name)
{
    if      (name == "default") return std::make_unique<CGL33Device>(config);
    else if (name == "opengl")  return std::make_unique<CGL33Device>(config);
    else if (name == "gl33")    return std::make_unique<CGL33Device>(config);
    else if (name == "auto")
    {
        int version = GetOpenGLVersion();

        if (version >= 33) return std::make_unique<CGL33Device>(config);
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
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    sscanf(version, "%d.%d", &major, &minor);

    return 10 * major + minor;
}

bool AreExtensionsSupported(std::string list)
{
    // Extract extensions to find
    std::vector<std::string> extensions;
    std::stringstream stream(list);

    std::string value;

    while (true)
    {
        stream >> value;

        if (stream.eof())
            break;

        extensions.push_back(value);
    }

    int version = GetOpenGLVersion();

    // Use glGetString
    if (version < 30)
    {
        const char* text = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        std::stringstream stream(text);

        while (!extensions.empty())
        {
            stream >> value;

            if (stream.eof())
                break;

            auto result = std::remove(extensions.begin(), extensions.end(), value);

            if (result != extensions.end())
                extensions.erase(result);
        }
    }
    // Use glGetStringi
    else
    {
        int n;
        glGetIntegerv(GL_NUM_EXTENSIONS, &n);

        for (int i = 0; i < n; i++)
        {
            const char* name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
            value = std::string(name);

            auto result = std::remove(extensions.begin(), extensions.end(), value);

            if (result != extensions.end())
                extensions.erase(result);

            if (extensions.empty())
                break;
        }
    }

    // Return true if found all required extensions
    return extensions.empty();
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
    result << "Framebuffer Object:\t\tsupported\n";
    result << "    Type:\t\t\tCore/ARB\n";

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &value);
    result << "    Max Renderbuffer Size:\t" << value << '\n';

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
    result << "    Max Color Attachments:\t" << value << '\n';

    result << "Multisampling:\t\tsupported\n";

    glGetIntegerv(GL_MAX_SAMPLES, &value);
    result << "    Max Framebuffer Samples:\t" << value << '\n';

    // VBO support
    result << "VBO:\t\t\tsupported (core)\n";

    return result.str();
}

int ClearGLErrors()
{
    int result = 0;

    while (true)
    {
        GLint error = glGetError();
        if (error == GL_NO_ERROR)
            break;

        result++;
    }

    return result;
}

bool CheckGLErrors()
{
    GLint error = glGetError();
    bool result = false;

    while (error != GL_NO_ERROR)
    {
        GetLogger()->Error("OpenGL error: %%", error);

        result = true;

        error = glGetError();
    }

    return result;
}

GLenum TranslateGfxPrimitive(PrimitiveType type)
{
    GLenum flag = 0;
    switch (type)
    {
    case PrimitiveType::POINTS:         flag = GL_POINTS; break;
    case PrimitiveType::LINES:          flag = GL_LINES; break;
    case PrimitiveType::LINE_STRIP:     flag = GL_LINE_STRIP; break;
    case PrimitiveType::LINE_LOOP:      flag = GL_LINE_LOOP; break;
    case PrimitiveType::TRIANGLES:      flag = GL_TRIANGLES; break;
    case PrimitiveType::TRIANGLE_STRIP: flag = GL_TRIANGLE_STRIP; break;
    case PrimitiveType::TRIANGLE_FAN:   flag = GL_TRIANGLE_FAN; break;
    default: assert(false); break;
    }
    return flag;
}

bool InPlane(glm::vec3 normal, float originPlane, glm::vec3 center, float radius)
{
    float distance = originPlane + glm::dot(normal, center);

    if (distance < -radius)
        return false;

    return true;
}

GLenum TranslateType(Type type)
{
    switch (type)
    {
    case Type::BYTE: return GL_BYTE;
    case Type::UBYTE: return GL_UNSIGNED_BYTE;
    case Type::SHORT: return GL_SHORT;
    case Type::USHORT: return GL_UNSIGNED_SHORT;
    case Type::INT: return GL_INT;
    case Type::UINT: return GL_UNSIGNED_INT;
    case Type::HALF: return GL_HALF_FLOAT;
    case Type::FLOAT: return GL_FLOAT;
    case Type::DOUBLE: return GL_DOUBLE;
    default: return 0;
    }
}

std::string lastShaderError;

std::string GetLastShaderError()
{
    return lastShaderError;
}

std::string LoadSource(const std::string& path)
{
    PHYSFS_file* file = PHYSFS_openRead(path.c_str());
    if (file == nullptr)
    {
        GetLogger()->Error("Cannot read shader source file");
        GetLogger()->Error("Missing file '%%'\n", path);
        return {};
    }

    size_t len = PHYSFS_fileLength(file);

    std::string source(len, ' ');

    PHYSFS_readBytes(file, source.data(), len);

    PHYSFS_close(file);

    return source;
}

GLint CreateShader(GLint type, const std::vector<std::string>& sources)
{
    std::vector<const GLchar*> parts;

    for (const auto& source : sources)
    {
        parts.push_back(source.c_str());
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, parts.size(), parts.data(), nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
    {
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        auto message = std::make_unique<GLchar[]>(len + 1);
        glGetShaderInfoLog(shader, len + 1, nullptr, message.get());

        GetLogger()->Error("Shader compilation error occurred!\n%%", message.get());
        lastShaderError = std::string("Shader compilation error occurred!\n\n") + std::string(message.get());

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLint LoadShader(GLint type, const char* filename)
{
    PHYSFS_file *file = PHYSFS_openRead(filename);
    if (file == nullptr)
    {
        GetLogger()->Error("Cannot read shader source file");
        GetLogger()->Error("Missing file '%%'", filename);
        return 0;
    }

    size_t len = PHYSFS_fileLength(file);

    std::vector<GLchar> source(len + 1);

    GLchar *sources[] = { source.data() };
    size_t length = PHYSFS_readBytes(file, source.data(), len);
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

        auto message = std::make_unique<GLchar[]>(len + 1);
        glGetShaderInfoLog(shader, len + 1, nullptr, message.get());

        GetLogger()->Error("Shader compilation error occurred!\n%%", message.get());
        lastShaderError = std::string("Shader compilation error occurred!\n\n") + std::string(message.get());

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLint LinkProgram(int count, const GLint* shaders)
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

        auto message = std::make_unique<GLchar[]>(len + 1);
        glGetProgramInfoLog(program, len + 1, nullptr, message.get());

        GetLogger()->Error("Shader program linking error occurred!\n%%", message.get());
        lastShaderError = std::string("Shader program linking error occurred!\n\n") + std::string(message.get());

        glDeleteProgram(program);

        return 0;
    }

    return program;
}

GLint LinkProgram(const std::vector<GLint>& shaders)
{
    GLint program = glCreateProgram();

    for (auto shader : shaders)
        glAttachShader(program, shader);

    glLinkProgram(program);

    for (auto shader : shaders)
        glDetachShader(program, shader);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

        std::string message(len, ' ');
        glGetProgramInfoLog(program, len + 1, nullptr, message.data());

        GetLogger()->Error("Shader program linking error occurred!\n%%", message);
        lastShaderError = std::string("Shader program linking error occurred!\n\n") + std::string(message.c_str());

        glDeleteProgram(program);

        return 0;
    }

    return program;
}


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

std::unique_ptr<CFrameBufferPixels> GetGLFrameBufferPixels(const glm::ivec2& size)
{
    auto pixels = std::make_unique<CGLFrameBufferPixels>(4 * size.x * size.y);

    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels->GetPixelsData());

    GLuint* p = static_cast<GLuint*> ( pixels->GetPixelsData() );

    for (int i = 0; i < size.x * size.y; ++i)
        p[i] |= 0xFF000000;

    return pixels;
}

PreparedTextureData PrepareTextureData(ImageData* imageData, TextureFormat format)
{
    PreparedTextureData texData;

    bool convert = false;

    texData.sourceFormat = 0;

    if (format == TextureFormat::RGB)
    {
        texData.sourceFormat = GL_RGB;
        texData.alpha = false;
    }
    else if (format == TextureFormat::BGR)
    {
        texData.sourceFormat = GL_BGR;
        texData.alpha = false;
    }
    else if (format == TextureFormat::RGBA)
    {
        texData.sourceFormat = GL_RGBA;
        texData.alpha = true;
    }
    else if (format == TextureFormat::BGRA)
    {
        texData.sourceFormat = GL_BGRA;
        texData.alpha = true;
    }
    else if (format == TextureFormat::AUTO)
    {
        if (imageData->surface->format->BytesPerPixel == 4)
        {
            if ((imageData->surface->format->Amask == 0xFF000000) &&
                (imageData->surface->format->Rmask == 0x00FF0000) &&
                (imageData->surface->format->Gmask == 0x0000FF00) &&
                (imageData->surface->format->Bmask == 0x000000FF))
            {
                texData.sourceFormat = GL_BGRA;
                texData.alpha = true;
            }
            else if ((imageData->surface->format->Amask == 0xFF000000) &&
                     (imageData->surface->format->Bmask == 0x00FF0000) &&
                     (imageData->surface->format->Gmask == 0x0000FF00) &&
                     (imageData->surface->format->Rmask == 0x000000FF))
            {
                texData.sourceFormat = GL_RGBA;
                texData.alpha = true;
            }
            else
            {
                texData.sourceFormat = GL_RGBA;
                convert = true;
            }
        }
        else if (imageData->surface->format->BytesPerPixel == 3)
        {
            if ((imageData->surface->format->Rmask == 0xFF0000) &&
                (imageData->surface->format->Gmask == 0x00FF00) &&
                (imageData->surface->format->Bmask == 0x0000FF))
            {
                texData.sourceFormat = GL_BGR;
                texData.alpha = false;
            }
            else if ((imageData->surface->format->Bmask == 0xFF0000) &&
                     (imageData->surface->format->Gmask == 0x00FF00) &&
                     (imageData->surface->format->Rmask == 0x0000FF))
            {
                texData.sourceFormat = GL_RGB;
                texData.alpha = false;
            }
            else
            {
                texData.sourceFormat = GL_RGBA;
                convert = true;
            }
        }
        else
        {
            GetLogger()->Error("Unknown data surface format");
            assert(false);
        }
    }
    else
        assert(false);

    texData.actualSurface = imageData->surface;
    texData.convertedSurface = nullptr;

    if (convert)
    {
        SDL_PixelFormat format = {};
        format.BytesPerPixel = 4;
        format.BitsPerPixel = 32;
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
        texData.convertedSurface = SDL_ConvertSurface(imageData->surface, &format, SDL_SWSURFACE);
        if (texData.convertedSurface != nullptr)
            texData.actualSurface = texData.convertedSurface;
    }

    return texData;
}

} // namespace Gfx
