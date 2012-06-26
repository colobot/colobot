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

// event.h

#ifndef _EVENT_H_
#define _EVENT_H_


#include "struct.h"


#if !defined (WM_XBUTTONDOWN)
#define WM_XBUTTONDOWN      0x020B
#define WM_XBUTTONUP        0x020C
#define XBUTTON1            0x0001
#define XBUTTON2            0x0002
#endif



class CInstanceManager;


#define MAXEVENT    100

// Events.

enum EventMsg
{
    EVENT_NULL              = 0,

    EVENT_QUIT              = 1,
    EVENT_FRAME             = 2,
    EVENT_LBUTTONDOWN       = 3,
    EVENT_RBUTTONDOWN       = 4,
    EVENT_LBUTTONUP         = 5,
    EVENT_RBUTTONUP         = 6,
    EVENT_MOUSEMOVE         = 7,
    EVENT_KEYDOWN           = 8,
    EVENT_KEYUP             = 9,
    EVENT_CHAR              = 10,
    EVENT_FOCUS             = 11,

    EVENT_UPDINTERFACE      = 20,
    EVENT_WIN               = 30,
    EVENT_LOST              = 31,

    EVENT_BUTTON_OK         = 40,
    EVENT_BUTTON_CANCEL     = 41,
    EVENT_BUTTON_NEXT       = 42,
    EVENT_BUTTON_PREV       = 43,
    EVENT_BUTTON_QUIT       = 44,

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

    EVENT_WINDOW0           = 80,   // the bottom panel
    EVENT_WINDOW1           = 81,   // map
    EVENT_WINDOW2           = 82,   // CDisplayText
    EVENT_WINDOW3           = 83,   // CStudio
    EVENT_WINDOW4           = 84,   // DisplayInfo
    EVENT_WINDOW5           = 85,   // setup
    EVENT_WINDOW6           = 86,
    EVENT_WINDOW7           = 87,
    EVENT_WINDOW8           = 88,
    EVENT_WINDOW9           = 89,   // dialogue

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
    EVENT_LABEL19           = 109,

    EVENT_LIST0             = 110,
    EVENT_LIST1             = 111,
    EVENT_LIST2             = 112,
    EVENT_LIST3             = 113,
    EVENT_LIST4             = 114,
    EVENT_LIST5             = 115,
    EVENT_LIST6             = 116,
    EVENT_LIST7             = 117,
    EVENT_LIST8             = 118,
    EVENT_LIST9             = 119,

    EVENT_TOOLTIP           = 200,

    EVENT_DIALOG_OK         = 300,
    EVENT_DIALOG_CANCEL     = 301,
    EVENT_DIALOG_LABEL      = 302,
    EVENT_DIALOG_LABEL1     = 303,
    EVENT_DIALOG_LABEL2     = 304,
    EVENT_DIALOG_LABEL3     = 305,
    EVENT_DIALOG_LIST       = 306,
    EVENT_DIALOG_EDIT       = 307,
    EVENT_DIALOG_CHECK1     = 308,
    EVENT_DIALOG_CHECK2     = 309,

    EVENT_INTERFACE_TRAINER = 400,
    EVENT_INTERFACE_DEFI    = 401,
    EVENT_INTERFACE_MISSION = 402,
    EVENT_INTERFACE_FREE    = 403,
    EVENT_INTERFACE_PROTO   = 404,
    EVENT_INTERFACE_NAME    = 405,
    EVENT_INTERFACE_SETUP   = 406,
    EVENT_INTERFACE_QUIT    = 407,
    EVENT_INTERFACE_BACK    = 408,
    EVENT_INTERFACE_AGAIN   = 409,
    EVENT_INTERFACE_WRITE   = 410,
    EVENT_INTERFACE_READ    = 411,
    EVENT_INTERFACE_ABORT   = 412,
    EVENT_INTERFACE_USER    = 413,
    EVENT_INTERFACE_TEEN    = 414,

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

