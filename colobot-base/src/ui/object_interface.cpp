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

#include "ui/object_interface.h"

#include "app/app.h"

#include "common/global.h"
#include "common/restext.h"

#include "graphics/core/color.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/old_object.h"

#include "object/interface/programmable_object.h"
#include "object/interface/slotted_object.h"
#include "object/interface/task_executor_object.h"

#include "object/motion/motion.h"
#include "object/motion/motionvehicle.h"

#include "object/subclass/shielder.h"

#include "physics/physics.h"

#include "script/script.h"

#include "sound/sound.h"

#include "ui/studio.h"

#include "ui/controls/button.h"
#include "ui/controls/color.h"
#include "ui/controls/gauge.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/slider.h"
#include "ui/controls/target.h"
#include "ui/controls/window.h"


namespace Ui
{

CObjectInterface::CObjectInterface(COldObject* object)
{
    m_object      = object;
    assert(object->Implements(ObjectInterfaceType::TaskExecutor));
    m_taskExecutor = dynamic_cast<CTaskExecutorObject*>(m_object);
    assert(object->Implements(ObjectInterfaceType::Programmable));
    m_programmable = dynamic_cast<CProgrammableObject*>(m_object);
    assert(object->Implements(ObjectInterfaceType::ProgramStorage));
    m_programStorage = dynamic_cast<CProgramStorageObject*>(m_object);

    m_physics     = m_object->GetPhysics();
    m_motion      = m_object->GetMotion();

    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_water       = m_engine->GetWater();
    m_particle    = m_engine->GetParticle();
    m_main        = CRobotMain::GetInstancePointer();
    m_terrain     = m_main->GetTerrain();
    m_camera      = m_main->GetCamera();
    m_interface   = m_main->GetInterface();
    m_sound       = CApplication::GetInstancePointer()->GetSound();

    m_time = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastAlarmTime = 0.0f;
    m_soundChannelAlarm = -1;
    m_flagColor = 0;

    m_defaultEnter = EVENT_NULL;
    m_manipStyle   = EVENT_OBJECT_MFRONT;

    m_selScript = 0;

    m_buildInterface = false;
}

// Object's destructor.

CObjectInterface::~CObjectInterface()
{
}


// Destroys the object.

void CObjectInterface::DeleteObject(bool all)
{
    if ( m_soundChannelAlarm != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelAlarm);
        m_sound->AddEnvelope(m_soundChannelAlarm, 0.0f, 0.5f, 0.5f, SOPER_STOP);
        m_soundChannelAlarm = -1;
    }

    if ( m_studio != nullptr )  // current edition?
    {
        StopEditScript(true);
    }
}

// Management of an event.

