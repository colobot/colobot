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

#include "object/level/parserline.h"


#include "app/app.h"
#include "common/logger.h"
#include "common/resources/resourcemanager.h"
#include "object/level/parser.h"
#include "object/robotmain.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

CLevelParserParam::CLevelParserParam(std::string name, std::string value)
{
    m_name = name;
    m_value = value;
    m_empty = false;
}

CLevelParserParam::CLevelParserParam(std::string name, bool empty)
{
    assert(empty == true); // we need a second argument because we don't want to create param with value "name"
    m_name = name;
    m_value = "";
    m_empty = true;
}

CLevelParserParam::~CLevelParserParam()
{
    for(auto& a : m_array)
        delete a;
}

void CLevelParserParam::SetLine(CLevelParserLine* line)
{
    m_line = line;
}

CLevelParserLine* CLevelParserParam::GetLine()
{
    return m_line;
}

std::string CLevelParserParam::GetName()
{
    return m_name;
}

std::string CLevelParserParam::GetValue()
{
    return m_value;
}

bool CLevelParserParam::IsDefined()
{
    return !m_empty;
}

template<typename T>
T CLevelParserParam::Cast(std::string value, std::string requestedType)
{
    try {
        return boost::lexical_cast<T>(value);
    }
    catch(...)
    {
        throw CLevelParserExceptionBadParam(this, requestedType);
    }
}

template<typename T>
T CLevelParserParam::Cast(std::string requestedType)
{
    return Cast<T>(m_value, requestedType);
}


int CLevelParserParam::AsInt()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return Cast<int>("int");
}


int CLevelParserParam::AsInt(int def)
{
    if(m_empty)
        return def;
    return AsInt();
}


float CLevelParserParam::AsFloat()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return Cast<float>("float");
}

float CLevelParserParam::AsFloat(float def)
{
    if(m_empty)
        return def;
    return AsFloat();
}


std::string CLevelParserParam::AsString()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    if((m_value[0] == '\"' && m_value[m_value.length()-1] == '\"') || (m_value[0] == '\'' && m_value[m_value.length()-1] == '\''))
    {
        return m_value.substr(1, m_value.length()-2);
    } else {
        throw CLevelParserExceptionBadParam(this, "string");
    }
}

std::string CLevelParserParam::AsString(std::string def)
{
    if(m_empty)
        return def;
    return AsString();
}


bool CLevelParserParam::AsBool()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    std::string value = m_value;
    boost::to_lower(value);
    if(value == "true") return true;
    if(value == "false") return false;
    return Cast<bool>("bool");
}

bool CLevelParserParam::AsBool(bool def)
{
    if(m_empty)
        return def;
    return AsBool();
}


std::string CLevelParserParam::InjectLevelDir(std::string path, const std::string defaultDir)
{
    std::string newPath = path;
    std::string lvlDir = CLevelParser::BuildSceneName(CRobotMain::GetInstancePointer()->GetSceneName(), CRobotMain::GetInstancePointer()->GetSceneRank()/100, CRobotMain::GetInstancePointer()->GetSceneRank()%100, false);
    boost::replace_all(newPath, "%lvl%", lvlDir);
    std::string chapDir = CLevelParser::BuildSceneName(CRobotMain::GetInstancePointer()->GetSceneName(), CRobotMain::GetInstancePointer()->GetSceneRank()/100, 0, false);
    boost::replace_all(newPath, "%chap%", chapDir);
    if(newPath == path && !path.empty())
    {
        newPath = defaultDir + (!defaultDir.empty() ? "/" : "") + newPath;
    }
    
    std::string langPath = newPath;
    std::string langStr(1, CApplication::GetInstancePointer()->GetLanguageChar());
    boost::replace_all(langPath, "%lng%", langStr);
    if(CResourceManager::Exists(langPath))
        return langPath;
    
    // Fallback to English if file doesn't exist
    boost::replace_all(newPath, "%lng%", "E");
    if(CResourceManager::Exists(newPath))
        return newPath;
    
    return langPath; // Return current language file if none of the files exist
}

std::string CLevelParserParam::ToPath(std::string path, const std::string defaultDir)
{
    if(defaultDir == "" && path.find("%lvl%") != std::string::npos)
        throw CLevelParserException("TODO: Param "+m_name+" does not yet support %lvl%! :(");
    
    return InjectLevelDir(path, defaultDir);
}

std::string CLevelParserParam::AsPath(const std::string defaultDir)
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    
    return ToPath(AsString(), defaultDir);
}

std::string CLevelParserParam::AsPath(const std::string defaultDir, std::string def)
{
    if(m_empty)
        return InjectLevelDir(def, defaultDir);
    
    return ToPath(AsString(def), defaultDir);
}


Gfx::Color CLevelParserParam::AsColor()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    
    ParseArray();
    
    if(m_array.size() == 3) { //RGB
        return Gfx::Color(m_array[0]->AsFloat(), m_array[1]->AsFloat(), m_array[2]->AsFloat());
    } else if(m_array.size() == 4) { //RGBA
        return Gfx::Color(m_array[0]->AsFloat(), m_array[1]->AsFloat(), m_array[2]->AsFloat(), m_array[3]->AsFloat());
    } else {
        throw CLevelParserExceptionBadParam(this, "color");
    }
}

Gfx::Color CLevelParserParam::AsColor(Gfx::Color def)
{
    if(m_empty)
        return def;
    return AsColor();
}


