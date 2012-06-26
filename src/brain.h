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

// brain.h

#ifndef _BRAIN_H_
#define _BRAIN_H_


#include "misc.h"
#include "event.h"
#include "object.h"
#include "taskmanip.h"
#include "taskflag.h"
#include "taskshield.h"


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CWater;
class CCamera;
class CObject;
class CPhysics;
class CMotion;
class CTaskManager;
class CInterface;
class CWindow;
class CDisplayText;
class CScript;
class CRobotMain;
class CStudio;
class CSound;
class CParticule;


#define BRAINMAXSCRIPT      10



enum TraceOper
{
    TO_STOP         = 0,    // stop
    TO_ADVANCE      = 1,    // advance
    TO_RECEDE       = 2,    // back
    TO_TURN         = 3,    // rotate
    TO_PEN          = 4,    // color change
};

typedef struct
{
    TraceOper   oper;
    float       param;
}
TraceRecord;



class CBrain
{
public:
    CBrain(CInstanceManager* iMan, CObject* object);
    ~CBrain();

    void        DeleteObject(BOOL bAll=FALSE);

    void        SetPhysics(CPhysics* physics);
    void        SetMotion(CMotion* motion);

    BOOL        EventProcess(const Event &event);
    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

    BOOL        IsBusy();
    void        SetActivity(BOOL bMode);
    BOOL        RetActivity();
    BOOL        IsProgram();
    BOOL        ProgramExist(int rank);
    void        RunProgram(int rank);
    int         FreeProgram();
    int         RetProgram();
    void        StopProgram();
    void        StopTask();

    BOOL        IntroduceVirus();
    void        SetActiveVirus(BOOL bActive);
    BOOL        RetActiveVirus();

    void        SetScriptRun(int rank);
    int         RetScriptRun();
    void        SetScriptName(int rank, char *name);
    char*       RetScriptName(int rank);
    void        SetSoluceName(char *name);
    char*       RetSoluceName();

    BOOL        ReadSoluce(char* filename);
    BOOL        ReadProgram(int rank, char* filename);
    BOOL        RetCompile(int rank);
    BOOL        WriteProgram(int rank, char* filename);
    BOOL        ReadStack(FILE *file);
    BOOL        WriteStack(FILE *file);

    Error       StartTaskTake();
    Error       StartTaskManip(TaskManipOrder order, TaskManipArm arm);
    Error       StartTaskFlag(TaskFlagOrder order, int rank);
    Error       StartTaskBuild(ObjectType type);
    Error       StartTaskSearch();
    Error       StartTaskTerraform();
    Error       StartTaskPen(BOOL bDown, int color);
    Error       StartTaskRecover();
    Error       StartTaskShield(TaskShieldMode mode);
    Error       StartTaskFire(float delay);
    Error       StartTaskFireAnt(D3DVECTOR impact);
    Error       StartTaskGunGoal(float dirV, float dirH);
    Error       StartTaskReset(D3DVECTOR goal, D3DVECTOR angle);

    void        UpdateInterface(float rTime);
    void        UpdateInterface();

protected:
    BOOL        EventFrame(const Event &event);

    void        StartEditScript(int rank, char* name);
    void        StopEditScript(BOOL bCancel);

    Error       EndedTask();

    void        GroundFlat();
    void        ColorFlag(int color);

    void        UpdateScript(CWindow *pw);
    int         RetSelScript();
    void        BlinkScript(BOOL bEnable);

    void        CheckInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        EnableInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        DeadInterface(CWindow *pw, EventMsg event, BOOL bState);
    void        DefaultEnter(CWindow *pw, EventMsg event, BOOL bState=TRUE);

    void        TraceRecordStart();
    void        TraceRecordFrame();
    void        TraceRecordStop();
    BOOL        TraceRecordOper(TraceOper oper, float param);
    BOOL        TraceRecordPut(char *buffer, int max, TraceOper oper, float param);

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CTerrain*       m_terrain;
    CWater*         m_water;
    CCamera*        m_camera;
    CObject*        m_object;
    CPhysics*       m_physics;
    CMotion*        m_motion;
    CInterface*     m_interface;
    CDisplayText*   m_displayText;
    CRobotMain*     m_main;
    CStudio*        m_studio;
    CSound*         m_sound;
    CParticule*     m_particule;
    CTaskManager*   m_primaryTask;
    CTaskManager*   m_secondaryTask;

    CScript*    m_script[BRAINMAXSCRIPT];
    int         m_selScript;        // rank of the selected script
    int         m_program;      // rank of the executed program / ​​-1
    BOOL        m_bActivity;
    BOOL        m_bBurn;
    BOOL        m_bActiveVirus;

    int         m_scriptRun;
    char        m_scriptName[BRAINMAXSCRIPT][50];
    char        m_soluceName[50];

    EventMsg    m_buttonAxe;
    EventMsg    m_manipStyle;
    EventMsg    m_defaultEnter;
    EventMsg    m_interfaceEvent[100];

    CObject*    m_antTarget;
    CObject*    m_beeBullet;
    float       m_beeBulletSpeed;
    D3DVECTOR   m_startPos;
    float       m_time;
    float       m_burnTime;
    float       m_lastUpdateTime;
    float       m_lastHumanTime;
    float       m_lastSpiderTime;
    float       m_lastWormTime;
    float       m_lastBulletTime;
    float       m_lastAlarmTime;
    int         m_soundChannelAlarm;
    int         m_flagColor;

    BOOL        m_bTraceRecord;
    TraceOper   m_traceOper;
    D3DVECTOR   m_tracePos;
    float       m_traceAngle;
    int         m_traceColor;
    int         m_traceRecordIndex;
    TraceRecord* m_traceRecordBuffer;
};


#endif //_BRAIN_H_
