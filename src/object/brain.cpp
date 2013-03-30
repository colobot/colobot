// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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


#include "object/brain.h"

#include "app/app.h"

#include "common/iman.h"
#include "common/misc.h"

#include "graphics/core/color.h"
#include "graphics/engine/terrain.h"

#include "object/motion/motion.h"
#include "object/task/taskmanager.h"

#include "physics/physics.h"

#include "script/cmdtoken.h"
#include "script/script.h"

#include "sound/sound.h"

#include "ui/displaytext.h"
#include "ui/interface.h"
#include "ui/slider.h"
#include "ui/studio.h"
#include "ui/window.h"



const int MAXTRACERECORD = 1000;



// Object's constructor.

CBrain::CBrain(CObject* object)
{
    CInstanceManager::GetInstancePointer()->AddInstance(CLASS_BRAIN, this, 100);

    m_object      = object;
    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_water       = m_engine->GetWater();
    m_particle    = m_engine->GetParticle();
    m_main        = CRobotMain::GetInstancePointer();
    m_terrain     = m_main->GetTerrain();
    m_camera      = m_main->GetCamera();
    m_interface   = m_main->GetInterface();
    m_displayText = m_main->GetDisplayText();
    m_sound       = CApplication::GetInstancePointer()->GetSound();
    m_physics     = nullptr;
    m_motion      = nullptr;
    m_primaryTask = nullptr;
    m_secondaryTask = nullptr;
    m_studio      = nullptr;

    m_program = -1;
    m_bActivity = true;
    m_bBurn = false;
    m_bActiveVirus = false;
    m_time = 0.0f;
    m_burnTime = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastHumanTime = 0.0f;
    m_lastWormTime = 0.0f;
    m_antTarget = 0;
    m_beeBullet = 0;
    m_lastAlarmTime = 0.0f;
    m_soundChannelAlarm = -1;
    m_flagColor = 0;

    m_buttonAxe    = EVENT_NULL;
    m_defaultEnter = EVENT_NULL;
    m_manipStyle   = EVENT_OBJECT_MFRONT;

    for (int i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        m_script[i] = 0;
        m_scriptName[i][0] = 0;
    }
    m_scriptRun = -1;
    m_soluceName[0] = 0;
    m_selScript = 0;

    m_bTraceRecord = false;
    m_traceRecordBuffer = nullptr;
}

// Object's destructor.

CBrain::~CBrain()
{
    for (int i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        delete m_script[i];
        m_script[i] = nullptr;
    }

    delete m_primaryTask;
    m_primaryTask = nullptr;

    delete m_secondaryTask;
    m_secondaryTask = nullptr;

    delete m_studio;
    m_studio = nullptr;

    delete[] m_traceRecordBuffer;
    m_traceRecordBuffer = nullptr;

    CInstanceManager::GetInstancePointer()->DeleteInstance(CLASS_BRAIN, this);
}


// Destroys the object.

void CBrain::DeleteObject(bool bAll)
{
    if ( m_soundChannelAlarm != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelAlarm);
        m_sound->AddEnvelope(m_soundChannelAlarm, 0.0f, 0.5f, 0.5f, SOPER_STOP);
        m_soundChannelAlarm = -1;
    }

    if ( !bAll )
    {
        if ( m_beeBullet != 0 )
        {
            m_beeBullet->DeleteObject();
            delete m_beeBullet;
            m_beeBullet = 0;
        }
    }

    if ( m_studio != 0 )  // current edition?
    {
        StopEditScript(true);
    }
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

bool CBrain::Write(char *line)
{
    char        name[100];

    sprintf(name, " bVirusActive=%d", m_bActiveVirus);
    strcat(line, name);

    return true;
}

// Restores all parameters of the object.

bool CBrain::Read(char *line)
{
    m_bActiveVirus = OpInt(line, "bVirusActive", 0);

    return true;
}


// Management of an event.

