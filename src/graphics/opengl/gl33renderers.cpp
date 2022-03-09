#include "graphics/opengl/gl33renderers.h"

#include "graphics/opengl/gl33device.h"
#include "graphics/opengl/glutil.h"

#include "graphics/core/material.h"
#include "graphics/core/transparency.h"
#include "graphics/core/vertex.h"

#include "common/logger.h"

#include <GL/glew.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

namespace Gfx
{

CGL33UIRenderer::CGL33UIRenderer(CGL33Device* device)
    : m_device(device)
{
    GetLogger()->Info("Creating CGL33UIRenderer\n");

    GLint shaders[2] = {};

    shaders[0] = LoadShader(GL_VERTEX_SHADER, "shaders/gl33/ui_vs.glsl");
    if (shaders[0] == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'ui_vs.glsl'\n");
        return;
    }

    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, "shaders/gl33/ui_fs.glsl");
    if (shaders[1] == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'ui_fs.glsl'\n");
        return;
    }

    m_program = LinkProgram(2, shaders);
    if (m_program == 0)
    {
        GetLogger()->Error("Cound not link shader program for interface renderer\n");
        return;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    glUseProgram(m_program);

    // Create uniform buffer
    glGenBuffers(1, &m_uniformBuffer);

    m_uniforms.projectionMatrix = glm::ortho(0.0f, +1.0f, 0.0f, +1.0f);
    m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    m_uniformsDirty = true;

    UpdateUniforms();

    // Bind uniform block to uniform buffer binding
    GLuint blockIndex = glGetUniformBlockIndex(m_program, "Uniforms");

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
    glUniformBlockBinding(m_program, blockIndex, 0);

    // Set texture unit to 8th
    auto texture = glGetUniformLocation(m_program, "uni_Texture");
    glUniform1i(texture, 8);

    // Generic buffer
    glGenBuffers(1, &m_bufferVBO);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_bufferVBO);
    glBufferData(GL_COPY_WRITE_BUFFER, m_bufferCapacity * sizeof(Vertex2D), nullptr, GL_STREAM_DRAW);
    m_bufferOffset = m_bufferCapacity;

    glGenVertexArrays(1, &m_bufferVAO);
    glBindVertexArray(m_bufferVAO);

    // White texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);

    glUseProgram(0);

    GetLogger()->Info("CGL33UIRenderer created successfully\n");
}

CGL33UIRenderer::~CGL33UIRenderer()
{
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_whiteTexture);

    glDeleteBuffers(1, &m_bufferVBO);
    glDeleteVertexArrays(1, &m_bufferVAO);
}

void CGL33UIRenderer::SetProjection(float left, float right, float bottom, float top)
{
    m_uniforms.projectionMatrix = glm::ortho(left, right, bottom, top);
    m_uniformsDirty = true;
}

void CGL33UIRenderer::SetTexture(const Texture& texture)
{
    if (m_currentTexture == texture.id) return;

    glActiveTexture(GL_TEXTURE8);

    m_currentTexture = texture.id;

    if (m_currentTexture == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, m_currentTexture);
}

void CGL33UIRenderer::SetColor(const glm::vec4& color)
{
    m_uniforms.color = color;
    m_uniformsDirty = true;
}

void CGL33UIRenderer::SetTransparency(TransparencyMode mode)
{
    m_device->SetTransparency(mode);
}

void CGL33UIRenderer::DrawPrimitive(PrimitiveType type, int count, const Vertex2D* vertices)
{
    auto ptr = BeginPrimitive(type, count);

    std::copy_n(vertices, count, ptr);

    EndPrimitive();
}

Vertex2D* CGL33UIRenderer::BeginPrimitive(PrimitiveType type, int count)
{
    return BeginPrimitives(type, 1, &count);
}

