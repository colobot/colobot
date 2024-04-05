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

#include <glm/glm.hpp>

#include <filesystem>
#include <string>

class CRobotMain;
class CObject;
class CEventQueue;
class CPauseManager;
struct ActivePause;
struct Event;

namespace Gfx
{
class CCamera;
class CEngine;
class CParticle;
class CLightManager;
} // namespace Gfx

namespace Ui
{

class CInterface;

class CDisplayInfo
{
public:
    CDisplayInfo();
    ~CDisplayInfo();

    bool        EventProcess(const Event &event);

    void        StartDisplayInfo(const std::filesystem::path& filename, int index, bool bSoluce);
    void        StopDisplayInfo();

    void        SetPosition(int pos);
    int         GetPosition();

protected:
    bool        EventFrame(const Event &event);
    void        HyperUpdate();
    void        AdjustDisplayInfo(glm::vec2 wpos, glm::vec2 wdim);
    void        ChangeIndexButton(int index);
    void        UpdateIndexButton();
    void        UpdateCopyButton();
    void        ViewDisplayInfo();
    CObject*    SearchToto();

protected:
    Gfx::CEngine*       m_engine;
    CEventQueue*        m_event;
    CRobotMain*         m_main;
    Gfx::CCamera*       m_camera;
    CInterface*         m_interface;
    Gfx::CParticle*     m_particle;
    Gfx::CLightManager* m_light;
    CPauseManager*      m_pause;

    bool        m_bInfoMaximized;
    bool        m_bInfoMinimized;

    int         m_index;
    glm::vec2   m_infoNormalPos;
    glm::vec2   m_infoNormalDim;
    glm::vec2   m_infoActualPos;
    glm::vec2   m_infoActualDim;
    glm::vec2   m_infoFinalPos;
    glm::vec2   m_infoFinalDim;
    int         m_lightSuppl;
    bool        m_bEditLock;
    ActivePause* m_satcomPause = nullptr;
    bool        m_bSoluce;
    CObject*    m_toto;
};


} // namespace Ui