bool CBrain::EventProcess(const Event &event)
{
    Ui::CWindow*    pw;
    Ui::CControl*   pc;
    Ui::CSlider*    ps;
    EventType           action;
    ObjectType      type;
    Error           err;
    float           axeX, axeY, axeZ, factor;

    type = m_object->GetType();

    if ( m_primaryTask != 0 )  // current task?
    {
        m_primaryTask->EventProcess(event);
    }

    if ( m_secondaryTask != 0 )  // current task?
    {
        m_secondaryTask->EventProcess(event);
    }

    action = EVENT_NULL;

    if ( event.type == EVENT_KEY_DOWN &&
         (event.key.key == m_main->GetInputBinding(INPUT_SLOT_ACTION).primary ||
          event.key.key == m_main->GetInputBinding(INPUT_SLOT_ACTION).secondary ) &&
         !m_main->GetEditLock() )
    {
        pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
        if ( pw != 0 )
        {
            pc = pw->SearchControl(m_defaultEnter);
            if ( pc != 0 )
            {
                if ( pc->TestState(Ui::STATE_ENABLE) )
                {
                    action = m_defaultEnter;
                }
            }
        }
    }
    else
    {
        action = event.type;
    }

    if ( action == EVENT_NULL )  return true;

    if ( action == EVENT_UPDINTERFACE )
    {
        if ( m_object->GetSelect() )  CreateInterface(true);
    }

    if ( action == EVENT_FRAME )
    {
        EventFrame(event);
    }

    if ( m_object->GetSelect() &&  // robot selected?
         m_studio != 0          )   // current issue?
    {
        m_studio->EventProcess(event);

        if ( action == EVENT_OBJECT_PROGRUN )
        {
            if ( m_program == -1 )
            {
                RunProgram(m_selScript);
            }
            else
            {
                StopProgram();
            }
        }
        if ( action == EVENT_OBJECT_PROGSTART )
        {
            m_main->SaveOneScript(m_object);
            RunProgram(m_selScript);
        }
        if ( action == EVENT_OBJECT_PROGSTOP )
        {
            StopProgram();
        }
        if ( action == EVENT_STUDIO_OK )
        {
            StopEditScript(false);
            m_main->SaveOneScript(m_object);
        }
        if ( action == EVENT_STUDIO_CANCEL )
        {
            StopEditScript(true);
            m_main->SaveOneScript(m_object);
        }
        return true;
    }

    if ( !m_object->GetSelect() &&  // robot pas sélectionné  ?
         m_program == -1        &&
         m_primaryTask == 0     )
    {
        axeX = 0.0f;
        axeY = 0.0f;
        axeZ = 0.0f;
        if ( m_object->GetBurn() )  // Gifted?
        {
            if ( !m_bBurn )  // beginning?
            {
                m_bBurn = true;
                m_burnTime = 0.0f;
            }

            axeZ = -1.0f;  // tomb

            if ( !m_object->GetFixed() &&
                 (type == OBJECT_ANT    ||
                  type == OBJECT_SPIDER ||
                  type == OBJECT_WORM   ) )
            {
                axeY = 2.0f;  // zigzag disorganized fast
                if ( type == OBJECT_WORM )  axeY = 5.0f;
                axeX = 0.5f+sinf(m_time* 1.0f)*0.5f+
                            sinf(m_time* 6.0f)*2.0f+
                            sinf(m_time*21.0f)*0.2f;
                factor = 1.0f-m_burnTime/15.0f;  // slow motion
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

    if ( m_program != -1     &&
         m_object->GetRuin() )
    {
        StopProgram();
        return true;
    }

    if ( !m_object->GetSelect() )  // robot not selected?
    {
        return true;
    }

    if ( m_secondaryTask != 0 )  // current task?
    {
        if ( action == EVENT_OBJECT_ENDSHIELD )
        {
            m_secondaryTask->StartTaskShield(TSM_DOWN, 0.0f);
        }
    }
    if ( m_primaryTask != 0 ||  // current task?
         m_program != -1    )
    {
        if ( action == EVENT_OBJECT_PROGRUN )
        {
            StopProgram();
        }
        if ( action == EVENT_OBJECT_PROGEDIT )
        {
            StartEditScript(m_selScript, m_main->GetScriptName());
        }
        if ( m_primaryTask == 0 || !m_primaryTask->IsPilot() )  return true;
    }

    if ( action == EVENT_OBJECT_LEFT    ||
         action == EVENT_OBJECT_RIGHT   ||
         action == EVENT_OBJECT_UP      ||
         action == EVENT_OBJECT_DOWN    ||
         action == EVENT_OBJECT_GASUP   ||
         action == EVENT_OBJECT_GASDOWN )
    {
        m_buttonAxe = action;
    }
    if ( action == EVENT_MOUSE_BUTTON_UP )
    {
        m_buttonAxe = EVENT_NULL;
    }

    axeX = event.motionInput.x;
    axeY = event.motionInput.y;
    axeZ = event.motionInput.z;

    if ( !m_main->GetTrainerPilot() &&
         m_object->GetTrainer()     )  // drive vehicle?
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

    if ( m_object->GetManual() )  // scribbler in manual mode?
    {
        if ( axeX != 0.0f )  axeY = 0.0f;  // if running -> not moving!
        axeX *= 0.5f;
        axeY *= 0.5f;
    }

    if ( (g_researchDone&RESEARCH_FLY) == 0 )
    {
        axeZ = -1.0f;  // tomb
    }

    axeX += m_camera->GetMotorTurn();  // additional power according to camera
    if ( axeX >  1.0f )  axeX =  1.0f;
    if ( axeX < -1.0f )  axeX = -1.0f;

    m_physics->SetMotorSpeedX(axeY);  // move forward/move back
    m_physics->SetMotorSpeedY(axeZ);  // up/down
    m_physics->SetMotorSpeedZ(axeX);  // rotate

    if ( action == EVENT_OBJECT_PROGLIST )
    {
        m_selScript = GetSelScript();
        UpdateInterface();
    }

    if ( action == EVENT_OBJECT_PROGEDIT )
    {
        StartEditScript(m_selScript, m_main->GetScriptName());
    }

    if ( action == EVENT_OBJECT_PROGRUN )
    {
        StopProgram();  // stops the current program
        RunProgram(m_selScript);
        UpdateInterface();
    }

    err = ERR_OK;

    if ( m_program == -1 )
    {
        if ( action == EVENT_OBJECT_HTAKE )
        {
            err = StartTaskTake();
        }

        if ( action == EVENT_OBJECT_MFRONT ||
             action == EVENT_OBJECT_MBACK  ||
             action == EVENT_OBJECT_MPOWER )
        {
            m_manipStyle = action;
            UpdateInterface();
        }

        if ( action == EVENT_OBJECT_MTAKE )
        {
            if ( m_manipStyle == EVENT_OBJECT_MFRONT )
            {
                err = StartTaskManip(TMO_AUTO, TMA_FFRONT);
            }
            if ( m_manipStyle == EVENT_OBJECT_MBACK )
            {
                err = StartTaskManip(TMO_AUTO, TMA_FBACK);
                if ( err == ERR_OK )
                {
                    m_manipStyle = EVENT_OBJECT_MFRONT;
                    UpdateInterface();
                }
            }
            if ( m_manipStyle == EVENT_OBJECT_MPOWER )
            {
                err = StartTaskManip(TMO_AUTO, TMA_POWER);
                if ( err == ERR_OK )
                {
                    m_manipStyle = EVENT_OBJECT_MFRONT;
                    UpdateInterface();
                }
            }
        }

        if ( action == EVENT_OBJECT_BDERRICK )
        {
            err = StartTaskBuild(OBJECT_DERRICK);
        }
        if ( action == EVENT_OBJECT_BSTATION )
        {
            err = StartTaskBuild(OBJECT_STATION);
        }
        if ( action == EVENT_OBJECT_BFACTORY )
        {
            err = StartTaskBuild(OBJECT_FACTORY);
        }
        if ( action == EVENT_OBJECT_BREPAIR )
        {
            err = StartTaskBuild(OBJECT_REPAIR);
        }
        if ( action == EVENT_OBJECT_BCONVERT )
        {
            err = StartTaskBuild(OBJECT_CONVERT);
        }
        if ( action == EVENT_OBJECT_BTOWER )
        {
            err = StartTaskBuild(OBJECT_TOWER);
        }
        if ( action == EVENT_OBJECT_BRESEARCH )
        {
            err = StartTaskBuild(OBJECT_RESEARCH);
        }
        if ( action == EVENT_OBJECT_BRADAR )
        {
            err = StartTaskBuild(OBJECT_RADAR);
        }
        if ( action == EVENT_OBJECT_BENERGY )
        {
            err = StartTaskBuild(OBJECT_ENERGY);
        }
        if ( action == EVENT_OBJECT_BLABO )
        {
            err = StartTaskBuild(OBJECT_LABO);
        }
        if ( action == EVENT_OBJECT_BNUCLEAR )
        {
            err = StartTaskBuild(OBJECT_NUCLEAR);
        }
        if ( action == EVENT_OBJECT_BPARA )
        {
            err = StartTaskBuild(OBJECT_PARA);
        }
        if ( action == EVENT_OBJECT_BINFO )
        {
            err = StartTaskBuild(OBJECT_INFO);
        }
        if ( action == EVENT_OBJECT_BDESTROYER )
        {
            err = StartTaskBuild(OBJECT_DESTROYER);
        }

        if ( action == EVENT_OBJECT_GFLAT )
        {
            GroundFlat();
        }
        if ( action == EVENT_OBJECT_FCREATE )
        {
            err = StartTaskFlag(TFL_CREATE, m_flagColor);
        }
        if ( action == EVENT_OBJECT_FDELETE )
        {
            err = StartTaskFlag(TFL_DELETE, m_flagColor);
        }
        if ( action == EVENT_OBJECT_FCOLORb ||
             action == EVENT_OBJECT_FCOLORr ||
             action == EVENT_OBJECT_FCOLORg ||
             action == EVENT_OBJECT_FCOLORy ||
             action == EVENT_OBJECT_FCOLORv )
        {
            ColorFlag(action - EVENT_OBJECT_FCOLORb);
        }

        if ( action == EVENT_OBJECT_SEARCH )
        {
            err = StartTaskSearch();
        }

        if ( action == EVENT_OBJECT_TERRAFORM )
        {
            err = StartTaskTerraform();
        }

        if ( action == EVENT_OBJECT_RECOVER )
        {
            err = StartTaskRecover();
        }

        if ( action == EVENT_OBJECT_BEGSHIELD )
        {
            err = StartTaskShield(TSM_UP);
        }

        if ( action == EVENT_OBJECT_DIMSHIELD )
        {
            pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != 0 )
            {
                ps = static_cast< Ui::CSlider* >(pw->SearchControl(EVENT_OBJECT_DIMSHIELD));
                if ( ps != 0 )
                {
                    m_object->SetParam((ps->GetVisibleValue()-(RADIUS_SHIELD_MIN/g_unit))/((RADIUS_SHIELD_MAX-RADIUS_SHIELD_MIN)/g_unit));
                }
            }
        }

        if ( action == EVENT_OBJECT_FIRE && m_primaryTask == 0 && !m_object->GetTrainer())
        {
            if ( m_camera->GetType() != Gfx::CAM_TYPE_ONBOARD )
            {
                m_camera->SetType(Gfx::CAM_TYPE_ONBOARD);
            }
            err = StartTaskFire(0.0f);
        }
        if ( action == EVENT_OBJECT_TARGET && !m_object->GetTrainer() )
        {
            err = StartTaskGunGoal((event.mousePos.y-0.50f)*1.3f, (event.mousePos.x-0.50f)*2.0f);
        }

        if ( action == EVENT_OBJECT_FIREANT )
        {
//?         err = StartTaskFireAnt();
        }

        if ( action == EVENT_OBJECT_SPIDEREXPLO && m_primaryTask == 0 )
        {
            err = StartTaskSpiderExplo();
        }

        if ( action == EVENT_OBJECT_PEN0 )  // up
        {
            err = StartTaskPen(false, m_object->GetTraceColor());
            m_object->SetTraceDown(false);
        }
        if ( action == EVENT_OBJECT_PEN1 )  // black
        {
            err = StartTaskPen(true, 1);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(1);
        }
        if ( action == EVENT_OBJECT_PEN2 )  // yellow
        {
            err = StartTaskPen(true, 8);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(8);
        }
        if ( action == EVENT_OBJECT_PEN3 )  // orange
        {
            err = StartTaskPen(true, 7);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(7);
        }
        if ( action == EVENT_OBJECT_PEN4 )  // red
        {
            err = StartTaskPen(true, 4);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(4);
        }
        if ( action == EVENT_OBJECT_PEN5 )  // violet
        {
            err = StartTaskPen(true, 6);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(6);
        }
        if ( action == EVENT_OBJECT_PEN6 )  // blue
        {
            err = StartTaskPen(true, 14);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(14);
        }
        if ( action == EVENT_OBJECT_PEN7 )  // green
        {
            err = StartTaskPen(true, 12);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(12);
        }
        if ( action == EVENT_OBJECT_PEN8 )  // brown
        {
            err = StartTaskPen(true, 10);
            m_object->SetTraceDown(true);
            m_object->SetTraceColor(10);
        }

        if ( action == EVENT_OBJECT_REC )  // registered?
        {
            if ( m_bTraceRecord )
            {
                m_bTraceRecord = false;
                TraceRecordStop();
            }
            else
            {
                m_bTraceRecord = true;
                TraceRecordStart();
            }
            UpdateInterface();
            pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != 0 )
            {
                UpdateScript(pw);
            }
        }
        if ( action == EVENT_OBJECT_STOP )  // stops?
        {
            if ( m_bTraceRecord )
            {
                m_bTraceRecord = false;
                TraceRecordStop();
            }
            UpdateInterface();
            pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != 0 )
            {
                UpdateScript(pw);
            }
        }

        if ( action == EVENT_OBJECT_RESET )
        {
            m_main->ResetObject();  // reset all objects
            UpdateInterface();
        }

#if 0
        if ( event.param == 'T' )
        {
            Math::Vector    p1, p2;
            float       h;
            p1 = m_object->GetPosition(0);
            h = m_terrain->GetFloorLevel(p1);
            p2 = p1;
            p1.x -= 20.0f;
            p1.z -= 20.0f;
            p2.x += 20.0f;
            p2.z += 20.0f;
            m_terrain->Terraform(p1, p2, h+1.0f);
        }
        if ( event.param == 'R' )
        {
            Math::Vector    p1, p2;
            float       h;
            p1 = m_object->GetPosition(0);
            h = m_terrain->GetFloorLevel(p1);
            p2 = p1;
            p1.x -= 20.0f;
            p1.z -= 20.0f;
            p2.x += 20.0f;
            p2.z += 20.0f;
            m_terrain->Terraform(p1, p2, h-1.0f);
        }
#endif
    }

    if ( err != ERR_OK )
    {
        m_displayText->DisplayError(err, m_object);
    }

    return true;
}


