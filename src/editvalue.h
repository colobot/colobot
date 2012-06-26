// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

#ifndef _EDITVALUE_H_
#define _EDITVALUE_H_


#include "control.h"


enum EditValueType
{
    EVT_INT     = 1,    // integer
    EVT_FLOAT   = 2,    // float value
    EVT_100     = 3,    // percent (0 .. 1)
};


class CD3DEngine;
class CEdit;
class CButton;



class CEditValue : public CControl
{
public:
    CEditValue(CInstanceManager* iMan);
    virtual ~CEditValue();

    BOOL        Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

    void        SetPos(FPOINT pos);
    void        SetDim(FPOINT dim);

    BOOL        EventProcess(const Event &event);
    void        Draw();

    void        SetType(EditValueType type);
    EditValueType RetType();

    void        SetValue(float value, BOOL bSendMessage=FALSE);
    float       RetValue();

    void        SetStepValue(float value);
    float       RetStepValue();

    void        SetMinValue(float value);
    float       RetMinValue();

    void        SetMaxValue(float value);
    float       RetMaxValue();

protected:
    void        MoveAdjust();
    void        HiliteValue(const Event &event);

protected:
    CEdit*      m_edit;
    CButton*    m_buttonUp;
    CButton*    m_buttonDown;

    EditValueType m_type;
    float       m_stepValue;
    float       m_minValue;
    float       m_maxValue;
};


#endif //_EDITVALUE_H_
