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

/**
 * \file level/scoreboard.h
 * \brief Code responsible for managing the scoreboard used to score complex code battles
 */

#pragma once

#include "level/scene_conditions.h"

#include <memory>
#include <vector>
#include <map>

class CObject;

/**
 * \class CScoreboard
 * \brief Scoreboard used to score complex code battles
 *
 * \todo This is pretty much a work-in-progress hack for Diversity. Be wary of possible API changes.
 *
 * \todo Proper documentation
 *
 * \see CRobotMain::GetScoreboard()
 *
 * \section example Usage example
 * \code{.scene}
 * Scoreboard enable=true // enable the scoreboard
 * ScoreboardKillRule type=WheeledShooter team=1 score=500 // destruction of team 1's WheeledShooter gives 100 points to the team that destroyed it
 * ScoreboardKillRule type=TargetBot score=100 // destruction of TargetBot (any team) gives 100 points
 * ScoreboardEndTakeRule score=1000 // completion of EndMissionTake objectives for any team results in 1000 points for that team
 * \endcode
 */
class CScoreboard
{
public:
    //! Creates the scoreboard
    //! The scoreboard exists only if enabled in level file
    CScoreboard() {};
    //! Destroys the scoreboard
    ~CScoreboard() {};

public:
    /**
     * \class CScoreboardRule
     * \brief Base class for scoreboard rules
     */
    class CScoreboardRule
    {
    public:
        int score = 0;

        //! Read from line in scene file
        virtual void Read(CLevelParserLine* line);
    };

    /**
     * \class CScoreboardKillRule
     * \brief Scoreboard rule for destroying other objects
     * \see CScoreboard::AddKillRule()
     */
    class CScoreboardKillRule : public CScoreboardRule, public CObjectCondition
    {
    public:
        //! Read from line in scene file
        void Read(CLevelParserLine* line) override;
    };

    /**
     * \class CScoreboardEndTakeRule
     * \brief Scoreboard rule for EndMissionTake rewards
     * \see CScoreboard::AddEndTakeRule()
     */
    class CScoreboardEndTakeRule : public CScoreboardRule
    {
    public:
        int team = 0;
        int order = 0;

        //! Read from line in scene file
        void Read(CLevelParserLine* line) override;
    };

public:
    //! Add ScoreboardKillRule
    void AddKillRule(std::unique_ptr<CScoreboardKillRule> rule);
    //! Add ScoreboardEndTakeRule
    void AddEndTakeRule(std::unique_ptr<CScoreboardEndTakeRule> rule);

    //! Called after an object is destroyed by another object
    //! \param target The object that has just been destroyed
    //! \param killer The object that caused the destruction, can be null
    void ProcessKill(CObject* target, CObject* killer = nullptr);
    //! Called after EndTake contition has been met, used to handle ScoreboardEndTakeRule
    void ProcessEndTake(int team);

    void AddPoints(int team, int points);
    int GetScore(int team);
    void SetScore(int team, int points);

private:
    std::vector<std::unique_ptr<CScoreboardKillRule>> m_rulesKill = {};
    std::vector<std::unique_ptr<CScoreboardEndTakeRule>> m_rulesEndTake = {};
    std::map<int, int> m_score;
    int m_finishCounter = 0;
};
