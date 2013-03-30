// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// editvalue.h

#pragma once


#include "ui/control.h"


namespace Gfx{
class CEngine;
}

namespace Ui {

enum EditValueType
{
    EVT_INT     = 1,    // integer
    EVT_FLOAT   = 2,    // float value
    EVT_100     = 3,    // percent (0 .. 1)
};

class CEdit;
class CButton;



class CEditValue : public CControl
{
public:
    CEditValue();
    virtual ~CEditValue();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventType);

    void        SetPos(Math::Point pos);
    void        SetDim(Math::Point dim);

    bool        EventProcess(const Event &event);
    void        Draw();

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

protected:
    void        MoveAdjust();
    void        HiliteValue(const Event &event);

    Ui::CEdit*      m_edit;
    Ui::CButton*    m_buttonUp;
    Ui::CButton*    m_buttonDown;
    EditValueType   m_type;
    float           m_stepValue;
    float           m_minValue;
    float           m_maxValue;
};


}