bool CObjectInterface::EventProcess(const Event &event)
{
    CWindow*    pw;
    CControl*   pc;
    CSlider*    ps;
    EventType   action;
    Error       err;

    action = EVENT_NULL;

    bool isActionSlot = false;
    if (event.type == EVENT_KEY_DOWN && !m_main->GetEditLock())
    {
        auto data = event.GetData<KeyEventData>();
        isActionSlot = data->slot == INPUT_SLOT_ACTION;
    }

    if (isActionSlot)
    {
        pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
        if ( pw != nullptr )
        {
            pc = pw->SearchControl(m_defaultEnter);
            if ( pc != nullptr )
            {
                if ( pc->TestState(STATE_ENABLE) )
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

    if (event.type == EVENT_KEY_DOWN && m_object->GetSelect())
    {
        auto data = event.GetData<KeyEventData>();

        bool control = (event.kmodState & KEY_MOD(CTRL)) != 0;
        bool alt = (event.kmodState & KEY_MOD(ALT)) != 0;
        CEventQueue* queue = CApplication::GetInstancePointer()->GetEventQueue();

        if (data->slot == INPUT_SLOT_ACTION && control)
        {
            queue->AddEvent(Event(m_studio == nullptr ? EVENT_OBJECT_PROGEDIT : EVENT_STUDIO_OK));
            return false;
        }

        if (data->slot == INPUT_SLOT_ACTION && alt)
        {
            pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                CButton* pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_PROGRUN));
                if(pb != nullptr)
                {
                    if(pb->TestState(STATE_ENABLE))
                    {
                        queue->AddEvent(Event(EVENT_OBJECT_PROGRUN));
                    }
                }
            }
            return false;
        }

        if (alt)
        {
            int index = GetSelScript();
            if(data->slot == INPUT_SLOT_UP)
                index--;
            else if(data->slot == INPUT_SLOT_DOWN)
                index++;
            else if(data->key >= KEY(1) && data->key <= KEY(9))
                index = data->key-KEY(1);
            else if(data->key == KEY(0))
                index = 9;
            if(index < 0) index = m_programStorage->GetProgramCount()-1;
            if(index > static_cast<int>(m_programStorage->GetProgramCount())-1) index = 0;

            if(GetSelScript() != index)
            {
                SetSelScript(index);

                queue->AddEvent(Event(EVENT_OBJECT_PROGLIST));
                return false;
            }
        }
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

    if ( action == EVENT_TAKE_OFF && m_studio != nullptr )
    {
        StopEditScript(true);
        m_main->SaveOneScript(m_object);
    }

    if ( m_object->GetSelect() &&  // robot selected?
         m_studio != nullptr    )   // current issue?
    {
        m_studio->EventProcess(event);

        if ( action == EVENT_OBJECT_PROGRUN )
        {
            if ( !m_programmable->IsProgram() )
            {
                if (m_selScript < m_programStorage->GetProgramCount())
                {
                    m_programmable->RunProgram(m_programStorage->GetProgram(m_selScript));
                }
            }
            else
            {
                m_programmable->StopProgram();
            }
        }
        if ( action == EVENT_OBJECT_PROGSTART )
        {
            m_main->SaveOneScript(m_object);
            if (m_selScript < m_programStorage->GetProgramCount())
            {
                m_programmable->RunProgram(m_programStorage->GetProgram(m_selScript));
            }
        }
        if ( action == EVENT_OBJECT_PROGSTOP )
        {
            m_programmable->StopProgram();
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
        if( action == EVENT_STUDIO_CLONE )
        {
            StopEditScript(true);
            Program* newProgram = m_programStorage->CloneProgram(m_programStorage->GetProgram(m_selScript));
            m_selScript = m_programStorage->GetProgramIndex(newProgram);
            m_main->SaveOneScript(m_object);

            UpdateInterface();
            CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                UpdateScript(pw);
            }
            SetSelScript(m_selScript);

            StartEditScript(newProgram, "");

            std::string res;
            GetResource(RES_TEXT, RT_STUDIO_CLONED, res);
            m_studio->SetInfoText(res, false);
        }

        return true;
    }

    if ( !m_object->GetSelect() ) return true;  // robot not selected?

    if ( m_taskExecutor->IsBackgroundTask() )  // current task?
    {
        if ( action == EVENT_OBJECT_ENDSHIELD )
        {
            m_taskExecutor->StartTaskShield(TSM_DOWN, 0.0f);
        }
    }
    if ( m_taskExecutor->IsForegroundTask() ||  // current task?
         m_programmable->IsProgram() )
    {
        if ( action == EVENT_OBJECT_PROGRUN )
        {
            m_programmable->StopProgram();
        }
        if ( action == EVENT_OBJECT_PROGEDIT )
        {
            if(m_selScript < m_programStorage->GetProgramCount())
            {
                StartEditScript(m_programStorage->GetProgram(m_selScript), m_main->GetScriptName());
            }
        }

        if ( !m_taskExecutor->IsForegroundTask() || !m_taskExecutor->GetForegroundTask()->IsPilot() )  return true;
    }

    if ( !m_programmable->IsProgram() )
    {
        if( action == EVENT_OBJECT_PROGADD )
        {
            Program* program = m_programStorage->AddProgram();
            m_selScript = m_programStorage->GetProgramIndex(program);
            m_main->SaveOneScript(m_object);

            UpdateInterface();
            CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                UpdateScript(pw);
            }
            SetSelScript(m_selScript);
        }

        if( action == EVENT_OBJECT_PROGREMOVE )
        {
            if(m_selScript < m_programStorage->GetProgramCount())
            {
                if(m_programmable->GetCurrentProgram() == m_programStorage->GetProgram(m_selScript))
                {
                    m_programmable->StopProgram();
                }
                m_programStorage->RemoveProgram(m_programStorage->GetProgram(m_selScript));
                if(m_selScript >= m_programStorage->GetProgramCount())
                    m_selScript = m_programStorage->GetProgramCount()-1;
                m_main->SaveOneScript(m_object);

                UpdateInterface();
                CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
                if ( pw != nullptr )
                {
                    UpdateScript(pw);
                }
                SetSelScript(m_selScript);
            }
        }

        if( action == EVENT_OBJECT_PROGCLONE )
        {
            if(m_selScript < m_programStorage->GetProgramCount())
            {
                m_programStorage->CloneProgram(m_programStorage->GetProgram(m_selScript));
                m_selScript = m_programStorage->GetProgramCount()-1;
                m_main->SaveOneScript(m_object);

                UpdateInterface();
                CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
                if ( pw != nullptr )
                {
                    UpdateScript(pw);
                }
                SetSelScript(m_selScript);
            }
        }


        if( action == EVENT_OBJECT_PROGMOVEUP )
        {
            std::iter_swap(m_programStorage->GetPrograms().begin() + m_selScript, m_programStorage->GetPrograms().begin() + m_selScript - 1);
            m_selScript--;
            m_main->SaveOneScript(m_object);

            UpdateInterface();
            CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                UpdateScript(pw);
            }
            SetSelScript(m_selScript);
        }

        if( action == EVENT_OBJECT_PROGMOVEDOWN )
        {
            std::iter_swap(m_programStorage->GetPrograms().begin() + m_selScript, m_programStorage->GetPrograms().begin() + m_selScript + 1);
            m_selScript++;
            m_main->SaveOneScript(m_object);

            UpdateInterface();
            CWindow* pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                UpdateScript(pw);
            }
            SetSelScript(m_selScript);
        }
    }

    if ( action == EVENT_OBJECT_PROGLIST )
    {
        m_selScript = GetSelScript();
        UpdateInterface();
    }

    if ( action == EVENT_OBJECT_PROGEDIT )
    {
        if(m_selScript < m_programStorage->GetProgramCount())
        {
            StartEditScript(m_programStorage->GetProgram(m_selScript), m_main->GetScriptName());
        }
    }

    if ( action == EVENT_OBJECT_PROGRUN )
    {
        m_programmable->StopProgram();  // stops the current program
        if(m_selScript < m_programStorage->GetProgramCount())
        {
            m_programmable->RunProgram(m_programStorage->GetProgram(m_selScript));
        }
        UpdateInterface();
    }

    err = ERR_OK;

    if ( !m_programmable->IsProgram() && m_main->CanPlayerInteract() )
    {
        if ( action == EVENT_OBJECT_HTAKE )
        {
            err = m_taskExecutor->StartTaskTake();
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
                err = m_taskExecutor->StartTaskManip(TMO_AUTO, TMA_FFRONT);
            }
            if ( m_manipStyle == EVENT_OBJECT_MBACK )
            {
                err = m_taskExecutor->StartTaskManip(TMO_AUTO, TMA_FBACK);
                if ( err == ERR_OK )
                {
                    m_manipStyle = EVENT_OBJECT_MFRONT;
                    UpdateInterface();
                }
            }
            if ( m_manipStyle == EVENT_OBJECT_MPOWER )
            {
                err = m_taskExecutor->StartTaskManip(TMO_AUTO, TMA_POWER);
                if ( err == ERR_OK )
                {
                    m_manipStyle = EVENT_OBJECT_MFRONT;
                    UpdateInterface();
                }
            }
        }

        if ( action == EVENT_OBJECT_BDERRICK )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_DERRICK);
        }
        if ( action == EVENT_OBJECT_BSTATION )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_STATION);
        }
        if ( action == EVENT_OBJECT_BFACTORY )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_FACTORY);
        }
        if ( action == EVENT_OBJECT_BREPAIR )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_REPAIR);
        }
        if ( action == EVENT_OBJECT_BCONVERT )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_CONVERT);
        }
        if ( action == EVENT_OBJECT_BTOWER )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_TOWER);
        }
        if ( action == EVENT_OBJECT_BRESEARCH )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_RESEARCH);
        }
        if ( action == EVENT_OBJECT_BRADAR )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_RADAR);
        }
        if ( action == EVENT_OBJECT_BENERGY )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_ENERGY);
        }
        if ( action == EVENT_OBJECT_BLABO )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_LABO);
        }
        if ( action == EVENT_OBJECT_BNUCLEAR )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_NUCLEAR);
        }
        if ( action == EVENT_OBJECT_BPARA )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_PARA);
        }
        if ( action == EVENT_OBJECT_BINFO )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_INFO);
        }
        if ( action == EVENT_OBJECT_BSAFE )
        {
            err = m_taskExecutor->StartTaskBuild(OBJECT_SAFE);
        }

        if ( action == EVENT_OBJECT_GFLAT )
        {
            GroundFlat();
        }
        if ( action == EVENT_OBJECT_FCREATE )
        {
            err = m_taskExecutor->StartTaskFlag(TFL_CREATE, m_flagColor);
        }
        if ( action == EVENT_OBJECT_FDELETE )
        {
            err = m_taskExecutor->StartTaskFlag(TFL_DELETE, m_flagColor);
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
            err = m_taskExecutor->StartTaskSearch();
        }

        if ( action == EVENT_OBJECT_DELSEARCH )
        {
            err = m_taskExecutor->StartTaskDeleteMark();
        }

        if ( action == EVENT_OBJECT_TERRAFORM )
        {
            err = m_taskExecutor->StartTaskTerraform();
        }

        if ( action == EVENT_OBJECT_RECOVER )
        {
            err = m_taskExecutor->StartTaskRecover();
        }

        if ( action == EVENT_OBJECT_BEGSHIELD )
        {
            err = m_taskExecutor->StartTaskShield(TSM_UP);
        }

        if ( action == EVENT_OBJECT_DIMSHIELD )
        {
            pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                ps = static_cast< CSlider* >(pw->SearchControl(EVENT_OBJECT_DIMSHIELD));
                if ( ps != nullptr )
                {
                    dynamic_cast<CShielder&>(*m_object).SetShieldRadius((ps->GetVisibleValue()-(RADIUS_SHIELD_MIN/g_unit))/((RADIUS_SHIELD_MAX-RADIUS_SHIELD_MIN)/g_unit));
                }
            }
        }

        if ( action == EVENT_OBJECT_FIRE && !m_taskExecutor->IsForegroundTask() && !m_object->GetTrainer() )
        {
            if ( m_camera->GetType() != Gfx::CAM_TYPE_ONBOARD )
            {
                m_camera->SetType(Gfx::CAM_TYPE_ONBOARD);
            }
            err = m_taskExecutor->StartTaskFire(0.0f);
        }
        if ( action == EVENT_OBJECT_TARGET && !m_object->GetTrainer() )
        {
            err = m_taskExecutor->StartTaskGunGoal((event.mousePos.y-0.50f)*1.3f, (event.mousePos.x-0.50f)*2.0f);
        }

        if ( action == EVENT_OBJECT_FIREANT )
        {
//?         err = m_taskExecutor->StartTaskFireAnt();
        }

        if ( action == EVENT_OBJECT_SPIDEREXPLO && !m_taskExecutor->IsForegroundTask() )
        {
            err = m_taskExecutor->StartTaskSpiderExplo();
        }

        if ( action == EVENT_OBJECT_BUILD )
        {
            m_buildInterface = !m_buildInterface;
            UpdateInterface();
        }

        if ( action == EVENT_OBJECT_PEN0 )  // up
        {
            err = m_taskExecutor->StartTaskPen(false);
        }
        if ( action == EVENT_OBJECT_PEN1 )  // black
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Black);
        }
        if ( action == EVENT_OBJECT_PEN2 )  // yellow
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Yellow);
        }
        if ( action == EVENT_OBJECT_PEN3 )  // orange
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Orange);
        }
        if ( action == EVENT_OBJECT_PEN4 )  // red
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Red);
        }
        if ( action == EVENT_OBJECT_PEN5 )  // violet
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Purple);
        }
        if ( action == EVENT_OBJECT_PEN6 )  // blue
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Blue);
        }
        if ( action == EVENT_OBJECT_PEN7 )  // green
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Green);
        }
        if ( action == EVENT_OBJECT_PEN8 )  // brown
        {
            err = m_taskExecutor->StartTaskPen(true, TraceColor::Brown);
        }

        if ( action == EVENT_OBJECT_REC )  // registered?
        {
            if ( m_programmable->IsTraceRecord() )
            {
                m_programmable->TraceRecordStop();
            }
            else
            {
                m_programmable->TraceRecordStart();
            }
            UpdateInterface();
            pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                UpdateScript(pw);
            }
        }
        if ( action == EVENT_OBJECT_STOP )  // stops?
        {
            if ( m_programmable->IsTraceRecord() )
            {
                m_programmable->TraceRecordStop();
            }
            UpdateInterface();
            pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
            if ( pw != nullptr )
            {
                UpdateScript(pw);
            }
        }

        if ( action == EVENT_OBJECT_RESET )
        {
            m_main->ResetObject();  // reset all objects
            UpdateInterface();
        }
    }

    if ( err != ERR_OK )
    {
        m_main->DisplayError(err, m_object);
    }

    return true;
}


