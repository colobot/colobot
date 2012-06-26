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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// restext.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdio.h>
#include "struct.h"
#include "d3dengine.h"
#include "language.h"
#include "misc.h"
#include "event.h"
#include "object.h"
#include "CBot/resource.h"
#include "restext.h"



//** -> text to translate!



// Gives the pointer to the engine.

void SetEngine(CD3DEngine *engine)
{
    g_engine = engine;
}

// Give the player's name.

void SetGlobalGamerName(char *name)
{
    strcpy(g_gamerName, name);
}



typedef struct
{
    KeyRank     key;
    char        name[20];
}
KeyDesc;

static KeyDesc keyTable[22] =
{
    { KEYRANK_LEFT,     "left;"    },
    { KEYRANK_RIGHT,    "right;"   },
    { KEYRANK_UP,       "up;"      },
    { KEYRANK_DOWN,     "down;"    },
    { KEYRANK_GUP,      "gup;"     },
    { KEYRANK_GDOWN,    "gdown;"   },
    { KEYRANK_CAMERA,   "camera;"  },
    { KEYRANK_DESEL,    "desel;"   },
    { KEYRANK_ACTION,   "action;"  },
    { KEYRANK_NEAR,     "near;"    },
    { KEYRANK_AWAY,     "away;"    },
    { KEYRANK_NEXT,     "next;"    },
    { KEYRANK_HUMAN,    "human;"   },
    { KEYRANK_QUIT,     "quit;"    },
    { KEYRANK_HELP,     "help;"    },
    { KEYRANK_PROG,     "prog;"    },
    { KEYRANK_CBOT,     "cbot;"    },
    { KEYRANK_VISIT,    "visit;"   },
    { KEYRANK_SPEED10,  "speed10;" },
    { KEYRANK_SPEED15,  "speed15;" },
    { KEYRANK_SPEED20,  "speed20;" },
    { KEYRANK_SPEED30,  "speed30;" },
};

// Seeks a key.

BOOL SearchKey(char *cmd, KeyRank &key)
{
    int     i;

    for ( i=0 ; i<22 ; i++ )
    {
        if ( strstr(cmd, keyTable[i].name) == cmd )
        {
            key = keyTable[i].key;
            return TRUE;
        }
    }
    return FALSE;
}

// Replaces the commands "\key name;" in a text.

void PutKeyName(char* dst, char* src)
{
    KeyRank key;
    char    name[50];
    int     s, d, n, res;

    s = d = 0;
    while ( src[s] != 0 )
    {
        if ( src[s+0] == '\\' &&
             src[s+1] == 'k'  &&
             src[s+2] == 'e'  &&
             src[s+3] == 'y'  &&
             src[s+4] == ' '  )
        {
            if ( SearchKey(src+s+5, key) )
            {
                res = g_engine->RetKey(key, 0);
                if ( res != 0 )
                {
                    if ( GetResource(RES_KEY, res, name) )
                    {
                        n = 0;
                        while ( name[n] != 0 )
                        {
                            dst[d++] = name[n++];
                        }
                        while ( src[s++] != ';' );
                        continue;
                    }
                }
            }
        }

        dst[d++] = src[s++];
    }
    dst[d++] = 0;
}


// Returns the text of a resource.

BOOL GetResource(ResType type, int num, char* text)
{
    char    buffer[100];

    if ( !GetResourceBase(type, num, buffer) )
    {
        text[0] = 0;
        return FALSE;
    }

    PutKeyName(text, buffer);
    return TRUE;
}


// Returns the text of a resource.

BOOL GetResourceBase(ResType type, int num, char* text)
{
    text[0] = 0;

#if _ENGLISH
    if ( type == RES_TEXT )
    {
        #if _FULL
        if ( num == RT_VERSION_ID          )  strcpy(text, "1.18 /e");
        #endif
        #if _NET
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A 1.18");
        #endif
        #if _SCHOOL & _EDU
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen EDU 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A EDU 1.18");
        #endif
        #endif
        #if _SCHOOL & _PERSO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen PERSO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A PERSO 1.18");
        #endif
        #endif
        #if _SCHOOL & _CEEBOTDEMO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen DEMO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A DEMO 1.18");
        #endif
        #endif
        #if _DEMO
        if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.18 /e");
        #endif
        if ( num == RT_DISINFO_TITLE       )  strcpy(text, "SatCom");
        if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximize");
        if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimize");
        if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normal size");
        if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Close");

        if ( num == RT_STUDIO_TITLE        )  strcpy(text, "Program editor");
        if ( num == RT_SCRIPT_NEW          )  strcpy(text, "New");
        if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Player");
        if ( num == RT_IO_NEW              )  strcpy(text, "New ...");
        if ( num == RT_KEY_OR              )  strcpy(text, " or ");

#if _NEWLOOK
        if ( num == RT_TITLE_BASE          )  strcpy(text, "CeeBot");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "CeeBot");
#else
        if ( num == RT_TITLE_BASE          )  strcpy(text, "COLOBOT");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "COLOBOT");
#endif
        if ( num == RT_TITLE_TRAINER       )  strcpy(text, "Programming exercises");
        if ( num == RT_TITLE_DEFI          )  strcpy(text, "Challenges");
        if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missions");
        if ( num == RT_TITLE_FREE          )  strcpy(text, "Free game");
        if ( num == RT_TITLE_TEEN          )  strcpy(text, "Free game");
        if ( num == RT_TITLE_USER          )  strcpy(text, "User levels");
        if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypes");
        if ( num == RT_TITLE_SETUP         )  strcpy(text, "Options");
        if ( num == RT_TITLE_NAME          )  strcpy(text, "Player's name");
        if ( num == RT_TITLE_PERSO         )  strcpy(text, "Customize your appearance");
        if ( num == RT_TITLE_WRITE         )  strcpy(text, "Save the current mission");
        if ( num == RT_TITLE_READ          )  strcpy(text, "Load a saved mission");

        if ( num == RT_PLAY_CHAPt          )  strcpy(text, " Chapters:");
        if ( num == RT_PLAY_CHAPd          )  strcpy(text, " Chapters:");
        if ( num == RT_PLAY_CHAPm          )  strcpy(text, " Planets:");
        if ( num == RT_PLAY_CHAPf          )  strcpy(text, " Planets:");
        if ( num == RT_PLAY_CHAPu          )  strcpy(text, " User levels:");
        if ( num == RT_PLAY_CHAPp          )  strcpy(text, " Planets:");
        if ( num == RT_PLAY_CHAPte         )  strcpy(text, " Chapters:");
        if ( num == RT_PLAY_LISTt          )  strcpy(text, " Exercises in the chapter:");
        if ( num == RT_PLAY_LISTd          )  strcpy(text, " Challenges in the chapter:");
        if ( num == RT_PLAY_LISTm          )  strcpy(text, " Missions on this planet:");
        if ( num == RT_PLAY_LISTf          )  strcpy(text, " Free game on this planet:");
        if ( num == RT_PLAY_LISTu          )  strcpy(text, " Missions on this level:");
        if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototypes on this planet:");
        if ( num == RT_PLAY_LISTk          )  strcpy(text, " Free game on this chapter:");
        if ( num == RT_PLAY_RESUME         )  strcpy(text, " Summary:");

        if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Drivers:");
        if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolution:");
        if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) First click on the key you want to redefine.");
        if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Then press the key you want to use instead.");

        if ( num == RT_PERSO_FACE          )  strcpy(text, "Face type:");
        if ( num == RT_PERSO_GLASSES       )  strcpy(text, "Eyeglasses:");
        if ( num == RT_PERSO_HAIR          )  strcpy(text, "Hair color:");
        if ( num == RT_PERSO_COMBI         )  strcpy(text, "Suit color:");
        if ( num == RT_PERSO_BAND          )  strcpy(text, "Strip color:");

#if _NEWLOOK
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Do you want to quit CeeBot ?");
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "CeeBot");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quit\\Quit CeeBot");
#else
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Do you want to quit COLOBOT ?");
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "COLOBOT");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quit\\Quit COLOBOT");
#endif
        if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Quit the mission?");
        if ( num == RT_DIALOG_YES          )  strcpy(text, "Abort\\Abort the current mission");
        if ( num == RT_DIALOG_NO           )  strcpy(text, "Continue\\Continue the current mission");
        if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continue\\Continue the game");
        if ( num == RT_DIALOG_DELOBJ       )  strcpy(text, "Do you really want to destroy the selected building?");
        if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Do you want to delete %s's saved games? ");
        if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Delete");
        if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancel");
        if ( num == RT_DIALOG_LOADING      )  strcpy(text, "LOADING");

        if ( num == RT_STUDIO_LISTTT       )  strcpy(text, "Keyword help(\\key cbot;)");
        if ( num == RT_STUDIO_COMPOK       )  strcpy(text, "Compilation ok (0 errors)");
        if ( num == RT_STUDIO_PROGSTOP     )  strcpy(text, "Program finished");

        if ( num == RT_SATCOM_LIST         )  strcpy(text, "\\b;List of objects\n");
        if ( num == RT_SATCOM_BOT          )  strcpy(text, "\\b;Robots\n");
        if ( num == RT_SATCOM_BUILDING     )  strcpy(text, "\\b;Buildings\n");
        if ( num == RT_SATCOM_FRET         )  strcpy(text, "\\b;Moveable objects\n");
        if ( num == RT_SATCOM_ALIEN        )  strcpy(text, "\\b;Aliens\n");
        if ( num == RT_SATCOM_NULL         )  strcpy(text, "\\c; (none)\\n;\n");
        if ( num == RT_SATCOM_ERROR1       )  strcpy(text, "\\b;Error\n");
        if ( num == RT_SATCOM_ERROR2       )  strcpy(text, "The list is only available if a \\l;radar station\\u object\\radar; is working.\n");

        if ( num == RT_IO_OPEN             )  strcpy(text, "Open");
        if ( num == RT_IO_SAVE             )  strcpy(text, "Save");
        if ( num == RT_IO_LIST             )  strcpy(text, "Folder: %s");
        if ( num == RT_IO_NAME             )  strcpy(text, "Name:");
        if ( num == RT_IO_DIR              )  strcpy(text, "Folder:");
        if ( num == RT_IO_PRIVATE          )  strcpy(text, "Private\\Private folder");
        if ( num == RT_IO_PUBLIC           )  strcpy(text, "Public\\Common folder");

        if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Developed by :");
        if ( num == RT_GENERIC_DEV2        )  strcpy(text, "www.epsitec.com");
//?     if ( num == RT_GENERIC_EDIT1       )  strcpy(text, "English version published by:");
//?     if ( num == RT_GENERIC_EDIT2       )  strcpy(text, "www.?.com");
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, " ");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, " ");

        if ( num == RT_INTERFACE_REC       )  strcpy(text, "Recorder");
    }

    if ( type == RES_EVENT )
    {
        if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
        if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancel");
        if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Next");
        if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Previous");
        if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

        if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
        if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancel");

        if ( num == EVENT_INTERFACE_TRAINER)  strcpy(text, "Exercises\\Programming exercises");
        if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Challenges\\Programming challenges");
        if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missions\\Select mission");
        if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Free game\\Free game without a specific goal");
        if ( num == EVENT_INTERFACE_TEEN   )  strcpy(text, "Free game\\Free game without a specific goal");
        if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "User\\User levels");
        if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypes under development");
        if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "New player\\Choose player's name");
        if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Options\\Preferences");
        if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Restart\\Restart the mission from the beginning");
        if ( num == EVENT_INTERFACE_WRITE  )  strcpy(text, "Save\\Save the current mission ");
        if ( num == EVENT_INTERFACE_READ   )  strcpy(text, "Load\\Load a saved mission");
#if _NEWLOOK
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Return to CeeBot");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quit\\Quit CeeBot");
#else
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Return to COLOBOT");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quit\\Quit COLOBOT");
#endif
        if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "<<  Back  \\Back to the previous screen");
        if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Play\\Start mission!");
        if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Device\\Driver and resolution settings");
        if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Graphics\\Graphics settings");
        if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Game\\Game settings");
        if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controls\\Keyboard, joystick and mouse settings");
        if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sound\\Music and game sound volume");
        if ( num == EVENT_INTERFACE_DEVICE )  strcpy(text, "Unit");
        if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolution");
        if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Full screen\\Full screen or window mode");
        if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Apply changes\\Activates the changed settings");

        if ( num == EVENT_INTERFACE_TOTO   )  strcpy(text, "Robbie\\Your assistant");
        if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Shadows\\Shadows on the ground");
        if ( num == EVENT_INTERFACE_GROUND )  strcpy(text, "Marks on the ground\\Marks on the ground");
        if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Dust\\Dust and dirt on bots and buildings");
        if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Fog\\Fog");
        if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Sunbeams\\Sunbeams in the sky");
        if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Sky\\Clouds and nebulae");
        if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planets and stars\\Astronomical objects in the sky");
        if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Dynamic lighting\\Mobile light sources");
        if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Number of particles\\Explosions, dust, reflections, etc.");
        if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Depth of field\\Maximum visibility");
        if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Details\\Visual quality of 3D objects");
        if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Textures\\Quality of textures ");
        if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Num of decorative objects\\Number of purely ornamental objects");
        if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Particles in the interface\\Steam clouds and sparks in the interface");
        if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Reflections on the buttons \\Shiny buttons");
        if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Help balloons\\Explain the function of the buttons");
        if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Film sequences\\Films before and after the missions");
        if ( num == EVENT_INTERFACE_NICERST)  strcpy(text, "Exit film\\Film at the exit of exercises");
        if ( num == EVENT_INTERFACE_HIMSELF)  strcpy(text, "Friendly fire\\Your shooting can damage your own objects ");
        if ( num == EVENT_INTERFACE_SCROLL )  strcpy(text, "Scrolling\\Scrolling when the mouse touches right or left border");
        if ( num == EVENT_INTERFACE_INVERTX)  strcpy(text, "Mouse inversion X\\Inversion of the scrolling direction on the X axis");
        if ( num == EVENT_INTERFACE_INVERTY)  strcpy(text, "Mouse inversion Y\\Inversion of the scrolling direction on the Y axis");
        if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Quake at explosions\\The screen shakes at explosions");
        if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Mouse shadow\\Gives the mouse a shadow");
        if ( num == EVENT_INTERFACE_EDITMODE) strcpy(text, "Automatic indent\\When program editing");
        if ( num == EVENT_INTERFACE_EDITVALUE)strcpy(text, "Big indent\\Indent 2 or 4 spaces per level defined by braces");
        if ( num == EVENT_INTERFACE_SOLUCE4)  strcpy(text, "Access to solutions\\Show program \"4: Solution\" in the exercises");  //**

        if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Standard controls\\Standard key functions");
        if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Turn left\\turns the bot to the left");
        if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Turn right\\turns the bot to the right");
        if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Forward\\Moves forward");
        if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Backward\\Moves backward");
        if ( num == EVENT_INTERFACE_KGUP   )  strcpy(text, "Climb\\Increases the power of the jet");
        if ( num == EVENT_INTERFACE_KGDOWN )  strcpy(text, "Descend\\Reduces the power of the jet");
        if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Change camera\\Switches between onboard camera and following camera");
        if ( num == EVENT_INTERFACE_KDESEL )  strcpy(text, "Previous object\\Selects the previous object");
        if ( num == EVENT_INTERFACE_KACTION)  strcpy(text, "Standard action\\Standard action of the bot (take/grab, shoot, sniff, etc)");
        if ( num == EVENT_INTERFACE_KNEAR  )  strcpy(text, "Camera closer\\Moves the camera forward");
        if ( num == EVENT_INTERFACE_KAWAY  )  strcpy(text, "Camera back\\Moves the camera backward");
        if ( num == EVENT_INTERFACE_KNEXT  )  strcpy(text, "Next object\\Selects the next object");
        if ( num == EVENT_INTERFACE_KHUMAN )  strcpy(text, "Select the astronaut\\Selects the astronaut");
        if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Quit\\Quit the current mission or exercise");
        if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instructions\\Shows the instructions for the current mission");
        if ( num == EVENT_INTERFACE_KPROG  )  strcpy(text, "Programming help\\Gives more detailed help with programming");
        if ( num == EVENT_INTERFACE_KCBOT  )  strcpy(text, "Key word help\\More detailed help about key words");
        if ( num == EVENT_INTERFACE_KVISIT )  strcpy(text, "Origin of last message\\Shows where the last message was sent from");
        if ( num == EVENT_INTERFACE_KSPEED10) strcpy(text, "Speed 1.0x\\Normal speed");
        if ( num == EVENT_INTERFACE_KSPEED15) strcpy(text, "Speed 1.5x\\1.5 times faster");
        if ( num == EVENT_INTERFACE_KSPEED20) strcpy(text, "Speed 2.0x\\Double speed");
        if ( num == EVENT_INTERFACE_KSPEED30) strcpy(text, "Speed 3.0x\\Three times faster");

        if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Sound effects:\\Volume of engines, voice, shooting, etc.");
        if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Background sound :\\Volume of audio tracks on the CD");
        if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "3D sound\\3D positioning of the sound");

        if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Lowest\\Minimum graphic quality (highest frame rate)");
        if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Normal graphic quality");
        if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Highest\\Highest graphic quality (lowest frame rate)");

        if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Mute\\No sound");
        if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Normal sound volume");

        if ( num == EVENT_INTERFACE_JOYSTICK) strcpy(text, "Use a joystick\\Joystick or keyboard");
        if ( num == EVENT_INTERFACE_SOLUCE )  strcpy(text, "Access to solution\\Shows the solution (detailed instructions for missions)");

        if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\New player name");
        if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "OK\\Choose the selected player");
        if ( num == EVENT_INTERFACE_NCANCEL)  strcpy(text, "Cancel\\Keep current player name");
        if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Delete player\\Deletes the player from the list");
        if ( num == EVENT_INTERFACE_NLABEL )  strcpy(text, "Player name");

        if ( num == EVENT_INTERFACE_IOWRITE)  strcpy(text, "Save\\Saves the current mission");
        if ( num == EVENT_INTERFACE_IOREAD )  strcpy(text, "Load\\Loads the selected mission");
        if ( num == EVENT_INTERFACE_IOLIST )  strcpy(text, "List of saved missions");
        if ( num == EVENT_INTERFACE_IOLABEL)  strcpy(text, "Filename:");
        if ( num == EVENT_INTERFACE_IONAME )  strcpy(text, "Mission name");
        if ( num == EVENT_INTERFACE_IOIMAGE)  strcpy(text, "Photography");
        if ( num == EVENT_INTERFACE_IODELETE) strcpy(text, "Delete\\Deletes the selected file");

        if ( num == EVENT_INTERFACE_PERSO  )  strcpy(text, "Appearance\\Choose your appearance");
        if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "OK");
        if ( num == EVENT_INTERFACE_PCANCEL)  strcpy(text, "Cancel");
        if ( num == EVENT_INTERFACE_PDEF   )  strcpy(text, "Standard\\Standard appearance settings");
        if ( num == EVENT_INTERFACE_PHEAD  )  strcpy(text, "Head\\Face and hair");
        if ( num == EVENT_INTERFACE_PBODY  )  strcpy(text, "Suit\\Astronaut suit");
        if ( num == EVENT_INTERFACE_PLROT  )  strcpy(text, "\\Turn left");
        if ( num == EVENT_INTERFACE_PRROT  )  strcpy(text, "\\Turn right");
        if ( num == EVENT_INTERFACE_PCRa   )  strcpy(text, "Red");
        if ( num == EVENT_INTERFACE_PCGa   )  strcpy(text, "Green");
        if ( num == EVENT_INTERFACE_PCBa   )  strcpy(text, "Blue");
        if ( num == EVENT_INTERFACE_PCRb   )  strcpy(text, "Red");
        if ( num == EVENT_INTERFACE_PCGb   )  strcpy(text, "Green");
        if ( num == EVENT_INTERFACE_PCBb   )  strcpy(text, "Blue");
        if ( num == EVENT_INTERFACE_PFACE1 )  strcpy(text, "\\Face 1");
        if ( num == EVENT_INTERFACE_PFACE2 )  strcpy(text, "\\Face 4");
        if ( num == EVENT_INTERFACE_PFACE3 )  strcpy(text, "\\Face 3");
        if ( num == EVENT_INTERFACE_PFACE4 )  strcpy(text, "\\Face 2");
        if ( num == EVENT_INTERFACE_PGLASS0)  strcpy(text, "\\No eyeglasses");
        if ( num == EVENT_INTERFACE_PGLASS1)  strcpy(text, "\\Eyeglasses 1");
        if ( num == EVENT_INTERFACE_PGLASS2)  strcpy(text, "\\Eyeglasses 2");
        if ( num == EVENT_INTERFACE_PGLASS3)  strcpy(text, "\\Eyeglasses 3");
        if ( num == EVENT_INTERFACE_PGLASS4)  strcpy(text, "\\Eyeglasses 4");
        if ( num == EVENT_INTERFACE_PGLASS5)  strcpy(text, "\\Eyeglasses 5");

        if ( num == EVENT_OBJECT_DESELECT  )  strcpy(text, "Previous selection (\\key desel;)");
        if ( num == EVENT_OBJECT_LEFT      )  strcpy(text, "Turn left (\\key left;)");
        if ( num == EVENT_OBJECT_RIGHT     )  strcpy(text, "Turn right (\\key right;)");
        if ( num == EVENT_OBJECT_UP        )  strcpy(text, "Forward (\\key up;)");
        if ( num == EVENT_OBJECT_DOWN      )  strcpy(text, "Backward (\\key down;)");
        if ( num == EVENT_OBJECT_GASUP     )  strcpy(text, "Up (\\key gup;)");
        if ( num == EVENT_OBJECT_GASDOWN   )  strcpy(text, "Down (\\key gdown;)");
        if ( num == EVENT_OBJECT_HTAKE     )  strcpy(text, "Grab or drop (\\key action;)");
        if ( num == EVENT_OBJECT_MTAKE     )  strcpy(text, "Grab or drop (\\key action;)");
        if ( num == EVENT_OBJECT_MFRONT    )  strcpy(text, "..in front");
        if ( num == EVENT_OBJECT_MBACK     )  strcpy(text, "..behind");
        if ( num == EVENT_OBJECT_MPOWER    )  strcpy(text, "..power cell");
        if ( num == EVENT_OBJECT_BHELP     )  strcpy(text, "Instructions for the mission (\\key help;)");
        if ( num == EVENT_OBJECT_BTAKEOFF  )  strcpy(text, "Take off to finish the mission");
        if ( num == EVENT_OBJECT_BDERRICK  )  strcpy(text, "Build a derrick");
        if ( num == EVENT_OBJECT_BSTATION  )  strcpy(text, "Build a power station");
        if ( num == EVENT_OBJECT_BFACTORY  )  strcpy(text, "Build a bot factory");
        if ( num == EVENT_OBJECT_BREPAIR   )  strcpy(text, "Build a repair center");
        if ( num == EVENT_OBJECT_BCONVERT  )  strcpy(text, "Build a converter");
        if ( num == EVENT_OBJECT_BTOWER    )  strcpy(text, "Build a defense tower");
        if ( num == EVENT_OBJECT_BRESEARCH )  strcpy(text, "Build a research center");
        if ( num == EVENT_OBJECT_BRADAR    )  strcpy(text, "Build a radar station");
        if ( num == EVENT_OBJECT_BENERGY   )  strcpy(text, "Build a power cell factory");
        if ( num == EVENT_OBJECT_BLABO     )  strcpy(text, "Build an autolab");
        if ( num == EVENT_OBJECT_BNUCLEAR  )  strcpy(text, "Build a nuclear power plant");
        if ( num == EVENT_OBJECT_BPARA     )  strcpy(text, "Build a lightning conductor");
        if ( num == EVENT_OBJECT_BINFO     )  strcpy(text, "Build a exchange post");
        if ( num == EVENT_OBJECT_GFLAT     )  strcpy(text, "Show if the ground is flat");
        if ( num == EVENT_OBJECT_FCREATE   )  strcpy(text, "Plant a flag");
        if ( num == EVENT_OBJECT_FDELETE   )  strcpy(text, "Remove a flag");
        if ( num == EVENT_OBJECT_FCOLORb   )  strcpy(text, "\\Blue flags");
        if ( num == EVENT_OBJECT_FCOLORr   )  strcpy(text, "\\Red flags");
        if ( num == EVENT_OBJECT_FCOLORg   )  strcpy(text, "\\Green flags");
        if ( num == EVENT_OBJECT_FCOLORy   )  strcpy(text, "\\Yellow flags");
        if ( num == EVENT_OBJECT_FCOLORv   )  strcpy(text, "\\Violet flags");
        if ( num == EVENT_OBJECT_FACTORYfa )  strcpy(text, "Build a winged grabber");
        if ( num == EVENT_OBJECT_FACTORYta )  strcpy(text, "Build a tracked grabber");
        if ( num == EVENT_OBJECT_FACTORYwa )  strcpy(text, "Build a wheeled grabber");
        if ( num == EVENT_OBJECT_FACTORYia )  strcpy(text, "Build a legged grabber");
        if ( num == EVENT_OBJECT_FACTORYfc )  strcpy(text, "Build a winged shooter");
        if ( num == EVENT_OBJECT_FACTORYtc )  strcpy(text, "Build a tracked shooter");
        if ( num == EVENT_OBJECT_FACTORYwc )  strcpy(text, "Build a wheeled shooter");
        if ( num == EVENT_OBJECT_FACTORYic )  strcpy(text, "Build a legged shooter");
        if ( num == EVENT_OBJECT_FACTORYfi )  strcpy(text, "Build a winged orga shooter");
        if ( num == EVENT_OBJECT_FACTORYti )  strcpy(text, "Build a tracked orga shooter");
        if ( num == EVENT_OBJECT_FACTORYwi )  strcpy(text, "Build a wheeled orga shooter");
        if ( num == EVENT_OBJECT_FACTORYii )  strcpy(text, "Build a legged orga shooter");
        if ( num == EVENT_OBJECT_FACTORYfs )  strcpy(text, "Build a winged sniffer");
        if ( num == EVENT_OBJECT_FACTORYts )  strcpy(text, "Build a tracked sniffer");
        if ( num == EVENT_OBJECT_FACTORYws )  strcpy(text, "Build a wheeled sniffer");
        if ( num == EVENT_OBJECT_FACTORYis )  strcpy(text, "Build a legged sniffer");
        if ( num == EVENT_OBJECT_FACTORYrt )  strcpy(text, "Build a thumper");
        if ( num == EVENT_OBJECT_FACTORYrc )  strcpy(text, "Build a phazer shooter");
        if ( num == EVENT_OBJECT_FACTORYrr )  strcpy(text, "Build a recycler");
        if ( num == EVENT_OBJECT_FACTORYrs )  strcpy(text, "Build a shielder");
        if ( num == EVENT_OBJECT_FACTORYsa )  strcpy(text, "Build a subber");
        if ( num == EVENT_OBJECT_RTANK     )  strcpy(text, "Run research program for tracked bots");
        if ( num == EVENT_OBJECT_RFLY      )  strcpy(text, "Run research program for winged bots");
        if ( num == EVENT_OBJECT_RTHUMP    )  strcpy(text, "Run research program for thumper");
        if ( num == EVENT_OBJECT_RCANON    )  strcpy(text, "Run research program for shooter");
        if ( num == EVENT_OBJECT_RTOWER    )  strcpy(text, "Run research program for defense tower");
        if ( num == EVENT_OBJECT_RPHAZER   )  strcpy(text, "Run research program for phazer shooter");
        if ( num == EVENT_OBJECT_RSHIELD   )  strcpy(text, "Run research program for shielder");
        if ( num == EVENT_OBJECT_RATOMIC   )  strcpy(text, "Run research program for nuclear power");
        if ( num == EVENT_OBJECT_RiPAW     )  strcpy(text, "Run research program for legged bots");
        if ( num == EVENT_OBJECT_RiGUN     )  strcpy(text, "Run research program for orga shooter");
        if ( num == EVENT_OBJECT_RESET     )  strcpy(text, "Return to start");
        if ( num == EVENT_OBJECT_SEARCH    )  strcpy(text, "Sniff (\\key action;)");
        if ( num == EVENT_OBJECT_TERRAFORM )  strcpy(text, "Thump (\\key action;)");
        if ( num == EVENT_OBJECT_FIRE      )  strcpy(text, "Shoot (\\key action;)");
        if ( num == EVENT_OBJECT_RECOVER   )  strcpy(text, "Recycle (\\key action;)");
        if ( num == EVENT_OBJECT_BEGSHIELD )  strcpy(text, "Extend shield (\\key action;)");
        if ( num == EVENT_OBJECT_ENDSHIELD )  strcpy(text, "Withdraw shield (\\key action;)");
        if ( num == EVENT_OBJECT_DIMSHIELD )  strcpy(text, "Shield radius");
        if ( num == EVENT_OBJECT_PROGRUN   )  strcpy(text, "Execute the selected program");
        if ( num == EVENT_OBJECT_PROGEDIT  )  strcpy(text, "Edit the selected program");
        if ( num == EVENT_OBJECT_INFOOK    )  strcpy(text, "\\SatCom on standby");
        if ( num == EVENT_OBJECT_DELETE    )  strcpy(text, "Destroy the building");
        if ( num == EVENT_OBJECT_GENERGY   )  strcpy(text, "Energy level");
        if ( num == EVENT_OBJECT_GSHIELD   )  strcpy(text, "Shield level");
        if ( num == EVENT_OBJECT_GRANGE    )  strcpy(text, "Jet temperature");
        if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Still working ...");
        if ( num == EVENT_OBJECT_GRADAR    )  strcpy(text, "Number of insects detected");
        if ( num == EVENT_OBJECT_GINFO     )  strcpy(text, "Transmitted information");
        if ( num == EVENT_OBJECT_COMPASS   )  strcpy(text, "Compass");
//?     if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-map");
        if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom mini-map");
        if ( num == EVENT_OBJECT_CAMERA    )  strcpy(text, "Camera (\\key camera;)");
        if ( num == EVENT_OBJECT_CAMERAleft)  strcpy(text, "Camera to left");
        if ( num == EVENT_OBJECT_CAMERAright) strcpy(text, "Camera to right");
        if ( num == EVENT_OBJECT_CAMERAnear)  strcpy(text, "Camera nearest");
        if ( num == EVENT_OBJECT_CAMERAaway)  strcpy(text, "Camera awayest");
        if ( num == EVENT_OBJECT_HELP      )  strcpy(text, "Help about selected object");
        if ( num == EVENT_OBJECT_SOLUCE    )  strcpy(text, "Show the solution");
        if ( num == EVENT_OBJECT_SHORTCUT00)  strcpy(text, "Switch bots <-> buildings");
        if ( num == EVENT_OBJECT_LIMIT     )  strcpy(text, "Show the range");
        if ( num == EVENT_OBJECT_PEN0      )  strcpy(text, "\\Raise the pencil");
        if ( num == EVENT_OBJECT_PEN1      )  strcpy(text, "\\Use the black pencil");
        if ( num == EVENT_OBJECT_PEN2      )  strcpy(text, "\\Use the yellow pencil");
        if ( num == EVENT_OBJECT_PEN3      )  strcpy(text, "\\Use the orange pencil");
        if ( num == EVENT_OBJECT_PEN4      )  strcpy(text, "\\Use the red pencil");
        if ( num == EVENT_OBJECT_PEN5      )  strcpy(text, "\\Use the purple pencil");
        if ( num == EVENT_OBJECT_PEN6      )  strcpy(text, "\\Use the blue pencil");
        if ( num == EVENT_OBJECT_PEN7      )  strcpy(text, "\\Use the green pencil");
        if ( num == EVENT_OBJECT_PEN8      )  strcpy(text, "\\Use the brown pencil");
        if ( num == EVENT_OBJECT_REC       )  strcpy(text, "\\Start recording");
        if ( num == EVENT_OBJECT_STOP      )  strcpy(text, "\\Stop recording");
        if ( num == EVENT_DT_VISIT0        ||
             num == EVENT_DT_VISIT1        ||
             num == EVENT_DT_VISIT2        ||
             num == EVENT_DT_VISIT3        ||
             num == EVENT_DT_VISIT4        )  strcpy(text, "Show the place");
        if ( num == EVENT_DT_END           )  strcpy(text, "Continue");
        if ( num == EVENT_CMD              )  strcpy(text, "Command line");
        if ( num == EVENT_SPEED            )  strcpy(text, "Game speed");

        if ( num == EVENT_HYPER_PREV       )  strcpy(text, "Back");
        if ( num == EVENT_HYPER_NEXT       )  strcpy(text, "Forward");
        if ( num == EVENT_HYPER_HOME       )  strcpy(text, "Home");
        if ( num == EVENT_HYPER_COPY       )  strcpy(text, "Copy");
        if ( num == EVENT_HYPER_SIZE1      )  strcpy(text, "Size 1");
        if ( num == EVENT_HYPER_SIZE2      )  strcpy(text, "Size 2");
        if ( num == EVENT_HYPER_SIZE3      )  strcpy(text, "Size 3");
        if ( num == EVENT_HYPER_SIZE4      )  strcpy(text, "Size 4");
        if ( num == EVENT_HYPER_SIZE5      )  strcpy(text, "Size 5");
        if ( num == EVENT_SATCOM_HUSTON    )  strcpy(text, "Instructions from Houston");
