#include "graphics/opengl/gl33renderers.h"

#include "graphics/opengl/glutil.h"

#include "graphics/core/vertex.h"

#include "common/logger.h"

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

namespace Gfx
{

CGL33UIRenderer::CGL33UIRenderer()
{
    GLint shaders[2];

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
        GetLogger()->Error("Cound not link shader program for normal rendering\n");
        return;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    glUseProgram(m_program);

    m_projectionMatrix = glGetUniformLocation(m_program, "uni_ProjectionMatrix");
    auto matrix = glm::ortho(0.0f, +1.0f, 0.0f, +1.0f);
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    // Set texture unit to 8th
    auto texture = glGetUniformLocation(m_program, "uni_Texture");
    glUniform1i(texture, 8);

    // Generic buffer
    glGenBuffers(1, &m_bufferVBO);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_bufferVBO);
    glBufferData(GL_COPY_WRITE_BUFFER, m_bufferCapacity, nullptr, GL_STREAM_DRAW);

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
    Flush();

    glUseProgram(m_program);

    auto matrix = glm::ortho(left, right, bottom, top);

    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    glUseProgram(0);
}

void CGL33UIRenderer::SetTexture(const Texture& texture)
{
    if (m_currentTexture == texture.id) return;

    Flush();

    glActiveTexture(GL_TEXTURE8);

    m_currentTexture = texture.id;

    if (m_currentTexture == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, m_currentTexture);
}

void CGL33UIRenderer::DrawPrimitive(PrimitiveType type, int count, const Vertex2D* vertices)
{
    // If too much data would be buffered, flush
    size_t totalSize = (m_buffer.size() + count) * sizeof(Vertex2D);

    if (totalSize > m_bufferCapacity)
        Flush();

    // Buffer data
    GLint first = m_buffer.size();

    m_buffer.insert(m_buffer.end(), vertices, vertices + count);
    m_types.push_back(TranslateGfxPrimitive(type));
    m_firsts.push_back(first);
    m_counts.push_back(count);
}

void CGL33UIRenderer::Flush()
{
    if (m_types.empty()) return;

    glUseProgram(m_program);

    // Increase buffer size if necessary
    size_t size = m_buffer.size() * sizeof(Vertex2D);

    if (m_bufferCapacity < size)
        m_bufferCapacity = size;

    // Send new vertices to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferVBO);
    glBufferData(GL_ARRAY_BUFFER, m_bufferCapacity, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_buffer.data());

    // Respecify vertex attributes
    glBindVertexArray(m_bufferVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
        reinterpret_cast<void*>(offsetof(Vertex2D, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
        reinterpret_cast<void*>(offsetof(Vertex2D, uv)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D),
        reinterpret_cast<void*>(offsetof(Vertex2D, color)));

    // Draw primitives by grouping by type
    for (size_t i = 0; i < m_types.size(); i++)
    {
        //glDrawArrays(m_types[i], m_firsts[i], m_counts[i]);

        //*
        size_t count = 1;

        for (; i + count < m_types.size(); count++)
        {
            if (m_types[i] != m_types[i + count])
                break;
        }

        glMultiDrawArrays(m_types[i], &m_firsts[i], &m_counts[i], count);

        i += count;
        // */
    }

    // Clear buffers
    m_buffer.clear();
    m_types.clear();
    m_firsts.clear();
    m_counts.clear();

    glUseProgram(0);
}

} // namespace Gfx
