/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

// slider.h

#pragma once


#include "ui/control.h"
#include "common/event.h"

namespace Ui {

class CButton;

class CSlider : public CControl
{
public:
    CSlider();
    ~CSlider();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);

    void        SetPos(Math::Point pos);
    void        SetDim(Math::Point dim);

    bool        SetState(int state, bool bState);
    bool        SetState(int state);
    bool        ClearState(int state);

    bool        EventProcess(const Event &event);
    void        Draw();

    void        SetLimit(float min, float max);

    void        SetVisibleValue(float value);
    float       GetVisibleValue();

    void        SetArrowStep(float step);
    float       GetArrowStep();

protected:
    void        MoveAdjust();
    void        AdjustGlint();
    void        DrawVertex(Math::Point pos, Math::Point dim, int icon);

protected:
    CButton*    m_buttonLeft;
    CButton*    m_buttonRight;

    float       m_min;
    float       m_max;
    float       m_visibleValue;
    float       m_step;

    bool        m_bHoriz;
    float       m_marginButton;

    bool        m_bCapture;
    Math::Point     m_pressPos;
    float       m_pressValue;

    EventType    m_eventUp;
    EventType    m_eventDown;
};


}

