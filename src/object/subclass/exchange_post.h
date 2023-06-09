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


#pragma once

#include "object/subclass/base_building.h"

#include "object/auto/auto.h"

#include <string>
#include <vector>

#include <optional>

struct ExchangePostInfo
{
    std::string name;       //!< name of the information
    float value = 0.0f;     //!< value of the information
};

struct ObjectCreateParams;

namespace Gfx
{
class COldModelManager;
class CEngine;
}

class CExchangePost : public CBaseBuilding
{
public:
    CExchangePost(int id);

    static std::unique_ptr<CExchangePost> Create(
        const ObjectCreateParams& params,
        Gfx::COldModelManager* modelManager,
        Gfx::CEngine* engine);

    static int GetMaximumInfoListSize();

    bool SetInfo(const std::string& name, float value);
    const std::vector<ExchangePostInfo>& GetInfoList();
    std::optional<float> GetInfoValue(const std::string& name);
    bool HasInfo(const std::string& name);
    bool DeleteInfo(const std::string& name);

    void SetInfoUpdate(bool update);
    bool GetInfoUpdate();

    void Write(CLevelParserLine* line) override;
    void Read(CLevelParserLine* line) override;

    void ReadInfo(CLevelParserLine* line);

private:
    std::vector<ExchangePostInfo> m_infoList;
    bool m_infoUpdate;
};

// TODO: integrate this with CExchangePost
class CAutoInfo : public CAuto
{
public:
    CAutoInfo(CExchangePost* object);
    ~CAutoInfo();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    void        Start(int param) override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool select) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    void        UpdateInterface(float rTime);
    void        UpdateList();
    void        UpdateListVirus();

protected:
    CExchangePost*  m_exchangePost;
    enum class Phase : unsigned int;
    Phase           m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastParticle;
    Math::Vector    m_goal;
    bool            m_lastVirus;
};
