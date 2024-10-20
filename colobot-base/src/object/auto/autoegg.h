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
#include <filesystem>

class CObject;

enum AutoEggPhase
{
    AEP_NULL    = 0,
    AEP_DELAY   = 1,
    AEP_INCUB   = 3,
    AEP_ZOOM    = 4,
    AEP_WAIT    = 5,
};



class CAutoEgg : public CAuto
{
public:
    CAutoEgg(COldObject* object);
    ~CAutoEgg();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    void        Start(int param) override;
    bool        EventProcess(const Event &event) override;
    Error       IsEnded() override;
    Error       GetError() override;

    bool        SetType(ObjectType type) override;
    bool        SetValue(int rank, float value) override;
    //! Sets program which will be run by created aliens
    // TODO: rename to be more meanigful
    bool        SetString(const std::string& string) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    CObject*    SearchAlien();

protected:
    ObjectType      m_type = OBJECT_NULL;
    float           m_value = 0.0f;
    std::filesystem::path m_alienProgramName;
    int             m_param = 0;
    AutoEggPhase    m_phase = AEP_NULL;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
};