Math::Vector CLevelParserParam::AsPoint()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    
    ParseArray();
    
    if(m_array.size() == 2) { //XZ
        return Math::Vector(m_array[0]->AsFloat(), 0.0f, m_array[1]->AsFloat());
    } else if(m_array.size() == 3) { //XYZ
        return Math::Vector(m_array[0]->AsFloat(), m_array[1]->AsFloat(), m_array[2]->AsFloat());
    } else {
        throw CLevelParserExceptionBadParam(this, "point");
    }
}

Math::Vector CLevelParserParam::AsPoint(Math::Vector def)
{
    if(m_empty)
        return def;
    return AsPoint();
}


ObjectType CLevelParserParam::ToObjectType(std::string value)
{
    if(value == "All"               ) return OBJECT_NULL;
    if(value == "Portico"           ) return OBJECT_PORTICO;
    if(value == "SpaceShip"         ) return OBJECT_BASE;
    if(value == "PracticeBot"       ) return OBJECT_MOBILEwt;
    if(value == "WingedGrabber"     ) return OBJECT_MOBILEfa;
    if(value == "TrackedGrabber"    ) return OBJECT_MOBILEta;
    if(value == "WheeledGrabber"    ) return OBJECT_MOBILEwa;
    if(value == "LeggedGrabber"     ) return OBJECT_MOBILEia;
    if(value == "WingedShooter"     ) return OBJECT_MOBILEfc;
    if(value == "TrackedShooter"    ) return OBJECT_MOBILEtc;
    if(value == "WheeledShooter"    ) return OBJECT_MOBILEwc;
    if(value == "LeggedShooter"     ) return OBJECT_MOBILEic;
    if(value == "WingedOrgaShooter" ) return OBJECT_MOBILEfi;
    if(value == "TrackedOrgaShooter") return OBJECT_MOBILEti;
    if(value == "WheeledOrgaShooter") return OBJECT_MOBILEwi;
    if(value == "LeggedOrgaShooter" ) return OBJECT_MOBILEii;
    if(value == "WingedSniffer"     ) return OBJECT_MOBILEfs;
    if(value == "TrackedSniffer"    ) return OBJECT_MOBILEts;
    if(value == "WheeledSniffer"    ) return OBJECT_MOBILEws;
    if(value == "LeggedSniffer"     ) return OBJECT_MOBILEis;
    if(value == "Thumper"           ) return OBJECT_MOBILErt;
    if(value == "PhazerShooter"     ) return OBJECT_MOBILErc;
    if(value == "Recycler"          ) return OBJECT_MOBILErr;
    if(value == "Shielder"          ) return OBJECT_MOBILErs;
    if(value == "Subber"            ) return OBJECT_MOBILEsa;
    if(value == "TargetBot"         ) return OBJECT_MOBILEtg;
    if(value == "Scribbler"         ) return OBJECT_MOBILEdr;
    if(value == "PowerSpot"         ) return OBJECT_MARKPOWER;
    if(value == "TitaniumSpot"      ) return OBJECT_MARKSTONE;
    if(value == "UraniumSpot"       ) return OBJECT_MARKURANIUM;
    if(value == "PlatinumSpot"      ) return OBJECT_MARKURANIUM;
    if(value == "KeyASpot"          ) return OBJECT_MARKKEYa;
    if(value == "KeyBSpot"          ) return OBJECT_MARKKEYb;
    if(value == "KeyCSpot"          ) return OBJECT_MARKKEYc;
    if(value == "KeyDSpot"          ) return OBJECT_MARKKEYd;
    if(value == "WayPoint"          ) return OBJECT_WAYPOINT;
    if(value == "BlueFlag"          ) return OBJECT_FLAGb;
    if(value == "RedFlag"           ) return OBJECT_FLAGr;
    if(value == "GreenFlag"         ) return OBJECT_FLAGg;
    if(value == "YellowFlag"        ) return OBJECT_FLAGy;
    if(value == "VioletFlag"        ) return OBJECT_FLAGv;
    if(value == "PowerCell"         ) return OBJECT_POWER;
    if(value == "FuelCellPlant"     ) return OBJECT_NUCLEAR;
    if(value == "FuelCell"          ) return OBJECT_ATOMIC;
    if(value == "NuclearCell"       ) return OBJECT_ATOMIC;
    if(value == "TitaniumOre"       ) return OBJECT_STONE;
    if(value == "UraniumOre"        ) return OBJECT_URANIUM;
    if(value == "PlatinumOre"       ) return OBJECT_URANIUM;
    if(value == "Titanium"          ) return OBJECT_METAL;
    if(value == "OrgaMatter"        ) return OBJECT_BULLET;
    if(value == "BlackBox"          ) return OBJECT_BBOX;
    if(value == "KeyA"              ) return OBJECT_KEYa;
    if(value == "KeyB"              ) return OBJECT_KEYb;
    if(value == "KeyC"              ) return OBJECT_KEYc;
    if(value == "KeyD"              ) return OBJECT_KEYd;
    if(value == "TNT"               ) return OBJECT_TNT;
    if(value == "Scrap1"            ) return OBJECT_SCRAP1;
    if(value == "Scrap2"            ) return OBJECT_SCRAP2;
    if(value == "Scrap3"            ) return OBJECT_SCRAP3;
    if(value == "Scrap4"            ) return OBJECT_SCRAP4;
    if(value == "Scrap5"            ) return OBJECT_SCRAP5;
    if(value == "Mine"              ) return OBJECT_BOMB;
    if(value == "Firework"          ) return OBJECT_WINFIRE;
    if(value == "Bag"               ) return OBJECT_BAG;
    if(value == "Greenery0"         ) return OBJECT_PLANT0;
    if(value == "Greenery1"         ) return OBJECT_PLANT1;
    if(value == "Greenery2"         ) return OBJECT_PLANT2;
    if(value == "Greenery3"         ) return OBJECT_PLANT3;
    if(value == "Greenery4"         ) return OBJECT_PLANT4;
    if(value == "Greenery5"         ) return OBJECT_PLANT5;
    if(value == "Greenery6"         ) return OBJECT_PLANT6;
    if(value == "Greenery7"         ) return OBJECT_PLANT7;
    if(value == "Greenery8"         ) return OBJECT_PLANT8;
    if(value == "Greenery9"         ) return OBJECT_PLANT9;
    if(value == "Greenery10"        ) return OBJECT_PLANT10;
    if(value == "Greenery11"        ) return OBJECT_PLANT11;
    if(value == "Greenery12"        ) return OBJECT_PLANT12;
    if(value == "Greenery13"        ) return OBJECT_PLANT13;
    if(value == "Greenery14"        ) return OBJECT_PLANT14;
    if(value == "Greenery15"        ) return OBJECT_PLANT15;
    if(value == "Greenery16"        ) return OBJECT_PLANT16;
    if(value == "Greenery17"        ) return OBJECT_PLANT17;
    if(value == "Greenery18"        ) return OBJECT_PLANT18;
    if(value == "Greenery19"        ) return OBJECT_PLANT19;
    if(value == "Tree0"             ) return OBJECT_TREE0;
    if(value == "Tree1"             ) return OBJECT_TREE1;
    if(value == "Tree2"             ) return OBJECT_TREE2;
    if(value == "Tree3"             ) return OBJECT_TREE3;
    if(value == "Tree4"             ) return OBJECT_TREE4;
    if(value == "Tree5"             ) return OBJECT_TREE5;
    if(value == "Mushroom1"         ) return OBJECT_MUSHROOM1;
    if(value == "Mushroom2"         ) return OBJECT_MUSHROOM2;
    if(value == "Home"              ) return OBJECT_HOME1;
    if(value == "Derrick"           ) return OBJECT_DERRICK;
    if(value == "BotFactory"        ) return OBJECT_FACTORY;
    if(value == "PowerStation"      ) return OBJECT_STATION;
    if(value == "Converter"         ) return OBJECT_CONVERT;
    if(value == "RepairCenter"      ) return OBJECT_REPAIR;
    if(value == "Destroyer"         ) return OBJECT_DESTROYER;
    if(value == "DefenseTower"      ) return OBJECT_TOWER;
    if(value == "AlienNest"         ) return OBJECT_NEST;
    if(value == "ResearchCenter"    ) return OBJECT_RESEARCH;
    if(value == "RadarStation"      ) return OBJECT_RADAR;
    if(value == "ExchangePost"      ) return OBJECT_INFO;
    if(value == "PowerPlant"        ) return OBJECT_ENERGY;
    if(value == "AutoLab"           ) return OBJECT_LABO;
    if(value == "NuclearPlant"      ) return OBJECT_NUCLEAR;
    if(value == "PowerCaptor"       ) return OBJECT_PARA;
    if(value == "Vault"             ) return OBJECT_SAFE;
    if(value == "Houston"           ) return OBJECT_HUSTON;
    if(value == "Target1"           ) return OBJECT_TARGET1;
    if(value == "Target2"           ) return OBJECT_TARGET2;
    if(value == "StartArea"         ) return OBJECT_START;
    if(value == "GoalArea"          ) return OBJECT_END;
    if(value == "AlienQueen"        ) return OBJECT_MOTHER;
    if(value == "AlienEgg"          ) return OBJECT_EGG;
    if(value == "AlienAnt"          ) return OBJECT_ANT;
    if(value == "AlienSpider"       ) return OBJECT_SPIDER;
    if(value == "AlienWasp"         ) return OBJECT_BEE;
    if(value == "AlienWorm"         ) return OBJECT_WORM;
    if(value == "WreckBotw1"        ) return OBJECT_RUINmobilew1;
    if(value == "WreckBotw2"        ) return OBJECT_RUINmobilew2;
    if(value == "WreckBott1"        ) return OBJECT_RUINmobilet1;
    if(value == "WreckBott2"        ) return OBJECT_RUINmobilet2;
    if(value == "WreckBotr1"        ) return OBJECT_RUINmobiler1;
    if(value == "WreckBotr2"        ) return OBJECT_RUINmobiler2;
    if(value == "RuinBotFactory"    ) return OBJECT_RUINfactory;
    if(value == "RuinDoor"          ) return OBJECT_RUINdoor;
    if(value == "RuinSupport"       ) return OBJECT_RUINsupport;
    if(value == "RuinRadar"         ) return OBJECT_RUINradar;
    if(value == "RuinConvert"       ) return OBJECT_RUINconvert;
    if(value == "RuinBaseCamp"      ) return OBJECT_RUINbase;
    if(value == "RuinHeadCamp"      ) return OBJECT_RUINhead;
    if(value == "Barrier0"          ) return OBJECT_BARRIER0;
    if(value == "Barrier1"          ) return OBJECT_BARRIER1;
    if(value == "Barrier2"          ) return OBJECT_BARRIER2;
    if(value == "Barrier3"          ) return OBJECT_BARRIER3;
    if(value == "Teen0"             ) return OBJECT_TEEN0;
    if(value == "Teen1"             ) return OBJECT_TEEN1;
    if(value == "Teen2"             ) return OBJECT_TEEN2;
    if(value == "Teen3"             ) return OBJECT_TEEN3;
    if(value == "Teen4"             ) return OBJECT_TEEN4;
    if(value == "Teen5"             ) return OBJECT_TEEN5;
    if(value == "Teen6"             ) return OBJECT_TEEN6;
    if(value == "Teen7"             ) return OBJECT_TEEN7;
    if(value == "Teen8"             ) return OBJECT_TEEN8;
    if(value == "Teen9"             ) return OBJECT_TEEN9;
    if(value == "Teen10"            ) return OBJECT_TEEN10;
    if(value == "Teen11"            ) return OBJECT_TEEN11;
    if(value == "Teen12"            ) return OBJECT_TEEN12;
    if(value == "Teen13"            ) return OBJECT_TEEN13;
    if(value == "Teen14"            ) return OBJECT_TEEN14;
    if(value == "Teen15"            ) return OBJECT_TEEN15;
    if(value == "Teen16"            ) return OBJECT_TEEN16;
    if(value == "Teen17"            ) return OBJECT_TEEN17;
    if(value == "Teen18"            ) return OBJECT_TEEN18;
    if(value == "Teen19"            ) return OBJECT_TEEN19;
    if(value == "Teen20"            ) return OBJECT_TEEN20;
    if(value == "Teen21"            ) return OBJECT_TEEN21;
    if(value == "Teen22"            ) return OBJECT_TEEN22;
    if(value == "Teen23"            ) return OBJECT_TEEN23;
    if(value == "Teen24"            ) return OBJECT_TEEN24;
    if(value == "Teen25"            ) return OBJECT_TEEN25;
    if(value == "Teen26"            ) return OBJECT_TEEN26;
    if(value == "Teen27"            ) return OBJECT_TEEN27;
    if(value == "Teen28"            ) return OBJECT_TEEN28;
    if(value == "Teen29"            ) return OBJECT_TEEN29;
    if(value == "Teen30"            ) return OBJECT_TEEN30;
    if(value == "Teen31"            ) return OBJECT_TEEN31;
    if(value == "Teen32"            ) return OBJECT_TEEN32;
    if(value == "Teen33"            ) return OBJECT_TEEN33;
    if(value == "Stone"             ) return OBJECT_TEEN34;
    if(value == "Teen35"            ) return OBJECT_TEEN35;
    if(value == "Teen36"            ) return OBJECT_TEEN36;
    if(value == "Teen37"            ) return OBJECT_TEEN37;
    if(value == "Teen38"            ) return OBJECT_TEEN38;
    if(value == "Teen39"            ) return OBJECT_TEEN39;
    if(value == "Teen40"            ) return OBJECT_TEEN40;
    if(value == "Teen41"            ) return OBJECT_TEEN41;
    if(value == "Teen42"            ) return OBJECT_TEEN42;
    if(value == "Teen43"            ) return OBJECT_TEEN43;
    if(value == "Teen44"            ) return OBJECT_TEEN44;
    if(value == "Quartz0"           ) return OBJECT_QUARTZ0;
    if(value == "Quartz1"           ) return OBJECT_QUARTZ1;
    if(value == "Quartz2"           ) return OBJECT_QUARTZ2;
    if(value == "Quartz3"           ) return OBJECT_QUARTZ3;
    if(value == "MegaStalk0"        ) return OBJECT_ROOT0;
    if(value == "MegaStalk1"        ) return OBJECT_ROOT1;
    if(value == "MegaStalk2"        ) return OBJECT_ROOT2;
    if(value == "MegaStalk3"        ) return OBJECT_ROOT3;
    if(value == "MegaStalk4"        ) return OBJECT_ROOT4;
    if(value == "MegaStalk5"        ) return OBJECT_ROOT5;
    if(value == "ApolloLEM"         ) return OBJECT_APOLLO1;
    if(value == "ApolloJeep"        ) return OBJECT_APOLLO2;
    if(value == "ApolloFlag"        ) return OBJECT_APOLLO3;
    if(value == "ApolloModule"      ) return OBJECT_APOLLO4;
    if(value == "ApolloAntenna"     ) return OBJECT_APOLLO5;
    if(value == "Me"                ) return OBJECT_HUMAN;
    if(value == "Tech"              ) return OBJECT_TECH;
    if(value == "MissionController" ) return OBJECT_CONTROLLER;
    return static_cast<ObjectType>(Cast<int>(value, "object"));
}

