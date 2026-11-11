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

#include "blueprint/blueprint_parser.h"
#include "blueprint/part/modeled_blueprint.h"

using B = ModeledBlueprint;

template<>
void ConfigBlueprintParser<B>(BlueprintParser<B>& p)
{
    p.line("Modeled");
    p.param("gfxType", AsString, &B::gfxType);

    p.line("Modeled:CreateAt", &B::createAt);
    p.param("isFixedPosition", AsBool,  &B::CreateAt::isFixedPosition);
    p.param("extraHeight",     AsFloat, &B::CreateAt::extraHeight);
    p.param("extraScale",      AsFloat, &B::CreateAt::extraScale);

    p.line("Modeled:ShadowCircle", &B::shadowCircle);
    p.param("radius",    AsFloat, &B::ShadowCircle::radius);
    p.param("intensity", AsFloat, &B::ShadowCircle::intensity);

    p.line("Modeled:CameraCollision", &B::cameraCollision);
    p.param("pos",    AsPoint, &Math::Sphere::pos);
    p.param("radius", AsFloat, &Math::Sphere::radius);

    p.list("Modeled:CrashSphere", &B::crashSpheres);
    p.param("pos",      AsPoint, &B::CrashSphere::pos);
    p.param("radius",   AsFloat, &B::CrashSphere::radius);
    p.param("hardness", AsFloat, &B::CrashSphere::hardness);

    p.list("Modeled:ModelNode", &B::modelNodes);
    p.param("partId",     AsInt,    &B::ModelNode::partId);
    p.param("parentId",   AsInt,    &B::ModelNode::parentId);
    p.param("file",       AsString, &B::ModelNode::file);
    p.param("isMirrored", AsBool,   &B::ModelNode::isMirrored);
    p.param("isCopy",     AsBool,   &B::ModelNode::isCopy);
}
