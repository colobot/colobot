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

/**
 * \file common/event.h
 * \brief Event types, structs and event queue
 */

#pragma once

#include "common/key.h"

#include <glm/glm.hpp>

#include <memory>
#include <mutex>
#include <string>

/**
  \enum EventType
  \brief Type of event message
 */
enum EventType
{

// TODO: document the meaning of each value

    //! Invalid event / no event
    EVENT_NULL              = 0,

    // System events (originating in CApplication)

    //! Event sent on system quit request
    EVENT_SYS_QUIT          = 1,

    //! Frame update event
    EVENT_FRAME             = 2,

    //! Event sent after pressing a mouse button
    EVENT_MOUSE_BUTTON_DOWN = 3,
    //! Event sent after releasing a mouse button
    EVENT_MOUSE_BUTTON_UP   = 4,
    //! Event sent after moving mouse wheel up or down
    EVENT_MOUSE_WHEEL       = 5,
    //! Event sent after moving the mouse
    EVENT_MOUSE_MOVE        = 7,
    //! Event sent when mouse enters the window
    EVENT_MOUSE_ENTER       = 8,
    //! Event sent when mouse leaves the window
    EVENT_MOUSE_LEAVE       = 9,

    //! Event sent after pressing a key
    EVENT_KEY_DOWN          = 10,
    //! Event sent after releasing a key
    EVENT_KEY_UP            = 11,
    //! Event sent when user inputs some character
    EVENT_TEXT_INPUT        = 12,

    //! Event sent after moving joystick axes
    EVENT_JOY_AXIS          = 13,
    //! Event sent after pressing a joystick button
    EVENT_JOY_BUTTON_DOWN   = 14,
    //! Event sent after releasing a joystick button
    EVENT_JOY_BUTTON_UP     = 15,

    //! Event sent when the app winddow gains focus
    EVENT_FOCUS_GAINED      = 16,
    //! Event sent when the app winddow loses focus
    EVENT_FOCUS_LOST        = 17,

    //!< Maximum value of system events
    EVENT_SYS_MAX,


    /* Events sent/received in game and user interface */

    //! Event sent on user quit request
    EVENT_QUIT              = 20,
    EVENT_UPDINTERFACE      = 21,
    //! Event sent on resolution change
    EVENT_RESOLUTION_CHANGED = 22,
    //! Event sent when textures have to be reloaded
    EVENT_RELOAD_TEXTURES   = 23,
    EVENT_WIN               = 30,
    EVENT_LOST              = 31,
    EVENT_TAKE_OFF          = 32,

    //! CEdit focus
    EVENT_FOCUS             = 35,

    EVENT_BUTTON_OK         = 40,
    EVENT_BUTTON_CANCEL     = 41,
    EVENT_BUTTON_NEXT       = 42,
    EVENT_BUTTON_PREV       = 43,

    EVENT_BUTTON0           = 50,
    EVENT_BUTTON1           = 51,
    EVENT_BUTTON2           = 52,
    EVENT_BUTTON3           = 53,
    EVENT_BUTTON4           = 54,
    EVENT_BUTTON5           = 55,
    EVENT_BUTTON6           = 56,
    EVENT_BUTTON7           = 57,
    EVENT_BUTTON8           = 58,
    EVENT_BUTTON9           = 59,
    EVENT_BUTTON10          = 60,
    EVENT_BUTTON11          = 61,
    EVENT_BUTTON12          = 62,
    EVENT_BUTTON13          = 63,
    EVENT_BUTTON14          = 64,
    EVENT_BUTTON15          = 65,
    EVENT_BUTTON16          = 66,
    EVENT_BUTTON17          = 67,
    EVENT_BUTTON18          = 68,
    EVENT_BUTTON19          = 69,

    EVENT_EDIT0             = 70,
    EVENT_EDIT1             = 71,
    EVENT_EDIT2             = 72,
    EVENT_EDIT3             = 73,
    EVENT_EDIT4             = 74,
    EVENT_EDIT5             = 75,
    EVENT_EDIT6             = 76,
    EVENT_EDIT7             = 77,
    EVENT_EDIT8             = 78,
    EVENT_EDIT9             = 79,

    EVENT_WINDOW0           = 80,   //!< object interface (CObjectInterface + CAuto classes)
    EVENT_WINDOW1           = 81,   //!< CMainMap
    EVENT_WINDOW2           = 82,   //!< CDisplayText
    EVENT_WINDOW3           = 83,   //!< CStudio
    EVENT_WINDOW4           = 84,   //!< CDisplayInfo
    EVENT_WINDOW5           = 85,   //!< all menu windows
    EVENT_WINDOW6           = 86,   //!< code battle interface
    EVENT_WINDOW7           = 87,   //!< debug interface
    EVENT_WINDOW8           = 88,   //!< (unused)
    EVENT_WINDOW9           = 89,   //!< CMainDialog and CStudio file selector

