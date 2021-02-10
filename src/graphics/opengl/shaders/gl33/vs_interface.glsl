#version 320 es
precision highp float;

uniform mat4 uni_ProjectionMatrix;
uniform mat4 uni_ViewMatrix;
uniform mat4 uni_ModelMatrix;

layout(location = 0) in vec4 in_VertexCoord;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord0;
layout(location = 4) in vec2 in_TexCoord1;

out VertexData
{
    vec4 Color;
    vec2 TexCoord;
} data;

void main()
{
    gl_Position = uni_ProjectionMatrix * uni_ViewMatrix * uni_ModelMatrix * in_VertexCoord;

    data.Color = in_Color;
    data.TexCoord = in_TexCoord0;
}