// The brain is changing by time.

bool CObjectInterface::EventFrame(const Event &event)
{
    m_time += event.rTime;

    if ( m_soundChannelAlarm != -1 )
    {
        m_sound->Position(m_soundChannelAlarm, m_object->GetPosition());
    }

    if ( m_studio != nullptr )  // current edition?
    {
        m_studio->EventProcess(event);
    }

    UpdateInterface(event.rTime);

    return true;
}

// Start editing a program.

void CObjectInterface::StartEditScript(Program* program, std::string name)
{
    CreateInterface(false);  // removes the control buttons

    m_studio = std::make_unique<CStudio>();
    m_studio->StartEditScript(program->script.get(), name, program);
}

// End of editing a program.

void CObjectInterface::StopEditScript(bool closeWithErrors)
{
    if ( !m_studio->StopEditScript(closeWithErrors) )  return;
    m_studio.reset();

    if ( !closeWithErrors )  m_programStorage->SetActiveVirus(false);

    CreateInterface(true);  // puts the control buttons
}


// Shows flat areas in the field.

void CObjectInterface::GroundFlat()
{
    glm::vec3    pos, speed;
    glm::vec2     dim;
    Error       err;
    float       level;

    if ( !m_physics->GetLand() )
    {
        err = ERR_FLAG_FLY;
        pos = m_object->GetPosition();
        if ( pos.y < m_water->GetLevel() )  err = ERR_FLAG_WATER;
        m_main->DisplayError(err, m_object);
        return;
    }

    pos = m_object->GetPosition();
    m_terrain->ShowFlatGround(pos);
    m_sound->Play(SOUND_GFLAT, pos);

    level = m_terrain->GetFloorLevel(pos)+2.0f;
    if ( pos.y < level )  pos.y = level;  // not below the soil
    speed = glm::vec3(0.0f, 0.0f, 0.0f);
    dim.x = 40.0f;
    dim.y = dim.x;
    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGFLAT, 1.0f);
}


// Not below the soil.

void CObjectInterface::ColorFlag(int color)
{
    m_flagColor = color;
    UpdateInterface();
}

// Creates all the interface when the object is selected.

