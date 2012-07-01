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

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <SDL/SDL.h>

#include <assert.h>

namespace Gfx {

struct GLDevicePrivate
{
    void (APIENTRY* glMultiTexCoord1fARB)(GLenum target, GLfloat s);
    void (APIENTRY* glMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t);
    void (APIENTRY* glMultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void (APIENTRY* glMultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void (APIENTRY* glActiveTextureARB)(GLenum texture);
    void (APIENTRY* glClientActiveTextureARB)(GLenum texture);

    GLDevicePrivate()
    {
        glMultiTexCoord1fARB = NULL;
        glMultiTexCoord2fARB = NULL;
        glMultiTexCoord3fARB = NULL;
        glMultiTexCoord4fARB = NULL;
        glActiveTextureARB   = NULL;
        glClientActiveTextureARB = NULL;
    }
};

}; // namespace Gfx


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




Gfx::CGLDevice::CGLDevice()
{
    m_private = new Gfx::GLDevicePrivate();
    m_wasInit = false;
}


Gfx::CGLDevice::~CGLDevice()
{
    delete m_private;
    m_private = NULL;
}

bool Gfx::CGLDevice::GetWasInit()
{
    return m_wasInit;
}

std::string Gfx::CGLDevice::GetError()
{
    return m_error;
}

bool Gfx::CGLDevice::Create()
{
    m_wasInit = true;

    // TODO: move to functions?
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    std::string extensions = std::string( (char*) glGetString(GL_EXTENSIONS));

    if (extensions.find("GL_ARB_multitexture") == std::string::npos)
    {
        m_error = "Required extension GL_ARB_multitexture not supported";
        return false;
    }

    if (extensions.find("GL_EXT_texture_env_combine") == std::string::npos)
    {
        m_error = "Required extension GL_EXT_texture_env_combine not supported";
        return false;
    }

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextures);
    m_textures = std::vector<Gfx::Texture*>(maxTextures, NULL);

    m_lights = std::vector<Gfx::Light>(GL_MAX_LIGHTS, Gfx::Light());
    m_lightsEnabled = std::vector<bool>(GL_MAX_LIGHTS, false);

    m_private->glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord1fARB");
    m_private->glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
    m_private->glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord3fARB");
    m_private->glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord4fARB");
    m_private->glActiveTextureARB   = (PFNGLACTIVETEXTUREARBPROC)   SDL_GL_GetProcAddress("glActiveTextureARB");
    m_private->glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTextureARB");

    return true;
}

void Gfx::CGLDevice::Destroy()
{
    m_private->glMultiTexCoord1fARB = NULL;
    m_private->glMultiTexCoord2fARB = NULL;
    m_private->glMultiTexCoord3fARB = NULL;
    m_private->glMultiTexCoord4fARB = NULL;
    m_private->glActiveTextureARB   = NULL;
    m_private->glClientActiveTextureARB = NULL;

    // Delete the remaining textures
    std::set<Gfx::Texture*>::iterator it;
    for (it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        delete *it;
    m_allTextures.clear();

    m_wasInit = false;
}

void Gfx::CGLDevice::BeginScene()
{
    Clear();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMat.Array());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m_modelviewMat.Array());
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
    if (type == Gfx::TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        m_modelviewMat = Math::MultiplyMatrices(m_worldMat, m_viewMat);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m_modelviewMat.Array());
    }
    else if (type == Gfx::TRANSFORM_VIEW)
    {
        m_viewMat = matrix;
        m_modelviewMat = Math::MultiplyMatrices(m_worldMat, m_viewMat);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m_modelviewMat.Array());
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
    if (type == Gfx::TRANSFORM_WORLD)
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
    if (type == Gfx::TRANSFORM_WORLD)
    {
        m_worldMat = Math::MultiplyMatrices(m_worldMat, matrix);
        m_modelviewMat = Math::MultiplyMatrices(m_worldMat, m_viewMat);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m_modelviewMat.Array());
    }
    else if (type == Gfx::TRANSFORM_VIEW)
    {
        m_viewMat = Math::MultiplyMatrices(m_viewMat, matrix);
        m_modelviewMat = Math::MultiplyMatrices(m_worldMat, m_viewMat);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m_modelviewMat.Array());
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

void Gfx::CGLDevice::SetMaterial(Gfx::Material &material)
{
    m_material = material;

    glMaterialfv(GL_AMBIENT,  GL_FRONT_AND_BACK, m_material.ambient.Array());
    glMaterialfv(GL_DIFFUSE,  GL_FRONT_AND_BACK, m_material.diffuse.Array());
    glMaterialfv(GL_SPECULAR, GL_FRONT_AND_BACK, m_material.specular.Array());
}