#if _TEEN
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Dictionnary");
#else
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Satellite report");
#endif
        if ( num == EVENT_SATCOM_LOADING   )  strcpy(text, "Programs dispatched by Houston");
        if ( num == EVENT_SATCOM_OBJECT    )  strcpy(text, "List of objects");
        if ( num == EVENT_SATCOM_PROG      )  strcpy(text, "Programming help");
        if ( num == EVENT_SATCOM_SOLUCE    )  strcpy(text, "Solution");

        if ( num == EVENT_STUDIO_OK        )  strcpy(text, "OK\\Close program editor and return to game");
        if ( num == EVENT_STUDIO_CANCEL    )  strcpy(text, "Cancel\\Cancel all changes");
        if ( num == EVENT_STUDIO_NEW       )  strcpy(text, "New");
        if ( num == EVENT_STUDIO_OPEN      )  strcpy(text, "Open (Ctrl+o)");
        if ( num == EVENT_STUDIO_SAVE      )  strcpy(text, "Save (Ctrl+s)");
        if ( num == EVENT_STUDIO_UNDO      )  strcpy(text, "Undo (Ctrl+z)");
        if ( num == EVENT_STUDIO_CUT       )  strcpy(text, "Cut (Ctrl+x)");
        if ( num == EVENT_STUDIO_COPY      )  strcpy(text, "Copy (Ctrl+c)");
        if ( num == EVENT_STUDIO_PASTE     )  strcpy(text, "Paste (Ctrl+v)");
        if ( num == EVENT_STUDIO_SIZE      )  strcpy(text, "Font size");
        if ( num == EVENT_STUDIO_TOOL      )  strcpy(text, "Instructions (\\key help;)");
        if ( num == EVENT_STUDIO_HELP      )  strcpy(text, "Programming help  (\\key prog;)");
        if ( num == EVENT_STUDIO_COMPILE   )  strcpy(text, "Compile");
        if ( num == EVENT_STUDIO_RUN       )  strcpy(text, "Execute/stop");
        if ( num == EVENT_STUDIO_REALTIME  )  strcpy(text, "Pause/continue");
        if ( num == EVENT_STUDIO_STEP      )  strcpy(text, "One step");
    }

    if ( type == RES_OBJECT )
    {
        if ( num == OBJECT_PORTICO      )  strcpy(text, "Gantry crane");
        if ( num == OBJECT_BASE         )  strcpy(text, "Spaceship");
        if ( num == OBJECT_DERRICK      )  strcpy(text, "Derrick");
        if ( num == OBJECT_FACTORY      )  strcpy(text, "Bot factory");
        if ( num == OBJECT_REPAIR       )  strcpy(text, "Repair center");
        if ( num == OBJECT_DESTROYER    )  strcpy(text, "Destroyer");
        if ( num == OBJECT_STATION      )  strcpy(text, "Power station");
        if ( num == OBJECT_CONVERT      )  strcpy(text, "Converts ore to titanium");
        if ( num == OBJECT_TOWER        )  strcpy(text, "Defense tower");
        if ( num == OBJECT_NEST         )  strcpy(text, "Nest");
        if ( num == OBJECT_RESEARCH     )  strcpy(text, "Research center");
        if ( num == OBJECT_RADAR        )  strcpy(text, "Radar station");
        if ( num == OBJECT_INFO         )  strcpy(text, "Information exchange post");
#if _TEEN
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Power cell factory");
#else
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Power cell factory");
#endif
        if ( num == OBJECT_LABO         )  strcpy(text, "Autolab");
        if ( num == OBJECT_NUCLEAR      )  strcpy(text, "Nuclear power station");
        if ( num == OBJECT_PARA         )  strcpy(text, "Lightning conductor");
        if ( num == OBJECT_SAFE         )  strcpy(text, "Vault");
        if ( num == OBJECT_HUSTON       )  strcpy(text, "Houston Mission Control");
        if ( num == OBJECT_TARGET1      )  strcpy(text, "Target");
        if ( num == OBJECT_TARGET2      )  strcpy(text, "Target");
        if ( num == OBJECT_START        )  strcpy(text, "Start");
        if ( num == OBJECT_END          )  strcpy(text, "Finish");
        if ( num == OBJECT_STONE        )  strcpy(text, "Titanium ore");
        if ( num == OBJECT_URANIUM      )  strcpy(text, "Uranium ore");
        if ( num == OBJECT_BULLET       )  strcpy(text, "Organic matter");
        if ( num == OBJECT_METAL        )  strcpy(text, "Titanium");
        if ( num == OBJECT_POWER        )  strcpy(text, "Power cell");
        if ( num == OBJECT_ATOMIC       )  strcpy(text, "Nuclear power cell");
        if ( num == OBJECT_BBOX         )  strcpy(text, "Black box");
        if ( num == OBJECT_KEYa         )  strcpy(text, "Key A");
        if ( num == OBJECT_KEYb         )  strcpy(text, "Key B");
        if ( num == OBJECT_KEYc         )  strcpy(text, "Key C");
        if ( num == OBJECT_KEYd         )  strcpy(text, "Key D");
        if ( num == OBJECT_TNT          )  strcpy(text, "Explosive");
        if ( num == OBJECT_BOMB         )  strcpy(text, "Fixed mine");
        if ( num == OBJECT_BAG          )  strcpy(text, "Survival kit");
        if ( num == OBJECT_WAYPOINT     )  strcpy(text, "Checkpoint");
        if ( num == OBJECT_FLAGb        )  strcpy(text, "Blue flag");
        if ( num == OBJECT_FLAGr        )  strcpy(text, "Red flag");
        if ( num == OBJECT_FLAGg        )  strcpy(text, "Green flag");
        if ( num == OBJECT_FLAGy        )  strcpy(text, "Yellow flag");
        if ( num == OBJECT_FLAGv        )  strcpy(text, "Violet flag");
        if ( num == OBJECT_MARKPOWER    )  strcpy(text, "Energy deposit (site for power station)");
        if ( num == OBJECT_MARKURANIUM  )  strcpy(text, "Uranium deposit (site for derrick)");
        if ( num == OBJECT_MARKKEYa     )  strcpy(text, "Found key A (site for derrick)");
        if ( num == OBJECT_MARKKEYb     )  strcpy(text, "Found key B (site for derrick)");
        if ( num == OBJECT_MARKKEYc     )  strcpy(text, "Found key C (site for derrick)");
        if ( num == OBJECT_MARKKEYd     )  strcpy(text, "Found key D (site for derrick)");
        if ( num == OBJECT_MARKSTONE    )  strcpy(text, "Titanium deposit (site for derrick)");
        if ( num == OBJECT_MOBILEft     )  strcpy(text, "Practice bot");
        if ( num == OBJECT_MOBILEtt     )  strcpy(text, "Practice bot");
        if ( num == OBJECT_MOBILEwt     )  strcpy(text, "Practice bot");
        if ( num == OBJECT_MOBILEit     )  strcpy(text, "Practice bot");
        if ( num == OBJECT_MOBILEfa     )  strcpy(text, "Winged grabber");
        if ( num == OBJECT_MOBILEta     )  strcpy(text, "Tracked grabber");
        if ( num == OBJECT_MOBILEwa     )  strcpy(text, "Wheeled grabber");
        if ( num == OBJECT_MOBILEia     )  strcpy(text, "Legged grabber");
        if ( num == OBJECT_MOBILEfc     )  strcpy(text, "Winged shooter");
        if ( num == OBJECT_MOBILEtc     )  strcpy(text, "Tracked shooter");
        if ( num == OBJECT_MOBILEwc     )  strcpy(text, "Wheeled shooter");
        if ( num == OBJECT_MOBILEic     )  strcpy(text, "Legged shooter");
        if ( num == OBJECT_MOBILEfi     )  strcpy(text, "Winged orga shooter");
        if ( num == OBJECT_MOBILEti     )  strcpy(text, "Tracked orga shooter");
        if ( num == OBJECT_MOBILEwi     )  strcpy(text, "Wheeled orga shooter");
        if ( num == OBJECT_MOBILEii     )  strcpy(text, "Legged orga shooter");
        if ( num == OBJECT_MOBILEfs     )  strcpy(text, "Winged sniffer");
        if ( num == OBJECT_MOBILEts     )  strcpy(text, "Tracked sniffer");
        if ( num == OBJECT_MOBILEws     )  strcpy(text, "Wheeled sniffer");
        if ( num == OBJECT_MOBILEis     )  strcpy(text, "Legged sniffer");
        if ( num == OBJECT_MOBILErt     )  strcpy(text, "Thumper");
        if ( num == OBJECT_MOBILErc     )  strcpy(text, "Phazer shooter");
        if ( num == OBJECT_MOBILErr     )  strcpy(text, "Recycler");
        if ( num == OBJECT_MOBILErs     )  strcpy(text, "Shielder");
        if ( num == OBJECT_MOBILEsa     )  strcpy(text, "Subber");
        if ( num == OBJECT_MOBILEtg     )  strcpy(text, "Target bot");
        if ( num == OBJECT_MOBILEdr     )  strcpy(text, "Drawer bot");
        if ( num == OBJECT_HUMAN        )  strcpy(text, g_gamerName);
        if ( num == OBJECT_TECH         )  strcpy(text, "Engineer");
        if ( num == OBJECT_TOTO         )  strcpy(text, "Robbie");
        if ( num == OBJECT_MOTHER       )  strcpy(text, "Alien Queen");
        if ( num == OBJECT_ANT          )  strcpy(text, "Ant");
        if ( num == OBJECT_SPIDER       )  strcpy(text, "Spider");
        if ( num == OBJECT_BEE          )  strcpy(text, "Wasp");
        if ( num == OBJECT_WORM         )  strcpy(text, "Worm");
        if ( num == OBJECT_EGG          )  strcpy(text, "Egg");
        if ( num == OBJECT_RUINmobilew1 )  strcpy(text, "Wreckage");
        if ( num == OBJECT_RUINmobilew2 )  strcpy(text, "Wreckage");
        if ( num == OBJECT_RUINmobilet1 )  strcpy(text, "Wreckage");
        if ( num == OBJECT_RUINmobilet2 )  strcpy(text, "Wreckage");
        if ( num == OBJECT_RUINmobiler1 )  strcpy(text, "Wreckage");
        if ( num == OBJECT_RUINmobiler2 )  strcpy(text, "Wreckage");
        if ( num == OBJECT_RUINfactory  )  strcpy(text, "Ruin");
        if ( num == OBJECT_RUINdoor     )  strcpy(text, "Ruin");
        if ( num == OBJECT_RUINsupport  )  strcpy(text, "Waste");
        if ( num == OBJECT_RUINradar    )  strcpy(text, "Ruin");
        if ( num == OBJECT_RUINconvert  )  strcpy(text, "Ruin");
        if ( num == OBJECT_RUINbase     )  strcpy(text, "Spaceship ruin");
        if ( num == OBJECT_RUINhead     )  strcpy(text, "Spaceship ruin");
        if ( num == OBJECT_APOLLO1      ||
             num == OBJECT_APOLLO3      ||
             num == OBJECT_APOLLO4      ||
             num == OBJECT_APOLLO5      )  strcpy(text, "Remains of Apollo mission");
        if ( num == OBJECT_APOLLO2      )  strcpy(text, "Lunar Roving Vehicle");
    }

    if ( type == RES_ERR )
    {
        strcpy(text, "Error");
        if ( num == ERR_CMD             )  strcpy(text, "Unknown command");
#if _NEWLOOK
        if ( num == ERR_INSTALL         )  strcpy(text, "CeeBot not installed.");
        if ( num == ERR_NOCD            )  strcpy(text, "Please insert the CeeBot CD\nand re-run the game.");
#else
        if ( num == ERR_INSTALL         )  strcpy(text, "COLOBOT not installed.");
        if ( num == ERR_NOCD            )  strcpy(text, "Please insert the COLOBOT CD\nand re-run the game.");
#endif
        if ( num == ERR_MANIP_VEH       )  strcpy(text, "Inappropriate bot");
        if ( num == ERR_MANIP_FLY       )  strcpy(text, "Impossible when flying");
        if ( num == ERR_MANIP_BUSY      )  strcpy(text, "Already carrying something");
        if ( num == ERR_MANIP_NIL       )  strcpy(text, "Nothing to grab");
        if ( num == ERR_MANIP_MOTOR     )  strcpy(text, "Impossible when moving");
        if ( num == ERR_MANIP_OCC       )  strcpy(text, "Place occupied");
        if ( num == ERR_MANIP_FRIEND    )  strcpy(text, "No other robot");
        if ( num == ERR_MANIP_RADIO     )  strcpy(text, "You can not carry a radioactive object");
        if ( num == ERR_MANIP_WATER     )  strcpy(text, "You can not carry an object under water");
        if ( num == ERR_MANIP_EMPTY     )  strcpy(text, "Nothing to drop");
        if ( num == ERR_BUILD_FLY       )  strcpy(text, "Impossible when flying");
        if ( num == ERR_BUILD_WATER     )  strcpy(text, "Impossible under water");
        if ( num == ERR_BUILD_ENERGY    )  strcpy(text, "Not enough energy");
        if ( num == ERR_BUILD_METALAWAY )  strcpy(text, "Titanium too far away");
        if ( num == ERR_BUILD_METALNEAR )  strcpy(text, "Titanium too close");
        if ( num == ERR_BUILD_METALINEX )  strcpy(text, "No titanium around");
        if ( num == ERR_BUILD_FLAT      )  strcpy(text, "Ground not flat enough");
        if ( num == ERR_BUILD_FLATLIT   )  strcpy(text, "Flat ground not large enough");
        if ( num == ERR_BUILD_BUSY      )  strcpy(text, "Place occupied");
        if ( num == ERR_BUILD_BASE      )  strcpy(text, "Too close to space ship");
        if ( num == ERR_BUILD_NARROW    )  strcpy(text, "Too close to a building");
        if ( num == ERR_BUILD_MOTOR     )  strcpy(text, "Impossible when moving");
        if ( num == ERR_SEARCH_FLY      )  strcpy(text, "Impossible when flying");
        if ( num == ERR_SEARCH_VEH      )  strcpy(text, "Inappropriate bot");
        if ( num == ERR_SEARCH_MOTOR    )  strcpy(text, "Impossible when moving");
        if ( num == ERR_TERRA_VEH       )  strcpy(text, "Inappropriate bot");
        if ( num == ERR_TERRA_ENERGY    )  strcpy(text, "Not enough energy");
        if ( num == ERR_TERRA_FLOOR     )  strcpy(text, "Ground inappropriate");
        if ( num == ERR_TERRA_BUILDING  )  strcpy(text, "Building too close");
        if ( num == ERR_TERRA_OBJECT    )  strcpy(text, "Object too close");
        if ( num == ERR_RECOVER_VEH     )  strcpy(text, "Inappropriate bot");
        if ( num == ERR_RECOVER_ENERGY  )  strcpy(text, "Not enough energy");
        if ( num == ERR_RECOVER_NULL    )  strcpy(text, "Nothing to recycle");
        if ( num == ERR_SHIELD_VEH      )  strcpy(text, "Inappropriate bot");
        if ( num == ERR_SHIELD_ENERGY   )  strcpy(text, "No more energy");
        if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Error in instruction move");
        if ( num == ERR_FIND_IMPOSSIBLE )  strcpy(text, "Object not found");
        if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Goto: inaccessible destination");
        if ( num == ERR_GOTO_ITER       )  strcpy(text, "Goto: inaccessible destination");
        if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Goto: destination occupied");
        if ( num == ERR_FIRE_VEH        )  strcpy(text, "Inappropriate bot");
        if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Not enough energy");
        if ( num == ERR_FIRE_FLY        )  strcpy(text, "Impossible when flying");
        if ( num == ERR_CONVERT_EMPTY   )  strcpy(text, "No titanium ore to convert");
        if ( num == ERR_DERRICK_NULL    )  strcpy(text, "No ore in the subsoil");
        if ( num == ERR_STATION_NULL    )  strcpy(text, "No energy in the subsoil");
        if ( num == ERR_TOWER_POWER     )  strcpy(text, "No power cell");
        if ( num == ERR_TOWER_ENERGY    )  strcpy(text, "No more energy");
        if ( num == ERR_RESEARCH_POWER  )  strcpy(text, "No power cell");
        if ( num == ERR_RESEARCH_ENERGY )  strcpy(text, "Not enough energy");
        if ( num == ERR_RESEARCH_TYPE   )  strcpy(text, "Inappropriate cell type");
        if ( num == ERR_RESEARCH_ALREADY)  strcpy(text, "Research program already performed");
        if ( num == ERR_ENERGY_NULL     )  strcpy(text, "No energy in the subsoil");
        if ( num == ERR_ENERGY_LOW      )  strcpy(text, "Not enough energy yet");
        if ( num == ERR_ENERGY_EMPTY    )  strcpy(text, "No titanium to transform");
        if ( num == ERR_ENERGY_BAD      )  strcpy(text, "Transforms only titanium");
        if ( num == ERR_BASE_DLOCK      )  strcpy(text, "Doors blocked by a robot or another object ");
        if ( num == ERR_BASE_DHUMAN     )  strcpy(text, "You must get on the spaceship to take off ");
        if ( num == ERR_LABO_NULL       )  strcpy(text, "Nothing to analyze");
        if ( num == ERR_LABO_BAD        )  strcpy(text, "Analyzes only organic matter");
        if ( num == ERR_LABO_ALREADY    )  strcpy(text, "Analysis already performed");
        if ( num == ERR_NUCLEAR_NULL    )  strcpy(text, "No energy in the subsoil");
        if ( num == ERR_NUCLEAR_LOW     )  strcpy(text, "Not yet enough energy");
        if ( num == ERR_NUCLEAR_EMPTY   )  strcpy(text, "No uranium to transform");
        if ( num == ERR_NUCLEAR_BAD     )  strcpy(text, "Transforms only uranium");
        if ( num == ERR_FACTORY_NULL    )  strcpy(text, "No titanium");
        if ( num == ERR_FACTORY_NEAR    )  strcpy(text, "Object too close");
        if ( num == ERR_RESET_NEAR      )  strcpy(text, "Place occupied");
        if ( num == ERR_INFO_NULL       )  strcpy(text, "No information exchange post within range");
        if ( num == ERR_VEH_VIRUS       )  strcpy(text, "Program infected by a virus");
        if ( num == ERR_BAT_VIRUS       )  strcpy(text, "Infected by a virus, temporarily out of order");
        if ( num == ERR_VEH_POWER       )  strcpy(text, "No power cell");
        if ( num == ERR_VEH_ENERGY      )  strcpy(text, "No more energy");
        if ( num == ERR_FLAG_FLY        )  strcpy(text, "Impossible when flying");
        if ( num == ERR_FLAG_WATER      )  strcpy(text, "Impossible when swimming");
        if ( num == ERR_FLAG_MOTOR      )  strcpy(text, "Impossible when moving");
        if ( num == ERR_FLAG_BUSY       )  strcpy(text, "Impossible when carrying an object");
        if ( num == ERR_FLAG_CREATE     )  strcpy(text, "Too many flags of this color (maximum 5)");
        if ( num == ERR_FLAG_PROXY      )  strcpy(text, "Too close to an existing flag");
        if ( num == ERR_FLAG_DELETE     )  strcpy(text, "No flag nearby");
        if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "The mission is not accomplished yet (press \\key help; for more details)");
        if ( num == ERR_DELETEMOBILE    )  strcpy(text, "Bot destroyed");
        if ( num == ERR_DELETEBUILDING  )  strcpy(text, "Building destroyed");
        if ( num == ERR_TOOMANY         )  strcpy(text, "Can not create this, there are too many objects");
        if ( num == ERR_OBLIGATORYTOKEN )  strcpy(text, "\"%s\" missing in this exercise");  //**
        if ( num == ERR_PROHIBITEDTOKEN )  strcpy(text, "Do not use in this exercise");  //**

        if ( num == INFO_BUILD          )  strcpy(text, "Building completed");
        if ( num == INFO_CONVERT        )  strcpy(text, "Titanium available");
        if ( num == INFO_RESEARCH       )  strcpy(text, "Research program completed");
        if ( num == INFO_RESEARCHTANK   )  strcpy(text, "Plans for tracked robots available ");
        if ( num == INFO_RESEARCHFLY    )  strcpy(text, "You can fly with the keys (\\key gup;) and (\\key gdown;)");
        if ( num == INFO_RESEARCHTHUMP  )  strcpy(text, "Plans for thumper available");
        if ( num == INFO_RESEARCHCANON  )  strcpy(text, "Plans for shooter available");
        if ( num == INFO_RESEARCHTOWER  )  strcpy(text, "Plans for defense tower available");
        if ( num == INFO_RESEARCHPHAZER )  strcpy(text, "Plans for phazer shooter available");
        if ( num == INFO_RESEARCHSHIELD )  strcpy(text, "Plans for shielder available");
        if ( num == INFO_RESEARCHATOMIC )  strcpy(text, "Plans for nuclear power plant available");
        if ( num == INFO_FACTORY        )  strcpy(text, "New bot available");
        if ( num == INFO_LABO           )  strcpy(text, "Analysis performed");
        if ( num == INFO_ENERGY         )  strcpy(text, "Power cell available");
        if ( num == INFO_NUCLEAR        )  strcpy(text, "Nuclear power cell available");
        if ( num == INFO_FINDING        )  strcpy(text, "You found a usable object");
        if ( num == INFO_MARKPOWER      )  strcpy(text, "Found a site for power station");
        if ( num == INFO_MARKURANIUM    )  strcpy(text, "Found a site for a derrick");
        if ( num == INFO_MARKSTONE      )  strcpy(text, "Found a site for a derrick");
        if ( num == INFO_MARKKEYa       )  strcpy(text, "Found a site for a derrick");
        if ( num == INFO_MARKKEYb       )  strcpy(text, "Found a site for a derrick");
        if ( num == INFO_MARKKEYc       )  strcpy(text, "Found a site for a derrick");
        if ( num == INFO_MARKKEYd       )  strcpy(text, "Found a site for a derrick");
        if ( num == INFO_WIN            )  strcpy(text, "<<< Well done, mission accomplished >>>");
        if ( num == INFO_LOST           )  strcpy(text, "<<< Sorry, mission failed >>>");
        if ( num == INFO_LOSTq          )  strcpy(text, "<<< Sorry, mission failed >>>");
        if ( num == INFO_WRITEOK        )  strcpy(text, "Current mission saved");
        if ( num == INFO_DELETEPATH     )  strcpy(text, "Checkpoint crossed");
        if ( num == INFO_DELETEMOTHER   )  strcpy(text, "Alien Queen killed");
        if ( num == INFO_DELETEANT      )  strcpy(text, "Ant fatally wounded");
        if ( num == INFO_DELETEBEE      )  strcpy(text, "Wasp fatally wounded");
        if ( num == INFO_DELETEWORM     )  strcpy(text, "Worm fatally wounded");
        if ( num == INFO_DELETESPIDER   )  strcpy(text, "Spider fatally wounded");
        if ( num == INFO_BEGINSATCOM    )  strcpy(text, "Press \\key help; to read instructions on your SatCom");
    }

    if ( type == RES_CBOT )
    {
        strcpy(text, "Error");
        if ( num == TX_OPENPAR       ) strcpy(text, "Opening bracket missing");
        if ( num == TX_CLOSEPAR      ) strcpy(text, "Closing bracket missing ");
        if ( num == TX_NOTBOOL       ) strcpy(text, "The expression must return a boolean value");
        if ( num == TX_UNDEFVAR      ) strcpy(text, "Variable not declared");
        if ( num == TX_BADLEFT       ) strcpy(text, "Assignment impossible");
        if ( num == TX_ENDOF         ) strcpy(text, "Semicolon terminator missing");
        if ( num == TX_OUTCASE       ) strcpy(text, "Instruction ""case"" outside a block ""switch""");
        if ( num == TX_NOTERM        ) strcpy(text, "Instructions after the final closing brace");
        if ( num == TX_CLOSEBLK      ) strcpy(text, "End of block missing");
        if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instruction ""else"" without corresponding ""if"" ");
        if ( num == TX_OPENBLK       ) strcpy(text, "Opening brace missing ");//dÈbut d'un bloc attendu?
        if ( num == TX_BADTYPE       ) strcpy(text, "Wrong type for the assignment");
        if ( num == TX_REDEFVAR      ) strcpy(text, "A variable can not be declared twice");
        if ( num == TX_BAD2TYPE      ) strcpy(text, "The types of the two operands are incompatible ");
        if ( num == TX_UNDEFCALL     ) strcpy(text, "Unknown function");
        if ( num == TX_MISDOTS       ) strcpy(text, "Sign "" : "" missing");
        if ( num == TX_WHILE         ) strcpy(text, "Keyword ""while"" missing");
        if ( num == TX_BREAK         ) strcpy(text, "Instruction ""break"" outside a loop");
        if ( num == TX_LABEL         ) strcpy(text, "A label must be followed by ""for"", ""while"", ""do"" or ""switch""");
        if ( num == TX_NOLABEL       ) strcpy(text, "This label does not exist");// Cette Ètiquette n'existe pas
        if ( num == TX_NOCASE        ) strcpy(text, "Instruction ""case"" missing");
        if ( num == TX_BADNUM        ) strcpy(text, "Number missing");
        if ( num == TX_VOID          ) strcpy(text, "Void parameter");
        if ( num == TX_NOTYP         ) strcpy(text, "Type declaration missing");
        if ( num == TX_NOVAR         ) strcpy(text, "Variable name missing");
        if ( num == TX_NOFONC        ) strcpy(text, "Function name missing");
        if ( num == TX_OVERPARAM     ) strcpy(text, "Too many parameters");
        if ( num == TX_REDEF         ) strcpy(text, "Function already exists");
        if ( num == TX_LOWPARAM      ) strcpy(text, "Parameters missing ");
        if ( num == TX_BADPARAM      ) strcpy(text, "No function with this name accepts this kind of parameter");
        if ( num == TX_NUMPARAM      ) strcpy(text, "No function with this name accepts this number of parameters");
        if ( num == TX_NOITEM        ) strcpy(text, "This is not a member of this class");
        if ( num == TX_DOT           ) strcpy(text, "This object is not a member of a class");
        if ( num == TX_NOCONST       ) strcpy(text, "Appropriate constructor missing");
        if ( num == TX_REDEFCLASS    ) strcpy(text, "This class already exists");
        if ( num == TX_CLBRK         ) strcpy(text, """ ] "" missing");
        if ( num == TX_RESERVED      ) strcpy(text, "Reserved keyword of CBOT language");
        if ( num == TX_BADNEW        ) strcpy(text, "Bad argument for ""new""");
        if ( num == TX_OPBRK         ) strcpy(text, """ [ "" expected");
        if ( num == TX_BADSTRING     ) strcpy(text, "String missing");
        if ( num == TX_BADINDEX      ) strcpy(text, "Incorrect index type");
        if ( num == TX_PRIVATE       ) strcpy(text, "Private element");
        if ( num == TX_NOPUBLIC      ) strcpy(text, "Public required");
        if ( num == TX_DIVZERO       ) strcpy(text, "Dividing by zero");
        if ( num == TX_NOTINIT       ) strcpy(text, "Variable not initialized");
        if ( num == TX_BADTHROW      ) strcpy(text, "Negative value rejected by ""throw""");//C'est quoi, Áa?
        if ( num == TX_NORETVAL      ) strcpy(text, "The function returned no value ");
        if ( num == TX_NORUN         ) strcpy(text, "No function running");
        if ( num == TX_NOCALL        ) strcpy(text, "Calling an unknown function");
        if ( num == TX_NOCLASS       ) strcpy(text, "This class does not exist");
        if ( num == TX_NULLPT        ) strcpy(text, "Unknown Object");
        if ( num == TX_OPNAN         ) strcpy(text, "Operation impossible with value ""nan""");
        if ( num == TX_OUTARRAY      ) strcpy(text, "Access beyond array limit");
        if ( num == TX_STACKOVER     ) strcpy(text, "Stack overflow");
        if ( num == TX_DELETEDPT     ) strcpy(text, "Illegal object");
        if ( num == TX_FILEOPEN      ) strcpy(text, "Can't open file");
        if ( num == TX_NOTOPEN       ) strcpy(text, "File not open");
        if ( num == TX_ERRREAD       ) strcpy(text, "Read error");
        if ( num == TX_ERRWRITE      ) strcpy(text, "Write error");
    }

    if ( type == RES_KEY )
    {
        if ( num == 0                   )  strcpy(text, "< none >");
        if ( num == VK_LEFT             )  strcpy(text, "Arrow left");
        if ( num == VK_RIGHT            )  strcpy(text, "Arrow right");
        if ( num == VK_UP               )  strcpy(text, "Arrow up");
        if ( num == VK_DOWN             )  strcpy(text, "Arrow down");
        if ( num == VK_CANCEL           )  strcpy(text, "Control-break");
        if ( num == VK_BACK             )  strcpy(text, "<--");
        if ( num == VK_TAB              )  strcpy(text, "Tab");
        if ( num == VK_CLEAR            )  strcpy(text, "Clear");
        if ( num == VK_RETURN           )  strcpy(text, "Enter");
        if ( num == VK_SHIFT            )  strcpy(text, "Shift");
        if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
        if ( num == VK_MENU             )  strcpy(text, "Alt");
        if ( num == VK_PAUSE            )  strcpy(text, "Pause");
        if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
        if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
        if ( num == VK_SPACE            )  strcpy(text, "Space");
        if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
        if ( num == VK_NEXT             )  strcpy(text, "Page Down");
        if ( num == VK_END              )  strcpy(text, "End");
        if ( num == VK_HOME             )  strcpy(text, "Home");
        if ( num == VK_SELECT           )  strcpy(text, "Select");
        if ( num == VK_EXECUTE          )  strcpy(text, "Execute");
        if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
        if ( num == VK_INSERT           )  strcpy(text, "Insert");
        if ( num == VK_DELETE           )  strcpy(text, "Delete");
        if ( num == VK_HELP             )  strcpy(text, "Help");
        if ( num == VK_LWIN             )  strcpy(text, "Left Windows");
        if ( num == VK_RWIN             )  strcpy(text, "Right Windows");
        if ( num == VK_APPS             )  strcpy(text, "Application key");
        if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
        if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
        if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
        if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
        if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
        if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
        if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
        if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
        if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
        if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
        if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
        if ( num == VK_ADD              )  strcpy(text, "NumPad +");
        if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
        if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
        if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
        if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
        if ( num == VK_F1               )  strcpy(text, "F1");
        if ( num == VK_F2               )  strcpy(text, "F2");
        if ( num == VK_F3               )  strcpy(text, "F3");
        if ( num == VK_F4               )  strcpy(text, "F4");
        if ( num == VK_F5               )  strcpy(text, "F5");
        if ( num == VK_F6               )  strcpy(text, "F6");
        if ( num == VK_F7               )  strcpy(text, "F7");
        if ( num == VK_F8               )  strcpy(text, "F8");
        if ( num == VK_F9               )  strcpy(text, "F9");
        if ( num == VK_F10              )  strcpy(text, "F10");
        if ( num == VK_F11              )  strcpy(text, "F11");
        if ( num == VK_F12              )  strcpy(text, "F12");
        if ( num == VK_F13              )  strcpy(text, "F13");
        if ( num == VK_F14              )  strcpy(text, "F14");
        if ( num == VK_F15              )  strcpy(text, "F15");
        if ( num == VK_F16              )  strcpy(text, "F16");
        if ( num == VK_F17              )  strcpy(text, "F17");
        if ( num == VK_F18              )  strcpy(text, "F18");
        if ( num == VK_F19              )  strcpy(text, "F19");
        if ( num == VK_F20              )  strcpy(text, "F20");
        if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
        if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
        if ( num == VK_ATTN             )  strcpy(text, "Attn");
        if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
        if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
        if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
        if ( num == VK_PLAY             )  strcpy(text, "Play");
        if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
        if ( num == VK_PA1              )  strcpy(text, "PA1");
        if ( num == VK_OEM_CLEAR        )  strcpy(text, "Clear");
        if ( num == VK_BUTTON1          )  strcpy(text, "Button 1");
        if ( num == VK_BUTTON2          )  strcpy(text, "Button 2");
        if ( num == VK_BUTTON3          )  strcpy(text, "Button 3");
        if ( num == VK_BUTTON4          )  strcpy(text, "Button 4");
        if ( num == VK_BUTTON5          )  strcpy(text, "Button 5");
        if ( num == VK_BUTTON6          )  strcpy(text, "Button 6");
        if ( num == VK_BUTTON7          )  strcpy(text, "Button 7");
        if ( num == VK_BUTTON8          )  strcpy(text, "Button 8");
        if ( num == VK_BUTTON9          )  strcpy(text, "Button 9");
        if ( num == VK_BUTTON10         )  strcpy(text, "Button 10");
        if ( num == VK_BUTTON11         )  strcpy(text, "Button 11");
        if ( num == VK_BUTTON12         )  strcpy(text, "Button 12");
        if ( num == VK_BUTTON13         )  strcpy(text, "Button 13");
        if ( num == VK_BUTTON14         )  strcpy(text, "Button 14");
        if ( num == VK_BUTTON15         )  strcpy(text, "Button 15");
        if ( num == VK_BUTTON16         )  strcpy(text, "Button 16");
        if ( num == VK_BUTTON17         )  strcpy(text, "Button 17");
        if ( num == VK_BUTTON18         )  strcpy(text, "Button 18");
        if ( num == VK_BUTTON19         )  strcpy(text, "Button 19");
        if ( num == VK_BUTTON20         )  strcpy(text, "Button 20");
        if ( num == VK_BUTTON21         )  strcpy(text, "Button 21");
        if ( num == VK_BUTTON22         )  strcpy(text, "Button 22");
        if ( num == VK_BUTTON23         )  strcpy(text, "Button 23");
        if ( num == VK_BUTTON24         )  strcpy(text, "Button 24");
        if ( num == VK_BUTTON25         )  strcpy(text, "Button 25");
        if ( num == VK_BUTTON26         )  strcpy(text, "Button 26");
        if ( num == VK_BUTTON27         )  strcpy(text, "Button 27");
        if ( num == VK_BUTTON28         )  strcpy(text, "Button 28");
        if ( num == VK_BUTTON29         )  strcpy(text, "Button 29");
        if ( num == VK_BUTTON30         )  strcpy(text, "Button 30");
        if ( num == VK_BUTTON31         )  strcpy(text, "Button 31");
        if ( num == VK_BUTTON32         )  strcpy(text, "Button 32");
        if ( num == VK_WHEELUP          )  strcpy(text, "Wheel up");
        if ( num == VK_WHEELDOWN        )  strcpy(text, "Wheel down");
    }
#endif

#if _FRENCH
    if ( type == RES_TEXT )
    {
        #if _FULL
        if ( num == RT_VERSION_ID          )  strcpy(text, "1.18 /f");
        #endif
        #if _NET
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A 1.18");
        #endif
        #if _SCHOOL & _EDU
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen EDU 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A EDU 1.18");
        #endif
        #endif
        #if _SCHOOL & _PERSO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen PERSO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A PERSO 1.18");
        #endif
        #endif
        #if _SCHOOL & _CEEBOTDEMO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen DEMO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A DEMO 1.18");
        #endif
        #endif
        #if _DEMO
        if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.18 /f");
        #endif
        if ( num == RT_DISINFO_TITLE       )  strcpy(text, "SatCom");
        if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Taille maximale");
        if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Taille rÈduite");
        if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Taille normale");
        if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Fermer");

        if ( num == RT_STUDIO_TITLE        )  strcpy(text, "Edition du programme");
        if ( num == RT_SCRIPT_NEW          )  strcpy(text, "Nouveau");
        if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Joueur");
        if ( num == RT_IO_NEW              )  strcpy(text, "Nouveau ...");
        if ( num == RT_KEY_OR              )  strcpy(text, " ou ");

#if _NEWLOOK
        if ( num == RT_TITLE_BASE          )  strcpy(text, "CeeBot");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "CeeBot");
#else
        if ( num == RT_TITLE_BASE          )  strcpy(text, "COLOBOT");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "COLOBOT");
#endif
        if ( num == RT_TITLE_TRAINER       )  strcpy(text, "Programmation");
        if ( num == RT_TITLE_DEFI          )  strcpy(text, "DÈfis");
        if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missions");
        if ( num == RT_TITLE_FREE          )  strcpy(text, "Jeu libre");
        if ( num == RT_TITLE_TEEN          )  strcpy(text, "Jeu libre");
        if ( num == RT_TITLE_USER          )  strcpy(text, "Niveaux supplÈmentaires");
        if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypes");
        if ( num == RT_TITLE_SETUP         )  strcpy(text, "Options");
        if ( num == RT_TITLE_NAME          )  strcpy(text, "Nom du joueur");
        if ( num == RT_TITLE_PERSO         )  strcpy(text, "Personnalisation de votre apparence");
        if ( num == RT_TITLE_WRITE         )  strcpy(text, "Enregistrement de la mission en cours");
        if ( num == RT_TITLE_READ          )  strcpy(text, "Chargement d'une mission enregistrÈe");

        if ( num == RT_PLAY_CHAPt          )  strcpy(text, " Liste des chapitres :");
        if ( num == RT_PLAY_CHAPd          )  strcpy(text, " Liste des chapitres :");
        if ( num == RT_PLAY_CHAPm          )  strcpy(text, " Liste des planËtes :");
        if ( num == RT_PLAY_CHAPf          )  strcpy(text, " Liste des planËtes :");
        if ( num == RT_PLAY_CHAPu          )  strcpy(text, " Niveaux supplÈmentaires :");
        if ( num == RT_PLAY_CHAPp          )  strcpy(text, " Liste des planËtes :");
        if ( num == RT_PLAY_CHAPte         )  strcpy(text, " Liste des chapitres :");
        if ( num == RT_PLAY_LISTt          )  strcpy(text, " Liste des exercices du chapitre :");
        if ( num == RT_PLAY_LISTd          )  strcpy(text, " Liste des dÈfis du chapitre :");
        if ( num == RT_PLAY_LISTm          )  strcpy(text, " Liste des missions du chapitre :");
        if ( num == RT_PLAY_LISTf          )  strcpy(text, " Liste des jeux libres du chapitre :");
        if ( num == RT_PLAY_LISTu          )  strcpy(text, " Missions du niveau :");
        if ( num == RT_PLAY_LISTp          )  strcpy(text, " Liste des prototypes du chapitre :");
        if ( num == RT_PLAY_LISTk          )  strcpy(text, " Liste des jeux libres du chapitre :");
        if ( num == RT_PLAY_RESUME         )  strcpy(text, " RÈsumÈ :");

        if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Pilotes :");
        if ( num == RT_SETUP_MODE          )  strcpy(text, " RÈsolutions :");
        if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Cliquez d'abord sur la touche ‡ redÈfinir.");
        if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Appuyez ensuite sur la nouvelle touche souhaitÈe.");

        if ( num == RT_PERSO_FACE          )  strcpy(text, "Type de visage :");
        if ( num == RT_PERSO_GLASSES       )  strcpy(text, "Lunettes :");
        if ( num == RT_PERSO_HAIR          )  strcpy(text, "Couleur des cheveux :");
        if ( num == RT_PERSO_COMBI         )  strcpy(text, "Couleur de la combinaison :");
        if ( num == RT_PERSO_BAND          )  strcpy(text, "Couleur des bandes :");