const std::string CLevelParserParam::FromObjectType(ObjectType value)
{
    if(value == OBJECT_PORTICO     ) return "Portico";
    if(value == OBJECT_BASE        ) return "SpaceShip";
    if(value == OBJECT_MOBILEwt    ) return "PracticeBot";
    if(value == OBJECT_MOBILEfa    ) return "WingedGrabber";
    if(value == OBJECT_MOBILEta    ) return "TrackedGrabber";
    if(value == OBJECT_MOBILEwa    ) return "WheeledGrabber";
    if(value == OBJECT_MOBILEia    ) return "LeggedGrabber";
    if(value == OBJECT_MOBILEfc    ) return "WingedShooter";
    if(value == OBJECT_MOBILEtc    ) return "TrackedShooter";
    if(value == OBJECT_MOBILEwc    ) return "WheeledShooter";
    if(value == OBJECT_MOBILEic    ) return "LeggedShooter";
    if(value == OBJECT_MOBILEfi    ) return "WingedOrgaShooter";
    if(value == OBJECT_MOBILEti    ) return "TrackedOrgaShooter";
    if(value == OBJECT_MOBILEwi    ) return "WheeledOrgaShooter";
    if(value == OBJECT_MOBILEii    ) return "LeggedOrgaShooter";
    if(value == OBJECT_MOBILEfs    ) return "WingedSniffer";
    if(value == OBJECT_MOBILEts    ) return "TrackedSniffer";
    if(value == OBJECT_MOBILEws    ) return "WheeledSniffer";
    if(value == OBJECT_MOBILEis    ) return "LeggedSniffer";
    if(value == OBJECT_MOBILErt    ) return "Thumper";
    if(value == OBJECT_MOBILErc    ) return "PhazerShooter";
    if(value == OBJECT_MOBILErr    ) return "Recycler";
    if(value == OBJECT_MOBILErs    ) return "Shielder";
    if(value == OBJECT_MOBILEsa    ) return "Subber";
    if(value == OBJECT_MOBILEtg    ) return "TargetBot";
    if(value == OBJECT_MOBILEdr    ) return "Scribbler";
    if(value == OBJECT_MARKPOWER   ) return "PowerSpot";
    if(value == OBJECT_MARKSTONE   ) return "TitaniumSpot";
    if(value == OBJECT_MARKURANIUM ) return "UraniumSpot";
    if(value == OBJECT_MARKKEYa    ) return "KeyASpot";
    if(value == OBJECT_MARKKEYb    ) return "KeyBSpot";
    if(value == OBJECT_MARKKEYc    ) return "KeyCSpot";
    if(value == OBJECT_MARKKEYd    ) return "KeyDSpot";
    if(value == OBJECT_WAYPOINT    ) return "WayPoint";
    if(value == OBJECT_FLAGb       ) return "BlueFlag";
    if(value == OBJECT_FLAGr       ) return "RedFlag";
    if(value == OBJECT_FLAGg       ) return "GreenFlag";
    if(value == OBJECT_FLAGy       ) return "YellowFlag";
    if(value == OBJECT_FLAGv       ) return "VioletFlag";
    if(value == OBJECT_POWER       ) return "PowerCell";
    if(value == OBJECT_ATOMIC      ) return "NuclearCell";
    if(value == OBJECT_STONE       ) return "TitaniumOre";
    if(value == OBJECT_URANIUM     ) return "UraniumOre";
    if(value == OBJECT_METAL       ) return "Titanium";
    if(value == OBJECT_BULLET      ) return "OrgaMatter";
    if(value == OBJECT_BBOX        ) return "BlackBox";
    if(value == OBJECT_KEYa        ) return "KeyA";
    if(value == OBJECT_KEYb        ) return "KeyB";
    if(value == OBJECT_KEYc        ) return "KeyC";
    if(value == OBJECT_KEYd        ) return "KeyD";
    if(value == OBJECT_TNT         ) return "TNT";
    if(value == OBJECT_SCRAP1      ) return "Scrap1";
    if(value == OBJECT_SCRAP2      ) return "Scrap2";
    if(value == OBJECT_SCRAP3      ) return "Scrap3";
    if(value == OBJECT_SCRAP4      ) return "Scrap4";
    if(value == OBJECT_SCRAP5      ) return "Scrap5";
    if(value == OBJECT_BOMB        ) return "Mine";
    if(value == OBJECT_WINFIRE     ) return "Firework";
    if(value == OBJECT_BAG         ) return "Bag";
    if(value == OBJECT_PLANT0      ) return "Greenery0";
    if(value == OBJECT_PLANT1      ) return "Greenery1";
    if(value == OBJECT_PLANT2      ) return "Greenery2";
    if(value == OBJECT_PLANT3      ) return "Greenery3";
    if(value == OBJECT_PLANT4      ) return "Greenery4";
    if(value == OBJECT_PLANT5      ) return "Greenery5";
    if(value == OBJECT_PLANT6      ) return "Greenery6";
    if(value == OBJECT_PLANT7      ) return "Greenery7";
    if(value == OBJECT_PLANT8      ) return "Greenery8";
    if(value == OBJECT_PLANT9      ) return "Greenery9";
    if(value == OBJECT_PLANT10     ) return "Greenery10";
    if(value == OBJECT_PLANT11     ) return "Greenery11";
    if(value == OBJECT_PLANT12     ) return "Greenery12";
    if(value == OBJECT_PLANT13     ) return "Greenery13";
    if(value == OBJECT_PLANT14     ) return "Greenery14";
    if(value == OBJECT_PLANT15     ) return "Greenery15";
    if(value == OBJECT_PLANT16     ) return "Greenery16";
    if(value == OBJECT_PLANT17     ) return "Greenery17";
    if(value == OBJECT_PLANT18     ) return "Greenery18";
    if(value == OBJECT_PLANT19     ) return "Greenery19";
    if(value == OBJECT_TREE0       ) return "Tree0";
    if(value == OBJECT_TREE1       ) return "Tree1";
    if(value == OBJECT_TREE2       ) return "Tree2";
    if(value == OBJECT_TREE3       ) return "Tree3";
    if(value == OBJECT_TREE4       ) return "Tree4";
    if(value == OBJECT_TREE5       ) return "Tree5";
    if(value == OBJECT_MUSHROOM1   ) return "Mushroom1";
    if(value == OBJECT_MUSHROOM2   ) return "Mushroom2";
    if(value == OBJECT_HOME1       ) return "Home";
    if(value == OBJECT_DERRICK     ) return "Derrick";
    if(value == OBJECT_FACTORY     ) return "BotFactory";
    if(value == OBJECT_STATION     ) return "PowerStation";
    if(value == OBJECT_CONVERT     ) return "Converter";
    if(value == OBJECT_REPAIR      ) return "RepairCenter";
    if(value == OBJECT_DESTROYER   ) return "Destroyer";
    if(value == OBJECT_TOWER       ) return "DefenseTower";
    if(value == OBJECT_NEST        ) return "AlienNest";
    if(value == OBJECT_RESEARCH    ) return "ResearchCenter";
    if(value == OBJECT_RADAR       ) return "RadarStation";
    if(value == OBJECT_INFO        ) return "ExchangePost";
    if(value == OBJECT_ENERGY      ) return "PowerPlant";
    if(value == OBJECT_LABO        ) return "AutoLab";
    if(value == OBJECT_NUCLEAR     ) return "NuclearPlant";
    if(value == OBJECT_PARA        ) return "PowerCaptor";
    if(value == OBJECT_SAFE        ) return "Vault";
    if(value == OBJECT_HUSTON      ) return "Houston";
    if(value == OBJECT_TARGET1     ) return "Target1";
    if(value == OBJECT_TARGET2     ) return "Target2";
    if(value == OBJECT_START       ) return "StartArea";
    if(value == OBJECT_END         ) return "GoalArea";
    if(value == OBJECT_MOTHER      ) return "AlienQueen";
    if(value == OBJECT_EGG         ) return "AlienEgg";
    if(value == OBJECT_ANT         ) return "AlienAnt";
    if(value == OBJECT_SPIDER      ) return "AlienSpider";
    if(value == OBJECT_BEE         ) return "AlienWasp";
    if(value == OBJECT_WORM        ) return "AlienWorm";
    if(value == OBJECT_RUINmobilew1) return "WreckBotw1";
    if(value == OBJECT_RUINmobilew2) return "WreckBotw2";
    if(value == OBJECT_RUINmobilet1) return "WreckBott1";
    if(value == OBJECT_RUINmobilet2) return "WreckBott2";
    if(value == OBJECT_RUINmobiler1) return "WreckBotr1";
    if(value == OBJECT_RUINmobiler2) return "WreckBotr2";
    if(value == OBJECT_RUINfactory ) return "RuinBotFactory";
    if(value == OBJECT_RUINdoor    ) return "RuinDoor";
    if(value == OBJECT_RUINsupport ) return "RuinSupport";
    if(value == OBJECT_RUINradar   ) return "RuinRadar";
    if(value == OBJECT_RUINconvert ) return "RuinConvert";
    if(value == OBJECT_RUINbase    ) return "RuinBaseCamp";
    if(value == OBJECT_RUINhead    ) return "RuinHeadCamp";
    if(value == OBJECT_BARRIER0    ) return "Barrier0";
    if(value == OBJECT_BARRIER1    ) return "Barrier1";
    if(value == OBJECT_BARRIER2    ) return "Barrier2";
    if(value == OBJECT_BARRIER3    ) return "Barrier3";
    if(value == OBJECT_TEEN0       ) return "Teen0";
    if(value == OBJECT_TEEN1       ) return "Teen1";
    if(value == OBJECT_TEEN2       ) return "Teen2";
    if(value == OBJECT_TEEN3       ) return "Teen3";
    if(value == OBJECT_TEEN4       ) return "Teen4";
    if(value == OBJECT_TEEN5       ) return "Teen5";
    if(value == OBJECT_TEEN6       ) return "Teen6";
    if(value == OBJECT_TEEN7       ) return "Teen7";
    if(value == OBJECT_TEEN8       ) return "Teen8";
    if(value == OBJECT_TEEN9       ) return "Teen9";
    if(value == OBJECT_TEEN10      ) return "Teen10";
    if(value == OBJECT_TEEN11      ) return "Teen11";
    if(value == OBJECT_TEEN12      ) return "Teen12";
    if(value == OBJECT_TEEN13      ) return "Teen13";
    if(value == OBJECT_TEEN14      ) return "Teen14";
    if(value == OBJECT_TEEN15      ) return "Teen15";
    if(value == OBJECT_TEEN16      ) return "Teen16";
    if(value == OBJECT_TEEN17      ) return "Teen17";
    if(value == OBJECT_TEEN18      ) return "Teen18";
    if(value == OBJECT_TEEN19      ) return "Teen19";
    if(value == OBJECT_TEEN20      ) return "Teen20";
    if(value == OBJECT_TEEN21      ) return "Teen21";
    if(value == OBJECT_TEEN22      ) return "Teen22";
    if(value == OBJECT_TEEN23      ) return "Teen23";
    if(value == OBJECT_TEEN24      ) return "Teen24";
    if(value == OBJECT_TEEN25      ) return "Teen25";
    if(value == OBJECT_TEEN26      ) return "Teen26";
    if(value == OBJECT_TEEN27      ) return "Teen27";
    if(value == OBJECT_TEEN28      ) return "Teen28";
    if(value == OBJECT_TEEN29      ) return "Teen29";
    if(value == OBJECT_TEEN30      ) return "Teen30";
    if(value == OBJECT_TEEN31      ) return "Teen31";
    if(value == OBJECT_TEEN32      ) return "Teen32";
    if(value == OBJECT_TEEN33      ) return "Teen33";
    if(value == OBJECT_TEEN34      ) return "Stone";
    if(value == OBJECT_TEEN35      ) return "Teen35";
    if(value == OBJECT_TEEN36      ) return "Teen36";
    if(value == OBJECT_TEEN37      ) return "Teen37";
    if(value == OBJECT_TEEN38      ) return "Teen38";
    if(value == OBJECT_TEEN39      ) return "Teen39";
    if(value == OBJECT_TEEN40      ) return "Teen40";
    if(value == OBJECT_TEEN41      ) return "Teen41";
    if(value == OBJECT_TEEN42      ) return "Teen42";
    if(value == OBJECT_TEEN43      ) return "Teen43";
    if(value == OBJECT_TEEN44      ) return "Teen44";
    if(value == OBJECT_QUARTZ0     ) return "Quartz0";
    if(value == OBJECT_QUARTZ1     ) return "Quartz1";
    if(value == OBJECT_QUARTZ2     ) return "Quartz2";
    if(value == OBJECT_QUARTZ3     ) return "Quartz3";
    if(value == OBJECT_ROOT0       ) return "MegaStalk0";
    if(value == OBJECT_ROOT1       ) return "MegaStalk1";
    if(value == OBJECT_ROOT2       ) return "MegaStalk2";
    if(value == OBJECT_ROOT3       ) return "MegaStalk3";
    if(value == OBJECT_ROOT4       ) return "MegaStalk4";
    if(value == OBJECT_ROOT5       ) return "MegaStalk5";
    if(value == OBJECT_APOLLO1     ) return "ApolloLEM";
    if(value == OBJECT_APOLLO2     ) return "ApolloJeep";
    if(value == OBJECT_APOLLO3     ) return "ApolloFlag";
    if(value == OBJECT_APOLLO4     ) return "ApolloModule";
    if(value == OBJECT_APOLLO5     ) return "ApolloAntenna";
    if(value == OBJECT_HUMAN       ) return "Me";
    if(value == OBJECT_TECH        ) return "Tech";
    if(value == OBJECT_CONTROLLER  ) return "MissionController";
    return boost::lexical_cast<std::string>(static_cast<int>(value));
}

