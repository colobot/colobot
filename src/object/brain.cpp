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


#include "object/brain.h"

#include "common/make_unique.h"

#include "math/geometry.h"

#include "object/old_object.h"
#include "object/robotmain.h"

#include "object/interface/task_executor_object.h"

#include "object/level/parserline.h"
#include "object/level/parserparam.h"

#include "object/motion/motion.h"
#include "object/motion/motionvehicle.h"

#include "object/task/taskmanager.h"

#include "physics/physics.h"

#include "script/script.h"

#include "ui/studio.h"

#include "ui/controls/edit.h"

#include <iomanip>
#include <algorithm>



const int MAXTRACERECORD = 1000;

// Object's constructor.

CBrain::CBrain(COldObject* object)
{
    m_object      = object;
    assert(object->Implements(ObjectInterfaceType::TaskExecutor));
    m_taskExecutor = dynamic_cast<CTaskExecutorObject*>(m_object);

    m_physics     = nullptr;
    m_motion      = nullptr;

    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_main        = CRobotMain::GetInstancePointer();
    m_camera      = m_main->GetCamera();

    m_program.clear();
    m_currentProgram = nullptr;
    m_bBurn = false;
    m_bActiveVirus = false;
    m_time = 0.0f;
    m_burnTime = 0.0f;

    m_buttonAxe    = EVENT_NULL;

    m_scriptRun = nullptr;
    m_soluceName[0] = 0;

    m_bTraceRecord = false;
    m_traceRecordBuffer = nullptr;
}

// Object's destructor.

CBrain::~CBrain()
{
    m_program.clear();

    delete[] m_traceRecordBuffer;
    m_traceRecordBuffer = nullptr;
}

void CBrain::SetPhysics(CPhysics* physics)
{
    m_physics = physics;
}

void CBrain::SetMotion(CMotion* motion)
{
    m_motion = motion;
}


// Saves all parameters of the object.

bool CBrain::Write(CLevelParserLine* line)
{
    line->AddParam("bVirusActive", MakeUnique<CLevelParserParam>(m_bActiveVirus));

    if ( m_object->GetType() == OBJECT_MOBILErs )
    {
        line->AddParam("bShieldActive", MakeUnique<CLevelParserParam>(m_taskExecutor->IsBackgroundTask()));
    }

    return true;
}

// Restores all parameters of the object.

bool CBrain::Read(CLevelParserLine* line)
{
    m_bActiveVirus = line->GetParam("bVirusActive")->AsBool(false);
    if ( m_object->GetType() == OBJECT_MOBILErs )
    {
        if( line->GetParam("bShieldActive")->AsBool(false) )
        {
            m_taskExecutor->StartTaskShield(TSM_START);
        }
    }
    return true;
}


// Management of an event.

