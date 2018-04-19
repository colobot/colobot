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

#pragma once


#include "object/auto/auto.h"

#include "graphics/engine/particle.h"



struct HustonLens
{
    int                 parti = 0;
    Gfx::ParticleType   type = {};
    Math::Vector        pos;
    float               dim = 0.0f;
    float               total = 0.0f;
    float               off = 0.0f;
};


const int HUSTONMAXLENS = 20;


class CAutoHouston : public CAuto
{
public:
    CAutoHouston(COldObject* object);
    ~CAutoHouston();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    void        Start(int param) override;
    bool        EventProcess(const Event &event) override;
    bool        Abort() override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

protected:
    float       m_progress = 0.0f;
    float       m_speed = 0.0f;
    HustonLens  m_lens[HUSTONMAXLENS];
    int         m_lensTotal = 0;
};