#if _NEWLOOK
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "CeeBot");
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Voulez-vous quitter CeeBot ?");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quitter\\Quitter CeeBot");
#else
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "COLOBOT");
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Voulez-vous quitter COLOBOT ?");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quitter\\Quitter COLOBOT");
#endif
        if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Quitter la mission ?");
        if ( num == RT_DIALOG_YES          )  strcpy(text, "Abandonner\\Abandonner la mission en cours");
        if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuer\\Continuer la mission en cours");
        if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuer\\Continuer de jouer");
        if ( num == RT_DIALOG_DELOBJ       )  strcpy(text, "Voulez-vous vraiment dÈtruire le b‚timent sÈlectionnÈ ?");
        if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Voulez-vous dÈtruire les sauvegardes de %s ?");
        if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "DÈtruire");
        if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Annuler");
        if ( num == RT_DIALOG_LOADING      )  strcpy(text, "CHARGEMENT");

        if ( num == RT_STUDIO_LISTTT       )  strcpy(text, "Aide sur le mot-clÈ (\\key cbot;)");
        if ( num == RT_STUDIO_COMPOK       )  strcpy(text, "Compilation ok (0 erreur)");
        if ( num == RT_STUDIO_PROGSTOP     )  strcpy(text, "Programme terminÈ");

        if ( num == RT_SATCOM_LIST         )  strcpy(text, "\\b;Listes des objets\n");
        if ( num == RT_SATCOM_BOT          )  strcpy(text, "\\b;Listes des robots\n");
        if ( num == RT_SATCOM_BUILDING     )  strcpy(text, "\\b;Listes des b‚timents\n");
        if ( num == RT_SATCOM_FRET         )  strcpy(text, "\\b;Listes des objets transportables\n");
        if ( num == RT_SATCOM_ALIEN        )  strcpy(text, "\\b;Listes des ennemis\n");
        if ( num == RT_SATCOM_NULL         )  strcpy(text, "\\c; (aucun)\\n;\n");
        if ( num == RT_SATCOM_ERROR1       )  strcpy(text, "\\b;Erreur\n");
        if ( num == RT_SATCOM_ERROR2       )  strcpy(text, "Liste non disponible sans \\l;radar\\u object\\radar; !\n");

        if ( num == RT_IO_OPEN             )  strcpy(text, "Ouvrir");
        if ( num == RT_IO_SAVE             )  strcpy(text, "Enregistrer");
        if ( num == RT_IO_LIST             )  strcpy(text, "Dossier: %s");
        if ( num == RT_IO_NAME             )  strcpy(text, "Nom:");
        if ( num == RT_IO_DIR              )  strcpy(text, "Dans:");
        if ( num == RT_IO_PRIVATE          )  strcpy(text, "PrivÈ\\Dossier privÈ");
        if ( num == RT_IO_PUBLIC           )  strcpy(text, "Public\\Dossier commun ‡ tous les joueurs");

        if ( num == RT_GENERIC_DEV1        )  strcpy(text, "DÈveloppÈ par :");
        if ( num == RT_GENERIC_DEV2        )  strcpy(text, "www.epsitec.com");
#if _SCHOOL
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, " ");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, " ");
#else
        //?if ( num == RT_GENERIC_EDIT1       )  strcpy(text, "Version franÁaise ÈditÈe par :");
        //?if ( num == RT_GENERIC_EDIT2       )  strcpy(text, "www.alsyd.com");
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, " ");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, " ");
#endif

        if ( num == RT_INTERFACE_REC       )  strcpy(text, "Enregistreur");
    }

    if ( type == RES_EVENT )
    {
        if ( num == EVENT_BUTTON_OK        )  strcpy(text, "D'accord");
        if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Annuler");
        if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Suivant");
        if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "PrÈcÈdent");
        if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

        if ( num == EVENT_DIALOG_OK        )  strcpy(text, "D'accord");
        if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Annuler");

        if ( num == EVENT_INTERFACE_TRAINER)  strcpy(text, "Programmation\\Exercices de programmation");
        if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "DÈfis\\DÈfis de programmation");
        if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missions\\La grande aventure");
        if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Jeu libre\\Jeu libre sans but prÈcis");
        if ( num == EVENT_INTERFACE_TEEN   )  strcpy(text, "Jeu libre\\Jeu libre sans but prÈcis");
        if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Suppl.\\Niveaux supplÈmentaires");
        if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypes en cours d'Èlaboration");
        if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Autre joueur\\Choix du nom du joueur");
        if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Options\\RÈglages");
        if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Recommencer\\Recommencer la mission au dÈbut");
        if ( num == EVENT_INTERFACE_WRITE  )  strcpy(text, "Enregistrer\\Enregistrer la mission en cours");
        if ( num == EVENT_INTERFACE_READ   )  strcpy(text, "Charger\\Charger une mission enregistrÈe");
#if _NEWLOOK
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Retourner dans CeeBot");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quitter\\Quitter CeeBot");
#else
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Retourner dans COLOBOT");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quitter\\Quitter COLOBOT");
#endif
        if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "<<  Retour  \\Retour au niveau prÈcÈdent");
        if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Jouer ...\\DÈmarrer l'action");
        if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Affichage\\Pilote et rÈsolution d'affichage");
        if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Graphique\\Options graphiques");
        if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Jeu\\Options de jouabilitÈ");
        if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Commandes\\Touches du clavier");
        if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Son\\Volumes bruitages & musiques");
        if ( num == EVENT_INTERFACE_DEVICE )  strcpy(text, "UnitÈ");
        if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "RÈsolution");
        if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Plein Ècran\\Plein Ècran ou fenÍtrÈ");
        if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Appliquer les changements\\Active les changements effectuÈs");

        if ( num == EVENT_INTERFACE_TOTO   )  strcpy(text, "Robbie\\Votre assistant");
        if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Ombres\\Ombres projetÈes au sol");
        if ( num == EVENT_INTERFACE_GROUND )  strcpy(text, "Marques sur le sol\\Marques dessinÈes sur le sol");
        if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Salissures\\Salissures des robots et b‚timents");
        if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Brouillard\\Nappes de brouillard");
        if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Rayons du soleil\\Rayons selon l'orientation");
        if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Ciel\\Ciel et nuages");
        if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "PlanËtes et Ètoiles\\Motifs mobiles dans le ciel");
        if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "LumiËres dynamiques\\Eclairages mobiles");
        if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "QuantitÈ de particules\\Explosions, poussiËres, reflets, etc.");
        if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Profondeur de champ\\Distance de vue maximale");
        if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "DÈtails des objets\\QualitÈ des objets en 3D");
        if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "QualitÈ des textures\\QualitÈ des images");
        if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Nb d'objets dÈcoratifs\\QualitÈ d'objets non indispensables");
        if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Particules dans l'interface\\Pluie de particules");
        if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Reflets sur les boutons\\Boutons brillants");
        if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Bulles d'aide\\Bulles explicatives");
        if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "SÈquences cinÈmatiques\\Films avant ou aprËs une mission");
        if ( num == EVENT_INTERFACE_NICERST)  strcpy(text, "Retour animÈ\\Retour animÈ dans les exercices");
        if ( num == EVENT_INTERFACE_HIMSELF)  strcpy(text, "DÈg‚ts ‡ soi-mÍme\\Vos tirs infligent des dommages ‡ vos unitÈs");
        if ( num == EVENT_INTERFACE_SCROLL )  strcpy(text, "DÈfilement dans les bords\\DÈfilement lorsque la souris touches les bords gauche ou droite");
        if ( num == EVENT_INTERFACE_INVERTX)  strcpy(text, "Inversion souris X\\Inversion de la rotation lorsque la souris touche un bord");
        if ( num == EVENT_INTERFACE_INVERTY)  strcpy(text, "Inversion souris Y\\Inversion de la rotation lorsque la souris touche un bord");
        if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Secousses lors d'explosions\\L'Ècran vibre lors d'une explosion");
        if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Souris ombrÈe\\Jolie souris avec une ombre");
        if ( num == EVENT_INTERFACE_EDITMODE) strcpy(text, "Indentation automatique\\Pendant l'Èdition d'un programme");
        if ( num == EVENT_INTERFACE_EDITVALUE)strcpy(text, "Grande indentation\\Indente avec 2 ou 4 espaces");
        if ( num == EVENT_INTERFACE_SOLUCE4)  strcpy(text, "AccËs aux solutions\\Programme \"4: Solution\" dans les exercices");

        if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Tout rÈinitialiser\\Remet toutes les touches standards");
        if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Tourner ‡ gauche\\Moteur ‡ gauche");
        if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Tourner ‡ droite\\Moteur ‡ droite");
        if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Avancer\\Moteur en avant");
        if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Reculer\\Moteur en arriËre");
        if ( num == EVENT_INTERFACE_KGUP   )  strcpy(text, "Monter\\Augmenter la puissance du rÈacteur");
        if ( num == EVENT_INTERFACE_KGDOWN )  strcpy(text, "Descendre\\Diminuer la puissance du rÈacteur");
        if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Changement de camÈra\\Autre de point de vue");
        if ( num == EVENT_INTERFACE_KDESEL )  strcpy(text, "SÈlection prÈcÈdente\\SÈlectionne l'objet prÈcÈdent");
        if ( num == EVENT_INTERFACE_KACTION)  strcpy(text, "Action standard\\Action du bouton avec le cadre rouge");
        if ( num == EVENT_INTERFACE_KNEAR  )  strcpy(text, "CamÈra plus proche\\Avance la camÈra");
        if ( num == EVENT_INTERFACE_KAWAY  )  strcpy(text, "CamÈra plus loin\\Recule la camÈra");
        if ( num == EVENT_INTERFACE_KNEXT  )  strcpy(text, "SÈlectionner l'objet suivant\\SÈlectionner l'objet suivant");
        if ( num == EVENT_INTERFACE_KHUMAN )  strcpy(text, "SÈlectionner le cosmonaute\\SÈlectionner le cosmonaute");
        if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Quitter la mission en cours\\Terminer un exercice ou une mssion");
        if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instructions mission\\Marche ‡ suivre");
        if ( num == EVENT_INTERFACE_KPROG  )  strcpy(text, "Instructions programmation\\Explication sur la programmation");
        if ( num == EVENT_INTERFACE_KCBOT  )  strcpy(text, "Instructions mot-clÈ\\Explication sur le mot-clÈ");
        if ( num == EVENT_INTERFACE_KVISIT )  strcpy(text, "Montrer le lieu d'un message\\Montrer le lieu du dernier message");
        if ( num == EVENT_INTERFACE_KSPEED10) strcpy(text, "Vitesse 1.0x\\Vitesse normale");
        if ( num == EVENT_INTERFACE_KSPEED15) strcpy(text, "Vitesse 1.5x\\Une fois et demi plus rapide");
        if ( num == EVENT_INTERFACE_KSPEED20) strcpy(text, "Vitesse 2.0x\\Deux fois plus rapide");
        if ( num == EVENT_INTERFACE_KSPEED30) strcpy(text, "Vitesse 3.0x\\Trois fois plus rapide");

        if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Bruitages :\\Volume des moteurs, voix, etc.");
        if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Fond sonore :\\Volume des pistes audio du CD");
        if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "Bruitages 3D\\Positionnement sonore dans l'espace");

        if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mini\\QualitÈ minimale (+ rapide)");
        if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\QualitÈ standard");
        if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Maxi\\Haute qualitÈ (+ lent)");

        if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silencieux\\Totalement silencieux");
        if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Niveaux normaux");

        if ( num == EVENT_INTERFACE_JOYSTICK) strcpy(text, "Utilise un joystick\\Joystick ou clavier");
        if ( num == EVENT_INTERFACE_SOLUCE )  strcpy(text, "AccËs ‡ la solution\\Donne la solution");

        if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nom du joueur ‡ crÈer");
        if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "D'accord\\Choisir le joueur");
        if ( num == EVENT_INTERFACE_NCANCEL)  strcpy(text, "Annuler\\Conserver le joueur actuel");
        if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Supprimer le joueur\\Supprimer le joueur de la liste");
        if ( num == EVENT_INTERFACE_NLABEL )  strcpy(text, "Nom du joueur");

        if ( num == EVENT_INTERFACE_IOWRITE)  strcpy(text, "Enregistrer\\Enregistrer la mission en cours");
        if ( num == EVENT_INTERFACE_IOREAD )  strcpy(text, "Charger\\Charger la mission sÈlectionnÈe");
        if ( num == EVENT_INTERFACE_IOLIST )  strcpy(text, "Liste des missions enregistrÈes");
        if ( num == EVENT_INTERFACE_IOLABEL)  strcpy(text, "Nom du fichier :");
        if ( num == EVENT_INTERFACE_IONAME )  strcpy(text, "Nom de la mission");
        if ( num == EVENT_INTERFACE_IOIMAGE)  strcpy(text, "Vue de la mission");
        if ( num == EVENT_INTERFACE_IODELETE) strcpy(text, "Supprimer\\Supprime l'enregistrement sÈlectionnÈ");

        if ( num == EVENT_INTERFACE_PERSO  )  strcpy(text, "Aspect\\Choisir votre aspect");
        if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "D'accord");
        if ( num == EVENT_INTERFACE_PCANCEL)  strcpy(text, "Annuler");
        if ( num == EVENT_INTERFACE_PDEF   )  strcpy(text, "Standard\\Remet les couleurs standards");
        if ( num == EVENT_INTERFACE_PHEAD  )  strcpy(text, "TÍte\\Visage et cheveux");
        if ( num == EVENT_INTERFACE_PBODY  )  strcpy(text, "Corps\\Combinaison");
        if ( num == EVENT_INTERFACE_PLROT  )  strcpy(text, "\\Rotation ‡ gauche");
        if ( num == EVENT_INTERFACE_PRROT  )  strcpy(text, "\\Rotation ‡ droite");
        if ( num == EVENT_INTERFACE_PCRa   )  strcpy(text, "Rouge");
        if ( num == EVENT_INTERFACE_PCGa   )  strcpy(text, "Vert");
        if ( num == EVENT_INTERFACE_PCBa   )  strcpy(text, "Bleu");
        if ( num == EVENT_INTERFACE_PCRb   )  strcpy(text, "Rouge");
        if ( num == EVENT_INTERFACE_PCGb   )  strcpy(text, "Vert");
        if ( num == EVENT_INTERFACE_PCBb   )  strcpy(text, "Bleu");
        if ( num == EVENT_INTERFACE_PFACE1 )  strcpy(text, "\\Visage 1");
        if ( num == EVENT_INTERFACE_PFACE2 )  strcpy(text, "\\Visage 4");
        if ( num == EVENT_INTERFACE_PFACE3 )  strcpy(text, "\\Visage 3");
        if ( num == EVENT_INTERFACE_PFACE4 )  strcpy(text, "\\Visage 2");
        if ( num == EVENT_INTERFACE_PGLASS0)  strcpy(text, "\\Pas de lunettes");
        if ( num == EVENT_INTERFACE_PGLASS1)  strcpy(text, "\\Lunettes 1");
        if ( num == EVENT_INTERFACE_PGLASS2)  strcpy(text, "\\Lunettes 2");
        if ( num == EVENT_INTERFACE_PGLASS3)  strcpy(text, "\\Lunettes 3");
        if ( num == EVENT_INTERFACE_PGLASS4)  strcpy(text, "\\Lunettes 4");
        if ( num == EVENT_INTERFACE_PGLASS5)  strcpy(text, "\\Lunettes 5");

        if ( num == EVENT_OBJECT_DESELECT  )  strcpy(text, "SÈlection prÈcÈdente (\\key desel;)");
        if ( num == EVENT_OBJECT_LEFT      )  strcpy(text, "Tourne ‡ gauche (\\key left;)");
        if ( num == EVENT_OBJECT_RIGHT     )  strcpy(text, "Tourne ‡ droite (\\key right;)");
        if ( num == EVENT_OBJECT_UP        )  strcpy(text, "Avance (\\key up;)");
        if ( num == EVENT_OBJECT_DOWN      )  strcpy(text, "Recule (\\key down;)");
        if ( num == EVENT_OBJECT_GASUP     )  strcpy(text, "Monte (\\key gup;)");
        if ( num == EVENT_OBJECT_GASDOWN   )  strcpy(text, "Descend (\\key gdown;)");
        if ( num == EVENT_OBJECT_HTAKE     )  strcpy(text, "Prend ou dÈpose  (\\key action;)");
        if ( num == EVENT_OBJECT_MTAKE     )  strcpy(text, "Prend ou dÈpose  (\\key action;)");
        if ( num == EVENT_OBJECT_MFRONT    )  strcpy(text, "..devant");
        if ( num == EVENT_OBJECT_MBACK     )  strcpy(text, "..derriËre");
        if ( num == EVENT_OBJECT_MPOWER    )  strcpy(text, "..pile");
        if ( num == EVENT_OBJECT_BHELP     )  strcpy(text, "Instructions sur la mission (\\key help;)");
        if ( num == EVENT_OBJECT_BTAKEOFF  )  strcpy(text, "DÈcolle pour terminer la mission");
        if ( num == EVENT_OBJECT_BDERRICK  )  strcpy(text, "Construit un derrick");
        if ( num == EVENT_OBJECT_BSTATION  )  strcpy(text, "Construit une station");
        if ( num == EVENT_OBJECT_BFACTORY  )  strcpy(text, "Construit une fabrique de robots");
        if ( num == EVENT_OBJECT_BREPAIR   )  strcpy(text, "Construit un centre de rÈparation");
        if ( num == EVENT_OBJECT_BCONVERT  )  strcpy(text, "Construit un convertisseur");
        if ( num == EVENT_OBJECT_BTOWER    )  strcpy(text, "Construit une tour");
        if ( num == EVENT_OBJECT_BRESEARCH )  strcpy(text, "Construit un centre de recherches");
        if ( num == EVENT_OBJECT_BRADAR    )  strcpy(text, "Construit un radar");
        if ( num == EVENT_OBJECT_BENERGY   )  strcpy(text, "Construit une fabrique de piles");
        if ( num == EVENT_OBJECT_BLABO     )  strcpy(text, "Construit un laboratoire");
        if ( num == EVENT_OBJECT_BNUCLEAR  )  strcpy(text, "Construit une centrale nuclÈaire");
        if ( num == EVENT_OBJECT_BPARA     )  strcpy(text, "Construit un paratonnerre");
        if ( num == EVENT_OBJECT_BINFO     )  strcpy(text, "Construit une borne d'information");
        if ( num == EVENT_OBJECT_GFLAT     )  strcpy(text, "Montre si le sol est plat");
        if ( num == EVENT_OBJECT_FCREATE   )  strcpy(text, "Pose un drapeau de couleur");
        if ( num == EVENT_OBJECT_FDELETE   )  strcpy(text, "EnlËve un drapeau");
        if ( num == EVENT_OBJECT_FCOLORb   )  strcpy(text, "\\Drapeaux bleus");
        if ( num == EVENT_OBJECT_FCOLORr   )  strcpy(text, "\\Drapeaux rouges");
        if ( num == EVENT_OBJECT_FCOLORg   )  strcpy(text, "\\Drapeaux verts");
        if ( num == EVENT_OBJECT_FCOLORy   )  strcpy(text, "\\Drapeaux jaunes");
        if ( num == EVENT_OBJECT_FCOLORv   )  strcpy(text, "\\Drapeaux violets");
        if ( num == EVENT_OBJECT_FACTORYfa )  strcpy(text, "Fabrique un dÈmÈnageur volant");
        if ( num == EVENT_OBJECT_FACTORYta )  strcpy(text, "Fabrique un dÈmÈnageur ‡ chenilles");
        if ( num == EVENT_OBJECT_FACTORYwa )  strcpy(text, "Fabrique un dÈmÈnageur ‡ roues");
        if ( num == EVENT_OBJECT_FACTORYia )  strcpy(text, "Fabrique un dÈmÈnageur ‡ pattes");
        if ( num == EVENT_OBJECT_FACTORYfc )  strcpy(text, "Fabrique un shooter volant");
        if ( num == EVENT_OBJECT_FACTORYtc )  strcpy(text, "Fabrique un shooter ‡ chenilles");
        if ( num == EVENT_OBJECT_FACTORYwc )  strcpy(text, "Fabrique un shooter ‡ roues");
        if ( num == EVENT_OBJECT_FACTORYic )  strcpy(text, "Fabrique un shooter ‡ pattes");
        if ( num == EVENT_OBJECT_FACTORYfi )  strcpy(text, "Fabrique un orgaShooter volant");
        if ( num == EVENT_OBJECT_FACTORYti )  strcpy(text, "Fabrique un orgaShooter ‡ chenilles");
        if ( num == EVENT_OBJECT_FACTORYwi )  strcpy(text, "Fabrique un orgaShooter ‡ roues");
        if ( num == EVENT_OBJECT_FACTORYii )  strcpy(text, "Fabrique un orgaShooter ‡ pattes");
        if ( num == EVENT_OBJECT_FACTORYfs )  strcpy(text, "Fabrique un renifleur volant");
        if ( num == EVENT_OBJECT_FACTORYts )  strcpy(text, "Fabrique un renifleur ‡ chenilles");
        if ( num == EVENT_OBJECT_FACTORYws )  strcpy(text, "Fabrique un renifleur ‡ roues");
        if ( num == EVENT_OBJECT_FACTORYis )  strcpy(text, "Fabrique un renifleur ‡ pattes");
        if ( num == EVENT_OBJECT_FACTORYrt )  strcpy(text, "Fabrique un robot secoueur");
        if ( num == EVENT_OBJECT_FACTORYrc )  strcpy(text, "Fabrique un robot phazer");
        if ( num == EVENT_OBJECT_FACTORYrr )  strcpy(text, "Fabrique un robot recycleur");
        if ( num == EVENT_OBJECT_FACTORYrs )  strcpy(text, "Fabrique un robot bouclier");
        if ( num == EVENT_OBJECT_FACTORYsa )  strcpy(text, "Fabrique un robot sous-marin");
        if ( num == EVENT_OBJECT_RTANK     )  strcpy(text, "Recherche les chenilles");
        if ( num == EVENT_OBJECT_RFLY      )  strcpy(text, "Recherche les robots volants");
        if ( num == EVENT_OBJECT_RTHUMP    )  strcpy(text, "Recherche le secoueur");
        if ( num == EVENT_OBJECT_RCANON    )  strcpy(text, "Recherche le canon shooter");
        if ( num == EVENT_OBJECT_RTOWER    )  strcpy(text, "Recherche la tour de dÈfense");
        if ( num == EVENT_OBJECT_RPHAZER   )  strcpy(text, "Recherche le canon phazer");
        if ( num == EVENT_OBJECT_RSHIELD   )  strcpy(text, "Recherche le bouclier");
        if ( num == EVENT_OBJECT_RATOMIC   )  strcpy(text, "Recherche le nuclÈaire");
        if ( num == EVENT_OBJECT_RiPAW     )  strcpy(text, "Recherche les pattes");
        if ( num == EVENT_OBJECT_RiGUN     )  strcpy(text, "Recherche le canon orgaShooter");
        if ( num == EVENT_OBJECT_RESET     )  strcpy(text, "Remet au dÈpart");
        if ( num == EVENT_OBJECT_SEARCH    )  strcpy(text, "Cherche (\\key action;)");
        if ( num == EVENT_OBJECT_TERRAFORM )  strcpy(text, "Secoue (\\key action;)");
        if ( num == EVENT_OBJECT_FIRE      )  strcpy(text, "Tir (\\key action;)");
        if ( num == EVENT_OBJECT_RECOVER   )  strcpy(text, "Recycle (\\key action;)");
        if ( num == EVENT_OBJECT_BEGSHIELD )  strcpy(text, "DÈploie le bouclier (\\key action;)");
        if ( num == EVENT_OBJECT_ENDSHIELD )  strcpy(text, "Stoppe le bouclier (\\key action;)");
        if ( num == EVENT_OBJECT_DIMSHIELD )  strcpy(text, "Rayon du bouclier");
        if ( num == EVENT_OBJECT_PROGRUN   )  strcpy(text, "ExÈcute le programme sÈlectionnÈ");
        if ( num == EVENT_OBJECT_PROGEDIT  )  strcpy(text, "Edite le programme sÈlectionnÈ");
        if ( num == EVENT_OBJECT_INFOOK    )  strcpy(text, "\\Mettre le SatCom en veille");
        if ( num == EVENT_OBJECT_DELETE    )  strcpy(text, "DÈmolit le b‚timent");
        if ( num == EVENT_OBJECT_GENERGY   )  strcpy(text, "Niveau d'Ènergie");
        if ( num == EVENT_OBJECT_GSHIELD   )  strcpy(text, "Niveau du bouclier");
        if ( num == EVENT_OBJECT_GRANGE    )  strcpy(text, "TempÈrature du rÈacteur");
        if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Travail en cours ...");
        if ( num == EVENT_OBJECT_GRADAR    )  strcpy(text, "Nombre d'insectes dÈtectÈs");
        if ( num == EVENT_OBJECT_GINFO     )  strcpy(text, "Informations diffusÈes");
        if ( num == EVENT_OBJECT_COMPASS   )  strcpy(text, "Boussole");
//?     if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-carte");
        if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom mini-carte");
        if ( num == EVENT_OBJECT_CAMERA    )  strcpy(text, "CamÈra (\\key camera;)");
        if ( num == EVENT_OBJECT_CAMERAleft)  strcpy(text, "CamÈra ‡ gauche");
        if ( num == EVENT_OBJECT_CAMERAright) strcpy(text, "CamÈra ‡ droite");
        if ( num == EVENT_OBJECT_CAMERAnear)  strcpy(text, "CamÈra plus proche");
        if ( num == EVENT_OBJECT_CAMERAaway)  strcpy(text, "CamÈra plus loin");
        if ( num == EVENT_OBJECT_HELP      )  strcpy(text, "Instructions sur la sÈlection");
        if ( num == EVENT_OBJECT_SOLUCE    )  strcpy(text, "Donne la solution");
        if ( num == EVENT_OBJECT_SHORTCUT00)  strcpy(text, "Permute robots <-> b‚timents");
        if ( num == EVENT_OBJECT_LIMIT     )  strcpy(text, "Montre le rayon d'action");
        if ( num == EVENT_OBJECT_PEN0      )  strcpy(text, "\\RelËve le crayon");
        if ( num == EVENT_OBJECT_PEN1      )  strcpy(text, "\\Abaisse le crayon noir");
        if ( num == EVENT_OBJECT_PEN2      )  strcpy(text, "\\Abaisse le crayon jaune");
        if ( num == EVENT_OBJECT_PEN3      )  strcpy(text, "\\Abaisse le crayon orange");
        if ( num == EVENT_OBJECT_PEN4      )  strcpy(text, "\\Abaisse le crayon rouge");
        if ( num == EVENT_OBJECT_PEN5      )  strcpy(text, "\\Abaisse le crayon violet");
        if ( num == EVENT_OBJECT_PEN6      )  strcpy(text, "\\Abaisse le crayon bleu");
        if ( num == EVENT_OBJECT_PEN7      )  strcpy(text, "\\Abaisse le crayon vert");
        if ( num == EVENT_OBJECT_PEN8      )  strcpy(text, "\\Abaisse le crayon brun");
        if ( num == EVENT_OBJECT_REC       )  strcpy(text, "\\DÈmarre l'enregistrement");
        if ( num == EVENT_OBJECT_STOP      )  strcpy(text, "\\Stoppe l'enregistrement");
        if ( num == EVENT_DT_VISIT0        ||
             num == EVENT_DT_VISIT1        ||
             num == EVENT_DT_VISIT2        ||
             num == EVENT_DT_VISIT3        ||
             num == EVENT_DT_VISIT4        )  strcpy(text, "Montre l'endroit");
        if ( num == EVENT_DT_END           )  strcpy(text, "Continuer");
        if ( num == EVENT_CMD              )  strcpy(text, "Console de commande");
        if ( num == EVENT_SPEED            )  strcpy(text, "Vitesse du jeu");

        if ( num == EVENT_HYPER_PREV       )  strcpy(text, "Page prÈcÈdente");
        if ( num == EVENT_HYPER_NEXT       )  strcpy(text, "Page suivante");
        if ( num == EVENT_HYPER_HOME       )  strcpy(text, "Page initiale");
        if ( num == EVENT_HYPER_COPY       )  strcpy(text, "Copier");
        if ( num == EVENT_HYPER_SIZE1      )  strcpy(text, "Taille 1");
        if ( num == EVENT_HYPER_SIZE2      )  strcpy(text, "Taille 2");
        if ( num == EVENT_HYPER_SIZE3      )  strcpy(text, "Taille 3");
        if ( num == EVENT_HYPER_SIZE4      )  strcpy(text, "Taille 4");
        if ( num == EVENT_HYPER_SIZE5      )  strcpy(text, "Taille 5");
        if ( num == EVENT_SATCOM_HUSTON    )  strcpy(text, "Instructions de Houston");