const Gfx::Material& Gfx::CGLDevice::GetMaterial()
{
    return m_material;
}

int Gfx::CGLDevice::GetMaxLightCount()
{
    return m_lights.size();
}

void Gfx::CGLDevice::SetLight(int index, Gfx::Light &light)
{
    assert(index >= 0);
    assert(index < (int)m_lights.size());

    m_lights[index] = light;

    // Indexing from GL_LIGHT0 should always work
    glLightfv(GL_LIGHT0 + index, GL_AMBIENT,  light.ambient.Array());
    glLightfv(GL_LIGHT0 + index, GL_DIFFUSE,  light.diffuse.Array());
    glLightfv(GL_LIGHT0 + index, GL_SPECULAR, light.specular.Array());

    GLfloat position[4] = { light.position.x, light.position.y, light.position.z, 0.0f };
    if (light.type == LIGHT_DIRECTIONAL)
        position[3] = 0.0f;
    else
        position[3] = 1.0f;
    glLightfv(GL_LIGHT0 + index, GL_POSITION, position);

    GLfloat direction[4] = { light.direction.x, light.direction.y, light.direction.z, 0.0f };
    glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction);

    glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, light.range);

    // TODO: falloff?, phi?, theta?

    glLightf(GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION,  light.attenuation0);
    glLightf(GL_LIGHT0 + index, GL_LINEAR_ATTENUATION,    light.attenuation1);
    glLightf(GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, light.attenuation2);
}

const Gfx::Light& Gfx::CGLDevice::GetLight(int index)
{
    assert(index >= 0);
    assert(index < (int)m_lights.size());

    return m_lights[index];
}

void Gfx::CGLDevice::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < (int)m_lightsEnabled.size());

    m_lightsEnabled[index] = enabled;

    glEnable(GL_LIGHT0 + index);
}

bool Gfx::CGLDevice::GetLightEnabled(int index)
{
    assert(index >= 0);
    assert(index < (int)m_lights.size());

    return m_lightsEnabled[index];
}

int Gfx::CGLDevice::GetMaxTextureCount()
{
    return m_textures.size();
}

Gfx::Texture* Gfx::CGLDevice::GetTexture(int index)
{
    assert(index >= 0);
    assert(index < (int)m_textures.size());

    return m_textures[index];
}

void Gfx::CGLDevice::SetTexture(int index, Gfx::Texture *texture)
{
    assert(index >= 0);
    assert(index < (int)m_textures.size());

    m_textures[index] = texture;

    // TODO
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType type, Vertex *vertices, int vertexCount)
{
    if (type == Gfx::PRIMITIVE_LINES)
        glBegin(GL_LINES);
    else if (type == Gfx::PRIMITIVE_TRIANGLES)
        glBegin(GL_TRIANGLES);
    else if (type == Gfx::PRIMITIVE_TRIANGLE_STRIP)
        glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < vertexCount; ++i)
    {
        glNormal3fv((GLfloat*)vertices[i].normal.Array());
        glTexCoord2fv((GLfloat*)vertices[i].texCoord.Array());
        glVertex3fv((GLfloat*)vertices[i].coord.Array());
    }

    glEnd();
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType type, Gfx::VertexCol *vertices, int vertexCount)
{
    if (type == Gfx::PRIMITIVE_LINES)
        glBegin(GL_LINES);
    else if (type == Gfx::PRIMITIVE_TRIANGLES)
        glBegin(GL_TRIANGLES);
    else if (type == Gfx::PRIMITIVE_TRIANGLE_STRIP)
        glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < vertexCount; ++i)
    {
        // TODO: specular?
        glColor4fv((GLfloat*)vertices[i].color.Array());
        glTexCoord2fv((GLfloat*)vertices[i].texCoord.Array());
        glVertex3fv((GLfloat*)vertices[i].coord.Array());
    }

    glEnd();
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType type, VertexTex2 *vertices, int vertexCount)
{
    if (type == Gfx::PRIMITIVE_LINES)
        glBegin(GL_LINES);
    else if (type == Gfx::PRIMITIVE_TRIANGLES)
        glBegin(GL_TRIANGLES);
    else if (type == Gfx::PRIMITIVE_TRIANGLE_STRIP)
        glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < vertexCount; ++i)
    {
        glNormal3fv((GLfloat*) vertices[i].normal.Array());
        // TODO glMultiTexCoord2fARB(GL_TEXTURE0_ARB, vertices[i].texCoord.x, vertices[i].texCoord.y);
        // TODO glMultiTexCoord2fARB(GL_TEXTURE1_ARB, vertices[i].texCoord2.x, vertices[i].texCoord2.y);
        glVertex3fv((GLfloat*) vertices[i].coord.Array());
    }

    glEnd();
}

