/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/subclass/buzzingcars/bc_piece.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

CBCPiece::CBCPiece(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCPiece::~CBCPiece()
{}

std::unique_ptr<CBCPiece> CBCPiece::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCPiece>(params.id, params.type);

    obj->SetTeam(params.team);

    bool bPlumb = params.bPlumb;

    char name[100]; // TODO (krzys_h): C strings...
    sprintf(name, "buzzingcars/piece%d.mod", params.type-OBJECT_PIECE0);
    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
    obj->SetObjectRank(0, rank);
    modelManager->AddModelReference(name, false, rank, params.team);
    obj->SetPartScale(0, params.zoom);
    obj->SetPartPosition(0, params.pos);
    obj->SetPartRotationY(0, params.angle);

    obj->CreateShadowCircle(1.0f, 0.5f);

    obj->SetFloorHeight(0.0f);

    engine->LoadAllTextures();
    if ( !bPlumb )  obj->FloorAdjust();

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    return obj;
}