bool CBrain::EventProcess(const Event &event)
{
    if ( event.type == EVENT_FRAME )
    {
        EventFrame(event);
    }

    if ( !m_object->GetSelect() &&  // robot pas sélectionné  ?
         m_currentProgram == nullptr &&
         !m_taskExecutor->IsForegroundTask() )
    {
        float axeX = 0.0f;
        float axeY = 0.0f;
        float axeZ = 0.0f;
        if ( m_object->GetBurn() )  // Gifted?
        {
            if ( !m_bBurn )  // beginning?
            {
                m_bBurn = true;
                m_burnTime = 0.0f;
            }

            axeZ = -1.0f;  // tomb

            if ( !m_object->GetFixed() &&
                 (m_object->GetType() == OBJECT_ANT    ||
                  m_object->GetType() == OBJECT_SPIDER ||
                  m_object->GetType() == OBJECT_WORM   ) )
            {
                axeY = 2.0f;  // zigzag disorganized fast
                if ( m_object->GetType() == OBJECT_WORM )  axeY = 5.0f;
                axeX = 0.5f+sinf(m_time* 1.0f)*0.5f+
                            sinf(m_time* 6.0f)*2.0f+
                            sinf(m_time*21.0f)*0.2f;
                float factor = 1.0f-m_burnTime/15.0f;  // slow motion
                if ( factor < 0.0f )  factor = 0.0f;
                axeY *= factor;
                axeX *= factor;
            }
        }
        m_physics->SetMotorSpeedX(axeY);  // move forward/move back
        m_physics->SetMotorSpeedY(axeZ);  // up / down
        m_physics->SetMotorSpeedZ(axeX);  // rotate
        return true;
    }

    if ( m_currentProgram != nullptr     &&
         m_object->GetRuin() )
    {
        StopProgram();
        return true;
    }

    if ( !m_object->GetSelect() ) return true;  // robot not selected?

    if ( m_taskExecutor->IsForegroundTask() ||  // current task?
         m_currentProgram != nullptr )
    {
        if ( !m_taskExecutor->IsForegroundTask() || !m_taskExecutor->GetForegroundTask()->IsPilot() )  return true;
    }

    if ( event.type == EVENT_OBJECT_LEFT    ||
         event.type == EVENT_OBJECT_RIGHT   ||
         event.type == EVENT_OBJECT_UP      ||
         event.type == EVENT_OBJECT_DOWN    ||
         event.type == EVENT_OBJECT_GASUP   ||
         event.type == EVENT_OBJECT_GASDOWN )
    {
        m_buttonAxe = event.type;
    }
    if ( event.type == EVENT_MOUSE_BUTTON_UP )
    {
        m_buttonAxe = EVENT_NULL;
    }

    float axeX = event.motionInput.x;
    float axeY = event.motionInput.y;
    float axeZ = event.motionInput.z;

    if ( (!m_main->GetTrainerPilot() &&
          m_object->GetTrainer()) ||
         !m_main->CanPlayerInteract() )  // drive vehicle?
    {
        axeX = 0.0f;
        axeY = 0.0f;
        axeZ = 0.0f;  // Remote control impossible!
    }

    if ( m_buttonAxe == EVENT_OBJECT_LEFT    )  axeX = -1.0f;
    if ( m_buttonAxe == EVENT_OBJECT_RIGHT   )  axeX =  1.0f;
    if ( m_buttonAxe == EVENT_OBJECT_UP      )  axeY =  1.0f;
    if ( m_buttonAxe == EVENT_OBJECT_DOWN    )  axeY = -1.0f;
    if ( m_buttonAxe == EVENT_OBJECT_GASUP   )  axeZ =  1.0f;
    if ( m_buttonAxe == EVENT_OBJECT_GASDOWN )  axeZ = -1.0f;

    if ( m_object->GetType() == OBJECT_MOBILEdr && m_object->GetManual() )  // scribbler in manual mode?
    {
        if ( axeX != 0.0f )  axeY = 0.0f;  // if running -> not moving!
        axeX *= 0.5f;
        axeY *= 0.5f;
    }

    if ( !m_main->IsResearchDone(RESEARCH_FLY, m_object->GetTeam()) )
    {
        axeZ = -1.0f;  // tomb
    }

    axeX += m_camera->GetMotorTurn();  // additional power according to camera
    if ( axeX >  1.0f )  axeX =  1.0f;
    if ( axeX < -1.0f )  axeX = -1.0f;

    m_physics->SetMotorSpeedX(axeY);  // move forward/move back
    m_physics->SetMotorSpeedY(axeZ);  // up/down
    m_physics->SetMotorSpeedZ(axeX);  // rotate

    return true;
}


// The brain is changing by time.

bool CBrain::EventFrame(const Event &event)
{
    m_time += event.rTime;
    if ( m_bBurn )  m_burnTime += event.rTime;

    if ( m_engine->GetPause() )  return true;
    if ( !m_object->GetActivity() ) return true;

    if ( m_currentProgram != nullptr )  // current program?
    {
        if ( m_currentProgram->script->Continue() )
        {
            StopProgram();
        }
    }

    if ( m_bTraceRecord )  // registration of the design in progress?
    {
        TraceRecordFrame();
    }

    return true;
}


