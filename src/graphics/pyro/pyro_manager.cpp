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

#include "graphics/pyro/pyro_manager.h"

#include "common/make_unique.h"

#include "graphics/pyro/pyro.h"

#include <assert.h>

namespace Gfx
{


Gfx::CPyroManager::CPyroManager()
{}

CPyroManager::~CPyroManager()
{}

void Gfx::CPyroManager::Create(PyroType type, CObject* obj, float force)
{
    std::unique_ptr<CPyro> pyroUPtr;
    switch (type)
    {
    case PT_FRAGT:
    case PT_FRAGO:
    case PT_FRAGW:
    case PT_EXPLOT:
    case PT_EXPLOO:
    case PT_EXPLOW:
    case PT_SHOTT:
    case PT_SHOTH:
    case PT_SHOTM:
    case PT_SHOTW:
        pyroUPtr = MakeUnique<CFragExploOrShotPyro>();
        break;
    case PT_BURNT:
    case PT_BURNO:
        pyroUPtr = MakeUnique<CBurnPyro>();
        break;
    case PT_FLCREATE: pyroUPtr = MakeUnique<CFlagCreatePyro>(); break;
    case PT_FLDELETE: pyroUPtr = MakeUnique<CFlagDeletePyro>(); break;
    case PT_WPCHECK:  pyroUPtr = MakeUnique<CWaypointHitPyro>(); break;
    case PT_FALL:     pyroUPtr = MakeUnique<CFallPyro>(); break;
    case PT_RESET:    pyroUPtr = MakeUnique<CResetPyro>(); break;
    case PT_LOST:     pyroUPtr = MakeUnique<CLostPyro>(); break;
    case PT_WIN:      pyroUPtr = MakeUnique<CWinPyro>(); break;
    case PT_SPIDER:   pyroUPtr = MakeUnique<CSpiderPyro>(); break;
    case PT_DEADG:    pyroUPtr = MakeUnique<CDeadGPyro>(); break;
    case PT_DEADW:    pyroUPtr = MakeUnique<CDeadWPyro>(); break;
    case PT_FINDING:  pyroUPtr = MakeUnique<CFindingPyro>(); break;
    case PT_SQUASH:   pyroUPtr = MakeUnique<CSquashPyro>(); break;
    case PT_FRAGV:    pyroUPtr = MakeUnique<CFragVPyro>(); break;
    default:
        // shouldn't get here
        assert(false);
        pyroUPtr = MakeUnique<CPyro>();
        break;
    }
    pyroUPtr->Create(type, obj, force);
    m_pyros.insert(std::move(pyroUPtr));
}

void CPyroManager::DeleteAll()
{
    for (auto& pyro : m_pyros)
    {
        pyro->DeleteObject();
    }

    m_pyros.clear();
}

void Gfx::CPyroManager::CutObjectLink(CObject* obj)
{
    for (auto& pyro : m_pyros)
    {
        pyro->CutObjectLink(obj);
    }
}

void Gfx::CPyroManager::EventProcess(const Event& event)
{
    auto it = m_pyros.begin();
    while (it != m_pyros.end())
    {
        (*it)->EventProcess(event);
        if ((*it)->IsEnded() == ERR_CONTINUE)
        {
            ++it;
        }
        else
        {
            (*it)->DeleteObject();
            it = m_pyros.erase(it);
        }
    }
}

} // namespace Gfx
