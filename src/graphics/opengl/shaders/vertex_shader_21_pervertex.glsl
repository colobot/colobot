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

uniform mat4 uni_ProjectionMatrix;
uniform mat4 uni_ViewMatrix;
uniform mat4 uni_ModelMatrix;
uniform mat4 uni_ShadowMatrix;
uniform mat4 uni_NormalMatrix;

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

    vec4 color = gl_Color;

    vec3 normal = normalize((uni_NormalMatrix * vec4(gl_Normal, 0.0f)).xyz);

    if (uni_LightingEnabled)
    {
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);

        for (int i = 0; i < 8; i++)
        {
            if (uni_Light[i].Enabled)
            {
                vec3 lightDirection = uni_Light[i].Position.xyz;
                float atten = 1.0f;

                // Point light
                if (abs(uni_Light[i].Position.w) > 1e-3f)
                {
                    vec3 lightDirection = normalize(uni_Light[i].Position.xyz - position.xyz);
                    float dist = distance(uni_Light[i].Position.xyz, position.xyz);

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

    gl_Position = uni_ProjectionMatrix * eyeSpace;
    gl_FrontColor = vec4(1.0f);
    gl_BackColor = vec4(0.0f);

    pass_Distance = abs(eyeSpace.z / eyeSpace.w);
    pass_Color = color;
    pass_Normal = normal;
    pass_TexCoord0 = gl_MultiTexCoord0.st;
    pass_TexCoord1 = gl_MultiTexCoord1.st;
    pass_TexCoord2 = shadowCoord.xyz / shadowCoord.w;
}
