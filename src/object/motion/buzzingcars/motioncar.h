/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/motion/motion.h"

#include "graphics/engine/camera.h"


#define MV_OPEN        0
#define MV_CLOSE    1
#define MV_FLATY    2
#define MV_FLATZ    3
#define MV_DEFLAT    4
#define MV_LOADBOT    5

#define MAX_PART    50


struct RemovePart
{
    short        part;        // num�ro de la pi�ce
    short        order;        // ordre pour destruction
    float        x,y,z;        // position
    short        light[4];    // phares attach�s � la pi�ce
    short        top;        // 1 -> partie du to�t
    short        file;        // num�ro pp dans "carmmpp.mod"
};

struct StatusPart
{
    char        bDeleted;    // pi�ce d�truite ?
    short        part;        // num�ro de la pi�ce
    short        order;        // ordre pour destruction
    short        light[4];    // phares sur cette partie
    short        top;        // fait partie du to�t ?
    short        twistState;    // avancement du d�labrement
    Math::Vector    twistAngle;    // angle max si d�labrement max
};

struct StatePart
{
    char        bDeleted;    // pi�ce d�truite
    short        twistState;    // avancement du d�labrement
    Math::Vector    twistAngle;    // angle max si d�labrement max
};

struct StateVehicle
{
    StatePart    state[MAX_PART];
};


class CMotionCar : public CMotion
{
public:
    CMotionCar(COldObject* object);
    ~CMotionCar();

    void        DeleteObject(bool bAll=false) override;
    void        Create(Math::Vector pos, float angle, ObjectType type, float power, Gfx::COldModelManager* modelManager) override;
    bool        EventProcess(const Event &event) override;
    Error       SetAction(int action, float time=0.2f) override;

    void        TwistInit();
    void        TwistPart(Math::Vector impact, float force);
    int         GetRemovePart(int &param);
    bool        GetLight(int rank);
    bool        ExistPart(TypePart part);
    int         GetTotalPart();
    int         GetUsedPart();

    void        SetWheelType(WheelType type) override;
    WheelType   GetWheelType() override;

    int         GetStateLength();
    void        GetStateBuffer(char *buffer);

protected:
    void        CreateWheel(Gfx::COldModelManager* modelManager, char* name);
    void CreatePart(Gfx::COldModelManager* modelManager, RemovePart* table, float zoom);
    void        CreatePhysics();
    bool        EventFrame(const Event &event);
    void        OpenClosePart(float progress);
    void        UpdateWheelMapping(float speed, bool bFront);
    void        UpdateGlassMapping(int part, float min, float max, float dir, Gfx::EngineTextureMapping mode, bool bBlack=false);
    void        UpdateTrackMapping(float left, float right);
    bool        CrashVehicle(Math::Vector zoom);
    bool        IsUnderWater();
    void        TwistInit(StateVehicle *sv);

    void        ActionLoadBot(float rTime);
    Math::Vector    GetVehiclePoint(CObject *pObj);
    void        StartObjectAction(COldObject* pObj, int action);

protected:
    int            m_model;
    int            m_option;
    float        m_posTrackLeft;
    float        m_posTrackRight;
    StatusPart    m_part[MAX_PART];
    int            m_partTotal;
    int            m_partUsed;
    int            m_topRest;        // nb de pi�ces du to�t restantes
    bool        m_bLight[6];    // pr�sence des phares
    bool        m_bWater;
    bool        m_bHelico;
    Math::Vector    m_wheelLastPos;
    Math::Vector    m_wheelLastAngle;
    float        m_flatTime;
    float        m_lastWheelSpeed[2];
    char        m_nameObjWheel[30];
    char        m_nameObjWBurn[30];
    char        m_nameTexWheel[20];
                                // action LoadBot :
    bool        m_lbAction;
    bool        m_lbStart;
    float        m_lbProgress;
    float        m_lbSpeed;
    float        m_lbTime;
    Gfx::CameraType    m_lbCamera;
    float        m_lbBackDist;
    COldObject*    m_lbWalker;
    Math::Vector    m_lbStartPos;
    Math::Vector    m_lbGoalPos;
    float        m_lbStartAngle;
    float        m_lbGoalAngle;
    float        m_lbLastParticle;
};
