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

/**
 * \file object/object.h
 * \brief CObject - base class for all game objects
 */

#pragma once


#include "graphics/engine/engine.h"
#include "graphics/engine/camera.h"

#include "sound/sound.h"


class CApplication;
class CPhysics;
class CBrain;
class CMotion;
class CAuto;
class CDisplayText;
class CRobotMain;
class CBotVar;
class CScript;


/**
 * \enum ObjectType
 * \brief Type of game object
 */
enum ObjectType
{
    OBJECT_NULL             = 0,    //! < object destroyed
    OBJECT_FIX              = 1,    //! < stationary scenery
    OBJECT_PORTICO          = 2,    //! < Portico
    OBJECT_BASE             = 3,    //! < SpaceShip
    OBJECT_DERRICK          = 4,    //! < Derrick
    OBJECT_FACTORY          = 5,    //! < BotFactory
    OBJECT_STATION          = 6,    //! < PowerStation
    OBJECT_CONVERT          = 7,    //! < Converter
    OBJECT_REPAIR           = 8,    //! < RepairStation
    OBJECT_TOWER            = 9,    //! < DefenseTower
    OBJECT_NEST             = 10,   //! < AlienNest
    OBJECT_RESEARCH         = 11,   //! < ResearchCenter
    OBJECT_RADAR            = 12,   //! < RadarStation
    OBJECT_ENERGY           = 13,   //! < PowerPlant
    OBJECT_LABO             = 14,   //! < AutoLab
    OBJECT_NUCLEAR          = 15,   //! < NuclearPlant
    OBJECT_START            = 16,   //! < StartArea
    OBJECT_END              = 17,   //! < EndArea
    OBJECT_INFO             = 18,   //! < ExchangePost
    OBJECT_PARA             = 19,   //! < PowerCaptor
    OBJECT_TARGET1          = 20,   //! < Target1 (gate)
    OBJECT_TARGET2          = 21,   //! < Target2 (center)
    OBJECT_SAFE             = 22,   //! < Vault
    OBJECT_HUSTON           = 23,   //! < Houston
    OBJECT_DESTROYER        = 24,   //! < Destroyer
    OBJECT_FRET             = 30,   //! < transportable (unused)
    OBJECT_STONE            = 31,   //! < TitaniumOre
    OBJECT_URANIUM          = 32,   //! < UraniumOre
    OBJECT_METAL            = 33,   //! < Titanium
    OBJECT_POWER            = 34,   //! < PowerCell
    OBJECT_ATOMIC           = 35,   //! < NuclearCell
    OBJECT_BULLET           = 36,   //! < OrgaMatter
    OBJECT_BBOX             = 37,   //! < BlackBox
    OBJECT_TNT              = 38,   //! < TNT
    OBJECT_SCRAP1           = 40,   //! < Scrap1 (metal)
    OBJECT_SCRAP2           = 41,   //! < Scrap2 (metal)
    OBJECT_SCRAP3           = 42,   //! < Scrap3 (metal)
    OBJECT_SCRAP4           = 43,   //! < Scrap4 (plastic)
    OBJECT_SCRAP5           = 44,   //! < Scrap5 (plastic)
    OBJECT_MARKPOWER        = 50,   //! < PowerSpot
    OBJECT_MARKSTONE        = 51,   //! < TitaniumSpot
    OBJECT_MARKURANIUM      = 52,   //! < UraniumSpot
    OBJECT_MARKKEYa         = 53,   //! < KeyASpot
    OBJECT_MARKKEYb         = 54,   //! < KeyBSpot
    OBJECT_MARKKEYc         = 55,   //! < KeyCSpot
    OBJECT_MARKKEYd         = 56,   //! < KeyDSpot
    OBJECT_BOMB             = 60,   //! < Mine
    OBJECT_WINFIRE          = 61,   //! < Firework
    OBJECT_SHOW             = 62,   //! < arrow above object (Visit)
    OBJECT_BAG              = 63,   //! < Bag
    OBJECT_PLANT0           = 70,   //! < Greenery0
    OBJECT_PLANT1           = 71,   //! < Greenery1
    OBJECT_PLANT2           = 72,   //! < Greenery2
    OBJECT_PLANT3           = 73,   //! < Greenery3
    OBJECT_PLANT4           = 74,   //! < Greenery4
    OBJECT_PLANT5           = 75,   //! < Greenery5
    OBJECT_PLANT6           = 76,   //! < Greenery6
    OBJECT_PLANT7           = 77,   //! < Greenery7
    OBJECT_PLANT8           = 78,   //! < Greenery8
    OBJECT_PLANT9           = 79,   //! < Greenery9
    OBJECT_PLANT10          = 80,   //! < Greenery10
    OBJECT_PLANT11          = 81,   //! < Greenery11
    OBJECT_PLANT12          = 82,   //! < Greenery12
    OBJECT_PLANT13          = 83,   //! < Greenery13
    OBJECT_PLANT14          = 84,   //! < Greenery14
    OBJECT_PLANT15          = 85,   //! < Greenery15
    OBJECT_PLANT16          = 86,   //! < Greenery16
    OBJECT_PLANT17          = 87,   //! < Greenery17
    OBJECT_PLANT18          = 88,   //! < Greenery18
    OBJECT_PLANT19          = 89,   //! < Greenery19
    OBJECT_TREE0            = 90,   //! < Tree0
    OBJECT_TREE1            = 91,   //! < Tree1
    OBJECT_TREE2            = 92,   //! < Tree2
    OBJECT_TREE3            = 93,   //! < Tree3
    OBJECT_TREE4            = 94,   //! < Tree4
    OBJECT_TREE5            = 95,   //! < Tree5
    OBJECT_MOBILEwt         = 100,  //! < PracticeBot
    OBJECT_MOBILEtt         = 101,  //! < track-trainer (unused)
    OBJECT_MOBILEft         = 102,  //! < fly-trainer (unused)
    OBJECT_MOBILEit         = 103,  //! < insect-trainer (unused)
    OBJECT_MOBILEwa         = 110,  //! < WheeledGrabber
    OBJECT_MOBILEta         = 111,  //! < TrackedGrabber
    OBJECT_MOBILEfa         = 112,  //! < WingedGrabber
    OBJECT_MOBILEia         = 113,  //! < LeggedGrabber
    OBJECT_MOBILEwc         = 120,  //! < WheeledShooter
    OBJECT_MOBILEtc         = 121,  //! < TrackedShooter
    OBJECT_MOBILEfc         = 122,  //! < WingedShooter
    OBJECT_MOBILEic         = 123,  //! < LeggedShooter
    OBJECT_MOBILEwi         = 130,  //! < WheeledOrgaShooter
    OBJECT_MOBILEti         = 131,  //! < TrackedOrgaShooter
    OBJECT_MOBILEfi         = 132,  //! < WingedOrgaShooter
    OBJECT_MOBILEii         = 133,  //! < LeggedOrgaShooter
    OBJECT_MOBILEws         = 140,  //! < WheeledSniffer
    OBJECT_MOBILEts         = 141,  //! < TrackedSniffer
    OBJECT_MOBILEfs         = 142,  //! < WingedSniffer
    OBJECT_MOBILEis         = 143,  //! < LeggedSniffer
    OBJECT_MOBILErt         = 200,  //! < Thumper
    OBJECT_MOBILErc         = 201,  //! < PhazerShooter
    OBJECT_MOBILErr         = 202,  //! < Recycler
    OBJECT_MOBILErs         = 203,  //! < Shielder
    OBJECT_MOBILEsa         = 210,  //! < Subber
    OBJECT_MOBILEtg         = 211,  //! < TargetBot
    OBJECT_MOBILEdr         = 212,  //! < Scribbler
    OBJECT_CONTROLLER       = 213,  //! < MissionController
    OBJECT_WAYPOINT         = 250,  //! < WayPoint
    OBJECT_FLAGb            = 260,  //! < BlueFlag
    OBJECT_FLAGr            = 261,  //! < RedFlag
    OBJECT_FLAGg            = 262,  //! < GreenFlag
    OBJECT_FLAGy            = 263,  //! < YellowFlag
    OBJECT_FLAGv            = 264,  //! < VioletFlag
    OBJECT_KEYa             = 270,  //! < KeyA
    OBJECT_KEYb             = 271,  //! < KeyB
    OBJECT_KEYc             = 272,  //! < KeyC
    OBJECT_KEYd             = 273,  //! < KeyD
    OBJECT_HUMAN            = 300,  //! < Me
    OBJECT_TOTO             = 301,  //! < Robby (toto)
    OBJECT_TECH             = 302,  //! < Tech
    OBJECT_BARRIER0         = 400,  //! < Barrier0
    OBJECT_BARRIER1         = 401,  //! < Barrier1
    OBJECT_BARRIER2         = 402,  //! < Barrier2
    OBJECT_BARRIER3         = 403,  //! < Barrier3
    OBJECT_MOTHER           = 500,  //! < AlienQueen
    OBJECT_EGG              = 501,  //! < AlienEgg
    OBJECT_ANT              = 502,  //! < AlienAnt
    OBJECT_SPIDER           = 503,  //! < AlienSpider
    OBJECT_BEE              = 504,  //! < AlienWasp
    OBJECT_WORM             = 505,  //! < AlienWorm
    OBJECT_RUINmobilew1     = 600,  //! < WreckBotw1
    OBJECT_RUINmobilew2     = 601,  //! < WreckBotw2
    OBJECT_RUINmobilet1     = 602,  //! < WreckBott1
    OBJECT_RUINmobilet2     = 603,  //! < WreckBott2
    OBJECT_RUINmobiler1     = 604,  //! < WreckBotr1
    OBJECT_RUINmobiler2     = 605,  //! < WreckBotr2
    OBJECT_RUINfactory      = 606,  //! < RuinBotFactory
    OBJECT_RUINdoor         = 607,  //! < RuinDoor
    OBJECT_RUINsupport      = 608,  //! < RuinSupport
    OBJECT_RUINradar        = 609,  //! < RuinRadar
    OBJECT_RUINconvert      = 610,  //! < RuinConvert
    OBJECT_RUINbase         = 611,  //! < RuinBaseCamp
    OBJECT_RUINhead         = 612,  //! < RuinHeadCamp
    OBJECT_TEEN0            = 620,  //! < Teen0
    OBJECT_TEEN1            = 621,  //! < Teen1
    OBJECT_TEEN2            = 622,  //! < Teen2
    OBJECT_TEEN3            = 623,  //! < Teen3
    OBJECT_TEEN4            = 624,  //! < Teen4
    OBJECT_TEEN5            = 625,  //! < Teen5
    OBJECT_TEEN6            = 626,  //! < Teen6
    OBJECT_TEEN7            = 627,  //! < Teen7
    OBJECT_TEEN8            = 628,  //! < Teen8
    OBJECT_TEEN9            = 629,  //! < Teen9
    OBJECT_TEEN10           = 630,  //! < Teen10
    OBJECT_TEEN11           = 631,  //! < Teen11
    OBJECT_TEEN12           = 632,  //! < Teen12
    OBJECT_TEEN13           = 633,  //! < Teen13
    OBJECT_TEEN14           = 634,  //! < Teen14
    OBJECT_TEEN15           = 635,  //! < Teen15
    OBJECT_TEEN16           = 636,  //! < Teen16
    OBJECT_TEEN17           = 637,  //! < Teen17
    OBJECT_TEEN18           = 638,  //! < Teen18
    OBJECT_TEEN19           = 639,  //! < Teen19
    OBJECT_TEEN20           = 640,  //! < Teen20
    OBJECT_TEEN21           = 641,  //! < Teen21
    OBJECT_TEEN22           = 642,  //! < Teen22
    OBJECT_TEEN23           = 643,  //! < Teen23
    OBJECT_TEEN24           = 644,  //! < Teen24
    OBJECT_TEEN25           = 645,  //! < Teen25
    OBJECT_TEEN26           = 646,  //! < Teen26
    OBJECT_TEEN27           = 647,  //! < Teen27
    OBJECT_TEEN28           = 648,  //! < Teen28
    OBJECT_TEEN29           = 649,  //! < Teen29
    OBJECT_TEEN30           = 650,  //! < Teen30
    OBJECT_TEEN31           = 651,  //! < Teen31
    OBJECT_TEEN32           = 652,  //! < Teen32
    OBJECT_TEEN33           = 653,  //! < Teen33
    OBJECT_TEEN34           = 654,  //! < Stone (Teen34)
    OBJECT_TEEN35           = 655,  //! < Teen35
    OBJECT_TEEN36           = 656,  //! < Teen36
    OBJECT_TEEN37           = 657,  //! < Teen37
    OBJECT_TEEN38           = 658,  //! < Teen38
    OBJECT_TEEN39           = 659,  //! < Teen39
    OBJECT_TEEN40           = 660,  //! < Teen40
    OBJECT_TEEN41           = 661,  //! < Teen41
    OBJECT_TEEN42           = 662,  //! < Teen42
    OBJECT_TEEN43           = 663,  //! < Teen43
    OBJECT_TEEN44           = 664,  //! < Teen44
    OBJECT_QUARTZ0          = 700,  //! < Quartz0
    OBJECT_QUARTZ1          = 701,  //! < Quartz1
    OBJECT_QUARTZ2          = 702,  //! < Quartz2
    OBJECT_QUARTZ3          = 703,  //! < Quartz3
    OBJECT_ROOT0            = 710,  //! < MegaStalk0
    OBJECT_ROOT1            = 711,  //! < MegaStalk1
    OBJECT_ROOT2            = 712,  //! < MegaStalk2
    OBJECT_ROOT3            = 713,  //! < MegaStalk3
    OBJECT_ROOT4            = 714,  //! < MegaStalk4
    OBJECT_ROOT5            = 715,  //! < MegaStalk5
    OBJECT_MUSHROOM1        = 731,  //! < Mushroom1
    OBJECT_MUSHROOM2        = 732,  //! < Mushroom2
    OBJECT_APOLLO1          = 900,  //! < ApolloLEM
    OBJECT_APOLLO2          = 901,  //! < ApolloJeep
    OBJECT_APOLLO3          = 902,  //! < ApolloFlag
    OBJECT_APOLLO4          = 903,  //! < ApolloModule
    OBJECT_APOLLO5          = 904,  //! < ApolloAntenna
    OBJECT_HOME1            = 910,  //! < Home

