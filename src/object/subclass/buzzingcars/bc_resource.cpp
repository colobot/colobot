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

#include "object/subclass/buzzingcars/bc_resource.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/auto/buzzingcars/building/autobarrel.h"
#include "object/auto/buzzingcars/building/autobomb.h"

CBCResource::CBCResource(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCResource::~CBCResource()
{}

std::unique_ptr<CBCResource> CBCResource::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCResource>(params.id, params.type);

    obj->SetTeam(params.team);

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // c'est un objet fixe
    obj->SetObjectRank(0, rank);
    obj->GetCharacter()->mass = 1000.0f;

    bool bPlumb = params.bPlumb;

    std::string name = "";
//    if ( params.type == OBJECT_STONE       )  name = "buzzingcars/stone.mod";
//    if ( params.type == OBJECT_URANIUM     )  name = "buzzingcars/uranium.mod";
//    if ( params.type == OBJECT_METAL       )  name = "buzzingcars/metal.mod";
    if ( params.type == OBJECT_BARREL      )  name = "buzzingcars/barrel.mod";
    if ( params.type == OBJECT_BARRELa     )  name = "buzzingcars/barrela.mod";
//    if ( params.type == OBJECT_ATOMIC      )  name = "buzzingcars/atomic.mod";
//    if ( params.type == OBJECT_BULLET      )  name = "buzzingcars/bullet.mod";
//    if ( params.type == OBJECT_BBOX        )  name = "buzzingcars/bbox.mod";
//    if ( params.type == OBJECT_KEYa        )  name = "buzzingcars/keya.mod";
//    if ( params.type == OBJECT_KEYb        )  name = "buzzingcars/keyb.mod";
//    if ( params.type == OBJECT_KEYc        )  name = "buzzingcars/keyc.mod";
//    if ( params.type == OBJECT_KEYd        )  name = "buzzingcars/keyd.mod";
//    if ( params.type == OBJECT_TNT         )  name = "buzzingcars/tnt.mod";
//    if ( params.type == OBJECT_MINE        )  name = "buzzingcars/mine.mod";
    if ( params.type == OBJECT_POLE        )  name = "buzzingcars/pole.mod";
    if ( params.type == OBJECT_BOMB        )  name = "buzzingcars/bomb1.mod";
    if ( params.type == OBJECT_CONE        )  name = "buzzingcars/cone.mod";
    if ( params.type == OBJECT_PIPES       )  name = "buzzingcars/pipes.mod";
//    if ( params.type == OBJECT_WAYPOINT    )  name = "buzzingcars/waypoint.mod";
    if ( params.type == OBJECT_TRAJECT     )  name = "buzzingcars/traject.mod";
    if ( params.type == OBJECT_TARGET      )  name = "buzzingcars/xxx.mod";
//    if ( params.type == OBJECT_SHOW        )  name = "buzzingcars/show.mod";
//    if ( params.type == OBJECT_WINFIRE     )  name = "buzzingcars/winfire.mod";
//    if ( params.type == OBJECT_BAG         )  name = "buzzingcars/bag.mod";
    if ( params.type == OBJECT_MARK        )  name = "buzzingcars/mark.mod";
    if ( params.type == OBJECT_CROWN       )  name = "buzzingcars/crown.mod";
    if ( params.type == OBJECT_ROADSIGN1   )  name = "buzzingcars/road1.mod";
    if ( params.type == OBJECT_ROADSIGN2   )  name = "buzzingcars/road2.mod";
    if ( params.type == OBJECT_ROADSIGN3   )  name = "buzzingcars/road3.mod";
    if ( params.type == OBJECT_ROADSIGN4   )  name = "buzzingcars/road4.mod";
    if ( params.type == OBJECT_ROADSIGN5   )  name = "buzzingcars/road5.mod";
    if ( params.type == OBJECT_ROADSIGN6   )  name = "buzzingcars/road6.mod";
    if ( params.type == OBJECT_ROADSIGN7   )  name = "buzzingcars/road7.mod";
    if ( params.type == OBJECT_ROADSIGN8   )  name = "buzzingcars/road8.mod";
    if ( params.type == OBJECT_ROADSIGN9   )  name = "buzzingcars/road9.mod";
    if ( params.type == OBJECT_ROADSIGN10  )  name = "buzzingcars/road10.mod";
    if ( params.type == OBJECT_ROADSIGN11  )  name = "buzzingcars/road11.mod";
    if ( params.type == OBJECT_ROADSIGN12  )  name = "buzzingcars/road12.mod";
    if ( params.type == OBJECT_ROADSIGN13  )  name = "buzzingcars/road13.mod";
    if ( params.type == OBJECT_ROADSIGN14  )  name = "buzzingcars/road14.mod";
    if ( params.type == OBJECT_ROADSIGN15  )  name = "buzzingcars/road15.mod";
    if ( params.type == OBJECT_ROADSIGN16  )  name = "buzzingcars/road16.mod";
    if ( params.type == OBJECT_ROADSIGN17  )  name = "buzzingcars/road17.mod";
    if ( params.type == OBJECT_ROADSIGN18  )  name = "buzzingcars/road18.mod";
    if ( params.type == OBJECT_ROADSIGN19  )  name = "buzzingcars/road19.mod";
    if ( params.type == OBJECT_ROADSIGN20  )  name = "buzzingcars/road20.mod";
    if ( params.type == OBJECT_ROADSIGN21  )  name = "buzzingcars/road21.mod";
    if ( params.type == OBJECT_ROADSIGN22  )  name = "buzzingcars/road22.mod";
    if ( params.type == OBJECT_ROADSIGN23  )  name = "buzzingcars/road23.mod";
    if ( params.type == OBJECT_ROADSIGN24  )  name = "buzzingcars/road24.mod";
    if ( params.type == OBJECT_ROADSIGN25  )  name = "buzzingcars/road25.mod";
    if ( params.type == OBJECT_ROADSIGN26  )  name = "buzzingcars/road26.mod";
    if ( params.type == OBJECT_ROADSIGN27  )  name = "buzzingcars/road27.mod";
    if ( params.type == OBJECT_ROADSIGN28  )  name = "buzzingcars/road28.mod";
    if ( params.type == OBJECT_ROADSIGN29  )  name = "buzzingcars/road29.mod";
    if ( params.type == OBJECT_ROADSIGN30  )  name = "buzzingcars/road30.mod";
    if ( params.type == OBJECT_PUB11       )  name = "buzzingcars/pub11.mod";
    if ( params.type == OBJECT_PUB12       )  name = "buzzingcars/pub12.mod";
    if ( params.type == OBJECT_PUB13       )  name = "buzzingcars/pub13.mod";
    if ( params.type == OBJECT_PUB14       )  name = "buzzingcars/pub14.mod";
    if ( params.type == OBJECT_PUB21       )  name = "buzzingcars/pub21.mod";
    if ( params.type == OBJECT_PUB22       )  name = "buzzingcars/pub22.mod";
    if ( params.type == OBJECT_PUB23       )  name = "buzzingcars/pub23.mod";
    if ( params.type == OBJECT_PUB24       )  name = "buzzingcars/pub24.mod";
    if ( params.type == OBJECT_PUB31       )  name = "buzzingcars/pub31.mod";
    if ( params.type == OBJECT_PUB32       )  name = "buzzingcars/pub32.mod";
    if ( params.type == OBJECT_PUB33       )  name = "buzzingcars/pub33.mod";
    if ( params.type == OBJECT_PUB34       )  name = "buzzingcars/pub34.mod";
    if ( params.type == OBJECT_PUB41       )  name = "buzzingcars/pub41.mod";
    if ( params.type == OBJECT_PUB42       )  name = "buzzingcars/pub42.mod";
    if ( params.type == OBJECT_PUB43       )  name = "buzzingcars/pub43.mod";
    if ( params.type == OBJECT_PUB44       )  name = "buzzingcars/pub44.mod";
    if ( params.type == OBJECT_PUB51       )  name = "buzzingcars/pub51.mod";
    if ( params.type == OBJECT_PUB52       )  name = "buzzingcars/pub52.mod";
    if ( params.type == OBJECT_PUB53       )  name = "buzzingcars/pub53.mod";
    if ( params.type == OBJECT_PUB54       )  name = "buzzingcars/pub54.mod";
    if ( params.type == OBJECT_PUB61       )  name = "buzzingcars/pub61.mod";
    if ( params.type == OBJECT_PUB62       )  name = "buzzingcars/pub62.mod";
    if ( params.type == OBJECT_PUB63       )  name = "buzzingcars/pub63.mod";
    if ( params.type == OBJECT_PUB64       )  name = "buzzingcars/pub64.mod";
    if ( params.type == OBJECT_PUB71       )  name = "buzzingcars/pub71.mod";
    if ( params.type == OBJECT_PUB72       )  name = "buzzingcars/pub72.mod";
    if ( params.type == OBJECT_PUB73       )  name = "buzzingcars/pub73.mod";
    if ( params.type == OBJECT_PUB74       )  name = "buzzingcars/pub74.mod";
    if ( params.type == OBJECT_PUB81       )  name = "buzzingcars/pub81.mod";
    if ( params.type == OBJECT_PUB82       )  name = "buzzingcars/pub82.mod";
    if ( params.type == OBJECT_PUB83       )  name = "buzzingcars/pub83.mod";
    if ( params.type == OBJECT_PUB84       )  name = "buzzingcars/pub84.mod";
    if ( params.type == OBJECT_PUB91       )  name = "buzzingcars/pub91.mod";
    if ( params.type == OBJECT_PUB92       )  name = "buzzingcars/pub92.mod";
    if ( params.type == OBJECT_PUB93       )  name = "buzzingcars/pub93.mod";
    if ( params.type == OBJECT_PUB94       )  name = "buzzingcars/pub94.mod";
    if ( params.type == OBJECT_TRAXf       )  name = "buzzingcars/trax1f.mod";
    assert(!name.empty());

    float angle = params.angle;
    if ( (params.type >= OBJECT_ROADSIGN1  && params.type <= OBJECT_ROADSIGN30) ||
         (params.type >= OBJECT_PUB11      && params.type <= OBJECT_PUB94     ) )
    {
        angle -= Math::PI/2.0f;  // pour des raisons historiques !
    }

    modelManager->AddModelReference(name, false, rank, params.team);

    obj->SetPosition(params.pos);
    obj->SetRotationY(angle);
    obj->SetScale(params.zoom);

    if ( params.type == OBJECT_SHOW )  // reste en l'air ?
    {
        return obj;
    }

    float radius  = 1.5f;
    float density = 1.0f;
    float height  = 0.0f;

    if ( params.type == OBJECT_BOMB )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/bomb2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 4.2f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/bomb2.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 2.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("buzzingcars/bomb2.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(0.0f, 2.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 3);
        modelManager->AddModelReference("buzzingcars/bomb2.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(0.0f, 2.0f, 0.0f));
    }

    if ( params.type == OBJECT_WAYPOINT    )
    {
    }
    else if ( params.type == OBJECT_TARGET )
    {
        radius = 0.0f;
    }
    else if ( params.type == OBJECT_BOMB )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 1.0f, 0.0f), 2.0f));
        radius = 3.0f;
    }
