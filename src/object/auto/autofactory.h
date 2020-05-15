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

enum AutoFactoryPhase
{
    AFP_WAIT        = 1,    // expected metal
    AFP_CLOSE_S     = 2,    // closes doors (shift)
    AFP_CLOSE_T     = 3,    // closes doors (turn)
    AFP_BUILD       = 4,    // building the vehicle
    AFP_OPEN_T      = 5,    // opens the doors (turn)
    AFP_OPEN_S      = 6,    // opens the doors (shift)
    AFP_ADVANCE     = 7,    // advance at the door
};



class CAutoFactory : public CAuto
{
public:
    CAutoFactory(COldObject* object);
    ~CAutoFactory();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;

    Error       StartAction(int param) override;
    void        SetProgram(const std::string& program);

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    void        UpdateInterface();
    void        UpdateButton(Ui::CWindow *pw, EventType event, bool bBusy);

    CObject*    SearchCargo();
    bool        NearestVehicle();
    bool        CreateVehicle();
    CObject*    SearchVehicle();

    void        SoundManip(float time, float amplitude, float frequency);

protected:
    AutoFactoryPhase    m_phase = AFP_WAIT;
    float               m_progress = 0.0f;
    float               m_speed = 0.0f;
    float               m_lastParticle = 0.0f;
    Math::Vector        m_cargoPos;
    int                 m_channelSound = 0;

    std::string         m_program;
};