    EVENT_LABEL0            = 90,
    EVENT_LABEL1            = 91,
    EVENT_LABEL2            = 92,
    EVENT_LABEL3            = 93,
    EVENT_LABEL4            = 94,
    EVENT_LABEL5            = 95,
    EVENT_LABEL6            = 96,
    EVENT_LABEL7            = 97,
    EVENT_LABEL8            = 98,
    EVENT_LABEL9            = 99,
    EVENT_LABEL10           = 100,
    EVENT_LABEL11           = 101,
    EVENT_LABEL12           = 102,
    EVENT_LABEL13           = 103,
    EVENT_LABEL14           = 104,
    EVENT_LABEL15           = 105,
    EVENT_LABEL16           = 106,
    EVENT_LABEL17           = 107,
    EVENT_LABEL18           = 108,
    EVENT_LABEL19           = 109, // cursor position overlay

    EVENT_LIST0             = 110,
    EVENT_LIST1             = 111,
    EVENT_LIST2             = 112, // list of resolutions
    EVENT_LIST3             = 113,
    EVENT_LIST4             = 114,
    EVENT_LIST5             = 115,
    EVENT_LIST6             = 116,
    EVENT_LIST7             = 117,
    EVENT_LIST8             = 118,
    EVENT_LIST9             = 119,

    EVENT_LOADING           = 120,

    EVENT_LABEL_CODE_BATTLE = 121,

    EVENT_SCOREBOARD        = 130,
    EVENT_SCOREBOARD_MAX    = 169,

    EVENT_TOOLTIP           = 200,

    EVENT_DIALOG_OK         = 300,
    EVENT_DIALOG_CANCEL     = 301,
    EVENT_DIALOG_LABEL      = 302,
    EVENT_DIALOG_LABEL1     = 303,
    EVENT_DIALOG_LABEL2     = 304,
    EVENT_DIALOG_LABEL3     = 305,
    EVENT_DIALOG_LIST       = 306,
    EVENT_DIALOG_EDIT       = 307,
    EVENT_DIALOG_EDIT2      = 308,
    EVENT_DIALOG_CHECK1     = 309,
    EVENT_DIALOG_CHECK2     = 310,
    EVENT_DIALOG_GROUP1     = 320,
    EVENT_DIALOG_NEWDIR     = 330,
    EVENT_DIALOG_ACTION     = 348,
    EVENT_DIALOG_STOP       = 349,

    EVENT_INTERFACE_TRAINER = 400,
    EVENT_INTERFACE_DEFI    = 401,
    EVENT_INTERFACE_MISSION = 402,
    EVENT_INTERFACE_FREE    = 403,
    EVENT_INTERFACE_CODE_BATTLES = 404,
    EVENT_INTERFACE_NAME    = 405,
    EVENT_INTERFACE_SETUP   = 406,
    EVENT_INTERFACE_QUIT    = 407,
    EVENT_INTERFACE_BACK    = 408,
    EVENT_INTERFACE_AGAIN   = 409,
    EVENT_INTERFACE_WRITE   = 410,
    EVENT_INTERFACE_READ    = 411,
    EVENT_INTERFACE_ABORT   = 412,
    EVENT_INTERFACE_USER    = 413,
    EVENT_INTERFACE_SATCOM  = 414,
    EVENT_INTERFACE_PLUS    = 415,
    EVENT_INTERFACE_MODS    = 416,

    EVENT_INTERFACE_CHAP    = 420,
    EVENT_INTERFACE_LIST    = 421,
    EVENT_INTERFACE_RESUME  = 422,
    EVENT_INTERFACE_PLAY    = 423,

    EVENT_INTERFACE_SETUPd  = 430,
    EVENT_INTERFACE_SETUPg  = 431,
    EVENT_INTERFACE_SETUPp  = 432,
    EVENT_INTERFACE_SETUPc  = 433,
    EVENT_INTERFACE_SETUPs  = 434,

    EVENT_INTERFACE_DEVICE  = 440,
    EVENT_INTERFACE_RESOL   = 441,
    EVENT_INTERFACE_FULL    = 442,
    EVENT_INTERFACE_APPLY   = 443,