#if _TEEN
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Dictionnaire anglais-franÁais");
#else
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Rapport du satellite");
#endif
        if ( num == EVENT_SATCOM_LOADING   )  strcpy(text, "Programmes envoyÈs par Houston");
        if ( num == EVENT_SATCOM_OBJECT    )  strcpy(text, "Liste des objets");
        if ( num == EVENT_SATCOM_PROG      )  strcpy(text, "Aide ‡ la programmation");
        if ( num == EVENT_SATCOM_SOLUCE    )  strcpy(text, "Solution");

        if ( num == EVENT_STUDIO_OK        )  strcpy(text, "D'accord\\Compiler le programme");
        if ( num == EVENT_STUDIO_CANCEL    )  strcpy(text, "Annuler\\Annuler toutes les modifications");
        if ( num == EVENT_STUDIO_NEW       )  strcpy(text, "Nouveau");
        if ( num == EVENT_STUDIO_OPEN      )  strcpy(text, "Ouvrir (Ctrl+o)");
        if ( num == EVENT_STUDIO_SAVE      )  strcpy(text, "Enregistrer (Ctrl+s)");
        if ( num == EVENT_STUDIO_UNDO      )  strcpy(text, "Annuler (Ctrl+z)");
        if ( num == EVENT_STUDIO_CUT       )  strcpy(text, "Couper (Ctrl+x)");
        if ( num == EVENT_STUDIO_COPY      )  strcpy(text, "Copier (Ctrl+c)");
        if ( num == EVENT_STUDIO_PASTE     )  strcpy(text, "Coller (Ctrl+v)");
        if ( num == EVENT_STUDIO_SIZE      )  strcpy(text, "Taille des caractËres");
        if ( num == EVENT_STUDIO_TOOL      )  strcpy(text, "Instructions (\\key help;)");
        if ( num == EVENT_STUDIO_HELP      )  strcpy(text, "Aide ‡ la programmation (\\key prog;)");
        if ( num == EVENT_STUDIO_COMPILE   )  strcpy(text, "Compiler");
        if ( num == EVENT_STUDIO_RUN       )  strcpy(text, "DÈmarrer/stopper");
        if ( num == EVENT_STUDIO_REALTIME  )  strcpy(text, "Pause/continuer");
        if ( num == EVENT_STUDIO_STEP      )  strcpy(text, "Un pas");
    }

    if ( type == RES_OBJECT )
    {
        if ( num == OBJECT_PORTICO      )  strcpy(text, "Portique");
        if ( num == OBJECT_BASE         )  strcpy(text, "Vaisseau spatial");
        if ( num == OBJECT_DERRICK      )  strcpy(text, "Derrick");
        if ( num == OBJECT_FACTORY      )  strcpy(text, "Fabrique de robots");
        if ( num == OBJECT_REPAIR       )  strcpy(text, "Centre de rÈparation");
        if ( num == OBJECT_DESTROYER    )  strcpy(text, "Destructeur");
        if ( num == OBJECT_STATION      )  strcpy(text, "Station de recharge");
        if ( num == OBJECT_CONVERT      )  strcpy(text, "Conversion minerai en titanium");
        if ( num == OBJECT_TOWER        )  strcpy(text, "Tour de dÈfense");
        if ( num == OBJECT_NEST         )  strcpy(text, "Nid");
        if ( num == OBJECT_RESEARCH     )  strcpy(text, "Centre de recherches");
        if ( num == OBJECT_RADAR        )  strcpy(text, "Radar");
        if ( num == OBJECT_INFO         )  strcpy(text, "Borne d'information");
#if _TEEN
        if ( num == OBJECT_ENERGY       )  strcpy(text, "DÈsintÈgrateur");
#else
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Fabrique de piles");
#endif
        if ( num == OBJECT_LABO         )  strcpy(text, "Laboratoire de matiËres organiques");
        if ( num == OBJECT_NUCLEAR      )  strcpy(text, "Centrale nuclÈaire");
        if ( num == OBJECT_PARA         )  strcpy(text, "Paratonnerre");
        if ( num == OBJECT_SAFE         )  strcpy(text, "Coffre-fort");
        if ( num == OBJECT_HUSTON       )  strcpy(text, "Centre de contrÙle");
        if ( num == OBJECT_TARGET1      )  strcpy(text, "Cible");
        if ( num == OBJECT_TARGET2      )  strcpy(text, "Cible");
        if ( num == OBJECT_START        )  strcpy(text, "DÈpart");
        if ( num == OBJECT_END          )  strcpy(text, "But");
        if ( num == OBJECT_STONE        )  strcpy(text, "Minerai de titanium");
        if ( num == OBJECT_URANIUM      )  strcpy(text, "Minerai d'uranium");
        if ( num == OBJECT_BULLET       )  strcpy(text, "MatiËre organique");
        if ( num == OBJECT_METAL        )  strcpy(text, "Titanium");
        if ( num == OBJECT_POWER        )  strcpy(text, "Pile normale");
        if ( num == OBJECT_ATOMIC       )  strcpy(text, "Pile nuclÈaire");
        if ( num == OBJECT_BBOX         )  strcpy(text, "BoÓte noire");
        if ( num == OBJECT_KEYa         )  strcpy(text, "ClÈ A");
        if ( num == OBJECT_KEYb         )  strcpy(text, "ClÈ B");
        if ( num == OBJECT_KEYc         )  strcpy(text, "ClÈ C");
        if ( num == OBJECT_KEYd         )  strcpy(text, "ClÈ D");
        if ( num == OBJECT_TNT          )  strcpy(text, "Explosif");
        if ( num == OBJECT_BOMB         )  strcpy(text, "Mine fixe");
        if ( num == OBJECT_BAG          )  strcpy(text, "Sac de survie");
        if ( num == OBJECT_WAYPOINT     )  strcpy(text, "Indicateur");
        if ( num == OBJECT_FLAGb        )  strcpy(text, "Drapeau bleu");
        if ( num == OBJECT_FLAGr        )  strcpy(text, "Drapeau rouge");
        if ( num == OBJECT_FLAGg        )  strcpy(text, "Drapeau vert");
        if ( num == OBJECT_FLAGy        )  strcpy(text, "Drapeau jaune");
        if ( num == OBJECT_FLAGv        )  strcpy(text, "Drapeau violet");
        if ( num == OBJECT_MARKPOWER    )  strcpy(text, "Emplacement pour station");
        if ( num == OBJECT_MARKURANIUM  )  strcpy(text, "Emplacement pour derrick (uranium)");
        if ( num == OBJECT_MARKKEYa     )  strcpy(text, "Emplacement pour derrick (clÈ A)");
        if ( num == OBJECT_MARKKEYb     )  strcpy(text, "Emplacement pour derrick (clÈ B)");
        if ( num == OBJECT_MARKKEYc     )  strcpy(text, "Emplacement pour derrick (clÈ C)");
        if ( num == OBJECT_MARKKEYd     )  strcpy(text, "Emplacement pour derrick (clÈ D)");
        if ( num == OBJECT_MARKSTONE    )  strcpy(text, "Emplacement pour derrick (titanium)");
        if ( num == OBJECT_MOBILEft     )  strcpy(text, "Robot d'entraÓnement");
        if ( num == OBJECT_MOBILEtt     )  strcpy(text, "Robot d'entraÓnement");
        if ( num == OBJECT_MOBILEwt     )  strcpy(text, "Robot d'entraÓnement");
        if ( num == OBJECT_MOBILEit     )  strcpy(text, "Robot d'entraÓnement");
        if ( num == OBJECT_MOBILEfa     )  strcpy(text, "Robot dÈmÈnageur");
        if ( num == OBJECT_MOBILEta     )  strcpy(text, "Robot dÈmÈnageur");
        if ( num == OBJECT_MOBILEwa     )  strcpy(text, "Robot dÈmÈnageur");
        if ( num == OBJECT_MOBILEia     )  strcpy(text, "Robot dÈmÈnageur");
        if ( num == OBJECT_MOBILEfc     )  strcpy(text, "Robot shooter");
        if ( num == OBJECT_MOBILEtc     )  strcpy(text, "Robot shooter");
        if ( num == OBJECT_MOBILEwc     )  strcpy(text, "Robot shooter");
        if ( num == OBJECT_MOBILEic     )  strcpy(text, "Robot shooter");
        if ( num == OBJECT_MOBILEfi     )  strcpy(text, "Robot orgaShooter");
        if ( num == OBJECT_MOBILEti     )  strcpy(text, "Robot orgaShooter");
        if ( num == OBJECT_MOBILEwi     )  strcpy(text, "Robot orgaShooter");
        if ( num == OBJECT_MOBILEii     )  strcpy(text, "Robot orgaShooter");
        if ( num == OBJECT_MOBILEfs     )  strcpy(text, "Robot renifleur");
        if ( num == OBJECT_MOBILEts     )  strcpy(text, "Robot renifleur");
        if ( num == OBJECT_MOBILEws     )  strcpy(text, "Robot renifleur");
        if ( num == OBJECT_MOBILEis     )  strcpy(text, "Robot renifleur");
        if ( num == OBJECT_MOBILErt     )  strcpy(text, "Robot secoueur");
        if ( num == OBJECT_MOBILErc     )  strcpy(text, "Robot phazer");
        if ( num == OBJECT_MOBILErr     )  strcpy(text, "Robot recycleur");
        if ( num == OBJECT_MOBILErs     )  strcpy(text, "Robot bouclier");
        if ( num == OBJECT_MOBILEsa     )  strcpy(text, "Robot sous-marin");
        if ( num == OBJECT_MOBILEtg     )  strcpy(text, "Cible d'entraÓnement");
        if ( num == OBJECT_MOBILEdr     )  strcpy(text, "Robot dessinateur");
        if ( num == OBJECT_HUMAN        )  strcpy(text, g_gamerName);
        if ( num == OBJECT_TECH         )  strcpy(text, "Technicien");
        if ( num == OBJECT_TOTO         )  strcpy(text, "Robbie");
        if ( num == OBJECT_MOTHER       )  strcpy(text, "Pondeuse");
        if ( num == OBJECT_ANT          )  strcpy(text, "Fourmi");
        if ( num == OBJECT_SPIDER       )  strcpy(text, "AraignÈe");
        if ( num == OBJECT_BEE          )  strcpy(text, "GuÍpe");
        if ( num == OBJECT_WORM         )  strcpy(text, "Ver");
        if ( num == OBJECT_EGG          )  strcpy(text, "Oeuf");
        if ( num == OBJECT_RUINmobilew1 )  strcpy(text, "Epave de robot");
        if ( num == OBJECT_RUINmobilew2 )  strcpy(text, "Epave de robot");
        if ( num == OBJECT_RUINmobilet1 )  strcpy(text, "Epave de robot");
        if ( num == OBJECT_RUINmobilet2 )  strcpy(text, "Epave de robot");
        if ( num == OBJECT_RUINmobiler1 )  strcpy(text, "Epave de robot");
        if ( num == OBJECT_RUINmobiler2 )  strcpy(text, "Epave de robot");
        if ( num == OBJECT_RUINfactory  )  strcpy(text, "B‚timent en ruine");
        if ( num == OBJECT_RUINdoor     )  strcpy(text, "B‚timent en ruine");
        if ( num == OBJECT_RUINsupport  )  strcpy(text, "DÈchet");
        if ( num == OBJECT_RUINradar    )  strcpy(text, "B‚timent en ruine");
        if ( num == OBJECT_RUINconvert  )  strcpy(text, "B‚timent en ruine");
        if ( num == OBJECT_RUINbase     )  strcpy(text, "Epave de vaisseau spatial");
        if ( num == OBJECT_RUINhead     )  strcpy(text, "Epave de vaisseau spatial");
        if ( num == OBJECT_APOLLO1      ||
             num == OBJECT_APOLLO3      ||
             num == OBJECT_APOLLO4      ||
             num == OBJECT_APOLLO5      )  strcpy(text, "Vestige d'une mission Apollo");
        if ( num == OBJECT_APOLLO2      )  strcpy(text, "Lunar Roving Vehicle");
    }

    if ( type == RES_ERR )
    {
        strcpy(text, "Erreur");
        if ( num == ERR_CMD             )  strcpy(text, "Commande inconnue");
#if _NEWLOOK
        if ( num == ERR_INSTALL         )  strcpy(text, "CeeBot n'est pas installÈ.");
        if ( num == ERR_NOCD            )  strcpy(text, "Veuillez mettre le CD de CeeBot\net relancer le jeu.");
#else
        if ( num == ERR_INSTALL         )  strcpy(text, "COLOBOT n'est pas installÈ.");
        if ( num == ERR_NOCD            )  strcpy(text, "Veuillez mettre le CD de COLOBOT\net relancer le jeu.");
#endif
        if ( num == ERR_MANIP_VEH       )  strcpy(text, "Robot inadaptÈ");
        if ( num == ERR_MANIP_FLY       )  strcpy(text, "Impossible en vol");
        if ( num == ERR_MANIP_BUSY      )  strcpy(text, "Porte dÈj‡ quelque chose");
        if ( num == ERR_MANIP_NIL       )  strcpy(text, "Rien ‡ prendre");
        if ( num == ERR_MANIP_MOTOR     )  strcpy(text, "Impossible en mouvement");
        if ( num == ERR_MANIP_OCC       )  strcpy(text, "Emplacement occupÈ");
        if ( num == ERR_MANIP_FRIEND    )  strcpy(text, "Pas d'autre robot");
        if ( num == ERR_MANIP_RADIO     )  strcpy(text, "Vous ne pouvez pas transporter un objet radioactif");
        if ( num == ERR_MANIP_WATER     )  strcpy(text, "Vous ne pouvez pas transporter un objet sous l'eau");
        if ( num == ERR_MANIP_EMPTY     )  strcpy(text, "Rien ‡ dÈposer");
        if ( num == ERR_BUILD_FLY       )  strcpy(text, "Impossible en vol");
        if ( num == ERR_BUILD_WATER     )  strcpy(text, "Impossible sous l'eau");
        if ( num == ERR_BUILD_ENERGY    )  strcpy(text, "Pas assez d'Ènergie");
        if ( num == ERR_BUILD_METALAWAY )  strcpy(text, "Titanium trop loin");
        if ( num == ERR_BUILD_METALNEAR )  strcpy(text, "Titanium trop proche");
        if ( num == ERR_BUILD_METALINEX )  strcpy(text, "Titanium inexistant");
        if ( num == ERR_BUILD_FLAT      )  strcpy(text, "Sol pas assez plat");
        if ( num == ERR_BUILD_FLATLIT   )  strcpy(text, "Sol plat pas assez grand");
        if ( num == ERR_BUILD_BUSY      )  strcpy(text, "Emplacement occupÈ");
        if ( num == ERR_BUILD_BASE      )  strcpy(text, "Trop proche du vaisseau spatial");
        if ( num == ERR_BUILD_NARROW    )  strcpy(text, "Trop proche d'un b‚timent");
        if ( num == ERR_BUILD_MOTOR     )  strcpy(text, "Impossible en mouvement");
        if ( num == ERR_SEARCH_FLY      )  strcpy(text, "Impossible en vol");
        if ( num == ERR_SEARCH_VEH      )  strcpy(text, "Robot inadaptÈ");
        if ( num == ERR_SEARCH_MOTOR    )  strcpy(text, "Impossible en mouvement");
        if ( num == ERR_TERRA_VEH       )  strcpy(text, "Robot inadaptÈ");
        if ( num == ERR_TERRA_ENERGY    )  strcpy(text, "Pas assez d'Ènergie");
        if ( num == ERR_TERRA_FLOOR     )  strcpy(text, "Terrain inadaptÈ");
        if ( num == ERR_TERRA_BUILDING  )  strcpy(text, "B‚timent trop proche");
        if ( num == ERR_TERRA_OBJECT    )  strcpy(text, "Objet trop proche");
        if ( num == ERR_RECOVER_VEH     )  strcpy(text, "Robot inadaptÈ");
        if ( num == ERR_RECOVER_ENERGY  )  strcpy(text, "Pas assez d'Ènergie");
        if ( num == ERR_RECOVER_NULL    )  strcpy(text, "Rien ‡ recycler");
        if ( num == ERR_SHIELD_VEH      )  strcpy(text, "Robot inadaptÈ");
        if ( num == ERR_SHIELD_ENERGY   )  strcpy(text, "Plus d'Ènergie");
        if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "DÈplacement impossible");
        if ( num == ERR_FIND_IMPOSSIBLE )  strcpy(text, "Objet n'existe pas");
        if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Chemin introuvable");
        if ( num == ERR_GOTO_ITER       )  strcpy(text, "Position inaccessible");
        if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Destination occupÈe");
        if ( num == ERR_FIRE_VEH        )  strcpy(text, "Robot inadaptÈ");
        if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Pas assez d'Ènergie");
        if ( num == ERR_FIRE_FLY        )  strcpy(text, "Impossible en vol");
        if ( num == ERR_CONVERT_EMPTY   )  strcpy(text, "Pas de minerai de titanium ‡ convertir");
        if ( num == ERR_DERRICK_NULL    )  strcpy(text, "Pas de minerai en sous-sol");
        if ( num == ERR_STATION_NULL    )  strcpy(text, "Pas d'Ènergie en sous-sol");
        if ( num == ERR_TOWER_POWER     )  strcpy(text, "Pas de pile");
        if ( num == ERR_TOWER_ENERGY    )  strcpy(text, "Plus d'Ènergie");
        if ( num == ERR_RESEARCH_POWER  )  strcpy(text, "Pas de pile");
        if ( num == ERR_RESEARCH_ENERGY )  strcpy(text, "Plus assez d'Ènergie");
        if ( num == ERR_RESEARCH_TYPE   )  strcpy(text, "Pas le bon type de pile");
        if ( num == ERR_RESEARCH_ALREADY)  strcpy(text, "Recherche dÈj‡ effectuÈe");
        if ( num == ERR_ENERGY_NULL     )  strcpy(text, "Pas d'Ènergie en sous-sol");
        if ( num == ERR_ENERGY_LOW      )  strcpy(text, "Pas encore assez d'Ènergie");
        if ( num == ERR_ENERGY_EMPTY    )  strcpy(text, "Pas de titanium ‡ transformer");
        if ( num == ERR_ENERGY_BAD      )  strcpy(text, "Ne transforme que le titanium");
        if ( num == ERR_BASE_DLOCK      )  strcpy(text, "Portes bloquÈes par un robot ou un objet");
        if ( num == ERR_BASE_DHUMAN     )  strcpy(text, "Vous devez embarquer pour pouvoir dÈcoller");
        if ( num == ERR_LABO_NULL       )  strcpy(text, "Rien ‡ analyser");
        if ( num == ERR_LABO_BAD        )  strcpy(text, "N'analyse que la matiËre organique");
        if ( num == ERR_LABO_ALREADY    )  strcpy(text, "Analyse dÈj‡ effectuÈe");
        if ( num == ERR_NUCLEAR_NULL    )  strcpy(text, "Pas d'Ènergie en sous-sol");
        if ( num == ERR_NUCLEAR_LOW     )  strcpy(text, "Pas encore assez d'Ènergie");
        if ( num == ERR_NUCLEAR_EMPTY   )  strcpy(text, "Pas d'uranium ‡ transformer");
        if ( num == ERR_NUCLEAR_BAD     )  strcpy(text, "Ne transforme que l'uranium");
        if ( num == ERR_FACTORY_NULL    )  strcpy(text, "Pas de titanium");
        if ( num == ERR_FACTORY_NEAR    )  strcpy(text, "Quelque chose est trop proche");
        if ( num == ERR_RESET_NEAR      )  strcpy(text, "Emplacement occupÈ");
        if ( num == ERR_INFO_NULL       )  strcpy(text, "Pas trouvÈ de borne d'information");
        if ( num == ERR_VEH_VIRUS       )  strcpy(text, "Un programme est infectÈ par un virus");
        if ( num == ERR_BAT_VIRUS       )  strcpy(text, "InfectÈ par un virus, ne fonctionne plus temporairement");
        if ( num == ERR_VEH_POWER       )  strcpy(text, "Pas de pile");
        if ( num == ERR_VEH_ENERGY      )  strcpy(text, "Plus d'Ènergie");
        if ( num == ERR_FLAG_FLY        )  strcpy(text, "Impossible en vol");
        if ( num == ERR_FLAG_WATER      )  strcpy(text, "Impossible en nageant");
        if ( num == ERR_FLAG_MOTOR      )  strcpy(text, "Impossible en mouvement");
        if ( num == ERR_FLAG_BUSY       )  strcpy(text, "Impossible en portant un objet");
        if ( num == ERR_FLAG_CREATE     )  strcpy(text, "Trop de drapeaux de cette couleur (maximum 5)");
        if ( num == ERR_FLAG_PROXY      )  strcpy(text, "Trop proche d'un drapeau existant");
        if ( num == ERR_FLAG_DELETE     )  strcpy(text, "Aucun drapeau ‡ proximitÈ");
        if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "La misssion n'est pas terminÈe (appuyez sur \\key help; pour plus de dÈtails)");
        if ( num == ERR_DELETEMOBILE    )  strcpy(text, "Robot dÈtruit");
        if ( num == ERR_DELETEBUILDING  )  strcpy(text, "B‚timent dÈtruit");
        if ( num == ERR_TOOMANY         )  strcpy(text, "CrÈation impossible, il y a trop d'objets");
        if ( num == ERR_OBLIGATORYTOKEN )  strcpy(text, "Il manque \"%s\" dans le programme");
        if ( num == ERR_PROHIBITEDTOKEN )  strcpy(text, "Interdit dans cet exercice");

        if ( num == INFO_BUILD          )  strcpy(text, "B‚timent terminÈ");
        if ( num == INFO_CONVERT        )  strcpy(text, "Titanium disponible");
        if ( num == INFO_RESEARCH       )  strcpy(text, "Recherche terminÈe");
        if ( num == INFO_RESEARCHTANK   )  strcpy(text, "Fabrication d'un robot ‡ chenilles possible");
        if ( num == INFO_RESEARCHFLY    )  strcpy(text, "Il est possible de voler avec les touches (\\key gup;) et (\\key gdown;)");
        if ( num == INFO_RESEARCHTHUMP  )  strcpy(text, "Fabrication d'un robot secoueur possible");
        if ( num == INFO_RESEARCHCANON  )  strcpy(text, "Fabrication de robots shooter possible");
        if ( num == INFO_RESEARCHTOWER  )  strcpy(text, "Construction d'une tour de dÈfense possible");
        if ( num == INFO_RESEARCHPHAZER )  strcpy(text, "Fabrication d'un robot phazer possible");
        if ( num == INFO_RESEARCHSHIELD )  strcpy(text, "Fabrication d'un robot bouclier possible");
        if ( num == INFO_RESEARCHATOMIC )  strcpy(text, "Construction d'une centrale nuclÈaire possible");
        if ( num == INFO_FACTORY        )  strcpy(text, "Nouveau robot disponible");
        if ( num == INFO_LABO           )  strcpy(text, "Analyse terminÈe");
        if ( num == INFO_ENERGY         )  strcpy(text, "Pile disponible");
        if ( num == INFO_NUCLEAR        )  strcpy(text, "Pile nuclÈaire disponible");
        if ( num == INFO_FINDING        )  strcpy(text, "Vous avez trouvÈ un objet utilisable");
        if ( num == INFO_MARKPOWER      )  strcpy(text, "Emplacement pour station trouvÈ");
        if ( num == INFO_MARKURANIUM    )  strcpy(text, "Emplacement pour derrick trouvÈ");
        if ( num == INFO_MARKSTONE      )  strcpy(text, "Emplacement pour derrick trouvÈ");
        if ( num == INFO_MARKKEYa       )  strcpy(text, "Emplacement pour derrick trouvÈ");
        if ( num == INFO_MARKKEYb       )  strcpy(text, "Emplacement pour derrick trouvÈ");
        if ( num == INFO_MARKKEYc       )  strcpy(text, "Emplacement pour derrick trouvÈ");
        if ( num == INFO_MARKKEYd       )  strcpy(text, "Emplacement pour derrick trouvÈ");
        if ( num == INFO_WIN            )  strcpy(text, "<<< Bravo, mission terminÈe >>>");
        if ( num == INFO_LOST           )  strcpy(text, "<<< DÈsolÈ, mission ÈchouÈe >>>");
        if ( num == INFO_LOSTq          )  strcpy(text, "<<< DÈsolÈ, mission ÈchouÈe >>>");
        if ( num == INFO_WRITEOK        )  strcpy(text, "Enregistrement effectuÈ");
        if ( num == INFO_DELETEPATH     )  strcpy(text, "Indicateur atteint");
        if ( num == INFO_DELETEMOTHER   )  strcpy(text, "Pondeuse mortellement touchÈe");
        if ( num == INFO_DELETEANT      )  strcpy(text, "Fourmi mortellement touchÈe");
        if ( num == INFO_DELETEBEE      )  strcpy(text, "GuÍpe mortellement touchÈe");
        if ( num == INFO_DELETEWORM     )  strcpy(text, "Ver mortellement touchÈ");
        if ( num == INFO_DELETESPIDER   )  strcpy(text, "AraignÈe mortellement touchÈe");
        if ( num == INFO_BEGINSATCOM    )  strcpy(text, "Consultez votre SatCom en appuyant sur \\key help;");
    }

    if ( type == RES_CBOT )
    {
        strcpy(text, "Erreur");
        if ( num == TX_OPENPAR       ) strcpy(text, "Il manque une parenthËse ouvrante");
        if ( num == TX_CLOSEPAR      ) strcpy(text, "Il manque une parenthËse fermante");
        if ( num == TX_NOTBOOL       ) strcpy(text, "L'expression doit Ítre un boolean");
        if ( num == TX_UNDEFVAR      ) strcpy(text, "Variable non dÈclarÈe");
        if ( num == TX_BADLEFT       ) strcpy(text, "Assignation impossible");
        if ( num == TX_ENDOF         ) strcpy(text, "Terminateur point-virgule non trouvÈ");
        if ( num == TX_OUTCASE       ) strcpy(text, "Instruction ""case"" hors d'un bloc ""switch""");
        if ( num == TX_NOTERM        ) strcpy(text, "Instructions aprËs la fin");
        if ( num == TX_CLOSEBLK      ) strcpy(text, "Il manque la fin du bloc");
        if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instruction ""else"" sans ""if"" correspondant");
        if ( num == TX_OPENBLK       ) strcpy(text, "DÈbut d'un bloc attendu");
        if ( num == TX_BADTYPE       ) strcpy(text, "Mauvais type de rÈsultat pour l'assignation");
        if ( num == TX_REDEFVAR      ) strcpy(text, "RedÈfinition d'une variable");
        if ( num == TX_BAD2TYPE      ) strcpy(text, "Les deux opÈrandes ne sont pas de types compatibles");
        if ( num == TX_UNDEFCALL     ) strcpy(text, "Routine inconnue");
        if ( num == TX_MISDOTS       ) strcpy(text, "SÈparateur "" : "" attendu");
        if ( num == TX_WHILE         ) strcpy(text, "Manque le mot ""while""");
        if ( num == TX_BREAK         ) strcpy(text, "Instruction ""break"" en dehors d'une boucle");
        if ( num == TX_LABEL         ) strcpy(text, "Un label ne peut se placer que devant un ""for"", un ""while"", un ""do"" ou un ""switch""");
        if ( num == TX_NOLABEL       ) strcpy(text, "Cette Ètiquette n'existe pas");
        if ( num == TX_NOCASE        ) strcpy(text, "Manque une instruction ""case""");
        if ( num == TX_BADNUM        ) strcpy(text, "Un nombre est attendu");
        if ( num == TX_VOID          ) strcpy(text, "ParamËtre void");
        if ( num == TX_NOTYP         ) strcpy(text, "DÈclaration de type attendu");
        if ( num == TX_NOVAR         ) strcpy(text, "Nom d'une variable attendu");
        if ( num == TX_NOFONC        ) strcpy(text, "Nom de la fonction attendu");
        if ( num == TX_OVERPARAM     ) strcpy(text, "Trop de paramËtres");
        if ( num == TX_REDEF         ) strcpy(text, "Cette fonction existe dÈj‡");
        if ( num == TX_LOWPARAM      ) strcpy(text, "Pas assez de paramËtres");
        if ( num == TX_BADPARAM      ) strcpy(text, "Aucune fonction de ce nom n'accepte ce(s) type(s) de paramËtre(s)");
        if ( num == TX_NUMPARAM      ) strcpy(text, "Aucune fonction de ce nom n'accepte ce nombre de paramËtres");
        if ( num == TX_NOITEM        ) strcpy(text, "Cet ÈlÈment n'existe pas dans cette classe");
        if ( num == TX_DOT           ) strcpy(text, "L'objet n'est pas une instance d'une classe");
        if ( num == TX_NOCONST       ) strcpy(text, "Il n'y a pas de constructeur appropriÈ");
        if ( num == TX_REDEFCLASS    ) strcpy(text, "Cette classe existe dÈj‡");
        if ( num == TX_CLBRK         ) strcpy(text, """ ] "" attendu");
        if ( num == TX_RESERVED      ) strcpy(text, "Ce mot est rÈservÈ");
        if ( num == TX_BADNEW        ) strcpy(text, "Mauvais argument pour ""new""");
        if ( num == TX_OPBRK         ) strcpy(text, """ [ "" attendu");
        if ( num == TX_BADSTRING     ) strcpy(text, "Une chaÓne de caractËre est attendue");
        if ( num == TX_BADINDEX      ) strcpy(text, "Mauvais type d'index");
        if ( num == TX_PRIVATE       ) strcpy(text, "ElÈment protÈgÈ");
        if ( num == TX_NOPUBLIC      ) strcpy(text, "Public requis");
        if ( num == TX_DIVZERO       ) strcpy(text, "Division par zÈro");
        if ( num == TX_NOTINIT       ) strcpy(text, "Variable non initialisÈe");
        if ( num == TX_BADTHROW      ) strcpy(text, "Valeur nÈgative refusÈe pour ""throw""");
        if ( num == TX_NORETVAL      ) strcpy(text, "La fonction n'a pas retournÈ de rÈsultat");
        if ( num == TX_NORUN         ) strcpy(text, "Pas de fonction en exÈcution");
        if ( num == TX_NOCALL        ) strcpy(text, "Appel d'une fonction inexistante");
        if ( num == TX_NOCLASS       ) strcpy(text, "Cette classe n'existe pas");
        if ( num == TX_NULLPT        ) strcpy(text, "Objet n'existe pas");
        if ( num == TX_OPNAN         ) strcpy(text, "OpÈration sur un ""nan""");
        if ( num == TX_OUTARRAY      ) strcpy(text, "AccËs hors du tableau");
        if ( num == TX_STACKOVER     ) strcpy(text, "DÈbordement de la pile");
        if ( num == TX_DELETEDPT     ) strcpy(text, "Objet inaccessible");
        if ( num == TX_FILEOPEN      ) strcpy(text, "Ouverture du fichier impossible");
        if ( num == TX_NOTOPEN       ) strcpy(text, "Le fichier n'est pas ouvert");
        if ( num == TX_ERRREAD       ) strcpy(text, "Erreur ‡ la lecture");
        if ( num == TX_ERRWRITE      ) strcpy(text, "Erreur ‡ l'Ècriture");
    }

    if ( type == RES_KEY )
    {
        if ( num == 0                   )  strcpy(text, "< aucune >");
        if ( num == VK_LEFT             )  strcpy(text, "FlËche Gauche");
        if ( num == VK_RIGHT            )  strcpy(text, "FlËche Droite");
        if ( num == VK_UP               )  strcpy(text, "FlËche Haut");
        if ( num == VK_DOWN             )  strcpy(text, "FlËche Bas");
        if ( num == VK_CANCEL           )  strcpy(text, "Control-break");
        if ( num == VK_BACK             )  strcpy(text, "<--");
        if ( num == VK_TAB              )  strcpy(text, "Tab");
        if ( num == VK_CLEAR            )  strcpy(text, "Clear");
        if ( num == VK_RETURN           )  strcpy(text, "EntrÈe");
        if ( num == VK_SHIFT            )  strcpy(text, "Shift");
        if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
        if ( num == VK_MENU             )  strcpy(text, "Alt");
        if ( num == VK_PAUSE            )  strcpy(text, "Pause");
        if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
        if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
        if ( num == VK_SPACE            )  strcpy(text, "Espace");
        if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
        if ( num == VK_NEXT             )  strcpy(text, "Page Down");
        if ( num == VK_END              )  strcpy(text, "End");
        if ( num == VK_HOME             )  strcpy(text, "Home");
        if ( num == VK_SELECT           )  strcpy(text, "Select");
        if ( num == VK_EXECUTE          )  strcpy(text, "Execute");
        if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
        if ( num == VK_INSERT           )  strcpy(text, "Insert");
        if ( num == VK_DELETE           )  strcpy(text, "Delete");
        if ( num == VK_HELP             )  strcpy(text, "Help");
        if ( num == VK_LWIN             )  strcpy(text, "Left Windows");
        if ( num == VK_RWIN             )  strcpy(text, "Right Windows");
        if ( num == VK_APPS             )  strcpy(text, "Application key");
        if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
        if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
        if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
        if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
        if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
        if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
        if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
        if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
        if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
        if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
        if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
        if ( num == VK_ADD              )  strcpy(text, "NumPad +");
        if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
        if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
        if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
        if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
        if ( num == VK_F1               )  strcpy(text, "F1");
        if ( num == VK_F2               )  strcpy(text, "F2");
        if ( num == VK_F3               )  strcpy(text, "F3");
        if ( num == VK_F4               )  strcpy(text, "F4");
        if ( num == VK_F5               )  strcpy(text, "F5");
        if ( num == VK_F6               )  strcpy(text, "F6");
        if ( num == VK_F7               )  strcpy(text, "F7");
        if ( num == VK_F8               )  strcpy(text, "F8");
        if ( num == VK_F9               )  strcpy(text, "F9");
        if ( num == VK_F10              )  strcpy(text, "F10");
        if ( num == VK_F11              )  strcpy(text, "F11");
        if ( num == VK_F12              )  strcpy(text, "F12");
        if ( num == VK_F13              )  strcpy(text, "F13");
        if ( num == VK_F14              )  strcpy(text, "F14");
        if ( num == VK_F15              )  strcpy(text, "F15");
        if ( num == VK_F16              )  strcpy(text, "F16");
        if ( num == VK_F17              )  strcpy(text, "F17");
        if ( num == VK_F18              )  strcpy(text, "F18");
        if ( num == VK_F19              )  strcpy(text, "F19");
        if ( num == VK_F20              )  strcpy(text, "F20");
        if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
        if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
        if ( num == VK_ATTN             )  strcpy(text, "Attn");
        if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
        if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
        if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
        if ( num == VK_PLAY             )  strcpy(text, "Play");
        if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
        if ( num == VK_PA1              )  strcpy(text, "PA1");
        if ( num == VK_OEM_CLEAR        )  strcpy(text, "Clear");
        if ( num == VK_BUTTON1          )  strcpy(text, "Bouton 1");
        if ( num == VK_BUTTON2          )  strcpy(text, "Bouton 2");
        if ( num == VK_BUTTON3          )  strcpy(text, "Bouton 3");
        if ( num == VK_BUTTON4          )  strcpy(text, "Bouton 4");
        if ( num == VK_BUTTON5          )  strcpy(text, "Bouton 5");
        if ( num == VK_BUTTON6          )  strcpy(text, "Bouton 6");
        if ( num == VK_BUTTON7          )  strcpy(text, "Bouton 7");
        if ( num == VK_BUTTON8          )  strcpy(text, "Bouton 8");
        if ( num == VK_BUTTON9          )  strcpy(text, "Bouton 9");
        if ( num == VK_BUTTON10         )  strcpy(text, "Bouton 10");
        if ( num == VK_BUTTON11         )  strcpy(text, "Bouton 11");
        if ( num == VK_BUTTON12         )  strcpy(text, "Bouton 12");
        if ( num == VK_BUTTON13         )  strcpy(text, "Bouton 13");
        if ( num == VK_BUTTON14         )  strcpy(text, "Bouton 14");
        if ( num == VK_BUTTON15         )  strcpy(text, "Bouton 15");
        if ( num == VK_BUTTON16         )  strcpy(text, "Bouton 16");
        if ( num == VK_BUTTON17         )  strcpy(text, "Bouton 17");
        if ( num == VK_BUTTON18         )  strcpy(text, "Bouton 18");
        if ( num == VK_BUTTON19         )  strcpy(text, "Bouton 19");
        if ( num == VK_BUTTON20         )  strcpy(text, "Bouton 20");
        if ( num == VK_BUTTON21         )  strcpy(text, "Bouton 21");
        if ( num == VK_BUTTON22         )  strcpy(text, "Bouton 22");
        if ( num == VK_BUTTON23         )  strcpy(text, "Bouton 23");
        if ( num == VK_BUTTON24         )  strcpy(text, "Bouton 24");
        if ( num == VK_BUTTON25         )  strcpy(text, "Bouton 25");
        if ( num == VK_BUTTON26         )  strcpy(text, "Bouton 26");
        if ( num == VK_BUTTON27         )  strcpy(text, "Bouton 27");
        if ( num == VK_BUTTON28         )  strcpy(text, "Bouton 28");
        if ( num == VK_BUTTON29         )  strcpy(text, "Bouton 29");
        if ( num == VK_BUTTON30         )  strcpy(text, "Bouton 30");
        if ( num == VK_BUTTON31         )  strcpy(text, "Bouton 31");
        if ( num == VK_BUTTON32         )  strcpy(text, "Bouton 32");
        if ( num == VK_WHEELUP          )  strcpy(text, "Molette haut");
        if ( num == VK_WHEELDOWN        )  strcpy(text, "Molette bas");
    }
#endif

#if _GERMAN | _WG
    if ( type == RES_TEXT )
    {
        #if _FULL
        if ( num == RT_VERSION_ID          )  strcpy(text, "1.18 /d");
        #endif
        #if _NET
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A 1.18");
        #endif
        #if _SCHOOL & _EDU
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen EDU 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A EDU 1.18");
        #endif
        #endif
        #if _SCHOOL & _PERSO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen PERSO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A PERSO 1.18");
        #endif
        #endif
        #if _SCHOOL & _CEEBOTDEMO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen DEMO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A DEMO 1.18");
        #endif
        #endif
        #if _DEMO
        if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.18 /d");
        #endif
        if ( num == RT_DISINFO_TITLE       )  strcpy(text, "SatCom");
        if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Groﬂes Fenster");
        if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Reduzieren");
        if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normale Grˆﬂe");
        if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Schlieﬂen");

        if ( num == RT_STUDIO_TITLE        )  strcpy(text, "Programmeditor");
        if ( num == RT_SCRIPT_NEW          )  strcpy(text, "Neu");
        if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Spieler");
        if ( num == RT_IO_NEW              )  strcpy(text, "Neu ...");
        if ( num == RT_KEY_OR              )  strcpy(text, " oder ");

#if _NEWLOOK
        if ( num == RT_TITLE_BASE          )  strcpy(text, "CeeBot");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "CeeBot");
#else
        if ( num == RT_TITLE_BASE          )  strcpy(text, "COLOBOT");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "COLOBOT");
#endif
#if _SCHOOL
        if ( num == RT_TITLE_TRAINER       )  strcpy(text, "‹bungen");
#else
        if ( num == RT_TITLE_TRAINER       )  strcpy(text, "Programmieren");
#endif
        if ( num == RT_TITLE_DEFI          )  strcpy(text, "Challenges");
        if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missionen");
        if ( num == RT_TITLE_FREE          )  strcpy(text, "Freestyle");
        if ( num == RT_TITLE_TEEN          )  strcpy(text, "Freestyle");
        if ( num == RT_TITLE_USER          )  strcpy(text, "Userlevels");
        if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypen");
        if ( num == RT_TITLE_SETUP         )  strcpy(text, "Einstellungen");
        if ( num == RT_TITLE_NAME          )  strcpy(text, "Name ");
        if ( num == RT_TITLE_PERSO         )  strcpy(text, "Aussehen einstellen");
        if ( num == RT_TITLE_WRITE         )  strcpy(text, "Aktuelle Mission speichern");
        if ( num == RT_TITLE_READ          )  strcpy(text, "Gespeicherte Mission laden");

        if ( num == RT_PLAY_CHAPt          )  strcpy(text, " Liste der Kapitel:");
        if ( num == RT_PLAY_CHAPd          )  strcpy(text, " Liste der Kapitel:");
        if ( num == RT_PLAY_CHAPm          )  strcpy(text, " Liste der Planeten:");
        if ( num == RT_PLAY_CHAPf          )  strcpy(text, " Liste der Planeten:");
        if ( num == RT_PLAY_CHAPu          )  strcpy(text, " Userlevels:");
        if ( num == RT_PLAY_CHAPp          )  strcpy(text, " Liste der Planeten:");
        if ( num == RT_PLAY_CHAPte         )  strcpy(text, " Liste der Kapitel:");
        if ( num == RT_PLAY_LISTt          )  strcpy(text, " Liste der ‹bungen des Kapitels:");
        if ( num == RT_PLAY_LISTd          )  strcpy(text, " Liste der Challenges des Kapitels:");
        if ( num == RT_PLAY_LISTm          )  strcpy(text, " Liste der Missionen des Planeten:");
        if ( num == RT_PLAY_LISTf          )  strcpy(text, " Liste der freien Levels des Planeten:");
        if ( num == RT_PLAY_LISTu          )  strcpy(text, " Missionen des Userlevels:");
        if ( num == RT_PLAY_LISTp          )  strcpy(text, " Liste der Prototypen des Planeten:");
        if ( num == RT_PLAY_LISTk          )  strcpy(text, " Liste der freien Levels des Kapitel:");
        if ( num == RT_PLAY_RESUME         )  strcpy(text, " Zusammenfassung:");

        if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Driver:");
        if ( num == RT_SETUP_MODE          )  strcpy(text, " Auflˆsung:");
        if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Klicken Sie auf die neu zu definierende Taste.");
        if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Dr¸cken Sie auf die neue Taste.");

        if ( num == RT_PERSO_FACE          )  strcpy(text, "Kopf:");
        if ( num == RT_PERSO_GLASSES       )  strcpy(text, "Brille:");
        if ( num == RT_PERSO_HAIR          )  strcpy(text, "Haarfarbe:");
        if ( num == RT_PERSO_COMBI         )  strcpy(text, "Farbe des Anzugs:");
        if ( num == RT_PERSO_BAND          )  strcpy(text, "Farbe der Streifen:");

