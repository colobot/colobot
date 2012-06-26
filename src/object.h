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

// object.h

#ifndef _OBJECT_H_
#define _OBJECT_H_


#include "d3dengine.h"
#include "camera.h"
#include "sound.h"


class CInstanceManager;
class CLight;
class CTerrain;
class CWater;
class CParticule;
class CPhysics;
class CBrain;
class CMotion;
class CAuto;
class CDisplayText;
class CRobotMain;
class CBotVar;
class CScript;



// The father of all parts must always be the part number zero!

#define OBJECTMAXPART       40
#define MAXCRASHSPHERE      40
#define OBJECTMAXDESELLIST  10
#define OBJECTMAXINFO       10
#define OBJECTMAXCMDLINE    20

enum ObjectType
{
    OBJECT_NULL     = 0,    // object destroyed
    OBJECT_FIX      = 1,    // stationary scenery
    OBJECT_PORTICO          = 2,    // gantry
    OBJECT_BASE     = 3,    // great main base
    OBJECT_DERRICK          = 4,    // derrick set
    OBJECT_FACTORY          = 5,    // factory set
    OBJECT_STATION      = 6,    // recharging station
    OBJECT_CONVERT      = 7,    // converter station
    OBJECT_REPAIR       = 8,    // reparation
    OBJECT_TOWER            = 9,    // defense tower
    OBJECT_NEST     = 10,   // nest
    OBJECT_RESEARCH         = 11,   // research center
    OBJECT_RADAR            = 12,   // radar
    OBJECT_ENERGY           = 13,   // energy factory
    OBJECT_LABO     = 14,   // analytical laboratory for insect
    OBJECT_NUCLEAR      = 15,   // nuclear power plant
    OBJECT_START        = 16,   // starting
    OBJECT_END      = 17,   // finish
    OBJECT_INFO     = 18,   // information terminal
    OBJECT_PARA     = 19,   // lightning conductor
    OBJECT_TARGET1      = 20,   // gate target
    OBJECT_TARGET2      = 21,   // center target
    OBJECT_SAFE     = 22,   // safe
    OBJECT_HUSTON       = 23,   // control centre
    OBJECT_DESTROYER    = 24,   // destroyer
    OBJECT_FRET     = 30,   // transportable
    OBJECT_STONE        = 31,   // stone
    OBJECT_URANIUM      = 32,   // uranium
    OBJECT_METAL        = 33,   // metal
    OBJECT_POWER        = 34,   // normal battery
    OBJECT_ATOMIC           = 35,   // atomic battery
    OBJECT_BULLET           = 36,   // bullet
    OBJECT_BBOX     = 37,   // black-box
    OBJECT_TNT      = 38,   // box of TNT
    OBJECT_SCRAP1       = 40,   // metal waste
    OBJECT_SCRAP2       = 41,   // metal waste
    OBJECT_SCRAP3       = 42,   // metal waste
    OBJECT_SCRAP4       = 43,   // plastic waste
    OBJECT_SCRAP5       = 44,   // plastic waste
    OBJECT_MARKPOWER    = 50,   // mark underground energy source
    OBJECT_MARKSTONE    = 51,   // mark underground ore
    OBJECT_MARKURANIUM      = 52,   // mark underground uranium
    OBJECT_MARKKEYa     = 53,   // mark underground key
    OBJECT_MARKKEYb     = 54,   // mark underground key
    OBJECT_MARKKEYc     = 55,   // mark underground key
    OBJECT_MARKKEYd     = 56,   // mark underground key
    OBJECT_BOMB     = 60,   // bomb
    OBJECT_WINFIRE      = 61,   // fireworks
    OBJECT_SHOW     = 62,   // shows a place
    OBJECT_BAG      = 63,   // survival bag
    OBJECT_PLANT0       = 70,   // plant 0
    OBJECT_PLANT1       = 71,   // plant 1
    OBJECT_PLANT2       = 72,   // plant 2
    OBJECT_PLANT3       = 73,   // plant 3
    OBJECT_PLANT4       = 74,   // plant 4
    OBJECT_PLANT5       = 75,   // plant 5
    OBJECT_PLANT6       = 76,   // plant 6
    OBJECT_PLANT7       = 77,   // plant 7
    OBJECT_PLANT8       = 78,   // plant 8
    OBJECT_PLANT9       = 79,   // plant 9
    OBJECT_PLANT10      = 80,   // plant 10
    OBJECT_PLANT11      = 81,   // plant 11
    OBJECT_PLANT12      = 82,   // plant 12
    OBJECT_PLANT13      = 83,   // plant 13
    OBJECT_PLANT14      = 84,   // plant 14
    OBJECT_PLANT15      = 85,   // plant 15
    OBJECT_PLANT16      = 86,   // plant 16
    OBJECT_PLANT17      = 87,   // plant 17
    OBJECT_PLANT18      = 88,   // plant 18
    OBJECT_PLANT19      = 89,   // plant 19
    OBJECT_TREE0        = 90,   // tree 0
    OBJECT_TREE1        = 91,   // tree 1
    OBJECT_TREE2        = 92,   // tree 2
    OBJECT_TREE3        = 93,   // tree 3
    OBJECT_TREE4        = 94,   // tree 4
    OBJECT_TREE5        = 95,   // tree 5
    OBJECT_TREE6        = 96,   // tree 6
    OBJECT_TREE7        = 97,   // tree 7
    OBJECT_TREE8        = 98,   // tree 8
    OBJECT_TREE9        = 99,   // tree 9
    OBJECT_MOBILEwt     = 100,  // wheel-trainer
    OBJECT_MOBILEtt         = 101,  // track-trainer
    OBJECT_MOBILEft         = 102,  // fly-trainer
    OBJECT_MOBILEit         = 103,  // insect-trainer
    OBJECT_MOBILEwa         = 110,  // wheel-arm
    OBJECT_MOBILEta         = 111,  // track-arm
    OBJECT_MOBILEfa         = 112,  // fly-arm
    OBJECT_MOBILEia         = 113,  // insect-arm
    OBJECT_MOBILEwc         = 120,  // wheel-cannon
    OBJECT_MOBILEtc         = 121,  // track-cannon
    OBJECT_MOBILEfc         = 122,  // fly-cannon
    OBJECT_MOBILEic         = 123,  // insect-cannon
    OBJECT_MOBILEwi         = 130,  // wheel-insect-cannon
    OBJECT_MOBILEti         = 131,  // track-insect-cannon
    OBJECT_MOBILEfi         = 132,  // fly-insect-cannon
    OBJECT_MOBILEii         = 133,  // insect-insect-cannon
    OBJECT_MOBILEws         = 140,  // wheel-search
    OBJECT_MOBILEts         = 141,  // track-search
    OBJECT_MOBILEfs         = 142,  // fly-search
    OBJECT_MOBILEis         = 143,  // insect-search
    OBJECT_MOBILErt         = 200,  // roller-terraform
    OBJECT_MOBILErc         = 201,  // roller-canon
    OBJECT_MOBILErr         = 202,  // roller-recover
    OBJECT_MOBILErs         = 203,  // roller-shield
    OBJECT_MOBILEsa         = 210,  // submarine
    OBJECT_MOBILEtg         = 211,  // training target
    OBJECT_MOBILEdr         = 212,  // robot drawing
    OBJECT_WAYPOINT     = 250,  // waypoint
    OBJECT_FLAGb        = 260,  // blue flag
    OBJECT_FLAGr        = 261,  // red flag
    OBJECT_FLAGg        = 262,  // green flag
    OBJECT_FLAGy        = 263,  // yellow flag
    OBJECT_FLAGv        = 264,  // violet flag
    OBJECT_KEYa     = 270,  // key a
    OBJECT_KEYb     = 271,  // key b
    OBJECT_KEYc     = 272,  // key c
    OBJECT_KEYd     = 273,  // key d
    OBJECT_HUMAN            = 300,  // human
    OBJECT_TOTO     = 301,  // toto
    OBJECT_TECH     = 302,  // technician
    OBJECT_BARRIER0     = 400,  // barrier
    OBJECT_BARRIER1     = 401,  // barrier
    OBJECT_BARRIER2     = 402,  // barrier
    OBJECT_BARRIER3     = 403,  // barrier
    OBJECT_BARRIER4     = 404,  // barrier
    OBJECT_MOTHER           = 500,  // insect queen
    OBJECT_EGG      = 501,  // egg
    OBJECT_ANT      = 502,  // ant
    OBJECT_SPIDER           = 503,  // spider
    OBJECT_BEE      = 504,  // bee
    OBJECT_WORM     = 505,  // worm
    OBJECT_RUINmobilew1     = 600,  // ruin 1
    OBJECT_RUINmobilew2     = 601,  // ruin 1
    OBJECT_RUINmobilet1     = 602,  // ruin 2
    OBJECT_RUINmobilet2     = 603,  // ruin 2
    OBJECT_RUINmobiler1     = 604,  // ruin 3
    OBJECT_RUINmobiler2     = 605,  // ruin 3
    OBJECT_RUINfactory      = 606,  // ruin 4
    OBJECT_RUINdoor         = 607,  // ruin 5
    OBJECT_RUINsupport      = 608,  // ruin 6
    OBJECT_RUINradar        = 609,  // ruin 7
    OBJECT_RUINconvert      = 610,  // ruin 8
    OBJECT_RUINbase         = 611,  // ruin 9
    OBJECT_RUINhead         = 612,  // ruin 10
    OBJECT_TEEN0        = 620,  // toy
    OBJECT_TEEN1        = 621,  // toy
    OBJECT_TEEN2        = 622,  // toy
    OBJECT_TEEN3        = 623,  // toy
    OBJECT_TEEN4        = 624,  // toy
    OBJECT_TEEN5        = 625,  // toy
    OBJECT_TEEN6        = 626,  // toy
    OBJECT_TEEN7        = 627,  // toy
    OBJECT_TEEN8        = 628,  // toy
    OBJECT_TEEN9        = 629,  // toy
    OBJECT_TEEN10           = 630,  // toy
    OBJECT_TEEN11           = 631,  // toy
    OBJECT_TEEN12           = 632,  // toy
    OBJECT_TEEN13           = 633,  // toy
    OBJECT_TEEN14           = 634,  // toy
    OBJECT_TEEN15           = 635,  // toy
    OBJECT_TEEN16           = 636,  // toy
    OBJECT_TEEN17           = 637,  // toy
    OBJECT_TEEN18           = 638,  // toy
    OBJECT_TEEN19           = 639,  // toy
    OBJECT_TEEN20           = 640,  // toy
    OBJECT_TEEN21           = 641,  // toy
    OBJECT_TEEN22           = 642,  // toy
    OBJECT_TEEN23           = 643,  // toy
    OBJECT_TEEN24           = 644,  // toy
    OBJECT_TEEN25           = 645,  // toy
    OBJECT_TEEN26           = 646,  // toy
    OBJECT_TEEN27           = 647,  // toy
    OBJECT_TEEN28           = 648,  // toy
    OBJECT_TEEN29           = 649,  // toy
    OBJECT_TEEN30           = 650,  // toy
    OBJECT_TEEN31           = 651,  // toy
    OBJECT_TEEN32           = 652,  // toy
    OBJECT_TEEN33           = 653,  // toy
    OBJECT_TEEN34           = 654,  // toy
    OBJECT_TEEN35           = 655,  // toy
    OBJECT_TEEN36           = 656,  // toy
    OBJECT_TEEN37           = 657,  // toy
    OBJECT_TEEN38           = 658,  // toy
    OBJECT_TEEN39           = 659,  // toy
    OBJECT_TEEN40           = 660,  // toy
    OBJECT_TEEN41           = 661,  // toy
    OBJECT_TEEN42           = 662,  // toy
    OBJECT_TEEN43       = 663,  // toy
    OBJECT_TEEN44           = 664,  // toy
    OBJECT_TEEN45           = 665,  // toy
    OBJECT_TEEN46           = 666,  // toy
    OBJECT_TEEN47           = 667,  // toy
    OBJECT_TEEN48           = 668,  // toy
    OBJECT_TEEN49           = 669,  // toy
    OBJECT_QUARTZ0      = 700,  // crystal 0
    OBJECT_QUARTZ1      = 701,  // crystal 1
    OBJECT_QUARTZ2      = 702,  // crystal 2
    OBJECT_QUARTZ3      = 703,  // crystal 3
    OBJECT_QUARTZ4      = 704,  // crystal 4
    OBJECT_QUARTZ5      = 705,  // crystal 5
    OBJECT_QUARTZ6      = 706,  // crystal 6
    OBJECT_QUARTZ7      = 707,  // crystal 7
    OBJECT_QUARTZ8      = 708,  // crystal 8
    OBJECT_QUARTZ9      = 709,  // crystal 9
    OBJECT_ROOT0        = 710,  // root 0
    OBJECT_ROOT1        = 711,  // root 1
    OBJECT_ROOT2        = 712,  // root 2
    OBJECT_ROOT3        = 713,  // root 3
    OBJECT_ROOT4        = 714,  // root 4
    OBJECT_ROOT5        = 715,  // root 5
    OBJECT_ROOT6        = 716,  // root 6
    OBJECT_ROOT7        = 717,  // root 7
    OBJECT_ROOT8        = 718,  // root 8
    OBJECT_ROOT9        = 719,  // root 9
    OBJECT_SEAWEED0     = 720,  // seaweed 0
    OBJECT_SEAWEED1     = 721,  // seaweed 1
    OBJECT_SEAWEED2     = 722,  // seaweed 2
    OBJECT_SEAWEED3     = 723,  // seaweed 3
    OBJECT_SEAWEED4     = 724,  // seaweed 4
    OBJECT_SEAWEED5     = 725,  // seaweed 5
    OBJECT_SEAWEED6     = 726,  // seaweed 6
    OBJECT_SEAWEED7     = 727,  // seaweed 7
    OBJECT_SEAWEED8     = 728,  // seaweed 8
    OBJECT_SEAWEED9     = 729,  // seaweed 9
    OBJECT_MUSHROOM0    = 730,  // mushroom 0
    OBJECT_MUSHROOM1    = 731,  // mushroom 1
    OBJECT_MUSHROOM2    = 732,  // mushroom 2
    OBJECT_MUSHROOM3    = 733,  // mushroom 3
    OBJECT_MUSHROOM4    = 734,  // mushroom 4
    OBJECT_MUSHROOM5    = 735,  // mushroom 5
    OBJECT_MUSHROOM6    = 736,  // mushroom 6
    OBJECT_MUSHROOM7    = 737,  // mushroom 7
    OBJECT_MUSHROOM8    = 738,  // mushroom 8
    OBJECT_MUSHROOM9    = 739,  // mushroom 9
    OBJECT_APOLLO1      = 900,  // apollo lem
    OBJECT_APOLLO2      = 901,  // apollo jeep
    OBJECT_APOLLO3      = 902,  // apollo flag
    OBJECT_APOLLO4      = 903,  // apollo module
    OBJECT_APOLLO5      = 904,  // apollo antenna
    OBJECT_HOME1        = 910,  // home 1
    OBJECT_MAX      = 1000,
};