    EVENT_INTERFACE_SHADOW_SPOTS = 451,
    EVENT_INTERFACE_DIRTY   = 452,
    EVENT_INTERFACE_LIGHT   = 457,
    EVENT_INTERFACE_PARTI   = 458,
    EVENT_INTERFACE_CLIP    = 459,
    EVENT_INTERFACE_PAUSE_BLUR = 460,
    EVENT_INTERFACE_RAIN    = 462,
    EVENT_INTERFACE_GLINT   = 463,
    EVENT_INTERFACE_TOOLTIP = 464,
    EVENT_INTERFACE_MOVIES  = 465,
    EVENT_INTERFACE_SCROLL  = 467,
    EVENT_INTERFACE_INVERTX = 468,
    EVENT_INTERFACE_INVERTY = 469,
    EVENT_INTERFACE_EFFECT  = 470,
    EVENT_INTERFACE_BGPAUSE = 471,
    EVENT_INTERFACE_BGMUTE  = 472,
    EVENT_INTERFACE_FOG     = 474,
    EVENT_INTERFACE_EDITMODE= 476,
    EVENT_INTERFACE_EDITVALUE= 477,
    EVENT_INTERFACE_SOLUCE4 = 478,
    EVENT_INTERFACE_BLOOD   = 479,
    EVENT_INTERFACE_AUTOSAVE_ENABLE = 780,
    EVENT_INTERFACE_AUTOSAVE_INTERVAL = 781,
    EVENT_INTERFACE_AUTOSAVE_SLOTS = 782,
    EVENT_INTERFACE_TEXTURE_FILTER = 783,
    EVENT_INTERFACE_TEXTURE_MIPMAP = 784,
    EVENT_INTERFACE_TEXTURE_ANISOTROPY = 785,
    EVENT_INTERFACE_MSAA = 786,
    EVENT_INTERFACE_SHADOW_MAPPING = 787,
    EVENT_INTERFACE_SHADOW_MAPPING_QUALITY = 788,
    EVENT_INTERFACE_SHADOW_MAPPING_BUFFER = 789,
    EVENT_INTERFACE_LANGUAGE = 790,
    EVENT_INTERFACE_VSYNC = 791,

    EVENT_INTERFACE_KINFO1  = 500,
    EVENT_INTERFACE_KINFO2  = 501,
    EVENT_INTERFACE_KGROUP  = 502,
    EVENT_INTERFACE_KSCROLL = 503,
    EVENT_INTERFACE_KDEF    = 504,
    // Reserved space for keybindings
    // This is not the nicest solution, but it'll have to work like that until we move to CEGUI
    EVENT_INTERFACE_KEY     = 505,
    EVENT_INTERFACE_KEY_END = 539,

    EVENT_INTERFACE_MIN     = 540,
    EVENT_INTERFACE_NORM    = 541,
    EVENT_INTERFACE_MAX     = 542,

    EVENT_INTERFACE_VOLSOUND= 550,
    EVENT_INTERFACE_VOLMUSIC= 551,
    EVENT_INTERFACE_SILENT  = 552,
    EVENT_INTERFACE_NOISY   = 553,

    EVENT_INTERFACE_JOYSTICK= 560,
    EVENT_INTERFACE_SOLUCE  = 561,
    EVENT_INTERFACE_JOYSTICK_DEADZONE = 562,
    EVENT_INTERFACE_JOYSTICK_X = 563,
    EVENT_INTERFACE_JOYSTICK_Y = 564,
    EVENT_INTERFACE_JOYSTICK_Z = 565,
    EVENT_INTERFACE_JOYSTICK_CAM_X = 566,
    EVENT_INTERFACE_JOYSTICK_CAM_Y = 567,
    EVENT_INTERFACE_JOYSTICK_CAM_Z = 568,
    EVENT_INTERFACE_JOYSTICK_X_INVERT = 569,
    EVENT_INTERFACE_JOYSTICK_Y_INVERT = 570,
    EVENT_INTERFACE_JOYSTICK_Z_INVERT = 571,
    EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT = 572,
    EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT = 573,
    EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT = 574,

    EVENT_INTERFACE_PLUS_TRAINER  = 575,
    EVENT_INTERFACE_PLUS_RESEARCH = 576,
    EVENT_INTERFACE_PLUS_EXPLORER = 577,

    EVENT_INTERFACE_MOD_LIST = 580,
    EVENT_INTERFACE_WORKSHOP = 581,
    EVENT_INTERFACE_MODS_DIR = 582,
    EVENT_INTERFACE_MOD_ENABLE_OR_DISABLE = 583,
    EVENT_INTERFACE_MODS_APPLY = 584,
    EVENT_INTERFACE_MOD_SUMMARY = 585,
    EVENT_INTERFACE_MOD_DETAILS = 586,
    EVENT_INTERFACE_MOD_MOVE_UP = 587,
    EVENT_INTERFACE_MOD_MOVE_DOWN = 888,
    EVENT_INTERFACE_MODS_REFRESH = 589,

    EVENT_INTERFACE_GLINTl  = 590,
    EVENT_INTERFACE_GLINTr  = 591,
    EVENT_INTERFACE_GLINTu  = 592,
    EVENT_INTERFACE_GLINTb  = 593,

    EVENT_INTERFACE_NEDIT   = 595,
    EVENT_INTERFACE_NLIST   = 596,
    EVENT_INTERFACE_NOK     = 597,
    EVENT_INTERFACE_NDELETE = 598,
    EVENT_INTERFACE_NLABEL  = 599,

    EVENT_INTERFACE_IOWRITE = 600,
    EVENT_INTERFACE_IOREAD  = 601,
    EVENT_INTERFACE_IOLIST  = 602,
    EVENT_INTERFACE_IONAME  = 603,
    EVENT_INTERFACE_IOLABEL = 604,
    EVENT_INTERFACE_IOIMAGE = 605,
    EVENT_INTERFACE_IODELETE= 606,

