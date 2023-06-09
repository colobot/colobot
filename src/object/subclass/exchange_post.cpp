/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/subclass/exchange_post.h"

#include "core/stringutils.h"

#include "common/make_unique.h"
#include "common/regex_utils.h"

#include "graphics/engine/oldmodelmanager.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "object/object_create_params.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"


CExchangePost::CExchangePost(int id)
    : CBaseBuilding(id, OBJECT_INFO)
    , m_infoUpdate(false)
{}

std::unique_ptr<CExchangePost> CExchangePost::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CExchangePost>(params.id);

    obj->SetTeam(params.team);

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);

    modelManager->AddModelReference("info1.mod", false, rank);
    obj->SetPosition(params.pos);
    obj->SetRotationY(params.angle);
    obj->SetFloorHeight(0.0f);

    rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    obj->SetObjectRank(1, rank);
    obj->SetObjectParent(1, 0);
    modelManager->AddModelReference("info2.mod", false, rank);
    obj->SetPartPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));

    for (int i = 0; i < 3; ++i)
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2+i*2, rank);
        obj->SetObjectParent(2+i*2, 1);
        modelManager->AddModelReference("info3.mod", false, rank);
        obj->SetPartPosition(2+i*2, Math::Vector(0.0f, 4.5f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3+i*2, rank);
        obj->SetObjectParent(3+i*2, 2+i*2);
        modelManager->AddModelReference("radar4.mod", false, rank);
        obj->SetPartPosition(3+i*2, Math::Vector(0.0f, 0.0f, -4.0f));

        obj->SetPartRotationY(2+i*2, 2.0f*Math::PI/3.0f*i);
    }

    obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
    obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
    obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 5.0f, 0.0f), 6.0f));

    obj->CreateShadowCircle(8.0f, 1.0f);

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    auto objAuto = MakeUnique<CAutoInfo>(obj.get());
    objAuto->Init();
    obj->SetAuto(std::move(objAuto));

    engine->LoadAllTextures();

    return obj;
}

int CExchangePost::GetMaximumInfoListSize()
{
    return 10;
}

bool CExchangePost::SetInfo(const std::string& name, float value)
{
    for (auto& info : m_infoList)
    {
        if (info.name == name)
        {
            info.value = value;
            m_infoUpdate = true;
            return true;
        }
    }

    if (static_cast<int>(m_infoList.size()) == GetMaximumInfoListSize())
    {
        return false;
    }

    ExchangePostInfo info;
    info.name = name;
    info.value = value;
    m_infoList.push_back(info);
    m_infoUpdate = true;
    return true;
}

const std::vector<ExchangePostInfo>& CExchangePost::GetInfoList()
{
    return m_infoList;
}

std::optional<float> CExchangePost::GetInfoValue(const std::string& name)
{
    for (auto& info : m_infoList)
    {
        if (info.name == name)
        {
            return info.value;
        }
    }
    return std::nullopt;
}

bool CExchangePost::HasInfo(const std::string& name)
{
    for (auto& info : m_infoList)
    {
        if (info.name == name)
        {
            return true;
        }
    }
    return false;
}

bool CExchangePost::DeleteInfo(const std::string& name)
{
    for (auto it = m_infoList.begin(); it != m_infoList.end(); ++it)
    {
        if (it->name == name)
        {
            m_infoList.erase(it);
            m_infoUpdate = true;
            return true;
        }
    }
    return false;
}

bool CExchangePost::GetInfoUpdate()
{
    return m_infoUpdate;
}

void CExchangePost::SetInfoUpdate(bool update)
{
    m_infoUpdate = update;
}

void CExchangePost::Write(CLevelParserLine* line)
{
    COldObject::Write(line);

    int i = 0;
    for (const auto& info : m_infoList)
    {
        ++i;
        if (!info.name.empty())
        {
            auto key = "info" + StrUtils::ToString(i);
            auto paramValue = info.name + "=" + StrUtils::ToString(info.value);
            line->AddParam(key, MakeUnique<CLevelParserParam>(paramValue));
        }
    }
}

void CExchangePost::Read(CLevelParserLine* line)
{
    COldObject::Read(line);

    ReadInfo(line);
}