ObjectType CLevelParserParam::AsObjectType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToObjectType(m_value);
}

ObjectType CLevelParserParam::AsObjectType(ObjectType def)
{
    if(m_empty)
        return def;
    return AsObjectType();
}


DriveType CLevelParserParam::ToDriveType(std::string value)
{
    if(value == "Wheeled") return DRIVE_WHEELED;
    if(value == "Tracked") return DRIVE_TRACKED;
    if(value == "Winged" ) return DRIVE_WINGED;
    if(value == "Legged" ) return DRIVE_LEGGED;
    if(value == "Other"  ) return DRIVE_OTHER;
    return static_cast<DriveType>(Cast<int>(value, "drive"));
}

DriveType CLevelParserParam::AsDriveType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToDriveType(m_value);
}

DriveType CLevelParserParam::AsDriveType(DriveType def)
{
    if(m_empty)
        return def;
    return AsDriveType();
}


ToolType CLevelParserParam::ToToolType(std::string value)
{
    if(value == "Grabber"    ) return TOOL_GRABBER;
    if(value == "Shiffer"    ) return TOOL_SNIFFER;
    if(value == "Shooter"    ) return TOOL_SHOOTER;
    if(value == "OrgaShooter") return TOOL_ORGASHOOTER;
    if(value == "Other"      ) return TOOL_OTHER;
    return static_cast<ToolType>(Cast<int>(value, "tool"));
}

