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

enum AutoDerrickPhase
{
    ADP_WAIT        = 1,
    ADP_EXCAVATE        = 2,    // down the drill
    ADP_ASCEND      = 3,    // up the drill
    ADP_EXPORT      = 4,    // exports matter
    ADP_ISFREE      = 5,    // expected material loss
};



class CAutoDerrick : public CAuto
{
public:
    CAutoDerrick(COldObject* object);
    ~CAutoDerrick();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    CObject*    SearchCargo();
    bool        SearchFree(Math::Vector pos);
    void        CreateCargo(Math::Vector pos, float angle, ObjectType type, float height);
    bool        ExistKey();

    Math::Vector GetCargoPos();

protected:
    AutoDerrickPhase    m_phase = ADP_WAIT;
    float               m_progress = 0.0f;
    float               m_speed = 0.0f;
    float               m_timeVirus = 0.0f;
    float               m_lastParticle = 0.0f;
    float               m_lastTrack = 0.0f;
    int                 m_soundChannel = 0;
    bool                m_bSoundFall = false;
};
