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

#include "graphics/pyro/pyro.h"

#include "object/old_object.h"

#include "sound/sound.h"

// old_object.h only needed for OBJECTMAXPART - TODO improve this

using namespace Gfx;

CEggPyro::CEggPyro(CObject *obj)
    : CPyro(obj)
{}

bool CEggPyro::EventProcess(const Event& event)
{
    // Destroys the object that exploded.
    //It should not be destroyed at the end of the Create,
    //because it is sometimes the object itself that makes the Create:
    //  pyro->Create(PT_FRAGT, this);
    DeleteObject(true, true);

    return CPyro::EventProcess(event);
}

void CEggPyro::AfterCreate()
{
    m_sound->Play(SOUND_EGG, m_pos);

    for (int part = 0; part < OBJECTMAXPART; part++)
    {
        CreateTriangle(m_object, m_object->GetType(), part, 5, 15, 20, 40, 0.30f);
    }
}