bool CObjectInterface::CreateInterface(bool bSelect)
{
    ObjectType       type;
    CWindow*     pw;
    CButton*     pb;
    CSlider*     ps;
    CColor*      pc;
    CTarget*     pt;
    CLabel*      pl;
    glm::vec2      pos, dim, ddim;
    float       ox, oy, sx, sy;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != nullptr )
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
    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    pos.x = 0.0f;
    pos.y = 64.0f/480.0f;
    ddim.x = 540.0f/640.0f;
    if ( !m_main->GetShowMap() )  ddim.x = 640.0f/640.0f;
    ddim.y = 16.0f/480.0f;
    pw->CreateLabel(pos, ddim, 0, EVENT_LABEL0, m_object->GetTooltipText());

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
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEib ||
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
         type == OBJECT_MOBILErp ||
         type == OBJECT_MOBILEst ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_CONTROLLER)  // vehicle?
    {
        pos.x = ox+sx*6.2f;
        pos.y = oy+sy*0;
        ddim.x = dim.x*0.6f;
        ddim.y = dim.y*0.6f;
        pw->CreateButton(pos, ddim, 12, EVENT_OBJECT_DELETE);

        if (m_main->GetMissionType() != MISSION_RETRO)
        {
            ddim.x = dim.x*5.1f;
            ddim.y = dim.y*1.5f;
            pos.x = ox+sx*0.0f;
            pos.y = oy+sy*0.5f;

            pw->CreateList(pos, ddim, -1, EVENT_OBJECT_PROGLIST, -1.10f);
            UpdateScript(pw);

            ddim.y = dim.y*0.5f;
            pos.y = oy+sy*0.0f;
            ddim.x = dim.x*1.1f;
            pos.x = ox+sx*0.0f;
            pw->CreateButton(pos, ddim, 24, EVENT_OBJECT_PROGADD);
            ddim.x = dim.x*1.0f;
            pos.x = ox+sx*1.1f;
            pw->CreateButton(pos, ddim, 25, EVENT_OBJECT_PROGREMOVE);
            pos.x = ox+sx*2.1f;
            pw->CreateButton(pos, ddim, 61, EVENT_OBJECT_PROGCLONE);
            pos.x = ox+sx*3.1f;
            pw->CreateButton(pos, ddim, 49, EVENT_OBJECT_PROGMOVEUP);
            pos.x = ox+sx*4.1f;
            pw->CreateButton(pos, ddim, 50, EVENT_OBJECT_PROGMOVEDOWN);

            pos.x = ox+sx*5.2f;
            pos.y = oy+sy*1.0f;
            pw->CreateButton(pos, dim, 8, EVENT_OBJECT_PROGRUN);
            pos.y = oy+sy*0.0f;
            pw->CreateButton(pos, dim, 22, EVENT_OBJECT_PROGEDIT);
        }
    }

    if ( m_object->Implements(ObjectInterfaceType::Flying) )
    {
        pos.x = ox+sx*6.4f;
        pos.y = oy+sy*0;
        pb = pw->CreateButton(pos, dim, 29, EVENT_OBJECT_GASDOWN);
        pb->SetImmediat(true);

        pos.x = ox+sx*6.4f;
        pos.y = oy+sy*1;
        pb = pw->CreateButton(pos, dim, 28, EVENT_OBJECT_GASUP);
        pb->SetImmediat(true);

        if ( m_object->Implements(ObjectInterfaceType::JetFlying) )
        {
            if ( type != OBJECT_HUMAN || m_object->GetOption() != 2 ) // if not Me without a jetpack, display reactor temperature
            {
                pos.x = ox+sx*15.3f;
                pos.y = oy+sy*0;
                ddim.x = 14.0f/640.0f;
                ddim.y = 66.0f/480.0f;
                pw->CreateGauge(pos, ddim, 2, EVENT_OBJECT_GRANGE);
            }
        }
    }

    if ( (type == OBJECT_HUMAN ||
          type == OBJECT_TECH  ) &&
         !m_main->GetPlusExplorer() )
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

    if ( type == OBJECT_HUMAN && !m_main->GetPlusExplorer())  // builder?
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
        DeadInterface(pw, EVENT_OBJECT_BRESEARCH, m_main->CanBuild(OBJECT_RESEARCH, m_object->GetTeam()));

        pos.x = ox+sx*0.9f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+32, EVENT_OBJECT_BFACTORY);
        DeadInterface(pw, EVENT_OBJECT_BFACTORY, m_main->CanBuild(OBJECT_FACTORY, m_object->GetTeam()));

        pos.x = ox+sx*1.8f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+34, EVENT_OBJECT_BCONVERT);
        DeadInterface(pw, EVENT_OBJECT_BCONVERT, m_main->CanBuild(OBJECT_CONVERT, m_object->GetTeam()));

        pos.x = ox+sx*2.7f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+36, EVENT_OBJECT_BSTATION);
        DeadInterface(pw, EVENT_OBJECT_BSTATION, m_main->CanBuild(OBJECT_STATION, m_object->GetTeam()));

        pos.x = ox+sx*3.6f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+40, EVENT_OBJECT_BRADAR);
        DeadInterface(pw, EVENT_OBJECT_BRADAR, m_main->CanBuild(OBJECT_RADAR, m_object->GetTeam()));

        pos.x = ox+sx*4.5f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+41, EVENT_OBJECT_BREPAIR);
        DeadInterface(pw, EVENT_OBJECT_BREPAIR, m_main->CanBuild(OBJECT_REPAIR, m_object->GetTeam()));

        pos.x = ox+sx*5.4f;
        pos.y = oy+sy*1.0f;
        pw->CreateButton(pos, ddim, 128+44, EVENT_OBJECT_BINFO);
        DeadInterface(pw, EVENT_OBJECT_BINFO, m_main->CanBuild(OBJECT_INFO, m_object->GetTeam()));

        pos.x = ox+sx*0.0f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+37, EVENT_OBJECT_BTOWER);
        DeadInterface(pw, EVENT_OBJECT_BTOWER, m_main->CanBuild(OBJECT_TOWER, m_object->GetTeam()));

        pos.x = ox+sx*0.9f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+39, EVENT_OBJECT_BENERGY);
        DeadInterface(pw, EVENT_OBJECT_BENERGY, m_main->CanBuild(OBJECT_ENERGY, m_object->GetTeam()));

        pos.x = ox+sx*1.8f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+33, EVENT_OBJECT_BDERRICK);
        DeadInterface(pw, EVENT_OBJECT_BDERRICK, m_main->CanBuild(OBJECT_DERRICK, m_object->GetTeam()));

        pos.x = ox+sx*2.7f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+42, EVENT_OBJECT_BNUCLEAR);
        DeadInterface(pw, EVENT_OBJECT_BNUCLEAR, m_main->CanBuild(OBJECT_NUCLEAR, m_object->GetTeam()));

        pos.x = ox+sx*3.6f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+38, EVENT_OBJECT_BLABO);
        DeadInterface(pw, EVENT_OBJECT_BLABO, m_main->CanBuild(OBJECT_LABO, m_object->GetTeam()));

        pos.x = ox+sx*4.5f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+46, EVENT_OBJECT_BPARA);
        DeadInterface(pw, EVENT_OBJECT_BPARA, m_main->CanBuild(OBJECT_PARA, m_object->GetTeam()));

        pos.x = ox+sx*5.4f;
        pos.y = oy+sy*0.1f;
        pw->CreateButton(pos, ddim, 128+47, EVENT_OBJECT_BSAFE);
        DeadInterface(pw, EVENT_OBJECT_BSAFE, m_main->CanBuild(OBJECT_SAFE, m_object->GetTeam()));

        if ( m_main->IsBuildingEnabled(BUILD_GFLAT) )
        {
            pos.x = ox+sx*9.0f;
            pos.y = oy+sy*0.5f;
            pw->CreateButton(pos, dim, 64+47, EVENT_OBJECT_GFLAT);
        }

        if ( m_main->IsBuildingEnabled(BUILD_FLAG) )
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

        pos.x = ox+sx*9.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 11, EVENT_OBJECT_DELSEARCH);

        if ( m_main->IsBuildingEnabled(BUILD_FLAG) )
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
        ps->SetState(STATE_VALUE);
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
        pc->SetColor(Gfx::Color(0.0f, 0.4f, 1.0f, 0.0f));
        pos.x = ox+sx*9.40f;
        pos.y = oy+sy*0.75f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN7);  // green
        pc->SetColor(Gfx::Color(0.0f, 0.8f, 0.0f, 0.0f));
        pos.x = ox+sx*9.65f;
        pos.y = oy+sy*1.25f;
        pc = pw->CreateColor(pos, ddim, -1, EVENT_OBJECT_PEN8);  // brown
        pc->SetColor(Gfx::Color(0.53f, 0.27f, 0.0f, 0.0f));

        pos.x = ox+sx*6.9f;
        pos.y = oy+sy*1.2f;
        ddim.x = dim.x*2.2f;
        ddim.y = dim.y*0.4f;

        std::string recordLabel;
        GetResource(RES_TEXT, RT_INTERFACE_REC, recordLabel);
        pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, recordLabel);
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