    EVENT_INTERFACE_PERSO   = 620,
    EVENT_INTERFACE_POK     = 621,
    EVENT_INTERFACE_PCANCEL = 622,
    EVENT_INTERFACE_PDEF    = 623,
    EVENT_INTERFACE_PHEAD   = 624,
    EVENT_INTERFACE_PBODY   = 625,
    EVENT_INTERFACE_PLROT   = 626,
    EVENT_INTERFACE_PRROT   = 627,
    EVENT_INTERFACE_PC0a    = 640,
    EVENT_INTERFACE_PC1a    = 641,
    EVENT_INTERFACE_PC2a    = 642,
    EVENT_INTERFACE_PC3a    = 643,
    EVENT_INTERFACE_PC4a    = 644,
    EVENT_INTERFACE_PC5a    = 645,
    EVENT_INTERFACE_PC6a    = 646,
    EVENT_INTERFACE_PC7a    = 647,
    EVENT_INTERFACE_PC8a    = 648,
    EVENT_INTERFACE_PC9a    = 649,
    EVENT_INTERFACE_PCRa    = 650,
    EVENT_INTERFACE_PCGa    = 651,
    EVENT_INTERFACE_PCBa    = 652,
    EVENT_INTERFACE_PC0b    = 660,
    EVENT_INTERFACE_PC1b    = 661,
    EVENT_INTERFACE_PC2b    = 662,
    EVENT_INTERFACE_PC3b    = 663,
    EVENT_INTERFACE_PC4b    = 664,
    EVENT_INTERFACE_PC5b    = 665,
    EVENT_INTERFACE_PC6b    = 666,
    EVENT_INTERFACE_PC7b    = 667,
    EVENT_INTERFACE_PC8b    = 668,
    EVENT_INTERFACE_PC9b    = 669,
    EVENT_INTERFACE_PCRb    = 670,
    EVENT_INTERFACE_PCGb    = 671,
    EVENT_INTERFACE_PCBb    = 672,
    EVENT_INTERFACE_PFACE1  = 680,
    EVENT_INTERFACE_PFACE2  = 681,
    EVENT_INTERFACE_PFACE3  = 682,
    EVENT_INTERFACE_PFACE4  = 683,
    EVENT_INTERFACE_PGLASS0 = 690,
    EVENT_INTERFACE_PGLASS1 = 691,
    EVENT_INTERFACE_PGLASS2 = 692,
    EVENT_INTERFACE_PGLASS3 = 693,
    EVENT_INTERFACE_PGLASS4 = 694,
    EVENT_INTERFACE_PGLASS5 = 695,
    EVENT_INTERFACE_PGLASS6 = 696,
    EVENT_INTERFACE_PGLASS7 = 697,
    EVENT_INTERFACE_PGLASS8 = 698,
    EVENT_INTERFACE_PGLASS9 = 699,

    EVENT_DT_GROUP0         = 700,
    EVENT_DT_GROUP1         = 701,
    EVENT_DT_GROUP2         = 702,
    EVENT_DT_GROUP3         = 703,
    EVENT_DT_GROUP4         = 704,
    EVENT_DT_LABEL0         = 710,
    EVENT_DT_LABEL1         = 711,
    EVENT_DT_LABEL2         = 712,
    EVENT_DT_LABEL3         = 713,
    EVENT_DT_LABEL4         = 714,
    EVENT_DT_VISIT0         = 720,
    EVENT_DT_VISIT1         = 721,
    EVENT_DT_VISIT2         = 722,
    EVENT_DT_VISIT3         = 723,
    EVENT_DT_VISIT4         = 724,
    EVENT_DT_END            = 725,

    EVENT_CMD               = 800,
    EVENT_SPEED             = 801,

    EVENT_DBG_STATS         = 850,
    EVENT_DBG_SPAWN_OBJ     = 851,
    EVENT_DBG_TELEPORT      = 852,
    EVENT_DBG_LIGHTNING     = 853,
    EVENT_DBG_RESOURCES     = 854,
    EVENT_DBG_GOTO          = 855,
    EVENT_DBG_CRASHSPHERES  = 856,
    EVENT_DBG_LIGHTS        = 857,
    EVENT_DBG_LIGHTS_DUMP   = 858,

    EVENT_SPAWN_CANCEL      = 860,
    EVENT_SPAWN_ME          = 861,
    EVENT_SPAWN_WHEELEDGRABBER = 862,
    EVENT_SPAWN_WHEELEDSHOOTER = 863,
    EVENT_SPAWN_PHAZERSHOOTER  = 864,
    EVENT_SPAWN_BOTFACTORY  = 865,
    EVENT_SPAWN_CONVERTER   = 866,
    EVENT_SPAWN_DERRICK     = 867,
    EVENT_SPAWN_POWERSTATION= 868,
    EVENT_SPAWN_TITANIUM    = 869,
    EVENT_SPAWN_TITANIUMORE = 870,
    EVENT_SPAWN_URANIUMORE  = 871,
    EVENT_SPAWN_POWERCELL   = 872,
    EVENT_SPAWN_NUCLEARCELL = 873,

