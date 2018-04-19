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

#include "object/subclass/base_robot.h"

struct ObjectCreateParams;

namespace Gfx
{
class COldModelManager;
class CEngine;
}

class CShielder : public CBaseRobot
{
public:
    CShielder(int id);
    virtual ~CShielder();

public:
    static std::unique_ptr<CShielder> Create(
        const ObjectCreateParams& params,
        Gfx::COldModelManager* modelManager,
        Gfx::CEngine* engine);

public:
    //! Shielder radius (only while active) [0 or RADIUS_SHIELD_MIN..RADIUS_SHIELD_MAX]
    float GetActiveShieldRadius();
    //! Shielder radius [0..1]
    //@{
    void  SetShieldRadius(float shieldRadius);
    float GetShieldRadius();
    //@}

    void Write(CLevelParserLine* line) override;
    void Read(CLevelParserLine* line) override;

protected:
    float m_shieldRadius;
};