enum ObjectMaterial
{
    OM_METAL        = 0,    // metal
    OM_PLASTIC      = 1,    // plastic
    OM_HUMAN        = 2,    // cosmonaut
    OM_ANIMAL       = 3,    // insect
    OM_VEGETAL      = 4,    // plant
    OM_MINERAL      = 5,    // stone
};

typedef struct
{
    char        bUsed;
    int     object;         // number of the object in CD3DEngine
    int     parentPart;     // number of father part
    int     masterParti;        // master canal of the particle
    D3DVECTOR   position;
    D3DVECTOR   angle;
    D3DVECTOR   zoom;
    char        bTranslate;
    char        bRotate;
    char        bZoom;
    D3DMATRIX   matTranslate;
    D3DMATRIX   matRotate;
    D3DMATRIX   matTransform;
    D3DMATRIX   matWorld;
}
ObjectPart;

typedef struct
{
    float       wheelFront;     // position X of the front wheels
    float       wheelBack;      // position X of the back wheels
    float       wheelLeft;      // position Z of the left wheels
    float       wheelRight;     // position Z of the right wheels
    float       height;         // normal height on top of ground
    D3DVECTOR   posPower;       // position of the battery
}
Character;

typedef struct
{
    char        name[20];       // name of the information
    float       value;          // value of the information
}
Info;

