/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/engine/camera.h"

#include "graphics/pyro/pyro.h"

#include "object/object.h"

#include "object/interface/destroyable_object.h"
#include "object/interface/movable_object.h"

#include "object/motion/motion.h"
#include "object/motion/motionhuman.h"

#include "sound/sound.h"

using namespace Gfx;


CDeadGPyro::CDeadGPyro(CObject *obj)
    : CPyro(obj)
{}

void CDeadGPyro::AfterCreate()
{
    if (m_object->GetType() == OBJECT_HUMAN)
    {
        m_sound->Play(SOUND_DEADg, m_pos);
    }


    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    dynamic_cast<CDestroyableObject&>(*m_object).SetDying(DeathType::Dead);

    assert(m_object->Implements(ObjectInterfaceType::Movable));
    dynamic_cast<CMovableObject&>(*m_object).GetMotion()->SetAction(MHS_DEADg, 1.0f);

    m_camera->StartCentering(m_object, Math::PI*0.5f, 99.9f, 0.0f, 1.5f);
    m_camera->StartOver(CAM_OVER_EFFECT_FADEOUT_WHITE, m_pos, 1.0f);
    m_speed = 1.0f/10.0f;
}

CDeadWPyro::CDeadWPyro(CObject *obj)
    : CPyro(obj)
{}

void CDeadWPyro::AfterCreate()
{
    if (m_object->GetType() == OBJECT_HUMAN)
    {
        m_sound->Play(SOUND_DEADw, m_pos);
    }

    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    dynamic_cast<CDestroyableObject&>(*m_object).SetDying(DeathType::Dead);

    assert(m_object->Implements(ObjectInterfaceType::Movable));
    dynamic_cast<CMovableObject&>(*m_object).GetMotion()->SetAction(MHS_DEADw, 1.0f);

    m_camera->StartCentering(m_object, Math::PI*0.5f, 99.9f, 0.0f, 3.0f);
    m_camera->StartOver(CAM_OVER_EFFECT_FADEOUT_BLACK, m_pos, 1.0f);
    m_speed = 1.0f/10.0f;
}
