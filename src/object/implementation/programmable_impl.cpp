/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/implementation/programmable_impl.h"

#include "math/all.h"

#include "object/object.h"
#include "object/old_object.h"
#include "object/robotmain.h"

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
    : CProgrammableObject(types)
    , m_object(object)
    , m_activity(true)
    , m_cmdLine()
    , m_program()
    , m_currentProgram(nullptr)
    , m_activeVirus(false)
    , m_scriptRun(nullptr)
    , m_soluceName("")
    , m_traceRecord(false)
{
    //assert(m_object->Implements(ObjectInterfaceType::TaskExecutor));
}

CProgrammableObjectImpl::~CProgrammableObjectImpl()
{}

bool CProgrammableObjectImpl::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
    {
        if ( m_object->GetRuin() && IsProgram() )
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

bool CProgrammableObjectImpl::IsProgram()
{
    return m_currentProgram != nullptr;
}


// Introduces a virus into a program.
// Returns true if it was inserted.

bool CProgrammableObjectImpl::IntroduceVirus()
{
    if(m_program.size() == 0) return false;

    for ( int i=0 ; i<50 ; i++ )
    {
        int programIndex = rand() % m_program.size();
        if ( m_program[programIndex]->script->IntroduceVirus() )  // tries to introduce
        {
            m_activeVirus = true;  // active virus
            return true;
        }
    }
    return false;
}

// Active Virus indicates that the object is contaminated. Unlike ch'tites (??? - Programerus)
// letters which automatically disappear after a while,
// ActiveVirus does not disappear after you edit the program
// (Even if the virus is not fixed).

void CProgrammableObjectImpl::SetActiveVirus(bool bActive)
{
    m_activeVirus = bActive;

    if ( !m_activeVirus )  // virus disabled?
    {
        m_object->SetVirusMode(false);  // chtites (??? - Programerus) letters also
    }
}

bool CProgrammableObjectImpl::GetActiveVirus()
{
    return m_activeVirus;
}


Program* CProgrammableObjectImpl::AddProgram()
{
    assert(m_object->Implements(ObjectInterfaceType::Old)); //TODO
    auto program = MakeUnique<Program>();
    program->script = MakeUnique<CScript>(dynamic_cast<COldObject*>(this));
    program->readOnly = false;
    program->runnable = true;

    Program* prog = program.get();
    AddProgram(std::move(program));
    return prog;
}

void CProgrammableObjectImpl::AddProgram(std::unique_ptr<Program> program)
{
    m_program.push_back(std::move(program));
    m_object->UpdateInterface();
}

void CProgrammableObjectImpl::RemoveProgram(Program* program)
{
    if(m_currentProgram == program)
    {
        StopProgram();
    }

    m_program.erase(
        std::remove_if(m_program.begin(), m_program.end(),
            [program](std::unique_ptr<Program>& prog) { return prog.get() == program; }),
        m_program.end());

    m_object->UpdateInterface();
}

Program* CProgrammableObjectImpl::CloneProgram(Program* program)
{
    Program* newprog = AddProgram();

    // TODO: Is there any reason CScript doesn't have a function to get the program code directly?
    Ui::CEdit* edit = new Ui::CEdit();
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    program->script->PutScript(edit, "");
    newprog->script->GetScript(edit);
    delete edit;

    return newprog;
}

std::vector<std::unique_ptr<Program>>& CProgrammableObjectImpl::GetPrograms()
{
    return m_program;
}

int CProgrammableObjectImpl::GetProgramCount()
{
    return static_cast<int>(m_program.size());
}

int CProgrammableObjectImpl::GetProgramIndex(Program* program)
{
    for(unsigned int i = 0; i < m_program.size(); i++)
    {
        if(m_program[i].get() == program)
        {
            return i;
        }
    }
    return -1;
}

int CProgrammableObjectImpl::GetProgram()
{
    if(m_currentProgram == nullptr)
        return -1;

    for(unsigned int i = 0; i < m_program.size(); i++)
    {
        if(m_program[i].get() == m_currentProgram)
        {
            return i;
        }
    }
    return -1;
}

Program* CProgrammableObjectImpl::GetProgram(int index)
{
    if(index < 0 || index >= static_cast<int>(m_program.size()))
        return nullptr;

    return m_program[index].get();
}

Program* CProgrammableObjectImpl::GetOrAddProgram(int index)
{
    if(index < 0)
        return nullptr;

    if(index < static_cast<int>(m_program.size()))
        return m_program[index].get();

    for(int i = m_program.size(); i < index; i++)
    {
        AddProgram();
    }
    return AddProgram();
}

// Name management scripts to load.

void CProgrammableObjectImpl::SetScriptRun(Program* program)
{
    m_scriptRun = program;
}

Program* CProgrammableObjectImpl::GetScriptRun()
{
    return m_scriptRun;
}

void CProgrammableObjectImpl::SetSoluceName(const std::string& name)
{
    m_soluceName = name;
}

const std::string& CProgrammableObjectImpl::GetSoluceName()
{
    return m_soluceName;
}


// Load a script solution, in the first free script.
// If there is already an identical script, nothing is loaded.

bool CProgrammableObjectImpl::ReadSoluce(const std::string& filename)
{
    Program* prog = AddProgram();

    if ( !ReadProgram(prog, filename) )  return false;  // load solution
    prog->readOnly = true;

    for(unsigned int i = 0; i < m_program.size(); i++)
    {
        if(m_program[i].get() == prog) continue;

        //TODO: This is bad. It's very sensitive to things like \n vs \r\n etc.
        if ( m_program[i]->script->Compare(prog->script.get()) )  // the same already?
        {
            m_program[i]->readOnly = true; // Mark is as read-only
            RemoveProgram(prog);
            return false;
        }
    }

    return true;
}

// Load a script with a text file.

bool CProgrammableObjectImpl::ReadProgram(Program* program, const std::string& filename)
{
    if ( program->script->ReadScript(filename.c_str()) )  return true;

    return false;
}

// Indicates whether a program is compiled correctly.

bool CProgrammableObjectImpl::GetCompile(Program* program)
{
    return program->script->GetCompile();
}

// Saves a script in a text file.

bool CProgrammableObjectImpl::WriteProgram(Program* program, const std::string& filename)
{
    if ( program->script->WriteScript(filename.c_str()) )  return true;

    return false;
}


// Load a stack of script implementation from a file.

bool CProgrammableObjectImpl::ReadStack(FILE *file)
{
    short       op;

    fRead(&op, sizeof(short), 1, file);
    if ( op == 1 )  // run ?
    {
        fRead(&op, sizeof(short), 1, file);  // program rank
        if ( op >= 0 )
        {
            assert(op < static_cast<int>(m_program.size()));

            //TODO: m_selScript = op;

            if ( !m_program[op]->script->ReadStack(file) )  return false;
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
        fWrite(&op, sizeof(short), 1, file);

        op = GetProgram();
        fWrite(&op, sizeof(short), 1, file);

        return m_currentProgram->script->WriteStack(file);
    }

    op = 0;  // stop
    fWrite(&op, sizeof(short), 1, file);
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

    Program* prog = AddProgram();
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