// The brain is changing by time.

bool CBrain::EventFrame(const Event &event)
{
    m_time += event.rTime;
    if ( m_bBurn )  m_burnTime += event.rTime;

    if ( m_soundChannelAlarm != -1 )
    {
        m_sound->Position(m_soundChannelAlarm, m_object->GetPosition(0));
    }

    if ( m_studio != 0 )  // current edition?
    {
        m_studio->EventProcess(event);
    }

    UpdateInterface(event.rTime);

    if ( m_engine->GetPause() )  return true;
    if ( !m_bActivity )  return true;  // expected if idle
    if ( EndedTask() == ERR_CONTINUE )  return true;  // expected if not finished ...

    if ( m_program != -1 )  // current program?
    {
        if ( m_script[m_program]->Continue(event) )
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
    StopTask();

    if ( m_object->GetType() == OBJECT_HUMAN ||
         m_object->GetType() == OBJECT_TECH  )  return;

    if ( m_program != -1 &&
         m_script[m_program] != 0 )
    {
        m_script[m_program]->Stop();
    }

    BlinkScript(false);  // stops flashing

    m_program = -1;

    m_physics->SetMotorSpeedX(0.0f);
    m_physics->SetMotorSpeedY(0.0f);
    m_physics->SetMotorSpeedZ(0.0f);

    m_motion->SetAction(-1);

    UpdateInterface();
    m_main->UpdateShortcuts();
    m_object->CreateSelectParticle();
}

// Stops the current task.

void CBrain::StopTask()
{
    if ( m_primaryTask != 0 )
    {
        m_primaryTask->Abort();
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }
}


// Introduces a virus into a program.
// Returns true if it was inserted.

bool CBrain::IntroduceVirus()
{
    int     i, j;

    for ( i=0 ; i<50 ; i++ )
    {
        j = rand()%BRAINMAXSCRIPT;
        if ( m_script[j] != 0 )
        {
            if ( m_script[j]->IntroduceVirus() )  // tries to introduce
            {
                m_bActiveVirus = true;  // active virus
                return true;
            }
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


// Start editing a program.

void CBrain::StartEditScript(int rank, char* name)
{
    CreateInterface(false);  // removes the control buttons

    if ( m_script[rank] == 0 )
    {
        m_script[rank] = new CScript(m_object, &m_secondaryTask);
    }

    m_studio = new Ui::CStudio();
    m_studio->StartEditScript(m_script[rank], name, rank);
}

// End of editing a program.

void CBrain::StopEditScript(bool bCancel)
{
    if ( !bCancel )  SetActiveVirus(false);

    if ( !m_studio->StopEditScript(bCancel) )  return;

    delete m_studio;
    m_studio = 0;

    CreateInterface(true);  // puts the control buttons
}



// Move the manipulator arm.

Error CBrain::StartTaskTake()
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskTake();
    UpdateInterface();
    return err;
}

// Move the manipulator arm.

Error CBrain::StartTaskManip(TaskManipOrder order, TaskManipArm arm)
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskManip(order, arm);
    UpdateInterface();
    return err;
}

// Puts or removes a flag.

Error CBrain::StartTaskFlag(TaskFlagOrder order, int rank)
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskFlag(order, rank);
    UpdateInterface();
    return err;
}

// Built a building.

Error CBrain::StartTaskBuild(ObjectType type)
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskBuild(type);
    UpdateInterface();
    return err;
}

// Probe the ground.

Error CBrain::StartTaskSearch()
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskSearch();
    UpdateInterface();
    return err;
}

// Terraformed the ground.

Error CBrain::StartTaskTerraform()
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskTerraform();
    UpdateInterface();
    return err;
}

// Change pencil.

Error CBrain::StartTaskPen(bool bDown, int color)
{
    Error   err;

    m_physics->SetMotorSpeedX(0.0f);
    m_physics->SetMotorSpeedY(0.0f);
    m_physics->SetMotorSpeedZ(0.0f);

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskPen(bDown, color);
    UpdateInterface();
    return err;
}

// Recovers a ruin.

Error CBrain::StartTaskRecover()
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskRecover();
    UpdateInterface();
    return err;
}

// Deploys the shield.

Error CBrain::StartTaskShield(TaskShieldMode mode)
{
    Error   err;

    if ( m_secondaryTask != 0 )
    {
        delete m_secondaryTask;  // stops the current task
        m_secondaryTask = 0;
    }

    m_secondaryTask = new CTaskManager(m_object);
    err = m_secondaryTask->StartTaskShield(mode, 1000.0f);
    UpdateInterface();
    return err;
}

// Shoots.

Error CBrain::StartTaskFire(float delay)
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskFire(delay);
    UpdateInterface();
    return err;
}

// Explodes spider.

Error CBrain::StartTaskSpiderExplo()
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskSpiderExplo();
    UpdateInterface();
    return err;
}

// Shoots to the ant.

Error CBrain::StartTaskFireAnt(Math::Vector impact)
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskFireAnt(impact);
    UpdateInterface();
    return err;
}

// Adjusts upward.

Error CBrain::StartTaskGunGoal(float dirV, float dirH)
{
    Error   err;

    if ( m_secondaryTask != 0 )
    {
        delete m_secondaryTask;  // stops the current task
        m_secondaryTask = 0;
    }

    m_secondaryTask = new CTaskManager(m_object);
    err = m_secondaryTask->StartTaskGunGoal(dirV, dirH);
    UpdateInterface();
    return err;
}

// Reset.

Error CBrain::StartTaskReset(Math::Vector goal, Math::Vector angle)
{
    Error   err;

    if ( m_primaryTask != 0 )
    {
        delete m_primaryTask;  // stops the current task
        m_primaryTask = 0;
    }

    m_primaryTask = new CTaskManager(m_object);
    err = m_primaryTask->StartTaskReset(goal, angle);
    UpdateInterface();
    return err;
}

// Completes the task when the time came.

Error CBrain::EndedTask()
{
    Error   err;

    if ( m_secondaryTask != 0 )  // current task?
    {
        err = m_secondaryTask->IsEnded();
        if ( err != ERR_CONTINUE )  // job ended?
        {
            delete m_secondaryTask;
            m_secondaryTask = 0;
            UpdateInterface();
        }
    }

    if ( m_primaryTask != 0 )  // current task?
    {
        err = m_primaryTask->IsEnded();
        if ( err != ERR_CONTINUE )  // job ended?
        {
            delete m_primaryTask;
            m_primaryTask = 0;
            UpdateInterface();
        }
        return err;
    }
    return ERR_STOP;
}



// Shows flat areas in the field.

void CBrain::GroundFlat()
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    Error       err;
    float       level;

    if ( !m_physics->GetLand() )
    {
        err = ERR_FLAG_FLY;
        pos = m_object->GetPosition(0);
        if ( pos.y < m_water->GetLevel() )  err = ERR_FLAG_WATER;
        m_displayText->DisplayError(err, m_object);
        return;
    }

    pos = m_object->GetPosition(0);
    m_terrain->ShowFlatGround(pos);
    m_sound->Play(SOUND_GFLAT, pos);

    level = m_terrain->GetFloorLevel(pos)+2.0f;
    if ( pos.y < level )  pos.y = level;  // not below the soil
    speed = Math::Vector(0.0f, 0.0f, 0.0f);
    dim.x = 40.0f;
    dim.y = dim.x;
    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGFLAT, 1.0f);
}


// Not below the soil.

void CBrain::ColorFlag(int color)
{
    m_flagColor = color;
    UpdateInterface();
}


// Creates all the interface when the object is selected.