    EVENT_HYPER_PREV        = 900,
    EVENT_HYPER_NEXT        = 901,
    EVENT_HYPER_HOME        = 902,
    EVENT_HYPER_COPY        = 903,
    EVENT_HYPER_SIZE1       = 904,
    EVENT_HYPER_SIZE2       = 905,
    EVENT_HYPER_SIZE3       = 906,
    EVENT_HYPER_SIZE4       = 907,
    EVENT_HYPER_SIZE5       = 908,

    EVENT_SATCOM_HUSTON     = 920,
    EVENT_SATCOM_SAT        = 921,
    EVENT_SATCOM_LOADING    = 922,
    EVENT_SATCOM_OBJECT     = 923,
    EVENT_SATCOM_PROG       = 924,
    EVENT_SATCOM_SOLUCE     = 925,

    EVENT_OBJECT_DESELECT   = 1000,
    EVENT_OBJECT_LEFT       = 1001,
    EVENT_OBJECT_RIGHT      = 1002,
    EVENT_OBJECT_UP         = 1003,
    EVENT_OBJECT_DOWN       = 1004,
    EVENT_OBJECT_GASUP      = 1005,
    EVENT_OBJECT_GASDOWN    = 1006,
    EVENT_OBJECT_HTAKE      = 1020,
    EVENT_OBJECT_MTAKE      = 1021,
    EVENT_OBJECT_MFRONT     = 1022,
    EVENT_OBJECT_MBACK      = 1023,
    EVENT_OBJECT_MPOWER     = 1024,
    EVENT_OBJECT_BHELP      = 1040,
    EVENT_OBJECT_BTAKEOFF   = 1041,
    EVENT_OBJECT_BDESTROY   = 1042,
    EVENT_OBJECT_BDERRICK   = 1050,
    EVENT_OBJECT_BSTATION   = 1051,
    EVENT_OBJECT_BFACTORY   = 1052,
    EVENT_OBJECT_BCONVERT   = 1053,
    EVENT_OBJECT_BTOWER     = 1054,
    EVENT_OBJECT_BREPAIR    = 1055,
    EVENT_OBJECT_BRESEARCH  = 1056,
    EVENT_OBJECT_BRADAR     = 1057,
    EVENT_OBJECT_BENERGY    = 1058,
    EVENT_OBJECT_BLABO      = 1059,
    EVENT_OBJECT_BNUCLEAR   = 1060,
    EVENT_OBJECT_BPARA      = 1061,
    EVENT_OBJECT_BINFO      = 1062,
    EVENT_OBJECT_BSAFE      = 1063,
    EVENT_OBJECT_GFLAT      = 1070,
    EVENT_OBJECT_FCREATE    = 1071,
    EVENT_OBJECT_FDELETE    = 1072,
    EVENT_OBJECT_FCOLORb    = 1073,
    EVENT_OBJECT_FCOLORr    = 1074,
    EVENT_OBJECT_FCOLORg    = 1075,
    EVENT_OBJECT_FCOLORy    = 1076,
    EVENT_OBJECT_FCOLORv    = 1077,
    EVENT_OBJECT_FACTORYwa  = 1080,
    EVENT_OBJECT_FACTORYta  = 1081,
    EVENT_OBJECT_FACTORYfa  = 1082,
    EVENT_OBJECT_FACTORYia  = 1083,
    EVENT_OBJECT_FACTORYwc  = 1084,
    EVENT_OBJECT_FACTORYtc  = 1085,
    EVENT_OBJECT_FACTORYfc  = 1086,
    EVENT_OBJECT_FACTORYic  = 1087,
    EVENT_OBJECT_FACTORYwi  = 1088,
    EVENT_OBJECT_FACTORYti  = 1089,
    EVENT_OBJECT_FACTORYfi  = 1090,
    EVENT_OBJECT_FACTORYii  = 1091,
    EVENT_OBJECT_FACTORYws  = 1092,
    EVENT_OBJECT_FACTORYts  = 1093,
    EVENT_OBJECT_FACTORYfs  = 1094,
    EVENT_OBJECT_FACTORYis  = 1095,
    EVENT_OBJECT_FACTORYrt  = 1096,
    EVENT_OBJECT_FACTORYrc  = 1097,
    EVENT_OBJECT_FACTORYrr  = 1098,
    EVENT_OBJECT_FACTORYrs  = 1099,
    EVENT_OBJECT_FACTORYsa  = 1100,
    EVENT_OBJECT_FACTORYwb  = 1101,
    EVENT_OBJECT_FACTORYtb  = 1102,
    EVENT_OBJECT_FACTORYfb  = 1103,
    EVENT_OBJECT_FACTORYib  = 1104,
    EVENT_OBJECT_FACTORYtg  = 1105,
    EVENT_OBJECT_SEARCH     = 1200,
    EVENT_OBJECT_TERRAFORM  = 1201,
    EVENT_OBJECT_FIRE       = 1202,
    EVENT_OBJECT_FIREANT    = 1203,
    EVENT_OBJECT_SPIDEREXPLO= 1204,
    EVENT_OBJECT_RECOVER    = 1220,
    EVENT_OBJECT_BEGSHIELD  = 1221,
    EVENT_OBJECT_ENDSHIELD  = 1222,
    EVENT_OBJECT_RTANK      = 1223,
    EVENT_OBJECT_RFLY       = 1224,
    EVENT_OBJECT_RTHUMP     = 1225,
    EVENT_OBJECT_RCANON     = 1226,
    EVENT_OBJECT_RTOWER     = 1227,
    EVENT_OBJECT_RPHAZER    = 1228,
    EVENT_OBJECT_RSHIELD    = 1229,
    EVENT_OBJECT_RATOMIC    = 1230,
    EVENT_OBJECT_RiPAW      = 1231,
    EVENT_OBJECT_RiGUN      = 1232,
    EVENT_OBJECT_RESET      = 1233,
    EVENT_OBJECT_DIMSHIELD  = 1234,
    EVENT_OBJECT_TARGET     = 1235,
    EVENT_OBJECT_DELSEARCH  = 1236, // delete mark on ground
    EVENT_OBJECT_PROGLIST   = 1310,
    EVENT_OBJECT_PROGRUN    = 1311,
    EVENT_OBJECT_PROGEDIT   = 1312,
    EVENT_OBJECT_PROGSTART  = 1313,
    EVENT_OBJECT_PROGSTOP   = 1314,
    EVENT_OBJECT_PROGADD    = 1315,
    EVENT_OBJECT_PROGREMOVE = 1316,
    EVENT_OBJECT_PROGCLONE  = 1317,
    EVENT_OBJECT_PROGMOVEUP = 1318,
    EVENT_OBJECT_PROGMOVEDOWN = 1319,
    EVENT_OBJECT_INFOOK     = 1340,
    EVENT_OBJECT_DELETE     = 1350,
    EVENT_OBJECT_GENERGY    = 1360,
    EVENT_OBJECT_GSHIELD    = 1361,
    EVENT_OBJECT_GRANGE     = 1362,
    EVENT_OBJECT_MAP        = 1364,
    EVENT_OBJECT_MAPZOOM    = 1365,
    EVENT_OBJECT_GPROGRESS  = 1366,
    EVENT_OBJECT_GRADAR     = 1367,
    EVENT_OBJECT_GINFO      = 1368,
    EVENT_OBJECT_TYPE       = 1369,
    EVENT_OBJECT_CROSSHAIR  = 1370,
    EVENT_OBJECT_CORNERul   = 1371,
    EVENT_OBJECT_CORNERur   = 1372,
    EVENT_OBJECT_CORNERdl   = 1373,
    EVENT_OBJECT_CORNERdr   = 1374,
    EVENT_OBJECT_MAPi       = 1375,
    EVENT_OBJECT_MAPg       = 1376,
    EVENT_OBJECT_CAMERA     = 1400,
    EVENT_OBJECT_HELP       = 1401,
    EVENT_OBJECT_SOLUCE     = 1402,
    EVENT_OBJECT_CAMERAleft = 1403,
    EVENT_OBJECT_CAMERAright= 1404,
    EVENT_OBJECT_CAMERAnear = 1405,
    EVENT_OBJECT_CAMERAaway = 1406,
    EVENT_OBJECT_SHORTCUT_MODE = 1500,
    EVENT_OBJECT_SHORTCUT   = 1501,
    EVENT_OBJECT_SHORTCUT_MAX = 1549,
    EVENT_OBJECT_MOVIELOCK  = 1550,
    EVENT_OBJECT_EDITLOCK   = 1551,
    EVENT_OBJECT_SAVING     = 1552,
    EVENT_OBJECT_LIMIT      = 1560,

