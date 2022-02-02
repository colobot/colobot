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

// FRAGMENT SHADER - NORMAL MODE
#version 330 core

uniform sampler2D uni_PrimaryTexture;
uniform sampler2D uni_SecondaryTexture;

uniform bool uni_PrimaryTextureEnabled;
uniform bool uni_SecondaryTextureEnabled;

uniform bool uni_AlphaTestEnabled;
uniform float uni_AlphaReference;

in VertexData
{
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec3 Normal;
} data;

out vec4 out_FragColor;

void main()
{
    vec4 color = data.Color;

    if (uni_PrimaryTextureEnabled)
    {
        color = color * texture(uni_PrimaryTexture, data.TexCoord0);
    }

    if (uni_SecondaryTextureEnabled)
    {
        color = color * texture(uni_SecondaryTexture, data.TexCoord1);
    }

    if (uni_AlphaTestEnabled)
    {
        if(color.a < uni_AlphaReference)
            discard;
    }

    out_FragColor = color;
}
