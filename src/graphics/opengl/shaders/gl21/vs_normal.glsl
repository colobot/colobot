/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
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
// VERTEX SHADER - NORMAL MODE
#version 120

uniform mat4 uni_ProjectionMatrix;
uniform mat4 uni_ViewMatrix;
uniform mat4 uni_ModelMatrix;
uniform mat4 uni_ShadowMatrix;
uniform mat4 uni_NormalMatrix;

varying float pass_Distance;
varying vec4 pass_Color;
varying vec3 pass_Normal;
varying vec2 pass_TexCoord0;
varying vec2 pass_TexCoord1;
varying vec3 pass_TexCoord2;

void main()
{
    vec4 position = uni_ModelMatrix * gl_Vertex;
    vec4 eyeSpace = uni_ViewMatrix * position;
    vec4 shadowCoord = uni_ShadowMatrix * position;

    gl_Position = uni_ProjectionMatrix * eyeSpace;

    pass_Color = gl_Color;
    pass_Normal = normalize((uni_NormalMatrix * vec4(gl_Normal, 0.0f)).xyz);
    pass_Distance = abs(eyeSpace.z / eyeSpace.w);
    pass_TexCoord0 = gl_MultiTexCoord0.st;
    pass_TexCoord1 = gl_MultiTexCoord1.st;
    pass_TexCoord2 = shadowCoord.xyz / shadowCoord.w;
}
