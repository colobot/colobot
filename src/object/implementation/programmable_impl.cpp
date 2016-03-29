/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/implementation/programmable_impl.h"

#include "CBot/CBot.h"

#include "common/global.h"

#include "level/robotmain.h"

#include "math/all.h"

#include "object/object.h"
#include "object/old_object.h"

#include "object/interface/controllable_object.h"
#include "object/interface/task_executor_object.h"

#include "object/motion/motion.h"
#include "object/motion/motionvehicle.h"

#include "physics/physics.h"

#include "script/script.h"

#include "ui/controls/edit.h"

#include <algorithm>
#include <iomanip>

CProgrammableObjectImpl::CProgrammableObjectImpl(ObjectInterfaceTypes& types, CObject* object)
    : CProgrammableObject(types),
      m_object(object),
      m_activity(true),
      m_cmdLine(),
      m_currentProgram(nullptr),
      m_traceRecord(false),
      m_traceOper(TO_STOP),
      m_traceAngle(0.0f),
      m_traceColor(TraceColor::Default),
      m_traceRecordIndex(0)
{
    //assert(m_object->Implements(ObjectInterfaceType::TaskExecutor));
}

CProgrammableObjectImpl::~CProgrammableObjectImpl()
{}

bool CProgrammableObjectImpl::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
    {
        if ( m_object->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject*>(m_object)->IsDying() && IsProgram() )
        {
            StopProgram();
        }

        if ( GetActivity() )
        {
            if ( IsProgram() )  // current program?
            {
                if ( m_currentProgram->script->Continue() )
                {
                    StopProgram();
                }
            }

            if ( m_traceRecord )  // registration of the design in progress?
            {
                TraceRecordFrame();
            }
        }
    }

    return true;
}


void CProgrammableObjectImpl::SetActivity(bool activity)
{
    m_activity = activity;
}

bool CProgrammableObjectImpl::GetActivity()
{
    return m_activity;
}


void CProgrammableObjectImpl::RunProgram(Program* program)
{
    if ( program->script->Run() )
    {
        m_currentProgram = program;  // start new program
        m_object->UpdateInterface();
        if (m_object->Implements(ObjectInterfaceType::Controllable) && dynamic_cast<CControllableObject*>(m_object)->GetTrainer())
            CRobotMain::GetInstancePointer()->StartMissionTimer();
    }
}

void CProgrammableObjectImpl::StopProgram()
{
    if ( m_currentProgram != nullptr )
    {
        m_currentProgram->script->Stop();
    }

    m_currentProgram = nullptr;

    m_object->UpdateInterface();
}

Program* CProgrammableObjectImpl::GetCurrentProgram()
{
    return m_currentProgram;
}

bool CProgrammableObjectImpl::IsProgram()
{
    return m_currentProgram != nullptr;
}


// Load a stack of script implementation from a file.

