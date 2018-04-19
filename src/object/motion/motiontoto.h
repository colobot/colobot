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

#include "object/motion/motion.h"

#include "math/point.h"


enum MotionTotoAction
{
    MT_ERROR    = 0,
    MT_WARNING  = 1,
    MT_INFO     = 2,
    MT_MESSAGE  = 3
};


class CMotionToto : public CMotion
{
public:
    CMotionToto(COldObject* object);
    ~CMotionToto();

    void    DeleteObject(bool bAll=false) override;
    void    Create(Math::Vector pos, float angle, ObjectType type, float power, Gfx::COldModelManager* modelManager) override;
    bool    EventProcess(const Event &event) override;
    Error   SetAction(int action, float time=0.2f) override;

    void    StartDisplayInfo();
    void    StopDisplayInfo();
    void    SetMousePos(Math::Point pos);

protected:
    bool    EventFrame(const Event &event);

protected:
    float       m_time;
    float       m_lastMotorParticle;
    bool        m_bDisplayInfo;
    bool        m_bQuickPos;
    bool        m_bStartAction;
    float       m_speedAction;
    float       m_clownRadius;
    float       m_clownDelay;
    float       m_clownTime;
    float       m_blinkTime;
    float       m_blinkProgress;
    int         m_soundChannel;
    Math::Point     m_mousePos;
};