    EVENT_INTERFACE_TOTO    = 450,
    EVENT_INTERFACE_SHADOW  = 451,
    EVENT_INTERFACE_DIRTY   = 452,
    EVENT_INTERFACE_LENS    = 453,
    EVENT_INTERFACE_SKY     = 454,
    EVENT_INTERFACE_PLANET  = 456,
    EVENT_INTERFACE_LIGHT   = 457,
    EVENT_INTERFACE_PARTI   = 458,
    EVENT_INTERFACE_CLIP    = 459,
    EVENT_INTERFACE_DETAIL  = 460,
    EVENT_INTERFACE_TEXTURE = 461,
    EVENT_INTERFACE_RAIN    = 462,
    EVENT_INTERFACE_GLINT   = 463,
    EVENT_INTERFACE_TOOLTIP = 464,
    EVENT_INTERFACE_MOVIES  = 465,
    EVENT_INTERFACE_NICERST = 466,
    EVENT_INTERFACE_SCROLL  = 467,
    EVENT_INTERFACE_INVERTX = 468,
    EVENT_INTERFACE_INVERTY = 469,
    EVENT_INTERFACE_EFFECT  = 470,
    EVENT_INTERFACE_MOUSE   = 471,
    EVENT_INTERFACE_GROUND  = 472,
    EVENT_INTERFACE_GADGET  = 473,
    EVENT_INTERFACE_FOG     = 474,
    EVENT_INTERFACE_HIMSELF = 475,
    EVENT_INTERFACE_EDITMODE= 476,
    EVENT_INTERFACE_EDITVALUE= 477,
    EVENT_INTERFACE_SOLUCE4 = 478,

    EVENT_INTERFACE_KINFO1  = 500,
    EVENT_INTERFACE_KINFO2  = 501,
    EVENT_INTERFACE_KGROUP  = 502,
    EVENT_INTERFACE_KSCROLL = 503,
    EVENT_INTERFACE_KDEF    = 504,
    EVENT_INTERFACE_KLEFT   = 505,
    EVENT_INTERFACE_KRIGHT  = 506,
    EVENT_INTERFACE_KUP     = 507,
    EVENT_INTERFACE_KDOWN   = 508,
    EVENT_INTERFACE_KGUP    = 509,
    EVENT_INTERFACE_KGDOWN  = 510,
    EVENT_INTERFACE_KCAMERA = 511,
    EVENT_INTERFACE_KDESEL  = 512,
    EVENT_INTERFACE_KACTION = 513,
    EVENT_INTERFACE_KNEAR   = 514,
    EVENT_INTERFACE_KAWAY   = 515,
    EVENT_INTERFACE_KNEXT   = 516,
    EVENT_INTERFACE_KHUMAN  = 517,
    EVENT_INTERFACE_KQUIT   = 518,
    EVENT_INTERFACE_KHELP   = 519,
    EVENT_INTERFACE_KPROG   = 520,
    EVENT_INTERFACE_KCBOT   = 521,
    EVENT_INTERFACE_KVISIT  = 522,
    EVENT_INTERFACE_KSPEED10= 523,
    EVENT_INTERFACE_KSPEED15= 524,
    EVENT_INTERFACE_KSPEED20= 525,
    EVENT_INTERFACE_KSPEED30= 526,

    EVENT_INTERFACE_VOLSOUND= 530,
    EVENT_INTERFACE_VOLMUSIC= 531,
    EVENT_INTERFACE_SOUND3D = 532,

    EVENT_INTERFACE_MIN     = 540,
    EVENT_INTERFACE_NORM    = 541,
    EVENT_INTERFACE_MAX     = 542,

    EVENT_INTERFACE_SILENT  = 550,
    EVENT_INTERFACE_NOISY   = 551,

    EVENT_INTERFACE_JOYSTICK= 560,
    EVENT_INTERFACE_SOLUCE  = 561,