Vertex2D* CGL33UIRenderer::BeginPrimitives(PrimitiveType type, int drawCount, const int* counts)
{
    glBindVertexArray(m_bufferVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferVBO);

    m_currentCount = 0;

    for (size_t i = 0; i < drawCount; i++)
    {
        m_currentCount += counts[i];
    }

    GLuint total = m_bufferOffset + m_currentCount;

    // Buffer full, orphan
    if (total >= m_bufferCapacity)
    {
        glBufferData(GL_ARRAY_BUFFER, m_bufferCapacity * sizeof(Vertex2D), nullptr, GL_STREAM_DRAW);

        m_bufferOffset = 0;

        // Respecify vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
            reinterpret_cast<void*>(offsetof(Vertex2D, position)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
            reinterpret_cast<void*>(offsetof(Vertex2D, uv)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D),
            reinterpret_cast<void*>(offsetof(Vertex2D, color)));
    }

    m_first.resize(drawCount);
    m_count.resize(drawCount);

    GLsizei currentOffset = m_bufferOffset;

    for (size_t i = 0; i < drawCount; i++)
    {
        m_first[i] = currentOffset;
        m_count[i] = counts[i];

        currentOffset += counts[i];
    }

    auto ptr = glMapBufferRange(GL_ARRAY_BUFFER,
        m_bufferOffset * sizeof(Vertex2D),
        m_currentCount * sizeof(Vertex2D),
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    m_mapped = true;
    m_type = type;
    m_drawCount = drawCount;

    // Mapping failed, use backup buffer
    if (ptr == nullptr)
    {
        m_backup = true;
        m_buffer.resize(m_currentCount);

        return m_buffer.data();
    }
    else
    {
        return reinterpret_cast<Vertex2D*>(ptr);
    }
}

bool CGL33UIRenderer::EndPrimitive()
{
    if (!m_mapped) return false;

    if (m_backup)
    {
        glBufferSubData(GL_ARRAY_BUFFER,
            m_bufferOffset * sizeof(Vertex2D),
            m_currentCount * sizeof(Vertex2D),
            m_buffer.data());
    }
    else
    {
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glUseProgram(m_program);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);

    UpdateUniforms();
    
    m_device->SetDepthTest(false);
    m_device->SetCullFace(CullFace::NONE);

    if (m_drawCount == 1)
        glDrawArrays(TranslateGfxPrimitive(m_type), m_first.front(), m_count.front());
    else
        glMultiDrawArrays(TranslateGfxPrimitive(m_type), m_first.data(), m_count.data(), m_drawCount);

    m_bufferOffset += m_currentCount;

    m_mapped = false;
    m_backup = false;

    return true;
}

void CGL33UIRenderer::UpdateUniforms()
{
    if (!m_uniformsDirty) return;

    glBindBuffer(GL_COPY_WRITE_BUFFER, m_uniformBuffer);
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(Uniforms), nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_COPY_WRITE_BUFFER, 0, sizeof(Uniforms), &m_uniforms);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}


CGL33TerrainRenderer::CGL33TerrainRenderer(CGL33Device* device)
    : m_device(device)
{
    GetLogger()->Info("Creating CGL33TerrainRenderer\n");

    std::string preamble = LoadSource("shaders/gl33/preamble.glsl");
    std::string shadowSource = LoadSource("shaders/gl33/shadow.glsl");
    std::string vsSource = LoadSource("shaders/gl33/terrain_vs.glsl");
    std::string fsSource = LoadSource("shaders/gl33/terrain_fs.glsl");

    GLint vsShader = CreateShader(GL_VERTEX_SHADER, { preamble, shadowSource, vsSource });
    if (vsShader == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'terrain_vs.glsl'\n");
        return;
    }

    GLint fsShader = CreateShader(GL_FRAGMENT_SHADER, { preamble, shadowSource, fsSource });
    if (fsShader == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'terrain_vs.glsl'\n");
        return;
    }

    m_program = LinkProgram({ vsShader, fsShader });
    if (m_program == 0)
    {
        GetLogger()->Error("Cound not link shader program for terrain renderer\n");
        return;
    }

    glDeleteShader(vsShader);
    glDeleteShader(fsShader);

    glUseProgram(m_program);

    // Setup uniforms
    glm::mat4 identity(1.0f);

    m_projectionMatrix = glGetUniformLocation(m_program, "uni_ProjectionMatrix");
    m_viewMatrix = glGetUniformLocation(m_program, "uni_ViewMatrix");
    m_cameraMatrix = glGetUniformLocation(m_program, "uni_CameraMatrix");
    m_shadowMatrix = glGetUniformLocation(m_program, "uni_ShadowMatrix");
    m_modelMatrix = glGetUniformLocation(m_program, "uni_ModelMatrix");
    m_normalMatrix = glGetUniformLocation(m_program, "uni_NormalMatrix");
    m_lightPosition = glGetUniformLocation(m_program, "uni_LightPosition");
    m_lightIntensity = glGetUniformLocation(m_program, "uni_LightIntensity");
    m_lightColor = glGetUniformLocation(m_program, "uni_LightColor");
    m_fogRange = glGetUniformLocation(m_program, "uni_FogRange");
    m_fogColor = glGetUniformLocation(m_program, "uni_FogColor");

    m_shadowRegions = glGetUniformLocation(m_program, "uni_ShadowRegions");

    GLchar name[64];

    for (int i = 0; i < 4; i++)
    {
        sprintf(name, "uni_ShadowParam[%d].transform", i);
        m_shadows[i].transform = glGetUniformLocation(m_program, name);

        sprintf(name, "uni_ShadowParam[%d].uv_offset", i);
        m_shadows[i].offset = glGetUniformLocation(m_program, name);

        sprintf(name, "uni_ShadowParam[%d].uv_scale", i);
        m_shadows[i].scale = glGetUniformLocation(m_program, name);
    }

    // Set texture units to 10th and 11th
    auto texture = glGetUniformLocation(m_program, "uni_PrimaryTexture");
    glUniform1i(texture, 10);

    texture = glGetUniformLocation(m_program, "uni_SecondaryTexture");
    glUniform1i(texture, 11);

    texture = glGetUniformLocation(m_program, "uni_ShadowMap");
    glUniform1i(texture, 12);

    // White texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);

    glUseProgram(0);

    GetLogger()->Info("CGL33TerrainRenderer created successfully\n");
}

