/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <memory>

namespace Gfx
{
class CEngine;
}

namespace Ui
{

enum EditValueType
{
    EVT_INT     = 1,    // integer
    EVT_FLOAT   = 2,    // float value
    EVT_100     = 3,    // percent (0 .. 1)
};

class CEdit;
class CButton;
class CInterface;



class CEditValue : public CControl
{
public:
    CEditValue();
    virtual ~CEditValue();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventType) override;

    void        SetPos(Math::Point pos) override;
    void        SetDim(Math::Point dim) override;

    bool        EventProcess(const Event &event) override;
    void        Draw() override;

    void        SetType(EditValueType type);
    EditValueType GetType();

    void        SetValue(float value, bool bSendMessage=false);
    float       GetValue();

    void        SetStepValue(float value);
    float       GetStepValue();

    void        SetMinValue(float value);
    float       GetMinValue();

    void        SetMaxValue(float value);
    float       GetMaxValue();

    void        SetInterface(Ui::CInterface* interface);

protected:
    void        MoveAdjust();
    void        HiliteValue(const Event &event);

    Ui::CInterface* m_interface;
    std::unique_ptr<Ui::CEdit> m_edit;
    std::unique_ptr<Ui::CButton> m_buttonUp;
    std::unique_ptr<Ui::CButton> m_buttonDown;
    EditValueType   m_type;
    float           m_stepValue;
    float           m_minValue;
    float           m_maxValue;
};


}