    OBJECT_MAX              = 1000  //! < number of values
};



// The father of all parts must always be the part number zero!

const int OBJECTMAXPART         = 40;
const int MAXCRASHSPHERE        = 40;
const int OBJECTMAXDESELLIST    = 10;
const int OBJECTMAXINFO         = 10;
const int OBJECTMAXCMDLINE      = 20;


enum ObjectMaterial
{
    OM_METAL   = 0,    // metal
    OM_PLASTIC = 1,    // plastic
    OM_HUMAN   = 2,    // cosmonaut
    OM_ANIMAL  = 3,    // insect
    OM_VEGETAL = 4,    // plant
    OM_MINERAL = 5,    // stone
};

enum DriveType
{
    DRIVE_OTHER = 0,
    DRIVE_WHEELED,
    DRIVE_TRACKED,
    DRIVE_WINGED,
    DRIVE_LEGGED,
};

enum ToolType
{
    TOOL_OTHER = 0,
    TOOL_GRABBER,
    TOOL_SNIFFER,
    TOOL_SHOOTER,
    TOOL_ORGASHOOTER,
};

struct ObjectPart
{
    char         bUsed;
    int          object;         // number of the object in CEngine
    int          parentPart;     // number of father part
    int          masterParti;        // master canal of the particle
    Math::Vector position;
    Math::Vector angle;
    Math::Vector zoom;
    char         bTranslate;
    char         bRotate;
    char         bZoom;
    Math::Matrix matTranslate;
    Math::Matrix matRotate;
    Math::Matrix matTransform;
    Math::Matrix matWorld;
};