ToolType CLevelParserParam::AsToolType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToToolType(m_value);
}

ToolType CLevelParserParam::AsToolType(ToolType def)
{
    if(m_empty)
        return def;
    return AsToolType();
}


Gfx::WaterType CLevelParserParam::ToWaterType(std::string value)
{
    if(value == "NULL") return Gfx::WATER_NULL;
    if(value == "TT"  ) return Gfx::WATER_TT;
    if(value == "TO"  ) return Gfx::WATER_TO;
    if(value == "CT"  ) return Gfx::WATER_CT;
    if(value == "CO"  ) return Gfx::WATER_CO;
    return static_cast<Gfx::WaterType>(Cast<int>(value, "watertype"));
}

Gfx::WaterType CLevelParserParam::AsWaterType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToWaterType(m_value);
}

Gfx::WaterType CLevelParserParam::AsWaterType(Gfx::WaterType def)
{
    if(m_empty)
        return def;
    return AsWaterType();
}


Gfx::EngineObjectType CLevelParserParam::ToTerrainType(std::string value)
{
    if(value == "Terrain") return Gfx::ENG_OBJTYPE_TERRAIN;
    if(value == "Object" ) return Gfx::ENG_OBJTYPE_FIX;
    if(value == "Quartz" ) return Gfx::ENG_OBJTYPE_QUARTZ;
    if(value == "Metal"  ) return Gfx::ENG_OBJTYPE_METAL;
    return static_cast<Gfx::EngineObjectType>(Cast<int>(value, "terraintype"));
}