enum ExploType
{
    EXPLO_BOUM      = 1,
    EXPLO_BURN      = 2,
    EXPLO_WATER     = 3,
};

enum ResetCap
{
    RESET_NONE      = 0,
    RESET_MOVE      = 1,
    RESET_DELETE    = 2,
};

enum RadarFilter
{
    FILTER_NONE         = 0,
    FILTER_ONLYLANDING  = 1,
    FILTER_ONLYFLYING   = 2,
};




class CObject
{
public:
    CObject(CInstanceManager* iMan);
    ~CObject();

    void        DeleteObject(BOOL bAll=FALSE);
    void        Simplify();
    BOOL        ExploObject(ExploType type, float force, float decay=1.0f);

    BOOL        EventProcess(const Event &event);
    void        UpdateMapping();

    int         CreatePart();
    void        DeletePart(int part);
    void        SetObjectRank(int part, int objRank);
    int         RetObjectRank(int part);
    void        SetObjectParent(int part, int parent);
    void        SetType(ObjectType type);
    ObjectType  RetType();
    char*       RetName();
    void        SetOption(int option);
    int         RetOption();

    void        SetID(int id);
    int         RetID();

    BOOL        Write(char *line);
    BOOL        Read(char *line);

    void        SetDrawWorld(BOOL bDraw);
    void        SetDrawFront(BOOL bDraw);