//camera button no more disabled for humans
    if ( //type != OBJECT_HUMAN       &&
         //type != OBJECT_TECH        &&
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

    pos.x = ox+sx*13.4f;
    pos.y = oy+sy*0;
    if ( m_object->GetTrainer() )  // Training?
    {
        pw->CreateButton(pos, dim, 9, EVENT_OBJECT_RESET);
    }
    else
    {
        pw->CreateButton(pos, dim, 10, EVENT_OBJECT_DESELECT);
    }

    if ( HasPowerCellSlot(m_object) )  // vehicle?
    {
        pos.x = ox+sx*14.5f;
        pos.y = oy+sy*0;
        ddim.x = 14.0f/640.0f;
        ddim.y = 66.0f/480.0f;
        pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);
    }

    if ( m_object->Implements(ObjectInterfaceType::Shielded) )  // vehicle?
    {
        pos.x = ox+sx*14.9f;
        pos.y = oy+sy*0;
        ddim.x = 14.0f/640.0f;
        ddim.y = 66.0f/480.0f;
        pw->CreateGauge(pos, ddim, 3, EVENT_OBJECT_GSHIELD);
    }

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
        pt->ClearState(STATE_GLINT);
    }

    if (type != OBJECT_HUMAN &&
        type != OBJECT_TECH)
    {
        ddim.x = 64.0f / 640.0f;
        ddim.y = 64.0f / 480.0f;
        pos.x = 30.0f / 640.0f;
        pos.y = 430.0f / 480.0f - ddim.y;
        pw->CreateGroup(pos, ddim, 13, EVENT_OBJECT_CORNERul);

        ddim.x = 64.0f / 640.0f;
        ddim.y = 64.0f / 480.0f;
        pos.x = 610.0f / 640.0f - ddim.x;
        pos.y = 430.0f / 480.0f - ddim.y;
        pw->CreateGroup(pos, ddim, 14, EVENT_OBJECT_CORNERur);

        ddim.x = 64.0f / 640.0f;
        ddim.y = 64.0f / 480.0f;
        pos.x = 30.0f / 640.0f;
        pos.y = 110.0f / 480.0f;
        pw->CreateGroup(pos, ddim, 15, EVENT_OBJECT_CORNERdl);

        ddim.x = 64.0f / 640.0f;
        ddim.y = 64.0f / 480.0f;
        pos.x = 610.0f / 640.0f - ddim.x;
        pos.y = 110.0f / 480.0f;
        pw->CreateGroup(pos, ddim, 16, EVENT_OBJECT_CORNERdr);
    }

    if ( (type == OBJECT_MOBILEfb ||
          type == OBJECT_MOBILEtb ||
          type == OBJECT_MOBILEwb ||
          type == OBJECT_MOBILEib) &&  // builder?
         !m_object->GetTrainer() )
    {
        pos.x = ox+sx*7.7f;
        pos.y = oy+sy*0.5f;
        pb = pw->CreateButton(pos, dim, 192+4, EVENT_OBJECT_BUILD);
        pb->SetImmediat(true);
        DefaultEnter(pw, EVENT_OBJECT_BUILD);

        pos.x = 0.0f;
        pos.y = oy+sy*2.6f;
        ddim.x = 214.5f/640.0f;
        ddim.y = 66.0f/480.0f;
        pw->CreateGroup(pos, ddim, 6, EVENT_WINDOW3);

        ddim.x = dim.x*0.9f;
        ddim.y = dim.y*0.9f;
        pos.y = oy+sy*3.6f;

        pos.x = ox+sx*0.0f;
        pw->CreateButton(pos, ddim, 128+35, EVENT_OBJECT_BRESEARCH);
        DeadInterface(pw, EVENT_OBJECT_BRESEARCH, m_main->CanBuild(OBJECT_RESEARCH, m_object->GetTeam()));

        pos.x = ox+sx*0.9f;
        pw->CreateButton(pos, ddim, 128+32, EVENT_OBJECT_BFACTORY);
        DeadInterface(pw, EVENT_OBJECT_BFACTORY, m_main->CanBuild(OBJECT_FACTORY, m_object->GetTeam()));

        pos.x = ox+sx*1.8f;
        pw->CreateButton(pos, ddim, 128+34, EVENT_OBJECT_BCONVERT);
        DeadInterface(pw, EVENT_OBJECT_BCONVERT, m_main->CanBuild(OBJECT_CONVERT, m_object->GetTeam()));

        pos.x = ox+sx*2.7f;
        pw->CreateButton(pos, ddim, 128+36, EVENT_OBJECT_BSTATION);
        DeadInterface(pw, EVENT_OBJECT_BSTATION, m_main->CanBuild(OBJECT_STATION, m_object->GetTeam()));

        pos.x = ox+sx*3.6f;
        pw->CreateButton(pos, ddim, 128+40, EVENT_OBJECT_BRADAR);
        DeadInterface(pw, EVENT_OBJECT_BRADAR, m_main->CanBuild(OBJECT_RADAR, m_object->GetTeam()));

        pos.x = ox+sx*4.5f;
        pw->CreateButton(pos, ddim, 128+41, EVENT_OBJECT_BREPAIR);
        DeadInterface(pw, EVENT_OBJECT_BREPAIR, m_main->CanBuild(OBJECT_REPAIR, m_object->GetTeam()));

        pos.x = ox+sx*5.4f;
        pw->CreateButton(pos, ddim, 128+44, EVENT_OBJECT_BINFO);
        DeadInterface(pw, EVENT_OBJECT_BINFO, m_main->CanBuild(OBJECT_INFO, m_object->GetTeam()));

        pos.y = oy+sy*2.7f;

        pos.x = ox+sx*0.0f;
        pw->CreateButton(pos, ddim, 128+37, EVENT_OBJECT_BTOWER);
        DeadInterface(pw, EVENT_OBJECT_BTOWER, m_main->CanBuild(OBJECT_TOWER, m_object->GetTeam()));

        pos.x = ox+sx*0.9f;
        pw->CreateButton(pos, ddim, 128+39, EVENT_OBJECT_BENERGY);
        DeadInterface(pw, EVENT_OBJECT_BENERGY, m_main->CanBuild(OBJECT_ENERGY, m_object->GetTeam()));

        pos.x = ox+sx*1.8f;
        pw->CreateButton(pos, ddim, 128+33, EVENT_OBJECT_BDERRICK);
        DeadInterface(pw, EVENT_OBJECT_BDERRICK, m_main->CanBuild(OBJECT_DERRICK, m_object->GetTeam()));

        pos.x = ox+sx*2.7f;
        pw->CreateButton(pos, ddim, 128+42, EVENT_OBJECT_BNUCLEAR);
        DeadInterface(pw, EVENT_OBJECT_BNUCLEAR, m_main->CanBuild(OBJECT_NUCLEAR, m_object->GetTeam()));

        pos.x = ox+sx*3.6f;
        pw->CreateButton(pos, ddim, 128+38, EVENT_OBJECT_BLABO);
        DeadInterface(pw, EVENT_OBJECT_BLABO, m_main->CanBuild(OBJECT_LABO, m_object->GetTeam()));

        pos.x = ox+sx*4.5f;
        pw->CreateButton(pos, ddim, 128+46, EVENT_OBJECT_BPARA);
        DeadInterface(pw, EVENT_OBJECT_BPARA, m_main->CanBuild(OBJECT_PARA, m_object->GetTeam()));

        pos.x = ox+sx*5.4f;
        pw->CreateButton(pos, ddim, 128+47, EVENT_OBJECT_BSAFE);
        DeadInterface(pw, EVENT_OBJECT_BSAFE, m_main->CanBuild(OBJECT_SAFE, m_object->GetTeam()));

        if ( m_main->IsBuildingEnabled(BUILD_GFLAT) )
        {
            pos.x = ox+sx*9.0f;
            pos.y = oy+sy*0.5f;
            pw->CreateButton(pos, dim, 64+47, EVENT_OBJECT_GFLAT);
        }
    }
    UpdateInterface();
    m_lastUpdateTime = 0.0f;
    UpdateInterface(0.0f);

    return true;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CObjectInterface::UpdateInterface(float rTime)
{
    CWindow*    pw;
    CButton*    pb;
    CGroup*     pgr;
    CTarget*    ptg;
    glm::vec2     ppos;
    float       range;
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

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    CGauge* pg = static_cast< CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if (pg != nullptr)
    {
        float energy = 0.0f;
        float limit = 0.0f;

        if (CPowerContainerObject* powerContainer = GetObjectPowerCell(m_object))
        {
            energy = powerContainer->GetEnergyLevel();
            limit = powerContainer->GetEnergy();
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

    pg = static_cast< CGauge* >(pw->SearchControl(EVENT_OBJECT_GSHIELD));
    if ( pg != nullptr )
    {
        assert(m_object->Implements(ObjectInterfaceType::Shielded));
        icon = 3;  // orange/gray
        float shield = dynamic_cast<CShieldedObject*>(m_object)->GetShield();

        if ( shield < 0.4f && shield != 0.0f) // low but not zero?
        {
            if ( m_lastAlarmTime >= 0.8f )  // blinks?
            {
                shield = 1.0f;
                icon = 2;  // red
            }
            if ( m_lastAlarmTime >= 1.0f )
            {
                m_sound->Play(SOUND_ALARMs, 0.5f);  // beep-beep
                m_lastAlarmTime = 0.0f;
            }
        }

        pg->SetLevel(shield);
        pg->SetIcon(icon);
    }

    pg = static_cast< CGauge* >(pw->SearchControl(EVENT_OBJECT_GRANGE));
    if ( pg != nullptr )
    {
        assert(m_object->Implements(ObjectInterfaceType::JetFlying));
        icon = 2;  // blue/red
        range = dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange();

        if ( range < 0.2f && range != 0.0f && !m_physics->GetLand() ) // low but not zero/landed?
        {
            if ( Math::Mod(m_time, 0.5f) >= 0.2f )  // blinks?
            {
                range = 1.0f;
                icon = 1;  // yellow
            }
            if ( m_soundChannelAlarm == -1 )
            {
                m_soundChannelAlarm = m_sound->Play(SOUND_ALARMt, m_object->GetPosition(), 0.0f, 0.1f, true);
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

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_OBJECT_REC));
    if ( pb != nullptr )
    {
        if ( m_programmable->IsTraceRecord() && Math::Mod(m_time, 0.4f) >= 0.2f )
        {
            pb->SetState(STATE_CHECK);
        }
        else
        {
            pb->ClearState(STATE_CHECK);
        }
    }

    bOnBoard = m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD;

    pgr = static_cast< CGroup* >(pw->SearchControl(EVENT_OBJECT_CROSSHAIR));
    if ( pgr != nullptr )
    {
        if ( bOnBoard )
        {
            ppos.x = 0.50f-(64.0f/640.0f)/2.0f;
            ppos.y = 0.50f-(64.0f/480.0f)/2.0f;
            ppos.x += m_object->GetGunGoalH()/2.0f;
            ppos.y += m_object->GetGunGoalV()/1.3f;
            pgr->SetPos(ppos);
            pgr->SetState(STATE_VISIBLE, !m_main->GetFriendAim());
        }
        else
        {
            pgr->ClearState(STATE_VISIBLE);
        }
    }

    ptg = static_cast< CTarget* >(pw->SearchControl(EVENT_OBJECT_TARGET));
    if ( ptg != nullptr )
    {
        if ( bOnBoard )
        {
            ptg->SetState(STATE_VISIBLE);
        }
        else
        {
            ptg->ClearState(STATE_VISIBLE);
        }
    }

    pgr = static_cast< CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERul));
    if ( pgr != nullptr )
    {
        pgr->SetState(STATE_VISIBLE, bOnBoard);
    }

    pgr = static_cast< CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERur));
    if ( pgr != nullptr )
    {
        pgr->SetState(STATE_VISIBLE, bOnBoard);
    }

    pgr = static_cast< CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERdl));
    if ( pgr != nullptr )
    {
        pgr->SetState(STATE_VISIBLE, bOnBoard);
    }

    pgr = static_cast< CGroup* >(pw->SearchControl(EVENT_OBJECT_CORNERdr));
    if ( pgr != nullptr )
    {
        pgr->SetState(STATE_VISIBLE, bOnBoard);
    }
}