Gfx::EngineObjectType CLevelParserParam::AsTerrainType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToTerrainType(m_value);
}

Gfx::EngineObjectType CLevelParserParam::AsTerrainType(Gfx::EngineObjectType def)
{
    if(m_empty)
        return def;
    return AsTerrainType();
}


int CLevelParserParam::ToBuildFlag(std::string value)
{
    if(value == "BotFactory"    ) return BUILD_FACTORY;
    if(value == "Derrick"       ) return BUILD_DERRICK;
    if(value == "Converter"     ) return BUILD_CONVERT;
    if(value == "RadarStation"  ) return BUILD_RADAR;
    if(value == "PowerPlant"    ) return BUILD_ENERGY;
    if(value == "NuclearPlant"  ) return BUILD_NUCLEAR;
    if(value == "FuelCellPlant" ) return BUILD_NUCLEAR;
    if(value == "PowerStation"  ) return BUILD_STATION;
    if(value == "RepairCenter"  ) return BUILD_REPAIR;
    if(value == "DefenseTower"  ) return BUILD_TOWER;
    if(value == "ResearchCenter") return BUILD_RESEARCH;
    if(value == "AutoLab"       ) return BUILD_LABO;
    if(value == "PowerCaptor"   ) return BUILD_PARA;
    if(value == "ExchangePost"  ) return BUILD_INFO;
    if(value == "Destroyer"     ) return BUILD_DESTROYER;
    if(value == "FlatGround"    ) return BUILD_GFLAT;
    if(value == "Flag"          ) return BUILD_FLAG;
    return Cast<int>(value, "buildflag");
}