    EVENT_OBJECT_PEN0       = 1570,
    EVENT_OBJECT_PEN1       = 1571,
    EVENT_OBJECT_PEN2       = 1572,
    EVENT_OBJECT_PEN3       = 1573,
    EVENT_OBJECT_PEN4       = 1574,
    EVENT_OBJECT_PEN5       = 1575,
    EVENT_OBJECT_PEN6       = 1576,
    EVENT_OBJECT_PEN7       = 1577,
    EVENT_OBJECT_PEN8       = 1578,
    EVENT_OBJECT_REC        = 1580,
    EVENT_OBJECT_STOP       = 1581,

    EVENT_STUDIO_OK         = 2000,
    EVENT_STUDIO_CANCEL     = 2001,
    EVENT_STUDIO_EDIT       = 2002,
    EVENT_STUDIO_LIST       = 2003,
    EVENT_STUDIO_CLONE      = 2004,
    EVENT_STUDIO_NEW        = 2010,
    EVENT_STUDIO_OPEN       = 2011,
    EVENT_STUDIO_SAVE       = 2012,
    EVENT_STUDIO_UNDO       = 2013,
    EVENT_STUDIO_CUT        = 2014,
    EVENT_STUDIO_COPY       = 2015,
    EVENT_STUDIO_PASTE      = 2016,
    EVENT_STUDIO_SIZE       = 2017,
    EVENT_STUDIO_TOOL       = 2018,
    EVENT_STUDIO_HELP       = 2019,
    EVENT_STUDIO_COMPILE    = 2050,
    EVENT_STUDIO_RUN        = 2051,
    EVENT_STUDIO_REALTIME   = 2052,
    EVENT_STUDIO_STEP       = 2053,

