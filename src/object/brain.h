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

#include "object/trace_color.h"

#include <vector>
#include <sstream>
#include <memory>

class COldObject;
class CTaskExecutorObject;
class CPhysics;
class CMotion;
class CScript;
class CRobotMain;
class CLevelParserLine;

namespace Gfx
{
class CEngine;
class CCamera;
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

    void        SetPhysics(CPhysics* physics);
    void        SetMotion(CMotion* motion);

    bool        EventProcess(const Event &event);

    bool        Write(CLevelParserLine* line);
    bool        Read(CLevelParserLine* line);

    bool        IsProgram();
    void        RunProgram(Program* program);
    int         GetProgram();
    void        StopProgram();

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

    Program*    AddProgram();
    void        AddProgram(std::unique_ptr<Program> program);
    void        RemoveProgram(Program* program);
    Program*    CloneProgram(Program* program);

    std::vector<std::unique_ptr<Program>>& GetPrograms();
    int         GetProgramCount();
    Program*    GetProgram(int index);
    Program*    GetOrAddProgram(int index);
    int         GetProgramIndex(Program* program);

    //! Start recording trace
    void        TraceRecordStart();
    //! Stop recording trace and generate CBot program
    void        TraceRecordStop();
    //! Returns true if trace recording is in progress
    bool        IsTraceRecord();

protected:
    bool        EventFrame(const Event &event);

    //! Save current status to recording buffer
    void        TraceRecordFrame();
    //! Save this operation to recording buffer
    bool        TraceRecordOper(TraceOper oper, float param);
    //! Convert this recording operation to CBot instruction
    bool        TraceRecordPut(std::stringstream& buffer, TraceOper oper, float param);

protected:
    CRobotMain*         m_main;
    Gfx::CEngine*       m_engine;
    Gfx::CCamera*       m_camera;

    COldObject*         m_object;
    CTaskExecutorObject* m_taskExecutor;
    CPhysics*           m_physics;
    CMotion*            m_motion;

    std::vector<std::unique_ptr<Program>> m_program;
    Program*            m_currentProgram;

    bool                m_bBurn;
    bool                m_bActiveVirus;

    Program*            m_scriptRun;
    char                m_soluceName[50];

    EventType           m_buttonAxe;

    float               m_time;
    float               m_burnTime;

    bool                m_traceRecord;
    TraceOper           m_traceOper;
    Math::Vector        m_tracePos;
    float               m_traceAngle;
    TraceColor          m_traceColor;
    int                 m_traceRecordIndex;
    std::unique_ptr<TraceRecord[]> m_traceRecordBuffer;
};
