/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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


#include "common/restext.h"

#include "common/config.h"

#include "common/global.h"
#include "common/event.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "CBot/resource.h"

#include "object/object.h"
#include "object/robotmain.h"

#include <libintl.h>
#include <SDL_keyboard.h>

const char* stringsText[RT_MAX]         = { nullptr };
const char* stringsEvent[EVENT_STD_MAX] = { nullptr };
const char* stringsObject[OBJECT_MAX]   = { nullptr };
const char* stringsErr[ERR_MAX]         = { nullptr };
const char* stringsCbot[TX_MAX]         = { nullptr };

/* Macro to mark which texts are translatable by gettext
 * It doesn't do anything at compile-time, as all texts represented here are used later
 * in explicit call to gettext(), but it is used by xgettext executable to filter extracted
 * texts from this file.
 */
#define TR(x) x


void InitializeRestext()
{
    stringsText[RT_VERSION_ID]       = COLOBOT_FULLNAME;

    stringsText[RT_DISINFO_TITLE]    = TR("SatCom");
    stringsText[RT_WINDOW_MAXIMIZED] = TR("Maximize");
    stringsText[RT_WINDOW_MINIMIZED] = TR("Minimize");
    stringsText[RT_WINDOW_STANDARD]  = TR("Normal size");
    stringsText[RT_WINDOW_CLOSE]     = TR("Close");

    stringsText[RT_STUDIO_TITLE]     = TR("Program editor");
    stringsText[RT_SCRIPT_NEW]       = TR("New");
    stringsText[RT_NAME_DEFAULT]     = TR("Player");
    stringsText[RT_IO_NEW]           = TR("New ...");
    stringsText[RT_KEY_OR]           = TR(" or ");

    stringsText[RT_TITLE_BASE]       = TR("Colobot: Gold Edition");
    stringsText[RT_TITLE_INIT]       = TR("Colobot: Gold Edition");
    stringsText[RT_TITLE_TRAINER]    = TR("Programming exercises");
    stringsText[RT_TITLE_DEFI]       = TR("Challenges");
    stringsText[RT_TITLE_MISSION]    = TR("Missions");
    stringsText[RT_TITLE_FREE]       = TR("Free game");
    stringsText[RT_TITLE_USER]       = TR("User levels");
    stringsText[RT_TITLE_SETUP]      = TR("Options");
    stringsText[RT_TITLE_NAME]       = TR("Player's name");
    stringsText[RT_TITLE_PERSO]      = TR("Customize your appearance");
    stringsText[RT_TITLE_WRITE]      = TR("Save the current mission");
    stringsText[RT_TITLE_READ]       = TR("Load a saved mission");

    stringsText[RT_PLAY_CHAPt]       = TR(" Chapters:");
    stringsText[RT_PLAY_CHAPd]       = TR(" Chapters:");
    stringsText[RT_PLAY_CHAPm]       = TR(" Planets:");
    stringsText[RT_PLAY_CHAPf]       = TR(" Planets:");
    stringsText[RT_PLAY_CHAPu]       = TR(" User levels:");
    stringsText[RT_PLAY_CHAPte]      = TR(" Chapters:");
    stringsText[RT_PLAY_LISTt]       = TR(" Exercises in the chapter:");
    stringsText[RT_PLAY_LISTd]       = TR(" Challenges in the chapter:");
    stringsText[RT_PLAY_LISTm]       = TR(" Missions on this planet:");
    stringsText[RT_PLAY_LISTf]       = TR(" Free game on this planet:");
    stringsText[RT_PLAY_LISTu]       = TR(" Missions on this level:");
    stringsText[RT_PLAY_RESUME]      = TR(" Summary:");

    stringsText[RT_SETUP_DEVICE]     = TR(" Drivers:");
    stringsText[RT_SETUP_MODE]       = TR(" Resolution:");
    stringsText[RT_SETUP_KEY1]       = TR("1) First click on the key you want to redefine.");
    stringsText[RT_SETUP_KEY2]       = TR("2) Then press the key you want to use instead.");

    stringsText[RT_PERSO_FACE]       = TR("Face type:");
    stringsText[RT_PERSO_GLASSES]    = TR("Eyeglasses:");
    stringsText[RT_PERSO_HAIR]       = TR("Hair color:");
    stringsText[RT_PERSO_COMBI]      = TR("Suit color:");
    stringsText[RT_PERSO_BAND]       = TR("Strip color:");

    stringsText[RT_DIALOG_QUIT]      = TR("Do you want to quit Colobot: Gold Edition?");
    stringsText[RT_DIALOG_TITLE]     = TR("Colobot: Gold Edition");
    stringsText[RT_DIALOG_YESQUIT]   = TR("Quit\\Quit Colobot: Gold Edition");
    stringsText[RT_DIALOG_ABORT]     = TR("Quit the mission?");
    stringsText[RT_DIALOG_YES]       = TR("Abort\\Abort the current mission");
    stringsText[RT_DIALOG_NO]        = TR("Continue\\Continue the current mission");
    stringsText[RT_DIALOG_NOQUIT]    = TR("Continue\\Continue the game");
    stringsText[RT_DIALOG_DELOBJ]    = TR("Do you really want to destroy the selected building?");
    stringsText[RT_DIALOG_DELGAME]   = TR("Do you want to delete %s's saved games? ");
    stringsText[RT_DIALOG_YESDEL]    = TR("Delete");
    stringsText[RT_DIALOG_NODEL]     = TR("Cancel");
    stringsText[RT_DIALOG_LOADING]   = TR("LOADING");

    stringsText[RT_STUDIO_LISTTT]    = TR("Keyword help(\\key cbot;)");
    stringsText[RT_STUDIO_COMPOK]    = TR("Compilation ok (0 errors)");
    stringsText[RT_STUDIO_PROGSTOP]  = TR("Program finished");

    stringsText[RT_SATCOM_LIST]      = TR("\\b;List of objects\n");
    stringsText[RT_SATCOM_BOT]       = TR("\\b;Robots\n");
    stringsText[RT_SATCOM_BUILDING]  = TR("\\b;Buildings\n");
    stringsText[RT_SATCOM_FRET]      = TR("\\b;Moveable objects\n");
    stringsText[RT_SATCOM_ALIEN]     = TR("\\b;Aliens\n");
    stringsText[RT_SATCOM_NULL]      = TR("\\c; (none)\\n;\n");
    stringsText[RT_SATCOM_ERROR1]    = TR("\\b;Error\n");
    stringsText[RT_SATCOM_ERROR2]    = TR("The list is only available if a \\l;radar station\\u object\\radar; is working.\n");

    stringsText[RT_IO_OPEN]          = TR("Open");
    stringsText[RT_IO_SAVE]          = TR("Save");
    stringsText[RT_IO_LIST]          = TR("Folder: %s");
    stringsText[RT_IO_NAME]          = TR("Name:");
    stringsText[RT_IO_DIR]           = TR("Folder:");
    stringsText[RT_IO_PRIVATE]       = TR("Private\\Private folder");
    stringsText[RT_IO_PUBLIC]        = TR("Public\\Common folder");

    stringsText[RT_GENERIC_DEV1]     = TR("Developed by :");
    stringsText[RT_GENERIC_DEV2]     = TR("www.epsitec.com");
    stringsText[RT_GENERIC_EDIT1]    = TR(" ");
    stringsText[RT_GENERIC_EDIT2]    = TR(" ");

    stringsText[RT_INTERFACE_REC]    = TR("Recorder");



    stringsEvent[EVENT_BUTTON_OK]           = TR("OK");
    stringsEvent[EVENT_BUTTON_CANCEL]       = TR("Cancel");
    stringsEvent[EVENT_BUTTON_NEXT]         = TR("Next");
    stringsEvent[EVENT_BUTTON_PREV]         = TR("Previous");

    stringsEvent[EVENT_DIALOG_OK]           = TR("OK");
    stringsEvent[EVENT_DIALOG_CANCEL]       = TR("Cancel");

    stringsEvent[EVENT_INTERFACE_TRAINER]   = TR("Exercises\\Programming exercises");
    stringsEvent[EVENT_INTERFACE_DEFI]      = TR("Challenges\\Programming challenges");
    stringsEvent[EVENT_INTERFACE_MISSION]   = TR("Missions\\Select mission");
    stringsEvent[EVENT_INTERFACE_FREE]      = TR("Free game\\Free game without a specific goal");
    stringsEvent[EVENT_INTERFACE_USER]      = TR("User\\User levels");
    stringsEvent[EVENT_INTERFACE_NAME]      = TR("Change player\\Change player");
    stringsEvent[EVENT_INTERFACE_SETUP]     = TR("Options\\Preferences");
    stringsEvent[EVENT_INTERFACE_AGAIN]     = TR("Restart\\Restart the mission from the beginning");
    stringsEvent[EVENT_INTERFACE_WRITE]     = TR("Save\\Save the current mission ");
    stringsEvent[EVENT_INTERFACE_READ]      = TR("Load\\Load a saved mission");
    stringsEvent[EVENT_INTERFACE_ABORT]     = TR("\\Return to Colobot: Gold Edition");
    stringsEvent[EVENT_INTERFACE_QUIT]      = TR("Quit\\Quit Colobot: Gold Edition");
    stringsEvent[EVENT_INTERFACE_BACK]      = TR("<<  Back  \\Back to the previous screen");
    stringsEvent[EVENT_INTERFACE_PLAY]      = TR("Play\\Start mission!");
    stringsEvent[EVENT_INTERFACE_SETUPd]    = TR("Device\\Driver and resolution settings");
    stringsEvent[EVENT_INTERFACE_SETUPg]    = TR("Graphics\\Graphics settings");
    stringsEvent[EVENT_INTERFACE_SETUPp]    = TR("Game\\Game settings");
    stringsEvent[EVENT_INTERFACE_SETUPc]    = TR("Controls\\Keyboard, joystick and mouse settings");
    stringsEvent[EVENT_INTERFACE_SETUPs]    = TR("Sound\\Music and game sound volume");
    stringsEvent[EVENT_INTERFACE_DEVICE]    = TR("Unit");
    stringsEvent[EVENT_INTERFACE_RESOL]     = TR("Resolution");
    stringsEvent[EVENT_INTERFACE_FULL]      = TR("Full screen\\Full screen or window mode");
    stringsEvent[EVENT_INTERFACE_APPLY]     = TR("Apply changes\\Activates the changed settings");

    stringsEvent[EVENT_INTERFACE_TOTO]      = TR("Robbie\\Your assistant");
    stringsEvent[EVENT_INTERFACE_SHADOW]    = TR("Shadows\\Shadows on the ground");
    stringsEvent[EVENT_INTERFACE_GROUND]    = TR("Marks on the ground\\Marks on the ground");
    stringsEvent[EVENT_INTERFACE_DIRTY]     = TR("Dust\\Dust and dirt on bots and buildings");
    stringsEvent[EVENT_INTERFACE_FOG]       = TR("Fog\\Fog");
    stringsEvent[EVENT_INTERFACE_LENS]      = TR("Sunbeams\\Sunbeams in the sky");
    stringsEvent[EVENT_INTERFACE_SKY]       = TR("Sky\\Clouds and nebulae");
    stringsEvent[EVENT_INTERFACE_PLANET]    = TR("Planets and stars\\Astronomical objects in the sky");
    stringsEvent[EVENT_INTERFACE_LIGHT]     = TR("Dynamic lighting\\Mobile light sources");
    stringsEvent[EVENT_INTERFACE_PARTI]     = TR("Number of particles\\Explosions, dust, reflections, etc.");
    stringsEvent[EVENT_INTERFACE_CLIP]      = TR("Depth of field\\Maximum visibility");
    stringsEvent[EVENT_INTERFACE_DETAIL]    = TR("Details\\Visual quality of 3D objects");
    stringsEvent[EVENT_INTERFACE_TEXTURE]   = TR("Textures\\Quality of textures ");
    stringsEvent[EVENT_INTERFACE_GADGET]    = TR("Num of decorative objects\\Number of purely ornamental objects");
    stringsEvent[EVENT_INTERFACE_RAIN]      = TR("Particles in the interface\\Steam clouds and sparks in the interface");
    stringsEvent[EVENT_INTERFACE_GLINT]     = TR("Reflections on the buttons \\Shiny buttons");
    stringsEvent[EVENT_INTERFACE_TOOLTIP]   = TR("Help balloons\\Explain the function of the buttons");
    stringsEvent[EVENT_INTERFACE_MOVIES]    = TR("Film sequences\\Films before and after the missions");
    stringsEvent[EVENT_INTERFACE_NICERST]   = TR("Exit film\\Film at the exit of exercises");
    stringsEvent[EVENT_INTERFACE_HIMSELF]   = TR("Friendly fire\\Your shooting can damage your own objects ");
    stringsEvent[EVENT_INTERFACE_SCROLL]    = TR("Scrolling\\Scrolling when the mouse touches right or left border");
    stringsEvent[EVENT_INTERFACE_INVERTX]   = TR("Mouse inversion X\\Inversion of the scrolling direction on the X axis");
    stringsEvent[EVENT_INTERFACE_INVERTY]   = TR("Mouse inversion Y\\Inversion of the scrolling direction on the Y axis");
    stringsEvent[EVENT_INTERFACE_EFFECT]    = TR("Quake at explosions\\The screen shakes at explosions");
    stringsEvent[EVENT_INTERFACE_MOUSE]     = TR("Mouse shadow\\Gives the mouse a shadow");
    stringsEvent[EVENT_INTERFACE_EDITMODE]  = TR("Automatic indent\\When program editing");
    stringsEvent[EVENT_INTERFACE_EDITVALUE] = TR("Big indent\\Indent 2 or 4 spaces per level defined by braces");
    stringsEvent[EVENT_INTERFACE_SOLUCE4]   = TR("Access to solutions\\Show program \"4: Solution\" in the exercises");
    stringsEvent[EVENT_INTERFACE_BLOOD]     = TR("Blood\\Display blood when the astronaut or the alien queen is hit");

    stringsEvent[EVENT_INTERFACE_KDEF]      = TR("Standard controls\\Standard key functions");
    stringsEvent[EVENT_INTERFACE_KLEFT]     = TR("Turn left\\turns the bot to the left");
    stringsEvent[EVENT_INTERFACE_KRIGHT]    = TR("Turn right\\turns the bot to the right");
    stringsEvent[EVENT_INTERFACE_KUP]       = TR("Forward\\Moves forward");
    stringsEvent[EVENT_INTERFACE_KDOWN]     = TR("Backward\\Moves backward");
    stringsEvent[EVENT_INTERFACE_KGUP]      = TR("Climb\\Increases the power of the jet");
    stringsEvent[EVENT_INTERFACE_KGDOWN]    = TR("Descend\\Reduces the power of the jet");
    stringsEvent[EVENT_INTERFACE_KCAMERA]   = TR("Change camera\\Switches between onboard camera and following camera");
    stringsEvent[EVENT_INTERFACE_KDESEL]    = TR("Previous object\\Selects the previous object");
    stringsEvent[EVENT_INTERFACE_KACTION]   = TR("Standard action\\Standard action of the bot (take/grab, shoot, sniff, etc)");
    stringsEvent[EVENT_INTERFACE_KNEAR]     = TR("Camera closer\\Moves the camera forward");
    stringsEvent[EVENT_INTERFACE_KAWAY]     = TR("Camera back\\Moves the camera backward");
    stringsEvent[EVENT_INTERFACE_KNEXT]     = TR("Next object\\Selects the next object");
    stringsEvent[EVENT_INTERFACE_KHUMAN]    = TR("Select the astronaut\\Selects the astronaut");
    stringsEvent[EVENT_INTERFACE_KQUIT]     = TR("Quit\\Quit the current mission or exercise");
    stringsEvent[EVENT_INTERFACE_KHELP]     = TR("Instructions\\Shows the instructions for the current mission");
    stringsEvent[EVENT_INTERFACE_KPROG]     = TR("Programming help\\Gives more detailed help with programming");
    stringsEvent[EVENT_INTERFACE_KCBOT]     = TR("Key word help\\More detailed help about key words");
    stringsEvent[EVENT_INTERFACE_KVISIT]    = TR("Origin of last message\\Shows where the last message was sent from");
    stringsEvent[EVENT_INTERFACE_KSPEED10]  = TR("Speed 1.0x\\Normal speed");
    stringsEvent[EVENT_INTERFACE_KSPEED15]  = TR("Speed 1.5x\\1.5 times faster");
    stringsEvent[EVENT_INTERFACE_KSPEED20]  = TR("Speed 2.0x\\Double speed");
    stringsEvent[EVENT_INTERFACE_KSPEED30]  = TR("Speed 3.0x\\Three times faster");

    stringsEvent[EVENT_INTERFACE_VOLSOUND]  = TR("Sound effects:\\Volume of engines, voice, shooting, etc.");
    stringsEvent[EVENT_INTERFACE_VOLMUSIC]  = TR("Background sound :\\Volume of audio tracks on the CD");
    stringsEvent[EVENT_INTERFACE_SOUND3D]   = TR("3D sound\\3D positioning of the sound");

    stringsEvent[EVENT_INTERFACE_MIN]       = TR("Lowest\\Minimum graphic quality (highest frame rate)");
    stringsEvent[EVENT_INTERFACE_NORM]      = TR("Normal\\Normal graphic quality");
    stringsEvent[EVENT_INTERFACE_MAX]       = TR("Highest\\Highest graphic quality (lowest frame rate)");

    stringsEvent[EVENT_INTERFACE_SILENT]    = TR("Mute\\No sound");
    stringsEvent[EVENT_INTERFACE_NOISY]     = TR("Normal\\Normal sound volume");

    stringsEvent[EVENT_INTERFACE_JOYSTICK]  = TR("Use a joystick\\Joystick or keyboard");
    stringsEvent[EVENT_INTERFACE_SOLUCE]    = TR("Access to solution\\Shows the solution (detailed instructions for missions)");

    stringsEvent[EVENT_INTERFACE_NEDIT]     = TR("\\New player name");
    stringsEvent[EVENT_INTERFACE_NOK]       = TR("OK\\Choose the selected player");
    stringsEvent[EVENT_INTERFACE_NCANCEL]   = TR("Cancel\\Keep current player name");
    stringsEvent[EVENT_INTERFACE_NDELETE]   = TR("Delete player\\Deletes the player from the list");
    stringsEvent[EVENT_INTERFACE_NLABEL]    = TR("Player name");

    stringsEvent[EVENT_INTERFACE_IOWRITE]   = TR("Save\\Saves the current mission");
    stringsEvent[EVENT_INTERFACE_IOREAD]    = TR("Load\\Loads the selected mission");
    stringsEvent[EVENT_INTERFACE_IOLIST]    = TR("List of saved missions");
    stringsEvent[EVENT_INTERFACE_IOLABEL]   = TR("Filename:");
    stringsEvent[EVENT_INTERFACE_IONAME]    = TR("Mission name");
    stringsEvent[EVENT_INTERFACE_IOIMAGE]   = TR("Photography");
    stringsEvent[EVENT_INTERFACE_IODELETE]  = TR("Delete\\Deletes the selected file");

    stringsEvent[EVENT_INTERFACE_PERSO]     = TR("Appearance\\Choose your appearance");
    stringsEvent[EVENT_INTERFACE_POK]       = TR("OK");
    stringsEvent[EVENT_INTERFACE_PCANCEL]   = TR("Cancel");
    stringsEvent[EVENT_INTERFACE_PDEF]      = TR("Standard\\Standard appearance settings");
    stringsEvent[EVENT_INTERFACE_PHEAD]     = TR("Head\\Face and hair");
    stringsEvent[EVENT_INTERFACE_PBODY]     = TR("Suit\\Astronaut suit");
    stringsEvent[EVENT_INTERFACE_PLROT]     = TR("\\Turn left");
    stringsEvent[EVENT_INTERFACE_PRROT]     = TR("\\Turn right");
    stringsEvent[EVENT_INTERFACE_PCRa]      = TR("Red");
    stringsEvent[EVENT_INTERFACE_PCGa]      = TR("Green");
    stringsEvent[EVENT_INTERFACE_PCBa]      = TR("Blue");
    stringsEvent[EVENT_INTERFACE_PCRb]      = TR("Red");
    stringsEvent[EVENT_INTERFACE_PCGb]      = TR("Green");
    stringsEvent[EVENT_INTERFACE_PCBb]      = TR("Blue");
    stringsEvent[EVENT_INTERFACE_PFACE1]    = TR("\\Face 1");
    stringsEvent[EVENT_INTERFACE_PFACE2]    = TR("\\Face 4");
    stringsEvent[EVENT_INTERFACE_PFACE3]    = TR("\\Face 3");
    stringsEvent[EVENT_INTERFACE_PFACE4]    = TR("\\Face 2");
    stringsEvent[EVENT_INTERFACE_PGLASS0]   = TR("\\No eyeglasses");
    stringsEvent[EVENT_INTERFACE_PGLASS1]   = TR("\\Eyeglasses 1");
    stringsEvent[EVENT_INTERFACE_PGLASS2]   = TR("\\Eyeglasses 2");
    stringsEvent[EVENT_INTERFACE_PGLASS3]   = TR("\\Eyeglasses 3");
    stringsEvent[EVENT_INTERFACE_PGLASS4]   = TR("\\Eyeglasses 4");
    stringsEvent[EVENT_INTERFACE_PGLASS5]   = TR("\\Eyeglasses 5");

    stringsEvent[EVENT_OBJECT_DESELECT]     = TR("Previous selection (\\key desel;)");
    stringsEvent[EVENT_OBJECT_LEFT]         = TR("Turn left (\\key left;)");
    stringsEvent[EVENT_OBJECT_RIGHT]        = TR("Turn right (\\key right;)");
    stringsEvent[EVENT_OBJECT_UP]           = TR("Forward (\\key up;)");
    stringsEvent[EVENT_OBJECT_DOWN]         = TR("Backward (\\key down;)");
    stringsEvent[EVENT_OBJECT_GASUP]        = TR("Up (\\key gup;)");
    stringsEvent[EVENT_OBJECT_GASDOWN]      = TR("Down (\\key gdown;)");
    stringsEvent[EVENT_OBJECT_HTAKE]        = TR("Grab or drop (\\key action;)");
    stringsEvent[EVENT_OBJECT_MTAKE]        = TR("Grab or drop (\\key action;)");
    stringsEvent[EVENT_OBJECT_MFRONT]       = TR("..in front");
    stringsEvent[EVENT_OBJECT_MBACK]        = TR("..behind");
    stringsEvent[EVENT_OBJECT_MPOWER]       = TR("..power cell");
    stringsEvent[EVENT_OBJECT_BHELP]        = TR("Instructions for the mission (\\key help;)");
    stringsEvent[EVENT_OBJECT_BTAKEOFF]     = TR("Take off to finish the mission");
    stringsEvent[EVENT_OBJECT_BDESTROY]     = TR("Destroy");
    stringsEvent[EVENT_OBJECT_BDERRICK]     = TR("Build a derrick");
    stringsEvent[EVENT_OBJECT_BSTATION]     = TR("Build a power station");
    stringsEvent[EVENT_OBJECT_BFACTORY]     = TR("Build a bot factory");
    stringsEvent[EVENT_OBJECT_BREPAIR]      = TR("Build a repair center");
    stringsEvent[EVENT_OBJECT_BCONVERT]     = TR("Build a converter");
    stringsEvent[EVENT_OBJECT_BTOWER]       = TR("Build a defense tower");
    stringsEvent[EVENT_OBJECT_BRESEARCH]    = TR("Build a research center");
    stringsEvent[EVENT_OBJECT_BRADAR]       = TR("Build a radar station");
    stringsEvent[EVENT_OBJECT_BENERGY]      = TR("Build a power cell factory");
    stringsEvent[EVENT_OBJECT_BLABO]        = TR("Build an autolab");
    stringsEvent[EVENT_OBJECT_BNUCLEAR]     = TR("Build a nuclear power plant");
    stringsEvent[EVENT_OBJECT_BPARA]        = TR("Build a lightning conductor");
    stringsEvent[EVENT_OBJECT_BINFO]        = TR("Build a exchange post");
    stringsEvent[EVENT_OBJECT_BDESTROYER]   = TR("Build a destroyer");
    stringsEvent[EVENT_OBJECT_GFLAT]        = TR("Show if the ground is flat");
    stringsEvent[EVENT_OBJECT_FCREATE]      = TR("Plant a flag");
    stringsEvent[EVENT_OBJECT_FDELETE]      = TR("Remove a flag");
    stringsEvent[EVENT_OBJECT_FCOLORb]      = TR("\\Blue flags");
    stringsEvent[EVENT_OBJECT_FCOLORr]      = TR("\\Red flags");
    stringsEvent[EVENT_OBJECT_FCOLORg]      = TR("\\Green flags");
    stringsEvent[EVENT_OBJECT_FCOLORy]      = TR("\\Yellow flags");
    stringsEvent[EVENT_OBJECT_FCOLORv]      = TR("\\Violet flags");
    stringsEvent[EVENT_OBJECT_FACTORYfa]    = TR("Build a winged grabber");
    stringsEvent[EVENT_OBJECT_FACTORYta]    = TR("Build a tracked grabber");
    stringsEvent[EVENT_OBJECT_FACTORYwa]    = TR("Build a wheeled grabber");
    stringsEvent[EVENT_OBJECT_FACTORYia]    = TR("Build a legged grabber");
    stringsEvent[EVENT_OBJECT_FACTORYfc]    = TR("Build a winged shooter");
    stringsEvent[EVENT_OBJECT_FACTORYtc]    = TR("Build a tracked shooter");
    stringsEvent[EVENT_OBJECT_FACTORYwc]    = TR("Build a wheeled shooter");
    stringsEvent[EVENT_OBJECT_FACTORYic]    = TR("Build a legged shooter");
    stringsEvent[EVENT_OBJECT_FACTORYfi]    = TR("Build a winged orga shooter");
    stringsEvent[EVENT_OBJECT_FACTORYti]    = TR("Build a tracked orga shooter");
    stringsEvent[EVENT_OBJECT_FACTORYwi]    = TR("Build a wheeled orga shooter");
    stringsEvent[EVENT_OBJECT_FACTORYii]    = TR("Build a legged orga shooter");
    stringsEvent[EVENT_OBJECT_FACTORYfs]    = TR("Build a winged sniffer");
    stringsEvent[EVENT_OBJECT_FACTORYts]    = TR("Build a tracked sniffer");
    stringsEvent[EVENT_OBJECT_FACTORYws]    = TR("Build a wheeled sniffer");
    stringsEvent[EVENT_OBJECT_FACTORYis]    = TR("Build a legged sniffer");
    stringsEvent[EVENT_OBJECT_FACTORYrt]    = TR("Build a thumper");
    stringsEvent[EVENT_OBJECT_FACTORYrc]    = TR("Build a phazer shooter");
    stringsEvent[EVENT_OBJECT_FACTORYrr]    = TR("Build a recycler");
    stringsEvent[EVENT_OBJECT_FACTORYrs]    = TR("Build a shielder");
    stringsEvent[EVENT_OBJECT_FACTORYsa]    = TR("Build a subber");
    stringsEvent[EVENT_OBJECT_RTANK]        = TR("Run research program for tracked bots");
    stringsEvent[EVENT_OBJECT_RFLY]         = TR("Run research program for winged bots");
    stringsEvent[EVENT_OBJECT_RTHUMP]       = TR("Run research program for thumper");
    stringsEvent[EVENT_OBJECT_RCANON]       = TR("Run research program for shooter");
    stringsEvent[EVENT_OBJECT_RTOWER]       = TR("Run research program for defense tower");
    stringsEvent[EVENT_OBJECT_RPHAZER]      = TR("Run research program for phazer shooter");
    stringsEvent[EVENT_OBJECT_RSHIELD]      = TR("Run research program for shielder");
    stringsEvent[EVENT_OBJECT_RATOMIC]      = TR("Run research program for nuclear power");
    stringsEvent[EVENT_OBJECT_RiPAW]        = TR("Run research program for legged bots");
    stringsEvent[EVENT_OBJECT_RiGUN]        = TR("Run research program for orga shooter");
    stringsEvent[EVENT_OBJECT_RESET]        = TR("Return to start");
    stringsEvent[EVENT_OBJECT_SEARCH]       = TR("Sniff (\\key action;)");
    stringsEvent[EVENT_OBJECT_TERRAFORM]    = TR("Thump (\\key action;)");
    stringsEvent[EVENT_OBJECT_FIRE]         = TR("Shoot (\\key action;)");
    stringsEvent[EVENT_OBJECT_SPIDEREXPLO]  = TR("Explode (\\key action;)");
    stringsEvent[EVENT_OBJECT_RECOVER]      = TR("Recycle (\\key action;)");
    stringsEvent[EVENT_OBJECT_BEGSHIELD]    = TR("Extend shield (\\key action;)");
    stringsEvent[EVENT_OBJECT_ENDSHIELD]    = TR("Withdraw shield (\\key action;)");
    stringsEvent[EVENT_OBJECT_DIMSHIELD]    = TR("Shield radius");
    stringsEvent[EVENT_OBJECT_PROGRUN]      = TR("Execute the selected program");
    stringsEvent[EVENT_OBJECT_PROGEDIT]     = TR("Edit the selected program");
    stringsEvent[EVENT_OBJECT_INFOOK]       = TR("\\SatCom on standby");
    stringsEvent[EVENT_OBJECT_DELETE]       = TR("Destroy the building");
    stringsEvent[EVENT_OBJECT_GENERGY]      = TR("Energy level");
    stringsEvent[EVENT_OBJECT_GSHIELD]      = TR("Shield level");
    stringsEvent[EVENT_OBJECT_GRANGE]       = TR("Jet temperature");
    stringsEvent[EVENT_OBJECT_GPROGRESS]    = TR("Still working ...");
    stringsEvent[EVENT_OBJECT_GRADAR]       = TR("Number of insects detected");
    stringsEvent[EVENT_OBJECT_GINFO]        = TR("Transmitted information");
    stringsEvent[EVENT_OBJECT_COMPASS]      = TR("Compass");
    stringsEvent[EVENT_OBJECT_MAPZOOM]      = TR("Zoom mini-map");
    stringsEvent[EVENT_OBJECT_CAMERA]       = TR("Camera (\\key camera;)");
    stringsEvent[EVENT_OBJECT_CAMERAleft]   = TR("Camera to left");
    stringsEvent[EVENT_OBJECT_CAMERAright]  = TR("Camera to right");
    stringsEvent[EVENT_OBJECT_CAMERAnear]   = TR("Camera nearest");
    stringsEvent[EVENT_OBJECT_CAMERAaway]   = TR("Camera awayest");
    stringsEvent[EVENT_OBJECT_HELP]         = TR("Help about selected object");
    stringsEvent[EVENT_OBJECT_SOLUCE]       = TR("Show the solution");
    stringsEvent[EVENT_OBJECT_SHORTCUT00]   = TR("Switch bots <-> buildings");
    stringsEvent[EVENT_OBJECT_LIMIT]        = TR("Show the range");
    stringsEvent[EVENT_OBJECT_PEN0]         = TR("\\Raise the pencil");
    stringsEvent[EVENT_OBJECT_PEN1]         = TR("\\Use the black pencil");
    stringsEvent[EVENT_OBJECT_PEN2]         = TR("\\Use the yellow pencil");
    stringsEvent[EVENT_OBJECT_PEN3]         = TR("\\Use the orange pencil");
    stringsEvent[EVENT_OBJECT_PEN4]         = TR("\\Use the red pencil");
    stringsEvent[EVENT_OBJECT_PEN5]         = TR("\\Use the purple pencil");
    stringsEvent[EVENT_OBJECT_PEN6]         = TR("\\Use the blue pencil");
    stringsEvent[EVENT_OBJECT_PEN7]         = TR("\\Use the green pencil");
    stringsEvent[EVENT_OBJECT_PEN8]         = TR("\\Use the brown pencil");
    stringsEvent[EVENT_OBJECT_REC]          = TR("\\Start recording");
    stringsEvent[EVENT_OBJECT_STOP]         = TR("\\Stop recording");
    stringsEvent[EVENT_DT_VISIT0]           = TR("Show the place");
    stringsEvent[EVENT_DT_VISIT1]           = TR("Show the place");
    stringsEvent[EVENT_DT_VISIT2]           = TR("Show the place");
    stringsEvent[EVENT_DT_VISIT3]           = TR("Show the place");
    stringsEvent[EVENT_DT_VISIT4]           = TR("Show the place");
    stringsEvent[EVENT_DT_END]              = TR("Continue");
    stringsEvent[EVENT_CMD]                 = TR("Command line");
    stringsEvent[EVENT_SPEED]               = TR("Game speed");

    stringsEvent[EVENT_HYPER_PREV]          = TR("Back");
    stringsEvent[EVENT_HYPER_NEXT]          = TR("Forward");
    stringsEvent[EVENT_HYPER_HOME]          = TR("Home");
    stringsEvent[EVENT_HYPER_COPY]          = TR("Copy");
    stringsEvent[EVENT_HYPER_SIZE1]         = TR("Size 1");
    stringsEvent[EVENT_HYPER_SIZE2]         = TR("Size 2");
    stringsEvent[EVENT_HYPER_SIZE3]         = TR("Size 3");
    stringsEvent[EVENT_HYPER_SIZE4]         = TR("Size 4");
    stringsEvent[EVENT_HYPER_SIZE5]         = TR("Size 5");
    stringsEvent[EVENT_SATCOM_HUSTON]       = TR("Instructions from Houston");
    stringsEvent[EVENT_SATCOM_SAT]          = TR("Satellite report");
    stringsEvent[EVENT_SATCOM_LOADING]      = TR("Programs dispatched by Houston");
    stringsEvent[EVENT_SATCOM_OBJECT]       = TR("List of objects");
    stringsEvent[EVENT_SATCOM_PROG]         = TR("Programming help");
    stringsEvent[EVENT_SATCOM_SOLUCE]       = TR("Solution");

    stringsEvent[EVENT_STUDIO_OK]           = TR("OK\\Close program editor and return to game");
    stringsEvent[EVENT_STUDIO_CANCEL]       = TR("Cancel\\Cancel all changes");
    stringsEvent[EVENT_STUDIO_NEW]          = TR("New");
    stringsEvent[EVENT_STUDIO_OPEN]         = TR("Open (Ctrl+o)");
    stringsEvent[EVENT_STUDIO_SAVE]         = TR("Save (Ctrl+s)");
    stringsEvent[EVENT_STUDIO_UNDO]         = TR("Undo (Ctrl+z)");
    stringsEvent[EVENT_STUDIO_CUT]          = TR("Cut (Ctrl+x)");
    stringsEvent[EVENT_STUDIO_COPY]         = TR("Copy (Ctrl+c)");
    stringsEvent[EVENT_STUDIO_PASTE]        = TR("Paste (Ctrl+v)");
    stringsEvent[EVENT_STUDIO_SIZE]         = TR("Font size");
    stringsEvent[EVENT_STUDIO_TOOL]         = TR("Instructions (\\key help;)");
    stringsEvent[EVENT_STUDIO_HELP]         = TR("Programming help  (\\key prog;)");
    stringsEvent[EVENT_STUDIO_COMPILE]      = TR("Compile");
    stringsEvent[EVENT_STUDIO_RUN]          = TR("Execute/stop");
    stringsEvent[EVENT_STUDIO_REALTIME]     = TR("Pause/continue");
    stringsEvent[EVENT_STUDIO_STEP]         = TR("One step");



    stringsObject[OBJECT_PORTICO]      = TR("Gantry crane");
    stringsObject[OBJECT_BASE]         = TR("Spaceship");
    stringsObject[OBJECT_DERRICK]      = TR("Derrick");
    stringsObject[OBJECT_FACTORY]      = TR("Bot factory");
    stringsObject[OBJECT_REPAIR]       = TR("Repair center");
    stringsObject[OBJECT_DESTROYER]    = TR("Destroyer");
    stringsObject[OBJECT_STATION]      = TR("Power station");
    stringsObject[OBJECT_CONVERT]      = TR("Converts ore to titanium");
    stringsObject[OBJECT_TOWER]        = TR("Defense tower");
    stringsObject[OBJECT_NEST]         = TR("Nest");
    stringsObject[OBJECT_RESEARCH]     = TR("Research center");
    stringsObject[OBJECT_RADAR]        = TR("Radar station");
    stringsObject[OBJECT_INFO]         = TR("Information exchange post");
    stringsObject[OBJECT_ENERGY]       = TR("Power cell factory");
    stringsObject[OBJECT_LABO]         = TR("Autolab");
    stringsObject[OBJECT_NUCLEAR]      = TR("Nuclear power station");
    stringsObject[OBJECT_PARA]         = TR("Lightning conductor");
    stringsObject[OBJECT_SAFE]         = TR("Vault");
    stringsObject[OBJECT_HUSTON]       = TR("Houston Mission Control");
    stringsObject[OBJECT_TARGET1]      = TR("Target");
    stringsObject[OBJECT_TARGET2]      = TR("Target");
    stringsObject[OBJECT_START]        = TR("Start");
    stringsObject[OBJECT_END]          = TR("Finish");
    stringsObject[OBJECT_STONE]        = TR("Titanium ore");
    stringsObject[OBJECT_URANIUM]      = TR("Uranium ore");
    stringsObject[OBJECT_BULLET]       = TR("Organic matter");
    stringsObject[OBJECT_METAL]        = TR("Titanium");
    stringsObject[OBJECT_POWER]        = TR("Power cell");
    stringsObject[OBJECT_ATOMIC]       = TR("Nuclear power cell");
    stringsObject[OBJECT_BBOX]         = TR("Black box");
    stringsObject[OBJECT_KEYa]         = TR("Key A");
    stringsObject[OBJECT_KEYb]         = TR("Key B");
    stringsObject[OBJECT_KEYc]         = TR("Key C");
    stringsObject[OBJECT_KEYd]         = TR("Key D");
    stringsObject[OBJECT_TNT]          = TR("Explosive");
    stringsObject[OBJECT_BOMB]         = TR("Fixed mine");
    stringsObject[OBJECT_BAG]          = TR("Survival kit");
    stringsObject[OBJECT_WAYPOINT]     = TR("Checkpoint");
    stringsObject[OBJECT_FLAGb]        = TR("Blue flag");
    stringsObject[OBJECT_FLAGr]        = TR("Red flag");
    stringsObject[OBJECT_FLAGg]        = TR("Green flag");
    stringsObject[OBJECT_FLAGy]        = TR("Yellow flag");
    stringsObject[OBJECT_FLAGv]        = TR("Violet flag");
    stringsObject[OBJECT_MARKPOWER]    = TR("Energy deposit (site for power station)");
    stringsObject[OBJECT_MARKURANIUM]  = TR("Uranium deposit (site for derrick)");
    stringsObject[OBJECT_MARKKEYa]     = TR("Found key A (site for derrick)");
    stringsObject[OBJECT_MARKKEYb]     = TR("Found key B (site for derrick)");
    stringsObject[OBJECT_MARKKEYc]     = TR("Found key C (site for derrick)");
    stringsObject[OBJECT_MARKKEYd]     = TR("Found key D (site for derrick)");
    stringsObject[OBJECT_MARKSTONE]    = TR("Titanium deposit (site for derrick)");
    stringsObject[OBJECT_MOBILEft]     = TR("Practice bot");
    stringsObject[OBJECT_MOBILEtt]     = TR("Practice bot");
    stringsObject[OBJECT_MOBILEwt]     = TR("Practice bot");
    stringsObject[OBJECT_MOBILEit]     = TR("Practice bot");
    stringsObject[OBJECT_MOBILEfa]     = TR("Winged grabber");
    stringsObject[OBJECT_MOBILEta]     = TR("Tracked grabber");
    stringsObject[OBJECT_MOBILEwa]     = TR("Wheeled grabber");
    stringsObject[OBJECT_MOBILEia]     = TR("Legged grabber");
    stringsObject[OBJECT_MOBILEfc]     = TR("Winged shooter");
    stringsObject[OBJECT_MOBILEtc]     = TR("Tracked shooter");
    stringsObject[OBJECT_MOBILEwc]     = TR("Wheeled shooter");
    stringsObject[OBJECT_MOBILEic]     = TR("Legged shooter");
    stringsObject[OBJECT_MOBILEfi]     = TR("Winged orga shooter");
    stringsObject[OBJECT_MOBILEti]     = TR("Tracked orga shooter");
    stringsObject[OBJECT_MOBILEwi]     = TR("Wheeled orga shooter");
    stringsObject[OBJECT_MOBILEii]     = TR("Legged orga shooter");
    stringsObject[OBJECT_MOBILEfs]     = TR("Winged sniffer");
    stringsObject[OBJECT_MOBILEts]     = TR("Tracked sniffer");
    stringsObject[OBJECT_MOBILEws]     = TR("Wheeled sniffer");
    stringsObject[OBJECT_MOBILEis]     = TR("Legged sniffer");
    stringsObject[OBJECT_MOBILErt]     = TR("Thumper");
    stringsObject[OBJECT_MOBILErc]     = TR("Phazer shooter");
    stringsObject[OBJECT_MOBILErr]     = TR("Recycler");
    stringsObject[OBJECT_MOBILErs]     = TR("Shielder");
    stringsObject[OBJECT_MOBILEsa]     = TR("Subber");
    stringsObject[OBJECT_MOBILEtg]     = TR("Target bot");
    stringsObject[OBJECT_MOBILEdr]     = TR("Drawer bot");
    stringsObject[OBJECT_TECH]         = TR("Engineer");
    stringsObject[OBJECT_TOTO]         = TR("Robbie");
    stringsObject[OBJECT_MOTHER]       = TR("Alien Queen");
    stringsObject[OBJECT_ANT]          = TR("Ant");
    stringsObject[OBJECT_SPIDER]       = TR("Spider");
    stringsObject[OBJECT_BEE]          = TR("Wasp");
    stringsObject[OBJECT_WORM]         = TR("Worm");
    stringsObject[OBJECT_EGG]          = TR("Egg");
    stringsObject[OBJECT_RUINmobilew1] = TR("Wreckage");
    stringsObject[OBJECT_RUINmobilew2] = TR("Wreckage");
    stringsObject[OBJECT_RUINmobilet1] = TR("Wreckage");
    stringsObject[OBJECT_RUINmobilet2] = TR("Wreckage");
    stringsObject[OBJECT_RUINmobiler1] = TR("Wreckage");
    stringsObject[OBJECT_RUINmobiler2] = TR("Wreckage");
    stringsObject[OBJECT_RUINfactory]  = TR("Ruin");
    stringsObject[OBJECT_RUINdoor]     = TR("Ruin");
    stringsObject[OBJECT_RUINsupport]  = TR("Waste");
    stringsObject[OBJECT_RUINradar]    = TR("Ruin");
    stringsObject[OBJECT_RUINconvert]  = TR("Ruin");
    stringsObject[OBJECT_RUINbase]     = TR("Spaceship ruin");
    stringsObject[OBJECT_RUINhead]     = TR("Spaceship ruin");
    stringsObject[OBJECT_APOLLO1]      = TR("Remains of Apollo mission");
    stringsObject[OBJECT_APOLLO3]      = TR("Remains of Apollo mission");
    stringsObject[OBJECT_APOLLO4]      = TR("Remains of Apollo mission");
    stringsObject[OBJECT_APOLLO5]      = TR("Remains of Apollo mission");
    stringsObject[OBJECT_APOLLO2]      = TR("Lunar Roving Vehicle");



    stringsErr[ERR_GENERIC]         = TR("Internal error - tell the developers");
    stringsErr[ERR_CMD]             = TR("Unknown command");
    stringsErr[ERR_MANIP_VEH]       = TR("Inappropriate bot");
    stringsErr[ERR_MANIP_FLY]       = TR("Impossible when flying");
    stringsErr[ERR_MANIP_BUSY]      = TR("Already carrying something");
    stringsErr[ERR_MANIP_NIL]       = TR("Nothing to grab");
    stringsErr[ERR_MANIP_MOTOR]     = TR("Impossible when moving");
    stringsErr[ERR_MANIP_OCC]       = TR("Place occupied");
    stringsErr[ERR_MANIP_FRIEND]    = TR("No other robot");
    stringsErr[ERR_MANIP_RADIO]     = TR("You can not carry a radioactive object");
    stringsErr[ERR_MANIP_WATER]     = TR("You can not carry an object under water");
    stringsErr[ERR_MANIP_EMPTY]     = TR("Nothing to drop");
    stringsErr[ERR_BUILD_FLY]       = TR("Impossible when flying");
    stringsErr[ERR_BUILD_WATER]     = TR("Impossible under water");
    stringsErr[ERR_BUILD_ENERGY]    = TR("Not enough energy");
    stringsErr[ERR_BUILD_METALAWAY] = TR("Titanium too far away");
    stringsErr[ERR_BUILD_METALNEAR] = TR("Titanium too close");
    stringsErr[ERR_BUILD_METALINEX] = TR("No titanium around");
    stringsErr[ERR_BUILD_FLAT]      = TR("Ground not flat enough");
    stringsErr[ERR_BUILD_FLATLIT]   = TR("Flat ground not large enough");
    stringsErr[ERR_BUILD_BUSY]      = TR("Place occupied");
    stringsErr[ERR_BUILD_BASE]      = TR("Too close to space ship");
    stringsErr[ERR_BUILD_NARROW]    = TR("Too close to a building");
    stringsErr[ERR_BUILD_MOTOR]     = TR("Impossible when moving");
    stringsErr[ERR_SEARCH_FLY]      = TR("Impossible when flying");
    stringsErr[ERR_BUILD_DISABLED]  = TR("Can not produce this object in this mission");
    stringsErr[ERR_BUILD_RESEARCH]  = TR("Can not produce not researched object");
    stringsErr[ERR_SEARCH_VEH]      = TR("Inappropriate bot");
    stringsErr[ERR_SEARCH_MOTOR]    = TR("Impossible when moving");
    stringsErr[ERR_TERRA_VEH]       = TR("Inappropriate bot");
    stringsErr[ERR_TERRA_ENERGY]    = TR("Not enough energy");
    stringsErr[ERR_TERRA_FLOOR]     = TR("Ground inappropriate");
    stringsErr[ERR_TERRA_BUILDING]  = TR("Building too close");
    stringsErr[ERR_TERRA_OBJECT]    = TR("Object too close");
    stringsErr[ERR_RECOVER_VEH]     = TR("Inappropriate bot");
    stringsErr[ERR_RECOVER_ENERGY]  = TR("Not enough energy");
    stringsErr[ERR_RECOVER_NULL]    = TR("Nothing to recycle");
    stringsErr[ERR_SHIELD_VEH]      = TR("Inappropriate bot");
    stringsErr[ERR_SHIELD_ENERGY]   = TR("No more energy");
    stringsErr[ERR_MOVE_IMPOSSIBLE] = TR("Error in instruction move");
    stringsErr[ERR_FIND_IMPOSSIBLE] = TR("Object not found");
    stringsErr[ERR_GOTO_IMPOSSIBLE] = TR("Goto: inaccessible destination");
    stringsErr[ERR_GOTO_ITER]       = TR("Goto: inaccessible destination");
    stringsErr[ERR_GOTO_BUSY]       = TR("Goto: destination occupied");
    stringsErr[ERR_FIRE_VEH]        = TR("Inappropriate bot");
    stringsErr[ERR_FIRE_ENERGY]     = TR("Not enough energy");
    stringsErr[ERR_FIRE_FLY]        = TR("Impossible when flying");
    stringsErr[ERR_CONVERT_EMPTY]   = TR("No titanium ore to convert");
    stringsErr[ERR_DERRICK_NULL]    = TR("No ore in the subsoil");
    stringsErr[ERR_STATION_NULL]    = TR("No energy in the subsoil");
    stringsErr[ERR_TOWER_POWER]     = TR("No power cell");
    stringsErr[ERR_TOWER_ENERGY]    = TR("No more energy");
    stringsErr[ERR_RESEARCH_POWER]  = TR("No power cell");
    stringsErr[ERR_RESEARCH_ENERGY] = TR("Not enough energy");
    stringsErr[ERR_RESEARCH_TYPE]   = TR("Inappropriate cell type");
    stringsErr[ERR_RESEARCH_ALREADY]= TR("Research program already performed");
    stringsErr[ERR_ENERGY_NULL]     = TR("No energy in the subsoil");
    stringsErr[ERR_ENERGY_LOW]      = TR("Not enough energy yet");
    stringsErr[ERR_ENERGY_EMPTY]    = TR("No titanium to transform");
    stringsErr[ERR_ENERGY_BAD]      = TR("Transforms only titanium");
    stringsErr[ERR_BASE_DLOCK]      = TR("Doors blocked by a robot or another object ");
    stringsErr[ERR_BASE_DHUMAN]     = TR("You must get on the spaceship to take off ");
    stringsErr[ERR_LABO_NULL]       = TR("Nothing to analyze");
    stringsErr[ERR_LABO_BAD]        = TR("Analyzes only organic matter");
    stringsErr[ERR_LABO_ALREADY]    = TR("Analysis already performed");
    stringsErr[ERR_NUCLEAR_NULL]    = TR("No energy in the subsoil");
    stringsErr[ERR_NUCLEAR_LOW]     = TR("Not yet enough energy");
    stringsErr[ERR_NUCLEAR_EMPTY]   = TR("No uranium to transform");
    stringsErr[ERR_NUCLEAR_BAD]     = TR("Transforms only uranium");
    stringsErr[ERR_FACTORY_NULL]    = TR("No titanium");
    stringsErr[ERR_FACTORY_NEAR]    = TR("Object too close");
    stringsErr[ERR_RESET_NEAR]      = TR("Place occupied");
    stringsErr[ERR_INFO_NULL]       = TR("No information exchange post within range");
    stringsErr[ERR_VEH_VIRUS]       = TR("Program infected by a virus");
    stringsErr[ERR_BAT_VIRUS]       = TR("Infected by a virus; temporarily out of order");
    stringsErr[ERR_VEH_POWER]       = TR("No power cell");
    stringsErr[ERR_VEH_ENERGY]      = TR("No more energy");
    stringsErr[ERR_FLAG_FLY]        = TR("Impossible when flying");
    stringsErr[ERR_FLAG_WATER]      = TR("Impossible when swimming");
    stringsErr[ERR_FLAG_MOTOR]      = TR("Impossible when moving");
    stringsErr[ERR_FLAG_BUSY]       = TR("Impossible when carrying an object");
    stringsErr[ERR_FLAG_CREATE]     = TR("Too many flags of this color (maximum 5)");
    stringsErr[ERR_FLAG_PROXY]      = TR("Too close to an existing flag");
    stringsErr[ERR_FLAG_DELETE]     = TR("No flag nearby");
    stringsErr[ERR_DESTROY_NOTFOUND]= TR("Not found anything to destroy");
    stringsErr[ERR_WRONG_OBJ]       = TR("Inappropriate object");
    stringsErr[ERR_MISSION_NOTERM]  = TR("The mission is not accomplished yet (press \\key help; for more details)");
    stringsErr[ERR_DELETEMOBILE]    = TR("Bot destroyed");
    stringsErr[ERR_DELETEBUILDING]  = TR("Building destroyed");
    stringsErr[ERR_TOOMANY]         = TR("Can not create this; there are too many objects");
    stringsErr[ERR_OBLIGATORYTOKEN] = TR("\"%s\" missing in this exercise");
    stringsErr[ERR_PROHIBITEDTOKEN] = TR("Do not use in this exercise");

    stringsErr[INFO_BUILD]          = TR("Building completed");
    stringsErr[INFO_CONVERT]        = TR("Titanium available");
    stringsErr[INFO_RESEARCH]       = TR("Research program completed");
    stringsErr[INFO_RESEARCHTANK]   = TR("Plans for tracked robots available ");
    stringsErr[INFO_RESEARCHFLY]    = TR("You can fly with the keys (\\key gup;) and (\\key gdown;)");
    stringsErr[INFO_RESEARCHTHUMP]  = TR("Plans for thumper available");
    stringsErr[INFO_RESEARCHCANON]  = TR("Plans for shooter available");
    stringsErr[INFO_RESEARCHTOWER]  = TR("Plans for defense tower available");
    stringsErr[INFO_RESEARCHPHAZER] = TR("Plans for phazer shooter available");
    stringsErr[INFO_RESEARCHSHIELD] = TR("Plans for shielder available");
    stringsErr[INFO_RESEARCHATOMIC] = TR("Plans for nuclear power plant available");
    stringsErr[INFO_FACTORY]        = TR("New bot available");
    stringsErr[INFO_LABO]           = TR("Analysis performed");
    stringsErr[INFO_ENERGY]         = TR("Power cell available");
    stringsErr[INFO_NUCLEAR]        = TR("Nuclear power cell available");
    stringsErr[INFO_FINDING]        = TR("You found a usable object");
    stringsErr[INFO_MARKPOWER]      = TR("Found a site for power station");
    stringsErr[INFO_MARKURANIUM]    = TR("Found a site for a derrick");
    stringsErr[INFO_MARKSTONE]      = TR("Found a site for a derrick");
    stringsErr[INFO_MARKKEYa]       = TR("Found a site for a derrick");
    stringsErr[INFO_MARKKEYb]       = TR("Found a site for a derrick");
    stringsErr[INFO_MARKKEYc]       = TR("Found a site for a derrick");
    stringsErr[INFO_MARKKEYd]       = TR("Found a site for a derrick");
    stringsErr[INFO_WIN]            = TR("<<< Well done; mission accomplished >>>");
    stringsErr[INFO_LOST]           = TR("<<< Sorry; mission failed >>>");
    stringsErr[INFO_LOSTq]          = TR("<<< Sorry; mission failed >>>");
    stringsErr[INFO_WRITEOK]        = TR("Current mission saved");
    stringsErr[INFO_DELETEPATH]     = TR("Checkpoint crossed");
    stringsErr[INFO_DELETEMOTHER]   = TR("Alien Queen killed");
    stringsErr[INFO_DELETEANT]      = TR("Ant fatally wounded");
    stringsErr[INFO_DELETEBEE]      = TR("Wasp fatally wounded");
    stringsErr[INFO_DELETEWORM]     = TR("Worm fatally wounded");
    stringsErr[INFO_DELETESPIDER]   = TR("Spider fatally wounded");
    stringsErr[INFO_BEGINSATCOM]    = TR("Press \\key help; to read instructions on your SatCom");



    stringsCbot[TX_OPENPAR]       = TR("Opening bracket missing");
    stringsCbot[TX_CLOSEPAR]      = TR("Closing bracket missing ");
    stringsCbot[TX_NOTBOOL]       = TR("The expression must return a boolean value");
    stringsCbot[TX_UNDEFVAR]      = TR("Variable not declared");
    stringsCbot[TX_BADLEFT]       = TR("Assignment impossible");
    stringsCbot[TX_ENDOF]         = TR("Semicolon terminator missing");
    stringsCbot[TX_OUTCASE]       = TR("Instruction \"case\" outside a block \"switch\"");
    stringsCbot[TX_NOTERM]        = TR("Instructions after the final closing brace");
    stringsCbot[TX_CLOSEBLK]      = TR("End of block missing");
    stringsCbot[TX_ELSEWITHOUTIF] = TR("Instruction \"else\" without corresponding \"if\" ");
    stringsCbot[TX_OPENBLK]       = TR("Opening brace missing ");
    stringsCbot[TX_BADTYPE]       = TR("Wrong type for the assignment");
    stringsCbot[TX_REDEFVAR]      = TR("A variable can not be declared twice");
    stringsCbot[TX_BAD2TYPE]      = TR("The types of the two operands are incompatible ");
    stringsCbot[TX_UNDEFCALL]     = TR("Unknown function");
    stringsCbot[TX_MISDOTS]       = TR("Sign \" : \" missing");
    stringsCbot[TX_WHILE]         = TR("Keyword \"while\" missing");
    stringsCbot[TX_BREAK]         = TR("Instruction \"break\" outside a loop");
    stringsCbot[TX_LABEL]         = TR("A label must be followed by \"for\"; \"while\"; \"do\" or \"switch\"");
    stringsCbot[TX_NOLABEL]       = TR("This label does not exist");
    stringsCbot[TX_NOCASE]        = TR("Instruction \"case\" missing");
    stringsCbot[TX_BADNUM]        = TR("Number missing");
    stringsCbot[TX_VOID]          = TR("Void parameter");
    stringsCbot[TX_NOTYP]         = TR("Type declaration missing");
    stringsCbot[TX_NOVAR]         = TR("Variable name missing");
    stringsCbot[TX_NOFONC]        = TR("Function name missing");
    stringsCbot[TX_OVERPARAM]     = TR("Too many parameters");
    stringsCbot[TX_REDEF]         = TR("Function already exists");
    stringsCbot[TX_LOWPARAM]      = TR("Parameters missing ");
    stringsCbot[TX_BADPARAM]      = TR("No function with this name accepts this kind of parameter");
    stringsCbot[TX_NUMPARAM]      = TR("No function with this name accepts this number of parameters");
    stringsCbot[TX_NOITEM]        = TR("This is not a member of this class");
    stringsCbot[TX_DOT]           = TR("This object is not a member of a class");
    stringsCbot[TX_NOCONST]       = TR("Appropriate constructor missing");
    stringsCbot[TX_REDEFCLASS]    = TR("This class already exists");
    stringsCbot[TX_CLBRK]         = TR("\" ] \" missing");
    stringsCbot[TX_RESERVED]      = TR("Reserved keyword of CBOT language");
    stringsCbot[TX_BADNEW]        = TR("Bad argument for \"new\"");
    stringsCbot[TX_OPBRK]         = TR("\" [ \" expected");
    stringsCbot[TX_BADSTRING]     = TR("String missing");
    stringsCbot[TX_BADINDEX]      = TR("Incorrect index type");
    stringsCbot[TX_PRIVATE]       = TR("Private element");
    stringsCbot[TX_NOPUBLIC]      = TR("Public required");
    stringsCbot[TX_DIVZERO]       = TR("Dividing by zero");
    stringsCbot[TX_NOTINIT]       = TR("Variable not initialized");
    stringsCbot[TX_BADTHROW]      = TR("Negative value rejected by \"throw\"");
    stringsCbot[TX_NORETVAL]      = TR("The function returned no value ");
    stringsCbot[TX_NORUN]         = TR("No function running");
    stringsCbot[TX_NOCALL]        = TR("Calling an unknown function");
    stringsCbot[TX_NOCLASS]       = TR("This class does not exist");
    stringsCbot[TX_NULLPT]        = TR("Unknown Object");
    stringsCbot[TX_OPNAN]         = TR("Operation impossible with value \"nan\"");
    stringsCbot[TX_OUTARRAY]      = TR("Access beyond array limit");
    stringsCbot[TX_STACKOVER]     = TR("Stack overflow");
    stringsCbot[TX_DELETEDPT]     = TR("Illegal object");
    stringsCbot[TX_FILEOPEN]      = TR("Can't open file");
    stringsCbot[TX_NOTOPEN]       = TR("File not open");
    stringsCbot[TX_ERRREAD]       = TR("Read error");
    stringsCbot[TX_ERRWRITE]      = TR("Write error");
}