bool CBrain::CreateInterface(bool bSelect)
{
    ObjectType       type;
    Ui::CWindow*     pw;
    Ui::CButton*     pb;
    Ui::CSlider*     ps;
    Ui::CColor*      pc;
    Ui::CTarget*     pt;
    Ui::CLabel*      pl;
    Math::Point      pos, dim, ddim;
    float       ox, oy, sx, sy;
    char        name[100];

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != 0 )
    {
        pw->Flush();  // destroys the window buttons
        m_interface->DeleteControl(EVENT_WINDOW0);  // destroys the window
    }
    m_defaultEnter = EVENT_NULL;

    if ( !bSelect )  return true;

    pos.x = 0.0f;
    pos.y = 0.0f;
    dim.x = 540.0f/640.0f;
    if ( !m_main->GetShowMap() )  dim.x = 640.0f/640.0f;
    dim.y =  86.0f/480.0f;
    m_interface->CreateWindows(pos, dim, 3, EVENT_WINDOW0);
    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return false;

    m_object->GetTooltipName(name);
    pos.x = 0.0f;
    pos.y = 64.0f/480.0f;
    ddim.x = 540.0f/640.0f;
    if ( !m_main->GetShowMap() )  ddim.x = 640.0f/640.0f;
    ddim.y = 16.0f/480.0f;
    pw->CreateLabel(pos, ddim, 0, EVENT_LABEL0, name);

    dim.x = 33.0f/640.0f;
    dim.y = 33.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    type = m_object->GetType();

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     )  // vehicle?
    {
        if (!(m_main->GetRetroMode())) {
            ddim.x = dim.x*5.1f;
            ddim.y = dim.y*2.0f;
            pos.x = ox+sx*0.0f;
            pos.y = oy+sy*0.0f;
            pw->CreateList(pos, ddim, -1, EVENT_OBJECT_PROGLIST, 1.10f);
            UpdateScript(pw);

            pos.x = ox+sx*5.2f;
            pos.y = oy+sy*1.0f;
            pw->CreateButton(pos, dim, 8, EVENT_OBJECT_PROGRUN);
            pos.y = oy+sy*0.0f;
            pw->CreateButton(pos, dim, 22, EVENT_OBJECT_PROGEDIT);
        }
    }

    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_BEE      )  // driving?
    {
        pos.x = ox+sx*6.4f;
        pos.y = oy+sy*0;
        pb = pw->CreateButton(pos, dim, 29, EVENT_OBJECT_GASDOWN);
        pb->SetImmediat(true);

        pos.x = ox+sx*6.4f;
        pos.y = oy+sy*1;
        pb = pw->CreateButton(pos, dim, 28, EVENT_OBJECT_GASUP);
        pb->SetImmediat(true);

        if ( type != OBJECT_HUMAN       ||
             m_object->GetOption() != 2 )
        {
            pos.x = ox+sx*15.3f;
            pos.y = oy+sy*0;
            ddim.x = 14.0f/640.0f;
            ddim.y = 66.0f/480.0f;
            pw->CreateGauge(pos, ddim, 2, EVENT_OBJECT_GRANGE);
        }
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 31, EVENT_OBJECT_HTAKE);
        DefaultEnter(pw, EVENT_OBJECT_HTAKE);
    }

    if ( (type == OBJECT_MOBILEfa ||
          type == OBJECT_MOBILEta ||
          type == OBJECT_MOBILEwa ||
          type == OBJECT_MOBILEia ) &&  // arm?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 32, EVENT_OBJECT_MTAKE);
        DefaultEnter(pw, EVENT_OBJECT_MTAKE);

        pos.x = ox+sx*8.9f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 34, EVENT_OBJECT_MBACK);

        pos.x = ox+sx*9.9f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 35, EVENT_OBJECT_MPOWER);

        pos.x = ox+sx*10.9f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 33, EVENT_OBJECT_MFRONT);
    }

    if ( ( type == OBJECT_MOBILEsa &&  // underwater?
         !m_object->GetTrainer() ) ||
         type == OBJECT_BEE )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 32, EVENT_OBJECT_MTAKE);
        DefaultEnter(pw, EVENT_OBJECT_MTAKE);
    }

    if ( type == OBJECT_HUMAN )  // builder?
    {
        pos.x  =   1.0f/640.0f;
        pos.y  =   4.0f/480.0f;
        ddim.x = 212.0f/640.0f;
        ddim.y =  64.0f/480.0f;
        pw->CreateGroup(pos, ddim, 27, EVENT_NULL);

        ddim.x = dim.x*0.9f;
        ddim.y = dim.y*0.9f;

        pos.x = ox+sx*0.0f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+35, EVENT_OBJECT_BRESEARCH);
        DeadInterface(pw, EVENT_OBJECT_BRESEARCH, g_build&BUILD_RESEARCH);

        pos.x = ox+sx*0.9f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+32, EVENT_OBJECT_BFACTORY);
        DeadInterface(pw, EVENT_OBJECT_BFACTORY, g_build&BUILD_FACTORY);

        pos.x = ox+sx*1.8f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+34, EVENT_OBJECT_BCONVERT);
        DeadInterface(pw, EVENT_OBJECT_BCONVERT, g_build&BUILD_CONVERT);

        pos.x = ox+sx*2.7f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+36, EVENT_OBJECT_BSTATION);
        DeadInterface(pw, EVENT_OBJECT_BSTATION, g_build&BUILD_STATION);

        pos.x = ox+sx*3.6f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+40, EVENT_OBJECT_BRADAR);
        DeadInterface(pw, EVENT_OBJECT_BRADAR, g_build&BUILD_RADAR);

        pos.x = ox+sx*4.5f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+41, EVENT_OBJECT_BREPAIR);
        DeadInterface(pw, EVENT_OBJECT_BREPAIR, g_build&BUILD_REPAIR);

        pos.x = ox+sx*5.4f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+44, EVENT_OBJECT_BINFO);
        DeadInterface(pw, EVENT_OBJECT_BINFO, g_build&BUILD_INFO);

        pos.x = ox+sx*0.0f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+37, EVENT_OBJECT_BTOWER);
        DeadInterface(pw, EVENT_OBJECT_BTOWER,
                      (g_build&BUILD_TOWER) &&
                      (g_researchDone & RESEARCH_TOWER));

        pos.x = ox+sx*0.9f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+39, EVENT_OBJECT_BENERGY);
        DeadInterface(pw, EVENT_OBJECT_BENERGY, g_build&BUILD_ENERGY);

        pos.x = ox+sx*1.8f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+33, EVENT_OBJECT_BDERRICK);
        DeadInterface(pw, EVENT_OBJECT_BDERRICK, g_build&BUILD_DERRICK);

        pos.x = ox+sx*2.7f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+42, EVENT_OBJECT_BNUCLEAR);
        DeadInterface(pw, EVENT_OBJECT_BNUCLEAR,
                      (g_build&BUILD_NUCLEAR) &&
                      (g_researchDone & RESEARCH_ATOMIC));

        pos.x = ox+sx*3.6f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+38, EVENT_OBJECT_BLABO);
        DeadInterface(pw, EVENT_OBJECT_BLABO, g_build&BUILD_LABO);

        pos.x = ox+sx*4.5f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+46, EVENT_OBJECT_BPARA);
        DeadInterface(pw, EVENT_OBJECT_BPARA, g_build&BUILD_PARA);

        pos.x = ox+sx*5.4f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+41, EVENT_OBJECT_BDESTROYER);
        DeadInterface(pw, EVENT_OBJECT_BDESTROYER, g_build&BUILD_DESTROYER);

        if ( g_build&BUILD_GFLAT )
        {
            pos.x = ox+sx*9.0f;
            pos.y = oy+sy*0.5f;
            pw->CreateButton(pos, dim, 64+47, EVENT_OBJECT_GFLAT);
        }

        if ( g_build&BUILD_FLAG )
        {
            pos.x = ox+sx*10.1f;
            pos.y = oy+sy*0.5f;
            pw->CreateButton(pos, dim, 64+54, EVENT_OBJECT_FCREATE);

            pos.x = ox+sx*11.1f;
            pos.y = oy+sy*0.5f;
            pw->CreateButton(pos, dim, 64+55, EVENT_OBJECT_FDELETE);

            ddim.x = dim.x*0.4f;
            ddim.y = dim.y*0.4f;
            pos.x = ox+sx*10.1f;
            pos.y = oy+sy*2.0f-ddim.y;
            pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_FCOLORb);
            pc->SetColor(Gfx::Color(0.28f, 0.56f, 1.0f, 0.0f));
            pos.x += ddim.x;
            pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_FCOLORr);
            pc->SetColor(Gfx::Color(1.0f, 0.0f, 0.0f, 0.0f));
            pos.x += ddim.x;
            pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_FCOLORg);
            pc->SetColor(Gfx::Color(0.0f, 0.8f, 0.0f, 0.0f));
            pos.x += ddim.x;
            pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_FCOLORy);
            pc->SetColor(Gfx::Color(1.0f, 0.93f, 0.0f, 0.0f)); //0x00ffec00
            pos.x += ddim.x;
            pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_FCOLORv);
            pc->SetColor(Gfx::Color(0.82f, 0.004f, 0.99f, 0.0f)); //0x00d101fe
        }
    }

    if ( (type == OBJECT_MOBILEfs ||
          type == OBJECT_MOBILEts ||
          type == OBJECT_MOBILEws ||
          type == OBJECT_MOBILEis ) &&  // Investigator?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 40, EVENT_OBJECT_SEARCH);
        DefaultEnter(pw, EVENT_OBJECT_SEARCH);
    }

    if ( type == OBJECT_MOBILErt &&  // Terraformer?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 128+18, EVENT_OBJECT_TERRAFORM);
        DefaultEnter(pw, EVENT_OBJECT_TERRAFORM);

        pos.x = ox+sx*10.2f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 41, EVENT_OBJECT_LIMIT);
    }

    if ( type == OBJECT_MOBILErr &&  // recoverer?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 128+20, EVENT_OBJECT_RECOVER);
        DefaultEnter(pw, EVENT_OBJECT_RECOVER);
    }

    if ( type == OBJECT_MOBILErs &&  // shield?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 39, EVENT_OBJECT_BEGSHIELD);
        DefaultEnter(pw, EVENT_OBJECT_BEGSHIELD);

        pos.x = ox+sx*9.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 47, EVENT_OBJECT_ENDSHIELD);

