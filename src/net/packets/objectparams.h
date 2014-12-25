/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "net/packets/serialize.h"

#include "object/object.h"
#include "object/objman.h"

#include "physics/physics.h"

#include <boost/serialization/array.hpp>

struct ObjectParams {
    int id;
    ObjectType type;
    Math::Vector pos[OBJECTMAXPART];
    Math::Vector angle[OBJECTMAXPART];
    Math::Vector speed;
    float energy;
    float shield;
    int power;
    int load;
    int loadLink;
    
    template<typename Archive>
    void serialize(Archive& ar, unsigned int version)
    {
        ar & id & type & pos & angle & speed & energy & shield & power & load & loadLink;
    }
    
    static ObjectParams FromObject(CObject* o)
    {
        ObjectParams op;
        op.id = o->GetID();
        op.type = o->GetType();
        for(int i = 0; i < OBJECTMAXPART; i++)
        {
            op.pos[i] = o->GetPosition(i);
            op.angle[i] = o->GetAngle(i);
        }
        if(o->GetPhysics() != nullptr)
            op.speed = o->GetPhysics()->GetMotorSpeed();
        else
            op.speed = Math::Vector();
        op.energy = o->GetEnergy();
        op.shield = o->GetShield();
        if(o->GetPower() != nullptr)
            op.power = o->GetPower()->GetID();
        else
            op.power = -1;
        if(o->GetFret() != nullptr)
            op.load = o->GetFret()->GetID();
        else
            op.load = -1;
        op.loadLink = o->GetTruckPart();
        return op;
    }
    
    bool Apply(bool linkObjects = true)
    {
        CObject* obj = CObjectManager::GetInstancePointer()->GetObjectById(id);
        if(obj == nullptr) return false;
        for(int i = 0; i < OBJECTMAXPART; i++)
        {
            obj->SetPosition(i, pos[i]);
            obj->SetAngle(i, angle[i]);
        }
        if(obj->GetPhysics() != nullptr && !obj->GetSelect())
            obj->GetPhysics()->SetMotorSpeed(speed);
        obj->SetEnergy(energy);
        obj->SetShield(shield);
        if(linkObjects)
            LinkObjects();
        return true;
    }
    
    void LinkObjects()
    {
        CObjectManager* objman = CObjectManager::GetInstancePointer();
        CObject* obj = objman->GetObjectById(id);
        
        CObject* old_power = obj->GetPower();
        CObject* new_power = power < 0 ? nullptr : objman->GetObjectById(power);
        if(old_power != new_power)
        {
            if(old_power != nullptr) {
                old_power->SetTruck(nullptr);
            }
            obj->SetPower(new_power);
            if(new_power != nullptr) {
                new_power->SetTruck(obj);
            }
        }
        
        CObject* old_load = obj->GetFret();
        CObject* new_load = load < 0 ? nullptr : objman->GetObjectById(load);
        if(old_load != new_load)
        {
            if(old_load != nullptr) {
                old_load->SetTruck(nullptr);
            }
            obj->SetFret(new_load);
            if(new_load != nullptr) {
                new_load->SetTruck(obj);
            }
        }
        
        obj->SetTruckPart(loadLink);
    }
};