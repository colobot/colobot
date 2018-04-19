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

#include "ui/controls/control.h"

#include "common/event.h"

#include <memory>

namespace Ui
{

class CButton;

const float SCROLL_WIDTH = (15.0f/640.0f);



class CScroll : public CControl
{
public:
    CScroll();
    ~CScroll();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg) override;

    void        SetPos(Math::Point pos) override;
    void        SetDim(Math::Point dim) override;

    bool        SetState(int state, bool bState) override;
    bool        SetState(int state) override;
    bool        ClearState(int state) override;

    bool        EventProcess(const Event &event) override;
    void        Draw() override;

    void        SetVisibleValue(float value);
    float       GetVisibleValue();

    void        SetVisibleRatio(float value);
    float       GetVisibleRatio();

    void        SetArrowStep(float step);
    float       GetArrowStep();

protected:
    void        MoveAdjust();
    void        AdjustGlint();
    void        DrawVertex(Math::Point pos, Math::Point dim, int icon);

protected:
    std::unique_ptr<CButton> m_buttonUp;
    std::unique_ptr<CButton> m_buttonDown;

    float       m_visibleValue;
    float       m_visibleRatio;
    float       m_step;

    bool        m_bCapture;
    Math::Point     m_pressPos;
    float       m_pressValue;
};


} // namespace Ui
