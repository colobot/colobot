#version 320 es
precision highp float;

uniform mat4 uni_ProjectionMatrix;
uniform mat4 uni_ViewMatrix;
uniform mat4 uni_ModelMatrix;
uniform mat4 uni_ShadowMatrix;
uniform mat4 uni_NormalMatrix;
uniform vec3 uni_CameraPosition;

layout(location = 0) in vec4 in_VertexCoord;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord0;
layout(location = 4) in vec2 in_TexCoord1;

out VertexData
{
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec3 Normal;
    vec4 ShadowCoord;
    vec4 LightColor;
    float Distance;
    vec3 CameraDirection;
} data;

void main()
{
    vec4 position = uni_ModelMatrix * in_VertexCoord;
    vec4 eyeSpace = uni_ViewMatrix * position;
    gl_Position = uni_ProjectionMatrix * eyeSpace;
    vec4 shadowCoord = uni_ShadowMatrix * position;

    data.Color = in_Color;
    data.TexCoord0 = in_TexCoord0;
    data.TexCoord1 = in_TexCoord1;
    data.Normal = normalize((uni_NormalMatrix * vec4(in_Normal, 0.0f)).xyz);
    data.ShadowCoord = vec4(shadowCoord.xyz / shadowCoord.w, 1.0f);
    data.Distance = abs(eyeSpace.z);
    data.CameraDirection = uni_CameraPosition - position.xyz;
}
