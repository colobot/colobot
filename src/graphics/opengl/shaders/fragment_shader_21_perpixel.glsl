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

uniform bool uni_LightingEnabled;
uniform bool uni_LightEnabled[8];

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

        vec3 normal = pass_Normal;

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
                    vec3 lightDirection = normalize(gl_LightSource[i].position.xyz - pass_Position);
                    float dist = distance(gl_LightSource[i].position.xyz, pass_Position);

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
        color = color * mix(uni_ShadowColor, 1.0f, shadow2D(uni_ShadowTexture, gl_TexCoord[2].xyz).x);
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