#if _NEWLOOK
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "CeeBot");
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Wollen Sie CeeBot schlieﬂen ?");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Schlieﬂen\\CeeBot schlieﬂen");
#else
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "COLOBOT");
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Wollen Sie COLOBOT schlieﬂen ?");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Schlieﬂen\\COLOBOT schlieﬂen");
#endif
        if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Mission abbrechen ?");
        if ( num == RT_DIALOG_YES          )  strcpy(text, "Abbrechen\\Mission abbrechen");
        if ( num == RT_DIALOG_NO           )  strcpy(text, "Weitermachen\\Mission weitermachen");
        if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Weitermachen\\Weitermachen");
        if ( num == RT_DIALOG_DELOBJ       )  strcpy(text, "Wollen Sie das angew‰hlte Geb‰ude wirklich zerstˆren ?");
        if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Wollen Sie die gespeicherten Missionen von %s lˆschen ?");
        if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Zerstˆren");
        if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Abbrechen");
        if ( num == RT_DIALOG_LOADING      )  strcpy(text, "Laden");

        if ( num == RT_STUDIO_LISTTT       )  strcpy(text, "Hilfe ¸ber den Begriff (\\key cbot;)");
        if ( num == RT_STUDIO_COMPOK       )  strcpy(text, "Kompilieren OK (0 Fehler)");
        if ( num == RT_STUDIO_PROGSTOP     )  strcpy(text, "Programm beendet");

        if ( num == RT_SATCOM_LIST         )  strcpy(text, "\\b;Liste der Objekte\n");
        if ( num == RT_SATCOM_BOT          )  strcpy(text, "\\b;Liste der Roboter\n");
        if ( num == RT_SATCOM_BUILDING     )  strcpy(text, "\\b;Listes der Geb‰ude\n");
        if ( num == RT_SATCOM_FRET         )  strcpy(text, "\\b;Listes der tragbaren Gegenst‰nde\n");
        if ( num == RT_SATCOM_ALIEN        )  strcpy(text, "\\b;Listes der Feinde\n");
        if ( num == RT_SATCOM_NULL         )  strcpy(text, "\\c; (keine)\\n;\n");
        if ( num == RT_SATCOM_ERROR1       )  strcpy(text, "\\b;Fehler\n");
        if ( num == RT_SATCOM_ERROR2       )  strcpy(text, "Die Liste ist ohne \\l;Radar\\u object\\radar; nicht verf¸gbar !\n");

        if ( num == RT_IO_OPEN             )  strcpy(text, "÷ffnen");
        if ( num == RT_IO_SAVE             )  strcpy(text, "Speichern");
        if ( num == RT_IO_LIST             )  strcpy(text, "Ordner: %s");
        if ( num == RT_IO_NAME             )  strcpy(text, "Name:");
        if ( num == RT_IO_DIR              )  strcpy(text, "In:");
        if ( num == RT_IO_PRIVATE          )  strcpy(text, "Privat\\Privater Ordner");
        if ( num == RT_IO_PUBLIC           )  strcpy(text, "÷ffentlich\\Gemeinsamer Ordner f¸r alle Spieler");

        if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Entwickelt von:");
        if ( num == RT_GENERIC_DEV2        )  strcpy(text, "www.epsitec.com");
#if _WG
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, "Herausgegeben von:");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, "www.wg-verlag.ch");
#else
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, " ");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, " ");
#endif

        if ( num == RT_INTERFACE_REC       )  strcpy(text, "Recorder");
    }

    if ( type == RES_EVENT )
    {
        if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
        if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Abbrechen");
        if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "N‰chster");
        if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Vorherg.");
        if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Men¸ (\\key quit;)");

        if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
        if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Abbrechen");

#if _SCHOOL
        if ( num == EVENT_INTERFACE_TRAINER)  strcpy(text, "‹bungen\\Programmier¸bungen");
#else
        if ( num == EVENT_INTERFACE_TRAINER)  strcpy(text, "Programmieren\\Programmier¸bungen");
#endif
        if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Challenges\\Herausforderungen");
        if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missionen\\Aufbruch ins Weltall");
        if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Freestyle\\Freies Spielen ohne vorgegebenes Ziel");
        if ( num == EVENT_INTERFACE_TEEN   )  strcpy(text, "Freestyle\\Freies Spielen ohne vorgegebenes Ziel");
        if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "User\\Userlevels");
        if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\In Entwicklung befindliche Prototypen");
        if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Anderer Spieler\\Spielername ‰ndern");
        if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Einstellungen\\Einstellungen");
        if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Neu anfangen\\Die Mission von vorne anfangen");
        if ( num == EVENT_INTERFACE_WRITE  )  strcpy(text, "Speichern\\Aktuelle Mission speichern");
        if ( num == EVENT_INTERFACE_READ   )  strcpy(text, "Laden\\Eine gespeicherte Mission ˆffnen");
#if _NEWLOOK
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Zur¸ck zu CeeBot");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Schlieﬂen\\CeeBot schlieﬂen");
#else
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Zur¸ck zu COLOBOT");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Schlieﬂen\\COLOBOT schlieﬂen");
#endif
        if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "<<  Zur¸ck  \\Zur¸ck zum Hauptmen¸");
        if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Spielen ...\\Los geht's");
        if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Bildschirm\\Driver und Bildschirmauflˆsung");
        if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Grafik\\Grafische Einstellungen");
        if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Spiel\\Gameplay Einstellungen");
        if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Steuerung\\Auswahl der Tasten");
        if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Ger‰usche\\Lautst‰rke Ger‰usche und Musik");
        if ( num == EVENT_INTERFACE_DEVICE )  strcpy(text, "Einheit");
        if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Auflˆsung");
        if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Vollbildschirm\\Vollbildschirm oder Fenster");
        if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "ƒnderungen ausf¸hren\\Get‰tigte Einstellungen ausf¸hren");

        if ( num == EVENT_INTERFACE_TOTO   )  strcpy(text, "Robby\\Ihr Assistent");
        if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Schatten\\Schlagschatten auf dem Boden");
        if ( num == EVENT_INTERFACE_GROUND )  strcpy(text, "Markierungen\\Markierungen auf dem Boden");
        if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Schmutz\\Schmutz auf Robotern und Bauten");
        if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Nebel\\Nebelschwaden");
        if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Sonnenstrahlen\\Sonnenstrahlen");
        if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Himmel\\Himmel und Wolken");
        if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planeten und Sterne\\Kreisende Planeten und Sterne");
        if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Dynamische Beleuchtung\\Dynamische Beleuchtung");
        if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Anzahl Partikel\\Explosionen, Staub, usw.");
        if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Sichtweite\\Maximale Sichtweite");
        if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Details\\Detailliertheit der Objekte in 3D");
        if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Qualit‰t der Texturen\\Qualit‰t der Anzeige");
        if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Anzahl Ziergegenst‰nde\\Anzahl Gegenst‰nde ohne Funktion");
        if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Partikel in den Men¸s\\Funken und Sterne in den Men¸s");
        if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Gl‰nzende Tasten\\Gl‰nzende Tasten in den Men¸s");
        if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Hilfsblasen\\Hilfsblasen");
        if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Filme\\Filme vor und nach den Missionen");
        if ( num == EVENT_INTERFACE_NICERST)  strcpy(text, "Zur¸cksetzen \\Kleine Show beim Zur¸cksetzen in den ‹bungen");
        if ( num == EVENT_INTERFACE_HIMSELF)  strcpy(text, "Eigenbeschuss\\Ihre Einheiten werden von Ihren Waffen besch‰digt.");
        if ( num == EVENT_INTERFACE_SCROLL )  strcpy(text, "Kameradrehung mit der Maus\\Die Kamera dreht wenn die Maus den Rand erreicht");
        if ( num == EVENT_INTERFACE_INVERTX)  strcpy(text, "Umkehr X\\Umkehr der Kameradrehung X-Achse");
        if ( num == EVENT_INTERFACE_INVERTY)  strcpy(text, "Umkehr Y\\Umkehr der Kameradrehung Y-Achse");
        if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Beben bei Explosionen\\Die Kamera bebt bei Explosionen");
        if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Schatten unter der Maus\\Ein Schatten erscheint unter der Maus");
        if ( num == EVENT_INTERFACE_EDITMODE) strcpy(text, "Automatisches Einr¸cken\\Beim Bearbeiten der Programme");
        if ( num == EVENT_INTERFACE_EDITVALUE)strcpy(text, "Einr¸cken mit 4 Leerstellen\\Einr¸cken mit 2 oder 4 Leerstellen");
        if ( num == EVENT_INTERFACE_SOLUCE4)  strcpy(text, "Lˆsung zug‰nglich\\Die Lˆsung ist im Programmslot \"4: Lˆsung\" zug‰nglich");

        if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Alles zur¸cksetzen\\Standarddefinition aller Tasten");
        if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Drehung nach links\\Steuer links");
        if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Drehung nach rechts\\Steuer rechts");
        if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Vorw‰rts\\Bewegung nach vorne");
        if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "R¸ckw‰rts\\Bewegung nach hinten");
        if ( num == EVENT_INTERFACE_KGUP   )  strcpy(text, "Steigen\\Leistung des Triebwerks steigern");
        if ( num == EVENT_INTERFACE_KGDOWN )  strcpy(text, "Sinken\\Leistung des Triebwerks drosseln");
        if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Andere Kamera\\Sichtpunkt einstellen");
        if ( num == EVENT_INTERFACE_KDESEL )  strcpy(text, "Vorherg. Auswahl\\Das vorhergehende Objekt ausw‰hlen");
        if ( num == EVENT_INTERFACE_KACTION)  strcpy(text, "Standardhandlung\\F¸hrt die Standardhandlung des Roboters aus.");
        if ( num == EVENT_INTERFACE_KNEAR  )  strcpy(text, "Kamera n‰her\\Bewegung der Kamera vorw‰rts");
        if ( num == EVENT_INTERFACE_KAWAY  )  strcpy(text, "Kamera weiter\\Bewegung der Kamera r¸ckw‰rts");
        if ( num == EVENT_INTERFACE_KNEXT  )  strcpy(text, "N‰chstes ausw‰hlen\\N‰chstes Objekt ausw‰hlen");
        if ( num == EVENT_INTERFACE_KHUMAN )  strcpy(text, "Astronauten ausw‰hlen\\Astronauten ausw‰hlen");
        if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Mission verlassen\\Eine Mission oder ‹bung verlassen");
        if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Anweisungen\\Anweisungen f¸r die Mission oder ‹bung");
        if ( num == EVENT_INTERFACE_KPROG  )  strcpy(text, "Hilfe CBOT-Sprache\\Hilfe ¸ber die Programmiersprache CBOT");
        if ( num == EVENT_INTERFACE_KCBOT  )  strcpy(text, "Hilfe ¸ber Begriff\\Hilfe ¸ber einen Begriff");
        if ( num == EVENT_INTERFACE_KVISIT )  strcpy(text, "Ort der Meldung\\Zeigt den Ort, von dem die letzte Meldung stammt");
        if ( num == EVENT_INTERFACE_KSPEED10) strcpy(text, "Geschwindigkeit 1.0x\\Normale Spielgeschwindigkeit");
        if ( num == EVENT_INTERFACE_KSPEED15) strcpy(text, "Geschwindigkeit 1.5x\\Spielgeschwindigkeit anderthalb Mal schneller");
        if ( num == EVENT_INTERFACE_KSPEED20) strcpy(text, "Geschwindigkeit 2.0x\\Spielgeschwindigkeit doppelt so schnell");
        if ( num == EVENT_INTERFACE_KSPEED30) strcpy(text, "Geschwindigkeit 3.0x\\Spielgeschwindigkeit drei Mal schneller");

        if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Ger‰usche:\\Lautst‰rke Motoren, Stimmen, usw.");
        if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Ger‰uschkulisse:\\Lautst‰rke der Soundtracks der CD");
        if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "3D-Ger‰usche\\Orten der Ger‰usche im Raum");

        if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Min.\\Minimale Qualit‰t (groﬂes Framerate)");
        if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Standardqualit‰t");
        if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Max.\\Beste Qualit‰t (niedriges Framerate)");

        if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Kein Ton\\Keine Ger‰usche und Ger‰uschkulisse");
        if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Normale Lautst‰rke");

        if ( num == EVENT_INTERFACE_JOYSTICK) strcpy(text, "Joystick\\Joystick oder Tastatur");
        if ( num == EVENT_INTERFACE_SOLUCE )  strcpy(text, "Zeigt die Lˆsung\\Zeigt nach 3mal Scheitern die Lˆsung");

        if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Name des Spielers");
        if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "OK\\Spieler ausw‰hlen");
        if ( num == EVENT_INTERFACE_NCANCEL)  strcpy(text, "Abbrechen\\Beh‰lt den bisherigen Spieler bei");
        if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Spieler lˆschen\\Lˆscht den Spieler aus der Liste");
        if ( num == EVENT_INTERFACE_NLABEL )  strcpy(text, "Name ");

        if ( num == EVENT_INTERFACE_IOWRITE)  strcpy(text, "Speichern\\Speichert die Mission");
        if ( num == EVENT_INTERFACE_IOREAD )  strcpy(text, "Laden\\÷ffnet eine gespeicherte Mission");
        if ( num == EVENT_INTERFACE_IOLIST )  strcpy(text, "Liste der gespeicherten Missionen");
        if ( num == EVENT_INTERFACE_IOLABEL)  strcpy(text, "Dateiname:");
        if ( num == EVENT_INTERFACE_IONAME )  strcpy(text, "Name der Mission");
        if ( num == EVENT_INTERFACE_IOIMAGE)  strcpy(text, "Ansicht der Mission");
        if ( num == EVENT_INTERFACE_IODELETE) strcpy(text, "Lˆschen\\Lˆscht die gespeicherte Mission");

        if ( num == EVENT_INTERFACE_PERSO  )  strcpy(text, "Aussehen\\Erscheinungsbild des Astronauten einstellen");
        if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "OK");
        if ( num == EVENT_INTERFACE_PCANCEL)  strcpy(text, "Abbrechen");
        if ( num == EVENT_INTERFACE_PDEF   )  strcpy(text, "Standard\\Standardfarben einsetzen");
        if ( num == EVENT_INTERFACE_PHEAD  )  strcpy(text, "Kopf\\Gesicht und Haare");
        if ( num == EVENT_INTERFACE_PBODY  )  strcpy(text, "Anzug\\Raumfahrtanzug");
        if ( num == EVENT_INTERFACE_PLROT  )  strcpy(text, "\\Drehung links");
        if ( num == EVENT_INTERFACE_PRROT  )  strcpy(text, "\\Drehung rechts");
        if ( num == EVENT_INTERFACE_PCRa   )  strcpy(text, "Rot");
        if ( num == EVENT_INTERFACE_PCGa   )  strcpy(text, "Gr¸n");
        if ( num == EVENT_INTERFACE_PCBa   )  strcpy(text, "Blau");
        if ( num == EVENT_INTERFACE_PCRb   )  strcpy(text, "Rot");
        if ( num == EVENT_INTERFACE_PCGb   )  strcpy(text, "Gr¸n");
        if ( num == EVENT_INTERFACE_PCBb   )  strcpy(text, "Blau");
        if ( num == EVENT_INTERFACE_PFACE1 )  strcpy(text, "\\Kopf 1");
        if ( num == EVENT_INTERFACE_PFACE2 )  strcpy(text, "\\Kopf 4");
        if ( num == EVENT_INTERFACE_PFACE3 )  strcpy(text, "\\Kopf 3");
        if ( num == EVENT_INTERFACE_PFACE4 )  strcpy(text, "\\Kopf 2");
        if ( num == EVENT_INTERFACE_PGLASS0)  strcpy(text, "\\Keine Brille");
        if ( num == EVENT_INTERFACE_PGLASS1)  strcpy(text, "\\Brille 1");
        if ( num == EVENT_INTERFACE_PGLASS2)  strcpy(text, "\\Brille 2");
        if ( num == EVENT_INTERFACE_PGLASS3)  strcpy(text, "\\Brille 3");
        if ( num == EVENT_INTERFACE_PGLASS4)  strcpy(text, "\\Brille 4");
        if ( num == EVENT_INTERFACE_PGLASS5)  strcpy(text, "\\Brille 5");

        if ( num == EVENT_OBJECT_DESELECT  )  strcpy(text, "Vorherg. Auwahl (\\key desel;)");
        if ( num == EVENT_OBJECT_LEFT      )  strcpy(text, "Drehung links (\\key left;)");
        if ( num == EVENT_OBJECT_RIGHT     )  strcpy(text, "Drehung rechts (\\key right;)");
        if ( num == EVENT_OBJECT_UP        )  strcpy(text, "Vorw‰rts (\\key up;)");
        if ( num == EVENT_OBJECT_DOWN      )  strcpy(text, "R¸ckw‰rts (\\key down;)");
        if ( num == EVENT_OBJECT_GASUP     )  strcpy(text, "Steigt (\\key gup;)");
        if ( num == EVENT_OBJECT_GASDOWN   )  strcpy(text, "Sinkt (\\key gdown;)");
        if ( num == EVENT_OBJECT_HTAKE     )  strcpy(text, "Nehmen oder hinlegen  (\\key action;)");
        if ( num == EVENT_OBJECT_MTAKE     )  strcpy(text, "Nehmen oder hinlegen  (\\key action;)");
        if ( num == EVENT_OBJECT_MFRONT    )  strcpy(text, "..vorne");
        if ( num == EVENT_OBJECT_MBACK     )  strcpy(text, "..hinten");
        if ( num == EVENT_OBJECT_MPOWER    )  strcpy(text, "..Batterie");
        if ( num == EVENT_OBJECT_BHELP     )  strcpy(text, "Anweisungen ¸ber die Mission(\\key help;)");
        if ( num == EVENT_OBJECT_BTAKEOFF  )  strcpy(text, "Abheben nach vollbrachter Mission");
        if ( num == EVENT_OBJECT_BDERRICK  )  strcpy(text, "Baut einen Bohrturm");
        if ( num == EVENT_OBJECT_BSTATION  )  strcpy(text, "Baut ein Kraftwerk");
        if ( num == EVENT_OBJECT_BFACTORY  )  strcpy(text, "Baut eine Roboterfabrik");
        if ( num == EVENT_OBJECT_BREPAIR   )  strcpy(text, "Baut ein Reparaturzentrum");
        if ( num == EVENT_OBJECT_BCONVERT  )  strcpy(text, "Baut einen Konverter");
        if ( num == EVENT_OBJECT_BTOWER    )  strcpy(text, "Baut einen Gesch¸tzturm");
        if ( num == EVENT_OBJECT_BRESEARCH )  strcpy(text, "Baut ein Forschungszentrum");
        if ( num == EVENT_OBJECT_BRADAR    )  strcpy(text, "Baut ein Radar");
        if ( num == EVENT_OBJECT_BENERGY   )  strcpy(text, "Baut eine Batteriefabrik");
        if ( num == EVENT_OBJECT_BLABO     )  strcpy(text, "Baut ein automatisches Labor");
        if ( num == EVENT_OBJECT_BNUCLEAR  )  strcpy(text, "Baut eine Brennstoffzellenfabrik");
        if ( num == EVENT_OBJECT_BPARA     )  strcpy(text, "Baut einen Blitzableiter");
        if ( num == EVENT_OBJECT_BINFO     )  strcpy(text, "Baut einen Infoserver");
        if ( num == EVENT_OBJECT_GFLAT     )  strcpy(text, "Zeigt ob der Boden eben ist");
        if ( num == EVENT_OBJECT_FCREATE   )  strcpy(text, "Setzt eine Fahne");
        if ( num == EVENT_OBJECT_FDELETE   )  strcpy(text, "Sammelt die Fahne ein");
        if ( num == EVENT_OBJECT_FCOLORb   )  strcpy(text, "\\Blaue Fahne");
        if ( num == EVENT_OBJECT_FCOLORr   )  strcpy(text, "\\Rote Fahne");
        if ( num == EVENT_OBJECT_FCOLORg   )  strcpy(text, "\\Gr¸ne Fahne");
        if ( num == EVENT_OBJECT_FCOLORy   )  strcpy(text, "\\Gelbe Fahne");
        if ( num == EVENT_OBJECT_FCOLORv   )  strcpy(text, "\\Violette Fahne");
        if ( num == EVENT_OBJECT_FACTORYfa )  strcpy(text, "Baut einen Jettransporter");
        if ( num == EVENT_OBJECT_FACTORYta )  strcpy(text, "Baut einen Kettentransporter");
        if ( num == EVENT_OBJECT_FACTORYwa )  strcpy(text, "Baut einen Radtransporter");
        if ( num == EVENT_OBJECT_FACTORYia )  strcpy(text, "Baut einen Krabbeltransporter");
        if ( num == EVENT_OBJECT_FACTORYfc )  strcpy(text, "Baut einen Jetshooter");
        if ( num == EVENT_OBJECT_FACTORYtc )  strcpy(text, "Baut einen Kettenshooter");
        if ( num == EVENT_OBJECT_FACTORYwc )  strcpy(text, "Baut einen Radshooter");
        if ( num == EVENT_OBJECT_FACTORYic )  strcpy(text, "Baut einen Krabbelshooter");
        if ( num == EVENT_OBJECT_FACTORYfi )  strcpy(text, "Baut einen Jetorgashooter");
        if ( num == EVENT_OBJECT_FACTORYti )  strcpy(text, "Baut einen Kettenorgashooter");
        if ( num == EVENT_OBJECT_FACTORYwi )  strcpy(text, "Baut einen Radorgashooter");
        if ( num == EVENT_OBJECT_FACTORYii )  strcpy(text, "Baut einen Krabbelorgashooter");
        if ( num == EVENT_OBJECT_FACTORYfs )  strcpy(text, "Baut einen Jetschn¸ffler");
        if ( num == EVENT_OBJECT_FACTORYts )  strcpy(text, "Baut einen Kettenschn¸ffler");
        if ( num == EVENT_OBJECT_FACTORYws )  strcpy(text, "Baut einen Radschn¸ffler");
        if ( num == EVENT_OBJECT_FACTORYis )  strcpy(text, "Baut einen Krabbelschn¸ffler");
        if ( num == EVENT_OBJECT_FACTORYrt )  strcpy(text, "Baut einen Stampfer");
        if ( num == EVENT_OBJECT_FACTORYrc )  strcpy(text, "Baut einen Phazershooter");
        if ( num == EVENT_OBJECT_FACTORYrr )  strcpy(text, "Baut einen Recycler");
        if ( num == EVENT_OBJECT_FACTORYrs )  strcpy(text, "Baut einen Schutzschild");
        if ( num == EVENT_OBJECT_FACTORYsa )  strcpy(text, "Baut einen Kettentaucher");
        if ( num == EVENT_OBJECT_RTANK     )  strcpy(text, "Forschungsprogramm Kettenantrieb");
        if ( num == EVENT_OBJECT_RFLY      )  strcpy(text, "Forschungsprogramm Jetantrieb");
        if ( num == EVENT_OBJECT_RTHUMP    )  strcpy(text, "Forschungsprogramm Stampfer");
        if ( num == EVENT_OBJECT_RCANON    )  strcpy(text, "Forschungsprogramm Shooterkanone");
        if ( num == EVENT_OBJECT_RTOWER    )  strcpy(text, "Forschungsprogramm Gesch¸tzturm");
        if ( num == EVENT_OBJECT_RPHAZER   )  strcpy(text, "Forschungsprogramm Phazerkanone");
        if ( num == EVENT_OBJECT_RSHIELD   )  strcpy(text, "Forschungsprogramm Schutzschild");
        if ( num == EVENT_OBJECT_RATOMIC   )  strcpy(text, "Forschungsprogramm Brennstoffzelle");
        if ( num == EVENT_OBJECT_RiPAW     )  strcpy(text, "Forschungsprogramm Krabbelantrieb");
        if ( num == EVENT_OBJECT_RiGUN     )  strcpy(text, "Forschungsprogramm Orgashooterkanone");
        if ( num == EVENT_OBJECT_RESET     )  strcpy(text, "Alles zur¸cksetzen");
        if ( num == EVENT_OBJECT_SEARCH    )  strcpy(text, "Schn¸ffeln (\\key action;)");
        if ( num == EVENT_OBJECT_TERRAFORM )  strcpy(text, "Stampfen (\\key action;)");
        if ( num == EVENT_OBJECT_FIRE      )  strcpy(text, "Feuer (\\key action;)");
        if ( num == EVENT_OBJECT_RECOVER   )  strcpy(text, "Recyceln (\\key action;)");
        if ( num == EVENT_OBJECT_BEGSHIELD )  strcpy(text, "Schutzschild ausfahren (\\key action;)");
        if ( num == EVENT_OBJECT_ENDSHIELD )  strcpy(text, "Schutzschild einholen (\\key action;)");
        if ( num == EVENT_OBJECT_DIMSHIELD )  strcpy(text, "Reichweite Schutzschild");
        if ( num == EVENT_OBJECT_PROGRUN   )  strcpy(text, "Gew‰hltes Programm ausf¸hren");
        if ( num == EVENT_OBJECT_PROGEDIT  )  strcpy(text, "Gew‰hltes Programm bearbeiten");
        if ( num == EVENT_OBJECT_INFOOK    )  strcpy(text, "\\SatCom in Standby");
        if ( num == EVENT_OBJECT_DELETE    )  strcpy(text, "Geb‰ude sprengen");
        if ( num == EVENT_OBJECT_GENERGY   )  strcpy(text, "Energievorrat");
        if ( num == EVENT_OBJECT_GSHIELD   )  strcpy(text, "Sch‰den");
        if ( num == EVENT_OBJECT_GRANGE    )  strcpy(text, "Triebwerktemperatur");
        if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Prozess im Gang ...");
        if ( num == EVENT_OBJECT_GRADAR    )  strcpy(text, "Anzahl erfasster Insekten");
        if ( num == EVENT_OBJECT_GINFO     )  strcpy(text, "Gesendete Informationen");
        if ( num == EVENT_OBJECT_COMPASS   )  strcpy(text, "Kompass");
