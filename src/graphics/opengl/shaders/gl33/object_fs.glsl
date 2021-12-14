/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

// FRAGMENT SHADER - TERRAIN RENDERER

uniform bool uni_Lighting;
uniform mat4 uni_CameraMatrix;
uniform vec4 uni_LightPosition;
uniform float uni_LightIntensity;
uniform vec3 uni_LightColor;

uniform vec2 uni_FogRange;
uniform vec3 uni_FogColor;

uniform sampler2D uni_PrimaryTexture;
uniform sampler2D uni_SecondaryTexture;

uniform vec4 uni_Color;
uniform float uni_PrimaryEnabled;
uniform bool uni_TriplanarMode;
uniform float uni_TriplanarScale;
uniform float uni_Dirty;
uniform float uni_AlphaScissor;

const float PI = 3.1415926;

in VertexData
{
    vec4 Color;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec3 Normal;
    vec3 VertexCoord;
    vec3 VertexNormal;
    vec3 Position;
    vec3 ShadowCoords[4];
} data;

out vec4 out_FragColor;

vec3 schlickFresnel(float LdH, float metalness, vec3 color)
{
    vec3 f = mix(vec3(0.04), color, metalness);

    return f + (1.0 - f) * pow(1.0 - LdH, 5.0);
}

float geomSmith(float dotProd, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float denom = dotProd * (1 - k) + k;
    return 1.0 / denom;
}

float ggxDistribution(float NdH, float roughness)
{
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = (NdH * NdH) * (alpha2 - 1) + 1.0;
    return alpha2 / (PI * d * d);
}

vec3 PBR(vec3 position, vec3 color, vec3 normal, float roughness, float metalness)
{
    vec3 diffuseBrdf = mix(color, vec3(0.0), metalness);

    vec3 light = normalize(uni_LightPosition.xyz);
    float lightIntensity = 1.0;

    vec3 view = normalize(uni_CameraMatrix[3].xyz - position);
    vec3 halfway = normalize(view + light);

    float NdH = dot(normal, halfway);
    float LdH = dot(light, halfway);
    float NdL = max(dot(normal, light), 0.0);
    float NdV = dot(normal, view);

    vec3 specBrdf = 0.25 * ggxDistribution(NdH, roughness)
            * schlickFresnel(LdH, metalness, color)
            * geomSmith(NdL, roughness)
            * geomSmith(NdV, roughness);

    return (diffuseBrdf + PI * specBrdf) * lightIntensity * uni_LightColor * NdL;
}

vec3 Triplanar(vec3 position, vec3 normal)
{
    vec3 weights = normal * normal;

    vec3 sum = texture(uni_SecondaryTexture, position.yz).rgb * weights.x;
    sum += texture(uni_SecondaryTexture, position.zx).rgb * weights.y;
    sum += texture(uni_SecondaryTexture, position.xy).rgb * weights.z;

    return sum;
}

void main()
{
    vec4 albedo = data.Color * uni_Color;

    vec4 primary = texture(uni_PrimaryTexture, data.TexCoord0);
    primary = mix(vec4(1.0), primary, uni_PrimaryEnabled);
    albedo *= primary;

    vec3 dirty = vec3(0.0);

    if (uni_TriplanarMode)
    {
        dirty = Triplanar(data.VertexCoord * uni_TriplanarScale, data.VertexNormal);
    }
    else
    {
        dirty = texture(uni_SecondaryTexture, data.TexCoord1).rgb;
    }

    dirty = mix(vec3(1.0), dirty, uni_Dirty);
    albedo.rgb *= dirty;

    vec3 color = albedo.rgb;

    if (uni_Lighting)
    {
        float roughness = 0.9;
        float metalness = 0.0;

        float shadow = CalculateShadow(data.ShadowCoords);

        shadow = mix(0.5, 1.0, shadow);

        vec3 normal = mix(-data.Normal, data.Normal, float(gl_FrontFacing));

        vec3 lighting = PBR(data.Position, color.rgb, normal, roughness, metalness);

        vec3 skyColor = vec3(1.0);
        float skyIntensity = 0.25;

        color = lighting * shadow * (1.0 - skyIntensity) + color.rgb * skyColor * skyIntensity;
    }

    float dist = length(uni_CameraMatrix[3].xyz - data.Position);
    float fogAmount = clamp((dist - uni_FogRange.x) / (uni_FogRange.y - uni_FogRange.x), 0.0, 1.0);

    color = mix(color, uni_FogColor, fogAmount);

    if (albedo.a < uni_AlphaScissor) discard;

    out_FragColor = vec4(color, albedo.a);
}
