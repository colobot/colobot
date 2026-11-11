/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "math/sphere.h"

#include <string>
#include <vector>

struct ModeledBlueprint
{

    struct CreateAt
    {
        bool isFixedPosition = false;
        float extraHeight = 0.0;
        float extraScale = 1.0;
    };

    struct ShadowCircle
    {
        float radius = 0.0f;
        float intensity = 0.0f;
    };

    struct CrashSphere
    {
        glm::vec3 pos = { 0, 0, 0 };
        float radius = 0.0f;
        float hardness = 0.45f;
    };

    struct ModelNode
    {
        int partId = 0;
        int parentId = -1;
        std::string file = "";
        bool isMirrored = false;
        bool isCopy = false;
    };

    std::string gfxType = "Object";

    CreateAt createAt;
    ShadowCircle shadowCircle;
    Math::Sphere cameraCollision;

    std::vector<CrashSphere> crashSpheres;
    std::vector<ModelNode> modelNodes;
};