//?     if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Minikarte");
        if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom Minikarte");
        if ( num == EVENT_OBJECT_CAMERA    )  strcpy(text, "Kamera (\\key camera;)");
        if ( num == EVENT_OBJECT_CAMERAleft)  strcpy(text, "Kamera links");
        if ( num == EVENT_OBJECT_CAMERAright) strcpy(text, "Kamera rechts");
        if ( num == EVENT_OBJECT_CAMERAnear)  strcpy(text, "Kamera n‰her");
        if ( num == EVENT_OBJECT_CAMERAaway)  strcpy(text, "Kamera weiter weg");
        if ( num == EVENT_OBJECT_HELP      )  strcpy(text, "Anweisungen ¸ber das ausgew‰hlte Objekt");
        if ( num == EVENT_OBJECT_SOLUCE    )  strcpy(text, "Zeigt die Lˆsung");
        if ( num == EVENT_OBJECT_SHORTCUT00)  strcpy(text, "Anzeige Roboter <-> Bauten");
        if ( num == EVENT_OBJECT_LIMIT     )  strcpy(text, "Zeigt die Reichweite");
        if ( num == EVENT_OBJECT_PEN0      )  strcpy(text, "\\Bleistift abheben");
        if ( num == EVENT_OBJECT_PEN1      )  strcpy(text, "\\Schwarzen Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN2      )  strcpy(text, "\\Gelben Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN3      )  strcpy(text, "\\Orangefarbenen Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN4      )  strcpy(text, "\\Roten Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN5      )  strcpy(text, "\\Violetten Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN6      )  strcpy(text, "\\Blauen Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN7      )  strcpy(text, "\\Gr¸nen Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_PEN8      )  strcpy(text, "\\Braunen Bleistift hinunterlassen");
        if ( num == EVENT_OBJECT_REC       )  strcpy(text, "\\Aufnahme starten");
        if ( num == EVENT_OBJECT_STOP      )  strcpy(text, "\\Aufnahme stoppen");
        if ( num == EVENT_DT_VISIT0        ||
             num == EVENT_DT_VISIT1        ||
             num == EVENT_DT_VISIT2        ||
             num == EVENT_DT_VISIT3        ||
             num == EVENT_DT_VISIT4        )  strcpy(text, "Zeigt den Ort");
        if ( num == EVENT_DT_END           )  strcpy(text, "Weitermachen");
        if ( num == EVENT_CMD              )  strcpy(text, "Befehleingabe");
        if ( num == EVENT_SPEED            )  strcpy(text, "Spielgeschwindigkeit");

        if ( num == EVENT_HYPER_PREV       )  strcpy(text, "Vorherg. Seite");
        if ( num == EVENT_HYPER_NEXT       )  strcpy(text, "N‰chste Seite");
        if ( num == EVENT_HYPER_HOME       )  strcpy(text, "Home");
        if ( num == EVENT_HYPER_COPY       )  strcpy(text, "Kopieren");
        if ( num == EVENT_HYPER_SIZE1      )  strcpy(text, "Grˆﬂe 1");
        if ( num == EVENT_HYPER_SIZE2      )  strcpy(text, "Grˆﬂe 2");
        if ( num == EVENT_HYPER_SIZE3      )  strcpy(text, "Grˆﬂe 3");
        if ( num == EVENT_HYPER_SIZE4      )  strcpy(text, "Grˆﬂe 4");
        if ( num == EVENT_HYPER_SIZE5      )  strcpy(text, "Grˆﬂe 5");
        if ( num == EVENT_SATCOM_HUSTON    )  strcpy(text, "Anweisungen von Houston");
#if _TEEN
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Wˆrterbuch Englisch-Deutsch");
#else
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Satellitenbericht");
#endif
        if ( num == EVENT_SATCOM_LOADING   )  strcpy(text, "Von Houston ¸bermittelte Programme");
        if ( num == EVENT_SATCOM_OBJECT    )  strcpy(text, "Liste der Objekte");
        if ( num == EVENT_SATCOM_PROG      )  strcpy(text, "Hilfe ¸ber Programmieren");
        if ( num == EVENT_SATCOM_SOLUCE    )  strcpy(text, "Lˆsung");

        if ( num == EVENT_STUDIO_OK        )  strcpy(text, "OK\\Programm kompilieren");
        if ( num == EVENT_STUDIO_CANCEL    )  strcpy(text, "Abbrechen\\Editor schlieﬂen");
        if ( num == EVENT_STUDIO_NEW       )  strcpy(text, "Neu");
        if ( num == EVENT_STUDIO_OPEN      )  strcpy(text, "÷ffnen (Ctrl+o)");
        if ( num == EVENT_STUDIO_SAVE      )  strcpy(text, "Speichern (Ctrl+s)");
        if ( num == EVENT_STUDIO_UNDO      )  strcpy(text, "Widerrufen (Ctrl+z)");
        if ( num == EVENT_STUDIO_CUT       )  strcpy(text, "Ausschneiden (Ctrl+x)");
        if ( num == EVENT_STUDIO_COPY      )  strcpy(text, "Kopieren (Ctrl+c)");
        if ( num == EVENT_STUDIO_PASTE     )  strcpy(text, "Einf¸gen (Ctrl+v)");
        if ( num == EVENT_STUDIO_SIZE      )  strcpy(text, "Zeichengrˆﬂe");
        if ( num == EVENT_STUDIO_TOOL      )  strcpy(text, "Anweisungen (\\key help;)");
        if ( num == EVENT_STUDIO_HELP      )  strcpy(text, "Hilfe ¸ber Programmieren (\\key prog;)");
        if ( num == EVENT_STUDIO_COMPILE   )  strcpy(text, "Kompilieren");
        if ( num == EVENT_STUDIO_RUN       )  strcpy(text, "Start/Stop");
        if ( num == EVENT_STUDIO_REALTIME  )  strcpy(text, "Pause/Weitermachen");
        if ( num == EVENT_STUDIO_STEP      )  strcpy(text, "Ein Schritt");
    }

    if ( type == RES_OBJECT )
    {
        if ( num == OBJECT_PORTICO      )  strcpy(text, "Tr‰ger");
        if ( num == OBJECT_BASE         )  strcpy(text, "Raumschiff");
        if ( num == OBJECT_DERRICK      )  strcpy(text, "Bohrturm");
        if ( num == OBJECT_FACTORY      )  strcpy(text, "Roboterfabrik");
        if ( num == OBJECT_REPAIR       )  strcpy(text, "Reparaturzentrum");
        if ( num == OBJECT_DESTROYER    )  strcpy(text, "Einstampfer");
        if ( num == OBJECT_STATION      )  strcpy(text, "Kraftwerk");
        if ( num == OBJECT_CONVERT      )  strcpy(text, "Konverter Erz-Titan");
        if ( num == OBJECT_TOWER        )  strcpy(text, "Gesch¸tzturm");
        if ( num == OBJECT_NEST         )  strcpy(text, "Orgastoffquelle");
        if ( num == OBJECT_RESEARCH     )  strcpy(text, "Forschungszentrum");
        if ( num == OBJECT_RADAR        )  strcpy(text, "Radar");
        if ( num == OBJECT_INFO         )  strcpy(text, "Infoserver");
#if _TEEN
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Auflˆser");
#else
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Batteriefabrik");
#endif
        if ( num == OBJECT_LABO         )  strcpy(text, "Automatisches Labor");
        if ( num == OBJECT_NUCLEAR      )  strcpy(text, "Brennstoffzellenfabrik");
        if ( num == OBJECT_PARA         )  strcpy(text, "Blitzableiter");
        if ( num == OBJECT_SAFE         )  strcpy(text, "Bunker");
        if ( num == OBJECT_HUSTON       )  strcpy(text, "Kontrollzentrum");
        if ( num == OBJECT_TARGET1      )  strcpy(text, "Zielscheibe");
        if ( num == OBJECT_TARGET2      )  strcpy(text, "Zielscheibe");
        if ( num == OBJECT_START        )  strcpy(text, "Startfl‰che");
        if ( num == OBJECT_END          )  strcpy(text, "Zielfl‰che");
        if ( num == OBJECT_STONE        )  strcpy(text, "Titanerz");
        if ( num == OBJECT_URANIUM      )  strcpy(text, "Platinerz");
        if ( num == OBJECT_BULLET       )  strcpy(text, "Orgastoff");
        if ( num == OBJECT_METAL        )  strcpy(text, "Titan");
        if ( num == OBJECT_POWER        )  strcpy(text, "Elektrolytische Batterie");
        if ( num == OBJECT_ATOMIC       )  strcpy(text, "Brennstoffzelle");
        if ( num == OBJECT_BBOX         )  strcpy(text, "Flugschreiber");
        if ( num == OBJECT_KEYa         )  strcpy(text, "Schl¸ssel A");
        if ( num == OBJECT_KEYb         )  strcpy(text, "Schl¸ssel B");
        if ( num == OBJECT_KEYc         )  strcpy(text, "Schl¸ssel C");
        if ( num == OBJECT_KEYd         )  strcpy(text, "Schl¸ssel D");
        if ( num == OBJECT_TNT          )  strcpy(text, "Sprengstoff");
        if ( num == OBJECT_BOMB         )  strcpy(text, "Landmine");
        if ( num == OBJECT_BAG          )  strcpy(text, "‹berlebenskit");
        if ( num == OBJECT_WAYPOINT     )  strcpy(text, "Checkpoint");
        if ( num == OBJECT_FLAGb        )  strcpy(text, "Blaue Fahne");
        if ( num == OBJECT_FLAGr        )  strcpy(text, "Rote Fahne");
        if ( num == OBJECT_FLAGg        )  strcpy(text, "Gr¸ne Fahne");
        if ( num == OBJECT_FLAGy        )  strcpy(text, "Gelbe Fahne");
        if ( num == OBJECT_FLAGv        )  strcpy(text, "Violette Fahne");
        if ( num == OBJECT_MARKPOWER    )  strcpy(text, "Markierung f¸r unterirdische Energiequelle");
        if ( num == OBJECT_MARKURANIUM  )  strcpy(text, "Markierung f¸r unterirdisches Platinvorkommen");
        if ( num == OBJECT_MARKKEYa     )  strcpy(text, "Markierung f¸r vergrabenen Schl¸ssel A");
        if ( num == OBJECT_MARKKEYb     )  strcpy(text, "Markierung f¸r vergrabenen Schl¸ssel B");
        if ( num == OBJECT_MARKKEYc     )  strcpy(text, "Markierung f¸r vergrabenen Schl¸ssel C");
        if ( num == OBJECT_MARKKEYd     )  strcpy(text, "Markierung f¸r vergrabenen Schl¸ssel D");
        if ( num == OBJECT_MARKSTONE    )  strcpy(text, "Markierung f¸r unterirdisches Titanvorkommen");
        if ( num == OBJECT_MOBILEft     )  strcpy(text, "‹bungsroboter");
        if ( num == OBJECT_MOBILEtt     )  strcpy(text, "‹bungsroboter");
        if ( num == OBJECT_MOBILEwt     )  strcpy(text, "‹bungsroboter");
        if ( num == OBJECT_MOBILEit     )  strcpy(text, "‹bungsroboter");
        if ( num == OBJECT_MOBILEfa     )  strcpy(text, "Transporter");
        if ( num == OBJECT_MOBILEta     )  strcpy(text, "Transporter");
        if ( num == OBJECT_MOBILEwa     )  strcpy(text, "Transporter");
        if ( num == OBJECT_MOBILEia     )  strcpy(text, "Transporter");
        if ( num == OBJECT_MOBILEfc     )  strcpy(text, "Shooter");
        if ( num == OBJECT_MOBILEtc     )  strcpy(text, "Shooter");
        if ( num == OBJECT_MOBILEwc     )  strcpy(text, "Shooter");
        if ( num == OBJECT_MOBILEic     )  strcpy(text, "Shooter");
        if ( num == OBJECT_MOBILEfi     )  strcpy(text, "OrgaShooter");
        if ( num == OBJECT_MOBILEti     )  strcpy(text, "OrgaShooter");
        if ( num == OBJECT_MOBILEwi     )  strcpy(text, "OrgaShooter");
        if ( num == OBJECT_MOBILEii     )  strcpy(text, "OrgaShooter");
        if ( num == OBJECT_MOBILEfs     )  strcpy(text, "Schn¸ffler");
        if ( num == OBJECT_MOBILEts     )  strcpy(text, "Schn¸ffler");
        if ( num == OBJECT_MOBILEws     )  strcpy(text, "Schn¸ffler");
        if ( num == OBJECT_MOBILEis     )  strcpy(text, "Schn¸ffler");
        if ( num == OBJECT_MOBILErt     )  strcpy(text, "Stampfer");
        if ( num == OBJECT_MOBILErc     )  strcpy(text, "Phazershooter");
        if ( num == OBJECT_MOBILErr     )  strcpy(text, "Recycler");
        if ( num == OBJECT_MOBILErs     )  strcpy(text, "Schutzschild");
        if ( num == OBJECT_MOBILEsa     )  strcpy(text, "Kettentaucher");
        if ( num == OBJECT_MOBILEtg     )  strcpy(text, "Mobile Zielscheibe");
        if ( num == OBJECT_MOBILEdr     )  strcpy(text, "Zeichner");
        if ( num == OBJECT_HUMAN        )  strcpy(text, g_gamerName);
        if ( num == OBJECT_TECH         )  strcpy(text, "Techniker");
        if ( num == OBJECT_TOTO         )  strcpy(text, "Robby");
        if ( num == OBJECT_MOTHER       )  strcpy(text, "Insektenkˆnigin");
        if ( num == OBJECT_ANT          )  strcpy(text, "Ameise");
        if ( num == OBJECT_SPIDER       )  strcpy(text, "Spinne");
        if ( num == OBJECT_BEE          )  strcpy(text, "Wespe");
        if ( num == OBJECT_WORM         )  strcpy(text, "Wurm");
        if ( num == OBJECT_EGG          )  strcpy(text, "Ei");
        if ( num == OBJECT_RUINmobilew1 )  strcpy(text, "Roboterwrack");
        if ( num == OBJECT_RUINmobilew2 )  strcpy(text, "Roboterwrack");
        if ( num == OBJECT_RUINmobilet1 )  strcpy(text, "Roboterwrack");
        if ( num == OBJECT_RUINmobilet2 )  strcpy(text, "Roboterwrack");
        if ( num == OBJECT_RUINmobiler1 )  strcpy(text, "Roboterwrack");
        if ( num == OBJECT_RUINmobiler2 )  strcpy(text, "Roboterwrack");
        if ( num == OBJECT_RUINfactory  )  strcpy(text, "Geb‰uderuine");
        if ( num == OBJECT_RUINdoor     )  strcpy(text, "Geb‰uderuine");
        if ( num == OBJECT_RUINsupport  )  strcpy(text, "Abfall");
        if ( num == OBJECT_RUINradar    )  strcpy(text, "Geb‰uderuine");
        if ( num == OBJECT_RUINconvert  )  strcpy(text, "Geb‰uderuine");
        if ( num == OBJECT_RUINbase     )  strcpy(text, "Raumschiffruine");
        if ( num == OBJECT_RUINhead     )  strcpy(text, "Raumschiffruine");
        if ( num == OBJECT_APOLLO1      ||
             num == OBJECT_APOLLO3      ||
             num == OBJECT_APOLLO4      ||
             num == OBJECT_APOLLO5      )  strcpy(text, "‹berreste einer Apollo-Mission");
        if ( num == OBJECT_APOLLO2      )  strcpy(text, "Lunar Roving Vehicle");
    }

    if ( type == RES_ERR )
    {
        strcpy(text, "Fehler");
        if ( num == ERR_CMD             )  strcpy(text, "Befehl unbekannt");
#if _NEWLOOK
        if ( num == ERR_INSTALL         )  strcpy(text, "CeeBot wurde nicht installiert.");
        if ( num == ERR_NOCD            )  strcpy(text, "Legen Sie die CeeBot-CD ein\nund starten Sie das Spiel neu.");
#else
        if ( num == ERR_INSTALL         )  strcpy(text, "COLOBOT wurde nicht installiert.");
        if ( num == ERR_NOCD            )  strcpy(text, "Legen Sie die COLOBOT-CD ein\nund starten Sie das Spiel neu.");
#endif
        if ( num == ERR_MANIP_VEH       )  strcpy(text, "Roboter ungeeignet");
        if ( num == ERR_MANIP_FLY       )  strcpy(text, "Im Flug unmˆglich");
        if ( num == ERR_MANIP_BUSY      )  strcpy(text, "Tr‰gt schon etwas");
        if ( num == ERR_MANIP_NIL       )  strcpy(text, "Nichts zu ergreifen");
        if ( num == ERR_MANIP_MOTOR     )  strcpy(text, "In Fahrt unmˆglich");
        if ( num == ERR_MANIP_OCC       )  strcpy(text, "Stelle schon besetzt");
        if ( num == ERR_MANIP_FRIEND    )  strcpy(text, "Kein anderer Roboter");
        if ( num == ERR_MANIP_RADIO     )  strcpy(text, "Sie kˆnnen keinen radioaktiven Gegenstand tragen");
        if ( num == ERR_MANIP_WATER     )  strcpy(text, "Sie kˆnnen unter Wasser nichts tragen");
        if ( num == ERR_MANIP_EMPTY     )  strcpy(text, "Nichts abzulegen");
        if ( num == ERR_BUILD_FLY       )  strcpy(text, "Im Flug unmˆglich");
        if ( num == ERR_BUILD_WATER     )  strcpy(text, "Unter Wasser unmˆglich");
        if ( num == ERR_BUILD_ENERGY    )  strcpy(text, "Nicht genug Energie");
        if ( num == ERR_BUILD_METALAWAY )  strcpy(text, "Titan zu weit weg");
        if ( num == ERR_BUILD_METALNEAR )  strcpy(text, "Titan zu nahe");
        if ( num == ERR_BUILD_METALINEX )  strcpy(text, "Kein Titan vorhanden");
        if ( num == ERR_BUILD_FLAT      )  strcpy(text, "Boden nicht eben genug");
        if ( num == ERR_BUILD_FLATLIT   )  strcpy(text, "Ebener Boden nicht groﬂ genug");
        if ( num == ERR_BUILD_BUSY      )  strcpy(text, "Stelle schon besetzt");
        if ( num == ERR_BUILD_BASE      )  strcpy(text, "Zu nahe am Raumschiff");
        if ( num == ERR_BUILD_NARROW    )  strcpy(text, "Zu nahe an einem Geb‰ude");
        if ( num == ERR_BUILD_MOTOR     )  strcpy(text, "In Fahrt unmˆglich");
        if ( num == ERR_SEARCH_FLY      )  strcpy(text, "Im Flug unmˆglich");
        if ( num == ERR_SEARCH_VEH      )  strcpy(text, "Roboter ungeeignet");
        if ( num == ERR_SEARCH_MOTOR    )  strcpy(text, "In Fahrt unmˆglich");
        if ( num == ERR_TERRA_VEH       )  strcpy(text, "Roboter ungeeignet");
        if ( num == ERR_TERRA_ENERGY    )  strcpy(text, "Nicht genug Energie");
        if ( num == ERR_TERRA_FLOOR     )  strcpy(text, "Boden ungeeignet");
        if ( num == ERR_TERRA_BUILDING  )  strcpy(text, "Geb‰ude zu nahe");
        if ( num == ERR_TERRA_OBJECT    )  strcpy(text, "Gegenstand zu nahe");
        if ( num == ERR_RECOVER_VEH     )  strcpy(text, "Roboter ungeeignet");
        if ( num == ERR_RECOVER_ENERGY  )  strcpy(text, "Nicht genug Energie");
        if ( num == ERR_RECOVER_NULL    )  strcpy(text, "Nichts zu recyceln");
        if ( num == ERR_SHIELD_VEH      )  strcpy(text, "Roboter ungeeignet");
        if ( num == ERR_SHIELD_ENERGY   )  strcpy(text, "Keine Energie mehr");
//?     if ( num == ERR_COM             )  strcpy(text, "Kommunikationsproblem mit dem Roboter");
        if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Ziel kann nicht erreicht werden");
        if ( num == ERR_FIND_IMPOSSIBLE )  strcpy(text, "Das Objekt existiert nicht");
        if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Ziel kann nicht erreicht werden");
        if ( num == ERR_GOTO_ITER       )  strcpy(text, "Ziel kann nicht erreicht werden");
        if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Ziel ist schon besetzt");
        if ( num == ERR_FIRE_VEH        )  strcpy(text, "Roboter ungeeignet");
        if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Nicht genug Energie");
        if ( num == ERR_FIRE_FLY        )  strcpy(text, "Im Flug unmˆglich");
        if ( num == ERR_CONVERT_EMPTY   )  strcpy(text, "Kein konvertierbares Titanerz vorhanden");
        if ( num == ERR_DERRICK_NULL    )  strcpy(text, "Keine unterirdische Erzlagerst‰tte");
        if ( num == ERR_STATION_NULL    )  strcpy(text, "Kein unterirdisches Energievorkommen");
        if ( num == ERR_TOWER_POWER     )  strcpy(text, "Keine Batterie");
        if ( num == ERR_TOWER_ENERGY    )  strcpy(text, "Keine Energie mehr");
        if ( num == ERR_RESEARCH_POWER  )  strcpy(text, "Keine Batterie");
        if ( num == ERR_RESEARCH_ENERGY )  strcpy(text, "Nicht mehr genug Energie");
        if ( num == ERR_RESEARCH_TYPE   )  strcpy(text, "Falscher Batterietyp");
        if ( num == ERR_RESEARCH_ALREADY)  strcpy(text, "Forschungsprogramm schon ausgef¸hrt");
        if ( num == ERR_ENERGY_NULL     )  strcpy(text, "Kein unterirdisches Energievorkommen");
        if ( num == ERR_ENERGY_LOW      )  strcpy(text, "Noch nicht genug Energie");
        if ( num == ERR_ENERGY_EMPTY    )  strcpy(text, "Kein konvertierbares Titanerz vorhanden");
        if ( num == ERR_ENERGY_BAD      )  strcpy(text, "Wandelt nur Titanerz um");
        if ( num == ERR_BASE_DLOCK      )  strcpy(text, "Die T¸ren werden von einem Gegenstand blockiert");
        if ( num == ERR_BASE_DHUMAN     )  strcpy(text, "Gehen Sie an Bord, bevor Sie abheben");
        if ( num == ERR_LABO_NULL       )  strcpy(text, "Nichts zu analysieren");
        if ( num == ERR_LABO_BAD        )  strcpy(text, "Analysiert nur Orgastoff");
        if ( num == ERR_LABO_ALREADY    )  strcpy(text, "Analyse schon durchgef¸hrt");
        if ( num == ERR_NUCLEAR_NULL    )  strcpy(text, "Kein unterirdisches Energievorkommen");
        if ( num == ERR_NUCLEAR_LOW     )  strcpy(text, "Noch nicht genug Energie");
        if ( num == ERR_NUCLEAR_EMPTY   )  strcpy(text, "Kein konvertierbares Platin");
        if ( num == ERR_NUCLEAR_BAD     )  strcpy(text, "Wandelt nur Platin um");
        if ( num == ERR_FACTORY_NULL    )  strcpy(text, "Kein Titan vorhanden");
        if ( num == ERR_FACTORY_NEAR    )  strcpy(text, "Ein Gegenstand ist zu nahe");
        if ( num == ERR_RESET_NEAR      )  strcpy(text, "Stelle schon besetzt");
        if ( num == ERR_INFO_NULL       )  strcpy(text, "Kein Infoserver in Reichweite");
        if ( num == ERR_VEH_VIRUS       )  strcpy(text, "Ein Programm wurde von einem Virus infiziert");
        if ( num == ERR_BAT_VIRUS       )  strcpy(text, "Von Virus infiziert, zeitweise auﬂer Betrieb");
        if ( num == ERR_VEH_POWER       )  strcpy(text, "Keine Batterie");
        if ( num == ERR_VEH_ENERGY      )  strcpy(text, "Keine Energie mehr");
        if ( num == ERR_FLAG_FLY        )  strcpy(text, "Im Flug unmˆglich");
        if ( num == ERR_FLAG_WATER      )  strcpy(text, "Im Wasser unmˆglich");
        if ( num == ERR_FLAG_MOTOR      )  strcpy(text, "Beim Gehen unmˆglich");
        if ( num == ERR_FLAG_BUSY       )  strcpy(text, "Unmˆglich wenn Sie etwas tragen");
        if ( num == ERR_FLAG_CREATE     )  strcpy(text, "Zu viele Fahnen dieser Farbe (Maximum 5)");
        if ( num == ERR_FLAG_PROXY      )  strcpy(text, "Zu nahe an einer anderen Fahne");
        if ( num == ERR_FLAG_DELETE     )  strcpy(text, "Keine Fahne in Reichweite");
        if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "Mission noch nicht beendet (Dr¸cken Sie auf \\key help; f¸r weitere Informationen)");
        if ( num == ERR_DELETEMOBILE    )  strcpy(text, "Roboter zerstˆrt");
        if ( num == ERR_DELETEBUILDING  )  strcpy(text, "Geb‰ude zerstˆrt");
        if ( num == ERR_TOOMANY         )  strcpy(text, "Kein neues Objekt kann erstellt werden (zu viele vorhanden)");
        if ( num == ERR_OBLIGATORYTOKEN )  strcpy(text, "Es fehlt \"%s\" in Ihrem Programm");
        if ( num == ERR_PROHIBITEDTOKEN )  strcpy(text, "In dieser ‹bung verboten");

        if ( num == INFO_BUILD          )  strcpy(text, "Geb‰ude fertiggestellt");
        if ( num == INFO_CONVERT        )  strcpy(text, "Titan verf¸gbar");
        if ( num == INFO_RESEARCH       )  strcpy(text, "Forschungsprogramm abgeschlossen");
        if ( num == INFO_RESEARCHTANK   )  strcpy(text, "Herstellung eines Roboters mit Kettenantrieb mˆglich");
        if ( num == INFO_RESEARCHFLY    )  strcpy(text, "Sie kˆnnen jetzt mit den Tasten \\key gup; und \\key gdown; fliegen");
        if ( num == INFO_RESEARCHTHUMP  )  strcpy(text, "Herstellung eines Stampfers mˆglich");
        if ( num == INFO_RESEARCHCANON  )  strcpy(text, "Herstellung eines Shooters mˆglich");
        if ( num == INFO_RESEARCHTOWER  )  strcpy(text, "Errichtung eines Gesch¸tzturms mˆglich");
        if ( num == INFO_RESEARCHPHAZER )  strcpy(text, "Herstellung eines Phazershooters mˆglich");
        if ( num == INFO_RESEARCHSHIELD )  strcpy(text, "Herstellung eines Schutzschildes mˆglich");
        if ( num == INFO_RESEARCHATOMIC )  strcpy(text, "Errichtung einer Brennstoffzellenfabrik mˆglich");
        if ( num == INFO_FACTORY        )  strcpy(text, "Neuer Roboter verf¸gbar");
        if ( num == INFO_LABO           )  strcpy(text, "Analyse vollendet");
        if ( num == INFO_ENERGY         )  strcpy(text, "Batterie verf¸gbar");
        if ( num == INFO_NUCLEAR        )  strcpy(text, "Brennstoffzelle verf¸gbar");
        if ( num == INFO_FINDING        )  strcpy(text, "Sie haben ein brauchbares Objekt gefunden");
        if ( num == INFO_MARKPOWER      )  strcpy(text, "Geeignete Stelle f¸r Kraftwerk gefunden");
        if ( num == INFO_MARKURANIUM    )  strcpy(text, "Geeignete Stelle f¸r Bohrturm gefunden");
        if ( num == INFO_MARKSTONE      )  strcpy(text, "Geeignete Stelle f¸r Bohrturm gefunden");
        if ( num == INFO_MARKKEYa       )  strcpy(text, "Geeignete Stelle f¸r Bohrturm gefunden");
        if ( num == INFO_MARKKEYb       )  strcpy(text, "Geeignete Stelle f¸r Bohrturm gefunden");
        if ( num == INFO_MARKKEYc       )  strcpy(text, "Geeignete Stelle f¸r Bohrturm gefunden");
        if ( num == INFO_MARKKEYd       )  strcpy(text, "Geeignete Stelle f¸r Bohrturm gefunden");
        if ( num == INFO_WIN            )  strcpy(text, "<<< Bravo, Mission vollendet >>>");
        if ( num == INFO_LOST           )  strcpy(text, "<<< Mission gescheitert >>>");
        if ( num == INFO_LOSTq          )  strcpy(text, "<<< Mission gescheitert >>>");
        if ( num == INFO_WRITEOK        )  strcpy(text, "Mission gespeichert");
        if ( num == INFO_DELETEPATH     )  strcpy(text, "Checkpoint erreicht");
        if ( num == INFO_DELETEMOTHER   )  strcpy(text, "Insektenkˆnigin tˆdlich verwundet");
        if ( num == INFO_DELETEANT      )  strcpy(text, "Ameise tˆdlich verwundet");
        if ( num == INFO_DELETEBEE      )  strcpy(text, "Wespe tˆdlich verwundet");
        if ( num == INFO_DELETEWORM     )  strcpy(text, "Wurm tˆdlich verwundet");
        if ( num == INFO_DELETESPIDER   )  strcpy(text, "Spinne tˆdlich verwundet");
        if ( num == INFO_BEGINSATCOM    )  strcpy(text, "Beziehen Sie sich auf Ihren SatCom, indem Sie auf \\key help; dr¸cken");
    }

    if ( type == RES_CBOT )
    {
        strcpy(text, "Fehler");
        if ( num == TX_OPENPAR       ) strcpy(text, "Es fehlt eine offene Klammer ""(""");
        if ( num == TX_CLOSEPAR      ) strcpy(text, "Es fehlt eine geschlossene Klammer "")""");
        if ( num == TX_NOTBOOL       ) strcpy(text, "Der Ausdruck muss einen boolschen Wert ergeben");
        if ( num == TX_UNDEFVAR      ) strcpy(text, "Variable nicht deklariert");
        if ( num == TX_BADLEFT       ) strcpy(text, "Zuweisung unmˆglich");
        if ( num == TX_ENDOF         ) strcpy(text, "Es fehlt ein Strichpunkt "";"" am Ende der Anweisung");
        if ( num == TX_OUTCASE       ) strcpy(text, "Anweisung ""case"" ohne vorhergehende Anweisung ""switch""");
        if ( num == TX_NOTERM        ) strcpy(text, "Hier ist eine Anweisung nach dem Ende des Programms");
        if ( num == TX_CLOSEBLK      ) strcpy(text, "Es fehlt eine geschlossene geschweifte Klammer ""}"" (Ende des Blocks)");
        if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Anweisung ""else"" ohne vorhergehende Anweisung ""if""");
        if ( num == TX_OPENBLK       ) strcpy(text, "Es fehlt eine offene geschweifte Klammer""{""");
        if ( num == TX_BADTYPE       ) strcpy(text, "Der Ausdruck ergibt einen falschen Typ f¸r die Zuweisung");
        if ( num == TX_REDEFVAR      ) strcpy(text, "Eine Variable wird zum zweiten Mal deklariert");
        if ( num == TX_BAD2TYPE      ) strcpy(text, "Die zwei Operanden sind nicht kompatibel");
        if ( num == TX_UNDEFCALL     ) strcpy(text, "Unbekannte Funktion");
        if ( num == TX_MISDOTS       ) strcpy(text, "Es fehlt ein Doppelpunkt "" : """);
        if ( num == TX_WHILE         ) strcpy(text, "Es fehlt das Wort ""while""");
        if ( num == TX_BREAK         ) strcpy(text, "Anweisung ""break"" auﬂerhalb einer Schleife");
        if ( num == TX_LABEL         ) strcpy(text, "Ein Label kann nur vor den Anweisungen ""for"", ""while"", ""do"" oder ""switch"" vorkommen");
        if ( num == TX_NOLABEL       ) strcpy(text, "Dieses Label existiert nicht");
        if ( num == TX_NOCASE        ) strcpy(text, "Es fehlt eine Anweisung ""case""");
        if ( num == TX_BADNUM        ) strcpy(text, "Es fehlt eine Zahl");
        if ( num == TX_VOID          ) strcpy(text, "Parameter void");
        if ( num == TX_NOTYP         ) strcpy(text, "Hier muss ein Variablentyp stehen");
        if ( num == TX_NOVAR         ) strcpy(text, "Es fehlt der Name einer Variable");
        if ( num == TX_NOFONC        ) strcpy(text, "Hier muss der Name der Funktion stehen");
        if ( num == TX_OVERPARAM     ) strcpy(text, "Zu viele Parameter");
        if ( num == TX_REDEF         ) strcpy(text, "Diese Funktion gibt es schon");
        if ( num == TX_LOWPARAM      ) strcpy(text, "Nicht genug Parameter");
        if ( num == TX_BADPARAM      ) strcpy(text, "Keine Funktion mit diesem Namen vertr‰gt Parameter diesen Typs");
        if ( num == TX_NUMPARAM      ) strcpy(text, "Keine Funktion mit diesem Namen vertr‰gt diese Anzahl Parameter");
        if ( num == TX_NOITEM        ) strcpy(text, "Dieses Element gibt es nicht in dieser Klasse");
        if ( num == TX_DOT           ) strcpy(text, "Das Objekt ist nicht eine Instanz einer Klasse");
        if ( num == TX_NOCONST       ) strcpy(text, "Es gibt keinen geeigneten Konstruktor");
        if ( num == TX_REDEFCLASS    ) strcpy(text, "Diese Klasse gibt es schon");
        if ( num == TX_CLBRK         ) strcpy(text, "Es fehlt eine geschlossene eckige Klammer "" ] """);
        if ( num == TX_RESERVED      ) strcpy(text, "Dieses Wort ist reserviert");
        if ( num == TX_BADNEW        ) strcpy(text, "Falsche Argumente f¸r ""new""");
        if ( num == TX_OPBRK         ) strcpy(text, "Es fehlt eine offene eckige Klammer "" [ """);
        if ( num == TX_BADSTRING     ) strcpy(text, "Hier wird eine Zeichenkette erwartet");
        if ( num == TX_BADINDEX      ) strcpy(text, "Falscher Typ f¸r einen Index");
        if ( num == TX_PRIVATE       ) strcpy(text, "Gesch¸tztes Element (private)");
        if ( num == TX_NOPUBLIC      ) strcpy(text, "Hier muss das Wort ""public"" stehen");
        if ( num == TX_DIVZERO       ) strcpy(text, "Teilung durch Null");
        if ( num == TX_NOTINIT       ) strcpy(text, "Der Wert dieser Variable wurde nicht definiert");
        if ( num == TX_BADTHROW      ) strcpy(text, "Negativer Wert ungeeignet f¸r Anweisung ""throw""");
        if ( num == TX_NORETVAL      ) strcpy(text, "Die Funktion hat kein Ergebnis zur¸ckgegeben");
        if ( num == TX_NORUN         ) strcpy(text, "Keine Funktion wird ausgef¸hrt");
        if ( num == TX_NOCALL        ) strcpy(text, "Die aufgerufene Funktion existiert nicht");
        if ( num == TX_NOCLASS       ) strcpy(text, "Diese Klasse existiert nicht");
        if ( num == TX_NULLPT        ) strcpy(text, "Das Objekt existiert nicht");
        if ( num == TX_OPNAN         ) strcpy(text, "Operation mit dem Wert ""nan""");
        if ( num == TX_OUTARRAY      ) strcpy(text, "Zugriff im Array auﬂerhalb der Grenzen");
        if ( num == TX_STACKOVER     ) strcpy(text, "Stack overflow");
        if ( num == TX_DELETEDPT     ) strcpy(text, "Objekt nicht verf¸gbar");
        if ( num == TX_FILEOPEN      ) strcpy(text, "Die Datei kann nicht geˆffnet werden");
        if ( num == TX_NOTOPEN       ) strcpy(text, "Die Datei wurde nicht geˆffnet");
        if ( num == TX_ERRREAD       ) strcpy(text, "Fehler beim Lesezugriff");
        if ( num == TX_ERRWRITE      ) strcpy(text, "Fehler beim Schreibzugriff");
    }

    if ( type == RES_KEY )
    {
        if ( num == 0                   )  strcpy(text, "< keine >");
        if ( num == VK_LEFT             )  strcpy(text, "Pfeiltaste links");
        if ( num == VK_RIGHT            )  strcpy(text, "Pfeiltaste rechts");
        if ( num == VK_UP               )  strcpy(text, "Pfeil nach oben");
        if ( num == VK_DOWN             )  strcpy(text, "Pfeil nach unten");
        if ( num == VK_CANCEL           )  strcpy(text, "Ctrl-Break");
        if ( num == VK_BACK             )  strcpy(text, "<--");
        if ( num == VK_TAB              )  strcpy(text, "Tab");
        if ( num == VK_CLEAR            )  strcpy(text, "Clear");
        if ( num == VK_RETURN           )  strcpy(text, "Eingabe");
        if ( num == VK_SHIFT            )  strcpy(text, "Shift");
        if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
        if ( num == VK_MENU             )  strcpy(text, "Alt");
        if ( num == VK_PAUSE            )  strcpy(text, "Pause");
        if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
        if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
        if ( num == VK_SPACE            )  strcpy(text, "Leertaste");
        if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
        if ( num == VK_NEXT             )  strcpy(text, "Page Down");
        if ( num == VK_END              )  strcpy(text, "End");
        if ( num == VK_HOME             )  strcpy(text, "Home");
        if ( num == VK_SELECT           )  strcpy(text, "Select");
        if ( num == VK_EXECUTE          )  strcpy(text, "Execute");
        if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
        if ( num == VK_INSERT           )  strcpy(text, "Insert");
        if ( num == VK_DELETE           )  strcpy(text, "Delete");
        if ( num == VK_HELP             )  strcpy(text, "Help");
        if ( num == VK_LWIN             )  strcpy(text, "Left Windows");
        if ( num == VK_RWIN             )  strcpy(text, "Right Windows");
        if ( num == VK_APPS             )  strcpy(text, "Application key");
        if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
        if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
        if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
        if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
        if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
        if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
        if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
        if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
        if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
        if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
        if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
        if ( num == VK_ADD              )  strcpy(text, "NumPad +");
        if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
        if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
        if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
        if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
        if ( num == VK_F1               )  strcpy(text, "F1");
        if ( num == VK_F2               )  strcpy(text, "F2");
        if ( num == VK_F3               )  strcpy(text, "F3");
        if ( num == VK_F4               )  strcpy(text, "F4");
        if ( num == VK_F5               )  strcpy(text, "F5");
        if ( num == VK_F6               )  strcpy(text, "F6");
        if ( num == VK_F7               )  strcpy(text, "F7");
        if ( num == VK_F8               )  strcpy(text, "F8");
        if ( num == VK_F9               )  strcpy(text, "F9");
        if ( num == VK_F10              )  strcpy(text, "F10");
        if ( num == VK_F11              )  strcpy(text, "F11");
        if ( num == VK_F12              )  strcpy(text, "F12");
        if ( num == VK_F13              )  strcpy(text, "F13");
        if ( num == VK_F14              )  strcpy(text, "F14");
        if ( num == VK_F15              )  strcpy(text, "F15");
        if ( num == VK_F16              )  strcpy(text, "F16");
        if ( num == VK_F17              )  strcpy(text, "F17");
        if ( num == VK_F18              )  strcpy(text, "F18");
        if ( num == VK_F19              )  strcpy(text, "F19");
        if ( num == VK_F20              )  strcpy(text, "F20");
        if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
        if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
        if ( num == VK_ATTN             )  strcpy(text, "Attn");
        if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
        if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
        if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
        if ( num == VK_PLAY             )  strcpy(text, "Play");
        if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
        if ( num == VK_PA1              )  strcpy(text, "PA1");
        if ( num == VK_OEM_CLEAR        )  strcpy(text, "Clear");
        if ( num == VK_BUTTON1          )  strcpy(text, "Knopf 1");
        if ( num == VK_BUTTON2          )  strcpy(text, "Knopf 2");
        if ( num == VK_BUTTON3          )  strcpy(text, "Knopf 3");
        if ( num == VK_BUTTON4          )  strcpy(text, "Knopf 4");
        if ( num == VK_BUTTON5          )  strcpy(text, "Knopf 5");
        if ( num == VK_BUTTON6          )  strcpy(text, "Knopf 6");
        if ( num == VK_BUTTON7          )  strcpy(text, "Knopf 7");
        if ( num == VK_BUTTON8          )  strcpy(text, "Knopf 8");
        if ( num == VK_BUTTON9          )  strcpy(text, "Knopf 9");
        if ( num == VK_BUTTON10         )  strcpy(text, "Knopf 10");
        if ( num == VK_BUTTON11         )  strcpy(text, "Knopf 11");
        if ( num == VK_BUTTON12         )  strcpy(text, "Knopf 12");
        if ( num == VK_BUTTON13         )  strcpy(text, "Knopf 13");
        if ( num == VK_BUTTON14         )  strcpy(text, "Knopf 14");
        if ( num == VK_BUTTON15         )  strcpy(text, "Knopf 15");
        if ( num == VK_BUTTON16         )  strcpy(text, "Knopf 16");
        if ( num == VK_BUTTON17         )  strcpy(text, "Knopf 17");
        if ( num == VK_BUTTON18         )  strcpy(text, "Knopf 18");
        if ( num == VK_BUTTON19         )  strcpy(text, "Knopf 19");
        if ( num == VK_BUTTON20         )  strcpy(text, "Knopf 20");
        if ( num == VK_BUTTON21         )  strcpy(text, "Knopf 21");
        if ( num == VK_BUTTON22         )  strcpy(text, "Knopf 22");
        if ( num == VK_BUTTON23         )  strcpy(text, "Knopf 23");
        if ( num == VK_BUTTON24         )  strcpy(text, "Knopf 24");
        if ( num == VK_BUTTON25         )  strcpy(text, "Knopf 25");
        if ( num == VK_BUTTON26         )  strcpy(text, "Knopf 26");
        if ( num == VK_BUTTON27         )  strcpy(text, "Knopf 27");
        if ( num == VK_BUTTON28         )  strcpy(text, "Knopf 28");
        if ( num == VK_BUTTON29         )  strcpy(text, "Knopf 29");
        if ( num == VK_BUTTON30         )  strcpy(text, "Knopf 30");
        if ( num == VK_BUTTON31         )  strcpy(text, "Knopf 31");
        if ( num == VK_BUTTON32         )  strcpy(text, "Knopf 32");
        if ( num == VK_WHEELUP          )  strcpy(text, "Mausrad nach vorne");
        if ( num == VK_WHEELDOWN        )  strcpy(text, "Mausrad zur¸ck");
    }
#endif

#if _POLISH
    if ( type == RES_TEXT )
    {
        #if _FULL
        if ( num == RT_VERSION_ID          )  strcpy(text, "Wersja 1.18 /pl");
        #endif
        #if _NET
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A 1.18");
        #endif
        #if _SCHOOL & _EDU
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen EDU 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A EDU 1.18");
        #endif
        #endif
        #if _SCHOOL & _PERSO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen PERSO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A PERSO 1.18");
        #endif
        #endif
        #if _SCHOOL & _CEEBOTDEMO
        #if _TEEN
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-Teen DEMO 1.18");
        #else
        if ( num == RT_VERSION_ID          )  strcpy(text, "CeeBot-A DEMO 1.18");
        #endif
        #endif
        #if _DEMO
        if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.18 /pl");
        #endif
        if ( num == RT_DISINFO_TITLE       )  strcpy(text, "SatCom");
        if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "PowiÍksz");
        if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Pomniejsz");
        if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normalna wielkoúÊ");
        if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Zamknij");

        if ( num == RT_STUDIO_TITLE        )  strcpy(text, "Edytor programu");
        if ( num == RT_SCRIPT_NEW          )  strcpy(text, "Nowy");
        if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Gracz");
        if ( num == RT_IO_NEW              )  strcpy(text, "Nowy ...");
        if ( num == RT_KEY_OR              )  strcpy(text, " lub ");

#if _NEWLOOK
        if ( num == RT_TITLE_BASE          )  strcpy(text, "CeeBot");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "CeeBot");
#else
        if ( num == RT_TITLE_BASE          )  strcpy(text, "COLOBOT");
        if ( num == RT_TITLE_INIT          )  strcpy(text, "COLOBOT");
#endif
        if ( num == RT_TITLE_TRAINER       )  strcpy(text, "∆wiczenia programistyczne");
        if ( num == RT_TITLE_DEFI          )  strcpy(text, "Wyzwania");
        if ( num == RT_TITLE_MISSION       )  strcpy(text, "Misje");
        if ( num == RT_TITLE_FREE          )  strcpy(text, "Swobodna gra");
        if ( num == RT_TITLE_TEEN          )  strcpy(text, "Swobodna gra");
        if ( num == RT_TITLE_USER          )  strcpy(text, "Poziomy uøytkownika");
        if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypy");
        if ( num == RT_TITLE_SETUP         )  strcpy(text, "Opcje");
        if ( num == RT_TITLE_NAME          )  strcpy(text, "ImiÍ gracza");
        if ( num == RT_TITLE_PERSO         )  strcpy(text, "Dostosuj wyglπd");
        if ( num == RT_TITLE_WRITE         )  strcpy(text, "Zapisz bieøπcπ misjÍ");
        if ( num == RT_TITLE_READ          )  strcpy(text, "Wczytaj zapisanπ misjÍ");

        if ( num == RT_PLAY_CHAPt          )  strcpy(text, " Rozdzia≥y:");
        if ( num == RT_PLAY_CHAPd          )  strcpy(text, " Rozdzia≥y:");
        if ( num == RT_PLAY_CHAPm          )  strcpy(text, " Planety:");
        if ( num == RT_PLAY_CHAPf          )  strcpy(text, " Planety:");
        if ( num == RT_PLAY_CHAPu          )  strcpy(text, " Poziomy uøytkownika:");
        if ( num == RT_PLAY_CHAPp          )  strcpy(text, " Planety:");
        if ( num == RT_PLAY_CHAPte         )  strcpy(text, " Planety:");
        if ( num == RT_PLAY_LISTt          )  strcpy(text, " ∆wiczenia w tym rozdziale:");
        if ( num == RT_PLAY_LISTd          )  strcpy(text, " Wyzwania w tym rozdziale:");
        if ( num == RT_PLAY_LISTm          )  strcpy(text, " Misje na tej planecie:");
        if ( num == RT_PLAY_LISTf          )  strcpy(text, " Swobodna gra na tej planecie:");
        if ( num == RT_PLAY_LISTu          )  strcpy(text, " Misje na tym poziomie:");
        if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototypy na tej planecie:");
        if ( num == RT_PLAY_LISTk          )  strcpy(text, " Prototypy na tej planecie:");
        if ( num == RT_PLAY_RESUME         )  strcpy(text, " Streszczenie:");

        if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Sterowniki:");
        if ( num == RT_SETUP_MODE          )  strcpy(text, " RozdzielczoúÊ:");
        if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Najpierw kliknij klawisz, ktÛry chcesz przedefiniowaÊ.");
        if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) NastÍpnie naciúnij klawisz, ktÛrego chcesz uøywaÊ.");

        if ( num == RT_PERSO_FACE          )  strcpy(text, "Rodzaj twarzy:");
        if ( num == RT_PERSO_GLASSES       )  strcpy(text, "Okulary:");
        if ( num == RT_PERSO_HAIR          )  strcpy(text, "Kolor w≥osÛw:");
        if ( num == RT_PERSO_COMBI         )  strcpy(text, "Kolor skafandra:");
        if ( num == RT_PERSO_BAND          )  strcpy(text, "Kolor paskÛw:");

#if _NEWLOOK
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "CeeBot");
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Czy na pewno chcesz opuúciÊ grÍ CeeBot?");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "ZakoÒcz\\KoÒczy grÍ CeeBot");
#else
        if ( num == RT_DIALOG_TITLE        )  strcpy(text, "COLOBOT");
        if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Czy na pewno chcesz opuúciÊ grÍ COLOBOT?");
        if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "ZakoÒcz\\KoÒczy grÍ COLOBOT");
