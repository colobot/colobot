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

#include "common/error.h"
#include "common/event.h"

#include <array>

class CObject;
class CSoundInterface;

namespace Gfx
{
class CEngine;
}

namespace Ui
{

class CInterface;

enum TextType
{
    TT_ERROR    = 1,
    TT_WARNING  = 2,
    TT_INFO     = 3,
    TT_MESSAGE  = 4,
};

const int MAXDTLINE = 4;


class CDisplayText
{
public:
    CDisplayText();
    ~CDisplayText();

    void        DeleteObject();

    bool        EventProcess(const Event &event);

    void        DisplayError(Error err, CObject* pObj, float time=10.0f);
    void        DisplayError(Error err, Math::Vector goal, float height=15.0f, float dist=60.0f, float time=10.0f);
    void        DisplayText(const char *text, CObject* pObj, float time=10.0f, TextType type=TT_INFO);
    void        DisplayText(const char *text, Math::Vector goal, float height=15.0f, float dist=60.0f, float time=10.0f, TextType type=TT_INFO);
    void        HideText(bool bHide);
    void        ClearText();
    bool        ClearLastText();
    void        SetDelay(float factor);
    void        SetEnable(bool bEnable);

    Math::Vector    GetVisitGoal(EventType event);
    float       GetVisitDist(EventType event);
    float       GetVisitHeight(EventType event);

    float       GetIdealDist(CObject* pObj);
    float       GetIdealHeight(CObject* pObj);

    void        ClearVisit();
    void        SetVisit(EventType event);
    bool        IsVisit(EventType event);

protected:
    CObject*    SearchToto();

protected:
    Gfx::CEngine*     m_engine;
    Ui::CInterface*     m_interface;
    CSoundInterface*         m_sound;

    struct TextLine
    {
        bool exist = false;
        float time = 0.0f;
        Math::Vector visitGoal;
        float visitDist = 0.0f;
        float visitHeight = 0.0f;
    };
    std::array<TextLine, MAXDTLINE> m_textLines;

    bool            m_bHide;
    bool            m_bEnable;
    float           m_delayFactor;
};


} // namespace Ui
