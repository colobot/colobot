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


#include "script/cbottoken.h"

#include "object/object.h"
#include "app/app.h"

#include <string.h>


// Seeking the name of an object.

const char* GetObjectName(ObjectType type)
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

const char* GetObjectAlias(ObjectType type)
{
    if ( type == OBJECT_NUCLEAR     )  return "FuelCellPlant";
    if ( type == OBJECT_URANIUM     )  return "PlatinumOre";
    if ( type == OBJECT_ATOMIC      )  return "FuelCell";
    if ( type == OBJECT_MARKURANIUM )  return "PlatinumSpot";
    if ( type == OBJECT_ENERGY      )  return "Disintegrator";  // for K-CeeBot
    return "";
}


// Returns the help file to use for the object.

std::string GetHelpFilename(ObjectType type)
{
    if ( type == OBJECT_BASE        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/base.txt");
    if ( type == OBJECT_DERRICK     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/derrick.txt");
    if ( type == OBJECT_FACTORY     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/factory.txt");
    if ( type == OBJECT_STATION     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/station.txt");
    if ( type == OBJECT_CONVERT     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/convert.txt");
    if ( type == OBJECT_REPAIR      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/repair.txt");
    if ( type == OBJECT_DESTROYER   )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/destroy.txt");
    if ( type == OBJECT_TOWER       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/tower.txt");
    if ( type == OBJECT_NEST        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/nest.txt");
    if ( type == OBJECT_RESEARCH    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/research.txt");
    if ( type == OBJECT_RADAR       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/radar.txt");
    if ( type == OBJECT_INFO        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/exchange.txt");
    if ( type == OBJECT_ENERGY      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/energy.txt");
    if ( type == OBJECT_LABO        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/labo.txt");
    if ( type == OBJECT_NUCLEAR     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/nuclear.txt");
    if ( type == OBJECT_PARA        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/captor.txt");
    if ( type == OBJECT_SAFE        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/safe.txt");
    if ( type == OBJECT_HUSTON      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/huston.txt");
    if ( type == OBJECT_START       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/start.txt");
    if ( type == OBJECT_END         )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/goal.txt");
    if ( type == OBJECT_STONE       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/titanore.txt");
    if ( type == OBJECT_URANIUM     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/uranore.txt");
    if ( type == OBJECT_METAL       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/titan.txt");
    if ( type == OBJECT_POWER       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/power.txt");
    if ( type == OBJECT_ATOMIC      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/atomic.txt");
    if ( type == OBJECT_BULLET      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/bullet.txt");
    if ( type == OBJECT_BBOX        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/bbox.txt");
    if ( type == OBJECT_KEYa        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/key.txt");
    if ( type == OBJECT_KEYb        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/key.txt");
    if ( type == OBJECT_KEYc        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/key.txt");
    if ( type == OBJECT_KEYd        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/key.txt");
    if ( type == OBJECT_TNT         )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/tnt.txt");
    if ( type == OBJECT_SCRAP1      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/scrap.txt");
    if ( type == OBJECT_BOMB        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/mine.txt");
    if ( type == OBJECT_BARRIER1    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/barrier.txt");
    if ( type == OBJECT_WAYPOINT    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/waypoint.txt");
    if ( type == OBJECT_FLAGb       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/flag.txt");
    if ( type == OBJECT_FLAGr       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/flag.txt");
    if ( type == OBJECT_FLAGg       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/flag.txt");
    if ( type == OBJECT_FLAGy       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/flag.txt");
    if ( type == OBJECT_FLAGv       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/flag.txt");
    if ( type == OBJECT_MARKPOWER   )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/enerspot.txt");
    if ( type == OBJECT_MARKSTONE   )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/stonspot.txt");
    if ( type == OBJECT_MARKURANIUM )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/uranspot.txt");
    if ( type == OBJECT_MOBILEwa    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botgr.txt");
    if ( type == OBJECT_MOBILEta    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botgc.txt");
    if ( type == OBJECT_MOBILEfa    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botgj.txt");
    if ( type == OBJECT_MOBILEia    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botgs.txt");
    if ( type == OBJECT_MOBILEws    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botsr.txt");
    if ( type == OBJECT_MOBILEts    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botsc.txt");
    if ( type == OBJECT_MOBILEfs    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botsj.txt");
    if ( type == OBJECT_MOBILEis    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botss.txt");
    if ( type == OBJECT_MOBILEwi    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botor.txt");
    if ( type == OBJECT_MOBILEti    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botoc.txt");
    if ( type == OBJECT_MOBILEfi    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botoj.txt");
    if ( type == OBJECT_MOBILEii    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botos.txt");
    if ( type == OBJECT_MOBILEwc    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botfr.txt");
    if ( type == OBJECT_MOBILEtc    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botfc.txt");
    if ( type == OBJECT_MOBILEfc    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botfj.txt");
    if ( type == OBJECT_MOBILEic    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botfs.txt");
    if ( type == OBJECT_MOBILErt    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/bottump.txt");
    if ( type == OBJECT_MOBILErc    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botphaz.txt");
    if ( type == OBJECT_MOBILErr    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botrecy.txt");
    if ( type == OBJECT_MOBILErs    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botshld.txt");
    if ( type == OBJECT_MOBILEsa    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botsub.txt");
    if ( type == OBJECT_MOBILEwt    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/bottr.txt");
    if ( type == OBJECT_MOBILEtg    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/bottarg.txt");
    if ( type == OBJECT_MOBILEdr    )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/botdraw.txt");
    if ( type == OBJECT_APOLLO2     )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/lrv.txt");
    if ( type == OBJECT_HUMAN       )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/human.txt");
    if ( type == OBJECT_MOTHER      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/mother.txt");
    if ( type == OBJECT_EGG         )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/egg.txt");
    if ( type == OBJECT_ANT         )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/ant.txt");
    if ( type == OBJECT_SPIDER      )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/spider.txt");
    if ( type == OBJECT_BEE         )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/wasp.txt");
    if ( type == OBJECT_WORM        )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/worm.txt");
    if ( type == OBJECT_RUINmobilew1)  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/object/wreck.txt");
    return "";
}


// Returns the help file to use for instruction.

std::string GetHelpFilename(const char *token)
{
    if ( strcmp(token, "if"            ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/if.txt");
    if ( strcmp(token, "else"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/if.txt");
    if ( strcmp(token, "for"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/for.txt");
    if ( strcmp(token, "while"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/while.txt");
    if ( strcmp(token, "do"            ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/do.txt");
    if ( strcmp(token, "break"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/break.txt");
    if ( strcmp(token, "continue"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/continue.txt");
    if ( strcmp(token, "return"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/return.txt");
    if ( strcmp(token, "sizeof"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/sizeof.txt");
    if ( strcmp(token, "int"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/int.txt");
    if ( strcmp(token, "float"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/float.txt");
    if ( strcmp(token, "bool"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/bool.txt");
    if ( strcmp(token, "string"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "point"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/point.txt");
    if ( strcmp(token, "object"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/object.txt");
    if ( strcmp(token, "file"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/file.txt");
    if ( strcmp(token, "void"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/void.txt");
    if ( strcmp(token, "null"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/null.txt");
    if ( strcmp(token, "nan"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/nan.txt");
    if ( strcmp(token, "true"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/true.txt");
    if ( strcmp(token, "false"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/false.txt");
    if ( strcmp(token, "sin"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "cos"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "tan"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "asin"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "acos"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "atan"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "sqrt"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "pow"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "rand"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "abs"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/expr.txt");
    if ( strcmp(token, "getbuild"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/getbuild.txt");
    if ( strcmp(token, "getresearchenable" ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/getresen.txt");
    if ( strcmp(token, "getresearchdone"   ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/getresdo.txt");
    if ( strcmp(token, "retobject"     ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/retobj.txt");
    if ( strcmp(token, "progfunc"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/factory.txt");
    if ( strcmp(token, "busy"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/busy.txt");
    if ( strcmp(token, "takeoff"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/takeoff.txt");
    if ( strcmp(token, "research"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/research.txt");
    if ( strcmp(token, "factory"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/factory.txt");
    if ( strcmp(token, "destroy"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/destroy.txt");
    if ( strcmp(token, "search"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/search.txt");
    if ( strcmp(token, "radar"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/radar.txt");
    if ( strcmp(token, "direction"     ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/direct.txt");
    if ( strcmp(token, "distance"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/dist.txt");
    if ( strcmp(token, "distance2d"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/dist2d.txt");
    if ( strcmp(token, "space"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/space.txt");
    if ( strcmp(token, "flatground"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/flatgrnd.txt");
    if ( strcmp(token, "wait"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/wait.txt");
    if ( strcmp(token, "move"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/move.txt");
    if ( strcmp(token, "turn"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/turn.txt");
    if ( strcmp(token, "goto"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/goto.txt");
    if ( strcmp(token, "grab"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/grab.txt");
    if ( strcmp(token, "canbuild"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/canbuild.txt");
    if ( strcmp(token, "build"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/build.txt");
    if ( strcmp(token, "drop"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/drop.txt");
    if ( strcmp(token, "sniff"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/sniff.txt");
    if ( strcmp(token, "receive"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/receive.txt");
    if ( strcmp(token, "send"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/send.txt");
    if ( strcmp(token, "deleteinfo"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/delinfo.txt");
    if ( strcmp(token, "testinfo"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/testinfo.txt");
    if ( strcmp(token, "thump"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/thump.txt");
    if ( strcmp(token, "recycle"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/recycle.txt");
    if ( strcmp(token, "shield"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/shield.txt");
    if ( strcmp(token, "fire"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/fire.txt");
    if ( strcmp(token, "antfire"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/antfire.txt");
    if ( strcmp(token, "aim"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/aim.txt");
    if ( strcmp(token, "motor"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/motor.txt");
    if ( strcmp(token, "jet"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/jet.txt");
    if ( strcmp(token, "topo"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/topo.txt");
    if ( strcmp(token, "message"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/message.txt");
    if ( strcmp(token, "abstime"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/abstime.txt");
    if ( strcmp(token, "BlackArrow"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "RedArrow"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "White"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Black"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Gray"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "LightGray"     ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Red"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Pink"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Purple"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Orange"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Yellow"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Beige"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Brown"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Skin"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Green"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "LightGreen"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "Blue"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "LightBlue"     ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "InFront"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/grab.txt");
    if ( strcmp(token, "Behind"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/grab.txt");
    if ( strcmp(token, "EnergyCell"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/grab.txt");
    if ( strcmp(token, "DisplayError"  ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/message.txt");
    if ( strcmp(token, "DisplayWarning") == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/message.txt");
    if ( strcmp(token, "DisplayInfo"   ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/message.txt");
    if ( strcmp(token, "DisplayMessage") == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/message.txt");
    if ( strcmp(token, "strlen"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strleft"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strright"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strmid"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strval"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strfind"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strlower"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "strupper"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/string.txt");
    if ( strcmp(token, "open"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/open.txt");
    if ( strcmp(token, "close"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/close.txt");
    if ( strcmp(token, "writeln"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/writeln.txt");
    if ( strcmp(token, "readln "       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/readln.txt");
    if ( strcmp(token, "eof"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/eof.txt");
    if ( strcmp(token, "deletefile"    ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/deletef.txt");
    if ( strcmp(token, "openfile"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/openfile.txt");
    if ( strcmp(token, "pendown"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pendown.txt");
    if ( strcmp(token, "penup"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/penup.txt");
    if ( strcmp(token, "pencolor"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/pencolor.txt");
    if ( strcmp(token, "penwidth"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/penwidth.txt");
    if ( strcmp(token, "camerafocus"      ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/camerafocus.txt");
    if ( strcmp(token, "extern"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/extern.txt");
    if ( strcmp(token, "class"         ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/class.txt");
    if ( strcmp(token, "static"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/static.txt");
    if ( strcmp(token, "public"        ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/public.txt");
    if ( strcmp(token, "private"       ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/private.txt");
    if ( strcmp(token, "synchronized"  ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/synchro.txt");
    if ( strcmp(token, "new"           ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/new.txt");
    if ( strcmp(token, "this"          ) == 0 )  return std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + std::string("/cbot/this.txt");
    return "";
}


// Test if a keyword is a type of variable.

bool IsType(const char *token)
{
    if ( strcmp(token, "void"   ) == 0 )  return true;
    if ( strcmp(token, "int"    ) == 0 )  return true;
    if ( strcmp(token, "float"  ) == 0 )  return true;
    if ( strcmp(token, "bool"   ) == 0 )  return true;
    if ( strcmp(token, "string" ) == 0 )  return true;
    if ( strcmp(token, "point"  ) == 0 )  return true;
    if ( strcmp(token, "object" ) == 0 )  return true;
    if ( strcmp(token, "file"   ) == 0 )  return true;
    if ( strcmp(token, "this"   ) == 0 )  return true;
    return false;
}

// Test if a keyword is a function.

bool IsFunction(const char *token)
{
    if ( strcmp(token, "sin"          ) == 0 )  return true;
    if ( strcmp(token, "cos"          ) == 0 )  return true;
    if ( strcmp(token, "tan"          ) == 0 )  return true;
    if ( strcmp(token, "asin"         ) == 0 )  return true;
    if ( strcmp(token, "acos"         ) == 0 )  return true;
    if ( strcmp(token, "atan"         ) == 0 )  return true;
    if ( strcmp(token, "sqrt"         ) == 0 )  return true;
    if ( strcmp(token, "pow"          ) == 0 )  return true;
    if ( strcmp(token, "rand"         ) == 0 )  return true;
    if ( strcmp(token, "abs"          ) == 0 )  return true;
    if ( strcmp(token, "getbuild"     ) == 0 )  return true;
    if ( strcmp(token, "getresearchenable" ) == 0 )  return true;
    if ( strcmp(token, "getresearchdone"   ) == 0 )  return true;
    if ( strcmp(token, "retobjectbyid") == 0 )  return true;
    if ( strcmp(token, "retobject"    ) == 0 )  return true;
    if ( strcmp(token, "progfunc"     ) == 0 )  return true;
    if ( strcmp(token, "busy"         ) == 0 )  return true;
    if ( strcmp(token, "factory"      ) == 0 )  return true;
    if ( strcmp(token, "research"     ) == 0 )  return true;
    if ( strcmp(token, "takeoff"      ) == 0 )  return true;
    if ( strcmp(token, "destroy"      ) == 0 )  return true;
    if ( strcmp(token, "search"       ) == 0 )  return true;
    if ( strcmp(token, "radar"        ) == 0 )  return true;
    if ( strcmp(token, "detect"       ) == 0 )  return true;
    if ( strcmp(token, "direction"    ) == 0 )  return true;
    if ( strcmp(token, "distance"     ) == 0 )  return true;
    if ( strcmp(token, "distance2d"   ) == 0 )  return true;
    if ( strcmp(token, "space"        ) == 0 )  return true;
    if ( strcmp(token, "flatground"   ) == 0 )  return true;
    if ( strcmp(token, "wait"         ) == 0 )  return true;
    if ( strcmp(token, "move"         ) == 0 )  return true;
    if ( strcmp(token, "turn"         ) == 0 )  return true;
    if ( strcmp(token, "goto"         ) == 0 )  return true;
    if ( strcmp(token, "grab"         ) == 0 )  return true;
    if ( strcmp(token, "canbuild"     ) == 0 )  return true;
    if ( strcmp(token, "build"        ) == 0 )  return true;
    if ( strcmp(token, "drop"         ) == 0 )  return true;
    if ( strcmp(token, "sniff"        ) == 0 )  return true;
    if ( strcmp(token, "receive"      ) == 0 )  return true;
    if ( strcmp(token, "send"         ) == 0 )  return true;
    if ( strcmp(token, "deleteinfo"   ) == 0 )  return true;
    if ( strcmp(token, "testinfo"     ) == 0 )  return true;
    if ( strcmp(token, "thump"        ) == 0 )  return true;
    if ( strcmp(token, "recycle"      ) == 0 )  return true;
    if ( strcmp(token, "shield"       ) == 0 )  return true;
    if ( strcmp(token, "fire"         ) == 0 )  return true;
    if ( strcmp(token, "antfire"      ) == 0 )  return true;
    if ( strcmp(token, "aim"          ) == 0 )  return true;
    if ( strcmp(token, "motor"        ) == 0 )  return true;
    if ( strcmp(token, "jet"          ) == 0 )  return true;
    if ( strcmp(token, "topo"         ) == 0 )  return true;
    if ( strcmp(token, "message"      ) == 0 )  return true;
    if ( strcmp(token, "abstime"      ) == 0 )  return true;
    if ( strcmp(token, "ismovie"      ) == 0 )  return true;
    if ( strcmp(token, "errmode"      ) == 0 )  return true;
    if ( strcmp(token, "ipf"          ) == 0 )  return true;
    if ( strcmp(token, "strlen"       ) == 0 )  return true;
    if ( strcmp(token, "strleft"      ) == 0 )  return true;
    if ( strcmp(token, "strright"     ) == 0 )  return true;
    if ( strcmp(token, "strmid"       ) == 0 )  return true;
    if ( strcmp(token, "strval"       ) == 0 )  return true;
    if ( strcmp(token, "strfind"      ) == 0 )  return true;
    if ( strcmp(token, "strlower"     ) == 0 )  return true;
    if ( strcmp(token, "strupper"     ) == 0 )  return true;
    if ( strcmp(token, "open"         ) == 0 )  return true;
    if ( strcmp(token, "close"        ) == 0 )  return true;
    if ( strcmp(token, "writeln"      ) == 0 )  return true;
    if ( strcmp(token, "readln"       ) == 0 )  return true;
    if ( strcmp(token, "eof"          ) == 0 )  return true;
    if ( strcmp(token, "deletefile"   ) == 0 )  return true;
    if ( strcmp(token, "openfile"     ) == 0 )  return true;
    if ( strcmp(token, "pendown"      ) == 0 )  return true;
    if ( strcmp(token, "penup"        ) == 0 )  return true;
    if ( strcmp(token, "pencolor"     ) == 0 )  return true;
    if ( strcmp(token, "penwidth"     ) == 0 )  return true;
    if ( strcmp(token, "camerafocus"  ) == 0 )  return true;
    if ( strcmp(token, "sizeof"       ) == 0 )  return true;
    return false;
}


// Returns using a compact instruction.

const char* GetHelpText(const char *token)
{
    if ( strcmp(token, "if"        ) == 0 )  return "if ( condition ) { bloc }";
    if ( strcmp(token, "else"      ) == 0 )  return "else { bloc }";
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
    if ( strcmp(token, "getbuild"  ) == 0 )  return "getbuild ( );";
    if ( strcmp(token, "getresearchenable" ) == 0 )  return "getresearchenable ( );";
    if ( strcmp(token, "getresearchdone" ) == 0 )  return "getresearchdone ( );";
    if ( strcmp(token, "retobject" ) == 0 )  return "retobject ( rank );";
    if ( strcmp(token, "retobjectbyid") == 0 )  return "retobjectbyid ( rank );";
    if ( strcmp(token, "progfunc"  ) == 0 )  return "progfunc ( funcname );";
    if ( strcmp(token, "busy"      ) == 0 )  return "object.busy ( );";
    if ( strcmp(token, "factory"   ) == 0 )  return "object.factory ( cat, program );";
    if ( strcmp(token, "research"  ) == 0 )  return "object.research ( type );";
    if ( strcmp(token, "takeoff"   ) == 0 )  return "object.takeoff ( );";
    if ( strcmp(token, "destroy"   ) == 0 )  return "object.destroy ( );";
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
    if ( strcmp(token, "grab"      ) == 0 )  return "grab ( order );";
    if ( strcmp(token, "canbuild"  ) == 0 )  return "canbuild ( category );";
    if ( strcmp(token, "build"     ) == 0 )  return "build ( category );";
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
    //if ( strcmp(token, "antfire"   ) == 0 )  return "antfire ( );";
    if ( strcmp(token, "aim"       ) == 0 )  return "aim ( x, y );";
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
    if ( strcmp(token, "camerafocus") == 0 )  return "camerafocus ( object );";
    return "";
}

