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

#include "object/interface/programmable_object.h"

#include "math/vector.h"

#include "object/interface/interactive_object.h"
#include "object/interface/trace_drawing_object.h"

#include <sstream>

class CObject;

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
    TraceOper   oper = TO_STOP;
    float       param = 0.0f;
};

class CProgrammableObjectImpl : public CProgrammableObject
{
public:
    explicit CProgrammableObjectImpl(ObjectInterfaceTypes& types, CObject* object);
    virtual ~CProgrammableObjectImpl();

    bool EventProcess(const Event& event);

    bool IsProgram() override;
    void RunProgram(Program* program) override;
    Program* GetCurrentProgram() override;
    void StopProgram() override;

    bool ReadStack(FILE *file) override;
    bool WriteStack(FILE *file) override;

    void TraceRecordStart() override;
    void TraceRecordStop() override;
    bool IsTraceRecord() override;

    void SetActivity(bool activity) override;
    bool GetActivity() override;

    void SetCmdLine(unsigned int rank, float value);
    float GetCmdLine(unsigned int rank) override;
    std::vector<float>& GetCmdLine();

private:
    //! Save current status to recording buffer
    void        TraceRecordFrame();
    //! Save this operation to recording buffer
    bool        TraceRecordOper(TraceOper oper, float param);
    //! Convert this recording operation to CBot instruction
    bool        TraceRecordPut(std::stringstream& buffer, TraceOper oper, float param);

private:
    CObject* m_object;

private:
    bool                m_activity;

    std::vector<float>  m_cmdLine;

    Program*            m_currentProgram;

    bool                m_traceRecord;
    TraceOper           m_traceOper;
    Math::Vector        m_tracePos;
    float               m_traceAngle;
    TraceColor          m_traceColor;
    int                 m_traceRecordIndex;
    std::unique_ptr<TraceRecord[]> m_traceRecordBuffer;
};
