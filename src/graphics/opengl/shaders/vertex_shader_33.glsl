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

// VERTEX SHADER
#version 330

uniform mat4 uni_ProjectionMatrix;
uniform mat4 uni_ViewMatrix;
uniform mat4 uni_ModelMatrix;
uniform mat4 uni_ShadowMatrix;
uniform mat4 uni_NormalMatrix;

layout(location = 0) in vec4 in_VertexCoord;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord0;
layout(location = 4) in vec2 in_TexCoord1;

out VertexData
{
    vec3 NormalSmooth;
    flat vec3 NormalFlat;
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec4 ShadowCoord;
    vec4 Position;
    float Distance;
} data;

void main()
{
    vec4 position = uni_ModelMatrix * in_VertexCoord;
    vec4 eyeSpace = uni_ViewMatrix * position;
    gl_Position = uni_ProjectionMatrix * eyeSpace;

    vec3 normal = normalize((uni_NormalMatrix * vec4(in_Normal, 0.0f)).xyz);
    
    data.Color = in_Color;
    data.NormalSmooth = normal;
    data.NormalFlat = normal;
    data.TexCoord0 = in_TexCoord0;
    data.TexCoord1 = in_TexCoord1;
    data.ShadowCoord = uni_ShadowMatrix * position;
    data.Position = position;
    data.Distance = abs(eyeSpace.z);
}