//?     pos.x = ox+sx*10.2f;
//?     pos.y = oy+sy*0.5f;
//?     pw->CreateButton(pos, dim, 41, EVENT_OBJECT_LIMIT);

        pos.x = ox+sx*10.5f;
        pos.y = oy+sy*0.0f;
        ddim.x = dim.x*0.5f;
        ddim.y = dim.y*2.0f;
        ps = pw->CreateSlider(pos, ddim, 0, EVENT_OBJECT_DIMSHIELD);
        ps->SetState(Ui::STATE_VALUE);
        ps->SetLimit((RADIUS_SHIELD_MIN/g_unit), (RADIUS_SHIELD_MAX/g_unit));
        ps->SetArrowStep(1.0f);
    }

    if ( (type == OBJECT_MOBILEfc ||
          type == OBJECT_MOBILEtc ||
          type == OBJECT_MOBILEwc ||
          type == OBJECT_MOBILEic ||
          type == OBJECT_MOBILEfi ||
          type == OBJECT_MOBILEti ||
          type == OBJECT_MOBILEwi ||
          type == OBJECT_MOBILEii ||
          type == OBJECT_MOBILErc ) &&  // cannon?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pb = pw->CreateButton(pos, dim, 42, EVENT_OBJECT_FIRE);
        pb->SetImmediat(true);
        DefaultEnter(pw, EVENT_OBJECT_FIRE);

//?     pos.x = ox+sx*10.2f;
//?     pos.y = oy+sy*0.5f;
//?     pw->CreateButton(pos, dim, 41, EVENT_OBJECT_LIMIT);
    }

    if ( type == OBJECT_SPIDER )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pb = pw->CreateButton(pos, dim, 42, EVENT_OBJECT_SPIDEREXPLO);
        pb->SetImmediat(true);
        DefaultEnter(pw, EVENT_OBJECT_SPIDEREXPLO);
    }

    if ( type == OBJECT_MOBILEdr &&
         m_object->GetManual() )  // scribbler in manual mode?
    {
        pos.x = ox+sx*6.9f;
        pos.y = oy+sy*0.0f;
        ddim.x = dim.x*2.2f;
        ddim.y = dim.y*2.0f;
        pw->CreateGroup(pos, ddim, 20, EVENT_NULL);  // solid blue bottom

        pos.x = ox+sx*9.3f;
        pos.y = oy+sy*0.0f;
        ddim.x = dim.x*2.2f;
        ddim.y = dim.y*2.0f;
        pw->CreateGroup(pos, ddim, 20, EVENT_NULL);  // solid blue bottom

        pos.x = ox+sx*9.90f;
        pos.y = oy+sy*0.50f;
        pw->CreateButton(pos, dim, 43, EVENT_OBJECT_PEN0);

        ddim.x = dim.x*0.5f;
        ddim.y = dim.y*0.5f;
        pos.x = ox+sx*10.15f;
        pos.y = oy+sy*1.50f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN1);  // black
        pc->SetColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));
        pos.x = ox+sx*10.65f;
        pos.y = oy+sy*1.25f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN2);  // yellow
        pc->SetColor(Gfx::Color(1.0f, 1.0f, 0.0f, 0.0f ));
        pos.x = ox+sx*10.90f;
        pos.y = oy+sy*0.75f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN3);  // orange
        pc->SetColor(Gfx::Color(1.0f, 0.53f, 0x00, 0x00));
        pos.x = ox+sx*10.65f;
        pos.y = oy+sy*0.25f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN4);  // red
        pc->SetColor(Gfx::Color(1.0f, 0.0f, 0.0f, 0.0f));
        pos.x = ox+sx*10.15f;
        pos.y = oy+sy*0.00f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN5);  // violet
        pc->SetColor(Gfx::Color(1.0f, 0.0f, 1.0f, 0.0f));
        pos.x = ox+sx*9.65f;
        pos.y = oy+sy*0.25f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN6);  // blue
        pc->SetColor(Gfx::Color(0.0f, 0.4f, 1.0f, 0.0f));//0x000066ff));
        pos.x = ox+sx*9.40f;
        pos.y = oy+sy*0.75f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN7);  // green
        pc->SetColor(Gfx::Color(0.0f, 0.0f, 0.8f, 0.0f));//0x0000cc00));
        pos.x = ox+sx*9.65f;
        pos.y = oy+sy*1.25f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN8);  // brown
        pc->SetColor(Gfx::Color(0.53f, 0.27f, 0.0f, 0.0f));//0x00884400));

        pos.x = ox+sx*6.9f;
        pos.y = oy+sy*1.2f;
        ddim.x = dim.x*2.2f;
        ddim.y = dim.y*0.4f;
        GetResource(RES_TEXT, RT_INTERFACE_REC, name);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
        pl->SetFontSize(9.0f);

        pos.x = ox+sx*7.0f;
        pos.y = oy+sy*0.3f;
        pw->CreateButton(pos, dim, 44, EVENT_OBJECT_REC);
        pos.x = ox+sx*8.0f;
        pos.y = oy+sy*0.3f;
        pw->CreateButton(pos, dim, 45, EVENT_OBJECT_STOP);
    }

    if ( m_object->GetToy() )
    {
        pos.x = ox+sx*12.1f;
        pos.y = oy+sy*-0.1f;
        ddim.x = dim.x*1.2f;
        ddim.y = dim.y*2.1f;
        pw->CreateGroup(pos, ddim, 20, EVENT_NULL);  // solid blue bottom

        pos.x = ox+sx*12.2f;
        pos.y = oy+sy*1;
        pw->CreateGroup(pos, dim, 19, EVENT_NULL);  // sign SatCom

        pos.x = ox+sx*12.2f;
        pos.y = oy+sy*0.0f;
        pw->CreateButton(pos, dim, 128+57, EVENT_OBJECT_BHELP);
    }
    else
    {
        pos.x = ox+sx*12.3f;
        pos.y = oy+sy*-0.1f;
        ddim.x = dim.x*1.0f;
        ddim.y = dim.y*2.1f;
        pw->CreateGroup(pos, ddim, 20, EVENT_NULL);  // solid blue bottom

        pos.x = ox+sx*12.3f;
        pos.y = oy+sy*1;
        pw->CreateGroup(pos, dim, 19, EVENT_NULL);  // sign SatCom

        pos.x = ox+sx*12.4f;
        pos.y = oy+sy*0.5f;
        ddim.x = dim.x*0.8f;
        ddim.y = dim.y*0.5f;
        pw->CreateButton(pos, ddim, 18, EVENT_OBJECT_BHELP);
        pos.y = oy+sy*0.0f;
        pw->CreateButton(pos, ddim, 19, EVENT_OBJECT_HELP);
    }

    if ( type != OBJECT_HUMAN       &&
         type != OBJECT_TECH        &&
         !m_object->GetCameraLock() )
    {
//?     if ( m_main->GetShowMap() )
        if ( true )
        {
            pos.x = ox+sx*13.4f;
            pos.y = oy+sy*1;
            pw->CreateButton(pos, dim, 13, EVENT_OBJECT_CAMERA);
        }
        else
        {
            ddim.x = dim.x*0.66f;
            ddim.y = dim.y*0.66f;
            pos.x = ox+sx*(17.0f+0.66f);
            pos.y = oy+sy*0.66f;
            pw->CreateButton(pos, ddim, 13, EVENT_OBJECT_CAMERA);
        }
    }

    if ( m_object->GetToy() && !m_object->GetManual() )
    {
#if 0
        ddim.x = dim.x*0.66f;
        ddim.y = dim.y*0.66f;
        pos.x = ox+sx*10.0f;
        pos.y = oy+sy*0.66f;
        pb = pw->CreateButton(pos, ddim, 55, EVENT_OBJECT_CAMERAleft);
        pb->SetImmediat(true);
        pos.x = ox+sx*(10.0f+0.66f*2.0f);
        pos.y = oy+sy*0.66f;
        pb = pw->CreateButton(pos, ddim, 48, EVENT_OBJECT_CAMERAright);
        pb->SetImmediat(true);
        pos.x = ox+sx*(10.0f+0.66f);
        pos.y = oy+sy*(0.66f*2.0f);
        pb = pw->CreateButton(pos, ddim, 49, EVENT_OBJECT_CAMERAnear);
        pb->SetImmediat(true);
        pos.x = ox+sx*(10.0f+0.66f);
        pos.y = oy+sy*0.0f;
        pb = pw->CreateButton(pos, ddim, 50, EVENT_OBJECT_CAMERAaway);
        pb->SetImmediat(true);
#else
        pos.x = ox+sx*9.0f;
        pos.y = oy+sy*0;
        pb = pw->CreateButton(pos, dim, 55, EVENT_OBJECT_CAMERAleft);
        pb->SetImmediat(true);
        pos.x = ox+sx*11.0f;
        pos.y = oy+sy*0;
        pb = pw->CreateButton(pos, dim, 48, EVENT_OBJECT_CAMERAright);
        pb->SetImmediat(true);
        pos.x = ox+sx*10.0f;
        pos.y = oy+sy*1;
        pb = pw->CreateButton(pos, dim, 49, EVENT_OBJECT_CAMERAnear);
        pb->SetImmediat(true);
        pos.x = ox+sx*10.0f;
        pos.y = oy+sy*0;
        pb = pw->CreateButton(pos, dim, 50, EVENT_OBJECT_CAMERAaway);
        pb->SetImmediat(true);
#endif
    }

    pos.x = ox+sx*13.4f;
    pos.y = oy+sy*0;
#if _TEEN
    pw->CreateButton(pos, dim, 9, EVENT_OBJECT_RESET);
#else
    if ( m_object->GetTrainer() )  // Training?
    {
        pw->CreateButton(pos, dim, 9, EVENT_OBJECT_RESET);
    }
    else
    {
        pw->CreateButton(pos, dim, 10, EVENT_OBJECT_DESELECT);
    }
#endif

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit )  // vehicle?
    {
        pos.x = ox+sx*14.5f;
        pos.y = oy+sy*0;
        ddim.x = 14.0f/640.0f;
        ddim.y = 66.0f/480.0f;
        pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);
    }

    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit )  // vehicle?
    {
        pos.x = ox+sx*14.9f;
        pos.y = oy+sy*0;
        ddim.x = 14.0f/640.0f;
        ddim.y = 66.0f/480.0f;
        pw->CreateGauge(pos, ddim, 3, EVENT_OBJECT_GSHIELD);
    }