    EVENT_INTERFACE_GLINTl  = 570,
    EVENT_INTERFACE_GLINTr  = 571,
    EVENT_INTERFACE_GLINTu  = 572,
    EVENT_INTERFACE_GLINTb  = 573,

    EVENT_INTERFACE_NEDIT   = 580,
    EVENT_INTERFACE_NLIST   = 581,
    EVENT_INTERFACE_NOK     = 582,
    EVENT_INTERFACE_NCANCEL = 583,
    EVENT_INTERFACE_NDELETE = 584,
    EVENT_INTERFACE_NLABEL  = 585,

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
    EVENT_OBJECT_BXXXX      = 1063,
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
    EVENT_OBJECT_SEARCH     = 1200,
    EVENT_OBJECT_TERRAFORM  = 1201,
    EVENT_OBJECT_FIRE       = 1202,
    EVENT_OBJECT_FIREANT    = 1203,
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
    EVENT_OBJECT_PROGLIST   = 1310,
    EVENT_OBJECT_PROGRUN    = 1311,
    EVENT_OBJECT_PROGEDIT   = 1312,
    EVENT_OBJECT_PROGSTART  = 1313,
    EVENT_OBJECT_PROGSTOP   = 1314,
    EVENT_OBJECT_INFOOK     = 1340,
    EVENT_OBJECT_DELETE     = 1350,
    EVENT_OBJECT_GENERGY    = 1360,
    EVENT_OBJECT_GSHIELD    = 1361,
    EVENT_OBJECT_GRANGE     = 1362,
    EVENT_OBJECT_COMPASS    = 1363,
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
    EVENT_OBJECT_SHORTCUT00 = 1500,
    EVENT_OBJECT_SHORTCUT01 = 1501,
    EVENT_OBJECT_SHORTCUT02 = 1502,
    EVENT_OBJECT_SHORTCUT03 = 1503,
    EVENT_OBJECT_SHORTCUT04 = 1504,
    EVENT_OBJECT_SHORTCUT05 = 1505,
    EVENT_OBJECT_SHORTCUT06 = 1506,
    EVENT_OBJECT_SHORTCUT07 = 1507,
    EVENT_OBJECT_SHORTCUT08 = 1508,
    EVENT_OBJECT_SHORTCUT09 = 1509,
    EVENT_OBJECT_SHORTCUT10 = 1510,
    EVENT_OBJECT_SHORTCUT11 = 1511,
    EVENT_OBJECT_SHORTCUT12 = 1512,
    EVENT_OBJECT_SHORTCUT13 = 1513,
    EVENT_OBJECT_SHORTCUT14 = 1514,
    EVENT_OBJECT_SHORTCUT15 = 1515,
    EVENT_OBJECT_SHORTCUT16 = 1516,
    EVENT_OBJECT_SHORTCUT17 = 1517,
    EVENT_OBJECT_SHORTCUT18 = 1518,
    EVENT_OBJECT_SHORTCUT19 = 1519,
    EVENT_OBJECT_MOVIELOCK  = 1550,
    EVENT_OBJECT_EDITLOCK   = 1551,
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

    EVENT_USER              = 10000,
    EVENT_FORCE_DWORD       = 0x7fffffff
};

typedef struct
{
    EventMsg    event;      // event (EVENT *)
    long        param;      // parameter
    FPOINT      pos;        // mouse position (0 .. 1)
    float       axeX;       // control the X axis (-1 .. 1)
    float       axeY;       // control of the Y axis (-1 .. 1)
    float       axeZ;       // control the Z axis (-1 .. 1)
    short       keyState;   // state of the keyboard (KS_ *)
    float       rTime;      // relative time
}
Event;


