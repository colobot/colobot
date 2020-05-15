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

enum AutoVaultPhase
{
    ASAP_WAIT       = 1,
    ASAP_OPEN       = 2,
    ASAP_FINISH     = 3,
};



class CAutoVault : public CAuto
{
public:
    CAutoVault(COldObject* object);
    ~CAutoVault();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    int         CountKeys();
    void        LockKeys();
    void        DownKeys(float progress);
    void        DeleteKeys();
    CObject*    SearchVehicle();

protected:
    AutoVaultPhase   m_phase = ASAP_WAIT;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_timeVirus = 0.0f;
    float           m_lastParticle = 0.0f;
    int             m_channelSound = 0;
    bool            m_bLock = false;
    int             m_countKeys = 0;
    float           m_actualAngle = 0.0f;
    float           m_finalAngle = 0.0f;
    bool            m_bKey[4] = {};
    Math::Vector        m_keyPos[4];
    int             m_keyParti[4] = {};
};
