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


#include "ui/displaytext.h"

#include "app/app.h"

#include "common/event.h"
#include "common/restext.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/movable_object.h"

#include "object/motion/motion.h"
#include "object/motion/motiontoto.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"



namespace Ui
{

namespace
{
const float FONTSIZE = 12.0f;
} // anonymous namespace



// Object's constructor.

CDisplayText::CDisplayText()
{
    m_engine    = Gfx::CEngine::GetInstancePointer();
    m_interface = CRobotMain::GetInstancePointer()->GetInterface();
    m_sound     = CApplication::GetInstancePointer()->GetSound();

    m_bHide = false;
    m_bEnable = true;
    m_delayFactor = 1.0f;
}

// Object's destructor.

CDisplayText::~CDisplayText()
{
}


// Destroys the object.

void CDisplayText::DeleteObject()
{
    m_interface->DeleteControl(EVENT_WINDOW2);
}


// Management of an event.

bool CDisplayText::EventProcess(const Event &event)
{
    if (m_engine->GetPause())  return true;

    if (event.type == EVENT_FRAME)
    {
        for (auto& line : m_textLines)
        {
            if (! line.exist) break;
            line.time -= event.rTime;
        }

        while (true)
        {
            if (!m_textLines.front().exist ||
                m_textLines.front().time > 0.0f)
            {
                break;
            }

            if (!ClearLastText())
                break;
        }
    }

    return true;
}


// Displays an error.

void CDisplayText::DisplayError(Error err, CObject* pObj, float time)
{
    if (pObj == nullptr)
        return;

    glm::vec3 pos = pObj->GetPosition();
    float h = GetIdealHeight(pObj);
    float d = GetIdealDist(pObj);
    DisplayError(err, pos, h, d, time);
}

// Displays an error.

void CDisplayText::DisplayError(Error err, glm::vec3 goal, float height,
                                float dist, float time)
{
    if ( err == ERR_OK )  return;

    TextType type = TT_WARNING;
    if ( err >= INFO_FIRST )
    {
        type = TT_INFO;
    }
    if ( err == ERR_BAT_VIRUS      ||
         err == ERR_VEH_VIRUS      ||
         err == ERR_DELETEMOBILE   ||
         err == ERR_DELETEBUILDING ||
         err == INFO_LOST          )
    {
        type = TT_ERROR;
    }

    std::string text;
    GetResource(RES_ERR, err, text);
    DisplayText(text.c_str(), goal, height, dist, time, type);
}

// Displays text.

void CDisplayText::DisplayText(const char *text, CObject* pObj,
                               float time, TextType type)
{
    if (pObj == nullptr)  return;

    glm::vec3 pos = pObj->GetPosition();
    float h = GetIdealHeight(pObj);
    float d = GetIdealDist(pObj);
    DisplayText(text, pos, h, d, time, type);
}

// Displays text.

void CDisplayText::DisplayText(const char *text, glm::vec3 goal, float height,
                               float dist, float time, TextType type)
{
    CObject*    toto;
    CMotion*    motion;
    Ui::CWindow*    pw;
    Ui::CButton*    button;
    Ui::CGroup*     group;
    Ui::CLabel*     label;
    glm::vec2     pos, ppos, dim;
    SoundType   sound;
    float       hLine, hBox;
    int         nLine, icon, i;

    if ( !m_bEnable )  return;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw == nullptr )
    {
        pos.x = 0.0f;
        pos.y = 0.0f;
        dim.x = 0.0f;
        dim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, dim, 10, EVENT_WINDOW2);
    }

    hBox = 0.045f;
    hLine = m_engine->GetText()->GetHeight(Gfx::FONT_COMMON, FONTSIZE);

    nLine = 0;
    for ( i=0 ; i<MAXDTLINE ; i++ )
    {
        group = static_cast<CGroup*>(pw->SearchControl(EventType(EVENT_DT_GROUP0+i)));
        if ( group == nullptr )  break;
        nLine ++;
    }

    if ( nLine == MAXDTLINE )
    {
        ClearLastText();
        nLine --;
    }

    pos.x = 0.10f;
    pos.y = 0.92f-hBox-hBox*nLine;
    dim.x = 0.80f;
    dim.y = hBox;

    icon = 1;  // yellow
    if ( type == TT_ERROR   )  icon =  9;  // red
    if ( type == TT_WARNING )  icon = 10;  // blue
    if ( type == TT_INFO    )  icon =  8;  // green
    if ( type == TT_MESSAGE )  icon = 11;  // yellow
    pw->CreateGroup(pos, dim, icon, EventType(EVENT_DT_GROUP0+nLine));

    pw->SetTrashEvent(false);

    ppos = pos;
    ppos.y -= hLine/2.0f;
    label = pw->CreateLabelRaw(ppos, dim, -1, EventType(EVENT_DT_LABEL0+nLine), text);
    if ( label != nullptr )
    {
        label->SetFontSize(FONTSIZE);
    }

    dim.x = dim.y*0.75f;
    pos.x -= dim.x;
    button = pw->CreateButton(pos, dim, 14, EventType(EVENT_DT_VISIT0+nLine));

    if ( goal.x == 0.0f &&
         goal.y == 0.0f &&
         goal.z == 0.0f )
    {
        button->ClearState(STATE_ENABLE);
    }

    TextLine line;
    line.exist = true;
    line.visitGoal = goal;
    line.visitDist = dist;
    line.visitHeight = height;
    line.time = time*m_delayFactor;
    m_textLines[nLine] = line;

    toto = SearchToto();
    if ( toto != nullptr )
    {
        assert(toto->Implements(ObjectInterfaceType::Movable));
        motion = dynamic_cast<CMovableObject&>(*toto).GetMotion();

        if ( type == TT_ERROR )
        {
            motion->SetAction(MT_ERROR, 4.0f);
        }
        if ( type == TT_WARNING )
        {
            motion->SetAction(MT_WARNING, 4.0f);
        }
        if ( type == TT_INFO )
        {
            motion->SetAction(MT_INFO, 4.0f);
        }
        if ( type == TT_MESSAGE )
        {
            motion->SetAction(MT_MESSAGE, 4.0f);
        }
    }

    if ( m_bHide )
    {
        HideText(m_bHide);  // hide all
    }
    else
    {
        sound = SOUND_NONE;
        if ( type == TT_ERROR   )  sound = SOUND_ERROR;
        if ( type == TT_WARNING )  sound = SOUND_WARNING;
        if ( type == TT_INFO    )  sound = SOUND_INFO;
        if ( type == TT_MESSAGE )  sound = SOUND_MESSAGE;

        if ( sound != SOUND_NONE )
        {
            m_sound->Play(sound);
        }
    }
}

