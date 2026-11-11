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

#include "blueprint/blueprint_hardcode.h"
#include "blueprint/part/modeled_blueprint.h"

#include "object/object_type.h"

namespace {

std::string GfxType(ObjectType type)
{
    switch (type)
    {
        case OBJECT_STONE:
        case OBJECT_URANIUM:
        case OBJECT_METAL:
        case OBJECT_POWER:
        case OBJECT_ATOMIC:
        case OBJECT_BULLET:
        case OBJECT_BBOX:
        case OBJECT_KEYa:
        case OBJECT_KEYb:
        case OBJECT_KEYc:
        case OBJECT_KEYd:
        case OBJECT_TNT:
        case OBJECT_BOMB:
        case OBJECT_WAYPOINT:
        case OBJECT_SHOW:
        case OBJECT_WINFIRE:
        case OBJECT_BAG:
        case OBJECT_MARKPOWER:
        case OBJECT_MARKSTONE:
        case OBJECT_MARKURANIUM:
        case OBJECT_MARKKEYa:
        case OBJECT_MARKKEYb:
        case OBJECT_MARKKEYc:
        case OBJECT_MARKKEYd:
        case OBJECT_EGG:
            return "Object";

        default:
            return "Object";
    }
}

ModeledBlueprint::CreateAt CreateAt(ObjectType type)
{
    ModeledBlueprint::CreateAt result;
    switch (type)
    {
        case OBJECT_BAG:
            result.extraHeight = -1.4f;
            result.extraScale = 1.5f;
            break;

        case OBJECT_SHOW:
            result.isFixedPosition = true;
            break;

        default:
            break;
    }
    return result;
}

ModeledBlueprint::ShadowCircle ShadowCircle(ObjectType type)
{
    switch (type)
    {
        case OBJECT_EGG:
        case OBJECT_BOMB:
            return {3.0f, 1.0f};

        case OBJECT_BAG:
            return {5.0f, 1.0f};

        case OBJECT_STONE:
        case OBJECT_URANIUM:
        case OBJECT_METAL:
        case OBJECT_POWER:
        case OBJECT_ATOMIC:
        case OBJECT_BULLET:
        case OBJECT_BBOX:
        case OBJECT_KEYa:
        case OBJECT_KEYb:
        case OBJECT_KEYc:
        case OBJECT_KEYd:
        case OBJECT_TNT:
        case OBJECT_WAYPOINT:
        case OBJECT_SHOW:
        case OBJECT_WINFIRE:
        case OBJECT_MARKPOWER:
        case OBJECT_MARKSTONE:
        case OBJECT_MARKURANIUM:
        case OBJECT_MARKKEYa:
        case OBJECT_MARKKEYb:
        case OBJECT_MARKKEYc:
        case OBJECT_MARKKEYd:
            return {1.5f, 1.0f};

        default:
            break;
    }

    return ModeledBlueprint::ShadowCircle();
}

Math::Sphere CameraCollision(ObjectType type)
{
    switch (type)
    {
        case OBJECT_EGG:
            return Math::Sphere(glm::vec3(0.0f, 5.0f, 0.0f), 10.0f);
        case OBJECT_BOMB:
            return Math::Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 3.0f);
        case OBJECT_BAG:
            return Math::Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 4.0f);

        case OBJECT_STONE:
        case OBJECT_URANIUM:
        case OBJECT_METAL:
        case OBJECT_POWER:
        case OBJECT_ATOMIC:
        case OBJECT_BULLET:
        case OBJECT_BBOX:
        case OBJECT_KEYa:
        case OBJECT_KEYb:
        case OBJECT_KEYc:
        case OBJECT_KEYd:
        case OBJECT_TNT:
        case OBJECT_WINFIRE:
            return Math::Sphere(glm::vec3(0.0f, 1.0f, 0.0f), 1.5f);

        default:
            return Math::Sphere();
    }
}

std::vector<ModeledBlueprint::CrashSphere> CrashSpheres(ObjectType type)
{
    std::vector<ModeledBlueprint::CrashSphere> result;

    switch (type)
    {
        case OBJECT_EGG:
            result.push_back({glm::vec3(-1.0f, 2.8f, 0.0f), 3.0f, 0.45f});
            break;

        case OBJECT_BOMB:
            result.push_back({glm::vec3(0.0f, 0.0f, 0.0f), 3.0f, 0.45f});
            break;

        case OBJECT_BAG:
            result.push_back({glm::vec3(0.0f, 0.0f, 0.0f), 4.0f, 0.45f});
            break;

        case OBJECT_STONE:
        case OBJECT_URANIUM:
        case OBJECT_METAL:
        case OBJECT_POWER:
        case OBJECT_ATOMIC:
        case OBJECT_BULLET:
        case OBJECT_BBOX:
        case OBJECT_KEYa:
        case OBJECT_KEYb:
        case OBJECT_KEYc:
        case OBJECT_KEYd:
        case OBJECT_TNT:
        case OBJECT_WINFIRE:
            result.push_back({glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.45f});
            break;

        default:
            break;
    }

    return result;
}

std::vector<ModeledBlueprint::ModelNode> ModelNodes(ObjectType type)
{
    std::vector<ModeledBlueprint::ModelNode> result;

    std::string name;
    if ( type == OBJECT_STONE       )  name = "stone";
    if ( type == OBJECT_URANIUM     )  name = "uranium";
    if ( type == OBJECT_METAL       )  name = "metal";
    if ( type == OBJECT_BULLET      )  name = "bullet";
    if ( type == OBJECT_BBOX        )  name = "bbox";
    if ( type == OBJECT_KEYa        )  name = "keya";
    if ( type == OBJECT_KEYb        )  name = "keyb";
    if ( type == OBJECT_KEYc        )  name = "keyc";
    if ( type == OBJECT_KEYd        )  name = "keyd";
    if ( type == OBJECT_TNT         )  name = "tnt";
    if ( type == OBJECT_BOMB        )  name = "bomb";
    if ( type == OBJECT_WAYPOINT    )  name = "waypoint";
    if ( type == OBJECT_SHOW        )  name = "show";
    if ( type == OBJECT_WINFIRE     )  name = "winfire";
    if ( type == OBJECT_BAG         )  name = "bag";
    if ( type == OBJECT_MARKSTONE   )  name = "cross1";
    if ( type == OBJECT_MARKPOWER   )  name = "cross2";
    if ( type == OBJECT_MARKURANIUM )  name = "cross3";
    if ( type == OBJECT_MARKKEYa    )  name = "crossa";
    if ( type == OBJECT_MARKKEYb    )  name = "crossb";
    if ( type == OBJECT_MARKKEYc    )  name = "crossc";
    if ( type == OBJECT_MARKKEYd    )  name = "crossd";
    if ( type == OBJECT_EGG         )  name = "egg";
    if ( name.size() > 0 )
    {
        result.push_back({0, -1, name});
        return result;
    }

    if ( type == OBJECT_POWER       )  name = "power";
    if ( type == OBJECT_ATOMIC      )  name = "atomic";
    if ( name.size() > 0 )
    {
        result.push_back({0, -1, name, false, true});
        return result;
    }

    return result;
}

} // namespace

using B = ModeledBlueprint;

template<>
B GetBlueprintHardcode<B>(ObjectType type)
{
    B hardcode;
    hardcode.gfxType = GfxType(type);

    hardcode.createAt = CreateAt(type);
    hardcode.shadowCircle = ShadowCircle(type);
    hardcode.cameraCollision = CameraCollision(type);

    hardcode.crashSpheres = CrashSpheres(type);
    hardcode.modelNodes = ModelNodes(type);
    return hardcode;
}