int CLevelParserParam::AsBuildFlag()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToBuildFlag(m_value);
}

int CLevelParserParam::AsBuildFlag(int def)
{
    if(m_empty)
        return def;
    return AsBuildFlag();
}


int CLevelParserParam::ToResearchFlag(std::string value)
{
    if(value == "TRACKER" ) return RESEARCH_TANK;
    if(value == "WINGER"  ) return RESEARCH_FLY;
    if(value == "THUMPER" ) return RESEARCH_THUMP;
    if(value == "SHOOTER" ) return RESEARCH_CANON;
    if(value == "TOWER"   ) return RESEARCH_TOWER;
    if(value == "PHAZER"  ) return RESEARCH_PHAZER;
    if(value == "SHIELDER") return RESEARCH_SHIELD;
    if(value == "ATOMIC"  ) return RESEARCH_ATOMIC;
    if(value == "iPAW"    ) return RESEARCH_iPAW;
    if(value == "iGUN"    ) return RESEARCH_iGUN;
    if(value == "RECYCLER") return RESEARCH_RECYCLER;
    if(value == "SUBBER"  ) return RESEARCH_SUBM;
    if(value == "SNIFFER" ) return RESEARCH_SNIFFER;
    return Cast<int>(value, "researchflag");
}

int CLevelParserParam::AsResearchFlag()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToResearchFlag(m_value);
}