// Clears all text.

void CDisplayText::ClearText()
{
    Ui::CWindow* pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));

    for (int i = 0; i < MAXDTLINE; i++)
    {
        if (pw != nullptr)
        {
            pw->DeleteControl(EventType(EVENT_DT_GROUP0+i));
            pw->DeleteControl(EventType(EVENT_DT_LABEL0+i));
            pw->DeleteControl(EventType(EVENT_DT_VISIT0+i));
        }

        m_textLines[i] = TextLine();
    }
}

// Hides or shows all texts.

void CDisplayText::HideText(bool bHide)
{
    Ui::CWindow*    pw;
    Ui::CGroup*     pg;
    Ui::CLabel*     pl;
    Ui::CButton*    pb;
    int         i;

    m_bHide = bHide;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw == nullptr )  return;

    for ( i=0 ; i<MAXDTLINE ; i++ )
    {
        pg = static_cast<Ui::CGroup*>(pw->SearchControl(EventType(EVENT_DT_GROUP0+i)));
        if ( pg != nullptr )
        {
            pg->SetState(STATE_VISIBLE, !bHide);
        }

        pl = static_cast<Ui::CLabel*>(pw->SearchControl(EventType(EVENT_DT_LABEL0+i)));
        if ( pl != nullptr )
        {
            pl->SetState(STATE_VISIBLE, !bHide);
        }

        pb = static_cast<CButton*>(pw->SearchControl(EventType(EVENT_DT_VISIT0+i)));
        if ( pb != nullptr )
        {
            pb->SetState(STATE_VISIBLE, !bHide);
        }
    }
}

// Removes the last text (top of the list).

bool CDisplayText::ClearLastText()
{
    Ui::CWindow     *pw;
    Ui::CButton     *pb1, *pb2;
    Ui::CGroup      *pg1, *pg2;
    Ui::CLabel      *pl1, *pl2;
    int         i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw == nullptr )  return false;

    pb2 = static_cast<CButton*>(pw->SearchControl(EVENT_DT_VISIT0));
    if ( pb2 == nullptr )  return false;  // same not of first-line
    pg2 = static_cast<CGroup*>(pw->SearchControl(EVENT_DT_GROUP0));
    if ( pg2 == nullptr )  return false;
    pl2 = static_cast<CLabel*>(pw->SearchControl(EVENT_DT_LABEL0));
    if ( pl2 == nullptr )  return false;

    for ( i=0 ; i<MAXDTLINE-1 ; i++ )
    {
        pb1 = pb2;
        pg1 = pg2;
        pl1 = pl2;

        pb2 = static_cast<CButton*>(pw->SearchControl(EventType(EVENT_DT_VISIT0+i+1)));
        if ( pb2 == nullptr )  break;

        pg2 = static_cast<CGroup*>(pw->SearchControl(EventType(EVENT_DT_GROUP0+i+1)));
        if ( pg2 == nullptr )  break;

        pl2 = static_cast<CLabel*>(pw->SearchControl(EventType(EVENT_DT_LABEL0+i+1)));
        if ( pl2 == nullptr )  break;

        pb1->SetState(STATE_ENABLE, pb2->TestState(STATE_ENABLE));
        pg1->SetIcon(pg2->GetIcon());
        pl1->SetName(pl2->GetName(), false);

        m_textLines[i] = m_textLines[i+1];
    }

    pw->DeleteControl(EventType(EVENT_DT_VISIT0+i));
    pw->DeleteControl(EventType(EVENT_DT_GROUP0+i));
    pw->DeleteControl(EventType(EVENT_DT_LABEL0+i));
    m_textLines[i].exist = false;
    return true;
}


