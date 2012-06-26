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

// restext.h

#ifndef _RESTEXT_H_
#define _RESTEXT_H_


#define STRICT
#define D3D_OVERLOADS


#include "d3dengine.h"
#include "event.h"




// Possible types of the text resources.

enum ResType
{
    RES_TEXT        = 0,    // RT_*
    RES_EVENT       = 1,    // EVENT_*  (EventMsg)
    RES_OBJECT      = 2,    // OBJECT_* (ObjectType)
    RES_ERR         = 3,    // ERR_*    (Error)
    RES_KEY         = 4,    // VK_*     (keys)
    RES_CBOT        = 5,    // TX_*     (cbot.dll)
};


// Resources of type RES_TEXT.

#define RT_VERSION_ID       1
#define RT_DISINFO_TITLE    2
#define RT_WINDOW_MAXIMIZED 3
#define RT_WINDOW_MINIMIZED 4
#define RT_WINDOW_STANDARD  5
#define RT_WINDOW_CLOSE     6

#define RT_STUDIO_TITLE     10
#define RT_SCRIPT_NEW       20
#define RT_NAME_DEFAULT     21
#define RT_IO_NEW           22
#define RT_KEY_OR           23

#define RT_TITLE_BASE       40
#define RT_TITLE_INIT       41
#define RT_TITLE_TRAINER    42
#define RT_TITLE_DEFI       43
#define RT_TITLE_MISSION    44
#define RT_TITLE_FREE       45
#define RT_TITLE_PROTO      46
#define RT_TITLE_SETUP      47
#define RT_TITLE_NAME       48
#define RT_TITLE_PERSO      49
#define RT_TITLE_WRITE      50
#define RT_TITLE_READ       51
#define RT_TITLE_USER       52
#define RT_TITLE_TEEN       53

#define RT_PLAY_CHAPt       60
#define RT_PLAY_CHAPd       61
#define RT_PLAY_CHAPm       62
#define RT_PLAY_CHAPf       63
#define RT_PLAY_CHAPp       64
#define RT_PLAY_LISTt       65
#define RT_PLAY_LISTd       66
#define RT_PLAY_LISTm       67
#define RT_PLAY_LISTf       68
#define RT_PLAY_LISTp       69
#define RT_PLAY_RESUME      70
#define RT_PLAY_CHAPu       71
#define RT_PLAY_LISTu       72
#define RT_PLAY_CHAPte      73
#define RT_PLAY_LISTk       74

#define RT_SETUP_DEVICE     80
#define RT_SETUP_MODE       81
#define RT_SETUP_KEY1       82
#define RT_SETUP_KEY2       83

#define RT_PERSO_FACE       90
#define RT_PERSO_GLASSES    91
#define RT_PERSO_HAIR       92
#define RT_PERSO_COMBI      93
#define RT_PERSO_BAND       94

#define RT_DIALOG_TITLE     100
#define RT_DIALOG_ABORT     101
#define RT_DIALOG_QUIT      102
#define RT_DIALOG_YES       103
#define RT_DIALOG_NO        104
#define RT_DIALOG_DELOBJ    105
#define RT_DIALOG_DELGAME   106
#define RT_DIALOG_YESDEL    107
#define RT_DIALOG_NODEL     108
#define RT_DIALOG_LOADING   109
#define RT_DIALOG_YESQUIT   110
#define RT_DIALOG_NOQUIT    111

#define RT_STUDIO_LISTTT    120
#define RT_STUDIO_COMPOK    121
#define RT_STUDIO_PROGSTOP  122

#define RT_SATCOM_LIST      140
#define RT_SATCOM_BOT       141
#define RT_SATCOM_BUILDING  142
#define RT_SATCOM_FRET      143
#define RT_SATCOM_ALIEN     144
#define RT_SATCOM_NULL      145
#define RT_SATCOM_ERROR1    146
#define RT_SATCOM_ERROR2    147

#define RT_IO_OPEN          150
#define RT_IO_SAVE          151
#define RT_IO_LIST          152
#define RT_IO_NAME          153
#define RT_IO_DIR           154
#define RT_IO_PRIVATE       155
#define RT_IO_PUBLIC        156

#define RT_GENERIC_DEV1     170
#define RT_GENERIC_DEV2     171
#define RT_GENERIC_EDIT1    172
#define RT_GENERIC_EDIT2    173

#define RT_INTERFACE_REC    180

#define RT_MESSAGE_WIN      200
#define RT_MESSAGE_LOST     201


static CD3DEngine*  g_engine = 0;
static char         g_gamerName[100];

extern void     SetEngine(CD3DEngine *engine);
extern void     SetGlobalGamerName(char *name);
extern BOOL     SearchKey(char *cmd, KeyRank &key);
extern void     PutKeyName(char* dst, char* src);
extern BOOL     GetResource(ResType type, int num, char* text);
extern BOOL     GetResourceBase(ResType type, int num, char* text);


#endif //_RESTEXT_H_
