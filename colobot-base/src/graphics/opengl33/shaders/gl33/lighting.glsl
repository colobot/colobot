/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2022, Daniel Roux, EPSITEC SA & TerranovaTeam
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

// PBR lighting calculations

uniform bool uni_Lighting;

uniform vec3 uni_CameraPosition;

uniform float uni_LightIntensity;
uniform vec4 uni_LightPosition;
uniform vec3 uni_LightColor;

uniform float uni_SkyIntensity;
uniform vec3 uni_SkyColor;

const float PI = 3.1415926;

vec3 SchlickFresnel(float LdH, float metalness, vec3 color)
{
    vec3 f = mix(vec3(0.04), color, metalness);

    return f + (1.0 - f) * pow(1.0 - LdH, 5.0);
}

float GeometrySmith(float dotProd, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float denom = dotProd * (1 - k) + k;
    return 1.0 / denom;
}

float GGXDistribution(float NdH, float roughness)
{
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = (NdH * NdH) * (alpha2 - 1) + 1.0;
    return alpha2 / (PI * d * d);
}

vec3 CalculateLighting(
        vec3 position,
        vec3 normal,
        vec3 albedo,
        vec3 emissive,
        float shadow,
        float ambientOcclusion,
        float roughness,
        float metalness)
{
    vec3 diffuseBrdf = mix(albedo, vec3(0.0), metalness);

    vec3 light = normalize(uni_LightPosition.xyz);
    float lightIntensity = uni_LightIntensity;

    vec3 view = normalize(uni_CameraPosition - position);
    vec3 halfway = normalize(view + light);

    float NdH = dot(normal, halfway);
    float LdH = dot(light, halfway);
    float NdL = max(dot(normal, light), 0.0);
    float NdV = dot(normal, view);

    vec3 specBrdf = 0.25
            * GGXDistribution(NdH, roughness)
            * SchlickFresnel(LdH, metalness, albedo)
            * GeometrySmith(NdL, roughness)
            * GeometrySmith(NdV, roughness);

    vec3 diffuseSpecular = (diffuseBrdf + PI * specBrdf) * uni_LightIntensity * uni_LightColor * NdL * shadow;
    vec3 ambient = albedo * uni_SkyColor * uni_SkyIntensity * ambientOcclusion;

    return ambient + emissive + diffuseSpecular;
}
