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

#include "common/event.h"

class COldObject;
class CTaskExecutorObject;
class CProgrammableObject;
class CProgramStorageObject;
class CPhysics;
class CMotion;
class CRobotMain;
class CSoundInterface;
struct Program;

namespace Gfx
{
class CEngine;
class CTerrain;
class CWater;
class CCamera;
class CParticle;
} /* Gfx */

namespace Ui
{
class CStudio;
class CInterface;
class CWindow;


class CObjectInterface
{
public:
    CObjectInterface(COldObject* object);
    ~CObjectInterface();

    void        DeleteObject(bool all=false);
    void        DeletePrg();

    bool        EventProcess(const Event &event);
    bool        CreateInterface(bool bSelect);

    void        UpdateInterface(float rTime);
    void        UpdateInterface();

protected:
    bool        EventFrame(const Event &event);

    /**
     * \brief Start the program editor
     * \param program Program instance to edit
     * \param name Program name, used only if the program is empty to insert main function stub code
     * \remarks If the editor is already open, the behaviour is undefined
     */
    void        StartEditScript(Program* program, std::string name);
    /**
     * \brief Close the program editor
     * \param closeWithErrors If true, the editor will be forced to close even if the program contains errors
     * \remarks If the editor isn't open, the behaviour is undefined
     */
    void        StopEditScript(bool closeWithErrors);

    void        GroundFlat();
    void        ColorFlag(int color);

    void        UpdateScript(CWindow *pw);
    int         GetSelScript();
    void        SetSelScript(int index);
    void        BlinkScript(bool bEnable);

    void        CheckInterface(CWindow *pw, EventType event, bool bState);
    void        EnableInterface(CWindow *pw, EventType event, bool bState);
    void        DeadInterface(CWindow *pw, EventType event, bool bState);
    void        DefaultEnter(CWindow *pw, EventType event, bool bState=true);

protected:
    Gfx::CEngine*       m_engine;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    Gfx::CParticle*     m_particle;
    CRobotMain*         m_main;
    CInterface*         m_interface;
    CSoundInterface*    m_sound;

    COldObject*         m_object;
    CTaskExecutorObject* m_taskExecutor;
    CProgrammableObject* m_programmable;
    CProgramStorageObject* m_programStorage;
    CPhysics*           m_physics;
    CMotion*            m_motion;

    std::unique_ptr<CStudio> m_studio;

    int                 m_selScript;        // rank of the selected script

    EventType           m_manipStyle;
    EventType           m_defaultEnter;

    float               m_time;
    float               m_lastUpdateTime;
    float               m_lastAlarmTime;
    int                 m_soundChannelAlarm;
    int                 m_flagColor;
};

} // namespace Ui
