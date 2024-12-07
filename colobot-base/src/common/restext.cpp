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


#include "common/restext.h"

#include "CBot/CBot.h"

#include "app/input.h"

#include "common/error.h"
#include "common/event.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "object/object_type.h"

#include <SDL_keyboard.h>

#include <libintl.h>

const char* stringsText[RT_MAX]         = { nullptr };
const char* stringsEvent[EVENT_STD_MAX] = { nullptr };
const char* stringsObject[OBJECT_MAX]   = { nullptr };
const char* stringsErr[ERR_MAX]         = { nullptr };
const char* stringsCbot[CBot::CBotErrMAX]         = { nullptr };

static int InputSlot2EventType(InputSlot s)
{
    static_assert(EVENT_INTERFACE_KEY + static_cast<int>(INPUT_SLOT_MAX) < EVENT_INTERFACE_KEY_END);
    return EVENT_INTERFACE_KEY + static_cast<int>(s);
}

/* Macro to mark which texts are translatable by gettext
 * It doesn't do anything at compile-time, as all texts represented here are used later
 * in explicit call to gettext(), but it is used by xgettext executable to filter extracted
 * texts from this file.
 */
#define TR(x) x

/* Please run `cmake --build <path_to_build_folder> --target update-pot`
 * after changing this file in order to update translation files. Thank you.
 */