bool CProgrammableObjectImpl::ReadStack(FILE *file)
{
    short       op;

    CBot::fRead(&op, sizeof(short), 1, file);
    if ( op == 1 )  // run ?
    {
        CBot::fRead(&op, sizeof(short), 1, file);  // program rank
        if ( op >= 0 )
        {
            if (m_object->Implements(ObjectInterfaceType::ProgramStorage))
            {
                assert(op < static_cast<int>(dynamic_cast<CProgramStorageObject*>(m_object)->GetProgramCount()));
                m_currentProgram = dynamic_cast<CProgramStorageObject*>(m_object)->GetProgram(op);
                if ( !m_currentProgram->script->ReadStack(file) )  return false;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

// Save the script implementation stack of a file.

bool CProgrammableObjectImpl::WriteStack(FILE *file)
{
    short       op;

    if ( m_currentProgram != nullptr &&  // current program?
         m_currentProgram->script->IsRunning() )
    {
        op = 1;  // run
        CBot::fWrite(&op, sizeof(short), 1, file);

        op = -1;
        if (m_object->Implements(ObjectInterfaceType::ProgramStorage))
        {
            op = dynamic_cast<CProgramStorageObject*>(m_object)->GetProgramIndex(m_currentProgram);
        }
        CBot::fWrite(&op, sizeof(short), 1, file);

        return m_currentProgram->script->WriteStack(file);
    }

    op = 0;  // stop
    CBot::fWrite(&op, sizeof(short), 1, file);
    return true;
}



const int MAXTRACERECORD = 1000;

// Start of registration of the design.

void CProgrammableObjectImpl::TraceRecordStart()
{
    if (m_traceRecord)
    {
        TraceRecordStop();
    }

    assert(m_object->Implements(ObjectInterfaceType::TraceDrawing));
    CTraceDrawingObject* traceDrawing = dynamic_cast<CTraceDrawingObject*>(m_object);

    m_traceRecord = true;

    m_traceOper = TO_STOP;

    m_tracePos = m_object->GetPosition();
    m_traceAngle = m_object->GetRotationY();

    if ( traceDrawing->GetTraceDown() )  // pencil down?
    {
        m_traceColor = traceDrawing->GetTraceColor();
    }
    else    // pen up?
    {
        m_traceColor = TraceColor::Default;
    }

    m_traceRecordBuffer = MakeUniqueArray<TraceRecord>(MAXTRACERECORD);
    m_traceRecordIndex = 0;
}

// Saving the current drawing.

void CProgrammableObjectImpl::TraceRecordFrame()
{
    TraceOper   oper = TO_STOP;
    Math::Vector    pos;
    float       angle, len, speed;

    assert(m_object->Implements(ObjectInterfaceType::TraceDrawing));
    CTraceDrawingObject* traceDrawing = dynamic_cast<CTraceDrawingObject*>(m_object);

    CPhysics* physics = dynamic_cast<CMovableObject*>(m_object)->GetPhysics();

    speed = physics->GetLinMotionX(MO_REASPEED);
    if ( speed > 0.0f )  oper = TO_ADVANCE;
    if ( speed < 0.0f )  oper = TO_RECEDE;

    speed = physics->GetCirMotionY(MO_REASPEED);
    if ( speed != 0.0f )  oper = TO_TURN;

    TraceColor color = TraceColor::Default;
    if ( traceDrawing->GetTraceDown() )  // pencil down?
    {
        color = traceDrawing->GetTraceColor();
    }

    if ( oper != m_traceOper ||
         color != m_traceColor )
    {
        if ( m_traceOper == TO_ADVANCE ||
             m_traceOper == TO_RECEDE  )
        {
            pos = m_object->GetPosition();
            len = Math::DistanceProjected(pos, m_tracePos);
            TraceRecordOper(m_traceOper, len);
        }
        if ( m_traceOper == TO_TURN )
        {
            angle = m_object->GetRotationY()-m_traceAngle;
            TraceRecordOper(m_traceOper, angle);
        }

        if ( color != m_traceColor )
        {
            TraceRecordOper(TO_PEN, static_cast<float>(color));
        }

        m_traceOper = oper;
        m_tracePos = m_object->GetPosition();
        m_traceAngle = m_object->GetRotationY();
        m_traceColor = color;
    }
}

// End of the registration of the design. Program generates the CBOT.

void CProgrammableObjectImpl::TraceRecordStop()
{
    TraceOper   lastOper, curOper;
    float       lastParam, curParam;

    m_traceRecord = false;

    std::stringstream buffer;
    buffer << "extern void object::AutoDraw()\n{\n";

    lastOper = TO_STOP;
    lastParam = 0.0f;
    for ( int i=0 ; i<m_traceRecordIndex ; i++ )
    {
        curOper = m_traceRecordBuffer[i].oper;
        curParam = m_traceRecordBuffer[i].param;

        if ( curOper == lastOper )
        {
            if ( curOper == TO_PEN )
            {
                lastParam = curParam;
            }
            else
            {
                lastParam += curParam;
            }
        }
        else
        {
            TraceRecordPut(buffer, lastOper, lastParam);
            lastOper = curOper;
            lastParam = curParam;
        }
    }
    TraceRecordPut(buffer, lastOper, lastParam);

    m_traceRecordBuffer.reset();

    buffer << "}\n";

    assert(m_object->Implements(ObjectInterfaceType::ProgramStorage));
    Program* prog = dynamic_cast<CProgramStorageObject*>(m_object)->AddProgram();
    prog->script->SendScript(buffer.str().c_str());
}

// Saves an instruction CBOT.

bool CProgrammableObjectImpl::TraceRecordOper(TraceOper oper, float param)
{
    int     i;

    i = m_traceRecordIndex;
    if ( i >= MAXTRACERECORD )  return false;

    m_traceRecordBuffer[i].oper = oper;
    m_traceRecordBuffer[i].param = param;

    m_traceRecordIndex = i+1;
    return true;
}

// Generates an instruction CBOT.

bool CProgrammableObjectImpl::TraceRecordPut(std::stringstream& buffer, TraceOper oper, float param)
{
    if ( oper == TO_ADVANCE )
    {
        param /= g_unit;
        buffer << "\tmove(" << std::fixed << std::setprecision(1) << param << ");\n";
    }

    if ( oper == TO_RECEDE )
    {
        param /= g_unit;
        buffer << "\tmove(-" << std::fixed << std::setprecision(1) << param << ");\n";
    }

    if ( oper == TO_TURN )
    {
        param = -param*180.0f/Math::PI;
        buffer << "\tturn(" << static_cast<int>(param) << ");\n";
    }

    if ( oper == TO_PEN )
    {
        TraceColor color = static_cast<TraceColor>(static_cast<int>(param));
        if ( color == TraceColor::Default )
            buffer << "\tpenup();\n";
        else
            buffer << "\tpendown(" << TraceColorName(color) << ");\n";
    }

    return true;
}

bool CProgrammableObjectImpl::IsTraceRecord()
{
    return m_traceRecord;
}


void CProgrammableObjectImpl::SetCmdLine(unsigned int rank, float value)
{
    if (rank == m_cmdLine.size())
    {
        m_cmdLine.push_back(value);
    }
    else if (rank < m_cmdLine.size())
    {
        m_cmdLine[rank] = value;
    }
    else
    {
        // should never happen
        assert(false);
    }
}

float CProgrammableObjectImpl::GetCmdLine(unsigned int rank)
{
    if ( rank >= m_cmdLine.size() )  return 0.0f;
    return m_cmdLine[rank];
}

std::vector<float>& CProgrammableObjectImpl::GetCmdLine()
{
    return m_cmdLine;
}
