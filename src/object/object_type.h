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
 * \file object/object_type.h
 * \brief ObjectType enum
 */

#pragma once

#include <functional>

/**
 * \enum ObjectType
 * \brief Type of game object
 */
enum ObjectType
{
    OBJECT_NULL             = 0,
    OBJECT_PORTICO          = 2,    //!< Portico
    OBJECT_BASE             = 3,    //!< SpaceShip
    OBJECT_DERRICK          = 4,    //!< Derrick
    OBJECT_FACTORY          = 5,    //!< BotFactory
    OBJECT_STATION          = 6,    //!< PowerStation
    OBJECT_CONVERT          = 7,    //!< Converter
    OBJECT_REPAIR           = 8,    //!< RepairStation
    OBJECT_TOWER            = 9,    //!< DefenseTower
    OBJECT_NEST             = 10,   //!< AlienNest
    OBJECT_RESEARCH         = 11,   //!< ResearchCenter
    OBJECT_RADAR            = 12,   //!< RadarStation
    OBJECT_ENERGY           = 13,   //!< PowerPlant
    OBJECT_LABO             = 14,   //!< AutoLab
    OBJECT_NUCLEAR          = 15,   //!< NuclearPlant
    OBJECT_START            = 16,   //!< StartArea
    OBJECT_END              = 17,   //!< EndArea
    OBJECT_INFO             = 18,   //!< ExchangePost
    OBJECT_PARA             = 19,   //!< PowerCaptor
    OBJECT_TARGET1          = 20,   //!< Target1 (gate)
    OBJECT_TARGET2          = 21,   //!< Target2 (center)
    OBJECT_SAFE             = 22,   //!< Vault
    OBJECT_HUSTON           = 23,   //!< Houston
    OBJECT_DESTROYER        = 24,   //!< Destroyer
    OBJECT_STONE            = 31,   //!< TitaniumOre
    OBJECT_URANIUM          = 32,   //!< UraniumOre
    OBJECT_METAL            = 33,   //!< Titanium
    OBJECT_POWER            = 34,   //!< PowerCell
    OBJECT_ATOMIC           = 35,   //!< NuclearCell
    OBJECT_BULLET           = 36,   //!< OrgaMatter
    OBJECT_BBOX             = 37,   //!< BlackBox
    OBJECT_TNT              = 38,   //!< TNT
    OBJECT_MARKPOWER        = 50,   //!< PowerSpot
    OBJECT_MARKSTONE        = 51,   //!< TitaniumSpot
    OBJECT_MARKURANIUM      = 52,   //!< UraniumSpot
    OBJECT_MARKKEYa         = 53,   //!< KeyASpot
    OBJECT_MARKKEYb         = 54,   //!< KeyBSpot
    OBJECT_MARKKEYc         = 55,   //!< KeyCSpot
    OBJECT_MARKKEYd         = 56,   //!< KeyDSpot
    OBJECT_BOMB             = 60,   //!< Mine
    OBJECT_WINFIRE          = 61,   //!< Firework
    OBJECT_SHOW             = 62,   //!< arrow above object (Visit)
    OBJECT_BAG              = 63,   //!< Bag
    OBJECT_PLANT0           = 70,   //!< Greenery0
    OBJECT_PLANT1           = 71,   //!< Greenery1
    OBJECT_PLANT2           = 72,   //!< Greenery2
    OBJECT_PLANT3           = 73,   //!< Greenery3
    OBJECT_PLANT4           = 74,   //!< Greenery4
    OBJECT_PLANT5           = 75,   //!< Greenery5
    OBJECT_PLANT6           = 76,   //!< Greenery6
    OBJECT_PLANT7           = 77,   //!< Greenery7
    OBJECT_PLANT8           = 78,   //!< Greenery8
    OBJECT_PLANT9           = 79,   //!< Greenery9
    OBJECT_PLANT10          = 80,   //!< Greenery10
    OBJECT_PLANT11          = 81,   //!< Greenery11
    OBJECT_PLANT12          = 82,   //!< Greenery12
    OBJECT_PLANT13          = 83,   //!< Greenery13
    OBJECT_PLANT14          = 84,   //!< Greenery14
    OBJECT_PLANT15          = 85,   //!< Greenery15
    OBJECT_PLANT16          = 86,   //!< Greenery16
    OBJECT_PLANT17          = 87,   //!< Greenery17
    OBJECT_PLANT18          = 88,   //!< Greenery18
    OBJECT_PLANT19          = 89,   //!< Greenery19
    OBJECT_TREE0            = 90,   //!< Tree0
    OBJECT_TREE1            = 91,   //!< Tree1
    OBJECT_TREE2            = 92,   //!< Tree2
    OBJECT_TREE3            = 93,   //!< Tree3
    OBJECT_TREE4            = 94,   //!< Tree4
    OBJECT_TREE5            = 95,   //!< Tree5
    OBJECT_MOBILEwt         = 100,  //!< PracticeBot
    OBJECT_MOBILEtt         = 101,  //!< track-trainer (unused)
    OBJECT_MOBILEft         = 102,  //!< fly-trainer (unused)
    OBJECT_MOBILEit         = 103,  //!< insect-trainer (unused)
    OBJECT_MOBILEwa         = 110,  //!< WheeledGrabber
    OBJECT_MOBILEta         = 111,  //!< TrackedGrabber
    OBJECT_MOBILEfa         = 112,  //!< WingedGrabber
    OBJECT_MOBILEia         = 113,  //!< LeggedGrabber
    OBJECT_MOBILEwc         = 120,  //!< WheeledShooter
    OBJECT_MOBILEtc         = 121,  //!< TrackedShooter
    OBJECT_MOBILEfc         = 122,  //!< WingedShooter
    OBJECT_MOBILEic         = 123,  //!< LeggedShooter
    OBJECT_MOBILEwi         = 130,  //!< WheeledOrgaShooter
    OBJECT_MOBILEti         = 131,  //!< TrackedOrgaShooter
    OBJECT_MOBILEfi         = 132,  //!< WingedOrgaShooter
    OBJECT_MOBILEii         = 133,  //!< LeggedOrgaShooter
    OBJECT_MOBILEws         = 140,  //!< WheeledSniffer
    OBJECT_MOBILEts         = 141,  //!< TrackedSniffer
    OBJECT_MOBILEfs         = 142,  //!< WingedSniffer
    OBJECT_MOBILEis         = 143,  //!< LeggedSniffer
    OBJECT_MOBILErt         = 200,  //!< Thumper
    OBJECT_MOBILErc         = 201,  //!< PhazerShooter
    OBJECT_MOBILErr         = 202,  //!< Recycler
    OBJECT_MOBILErs         = 203,  //!< Shielder
    OBJECT_MOBILEsa         = 210,  //!< Subber
    OBJECT_MOBILEtg         = 211,  //!< TargetBot
    OBJECT_MOBILEdr         = 212,  //!< Scribbler
    OBJECT_CONTROLLER       = 213,  //!< MissionController
    OBJECT_WAYPOINT         = 250,  //!< WayPoint
    OBJECT_FLAGb            = 260,  //!< BlueFlag
    OBJECT_FLAGr            = 261,  //!< RedFlag
    OBJECT_FLAGg            = 262,  //!< GreenFlag
    OBJECT_FLAGy            = 263,  //!< YellowFlag
    OBJECT_FLAGv            = 264,  //!< VioletFlag
    OBJECT_KEYa             = 270,  //!< KeyA
    OBJECT_KEYb             = 271,  //!< KeyB
    OBJECT_KEYc             = 272,  //!< KeyC
    OBJECT_KEYd             = 273,  //!< KeyD
    OBJECT_HUMAN            = 300,  //!< Me
    OBJECT_TOTO             = 301,  //!< Robby (toto)
    OBJECT_TECH             = 302,  //!< Tech
    OBJECT_BARRIER0         = 400,  //!< Barrier0
    OBJECT_BARRIER1         = 401,  //!< Barrier1
    OBJECT_BARRIER2         = 402,  //!< Barrier2
    OBJECT_BARRIER3         = 403,  //!< Barrier3
    OBJECT_BARRICADE0       = 410,  //!< Barricade0
    OBJECT_BARRICADE1       = 411,  //!< Barricade1
    OBJECT_MOTHER           = 500,  //!< AlienQueen
    OBJECT_EGG              = 501,  //!< AlienEgg
    OBJECT_ANT              = 502,  //!< AlienAnt
    OBJECT_SPIDER           = 503,  //!< AlienSpider
    OBJECT_BEE              = 504,  //!< AlienWasp
    OBJECT_WORM             = 505,  //!< AlienWorm
    OBJECT_RUINmobilew1     = 600,  //!< WreckBotw1
    OBJECT_RUINmobilew2     = 601,  //!< WreckBotw2
    OBJECT_RUINmobilet1     = 602,  //!< WreckBott1
    OBJECT_RUINmobilet2     = 603,  //!< WreckBott2
    OBJECT_RUINmobiler1     = 604,  //!< WreckBotr1
    OBJECT_RUINmobiler2     = 605,  //!< WreckBotr2
    OBJECT_RUINfactory      = 606,  //!< RuinBotFactory
    OBJECT_RUINdoor         = 607,  //!< RuinDoor
    OBJECT_RUINsupport      = 608,  //!< RuinSupport
    OBJECT_RUINradar        = 609,  //!< RuinRadar
    OBJECT_RUINconvert      = 610,  //!< RuinConvert
    OBJECT_RUINbase         = 611,  //!< RuinBaseCamp
    OBJECT_RUINhead         = 612,  //!< RuinHeadCamp
    OBJECT_TEEN0            = 620,  //!< Teen0
    OBJECT_TEEN1            = 621,  //!< Teen1
    OBJECT_TEEN2            = 622,  //!< Teen2
    OBJECT_TEEN3            = 623,  //!< Teen3
    OBJECT_TEEN4            = 624,  //!< Teen4
    OBJECT_TEEN5            = 625,  //!< Teen5
    OBJECT_TEEN6            = 626,  //!< Teen6
    OBJECT_TEEN7            = 627,  //!< Teen7
    OBJECT_TEEN8            = 628,  //!< Teen8
    OBJECT_TEEN9            = 629,  //!< Teen9
    OBJECT_TEEN10           = 630,  //!< Teen10
    OBJECT_TEEN11           = 631,  //!< Teen11
    OBJECT_TEEN12           = 632,  //!< Teen12
    OBJECT_TEEN13           = 633,  //!< Teen13
    OBJECT_TEEN14           = 634,  //!< Teen14
    OBJECT_TEEN15           = 635,  //!< Teen15
    OBJECT_TEEN16           = 636,  //!< Teen16
    OBJECT_TEEN17           = 637,  //!< Teen17
    OBJECT_TEEN18           = 638,  //!< Teen18
    OBJECT_TEEN19           = 639,  //!< Teen19
    OBJECT_TEEN20           = 640,  //!< Teen20
    OBJECT_TEEN21           = 641,  //!< Teen21
    OBJECT_TEEN22           = 642,  //!< Teen22
    OBJECT_TEEN23           = 643,  //!< Teen23
    OBJECT_TEEN24           = 644,  //!< Teen24
    OBJECT_TEEN25           = 645,  //!< Teen25
    OBJECT_TEEN26           = 646,  //!< Teen26
    OBJECT_TEEN27           = 647,  //!< Teen27
    OBJECT_TEEN28           = 648,  //!< Teen28
    OBJECT_TEEN29           = 649,  //!< Teen29
    OBJECT_TEEN30           = 650,  //!< Teen30
    OBJECT_TEEN31           = 651,  //!< Teen31
    OBJECT_TEEN32           = 652,  //!< Teen32
    OBJECT_TEEN33           = 653,  //!< Teen33
    OBJECT_TEEN34           = 654,  //!< Stone (Teen34)
    OBJECT_TEEN35           = 655,  //!< Teen35
    OBJECT_TEEN36           = 656,  //!< Teen36
    OBJECT_TEEN37           = 657,  //!< Teen37
    OBJECT_TEEN38           = 658,  //!< Teen38
    OBJECT_TEEN39           = 659,  //!< Teen39
    OBJECT_TEEN40           = 660,  //!< Teen40
    OBJECT_TEEN41           = 661,  //!< Teen41
    OBJECT_TEEN42           = 662,  //!< Teen42
    OBJECT_TEEN43           = 663,  //!< Teen43
    OBJECT_TEEN44           = 664,  //!< Teen44
    OBJECT_QUARTZ0          = 700,  //!< Quartz0
    OBJECT_QUARTZ1          = 701,  //!< Quartz1
    OBJECT_QUARTZ2          = 702,  //!< Quartz2
    OBJECT_QUARTZ3          = 703,  //!< Quartz3
    OBJECT_ROOT0            = 710,  //!< MegaStalk0
    OBJECT_ROOT1            = 711,  //!< MegaStalk1
    OBJECT_ROOT2            = 712,  //!< MegaStalk2
    OBJECT_ROOT3            = 713,  //!< MegaStalk3
    OBJECT_ROOT4            = 714,  //!< MegaStalk4
    OBJECT_ROOT5            = 715,  //!< MegaStalk5
    OBJECT_MUSHROOM1        = 731,  //!< Mushroom1
    OBJECT_MUSHROOM2        = 732,  //!< Mushroom2
    OBJECT_APOLLO1          = 900,  //!< ApolloLEM
    OBJECT_APOLLO2          = 901,  //!< ApolloJeep
    OBJECT_APOLLO3          = 902,  //!< ApolloFlag
    OBJECT_APOLLO4          = 903,  //!< ApolloModule
    OBJECT_APOLLO5          = 904,  //!< ApolloAntenna
    OBJECT_HOME1            = 910,  //!< Home

    OBJECT_MAX              = 1000  //!< number of values
};

struct ObjectTypeHash
{
    inline std::size_t operator()(ObjectType t) const
    {
        return std::hash<int>()(t);
    }
};
