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


#include "object/auto/auto.h"

#include "graphics/core/color.h"

#include <set>
#include <filesystem>

class CObject;

enum AutoBaseParam
{
    PARAM_STOP      = 0,        // run=0 -> stops and open
    PARAM_LANDING   = 1,        // run=1 -> landing
    PARAM_PORTICO   = 2,        // run=2 -> gate on the ground
    PARAM_FIXSCENE  = 3,        // run=3 -> open and stops to win / lost
    PARAM_TRANSIT1  = 11,       // run=11 -> transit in space
    PARAM_TRANSIT2  = 12,       // run=12 -> transit in space
    PARAM_TRANSIT3  = 13        // run=13 -> transit in space
};

enum AutoBasePhase
{
    ABP_WAIT        = 1,    // expected
    ABP_START       = 2,    // start-up

    ABP_LAND        = 3,    // landing
    ABP_OPENWAIT        = 4,    // wait before opening
    ABP_OPEN        = 5,    // opens the gate
    ABP_OPEN2       = 6,    // opens supplements
    ABP_LDWAIT      = 7,    // expected

    ABP_CLOSE2      = 8,    // closes supplements
    ABP_CLOSE       = 9,    // closes gate
    ABP_TOWAIT      = 10,   // wait before takeoff
    ABP_TAKEOFF     = 11,   // take-off

    ABP_PORTICO_MOVE = 12,  // gate advance
    ABP_PORTICO_WAIT1= 13,  // gate expected
    ABP_PORTICO_DOWN = 14,  // gate down
    ABP_PORTICO_WAIT2= 15,  // gate expected
    ABP_PORTICO_OPEN = 16,  // gate opens

    ABP_TRANSIT_MOVE = 17,  // transit - moving
};



class CAutoBase : public CAuto
{
public:
    CAutoBase(COldObject* object);
    ~CAutoBase();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    void        Start(int param) override;
    bool        EventProcess(const Event &event) override;
    bool        Abort() override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

    Error       TakeOff(bool printMsg);

protected:
    void        UpdateInterface();
    void        FreezeCargo(bool freeze);
    void        MoveCargo();
    Error       CheckCloseDoor();
    void        BeginTransit();
    void        EndTransit();

protected:
    AutoBasePhase   m_phase = ABP_WAIT;
    bool            m_bOpen = false;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_lastParticle = 0.0f;
    float           m_lastMotorParticle = 0.0f;
    float           m_fogStart = 0.0f;
    float           m_deepView = 0.0f;
    glm::vec3       m_pos = { 0, 0, 0 };
    glm::vec3       m_finalPos = { 0, 0, 0 };
    glm::vec3       m_lastPos = { 0, 0, 0 };
    int             m_param = 0;
    int             m_soundChannel = 0;
    int             m_partiChannel[8] = {};

    std::filesystem::path m_bgBack;
    std::filesystem::path m_bgName;
    Gfx::Color      m_bgUp;
    Gfx::Color      m_bgDown;
    Gfx::Color      m_bgCloudUp;
    Gfx::Color      m_bgCloudDown;
    std::set<CObject*> m_cargoObjects;
};
