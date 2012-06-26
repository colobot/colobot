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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "language.h"
#include "global.h"
#include "event.h"
#include "object.h"
#include "cbottoken.h"




// Seeking the name of an object.

char* RetObjectName(ObjectType type)
{
    if ( type == OBJECT_PORTICO     )  return "Portico";
    if ( type == OBJECT_BASE        )  return "SpaceShip";
    if ( type == OBJECT_DERRICK     )  return "Derrick";
    if ( type == OBJECT_FACTORY     )  return "BotFactory";
    if ( type == OBJECT_STATION     )  return "PowerStation";
    if ( type == OBJECT_CONVERT     )  return "Converter";
    if ( type == OBJECT_REPAIR      )  return "RepairCenter";
    if ( type == OBJECT_DESTROYER   )  return "Destroyer";
    if ( type == OBJECT_TOWER       )  return "DefenseTower";
    if ( type == OBJECT_NEST        )  return "AlienNest";
    if ( type == OBJECT_RESEARCH    )  return "ResearchCenter";
    if ( type == OBJECT_RADAR       )  return "RadarStation";
    if ( type == OBJECT_INFO        )  return "ExchangePost";
    if ( type == OBJECT_ENERGY      )  return "PowerPlant";
    if ( type == OBJECT_LABO        )  return "AutoLab";
    if ( type == OBJECT_NUCLEAR     )  return "NuclearPlant";
    if ( type == OBJECT_PARA        )  return "PowerCaptor";
    if ( type == OBJECT_SAFE        )  return "Vault";
    if ( type == OBJECT_HUSTON      )  return "Houston";
    if ( type == OBJECT_TARGET1     )  return "Target1";
    if ( type == OBJECT_TARGET2     )  return "Target2";
    if ( type == OBJECT_START       )  return "StartArea";
    if ( type == OBJECT_END         )  return "GoalArea";
    if ( type == OBJECT_TEEN34      )  return "Stone";
    if ( type == OBJECT_STONE       )  return "TitaniumOre";
    if ( type == OBJECT_URANIUM     )  return "UraniumOre";
    if ( type == OBJECT_METAL       )  return "Titanium";
    if ( type == OBJECT_POWER       )  return "PowerCell";
    if ( type == OBJECT_ATOMIC      )  return "NuclearCell";
    if ( type == OBJECT_BULLET      )  return "OrgaMatter";
    if ( type == OBJECT_BBOX        )  return "BlackBox";
    if ( type == OBJECT_KEYa        )  return "KeyA";
    if ( type == OBJECT_KEYb        )  return "KeyB";
    if ( type == OBJECT_KEYc        )  return "KeyC";
    if ( type == OBJECT_KEYd        )  return "KeyD";
    if ( type == OBJECT_TNT         )  return "TNT";
    if ( type == OBJECT_SCRAP1      )  return "Scrap";
    if ( type == OBJECT_BOMB        )  return "Mine";
    if ( type == OBJECT_BARRIER1    )  return "Barrier";
    if ( type == OBJECT_WAYPOINT    )  return "WayPoint";
    if ( type == OBJECT_FLAGb       )  return "BlueFlag";
    if ( type == OBJECT_FLAGr       )  return "RedFlag";
    if ( type == OBJECT_FLAGg       )  return "GreenFlag";
    if ( type == OBJECT_FLAGy       )  return "YellowFlag";
    if ( type == OBJECT_FLAGv       )  return "VioletFlag";
    if ( type == OBJECT_MARKPOWER   )  return "PowerSpot";
    if ( type == OBJECT_MARKSTONE   )  return "TitaniumSpot";
    if ( type == OBJECT_MARKURANIUM )  return "UraniumSpot";
    if ( type == OBJECT_MARKKEYa    )  return "KeyASpot";
    if ( type == OBJECT_MARKKEYb    )  return "KeyBSpot";
    if ( type == OBJECT_MARKKEYc    )  return "KeyCSpot";
    if ( type == OBJECT_MARKKEYd    )  return "KeyDSpot";
    if ( type == OBJECT_MOBILEwt    )  return "PracticeBot";
    if ( type == OBJECT_MOBILEwa    )  return "WheeledGrabber";
    if ( type == OBJECT_MOBILEta    )  return "TrackedGrabber";
    if ( type == OBJECT_MOBILEfa    )  return "WingedGrabber";
    if ( type == OBJECT_MOBILEia    )  return "LeggedGrabber";
    if ( type == OBJECT_MOBILEwc    )  return "WheeledShooter";
    if ( type == OBJECT_MOBILEtc    )  return "TrackedShooter";
    if ( type == OBJECT_MOBILEfc    )  return "WingedShooter";
    if ( type == OBJECT_MOBILEic    )  return "LeggedShooter";
    if ( type == OBJECT_MOBILEwi    )  return "WheeledOrgaShooter";
    if ( type == OBJECT_MOBILEti    )  return "TrackedOrgaShooter";
    if ( type == OBJECT_MOBILEfi    )  return "WingedOrgaShooter";
    if ( type == OBJECT_MOBILEii    )  return "LeggedOrgaShooter";
    if ( type == OBJECT_MOBILEws    )  return "WheeledSniffer";
    if ( type == OBJECT_MOBILEts    )  return "TrackedSniffer";
    if ( type == OBJECT_MOBILEfs    )  return "WingedSniffer";
    if ( type == OBJECT_MOBILEis    )  return "LeggedSniffer";
    if ( type == OBJECT_MOBILErt    )  return "Thumper";
    if ( type == OBJECT_MOBILErc    )  return "PhazerShooter";
    if ( type == OBJECT_MOBILErr    )  return "Recycler";
    if ( type == OBJECT_MOBILErs    )  return "Shielder";
    if ( type == OBJECT_MOBILEsa    )  return "Subber";
    if ( type == OBJECT_MOBILEtg    )  return "TargetBot";
    if ( type == OBJECT_MOBILEdr    )  return "Scribbler";
    if ( type == OBJECT_HUMAN       )  return "Me";
    if ( type == OBJECT_TECH        )  return "Tech";
    if ( type == OBJECT_MOTHER      )  return "AlienQueen";
    if ( type == OBJECT_EGG         )  return "AlienEgg";
    if ( type == OBJECT_ANT         )  return "AlienAnt";
    if ( type == OBJECT_SPIDER      )  return "AlienSpider";
    if ( type == OBJECT_BEE         )  return "AlienWasp";
    if ( type == OBJECT_WORM        )  return "AlienWorm";
    if ( type == OBJECT_RUINmobilew1)  return "Wreck";
    return "";
}

