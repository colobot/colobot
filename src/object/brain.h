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

/**
 * \file object/brain.h
 * \brief CBrain - dispatches commands to objects
 */

#pragma once

#include "common/event.h"
#include "common/misc.h"

#include "object/trace_color.h"

#include "object/task/taskflag.h"
#include "object/task/taskmanip.h"
#include "object/task/taskshield.h"

#include <vector>
#include <sstream>
#include <memory>

class COldObject;
class CPhysics;
class CMotion;
class CTaskManager;
class CScript;
class CRobotMain;
class CSoundInterface;
class CLevelParserLine;

namespace Ui
{
class CStudio;
class CInterface;
class CWindow;
}

namespace Gfx
{
class CEngine;
class CTerrain;
class CWater;
class CCamera;
class CParticle;
} /* Gfx */



enum TraceOper
{
    TO_STOP         = 0,    // stop
    TO_ADVANCE      = 1,    // advance
    TO_RECEDE       = 2,    // back
    TO_TURN         = 3,    // rotate
    TO_PEN          = 4,    // color change
};

struct TraceRecord
{
    TraceOper   oper;
    float       param;
};

struct Program
{
    std::unique_ptr<CScript> script;
    std::string filename;
    bool        readOnly;
    bool        runnable;
};



class CBrain
{
public:
    CBrain(COldObject* object);
    ~CBrain();

    void        DeleteObject(bool bAll=false);

    void        SetPhysics(CPhysics* physics);
    void        SetMotion(CMotion* motion);

    bool        EventProcess(const Event &event);
    bool        CreateInterface(bool bSelect);

    bool        Write(CLevelParserLine* line);
    bool        Read(CLevelParserLine* line);

    bool        IsBusy();
    void        SetActivity(bool bMode);
    bool        GetActivity();
    bool        IsProgram();
    void        RunProgram(Program* program);
    int         GetProgram();
    void        StopProgram();
    void        StopTask();
    void        StopSecondaryTask();

    bool        IntroduceVirus();
    void        SetActiveVirus(bool bActive);
    bool        GetActiveVirus();

    void        SetScriptRun(Program* rank);
    Program*    GetScriptRun();
    void        SetSoluceName(char *name);
    char*       GetSoluceName();

    bool        ReadSoluce(char* filename);
    bool        ReadProgram(Program* program, const char* filename);
    bool        GetCompile(Program* program);
    bool        WriteProgram(Program* program, const char* filename);
    bool        ReadStack(FILE *file);
    bool        WriteStack(FILE *file);
    std::vector<std::unique_ptr<Program>>& GetPrograms();

    Error       StartTaskTake();
    Error       StartTaskManip(TaskManipOrder order, TaskManipArm arm);
    Error       StartTaskFlag(TaskFlagOrder order, int rank);
    Error       StartTaskBuild(ObjectType type);
    Error       StartTaskSearch();
    Error       StartTaskDeleteMark();
    Error       StartTaskTerraform();
    Error       StartTaskRecover();
    Error       StartTaskShield(TaskShieldMode mode);
    Error       StartTaskFire(float delay);
    Error       StartTaskFireAnt(Math::Vector impact);
    Error       StartTaskSpiderExplo();
    Error       StartTaskGunGoal(float dirV, float dirH);

    void        UpdateInterface(float rTime);
    void        UpdateInterface();

    Program*    AddProgram();
    void        AddProgram(std::unique_ptr<Program> program);
    void        RemoveProgram(Program* program);
    Program*    CloneProgram(Program* program);

    Program*    GetProgram(int index);
    Program*    GetOrAddProgram(int index);
    int         GetProgramIndex(Program* program);

protected:
    Error       StartTaskPen(bool down, TraceColor color = TraceColor::Default);

    bool        EventFrame(const Event &event);

    void        StartEditScript(Program* program, char* name);
    void        StopEditScript(bool bCancel);

    Error       EndedTask();

    void        GroundFlat();
    void        ColorFlag(int color);

    void        UpdateScript(Ui::CWindow *pw);
    int         GetSelScript();
    void        SetSelScript(int index);
    void        BlinkScript(bool bEnable);

    void        CheckInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        EnableInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        DeadInterface(Ui::CWindow *pw, EventType event, bool bState);
    void        DefaultEnter(Ui::CWindow *pw, EventType event, bool bState=true);

    //! Start recording trace
    void        TraceRecordStart();
    //! Save current status to recording buffer
    void        TraceRecordFrame();
    //! Stop recording trace and generate CBot program
    void        TraceRecordStop();
    //! Save this operation to recording buffer
    bool        TraceRecordOper(TraceOper oper, float param);
    //! Convert this recording operation to CBot instruction
    bool        TraceRecordPut(std::stringstream& buffer, TraceOper oper, float param);

protected:
    Gfx::CEngine*       m_engine;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    Gfx::CParticle*     m_particle;
    COldObject*         m_object;
    CPhysics*           m_physics;
    CMotion*            m_motion;
    Ui::CInterface*     m_interface;
    CRobotMain*         m_main;
    Ui::CStudio*        m_studio;
    CSoundInterface*    m_sound;

    CTaskManager*       m_primaryTask;
    CTaskManager*       m_secondaryTask;

    std::vector<std::unique_ptr<Program>> m_program;
    Program*            m_currentProgram;

    unsigned int        m_selScript;        // rank of the selected script

    bool                m_bActivity;
    bool                m_bBurn;
    bool                m_bActiveVirus;

    Program*            m_scriptRun;
    char                m_soluceName[50];

    EventType           m_buttonAxe;
    EventType           m_manipStyle;
    EventType           m_defaultEnter;

    float               m_time;
    float               m_burnTime;
    float               m_lastUpdateTime;
    float               m_lastAlarmTime;
    int                 m_soundChannelAlarm;
    int                 m_flagColor;

    bool                m_bTraceRecord;
    TraceOper           m_traceOper;
    Math::Vector        m_tracePos;
    float               m_traceAngle;
    TraceColor          m_traceColor;
    int                 m_traceRecordIndex;
    TraceRecord*        m_traceRecordBuffer;
};
