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
    int Type;
    vec4 Position;
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular;
    float Shininess;
    vec3 Attenuation;
    vec3 SpotDirection;
    float SpotCutoff;
    float SpotExponent;
};

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform Material uni_Material;

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
                LightParams light = uni_Light[i];

                vec3 lightDirection = light.Position.xyz;
                float atten = 1.0f;

                if (light.Position.w != 0.0f)
                {
                    float dist = distance(light.Position.xyz, position.xyz);

                    float atten = 1.0f / (light.Attenuation.x
                            + light.Attenuation.y * dist
                            + light.Attenuation.z * dist * dist);

                    lightDirection = normalize(light.Position.xyz - position.xyz);
                }

                float spot = 1.0f;

                if (light.SpotCutoff > 0.0f)
                {
                    float cone = dot(light.SpotDirection, lightDirection);

                    if (cone > light.SpotCutoff)
                    {
                        spot = pow(cone, light.SpotExponent);
                    }
                    else
                    {
                        spot = 0.0f;
                    }
                }

                vec3 reflectDirection = -reflect(lightDirection, normal);

                float component = atten * spot;
                float diffuseComponent = clamp(dot(normal, lightDirection), 0.0f, 1.0f);
                float specularComponent = clamp(pow(dot(normal, lightDirection + reflectDirection), light.Shininess), 0.0f, 1.0f);

                ambient += component * light.Ambient * uni_Material.ambient;
                diffuse += component * diffuseComponent * light.Diffuse * uni_Material.diffuse;
                specular += component * specularComponent * light.Specular * uni_Material.specular;
            }
        }

        vec4 result = ambient + diffuse + specular;

        color = clamp(vec4(result.rgb, uni_Material.diffuse), 0.0f, 1.0f);
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
