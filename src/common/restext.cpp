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


void InitializeRestext()
{
    stringsText[RT_VERSION_ID]       = COLOBOT_FULLNAME;

    stringsText[RT_DISINFO_TITLE]    = "SatCom";
    stringsText[RT_WINDOW_MAXIMIZED] = "Maximize";
    stringsText[RT_WINDOW_MINIMIZED] = "Minimize";
    stringsText[RT_WINDOW_STANDARD]  = "Normal size";
    stringsText[RT_WINDOW_CLOSE]     = "Close";

    stringsText[RT_STUDIO_TITLE]     = "Program editor";
    stringsText[RT_SCRIPT_NEW]       = "New";
    stringsText[RT_NAME_DEFAULT]     = "Player";
    stringsText[RT_IO_NEW]           = "New ...";
    stringsText[RT_KEY_OR]           = " or ";

    stringsText[RT_TITLE_BASE]       = "COLOBOT";
    stringsText[RT_TITLE_INIT]       = "COLOBOT";
    stringsText[RT_TITLE_TRAINER]    = "Programming exercises";
    stringsText[RT_TITLE_DEFI]       = "Challenges";
    stringsText[RT_TITLE_MISSION]    = "Missions";
    stringsText[RT_TITLE_FREE]       = "Free game";
    stringsText[RT_TITLE_TEEN]       = "Free game";
    stringsText[RT_TITLE_USER]       = "User levels";
    stringsText[RT_TITLE_PROTO]      = "Prototypes";
    stringsText[RT_TITLE_SETUP]      = "Options";
    stringsText[RT_TITLE_NAME]       = "Player's name";
    stringsText[RT_TITLE_PERSO]      = "Customize your appearance";
    stringsText[RT_TITLE_WRITE]      = "Save the current mission";
    stringsText[RT_TITLE_READ]       = "Load a saved mission";

    stringsText[RT_PLAY_CHAPt]       = " Chapters:";
    stringsText[RT_PLAY_CHAPd]       = " Chapters:";
    stringsText[RT_PLAY_CHAPm]       = " Planets:";
    stringsText[RT_PLAY_CHAPf]       = " Planets:";
    stringsText[RT_PLAY_CHAPu]       = " User levels:";
    stringsText[RT_PLAY_CHAPp]       = " Planets:";
    stringsText[RT_PLAY_CHAPte]      = " Chapters:";
    stringsText[RT_PLAY_LISTt]       = " Exercises in the chapter:";
    stringsText[RT_PLAY_LISTd]       = " Challenges in the chapter:";
    stringsText[RT_PLAY_LISTm]       = " Missions on this planet:";
    stringsText[RT_PLAY_LISTf]       = " Free game on this planet:";
    stringsText[RT_PLAY_LISTu]       = " Missions on this level:";
    stringsText[RT_PLAY_LISTp]       = " Prototypes on this planet:";
    stringsText[RT_PLAY_LISTk]       = " Free game on this chapter:";
    stringsText[RT_PLAY_RESUME]      = " Summary:";

    stringsText[RT_SETUP_DEVICE]     = " Drivers:";
    stringsText[RT_SETUP_MODE]       = " Resolution:";
    stringsText[RT_SETUP_KEY1]       = "1) First click on the key you want to redefine.";
    stringsText[RT_SETUP_KEY2]       = "2) Then press the key you want to use instead.";

    stringsText[RT_PERSO_FACE]       = "Face type:";
    stringsText[RT_PERSO_GLASSES]    = "Eyeglasses:";
    stringsText[RT_PERSO_HAIR]       = "Hair color:";
    stringsText[RT_PERSO_COMBI]      = "Suit color:";
    stringsText[RT_PERSO_BAND]       = "Strip color:";

    stringsText[RT_DIALOG_QUIT]      = "Do you want to quit COLOBOT ?";
    stringsText[RT_DIALOG_TITLE]     = "COLOBOT";
    stringsText[RT_DIALOG_YESQUIT]   = "Quit\\Quit COLOBOT";
    stringsText[RT_DIALOG_ABORT]     = "Quit the mission?";
    stringsText[RT_DIALOG_YES]       = "Abort\\Abort the current mission";
    stringsText[RT_DIALOG_NO]        = "Continue\\Continue the current mission";
    stringsText[RT_DIALOG_NOQUIT]    = "Continue\\Continue the game";
    stringsText[RT_DIALOG_DELOBJ]    = "Do you really want to destroy the selected building?";
    stringsText[RT_DIALOG_DELGAME]   = "Do you want to delete %s's saved games? ";
    stringsText[RT_DIALOG_YESDEL]    = "Delete";
    stringsText[RT_DIALOG_NODEL]     = "Cancel";
    stringsText[RT_DIALOG_LOADING]   = "LOADING";

    stringsText[RT_STUDIO_LISTTT]    = "Keyword help(\\key cbot;)";
    stringsText[RT_STUDIO_COMPOK]    = "Compilation ok (0 errors)";
    stringsText[RT_STUDIO_PROGSTOP]  = "Program finished";

    stringsText[RT_SATCOM_LIST]      = "\\b;List of objects\n";
    stringsText[RT_SATCOM_BOT]       = "\\b;Robots\n";
    stringsText[RT_SATCOM_BUILDING]  = "\\b;Buildings\n";
    stringsText[RT_SATCOM_FRET]      = "\\b;Moveable objects\n";
    stringsText[RT_SATCOM_ALIEN]     = "\\b;Aliens\n";
    stringsText[RT_SATCOM_NULL]      = "\\c; (none)\\n;\n";
    stringsText[RT_SATCOM_ERROR1]    = "\\b;Error\n";
    stringsText[RT_SATCOM_ERROR2]    = "The list is only available if a \\l;radar station\\u object\\radar; is working.\n";

    stringsText[RT_IO_OPEN]          = "Open";
    stringsText[RT_IO_SAVE]          = "Save";
    stringsText[RT_IO_LIST]          = "Folder: %s";
    stringsText[RT_IO_NAME]          = "Name:";
    stringsText[RT_IO_DIR]           = "Folder:";
    stringsText[RT_IO_PRIVATE]       = "Private\\Private folder";
    stringsText[RT_IO_PUBLIC]        = "Public\\Common folder";

    stringsText[RT_GENERIC_DEV1]     = "Developed by :";
    stringsText[RT_GENERIC_DEV2]     = "www.epsitec.com";
    stringsText[RT_GENERIC_EDIT1]    = " ";
    stringsText[RT_GENERIC_EDIT2]    = " ";

    stringsText[RT_INTERFACE_REC]    = "Recorder";



    stringsEvent[EVENT_BUTTON_OK]           = "OK";
    stringsEvent[EVENT_BUTTON_CANCEL]       = "Cancel";
    stringsEvent[EVENT_BUTTON_NEXT]         = "Next";
    stringsEvent[EVENT_BUTTON_PREV]         = "Previous";
    stringsEvent[EVENT_BUTTON_QUIT]         = "Menu (\\key quit;)";

    stringsEvent[EVENT_DIALOG_OK]           = "OK";
    stringsEvent[EVENT_DIALOG_CANCEL]       = "Cancel";

    stringsEvent[EVENT_INTERFACE_TRAINER]   = "Exercises\\Programming exercises";
    stringsEvent[EVENT_INTERFACE_DEFI]      = "Challenges\\Programming challenges";
    stringsEvent[EVENT_INTERFACE_MISSION]   = "Missions\\Select mission";
    stringsEvent[EVENT_INTERFACE_FREE]      = "Free game\\Free game without a specific goal";
    stringsEvent[EVENT_INTERFACE_TEEN]      = "Free game\\Free game without a specific goal";
    stringsEvent[EVENT_INTERFACE_USER]      = "User\\User levels";
    stringsEvent[EVENT_INTERFACE_PROTO]     = "Proto\\Prototypes under development";
    stringsEvent[EVENT_INTERFACE_NAME]      = "New player\\Choose player's name";
    stringsEvent[EVENT_INTERFACE_SETUP]     = "Options\\Preferences";
    stringsEvent[EVENT_INTERFACE_AGAIN]     = "Restart\\Restart the mission from the beginning";
    stringsEvent[EVENT_INTERFACE_WRITE]     = "Save\\Save the current mission ";
    stringsEvent[EVENT_INTERFACE_READ]      = "Load\\Load a saved mission";
    stringsEvent[EVENT_INTERFACE_ABORT]     = "\\Return to COLOBOT";
    stringsEvent[EVENT_INTERFACE_QUIT]      = "Quit\\Quit COLOBOT";
    stringsEvent[EVENT_INTERFACE_BACK]      = "<<  Back  \\Back to the previous screen";
    stringsEvent[EVENT_INTERFACE_PLAY]      = "Play\\Start mission!";
    stringsEvent[EVENT_INTERFACE_SETUPd]    = "Device\\Driver and resolution settings";
    stringsEvent[EVENT_INTERFACE_SETUPg]    = "Graphics\\Graphics settings";
    stringsEvent[EVENT_INTERFACE_SETUPp]    = "Game\\Game settings";
    stringsEvent[EVENT_INTERFACE_SETUPc]    = "Controls\\Keyboard, joystick and mouse settings";
    stringsEvent[EVENT_INTERFACE_SETUPs]    = "Sound\\Music and game sound volume";
    stringsEvent[EVENT_INTERFACE_DEVICE]    = "Unit";
    stringsEvent[EVENT_INTERFACE_RESOL]     = "Resolution";
    stringsEvent[EVENT_INTERFACE_FULL]      = "Full screen\\Full screen or window mode";
    stringsEvent[EVENT_INTERFACE_APPLY]     = "Apply changes\\Activates the changed settings";

    stringsEvent[EVENT_INTERFACE_TOTO]      = "Robbie\\Your assistant";
    stringsEvent[EVENT_INTERFACE_SHADOW]    = "Shadows\\Shadows on the ground";
    stringsEvent[EVENT_INTERFACE_GROUND]    = "Marks on the ground\\Marks on the ground";
    stringsEvent[EVENT_INTERFACE_DIRTY]     = "Dust\\Dust and dirt on bots and buildings";
    stringsEvent[EVENT_INTERFACE_FOG]       = "Fog\\Fog";
    stringsEvent[EVENT_INTERFACE_LENS]      = "Sunbeams\\Sunbeams in the sky";
    stringsEvent[EVENT_INTERFACE_SKY]       = "Sky\\Clouds and nebulae";
    stringsEvent[EVENT_INTERFACE_PLANET]    = "Planets and stars\\Astronomical objects in the sky";
    stringsEvent[EVENT_INTERFACE_LIGHT]     = "Dynamic lighting\\Mobile light sources";
    stringsEvent[EVENT_INTERFACE_PARTI]     = "Number of particles\\Explosions, dust, reflections, etc.";
    stringsEvent[EVENT_INTERFACE_CLIP]      = "Depth of field\\Maximum visibility";
    stringsEvent[EVENT_INTERFACE_DETAIL]    = "Details\\Visual quality of 3D objects";
    stringsEvent[EVENT_INTERFACE_TEXTURE]   = "Textures\\Quality of textures ";
    stringsEvent[EVENT_INTERFACE_GADGET]    = "Num of decorative objects\\Number of purely ornamental objects";
    stringsEvent[EVENT_INTERFACE_RAIN]      = "Particles in the interface\\Steam clouds and sparks in the interface";
    stringsEvent[EVENT_INTERFACE_GLINT]     = "Reflections on the buttons \\Shiny buttons";
    stringsEvent[EVENT_INTERFACE_TOOLTIP]   = "Help balloons\\Explain the function of the buttons";
    stringsEvent[EVENT_INTERFACE_MOVIES]    = "Film sequences\\Films before and after the missions";
    stringsEvent[EVENT_INTERFACE_NICERST]   = "Exit film\\Film at the exit of exercises";
    stringsEvent[EVENT_INTERFACE_HIMSELF]   = "Friendly fire\\Your shooting can damage your own objects ";
    stringsEvent[EVENT_INTERFACE_SCROLL]    = "Scrolling\\Scrolling when the mouse touches right or left border";
    stringsEvent[EVENT_INTERFACE_INVERTX]   = "Mouse inversion X\\Inversion of the scrolling direction on the X axis";
    stringsEvent[EVENT_INTERFACE_INVERTY]   = "Mouse inversion Y\\Inversion of the scrolling direction on the Y axis";
    stringsEvent[EVENT_INTERFACE_EFFECT]    = "Quake at explosions\\The screen shakes at explosions";
    stringsEvent[EVENT_INTERFACE_MOUSE]     = "Mouse shadow\\Gives the mouse a shadow";
    stringsEvent[EVENT_INTERFACE_EDITMODE]  = "Automatic indent\\When program editing";
    stringsEvent[EVENT_INTERFACE_EDITVALUE] = "Big indent\\Indent 2 or 4 spaces per level defined by braces";
    stringsEvent[EVENT_INTERFACE_SOLUCE4]   = "Access to solutions\\Show program \"4: Solution\" in the exercises";

    stringsEvent[EVENT_INTERFACE_KDEF]      = "Standard controls\\Standard key functions";
    stringsEvent[EVENT_INTERFACE_KLEFT]     = "Turn left\\turns the bot to the left";
    stringsEvent[EVENT_INTERFACE_KRIGHT]    = "Turn right\\turns the bot to the right";
    stringsEvent[EVENT_INTERFACE_KUP]       = "Forward\\Moves forward";
    stringsEvent[EVENT_INTERFACE_KDOWN]     = "Backward\\Moves backward";
    stringsEvent[EVENT_INTERFACE_KGUP]      = "Climb\\Increases the power of the jet";
    stringsEvent[EVENT_INTERFACE_KGDOWN]    = "Descend\\Reduces the power of the jet";
    stringsEvent[EVENT_INTERFACE_KCAMERA]   = "Change camera\\Switches between onboard camera and following camera";
    stringsEvent[EVENT_INTERFACE_KDESEL]    = "Previous object\\Selects the previous object";
    stringsEvent[EVENT_INTERFACE_KACTION]   = "Standard action\\Standard action of the bot (take/grab, shoot, sniff, etc)";
    stringsEvent[EVENT_INTERFACE_KNEAR]     = "Camera closer\\Moves the camera forward";
    stringsEvent[EVENT_INTERFACE_KAWAY]     = "Camera back\\Moves the camera backward";
    stringsEvent[EVENT_INTERFACE_KNEXT]     = "Next object\\Selects the next object";
    stringsEvent[EVENT_INTERFACE_KHUMAN]    = "Select the astronaut\\Selects the astronaut";
    stringsEvent[EVENT_INTERFACE_KQUIT]     = "Quit\\Quit the current mission or exercise";
    stringsEvent[EVENT_INTERFACE_KHELP]     = "Instructions\\Shows the instructions for the current mission";
    stringsEvent[EVENT_INTERFACE_KPROG]     = "Programming help\\Gives more detailed help with programming";
    stringsEvent[EVENT_INTERFACE_KCBOT]     = "Key word help\\More detailed help about key words";
    stringsEvent[EVENT_INTERFACE_KVISIT]    = "Origin of last message\\Shows where the last message was sent from";
    stringsEvent[EVENT_INTERFACE_KSPEED10]  = "Speed 1.0x\\Normal speed";
    stringsEvent[EVENT_INTERFACE_KSPEED15]  = "Speed 1.5x\\1.5 times faster";
    stringsEvent[EVENT_INTERFACE_KSPEED20]  = "Speed 2.0x\\Double speed";
    stringsEvent[EVENT_INTERFACE_KSPEED30]  = "Speed 3.0x\\Three times faster";

    stringsEvent[EVENT_INTERFACE_VOLSOUND]  = "Sound effects:\\Volume of engines, voice, shooting, etc.";
    stringsEvent[EVENT_INTERFACE_VOLMUSIC]  = "Background sound :\\Volume of audio tracks on the CD";
    stringsEvent[EVENT_INTERFACE_SOUND3D]   = "3D sound\\3D positioning of the sound";

    stringsEvent[EVENT_INTERFACE_MIN]       = "Lowest\\Minimum graphic quality (highest frame rate)";
    stringsEvent[EVENT_INTERFACE_NORM]      = "Normal\\Normal graphic quality";
    stringsEvent[EVENT_INTERFACE_MAX]       = "Highest\\Highest graphic quality (lowest frame rate)";

    stringsEvent[EVENT_INTERFACE_SILENT]    = "Mute\\No sound";
    stringsEvent[EVENT_INTERFACE_NOISY]     = "Normal\\Normal sound volume";

    stringsEvent[EVENT_INTERFACE_JOYSTICK]  = "Use a joystick\\Joystick or keyboard";
    stringsEvent[EVENT_INTERFACE_SOLUCE]    = "Access to solution\\Shows the solution (detailed instructions for missions)";

    stringsEvent[EVENT_INTERFACE_NEDIT]     = "\\New player name";
    stringsEvent[EVENT_INTERFACE_NOK]       = "OK\\Choose the selected player";
    stringsEvent[EVENT_INTERFACE_NCANCEL]   = "Cancel\\Keep current player name";
    stringsEvent[EVENT_INTERFACE_NDELETE]   = "Delete player\\Deletes the player from the list";
    stringsEvent[EVENT_INTERFACE_NLABEL]    = "Player name";

    stringsEvent[EVENT_INTERFACE_IOWRITE]   = "Save\\Saves the current mission";
    stringsEvent[EVENT_INTERFACE_IOREAD]    = "Load\\Loads the selected mission";
    stringsEvent[EVENT_INTERFACE_IOLIST]    = "List of saved missions";
    stringsEvent[EVENT_INTERFACE_IOLABEL]   = "Filename:";
    stringsEvent[EVENT_INTERFACE_IONAME]    = "Mission name";
    stringsEvent[EVENT_INTERFACE_IOIMAGE]   = "Photography";
    stringsEvent[EVENT_INTERFACE_IODELETE]  = "Delete\\Deletes the selected file";

    stringsEvent[EVENT_INTERFACE_PERSO]     = "Appearance\\Choose your appearance";
    stringsEvent[EVENT_INTERFACE_POK]       = "OK";
    stringsEvent[EVENT_INTERFACE_PCANCEL]   = "Cancel";
    stringsEvent[EVENT_INTERFACE_PDEF]      = "Standard\\Standard appearance settings";
    stringsEvent[EVENT_INTERFACE_PHEAD]     = "Head\\Face and hair";
    stringsEvent[EVENT_INTERFACE_PBODY]     = "Suit\\Astronaut suit";
    stringsEvent[EVENT_INTERFACE_PLROT]     = "\\Turn left";
    stringsEvent[EVENT_INTERFACE_PRROT]     = "\\Turn right";
    stringsEvent[EVENT_INTERFACE_PCRa]      = "Red";
    stringsEvent[EVENT_INTERFACE_PCGa]      = "Green";
    stringsEvent[EVENT_INTERFACE_PCBa]      = "Blue";
    stringsEvent[EVENT_INTERFACE_PCRb]      = "Red";
    stringsEvent[EVENT_INTERFACE_PCGb]      = "Green";
    stringsEvent[EVENT_INTERFACE_PCBb]      = "Blue";
    stringsEvent[EVENT_INTERFACE_PFACE1]    = "\\Face 1";
    stringsEvent[EVENT_INTERFACE_PFACE2]    = "\\Face 4";
    stringsEvent[EVENT_INTERFACE_PFACE3]    = "\\Face 3";
    stringsEvent[EVENT_INTERFACE_PFACE4]    = "\\Face 2";
    stringsEvent[EVENT_INTERFACE_PGLASS0]   = "\\No eyeglasses";
    stringsEvent[EVENT_INTERFACE_PGLASS1]   = "\\Eyeglasses 1";
    stringsEvent[EVENT_INTERFACE_PGLASS2]   = "\\Eyeglasses 2";
    stringsEvent[EVENT_INTERFACE_PGLASS3]   = "\\Eyeglasses 3";
    stringsEvent[EVENT_INTERFACE_PGLASS4]   = "\\Eyeglasses 4";
    stringsEvent[EVENT_INTERFACE_PGLASS5]   = "\\Eyeglasses 5";

    stringsEvent[EVENT_OBJECT_DESELECT]     = "Previous selection (\\key desel;)";
    stringsEvent[EVENT_OBJECT_LEFT]         = "Turn left (\\key left;)";
    stringsEvent[EVENT_OBJECT_RIGHT]        = "Turn right (\\key right;)";
    stringsEvent[EVENT_OBJECT_UP]           = "Forward (\\key up;)";
    stringsEvent[EVENT_OBJECT_DOWN]         = "Backward (\\key down;)";
    stringsEvent[EVENT_OBJECT_GASUP]        = "Up (\\key gup;)";
    stringsEvent[EVENT_OBJECT_GASDOWN]      = "Down (\\key gdown;)";
    stringsEvent[EVENT_OBJECT_HTAKE]        = "Grab or drop (\\key action;)";
    stringsEvent[EVENT_OBJECT_MTAKE]        = "Grab or drop (\\key action;)";
    stringsEvent[EVENT_OBJECT_MFRONT]       = "..in front";
    stringsEvent[EVENT_OBJECT_MBACK]        = "..behind";
    stringsEvent[EVENT_OBJECT_MPOWER]       = "..power cell";
    stringsEvent[EVENT_OBJECT_BHELP]        = "Instructions for the mission (\\key help;)";
    stringsEvent[EVENT_OBJECT_BTAKEOFF]     = "Take off to finish the mission";
    stringsEvent[EVENT_OBJECT_BDESTROY]     = "Destroy";
    stringsEvent[EVENT_OBJECT_BDERRICK]     = "Build a derrick";
    stringsEvent[EVENT_OBJECT_BSTATION]     = "Build a power station";
    stringsEvent[EVENT_OBJECT_BFACTORY]     = "Build a bot factory";
    stringsEvent[EVENT_OBJECT_BREPAIR]      = "Build a repair center";
    stringsEvent[EVENT_OBJECT_BCONVERT]     = "Build a converter";
    stringsEvent[EVENT_OBJECT_BTOWER]       = "Build a defense tower";
    stringsEvent[EVENT_OBJECT_BRESEARCH]    = "Build a research center";
    stringsEvent[EVENT_OBJECT_BRADAR]       = "Build a radar station";
    stringsEvent[EVENT_OBJECT_BENERGY]      = "Build a power cell factory";
    stringsEvent[EVENT_OBJECT_BLABO]        = "Build an autolab";
    stringsEvent[EVENT_OBJECT_BNUCLEAR]     = "Build a nuclear power plant";
    stringsEvent[EVENT_OBJECT_BPARA]        = "Build a lightning conductor";
    stringsEvent[EVENT_OBJECT_BINFO]        = "Build a exchange post";
    stringsEvent[EVENT_OBJECT_BDESTROYER]   = "Build a destroyer";
    stringsEvent[EVENT_OBJECT_GFLAT]        = "Show if the ground is flat";
    stringsEvent[EVENT_OBJECT_FCREATE]      = "Plant a flag";
    stringsEvent[EVENT_OBJECT_FDELETE]      = "Remove a flag";
    stringsEvent[EVENT_OBJECT_FCOLORb]      = "\\Blue flags";
    stringsEvent[EVENT_OBJECT_FCOLORr]      = "\\Red flags";
    stringsEvent[EVENT_OBJECT_FCOLORg]      = "\\Green flags";
    stringsEvent[EVENT_OBJECT_FCOLORy]      = "\\Yellow flags";
    stringsEvent[EVENT_OBJECT_FCOLORv]      = "\\Violet flags";
    stringsEvent[EVENT_OBJECT_FACTORYfa]    = "Build a winged grabber";
    stringsEvent[EVENT_OBJECT_FACTORYta]    = "Build a tracked grabber";
    stringsEvent[EVENT_OBJECT_FACTORYwa]    = "Build a wheeled grabber";
    stringsEvent[EVENT_OBJECT_FACTORYia]    = "Build a legged grabber";
    stringsEvent[EVENT_OBJECT_FACTORYfc]    = "Build a winged shooter";
    stringsEvent[EVENT_OBJECT_FACTORYtc]    = "Build a tracked shooter";
    stringsEvent[EVENT_OBJECT_FACTORYwc]    = "Build a wheeled shooter";
    stringsEvent[EVENT_OBJECT_FACTORYic]    = "Build a legged shooter";
    stringsEvent[EVENT_OBJECT_FACTORYfi]    = "Build a winged orga shooter";
    stringsEvent[EVENT_OBJECT_FACTORYti]    = "Build a tracked orga shooter";
    stringsEvent[EVENT_OBJECT_FACTORYwi]    = "Build a wheeled orga shooter";
    stringsEvent[EVENT_OBJECT_FACTORYii]    = "Build a legged orga shooter";
    stringsEvent[EVENT_OBJECT_FACTORYfs]    = "Build a winged sniffer";
    stringsEvent[EVENT_OBJECT_FACTORYts]    = "Build a tracked sniffer";
    stringsEvent[EVENT_OBJECT_FACTORYws]    = "Build a wheeled sniffer";
    stringsEvent[EVENT_OBJECT_FACTORYis]    = "Build a legged sniffer";
    stringsEvent[EVENT_OBJECT_FACTORYrt]    = "Build a thumper";
    stringsEvent[EVENT_OBJECT_FACTORYrc]    = "Build a phazer shooter";
    stringsEvent[EVENT_OBJECT_FACTORYrr]    = "Build a recycler";
    stringsEvent[EVENT_OBJECT_FACTORYrs]    = "Build a shielder";
    stringsEvent[EVENT_OBJECT_FACTORYsa]    = "Build a subber";
    stringsEvent[EVENT_OBJECT_RTANK]        = "Run research program for tracked bots";
    stringsEvent[EVENT_OBJECT_RFLY]         = "Run research program for winged bots";
    stringsEvent[EVENT_OBJECT_RTHUMP]       = "Run research program for thumper";
    stringsEvent[EVENT_OBJECT_RCANON]       = "Run research program for shooter";
    stringsEvent[EVENT_OBJECT_RTOWER]       = "Run research program for defense tower";
    stringsEvent[EVENT_OBJECT_RPHAZER]      = "Run research program for phazer shooter";
    stringsEvent[EVENT_OBJECT_RSHIELD]      = "Run research program for shielder";
    stringsEvent[EVENT_OBJECT_RATOMIC]      = "Run research program for nuclear power";
    stringsEvent[EVENT_OBJECT_RiPAW]        = "Run research program for legged bots";
    stringsEvent[EVENT_OBJECT_RiGUN]        = "Run research program for orga shooter";
    stringsEvent[EVENT_OBJECT_RESET]        = "Return to start";
    stringsEvent[EVENT_OBJECT_SEARCH]       = "Sniff (\\key action;)";
    stringsEvent[EVENT_OBJECT_TERRAFORM]    = "Thump (\\key action;)";
    stringsEvent[EVENT_OBJECT_FIRE]         = "Shoot (\\key action;)";
    stringsEvent[EVENT_OBJECT_SPIDEREXPLO]  = "Explode (\\key action;)";
    stringsEvent[EVENT_OBJECT_RECOVER]      = "Recycle (\\key action;)";
    stringsEvent[EVENT_OBJECT_BEGSHIELD]    = "Extend shield (\\key action;)";
    stringsEvent[EVENT_OBJECT_ENDSHIELD]    = "Withdraw shield (\\key action;)";
    stringsEvent[EVENT_OBJECT_DIMSHIELD]    = "Shield radius";
    stringsEvent[EVENT_OBJECT_PROGRUN]      = "Execute the selected program";
    stringsEvent[EVENT_OBJECT_PROGEDIT]     = "Edit the selected program";
    stringsEvent[EVENT_OBJECT_INFOOK]       = "\\SatCom on standby";
    stringsEvent[EVENT_OBJECT_DELETE]       = "Destroy the building";
    stringsEvent[EVENT_OBJECT_GENERGY]      = "Energy level";
    stringsEvent[EVENT_OBJECT_GSHIELD]      = "Shield level";
    stringsEvent[EVENT_OBJECT_GRANGE]       = "Jet temperature";
    stringsEvent[EVENT_OBJECT_GPROGRESS]    = "Still working ...";
    stringsEvent[EVENT_OBJECT_GRADAR]       = "Number of insects detected";
    stringsEvent[EVENT_OBJECT_GINFO]        = "Transmitted information";
    stringsEvent[EVENT_OBJECT_COMPASS]      = "Compass";
    stringsEvent[EVENT_OBJECT_MAPZOOM]      = "Zoom mini-map";
    stringsEvent[EVENT_OBJECT_CAMERA]       = "Camera (\\key camera;)";
    stringsEvent[EVENT_OBJECT_CAMERAleft]   = "Camera to left";
    stringsEvent[EVENT_OBJECT_CAMERAright]  = "Camera to right";
    stringsEvent[EVENT_OBJECT_CAMERAnear]   = "Camera nearest";
    stringsEvent[EVENT_OBJECT_CAMERAaway]   = "Camera awayest";
    stringsEvent[EVENT_OBJECT_HELP]         = "Help about selected object";
    stringsEvent[EVENT_OBJECT_SOLUCE]       = "Show the solution";
    stringsEvent[EVENT_OBJECT_SHORTCUT00]   = "Switch bots <-> buildings";
    stringsEvent[EVENT_OBJECT_LIMIT]        = "Show the range";
    stringsEvent[EVENT_OBJECT_PEN0]         = "\\Raise the pencil";
    stringsEvent[EVENT_OBJECT_PEN1]         = "\\Use the black pencil";
    stringsEvent[EVENT_OBJECT_PEN2]         = "\\Use the yellow pencil";
    stringsEvent[EVENT_OBJECT_PEN3]         = "\\Use the orange pencil";
    stringsEvent[EVENT_OBJECT_PEN4]         = "\\Use the red pencil";
    stringsEvent[EVENT_OBJECT_PEN5]         = "\\Use the purple pencil";
    stringsEvent[EVENT_OBJECT_PEN6]         = "\\Use the blue pencil";
    stringsEvent[EVENT_OBJECT_PEN7]         = "\\Use the green pencil";
    stringsEvent[EVENT_OBJECT_PEN8]         = "\\Use the brown pencil";
    stringsEvent[EVENT_OBJECT_REC]          = "\\Start recording";
    stringsEvent[EVENT_OBJECT_STOP]         = "\\Stop recording";
    stringsEvent[EVENT_DT_VISIT0]           = "Show the place";
    stringsEvent[EVENT_DT_VISIT1]           = "Show the place";
    stringsEvent[EVENT_DT_VISIT2]           = "Show the place";
    stringsEvent[EVENT_DT_VISIT3]           = "Show the place";
    stringsEvent[EVENT_DT_VISIT4]           = "Show the place";
    stringsEvent[EVENT_DT_END]              = "Continue";
    stringsEvent[EVENT_CMD]                 = "Command line";
    stringsEvent[EVENT_SPEED]               = "Game speed";

    stringsEvent[EVENT_HYPER_PREV]          = "Back";
    stringsEvent[EVENT_HYPER_NEXT]          = "Forward";
    stringsEvent[EVENT_HYPER_HOME]          = "Home";
    stringsEvent[EVENT_HYPER_COPY]          = "Copy";
    stringsEvent[EVENT_HYPER_SIZE1]         = "Size 1";
    stringsEvent[EVENT_HYPER_SIZE2]         = "Size 2";
    stringsEvent[EVENT_HYPER_SIZE3]         = "Size 3";
    stringsEvent[EVENT_HYPER_SIZE4]         = "Size 4";
    stringsEvent[EVENT_HYPER_SIZE5]         = "Size 5";
    stringsEvent[EVENT_SATCOM_HUSTON]       = "Instructions from Houston";
    stringsEvent[EVENT_SATCOM_SAT]          = "Satellite report";
    stringsEvent[EVENT_SATCOM_LOADING]      = "Programs dispatched by Houston";
    stringsEvent[EVENT_SATCOM_OBJECT]       = "List of objects";
    stringsEvent[EVENT_SATCOM_PROG]         = "Programming help";
    stringsEvent[EVENT_SATCOM_SOLUCE]       = "Solution";

    stringsEvent[EVENT_STUDIO_OK]           = "OK\\Close program editor and return to game";
    stringsEvent[EVENT_STUDIO_CANCEL]       = "Cancel\\Cancel all changes";
    stringsEvent[EVENT_STUDIO_NEW]          = "New";
    stringsEvent[EVENT_STUDIO_OPEN]         = "Open (Ctrl+o)";
    stringsEvent[EVENT_STUDIO_SAVE]         = "Save (Ctrl+s)";
    stringsEvent[EVENT_STUDIO_UNDO]         = "Undo (Ctrl+z)";
    stringsEvent[EVENT_STUDIO_CUT]          = "Cut (Ctrl+x)";
    stringsEvent[EVENT_STUDIO_COPY]         = "Copy (Ctrl+c)";
    stringsEvent[EVENT_STUDIO_PASTE]        = "Paste (Ctrl+v)";
    stringsEvent[EVENT_STUDIO_SIZE]         = "Font size";
    stringsEvent[EVENT_STUDIO_TOOL]         = "Instructions (\\key help;)";
    stringsEvent[EVENT_STUDIO_HELP]         = "Programming help  (\\key prog;)";
    stringsEvent[EVENT_STUDIO_COMPILE]      = "Compile";
    stringsEvent[EVENT_STUDIO_RUN]          = "Execute/stop";
    stringsEvent[EVENT_STUDIO_REALTIME]     = "Pause/continue";
    stringsEvent[EVENT_STUDIO_STEP]         = "One step";



    stringsObject[OBJECT_PORTICO]      = "Gantry crane";
    stringsObject[OBJECT_BASE]         = "Spaceship";
    stringsObject[OBJECT_DERRICK]      = "Derrick";
    stringsObject[OBJECT_FACTORY]      = "Bot factory";
    stringsObject[OBJECT_REPAIR]       = "Repair center";
    stringsObject[OBJECT_DESTROYER]    = "Destroyer";
    stringsObject[OBJECT_STATION]      = "Power station";
    stringsObject[OBJECT_CONVERT]      = "Converts ore to titanium";
    stringsObject[OBJECT_TOWER]        = "Defense tower";
    stringsObject[OBJECT_NEST]         = "Nest";
    stringsObject[OBJECT_RESEARCH]     = "Research center";
    stringsObject[OBJECT_RADAR]        = "Radar station";
    stringsObject[OBJECT_INFO]         = "Information exchange post";
    stringsObject[OBJECT_ENERGY]       = "Power cell factory";
    stringsObject[OBJECT_LABO]         = "Autolab";
    stringsObject[OBJECT_NUCLEAR]      = "Nuclear power station";
    stringsObject[OBJECT_PARA]         = "Lightning conductor";
    stringsObject[OBJECT_SAFE]         = "Vault";
    stringsObject[OBJECT_HUSTON]       = "Houston Mission Control";
    stringsObject[OBJECT_TARGET1]      = "Target";
    stringsObject[OBJECT_TARGET2]      = "Target";
    stringsObject[OBJECT_START]        = "Start";
    stringsObject[OBJECT_END]          = "Finish";
    stringsObject[OBJECT_STONE]        = "Titanium ore";
    stringsObject[OBJECT_URANIUM]      = "Uranium ore";
    stringsObject[OBJECT_BULLET]       = "Organic matter";
    stringsObject[OBJECT_METAL]        = "Titanium";
    stringsObject[OBJECT_POWER]        = "Power cell";
    stringsObject[OBJECT_ATOMIC]       = "Nuclear power cell";
    stringsObject[OBJECT_BBOX]         = "Black box";
    stringsObject[OBJECT_KEYa]         = "Key A";
    stringsObject[OBJECT_KEYb]         = "Key B";
    stringsObject[OBJECT_KEYc]         = "Key C";
    stringsObject[OBJECT_KEYd]         = "Key D";
    stringsObject[OBJECT_TNT]          = "Explosive";
    stringsObject[OBJECT_BOMB]         = "Fixed mine";
    stringsObject[OBJECT_BAG]          = "Survival kit";
    stringsObject[OBJECT_WAYPOINT]     = "Checkpoint";
    stringsObject[OBJECT_FLAGb]        = "Blue flag";
    stringsObject[OBJECT_FLAGr]        = "Red flag";
    stringsObject[OBJECT_FLAGg]        = "Green flag";
    stringsObject[OBJECT_FLAGy]        = "Yellow flag";
    stringsObject[OBJECT_FLAGv]        = "Violet flag";
    stringsObject[OBJECT_MARKPOWER]    = "Energy deposit (site for power station)";
    stringsObject[OBJECT_MARKURANIUM]  = "Uranium deposit (site for derrick)";
    stringsObject[OBJECT_MARKKEYa]     = "Found key A (site for derrick)";
    stringsObject[OBJECT_MARKKEYb]     = "Found key B (site for derrick)";
    stringsObject[OBJECT_MARKKEYc]     = "Found key C (site for derrick)";
    stringsObject[OBJECT_MARKKEYd]     = "Found key D (site for derrick)";
    stringsObject[OBJECT_MARKSTONE]    = "Titanium deposit (site for derrick)";
    stringsObject[OBJECT_MOBILEft]     = "Practice bot";
    stringsObject[OBJECT_MOBILEtt]     = "Practice bot";
    stringsObject[OBJECT_MOBILEwt]     = "Practice bot";
    stringsObject[OBJECT_MOBILEit]     = "Practice bot";
    stringsObject[OBJECT_MOBILEfa]     = "Winged grabber";
    stringsObject[OBJECT_MOBILEta]     = "Tracked grabber";
    stringsObject[OBJECT_MOBILEwa]     = "Wheeled grabber";
    stringsObject[OBJECT_MOBILEia]     = "Legged grabber";
    stringsObject[OBJECT_MOBILEfc]     = "Winged shooter";
    stringsObject[OBJECT_MOBILEtc]     = "Tracked shooter";
    stringsObject[OBJECT_MOBILEwc]     = "Wheeled shooter";
    stringsObject[OBJECT_MOBILEic]     = "Legged shooter";
    stringsObject[OBJECT_MOBILEfi]     = "Winged orga shooter";
    stringsObject[OBJECT_MOBILEti]     = "Tracked orga shooter";
    stringsObject[OBJECT_MOBILEwi]     = "Wheeled orga shooter";
    stringsObject[OBJECT_MOBILEii]     = "Legged orga shooter";
    stringsObject[OBJECT_MOBILEfs]     = "Winged sniffer";
    stringsObject[OBJECT_MOBILEts]     = "Tracked sniffer";
    stringsObject[OBJECT_MOBILEws]     = "Wheeled sniffer";
    stringsObject[OBJECT_MOBILEis]     = "Legged sniffer";
    stringsObject[OBJECT_MOBILErt]     = "Thumper";
    stringsObject[OBJECT_MOBILErc]     = "Phazer shooter";
    stringsObject[OBJECT_MOBILErr]     = "Recycler";
    stringsObject[OBJECT_MOBILErs]     = "Shielder";
    stringsObject[OBJECT_MOBILEsa]     = "Subber";
    stringsObject[OBJECT_MOBILEtg]     = "Target bot";
    stringsObject[OBJECT_MOBILEdr]     = "Drawer bot";
    stringsObject[OBJECT_TECH]         = "Engineer";
    stringsObject[OBJECT_TOTO]         = "Robbie";
    stringsObject[OBJECT_MOTHER]       = "Alien Queen";
    stringsObject[OBJECT_ANT]          = "Ant";
    stringsObject[OBJECT_SPIDER]       = "Spider";
    stringsObject[OBJECT_BEE]          = "Wasp";
    stringsObject[OBJECT_WORM]         = "Worm";
    stringsObject[OBJECT_EGG]          = "Egg";
    stringsObject[OBJECT_RUINmobilew1] = "Wreckage";
    stringsObject[OBJECT_RUINmobilew2] = "Wreckage";
    stringsObject[OBJECT_RUINmobilet1] = "Wreckage";
    stringsObject[OBJECT_RUINmobilet2] = "Wreckage";
    stringsObject[OBJECT_RUINmobiler1] = "Wreckage";
    stringsObject[OBJECT_RUINmobiler2] = "Wreckage";
    stringsObject[OBJECT_RUINfactory]  = "Ruin";
    stringsObject[OBJECT_RUINdoor]     = "Ruin";
    stringsObject[OBJECT_RUINsupport]  = "Waste";
    stringsObject[OBJECT_RUINradar]    = "Ruin";
    stringsObject[OBJECT_RUINconvert]  = "Ruin";
    stringsObject[OBJECT_RUINbase]     = "Spaceship ruin";
    stringsObject[OBJECT_RUINhead]     = "Spaceship ruin";
    stringsObject[OBJECT_APOLLO1]      = "Remains of Apollo mission";
    stringsObject[OBJECT_APOLLO3]      = "Remains of Apollo mission";
    stringsObject[OBJECT_APOLLO4]      = "Remains of Apollo mission";
    stringsObject[OBJECT_APOLLO5]      = "Remains of Apollo mission";
    stringsObject[OBJECT_APOLLO2]      = "Lunar Roving Vehicle";



    stringsErr[ERR_GENERIC]         = "Internal error - tell the developers";
    stringsErr[ERR_CMD]             = "Unknown command";
    stringsErr[ERR_MANIP_VEH]       = "Inappropriate bot";
    stringsErr[ERR_MANIP_FLY]       = "Impossible when flying";
    stringsErr[ERR_MANIP_BUSY]      = "Already carrying something";
    stringsErr[ERR_MANIP_NIL]       = "Nothing to grab";
    stringsErr[ERR_MANIP_MOTOR]     = "Impossible when moving";
    stringsErr[ERR_MANIP_OCC]       = "Place occupied";
    stringsErr[ERR_MANIP_FRIEND]    = "No other robot";
    stringsErr[ERR_MANIP_RADIO]     = "You can not carry a radioactive object";
    stringsErr[ERR_MANIP_WATER]     = "You can not carry an object under water";
    stringsErr[ERR_MANIP_EMPTY]     = "Nothing to drop";
    stringsErr[ERR_BUILD_FLY]       = "Impossible when flying";
    stringsErr[ERR_BUILD_WATER]     = "Impossible under water";
    stringsErr[ERR_BUILD_ENERGY]    = "Not enough energy";
    stringsErr[ERR_BUILD_METALAWAY] = "Titanium too far away";
    stringsErr[ERR_BUILD_METALNEAR] = "Titanium too close";
    stringsErr[ERR_BUILD_METALINEX] = "No titanium around";
    stringsErr[ERR_BUILD_FLAT]      = "Ground not flat enough";
    stringsErr[ERR_BUILD_FLATLIT]   = "Flat ground not large enough";
    stringsErr[ERR_BUILD_BUSY]      = "Place occupied";
    stringsErr[ERR_BUILD_BASE]      = "Too close to space ship";
    stringsErr[ERR_BUILD_NARROW]    = "Too close to a building";
    stringsErr[ERR_BUILD_MOTOR]     = "Impossible when moving";
    stringsErr[ERR_SEARCH_FLY]      = "Impossible when flying";
    stringsErr[ERR_BUILD_DISABLED]  = "Can not produce this object in this mission";
    stringsErr[ERR_BUILD_RESEARCH]  = "Can not produce not researched object";
    stringsErr[ERR_SEARCH_VEH]      = "Inappropriate bot";
    stringsErr[ERR_SEARCH_MOTOR]    = "Impossible when moving";
    stringsErr[ERR_TERRA_VEH]       = "Inappropriate bot";
    stringsErr[ERR_TERRA_ENERGY]    = "Not enough energy";
    stringsErr[ERR_TERRA_FLOOR]     = "Ground inappropriate";
    stringsErr[ERR_TERRA_BUILDING]  = "Building too close";
    stringsErr[ERR_TERRA_OBJECT]    = "Object too close";
    stringsErr[ERR_RECOVER_VEH]     = "Inappropriate bot";
    stringsErr[ERR_RECOVER_ENERGY]  = "Not enough energy";
    stringsErr[ERR_RECOVER_NULL]    = "Nothing to recycle";
    stringsErr[ERR_SHIELD_VEH]      = "Inappropriate bot";
    stringsErr[ERR_SHIELD_ENERGY]   = "No more energy";
    stringsErr[ERR_MOVE_IMPOSSIBLE] = "Error in instruction move";
    stringsErr[ERR_FIND_IMPOSSIBLE] = "Object not found";
    stringsErr[ERR_GOTO_IMPOSSIBLE] = "Goto: inaccessible destination";
    stringsErr[ERR_GOTO_ITER]       = "Goto: inaccessible destination";
    stringsErr[ERR_GOTO_BUSY]       = "Goto: destination occupied";
    stringsErr[ERR_FIRE_VEH]        = "Inappropriate bot";
    stringsErr[ERR_FIRE_ENERGY]     = "Not enough energy";
    stringsErr[ERR_FIRE_FLY]        = "Impossible when flying";
    stringsErr[ERR_CONVERT_EMPTY]   = "No titanium ore to convert";
    stringsErr[ERR_DERRICK_NULL]    = "No ore in the subsoil";
    stringsErr[ERR_STATION_NULL]    = "No energy in the subsoil";
    stringsErr[ERR_TOWER_POWER]     = "No power cell";
    stringsErr[ERR_TOWER_ENERGY]    = "No more energy";
    stringsErr[ERR_RESEARCH_POWER]  = "No power cell";
    stringsErr[ERR_RESEARCH_ENERGY] = "Not enough energy";
    stringsErr[ERR_RESEARCH_TYPE]   = "Inappropriate cell type";
    stringsErr[ERR_RESEARCH_ALREADY]= "Research program already performed";
    stringsErr[ERR_ENERGY_NULL]     = "No energy in the subsoil";
    stringsErr[ERR_ENERGY_LOW]      = "Not enough energy yet";
    stringsErr[ERR_ENERGY_EMPTY]    = "No titanium to transform";
    stringsErr[ERR_ENERGY_BAD]      = "Transforms only titanium";
    stringsErr[ERR_BASE_DLOCK]      = "Doors blocked by a robot or another object ";
    stringsErr[ERR_BASE_DHUMAN]     = "You must get on the spaceship to take off ";
    stringsErr[ERR_LABO_NULL]       = "Nothing to analyze";
    stringsErr[ERR_LABO_BAD]        = "Analyzes only organic matter";
    stringsErr[ERR_LABO_ALREADY]    = "Analysis already performed";
    stringsErr[ERR_NUCLEAR_NULL]    = "No energy in the subsoil";
    stringsErr[ERR_NUCLEAR_LOW]     = "Not yet enough energy";
    stringsErr[ERR_NUCLEAR_EMPTY]   = "No uranium to transform";
    stringsErr[ERR_NUCLEAR_BAD]     = "Transforms only uranium";
    stringsErr[ERR_FACTORY_NULL]    = "No titanium";
    stringsErr[ERR_FACTORY_NEAR]    = "Object too close";
    stringsErr[ERR_RESET_NEAR]      = "Place occupied";
    stringsErr[ERR_INFO_NULL]       = "No information exchange post within range";
    stringsErr[ERR_VEH_VIRUS]       = "Program infected by a virus";
    stringsErr[ERR_BAT_VIRUS]       = "Infected by a virus; temporarily out of order";
    stringsErr[ERR_VEH_POWER]       = "No power cell";
    stringsErr[ERR_VEH_ENERGY]      = "No more energy";
    stringsErr[ERR_FLAG_FLY]        = "Impossible when flying";
    stringsErr[ERR_FLAG_WATER]      = "Impossible when swimming";
    stringsErr[ERR_FLAG_MOTOR]      = "Impossible when moving";
    stringsErr[ERR_FLAG_BUSY]       = "Impossible when carrying an object";
    stringsErr[ERR_FLAG_CREATE]     = "Too many flags of this color (maximum 5)";
    stringsErr[ERR_FLAG_PROXY]      = "Too close to an existing flag";
    stringsErr[ERR_FLAG_DELETE]     = "No flag nearby";
    stringsErr[ERR_DESTROY_NOTFOUND]= "Not found anything to destroy";
    stringsErr[ERR_WRONG_OBJ]       = "Inappropriate object";
    stringsErr[ERR_MISSION_NOTERM]  = "The mission is not accomplished yet (press \\key help; for more details)";
    stringsErr[ERR_DELETEMOBILE]    = "Bot destroyed";
    stringsErr[ERR_DELETEBUILDING]  = "Building destroyed";
    stringsErr[ERR_TOOMANY]         = "Can not create this; there are too many objects";
    stringsErr[ERR_OBLIGATORYTOKEN] = "\"%s\" missing in this exercise";
    stringsErr[ERR_PROHIBITEDTOKEN] = "Do not use in this exercise";

    stringsErr[INFO_BUILD]          = "Building completed";
    stringsErr[INFO_CONVERT]        = "Titanium available";
    stringsErr[INFO_RESEARCH]       = "Research program completed";
    stringsErr[INFO_RESEARCHTANK]   = "Plans for tracked robots available ";
    stringsErr[INFO_RESEARCHFLY]    = "You can fly with the keys (\\key gup;) and (\\key gdown;)";
    stringsErr[INFO_RESEARCHTHUMP]  = "Plans for thumper available";
    stringsErr[INFO_RESEARCHCANON]  = "Plans for shooter available";
    stringsErr[INFO_RESEARCHTOWER]  = "Plans for defense tower available";
    stringsErr[INFO_RESEARCHPHAZER] = "Plans for phazer shooter available";
    stringsErr[INFO_RESEARCHSHIELD] = "Plans for shielder available";
    stringsErr[INFO_RESEARCHATOMIC] = "Plans for nuclear power plant available";
    stringsErr[INFO_FACTORY]        = "New bot available";
    stringsErr[INFO_LABO]           = "Analysis performed";
    stringsErr[INFO_ENERGY]         = "Power cell available";
    stringsErr[INFO_NUCLEAR]        = "Nuclear power cell available";
    stringsErr[INFO_FINDING]        = "You found a usable object";
    stringsErr[INFO_MARKPOWER]      = "Found a site for power station";
    stringsErr[INFO_MARKURANIUM]    = "Found a site for a derrick";
    stringsErr[INFO_MARKSTONE]      = "Found a site for a derrick";
    stringsErr[INFO_MARKKEYa]       = "Found a site for a derrick";
    stringsErr[INFO_MARKKEYb]       = "Found a site for a derrick";
    stringsErr[INFO_MARKKEYc]       = "Found a site for a derrick";
    stringsErr[INFO_MARKKEYd]       = "Found a site for a derrick";
    stringsErr[INFO_WIN]            = "<<< Well done; mission accomplished >>>";
    stringsErr[INFO_LOST]           = "<<< Sorry; mission failed >>>";
    stringsErr[INFO_LOSTq]          = "<<< Sorry; mission failed >>>";
    stringsErr[INFO_WRITEOK]        = "Current mission saved";
    stringsErr[INFO_DELETEPATH]     = "Checkpoint crossed";
    stringsErr[INFO_DELETEMOTHER]   = "Alien Queen killed";
    stringsErr[INFO_DELETEANT]      = "Ant fatally wounded";
    stringsErr[INFO_DELETEBEE]      = "Wasp fatally wounded";
    stringsErr[INFO_DELETEWORM]     = "Worm fatally wounded";
    stringsErr[INFO_DELETESPIDER]   = "Spider fatally wounded";
    stringsErr[INFO_BEGINSATCOM]    = "Press \\key help; to read instructions on your SatCom";



    stringsCbot[TX_OPENPAR]       = "Opening bracket missing";
    stringsCbot[TX_CLOSEPAR]      = "Closing bracket missing ";
    stringsCbot[TX_NOTBOOL]       = "The expression must return a boolean value";
    stringsCbot[TX_UNDEFVAR]      = "Variable not declared";
    stringsCbot[TX_BADLEFT]       = "Assignment impossible";
    stringsCbot[TX_ENDOF]         = "Semicolon terminator missing";
    stringsCbot[TX_OUTCASE]       = "Instruction \"case\" outside a block \"switch\"";
    stringsCbot[TX_NOTERM]        = "Instructions after the final closing brace";
    stringsCbot[TX_CLOSEBLK]      = "End of block missing";
    stringsCbot[TX_ELSEWITHOUTIF] = "Instruction \"else\" without corresponding \"if\" ";
    stringsCbot[TX_OPENBLK]       = "Opening brace missing ";
    stringsCbot[TX_BADTYPE]       = "Wrong type for the assignment";
    stringsCbot[TX_REDEFVAR]      = "A variable can not be declared twice";
    stringsCbot[TX_BAD2TYPE]      = "The types of the two operands are incompatible ";
    stringsCbot[TX_UNDEFCALL]     = "Unknown function";
    stringsCbot[TX_MISDOTS]       = "Sign \" : \" missing";
    stringsCbot[TX_WHILE]         = "Keyword \"while\" missing";
    stringsCbot[TX_BREAK]         = "Instruction \"break\" outside a loop";
    stringsCbot[TX_LABEL]         = "A label must be followed by \"for\"; \"while\"; \"do\" or \"switch\"";
    stringsCbot[TX_NOLABEL]       = "This label does not exist";
    stringsCbot[TX_NOCASE]        = "Instruction \"case\" missing";
    stringsCbot[TX_BADNUM]        = "Number missing";
    stringsCbot[TX_VOID]          = "Void parameter";
    stringsCbot[TX_NOTYP]         = "Type declaration missing";
    stringsCbot[TX_NOVAR]         = "Variable name missing";
    stringsCbot[TX_NOFONC]        = "Function name missing";
    stringsCbot[TX_OVERPARAM]     = "Too many parameters";
    stringsCbot[TX_REDEF]         = "Function already exists";
    stringsCbot[TX_LOWPARAM]      = "Parameters missing ";
    stringsCbot[TX_BADPARAM]      = "No function with this name accepts this kind of parameter";
    stringsCbot[TX_NUMPARAM]      = "No function with this name accepts this number of parameters";
    stringsCbot[TX_NOITEM]        = "This is not a member of this class";
    stringsCbot[TX_DOT]           = "This object is not a member of a class";
    stringsCbot[TX_NOCONST]       = "Appropriate constructor missing";
    stringsCbot[TX_REDEFCLASS]    = "This class already exists";
    stringsCbot[TX_CLBRK]         = "\" ] \" missing";
    stringsCbot[TX_RESERVED]      = "Reserved keyword of CBOT language";
    stringsCbot[TX_BADNEW]        = "Bad argument for \"new\"";
    stringsCbot[TX_OPBRK]         = "\" [ \" expected";
    stringsCbot[TX_BADSTRING]     = "String missing";
    stringsCbot[TX_BADINDEX]      = "Incorrect index type";
    stringsCbot[TX_PRIVATE]       = "Private element";
    stringsCbot[TX_NOPUBLIC]      = "Public required";
    stringsCbot[TX_DIVZERO]       = "Dividing by zero";
    stringsCbot[TX_NOTINIT]       = "Variable not initialized";
    stringsCbot[TX_BADTHROW]      = "Negative value rejected by \"throw\"";
    stringsCbot[TX_NORETVAL]      = "The function returned no value ";
    stringsCbot[TX_NORUN]         = "No function running";
    stringsCbot[TX_NOCALL]        = "Calling an unknown function";
    stringsCbot[TX_NOCLASS]       = "This class does not exist";
    stringsCbot[TX_NULLPT]        = "Unknown Object";
    stringsCbot[TX_OPNAN]         = "Operation impossible with value \"nan\"";
    stringsCbot[TX_OUTARRAY]      = "Access beyond array limit";
    stringsCbot[TX_STACKOVER]     = "Stack overflow";
    stringsCbot[TX_DELETEDPT]     = "Illegal object";
    stringsCbot[TX_FILEOPEN]      = "Can't open file";
    stringsCbot[TX_NOTOPEN]       = "File not open";
    stringsCbot[TX_ERRREAD]       = "Read error";
    stringsCbot[TX_ERRWRITE]      = "Write error";
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
    int     i;

    for ( i=0 ; i<22 ; i++ )
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

static void PutKeyName(char* dst, const char* src)
{
    InputSlot key;
    char    name[50];
    int     s, d, n;
    unsigned int res;

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
                res = CRobotMain::GetInstancePointer()->GetInputBinding(key).primary;
                if (res != KEY_INVALID)
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
            {
                GetLogger()->Trace("GetResource event num out of range: %d\n", num); // TODO: fix later
                return "";
            }
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

bool GetResource(ResType type, int num, char* text)
{
    const char *tmpl = GetResourceBase(type, num);

    if (!tmpl)
    {
        text[0] = 0;
        return false;
    }

    PutKeyName(text, tmpl);
    return true;
}