static char         g_gamerName[100];

void SetGlobalGamerName(std::string name)
{
    strcpy(g_gamerName, name.c_str());
}


struct KeyDesc
{
    InputSlot   key;
    char        name[20];
};

static KeyDesc keyTable[22] =
{
    { INPUT_SLOT_LEFT,     "left;"    },
    { INPUT_SLOT_RIGHT,    "right;"   },
    { INPUT_SLOT_UP,       "up;"      },
    { INPUT_SLOT_DOWN,     "down;"    },
    { INPUT_SLOT_GUP,      "gup;"     },
    { INPUT_SLOT_GDOWN,    "gdown;"   },
    { INPUT_SLOT_CAMERA,   "camera;"  },
    { INPUT_SLOT_DESEL,    "desel;"   },
    { INPUT_SLOT_ACTION,   "action;"  },
    { INPUT_SLOT_NEAR,     "near;"    },
    { INPUT_SLOT_AWAY,     "away;"    },
    { INPUT_SLOT_NEXT,     "next;"    },
    { INPUT_SLOT_HUMAN,    "human;"   },
    { INPUT_SLOT_QUIT,     "quit;"    },
    { INPUT_SLOT_HELP,     "help;"    },
    { INPUT_SLOT_PROG,     "prog;"    },
    { INPUT_SLOT_CBOT,     "cbot;"    },
    { INPUT_SLOT_VISIT,    "visit;"   },
    { INPUT_SLOT_SPEED10,  "speed10;" },
    { INPUT_SLOT_SPEED15,  "speed15;" },
    { INPUT_SLOT_SPEED20,  "speed20;" }
};