// Specifies the factor of time.

void CDisplayText::SetDelay(float factor)
{
    m_delayFactor = factor;
}


// Enables the display of text.

void CDisplayText::SetEnable(bool bEnable)
{
    m_bEnable = bEnable;
}


// Returns the goal during a visit.

glm::vec3 CDisplayText::GetVisitGoal(EventType event)
{
    int i = event - EVENT_DT_VISIT0;
    if (i < 0 || i >= MAXDTLINE)  return glm::vec3(0.0f, 0.0f, 0.0f);
    return m_textLines[i].visitGoal;
}

// Returns the distance during a visit.

float CDisplayText::GetVisitDist(EventType event)
{
    int i = event-EVENT_DT_VISIT0;
    if (i < 0 || i >= MAXDTLINE)  return 0.0f;
    return m_textLines[i].visitDist;
}

// Returns the height on a visit.

float CDisplayText::GetVisitHeight(EventType event)
{
    int i = event-EVENT_DT_VISIT0;
    if (i < 0 || i >= MAXDTLINE)  return 0.0f;
    return m_textLines[i].visitHeight;
}


// Ranges from ideal visit for a given object.

float CDisplayText::GetIdealDist(CObject* pObj)
{
    ObjectType  type;

    if ( pObj == nullptr )  return 40.0f;

    type = pObj->GetType();
    if ( type == OBJECT_PORTICO )  return 200.0f;
    if ( type == OBJECT_BASE    )  return 200.0f;
    if ( type == OBJECT_NUCLEAR )  return 100.0f;
    if ( type == OBJECT_PARA    )  return 100.0f;
    if ( type == OBJECT_SAFE    )  return 100.0f;
    if ( type == OBJECT_TOWER   )  return  80.0f;

    return 60.0f;
}

// Returns the height of ideal visit for a given object.

float CDisplayText::GetIdealHeight(CObject* pObj)
{
    ObjectType  type;

    if ( pObj == nullptr )  return 5.0f;

    type = pObj->GetType();
    if ( type == OBJECT_DERRICK  )  return 35.0f;
    if ( type == OBJECT_FACTORY  )  return 22.0f;
    if ( type == OBJECT_REPAIR   )  return 30.0f;
    if ( type == OBJECT_DESTROYER)  return 30.0f;
    if ( type == OBJECT_STATION  )  return 13.0f;
    if ( type == OBJECT_CONVERT  )  return 20.0f;
    if ( type == OBJECT_TOWER    )  return 30.0f;
    if ( type == OBJECT_RESEARCH )  return 22.0f;
    if ( type == OBJECT_RADAR    )  return 19.0f;
    if ( type == OBJECT_INFO     )  return 19.0f;
    if ( type == OBJECT_ENERGY   )  return 20.0f;
    if ( type == OBJECT_LABO     )  return 16.0f;
    if ( type == OBJECT_NUCLEAR  )  return 40.0f;
    if ( type == OBJECT_PARA     )  return 40.0f;
    if ( type == OBJECT_SAFE     )  return 20.0f;

    return 15.0f;
}


// Removes all visits.

void CDisplayText::ClearVisit()
{
    Ui::CWindow*    pw;
    Ui::CButton*    pb;
    int         i;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw == nullptr )  return;

    for ( i=0 ; i<MAXDTLINE ; i++ )
    {
        pb = static_cast<CButton*>(pw->SearchControl(EventType(EVENT_DT_VISIT0+i)));
        if ( pb == nullptr )  break;
        pb->SetIcon(14);  // eyes
    }
}

// Puts a button in "visit".

void CDisplayText::SetVisit(EventType event)
{
    Ui::CWindow*    pw;
    Ui::CButton*    pb;
    int         i;

    i = event-EVENT_DT_VISIT0;
    if ( i < 0 || i >= MAXDTLINE )  return;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw == nullptr )  return;
    pb = static_cast<CButton*>(pw->SearchControl(EventType(EVENT_DT_VISIT0+i)));
    if ( pb == nullptr )  return;
    pb->SetIcon(48);  // >
}

// Indicates whether a button is set to "visit".

bool CDisplayText::IsVisit(EventType event)
{
    Ui::CWindow*    pw;
    Ui::CButton*    pb;
    int         i;

    i = event-EVENT_DT_VISIT0;
    if ( i < 0 || i >= MAXDTLINE )  return false;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw == nullptr )  return false;
    pb = static_cast<CButton*>(pw->SearchControl(EventType(EVENT_DT_VISIT0+i)));
    if ( pb == nullptr )  return false;
    return (pb->GetIcon() == 48);  // > ?
}


// Returns the object toto.

CObject* CDisplayText::SearchToto()
{
    return CObjectManager::GetInstancePointer()->FindNearest(nullptr, OBJECT_TOTO);
}

} // namespace Ui
