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

// pyro.h

#ifndef _PYRO_H_
#define _PYRO_H_


#include "d3dengine.h"
#include "object.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CCamera;
class CParticule;
class CLight;
class CObject;
class CDisplayText;
class CRobotMain;
class CSound;



enum PyroType
{
    PT_NULL     = 0,
    PT_FRAGT    = 1,        // fragmentation of technical object
    PT_FRAGO    = 2,        // fragmentation of organic object
    PT_FRAGW    = 4,        // fragmentation of object under water
    PT_EXPLOT   = 5,        // explosion of technical object
    PT_EXPLOO   = 6,        // explosion of organic object
    PT_EXPLOW   = 8,        // explosion of object under water
    PT_SHOTT    = 9,        // hit technical object
    PT_SHOTH    = 10,       // hit human
    PT_SHOTM    = 11,       // hit queen
    PT_SHOTW    = 12,       // hit under water
    PT_EGG      = 13,       // break the egg
    PT_BURNT    = 14,       // burning of technical object
    PT_BURNO    = 15,       // burning of organic object
    PT_SPIDER   = 16,       // spider explosion
    PT_FALL     = 17,       // cargo falling
    PT_WPCHECK  = 18,       // indicator reaches
    PT_FLCREATE = 19,       // flag create
    PT_FLDELETE = 20,       // flag destroy
    PT_RESET    = 21,       // reset position of the object
    PT_WIN      = 22,       // fireworks
    PT_LOST     = 23,       // black smoke
    PT_DEADG    = 24,       // shooting death
    PT_DEADW    = 25,       // drowning death
    PT_FINDING  = 26,       // object discovered
};


typedef struct
{
    int             part;
    D3DVECTOR       initialPos;
    D3DVECTOR       finalPos;
    D3DVECTOR       initialAngle;
    D3DVECTOR       finalAngle;
}
PyroBurnPart;

typedef struct
{
    float           progress;
    float           intensity;
    D3DCOLORVALUE   color;
}
PyroLightOper;



class CPyro
{
public:
    CPyro(CInstanceManager* iMan);
    ~CPyro();

    void        DeleteObject(BOOL bAll=FALSE);
    BOOL        Create(PyroType type, CObject* pObj, float force=1.0f);
    BOOL        EventProcess(const Event &event);
    Error       IsEnded();
    void        CutObjectLink(CObject* pObj);

protected:
    void        DisplayError(PyroType type, CObject* pObj);
    BOOL        CreateLight(D3DVECTOR pos, float height);
    void        DeleteObject(BOOL bPrimary, BOOL bSecondary);

    void        CreateTriangle(CObject* pObj, ObjectType oType, int part);

    void        ExploStart();
    void        ExploTerminate();

    void        BurnStart();
    void        BurnAddPart(int part, D3DVECTOR pos, D3DVECTOR angle);
    void        BurnProgress();
    BOOL        BurnIsKeepPart(int part);
    void        BurnTerminate();

    void        FallStart();
    CObject*    FallSearchBeeExplo();
    void        FallProgress(float rTime);
    Error       FallIsEnded();

    void        LightOperFlush();
    void        LightOperAdd(float progress, float intensity, float r, float g, float b);
    void        LightOperFrame(float rTime);

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CTerrain*       m_terrain;
    CCamera*        m_camera;
    CParticule*     m_particule;
    CLight*         m_light;
    CObject*        m_object;
    CDisplayText*   m_displayText;
    CRobotMain*     m_main;
    CSound*         m_sound;

    D3DVECTOR       m_pos;          // center of the effect
    D3DVECTOR       m_posPower;     // center of the battery
    BOOL            m_bPower;       // battery exists?
    PyroType        m_type;
    float           m_force;
    float           m_size;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_lastParticule;
    float           m_lastParticuleSmoke;
    int             m_soundChannel;

    int             m_lightRank;
    int             m_lightOperTotal;
    PyroLightOper   m_lightOper[10];
    float           m_lightHeight;

    ObjectType      m_burnType;
    int             m_burnPartTotal;
    PyroBurnPart    m_burnPart[10];
    int             m_burnKeepPart[10];
    float           m_burnFall;

    float           m_fallFloor;
    float           m_fallSpeed;
    float           m_fallBulletTime;
    BOOL            m_bFallEnding;

    int             m_crashSphereUsed;  // number of spheres used
    D3DVECTOR       m_crashSpherePos[50];
    float           m_crashSphereRadius[50];
};


#endif //_PYRO_H_