CGL33TerrainRenderer::~CGL33TerrainRenderer()
{
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_whiteTexture);
}

void CGL33TerrainRenderer::Begin()
{
    glUseProgram(m_program);

    m_device->SetDepthTest(true);
    m_device->SetDepthMask(true);
    m_device->SetCullFace(CullFace::BACK);

    m_device->SetTransparency(TransparencyMode::NONE);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_primaryTexture = 0;
    m_secondaryTexture = 0;
    m_shadowMap = 0;
}

void CGL33TerrainRenderer::End()
{
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_primaryTexture = 0;
    m_secondaryTexture = 0;
    m_shadowMap = 0;
}

void CGL33TerrainRenderer::SetProjectionMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33TerrainRenderer::SetViewMatrix(const glm::mat4& matrix)
{
    glm::mat4 scale(1.0f);
    scale[2][2] = -1.0f;

    auto viewMatrix = scale * matrix;
    auto cameraMatrix = glm::inverse(viewMatrix);

    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, value_ptr(viewMatrix));
    glUniformMatrix4fv(m_cameraMatrix, 1, GL_FALSE, value_ptr(cameraMatrix));
}

void CGL33TerrainRenderer::SetModelMatrix(const glm::mat4& matrix)
{
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));

    glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, value_ptr(matrix));
    glUniformMatrix3fv(m_normalMatrix, 1, GL_FALSE, value_ptr(normalMatrix));
}

