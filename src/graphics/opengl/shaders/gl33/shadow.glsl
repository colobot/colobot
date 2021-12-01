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

// Shadow mapping implementation

struct ShadowParam
{
    mat4 transform;
    vec2 uv_offset;
    vec2 uv_scale;
};

uniform sampler2DShadow uni_ShadowMap;
uniform int uni_ShadowRegions;
uniform ShadowParam uni_ShadowParam[4];

// Projects world-space position into an array of shadow coordinates
vec3[4] ProjectShadows(vec3 position)
{
    vec3 result[4];

    for (int i = 0; i < uni_ShadowRegions; i++)
    {
        vec4 projected = uni_ShadowParam[i].transform * vec4(position, 1.0);

        result[i] = projected.xyz / projected.w;
    }

    return result;
}

// Calculates shadow based on array of shadow coordinates
float CalculateShadow(vec3 projected[4])
{
    for (int i = 0; i < uni_ShadowRegions; i++)
    {
        if (projected[i].x < 0.0 || projected[i].x > 1.0) continue;
        if (projected[i].y < 0.0 || projected[i].y > 1.0) continue;

        vec2 uv = projected[i].xy * uni_ShadowParam[i].uv_scale + uni_ShadowParam[i].uv_offset;
        float depth = projected[i].z;

        vec3 coord = vec3(uv, depth);

        /*
        float value = texture(uni_ShadowMap, coord);
        value += textureOffset(uni_ShadowMap, coord, ivec2( 1, 0));
        value += textureOffset(uni_ShadowMap, coord, ivec2(-1, 0));
        value += textureOffset(uni_ShadowMap, coord, ivec2( 0, 1));
        value += textureOffset(uni_ShadowMap, coord, ivec2( 0,-1));
        return value * (1.0f / 5.0f);
        // */

        return texture(uni_ShadowMap, coord);
    }

    return 1.0;
}
