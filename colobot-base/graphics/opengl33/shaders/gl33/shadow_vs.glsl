/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

// VERTEX SHADER - SHADOW MODE
#version 330 core

uniform mat4 uni_ProjectionMatrix;
uniform mat4 uni_ViewMatrix;
uniform mat4 uni_ModelMatrix;

layout(location = 0) in vec4 in_VertexCoord;
layout(location = 3) in vec2 in_TexCoord0;

out VertexData
{
    vec2 TexCoord;
} data;

void main()
{
    gl_Position = uni_ProjectionMatrix * uni_ViewMatrix * uni_ModelMatrix * in_VertexCoord;

    data.TexCoord = in_TexCoord0;
}