struct Character
{
    float        wheelFront;     // position X of the front wheels
    float        wheelBack;      // position X of the back wheels
    float        wheelLeft;      // position Z of the left wheels
    float        wheelRight;     // position Z of the right wheels
    float        height;         // normal height on top of ground
    Math::Vector posPower;       // position of the battery
};

struct Info
{
    char  name[20];       // name of the information
    float value;          // value of the information
};

enum ExploType
{
    EXPLO_BOUM  = 1,
    EXPLO_BURN  = 2,
    EXPLO_WATER = 3,
};

enum ResetCap
{
    RESET_NONE   = 0,
    RESET_MOVE   = 1,
    RESET_DELETE = 2,
};

enum RadarFilter
{
    FILTER_NONE        = 0,
    FILTER_ONLYLANDING = 1,
    FILTER_ONLYFLYING  = 2,
};




class CObject
{
public:
    CObject();
    ~CObject();

    void        DeleteObject(bool bAll=false);
    void        Simplify();
    bool        ExploObject(ExploType type, float force, float decay=1.0f);

    bool        EventProcess(const Event &event);
    void        UpdateMapping();

    int         CreatePart();
    void        DeletePart(int part);
    void        SetObjectRank(int part, int objRank);
    int         GetObjectRank(int part);
    void        SetObjectParent(int part, int parent);
    void        SetType(ObjectType type);
    ObjectType  GetType();
    char*       GetName();
    void        SetOption(int option);
    int         GetOption();

