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

#include "common/event.h"

#include "graphics/engine/text.h"

#include <glm/glm.hpp>

#include <string>

class CRobotMain;
class CSoundInterface;
class CSettings;

namespace Gfx
{
class CEngine;
class CParticle;
}


namespace Ui
{

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
    STATE_RUN       = (1<<16),  // running program
    STATE_DAMAGE    = (1<<17)   // taking damage
};



class CControl
{
public:
    CControl();
    virtual ~CControl();

    virtual bool          Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventType);

    virtual bool          EventProcess(const Event &event);

    virtual void          SetPos(const glm::vec2& pos);
    virtual glm::vec2     GetPos();
    virtual void          SetDim(const glm::vec2& dim);
    virtual glm::vec2     GetDim();
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
    virtual bool          GetTooltip(const glm::vec2& pos, std::string &name);
    virtual void          SetFocus(CControl* focusControl);
    virtual bool          GetFocus();

    virtual EventType     GetEventType();

    virtual void          Draw();

protected:
            void    GlintDelete();
            void    GlintCreate(const glm::vec2& ref, bool bLeft=true, bool bUp=true);
            void    GlintFrame(const Event &event);
            void    DrawPart(int icon, float zoom, float ex);
            void    DrawIcon(const glm::vec2& pos, const glm::vec2& dim, const glm::vec2& uv1, const glm::vec2& uv2, float ex=0.0f);
            void    DrawIcon(const glm::vec2& pos, const glm::vec2& dim, const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& corner, float ex);
            void    DrawWarning(const glm::vec2& pos, const glm::vec2& dim);
            void    DrawShadow(const glm::vec2& pos, const glm::vec2& dim, float deep=1.0f);
    virtual bool    Detect(const glm::vec2& pos);

    std::string     GetResourceName(EventType eventType);

    /**
     * \brief Set texture in m_engine to correct buttonX.png for given icon
     * \param icon Icon to draw
     * \return Index inside the selected texture of the icon to draw
     */
    int             SetButtonTextureForIcon(int icon);

protected:
    Gfx::CEngine*     m_engine;
    Gfx::CParticle*   m_particle;
    CEventQueue*      m_event;
    CRobotMain*       m_main;
    CSoundInterface*  m_sound;
    CSettings*        m_settings;

    glm::vec2         m_pos;          // corner upper / left
    glm::vec2         m_dim;          // dimensions
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
    glm::vec2         m_glintCorner1;
    glm::vec2         m_glintCorner2;
    float             m_glintProgress;
    glm::vec2         m_glintMouse;
};

} // namespace Ui