void InitializeRestext()
{
    stringsText[RT_WINDOW_MAXIMIZED] = TR("Maximize");
    stringsText[RT_WINDOW_MINIMIZED] = TR("Minimize");
    stringsText[RT_WINDOW_STANDARD]  = TR("Normal size");
    stringsText[RT_WINDOW_CLOSE]     = TR("Close");

    stringsText[RT_STUDIO_TITLE]     = TR("Program editor");
    stringsText[RT_SCRIPT_NEW]       = TR("New");
    stringsText[RT_NAME_DEFAULT]     = TR("Player");
    stringsText[RT_IO_NEW]           = TR("New ...");
    stringsText[RT_KEY_OR]           = TR(" or ");

    stringsText[RT_TITLE_APPNAME]    = TR("Colobot: Gold Edition");
    stringsText[RT_TITLE_TRAINER]    = TR("Programming exercises");
    stringsText[RT_TITLE_DEFI]       = TR("Challenges");
    stringsText[RT_TITLE_MISSION]    = TR("Missions");
    stringsText[RT_TITLE_FREE]       = TR("Free game");
    stringsText[RT_TITLE_USER]       = TR("User levels");
    stringsText[RT_TITLE_CODE_BATTLES] = TR("Code battles");
    stringsText[RT_TITLE_SETUP]      = TR("Options");
    stringsText[RT_TITLE_NAME]       = TR("Player's name");
    stringsText[RT_TITLE_PERSO]      = TR("Customize your appearance");
    stringsText[RT_TITLE_WRITE]      = TR("Save the current mission");
    stringsText[RT_TITLE_READ]       = TR("Load a saved mission");
    stringsText[RT_TITLE_PLUS]       = TR("Missions+");
    stringsText[RT_TITLE_MODS]       = TR("Mods");

    stringsText[RT_PLAY_CHAP_CHAPTERS]   = TR("Chapters:");
    stringsText[RT_PLAY_CHAP_PLANETS]    = TR("Planets:");
    stringsText[RT_PLAY_CHAP_USERLVL]    = TR("Custom levels:");
    stringsText[RT_PLAY_LIST_LEVELS]     = TR("Levels in this chapter:");
    stringsText[RT_PLAY_LIST_EXERCISES]  = TR("Exercises in the chapter:");
    stringsText[RT_PLAY_LIST_CHALLENGES] = TR("Challenges in the chapter:");
    stringsText[RT_PLAY_LIST_MISSIONS]   = TR("Missions on this planet:");
    stringsText[RT_PLAY_LIST_FREEGAME]   = TR("Free game on this planet:");
    stringsText[RT_PLAY_RESUME]          = TR("Summary:");

    stringsText[RT_SETUP_MODE]       = TR("Resolution:");
    stringsText[RT_SETUP_KEY1]       = TR("1) First click on the key you want to redefine.");
    stringsText[RT_SETUP_KEY2]       = TR("2) Then press the key you want to use instead.");

    stringsText[RT_PERSO_FACE]       = TR("Face type:");
    stringsText[RT_PERSO_GLASSES]    = TR("Eyeglasses:");
    stringsText[RT_PERSO_HAIR]       = TR("Hair color:");
    stringsText[RT_PERSO_COMBI]      = TR("Suit color:");
    stringsText[RT_PERSO_BAND]       = TR("Strip color:");

    stringsText[RT_DIALOG_ABORT]     = TR("Abort\\Abort the current mission");
    stringsText[RT_DIALOG_CONTINUE]  = TR("Continue\\Continue the current mission");
    stringsText[RT_DIALOG_DELOBJ]    = TR("Do you really want to destroy the selected building?");
    stringsText[RT_DIALOG_DELGAME]   = TR("Do you want to delete %s's saved games?");
    stringsText[RT_DIALOG_YES]       = TR("Yes");
    stringsText[RT_DIALOG_NO]        = TR("No");
    stringsText[RT_DIALOG_LOADING]   = TR("LOADING");
    stringsText[RT_DIALOG_OK]        = TR("OK");
    stringsText[RT_DIALOG_NOUSRLVL_TITLE] = TR("No userlevels installed!");
    stringsText[RT_DIALOG_NOUSRLVL_TEXT]  = TR("This menu is for userlevels from mods, but you didn't install any");
    stringsText[RT_DIALOG_OPEN_PATH_FAILED_TITLE] = TR("Could not open the file explorer!");
    stringsText[RT_DIALOG_OPEN_PATH_FAILED_TEXT] = TR("The path %s could not be opened in a file explorer.");
    stringsText[RT_DIALOG_OPEN_WEBSITE_FAILED_TITLE] = TR("Could not open the web browser!");
    stringsText[RT_DIALOG_OPEN_WEBSITE_FAILED_TEXT] = TR("The address %s could not be opened in a web browser.");
    stringsText[RT_DIALOG_CHANGES_QUESTION] = TR("There are unsaved changes. Do you want to save them before leaving?");
    stringsText[RT_DIALOG_OPEN_UNSUPPORTED] = TR("This save file is from a different version of the game and might not work correctly.\nDo you want to open it anyway?");

    stringsText[RT_STUDIO_LISTTT]    = TR("Keyword help(\\key cbot;)");
    stringsText[RT_STUDIO_COMPOK]    = TR("Compilation ok (0 errors)");
    stringsText[RT_STUDIO_PROGSTOP]  = TR("Program finished");
    stringsText[RT_STUDIO_CLONED]    = TR("Program cloned");

    stringsText[RT_PROGRAM_READONLY] = TR("This program is read-only, clone it to edit");
    stringsText[RT_PROGRAM_EXAMPLE]  = TR("This is example code that cannot be run directly");

    stringsText[RT_IO_OPEN]          = TR("Open");
    stringsText[RT_IO_SAVE]          = TR("Save");
    stringsText[RT_IO_LIST]          = TR("Folder: %s");
    stringsText[RT_IO_NAME]          = TR("Name:");
    stringsText[RT_IO_DIR]           = TR("Folder:");
    stringsText[RT_IO_PRIVATE]       = TR("Private\\Private folder");
    stringsText[RT_IO_PUBLIC]        = TR("Public\\Common folder");
    stringsText[RT_IO_REPLACE]       = TR("Overwrite existing file?");
    stringsText[RT_IO_SELECT_DIR]    = TR("Select Folder");

    stringsText[RT_GENERIC_DEV1]     = TR("Original game developed by:");
    stringsText[RT_GENERIC_DEV2]     = TR("epsitec.com");
    stringsText[RT_GENERIC_EDIT1]    = TR("Gold Edition development by:");
    stringsText[RT_GENERIC_EDIT2]    = TR("colobot.info");

    stringsText[RT_INTERFACE_REC]    = TR("Recorder");

    stringsText[RT_LOADING_INIT]           = TR("Starting...");
    stringsText[RT_LOADING_PROCESSING]     = TR("Processing level file");
    stringsText[RT_LOADING_LEVEL_SETTINGS] = TR("Loading basic level settings");
    stringsText[RT_LOADING_MUSIC]          = TR("Loading music");
    stringsText[RT_LOADING_TERRAIN]        = TR("Loading terrain");
    stringsText[RT_LOADING_OBJECTS]        = TR("Loading objects");
    stringsText[RT_LOADING_OBJECTS_SAVED]  = TR("Restoring saved objects");
    stringsText[RT_LOADING_CBOT_SAVE]      = TR("Restoring CBot execution state");
    stringsText[RT_LOADING_FINISHED]       = TR("Loading finished!");
    stringsText[RT_LOADING_TERRAIN_RELIEF] = TR("Terrain relief");
    stringsText[RT_LOADING_TERRAIN_RES]    = TR("Resources");
    stringsText[RT_LOADING_TERRAIN_TEX]    = TR("Textures");
    stringsText[RT_LOADING_TERRAIN_GEN]    = TR("Generating");

    stringsText[RT_SCOREBOARD_RESULTS]     = TR("Results");
    stringsText[RT_SCOREBOARD_RESULTS_TEXT]= TR("The battle has ended");
    stringsText[RT_SCOREBOARD_RESULTS_TIME]= TR("Time: %s");
    stringsText[RT_SCOREBOARD_RESULTS_LINE]= TR("%s: %d pts");

    stringsText[RT_MOD_LIST]               = TR("Mods:");
    stringsText[RT_MOD_DETAILS]            = TR("Information:");
    stringsText[RT_MOD_SUMMARY]            = TR("Description:");
    stringsText[RT_MOD_ENABLE]             = TR("Enable\\Enable the selected mod");
    stringsText[RT_MOD_DISABLE]            = TR("Disable\\Disable the selected mod");
    stringsText[RT_MOD_UNKNOWN_AUTHOR]     = TR("Unknown author");
    stringsText[RT_MOD_AUTHOR_FIELD_NAME]  = TR("by");
    stringsText[RT_MOD_VERSION_FIELD_NAME] = TR("Version");
    stringsText[RT_MOD_WEBSITE_FIELD_NAME] = TR("Website");
    stringsText[RT_MOD_CHANGES_FIELD_NAME] = TR("Changes");
    stringsText[RT_MOD_NO_SUMMARY]         = TR("No description.");
    stringsText[RT_MOD_NO_CHANGES]         = TR("No changes.");

    stringsEvent[EVENT_LABEL_CODE_BATTLE]   = TR("Code battle");

    stringsEvent[EVENT_BUTTON_OK]           = TR("OK");
    stringsEvent[EVENT_BUTTON_CANCEL]       = TR("Cancel");
    stringsEvent[EVENT_BUTTON_NEXT]         = TR("Next");
    stringsEvent[EVENT_BUTTON_PREV]         = TR("Previous");

    stringsEvent[EVENT_DIALOG_OK]           = TR("OK");
    stringsEvent[EVENT_DIALOG_CANCEL]       = TR("Cancel");
    stringsEvent[EVENT_DIALOG_NEWDIR]       = TR("New Folder");

    stringsEvent[EVENT_INTERFACE_TRAINER]   = TR("Exercises\\Programming exercises");
    stringsEvent[EVENT_INTERFACE_DEFI]      = TR("Challenges\\Programming challenges");
    stringsEvent[EVENT_INTERFACE_MISSION]   = TR("Missions\\Select mission");
    stringsEvent[EVENT_INTERFACE_FREE]      = TR("Free game\\Free game without a specific goal");
    stringsEvent[EVENT_INTERFACE_CODE_BATTLES] = TR("Code battles\\Program your robot to be the best of them all!");
    stringsEvent[EVENT_INTERFACE_USER]      = TR("Custom levels\\Levels from mods created by the users");
    stringsEvent[EVENT_INTERFACE_SATCOM]    = TR("SatCom");
    stringsEvent[EVENT_INTERFACE_MODS]      = TR("Mods\\Mod manager");
    stringsEvent[EVENT_INTERFACE_NAME]      = TR("Change player\\Change player");
    stringsEvent[EVENT_INTERFACE_SETUP]     = TR("Options\\Preferences");
    stringsEvent[EVENT_INTERFACE_AGAIN]     = TR("Restart\\Restart the mission from the beginning");
    stringsEvent[EVENT_INTERFACE_WRITE]     = TR("Save\\Save the current mission");
    stringsEvent[EVENT_INTERFACE_READ]      = TR("Load\\Load a saved mission");
    stringsEvent[EVENT_INTERFACE_ABORT]     = TR("\\Return to Colobot: Gold Edition");
    stringsEvent[EVENT_INTERFACE_QUIT]      = TR("Quit\\Quit Colobot: Gold Edition");
    stringsEvent[EVENT_INTERFACE_BACK]      = TR("<<  Back  \\Back to the previous screen");
    stringsEvent[EVENT_INTERFACE_PLUS]      = TR("+\\Missions with bonus content and optional challenges");
    stringsEvent[EVENT_INTERFACE_PLAY]      = TR("Play\\Start mission!");
    stringsEvent[EVENT_INTERFACE_WORKSHOP]  = TR("Workshop\\Open the workshop to search for mods");
    stringsEvent[EVENT_INTERFACE_MODS_DIR]  = TR("Open Directory\\Open the mods directory");
    stringsEvent[EVENT_INTERFACE_MODS_APPLY] = TR("Apply\\Apply the current mod configuration");
    stringsEvent[EVENT_INTERFACE_MOD_MOVE_UP] = TR("Up\\Move the selected mod up so it's loaded sooner (mods may overwrite files from the mods above them)");
    stringsEvent[EVENT_INTERFACE_MOD_MOVE_DOWN] = TR("Down\\Move the selected mod down so it's loaded later (mods may overwrite files from the mods above them)");
    stringsEvent[EVENT_INTERFACE_MODS_REFRESH] = TR("Refresh\\Refresh the list of currently installed mods");
    stringsEvent[EVENT_INTERFACE_SETUPd]    = TR("Device\\Driver and resolution settings");
    stringsEvent[EVENT_INTERFACE_SETUPg]    = TR("Graphics\\Graphics settings");
    stringsEvent[EVENT_INTERFACE_SETUPp]    = TR("Game\\Game settings");
    stringsEvent[EVENT_INTERFACE_SETUPc]    = TR("Controls\\Keyboard, joystick and mouse settings");
    stringsEvent[EVENT_INTERFACE_SETUPs]    = TR("Sound\\Music and game sound volume");
    stringsEvent[EVENT_INTERFACE_DEVICE]    = TR("Unit");
    stringsEvent[EVENT_INTERFACE_RESOL]     = TR("Resolution");
    stringsEvent[EVENT_INTERFACE_FULL]      = TR("Full screen\\Full screen or window mode");
    stringsEvent[EVENT_INTERFACE_APPLY]     = TR("Apply changes\\Activates the changed settings");

    stringsEvent[EVENT_INTERFACE_DIRTY]     = TR("Dust\\Dust and dirt on bots and buildings");
    stringsEvent[EVENT_INTERFACE_FOG]       = TR("Fog\\Fog");
    stringsEvent[EVENT_INTERFACE_LIGHT]     = TR("Dynamic lighting\\Mobile light sources");
    stringsEvent[EVENT_INTERFACE_PARTI]     = TR("Number of particles\\Explosions, dust, reflections, etc.");
    stringsEvent[EVENT_INTERFACE_CLIP]      = TR("Render distance\\Maximum visibility");
    stringsEvent[EVENT_INTERFACE_PAUSE_BLUR]= TR("Pause blur\\Blur the background on the pause screen");
    stringsEvent[EVENT_INTERFACE_RAIN]      = TR("Particles in the interface\\Steam clouds and sparks in the interface");
    stringsEvent[EVENT_INTERFACE_GLINT]     = TR("Reflections on the buttons \\Shiny buttons");
    stringsEvent[EVENT_INTERFACE_TOOLTIP]   = TR("Help balloons\\Explain the function of the buttons");
    stringsEvent[EVENT_INTERFACE_MOVIES]    = TR("Film sequences\\Films before and after the missions");
    stringsEvent[EVENT_INTERFACE_SCROLL]    = TR("Camera border scrolling\\Scrolling when the mouse touches right or left border");
    stringsEvent[EVENT_INTERFACE_INVERTX]   = TR("Mouse inversion X\\Inversion of the scrolling direction on the X axis");
    stringsEvent[EVENT_INTERFACE_INVERTY]   = TR("Mouse inversion Y\\Inversion of the scrolling direction on the Y axis");
    stringsEvent[EVENT_INTERFACE_EFFECT]    = TR("Quake at explosions\\The screen shakes at explosions");
    stringsEvent[EVENT_INTERFACE_BGPAUSE]   = TR("Pause in background\\Pause the game when the window is unfocused");
    stringsEvent[EVENT_INTERFACE_BGMUTE]    = TR("Mute sounds in background\\Mute all game sounds when the window is unfocused");
    stringsEvent[EVENT_INTERFACE_EDITMODE]  = TR("Automatic indent\\When program editing");
    stringsEvent[EVENT_INTERFACE_EDITVALUE] = TR("Big indent\\Indent 2 or 4 spaces per level defined by braces");
    stringsEvent[EVENT_INTERFACE_SOLUCE4]   = TR("Access to solutions\\Show program \"4: Solution\" in the exercises");
    stringsEvent[EVENT_INTERFACE_BLOOD]     = TR("Blood\\Display blood when the astronaut is hit");
    stringsEvent[EVENT_INTERFACE_AUTOSAVE_ENABLE]   = TR("Autosave\\Enables autosave");
    stringsEvent[EVENT_INTERFACE_AUTOSAVE_INTERVAL] = TR("Autosave interval\\How often your game will autosave");
    stringsEvent[EVENT_INTERFACE_AUTOSAVE_SLOTS]    = TR("Autosave slots\\How many autosave slots you'll have");
    stringsEvent[EVENT_INTERFACE_TEXTURE_FILTER]    = TR("Texture filtering\\Texture filtering");
    stringsEvent[EVENT_INTERFACE_TEXTURE_MIPMAP]    = TR("Mipmap level\\Mipmap level");
    stringsEvent[EVENT_INTERFACE_TEXTURE_ANISOTROPY]= TR("Anisotropy level\\Anisotropy level");
    stringsEvent[EVENT_INTERFACE_MSAA]              = TR("MSAA\\Multisample anti-aliasing");
    stringsEvent[EVENT_INTERFACE_SHADOW_SPOTS]      = TR("Simple shadows\\Shadows spots on the ground");
    stringsEvent[EVENT_INTERFACE_SHADOW_MAPPING]    = TR("Dynamic shadows\\Beautiful shadows!");
    stringsEvent[EVENT_INTERFACE_SHADOW_MAPPING_QUALITY]= TR("Dynamic shadows ++\\Dynamic shadows + self shadowing");
    stringsEvent[EVENT_INTERFACE_SHADOW_MAPPING_BUFFER] = TR("Shadow resolution\\Higher means better range and quality, but slower");
    stringsEvent[EVENT_INTERFACE_VSYNC]     = TR("Vertical Synchronization\\Limits the number of frames per second to display frequency");

    stringsEvent[EVENT_INTERFACE_KDEF]      = TR("Standard controls\\Standard key functions");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_LEFT)]        = TR("Turn left\\turns the bot to the left");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_RIGHT)]       = TR("Turn right\\turns the bot to the right");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_UP)]          = TR("Forward\\Moves forward");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_DOWN)]        = TR("Backward\\Moves backward");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_GUP)]         = TR("Climb\\Increases the power of the jet");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_GDOWN)]       = TR("Descend\\Reduces the power of the jet");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAMERA)]      = TR("Change camera\\Switches between onboard camera and following camera");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_DESEL)]       = TR("Previous object\\Selects the previous object");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_ACTION)]      = TR("Standard action\\Standard action of the bot (take/grab, shoot, sniff, etc)");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_LEFT)]    = TR("Camera left\\Turns the camera left");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_RIGHT)]   = TR("Camera right\\Turns the camera right");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_UP)]      = TR("Camera up\\Turns the camera up");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_DOWN)]    = TR("Camera down\\Turns the camera down");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_NEAR)]    = TR("Camera closer\\Moves the camera forward");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_AWAY)]    = TR("Camera back\\Moves the camera backward");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CAM_ALT)]     = TR("Alternative camera mode\\Move sideways instead of rotating (in free camera)");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_NEXT)]        = TR("Next object\\Selects the next object");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_HUMAN)]       = TR("Select the astronaut\\Selects the astronaut");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_QUIT)]        = TR("Quit\\Quit the current mission or exercise");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_HELP)]        = TR("Instructions\\Shows the instructions for the current mission");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_PROG)]        = TR("Programming help\\Gives more detailed help with programming");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_VISIT)]       = TR("Origin of last message\\Shows where the last message was sent from");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_SPEED_DEC)]   = TR("Lower speed\\Decrease speed by half");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_SPEED_RESET)] = TR("Standard speed\\Reset speed to normal");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_SPEED_INC)]   = TR("Higher speed\\Doubles speed");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_QUICKSAVE)]   = TR("Quick save\\Immediately save game");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_QUICKLOAD)]   = TR("Quick load\\Immediately load game");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_PAUSE)]       = TR("Pause\\Pause the game without opening menu");
    stringsEvent[InputSlot2EventType(INPUT_SLOT_CMDLINE)]     = TR("Cheat console\\Show cheat console");

    stringsEvent[EVENT_INTERFACE_VOLSOUND]  = TR("Sound effects:\\Volume of engines, voice, shooting, etc.");
    stringsEvent[EVENT_INTERFACE_VOLMUSIC]  = TR("Background sound:\\Volume of audio tracks");

    stringsEvent[EVENT_INTERFACE_MIN]       = TR("Lowest\\Minimum graphic quality (highest frame rate)");
    stringsEvent[EVENT_INTERFACE_NORM]      = TR("Normal\\Normal graphic quality");
    stringsEvent[EVENT_INTERFACE_MAX]       = TR("Highest\\Highest graphic quality (lowest frame rate)");

    stringsEvent[EVENT_INTERFACE_SILENT]    = TR("Mute\\No sound");
    stringsEvent[EVENT_INTERFACE_NOISY]     = TR("Normal\\Normal sound volume");

    stringsEvent[EVENT_INTERFACE_SOLUCE]    = TR("Access to solution\\Shows the solution (detailed instructions for missions)");
    stringsEvent[EVENT_INTERFACE_JOYSTICK_X_INVERT] = TR("Invert\\Invert values on this axis");
    stringsEvent[EVENT_INTERFACE_JOYSTICK_Y_INVERT] = TR("Invert\\Invert values on this axis");
    stringsEvent[EVENT_INTERFACE_JOYSTICK_Z_INVERT] = TR("Invert\\Invert values on this axis");
    stringsEvent[EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT] = TR("Invert\\Invert values on this axis");
    stringsEvent[EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT] = TR("Invert\\Invert values on this axis");
    stringsEvent[EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT] = TR("Invert\\Invert values on this axis");

    stringsEvent[EVENT_INTERFACE_PLUS_TRAINER]  = TR("Space Programmer\\Disables radio-control");
    stringsEvent[EVENT_INTERFACE_PLUS_RESEARCH] = TR("Space Researcher\\Disables using all previously researched technologies");
    stringsEvent[EVENT_INTERFACE_PLUS_EXPLORER] = TR("Space Explorer\\Disables astronaut abilities");

    stringsEvent[EVENT_INTERFACE_NEDIT]     = TR("\\New player name");
    stringsEvent[EVENT_INTERFACE_NOK]       = TR("OK\\Choose the selected player");
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
    stringsEvent[EVENT_INTERFACE_PFACE2]    = TR("\\Face 2");
    stringsEvent[EVENT_INTERFACE_PFACE3]    = TR("\\Face 3");
    stringsEvent[EVENT_INTERFACE_PFACE4]    = TR("\\Face 4");
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
    stringsEvent[EVENT_OBJECT_BSAFE]        = TR("Build a vault");
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
    stringsEvent[EVENT_OBJECT_FACTORYfb]    = TR("Build a winged builder");
    stringsEvent[EVENT_OBJECT_FACTORYtb]    = TR("Build a tracked builder");
    stringsEvent[EVENT_OBJECT_FACTORYwb]    = TR("Build a wheeled builder");
    stringsEvent[EVENT_OBJECT_FACTORYib]    = TR("Build a legged builder");
    stringsEvent[EVENT_OBJECT_FACTORYrt]    = TR("Build a thumper");
    stringsEvent[EVENT_OBJECT_FACTORYrc]    = TR("Build a phazer shooter");
    stringsEvent[EVENT_OBJECT_FACTORYrr]    = TR("Build a recycler");
    stringsEvent[EVENT_OBJECT_FACTORYrs]    = TR("Build a shielder");
    stringsEvent[EVENT_OBJECT_FACTORYsa]    = TR("Build a subber");
    stringsEvent[EVENT_OBJECT_FACTORYtg]    = TR("Build a target bot");
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
    stringsEvent[EVENT_OBJECT_RBUILDER]     = TR("Run research program for builder");
    stringsEvent[EVENT_OBJECT_RTARGET]      = TR("Run research program for target bot");
    stringsEvent[EVENT_OBJECT_RESET]        = TR("Return to start");
    stringsEvent[EVENT_OBJECT_SEARCH]       = TR("Sniff (\\key action;)");
    stringsEvent[EVENT_OBJECT_TERRAFORM]    = TR("Thump (\\key action;)");
    stringsEvent[EVENT_OBJECT_FIRE]         = TR("Shoot (\\key action;)");
    stringsEvent[EVENT_OBJECT_SPIDEREXPLO]  = TR("Explode (\\key action;)");
    stringsEvent[EVENT_OBJECT_BUILD]        = TR("Build (\\key action;)");
    stringsEvent[EVENT_OBJECT_RECOVER]      = TR("Recycle (\\key action;)");
    stringsEvent[EVENT_OBJECT_BEGSHIELD]    = TR("Extend shield (\\key action;)");
    stringsEvent[EVENT_OBJECT_ENDSHIELD]    = TR("Withdraw shield (\\key action;)");
    stringsEvent[EVENT_OBJECT_DIMSHIELD]    = TR("Shield radius");
    stringsEvent[EVENT_OBJECT_DELSEARCH]    = TR("Delete mark");
    stringsEvent[EVENT_OBJECT_PROGRUN]      = TR("Execute the selected program");
    stringsEvent[EVENT_OBJECT_PROGEDIT]     = TR("Edit the selected program");
    stringsEvent[EVENT_OBJECT_PROGADD]      = TR("Add new program");
    stringsEvent[EVENT_OBJECT_PROGREMOVE]   = TR("Remove selected program");
    stringsEvent[EVENT_OBJECT_PROGCLONE]    = TR("Clone selected program");
    stringsEvent[EVENT_OBJECT_PROGMOVEUP]   = TR("Move selected program up");
    stringsEvent[EVENT_OBJECT_PROGMOVEDOWN] = TR("Move selected program down");
    stringsEvent[EVENT_OBJECT_INFOOK]       = TR("\\SatCom on standby");
    stringsEvent[EVENT_OBJECT_DELETE]       = TR("Destroy the building");
    stringsEvent[EVENT_OBJECT_GENERGY]      = TR("Energy level");
    stringsEvent[EVENT_OBJECT_GSHIELD]      = TR("Shield level");
    stringsEvent[EVENT_OBJECT_GRANGE]       = TR("Jet temperature");
    stringsEvent[EVENT_OBJECT_GPROGRESS]    = TR("Still working ...");
    stringsEvent[EVENT_OBJECT_GRADAR]       = TR("Number of insects detected");
    stringsEvent[EVENT_OBJECT_GINFO]        = TR("Transmitted information");
    stringsEvent[EVENT_OBJECT_MAPZOOM]      = TR("Zoom mini-map");
    stringsEvent[EVENT_OBJECT_CAMERA]       = TR("Camera (\\key camera;)");
    stringsEvent[EVENT_OBJECT_HELP]         = TR("Help about selected object");
    stringsEvent[EVENT_OBJECT_SOLUCE]       = TR("Show the solution");
    stringsEvent[EVENT_OBJECT_SHORTCUT_MODE]= TR("Switch bots <-> buildings");
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
    stringsEvent[EVENT_STUDIO_CLONE]        = TR("Clone program");
    stringsEvent[EVENT_STUDIO_NEW]          = TR("New");
    stringsEvent[EVENT_STUDIO_OPEN]         = TR("Open (Ctrl+O)");
    stringsEvent[EVENT_STUDIO_SAVE]         = TR("Save (Ctrl+S)");
    stringsEvent[EVENT_STUDIO_UNDO]         = TR("Undo (Ctrl+Z)");
    stringsEvent[EVENT_STUDIO_CUT]          = TR("Cut (Ctrl+X)");
    stringsEvent[EVENT_STUDIO_COPY]         = TR("Copy (Ctrl+C)");
    stringsEvent[EVENT_STUDIO_PASTE]        = TR("Paste (Ctrl+V)");
    stringsEvent[EVENT_STUDIO_SIZE]         = TR("Font size");
    stringsEvent[EVENT_STUDIO_TOOL]         = TR("Instructions (\\key help;)");
    stringsEvent[EVENT_STUDIO_HELP]         = TR("Programming help (\\key prog;)");
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
    stringsObject[OBJECT_MOBILErp]     = TR("Practice bot");
    stringsObject[OBJECT_MOBILEst]     = TR("Practice bot");
    stringsObject[OBJECT_MOBILEfa]     = TR("Winged grabber");
    stringsObject[OBJECT_MOBILEta]     = TR("Tracked grabber");
    stringsObject[OBJECT_MOBILEwa]     = TR("Wheeled grabber");
    stringsObject[OBJECT_MOBILEia]     = TR("Legged grabber");
    stringsObject[OBJECT_MOBILEfb]     = TR("Winged builder");
    stringsObject[OBJECT_MOBILEtb]     = TR("Tracked builder");
    stringsObject[OBJECT_MOBILEwb]     = TR("Wheeled builder");
    stringsObject[OBJECT_MOBILEib]     = TR("Legged builder");
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



    stringsErr[ERR_UNKNOWN]         = TR("Internal error - tell the developers");
    stringsErr[ERR_CMD]             = TR("Unknown command");
    stringsErr[ERR_OBJ_BUSY]        = TR("This object is currently busy");
    stringsErr[ERR_MANIP_FLY]       = TR("Impossible when flying");
    stringsErr[ERR_MANIP_BUSY]      = TR("Already carrying something");
    stringsErr[ERR_MANIP_NIL]       = TR("Nothing to grab");
    stringsErr[ERR_MANIP_MOTOR]     = TR("Impossible when moving");
    stringsErr[ERR_MANIP_OCC]       = TR("Place occupied");
    stringsErr[ERR_MANIP_RADIO]     = TR("You can not carry a radioactive object");
    stringsErr[ERR_MANIP_WATER]     = TR("You can not carry an object under water");
    stringsErr[ERR_MANIP_EMPTY]     = TR("Nothing to drop");
    stringsErr[ERR_BUILD_FLY]       = TR("Impossible when flying");
    stringsErr[ERR_BUILD_WATER]     = TR("Impossible under water");
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
    stringsErr[ERR_SEARCH_MOTOR]    = TR("Impossible when moving");
    stringsErr[ERR_TERRA_ENERGY]    = TR("Not enough energy");
    stringsErr[ERR_RECOVER_ENERGY]  = TR("Not enough energy");
    stringsErr[ERR_RECOVER_NULL]    = TR("Nothing to recycle");
    stringsErr[ERR_SHIELD_ENERGY]   = TR("No more energy");
    stringsErr[ERR_MOVE_IMPOSSIBLE] = TR("Error in instruction move");
    stringsErr[ERR_GOTO_IMPOSSIBLE] = TR("Goto: inaccessible destination");
    stringsErr[ERR_GOTO_ITER]       = TR("Goto: inaccessible destination");
    stringsErr[ERR_GOTO_BUSY]       = TR("Goto: destination occupied");
    stringsErr[ERR_FIRE_ENERGY]     = TR("Not enough energy");
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
    stringsErr[ERR_BASE_DLOCK]      = TR("Doors blocked by a robot or another object");
    stringsErr[ERR_BASE_DHUMAN]     = TR("You must get on the spaceship to take off");
    stringsErr[ERR_LABO_NULL]       = TR("Nothing to analyze");
    stringsErr[ERR_LABO_BAD]        = TR("Inappropriate sample");
    stringsErr[ERR_LABO_ALREADY]    = TR("Analysis already performed");
    stringsErr[ERR_NUCLEAR_EMPTY]   = TR("No uranium to transform");
    stringsErr[ERR_NUCLEAR_BAD]     = TR("Transforms only uranium");
    stringsErr[ERR_FACTORY_NULL]    = TR("No titanium");
    stringsErr[ERR_FACTORY_NEAR]    = TR("Object too close");
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
    stringsErr[ERR_ENEMY_OBJECT]    = TR("Unable to control enemy objects");
    stringsErr[ERR_WRONG_BOT]       = TR("Inappropriate bot");
    stringsErr[ERR_NO_QUICK_SLOT]   = TR("Quicksave slot not found");

    stringsErr[INFO_BUILD]          = TR("Building completed");
    stringsErr[INFO_CONVERT]        = TR("Titanium available");
    stringsErr[INFO_RESEARCH]       = TR("Research program completed");
    stringsErr[INFO_RESEARCHTANK]   = TR("Plans for tracked robots available");
    stringsErr[INFO_RESEARCHFLY]    = TR("You can fly with the keys (\\key gup;) and (\\key gdown;)");
    stringsErr[INFO_RESEARCHTHUMP]  = TR("Plans for thumper available");
    stringsErr[INFO_RESEARCHCANON]  = TR("Plans for shooter available");
    stringsErr[INFO_RESEARCHTOWER]  = TR("Plans for defense tower available");
    stringsErr[INFO_RESEARCHPHAZER] = TR("Plans for phazer shooter available");
    stringsErr[INFO_RESEARCHSHIELD] = TR("Plans for shielder available");
    stringsErr[INFO_RESEARCHATOMIC] = TR("Plans for nuclear power plant available");
    stringsErr[INFO_RESEARCHBUILDER]= TR("Plans for builder available");
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
    stringsErr[INFO_DELETEMOTHER]   = TR("Alien Queen killed");
    stringsErr[INFO_DELETEANT]      = TR("Ant fatally wounded");
    stringsErr[INFO_DELETEBEE]      = TR("Wasp fatally wounded");
    stringsErr[INFO_DELETEWORM]     = TR("Worm fatally wounded");
    stringsErr[INFO_DELETESPIDER]   = TR("Spider fatally wounded");
    stringsErr[INFO_BEGINSATCOM]    = TR("Press \\key help; to read instructions on your SatCom");
    stringsErr[INFO_TEAM_FINISH]    = TR("<<< Team %s finished! >>>");
    stringsErr[INFO_TEAM_DEAD]      = TR("<<< Team %s lost! >>>");
    stringsErr[INFO_TEAM_SCORE]     = TR("<<< Team %s recieved %d points >>>");



    stringsCbot[CBot::CBotErrOpenPar]       = TR("Opening bracket missing");
    stringsCbot[CBot::CBotErrClosePar]      = TR("Closing bracket missing");
    stringsCbot[CBot::CBotErrNotBoolean]    = TR("The expression must return a boolean value");
    stringsCbot[CBot::CBotErrUndefVar]      = TR("Variable not declared");
    stringsCbot[CBot::CBotErrBadLeft]       = TR("Assignment impossible");
    stringsCbot[CBot::CBotErrNoTerminator]  = TR("Semicolon terminator missing");
    stringsCbot[CBot::CBotErrCaseOut]       = TR("Instruction \"case\" outside a block \"switch\"");
    stringsCbot[CBot::CBotErrNoEnd]         = TR("Instructions after the final closing brace");
    stringsCbot[CBot::CBotErrCloseBlock]    = TR("End of block missing");
    stringsCbot[CBot::CBotErrElseWhitoutIf] = TR("Instruction \"else\" without corresponding \"if\"");
    stringsCbot[CBot::CBotErrOpenBlock]     = TR("Opening brace missing");
    stringsCbot[CBot::CBotErrBadType1]      = TR("Wrong type for the assignment");
    stringsCbot[CBot::CBotErrRedefVar]      = TR("A variable can not be declared twice");
    stringsCbot[CBot::CBotErrBadType2]      = TR("The types of the two operands are incompatible");
    stringsCbot[CBot::CBotErrUndefCall]     = TR("Unknown function");
    stringsCbot[CBot::CBotErrNoDoubleDots]  = TR("Sign \" : \" missing");
    stringsCbot[CBot::CBotErrNoWhile]       = TR("Keyword \"while\" missing");
    stringsCbot[CBot::CBotErrBreakOutside]  = TR("Instruction \"break\" outside a loop");
    stringsCbot[CBot::CBotErrLabel]         = TR("A label must be followed by \"for\"; \"while\"; \"do\" or \"switch\"");
    stringsCbot[CBot::CBotErrUndefLabel]    = TR("This label does not exist");
    stringsCbot[CBot::CBotErrNoCase]        = TR("Instruction \"case\" missing");
    stringsCbot[CBot::CBotErrBadNum]        = TR("Number missing");
    stringsCbot[CBot::CBotErrVoid]          = TR("Void parameter");
    stringsCbot[CBot::CBotErrNoType]        = TR("Type declaration missing");
    stringsCbot[CBot::CBotErrNoVar]         = TR("Variable name missing");
    stringsCbot[CBot::CBotErrNoFunc]        = TR("Function name missing");
    stringsCbot[CBot::CBotErrOverParam]     = TR("Too many parameters");
    stringsCbot[CBot::CBotErrRedefFunc]     = TR("Function already exists");
    stringsCbot[CBot::CBotErrLowParam]      = TR("Parameters missing");
    stringsCbot[CBot::CBotErrBadParam]      = TR("No function with this name accepts this kind of parameter");
    stringsCbot[CBot::CBotErrNbParam]       = TR("No function with this name accepts this number of parameters");
    stringsCbot[CBot::CBotErrUndefItem]     = TR("This is not a member of this class");
    stringsCbot[CBot::CBotErrUndefClass]    = TR("This object is not a member of a class");
    stringsCbot[CBot::CBotErrNoConstruct]   = TR("Appropriate constructor missing");
    stringsCbot[CBot::CBotErrRedefClass]    = TR("This class already exists");
    stringsCbot[CBot::CBotErrCloseIndex]    = TR("\" ] \" missing");
    stringsCbot[CBot::CBotErrReserved]      = TR("Reserved keyword of CBOT language");
    stringsCbot[CBot::CBotErrBadNew]        = TR("Bad argument for \"new\"");
    stringsCbot[CBot::CBotErrOpenIndex]     = TR("\" [ \" expected");
    stringsCbot[CBot::CBotErrBadString]     = TR("String missing");
    stringsCbot[CBot::CBotErrBadIndex]      = TR("Incorrect index type");
    stringsCbot[CBot::CBotErrPrivate]       = TR("Private element");
    stringsCbot[CBot::CBotErrNoPublic]      = TR("Public required");
    stringsCbot[CBot::CBotErrNoExpression]  = TR("Expression expected after =");
    stringsCbot[CBot::CBotErrAmbiguousCall] = TR("Ambiguous call to overloaded function");
    stringsCbot[CBot::CBotErrFuncNotVoid]   = TR("Function needs return type \"void\"");
    stringsCbot[CBot::CBotErrNoClassName]   = TR("Class name expected");
    stringsCbot[CBot::CBotErrNoReturn]      = TR("Non-void function needs \"return;\"");
    stringsCbot[CBot::CBotErrDefaultValue]  = TR("This parameter needs a default value");
    stringsCbot[CBot::CBotErrEndQuote]      = TR("Missing end quote");
    stringsCbot[CBot::CBotErrBadEscape]     = TR("Unknown escape sequence");
    stringsCbot[CBot::CBotErrOctalRange]    = TR("Octal value out of range");
    stringsCbot[CBot::CBotErrHexDigits]     = TR("Missing hex digits after escape sequence");
    stringsCbot[CBot::CBotErrHexRange]      = TR("Hex value out of range");
    stringsCbot[CBot::CBotErrUnicodeName]   = TR("Invalid universal character name");
    stringsCbot[CBot::CBotErrCharEmpty]     = TR("Empty character constant");
    stringsCbot[CBot::CBotErrRedefCase]     = TR("Duplicate label in switch");

    stringsCbot[CBot::CBotErrZeroDiv]       = TR("Dividing by zero");
    stringsCbot[CBot::CBotErrNotInit]       = TR("Variable not initialized");
    stringsCbot[CBot::CBotErrBadThrow]      = TR("Negative value rejected by \"throw\"");
    stringsCbot[CBot::CBotErrNoRetVal]      = TR("The function returned no value");
    stringsCbot[CBot::CBotErrNoRun]         = TR("No function running");
    stringsCbot[CBot::CBotErrUndefFunc]     = TR("Calling an unknown function");
    stringsCbot[CBot::CBotErrNotClass]      = TR("This class does not exist");
    stringsCbot[CBot::CBotErrNull]          = TR("Unknown Object");
    stringsCbot[CBot::CBotErrNan]           = TR("Operation impossible with value \"nan\"");
    stringsCbot[CBot::CBotErrOutArray]      = TR("Access beyond array limit");
    stringsCbot[CBot::CBotErrStackOver]     = TR("Stack overflow");
    stringsCbot[CBot::CBotErrDeletedPtr]    = TR("Illegal object");
    stringsCbot[CBot::CBotErrFileOpen]      = TR("Can't open file");
    stringsCbot[CBot::CBotErrNotOpen]       = TR("File not open");
    stringsCbot[CBot::CBotErrRead]          = TR("Read error");
    stringsCbot[CBot::CBotErrWrite]         = TR("Write error");
}