    void        SetID(int id);
    int         GetID();

    bool        Write(char *line);
    bool        Read(char *line);

    void        SetDrawWorld(bool bDraw);
    void        SetDrawFront(bool bDraw);

    bool        CreateVehicle(Math::Vector pos, float angle, ObjectType type, float power, bool bTrainer, bool bToy);
    bool        CreateInsect(Math::Vector pos, float angle, ObjectType type);
    bool        CreateBuilding(Math::Vector pos, float angle, float height, ObjectType type, float power=1.0f);
    bool        CreateResource(Math::Vector pos, float angle, ObjectType type, float power=1.0f);
    bool        CreateFlag(Math::Vector pos, float angle, ObjectType type);
    bool        CreateBarrier(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreatePlant(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreateMushroom(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreateTeen(Math::Vector pos, float angle, float zoom, float height, ObjectType type);
    bool        CreateQuartz(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreateRoot(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreateHome(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreateRuin(Math::Vector pos, float angle, float height, ObjectType type);
    bool        CreateApollo(Math::Vector pos, float angle, ObjectType type);

    bool        ReadProgram(int rank, const char* filename);
    bool        WriteProgram(int rank, char* filename);
    bool        RunProgram(int rank);

    int         GetShadowLight();
    int         GetEffectLight();

    void        FlushCrashShere();
    int         CreateCrashSphere(Math::Vector pos, float radius, Sound sound, float hardness=0.45f);
    int         GetCrashSphereTotal();
    bool        GetCrashSphere(int rank, Math::Vector &pos, float &radius);
    float       GetCrashSphereHardness(int rank);
    Sound       GetCrashSphereSound(int rank);
    void        DeleteCrashSphere(int rank);
    void        SetGlobalSphere(Math::Vector pos, float radius);
    void        GetGlobalSphere(Math::Vector &pos, float &radius);
    void        SetJotlerSphere(Math::Vector pos, float radius);
    void        GetJotlerSphere(Math::Vector &pos, float &radius);
    void        SetShieldRadius(float radius);
    float       GetShieldRadius();

    void        SetFloorHeight(float height);
    void        FloorAdjust();

    void        SetLinVibration(Math::Vector dir);
    Math::Vector    GetLinVibration();
    void        SetCirVibration(Math::Vector dir);
    Math::Vector    GetCirVibration();
    void        SetInclinaison(Math::Vector dir);
    Math::Vector    GetInclinaison();

    void        SetPosition(int part, const Math::Vector &pos);
    Math::Vector    GetPosition(int part);
    void        SetAngle(int part, const Math::Vector &angle);
    Math::Vector    GetAngle(int part);
    void        SetAngleY(int part, float angle);
    void        SetAngleX(int part, float angle);
    void        SetAngleZ(int part, float angle);
    float       GetAngleY(int part);
    float       GetAngleX(int part);
    float       GetAngleZ(int part);
    void        SetZoom(int part, float zoom);
    void        SetZoom(int part, Math::Vector zoom);
    Math::Vector    GetZoom(int part);
    void        SetZoomX(int part, float zoom);
    float       GetZoomX(int part);
    void        SetZoomY(int part, float zoom);
    float       GetZoomY(int part);
    void        SetZoomZ(int part, float zoom);
    float       GetZoomZ(int part);

    float       GetWaterLevel();

    void        SetTrainer(bool bEnable);
    bool        GetTrainer();

    void        SetToy(bool bEnable);
    bool        GetToy();

    void        SetManual(bool bManual);
    bool        GetManual();

    void        SetResetCap(ResetCap cap);
    ResetCap    GetResetCap();
    void        SetResetBusy(bool bBusy);
    bool        GetResetBusy();
    void        SetResetPosition(const Math::Vector &pos);
    Math::Vector    GetResetPosition();
    void        SetResetAngle(const Math::Vector &angle);
    Math::Vector    GetResetAngle();
    void        SetResetRun(int run);
    int         GetResetRun();

    void        SetMasterParticle(int part, int parti);
    int         GetMasterParticle(int part);

    void        SetPower(CObject* power);
    CObject*    GetPower();
    void        SetFret(CObject* fret);
    CObject*    GetFret();
    void        SetTruck(CObject* truck);
    CObject*    GetTruck();
    void        SetTruckPart(int part);
    int         GetTruckPart();

    void        InfoFlush();
    void        DeleteInfo(int rank);
    void        SetInfo(int rank, Info info);
    Info        GetInfo(int rank);
    int         GetInfoTotal();
    void        SetInfoReturn(float value);
    float       GetInfoReturn();
    void        SetInfoUpdate(bool bUpdate);
    bool        GetInfoUpdate();

    bool        SetCmdLine(int rank, float value);
    float       GetCmdLine(int rank);

    Math::Matrix*   GetRotateMatrix(int part);
    Math::Matrix*   GetTranslateMatrix(int part);
    Math::Matrix*   GetTransformMatrix(int part);
    Math::Matrix*   GetWorldMatrix(int part);

    void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type);

    void        SetCharacter(Character* character);
    void        GetCharacter(Character* character);
    Character*  GetCharacter();

    float       GetAbsTime();

    void        SetEnergy(float level);
    float       GetEnergy();

    void        SetCapacity(float capacity);
    float       GetCapacity();

    void        SetShield(float level);
    float       GetShield();

    void        SetRange(float delay);
    float       GetRange();

    void        SetTransparency(float value);
    float       GetTransparency();

    ObjectMaterial GetMaterial();

    void        SetGadget(bool bMode);
    bool        GetGadget();

    void        SetFixed(bool bFixed);
    bool        GetFixed();

    void        SetClip(bool bClip);
    bool        GetClip();

    bool        JostleObject(float force);

    void        StartDetectEffect(CObject *target, bool bFound);

    void        SetVirusMode(bool bEnable);
    bool        GetVirusMode();
    float       GetVirusTime();

    void        SetCameraType(Gfx::CameraType type);
    Gfx::CameraType  GetCameraType();
    void        SetCameraDist(float dist);
    float       GetCameraDist();
    void        SetCameraLock(bool bLock);
    bool        GetCameraLock();

    void        SetHilite(bool bMode);
    bool        GetHilite();

    void        SetSelect(bool bMode, bool bDisplayError=true);
    bool        GetSelect(bool bReal=false);

    void        SetSelectable(bool bMode);
    bool        GetSelectable();

    void        SetActivity(bool bMode);
    bool        GetActivity();

    void        SetVisible(bool bVisible);
    bool        GetVisible();

    void        SetEnable(bool bEnable);
    bool        GetEnable();

    void        SetCheckToken(bool bMode);
    bool        GetCheckToken();

    void        SetProxyActivate(bool bActivate);
    bool        GetProxyActivate();
    void        SetProxyDistance(float distance);
    float       GetProxyDistance();

    void        SetMagnifyDamage(float factor);
    float       GetMagnifyDamage();

    void        SetParam(float value);
    float       GetParam();
    
    void        SetIgnoreBuildCheck(bool bIgnoreBuildCheck);
    bool        GetIgnoreBuildCheck();

    void        SetExplo(bool bExplo);
    bool        GetExplo();
    void        SetLock(bool bLock);
    bool        GetLock();
    void        SetCargo(bool bCargo);
    bool        GetCargo();
    void        SetBurn(bool bBurn);
    bool        GetBurn();
    void        SetDead(bool bDead);
    bool        GetDead();
    bool        GetRuin();
    bool        GetActif();

    void        SetGunGoalV(float gunGoal);
    void        SetGunGoalH(float gunGoal);
    float       GetGunGoalV();
    float       GetGunGoalH();

    bool        StartShowLimit();
    void        StopShowLimit();

    bool        IsProgram();
    void        CreateSelectParticle();

    void        SetRunScript(CScript* script);
    CScript*    GetRunScript();
    CBotVar*    GetBotVar();
    CPhysics*   GetPhysics();
    CBrain*     GetBrain();
    CMotion*    GetMotion();
    CAuto*      GetAuto();
    void        SetAuto(CAuto* automat);

    void        SetDefRank(int rank);
    int         GetDefRank();

    bool        GetTooltipName(std::string& name);

    void        AddDeselList(CObject* pObj);
    CObject*    SubDeselList();
    void        DeleteDeselList(CObject* pObj);

    bool        CreateShadowCircle(float radius, float intensity, Gfx::EngineShadowType type = Gfx::ENG_SHADOW_NORM);
    bool        CreateShadowLight(float height, Gfx::Color color);
    bool        CreateEffectLight(float height, Gfx::Color color);

    void        FlatParent();

    bool        GetTraceDown();
    void        SetTraceDown(bool bDown);
    int         GetTraceColor();
    void        SetTraceColor(int color);
    float       GetTraceWidth();
    void        SetTraceWidth(float width);

    std::string GetModelDirName();
    
    static DriveType GetDriveFromObject(ObjectType type);
    static ToolType  GetToolFromObject(ObjectType type);

protected:
    bool        EventFrame(const Event &event);
    void        VirusFrame(float rTime);
    void        PartiFrame(float rTime);
    void        CreateOtherObject(ObjectType type);
    void        InitPart(int part);
    void        UpdateTotalPart();
    int         SearchDescendant(int parent, int n);
    void        UpdateEnergyMapping();
    bool        UpdateTransformObject(int part, bool bForceUpdate);
    bool        UpdateTransformObject();
    void        UpdateSelectParticle();

protected:
    CApplication*       m_app;
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    Gfx::CParticle*     m_particle;
    CPhysics*           m_physics;
    CBrain*             m_brain;
    CMotion*            m_motion;
    CAuto*              m_auto;
    CRobotMain*         m_main;
    CSoundInterface*    m_sound;
    CBotVar*            m_botVar;
    CScript*            m_runScript;

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
    Math::Vector    m_linVibration;         // linear vibration
    Math::Vector    m_cirVibration;         // circular vibration
    Math::Vector    m_inclinaison;          // tilt
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
    bool        m_bVirusMode;           // virus activated/triggered
    float       m_virusTime;            // lifetime of the virus
    float       m_lastVirusParticle;
    float       m_lastParticle;
    bool        m_bHilite;
    bool        m_bSelect;          // object selected
    bool        m_bSelectable;          // selectable object
    bool        m_bCheckToken;          // object with audited tokens
    bool        m_bVisible;         // object active but undetectable
    bool        m_bEnable;          // dead object
    bool        m_bProxyActivate;       // active object so close
    bool        m_bGadget;          // object nonessential
    bool        m_bLock;
    bool        m_bExplo;
    bool        m_bCargo;
    bool        m_bBurn;
    bool        m_bDead;
    bool        m_bFlat;
    bool        m_bTrainer;         // drive vehicle (without remote)
    bool        m_bToy;             // toy key
    bool        m_bManual;          // manual control (Scribbler)
    bool        m_bIgnoreBuildCheck;
    bool        m_bFixed;
    bool        m_bClip;
    bool        m_bShowLimit;
    float       m_showLimitRadius;
    float       m_gunGoalV;
    float       m_gunGoalH;
    Gfx::CameraType  m_cameraType;
    float       m_cameraDist;
    bool        m_bCameraLock;
    int         m_defRank;
    float       m_magnifyDamage;
    float       m_proxyDistance;
    float       m_param;

    int         m_crashSphereUsed;  // number of spheres used
    Math::Vector    m_crashSpherePos[MAXCRASHSPHERE];
    float       m_crashSphereRadius[MAXCRASHSPHERE];
    float       m_crashSphereHardness[MAXCRASHSPHERE];
    Sound       m_crashSphereSound[MAXCRASHSPHERE];
    Math::Vector    m_globalSpherePos;
    float       m_globalSphereRadius;
    Math::Vector    m_jotlerSpherePos;
    float       m_jotlerSphereRadius;
    float       m_shieldRadius;

    int         m_totalPart;
    ObjectPart  m_objectPart[OBJECTMAXPART];

    int         m_totalDesectList;
    CObject*    m_objectDeselectList[OBJECTMAXDESELLIST];

    int         m_partiSel[4];

    ResetCap    m_resetCap;
    bool        m_bResetBusy;
    Math::Vector    m_resetPosition;
    Math::Vector    m_resetAngle;
    int         m_resetRun;

    int         m_infoTotal;
    Info        m_info[OBJECTMAXINFO];
    float       m_infoReturn;
    bool        m_bInfoUpdate;

    float       m_cmdLine[OBJECTMAXCMDLINE];
};

