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

#include "object/task/task.h"

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <vector>


class CObject;

const int MAXPOINTS = 50000;
const int NUMQUEUEBUCKETS = 32;

enum TaskGotoGoal
{
    TGG_DEFAULT     = -1,   // default mode
    TGG_STOP        = 0,    // goes to destination pausing with precision
    TGG_EXPRESS     = 1,    // goes to destination without stopping
};

enum TaskGotoCrash
{
    TGC_DEFAULT     = -1,   // default mode
    TGC_HALT        = 0,    // stops if collision
    TGC_RIGHTLEFT       = 1,    // right-left
    TGC_LEFTRIGHT       = 2,    // left-right
    TGC_LEFT        = 3,    // left
    TGC_RIGHT       = 4,    // right
    TGC_BEAM        = 5,    // algorithm "sunlight"
};


enum TaskGotoPhase
{
    TGP_ADVANCE     = 1,    // advance
    TGP_LAND        = 2,    // landed
    TGP_TURN        = 3,    // turns to finish
    TGP_MOVE        = 4,    // advance to finish
    TGP_CRWAIT      = 5,    // waits after collision
    TGP_CRTURN      = 6,    // turns right after collision
    TGP_CRADVANCE       = 7,    // advance to the right after collision
    TGP_CLWAIT      = 8,    // waits after collision
    TGP_CLTURN      = 9,    // turns left after collision
    TGP_CLADVANCE       = 10,   // advance to the left after collision
    TGP_BEAMLEAK        = 11,   // beam: leak (leaking)
    TGP_BEAMSEARCH      = 12,   // beam: search
    TGP_BEAMWCOLD       = 13,   // beam: expects cool reactor
    TGP_BEAMUP      = 14,   // beam: off
    TGP_BEAMGOTO        = 15,   // beam: goto dot list
    TGP_BEAMDOWN        = 16,   // beam: landed
};



class CTaskGoto : public CForegroundTask
{
public:
    CTaskGoto(COldObject* object);
    ~CTaskGoto();

    bool        EventProcess(const Event &event) override;

    Error       Start(glm::vec3 goal, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
    Error       IsEnded() override;

protected:
    CObject*    WormSearch(glm::vec3 &impact);
    void        WormFrame(float rTime);
    CObject*    SearchTarget(glm::vec3 pos, float margin);
    bool        AdjustTarget(CObject* pObj, glm::vec3 &pos, float &distance);
    bool        AdjustBuilding(glm::vec3 &pos, float margin, float &distance);
    bool        GetHotPoint(CObject *pObj, glm::vec3 &pos, bool bTake, float distance, float &suppl);
    bool        LeakSearch(glm::vec3 &pos, float &delay);
    void        ComputeRepulse(glm::vec2& dir);
    void        ComputeFlyingRepulse(float &dir);

    int         PathFindingShortcut();
    void        PathFindingStart();
    void        PathFindingInit();
    Error       PathFindingSearch(const glm::vec3 &start, const glm::vec3 &goal, float goalRadius);

    bool        BitmapTestLine(const glm::vec3 &start, const glm::vec3 &goal);
    void        BitmapObject();
    void        BitmapTerrain(const glm::vec3 &min, const glm::vec3 &max);
    void        BitmapTerrain(int minx, int miny, int maxx, int maxy);
    bool        BitmapOpen();
    bool        BitmapClose();
    void        BitmapSetCircle(const glm::vec3 &pos, float radius);
    void        BitmapClearCircle(const glm::vec3 &pos, float radius);
    void        BitmapSetDot(int rank, int x, int y);
    void        BitmapClearDot(int rank, int x, int y);
    bool        BitmapTestDot(int rank, int x, int y);
    bool        BitmapTestDotIsVisitable(int x, int y);

protected:
    glm::vec3       m_goal = { 0, 0, 0 };
    glm::vec3       m_goalObject = { 0, 0, 0 };
    float           m_angle = 0.0f;
    float           m_altitude = 0.0f;
    TaskGotoCrash   m_crashMode = TGC_DEFAULT;
    TaskGotoGoal    m_goalMode = TGG_DEFAULT;
    TaskGotoPhase   m_phase = TGP_ADVANCE;
    int             m_try = 0;
    Error           m_error = ERR_OK;
    bool            m_bTake = false;
    float           m_stopLength = 0.0f;   // braking distance
    float           m_time = 0.0f;
    glm::vec3       m_pos = { 0, 0, 0 };
    bool            m_bWorm = false;
    bool            m_bApprox = false;
    float           m_wormLastTime = 0.0f;
    float           m_lastDistance = 0.0f;

    bool            m_bmChanged = true;
    int             m_bmSize = 0;       // width or height of the table
    int             m_bmOffset = 0;     // m_bmSize/2
    int             m_bmLine = 0;       // increment line m_bmSize/8
    std::unique_ptr<unsigned char[]> m_bmArray;      // Bit table
    std::unique_ptr<int32_t[]> m_bfsDistances; // Distances to the goal for breadth-first search.
    std::array<std::vector<uint32_t>, NUMQUEUEBUCKETS + 1> m_bfsQueue; // Priority queue with indices to nodes. Nodes are sorted into buckets. The last bucket contains oversized costs.
    int             m_bfsQueueMin = 0;  // Front of the queue. This value mod 8 is the index to the bucket with the next node to be expanded.
    int             m_bfsQueueCountPushed = 0; // Number of nodes inserted into the queue.
    int             m_bfsQueueCountPopped = 0; // Number of nodes extacted from the queue.
    int             m_bfsQueueCountRepeated = 0; // Number of nodes re-inserted into the queue.
    int             m_bfsQueueCountSkipped = 0; // Number of nodes skipped because of unexpected distance (likely re-added).
    int             m_bmMinX = 0, m_bmMinY = 0;
    int             m_bmMaxX = 0, m_bmMaxY = 0;
    int             m_bmTotal = 0;      // index of final point in m_bmPoints
    int             m_bmIndex = 0;      // index in m_bmPoints
    glm::vec3       m_bmPoints[MAXPOINTS+2];
    signed char     m_bmIter[MAXPOINTS+2] = {};
    int             m_bmIterCounter = 0;
    CObject*        m_bmCargoObject = nullptr;
    float           m_bmFinalMove = 0.0f;  // final advance distance
    float           m_bmFinalDist = 0.0f;  // effective distance to advance
    glm::vec3       m_bmFinalPos = { 0, 0, 0 };   // initial position before advance
    float           m_bmTimeLimit = 0.0f;
    int             m_bmStep = 0;
    glm::vec3       m_bmWatchDogPos = { 0, 0, 0 };
    float           m_bmWatchDogTime = 0.0f;
    glm::vec3       m_leakPos = { 0, 0, 0 };      // initial position leak
    float           m_leakDelay = 0.0f;
    float           m_leakTime = 0.0f;
    bool            m_bLeakRecede = false;
};