// Stops the running program.

void CBrain::StopProgram()
{
    m_taskExecutor->StopForegroundTask();

    if ( m_object->GetType() == OBJECT_HUMAN ||
         m_object->GetType() == OBJECT_TECH  )  return;

    if ( m_currentProgram != nullptr )
    {
        m_currentProgram->script->Stop();
    }

    m_currentProgram = nullptr;

    m_physics->SetMotorSpeedX(0.0f);
    m_physics->SetMotorSpeedY(0.0f);
    m_physics->SetMotorSpeedZ(0.0f);

    m_motion->SetAction(-1);

    m_object->UpdateInterface();
    m_main->UpdateShortcuts();
    m_object->CreateSelectParticle();
}


// Introduces a virus into a program.
// Returns true if it was inserted.

bool CBrain::IntroduceVirus()
{
    if(m_program.size() == 0) return false;

    for ( int i=0 ; i<50 ; i++ )
    {
        int j = rand()%m_program.size();
        if ( m_program[j]->script->IntroduceVirus() )  // tries to introduce
        {
            m_bActiveVirus = true;  // active virus
            return true;
        }
    }
    return false;
}

// Active Virus indicates that the object is contaminated. Unlike ch'tites (??? - Programerus)
// letters which automatically disappear after a while,
// ActiveVirus does not disappear after you edit the program
// (Even if the virus is not fixed).


void CBrain::SetActiveVirus(bool bActive)
{
    m_bActiveVirus = bActive;

    if ( !m_bActiveVirus )  // virus disabled?
    {
        m_object->SetVirusMode(false);  // chtites (??? - Programerus) letters also
    }
}

bool CBrain::GetActiveVirus()
{
    return m_bActiveVirus;
}

// Indicates whether a program is running.

bool CBrain::IsProgram()
{
    return m_currentProgram != nullptr;
}

// Starts a program.

void CBrain::RunProgram(Program* program)
{
    if ( program->script->Run() )
    {
        m_currentProgram = program;  // start new program
        m_object->UpdateInterface();
        m_object->CreateSelectParticle();
        m_main->UpdateShortcuts();
        if(m_object->GetTrainer())
            m_main->StartMissionTimer();
    }
}


// Returns the current program.

int CBrain::GetProgram()
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


// Name management scripts to load.

void CBrain::SetScriptRun(Program* program)
{
    m_scriptRun = program;
}

Program* CBrain::GetScriptRun()
{
    return m_scriptRun;
}

void CBrain::SetSoluceName(char *name)
{
    strcpy(m_soluceName, name);
}

char* CBrain::GetSoluceName()
{
    return m_soluceName;
}


// Load a script solution, in the first free script.
// If there is already an identical script, nothing is loaded.

bool CBrain::ReadSoluce(char* filename)
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

bool CBrain::ReadProgram(Program* program, const char* filename)
{
    if ( program->script->ReadScript(filename) )  return true;

    return false;
}

// Indicates whether a program is compiled correctly.

bool CBrain::GetCompile(Program* program)
{
    return program->script->GetCompile();
}

// Saves a script in a text file.

bool CBrain::WriteProgram(Program* program, const char* filename)
{
    if ( program->script->WriteScript(filename) )  return true;

    return false;
}


// Load a stack of script implementation from a file.

bool CBrain::ReadStack(FILE *file)
{
    short       op;

    fRead(&op, sizeof(short), 1, file);
    if ( op == 1 )  // run ?
    {
        fRead(&op, sizeof(short), 1, file);  // program rank
        if ( op >= 0 )
        {
            assert(op < static_cast<int>(m_program.size())); //TODO: is it good?

            //TODO: m_selScript = op;

            if ( !m_program[op]->script->ReadStack(file) )  return false;
        }
    }

    return true;
}

