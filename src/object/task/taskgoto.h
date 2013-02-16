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

// taskgoto.h

#pragma once


#include "object/task/task.h"

#include "math/vector.h"



const int MAXPOINTS = 500;



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



class CTaskGoto : public CTask
{
public:
    CTaskGoto(CObject* object);
    ~CTaskGoto();

    bool        EventProcess(const Event &event);

    Error       Start(Math::Vector goal, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
    Error       IsEnded();

protected:
    CObject*    WormSearch(Math::Vector &impact);
    void        WormFrame(float rTime);
    CObject*    SearchTarget(Math::Vector pos, float margin);
    bool        AdjustTarget(CObject* pObj, Math::Vector &pos, float &distance);
    bool        AdjustBuilding(Math::Vector &pos, float margin, float &distance);
    bool        GetHotPoint(CObject *pObj, Math::Vector &pos, bool bTake, float distance, float &suppl);
    bool        LeakSearch(Math::Vector &pos, float &delay);
    void        ComputeRepulse(Math::Point &dir);
    void        ComputeFlyingRepulse(float &dir);

    int         BeamShortcut();
    void        BeamStart();
    void        BeamInit();
    Error       BeamSearch(const Math::Vector &start, const Math::Vector &goal, float goalRadius);
    Error       BeamExplore(const Math::Vector &prevPos, const Math::Vector &curPos, const Math::Vector &goalPos, float goalRadius, float angle, int nbDiv, float step, int i, int nbIter);
    Math::Vector    BeamPoint(const Math::Vector &startPoint, const Math::Vector &goalPoint, float angle, float step);

    void        BitmapDebug(const Math::Vector &min, const Math::Vector &max, const Math::Vector &start, const Math::Vector &goal);
    bool        BitmapTestLine(const Math::Vector &start, const Math::Vector &goal, float stepAngle, bool bSecond);
    void        BitmapObject();
    void        BitmapTerrain(const Math::Vector &min, const Math::Vector &max);
    void        BitmapTerrain(int minx, int miny, int maxx, int maxy);
    bool        BitmapOpen();
    bool        BitmapClose();
    void        BitmapSetCircle(const Math::Vector &pos, float radius);
    void        BitmapClearCircle(const Math::Vector &pos, float radius);
    void        BitmapSetDot(int rank, int x, int y);
    void        BitmapClearDot(int rank, int x, int y);
    bool        BitmapTestDot(int rank, int x, int y);

protected:
    Math::Vector        m_goal;
    Math::Vector        m_goalObject;
    float           m_angle;
    float           m_altitude;
    TaskGotoCrash   m_crashMode;
    TaskGotoGoal    m_goalMode;
    TaskGotoPhase   m_phase;
    int             m_try;
    Error           m_error;
    bool            m_bTake;
    float           m_stopLength;   // braking distance
    float           m_time;
    Math::Vector        m_pos;
    bool            m_bWorm;
    bool            m_bApprox;
    float           m_wormLastTime;
    float           m_lastDistance;

    int             m_bmSize;       // width or height of the table
    int             m_bmOffset;     // m_bmSize/2
    int             m_bmLine;       // increment line m_bmSize/8
    unsigned char*          m_bmArray;      // bit table
    int             m_bmMinX, m_bmMinY;
    int             m_bmMaxX, m_bmMaxY;
    int             m_bmTotal;      // number of points in m_bmPoints
    int             m_bmIndex;      // index in m_bmPoints
    Math::Vector        m_bmPoints[MAXPOINTS+2];
    char            m_bmIter[MAXPOINTS+2];
    int             m_bmIterCounter;
    CObject*        m_bmFretObject;
    float           m_bmFinalMove;  // final advance distance
    float           m_bmFinalDist;  // effective distance to advance
    Math::Vector        m_bmFinalPos;   // initial position before advance
    float           m_bmTimeLimit;
    int             m_bmStep;
    Math::Vector        m_bmWatchDogPos;
    float           m_bmWatchDogTime;
    Math::Vector        m_leakPos;      // initial position leak
    float           m_leakDelay;
    float           m_leakTime;
    bool            m_bLeakRecede;
};