#if 0
    if ( false )
    {
        pos.x = 505.0f/640.0f;
        pos.y =   3.0f/480.0f;
        ddim.x = 33.0f/640.0f;
        ddim.y = 33.0f/480.0f;
        pw->CreateCompass(pos, ddim, 0, EVENT_OBJECT_COMPASS);

        pc = (CCompass*)pw->SearchControl(EVENT_OBJECT_COMPASS);
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_VISIBLE, m_main->GetShowMap());
        }
    }
#endif

    if ( type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILErc )  // cannon?
    {
        ddim.x = 64.0f/640.0f;
        ddim.y = 64.0f/480.0f;
        pos.x = 0.5f-ddim.x/2.0f;
        pos.y = 0.5f-ddim.y/2.0f;
        pw->CreateGroup(pos, ddim, 12, EVENT_OBJECT_CROSSHAIR);

        pos.x  =  20.0f/640.0f;
        pos.y  = 100.0f/480.0f;
        ddim.x = 600.0f/640.0f;
        ddim.y = 340.0f/480.0f;
        pt = pw->CreateTarget(pos, ddim, 0, EVENT_OBJECT_TARGET);
        pt->ClearState(Ui::STATE_GLINT);
    }

    ddim.x = 64.0f/640.0f;
    ddim.y = 64.0f/480.0f;
    pos.x =  30.0f/640.0f;
    pos.y = 430.0f/480.0f-ddim.y;
    pw->CreateGroup(pos, ddim, 13, EVENT_OBJECT_CORNERul);

    ddim.x = 64.0f/640.0f;
    ddim.y = 64.0f/480.0f;
    pos.x = 610.0f/640.0f-ddim.x;
    pos.y = 430.0f/480.0f-ddim.y;
    pw->CreateGroup(pos, ddim, 14, EVENT_OBJECT_CORNERur);

    ddim.x = 64.0f/640.0f;
    ddim.y = 64.0f/480.0f;
    pos.x =  30.0f/640.0f;
    pos.y = 110.0f/480.0f;
    pw->CreateGroup(pos, ddim, 15, EVENT_OBJECT_CORNERdl);

    ddim.x = 64.0f/640.0f;
    ddim.y = 64.0f/480.0f;
    pos.x = 610.0f/640.0f-ddim.x;
    pos.y = 110.0f/480.0f;
    pw->CreateGroup(pos, ddim, 16, EVENT_OBJECT_CORNERdr);

    UpdateInterface();
    m_lastUpdateTime = 0.0f;
    UpdateInterface(0.0f);

    return true;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CBrain::UpdateInterface(float rTime)
{
    Ui::CWindow*    pw;
    Ui::CButton*    pb;
    Ui::CGauge*     pg;
    Ui::CCompass*   pc;
    Ui::CGroup*     pgr;
    Ui::CTarget*    ptg;
    CObject*        power;
    Math::Vector    pos, hPos;
    Math::Point     ppos;
    float       energy, limit, angle, range;
    int         icon;
    bool        bOnBoard;

    m_lastAlarmTime += rTime;
    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )
    {
        if ( m_soundChannelAlarm != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelAlarm);
            m_sound->AddEnvelope(m_soundChannelAlarm, 0.0f, 1.0f, 0.1f, SOPER_STOP);
            m_soundChannelAlarm = -1;
        }
        return;
    }

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return;

    pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if ( pg != 0 )
    {
        power = m_object->GetPower();
        if ( power == 0 )
        {
            energy = 0.0f;
            limit = 0.0f;
        }
        else
        {
            energy = power->GetEnergy();
            limit = energy*power->GetCapacity();
        }
        icon = 0;  // red/green

        if ( limit < 0.2f && energy != 0.0f )  // low but not zero?
        {
            if ( m_lastAlarmTime >= 0.8f )  // blinks?
            {
                energy = 1.0f;
                icon = 1;  // brun
            }
            if ( m_lastAlarmTime >= 1.0f )
            {
                m_sound->Play(SOUND_ALARM, 0.5f);  // bip-bip-bip
                m_lastAlarmTime = 0.0f;
            }
        }
        pg->SetLevel(energy);
        pg->SetIcon(icon);
    }

    pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GSHIELD));
    if ( pg != 0 )
    {
        pg->SetLevel(m_object->GetShield());
    }

    pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GRANGE));
    if ( pg != 0 )
    {
        icon = 2;  // blue/red
        range = m_physics->GetReactorRange();

        if ( range < 0.2f && range != 0.0f && !m_physics->GetLand() )
        {
            if ( Math::Mod(m_time, 0.5f) >= 0.2f )  // blinks?
            {
                range = 1.0f;
                icon = 1;  // yellow
            }
            if ( m_soundChannelAlarm == -1 )
            {
                m_soundChannelAlarm = m_sound->Play(SOUND_ALARMt, m_object->GetPosition(0), 0.0f, 0.1f, true);
                m_sound->AddEnvelope(m_soundChannelAlarm, 1.0f, 1.0f, 1.0f, SOPER_CONTINUE);
                m_sound->AddEnvelope(m_soundChannelAlarm, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
            }
        }
        else
        {
            if ( m_soundChannelAlarm != -1 )
            {
                m_sound->FlushEnvelope(m_soundChannelAlarm);
                m_sound->AddEnvelope(m_soundChannelAlarm, 0.0f, 0.1f, 1.0f, SOPER_STOP);
                m_soundChannelAlarm = -1;
            }
        }

        pg->SetLevel(1.0f-range);
        pg->SetIcon(icon);
    }

    pc = static_cast< Ui::CCompass* >(pw->SearchControl(EVENT_OBJECT_COMPASS));
    if ( pc != 0 )
    {
        angle = -(m_object->GetAngleY(0)+Math::PI/2.0f);
        pc->SetDirection(angle);

        pc->SetState(Ui::STATE_VISIBLE, m_main->GetShowMap());
    }

    pb = static_cast<Ui::CButton*>(pw->SearchControl(EVENT_OBJECT_REC));
    if ( pb != 0 )
    {
        if ( m_bTraceRecord && Math::Mod(m_time, 0.4f) >= 0.2f )
        {
            pb->SetState(Ui::STATE_CHECK);
        }
        else
        {
            pb->ClearState(Ui::STATE_CHECK);
        }
    }

    bOnBoard = m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD;

    pgr = static_cast< Ui::CGroup* >(pw->SearchControl(EVENT_OBJECT_CROSSHAIR));
    if ( pgr != 0 )
    {
        if ( bOnBoard )
        {
#if 0
            angle = m_object->GetGunGoalV();
            if ( m_object->GetType() != OBJECT_MOBILErc )
            {
                angle += 10.0f*Math::PI/360.0f;
            }
            ppos.x = 0.5f-(64.0f/640.0f)/2.0f;
            ppos.y = 0.5f-(64.0f/480.0f)/2.0f;
            ppos.y += sinf(angle)*0.6f;
            pgr->SetPos(ppos);
#else
            ppos.x = 0.50f-(64.0f/640.0f)/2.0f;
            ppos.y = 0.50f-(64.0f/480.0f)/2.0f;
            ppos.x += m_object->GetGunGoalH()/2.0f;
            ppos.y += m_object->GetGunGoalV()/1.3f;
            pgr->SetPos(ppos);
#endif
            pgr->SetState(Ui::STATE_VISIBLE, !m_main->GetFriendAim());
        }
        else
        {
            pgr->ClearState(Ui::STATE_VISIBLE);
        }
    }

    ptg = static_cast< Ui::CTarget* >(pw->SearchControl(EVENT_OBJECT_TARGET));
    if ( ptg != 0 )
    {
        if ( bOnBoard )
        {
            ptg->SetState(Ui::STATE_VISIBLE);
        }
        else
        {
            ptg->ClearState(Ui::STATE_VISIBLE);
        }
    }

    pgr = static_cast< Ui::CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERul));
    if ( pgr != 0 )
    {
        pgr->SetState(Ui::STATE_VISIBLE, bOnBoard);
    }

    pgr = static_cast< Ui::CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERur));
    if ( pgr != 0 )
    {
        pgr->SetState(Ui::STATE_VISIBLE, bOnBoard);
    }

    pgr = static_cast< Ui::CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERdl));
    if ( pgr != 0 )
    {
        pgr->SetState(Ui::STATE_VISIBLE, bOnBoard);
    }

    pgr = static_cast< Ui::CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERdr));
    if ( pgr != 0 )
    {
        pgr->SetState(Ui::STATE_VISIBLE, bOnBoard);
    }
}

// Updates the status of all interface buttons.

