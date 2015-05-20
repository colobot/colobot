/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

// FRAGMENT SHADER
#version 330

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

uniform vec4 uni_AmbientColor;
uniform vec4 uni_DiffuseColor;
uniform vec4 uni_SpecularColor;

uniform bool uni_LightingEnabled;
uniform LightParams uni_Light[8];

uniform bool uni_SmoothShading;

in VertexData
{
    vec3 NormalSmooth;
    flat vec3 NormalFlat;
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec4 ShadowCoord;
    vec4 Position;
    float Distance;
} data;

out vec4 out_FragColor;

void main()
{
    vec4 color = data.Color;
    
    if (uni_LightingEnabled)
    {
        vec4 ambient = vec4(0.0f);
        vec4 diffuse = vec4(0.0f);
        vec4 specular = vec4(0.0f);
        
        for(int i=0; i<8; i++)
        {
            if(uni_Light[i].Enabled)
            {
                ambient += uni_Light[i].Ambient;
                
                vec3 normal = (uni_SmoothShading ? data.NormalSmooth : data.NormalFlat);
                normal = (gl_FrontFacing ? normal : -normal);
                
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
                    vec3 lightDirection = normalize(uni_Light[i].Position.xyz - data.Position.xyz);
                    float dist = distance(uni_Light[i].Position.xyz, data.Position.xyz);
                    
                    atten = 1.0f / (uni_Light[i].Attenuation.x
                            + uni_Light[i].Attenuation.y * dist
                            + uni_Light[i].Attenuation.z * dist * dist);
                }
                
                vec3 reflectDirection = -reflect(lightDirection, normal);
                
                diffuse += atten * clamp(dot(normal, lightDirection), 0.0f, 1.0f) * uni_Light[i].Diffuse;
                specular += atten * clamp(pow(dot(normal, lightDirection + reflectDirection), 10.0f), 0.0f, 1.0f) * uni_Light[i].Specular;
            }
        }
        
        vec4 result = uni_AmbientColor * ambient
                + uni_DiffuseColor * diffuse
                + uni_SpecularColor * specular;
        
        color.rgb = min(vec3(1.0f), result.rgb);
        color.a = 1.0f; //min(1.0f, 1.0f);
    }
    
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