#define VK_BUTTON1      (0x100+1)   // joystick button 1
#define VK_BUTTON2      (0x100+2)   // joystick button 2
#define VK_BUTTON3      (0x100+3)   // joystick button 3
#define VK_BUTTON4      (0x100+4)   // joystick button 4
#define VK_BUTTON5      (0x100+5)   // joystick button 5
#define VK_BUTTON6      (0x100+6)   // joystick button 6
#define VK_BUTTON7      (0x100+7)   // joystick button 7
#define VK_BUTTON8      (0x100+8)   // joystick button 8
#define VK_BUTTON9      (0x100+9)   // joystick button 9
#define VK_BUTTON10     (0x100+10)  // joystick button 10
#define VK_BUTTON11     (0x100+11)  // joystick button 11
#define VK_BUTTON12     (0x100+12)  // joystick button 12
#define VK_BUTTON13     (0x100+13)  // joystick button 13
#define VK_BUTTON14     (0x100+14)  // joystick button 14
#define VK_BUTTON15     (0x100+15)  // joystick button 15
#define VK_BUTTON16     (0x100+16)  // joystick button 16
#define VK_BUTTON17     (0x100+17)  // joystick button 17
#define VK_BUTTON18     (0x100+18)  // joystick button 18
#define VK_BUTTON19     (0x100+19)  // joystick button 19
#define VK_BUTTON20     (0x100+20)  // joystick button 20
#define VK_BUTTON21     (0x100+21)  // joystick button 21
#define VK_BUTTON22     (0x100+22)  // joystick button 22
#define VK_BUTTON23     (0x100+23)  // joystick button 23
#define VK_BUTTON24     (0x100+24)  // joystick button 24
#define VK_BUTTON25     (0x100+25)  // joystick button 25
#define VK_BUTTON26     (0x100+26)  // joystick button 26
#define VK_BUTTON27     (0x100+27)  // joystick button 27
#define VK_BUTTON28     (0x100+28)  // joystick button 28
#define VK_BUTTON29     (0x100+29)  // joystick button 29
#define VK_BUTTON30     (0x100+30)  // joystick button 30
#define VK_BUTTON31     (0x100+31)  // joystick button 31
#define VK_BUTTON32     (0x100+32)  // joystick button 32

#define VK_WHEELUP      (0x200+1)   // Mousewheel up
#define VK_WHEELDOWN    (0x200+2)   // Mousewheel down


enum KeyRank
{
    KEYRANK_LEFT    = 0,
    KEYRANK_RIGHT   = 1,
    KEYRANK_UP      = 2,
    KEYRANK_DOWN    = 3,
    KEYRANK_GUP     = 4,
    KEYRANK_GDOWN   = 5,
    KEYRANK_CAMERA  = 6,
    KEYRANK_DESEL   = 7,
    KEYRANK_ACTION  = 8,
    KEYRANK_NEAR    = 9,
    KEYRANK_AWAY    = 10,
    KEYRANK_NEXT    = 11,
    KEYRANK_HUMAN   = 12,
    KEYRANK_QUIT    = 13,
    KEYRANK_HELP    = 14,
    KEYRANK_PROG    = 15,
    KEYRANK_VISIT   = 16,
    KEYRANK_SPEED10 = 17,
    KEYRANK_SPEED15 = 18,
    KEYRANK_SPEED20 = 19,
    KEYRANK_SPEED30 = 20,
    KEYRANK_AIMUP   = 21,
    KEYRANK_AIMDOWN = 22,
    KEYRANK_CBOT    = 23,
};



class CEvent
{
public:
    CEvent(CInstanceManager* iMan);
    ~CEvent();

    void    Flush();
    void    MakeEvent(Event &event, EventMsg msg);
    BOOL    AddEvent(const Event &event);
    BOOL    GetEvent(Event &event);

protected:

protected:
    CInstanceManager* m_iMan;

    Event   m_fifo[MAXEVENT];
    int     m_head;
    int     m_tail;
    int     m_total;
};


#endif //_EVENT_H_
