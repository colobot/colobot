/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file graphics/engine/pyro_manager.h
 * \brief Manager for CPyro objects
 */

#pragma once

#include "graphics/engine/pyro_type.h"

#include <memory>
#include <set>

struct Event;
class CObject;

namespace Gfx
{

class CPyro;
using CPyroUPtr = std::unique_ptr<CPyro>;

class CPyroManager
{
public:
    CPyroManager();
    ~CPyroManager();

    void Create(PyroType type, CObject* obj, float force=1.0f);
    void DeleteAll();

    void CutObjectLink(CObject* obj);

    void EventProcess(const Event& event);

private:
    std::set<CPyroUPtr> m_pyros;
};

} // namespace Gfx