#endif
        if ( num == RT_DIALOG_ABORT        )  strcpy(text, "OpuúciÊ misjÍ?");
        if ( num == RT_DIALOG_YES          )  strcpy(text, "Przerwij\\Przerywa bieøπcπ misjÍ");
        if ( num == RT_DIALOG_NO           )  strcpy(text, "Kontynuuj\\Kontynuuje bieøπcπ misjÍ");
        if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Kontynuuj\\Kontynuuje grÍ");
        if ( num == RT_DIALOG_DELOBJ       )  strcpy(text, "Czy na pewno chcesz zniszczyÊ zaznaczony budynek?");
        if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Czy na pewno chcesz skasowaÊ zapisane gry gracza %s? ");
        if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "UsuÒ");
        if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Anuluj");
        if ( num == RT_DIALOG_LOADING      )  strcpy(text, "WCZYTYWANIE");

        if ( num == RT_STUDIO_LISTTT       )  strcpy(text, "SkrÛty klawiszowe (\\key cbot;)");
        if ( num == RT_STUDIO_COMPOK       )  strcpy(text, "Program skompilowany (0 b≥ÍdÛw)");
        if ( num == RT_STUDIO_PROGSTOP     )  strcpy(text, "Program zakoÒczony");

        if ( num == RT_SATCOM_LIST         )  strcpy(text, "\\b;Lista obiektÛw\n");
        if ( num == RT_SATCOM_BOT          )  strcpy(text, "\\b;Roboty\n");
        if ( num == RT_SATCOM_BUILDING     )  strcpy(text, "\\b;Budynki\n");
        if ( num == RT_SATCOM_FRET         )  strcpy(text, "\\b;Obiekty ruchome\n");
        if ( num == RT_SATCOM_ALIEN        )  strcpy(text, "\\b;Obcy\n");
        if ( num == RT_SATCOM_NULL         )  strcpy(text, "\\c; (brak)\\n;\n");
        if ( num == RT_SATCOM_ERROR1       )  strcpy(text, "\\b;B≥πd\n");
        if ( num == RT_SATCOM_ERROR2       )  strcpy(text, "Lista jest dostÍpna jedynie gdy dzia≥a \\l;stacja radarowa\\u object\\radar;.\n");

        if ( num == RT_IO_OPEN             )  strcpy(text, "OtwÛrz");
        if ( num == RT_IO_SAVE             )  strcpy(text, "Zapisz");
        if ( num == RT_IO_LIST             )  strcpy(text, "Folder: %s");
        if ( num == RT_IO_NAME             )  strcpy(text, "Nazwa:");
        if ( num == RT_IO_DIR              )  strcpy(text, "Folder:");
        if ( num == RT_IO_PRIVATE          )  strcpy(text, "Prywatny\\Folder prywatny");
        if ( num == RT_IO_PUBLIC           )  strcpy(text, "Publiczny\\Folder ogÛlnodostÍpny");

        if ( num == RT_GENERIC_DEV1        )  strcpy(text, "TwÛrcy:");
        if ( num == RT_GENERIC_DEV2        )  strcpy(text, "www.epsitec.com");
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, "Wersja polska wydana przez:");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, "www.manta.com.pl");
        if ( num == RT_GENERIC_EDIT1       )  strcpy(text, " ");
        if ( num == RT_GENERIC_EDIT2       )  strcpy(text, " ");

        if ( num == RT_INTERFACE_REC       )  strcpy(text, "Recorder");
    }

    if ( type == RES_EVENT )
    {
        if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
        if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Anuluj");
        if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "NastÍpny");
        if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Poprzedni");
        if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

        if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
        if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Anuluj");

        if ( num == EVENT_INTERFACE_TRAINER)  strcpy(text, "∆wiczenia\\∆wiczenia programistyczne");
        if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Wyzwania\\Wyzwania programistyczne");
        if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Misje\\Wybierz misjÍ");
        if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Swobodna gra\\Swobodna gra bez konkretnych celÛw");
        if ( num == EVENT_INTERFACE_TEEN   )  strcpy(text, "Swobodna gra\\Swobodna gra bez konkretnych celÛw");
        if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Poziomy\\Poziomy uøytkownika");
        if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Prototypy\\Prototypy w trakcie rozwijania");
        if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Nowy gracz\\Wybierz imiÍ gracza");
        if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opcje\\Preferencje");
        if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Uruchom ponownie\\Uruchamia ponownie misjÍ od poczπtku");
        if ( num == EVENT_INTERFACE_WRITE  )  strcpy(text, "Zapisz\\Zapisuje bieøπcπ misjÍ");
        if ( num == EVENT_INTERFACE_READ   )  strcpy(text, "Wczytaj\\Wczytuje zapisanπ misjÍ");
#if _NEWLOOK
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\PowrÛÊ do gry CeeBot");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "ZakoÒcz\\KoÒczy grÍ CeeBot");
#else
        if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\PowrÛÊ do gry COLOBOT");
        if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "ZakoÒcz\\KoÒczy grÍ COLOBOT");
#endif
        if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "<<  Wstecz  \\Wraca do poprzedniego ekranu");
        if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Graj\\Rozpoczyna misjÍ!");
        if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Urzπdzenie\\Ustawienia sterownika i rozdzielczoúci");
        if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Grafika\\Ustawienia grafiki");
        if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Gra\\Ustawienia gry");
        if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Sterowanie\\Ustawienia klawiatury, joysticka i myszy");
        if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "DüwiÍk\\G≥oúnoúÊ muzyki i düwiÍkÛw gry");
        if ( num == EVENT_INTERFACE_DEVICE )  strcpy(text, "Jednostka");
        if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "RozdzielczoúÊ");
        if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Pe≥ny ekran\\Pe≥ny ekran lub tryb okna");
        if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Zastosuj zmiany\\Aktywuje zmienione ustawienia");

        if ( num == EVENT_INTERFACE_TOTO   )  strcpy(text, "Robbie\\TwÛj asystent");
        if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Cienie\\Cienie na ziemi");
        if ( num == EVENT_INTERFACE_GROUND )  strcpy(text, "Znaki na ziemi\\Znaki na ziemi");
        if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Kurz\\Kurz i brÛd na robotach i budynkach");
        if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Mg≥a\\Mg≥a");
        if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Promienie s≥oneczne\\Promienie s≥oneczne na niebie");
        if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Niebo\\Chmury i mg≥awice");
        if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planety i gwiazdy\\Obiekty astronomiczne na niebie");
        if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Dynamiczne oúwietlenie\\Ruchome ürÛd≥a úwiat≥a");
        if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Liczba czπstek\\Wybuchy, kurz, odbicia, itp.");
        if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "G≥ÍbokoúÊ pola\\Maksymalna widocznoúÊ");
        if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "SzczegÛ≥y\\JakoúÊ wizualna obiektÛw 3D");
        if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Tekstury\\JakoúÊ tekstur ");
        if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "IloúÊ elementÛw dekoracyjnych \\IloúÊ elementÛw czysto dekoracyjnych");
        if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Czπstki w interfejsie\\Para i iskry z silnikÛw w interfejsie");
        if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Odbicia na przyciskach \\åwiecπce przyciski");
        if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Dymki pomocy\\Wyjaúnia funkcje przyciskÛw");
        if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Sekwencje filmowe\\Filmy przed rozpoczÍciem i na zakoÒczenie misji");
        if ( num == EVENT_INTERFACE_NICERST)  strcpy(text, "KoÒcowy film\\Film na zakoÒczenie ÊwiczeÒ");
        if ( num == EVENT_INTERFACE_HIMSELF)  strcpy(text, "Przyjacielski ogieÒ\\W≥asne strza≥y uszkadzajπ Twoje obiekty");
        if ( num == EVENT_INTERFACE_SCROLL )  strcpy(text, "Przewijanie\\Ekran jest przewijany gdy mysz dotknie prawej lub lewej jego krawÍdzi");
        if ( num == EVENT_INTERFACE_INVERTX)  strcpy(text, "OdwrÛcenie myszy X\\OdwrÛcenie kierunkÛw przewijania w poziomie");
        if ( num == EVENT_INTERFACE_INVERTY)  strcpy(text, "OdwrÛcenie myszy Y\\OdwrÛcenie kierunkÛw przewijania w pionie");
        if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Wstrzπsy przy wybuchach\\Ekran trzÍsie siÍ podczas wybuchÛw");
        if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "CieÒ kursora myszy\\Dodaje cieÒ kursorowi myszy");
        if ( num == EVENT_INTERFACE_EDITMODE) strcpy(text, "Automatyczne wciÍcia\\Automatyczne wciÍcia podczas edycji programu");
        if ( num == EVENT_INTERFACE_EDITVALUE)strcpy(text, "Duøe wciÍcie\\2 lub 4 spacje wciÍcia na kaødy poziom zdefiniowany przez klamry");
        if ( num == EVENT_INTERFACE_SOLUCE4)  strcpy(text, "AccËs aux solutions\\Programme \"4: Solution\" dans les exercices");

        if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Standardowa kontrola\\Standardowe klawisze funkcyjne");
        if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "SkrÍÊ w lewo\\Obraca robota w lewo");
        if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "ObrÛÊ w prawo\\Obraca robota w prawo");
        if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "NaprzÛd\\Porusza do przodu");
        if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Wstecz\\Porusza do ty≥u");
        if ( num == EVENT_INTERFACE_KGUP   )  strcpy(text, "W gÛrÍ\\ZwiÍksza moc silnika");
        if ( num == EVENT_INTERFACE_KGDOWN )  strcpy(text, "W dÛ≥\\Zmniejsza moc silnika");
        if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "ZmieÒ kamerÍ\\Prze≥πcza pomiÍdzy kamerπ pok≥adowπ i úledzπcπ");
        if ( num == EVENT_INTERFACE_KDESEL )  strcpy(text, "Poprzedni obiekt\\Zaznacz poprzedni obiekt");
        if ( num == EVENT_INTERFACE_KACTION)  strcpy(text, "Standardowa akcja\\Standardowa akcja robota (podnieú/upuúÊ, strzelaj, szukaj, itp.)");
        if ( num == EVENT_INTERFACE_KNEAR  )  strcpy(text, "Kamera bliøej\\Przybliøa kamerÍ");
        if ( num == EVENT_INTERFACE_KAWAY  )  strcpy(text, "Kamera dalej\\Oddala kamerÍ");
        if ( num == EVENT_INTERFACE_KNEXT  )  strcpy(text, "NastÍpny obiekt\\Zaznacza nastÍpny obiekt");
        if ( num == EVENT_INTERFACE_KHUMAN )  strcpy(text, "Zaznacz astronautÍ\\Zaznacza astronautÍ");
        if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "ZakoÒcz\\KoÒczy bieøπcπ misjÍ lub Êwiczenie");
        if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Rozkazy\\Pokazuje rozkazy dotyczπce bieøπcej misji");
        if ( num == EVENT_INTERFACE_KPROG  )  strcpy(text, "PodrÍcznik programowania\\Dostarcza szczegÛ≥owπ pomoc w programowaniu");
        if ( num == EVENT_INTERFACE_KCBOT  )  strcpy(text, "Pomoc dot. s≥Ûw kluczowych\\Dok≥adniejsza pomoc na temat s≥Ûw kluczowych");
        if ( num == EVENT_INTERFACE_KVISIT )  strcpy(text, "Miejsce nadania wiadomoúci\\Pokazuje skπd zosta≥a wys≥ana ostatnia wiadomoúÊ");
        if ( num == EVENT_INTERFACE_KSPEED10) strcpy(text, "PrÍdkoúÊ 1,0x\\PrÍdkoúÊ normalna");
        if ( num == EVENT_INTERFACE_KSPEED15) strcpy(text, "PrÍdkoúÊ 1,5x\\1,5 raza szybciej");
        if ( num == EVENT_INTERFACE_KSPEED20) strcpy(text, "PrÍdkoúÊ 2,0x\\Dwa razy szybciej");
        if ( num == EVENT_INTERFACE_KSPEED30) strcpy(text, "PrÍdkoúÊ 3,0x\\Trzy razy szybciej");

        if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Efekty düwiÍkowe:\\G≥oúnoúÊ silnikÛw, g≥osÛw, strza≥Ûw, itp.");
        if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Muzyka w tle :\\G≥oúnoúÊ úcieøek düwiÍkowych z p≥yty CD");
        if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "DüwiÍk 3D\\Przestrzenne pozycjonowanie düwiÍkÛw");

        if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Najniøsza\\Minimalna jakoúÊ grafiki (najwyøsza czÍstotliwoúÊ odúwieøania)");
        if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normalna\\Normalna jakoúÊ grafiki");
        if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Najwyøsza\\Maksymalna jakoúÊ grafiki (najniøsza czÍstotliwoúÊ odúwieøania)");

        if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Cisza\\Brak düwiÍkÛw");
        if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normalne\\Normalna g≥oúnoúÊ düwiÍkÛw");

        if ( num == EVENT_INTERFACE_JOYSTICK) strcpy(text, "Uøywaj joysticka\\Joystick lub klawiatura");
        if ( num == EVENT_INTERFACE_SOLUCE )  strcpy(text, "DostÍp do rozwiπzania\\Pokazuje rozwiπzanie (szczegÛ≥owe instrukcje dotyczπce misji)");

        if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nowe imiÍ gracza");
        if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "OK\\Wybiera zaznaczonego gracza");
        if ( num == EVENT_INTERFACE_NCANCEL)  strcpy(text, "Anuluj\\Zachowuje bieøπce imiÍ gracza");
        if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "UsuÒ gracza\\Usuwa gracza z listy");
        if ( num == EVENT_INTERFACE_NLABEL )  strcpy(text, "ImiÍ gracza");

        if ( num == EVENT_INTERFACE_IOWRITE)  strcpy(text, "Zapisz\\Zapisuje bieøπcπ misjÍ");
        if ( num == EVENT_INTERFACE_IOREAD )  strcpy(text, "Wczytaj\\Wczytuje zaznaczonπ misjÍ");
        if ( num == EVENT_INTERFACE_IOLIST )  strcpy(text, "Lista zapisanych misji");
        if ( num == EVENT_INTERFACE_IOLABEL)  strcpy(text, "Nazwa pliku:");
        if ( num == EVENT_INTERFACE_IONAME )  strcpy(text, "Nazwa misji");
        if ( num == EVENT_INTERFACE_IOIMAGE)  strcpy(text, "Fotografia");
        if ( num == EVENT_INTERFACE_IODELETE) strcpy(text, "UsuÒ\\Usuwa zaznaczony plik");

        if ( num == EVENT_INTERFACE_PERSO  )  strcpy(text, "Wyglπd\\Wybierz swojπ postaÊ");
        if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "OK");
        if ( num == EVENT_INTERFACE_PCANCEL)  strcpy(text, "Anuluj");
        if ( num == EVENT_INTERFACE_PDEF   )  strcpy(text, "Standardowe\\Standardowe ustawienia wyglπdu");
        if ( num == EVENT_INTERFACE_PHEAD  )  strcpy(text, "G≥owa\\Twarz i w≥osy");
        if ( num == EVENT_INTERFACE_PBODY  )  strcpy(text, "Skafander\\Skafander astronauty");
        if ( num == EVENT_INTERFACE_PLROT  )  strcpy(text, "\\ObrÛÊ w lewo");
        if ( num == EVENT_INTERFACE_PRROT  )  strcpy(text, "\\ObrÛÊ w prawo");
        if ( num == EVENT_INTERFACE_PCRa   )  strcpy(text, "Czerwony");
        if ( num == EVENT_INTERFACE_PCGa   )  strcpy(text, "Zielony");
        if ( num == EVENT_INTERFACE_PCBa   )  strcpy(text, "Niebieski");
        if ( num == EVENT_INTERFACE_PCRb   )  strcpy(text, "Czerwony");
        if ( num == EVENT_INTERFACE_PCGb   )  strcpy(text, "Zielony");
        if ( num == EVENT_INTERFACE_PCBb   )  strcpy(text, "Niebieski");
        if ( num == EVENT_INTERFACE_PFACE1 )  strcpy(text, "\\Twarz 1");
        if ( num == EVENT_INTERFACE_PFACE2 )  strcpy(text, "\\Twarz 4");
        if ( num == EVENT_INTERFACE_PFACE3 )  strcpy(text, "\\Twarz 3");
        if ( num == EVENT_INTERFACE_PFACE4 )  strcpy(text, "\\Twarz 2");
        if ( num == EVENT_INTERFACE_PGLASS0)  strcpy(text, "\\Bez okularÛw");
        if ( num == EVENT_INTERFACE_PGLASS1)  strcpy(text, "\\Okulary 1");
        if ( num == EVENT_INTERFACE_PGLASS2)  strcpy(text, "\\Okulary 2");
        if ( num == EVENT_INTERFACE_PGLASS3)  strcpy(text, "\\Okulary 3");
        if ( num == EVENT_INTERFACE_PGLASS4)  strcpy(text, "\\Okulary 4");
        if ( num == EVENT_INTERFACE_PGLASS5)  strcpy(text, "\\Okulary 5");

        if ( num == EVENT_OBJECT_DESELECT  )  strcpy(text, "Poprzednie zaznaczenie (\\key desel;)");
        if ( num == EVENT_OBJECT_LEFT      )  strcpy(text, "SkrÍÊ w lewo (\\key left;)");
        if ( num == EVENT_OBJECT_RIGHT     )  strcpy(text, "SkrÍÊ w prawo (\\key right;)");
        if ( num == EVENT_OBJECT_UP        )  strcpy(text, "NaprzÛd (\\key up;)");
        if ( num == EVENT_OBJECT_DOWN      )  strcpy(text, "Cofnij (\\key down;)");
        if ( num == EVENT_OBJECT_GASUP     )  strcpy(text, "GÛra (\\key gup;)");
        if ( num == EVENT_OBJECT_GASDOWN   )  strcpy(text, "DÛ≥ (\\key gdown;)");
        if ( num == EVENT_OBJECT_HTAKE     )  strcpy(text, "Podnieú lub upuúÊ (\\key action;)");
        if ( num == EVENT_OBJECT_MTAKE     )  strcpy(text, "Podnieú lub upuúÊ (\\key action;)");
        if ( num == EVENT_OBJECT_MFRONT    )  strcpy(text, "..przed");
        if ( num == EVENT_OBJECT_MBACK     )  strcpy(text, "..za");
        if ( num == EVENT_OBJECT_MPOWER    )  strcpy(text, "..ogniwo elektryczne");
        if ( num == EVENT_OBJECT_BHELP     )  strcpy(text, "Rozkazy dotyczπce misji (\\key help;)");
        if ( num == EVENT_OBJECT_BTAKEOFF  )  strcpy(text, "OdleÊ, aby zakoÒczyÊ misjÍ");
        if ( num == EVENT_OBJECT_BDERRICK  )  strcpy(text, "Zbuduj kopalniÍ");
        if ( num == EVENT_OBJECT_BSTATION  )  strcpy(text, "Zbuduj elektrowniÍ");
        if ( num == EVENT_OBJECT_BFACTORY  )  strcpy(text, "Zbuduj fabrykÍ robotÛw");
        if ( num == EVENT_OBJECT_BREPAIR   )  strcpy(text, "Zbuduj warsztat");
        if ( num == EVENT_OBJECT_BCONVERT  )  strcpy(text, "Zbuduj hutÍ");
        if ( num == EVENT_OBJECT_BTOWER    )  strcpy(text, "Zbuduj wieøÍ obronnπ");
        if ( num == EVENT_OBJECT_BRESEARCH )  strcpy(text, "Zbuduj centrum badawcze");
        if ( num == EVENT_OBJECT_BRADAR    )  strcpy(text, "Zbuduj stacjÍ radarowπ");
        if ( num == EVENT_OBJECT_BENERGY   )  strcpy(text, "Zbuduj fabrykÍ ogniw elektrycznych");
        if ( num == EVENT_OBJECT_BLABO     )  strcpy(text, "Zbuduj laboratorium");
        if ( num == EVENT_OBJECT_BNUCLEAR  )  strcpy(text, "Zbuduj elektrowniÍ atomowπ");
        if ( num == EVENT_OBJECT_BPARA     )  strcpy(text, "Zbuduj odgromnik");
        if ( num == EVENT_OBJECT_BINFO     )  strcpy(text, "Zbuduj stacjÍ przekaünikowπ");
        if ( num == EVENT_OBJECT_GFLAT     )  strcpy(text, "Pokaø czy teren jest p≥aski");
        if ( num == EVENT_OBJECT_FCREATE   )  strcpy(text, "Postaw flagÍ");
        if ( num == EVENT_OBJECT_FDELETE   )  strcpy(text, "UsuÒ flagÍ");
        if ( num == EVENT_OBJECT_FCOLORb   )  strcpy(text, "\\Niebieskie flagi");
        if ( num == EVENT_OBJECT_FCOLORr   )  strcpy(text, "\\Czerwone flagi");
        if ( num == EVENT_OBJECT_FCOLORg   )  strcpy(text, "\\Zielone flagi");
        if ( num == EVENT_OBJECT_FCOLORy   )  strcpy(text, "\\ØÛ≥te flagi");
        if ( num == EVENT_OBJECT_FCOLORv   )  strcpy(text, "\\Fioletowe flagi");
        if ( num == EVENT_OBJECT_FACTORYfa )  strcpy(text, "Zbuduj transporter latajπcy");
        if ( num == EVENT_OBJECT_FACTORYta )  strcpy(text, "Zbuduj transporter na gπsienicach");
        if ( num == EVENT_OBJECT_FACTORYwa )  strcpy(text, "Zbuduj transporter na ko≥ach");
        if ( num == EVENT_OBJECT_FACTORYia )  strcpy(text, "Zbuduj transporter na nogach");
        if ( num == EVENT_OBJECT_FACTORYfc )  strcpy(text, "Zbuduj dzia≥o latajπce");
        if ( num == EVENT_OBJECT_FACTORYtc )  strcpy(text, "Zbuduj dzia≥o na gπsienicach");
        if ( num == EVENT_OBJECT_FACTORYwc )  strcpy(text, "Zbuduj dzia≥o na ko≥ach");
        if ( num == EVENT_OBJECT_FACTORYic )  strcpy(text, "Zbuduj dzia≥o na nogach");
        if ( num == EVENT_OBJECT_FACTORYfi )  strcpy(text, "Zbuduj latajπce dzia≥o organiczne");
        if ( num == EVENT_OBJECT_FACTORYti )  strcpy(text, "Zbuduj dzia≥o organiczne na gπsienicach");
        if ( num == EVENT_OBJECT_FACTORYwi )  strcpy(text, "Zbuduj dzia≥o organiczne na ko≥ach");
        if ( num == EVENT_OBJECT_FACTORYii )  strcpy(text, "Zbuduj dzia≥o organiczne na nogach");
        if ( num == EVENT_OBJECT_FACTORYfs )  strcpy(text, "Zbuduj szperacz latajπcy");
        if ( num == EVENT_OBJECT_FACTORYts )  strcpy(text, "Zbuduj szperacz na gπsienicach");
        if ( num == EVENT_OBJECT_FACTORYws )  strcpy(text, "Zbuduj szperacz na ko≥ach");
        if ( num == EVENT_OBJECT_FACTORYis )  strcpy(text, "Zbuduj szperacz na nogach");
        if ( num == EVENT_OBJECT_FACTORYrt )  strcpy(text, "Zbuduj robota uderzacza");
        if ( num == EVENT_OBJECT_FACTORYrc )  strcpy(text, "Zbuduj dzia≥o fazowe");
        if ( num == EVENT_OBJECT_FACTORYrr )  strcpy(text, "Zbuduj robota recyklera");
        if ( num == EVENT_OBJECT_FACTORYrs )  strcpy(text, "Zbuduj robota os≥aniajacza");
        if ( num == EVENT_OBJECT_FACTORYsa )  strcpy(text, "Zbuduj robota nurka");
        if ( num == EVENT_OBJECT_RTANK     )  strcpy(text, "Rozpocznij prace badawcze nad transporterem na gπsienicach");
        if ( num == EVENT_OBJECT_RFLY      )  strcpy(text, "Rozpocznij prace badawcze nad transporterem latajπcym");
        if ( num == EVENT_OBJECT_RTHUMP    )  strcpy(text, "Rozpocznij prace badawcze nad robotem uderzaczem");
        if ( num == EVENT_OBJECT_RCANON    )  strcpy(text, "Rozpocznij prace badawcze nad dzia≥em");
        if ( num == EVENT_OBJECT_RTOWER    )  strcpy(text, "Rozpocznij prace badawcze nad wieøπ obronnπ");
        if ( num == EVENT_OBJECT_RPHAZER   )  strcpy(text, "Rozpocznij prace badawcze nad dzia≥em fazowym");
        if ( num == EVENT_OBJECT_RSHIELD   )  strcpy(text, "Rozpocznij prace badawcze nad robotem os≥aniaczem");
        if ( num == EVENT_OBJECT_RATOMIC   )  strcpy(text, "Rozpocznij prace badawcze nad energiπ atomowπ");
        if ( num == EVENT_OBJECT_RiPAW     )  strcpy(text, "Rozpocznij prace badawcze nad transporterem na nogach");
        if ( num == EVENT_OBJECT_RiGUN     )  strcpy(text, "Rozpocznij prace badawcze nad dzia≥em organicznym");
        if ( num == EVENT_OBJECT_RESET     )  strcpy(text, "PowrÛt do poczπtku");
        if ( num == EVENT_OBJECT_SEARCH    )  strcpy(text, "Szukaj (\\key action;)");
        if ( num == EVENT_OBJECT_TERRAFORM )  strcpy(text, "Uderz (\\key action;)");
        if ( num == EVENT_OBJECT_FIRE      )  strcpy(text, "Strzelaj (\\key action;)");
        if ( num == EVENT_OBJECT_RECOVER   )  strcpy(text, "Odzyskaj (\\key action;)");
        if ( num == EVENT_OBJECT_BEGSHIELD )  strcpy(text, "Rozszerz os≥onÍ (\\key action;)");
        if ( num == EVENT_OBJECT_ENDSHIELD )  strcpy(text, "Wy≥πcz os≥onÍ (\\key action;)");
        if ( num == EVENT_OBJECT_DIMSHIELD )  strcpy(text, "ZasiÍg os≥ony");
        if ( num == EVENT_OBJECT_PROGRUN   )  strcpy(text, "Wykonaj zaznaczony program");
        if ( num == EVENT_OBJECT_PROGEDIT  )  strcpy(text, "Edytuj zaznaczony program");
        if ( num == EVENT_OBJECT_INFOOK    )  strcpy(text, "\\Prze≥πcz przekaünik SatCom w stan gotowoúci");
        if ( num == EVENT_OBJECT_DELETE    )  strcpy(text, "Zniszcz budynek");
        if ( num == EVENT_OBJECT_GENERGY   )  strcpy(text, "Poziom energii");
        if ( num == EVENT_OBJECT_GSHIELD   )  strcpy(text, "Poziom os≥ony");
        if ( num == EVENT_OBJECT_GRANGE    )  strcpy(text, "Temperatura silnika");
        if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Wciπø pracuje...");
        if ( num == EVENT_OBJECT_GRADAR    )  strcpy(text, "Liczba wykrytych insektÛw");
        if ( num == EVENT_OBJECT_GINFO     )  strcpy(text, "Przes≥ane informacje");
        if ( num == EVENT_OBJECT_COMPASS   )  strcpy(text, "Kompas");
//?     if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mapka");
        if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "PowiÍkszenie mapki");
        if ( num == EVENT_OBJECT_CAMERA    )  strcpy(text, "Kamera (\\key camera;)");
        if ( num == EVENT_OBJECT_CAMERAleft)  strcpy(text, "Camera to left");
        if ( num == EVENT_OBJECT_CAMERAright) strcpy(text, "Camera to right");
        if ( num == EVENT_OBJECT_CAMERAnear)  strcpy(text, "Camera nearest");
        if ( num == EVENT_OBJECT_CAMERAaway)  strcpy(text, "Camera awayest");
        if ( num == EVENT_OBJECT_HELP      )  strcpy(text, "Pomoc na temat zaznaczonego obiektu");
        if ( num == EVENT_OBJECT_SOLUCE    )  strcpy(text, "Pokaø rozwiπzanie");
        if ( num == EVENT_OBJECT_SHORTCUT00)  strcpy(text, "Prze≥πcz roboty <-> budynki");
        if ( num == EVENT_OBJECT_LIMIT     )  strcpy(text, "Pokaø zasiÍg");
        if ( num == EVENT_OBJECT_PEN0      )  strcpy(text, "\\RelËve le crayon");
        if ( num == EVENT_OBJECT_PEN1      )  strcpy(text, "\\Abaisse le crayon noir");
        if ( num == EVENT_OBJECT_PEN2      )  strcpy(text, "\\Abaisse le crayon jaune");
        if ( num == EVENT_OBJECT_PEN3      )  strcpy(text, "\\Abaisse le crayon orange");
        if ( num == EVENT_OBJECT_PEN4      )  strcpy(text, "\\Abaisse le crayon rouge");
        if ( num == EVENT_OBJECT_PEN5      )  strcpy(text, "\\Abaisse le crayon violet");
        if ( num == EVENT_OBJECT_PEN6      )  strcpy(text, "\\Abaisse le crayon bleu");
        if ( num == EVENT_OBJECT_PEN7      )  strcpy(text, "\\Abaisse le crayon vert");
        if ( num == EVENT_OBJECT_PEN8      )  strcpy(text, "\\Abaisse le crayon brun");
        if ( num == EVENT_OBJECT_REC       )  strcpy(text, "\\DÈmarre l'enregistrement");
        if ( num == EVENT_OBJECT_STOP      )  strcpy(text, "\\Stoppe l'enregistrement");
        if ( num == EVENT_DT_VISIT0        ||
             num == EVENT_DT_VISIT1        ||
             num == EVENT_DT_VISIT2        ||
             num == EVENT_DT_VISIT3        ||
             num == EVENT_DT_VISIT4        )  strcpy(text, "Pokaø miejsce");
        if ( num == EVENT_DT_END           )  strcpy(text, "Kontynuuj");
        if ( num == EVENT_CMD              )  strcpy(text, "Linia polecenia");
        if ( num == EVENT_SPEED            )  strcpy(text, "PrÍdkoúÊ gry");

        if ( num == EVENT_HYPER_PREV       )  strcpy(text, "Wstecz");
        if ( num == EVENT_HYPER_NEXT       )  strcpy(text, "NaprzÛd");
        if ( num == EVENT_HYPER_HOME       )  strcpy(text, "Poczπtek");
        if ( num == EVENT_HYPER_COPY       )  strcpy(text, "Kopiuj");
        if ( num == EVENT_HYPER_SIZE1      )  strcpy(text, "WielkoúÊ 1");
        if ( num == EVENT_HYPER_SIZE2      )  strcpy(text, "WielkoúÊ 2");
        if ( num == EVENT_HYPER_SIZE3      )  strcpy(text, "WielkoúÊ 3");
        if ( num == EVENT_HYPER_SIZE4      )  strcpy(text, "WielkoúÊ 4");
        if ( num == EVENT_HYPER_SIZE5      )  strcpy(text, "WielkoúÊ 5");
        if ( num == EVENT_SATCOM_HUSTON    )  strcpy(text, "Rozkazy z Houston");
#if _TEEN
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Raport z satelity");
#else
        if ( num == EVENT_SATCOM_SAT       )  strcpy(text, "Raport z satelity");
