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

uniform bool uni_LightingEnabled;
uniform bool uni_LightEnabled[8];

varying float pass_Distance;

void main()
{
    vec4 position = uni_ModelMatrix * gl_Vertex;
    vec4 eyeSpace = uni_ViewMatrix * position;
    vec4 shadowCoord = uni_ShadowMatrix * position;
    gl_Position = uni_ProjectionMatrix * eyeSpace;
    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;
    gl_TexCoord[2] = vec4(shadowCoord.xyz / shadowCoord.w, 1.0f);
    pass_Distance = abs(eyeSpace.z / eyeSpace.w);

    if (uni_LightingEnabled)
    {
        vec4 color;
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);

        vec3 normal = normalize((uni_NormalMatrix * vec4(gl_Normal, 0.0f)).xyz);

        for(int i=0; i<8; i++)
        {
            if(uni_LightEnabled[i])
            {
                vec3 lightDirection = vec3(0.0f);
                float atten;

                // Directional light
                if(gl_LightSource[i].position.w == 0.0f)
                {
                    lightDirection = gl_LightSource[i].position.xyz;
                    atten = 1.0f;
                }
                // Point light
                else
                {
                    vec3 lightDirection = normalize(gl_LightSource[i].position.xyz - position.xyz);
                    float dist = distance(gl_LightSource[i].position.xyz, position.xyz);

                    atten = 1.0f / (gl_LightSource[i].constantAttenuation
                            + gl_LightSource[i].linearAttenuation * dist
                            + gl_LightSource[i].quadraticAttenuation * dist * dist);
                }

                vec3 reflectDirection = -reflect(lightDirection, normal);

                ambient += gl_LightSource[i].ambient;
                diffuse += atten * clamp(dot(normal, lightDirection), 0.0f, 1.0f) * gl_LightSource[i].diffuse;
                specular += atten * clamp(pow(dot(normal, lightDirection + reflectDirection), 10.0f), 0.0f, 1.0f) * gl_LightSource[i].specular;
            }
        }

        vec4 result = gl_FrontMaterial.ambient * ambient
                + gl_FrontMaterial.diffuse * diffuse
                + gl_FrontMaterial.specular * specular;

        color.rgb = min(vec3(1.0f), result.rgb);
        color.a = 1.0f; //min(1.0f, 1.0f);

        gl_FrontColor = color;
    }
}