void CBrain::UpdateInterface()
{
    ObjectType  type;
    Ui::CWindow*    pw;
    Ui::CButton*    pb;
    Ui::CSlider*    ps;
    Ui::CColor*     pc;
    int         color;
    bool        bEnable, bFly, bRun;
    char        title[100];

    if ( !m_object->GetSelect() )  return;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return;

    type = m_object->GetType();

    bEnable = ( m_secondaryTask == 0 && m_program == -1 );

    bEnable = ( m_primaryTask == 0 && m_program == -1 );

    EnableInterface(pw, EVENT_OBJECT_PROGEDIT,    (m_primaryTask == 0 && !m_bTraceRecord));
    EnableInterface(pw, EVENT_OBJECT_PROGLIST,    bEnable && !m_bTraceRecord);
    EnableInterface(pw, EVENT_OBJECT_LEFT,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_RIGHT,       bEnable);
    EnableInterface(pw, EVENT_OBJECT_UP,          bEnable);
    EnableInterface(pw, EVENT_OBJECT_DOWN,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_HTAKE,       bEnable);
    EnableInterface(pw, EVENT_OBJECT_MTAKE,       bEnable);
    EnableInterface(pw, EVENT_OBJECT_MBACK,       bEnable);
    EnableInterface(pw, EVENT_OBJECT_MPOWER,      bEnable);
    EnableInterface(pw, EVENT_OBJECT_MFRONT,      bEnable);
    EnableInterface(pw, EVENT_OBJECT_GFLAT,       bEnable);
    EnableInterface(pw, EVENT_OBJECT_FCREATE,     bEnable);
    EnableInterface(pw, EVENT_OBJECT_FDELETE,     bEnable);
    EnableInterface(pw, EVENT_OBJECT_SEARCH,      bEnable);
    EnableInterface(pw, EVENT_OBJECT_TERRAFORM,   bEnable);
    EnableInterface(pw, EVENT_OBJECT_RECOVER,     bEnable);
    EnableInterface(pw, EVENT_OBJECT_FIRE,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_SPIDEREXPLO, bEnable);
    EnableInterface(pw, EVENT_OBJECT_RESET,       bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN0,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN1,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN2,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN3,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN4,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN5,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN6,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN7,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_PEN8,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_REC,         bEnable);
    EnableInterface(pw, EVENT_OBJECT_STOP,        bEnable);

    if ( type == OBJECT_HUMAN )  // builder?
    {
        EnableInterface(pw, EVENT_OBJECT_BFACTORY,  bEnable);
        EnableInterface(pw, EVENT_OBJECT_BDERRICK,  bEnable);
        EnableInterface(pw, EVENT_OBJECT_BCONVERT,  bEnable);
        EnableInterface(pw, EVENT_OBJECT_BSTATION,  bEnable);
        EnableInterface(pw, EVENT_OBJECT_BREPAIR,   bEnable);
        EnableInterface(pw, EVENT_OBJECT_BTOWER,    bEnable);
        EnableInterface(pw, EVENT_OBJECT_BRESEARCH, bEnable);
        EnableInterface(pw, EVENT_OBJECT_BRADAR,    bEnable);
        EnableInterface(pw, EVENT_OBJECT_BENERGY,   bEnable);
        EnableInterface(pw, EVENT_OBJECT_BLABO,     bEnable);
        EnableInterface(pw, EVENT_OBJECT_BNUCLEAR,  bEnable);
        EnableInterface(pw, EVENT_OBJECT_BPARA,     bEnable);
        EnableInterface(pw, EVENT_OBJECT_BINFO,     bEnable);
        EnableInterface(pw, EVENT_OBJECT_BDESTROYER,bEnable);
    }

    pb = static_cast< Ui::CButton* >(pw->SearchControl(EVENT_OBJECT_GFLAT));
    if ( pb != 0 )
    {
        pb->SetState(Ui::STATE_VISIBLE, m_engine->GetGroundSpot());
    }

    if ( type == OBJECT_HUMAN ||  // builder?
         type == OBJECT_TECH  )
    {
        CheckInterface(pw, EVENT_OBJECT_FCOLORb, m_flagColor==0);
        CheckInterface(pw, EVENT_OBJECT_FCOLORr, m_flagColor==1);
        CheckInterface(pw, EVENT_OBJECT_FCOLORg, m_flagColor==2);
        CheckInterface(pw, EVENT_OBJECT_FCOLORy, m_flagColor==3);
        CheckInterface(pw, EVENT_OBJECT_FCOLORv, m_flagColor==4);
    }

    if ( type == OBJECT_MOBILErs )  // shield?
    {
        if ( (m_secondaryTask == 0 || !m_secondaryTask->IsBusy()) && m_program == -1 )
        {
            EnableInterface(pw, EVENT_OBJECT_BEGSHIELD, (m_secondaryTask == 0));
            EnableInterface(pw, EVENT_OBJECT_ENDSHIELD, (m_secondaryTask != 0));
            DefaultEnter   (pw, EVENT_OBJECT_BEGSHIELD, (m_secondaryTask == 0));
            DefaultEnter   (pw, EVENT_OBJECT_ENDSHIELD, (m_secondaryTask != 0));
        }
        else
        {
            EnableInterface(pw, EVENT_OBJECT_BEGSHIELD, false);
            EnableInterface(pw, EVENT_OBJECT_ENDSHIELD, false);
            DefaultEnter   (pw, EVENT_OBJECT_BEGSHIELD, false);
            DefaultEnter   (pw, EVENT_OBJECT_ENDSHIELD, false);
        }

        ps = static_cast< Ui::CSlider* >(pw->SearchControl(EVENT_OBJECT_DIMSHIELD));
        if ( ps != 0 )
        {
            ps->SetVisibleValue((RADIUS_SHIELD_MIN/g_unit)+m_object->GetParam()*((RADIUS_SHIELD_MAX-RADIUS_SHIELD_MIN)/g_unit));
        }
    }

    bFly = bEnable;
    if ( bFly && (type == OBJECT_HUMAN || type == OBJECT_TECH) )
    {
        if ( m_object->GetFret() != 0 )  bFly = false;  // if holder -> not fly
    }
    EnableInterface(pw, EVENT_OBJECT_GASUP,   bFly);
    EnableInterface(pw, EVENT_OBJECT_GASDOWN, bFly);
    if ( m_object->GetTrainer() )  // Training?
    {
        DeadInterface(pw, EVENT_OBJECT_GASUP,   false);
        DeadInterface(pw, EVENT_OBJECT_GASDOWN, false);
    }
    else
    {
        DeadInterface(pw, EVENT_OBJECT_GASUP,   g_researchDone&RESEARCH_FLY);
        DeadInterface(pw, EVENT_OBJECT_GASDOWN, g_researchDone&RESEARCH_FLY);
    }

    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     )  // vehicle?
    {
        bRun = false;
        if ( m_script[m_selScript] != 0 )
        {
            m_script[m_selScript]->GetTitle(title);
            if ( title[0] != 0 )
            {
                bRun = true;
            }
        }
        if ( !bEnable && m_program == -1 )  bRun = false;
        if ( m_bTraceRecord )  bRun = false;
        EnableInterface(pw, EVENT_OBJECT_PROGRUN, bRun);

        pb = static_cast< Ui::CButton* >(pw->SearchControl(EVENT_OBJECT_PROGRUN));
        if ( pb != 0 )
        {
            pb->SetIcon(m_program==-1?21:8);  // run/stop
        }

//?     pb = (CButton*)pw->SearchControl(EVENT_OBJECT_PROGEDIT);
//?     if ( pb != 0 )
//?     {
//?         pb->SetIcon(m_program==-1?22:40);  // edit/debug
//?     }

        BlinkScript(m_program != -1);  // blinks if script execution
    }

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia )  // arm?
    {
        CheckInterface(pw, EVENT_OBJECT_MPOWER, m_manipStyle==EVENT_OBJECT_MPOWER);
        CheckInterface(pw, EVENT_OBJECT_MBACK,  m_manipStyle==EVENT_OBJECT_MBACK);
        CheckInterface(pw, EVENT_OBJECT_MFRONT, m_manipStyle==EVENT_OBJECT_MFRONT);
    }

    if ( m_object->GetTraceDown() )
    {
        pb = static_cast< Ui::CButton* >(pw->SearchControl(EVENT_OBJECT_PEN0));
        if ( pb != 0 )
        {
            pb->ClearState(Ui::STATE_CHECK);
        }

        color = m_object->GetTraceColor();
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN1));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==1);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN2));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==8);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN3));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==7);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN4));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==4);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN5));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==6);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN6));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==14);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN7));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==12);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN8));
        if ( pc != 0 )
        {
            pc->SetState(Ui::STATE_CHECK, color==10);
        }
    }
    else
    {
        pb = static_cast< Ui::CButton* >(pw->SearchControl(EVENT_OBJECT_PEN0));
        if ( pb != 0 )
        {
            pb->SetState(Ui::STATE_CHECK);
        }

        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN1));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN2));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN3));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN4));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN5));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN6));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN7));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
        pc = static_cast< Ui::CColor* >(pw->SearchControl(EVENT_OBJECT_PEN8));
        if ( pc != 0 )
        {
            pc->ClearState(Ui::STATE_CHECK);
        }
    }
}

// Updates the list of programs.

void CBrain::UpdateScript(Ui::CWindow *pw)
{
    Ui::CList*      pl;
    char        name[100];
    char        title[100];
    int         i;
    bool        bSoluce;

    pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == 0 )  return;

#if _SCHOOL
    bSoluce = m_main->GetSoluce4();
#else
    bSoluce = true;
#endif

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        sprintf(name, "%d", i+1);

        if ( m_script[i] != 0 )
        {
            m_script[i]->GetTitle(title);
            if ( !bSoluce && i == 3 )
            {
                title[0] = 0;
            }
            if ( title[0] != 0 )
            {
                sprintf(name, "%d: %s", i+1, title);
            }
        }

        pl->SetName(i, name);
    }

    if ( !bSoluce )
    {
        pl->SetEnable(3, false);
    }

    pl->SetSelect(m_selScript);
    pl->ShowSelect(true);
}

// Returns the rank of selected script.

int CBrain::GetSelScript()
{
    Ui::CWindow*    pw;
    Ui::CList*      pl;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return -1;

    pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == 0 )  return -1;

    return pl->GetSelect();
}