#endif
        if ( num == EVENT_SATCOM_LOADING   )  strcpy(text, "Program dostarczony z Houston");
        if ( num == EVENT_SATCOM_OBJECT    )  strcpy(text, "Lista obiektÛw");
        if ( num == EVENT_SATCOM_PROG      )  strcpy(text, "PodrÍcznik programowania");
        if ( num == EVENT_SATCOM_SOLUCE    )  strcpy(text, "Rozwiπzanie");

        if ( num == EVENT_STUDIO_OK        )  strcpy(text, "OK\\Zamyka edytor programu i powraca do gry");
        if ( num == EVENT_STUDIO_CANCEL    )  strcpy(text, "Anuluj\\Pomija wszystkie zmiany");
        if ( num == EVENT_STUDIO_NEW       )  strcpy(text, "Nowy");
        if ( num == EVENT_STUDIO_OPEN      )  strcpy(text, "OtwÛrz (Ctrl+O)");
        if ( num == EVENT_STUDIO_SAVE      )  strcpy(text, "Zapisz (Ctrl+S)");
        if ( num == EVENT_STUDIO_UNDO      )  strcpy(text, "Cofnij (Ctrl+Z)");
        if ( num == EVENT_STUDIO_CUT       )  strcpy(text, "Wytnij (Ctrl+X)");
        if ( num == EVENT_STUDIO_COPY      )  strcpy(text, "Kopiuj (Ctrl+C)");
        if ( num == EVENT_STUDIO_PASTE     )  strcpy(text, "Wklej (Ctrl+V)");
        if ( num == EVENT_STUDIO_SIZE      )  strcpy(text, "WielkoúÊ czcionki");
        if ( num == EVENT_STUDIO_TOOL      )  strcpy(text, "Rozkazy (\\key help;)");
        if ( num == EVENT_STUDIO_HELP      )  strcpy(text, "PodrÍcznik programowania  (\\key prog;)");
        if ( num == EVENT_STUDIO_COMPILE   )  strcpy(text, "Kompiluj");
        if ( num == EVENT_STUDIO_RUN       )  strcpy(text, "Wykonaj/Zatrzymaj");
        if ( num == EVENT_STUDIO_REALTIME  )  strcpy(text, "Pauza/Kontynuuj");
        if ( num == EVENT_STUDIO_STEP      )  strcpy(text, "Jeden krok");
    }

    if ( type == RES_OBJECT )
    {
        if ( num == OBJECT_PORTICO      )  strcpy(text, "Øuraw przesuwalny");
        if ( num == OBJECT_BASE         )  strcpy(text, "Statek kosmiczny");
        if ( num == OBJECT_DERRICK      )  strcpy(text, "Kopalnia");
        if ( num == OBJECT_FACTORY      )  strcpy(text, "Fabryka robotÛw");
        if ( num == OBJECT_REPAIR       )  strcpy(text, "Warsztat");
        if ( num == OBJECT_DESTROYER    )  strcpy(text, "Destroyer");
        if ( num == OBJECT_STATION      )  strcpy(text, "Stacja energetyczna");
        if ( num == OBJECT_CONVERT      )  strcpy(text, "Przetop rudÍ na tytan");
        if ( num == OBJECT_TOWER        )  strcpy(text, "Wieøa obronna");
        if ( num == OBJECT_NEST         )  strcpy(text, "Gniazdo");
        if ( num == OBJECT_RESEARCH     )  strcpy(text, "Centrum badawcze");
        if ( num == OBJECT_RADAR        )  strcpy(text, "Stacja radarowa");
        if ( num == OBJECT_INFO         )  strcpy(text, "Stacja przekaünikowa informacji");
#if _TEEN
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Fabryka ogniw elektrycznych");
#else
        if ( num == OBJECT_ENERGY       )  strcpy(text, "Fabryka ogniw elektrycznych");
#endif
        if ( num == OBJECT_LABO         )  strcpy(text, "Laboratorium");
        if ( num == OBJECT_NUCLEAR      )  strcpy(text, "Elektrownia atomowa");
        if ( num == OBJECT_PARA         )  strcpy(text, "Odgromnik");
        if ( num == OBJECT_SAFE         )  strcpy(text, "Skrytka");
        if ( num == OBJECT_HUSTON       )  strcpy(text, "Centrum Kontroli Misji w Houston");
        if ( num == OBJECT_TARGET1      )  strcpy(text, "Cel");
        if ( num == OBJECT_TARGET2      )  strcpy(text, "Cel");
        if ( num == OBJECT_START        )  strcpy(text, "Poczπtek");
        if ( num == OBJECT_END          )  strcpy(text, "Koniec");
        if ( num == OBJECT_STONE        )  strcpy(text, "Ruda tytanu");
        if ( num == OBJECT_URANIUM      )  strcpy(text, "Ruda uranu");
        if ( num == OBJECT_BULLET       )  strcpy(text, "Materia organiczna");
        if ( num == OBJECT_METAL        )  strcpy(text, "Tytan");
        if ( num == OBJECT_POWER        )  strcpy(text, "Ogniwo elektryczne");
        if ( num == OBJECT_ATOMIC       )  strcpy(text, "Atomowe ogniwa elektryczne");
        if ( num == OBJECT_BBOX         )  strcpy(text, "Czarna skrzynka");
        if ( num == OBJECT_KEYa         )  strcpy(text, "Klucz A");
        if ( num == OBJECT_KEYb         )  strcpy(text, "Klucz B");
        if ( num == OBJECT_KEYc         )  strcpy(text, "Klucz C");
        if ( num == OBJECT_KEYd         )  strcpy(text, "Klucz D");
        if ( num == OBJECT_TNT          )  strcpy(text, "Materia≥y wybuchowe");
        if ( num == OBJECT_BOMB         )  strcpy(text, "Mina");
        if ( num == OBJECT_BAG          )  strcpy(text, "Zestaw przetrwania");
        if ( num == OBJECT_WAYPOINT     )  strcpy(text, "Punkt kontrolny");
        if ( num == OBJECT_FLAGb        )  strcpy(text, "Niebieska flaga");
        if ( num == OBJECT_FLAGr        )  strcpy(text, "Czerwona flaga");
        if ( num == OBJECT_FLAGg        )  strcpy(text, "Zielona flaga");
        if ( num == OBJECT_FLAGy        )  strcpy(text, "ØÛ≥ta flaga");
        if ( num == OBJECT_FLAGv        )  strcpy(text, "Fioletowa flaga");
        if ( num == OBJECT_MARKPOWER    )  strcpy(text, "èrÛd≥o energii (miejsce na elektrowniÍ)");
        if ( num == OBJECT_MARKURANIUM  )  strcpy(text, "Z≥oøe uranu (miejsce na kopalniÍ)");
        if ( num == OBJECT_MARKKEYa     )  strcpy(text, "Znaleziono klucz A (miejsce na kopalniÍ)");
        if ( num == OBJECT_MARKKEYb     )  strcpy(text, "Znaleziono klucz B (miejsce na kopalniÍ)");
        if ( num == OBJECT_MARKKEYc     )  strcpy(text, "Znaleziono klucz C (miejsce na kopalniÍ)");
        if ( num == OBJECT_MARKKEYd     )  strcpy(text, "Znaleziono klucz D (miejsce na kopalniÍ)");
        if ( num == OBJECT_MARKSTONE    )  strcpy(text, "Z≥oøe tytanu (miejsce na kopalniÍ)");
        if ( num == OBJECT_MOBILEft     )  strcpy(text, "Robot treningowy");
        if ( num == OBJECT_MOBILEtt     )  strcpy(text, "Robot treningowy");
        if ( num == OBJECT_MOBILEwt     )  strcpy(text, "Robot treningowy");
        if ( num == OBJECT_MOBILEit     )  strcpy(text, "Robot treningowy");
        if ( num == OBJECT_MOBILEfa     )  strcpy(text, "Transporter latajπcy");
        if ( num == OBJECT_MOBILEta     )  strcpy(text, "Transporter na gπsienicach");
        if ( num == OBJECT_MOBILEwa     )  strcpy(text, "Transporter na ko≥ach");
        if ( num == OBJECT_MOBILEia     )  strcpy(text, "Transporter na nogach");
        if ( num == OBJECT_MOBILEfc     )  strcpy(text, "Dzia≥o latajπce");
        if ( num == OBJECT_MOBILEtc     )  strcpy(text, "Dzia≥o na gπsienicach");
        if ( num == OBJECT_MOBILEwc     )  strcpy(text, "Dzia≥o na ko≥ach");
        if ( num == OBJECT_MOBILEic     )  strcpy(text, "Dzia≥o na nogach");
        if ( num == OBJECT_MOBILEfi     )  strcpy(text, "Latajπce dzia≥o organiczne");
        if ( num == OBJECT_MOBILEti     )  strcpy(text, "Dzia≥o organiczne na gπsienicach");
        if ( num == OBJECT_MOBILEwi     )  strcpy(text, "Dzia≥o organiczne na ko≥ach");
        if ( num == OBJECT_MOBILEii     )  strcpy(text, "Dzia≥o organiczne na nogach");
        if ( num == OBJECT_MOBILEfs     )  strcpy(text, "Szperacz latajπcy");
        if ( num == OBJECT_MOBILEts     )  strcpy(text, "Szperacz na gπsienicach");
        if ( num == OBJECT_MOBILEws     )  strcpy(text, "Szperacz na ko≥ach");
        if ( num == OBJECT_MOBILEis     )  strcpy(text, "Szperacz na nogach");
        if ( num == OBJECT_MOBILErt     )  strcpy(text, "Uderzacz");
        if ( num == OBJECT_MOBILErc     )  strcpy(text, "Dzia≥o fazowe");
        if ( num == OBJECT_MOBILErr     )  strcpy(text, "Recykler");
        if ( num == OBJECT_MOBILErs     )  strcpy(text, "Os≥aniacz");
        if ( num == OBJECT_MOBILEsa     )  strcpy(text, "Robot nurek");
        if ( num == OBJECT_MOBILEtg     )  strcpy(text, "Robot cel");
        if ( num == OBJECT_MOBILEdr     )  strcpy(text, "Drawer bot");
        if ( num == OBJECT_HUMAN        )  strcpy(text, g_gamerName);
        if ( num == OBJECT_TECH         )  strcpy(text, "Inøynier");
        if ( num == OBJECT_TOTO         )  strcpy(text, "Robbie");
        if ( num == OBJECT_MOTHER       )  strcpy(text, "KrÛlowa Obcych");
        if ( num == OBJECT_ANT          )  strcpy(text, "MrÛwka");
        if ( num == OBJECT_SPIDER       )  strcpy(text, "Pajπk");
        if ( num == OBJECT_BEE          )  strcpy(text, "Osa");
        if ( num == OBJECT_WORM         )  strcpy(text, "Robal");
        if ( num == OBJECT_EGG          )  strcpy(text, "Jajo");
        if ( num == OBJECT_RUINmobilew1 )  strcpy(text, "Wrak");
        if ( num == OBJECT_RUINmobilew2 )  strcpy(text, "Wrak");
        if ( num == OBJECT_RUINmobilet1 )  strcpy(text, "Wrak");
        if ( num == OBJECT_RUINmobilet2 )  strcpy(text, "Wrak");
        if ( num == OBJECT_RUINmobiler1 )  strcpy(text, "Wrak");
        if ( num == OBJECT_RUINmobiler2 )  strcpy(text, "Wrak");
        if ( num == OBJECT_RUINfactory  )  strcpy(text, "Ruiny");
        if ( num == OBJECT_RUINdoor     )  strcpy(text, "Ruiny");
        if ( num == OBJECT_RUINsupport  )  strcpy(text, "Odpady");
        if ( num == OBJECT_RUINradar    )  strcpy(text, "Ruiny");
        if ( num == OBJECT_RUINconvert  )  strcpy(text, "Ruiny");
        if ( num == OBJECT_RUINbase     )  strcpy(text, "Ruiny statku kosmicznego");
        if ( num == OBJECT_RUINhead     )  strcpy(text, "Ruiny statku kosmicznego");
        if ( num == OBJECT_APOLLO1      ||
             num == OBJECT_APOLLO3      ||
             num == OBJECT_APOLLO4      ||
             num == OBJECT_APOLLO5      )  strcpy(text, "Pozosta≥oúci z misji Apollo");
        if ( num == OBJECT_APOLLO2      )  strcpy(text, "Pojazd KsiÍøycowy");
    }

    if ( type == RES_ERR )
    {
        strcpy(text, "B≥πd");
        if ( num == ERR_CMD             )  strcpy(text, "Nieznane polecenie");
#if _NEWLOOK
        if ( num == ERR_INSTALL         )  strcpy(text, "Gra CeeBot nie jest zainstalowana.");
        if ( num == ERR_NOCD            )  strcpy(text, "W≥Ûø dysk CD z grπ CeeBot\ni uruchom grÍ jeszcze raz.");
#else
        if ( num == ERR_INSTALL         )  strcpy(text, "Gra COLOBOT nie jest zainstalowana.");
        if ( num == ERR_NOCD            )  strcpy(text, "W≥Ûø dysk CD z grπ COLOBOT\ni uruchom grÍ jeszcze raz.");
#endif
        if ( num == ERR_MANIP_VEH       )  strcpy(text, "Nieodpowiedni robot");
        if ( num == ERR_MANIP_FLY       )  strcpy(text, "Niemoøliwe podczas lotu");
        if ( num == ERR_MANIP_BUSY      )  strcpy(text, "Nie moøna nieúÊ wiÍcej przedmiotÛw");
        if ( num == ERR_MANIP_NIL       )  strcpy(text, "Nie ma nic do podniesienia");
        if ( num == ERR_MANIP_MOTOR     )  strcpy(text, "Niemoøliwe podczas ruchu");
        if ( num == ERR_MANIP_OCC       )  strcpy(text, "Miejsce zajÍte");
        if ( num == ERR_MANIP_FRIEND    )  strcpy(text, "Brak innego robota");
        if ( num == ERR_MANIP_RADIO     )  strcpy(text, "Nie moøesz przenosiÊ przedmiotÛw radioaktywnych");
        if ( num == ERR_MANIP_WATER     )  strcpy(text, "Nie moøesz przenosiÊ przedmiotÛw pod wodπ");
        if ( num == ERR_MANIP_EMPTY     )  strcpy(text, "Nie ma nic do upuszczenia");
        if ( num == ERR_BUILD_FLY       )  strcpy(text, "Niemoøliwe podczas lotu");
        if ( num == ERR_BUILD_WATER     )  strcpy(text, "Niemoøliwe pod wodπ");
        if ( num == ERR_BUILD_ENERGY    )  strcpy(text, "Za ma≥o energii");
        if ( num == ERR_BUILD_METALAWAY )  strcpy(text, "Tytan za daleko");
        if ( num == ERR_BUILD_METALNEAR )  strcpy(text, "Tytan za blisko");
        if ( num == ERR_BUILD_METALINEX )  strcpy(text, "Brak tytanu w pobliøu");
        if ( num == ERR_BUILD_FLAT      )  strcpy(text, "Powierzchnia nie jest wystarczajπco p≥aska");
        if ( num == ERR_BUILD_FLATLIT   )  strcpy(text, "Za ma≥o p≥askiego terenu");
        if ( num == ERR_BUILD_BUSY      )  strcpy(text, "Miejsce zajÍte");
        if ( num == ERR_BUILD_BASE      )  strcpy(text, "Za blisko statku kosmicznego");
        if ( num == ERR_BUILD_NARROW    )  strcpy(text, "Za blisko budynku");
        if ( num == ERR_BUILD_MOTOR     )  strcpy(text, "Niemoøliwe podczas ruchu");
        if ( num == ERR_SEARCH_FLY      )  strcpy(text, "Niemoøliwe podczas lotu");
        if ( num == ERR_SEARCH_VEH      )  strcpy(text, "Nieodpowiedni robot");
        if ( num == ERR_SEARCH_MOTOR    )  strcpy(text, "Niemoøliwe podczas ruchu");
        if ( num == ERR_TERRA_VEH       )  strcpy(text, "Nieodpowiedni robot");
        if ( num == ERR_TERRA_ENERGY    )  strcpy(text, "Za ma≥o energii");
        if ( num == ERR_TERRA_FLOOR     )  strcpy(text, "Nieodpowiedni teren");
        if ( num == ERR_TERRA_BUILDING  )  strcpy(text, "Budynek za blisko");
        if ( num == ERR_TERRA_OBJECT    )  strcpy(text, "Obiekt za blisko");
        if ( num == ERR_RECOVER_VEH     )  strcpy(text, "Nieodpowiedni robot");
        if ( num == ERR_RECOVER_ENERGY  )  strcpy(text, "Za ma≥o energii");
        if ( num == ERR_RECOVER_NULL    )  strcpy(text, "Nie ma niczego do odzysku");
        if ( num == ERR_SHIELD_VEH      )  strcpy(text, "Nieodpowiedni robot");
        if ( num == ERR_SHIELD_ENERGY   )  strcpy(text, "Nie ma wiÍcej energii");
        if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "B≥πd w poleceniu ruchu");
        if ( num == ERR_FIND_IMPOSSIBLE )  strcpy(text, "Obiekt nieznany");
        if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Goto: miejsce docelowe niedostÍpne");
        if ( num == ERR_GOTO_ITER       )  strcpy(text, "Goto: miejsce docelowe niedostÍpne");
        if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Goto: miejsce docelowe zajÍte");
        if ( num == ERR_FIRE_VEH        )  strcpy(text, "Nieodpowiedni robot");
        if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Za ma≥o energii");
        if ( num == ERR_FIRE_FLY        )  strcpy(text, "Niemoøliwe podczas lotu");
        if ( num == ERR_CONVERT_EMPTY   )  strcpy(text, "Brak rudy tytanu do przetopienia");
        if ( num == ERR_DERRICK_NULL    )  strcpy(text, "W ziemi nie ma øadnej rudy");
        if ( num == ERR_STATION_NULL    )  strcpy(text, "Brak energii w ziemi");
        if ( num == ERR_TOWER_POWER     )  strcpy(text, "Brak ogniwa elektrycznego");
        if ( num == ERR_TOWER_ENERGY    )  strcpy(text, "Nie ma wiÍcej energii");
        if ( num == ERR_RESEARCH_POWER  )  strcpy(text, "Brak ogniwa elektrycznego");
        if ( num == ERR_RESEARCH_ENERGY )  strcpy(text, "Za ma≥o energii");
        if ( num == ERR_RESEARCH_TYPE   )  strcpy(text, "Nieodpowiedni rodzaj ogniw");
        if ( num == ERR_RESEARCH_ALREADY)  strcpy(text, "Program badawczy zosta≥ juø wykonany");
        if ( num == ERR_ENERGY_NULL     )  strcpy(text, "Brak energii w ziemi");
        if ( num == ERR_ENERGY_LOW      )  strcpy(text, "Wciπø za ma≥o energii");
        if ( num == ERR_ENERGY_EMPTY    )  strcpy(text, "Brak tytanu do przetworzenia");
        if ( num == ERR_ENERGY_BAD      )  strcpy(text, "Przetwarza jedynie tytan");
        if ( num == ERR_BASE_DLOCK      )  strcpy(text, "Drzwi zablokowane przez robota lub inny obiekt ");
        if ( num == ERR_BASE_DHUMAN     )  strcpy(text, "Musisz byÊ na statku kosmicznym aby nim odlecieÊ");
        if ( num == ERR_LABO_NULL       )  strcpy(text, "Nie ma niczego do zanalizowania");
        if ( num == ERR_LABO_BAD        )  strcpy(text, "Analizuje jedynie materiÍ organicznπ");
        if ( num == ERR_LABO_ALREADY    )  strcpy(text, "Analiza zosta≥a juø wykonana");
        if ( num == ERR_NUCLEAR_NULL    )  strcpy(text, "Brak energii w ziemi");
        if ( num == ERR_NUCLEAR_LOW     )  strcpy(text, "Wciπø za ma≥o energii");
        if ( num == ERR_NUCLEAR_EMPTY   )  strcpy(text, "Brak uranu do przetworzenia");
        if ( num == ERR_NUCLEAR_BAD     )  strcpy(text, "Przetwarza jedynie uran");
        if ( num == ERR_FACTORY_NULL    )  strcpy(text, "Brak tytanu");
        if ( num == ERR_FACTORY_NEAR    )  strcpy(text, "Obiekt za blisko");
        if ( num == ERR_RESET_NEAR      )  strcpy(text, "Miejsce zajÍte");
        if ( num == ERR_INFO_NULL       )  strcpy(text, "Nie ma øadnej stacji przekaünikowej w zasiÍgu");
        if ( num == ERR_VEH_VIRUS       )  strcpy(text, "Program zawirusowany");
        if ( num == ERR_BAT_VIRUS       )  strcpy(text, "Zainfekowane wirusem, chwilowo niesprawne");
        if ( num == ERR_VEH_POWER       )  strcpy(text, "Brak ogniwa elektrycznego");
        if ( num == ERR_VEH_ENERGY      )  strcpy(text, "Nie ma wiÍcej energii");
        if ( num == ERR_FLAG_FLY        )  strcpy(text, "Niemoøliwe podczas lotu");
        if ( num == ERR_FLAG_WATER      )  strcpy(text, "Niemoøliwe podczas p≥ywania");
        if ( num == ERR_FLAG_MOTOR      )  strcpy(text, "Niemoøliwe podczas ruchu");
        if ( num == ERR_FLAG_BUSY       )  strcpy(text, "Niemoøliwe podczas przenoszenia przedmiotu");
        if ( num == ERR_FLAG_CREATE     )  strcpy(text, "Za duøo flag w tym kolorze (maksymalnie 5)");
        if ( num == ERR_FLAG_PROXY      )  strcpy(text, "Za blisko istniejπcej flagi");
        if ( num == ERR_FLAG_DELETE     )  strcpy(text, "Nie ma flagi w pobliøu");
        if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "Misja nie jest wype≥niona (naciúnij \\key help; aby uzyskaÊ szczegÛ≥y)");
        if ( num == ERR_DELETEMOBILE    )  strcpy(text, "Robot zniszczony");
        if ( num == ERR_DELETEBUILDING  )  strcpy(text, "Budynek zniszczony");
        if ( num == ERR_TOOMANY         )  strcpy(text, "Nie moøna tego utworzyÊ, za duøo obiektÛw");
        if ( num == ERR_OBLIGATORYTOKEN )  strcpy(text, "It misses \"%s\" in this exercise");
        if ( num == ERR_PROHIBITEDTOKEN )  strcpy(text, "Do not use in this exercise");

        if ( num == INFO_BUILD          )  strcpy(text, "Budowa zakoÒczona");
        if ( num == INFO_CONVERT        )  strcpy(text, "Tytan dostÍpny");
        if ( num == INFO_RESEARCH       )  strcpy(text, "Program badawczy zakoÒczony");
        if ( num == INFO_RESEARCHTANK   )  strcpy(text, "DostÍpne plany tranporterÛw na gπsienicach");
        if ( num == INFO_RESEARCHFLY    )  strcpy(text, "Moøesz lataÊ uøywajπc klawiszy (\\key gup;) oraz (\\key gdown;)");
        if ( num == INFO_RESEARCHTHUMP  )  strcpy(text, "DostÍpne plany robota uderzacza");
        if ( num == INFO_RESEARCHCANON  )  strcpy(text, "DostÍpne plany dzia≥a");
        if ( num == INFO_RESEARCHTOWER  )  strcpy(text, "DostÍpne plany wieøy obronnej");
        if ( num == INFO_RESEARCHPHAZER )  strcpy(text, "DostÍpne plany dzia≥a fazowego");
        if ( num == INFO_RESEARCHSHIELD )  strcpy(text, "DostÍpne plany robota os≥aniacza");
        if ( num == INFO_RESEARCHATOMIC )  strcpy(text, "DostÍpne plany elektrowni atomowej");
        if ( num == INFO_FACTORY        )  strcpy(text, "DostÍpny nowy robot");
        if ( num == INFO_LABO           )  strcpy(text, "Analiza wykonana");
        if ( num == INFO_ENERGY         )  strcpy(text, "Wytworzono ogniwo elektryczne");
        if ( num == INFO_NUCLEAR        )  strcpy(text, "Wytworzono atomowe ogniwo elektryczne");
        if ( num == INFO_FINDING        )  strcpy(text, "Znaleziono uøyteczny przedmiot");
        if ( num == INFO_MARKPOWER      )  strcpy(text, "Znaleziono miejsce na elektrowniÍ");
        if ( num == INFO_MARKURANIUM    )  strcpy(text, "Znaleziono miejsce na kopalniÍ");
        if ( num == INFO_MARKSTONE      )  strcpy(text, "Znaleziono miejsce na kopalniÍ");
        if ( num == INFO_MARKKEYa       )  strcpy(text, "Znaleziono miejsce na kopalniÍ");
        if ( num == INFO_MARKKEYb       )  strcpy(text, "Znaleziono miejsce na kopalniÍ");
        if ( num == INFO_MARKKEYc       )  strcpy(text, "Znaleziono miejsce na kopalniÍ");
        if ( num == INFO_MARKKEYd       )  strcpy(text, "Znaleziono miejsce na kopalniÍ");
        if ( num == INFO_WIN            )  strcpy(text, "<<< Dobra robota, misja wype≥niona >>>");
        if ( num == INFO_LOST           )  strcpy(text, "<<< Niestety, misja nie powiod≥a siÍ >>>");
        if ( num == INFO_LOSTq          )  strcpy(text, "<<< Niestety, misja nie powiod≥a siÍ >>>");
        if ( num == INFO_WRITEOK        )  strcpy(text, "Bieøπca misja zapisana");
        if ( num == INFO_DELETEPATH     )  strcpy(text, "Przekroczono punkt kontrolny");
        if ( num == INFO_DELETEMOTHER   )  strcpy(text, "KrÛlowa Obcych zosta≥a zabita");
        if ( num == INFO_DELETEANT      )  strcpy(text, "MrÛwka úmiertelnie raniona");
        if ( num == INFO_DELETEBEE      )  strcpy(text, "Osa úmiertelnie raniona");
        if ( num == INFO_DELETEWORM     )  strcpy(text, "Robal úmiertelnie raniony");
        if ( num == INFO_DELETESPIDER   )  strcpy(text, "Pajπk úmiertelnie raniony");
        if ( num == INFO_BEGINSATCOM    )  strcpy(text, "Naciúnij klawisz \\key help; aby wyúwietliÊ rozkazy na przekaüniku SatCom");
    }

    if ( type == RES_CBOT )
    {
        strcpy(text, "B≥πd");
        if ( num == TX_OPENPAR       ) strcpy(text, "Brak nawiasu otwierajπcego");
        if ( num == TX_CLOSEPAR      ) strcpy(text, "Brak nawiasu zamykajπcego");
        if ( num == TX_NOTBOOL       ) strcpy(text, "Wyraøenie musi zwrÛciÊ wartoúÊ logicznπ");
        if ( num == TX_UNDEFVAR      ) strcpy(text, "Zmienna nie zosta≥a zadeklarowana");
        if ( num == TX_BADLEFT       ) strcpy(text, "Przypisanie niemoøliwe");
        if ( num == TX_ENDOF         ) strcpy(text, "Brak úrednika na koÒcu wiersza");
        if ( num == TX_OUTCASE       ) strcpy(text, "Polecenie ""case"" na zewnπtrz bloku ""switch""");
        if ( num == TX_NOTERM        ) strcpy(text, "Polecenie po koÒcowej klamrze zamykajπcej");
        if ( num == TX_CLOSEBLK      ) strcpy(text, "Brak koÒca bloku");
        if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Polecenie ""else"" bez wystπpienia ""if"" ");
        if ( num == TX_OPENBLK       ) strcpy(text, "Brak klamry otwierajπcej");//dÈbut d'un bloc attendu?
        if ( num == TX_BADTYPE       ) strcpy(text, "Z≥y typ dla przypisania");
        if ( num == TX_REDEFVAR      ) strcpy(text, "Zmienna nie moøe byÊ zadeklarowana dwukrotnie");
        if ( num == TX_BAD2TYPE      ) strcpy(text, "Niezgodne typy operatorÛw");
        if ( num == TX_UNDEFCALL     ) strcpy(text, "Funkcja nieznana");
        if ( num == TX_MISDOTS       ) strcpy(text, "Brak znaku "" : ");
        if ( num == TX_WHILE         ) strcpy(text, "Brak kluczowego s≥owa ""while");
        if ( num == TX_BREAK         ) strcpy(text, "Polecenie ""break"" na zewnπtrz pÍtli");
        if ( num == TX_LABEL         ) strcpy(text, "Po etykiecie musi wystπpiÊ ""for"", ""while"", ""do"" lub ""switch""");
        if ( num == TX_NOLABEL       ) strcpy(text, "Taka etykieta nie istnieje");// Cette Ètiquette n'existe pas
        if ( num == TX_NOCASE        ) strcpy(text, "Brak polecenia ""case");
        if ( num == TX_BADNUM        ) strcpy(text, "Brak liczby");
        if ( num == TX_VOID          ) strcpy(text, "Pusty parametr");
        if ( num == TX_NOTYP         ) strcpy(text, "Brak deklaracji typu");
        if ( num == TX_NOVAR         ) strcpy(text, "Brak nazwy zmiennej");
        if ( num == TX_NOFONC        ) strcpy(text, "Brakujπca nazwa funkcji");
        if ( num == TX_OVERPARAM     ) strcpy(text, "Za duøo parametrÛw");
        if ( num == TX_REDEF         ) strcpy(text, "Funkcja juø istnieje");
        if ( num == TX_LOWPARAM      ) strcpy(text, "Brak wymaganego parametru");
        if ( num == TX_BADPARAM      ) strcpy(text, "Funkcja o tej nazwie nie akceptuje parametrÛw tego typu");
        if ( num == TX_NUMPARAM      ) strcpy(text, "Funkcja o tej nazwie nie akceptuje takiej liczby parametrÛw");
        if ( num == TX_NOITEM        ) strcpy(text, "To nie jest obiekt tej klasy");
        if ( num == TX_DOT           ) strcpy(text, "Ten obiekt nie jest cz≥onkiem klasy");
        if ( num == TX_NOCONST       ) strcpy(text, "Brak odpowiedniego konstruktora");
        if ( num == TX_REDEFCLASS    ) strcpy(text, "Taka klasa juø istnieje");
        if ( num == TX_CLBRK         ) strcpy(text, "Brak "" ] """);
        if ( num == TX_RESERVED      ) strcpy(text, "S≥owo zarezerwowane jÍzyka CBOT");
        if ( num == TX_BADNEW        ) strcpy(text, "Z≥y argument dla funkcji ""new""");
        if ( num == TX_OPBRK         ) strcpy(text, "Oczekiwane "" [ """);
        if ( num == TX_BADSTRING     ) strcpy(text, "Brak ≥aÒcucha");
        if ( num == TX_BADINDEX      ) strcpy(text, "Nieprawid≥owy typ indeksu");
        if ( num == TX_PRIVATE       ) strcpy(text, "Element prywatny");
        if ( num == TX_NOPUBLIC      ) strcpy(text, "Wymagany publiczny");
        if ( num == TX_DIVZERO       ) strcpy(text, "Dzielenie przez zero");
        if ( num == TX_NOTINIT       ) strcpy(text, "Zmienna nie zosta≥a zainicjalizowana");
        if ( num == TX_BADTHROW      ) strcpy(text, "WartoúÊ ujemna odrzucona przez ""throw""");//C'est quoi, Áa?
        if ( num == TX_NORETVAL      ) strcpy(text, "Funkcja nie zwrÛci≥a øadnej wartoúci ");
        if ( num == TX_NORUN         ) strcpy(text, "Øadna funkcja nie dzia≥a");
        if ( num == TX_NOCALL        ) strcpy(text, "Odwo≥anie do nieznanej funkcji");
        if ( num == TX_NOCLASS       ) strcpy(text, "Taka klasa nie istnieje");
        if ( num == TX_NULLPT        ) strcpy(text, "Obiekt nieznany");
        if ( num == TX_OPNAN         ) strcpy(text, "Dzia≥anie niemoøliwe z wartoúciπ ""nan""");
        if ( num == TX_OUTARRAY      ) strcpy(text, "DostÍp poza tablicÍ");
        if ( num == TX_STACKOVER     ) strcpy(text, "Przepe≥nienie stosu");
        if ( num == TX_DELETEDPT     ) strcpy(text, "Nieprawid≥owy obiekt");
        if ( num == TX_FILEOPEN      ) strcpy(text, "Nie moøna otworzyÊ pliku");
        if ( num == TX_NOTOPEN       ) strcpy(text, "Plik nie jest otwarty");
        if ( num == TX_ERRREAD       ) strcpy(text, "B≥πd odczytu");
        if ( num == TX_ERRWRITE      ) strcpy(text, "B≥πd zapisu");
    }

    if ( type == RES_KEY )
    {
        if ( num == 0                   )  strcpy(text, "< brak >");
        if ( num == VK_LEFT             )  strcpy(text, "Strza≥ka w lewo");
        if ( num == VK_RIGHT            )  strcpy(text, "Strza≥ka w prawo");
        if ( num == VK_UP               )  strcpy(text, "Strza≥ka w gÛrÍ");
        if ( num == VK_DOWN             )  strcpy(text, "Strza≥ka w dÛ≥");
        if ( num == VK_CANCEL           )  strcpy(text, "Ctrl-break");
        if ( num == VK_BACK             )  strcpy(text, "<--");
        if ( num == VK_TAB              )  strcpy(text, "Tab");
        if ( num == VK_CLEAR            )  strcpy(text, "Delete");
        if ( num == VK_RETURN           )  strcpy(text, "Enter");
        if ( num == VK_SHIFT            )  strcpy(text, "Shift");
        if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
        if ( num == VK_MENU             )  strcpy(text, "Alt");
        if ( num == VK_PAUSE            )  strcpy(text, "Pause");
        if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
        if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
        if ( num == VK_SPACE            )  strcpy(text, "Spacja");
        if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
        if ( num == VK_NEXT             )  strcpy(text, "Page Down");
        if ( num == VK_END              )  strcpy(text, "End");
        if ( num == VK_HOME             )  strcpy(text, "Home");
        if ( num == VK_SELECT           )  strcpy(text, "Zaznacz");
        if ( num == VK_EXECUTE          )  strcpy(text, "Wykonaj");
        if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
        if ( num == VK_INSERT           )  strcpy(text, "Insert");
        if ( num == VK_DELETE           )  strcpy(text, "Delete");
        if ( num == VK_HELP             )  strcpy(text, "Pomoc");
        if ( num == VK_LWIN             )  strcpy(text, "Lewy klawisz Windows");
        if ( num == VK_RWIN             )  strcpy(text, "Prawy klawisz Windows");
        if ( num == VK_APPS             )  strcpy(text, "Klawisz menu kontekstowego");
        if ( num == VK_NUMPAD0          )  strcpy(text, "Klaw. Num. 0");
        if ( num == VK_NUMPAD1          )  strcpy(text, "Klaw. Num. 1");
        if ( num == VK_NUMPAD2          )  strcpy(text, "Klaw. Num. 2");
        if ( num == VK_NUMPAD3          )  strcpy(text, "Klaw. Num. 3");
        if ( num == VK_NUMPAD4          )  strcpy(text, "Klaw. Num. 4");
        if ( num == VK_NUMPAD5          )  strcpy(text, "Klaw. Num. 5");
        if ( num == VK_NUMPAD6          )  strcpy(text, "Klaw. Num. 6");
        if ( num == VK_NUMPAD7          )  strcpy(text, "Klaw. Num. 7");
        if ( num == VK_NUMPAD8          )  strcpy(text, "Klaw. Num. 8");
        if ( num == VK_NUMPAD9          )  strcpy(text, "Klaw. Num. 9");
        if ( num == VK_MULTIPLY         )  strcpy(text, "Klaw. Num. *");
        if ( num == VK_ADD              )  strcpy(text, "Klaw. Num. +");
        if ( num == VK_SEPARATOR        )  strcpy(text, "Klaw. Num. separator");
        if ( num == VK_SUBTRACT         )  strcpy(text, "Klaw. Num. -");
        if ( num == VK_DECIMAL          )  strcpy(text, "Klaw. Num. .");
        if ( num == VK_DIVIDE           )  strcpy(text, "Klaw. Num. /");
        if ( num == VK_F1               )  strcpy(text, "F1");
        if ( num == VK_F2               )  strcpy(text, "F2");
        if ( num == VK_F3               )  strcpy(text, "F3");
        if ( num == VK_F4               )  strcpy(text, "F4");
        if ( num == VK_F5               )  strcpy(text, "F5");
        if ( num == VK_F6               )  strcpy(text, "F6");
        if ( num == VK_F7               )  strcpy(text, "F7");
        if ( num == VK_F8               )  strcpy(text, "F8");
        if ( num == VK_F9               )  strcpy(text, "F9");
        if ( num == VK_F10              )  strcpy(text, "F10");
        if ( num == VK_F11              )  strcpy(text, "F11");
        if ( num == VK_F12              )  strcpy(text, "F12");
        if ( num == VK_F13              )  strcpy(text, "F13");
        if ( num == VK_F14              )  strcpy(text, "F14");
        if ( num == VK_F15              )  strcpy(text, "F15");
        if ( num == VK_F16              )  strcpy(text, "F16");
        if ( num == VK_F17              )  strcpy(text, "F17");
        if ( num == VK_F18              )  strcpy(text, "F18");
        if ( num == VK_F19              )  strcpy(text, "F19");
        if ( num == VK_F20              )  strcpy(text, "F20");
        if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
        if ( num == VK_SCROLL           )  strcpy(text, "Scroll Lock");
        if ( num == VK_ATTN             )  strcpy(text, "Attn");
        if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
        if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
        if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
        if ( num == VK_PLAY             )  strcpy(text, "Graj");
        if ( num == VK_ZOOM             )  strcpy(text, "PowiÍkszenie");
        if ( num == VK_PA1              )  strcpy(text, "PA1");
        if ( num == VK_OEM_CLEAR        )  strcpy(text, "WyczyúÊ");
        if ( num == VK_BUTTON1          )  strcpy(text, "Przycisk 1");
        if ( num == VK_BUTTON2          )  strcpy(text, "Przycisk 2");
        if ( num == VK_BUTTON3          )  strcpy(text, "Przycisk 3");
        if ( num == VK_BUTTON4          )  strcpy(text, "Przycisk 4");
        if ( num == VK_BUTTON5          )  strcpy(text, "Przycisk 5");
        if ( num == VK_BUTTON6          )  strcpy(text, "Przycisk 6");
        if ( num == VK_BUTTON7          )  strcpy(text, "Przycisk 7");
        if ( num == VK_BUTTON8          )  strcpy(text, "Przycisk 8");
        if ( num == VK_BUTTON9          )  strcpy(text, "Przycisk 9");
        if ( num == VK_BUTTON10         )  strcpy(text, "Przycisk 10");
        if ( num == VK_BUTTON11         )  strcpy(text, "Przycisk 11");
        if ( num == VK_BUTTON12         )  strcpy(text, "Przycisk 12");
        if ( num == VK_BUTTON13         )  strcpy(text, "Przycisk 13");
        if ( num == VK_BUTTON14         )  strcpy(text, "Przycisk 14");
        if ( num == VK_BUTTON15         )  strcpy(text, "Przycisk 15");
        if ( num == VK_BUTTON16         )  strcpy(text, "Przycisk 16");
        if ( num == VK_BUTTON17         )  strcpy(text, "Przycisk 17");
        if ( num == VK_BUTTON18         )  strcpy(text, "Przycisk 18");
        if ( num == VK_BUTTON19         )  strcpy(text, "Przycisk 19");
        if ( num == VK_BUTTON20         )  strcpy(text, "Przycisk 20");
        if ( num == VK_BUTTON21         )  strcpy(text, "Przycisk 21");
        if ( num == VK_BUTTON22         )  strcpy(text, "Przycisk 22");
        if ( num == VK_BUTTON23         )  strcpy(text, "Przycisk 23");
        if ( num == VK_BUTTON24         )  strcpy(text, "Przycisk 24");
        if ( num == VK_BUTTON25         )  strcpy(text, "Przycisk 25");
        if ( num == VK_BUTTON26         )  strcpy(text, "Przycisk 26");
        if ( num == VK_BUTTON27         )  strcpy(text, "Przycisk 27");
        if ( num == VK_BUTTON28         )  strcpy(text, "Przycisk 28");
        if ( num == VK_BUTTON29         )  strcpy(text, "Przycisk 29");
        if ( num == VK_BUTTON30         )  strcpy(text, "Przycisk 30");
        if ( num == VK_BUTTON31         )  strcpy(text, "Przycisk 31");
        if ( num == VK_BUTTON32         )  strcpy(text, "Przycisk 32");
        if ( num == VK_WHEELUP          )  strcpy(text, "KÛ≥ko w gÛrÍ");
        if ( num == VK_WHEELDOWN        )  strcpy(text, "KÛ≥ko w dÛ≥");
    }
#endif

    return ( text[0] != 0 );
}