//    else if ( params.type == OBJECT_MINE )
//    {
//        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, -1.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f));
//        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f));
//        radius = 3.0f;
//    }
    else if ( params.type == OBJECT_POLE )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.5f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.5f));
        radius = 3.0f;
    }
//    else if ( params.type == OBJECT_BAG )
//    {
//        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
//        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f));
//        obj->SetScale(1.5f);
//        radius =  5.0f;
//        height = -1.4f;
//    }
    else if ( params.type == OBJECT_MARK )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 4.0f), 6.0f, SOUND_BOUMm, 0.45f));
        radius =  10.0f;
        density = 0.6f;
    }
    else if ( params.type == OBJECT_CROWN )
    {
        Math::Vector    z;
        z = obj->GetScale();
        z.y *= 0.8f;
        obj->SetScale(z);
        radius =  12.0f;
        density = 0.8f;
    }
    else if ( params.type == OBJECT_BARREL  ||
              params.type == OBJECT_BARRELa )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f));
        radius  = 3.0f;
        density = 0.6f;
        obj->GetCharacter()->mass = 1500.0f;
    }
    else if ( params.type == OBJECT_CONE )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMv, 0.45f));
        radius  = 1.2f;
        density = 1.0f;
        obj->GetCharacter()->mass = 700.0f;
    }
    else if ( params.type == OBJECT_PIPES )
    {
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -6.0f), SOUND_CHOCm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f, -6.0f), SOUND_CHOCm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f,  6.0f), SOUND_CHOCm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f,  6.0f), SOUND_CHOCm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -6.0f), SOUND_CHOCm, 0.45f));
        radius  = 0.0f;
        obj->CreateShadowCircle(20.0f, 1.0f, Gfx::ENG_SHADOW_WORM);
    }
    else if ( params.type >= OBJECT_ROADSIGN1 &&
              params.type <= OBJECT_ROADSIGN5 )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector( 4.0f, 3.0f, 0.0f), 2.0f, SOUND_CHOCo, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 2.0f, SOUND_CHOCo, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-3.0f, 3.0f, 0.0f), 2.0f, SOUND_CHOCo, 0.45f));
        radius  = 5.0f;
        density = 0.3f;
    }
    else if ( params.type >= OBJECT_ROADSIGN6  &&
              params.type <= OBJECT_ROADSIGN21 )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 2.5f, SOUND_CHOCo, 0.45f));
        radius  = 2.0f;
        density = 0.3f;
        bPlumb  = true;  // toujours droit
    }
    else if ( params.type >= OBJECT_ROADSIGN22 &&
              params.type <= OBJECT_ROADSIGN25 )
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f,  32.5f), 3.0f, SOUND_CHOCm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, -32.5f), 3.0f, SOUND_CHOCm, 0.45f));
        radius  = 0.0f;
        bPlumb  = true;  // toujours droit
    }
    else if ( params.type >= OBJECT_PUB11 &&
              params.type <= OBJECT_PUB94 )
    {
        obj->SetScale(0.5f*params.zoom);  // pour éclaircir l'image !!!
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, -1.0f), SOUND_CHOCo, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 15.0f, -1.0f), SOUND_CHOCo, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 15.0f,  1.0f), SOUND_CHOCo, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f,  1.0f), SOUND_CHOCo, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, -1.0f), SOUND_CHOCo, 0.45f));
        radius  = 0.0f;
        bPlumb  = true;  // toujours droit
    }
    else if ( params.type == OBJECT_TRAXf )
    {
        // Crée la chenille droite.
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/trax2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(-1.0f, 3.0f, -4.0f));

        // Crée la chenille gauche.
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/trax3.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(-1.0f, 3.0f, 4.0f));

        // Crée la pelle.
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        modelManager->AddModelReference("buzzingcars/trax4.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(-1.0f, 4.0f, 0.0f));

        // Crée le levier droite.
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 0);
        modelManager->AddModelReference("buzzingcars/trax5.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(-3.5f, 8.0f, -3.0f));

        // Crée le levier gauche.
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 0);
        modelManager->AddModelReference("buzzingcars/trax5.mod", false, rank, params.team);
        obj->SetPartPosition(5, Math::Vector(-3.5f, 8.0f, 3.0f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 11.0f, SOUND_CHOCo, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 4.0f, 0.0f), 14.0f));
        radius = 10.0f;
    }
    else if ( params.type == OBJECT_TRAJECT )
    {
        radius = 0.0f;
    }
    else
    {
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.5f));
    }

    if ( radius != 0.0f )
    {
        obj->CreateShadowCircle(radius, density);
    }

    obj->SetFloorHeight(0.0f);

    std::unique_ptr<CAuto> objAuto;
    if ( params.type == OBJECT_BARREL    ||
         params.type == OBJECT_BARRELa   )
    {
        objAuto = MakeUnique<CAutoBarrel>(obj.get());
    }
    if ( params.type == OBJECT_BOMB )
    {
        objAuto = MakeUnique<CAutoBomb>(obj.get());
    }

    if (objAuto != nullptr)
    {
        objAuto->Init();
        obj->SetAuto(std::move(objAuto));
    }

    engine->LoadAllTextures();
    if ( !bPlumb )  obj->FloorAdjust();

    Math::Vector pos = obj->GetPosition();
    pos.y += height;
    obj->SetPosition(pos);  // to display the shadows immediately

    return obj;
}
