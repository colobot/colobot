// * This file is part of the COLOBOT source code;
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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

// control.h

#pragma once

#include <string>

#include "common/event.h"
#include "common/misc.h"
#include "common/restext.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/text.h"

#include "object/robotmain.h"

#include "sound/sound.h"



namespace Ui {

enum ControlState
{
    STATE_ENABLE    = (1<<0),   // active
    STATE_CHECK     = (1<<1),   // pressed
    STATE_HILIGHT   = (1<<2),   // overflown by mouse
    STATE_PRESS     = (1<<3),   // pressed by mouse
    STATE_VISIBLE   = (1<<4),   // visible
    STATE_DEAD      = (1<<5),   // inaccessible (x)
    STATE_DEFAULT   = (1<<6),   // actuated by RETURN
    STATE_OKAY      = (1<<7),   // green point at the bottom right
    STATE_SHADOW    = (1<<8),   // shadow
    STATE_GLINT     = (1<<9),   // dynamic reflection
    STATE_CARD      = (1<<10),  // tab
    STATE_EXTEND    = (1<<11),  // extended mode
    STATE_SIMPLY    = (1<<12),  // undecorated
    STATE_FRAME     = (1<<13),  // framework highlighting
    STATE_WARNING   = (1<<14),  // framework hatched yellow / black
    STATE_VALUE     = (1<<15),  // displays the value
    STATE_RUN       = (1<<16)   // running program
};



class CControl
{
public:
    CControl();
    virtual ~CControl();

    virtual bool          Create(Math::Point pos, Math::Point dim, int icon, EventType eventType);

    virtual bool          EventProcess(const Event &event);

    virtual void          SetPos(Math::Point pos);
    virtual Math::Point   GetPos();
    virtual void          SetDim(Math::Point dim);
    virtual Math::Point   GetDim();
    virtual bool          SetState(int state, bool bState);
    virtual bool          SetState(int state);
    virtual bool          ClearState(int state);
    virtual bool          TestState(int state);
    virtual int           GetState();
    virtual void          SetIcon(int icon);
    virtual int           GetIcon();
    virtual void          SetName(std::string name, bool bTooltip=true);
    virtual std::string   GetName();
    virtual void          SetTextAlign(Gfx::TextAlign mode);
    virtual int           GetTextAlign();
    virtual void          SetFontSize(float size);
    virtual float         GetFontSize();
    virtual void          SetFontStretch(float stretch);
    virtual float         GetFontStretch();
    virtual void          SetFontType(Gfx::FontType font);
    virtual Gfx::FontType GetFontType();
    virtual bool          SetTooltip(std::string name);
    virtual bool          GetTooltip(Math::Point pos, std::string &name);
    virtual void          SetFocus(bool bFocus);
    virtual bool          GetFocus();

    virtual EventType     GetEventType();

    virtual void          Draw();

protected:
            void    GlintDelete();
            void    GlintCreate(Math::Point ref, bool bLeft=true, bool bUp=true);
            void    GlintFrame(const Event &event);
            void    DrawPart(int icon, float zoom, float ex);
            void    DrawIcon(Math::Point pos, Math::Point dim, Math::Point uv1, Math::Point uv2, float ex=0.0f);
            void    DrawIcon(Math::Point pos, Math::Point dim, Math::Point uv1, Math::Point uv2, Math::Point corner, float ex);
            void    DrawWarning(Math::Point pos, Math::Point dim);
            void    DrawShadow(Math::Point pos, Math::Point dim, float deep=1.0f);
    virtual bool    Detect(Math::Point pos);

protected:
    Gfx::CEngine*     m_engine;
    Gfx::CParticle*   m_particle;
    CEventQueue*      m_event;
    CRobotMain*       m_main;
    CSoundInterface*  m_sound;

    Math::Point       m_pos;          // corner upper / left
    Math::Point       m_dim;          // dimensions
    int               m_icon;
    EventType         m_eventType;     // message to send when clicking
    int               m_state;        // states (STATE_ *)
    float             m_fontSize;     // size of the button name
    float             m_fontStretch;      // stretch of the font
    Gfx::FontType     m_fontType;     // type of font
    Gfx::TextAlign    m_textAlign;    //type of alignment //comes in the place of m_justif
    std::string       m_name;        // name of the button
    std::string       m_tooltip;     // name of tooltip
    bool              m_bFocus;
    bool              m_bCapture;

    bool              m_bGlint;
    Math::Point       m_glintCorner1;
    Math::Point       m_glintCorner2;
    float             m_glintProgress;
    Math::Point       m_glintMouse;
};

} // namespace Ui
