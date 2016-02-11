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

#version 120

uniform sampler2D uni_PrimaryTexture;
uniform sampler2D uni_SecondaryTexture;
uniform sampler2DShadow uni_ShadowTexture;

uniform bool uni_TextureEnabled[3];

uniform bool uni_AlphaTestEnabled;
uniform float uni_AlphaReference;

uniform bool uni_FogEnabled;
uniform vec2 uni_FogRange;
uniform vec4 uni_FogColor;

uniform float uni_ShadowColor;

varying float pass_Distance;
varying vec4 pass_Color;
varying vec3 pass_Normal;
varying vec2 pass_TexCoord0;
varying vec2 pass_TexCoord1;
varying vec3 pass_TexCoord2;

const vec3 const_LightDirection = vec3(1.0f, 2.0f, -1.0f);

void main()
{
    vec4 color = pass_Color;

    if (uni_TextureEnabled[0])
    {
        color = color * texture2D(uni_PrimaryTexture, pass_TexCoord0);
    }

    if (uni_TextureEnabled[1])
    {
        color = color * texture2D(uni_SecondaryTexture, pass_TexCoord1);
    }

    if (uni_TextureEnabled[2])
    {
        vec3 normal = pass_Normal * (2.0f * gl_Color.x - 1.0f);

        if (dot(normal, const_LightDirection) < 0.0f)
            color.rgb *= uni_ShadowColor;
        else
            color.rgb *= mix(uni_ShadowColor, 1.0f, shadow2D(uni_ShadowTexture, pass_TexCoord2).x);
    }

    if (uni_FogEnabled)
    {
        float interpolate = (pass_Distance - uni_FogRange.x) / (uni_FogRange.y - uni_FogRange.x);

        color = mix(color, uni_FogColor, clamp(interpolate, 0.0f, 1.0f));
    }

    if (uni_AlphaTestEnabled)
    {
        if(color.a < uni_AlphaReference)
            discard;
    }

    gl_FragColor = color;
}
