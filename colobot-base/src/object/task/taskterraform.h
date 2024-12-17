/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/task/task.h"

#include <glm/glm.hpp>


enum TaskTerraPhase
{
    TTP_CHARGE  = 1,    // charge of energy
    TTP_DOWN    = 2,    // down
    TTP_TERRA   = 3,    // strike
    TTP_UP      = 4,    // up
};



class CTaskTerraform : public CForegroundTask
{
public:
    CTaskTerraform(COldObject* object);
    ~CTaskTerraform();

    bool        EventProcess(const Event &event) override;

    Error       Start();
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    bool        Terraform();

protected:
    TaskTerraPhase  m_phase = TTP_CHARGE;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_time = 0.0f;
    float           m_lastParticle = 0.0f;
    int             m_soundChannel = 0;
    Error           m_error = ERR_OK;
    glm::vec3       m_terraPos = { 0, 0, 0 };
};
