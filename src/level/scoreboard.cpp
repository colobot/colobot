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

#include "level/scoreboard.h"

#include "common/restext.h"
#include "common/stringutils.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"

#include "object/object.h"

#include "ui/displaytext.h"

#include <boost/lexical_cast.hpp>
#include <algorithm>

void CScoreboard::CScoreboardRule::Read(CLevelParserLine* line)
{
    this->score = line->GetParam("score")->AsInt();
}

void CScoreboard::CScoreboardKillRule::Read(CLevelParserLine* line)
{
    CScoreboardRule::Read(line);
    CObjectCondition::Read(line);
    this->friendlyFire = line->GetParam("friendlyFire")->AsBool(false);
}

void CScoreboard::CScoreboardObjectRule::Read(CLevelParserLine* line)
{
    CScoreboardRule::Read(line);
    CObjectCondition::Read(line);
    this->winTeam = line->GetParam("winTeam")->AsInt();
}

void CScoreboard::CScoreboardEndTakeRule::Read(CLevelParserLine* line)
{
    CScoreboardRule::Read(line);
    this->team = line->GetParam("team")->AsInt(0);
    this->order = line->GetParam("order")->AsInt(0);
}

void CScoreboard::AddKillRule(std::unique_ptr<CScoreboardKillRule> rule)
{
    m_rulesKill.push_back(std::move(rule));
}

void CScoreboard::AddObjectRule(std::unique_ptr<CScoreboard::CScoreboardObjectRule> rule)
{
    m_rulesObject.push_back(std::move(rule));
}

void CScoreboard::AddEndTakeRule(std::unique_ptr<CScoreboardEndTakeRule> rule)
{
    m_rulesEndTake.push_back(std::move(rule));
}

void CScoreboard::ProcessKill(CObject* target, CObject* killer)
{
    if (killer == nullptr) return;
    if (killer->GetTeam() == 0) return;
    for (auto& rule : m_rulesKill)
    {
        if ((rule->team == killer->GetTeam() || rule->team == 0) &&
            killer->GetTeam() != 0 &&
            rule->CheckForObject(target))
        {
            if (killer->GetTeam() == target->GetTeam() && !rule->friendlyFire)
                continue;
            AddPoints(killer->GetTeam(), rule->score);
        }
    }
}

void CScoreboard::UpdateObjectCount()
{
    for (auto& rule : m_rulesObject)
    {
        assert(rule->winTeam != 0);
        int count = rule->CountObjects();
        int countDiff = count - rule->lastCount;
        if (countDiff != 0)
        {
            rule->lastCount = count;
            AddPoints(rule->winTeam, rule->score * countDiff);
        }
    }
}

void CScoreboard::ProcessEndTake(int team)
{
    if (team == 0) return;
    m_finishCounter++;
    for (auto& rule : m_rulesEndTake)
    {
        if ((rule->team == team || rule->team == 0) &&
            (rule->order == m_finishCounter || rule->order == 0))
        {
            AddPoints(team, rule->score);
        }
    }
}

void CScoreboard::AddPoints(int team, int points)
{
    GetLogger()->Info("Team %d earned %d points\n", team, points);

    CRobotMain* main = CRobotMain::GetInstancePointer();
    std::string text;
    GetResource(RES_ERR, INFO_TEAM_SCORE, text);
    text = StrUtils::Format(text.c_str(), main->GetTeamName(team).c_str(), points);
    main->GetDisplayText()->DisplayText(text.c_str(), Math::Vector(0.0f,0.0f,0.0f), 15.0f, 60.0f, 10.0f, Ui::TT_WARNING);

    m_score[team].points += points;
    m_score[team].time = main->GetGameTime();
}

CScoreboard::Score CScoreboard::GetScore(int team)
{
    return m_score[team];
}

void CScoreboard::SetScore(int team, int points)
{
    m_score[team].points = points;
}

CScoreboard::SortType CScoreboard::GetSortType()
{
    return m_sortType;
}

void CScoreboard::SetSortType(SortType type)
{
    m_sortType = type;
}

std::vector<std::pair<int, CScoreboard::Score>> CScoreboard::GetSortedScores()
{
    CRobotMain* main = CRobotMain::GetInstancePointer();
    std::set<int> teams = main->GetAllTeams();
    std::vector<std::pair<int, Score>> sortedTeams(teams.size());
    std::transform(teams.begin(), teams.end(), sortedTeams.begin(), [&](int team)
    {
        return *m_score.find(team);
    });
    if (m_sortType == SortType::SORT_POINTS)
    {
        std::sort(sortedTeams.begin(), sortedTeams.end(), [&](std::pair<int, Score> teamA, std::pair<int, Score> teamB)
        {
            if (teamA.second.points > teamB.second.points) return true; // Team A have more points than B?
            if (teamA.second.points < teamB.second.points) return false; // Team A have less points than B?

            return teamA.second.time < teamB.second.time; // Team A scored slower than B?
        });
    }
    return sortedTeams;
}
