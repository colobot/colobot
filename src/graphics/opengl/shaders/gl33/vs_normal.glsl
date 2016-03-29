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
#version 330 core

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
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec4 ShadowCoord;
    vec4 LightColor;
    float Distance;
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
    data.ShadowCoord = vec4(shadowCoord.xyz / shadowCoord.w, 1.0f);
    data.Distance = abs(eyeSpace.z);

    vec4 color = in_Color;

    if (uni_LightingEnabled)
    {
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);

        vec3 normal = normalize((uni_NormalMatrix * vec4(in_Normal, 0.0f)).xyz);

        for(int i=0; i<8; i++)
        {
            if(uni_Light[i].Enabled)
            {
                vec3 lightDirection = vec3(0.0f);
                float atten;

                // Directional light
                if(uni_Light[i].Position[3] == 0.0f)
                {
                    lightDirection = uni_Light[i].Position.xyz;
                    atten = 1.0f;
                }
                // Point light
                else
                {
                    vec3 lightDirection = normalize(uni_Light[i].Position.xyz - position.xyz);
                    float dist = distance(uni_Light[i].Position.xyz, position.xyz);

                    atten = 1.0f / (uni_Light[i].Attenuation.x
                            + uni_Light[i].Attenuation.y * dist
                            + uni_Light[i].Attenuation.z * dist * dist);
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

        color.rgb = min(vec3(1.0f), result.rgb);
        color.a = 1.0f; //min(1.0f, 1.0f);

        data.Color = color;
    }
}