// Seeking the name of a secondary object.
// (because Otto thinks that Germans do not like nuclear power)

char* RetObjectAlias(ObjectType type)
{
    if ( type == OBJECT_NUCLEAR     )  return "FuelCellPlant";
    if ( type == OBJECT_URANIUM     )  return "PlatinumOre";
    if ( type == OBJECT_ATOMIC      )  return "FuelCell";
    if ( type == OBJECT_MARKURANIUM )  return "PlatinumSpot";
    if ( type == OBJECT_ENERGY      )  return "Disintegrator";  // for K-CeeBot
    return "";
}


// Returns the help file to use for the object.

char* RetHelpFilename(ObjectType type)
{
    if ( type == OBJECT_BASE        )  return "help\\object\\base.txt";
    if ( type == OBJECT_DERRICK     )  return "help\\object\\derrick.txt";
    if ( type == OBJECT_FACTORY     )  return "help\\object\\factory.txt";
    if ( type == OBJECT_STATION     )  return "help\\object\\station.txt";
    if ( type == OBJECT_CONVERT     )  return "help\\object\\convert.txt";
    if ( type == OBJECT_REPAIR      )  return "help\\object\\repair.txt";
    if ( type == OBJECT_DESTROYER   )  return "help\\object\\destroy.txt";
    if ( type == OBJECT_TOWER       )  return "help\\object\\tower.txt";
    if ( type == OBJECT_NEST        )  return "help\\object\\nest.txt";
    if ( type == OBJECT_RESEARCH    )  return "help\\object\\research.txt";
    if ( type == OBJECT_RADAR       )  return "help\\object\\radar.txt";
    if ( type == OBJECT_INFO        )  return "help\\object\\exchange.txt";
    if ( type == OBJECT_ENERGY      )  return "help\\object\\energy.txt";
    if ( type == OBJECT_LABO        )  return "help\\object\\labo.txt";
    if ( type == OBJECT_NUCLEAR     )  return "help\\object\\nuclear.txt";
    if ( type == OBJECT_PARA        )  return "help\\object\\captor.txt";
    if ( type == OBJECT_SAFE        )  return "help\\object\\safe.txt";
    if ( type == OBJECT_HUSTON      )  return "help\\object\\huston.txt";
    if ( type == OBJECT_START       )  return "help\\object\\start.txt";
    if ( type == OBJECT_END         )  return "help\\object\\goal.txt";
    if ( type == OBJECT_STONE       )  return "help\\object\\titanore.txt";
    if ( type == OBJECT_URANIUM     )  return "help\\object\\uranore.txt";
    if ( type == OBJECT_METAL       )  return "help\\object\\titan.txt";
    if ( type == OBJECT_POWER       )  return "help\\object\\power.txt";
    if ( type == OBJECT_ATOMIC      )  return "help\\object\\atomic.txt";
    if ( type == OBJECT_BULLET      )  return "help\\object\\bullet.txt";
    if ( type == OBJECT_BBOX        )  return "help\\object\\bbox.txt";
    if ( type == OBJECT_KEYa        )  return "help\\object\\key.txt";
    if ( type == OBJECT_KEYb        )  return "help\\object\\key.txt";
    if ( type == OBJECT_KEYc        )  return "help\\object\\key.txt";
    if ( type == OBJECT_KEYd        )  return "help\\object\\key.txt";
    if ( type == OBJECT_TNT         )  return "help\\object\\tnt.txt";
    if ( type == OBJECT_SCRAP1      )  return "help\\object\\scrap.txt";
    if ( type == OBJECT_BOMB        )  return "help\\object\\mine.txt";
    if ( type == OBJECT_BARRIER1    )  return "help\\object\\barrier.txt";
    if ( type == OBJECT_WAYPOINT    )  return "help\\object\\waypoint.txt";
    if ( type == OBJECT_FLAGb       )  return "help\\object\\flag.txt";
    if ( type == OBJECT_FLAGr       )  return "help\\object\\flag.txt";
    if ( type == OBJECT_FLAGg       )  return "help\\object\\flag.txt";
    if ( type == OBJECT_FLAGy       )  return "help\\object\\flag.txt";
    if ( type == OBJECT_FLAGv       )  return "help\\object\\flag.txt";
    if ( type == OBJECT_MARKPOWER   )  return "help\\object\\enerspot.txt";
    if ( type == OBJECT_MARKSTONE   )  return "help\\object\\stonspot.txt";
    if ( type == OBJECT_MARKURANIUM )  return "help\\object\\uranspot.txt";
    if ( type == OBJECT_MOBILEwa    )  return "help\\object\\botgr.txt";
    if ( type == OBJECT_MOBILEta    )  return "help\\object\\botgc.txt";
    if ( type == OBJECT_MOBILEfa    )  return "help\\object\\botgj.txt";
    if ( type == OBJECT_MOBILEia    )  return "help\\object\\botgs.txt";
    if ( type == OBJECT_MOBILEws    )  return "help\\object\\botsr.txt";
    if ( type == OBJECT_MOBILEts    )  return "help\\object\\botsc.txt";
    if ( type == OBJECT_MOBILEfs    )  return "help\\object\\botsj.txt";
    if ( type == OBJECT_MOBILEis    )  return "help\\object\\botss.txt";
    if ( type == OBJECT_MOBILEwi    )  return "help\\object\\botor.txt";
    if ( type == OBJECT_MOBILEti    )  return "help\\object\\botoc.txt";
    if ( type == OBJECT_MOBILEfi    )  return "help\\object\\botoj.txt";
    if ( type == OBJECT_MOBILEii    )  return "help\\object\\botos.txt";
    if ( type == OBJECT_MOBILEwc    )  return "help\\object\\botfr.txt";
    if ( type == OBJECT_MOBILEtc    )  return "help\\object\\botfc.txt";
    if ( type == OBJECT_MOBILEfc    )  return "help\\object\\botfj.txt";
    if ( type == OBJECT_MOBILEic    )  return "help\\object\\botfs.txt";
    if ( type == OBJECT_MOBILErt    )  return "help\\object\\bottump.txt";
    if ( type == OBJECT_MOBILErc    )  return "help\\object\\botphaz.txt";
    if ( type == OBJECT_MOBILErr    )  return "help\\object\\botrecy.txt";
    if ( type == OBJECT_MOBILErs    )  return "help\\object\\botshld.txt";
    if ( type == OBJECT_MOBILEsa    )  return "help\\object\\botsub.txt";
    if ( type == OBJECT_MOBILEwt    )  return "help\\object\\bottr.txt";
    if ( type == OBJECT_MOBILEtg    )  return "help\\object\\bottarg.txt";
    if ( type == OBJECT_MOBILEdr    )  return "help\\object\\botdraw.txt";
    if ( type == OBJECT_APOLLO2     )  return "help\\object\\lrv.txt";
    if ( type == OBJECT_HUMAN       )  return "help\\object\\human.txt";
    if ( type == OBJECT_MOTHER      )  return "help\\object\\mother.txt";
    if ( type == OBJECT_EGG         )  return "help\\object\\egg.txt";
    if ( type == OBJECT_ANT         )  return "help\\object\\ant.txt";
    if ( type == OBJECT_SPIDER      )  return "help\\object\\spider.txt";
    if ( type == OBJECT_BEE         )  return "help\\object\\wasp.txt";
    if ( type == OBJECT_WORM        )  return "help\\object\\worm.txt";
    if ( type == OBJECT_RUINmobilew1)  return "help\\object\\wreck.txt";
    return "";
}