int CLevelParserParam::AsResearchFlag(int def)
{
    if(m_empty)
        return def;
    return AsResearchFlag();
}


Gfx::PyroType CLevelParserParam::ToPyroType(std::string value)
{
    if(value == "FRAGt" ) return Gfx::PT_FRAGT;
    if(value == "FRAGo" ) return Gfx::PT_FRAGO;
    if(value == "FRAGw" ) return Gfx::PT_FRAGW;
    if(value == "EXPLOt") return Gfx::PT_EXPLOT;
    if(value == "EXPLOo") return Gfx::PT_EXPLOO;
    if(value == "EXPLOw") return Gfx::PT_EXPLOW;
    if(value == "SHOTt" ) return Gfx::PT_SHOTT;
    if(value == "SHOTh" ) return Gfx::PT_SHOTH;
    if(value == "SHOTm" ) return Gfx::PT_SHOTM;
    if(value == "SHOTw" ) return Gfx::PT_SHOTW;
    if(value == "EGG"   ) return Gfx::PT_EGG;
    if(value == "BURNt" ) return Gfx::PT_BURNT;
    if(value == "BURNo" ) return Gfx::PT_BURNO;
    if(value == "SPIDER") return Gfx::PT_SPIDER;
    if(value == "FALL"  ) return Gfx::PT_FALL;
    if(value == "RESET" ) return Gfx::PT_RESET;
    if(value == "WIN"   ) return Gfx::PT_WIN;
    if(value == "LOST"  ) return Gfx::PT_LOST;
    return static_cast<Gfx::PyroType>(Cast<int>(value, "pyrotype"));
}

Gfx::PyroType CLevelParserParam::AsPyroType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToPyroType(m_value);
}

Gfx::PyroType CLevelParserParam::AsPyroType(Gfx::PyroType def)
{
    if(m_empty)
        return def;
    return AsPyroType();
}


Gfx::CameraType CLevelParserParam::ToCameraType(std::string value)
{
    if(value == "BACK"   ) return Gfx::CAM_TYPE_BACK;
    if(value == "PLANE"  ) return Gfx::CAM_TYPE_PLANE;
    if(value == "ONBOARD") return Gfx::CAM_TYPE_ONBOARD;
    if(value == "FIX"    ) return Gfx::CAM_TYPE_FIX;
    return static_cast<Gfx::CameraType>(Cast<int>(value, "camera"));
}

const std::string CLevelParserParam::FromCameraType(Gfx::CameraType value)
{
    if(value == Gfx::CAM_TYPE_ONBOARD) return "ONBOARD";
    if(value == Gfx::CAM_TYPE_FIX    ) return "FIX";
    return boost::lexical_cast<std::string>(static_cast<int>(value));
}

Gfx::CameraType CLevelParserParam::AsCameraType()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToCameraType(m_value);
}

Gfx::CameraType CLevelParserParam::AsCameraType(Gfx::CameraType def)
{
    if(m_empty)
        return def;
    return AsCameraType();
}


void CLevelParserParam::ParseArray()
{
    if(m_array.size() != 0)
        return;
    
    std::vector<std::string> values;
    boost::split(values, m_value, boost::is_any_of(";"));
    int i = 0;
    for(auto& value : values) {
        boost::algorithm::trim(value);
        if(value.empty()) continue;
        CLevelParserParam* param = new CLevelParserParam(m_name+"["+boost::lexical_cast<std::string>(i)+"]", value);
        param->SetLine(m_line);
        m_array.push_back(param);
        i++;
    }
}

const std::vector<CLevelParserParam*>& CLevelParserParam::AsArray()
{
    if(m_empty)
        throw CLevelParserExceptionMissingParam(this);
    
    ParseArray();
    
    return m_array;
}