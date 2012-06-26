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

#ifndef _TASKGOTO_H_
#define _TASKGOTO_H_


#include "misc.h"
#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



#define MAXPOINTS   500



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
    CTaskGoto(CInstanceManager* iMan, CObject* object);
    ~CTaskGoto();

    BOOL        EventProcess(const Event &event);

    Error       Start(D3DVECTOR goal, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
    Error       IsEnded();

protected:
    CObject*    WormSearch(D3DVECTOR &impact);
    void        WormFrame(float rTime);
    CObject*    SearchTarget(D3DVECTOR pos, float margin);
    BOOL        AdjustTarget(CObject* pObj, D3DVECTOR &pos, float &distance);
    BOOL        AdjustBuilding(D3DVECTOR &pos, float margin, float &distance);
    BOOL        GetHotPoint(CObject *pObj, D3DVECTOR &pos, BOOL bTake, float distance, float &suppl);
    BOOL        LeakSearch(D3DVECTOR &pos, float &delay);
    void        ComputeRepulse(FPOINT &dir);
    void        ComputeFlyingRepulse(float &dir);

    int         BeamShortcut();
    void        BeamStart();
    void        BeamInit();
    Error       BeamSearch(const D3DVECTOR &start, const D3DVECTOR &goal, float goalRadius);
    Error       BeamExplore(const D3DVECTOR &prevPos, const D3DVECTOR &curPos, const D3DVECTOR &goalPos, float goalRadius, float angle, int nbDiv, float step, int i, int nbIter);
    D3DVECTOR   BeamPoint(const D3DVECTOR &startPoint, const D3DVECTOR &goalPoint, float angle, float step);

    void        BitmapDebug(const D3DVECTOR &min, const D3DVECTOR &max, const D3DVECTOR &start, const D3DVECTOR &goal);
    BOOL        BitmapTestLine(const D3DVECTOR &start, const D3DVECTOR &goal, float stepAngle, BOOL bSecond);
    void        BitmapObject();
    void        BitmapTerrain(const D3DVECTOR &min, const D3DVECTOR &max);
    void        BitmapTerrain(int minx, int miny, int maxx, int maxy);
    BOOL        BitmapOpen();
    BOOL        BitmapClose();
    void        BitmapSetCircle(const D3DVECTOR &pos, float radius);
    void        BitmapClearCircle(const D3DVECTOR &pos, float radius);
    void        BitmapSetDot(int rank, int x, int y);
    void        BitmapClearDot(int rank, int x, int y);
    BOOL        BitmapTestDot(int rank, int x, int y);

protected:
    D3DVECTOR       m_goal;
    D3DVECTOR       m_goalObject;
    float           m_angle;
    float           m_altitude;
    TaskGotoCrash   m_crashMode;
    TaskGotoGoal    m_goalMode;
    TaskGotoPhase   m_phase;
    int             m_try;
    Error           m_error;
    BOOL            m_bTake;
    float           m_stopLength;   // braking distance
    float           m_time;
    D3DVECTOR       m_pos;
    BOOL            m_bWorm;
    BOOL            m_bApprox;
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
    D3DVECTOR       m_bmPoints[MAXPOINTS+2];
    char            m_bmIter[MAXPOINTS+2];
    int             m_bmIterCounter;
    CObject*        m_bmFretObject;
    float           m_bmFinalMove;  // final advance distance
    float           m_bmFinalDist;  // effective distance to advance
    D3DVECTOR       m_bmFinalPos;   // initial position before advance
    float           m_bmTimeLimit;
    int             m_bmStep;
    D3DVECTOR       m_bmWatchDogPos;
    float           m_bmWatchDogTime;
    D3DVECTOR       m_leakPos;      // initial position leak
    float           m_leakDelay;
    float           m_leakTime;
    BOOL            m_bLeakRecede;
};


#endif //_TASKGOTO_H_
