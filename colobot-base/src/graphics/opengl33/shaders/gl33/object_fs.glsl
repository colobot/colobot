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

// FRAGMENT SHADER - OBJECT RENDERER

uniform vec2 uni_FogRange;
uniform vec3 uni_FogColor;

uniform vec4 uni_AlbedoColor;
uniform sampler2D uni_AlbedoTexture;
uniform sampler2D uni_DetailTexture;

uniform float uni_Roughness;
uniform float uni_Metalness;
uniform float uni_AOStrength;
uniform sampler2D uni_MaterialTexture;

uniform vec3 uni_EmissiveColor;
uniform sampler2D uni_EmissiveTexture;

uniform bool uni_TriplanarMode;
uniform float uni_TriplanarScale;

uniform float uni_AlphaScissor;

uniform bool uni_Recolor;
uniform vec3 uni_RecolorFrom;
uniform vec3 uni_RecolorTo;
uniform float uni_RecolorThreshold;

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

vec3 Triplanar(vec3 position, vec3 normal)
{
    vec3 weights = normal * normal;

    vec3 sum = texture(uni_DetailTexture, position.yz).rgb * weights.x;
    sum += texture(uni_DetailTexture, position.zx).rgb * weights.y;
    sum += texture(uni_DetailTexture, position.xy).rgb * weights.z;

    return sum;
}

// All components are in the range [0...1], including hue.
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0...1], including hue.
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 albedo = data.Color * uni_AlbedoColor;

    vec4 texColor = texture(uni_AlbedoTexture, data.TexCoord0);

    if (uni_Recolor)
    {
        vec3 hsv = rgb2hsv(texColor.rgb);

        if (abs(hsv.x - uni_RecolorFrom.x) < uni_RecolorThreshold)
        {
            hsv.x += (uni_RecolorTo.x - uni_RecolorFrom.x);
            hsv.y += (uni_RecolorTo.y - uni_RecolorFrom.y);

            if (hsv.x < 0.0) hsv.x += 1.0;
            if (hsv.x > 1.0) hsv.x -= 1.0;

            hsv.y = clamp(hsv.y, 0.0, 1.0);
        }

        texColor.rgb = hsv2rgb(hsv);
    }

    albedo *= texColor;

    vec3 dirty = vec3(0.0);

    if (uni_TriplanarMode)
    {
        dirty = Triplanar(data.VertexCoord * uni_TriplanarScale, data.VertexNormal);
    }
    else
    {
        dirty = texture(uni_DetailTexture, data.TexCoord1).rgb;
    }

    albedo.rgb *= dirty;

    vec3 color = albedo.rgb;
    float alpha = albedo.a;

    if (uni_Lighting)
    {
        float shadow = CalculateShadow(data.ShadowCoords);

        shadow = mix(0.5, 1.0, shadow);

        vec3 normal = mix(-data.Normal, data.Normal, float(gl_FrontFacing));

        vec3 emissive = uni_EmissiveColor * texture(uni_EmissiveTexture, data.TexCoord0).rgb;

        vec3 params = texture(uni_MaterialTexture, data.TexCoord0).xyz;

        float ambientOcclusion = 1.0 + uni_AOStrength * (params.r - 1.0);
        float roughness = uni_Roughness * params.g;
        float metalness = uni_Metalness * params.b;

        color = CalculateLighting(
                data.Position,
                normal,
                color,
                emissive,
                shadow,
                ambientOcclusion,
                roughness,
                metalness);
    }

    float dist = length(uni_CameraPosition - data.Position);
    float fogAmount = clamp((dist - uni_FogRange.x) / (uni_FogRange.y - uni_FogRange.x), 0.0, 1.0);

    color = mix(color, uni_FogColor, fogAmount);

    if (alpha < uni_AlphaScissor) discard;

    out_FragColor = vec4(color, alpha);
}