// Seeks a key.

bool SearchKey(const char *cmd, InputSlot &key)
{
    for (int i = 0; i < 22 ;i++)
    {
        if ( strstr(cmd, keyTable[i].name) == cmd )
        {
            key = keyTable[i].key;
            return true;
        }
    }
    return false;
}

// Replaces the commands "\key name;" in a text.

static void PutKeyName(std::string& dst, const char* src)
{
    dst.clear();

    int s = 0;
    while ( src[s] != 0 )
    {
        if ( src[s+0] == '\\' &&
             src[s+1] == 'k'  &&
             src[s+2] == 'e'  &&
             src[s+3] == 'y'  &&
             src[s+4] == ' '  )
        {
            InputSlot key;
            if ( SearchKey(src+s+5, key) )
            {
                unsigned int res = CRobotMain::GetInstancePointer()->GetInputBinding(key).primary;
                if (res != KEY_INVALID)
                {
                    std::string keyName;
                    if ( GetResource(RES_KEY, res, keyName) )
                    {
                        dst.append(keyName);
                        while ( src[s++] != ';' );
                        continue;
                    }
                }
            }
        }

        dst.append(1, src[s++]);
    }
}

// Returns the translated text of a resource that needs key substitution

static const char* GetResourceBase(ResType type, int num)
{
    const char *str = NULL;

    assert(num >= 0);

    switch (type)
    {
        case RES_TEXT:
            assert(num < RT_MAX);
            str = stringsText[num];
            break;

        case RES_EVENT:
            if (num >= EVENT_STD_MAX)
                return ""; // can be safely ignored (user events)

            str = stringsEvent[num];
            break;

        case RES_OBJECT:
            assert(num < OBJECT_MAX);
            if (num == OBJECT_HUMAN)
                return g_gamerName;

            str = stringsObject[num];
            break;

        case RES_ERR:
            assert(num < ERR_MAX);
            str = stringsErr[num];
            break;

        case RES_CBOT:
            assert(num < TX_MAX);
            str = stringsCbot[num];
            break;

        case RES_KEY:

            if (static_cast<unsigned int>(num) == KEY_INVALID)
                return "";
            else if (num == VIRTUAL_KMOD_CTRL)
                return "Ctrl";
            else if (num == VIRTUAL_KMOD_SHIFT)
                return "Shift";
            else if (num == VIRTUAL_KMOD_ALT)
                return "Alt";
            else if (num == VIRTUAL_KMOD_META)
                return "Win";
            else if (num > VIRTUAL_JOY(0))
            {
                // TODO: temporary fix
                static std::string sstr;
                sstr = gettext("Button %1");
                sstr = StrUtils::Replace(sstr, "%1", StrUtils::ToString<int>(1 + num - VIRTUAL_JOY(0)));
                return sstr.c_str();
            }
            else
                str = SDL_GetKeyName(static_cast<SDLKey>(num));

            break;

        default:
            assert(false);
    }

    return gettext(str);
}

// Returns the text of a resource.

bool GetResource(ResType type, int num, std::string& text)
{
    const char *tmpl = GetResourceBase(type, num);

    if (!tmpl)
    {
        text.clear();
        return false;
    }

    PutKeyName(text, tmpl);
    return true;
}

