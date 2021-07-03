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

#include "graphics/engine/particle.h"

#include "graphics/pyro/pyro.h"
#include "graphics/pyro/pyro_manager.h"

#include "object/object.h"

using namespace Gfx;

CSquashPyro::CSquashPyro(CObject *obj)
    : CPyro(PT_OTHER, obj)
{}

void CSquashPyro::AfterCreate()
{
    m_speed = 1.0f/2.0f;
    m_object->SetLock(true);
}

void CSquashPyro::UpdateEffect()
{
    if ( m_object != nullptr )
    {
        m_object->SetScaleY(1.0f-sinf(m_progress)*0.5f);
    }
}

void CSquashPyro::AfterEnd()
{
    if ( m_object != nullptr )
    {
        m_object->SetType(OBJECT_PLANT19);
    }
}