void CExchangePost::ReadInfo(CLevelParserLine* line)
{
    for (int i = 1; i <= GetMaximumInfoListSize(); i++)
    {
        std::string op = std::string("info") + StrUtils::ToString(i);

        if (!line->GetParam(op)->IsDefined())
            break;

        std::string text = line->GetParam(op)->AsString();

        ExchangePostInfo info;

        try
        {
            auto matches = RegexUtils::AssertRegexMatch(text, "([^=]+)=(.*)");
            info.name = matches[1];
            info.value = StrUtils::FromString<float>(matches[2]);
        }
        catch (...)
        {
            throw CLevelParserExceptionBadParam(line->GetParam(op), op);
        }


        m_infoList.push_back(info);
    }
}

//////////////////

enum class CAutoInfo::Phase : unsigned int
{
    Wait      = 1,
    Send      = 2,
    Receive   = 3,
    Error     = 4,
};

// Object's constructor.

CAutoInfo::CAutoInfo(CExchangePost* object)
    : CAuto(object),
      m_exchangePost(object),
      m_phase(Phase::Wait),
      m_progress(0.0f),
      m_speed(0.0f),
      m_timeVirus(0.0f),
      m_lastParticle(0.0f),
      m_lastVirus(false)
{
    Init();
}

CAutoInfo::~CAutoInfo()
{
}

void CAutoInfo::DeleteObject(bool all)
{
    CAuto::DeleteObject(all);
}


void CAutoInfo::Init()
{
    m_phase = Phase::Wait;
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastVirus = false;

    CAuto::Init();
}

void CAutoInfo::Start(int param)
{
    if (param == 0)  // instruction "receive" ?
    {
        m_phase = Phase::Send;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }
    else if (param == 2)  // instruction "send" ?
    {
        m_phase = Phase::Receive;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }
    else
    {
        m_phase = Phase::Error;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }

    m_lastParticle = 0;
    m_goal = m_object->GetPosition();

    Math::Vector    pos, speed;
    Math::Point     dim;

    if (m_phase == Phase::Send)
    {
        pos = m_goal;
        pos.y += 9.5f;
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = 30.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISPHERE4, 1.5f, 0.0f, 0.0f);

        m_sound->Play(SOUND_LABO, pos, 1.0f, 2.0f);
    }
    if (m_phase == Phase::Receive)
    {
        pos = m_goal;
        pos.y += 9.5f;
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = 50.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISPHERE6, 1.5f, 0.0f, 0.0f);

        m_sound->Play(SOUND_LABO, pos, 1.0f, 2.0f);
    }
    if (m_phase == Phase::Error)
    {
        m_sound->Play(SOUND_GGG, pos, 1.0f, 0.5f);
    }
}