// Returns the help file to use for instruction.

char* RetHelpFilename(const char *token)
{
    if ( strcmp(token, "if"            ) == 0 )  return "help\\cbot\\if.txt";
    if ( strcmp(token, "else"          ) == 0 )  return "help\\cbot\\if.txt";
    if ( strcmp(token, "repeat"        ) == 0 )  return "help\\cbot\\repeat.txt";
    if ( strcmp(token, "for"           ) == 0 )  return "help\\cbot\\for.txt";
    if ( strcmp(token, "while"         ) == 0 )  return "help\\cbot\\while.txt";
    if ( strcmp(token, "do"            ) == 0 )  return "help\\cbot\\do.txt";
    if ( strcmp(token, "break"         ) == 0 )  return "help\\cbot\\break.txt";
    if ( strcmp(token, "continue"      ) == 0 )  return "help\\cbot\\continue.txt";
    if ( strcmp(token, "return"        ) == 0 )  return "help\\cbot\\return.txt";
    if ( strcmp(token, "sizeof"        ) == 0 )  return "help\\cbot\\sizeof.txt";
    if ( strcmp(token, "int"           ) == 0 )  return "help\\cbot\\int.txt";
    if ( strcmp(token, "float"         ) == 0 )  return "help\\cbot\\float.txt";
    if ( strcmp(token, "bool"          ) == 0 )  return "help\\cbot\\bool.txt";
    if ( strcmp(token, "string"        ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "point"         ) == 0 )  return "help\\cbot\\point.txt";
    if ( strcmp(token, "object"        ) == 0 )  return "help\\cbot\\object.txt";
    if ( strcmp(token, "file"          ) == 0 )  return "help\\cbot\\file.txt";
    if ( strcmp(token, "void"          ) == 0 )  return "help\\cbot\\void.txt";
    if ( strcmp(token, "null"          ) == 0 )  return "help\\cbot\\null.txt";
    if ( strcmp(token, "nan"           ) == 0 )  return "help\\cbot\\nan.txt";
    if ( strcmp(token, "true"          ) == 0 )  return "help\\cbot\\true.txt";
    if ( strcmp(token, "false"         ) == 0 )  return "help\\cbot\\false.txt";
    if ( strcmp(token, "sin"           ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "cos"           ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "tan"           ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "asin"          ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "acos"          ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "atan"          ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "sqrt"          ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "pow"           ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "rand"          ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "abs"           ) == 0 )  return "help\\cbot\\expr.txt";
    if ( strcmp(token, "retobject"     ) == 0 )  return "help\\cbot\\retobj.txt";
    if ( strcmp(token, "search"        ) == 0 )  return "help\\cbot\\search.txt";
    if ( strcmp(token, "radar"         ) == 0 )  return "help\\cbot\\radar.txt";
    if ( strcmp(token, "direction"     ) == 0 )  return "help\\cbot\\direct.txt";
    if ( strcmp(token, "distance"      ) == 0 )  return "help\\cbot\\dist.txt";
    if ( strcmp(token, "distance2d"    ) == 0 )  return "help\\cbot\\dist2d.txt";
    if ( strcmp(token, "space"         ) == 0 )  return "help\\cbot\\space.txt";
    if ( strcmp(token, "flatground"    ) == 0 )  return "help\\cbot\\flatgrnd.txt";
    if ( strcmp(token, "wait"          ) == 0 )  return "help\\cbot\\wait.txt";
    if ( strcmp(token, "move"          ) == 0 )  return "help\\cbot\\move.txt";
    if ( strcmp(token, "turn"          ) == 0 )  return "help\\cbot\\turn.txt";
    if ( strcmp(token, "goto"          ) == 0 )  return "help\\cbot\\goto.txt";
    if ( strcmp(token, "find"          ) == 0 )  return "help\\cbot\\find.txt";
    if ( strcmp(token, "grab"          ) == 0 )  return "help\\cbot\\grab.txt";
    if ( strcmp(token, "drop"          ) == 0 )  return "help\\cbot\\drop.txt";
    if ( strcmp(token, "sniff"         ) == 0 )  return "help\\cbot\\sniff.txt";
    if ( strcmp(token, "receive"       ) == 0 )  return "help\\cbot\\receive.txt";
    if ( strcmp(token, "send"          ) == 0 )  return "help\\cbot\\send.txt";
    if ( strcmp(token, "deleteinfo"    ) == 0 )  return "help\\cbot\\delinfo.txt";
    if ( strcmp(token, "testinfo"      ) == 0 )  return "help\\cbot\\testinfo.txt";
    if ( strcmp(token, "thump"         ) == 0 )  return "help\\cbot\\thump.txt";
    if ( strcmp(token, "recycle"       ) == 0 )  return "help\\cbot\\recycle.txt";
    if ( strcmp(token, "shield"        ) == 0 )  return "help\\cbot\\shield.txt";
    if ( strcmp(token, "fire"          ) == 0 )  return "help\\cbot\\fire.txt";
    if ( strcmp(token, "antfire"       ) == 0 )  return "help\\cbot\\antfire.txt";
    if ( strcmp(token, "aim"           ) == 0 )  return "help\\cbot\\aim.txt";
    if ( strcmp(token, "motor"         ) == 0 )  return "help\\cbot\\motor.txt";
    if ( strcmp(token, "jet"           ) == 0 )  return "help\\cbot\\jet.txt";
    if ( strcmp(token, "topo"          ) == 0 )  return "help\\cbot\\topo.txt";
    if ( strcmp(token, "message"       ) == 0 )  return "help\\cbot\\message.txt";
    if ( strcmp(token, "abstime"       ) == 0 )  return "help\\cbot\\abstime.txt";
    if ( strcmp(token, "BlackArrow"    ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "RedArrow"      ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "White"         ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Black"         ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Gray"          ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "LightGray"     ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Red"           ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Pink"          ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Purple"        ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Orange"        ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Yellow"        ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Beige"         ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Brown"         ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Skin"          ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Green"         ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "LightGreen"    ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "Blue"          ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "LightBlue"     ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "InFront"       ) == 0 )  return "help\\cbot\\grab.txt";
    if ( strcmp(token, "Behind"        ) == 0 )  return "help\\cbot\\grab.txt";
    if ( strcmp(token, "EnergyCell"    ) == 0 )  return "help\\cbot\\grab.txt";
    if ( strcmp(token, "DisplayError"  ) == 0 )  return "help\\cbot\\message.txt";
    if ( strcmp(token, "DisplayWarning") == 0 )  return "help\\cbot\\message.txt";
    if ( strcmp(token, "DisplayInfo"   ) == 0 )  return "help\\cbot\\message.txt";
    if ( strcmp(token, "DisplayMessage") == 0 )  return "help\\cbot\\message.txt";
    if ( strcmp(token, "strlen"        ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strleft"       ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strright"      ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strmid"        ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strval"        ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strfind"       ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strlower"      ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "strupper"      ) == 0 )  return "help\\cbot\\string.txt";
    if ( strcmp(token, "open"          ) == 0 )  return "help\\cbot\\open.txt";
    if ( strcmp(token, "close"         ) == 0 )  return "help\\cbot\\close.txt";
    if ( strcmp(token, "writeln"       ) == 0 )  return "help\\cbot\\writeln.txt";
    if ( strcmp(token, "readln "       ) == 0 )  return "help\\cbot\\readln.txt";
    if ( strcmp(token, "eof"           ) == 0 )  return "help\\cbot\\eof.txt";
    if ( strcmp(token, "deletefile"    ) == 0 )  return "help\\cbot\\deletef.txt";
    if ( strcmp(token, "openfile"      ) == 0 )  return "help\\cbot\\openfile.txt";
    if ( strcmp(token, "pendown"       ) == 0 )  return "help\\cbot\\pendown.txt";
    if ( strcmp(token, "penup"         ) == 0 )  return "help\\cbot\\penup.txt";
    if ( strcmp(token, "pencolor"      ) == 0 )  return "help\\cbot\\pencolor.txt";
    if ( strcmp(token, "penwidth"      ) == 0 )  return "help\\cbot\\penwidth.txt";
    if ( strcmp(token, "extern"        ) == 0 )  return "help\\cbot\\extern.txt";
    if ( strcmp(token, "class"         ) == 0 )  return "help\\cbot\\class.txt";
    if ( strcmp(token, "static"        ) == 0 )  return "help\\cbot\\static.txt";
    if ( strcmp(token, "public"        ) == 0 )  return "help\\cbot\\public.txt";
    if ( strcmp(token, "private"       ) == 0 )  return "help\\cbot\\private.txt";
    if ( strcmp(token, "synchronized"  ) == 0 )  return "help\\cbot\\synchro.txt";
    if ( strcmp(token, "new"           ) == 0 )  return "help\\cbot\\new.txt";
    if ( strcmp(token, "this"          ) == 0 )  return "help\\cbot\\this.txt";
    return "";
}


// Test if a keyword is a type of variable.

BOOL IsType(const char *token)
{
    if ( strcmp(token, "void"   ) == 0 )  return TRUE;
    if ( strcmp(token, "int"    ) == 0 )  return TRUE;
    if ( strcmp(token, "float"  ) == 0 )  return TRUE;
    if ( strcmp(token, "bool"   ) == 0 )  return TRUE;
    if ( strcmp(token, "string" ) == 0 )  return TRUE;
    if ( strcmp(token, "point"  ) == 0 )  return TRUE;
    if ( strcmp(token, "object" ) == 0 )  return TRUE;
    if ( strcmp(token, "file"   ) == 0 )  return TRUE;
    if ( strcmp(token, "this"   ) == 0 )  return TRUE;
    return FALSE;
}

// Test if a keyword is a function.

BOOL IsFunction(const char *token)
{
    if ( strcmp(token, "sin"          ) == 0 )  return TRUE;
    if ( strcmp(token, "cos"          ) == 0 )  return TRUE;
    if ( strcmp(token, "tan"          ) == 0 )  return TRUE;
    if ( strcmp(token, "asin"         ) == 0 )  return TRUE;
    if ( strcmp(token, "acos"         ) == 0 )  return TRUE;
    if ( strcmp(token, "atan"         ) == 0 )  return TRUE;
    if ( strcmp(token, "sqrt"         ) == 0 )  return TRUE;
    if ( strcmp(token, "pow"          ) == 0 )  return TRUE;
    if ( strcmp(token, "rand"         ) == 0 )  return TRUE;
    if ( strcmp(token, "abs"          ) == 0 )  return TRUE;
    if ( strcmp(token, "retobject"    ) == 0 )  return TRUE;
    if ( strcmp(token, "search"       ) == 0 )  return TRUE;
    if ( strcmp(token, "radar"        ) == 0 )  return TRUE;
    if ( strcmp(token, "detect"       ) == 0 )  return TRUE;
    if ( strcmp(token, "direction"    ) == 0 )  return TRUE;
    if ( strcmp(token, "distance"     ) == 0 )  return TRUE;
    if ( strcmp(token, "distance2d"   ) == 0 )  return TRUE;
    if ( strcmp(token, "space"        ) == 0 )  return TRUE;
    if ( strcmp(token, "flatground"   ) == 0 )  return TRUE;
    if ( strcmp(token, "wait"         ) == 0 )  return TRUE;
    if ( strcmp(token, "move"         ) == 0 )  return TRUE;
    if ( strcmp(token, "turn"         ) == 0 )  return TRUE;
    if ( strcmp(token, "goto"         ) == 0 )  return TRUE;
    if ( strcmp(token, "find"         ) == 0 )  return TRUE;
    if ( strcmp(token, "grab"         ) == 0 )  return TRUE;
    if ( strcmp(token, "drop"         ) == 0 )  return TRUE;
    if ( strcmp(token, "sniff"        ) == 0 )  return TRUE;
    if ( strcmp(token, "receive"      ) == 0 )  return TRUE;
    if ( strcmp(token, "send"         ) == 0 )  return TRUE;
    if ( strcmp(token, "deleteinfo"   ) == 0 )  return TRUE;
    if ( strcmp(token, "testinfo"     ) == 0 )  return TRUE;
    if ( strcmp(token, "thump"        ) == 0 )  return TRUE;
    if ( strcmp(token, "recycle"      ) == 0 )  return TRUE;
    if ( strcmp(token, "shield"       ) == 0 )  return TRUE;
    if ( strcmp(token, "fire"         ) == 0 )  return TRUE;
    if ( strcmp(token, "antfire"      ) == 0 )  return TRUE;
    if ( strcmp(token, "aim"          ) == 0 )  return TRUE;
    if ( strcmp(token, "motor"        ) == 0 )  return TRUE;
    if ( strcmp(token, "jet"          ) == 0 )  return TRUE;
    if ( strcmp(token, "topo"         ) == 0 )  return TRUE;
    if ( strcmp(token, "message"      ) == 0 )  return TRUE;
    if ( strcmp(token, "abstime"      ) == 0 )  return TRUE;
    if ( strcmp(token, "ismovie"      ) == 0 )  return TRUE;
    if ( strcmp(token, "errmode"      ) == 0 )  return TRUE;
    if ( strcmp(token, "ipf"          ) == 0 )  return TRUE;
    if ( strcmp(token, "strlen"       ) == 0 )  return TRUE;
    if ( strcmp(token, "strleft"      ) == 0 )  return TRUE;
    if ( strcmp(token, "strright"     ) == 0 )  return TRUE;
    if ( strcmp(token, "strmid"       ) == 0 )  return TRUE;
    if ( strcmp(token, "strval"       ) == 0 )  return TRUE;
    if ( strcmp(token, "strfind"      ) == 0 )  return TRUE;
    if ( strcmp(token, "strlower"     ) == 0 )  return TRUE;
    if ( strcmp(token, "strupper"     ) == 0 )  return TRUE;
    if ( strcmp(token, "open"         ) == 0 )  return TRUE;
    if ( strcmp(token, "close"        ) == 0 )  return TRUE;
    if ( strcmp(token, "writeln"      ) == 0 )  return TRUE;
    if ( strcmp(token, "readln"       ) == 0 )  return TRUE;
    if ( strcmp(token, "eof"          ) == 0 )  return TRUE;
    if ( strcmp(token, "deletefile"   ) == 0 )  return TRUE;
    if ( strcmp(token, "openfile"     ) == 0 )  return TRUE;
    if ( strcmp(token, "pendown"      ) == 0 )  return TRUE;
    if ( strcmp(token, "penup"        ) == 0 )  return TRUE;
    if ( strcmp(token, "pencolor"     ) == 0 )  return TRUE;
    if ( strcmp(token, "penwidth"     ) == 0 )  return TRUE;
    if ( strcmp(token, "sizeof"       ) == 0 )  return TRUE;
    return FALSE;
}


// Returns using a compact instruction.

char* RetHelpText(const char *token)
{
    if ( strcmp(token, "if"        ) == 0 )  return "if ( condition ) { bloc }";
    if ( strcmp(token, "else"      ) == 0 )  return "else { bloc }";
    if ( strcmp(token, "repeat"    ) == 0 )  return "repeat ( number )";
    if ( strcmp(token, "for"       ) == 0 )  return "for ( before ; condition ; end )";
    if ( strcmp(token, "while"     ) == 0 )  return "while ( condition ) { bloc }";
    if ( strcmp(token, "do"        ) == 0 )  return "do { bloc } while ( condition );";
    if ( strcmp(token, "break"     ) == 0 )  return "break;";
    if ( strcmp(token, "continue"  ) == 0 )  return "continue;";
    if ( strcmp(token, "return"    ) == 0 )  return "return;";
    if ( strcmp(token, "sizeof"    ) == 0 )  return "sizeof( array );";
    if ( strcmp(token, "int"       ) == 0 )  return "int";
    if ( strcmp(token, "sin"       ) == 0 )  return "sin ( angle );";
    if ( strcmp(token, "cos"       ) == 0 )  return "cos ( angle );";
    if ( strcmp(token, "tan"       ) == 0 )  return "tan ( angle );";
    if ( strcmp(token, "asin"      ) == 0 )  return "asin ( value );";
    if ( strcmp(token, "acos"      ) == 0 )  return "acos ( value );";
    if ( strcmp(token, "atan"      ) == 0 )  return "atan ( value );";
    if ( strcmp(token, "sqrt"      ) == 0 )  return "sqrt ( value );";
    if ( strcmp(token, "pow"       ) == 0 )  return "pow ( x, y );";
    if ( strcmp(token, "rand"      ) == 0 )  return "rand ( );";
    if ( strcmp(token, "abs"       ) == 0 )  return "abs ( value );";
    if ( strcmp(token, "retobject" ) == 0 )  return "retobjet ( );";
    if ( strcmp(token, "search"    ) == 0 )  return "search ( );";
    if ( strcmp(token, "radar"     ) == 0 )  return "radar ( cat, angle, focus, min, max, sens );";
    if ( strcmp(token, "detect"    ) == 0 )  return "detect ( cat );";
    if ( strcmp(token, "direction" ) == 0 )  return "direction ( position );";
    if ( strcmp(token, "distance2d") == 0 )  return "distance2d ( p1, p2 );";
    if ( strcmp(token, "distance"  ) == 0 )  return "distance ( p1, p2 );";
    if ( strcmp(token, "space"     ) == 0 )  return "space ( center, rmin, rmax, dist );";
    if ( strcmp(token, "flatground") == 0 )  return "flatground ( center, rmax );";
    if ( strcmp(token, "wait"      ) == 0 )  return "wait ( time );";
    if ( strcmp(token, "move"      ) == 0 )  return "move ( distance );";
    if ( strcmp(token, "turn"      ) == 0 )  return "turn ( angle );";
    if ( strcmp(token, "goto"      ) == 0 )  return "goto ( position, altitude );";
    if ( strcmp(token, "find"      ) == 0 )  return "find ( cat );";
    if ( strcmp(token, "grab"      ) == 0 )  return "grab ( order );";
    if ( strcmp(token, "drop"      ) == 0 )  return "drop ( order );";
    if ( strcmp(token, "sniff"     ) == 0 )  return "sniff ( );";
    if ( strcmp(token, "receive"   ) == 0 )  return "receive ( name, power );";
    if ( strcmp(token, "send"      ) == 0 )  return "send ( name, value, power );";
    if ( strcmp(token, "deleteinfo") == 0 )  return "deleteinfo ( name, power );";
    if ( strcmp(token, "testinfo"  ) == 0 )  return "testinfo ( name, power );";
    if ( strcmp(token, "thump"     ) == 0 )  return "thump ( );";
    if ( strcmp(token, "recycle"   ) == 0 )  return "recycle ( );";
    if ( strcmp(token, "shield"    ) == 0 )  return "shield ( oper, radius );";
    if ( strcmp(token, "fire"      ) == 0 )  return "fire ( time );";
    if ( strcmp(token, "antfire"   ) == 0 )  return "antfire ( );";
    if ( strcmp(token, "aim"       ) == 0 )  return "aim ( angle );";
    if ( strcmp(token, "motor"     ) == 0 )  return "motor ( left, right );";
    if ( strcmp(token, "jet"       ) == 0 )  return "jet ( power );";
    if ( strcmp(token, "topo"      ) == 0 )  return "topo ( position );";
    if ( strcmp(token, "message"   ) == 0 )  return "message ( string, type );";
    if ( strcmp(token, "abstime"   ) == 0 )  return "abstime ( );";
    if ( strcmp(token, "ismovie"   ) == 0 )  return "ismovie ( );";
    if ( strcmp(token, "errmode"   ) == 0 )  return "errmode ( mdoe );";
    if ( strcmp(token, "ipf"       ) == 0 )  return "ipf ( number );";
    if ( strcmp(token, "strlen"    ) == 0 )  return "strlen ( string );";
    if ( strcmp(token, "strleft"   ) == 0 )  return "strleft ( string, len );";
    if ( strcmp(token, "strright"  ) == 0 )  return "strright ( string, len );";
    if ( strcmp(token, "strmid"    ) == 0 )  return "strmid ( string, pos, len );";
    if ( strcmp(token, "strval"    ) == 0 )  return "strval ( string );";
    if ( strcmp(token, "strfind"   ) == 0 )  return "strfind ( string, substring );";
    if ( strcmp(token, "strlower"  ) == 0 )  return "strlower ( string );";
    if ( strcmp(token, "strupper"  ) == 0 )  return "strupper ( string );";
    if ( strcmp(token, "open"      ) == 0 )  return "open ( filename, mode );";
    if ( strcmp(token, "close"     ) == 0 )  return "close ( );";
    if ( strcmp(token, "writeln"   ) == 0 )  return "writeln ( string );";
    if ( strcmp(token, "readln"    ) == 0 )  return "readln ( );";
    if ( strcmp(token, "eof"       ) == 0 )  return "eof ( );";
    if ( strcmp(token, "deletefile") == 0 )  return "deletefile ( filename );";
    if ( strcmp(token, "openfile"  ) == 0 )  return "openfile ( filename, mode );";
    if ( strcmp(token, "pendown"   ) == 0 )  return "pendown ( color, width );";
    if ( strcmp(token, "penup"     ) == 0 )  return "penup ( );";
    if ( strcmp(token, "pencolor"  ) == 0 )  return "pencolor ( color );";
    if ( strcmp(token, "penwidth"  ) == 0 )  return "penwidth ( width );";
    return "";
}