// Updates the status of all interface buttons.

void CObjectInterface::UpdateInterface()
{
    ObjectType  type;
    CWindow*    pw;
    CButton*    pb;
    CSlider*    ps;
    CColor*     pc;
    bool        bFly, bRun;

    if ( !m_object->GetSelect() )  return;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    // This is needed because currently started program is loaded by CProgrammableObject
    // TODO: Isn't there a better way to do it?
    if (m_programmable->IsProgram())
    {
        SetSelScript(m_programStorage->GetProgramIndex(m_programmable->GetCurrentProgram()));
    }

    type = m_object->GetType();

    bool bEnable = ( !m_taskExecutor->IsForegroundTask() && !m_programmable->IsProgram() ) && m_main->CanPlayerInteract();
    bool bProgEnable = !m_programmable->IsProgram() && m_main->CanPlayerInteract();
    bool scriptSelected = m_selScript >= 0 && m_selScript < m_programStorage->GetProgramCount();

    EnableInterface(pw, EVENT_OBJECT_PROGEDIT,    scriptSelected && !m_programmable->IsTraceRecord());
    EnableInterface(pw, EVENT_OBJECT_PROGLIST,    bProgEnable && !m_programmable->IsTraceRecord());
    EnableInterface(pw, EVENT_OBJECT_PROGADD,     bProgEnable);
    EnableInterface(pw, EVENT_OBJECT_PROGREMOVE,  bProgEnable && scriptSelected && !m_programStorage->GetProgram(m_selScript)->readOnly);
    EnableInterface(pw, EVENT_OBJECT_PROGCLONE,   bProgEnable && scriptSelected && m_programStorage->GetProgram(m_selScript)->runnable);
    EnableInterface(pw, EVENT_OBJECT_PROGMOVEUP,  bProgEnable && scriptSelected && m_programStorage->GetProgramCount() >= 2 && m_selScript > 0);
    EnableInterface(pw, EVENT_OBJECT_PROGMOVEDOWN,bProgEnable && scriptSelected && m_programStorage->GetProgramCount() >= 2 && m_selScript < m_programStorage->GetProgramCount()-1);
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
    EnableInterface(pw, EVENT_OBJECT_DELSEARCH,   bEnable);
    EnableInterface(pw, EVENT_OBJECT_TERRAFORM,   bEnable);
    EnableInterface(pw, EVENT_OBJECT_RECOVER,     bEnable);
    EnableInterface(pw, EVENT_OBJECT_FIRE,        bEnable);
    EnableInterface(pw, EVENT_OBJECT_BUILD,       bEnable);
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

    if ( type == OBJECT_HUMAN    ||  // builder?
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEib )
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
        EnableInterface(pw, EVENT_OBJECT_BSAFE,     bEnable);
    }

    if ( type == OBJECT_HUMAN    ||  // can create flags?
         type == OBJECT_TECH     ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis )
    {
        CheckInterface(pw, EVENT_OBJECT_FCOLORb, m_flagColor==0);
        CheckInterface(pw, EVENT_OBJECT_FCOLORr, m_flagColor==1);
        CheckInterface(pw, EVENT_OBJECT_FCOLORg, m_flagColor==2);
        CheckInterface(pw, EVENT_OBJECT_FCOLORy, m_flagColor==3);
        CheckInterface(pw, EVENT_OBJECT_FCOLORv, m_flagColor==4);
    }

    if ( type == OBJECT_MOBILErs )  // shield?
    {
        if ( (!m_taskExecutor->IsBackgroundTask() || !m_taskExecutor->GetBackgroundTask()->IsBusy()) && !m_programmable->IsProgram() )
        {
            EnableInterface(pw, EVENT_OBJECT_BEGSHIELD, !m_taskExecutor->IsBackgroundTask() && m_main->CanPlayerInteract());
            EnableInterface(pw, EVENT_OBJECT_ENDSHIELD,  m_taskExecutor->IsBackgroundTask() && m_main->CanPlayerInteract());
            DefaultEnter   (pw, EVENT_OBJECT_BEGSHIELD, !m_taskExecutor->IsBackgroundTask());
            DefaultEnter   (pw, EVENT_OBJECT_ENDSHIELD,  m_taskExecutor->IsBackgroundTask());
        }
        else
        {
            EnableInterface(pw, EVENT_OBJECT_BEGSHIELD, false);
            EnableInterface(pw, EVENT_OBJECT_ENDSHIELD, false);
            DefaultEnter   (pw, EVENT_OBJECT_BEGSHIELD, false);
            DefaultEnter   (pw, EVENT_OBJECT_ENDSHIELD, false);
        }

        ps = static_cast< CSlider* >(pw->SearchControl(EVENT_OBJECT_DIMSHIELD));
        if ( ps != nullptr )
        {
            ps->SetVisibleValue((RADIUS_SHIELD_MIN/g_unit)+dynamic_cast<CShielder&>(*m_object).GetShieldRadius()*((RADIUS_SHIELD_MAX-RADIUS_SHIELD_MIN)/g_unit));
        }
    }

    if ( (type == OBJECT_MOBILEfb ||
          type == OBJECT_MOBILEtb ||
          type == OBJECT_MOBILEwb ||
          type == OBJECT_MOBILEib) &&  // builder?
         !m_object->GetTrainer() )
    {
        if(!bEnable) m_buildInterface = false;
        CheckInterface(pw, EVENT_OBJECT_BUILD, m_buildInterface);

        pb = static_cast< CButton* >(pw->SearchControl(EVENT_WINDOW3));
        pb->SetState(STATE_VISIBLE, m_buildInterface);

        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BFACTORY));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BDERRICK));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BCONVERT));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BSTATION));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BREPAIR));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BTOWER));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BRESEARCH));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BRADAR));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BENERGY));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BLABO));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BNUCLEAR));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BPARA));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BSAFE));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_BINFO));
        pb->SetState(STATE_VISIBLE, m_buildInterface);
    }

    bFly = bEnable;
    if ( bFly && (type == OBJECT_HUMAN || type == OBJECT_TECH) )
    {
        if (dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObjectOpt(CSlottedObject::Pseudoslot::CARRYING) != nullptr)
            bFly = false;
    }
    EnableInterface(pw, EVENT_OBJECT_GASUP,   bFly && m_main->CanPlayerInteract());
    EnableInterface(pw, EVENT_OBJECT_GASDOWN, bFly && m_main->CanPlayerInteract());

    if ( m_object->GetTrainer() )  // Training?
    {
        DeadInterface(pw, EVENT_OBJECT_GASUP,   false);
        DeadInterface(pw, EVENT_OBJECT_GASDOWN, false);
    }
    else if ( type == OBJECT_HUMAN )
    {
        DeadInterface(pw, EVENT_OBJECT_GASUP,   m_main->IsResearchDone(RESEARCH_FLY, m_object->GetTeam()));
        DeadInterface(pw, EVENT_OBJECT_GASDOWN, m_main->IsResearchDone(RESEARCH_FLY, m_object->GetTeam()));
    }
    else
    {
        DeadInterface(pw, EVENT_OBJECT_GASUP,   true);
        DeadInterface(pw, EVENT_OBJECT_GASDOWN, true);
    }

    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEib ||
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
         type == OBJECT_MOBILErp ||
         type == OBJECT_MOBILEst ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_CONTROLLER)  // vehicle?
    {
        bRun = false;
        if (m_selScript >= 0 && m_selScript < m_programStorage->GetProgramCount())
        {
            if (m_programStorage->GetProgram(m_selScript)->runnable)
            {
                std::string title = m_programStorage->GetProgram(m_selScript)->script->GetTitle();
                if ( !title.empty() )
                {
                    bRun = true;
                }
            }
            else
            {
                bRun = false;
            }
        }
        if ( !bEnable && !m_programmable->IsProgram() )  bRun = false;
        if ( m_programmable->IsTraceRecord() )  bRun = false;
        EnableInterface(pw, EVENT_OBJECT_PROGRUN, bRun && m_main->CanPlayerInteract());

        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_PROGRUN));
        if ( pb != nullptr )
        {
            pb->SetIcon(!m_programmable->IsProgram() ? 21 : 8);  // run/stop
        }