namespace
{
char g_gamerName[100];
} // anonymous namespace

void SetGlobalGamerName(std::string name)
{
    strcpy(g_gamerName, name.c_str());
}

// Replaces the commands "\key name;" in a text.

namespace
{

void PutKeyName(std::string& dst, const char* src)
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
            unsigned int count;
            for(count = 0; src[s+5+count] != ';'; count++);
            CInput* input = CInput::GetInstancePointer();
            InputSlot key = input->SearchKeyById(std::string(&src[s+5], count));
            if (key != INPUT_SLOT_MAX)
            {
                dst.append(input->GetKeysString(key));
            }
            s = s+5+count+1;
        }

        dst.append(1, src[s++]);
    }
}

// Returns the translated text of a resource that needs key substitution

const char* GetResourceBase(ResType type, unsigned int num)
{
    const char *str = nullptr;

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
            assert(num < CBot::CBotErrMAX);
            str = stringsCbot[num];
            break;

        default:
            assert(false);
    }

    return gettext(str);
}

} // anonymous namespace

// Returns the text of a resource.

bool GetResource(ResType type, unsigned int num, std::string& text)
{
    if(type != RES_KEY)
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
    else
    {
        if (num == static_cast<unsigned int>(KEY_INVALID))
            text.clear();
        else if (num == static_cast<unsigned int>(VIRTUAL_KMOD(CTRL)))
            text = "Ctrl";
        else if (num == static_cast<unsigned int>(VIRTUAL_KMOD(SHIFT)))
            text = "Shift";
        else if (num == static_cast<unsigned int>(VIRTUAL_KMOD(ALT)))
            text = "Alt";
        else if (num == static_cast<unsigned int>(VIRTUAL_KMOD(GUI)))
            text = "Win"; // TODO: Better description of this key?
        else if (num >= static_cast<unsigned int>(VIRTUAL_JOY(0)))
        {
            text = gettext("Button %1");
            text = StrUtils::Replace(text, "%1", StrUtils::ToString<int>(1 + num - VIRTUAL_JOY(0)));
        }
        else
        {
            text = SDL_GetKeyName(static_cast<SDL_Keycode>(num));
        }
        return true;
    }
}