bool CAutoInfo::EventProcess(const Event &event)
{
    CAuto::EventProcess(event);

    if (m_engine->GetPause())  return true;
    if (event.type != EVENT_FRAME)  return true;

    m_timeVirus -= event.rTime;

    if (m_exchangePost->GetVirusMode())  // contaminated by a virus?
    {
        if (m_timeVirus <= 0.0f)
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;

            float angle = m_exchangePost->GetPartRotationY(1);
            angle += Math::Rand()*0.3f;
            m_exchangePost->SetPartRotationY(1, angle);

            m_exchangePost->SetPartRotationX(2, (Math::Rand()-0.5f)*0.3f);
            m_exchangePost->SetPartRotationX(4, (Math::Rand()-0.5f)*0.3f);
            m_exchangePost->SetPartRotationX(6, (Math::Rand()-0.5f)*0.3f);

            m_exchangePost->SetPartRotationZ(2, (Math::Rand()-0.5f)*0.3f);
            m_exchangePost->SetPartRotationZ(4, (Math::Rand()-0.5f)*0.3f);
            m_exchangePost->SetPartRotationZ(6, (Math::Rand()-0.5f)*0.3f);

            UpdateListVirus();
        }
        m_lastVirus = true;
        return true;
    }
    else
    {
        if (m_lastVirus)
        {
            m_lastVirus = false;
            UpdateList();  // normally returns the list
        }
        else
        {
            if (m_exchangePost->GetInfoUpdate())
            {
                UpdateList();  // updates the list
            }
        }
    }

    UpdateInterface(event.rTime);

    float rTime = event.rTime;

    if (m_phase == Phase::Send)  // instruction "receive" ?
    {
        if (m_progress < 0.5f &&
            m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time)
        {
            m_lastParticle = m_time;

            for (int i = 0; i < 4; i++)
            {
                Math::Vector pos = m_goal;
                pos.y += 9.5f;
                Math::Vector speed;
                speed.x = (Math::Rand()-0.5f)*50.0f;
                speed.z = (Math::Rand()-0.5f)*50.0f;
                speed.y = (Math::Rand()-0.5f)*50.0f;
                speed *= 0.5f+m_progress*0.5f;
                Math::Point dim(0.6f, 0.6f);
                float duration = Math::Rand()*0.5f+0.5f;
                m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                         duration, 0.0f,
                                         duration*0.9f, 0.7f);
            }
        }

        if (m_progress < 1.0f)
        {
            m_progress += rTime*m_speed;

            m_exchangePost->SetPartRotationZ(2, m_progress*2.0f*Math::PI);
            m_exchangePost->SetPartRotationZ(4, m_progress*2.0f*Math::PI);
            m_exchangePost->SetPartRotationZ(6, m_progress*2.0f*Math::PI);
        }
        else
        {
            m_phase = Phase::Wait;

            m_exchangePost->SetPartRotationX(2, 0.0f);
            m_exchangePost->SetPartRotationX(4, 0.0f);
            m_exchangePost->SetPartRotationX(6, 0.0f);

            m_exchangePost->SetPartRotationZ(2, 0.0f);
            m_exchangePost->SetPartRotationZ(4, 0.0f);
            m_exchangePost->SetPartRotationZ(6, 0.0f);
        }
    }

    if (m_phase == Phase::Receive)  // instruction "send" ?
    {
        if (m_progress < 0.5f &&
            m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time)
        {
            m_lastParticle = m_time;

            for (int i = 0; i < 4; i++)
            {
                Math::Vector pos = m_goal;
                pos.y += 9.5f;
                Math::Vector speed = pos;
                pos.x += (Math::Rand()-0.5f)*40.0f;
                pos.y += (Math::Rand()-0.5f)*40.0f;
                pos.z += (Math::Rand()-0.5f)*40.0f;
                speed = (speed-pos)*1.0f;
//?             speed *= 0.5f+m_progress*0.5f;
                Math::Point dim(0.6f, 0.6f);
                float duration = Math::Rand()*0.5f+0.5f;
                m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                         duration, 0.0f,
                                         duration*0.9f, 0.7f);
            }
        }

        if (m_progress < 1.0f)
        {
            m_progress += rTime*m_speed;

            m_exchangePost->SetPartRotationZ(2, m_progress*2.0f*Math::PI);
            m_exchangePost->SetPartRotationZ(4, m_progress*2.0f*Math::PI);
            m_exchangePost->SetPartRotationZ(6, m_progress*2.0f*Math::PI);
        }
        else
        {
            m_phase = Phase::Wait;

            m_exchangePost->SetPartRotationX(2, 0.0f);
            m_exchangePost->SetPartRotationX(4, 0.0f);
            m_exchangePost->SetPartRotationX(6, 0.0f);

            m_exchangePost->SetPartRotationZ(2, 0.0f);
            m_exchangePost->SetPartRotationZ(4, 0.0f);
            m_exchangePost->SetPartRotationZ(6, 0.0f);
        }
    }

    if ( m_phase == Phase::Error )
    {
        if (m_progress < 0.5f &&
            m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time)
        {
            m_lastParticle = m_time;

            Math::Vector pos = m_goal;
            Math::Vector speed;
            speed.x = (Math::Rand()-0.5f)*5.0f;
            speed.z = (Math::Rand()-0.5f)*5.0f;
            speed.y = 5.0f+Math::Rand()*5.0f;
            Math::Point dim;
            dim.x = 5.0f+Math::Rand()*5.0f;
            dim.y = dim.x;
            float duration = 4.0f;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, duration);
        }

        if (m_progress < 1.0f)
        {
            m_progress += rTime*m_speed;
            rTime = 0.0f;  // stops the rotation

            float angle = 0.0f;
            if ( m_progress < 0.5f )
            {
                angle = m_progress/0.5f;
            }
            else
            {
                angle = 1.0f-(m_progress-0.5f)/0.5f;
            }
            m_exchangePost->SetPartRotationX(2, angle*0.5f);
            m_exchangePost->SetPartRotationX(4, angle*0.5f);
            m_exchangePost->SetPartRotationX(6, angle*0.5f);

            m_exchangePost->SetPartRotationZ(2, (Math::Rand()-0.5f)*0.2f);
            m_exchangePost->SetPartRotationZ(4, (Math::Rand()-0.5f)*0.2f);
            m_exchangePost->SetPartRotationZ(6, (Math::Rand()-0.5f)*0.2f);
        }
        else
        {
            m_phase = Phase::Wait;

            m_exchangePost->SetPartRotationX(2, 0.0f);
            m_exchangePost->SetPartRotationX(4, 0.0f);
            m_exchangePost->SetPartRotationX(6, 0.0f);

            m_exchangePost->SetPartRotationZ(2, 0.0f);
            m_exchangePost->SetPartRotationZ(4, 0.0f);
            m_exchangePost->SetPartRotationZ(6, 0.0f);
        }
    }

    float angle = m_exchangePost->GetPartRotationY(1);
    angle += rTime*0.5f;
    m_exchangePost->SetPartRotationY(1, angle);

    m_exchangePost->SetPartRotationX(3, sinf(m_time*6.0f+Math::PI*0.0f/3.0f)*0.3f);
    m_exchangePost->SetPartRotationX(5, sinf(m_time*6.0f+Math::PI*2.0f/3.0f)*0.3f);
    m_exchangePost->SetPartRotationX(7, sinf(m_time*6.0f+Math::PI*4.0f/3.0f)*0.3f);

    return true;
}