//?     pb = (CButton*)pw->SearchControl(EVENT_OBJECT_PROGEDIT);
//?     if ( pb != 0 )
//?     {
//?         pb->SetIcon(!m_programmable->IsProgram() ? 22 : 40);  // edit/debug
//?     }

        BlinkScript(m_programmable->IsProgram());  // blinks if script execution
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

    CTraceDrawingObject* traceDrawing = nullptr;
    if (m_object->Implements(ObjectInterfaceType::TraceDrawing))
        traceDrawing = dynamic_cast<CTraceDrawingObject*>(m_object);
    if (traceDrawing != nullptr && traceDrawing->GetTraceDown())
    {
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_PEN0));
        if ( pb != nullptr )
        {
            pb->ClearState(STATE_CHECK);
        }

        TraceColor color = traceDrawing->GetTraceColor();
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN1));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Black);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN2));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Yellow);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN3));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Orange);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN4));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Red);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN5));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Purple);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN6));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Blue);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN7));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Green);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN8));
        if ( pc != nullptr )
        {
            pc->SetState(STATE_CHECK, color == TraceColor::Brown);
        }
    }
    else
    {
        pb = static_cast< CButton* >(pw->SearchControl(EVENT_OBJECT_PEN0));
        if ( pb != nullptr )
        {
            pb->SetState(STATE_CHECK);
        }

        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN1));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN2));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN3));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN4));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN5));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN6));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN7));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
        pc = static_cast< CColor* >(pw->SearchControl(EVENT_OBJECT_PEN8));
        if ( pc != nullptr )
        {
            pc->ClearState(STATE_CHECK);
        }
    }
}