    BOOL        CreateVehicle(D3DVECTOR pos, float angle, ObjectType type, float power, BOOL bTrainer, BOOL bToy);
    BOOL        CreateInsect(D3DVECTOR pos, float angle, ObjectType type);
    BOOL        CreateBuilding(D3DVECTOR pos, float angle, float height, ObjectType type, float power=1.0f);
    BOOL        CreateResource(D3DVECTOR pos, float angle, ObjectType type, float power=1.0f);
    BOOL        CreateFlag(D3DVECTOR pos, float angle, ObjectType type);
    BOOL        CreateBarrier(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreatePlant(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreateMushroom(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreateTeen(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type);
    BOOL        CreateQuartz(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreateRoot(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreateHome(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreateRuin(D3DVECTOR pos, float angle, float height, ObjectType type);
    BOOL        CreateApollo(D3DVECTOR pos, float angle, ObjectType type);

    BOOL        ReadProgram(int rank, char* filename);
    BOOL        WriteProgram(int rank, char* filename);
    BOOL        RunProgram(int rank);

    int         RetShadowLight();
    int         RetEffectLight();

    void        FlushCrashShere();
    int         CreateCrashSphere(D3DVECTOR pos, float radius, Sound sound, float hardness=0.45f);
    int         RetCrashSphereTotal();
    BOOL        GetCrashSphere(int rank, D3DVECTOR &pos, float &radius);
    float       RetCrashSphereHardness(int rank);
    Sound       RetCrashSphereSound(int rank);
    void        DeleteCrashSphere(int rank);
    void        SetGlobalSphere(D3DVECTOR pos, float radius);
    void        GetGlobalSphere(D3DVECTOR &pos, float &radius);
    void        SetJotlerSphere(D3DVECTOR pos, float radius);
    void        GetJotlerSphere(D3DVECTOR &pos, float &radius);
    void        SetShieldRadius(float radius);
    float       RetShieldRadius();

    void        SetFloorHeight(float height);
    void        FloorAdjust();

    void        SetLinVibration(D3DVECTOR dir);
    D3DVECTOR   RetLinVibration();
    void        SetCirVibration(D3DVECTOR dir);
    D3DVECTOR   RetCirVibration();
    void        SetInclinaison(D3DVECTOR dir);
    D3DVECTOR   RetInclinaison();

    void        SetPosition(int part, const D3DVECTOR &pos);
    D3DVECTOR   RetPosition(int part);
    void        SetAngle(int part, const D3DVECTOR &angle);
    D3DVECTOR   RetAngle(int part);
    void        SetAngleY(int part, float angle);
    void        SetAngleX(int part, float angle);
    void        SetAngleZ(int part, float angle);
    float       RetAngleY(int part);
    float       RetAngleX(int part);
    float       RetAngleZ(int part);
    void        SetZoom(int part, float zoom);
    void        SetZoom(int part, D3DVECTOR zoom);
    D3DVECTOR   RetZoom(int part);
    void        SetZoomX(int part, float zoom);
    float       RetZoomX(int part);
    void        SetZoomY(int part, float zoom);
    float       RetZoomY(int part);
    void        SetZoomZ(int part, float zoom);
    float       RetZoomZ(int part);

    float       RetWaterLevel();

    void        SetTrainer(BOOL bEnable);
    BOOL        RetTrainer();

    void        SetToy(BOOL bEnable);
    BOOL        RetToy();

    void        SetManual(BOOL bManual);
    BOOL        RetManual();

    void        SetResetCap(ResetCap cap);
    ResetCap    RetResetCap();
    void        SetResetBusy(BOOL bBusy);
    BOOL        RetResetBusy();
    void        SetResetPosition(const D3DVECTOR &pos);
    D3DVECTOR   RetResetPosition();
    void        SetResetAngle(const D3DVECTOR &angle);
    D3DVECTOR   RetResetAngle();
    void        SetResetRun(int run);
    int         RetResetRun();

    void        SetMasterParticule(int part, int parti);
    int         RetMasterParticule(int part);

    void        SetPower(CObject* power);
    CObject*    RetPower();
    void        SetFret(CObject* fret);
    CObject*    RetFret();
    void        SetTruck(CObject* truck);
    CObject*    RetTruck();
    void        SetTruckPart(int part);
    int         RetTruckPart();

    void        InfoFlush();
    void        DeleteInfo(int rank);
    void        SetInfo(int rank, Info info);
    Info        RetInfo(int rank);
    int         RetInfoTotal();
    void        SetInfoReturn(float value);
    float       RetInfoReturn();
    void        SetInfoUpdate(BOOL bUpdate);
    BOOL        RetInfoUpdate();

    BOOL        SetCmdLine(int rank, float value);
    float       RetCmdLine(int rank);

    D3DMATRIX*  RetRotateMatrix(int part);
    D3DMATRIX*  RetTranslateMatrix(int part);
    D3DMATRIX*  RetTransformMatrix(int part);
    D3DMATRIX*  RetWorldMatrix(int part);

    void        SetViewFromHere(D3DVECTOR &eye, float &dirH, float &dirV, D3DVECTOR &lookat, D3DVECTOR &upVec, CameraType type);

    void        SetCharacter(Character* character);
    void        GetCharacter(Character* character);
    Character*  RetCharacter();

    float       RetAbsTime();

    void        SetEnergy(float level);
    float       RetEnergy();

    void        SetCapacity(float capacity);
    float       RetCapacity();

    void        SetShield(float level);
    float       RetShield();

    void        SetRange(float delay);
    float       RetRange();

    void        SetTransparency(float value);
    float       RetTransparency();

    ObjectMaterial RetMaterial();

    void        SetGadget(BOOL bMode);
    BOOL        RetGadget();

    void        SetFixed(BOOL bFixed);
    BOOL        RetFixed();

    void        SetClip(BOOL bClip);
    BOOL        RetClip();

    BOOL        JostleObject(float force);

    void        StartDetectEffect(CObject *target, BOOL bFound);

    void        SetVirusMode(BOOL bEnable);
    BOOL        RetVirusMode();
    float       RetVirusTime();

    void        SetCameraType(CameraType type);
    CameraType  RetCameraType();
    void        SetCameraDist(float dist);
    float       RetCameraDist();
    void        SetCameraLock(BOOL bLock);
    BOOL        RetCameraLock();

    void        SetHilite(BOOL bMode);
    BOOL        RetHilite();

    void        SetSelect(BOOL bMode, BOOL bDisplayError=TRUE);
    BOOL        RetSelect(BOOL bReal=FALSE);

    void        SetSelectable(BOOL bMode);
    BOOL        RetSelectable();

    void        SetActivity(BOOL bMode);
    BOOL        RetActivity();

    void        SetVisible(BOOL bVisible);
    BOOL        RetVisible();

    void        SetEnable(BOOL bEnable);
    BOOL        RetEnable();

    void        SetCheckToken(BOOL bMode);
    BOOL        RetCheckToken();

    void        SetProxyActivate(BOOL bActivate);
    BOOL        RetProxyActivate();
    void        SetProxyDistance(float distance);
    float       RetProxyDistance();

    void        SetMagnifyDamage(float factor);
    float       RetMagnifyDamage();

    void        SetParam(float value);
    float       RetParam();

    void        SetExplo(BOOL bExplo);
    BOOL        RetExplo();
    void        SetLock(BOOL bLock);
    BOOL        RetLock();
    void        SetCargo(BOOL bCargo);
    BOOL        RetCargo();
    void        SetBurn(BOOL bBurn);
    BOOL        RetBurn();
    void        SetDead(BOOL bDead);
    BOOL        RetDead();
    BOOL        RetRuin();
    BOOL        RetActif();

    void        SetGunGoalV(float gunGoal);
    void        SetGunGoalH(float gunGoal);
    float       RetGunGoalV();
    float       RetGunGoalH();

    BOOL        StartShowLimit();
    void        StopShowLimit();

    BOOL        IsProgram();
    void        CreateSelectParticule();

    void        SetRunScript(CScript* script);
    CScript*    RetRunScript();
    CBotVar*    RetBotVar();
    CPhysics*   RetPhysics();
    CBrain*     RetBrain();
    CMotion*    RetMotion();
    CAuto*      RetAuto();
    void        SetAuto(CAuto* automat);

    void        SetDefRank(int rank);
    int         RetDefRank();

    BOOL        GetTooltipName(char* name);

    void        AddDeselList(CObject* pObj);
    CObject*    SubDeselList();
    void        DeleteDeselList(CObject* pObj);

    BOOL        CreateShadowCircle(float radius, float intensity, D3DShadowType type=D3DSHADOWNORM);
    BOOL        CreateShadowLight(float height, D3DCOLORVALUE color);
    BOOL        CreateEffectLight(float height, D3DCOLORVALUE color);

    void        FlatParent();

    BOOL        RetTraceDown();
    void        SetTraceDown(BOOL bDown);
    int         RetTraceColor();
    void        SetTraceColor(int color);
    float       RetTraceWidth();
    void        SetTraceWidth(float width);

protected:
    BOOL        EventFrame(const Event &event);
    void        VirusFrame(float rTime);
    void        PartiFrame(float rTime);
    void        CreateOtherObject(ObjectType type);
    void        InitPart(int part);
    void        UpdateTotalPart();
    int         SearchDescendant(int parent, int n);
    void        UpdateEnergyMapping();
    BOOL        UpdateTransformObject(int part, BOOL bForceUpdate);
    BOOL        UpdateTransformObject();
    void        UpdateSelectParticule();

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CLight*         m_light;
    CTerrain*       m_terrain;
    CWater*         m_water;
    CCamera*        m_camera;
    CParticule*     m_particule;
    CPhysics*       m_physics;
    CBrain*         m_brain;
    CMotion*        m_motion;
    CAuto*          m_auto;
    CDisplayText*   m_displayText;
    CRobotMain*     m_main;
    CSound*         m_sound;
    CBotVar*        m_botVar;
    CScript*        m_runScript;

    ObjectType  m_type;             // OBJECT_*
    int     m_id;               // unique identifier
    char        m_name[50];         // name of the object
    Character   m_character;            // characteristic
    int     m_option;           // option
    int     m_partiReactor;         // number of the particle of the reactor
    int     m_shadowLight;          // number of light from the shadows
    float       m_shadowHeight;         // height of light from the shadows
    int     m_effectLight;          // number of light effects
    float       m_effectHeight;         // height of light effects
    D3DVECTOR   m_linVibration;         // linear vibration
    D3DVECTOR   m_cirVibration;         // circular vibration
    D3DVECTOR   m_inclinaison;          // tilt
    CObject*    m_power;            // battery used by the vehicle
    CObject*    m_fret;             // object transported
    CObject*    m_truck;            // object with the latter
    int     m_truckLink;            // part
    float       m_energy;           // energy contained (if battery)
    float       m_lastEnergy;
    float       m_capacity;         // capacity (if battery)
    float       m_shield;           // shield
    float       m_range;            // flight range
    float       m_transparency;         // transparency (0..1)
    int     m_material;         // matter(0..n)
    float       m_aTime;
    float       m_shotTime;         // time since last shot
    BOOL        m_bVirusMode;           // virus activated/triggered
    float       m_virusTime;            // lifetime of the virus
    float       m_lastVirusParticule;
    float       m_lastParticule;
    BOOL        m_bHilite;
    BOOL        m_bSelect;          // object selected
    BOOL        m_bSelectable;          // selectable object
    BOOL        m_bCheckToken;          // object with audited tokens
    BOOL        m_bVisible;         // object active but undetectable
    BOOL        m_bEnable;          // dead object
    BOOL        m_bProxyActivate;       // active object so close
    BOOL        m_bGadget;          // object nonessential
    BOOL        m_bLock;
    BOOL        m_bExplo;
    BOOL        m_bCargo;
    BOOL        m_bBurn;
    BOOL        m_bDead;
    BOOL        m_bFlat;
    BOOL        m_bTrainer;         // drive vehicle (without remote)
    BOOL        m_bToy;             // toy key
    BOOL        m_bManual;          // manual control (Scribbler)
    BOOL        m_bFixed;
    BOOL        m_bClip;
    BOOL        m_bShowLimit;
    float       m_showLimitRadius;
    float       m_gunGoalV;
    float       m_gunGoalH;
    CameraType  m_cameraType;
    float       m_cameraDist;
    BOOL        m_bCameraLock;
    int         m_defRank;
    float       m_magnifyDamage;
    float       m_proxyDistance;
    float       m_param;

    int         m_crashSphereUsed;  // number of spheres used
    D3DVECTOR   m_crashSpherePos[MAXCRASHSPHERE];
    float       m_crashSphereRadius[MAXCRASHSPHERE];
    float       m_crashSphereHardness[MAXCRASHSPHERE];
    Sound       m_crashSphereSound[MAXCRASHSPHERE];
    D3DVECTOR   m_globalSpherePos;
    float       m_globalSphereRadius;
    D3DVECTOR   m_jotlerSpherePos;
    float       m_jotlerSphereRadius;
    float       m_shieldRadius;

    int         m_totalPart;
    ObjectPart  m_objectPart[OBJECTMAXPART];

    int         m_totalDesectList;
    CObject*    m_objectDeselectList[OBJECTMAXDESELLIST];

    int         m_partiSel[4];

    ResetCap    m_resetCap;
    BOOL        m_bResetBusy;
    D3DVECTOR   m_resetPosition;
    D3DVECTOR   m_resetAngle;
    int         m_resetRun;

    int         m_infoTotal;
    Info        m_info[OBJECTMAXINFO];
    float       m_infoReturn;
    BOOL        m_bInfoUpdate;

    float       m_cmdLine[OBJECTMAXCMDLINE];
};


#endif //_OBJECT_H_
