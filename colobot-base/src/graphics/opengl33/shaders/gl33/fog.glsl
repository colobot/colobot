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

// Fog calculations

uniform float uni_FogUpperHeight;
uniform vec2 uni_FogUpperRange;
uniform vec3 uni_FogUpperColor;

uniform float uni_FogLowerHeight;
uniform vec2 uni_FogLowerRange;
uniform vec3 uni_FogLowerColor;

vec3 ApplyFog(vec3 color, vec3 position)
{
    float distance = length(uni_CameraPosition - position);

    float fog_amount = clamp((distance - uni_FogUpperRange.x) / (uni_FogUpperRange.y - uni_FogUpperRange.x), 0.0, 1.0);

    float fog_transition = clamp((position.y - uni_FogLowerHeight) / (uni_FogUpperHeight - uni_FogLowerHeight), 0.0, 1.0);

    vec3 fog_color = mix(uni_FogLowerColor, uni_FogUpperColor, fog_transition);

    color = mix(color, fog_color, fog_amount);

    // Apply depth darkening below water surface
    float depth = clamp((position.y - uni_FogUpperHeight + 64.0) / 64.0, 0.0, 1.0);

    return color * mix(0.2, 1.0, depth);
}
