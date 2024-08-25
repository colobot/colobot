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

#pragma once


#include "object/task/task.h"

#include "object/object_type.h"

#include <glm/glm.hpp>


class CObject;

const float BUILDMARGIN = 16.0f;
const int TBMAXLIGHT    = 4;


enum TaskBuildPhase
{
    TBP_STOP    = 0,    // cancels task
    TBP_TURN    = 1,    // turns
    TBP_MOVE    = 2,    // forward/backward
    TBP_TAKE    = 3,    // takes gun
    TBP_PREP    = 4,    // prepares
    TBP_BUILD   = 5,    // builds
    TBP_TERM    = 6,    // ends
    TBP_RECEDE  = 7,    // back terminal
};



class CTaskBuild : public CForegroundTask
{
public:
    CTaskBuild(COldObject* object);
    ~CTaskBuild();

    bool        EventProcess(const Event &event) override;

    Error       Start(ObjectType type);
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    Error       FlatFloor();
    void        CreateBuilding(glm::vec3 pos, float angle, bool trainer);
    void        CreateLight();
    void        BlackLight();
    CObject*    SearchMetalObject(float &angle, float dMin, float dMax, float aLimit, Error &err);
    void        DeleteMark(glm::vec3 pos, float radius);

protected:
    ObjectType      m_type = OBJECT_NULL;                  // type of construction
    CObject*        m_metal = nullptr;                 // transforms metal object
    CObject*        m_building = nullptr;              // building built
    TaskBuildPhase  m_phase = TBP_STOP;                 // phase of the operation
    bool            m_bError = false;                // true -> operation impossible
    bool            m_bBuild = false;                // true -> building built
    bool            m_bBlack = false;                // true -> lights black -> white
    float           m_time = 0.0f;                  // absolute time
    float           m_lastParticle = 0.0f;          // time of generation last particle
    float           m_progress = 0.0f;              // progression (0..1)
    float           m_speed = 0.0f;                 // speed of progression
    float           m_angleY = 0.0f;                // rotation angle of the vehicle
    float           m_angleZ = 0.0f;                // angle of rotation of the gun
    glm::vec3       m_buildingPos = { 0, 0, 0 };           // initial position of the building
    float           m_buildingHeight = 0.0f;        // height of the building
    int             m_lightRank[TBMAXLIGHT] = {}; // lights for the effects
    int             m_soundChannel = 0;
};