void Gfx::CGLDevice::SetRenderState(Gfx::RenderState state, bool enabled)
{
    if (state == RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == RENDER_STATE_TEXTURING)
    {
        if (enabled)
        {
            glEnable(GL_TEXTURE_2D);
            // TODO multitexture
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
        }
        return;
    }

    GLenum flag = 0;

    switch (state)
    {
        case Gfx::RENDER_STATE_LIGHTING:    flag = GL_DEPTH_WRITEMASK; break;
        case Gfx::RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case Gfx::RENDER_STATE_FOG:         flag = GL_FOG; break;
        case Gfx::RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case Gfx::RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
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
    GLenum flag = 0;

    switch (state)
    {
        case Gfx::RENDER_STATE_DEPTH_WRITE: flag = GL_DEPTH_WRITEMASK; break;
        case Gfx::RENDER_STATE_TEXTURING:   flag = GL_TEXTURE_2D; break;
        case Gfx::RENDER_STATE_LIGHTING:    flag = GL_DEPTH_WRITEMASK; break;
        case Gfx::RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case Gfx::RENDER_STATE_FOG:         flag = GL_FOG; break;
        case Gfx::RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case Gfx::RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
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
    GLenum flag = 0;
    glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&flag);
    return TranslateGLCompFunc(flag);
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
    GLenum flag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, (GLint*)&flag);
    func = TranslateGLCompFunc(flag);

    glGetFloatv(GL_ALPHA_TEST_REF, (GLfloat*) &refValue);
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
    GLenum srcFlag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, (GLint*)&srcFlag);
    srcBlend = TranslateGLBlendFunc(srcFlag);

    GLenum dstFlag = 0;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, (GLint*)&dstFlag);
    dstBlend = TranslateGLBlendFunc(dstFlag);
}

void Gfx::CGLDevice::SetClearColor(Gfx::Color color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

Gfx::Color Gfx::CGLDevice::GetClearColor()
{
    float color[4] = { 0.0f };
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    return Gfx::Color(color[0], color[1], color[2], color[3]);
}

void Gfx::CGLDevice::SetGlobalAmbient(Gfx::Color color)
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

Gfx::Color Gfx::CGLDevice::GetGlobalAmbient()
{
    float color[4] = { 0.0f };
    glGetFloatv(GL_LIGHT_MODEL_AMBIENT, color);
    return Gfx::Color(color[0], color[1], color[2], color[3]);
}

void Gfx::CGLDevice::SetFogParams(Gfx::FogMode mode, Gfx::Color color, float start, float end, float density)
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
    GLenum flag = 0;
    glGetIntegerv(GL_FOG_MODE, (GLint*)&flag);
    if      (flag == GL_LINEAR) mode = Gfx::FOG_LINEAR;
    else if (flag == GL_EXP)    mode = Gfx::FOG_EXP;
    else if (flag == GL_EXP2)   mode = Gfx::FOG_EXP2;
    else assert(false);

    glGetFloatv(GL_FOG_START,   (GLfloat*)&start);
    glGetFloatv(GL_FOG_END,     (GLfloat*)&end);
    glGetFloatv(GL_FOG_DENSITY, (GLfloat*)&density);
}

void Gfx::CGLDevice::SetCullMode(Gfx::CullMode mode)
{
    if      (mode == Gfx::CULL_CW)  glCullFace(GL_CW);
    else if (mode == Gfx::CULL_CCW) glCullFace(GL_CCW);
    else assert(false);
}

Gfx::CullMode Gfx::CGLDevice::GetCullMode()
{
    GLenum flag = 0;
    glGetIntegerv(GL_CULL_FACE, (GLint*)&flag);
    if      (flag == GL_CW)  return Gfx::CULL_CW;
    else if (flag == GL_CCW) return Gfx::CULL_CCW;
    else assert(false);
    return Gfx::CULL_CW;
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
    GLenum flag = 0;
    glGetIntegerv(GL_POLYGON_MODE, (GLint*)&flag);
    if      (flag == GL_POINT) return Gfx::FILL_POINT;
    else if (flag == GL_LINE)  return Gfx::FILL_LINES;
    else if (flag == GL_FILL)  return Gfx::FILL_FILL;
    else assert(false);
    return Gfx::FILL_POINT;
}
