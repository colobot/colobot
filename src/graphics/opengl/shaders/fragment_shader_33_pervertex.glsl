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

// FRAGMENT SHADER - PER-VERTEX LIGHTING
#version 330

uniform sampler2D uni_PrimaryTexture;
uniform sampler2D uni_SecondaryTexture;
uniform sampler2DShadow uni_ShadowTexture;

uniform bool uni_PrimaryTextureEnabled;
uniform bool uni_SecondaryTextureEnabled;
uniform bool uni_ShadowTextureEnabled;

uniform bool uni_FogEnabled;
uniform vec2 uni_FogRange;
uniform vec4 uni_FogColor;

uniform bool uni_AlphaTestEnabled;
uniform float uni_AlphaReference;

in VertexData
{
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec4 ShadowCoord;
    vec4 LightColor;
    float Distance;
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
    
    if (uni_ShadowTextureEnabled)
    {
        color = color * (0.35f + 0.65f * texture(uni_ShadowTexture, data.ShadowCoord.xyz));
    }
    
    if (uni_FogEnabled)
    {
        float interpolate = (data.Distance - uni_FogRange.x) / (uni_FogRange.y - uni_FogRange.x);
        
        color = mix(color, uni_FogColor, clamp(interpolate, 0.0f, 1.0f));
    }
    
    if (uni_AlphaTestEnabled)
    {
        if(color.a < uni_AlphaReference)
            discard;
    }
    
    out_FragColor = color;
}