// Updates the list of programs.

void CObjectInterface::UpdateScript(CWindow *pw)
{
    CList*      pl;
    std::array<char, 100> name;

    pl = static_cast< CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == nullptr )  return;

    pl->Flush();
    for ( int i = 0 ; i < m_programStorage->GetProgramCount() ; i++ )
    {
        snprintf(name.data(), name.size(), "%d", i+1);

        std::string title = m_programStorage->GetProgram(i)->script->GetTitle();
        if ( !title.empty() )
        {
            if(!m_programStorage->GetProgram(i)->readOnly)
            {
                snprintf(name.data(), name.size(), "%d: %s", i+1, title.c_str());
            }
            else
            {
                snprintf(name.data(), name.size(), "*%d: %s", i+1, title.c_str());
            }
        }

        pl->SetItemName(i, name.data());
    }

    pl->SetSelect(m_selScript);
    pl->ShowSelect(true);
}

// Returns the rank of selected script.

int CObjectInterface::GetSelScript()
{
    CWindow*    pw;
    CList*      pl;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return -1;

    pl = static_cast< CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == nullptr )  return -1;

    return pl->GetSelect();
}

// Changes selected script

void CObjectInterface::SetSelScript(int index)
{
    CWindow*    pw;
    CList*      pl;

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    pl = static_cast< CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == nullptr )  return;

    pl->SetSelect(index);
    pl->ShowSelect(true);
}

// Blinks the running program.

void CObjectInterface::BlinkScript(bool bEnable)
{
    CWindow*    pw;
    CList*      pl;

    if ( !m_object->GetSelect() )  return;  // robot not selected?

    pw = static_cast< CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    pl = static_cast< CList* >(pw->SearchControl(EVENT_OBJECT_PROGLIST));
    if ( pl == nullptr )  return;

    pl->SetBlink(bEnable);
}

// Check the status of a button interface.

void CObjectInterface::CheckInterface(CWindow *pw, EventType event, bool bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(STATE_CHECK, bState);
}

// Changes the state of a button interface.

void CObjectInterface::EnableInterface(CWindow *pw, EventType event, bool bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(STATE_ENABLE, bState);
}

// Changes the state of a button on the interface.

void CObjectInterface::DeadInterface(CWindow *pw, EventType event, bool bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(STATE_DEAD, !bState);
}

// Change the default input state of a button interface.

void CObjectInterface::DefaultEnter(CWindow *pw, EventType event, bool bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    if ( bState )
    {
        control->SetState(STATE_DEFAULT);
        m_defaultEnter = event;
    }
    else
    {
        control->ClearState(STATE_DEFAULT);
    }
}

} // namespace Ui