void CGL33TerrainRenderer::SetPrimaryTexture(const Texture& texture)
{
    if (m_primaryTexture == texture.id) return;

    m_primaryTexture = texture.id;

    glActiveTexture(GL_TEXTURE10);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33TerrainRenderer::SetSecondaryTexture(const Texture& texture)
{
    if (m_secondaryTexture == texture.id) return;

    m_secondaryTexture = texture.id;

    glActiveTexture(GL_TEXTURE11);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33TerrainRenderer::SetShadowMap(const Texture& texture)
{
    if (m_shadowMap == texture.id) return;

    m_shadowMap = texture.id;

    glActiveTexture(GL_TEXTURE12);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33TerrainRenderer::SetLight(const glm::vec4& position, const float& intensity, const glm::vec3& color)
{
    glUniform4fv(m_lightPosition, 1, glm::value_ptr(position));
    glUniform1f(m_lightIntensity, intensity);
    glUniform3fv(m_lightColor, 1, glm::value_ptr(color));
}

void CGL33TerrainRenderer::SetShadowParams(int count, const ShadowParam* params)
{
    glUniform1i(m_shadowRegions, count);

    for (int i = 0; i < count; i++)
    {
        glUniformMatrix4fv(m_shadows[i].transform, 1, GL_FALSE, glm::value_ptr(params[i].matrix));
        glUniform2fv(m_shadows[i].offset, 1, glm::value_ptr(params[i].uv_offset));
        glUniform2fv(m_shadows[i].scale, 1, glm::value_ptr(params[i].uv_scale));
    }
}

void CGL33TerrainRenderer::SetFog(float min, float max, const glm::vec3& color)
{
    glUniform2f(m_fogRange, min, max);
    glUniform3f(m_fogColor, color.r, color.g, color.b);
}

void CGL33TerrainRenderer::DrawObject(const glm::mat4& matrix, const CVertexBuffer* buffer)
{
    auto b = dynamic_cast<const CGL33VertexBuffer*>(buffer);

    if (b == nullptr) return;

    SetModelMatrix(matrix);
    glBindVertexArray(b->GetVAO());

    glDrawArrays(TranslateGfxPrimitive(b->GetType()), 0, b->Size());
}

CGL33ShadowRenderer::CGL33ShadowRenderer(CGL33Device* device)
    : m_device(device)
{
    GetLogger()->Info("Creating CGL33ShadowRenderer\n");

    GLint shaders[2] = {};

    shaders[0] = LoadShader(GL_VERTEX_SHADER, "shaders/gl33/shadow_vs.glsl");
    if (shaders[0] == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'shadow_vs.glsl'\n");
        return;
    }

    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, "shaders/gl33/shadow_fs.glsl");
    if (shaders[1] == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'shadow_fs.glsl'\n");
        return;
    }

    m_program = LinkProgram(2, shaders);
    if (m_program == 0)
    {
        GetLogger()->Error("Cound not link shader program for terrain renderer\n");
        return;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    glUseProgram(m_program);

    // Setup uniforms
    auto texture = glGetUniformLocation(m_program, "uni_Texture");
    glUniform1i(texture, 0);

    glm::mat4 identity(1.0f);

    m_projectionMatrix = glGetUniformLocation(m_program, "uni_ProjectionMatrix");
    m_viewMatrix = glGetUniformLocation(m_program, "uni_ViewMatrix");
    m_modelMatrix = glGetUniformLocation(m_program, "uni_ModelMatrix");
    m_alphaScissor = glGetUniformLocation(m_program, "uni_AlphaScissor");

    glUseProgram(0);

    glGenFramebuffers(1, &m_framebuffer);

    GetLogger()->Info("CGL33ShadowRenderer created successfully\n");
}

CGL33ShadowRenderer::~CGL33ShadowRenderer()
{
    glDeleteProgram(m_program);

    glDeleteFramebuffers(1, &m_framebuffer);
}

void CGL33ShadowRenderer::Begin()
{
    glViewport(0, 0, m_width, m_height);
    m_device->SetDepthMask(true);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_program);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 8.0f);

    m_device->SetColorMask(false, false, false, false);
    m_device->SetDepthTest(true);
    m_device->SetDepthMask(true);

    m_device->SetTransparency(TransparencyMode::NONE);
    m_device->SetCullFace(CullFace::NONE);
}

void CGL33ShadowRenderer::End()
{
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.0f, 0.0f);

    m_device->SetColorMask(true, true, true, true);
}

void CGL33ShadowRenderer::SetProjectionMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ShadowRenderer::SetViewMatrix(const glm::mat4& matrix)
{
    glm::mat4 scale(1.0f);
    scale[2][2] = -1.0f;

    auto viewMatrix = scale * matrix;

    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, value_ptr(viewMatrix));
}

void CGL33ShadowRenderer::SetModelMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ShadowRenderer::SetTexture(const Texture& texture)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ShadowRenderer::SetShadowMap(const Texture& texture)
{
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id, 0);

    m_width = texture.size.x;
    m_height = texture.size.y;

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        GetLogger()->Error("Framebuffer incomplete: %d\n", status);
    }
}

void CGL33ShadowRenderer::SetShadowRegion(const glm::vec2& offset, const glm::vec2& scale)
{
    int x = static_cast<int>(m_width * offset.x);
    int y = static_cast<int>(m_height * offset.y);
    int width = static_cast<int>(m_width * scale.x);
    int height = static_cast<int>(m_height * scale.y);

    glViewport(x, y, width, height);
}

void CGL33ShadowRenderer::DrawObject(const CVertexBuffer* buffer, bool transparent)
{
    auto b = dynamic_cast<const CGL33VertexBuffer*>(buffer);

    if (b == nullptr) return;

    glUniform1i(m_alphaScissor, transparent ? 1 : 0);

    glBindVertexArray(b->GetVAO());

    glDrawArrays(TranslateGfxPrimitive(b->GetType()), 0, b->Size());
}

} // namespace Gfx