    EVENT_WRITE_SCENE_FINISHED = 2100, //!< indicates end of writing scene (writing screenshot image)

    EVENT_CODE_BATTLE_START = 2200, //!< button that starts the code battle
    EVENT_CODE_BATTLE_SPECTATOR = 2201, //!< button that controls the code battle spectator camera

    EVENT_OBJECT_RBUILDER       = 2300,
    EVENT_OBJECT_BUILD          = 2301,
    EVENT_OBJECT_RTARGET        = 2302,

    //! Buttons that switch viewpoints
    EVENT_VIEWPOINT0 = 3000,
    EVENT_VIEWPOINT1 = 3001,
    EVENT_VIEWPOINT2 = 3002,
    EVENT_VIEWPOINT3 = 3003,
    EVENT_VIEWPOINT4 = 3004,
    EVENT_VIEWPOINT5 = 3005,
    EVENT_VIEWPOINT6 = 3006,
    EVENT_VIEWPOINT7 = 3007,
    EVENT_VIEWPOINT8 = 3008,
    EVENT_VIEWPOINT9 = 3009,
    //! Maximum value of standard events
    EVENT_STD_MAX,

    EVENT_USER              = 10000,
    EVENT_FORCE_LONG        = 0x7fffffff
};

/**
 * \struct EventData
 * \brief Base class for additional event data
 */
struct EventData
{
    virtual ~EventData()
    {}

    virtual std::unique_ptr<EventData> Clone() const = 0;
};

/**
 * \struct KeyEventData
 * \brief Additional data for keyboard event
 */
struct KeyEventData : public EventData
{
    std::unique_ptr<EventData> Clone() const override
    {
        return std::make_unique<KeyEventData>(*this);
    }

    //! If true, the key is a virtual code generated by certain key modifiers or joystick buttons
    bool virt = false;
    //! Key symbol: KEY(...) macro value or virtual key VIRTUAL_... (from common/key.h)
    unsigned int key = 0;
    //! Input binding slot for this key
    InputSlot slot = INPUT_SLOT_MAX;
};

/**
 * \struct TextInputData
 * \brief Additional data for text input event
 */
 struct TextInputData : public EventData
 {
    std::unique_ptr<EventData> Clone() const override
    {
        return std::make_unique<TextInputData>(*this);
    }

    //! Text entered by the user (usually one character, UTF-8 encoded)
    std::string text = "";
 };

/**
 * \enum MouseButton
 * \brief Mouse button
 *
 * Values are a bitmask to have a state bitmask
 */
enum MouseButton
{
    MOUSE_BUTTON_LEFT   = (1<<1),
    MOUSE_BUTTON_MIDDLE = (1<<2),
    MOUSE_BUTTON_RIGHT  = (1<<3),
    //! There may be additional mouse buttons >= this value
    MOUSE_BUTTON_OTHER  = (1<<4)
};

/**
 * \struct MouseButtonEventData
 * \brief Additional data mouse button event
 */
struct MouseButtonEventData : public EventData
{
    std::unique_ptr<EventData> Clone() const override
    {
        return std::make_unique<MouseButtonEventData>(*this);
    }

    //! The mouse button
    MouseButton button = MOUSE_BUTTON_LEFT;
};

/**
 * \struct MouseWheelEventData
 * \brief Additional data for mouse wheel event.
 */
struct MouseWheelEventData : public EventData
{
    std::unique_ptr<EventData> Clone() const override
    {
        return std::make_unique<MouseWheelEventData>(*this);
    }

    //! Amount scrolled vertically, positive value is away from the user
    signed int y = 0;
    //! Amount scrolled horizontally (if the mouse supports it), positive value is to the right
    signed int x = 0;
};

/**
 * \struct JoyAxisEventData
 * \brief Additional data for joystick axis event
 */
struct JoyAxisEventData : public EventData
{
    std::unique_ptr<EventData> Clone() const override
    {
        return std::make_unique<JoyAxisEventData>(*this);
    }

    //! The joystick axis index
    unsigned char axis = 0;
    //! The axis value (range: -32768 to 32767)
    int value = 0;
};

/**
 * \struct JoyButtonEventData
 * \brief Additional data for joystick button event
 */
