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

struct LightParams
{
    bool Enabled;
    vec4 Position;
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular;
    float Shininess;
    vec3 Attenuation;
};

uniform vec4 uni_AmbientColor;
uniform vec4 uni_DiffuseColor;
uniform vec4 uni_SpecularColor;

uniform bool uni_LightingEnabled;
uniform LightParams uni_Light[8];

varying vec3 pass_Normal;
varying vec3 pass_Position;
varying float pass_Distance;

void main()
{
    vec4 color = gl_Color;

    if (uni_LightingEnabled)
    {
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);

        vec3 normal = (gl_FrontFacing ? pass_Normal : -pass_Normal);

        for (int i = 0; i < 8; i++)
        {
            if (uni_Light[i].Enabled)
            {
                vec3 lightDirection = uni_Light[i].Position.xyz;
                float atten = 1.0f;

                // Point light
                if (abs(uni_Light[i].Position.w) > 1e-3f)
                {
                    vec3 lightDirection = normalize(uni_Light[i].Position.xyz - pass_Position.xyz);
                    float dist = distance(uni_Light[i].Position.xyz, pass_Position.xyz);

                    vec3 lightAtten = uni_Light[i].Attenuation;

                    atten = 1.0f / (lightAtten.x
                            + lightAtten.y * dist
                            + lightAtten.z * dist * dist);
                }

                vec3 reflectDirection = -reflect(lightDirection, normal);

                ambient += uni_Light[i].Ambient;
                diffuse += atten * clamp(dot(normal, lightDirection), 0.0f, 1.0f) * uni_Light[i].Diffuse;
                specular += atten * clamp(pow(dot(normal, lightDirection + reflectDirection), 10.0f), 0.0f, 1.0f) * uni_Light[i].Specular;
            }
        }

        vec4 result = uni_AmbientColor * ambient
                + uni_DiffuseColor * diffuse
                + uni_SpecularColor * specular;

        color = vec4(min(vec3(1.0f), result.rgb), 1.0f);
    }

    if (uni_TextureEnabled[0])
    {
        color = color * texture2D(uni_PrimaryTexture, gl_TexCoord[0].st);
    }

    if (uni_TextureEnabled[1])
    {
        color = color * texture2D(uni_SecondaryTexture, gl_TexCoord[1].st);
    }

    if (uni_TextureEnabled[2])
    {
        if (gl_FrontFacing)
            color.rgb *= mix(uni_ShadowColor, 1.0f, shadow2D(uni_ShadowTexture, gl_TexCoord[2].xyz).x);
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
