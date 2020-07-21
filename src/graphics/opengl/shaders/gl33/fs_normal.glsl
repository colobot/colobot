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

#define CONFIG_QUALITY_SHADOWS 1

uniform sampler2D uni_PrimaryTexture;
uniform sampler2D uni_SecondaryTexture;
uniform sampler2DShadow uni_ShadowTexture;

uniform bool uni_PrimaryTextureEnabled;
uniform bool uni_SecondaryTextureEnabled;
uniform bool uni_ShadowTextureEnabled;

uniform bool uni_FogEnabled;
uniform vec2 uni_FogRange;
uniform vec4 uni_FogColor;

uniform float uni_ShadowColor;

uniform bool uni_AlphaTestEnabled;
uniform float uni_AlphaReference;

struct LightParams
{
    vec4 Position;
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular;
};

uniform vec4 uni_AmbientColor;
uniform vec4 uni_DiffuseColor;
uniform vec4 uni_SpecularColor;

uniform int uni_LightCount;
uniform LightParams uni_Light[4];

in VertexData
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

out vec4 out_FragColor;

void main()
{
    vec4 color = data.Color;

    if (uni_LightCount > 0)
    {
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);

        vec3 normal = normalize(data.Normal);
        vec3 camera = normalize(data.CameraDirection);

        for (int i = 0; i < uni_LightCount; i++)
        {
            vec3 lightDirection = normalize(uni_Light[i].Position.xyz);
            vec3 reflectAxis = normalize(lightDirection + camera);

            ambient += uni_Light[i].Ambient;
            diffuse += clamp(dot(normal, lightDirection), 0.0f, 1.0f)
                    * uni_Light[i].Diffuse;
            specular += pow(clamp(dot(normal, reflectAxis), 0.0f, 1.0f), 10.0f)
                    * uni_Light[i].Specular;
        }

        float shadow = 1.0f;

        if (uni_ShadowTextureEnabled)
        {
            float value = texture(uni_ShadowTexture, data.ShadowCoord.xyz);
#ifdef CONFIG_QUALITY_SHADOWS
            value += textureOffset(uni_ShadowTexture, data.ShadowCoord.xyz, ivec2( 1, 0))
                   + textureOffset(uni_ShadowTexture, data.ShadowCoord.xyz, ivec2(-1, 0))
                   + textureOffset(uni_ShadowTexture, data.ShadowCoord.xyz, ivec2( 0, 1))
                   + textureOffset(uni_ShadowTexture, data.ShadowCoord.xyz, ivec2( 0,-1));
            value = value * (1.0f / 5.0f);
#endif
            shadow = mix(uni_ShadowColor, 1.0f, value);
        }

        vec4 result = uni_AmbientColor * ambient
                + uni_DiffuseColor * diffuse * shadow
                + uni_SpecularColor * specular * shadow;

        color = vec4(min(vec3(1.0f), result.rgb), 1.0f);
    }

    if (uni_PrimaryTextureEnabled)
    {
        color = color * texture(uni_PrimaryTexture, data.TexCoord0);
    }

    if (uni_SecondaryTextureEnabled)
    {
        color = color * texture(uni_SecondaryTexture, data.TexCoord1);
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
