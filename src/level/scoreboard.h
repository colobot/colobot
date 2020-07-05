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
 * ScoreboardSortType sort=Name // sort teams alphabetically, another option is sort=Points, sorting teams in order of points
 * ScoreboardKillRule type=WheeledShooter team=1 score=500 // destruction of team 1's WheeledShooter gives 100 points to the team that destroyed it
 * ScoreboardKillRule type=TargetBot score=100 // destruction of TargetBot (any team) gives 100 points
 * ScoreboardObjectRule pos=0.0;0.5 dist=5.0 type=Titanium winTeam=1 score=5 // each Titanium within 5 meters of 0;0 gives team 1 5 points, losing Titanium gives -5
 * ScoreboardEndTakeRule score=1000 // completion of EndMissionTake objectives for any team results in 1000 points for that team
 * \endcode
 */
class CScoreboard
{
public:
    /**
     * \struct Score
     * \brief Struct containing score of individual team and additional variables to allow sorting teams through different criteria
    */
    struct Score
    {
        int points = 0; //!< Team score
        float time = 0; //!< Time when points were scored
    };

    /**
     * \enum SortType
     * \brief Enum defining the scoreboard sorting criteria
    */
    enum class SortType
    {
        SORT_ID,     //!< Sort by team ID
        SORT_POINTS, //!< Sort by points
    };

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
    class CScoreboardKillRule final : public CScoreboardRule, public CObjectCondition
    {
    public:
        bool friendlyFire = false;

        //! Read from line in scene file
        void Read(CLevelParserLine* line) override;
    };

    /**
     * \class CScoreboardObjectRule
     * \brief Scoreboard rule for counting objects
     * \see CScoreboard::AddObjectRule()
     */
    class CScoreboardObjectRule final : public CScoreboardRule, public CObjectCondition
    {
    public:
        int winTeam = 0;

        //! Read from line in scene file
        void Read(CLevelParserLine* line) override;

        int lastCount = 0;
    };

    /**
     * \class CScoreboardEndTakeRule
     * \brief Scoreboard rule for EndMissionTake rewards
     * \see CScoreboard::AddEndTakeRule()
     */
    class CScoreboardEndTakeRule final : public CScoreboardRule
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
    //! Add ScoreboardObjectRule
    void AddObjectRule(std::unique_ptr<CScoreboardObjectRule> rule);
    //! Add ScoreboardEndTakeRule
    void AddEndTakeRule(std::unique_ptr<CScoreboardEndTakeRule> rule);

    //! Called after an object is destroyed by another object
    //! \param target The object that has just been destroyed
    //! \param killer The object that caused the destruction, can be null
    void ProcessKill(CObject* target, CObject* killer = nullptr);
    //! Updates the object count rules
    void UpdateObjectCount();
    //! Called after EndTake contition has been met, used to handle ScoreboardEndTakeRule
    void ProcessEndTake(int team);

    void AddPoints(int team, int points);
    Score GetScore(int team);
    void SetScore(int team, int points);

    SortType GetSortType();
    void SetSortType(SortType type);

    std::vector<std::pair<int, Score>> GetSortedScores();

private:
    std::vector<std::unique_ptr<CScoreboardKillRule>> m_rulesKill = {};
    std::vector<std::unique_ptr<CScoreboardObjectRule>> m_rulesObject = {};
    std::vector<std::unique_ptr<CScoreboardEndTakeRule>> m_rulesEndTake = {};
    std::map<int, Score> m_score;
    int m_finishCounter = 0;
    SortType m_sortType = SortType::SORT_ID;
};