// Blinks the running program.

void CBrain::BlinkScript(bool bEnable)
{
    Ui::CWindow*    pw;
    Ui::CList*      pl;

    if ( !m_object->GetSelect() )  return;  // robot not selected?

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return;

    pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == 0 )  return;

    pl->SetBlink(bEnable);
}

// Check the status of a button interface.

void CBrain::CheckInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(Ui::STATE_CHECK, bState);
}

// Changes the state of a button interface.

void CBrain::EnableInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(Ui::STATE_ENABLE, bState);
}

// Changes the state of a button on the interface.

void CBrain::DeadInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(Ui::STATE_DEAD, !bState);
}

// Change the default input state of a button interface.

void CBrain::DefaultEnter(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    if ( bState )
    {
        control->SetState(Ui::STATE_DEFAULT);
        m_defaultEnter = event;
    }
    else
    {
        control->ClearState(Ui::STATE_DEFAULT);
    }
}


// Indicates whether the object is busy with a task.

bool CBrain::IsBusy()
{
    return (m_primaryTask != 0);
}

// Management of the activity of an object.

void CBrain::SetActivity(bool bMode)
{
    m_bActivity = bMode;
}

bool CBrain::GetActivity()
{
    return m_bActivity;
}

// Indicates whether a program is running.

bool CBrain::IsProgram()
{
    return ( m_program != -1 );
}

// Indicates whether a program exists.

bool CBrain::ProgramExist(int rank)
{
    return ( m_script[rank] != 0 );
}

// Starts a program.

void CBrain::RunProgram(int rank)
{
    if ( rank < 0 )  return;

    if ( m_script[rank] != 0 &&
         m_script[rank]->Run() )
    {
        m_program = rank;  // start new program
        BlinkScript(true);  // blink
        m_object->CreateSelectParticle();
        m_main->UpdateShortcuts();
    }
}

// Returns the first free program.

int CBrain::FreeProgram()
{
    int     i;

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        if ( m_script[i] == 0 )  return i;
    }
    return -1;
}


// Returns the current program.

int CBrain::GetProgram()
{
    return m_program;
}


// Name management scripts to load.

void CBrain::SetScriptRun(int rank)
{
    m_scriptRun = rank;
}

int CBrain::GetScriptRun()
{
    return m_scriptRun;
}

void CBrain::SetScriptName(int rank, char *name)
{
    strcpy(m_scriptName[rank], name);
}

char* CBrain::GetScriptName(int rank)
{
    return m_scriptName[rank];
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
    int     rank, i;

    rank = FreeProgram();
    if ( rank == -1 )  return false;

    if ( !ReadProgram(rank, filename) )  return false;  // load solution

    for ( i=0 ; i<BRAINMAXSCRIPT ; i++ )
    {
        if ( i == rank || m_script[i] == 0 )  continue;

        if ( m_script[i]->Compare(m_script[rank]) )  // the same already?
        {
            delete m_script[rank];
            m_script[rank] = 0;
            return false;
        }
    }

    return true;
}

// Load a script with a text file.

bool CBrain::ReadProgram(int rank, const char* filename)
{
    if ( m_script[rank] == 0 )
    {
        m_script[rank] = new CScript(m_object, &m_secondaryTask);
    }

    if ( m_script[rank]->ReadScript(filename) )  return true;

    delete m_script[rank];
    m_script[rank] = 0;

    return false;
}

// Indicates whether a program is compiled correctly.

bool CBrain::GetCompile(int rank)
{
    if ( m_script[rank] == 0 )  return false;
    return m_script[rank]->GetCompile();
}

// Saves a script in a text file.

bool CBrain::WriteProgram(int rank, char* filename)
{
    if ( m_script[rank] == 0 )
    {
        m_script[rank] = new CScript(m_object, &m_secondaryTask);
    }

    if ( m_script[rank]->WriteScript(filename) )  return true;

    delete m_script[rank];
    m_script[rank] = 0;

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
        if ( op >= 0 && op < BRAINMAXSCRIPT )
        {
            m_program = op;  // program restarts
            m_selScript = op;
            BlinkScript(true);  // blink

            if ( m_script[op] == 0 )
            {
                m_script[op] = new CScript(m_object, &m_secondaryTask);
            }
            if ( !m_script[op]->ReadStack(file) )  return false;
        }
    }

    return true;
}

// ave the script implementation stack of a file.

bool CBrain::WriteStack(FILE *file)
{
    short       op;

    if ( m_program != -1 &&  // current program?
         m_script[m_program]->IsRunning() )
    {
        op = 1;  // run
        fWrite(&op, sizeof(short), 1, file);

        op = m_program;
        fWrite(&op, sizeof(short), 1, file);

        return m_script[m_program]->WriteStack(file);
    }

    op = 0;  // stop
    fWrite(&op, sizeof(short), 1, file);
    return true;
}



// Start of registration of the design.

void CBrain::TraceRecordStart()
{
    m_traceOper = TO_STOP;

    m_tracePos = m_object->GetPosition(0);
    m_traceAngle = m_object->GetAngleY(0);

    if ( m_object->GetTraceDown() )  // pencil down?
    {
        m_traceColor = m_object->GetTraceColor();
    }
    else    // pen up?
    {
        m_traceColor = -1;
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
    int         color;

    speed = m_physics->GetLinMotionX(MO_REASPEED);
    if ( speed > 0.0f )  oper = TO_ADVANCE;
    if ( speed < 0.0f )  oper = TO_RECEDE;

    speed = m_physics->GetCirMotionY(MO_REASPEED);
    if ( speed != 0.0f )  oper = TO_TURN;

    if ( m_object->GetTraceDown() )  // pencil down?
    {
        color = m_object->GetTraceColor();
    }
    else    // pen up?
    {
        color = -1;
    }

    if ( oper != m_traceOper ||
         color != m_traceColor )
    {
        if ( m_traceOper == TO_ADVANCE ||
             m_traceOper == TO_RECEDE  )
        {
            pos = m_object->GetPosition(0);
            len = Math::DistanceProjected(pos, m_tracePos);
            TraceRecordOper(m_traceOper, len);
        }
        if ( m_traceOper == TO_TURN )
        {
            angle = m_object->GetAngleY(0)-m_traceAngle;
            TraceRecordOper(m_traceOper, angle);
        }

        if ( color != m_traceColor )
        {
            TraceRecordOper(TO_PEN, static_cast<float>(color));
        }

        m_traceOper = oper;
        m_tracePos = m_object->GetPosition(0);
        m_traceAngle = m_object->GetAngleY(0);
        m_traceColor = color;
    }
}

// End of the registration of the design. Program generates the CBOT.

void CBrain::TraceRecordStop()
{
    TraceOper   lastOper, curOper;
    float       lastParam, curParam;
    int         max, i;
    char*       buffer;

    if ( m_traceRecordBuffer == nullptr )  return;

    max = 10000;
    buffer = new char[max];
    *buffer = 0;
    strncat(buffer, "extern void object::AutoDraw()\n{\n", max-1);

    lastOper = TO_STOP;
    lastParam = 0.0f;
    for ( i=0 ; i<m_traceRecordIndex ; i++ )
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
            TraceRecordPut(buffer, max, lastOper, lastParam);
            lastOper = curOper;
            lastParam = curParam;
        }
    }
    TraceRecordPut(buffer, max, lastOper, lastParam);

    delete[] m_traceRecordBuffer;
    m_traceRecordBuffer = nullptr;

    strncat(buffer, "}\n", max-1);
    buffer[max-1] = 0;

    i = m_selScript;
    if ( m_script[i] == 0 )
    {
        m_script[i] = new CScript(m_object, &m_secondaryTask);
    }
    m_script[i]->SendScript(buffer);
    delete[] buffer;
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

bool CBrain::TraceRecordPut(char *buffer, int max, TraceOper oper, float param)
{
    char    line[100];
    int     color;

    if ( oper == TO_ADVANCE )
    {
        param /= g_unit;
        sprintf(line, "\tmove(%.1f);\n", param);
        strncat(buffer, line, max-1);
    }

    if ( oper == TO_RECEDE )
    {
        param /= g_unit;
        sprintf(line, "\tmove(-%.1f);\n", param);
        strncat(buffer, line, max-1);
    }

    if ( oper == TO_TURN )
    {
        param = -param*180.0f/Math::PI;
        sprintf(line, "\tturn(%d);\n", static_cast<int>(param));
//?     sprintf(line, "\tturn(%.1f);\n", param);
        strncat(buffer, line, max-1);
    }

    if ( oper == TO_PEN )
    {
        color = static_cast<int>(param);
        if ( color == -1 )  strncat(buffer, "\tpenup();\n",         max-1);
        if ( color ==  1 )  strncat(buffer, "\tpendown(Black);\n",  max-1);
        if ( color ==  8 )  strncat(buffer, "\tpendown(Yellow);\n", max-1);
        if ( color ==  7 )  strncat(buffer, "\tpendown(Orange);\n", max-1);
        if ( color ==  4 )  strncat(buffer, "\tpendown(Red);\n",    max-1);
        if ( color ==  6 )  strncat(buffer, "\tpendown(Purple);\n", max-1);
        if ( color == 14 )  strncat(buffer, "\tpendown(Blue);\n",   max-1);
        if ( color == 12 )  strncat(buffer, "\tpendown(Green);\n",  max-1);
        if ( color == 10 )  strncat(buffer, "\tpendown(Brown);\n",  max-1);
    }

    return true;
}