// Save the script implementation stack of a file.

bool CBrain::WriteStack(FILE *file)
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



// Start of registration of the design.

void CBrain::TraceRecordStart()
{
    CMotionVehicle* motionVehicle = dynamic_cast<CMotionVehicle*>(m_motion);
    assert(motionVehicle != nullptr);

    m_bTraceRecord = true;

    m_traceOper = TO_STOP;

    m_tracePos = m_object->GetPosition();
    m_traceAngle = m_object->GetRotationY();

    if ( motionVehicle->GetTraceDown() )  // pencil down?
    {
        m_traceColor = motionVehicle->GetTraceColor();
    }
    else    // pen up?
    {
        m_traceColor = TraceColor::Default;
    }

    delete[] m_traceRecordBuffer;
    m_traceRecordBuffer = new TraceRecord[MAXTRACERECORD];
    m_traceRecordIndex = 0;
}

// Saving the current drawing.

void CBrain::TraceRecordFrame()
{
    TraceOper   oper = TO_STOP;
    Math::Vector    pos;
    float       angle, len, speed;

    CMotionVehicle* motionVehicle = dynamic_cast<CMotionVehicle*>(m_motion);
    assert(motionVehicle != nullptr);

    speed = m_physics->GetLinMotionX(MO_REASPEED);
    if ( speed > 0.0f )  oper = TO_ADVANCE;
    if ( speed < 0.0f )  oper = TO_RECEDE;

    speed = m_physics->GetCirMotionY(MO_REASPEED);
    if ( speed != 0.0f )  oper = TO_TURN;

    TraceColor color = TraceColor::Default;
    if ( motionVehicle->GetTraceDown() )  // pencil down?
    {
        color = motionVehicle->GetTraceColor();
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

void CBrain::TraceRecordStop()
{
    TraceOper   lastOper, curOper;
    float       lastParam, curParam;

    m_bTraceRecord = false;

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

    delete[] m_traceRecordBuffer;
    m_traceRecordBuffer = nullptr;

    buffer << "}\n";

    Program* prog = AddProgram();
    prog->script->SendScript(buffer.str().c_str());
}

// Saves an instruction CBOT.

bool CBrain::TraceRecordOper(TraceOper oper, float param)
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

bool CBrain::TraceRecordPut(std::stringstream& buffer, TraceOper oper, float param)
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

bool CBrain::IsTraceRecord()
{
    return m_bTraceRecord;
}

Program* CBrain::AddProgram()
{
    auto program = MakeUnique<Program>();
    program->script = MakeUnique<CScript>(m_object);
    program->readOnly = false;
    program->runnable = true;

    Program* prog = program.get();
    AddProgram(std::move(program));
    return prog;
}

void CBrain::AddProgram(std::unique_ptr<Program> program)
{
    m_program.push_back(std::move(program));
    m_object->UpdateInterface();
}

void CBrain::RemoveProgram(Program* program)
{
    if(m_currentProgram == program)
    {
        StopProgram();
    }
    m_program.erase(
        std::remove_if(m_program.begin(), m_program.end(),
            [program](std::unique_ptr<Program>& prog) { return prog.get() == program; }),
        m_program.end());
}

Program* CBrain::CloneProgram(Program* program)
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

std::vector<std::unique_ptr<Program>>& CBrain::GetPrograms()
{
    return m_program;
}

int CBrain::GetProgramCount()
{
    return static_cast<int>(m_program.size());
}

int CBrain::GetProgramIndex(Program* program)
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

Program* CBrain::GetProgram(int index)
{
    if(index < 0 || index >= static_cast<int>(m_program.size()))
        return nullptr;

    return m_program[index].get();
}

Program* CBrain::GetOrAddProgram(int index)
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