Error CAutoInfo::GetError()
{
    if (m_object->GetVirusMode())
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}

bool CAutoInfo::CreateInterface(bool select)
{
    CAuto::CreateInterface(select);

    if (!select)  return true;

    Ui::CWindow* pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if (pw == nullptr)  return false;

    float ox = 3.0f/640.0f;
    float oy = 3.0f/480.0f;
    float sx = 33.0f/640.0f;
    float sy = 33.0f/480.0f;

    Math::Point pos, ddim;

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.0f;
    ddim.x = 160.0f/640.0f;
    ddim.y =  66.0f/480.0f;
    Ui::CList* pl = pw->CreateList(pos, ddim, 1, EVENT_OBJECT_GINFO, 1.10f);
    pl->SetSelectCap(false);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 112, EVENT_OBJECT_TYPE);

    UpdateList();
    return true;
}

void CAutoInfo::UpdateInterface(float rTime)
{
    CAuto::UpdateInterface(rTime);
}

void CAutoInfo::UpdateList()
{
    CExchangePost* object = static_cast<CExchangePost*>(m_object);

    Ui::CWindow* pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if (pw == nullptr)  return;

    Ui::CList* pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_GINFO));
    if (pl == nullptr)  return;

    pl->Flush();
    const auto& infoList = object->GetInfoList();
    if (infoList.empty())
    {
        pl->ClearState(Ui::STATE_ENABLE);
    }
    else
    {
        pl->SetState(Ui::STATE_ENABLE);

        for (int i = 0; i < static_cast<int>(infoList.size()); i++)
        {
            char text[100];
            sprintf(text, "%s = %.2f", infoList[i].name.c_str(), infoList[i].value);
            pl->SetItemName(i, text);
        }
    }

    object->SetInfoUpdate(false);
}

void CAutoInfo::UpdateListVirus()
{
    Ui::CWindow* pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if (pw == nullptr)  return;

    Ui::CList* pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_GINFO));
    if (pl == nullptr)  return;

    pl->SetState(Ui::STATE_ENABLE);

    pl->Flush();
    for (int i = 0; i < 4; ++i)
    {
        char text[100];
        int max = static_cast< int >(2.0f+Math::Rand()*10.0f);
        for (int j = 0; j < max; ++j)
        {
            do
            {
                text[j] = ' '+static_cast<int>(Math::Rand()*94.0f);
            }
            while (text[j] == '\\');
        }
        text[max] = 0;

        pl->SetItemName(i, text);
    }
}

bool CAutoInfo::Write(CLevelParserLine* line)
{
    if (m_phase == Phase::Wait)
        return false;

    line->AddParam("aExist", MakeUnique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", MakeUnique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", MakeUnique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", MakeUnique<CLevelParserParam>(m_speed));

    return true;
}

bool CAutoInfo::Read(CLevelParserLine* line)
{
    if (!line->GetParam("aExist")->AsBool(false))
        return false;

    CAuto::Read(line);
    m_phase = static_cast<Phase>(line->GetParam("aPhase")->AsInt(static_cast<int>(Phase::Wait)));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}
