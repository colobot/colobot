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


class CObject;

enum AutoNestPhase
{
    ANP_WAIT        = 1,
    ANP_BIRTH       = 2,    // appearance of a ball
};



class CAutoNest : public CAuto
{
public:
    CAutoNest(COldObject* object);
    ~CAutoNest();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    bool        SearchFree(Math::Vector pos);
    void        CreateCargo(Math::Vector pos, float angle, ObjectType type);
    CObject*    SearchCargo();

protected:
    AutoNestPhase   m_phase = ANP_WAIT;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_lastParticle = 0.0f;
    Math::Vector    m_cargoPos;
};