struct JoyButtonEventData : public EventData
{
    std::unique_ptr<EventData> Clone() const override
    {
        return std::make_unique<JoyButtonEventData>(*this);
    }

    //! The joystick button index
    unsigned char button = 0;
};

/**
 * \struct Event
 * \brief Event sent by system, interface or game
 *
 * Event is described by its type (EventType) and anonymous union that
 * contains additional data about the event.
 * Different members of the union are filled with different event types.
 * With some events, nothing is filled (it's zeroed out).
 * The union contains roughly the same information as SDL_Event struct
 * but packaged to independent structs and fields.
 **/
struct Event
{
    explicit Event(EventType type = EVENT_NULL)
     : type(type),
       rTime(0.0f),
       kmodState(0),
       mouseButtonsState(0),
       customParam(0)
    {}

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    // Workaround for MSVC2013
    Event(Event&& other)
        : type(std::move(other.type)),
          rTime(std::move(other.rTime)),
          motionInput(std::move(other.motionInput)),
          cameraInput(std::move(other.cameraInput)),
          kmodState(std::move(other.kmodState)),
          mousePos(std::move(other.mousePos)),
          mouseButtonsState(std::move(other.mouseButtonsState)),
          customParam(std::move(other.customParam)),
          data(std::move(other.data))
    {}

    Event& operator=(Event&& other)
    {
        type = std::move(other.type);
        rTime = std::move(other.rTime);
        motionInput = std::move(other.motionInput);
        cameraInput = std::move(other.cameraInput);
        kmodState = std::move(other.kmodState);
        mousePos = std::move(other.mousePos);
        mouseButtonsState = std::move(other.mouseButtonsState);
        customParam = std::move(other.customParam);
        data = std::move(other.data);
        return *this;
    }

    //! Convenience function for getting appropriate EventData subclass
    template<typename EventDataSubclass>
    EventDataSubclass* GetData()
    {
        return static_cast<EventDataSubclass*>(data.get());
    }

    //! Convenience function for getting appropriate EventData subclass
    template<typename EventDataSubclass>
    const EventDataSubclass* GetData() const
    {
        return static_cast<EventDataSubclass*>(data.get());
    }

    //! Returns a clone of this event
    Event Clone() const
    {
        Event clone;

        clone.type = type;
        clone.rTime = rTime;
        clone.motionInput = motionInput;
        clone.cameraInput = cameraInput;
        clone.kmodState = kmodState;
        clone.mousePos = mousePos;
        clone.mouseButtonsState = mouseButtonsState;
        clone.customParam = customParam;

        if (data != nullptr)
        {
            clone.data = data->Clone();
        }

        return clone;
    }

    //! Type of event
    EventType type;

    //! Relative time since last EVENT_FRAME
    //! Scope: only EVENT_FRAME events
    // TODO: gradually replace the usage of this with new CApplication's time functions
    float        rTime;

    //! Motion vector set by keyboard or joystick (managed by CInput)
    //! Scope: all system events
    glm::vec3 motionInput{ 0, 0, 0 };

    //! Motion vector set by numeric keyboard (managed by CInput)
    //! Scope: all system events
    glm::vec3 cameraInput{ 0, 0, 0 };

    //! Current state of keyboard modifier keys: bitmask made of KEY_MOD(...) macro values (from common/key.h)
    //! Scope: all system events
    unsigned int kmodState;

    //! Current position of mouse cursor in interface coords
    //! Scope: all system events
    glm::vec2    mousePos;

    //! Current state of mouse buttons: bitmask of MouseButton enum values
    //! Scope: all system events
    unsigned int mouseButtonsState;

    //! Custom parameter that may be set for some events
    //! Scope: some interface events
    long         customParam;

    //! Additional data for some events
    std::unique_ptr<EventData> data;
};


//! Returns an unique event type (above the standard IDs)
EventType GetUniqueEventType();

//! Initializes static array with event type strings
void InitializeEventTypeTexts();

//! Parses event type to string
std::string ParseEventType(EventType eventType);

/**
 * \class CEventQueue
 * \brief Global event queue
 *
 * Provides an interface to a global FIFO queue with events (both system- and user-generated).
 * The queue has a fixed maximum size but it should not be a problem.
 *
 * This class is thread-safe
 */
class CEventQueue
{
public:
    //! Constant maximum size of queue
    static const int MAX_EVENT_QUEUE = 100;

public:
    //! Object's constructor
    CEventQueue();
    //! Object's destructor
    ~CEventQueue();

    //! Checks if queue is empty
    bool IsEmpty();
    //! Adds an event to the queue
    bool AddEvent(Event&& event);
    //! Removes and returns an event from queue front; if queue is empty, returns event of type EVENT_NULL
    Event GetEvent();

protected:
    std::mutex   m_mutex;
    Event        m_fifo[MAX_EVENT_QUEUE];
    int          m_head;
    int          m_tail;
    int          m_total;
};
