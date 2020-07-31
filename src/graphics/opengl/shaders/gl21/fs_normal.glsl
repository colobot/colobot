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
#version 120

#define CONFIG_QUALITY_SHADOWS 1

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
uniform float uni_ShadowTexelSize;

struct LightParams
{
    vec4 Position;
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular;
};

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform Material uni_Material;

uniform int uni_LightCount;
uniform LightParams uni_Light[4];

varying vec3 pass_CameraDirection;
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

    if (uni_LightCount > 0)
    {
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);

        vec3 normal = normalize(pass_Normal);
        vec3 camera = normalize(pass_CameraDirection);

        for (int i = 0; i < uni_LightCount; i++)
        {
            LightParams light = uni_Light[i];

            vec3 lightDirection = light.Position.xyz;
            vec3 reflectAxis = normalize(normalize(lightDirection) + camera);

            float diffuseComponent = clamp(dot(normal, lightDirection), 0.0f, 1.0f);
            float specularComponent = pow(clamp(dot(normal, reflectAxis), 0.0f, 1.0f), 10.0f);

            ambient += light.Ambient;
            diffuse += diffuseComponent * light.Diffuse;
            specular += specularComponent * light.Specular;
        }

        float shadow = 1.0f;

        if (uni_TextureEnabled[2])
        {
#ifdef CONFIG_QUALITY_SHADOWS
            float value = (1.0f / 5.0f) * (shadow2D(uni_ShadowTexture, pass_TexCoord2).x
                    + shadow2D(uni_ShadowTexture, pass_TexCoord2 + vec3( uni_ShadowTexelSize,    0.0f, 0.0f)).x
                    + shadow2D(uni_ShadowTexture, pass_TexCoord2 + vec3(-uni_ShadowTexelSize,    0.0f, 0.0f)).x
                    + shadow2D(uni_ShadowTexture, pass_TexCoord2 + vec3(   0.0f,  uni_ShadowTexelSize, 0.0f)).x
                    + shadow2D(uni_ShadowTexture, pass_TexCoord2 + vec3(   0.0f, -uni_ShadowTexelSize, 0.0f)).x);

            shadow = mix(uni_ShadowColor, 1.0f, value);
#else
            shadow = mix(uni_ShadowColor, 1.0f, shadow2D(uni_ShadowTexture, pass_TexCoord2).x);
#endif
        }

        vec4 result = ambient * uni_Material.ambient
                + diffuse * uni_Material.diffuse * shadow
                + specular * uni_Material.specular * shadow;

        color = clamp(vec4(result.rgb, 1.0f), 0.0f, 1.0f);
    }

    if (uni_TextureEnabled[0])
    {
        color = color * texture2D(uni_PrimaryTexture, pass_TexCoord0);
    }

    if (uni_TextureEnabled[1])
    {
        color = color * texture2D(uni_SecondaryTexture, pass_TexCoord1);
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
