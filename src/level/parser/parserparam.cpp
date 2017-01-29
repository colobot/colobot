/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "level/parser/parserparam.h"

#include "app/app.h"

#include "common/logger.h"
#include "common/make_unique.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "level/robotmain.h"

#include "level/parser/parser.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

CLevelParserParam::CLevelParserParam(std::string name, std::string value)
  : m_name(name)
  , m_value(value)
{}

CLevelParserParam::CLevelParserParam(std::string name, bool empty)
  : m_empty(empty)
  , m_name(name)
{
    assert(empty == true); // we need a second argument because we don't want to create param with value "name"
}

CLevelParserParam::CLevelParserParam(int value)
  : m_value(boost::lexical_cast<std::string>(value))
{}

CLevelParserParam::CLevelParserParam(float value)
  : m_value(boost::lexical_cast<std::string>(value))
{}

CLevelParserParam::CLevelParserParam(std::string value)
  : m_value("\""+value+"\"")
{}

CLevelParserParam::CLevelParserParam(bool value)
  : m_value(value ? "1" : "0")
{}

CLevelParserParam::CLevelParserParam(Gfx::Color value)
{
    m_array.push_back(MakeUnique<CLevelParserParam>(value.r));
    m_array.push_back(MakeUnique<CLevelParserParam>(value.g));
    m_array.push_back(MakeUnique<CLevelParserParam>(value.b));
    m_array.push_back(MakeUnique<CLevelParserParam>(value.a));

    LoadArray();
}

CLevelParserParam::CLevelParserParam(Math::Point value)
{
    m_array.push_back(MakeUnique<CLevelParserParam>(value.x));
    m_array.push_back(MakeUnique<CLevelParserParam>(value.y));

    LoadArray();
}

CLevelParserParam::CLevelParserParam(Math::Vector value)
{
    m_array.push_back(MakeUnique<CLevelParserParam>(value.x));
    if(value.y != 0.0f)
        m_array.push_back(MakeUnique<CLevelParserParam>(value.y));
    m_array.push_back(MakeUnique<CLevelParserParam>(value.z));

    LoadArray();
}

CLevelParserParam::CLevelParserParam(ObjectType value)
  : m_value(FromObjectType(value))
{}

CLevelParserParam::CLevelParserParam(Gfx::CameraType value)
  : m_value(FromCameraType(value))
{}

CLevelParserParam::CLevelParserParam(CLevelParserParamVec&& array)
{
    m_array.swap(array);

    LoadArray();
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
    try
    {
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
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return Cast<int>("int");
}


int CLevelParserParam::AsInt(int def)
{
    if (m_empty)
        return def;
    return AsInt();
}


float CLevelParserParam::AsFloat()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return Cast<float>("float");
}

float CLevelParserParam::AsFloat(float def)
{
    if (m_empty)
        return def;
    return AsFloat();
}


std::string CLevelParserParam::AsString()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    if ((m_value[0] == '\"' && m_value[m_value.length()-1] == '\"') || (m_value[0] == '\'' && m_value[m_value.length()-1] == '\''))
    {
        return m_value.substr(1, m_value.length()-2);
    }
    else
    {
        throw CLevelParserExceptionBadParam(this, "string");
    }
}

std::string CLevelParserParam::AsString(std::string def)
{
    if (m_empty)
        return def;
    return AsString();
}


bool CLevelParserParam::AsBool()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    std::string value = m_value;
    boost::to_lower(value);
    if (value == "true") return true;
    if (value == "false") return false;
    return Cast<bool>("bool");
}

bool CLevelParserParam::AsBool(bool def)
{
    if (m_empty)
        return def;
    return AsBool();
}


std::string CLevelParserParam::ToPath(std::string path, const std::string defaultDir)
{
    if (defaultDir == "" && path.find("%lvl%") != std::string::npos)
        throw CLevelParserException("TODO: Param "+m_name+" does not yet support %lvl%! :(");

    return GetLine()->GetLevel()->InjectLevelPaths(path, defaultDir);
}

std::string CLevelParserParam::AsPath(const std::string defaultDir)
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);

    return ToPath(AsString(), defaultDir);
}

std::string CLevelParserParam::AsPath(const std::string defaultDir, std::string def)
{
    if (m_empty)
        return GetLine()->GetLevel()->InjectLevelPaths(def, defaultDir);

    return ToPath(AsString(def), defaultDir);
}


Gfx::Color CLevelParserParam::AsColor()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);

    float red, green, blue, alpha;
    if (m_value.length() >= 1 && m_value[0] == '#')
    {
        if (m_value.length() != 7 && m_value.length() != 9)
            throw CLevelParserExceptionBadParam(this, "color");

        try
        {
            red = StrUtils::HexStringToInt(m_value.substr(1, 2));
            green = StrUtils::HexStringToInt(m_value.substr(3, 2));
            blue = StrUtils::HexStringToInt(m_value.substr(5, 2));
            alpha = (m_value.length() == 9) ? StrUtils::HexStringToInt(m_value.substr(7, 2)) : 1.0f;
        }
        catch (...)
        {
            // TODO: Show original exception text
            throw CLevelParserExceptionBadParam(this, "color");
        }
    }
    else
    {
        ParseArray();

        if (m_array.size() != 3 && m_array.size() != 4)
            throw CLevelParserExceptionBadParam(this, "color");

        red = m_array[0]->AsFloat();
        green = m_array[1]->AsFloat();
        blue = m_array[2]->AsFloat();
        alpha = (m_array.size() == 4) ? m_array[3]->AsFloat() : 1.0f;
    }

    if (red > 1.0f || green > 1.0f || blue > 1.0f || alpha > 1.0f)
    {
        red = red / 255.0f;
        green = green / 255.0f;
        blue = blue / 255.0f;
        alpha = alpha / 255.0f;
    }

    return Gfx::Color(red, green, blue, alpha);
}

Gfx::Color CLevelParserParam::AsColor(Gfx::Color def)
{
    if (m_empty)
        return def;
    return AsColor();
}


Math::Vector CLevelParserParam::AsPoint()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);

    ParseArray();

    if (m_array.size() == 2) //XZ
    {
        return Math::Vector(m_array[0]->AsFloat(), 0.0f, m_array[1]->AsFloat());
    }
    else if (m_array.size() == 3) //XYZ
    {
        return Math::Vector(m_array[0]->AsFloat(), m_array[1]->AsFloat(), m_array[2]->AsFloat());
    }
    else
    {
        throw CLevelParserExceptionBadParam(this, "point");
    }
}

Math::Vector CLevelParserParam::AsPoint(Math::Vector def)
{
    if (m_empty)
        return def;
    return AsPoint();
}


ObjectType CLevelParserParam::ToObjectType(std::string value)
{
    if (value == "All"               ) return OBJECT_NULL; // For use in NewScript
    if (value == "Any"               ) return OBJECT_NULL; // For use in type= in ending conditions
    if (value == "Portico"           ) return OBJECT_PORTICO;
    if (value == "SpaceShip"         ) return OBJECT_BASE;
    if (value == "PracticeBot"       ) return OBJECT_MOBILEwt;
    if (value == "WingedGrabber"     ) return OBJECT_MOBILEfa;
    if (value == "TrackedGrabber"    ) return OBJECT_MOBILEta;
    if (value == "WheeledGrabber"    ) return OBJECT_MOBILEwa;
    if (value == "LeggedGrabber"     ) return OBJECT_MOBILEia;
    if (value == "WingedShooter"     ) return OBJECT_MOBILEfc;
    if (value == "TrackedShooter"    ) return OBJECT_MOBILEtc;
    if (value == "WheeledShooter"    ) return OBJECT_MOBILEwc;
    if (value == "LeggedShooter"     ) return OBJECT_MOBILEic;
    if (value == "WingedOrgaShooter" ) return OBJECT_MOBILEfi;
    if (value == "TrackedOrgaShooter") return OBJECT_MOBILEti;
    if (value == "WheeledOrgaShooter") return OBJECT_MOBILEwi;
    if (value == "LeggedOrgaShooter" ) return OBJECT_MOBILEii;
    if (value == "WingedSniffer"     ) return OBJECT_MOBILEfs;
    if (value == "TrackedSniffer"    ) return OBJECT_MOBILEts;
    if (value == "WheeledSniffer"    ) return OBJECT_MOBILEws;
    if (value == "LeggedSniffer"     ) return OBJECT_MOBILEis;
    if (value == "Thumper"           ) return OBJECT_MOBILErt;
    if (value == "PhazerShooter"     ) return OBJECT_MOBILErc;
    if (value == "Recycler"          ) return OBJECT_MOBILErr;
    if (value == "Shielder"          ) return OBJECT_MOBILErs;
    if (value == "Subber"            ) return OBJECT_MOBILEsa;
    if (value == "TargetBot"         ) return OBJECT_MOBILEtg;
    if (value == "Scribbler"         ) return OBJECT_MOBILEdr;
    if (value == "PowerSpot"         ) return OBJECT_MARKPOWER;
    if (value == "TitaniumSpot"      ) return OBJECT_MARKSTONE;
    if (value == "UraniumSpot"       ) return OBJECT_MARKURANIUM;
    if (value == "PlatinumSpot"      ) return OBJECT_MARKURANIUM;
    if (value == "KeyASpot"          ) return OBJECT_MARKKEYa;
    if (value == "KeyBSpot"          ) return OBJECT_MARKKEYb;
    if (value == "KeyCSpot"          ) return OBJECT_MARKKEYc;
    if (value == "KeyDSpot"          ) return OBJECT_MARKKEYd;
    if (value == "WayPoint"          ) return OBJECT_WAYPOINT;
    if (value == "BlueFlag"          ) return OBJECT_FLAGb;
    if (value == "RedFlag"           ) return OBJECT_FLAGr;
    if (value == "GreenFlag"         ) return OBJECT_FLAGg;
    if (value == "YellowFlag"        ) return OBJECT_FLAGy;
    if (value == "VioletFlag"        ) return OBJECT_FLAGv;
    if (value == "PowerCell"         ) return OBJECT_POWER;
    if (value == "FuelCellPlant"     ) return OBJECT_NUCLEAR;
    if (value == "FuelCell"          ) return OBJECT_ATOMIC;
    if (value == "NuclearCell"       ) return OBJECT_ATOMIC;
    if (value == "TitaniumOre"       ) return OBJECT_STONE;
    if (value == "UraniumOre"        ) return OBJECT_URANIUM;
    if (value == "PlatinumOre"       ) return OBJECT_URANIUM;
    if (value == "Titanium"          ) return OBJECT_METAL;
    if (value == "OrgaMatter"        ) return OBJECT_BULLET;
    if (value == "BlackBox"          ) return OBJECT_BBOX;
    if (value == "KeyA"              ) return OBJECT_KEYa;
    if (value == "KeyB"              ) return OBJECT_KEYb;
    if (value == "KeyC"              ) return OBJECT_KEYc;
    if (value == "KeyD"              ) return OBJECT_KEYd;
    if (value == "TNT"               ) return OBJECT_TNT;
    if (value == "Mine"              ) return OBJECT_BOMB;
    if (value == "Firework"          ) return OBJECT_WINFIRE;
    if (value == "Bag"               ) return OBJECT_BAG;
    if (value == "Greenery0"         ) return OBJECT_PLANT0;
    if (value == "Greenery1"         ) return OBJECT_PLANT1;
    if (value == "Greenery2"         ) return OBJECT_PLANT2;
    if (value == "Greenery3"         ) return OBJECT_PLANT3;
    if (value == "Greenery4"         ) return OBJECT_PLANT4;
    if (value == "Greenery5"         ) return OBJECT_PLANT5;
    if (value == "Greenery6"         ) return OBJECT_PLANT6;
    if (value == "Greenery7"         ) return OBJECT_PLANT7;
    if (value == "Greenery8"         ) return OBJECT_PLANT8;
    if (value == "Greenery9"         ) return OBJECT_PLANT9;
    if (value == "Greenery10"        ) return OBJECT_PLANT10;
    if (value == "Greenery11"        ) return OBJECT_PLANT11;
    if (value == "Greenery12"        ) return OBJECT_PLANT12;
    if (value == "Greenery13"        ) return OBJECT_PLANT13;
    if (value == "Greenery14"        ) return OBJECT_PLANT14;
    if (value == "Greenery15"        ) return OBJECT_PLANT15;
    if (value == "Greenery16"        ) return OBJECT_PLANT16;
    if (value == "Greenery17"        ) return OBJECT_PLANT17;
    if (value == "Greenery18"        ) return OBJECT_PLANT18;
    if (value == "Greenery19"        ) return OBJECT_PLANT19;
    if (value == "Tree0"             ) return OBJECT_TREE0;
    if (value == "Tree1"             ) return OBJECT_TREE1;
    if (value == "Tree2"             ) return OBJECT_TREE2;
    if (value == "Tree3"             ) return OBJECT_TREE3;
    if (value == "Tree4"             ) return OBJECT_TREE4;
    if (value == "Tree5"             ) return OBJECT_TREE5;
    if (value == "Mushroom1"         ) return OBJECT_MUSHROOM1;
    if (value == "Mushroom2"         ) return OBJECT_MUSHROOM2;
    if (value == "Home"              ) return OBJECT_HOME1;
    if (value == "Derrick"           ) return OBJECT_DERRICK;
    if (value == "BotFactory"        ) return OBJECT_FACTORY;
    if (value == "PowerStation"      ) return OBJECT_STATION;
    if (value == "Converter"         ) return OBJECT_CONVERT;
    if (value == "RepairCenter"      ) return OBJECT_REPAIR;
    if (value == "Destroyer"         ) return OBJECT_DESTROYER;
    if (value == "DefenseTower"      ) return OBJECT_TOWER;
    if (value == "AlienNest"         ) return OBJECT_NEST;
    if (value == "ResearchCenter"    ) return OBJECT_RESEARCH;
    if (value == "RadarStation"      ) return OBJECT_RADAR;
    if (value == "ExchangePost"      ) return OBJECT_INFO;
    if (value == "PowerPlant"        ) return OBJECT_ENERGY;
    if (value == "AutoLab"           ) return OBJECT_LABO;
    if (value == "NuclearPlant"      ) return OBJECT_NUCLEAR;
    if (value == "PowerCaptor"       ) return OBJECT_PARA;
    if (value == "Vault"             ) return OBJECT_SAFE;
    if (value == "Houston"           ) return OBJECT_HUSTON;
    if (value == "Target1"           ) return OBJECT_TARGET1;
    if (value == "Target2"           ) return OBJECT_TARGET2;
    if (value == "StartArea"         ) return OBJECT_START;
    if (value == "GoalArea"          ) return OBJECT_END;
    if (value == "AlienQueen"        ) return OBJECT_MOTHER;
    if (value == "AlienEgg"          ) return OBJECT_EGG;
    if (value == "AlienAnt"          ) return OBJECT_ANT;
    if (value == "AlienSpider"       ) return OBJECT_SPIDER;
    if (value == "AlienWasp"         ) return OBJECT_BEE;
    if (value == "AlienWorm"         ) return OBJECT_WORM;
    if (value == "WreckBotw1"        ) return OBJECT_RUINmobilew1;
    if (value == "WreckBotw2"        ) return OBJECT_RUINmobilew2;
    if (value == "WreckBott1"        ) return OBJECT_RUINmobilet1;
    if (value == "WreckBott2"        ) return OBJECT_RUINmobilet2;
    if (value == "WreckBotr1"        ) return OBJECT_RUINmobiler1;
    if (value == "WreckBotr2"        ) return OBJECT_RUINmobiler2;
    if (value == "RuinBotFactory"    ) return OBJECT_RUINfactory;
    if (value == "RuinDoor"          ) return OBJECT_RUINdoor;
    if (value == "RuinSupport"       ) return OBJECT_RUINsupport;
    if (value == "RuinRadar"         ) return OBJECT_RUINradar;
    if (value == "RuinConvert"       ) return OBJECT_RUINconvert;
    if (value == "RuinBaseCamp"      ) return OBJECT_RUINbase;
    if (value == "RuinHeadCamp"      ) return OBJECT_RUINhead;
    if (value == "Barrier0"          ) return OBJECT_BARRIER0;
    if (value == "Barrier1"          ) return OBJECT_BARRIER1;
    if (value == "Barrier2"          ) return OBJECT_BARRIER2;
    if (value == "Barrier3"          ) return OBJECT_BARRIER3;
    if (value == "Teen0"             ) return OBJECT_TEEN0;
    if (value == "Teen1"             ) return OBJECT_TEEN1;
    if (value == "Teen2"             ) return OBJECT_TEEN2;
    if (value == "Teen3"             ) return OBJECT_TEEN3;
    if (value == "Teen4"             ) return OBJECT_TEEN4;
    if (value == "Teen5"             ) return OBJECT_TEEN5;
    if (value == "Teen6"             ) return OBJECT_TEEN6;
    if (value == "Teen7"             ) return OBJECT_TEEN7;
    if (value == "Teen8"             ) return OBJECT_TEEN8;
    if (value == "Teen9"             ) return OBJECT_TEEN9;
    if (value == "Teen10"            ) return OBJECT_TEEN10;
    if (value == "Teen11"            ) return OBJECT_TEEN11;
    if (value == "Teen12"            ) return OBJECT_TEEN12;
    if (value == "Teen13"            ) return OBJECT_TEEN13;
    if (value == "Teen14"            ) return OBJECT_TEEN14;
    if (value == "Teen15"            ) return OBJECT_TEEN15;
    if (value == "Teen16"            ) return OBJECT_TEEN16;
    if (value == "Teen17"            ) return OBJECT_TEEN17;
    if (value == "Teen18"            ) return OBJECT_TEEN18;
    if (value == "Teen19"            ) return OBJECT_TEEN19;
    if (value == "Teen20"            ) return OBJECT_TEEN20;
    if (value == "Teen21"            ) return OBJECT_TEEN21;
    if (value == "Teen22"            ) return OBJECT_TEEN22;
    if (value == "Teen23"            ) return OBJECT_TEEN23;
    if (value == "Teen24"            ) return OBJECT_TEEN24;
    if (value == "Teen25"            ) return OBJECT_TEEN25;
    if (value == "Teen26"            ) return OBJECT_TEEN26;
    if (value == "Teen27"            ) return OBJECT_TEEN27;
    if (value == "Teen28"            ) return OBJECT_TEEN28;
    if (value == "Teen29"            ) return OBJECT_TEEN29;
    if (value == "Teen30"            ) return OBJECT_TEEN30;
    if (value == "Teen31"            ) return OBJECT_TEEN31;
    if (value == "Teen32"            ) return OBJECT_TEEN32;
    if (value == "Teen33"            ) return OBJECT_TEEN33;
    if (value == "Stone"             ) return OBJECT_TEEN34;
    if (value == "Teen35"            ) return OBJECT_TEEN35;
    if (value == "Teen36"            ) return OBJECT_TEEN36;
    if (value == "Teen37"            ) return OBJECT_TEEN37;
    if (value == "Teen38"            ) return OBJECT_TEEN38;
    if (value == "Teen39"            ) return OBJECT_TEEN39;
    if (value == "Teen40"            ) return OBJECT_TEEN40;
    if (value == "Teen41"            ) return OBJECT_TEEN41;
    if (value == "Teen42"            ) return OBJECT_TEEN42;
    if (value == "Teen43"            ) return OBJECT_TEEN43;
    if (value == "Teen44"            ) return OBJECT_TEEN44;
    if (value == "Quartz0"           ) return OBJECT_QUARTZ0;
    if (value == "Quartz1"           ) return OBJECT_QUARTZ1;
    if (value == "Quartz2"           ) return OBJECT_QUARTZ2;
    if (value == "Quartz3"           ) return OBJECT_QUARTZ3;
    if (value == "MegaStalk0"        ) return OBJECT_ROOT0;
    if (value == "MegaStalk1"        ) return OBJECT_ROOT1;
    if (value == "MegaStalk2"        ) return OBJECT_ROOT2;
    if (value == "MegaStalk3"        ) return OBJECT_ROOT3;
    if (value == "MegaStalk4"        ) return OBJECT_ROOT4;
    if (value == "MegaStalk5"        ) return OBJECT_ROOT5;
    if (value == "ApolloLEM"         ) return OBJECT_APOLLO1;
    if (value == "ApolloJeep"        ) return OBJECT_APOLLO2;
    if (value == "ApolloFlag"        ) return OBJECT_APOLLO3;
    if (value == "ApolloModule"      ) return OBJECT_APOLLO4;
    if (value == "ApolloAntenna"     ) return OBJECT_APOLLO5;
    if (value == "Me"                ) return OBJECT_HUMAN;
    if (value == "Tech"              ) return OBJECT_TECH;
    if (value == "MissionController" ) return OBJECT_CONTROLLER;

    // BuzzingCars specific
    // TODO: Remove duplicates from the list
    // I commented some out, but not all ~krzys_h
//    if (value == "All"               ) return OBJECT_NULL;
    if (value == "TargetBot"         ) return OBJECT_MOBILEtg;
    if (value == "FireballBot"       ) return OBJECT_MOBILEfb;
    if (value == "OrgaballBot"       ) return OBJECT_MOBILEob;
    if (value == "TraxFix"           ) return OBJECT_TRAXf;
    if (value == "Trax"              ) return OBJECT_TRAX;
    if (value == "UFO"               ) return OBJECT_UFO;
    if (value == "Carrot"            ) return OBJECT_CARROT;
    if (value == "Starter"           ) return OBJECT_STARTER;
    if (value == "Walker"            ) return OBJECT_WALKER;
    if (value == "Crazy"             ) return OBJECT_CRAZY;
    if (value == "Guide"             ) return OBJECT_GUIDE;
    if (value == "Evil1"             ) return OBJECT_EVIL1;
    if (value == "Evil2"             ) return OBJECT_EVIL2;
    if (value == "Evil3"             ) return OBJECT_EVIL3;
    if (value == "Evil4"             ) return OBJECT_EVIL4;
    if (value == "Evil5"             ) return OBJECT_EVIL5;
//    if (value == "WayPoint"          ) return OBJECT_WAYPOINT;
    if (value == "Traject"           ) return OBJECT_TRAJECT;
    if (value == "Target"            ) return OBJECT_TARGET;
    if (value == "BarrelAtomic"      ) return OBJECT_BARRELa;
    if (value == "Barrel"            ) return OBJECT_BARREL;
//    if (value == "NuclearCell"       ) return OBJECT_ATOMIC;
//    if (value == "TitaniumOre"       ) return OBJECT_STONE;
//    if (value == "UraniumOre"        ) return OBJECT_URANIUM;
//    if (value == "Titanium"          ) return OBJECT_METAL;
//    if (value == "OrgaMatter"        ) return OBJECT_BULLET;
//    if (value == "BlackBox"          ) return OBJECT_BBOX;
//    if (value == "KeyA"              ) return OBJECT_KEYa;
//    if (value == "KeyB"              ) return OBJECT_KEYb;
//    if (value == "KeyC"              ) return OBJECT_KEYc;
//    if (value == "KeyD"              ) return OBJECT_KEYd;
    if (value == "Bot1"              ) return OBJECT_BOT1;
    if (value == "Bot2"              ) return OBJECT_BOT2;
    if (value == "Bot3"              ) return OBJECT_BOT3;
    if (value == "Bot4"              ) return OBJECT_BOT4;
    if (value == "Bot5"              ) return OBJECT_BOT5;
//    if (value == "TNT"               ) return OBJECT_TNT;
//    if (value == "Mine"              ) return OBJECT_MINE;
    if (value == "Pole"              ) return OBJECT_POLE;
    if (value == "Bomb"              ) return OBJECT_BOMB;
    if (value == "Cone"              ) return OBJECT_CONE;
    if (value == "Pipes"             ) return OBJECT_PIPES;
//    if (value == "Firework"          ) return OBJECT_WINFIRE;
    if (value == "Fire"              ) return OBJECT_FIRE;
    if (value == "Helico"            ) return OBJECT_HELICO;
    if (value == "Compass"           ) return OBJECT_COMPASS;
    if (value == "Blitzer"           ) return OBJECT_BLITZER;
    if (value == "Hook"              ) return OBJECT_HOOK;
    if (value == "Aquarium"          ) return OBJECT_AQUA;
//    if (value == "Bag"               ) return OBJECT_BAG;
    if (value == "Cross1"            ) return OBJECT_CROSS1;
    if (value == "Mark"              ) return OBJECT_MARK;
    if (value == "Crown"             ) return OBJECT_CROWN;
    if (value == "Toys1"             ) return OBJECT_TOYS1;
    if (value == "Toys2"             ) return OBJECT_TOYS2;
    if (value == "Toys3"             ) return OBJECT_TOYS3;
    if (value == "Toys4"             ) return OBJECT_TOYS4;
    if (value == "Toys5"             ) return OBJECT_TOYS5;
    if (value == "Greenery10"        ) return OBJECT_PLANT10;
    if (value == "Greenery11"        ) return OBJECT_PLANT11;
    if (value == "Greenery12"        ) return OBJECT_PLANT12;
    if (value == "Greenery13"        ) return OBJECT_PLANT13;
    if (value == "Greenery14"        ) return OBJECT_PLANT14;
    if (value == "Greenery15"        ) return OBJECT_PLANT15;
    if (value == "Greenery16"        ) return OBJECT_PLANT16;
    if (value == "Greenery17"        ) return OBJECT_PLANT17;
    if (value == "Greenery18"        ) return OBJECT_PLANT18;
    if (value == "Greenery19"        ) return OBJECT_PLANT19;
    if (value == "Greenery0"         ) return OBJECT_PLANT0;
    if (value == "Greenery1"         ) return OBJECT_PLANT1;
    if (value == "Greenery2"         ) return OBJECT_PLANT2;
    if (value == "Greenery3"         ) return OBJECT_PLANT3;
    if (value == "Greenery4"         ) return OBJECT_PLANT4;
    if (value == "Greenery5"         ) return OBJECT_PLANT5;
    if (value == "Greenery6"         ) return OBJECT_PLANT6;
    if (value == "Greenery7"         ) return OBJECT_PLANT7;
    if (value == "Greenery8"         ) return OBJECT_PLANT8;
    if (value == "Greenery9"         ) return OBJECT_PLANT9;
    if (value == "Tree0"             ) return OBJECT_TREE0;
    if (value == "Tree1"             ) return OBJECT_TREE1;
    if (value == "Tree2"             ) return OBJECT_TREE2;
    if (value == "Tree3"             ) return OBJECT_TREE3;
    if (value == "Tree4"             ) return OBJECT_TREE4;
    if (value == "Tree5"             ) return OBJECT_TREE5;
    if (value == "Tree6"             ) return OBJECT_TREE6;
    if (value == "Tree7"             ) return OBJECT_TREE7;
    if (value == "Tree8"             ) return OBJECT_TREE8;
    if (value == "Tree9"             ) return OBJECT_TREE9;
    if (value == "Mushroom0"         ) return OBJECT_MUSHROOM0;
    if (value == "Mushroom1"         ) return OBJECT_MUSHROOM1;
    if (value == "Mushroom2"         ) return OBJECT_MUSHROOM2;
    if (value == "Mushroom3"         ) return OBJECT_MUSHROOM3;
    if (value == "Mushroom4"         ) return OBJECT_MUSHROOM4;
    if (value == "Mushroom5"         ) return OBJECT_MUSHROOM5;
    if (value == "Mushroom6"         ) return OBJECT_MUSHROOM6;
    if (value == "Mushroom7"         ) return OBJECT_MUSHROOM7;
    if (value == "Mushroom8"         ) return OBJECT_MUSHROOM8;
    if (value == "Mushroom9"         ) return OBJECT_MUSHROOM9;
    if (value == "Home1"             ) return OBJECT_HOME1;
    if (value == "Home2"             ) return OBJECT_HOME2;
    if (value == "Home3"             ) return OBJECT_HOME3;
    if (value == "Home4"             ) return OBJECT_HOME4;
    if (value == "Home5"             ) return OBJECT_HOME5;
    if (value == "Factory10"         ) return OBJECT_FACTORY10;
    if (value == "Factory11"         ) return OBJECT_FACTORY11;
    if (value == "Factory12"         ) return OBJECT_FACTORY12;
    if (value == "Factory13"         ) return OBJECT_FACTORY13;
    if (value == "Factory14"         ) return OBJECT_FACTORY14;
    if (value == "Factory15"         ) return OBJECT_FACTORY15;
    if (value == "Factory16"         ) return OBJECT_FACTORY16;
    if (value == "Factory17"         ) return OBJECT_FACTORY17;
    if (value == "Factory18"         ) return OBJECT_FACTORY18;
    if (value == "Factory19"         ) return OBJECT_FACTORY19;
    if (value == "Factory20"         ) return OBJECT_FACTORY20;
    if (value == "Factory1"          ) return OBJECT_FACTORY1;
    if (value == "Factory2"          ) return OBJECT_FACTORY2;
    if (value == "Factory3"          ) return OBJECT_FACTORY3;
    if (value == "Factory4"          ) return OBJECT_FACTORY4;
    if (value == "Factory5"          ) return OBJECT_FACTORY5;
    if (value == "Factory6"          ) return OBJECT_FACTORY6;
    if (value == "Factory7"          ) return OBJECT_FACTORY7;
    if (value == "Factory8"          ) return OBJECT_FACTORY8;
    if (value == "Factory9"          ) return OBJECT_FACTORY9;
    if (value == "Gravel"            ) return OBJECT_GRAVEL;
    if (value == "Tub"               ) return OBJECT_TUB;
    if (value == "Alien10"           ) return OBJECT_ALIEN10;
    if (value == "Alien1"            ) return OBJECT_ALIEN1;
    if (value == "Alien2"            ) return OBJECT_ALIEN2;
    if (value == "Alien3"            ) return OBJECT_ALIEN3;
    if (value == "Alien4"            ) return OBJECT_ALIEN4;
    if (value == "Alien5"            ) return OBJECT_ALIEN5;
    if (value == "Alien6"            ) return OBJECT_ALIEN6;
    if (value == "Alien7"            ) return OBJECT_ALIEN7;
    if (value == "Alien8"            ) return OBJECT_ALIEN8;
    if (value == "Alien9"            ) return OBJECT_ALIEN9;
    if (value == "Inca10"            ) return OBJECT_INCA10;
    if (value == "Inca1"             ) return OBJECT_INCA1;
    if (value == "Inca2"             ) return OBJECT_INCA2;
    if (value == "Inca3"             ) return OBJECT_INCA3;
    if (value == "Inca4"             ) return OBJECT_INCA4;
    if (value == "Inca5"             ) return OBJECT_INCA5;
    if (value == "Inca6"             ) return OBJECT_INCA6;
    if (value == "Inca7"             ) return OBJECT_INCA7;
    if (value == "Inca8"             ) return OBJECT_INCA8;
    if (value == "Inca9"             ) return OBJECT_INCA9;
    if (value == "Building10"        ) return OBJECT_BUILDING10;
    if (value == "Building1"         ) return OBJECT_BUILDING1;
    if (value == "Building2"         ) return OBJECT_BUILDING2;
    if (value == "Building3"         ) return OBJECT_BUILDING3;
    if (value == "Building4"         ) return OBJECT_BUILDING4;
    if (value == "Building5"         ) return OBJECT_BUILDING5;
    if (value == "Building6"         ) return OBJECT_BUILDING6;
    if (value == "Building7"         ) return OBJECT_BUILDING7;
    if (value == "Building8"         ) return OBJECT_BUILDING8;
    if (value == "Building9"         ) return OBJECT_BUILDING9;
    if (value == "Carcass10"         ) return OBJECT_CARCASS10;
    if (value == "Carcass1"          ) return OBJECT_CARCASS1;
    if (value == "Carcass2"          ) return OBJECT_CARCASS2;
    if (value == "Carcass3"          ) return OBJECT_CARCASS3;
    if (value == "Carcass4"          ) return OBJECT_CARCASS4;
    if (value == "Carcass5"          ) return OBJECT_CARCASS5;
    if (value == "Carcass6"          ) return OBJECT_CARCASS6;
    if (value == "Carcass7"          ) return OBJECT_CARCASS7;
    if (value == "Carcass8"          ) return OBJECT_CARCASS8;
    if (value == "Carcass9"          ) return OBJECT_CARCASS9;
    if (value == "Orga10"            ) return OBJECT_ORGA10;
    if (value == "Orga1"             ) return OBJECT_ORGA1;
    if (value == "Orga2"             ) return OBJECT_ORGA2;
    if (value == "Orga3"             ) return OBJECT_ORGA3;
    if (value == "Orga4"             ) return OBJECT_ORGA4;
    if (value == "Orga5"             ) return OBJECT_ORGA5;
    if (value == "Orga6"             ) return OBJECT_ORGA6;
    if (value == "Orga7"             ) return OBJECT_ORGA7;
    if (value == "Orga8"             ) return OBJECT_ORGA8;
    if (value == "Orga9"             ) return OBJECT_ORGA9;
    if (value == "Car"               ) return OBJECT_CAR;
    if (value == "DefenseTower"      ) return OBJECT_TOWER;
    if (value == "NuclearPlant"      ) return OBJECT_NUCLEAR;
    if (value == "PowerCaptor"       ) return OBJECT_PARA;
    if (value == "Computer"          ) return OBJECT_COMPUTER;
    if (value == "RepairCenter"      ) return OBJECT_REPAIR;
    if (value == "Sweet"             ) return OBJECT_SWEET;
    if (value == "Door1"             ) return OBJECT_DOOR1;
    if (value == "Door2"             ) return OBJECT_DOOR2;
    if (value == "Door3"             ) return OBJECT_DOOR3;
    if (value == "Door4"             ) return OBJECT_DOOR4;
    if (value == "Door5"             ) return OBJECT_DOOR5;
    if (value == "Dock"              ) return OBJECT_DOCK;
    if (value == "Remote"            ) return OBJECT_REMOTE;
    if (value == "Stand"             ) return OBJECT_STAND;
    if (value == "Generator"         ) return OBJECT_GENERATOR;
    if (value == "StartArea"         ) return OBJECT_START;
    if (value == "GoalArea"          ) return OBJECT_END;
    if (value == "Support"           ) return OBJECT_SUPPORT;
    if (value == "RoadSign10"        ) return OBJECT_ROADSIGN10;
    if (value == "RoadSign11"        ) return OBJECT_ROADSIGN11;
    if (value == "RoadSign12"        ) return OBJECT_ROADSIGN12;
    if (value == "RoadSign13"        ) return OBJECT_ROADSIGN13;
    if (value == "RoadSign14"        ) return OBJECT_ROADSIGN14;
    if (value == "RoadSign15"        ) return OBJECT_ROADSIGN15;
    if (value == "RoadSign16"        ) return OBJECT_ROADSIGN16;
    if (value == "RoadSign17"        ) return OBJECT_ROADSIGN17;
    if (value == "RoadSign18"        ) return OBJECT_ROADSIGN18;
    if (value == "RoadSign19"        ) return OBJECT_ROADSIGN19;
    if (value == "RoadSign20"        ) return OBJECT_ROADSIGN20;
    if (value == "RoadSign21"        ) return OBJECT_ROADSIGN21;
    if (value == "RoadSign22"        ) return OBJECT_ROADSIGN22;
    if (value == "RoadSign23"        ) return OBJECT_ROADSIGN23;
    if (value == "RoadSign24"        ) return OBJECT_ROADSIGN24;
    if (value == "RoadSign25"        ) return OBJECT_ROADSIGN25;
    if (value == "RoadSign26"        ) return OBJECT_ROADSIGN26;
    if (value == "RoadSign27"        ) return OBJECT_ROADSIGN27;
    if (value == "RoadSign28"        ) return OBJECT_ROADSIGN28;
    if (value == "RoadSign29"        ) return OBJECT_ROADSIGN29;
    if (value == "RoadSign30"        ) return OBJECT_ROADSIGN30;
    if (value == "RoadSign1"         ) return OBJECT_ROADSIGN1;
    if (value == "RoadSign2"         ) return OBJECT_ROADSIGN2;
    if (value == "RoadSign3"         ) return OBJECT_ROADSIGN3;
    if (value == "RoadSign4"         ) return OBJECT_ROADSIGN4;
    if (value == "RoadSign5"         ) return OBJECT_ROADSIGN5;
    if (value == "RoadSign6"         ) return OBJECT_ROADSIGN6;
    if (value == "RoadSign7"         ) return OBJECT_ROADSIGN7;
    if (value == "RoadSign8"         ) return OBJECT_ROADSIGN8;
    if (value == "RoadSign9"         ) return OBJECT_ROADSIGN9;
    if (value == "Pub11"             ) return OBJECT_PUB11;
    if (value == "Pub12"             ) return OBJECT_PUB12;
    if (value == "Pub13"             ) return OBJECT_PUB13;
    if (value == "Pub14"             ) return OBJECT_PUB14;
    if (value == "Pub21"             ) return OBJECT_PUB21;
    if (value == "Pub22"             ) return OBJECT_PUB22;
    if (value == "Pub23"             ) return OBJECT_PUB23;
    if (value == "Pub24"             ) return OBJECT_PUB24;
    if (value == "Pub31"             ) return OBJECT_PUB31;
    if (value == "Pub32"             ) return OBJECT_PUB32;
    if (value == "Pub33"             ) return OBJECT_PUB33;
    if (value == "Pub34"             ) return OBJECT_PUB34;
    if (value == "Pub41"             ) return OBJECT_PUB41;
    if (value == "Pub42"             ) return OBJECT_PUB42;
    if (value == "Pub43"             ) return OBJECT_PUB43;
    if (value == "Pub44"             ) return OBJECT_PUB44;
    if (value == "Pub51"             ) return OBJECT_PUB51;
    if (value == "Pub52"             ) return OBJECT_PUB52;
    if (value == "Pub53"             ) return OBJECT_PUB53;
    if (value == "Pub54"             ) return OBJECT_PUB54;
    if (value == "Pub61"             ) return OBJECT_PUB61;
    if (value == "Pub62"             ) return OBJECT_PUB62;
    if (value == "Pub63"             ) return OBJECT_PUB63;
    if (value == "Pub64"             ) return OBJECT_PUB64;
    if (value == "Pub71"             ) return OBJECT_PUB71;
    if (value == "Pub72"             ) return OBJECT_PUB72;
    if (value == "Pub73"             ) return OBJECT_PUB73;
    if (value == "Pub74"             ) return OBJECT_PUB74;
    if (value == "Pub81"             ) return OBJECT_PUB81;
    if (value == "Pub82"             ) return OBJECT_PUB82;
    if (value == "Pub83"             ) return OBJECT_PUB83;
    if (value == "Pub84"             ) return OBJECT_PUB84;
    if (value == "Pub91"             ) return OBJECT_PUB91;
    if (value == "Pub92"             ) return OBJECT_PUB92;
    if (value == "Pub93"             ) return OBJECT_PUB93;
    if (value == "Pub94"             ) return OBJECT_PUB94;
//    if (value == "WreckBotw1"        ) return OBJECT_RUINmobilew1;
//    if (value == "WreckBotw2"        ) return OBJECT_RUINmobilew2;
//    if (value == "WreckBott1"        ) return OBJECT_RUINmobilet1;
//    if (value == "WreckBott2"        ) return OBJECT_RUINmobilet2;
//    if (value == "WreckBotr1"        ) return OBJECT_RUINmobiler1;
//    if (value == "WreckBotr2"        ) return OBJECT_RUINmobiler2;
//    if (value == "RuinBotFactory"    ) return OBJECT_RUINfactory;
//    if (value == "RuinDoor"          ) return OBJECT_RUINdoor;
//    if (value == "RuinSupport"       ) return OBJECT_RUINsupport;
//    if (value == "RuinRadar"         ) return OBJECT_RUINradar;
//    if (value == "RuinConvert"       ) return OBJECT_RUINconvert;
//    if (value == "RuinBaseCamp"      ) return OBJECT_RUINbase;
//    if (value == "RuinHeadCamp"      ) return OBJECT_RUINhead;
    if (value == "Barrier10"         ) return OBJECT_BARRIER10;
    if (value == "Barrier11"         ) return OBJECT_BARRIER11;
    if (value == "Barrier12"         ) return OBJECT_BARRIER12;
    if (value == "Barrier13"         ) return OBJECT_BARRIER13;
    if (value == "Barrier14"         ) return OBJECT_BARRIER14;
    if (value == "Barrier15"         ) return OBJECT_BARRIER15;
    if (value == "Barrier16"         ) return OBJECT_BARRIER16;
    if (value == "Barrier17"         ) return OBJECT_BARRIER17;
    if (value == "Barrier18"         ) return OBJECT_BARRIER18;
    if (value == "Barrier19"         ) return OBJECT_BARRIER19;
    if (value == "Barrier0"          ) return OBJECT_BARRIER0;
    if (value == "Barrier1"          ) return OBJECT_BARRIER1;
    if (value == "Barrier2"          ) return OBJECT_BARRIER2;
    if (value == "Barrier3"          ) return OBJECT_BARRIER3;
    if (value == "Barrier4"          ) return OBJECT_BARRIER4;
    if (value == "Barrier5"          ) return OBJECT_BARRIER5;
    if (value == "Barrier6"          ) return OBJECT_BARRIER6;
    if (value == "Barrier7"          ) return OBJECT_BARRIER7;
    if (value == "Barrier8"          ) return OBJECT_BARRIER8;
    if (value == "Barrier9"          ) return OBJECT_BARRIER9;
    if (value == "Box10"             ) return OBJECT_BOX10;
    if (value == "Box1"              ) return OBJECT_BOX1;
    if (value == "Box2"              ) return OBJECT_BOX2;
    if (value == "Box3"              ) return OBJECT_BOX3;
    if (value == "Box4"              ) return OBJECT_BOX4;
    if (value == "Box5"              ) return OBJECT_BOX5;
    if (value == "Box6"              ) return OBJECT_BOX6;
    if (value == "Box7"              ) return OBJECT_BOX7;
    if (value == "Box8"              ) return OBJECT_BOX8;
    if (value == "Box9"              ) return OBJECT_BOX9;
    if (value == "Stone10"           ) return OBJECT_STONE10;
    if (value == "Stone1"            ) return OBJECT_STONE1;
    if (value == "Stone2"            ) return OBJECT_STONE2;
    if (value == "Stone3"            ) return OBJECT_STONE3;
    if (value == "Stone4"            ) return OBJECT_STONE4;
    if (value == "Stone5"            ) return OBJECT_STONE5;
    if (value == "Stone6"            ) return OBJECT_STONE6;
    if (value == "Stone7"            ) return OBJECT_STONE7;
    if (value == "Stone8"            ) return OBJECT_STONE8;
    if (value == "Stone9"            ) return OBJECT_STONE9;
    if (value == "Piece0"            ) return OBJECT_PIECE0;
    if (value == "Piece1"            ) return OBJECT_PIECE1;
    if (value == "Piece2"            ) return OBJECT_PIECE2;
    if (value == "Piece3"            ) return OBJECT_PIECE3;
    if (value == "Piece4"            ) return OBJECT_PIECE4;
    if (value == "Piece5"            ) return OBJECT_PIECE5;
    if (value == "Piece6"            ) return OBJECT_PIECE6;
    if (value == "Piece7"            ) return OBJECT_PIECE7;
    if (value == "Piece8"            ) return OBJECT_PIECE8;
    if (value == "Piece9"            ) return OBJECT_PIECE9;
    if (value == "Quartz0"           ) return OBJECT_QUARTZ0;
    if (value == "Quartz1"           ) return OBJECT_QUARTZ1;
    if (value == "Quartz2"           ) return OBJECT_QUARTZ2;
    if (value == "Quartz3"           ) return OBJECT_QUARTZ3;
    if (value == "Quartz4"           ) return OBJECT_QUARTZ4;
    if (value == "Quartz5"           ) return OBJECT_QUARTZ5;
    if (value == "Quartz6"           ) return OBJECT_QUARTZ6;
    if (value == "Quartz7"           ) return OBJECT_QUARTZ7;
    if (value == "Quartz8"           ) return OBJECT_QUARTZ8;
    if (value == "Quartz9"           ) return OBJECT_QUARTZ9;
    if (value == "MegaStalk0"        ) return OBJECT_ROOT0;
    if (value == "MegaStalk1"        ) return OBJECT_ROOT1;
    if (value == "MegaStalk2"        ) return OBJECT_ROOT2;
    if (value == "MegaStalk3"        ) return OBJECT_ROOT3;
    if (value == "MegaStalk4"        ) return OBJECT_ROOT4;
    if (value == "MegaStalk5"        ) return OBJECT_ROOT5;
    if (value == "MegaStalk6"        ) return OBJECT_ROOT6;
    if (value == "MegaStalk7"        ) return OBJECT_ROOT7;
    if (value == "MegaStalk8"        ) return OBJECT_ROOT8;
    if (value == "MegaStalk9"        ) return OBJECT_ROOT9;
//    if (value == "Me"                ) return OBJECT_HUMAN;
//    if (value == "Tech"              ) return OBJECT_TECH;

    return static_cast<ObjectType>(Cast<int>(value, "object"));
}

const std::string CLevelParserParam::FromObjectType(ObjectType value)
{
    if (value == OBJECT_PORTICO     ) return "Portico";
    if (value == OBJECT_BASE        ) return "SpaceShip";
    if (value == OBJECT_MOBILEwt    ) return "PracticeBot";
    if (value == OBJECT_MOBILEfa    ) return "WingedGrabber";
    if (value == OBJECT_MOBILEta    ) return "TrackedGrabber";
    if (value == OBJECT_MOBILEwa    ) return "WheeledGrabber";
    if (value == OBJECT_MOBILEia    ) return "LeggedGrabber";
    if (value == OBJECT_MOBILEfc    ) return "WingedShooter";
    if (value == OBJECT_MOBILEtc    ) return "TrackedShooter";
    if (value == OBJECT_MOBILEwc    ) return "WheeledShooter";
    if (value == OBJECT_MOBILEic    ) return "LeggedShooter";
    if (value == OBJECT_MOBILEfi    ) return "WingedOrgaShooter";
    if (value == OBJECT_MOBILEti    ) return "TrackedOrgaShooter";
    if (value == OBJECT_MOBILEwi    ) return "WheeledOrgaShooter";
    if (value == OBJECT_MOBILEii    ) return "LeggedOrgaShooter";
    if (value == OBJECT_MOBILEfs    ) return "WingedSniffer";
    if (value == OBJECT_MOBILEts    ) return "TrackedSniffer";
    if (value == OBJECT_MOBILEws    ) return "WheeledSniffer";
    if (value == OBJECT_MOBILEis    ) return "LeggedSniffer";
    if (value == OBJECT_MOBILErt    ) return "Thumper";
    if (value == OBJECT_MOBILErc    ) return "PhazerShooter";
    if (value == OBJECT_MOBILErr    ) return "Recycler";
    if (value == OBJECT_MOBILErs    ) return "Shielder";
    if (value == OBJECT_MOBILEsa    ) return "Subber";
    if (value == OBJECT_MOBILEtg    ) return "TargetBot";
    if (value == OBJECT_MOBILEdr    ) return "Scribbler";
    if (value == OBJECT_MARKPOWER   ) return "PowerSpot";
    if (value == OBJECT_MARKSTONE   ) return "TitaniumSpot";
    if (value == OBJECT_MARKURANIUM ) return "UraniumSpot";
    if (value == OBJECT_MARKKEYa    ) return "KeyASpot";
    if (value == OBJECT_MARKKEYb    ) return "KeyBSpot";
    if (value == OBJECT_MARKKEYc    ) return "KeyCSpot";
    if (value == OBJECT_MARKKEYd    ) return "KeyDSpot";
    if (value == OBJECT_WAYPOINT    ) return "WayPoint";
    if (value == OBJECT_FLAGb       ) return "BlueFlag";
    if (value == OBJECT_FLAGr       ) return "RedFlag";
    if (value == OBJECT_FLAGg       ) return "GreenFlag";
    if (value == OBJECT_FLAGy       ) return "YellowFlag";
    if (value == OBJECT_FLAGv       ) return "VioletFlag";
    if (value == OBJECT_POWER       ) return "PowerCell";
    if (value == OBJECT_ATOMIC      ) return "NuclearCell";
    if (value == OBJECT_STONE       ) return "TitaniumOre";
    if (value == OBJECT_URANIUM     ) return "UraniumOre";
    if (value == OBJECT_METAL       ) return "Titanium";
    if (value == OBJECT_BULLET      ) return "OrgaMatter";
    if (value == OBJECT_BBOX        ) return "BlackBox";
    if (value == OBJECT_KEYa        ) return "KeyA";
    if (value == OBJECT_KEYb        ) return "KeyB";
    if (value == OBJECT_KEYc        ) return "KeyC";
    if (value == OBJECT_KEYd        ) return "KeyD";
    if (value == OBJECT_TNT         ) return "TNT";
    if (value == OBJECT_BOMB        ) return "Mine";
    if (value == OBJECT_WINFIRE     ) return "Firework";
    if (value == OBJECT_BAG         ) return "Bag";
    if (value == OBJECT_PLANT0      ) return "Greenery0";
    if (value == OBJECT_PLANT1      ) return "Greenery1";
    if (value == OBJECT_PLANT2      ) return "Greenery2";
    if (value == OBJECT_PLANT3      ) return "Greenery3";
    if (value == OBJECT_PLANT4      ) return "Greenery4";
    if (value == OBJECT_PLANT5      ) return "Greenery5";
    if (value == OBJECT_PLANT6      ) return "Greenery6";
    if (value == OBJECT_PLANT7      ) return "Greenery7";
    if (value == OBJECT_PLANT8      ) return "Greenery8";
    if (value == OBJECT_PLANT9      ) return "Greenery9";
    if (value == OBJECT_PLANT10     ) return "Greenery10";
    if (value == OBJECT_PLANT11     ) return "Greenery11";
    if (value == OBJECT_PLANT12     ) return "Greenery12";
    if (value == OBJECT_PLANT13     ) return "Greenery13";
    if (value == OBJECT_PLANT14     ) return "Greenery14";
    if (value == OBJECT_PLANT15     ) return "Greenery15";
    if (value == OBJECT_PLANT16     ) return "Greenery16";
    if (value == OBJECT_PLANT17     ) return "Greenery17";
    if (value == OBJECT_PLANT18     ) return "Greenery18";
    if (value == OBJECT_PLANT19     ) return "Greenery19";
    if (value == OBJECT_TREE0       ) return "Tree0";
    if (value == OBJECT_TREE1       ) return "Tree1";
    if (value == OBJECT_TREE2       ) return "Tree2";
    if (value == OBJECT_TREE3       ) return "Tree3";
    if (value == OBJECT_TREE4       ) return "Tree4";
    if (value == OBJECT_TREE5       ) return "Tree5";
    if (value == OBJECT_MUSHROOM1   ) return "Mushroom1";
    if (value == OBJECT_MUSHROOM2   ) return "Mushroom2";
    if (value == OBJECT_HOME1       ) return "Home";
    if (value == OBJECT_DERRICK     ) return "Derrick";
    if (value == OBJECT_FACTORY     ) return "BotFactory";
    if (value == OBJECT_STATION     ) return "PowerStation";
    if (value == OBJECT_CONVERT     ) return "Converter";
    if (value == OBJECT_REPAIR      ) return "RepairCenter";
    if (value == OBJECT_DESTROYER   ) return "Destroyer";
    if (value == OBJECT_TOWER       ) return "DefenseTower";
    if (value == OBJECT_NEST        ) return "AlienNest";
    if (value == OBJECT_RESEARCH    ) return "ResearchCenter";
    if (value == OBJECT_RADAR       ) return "RadarStation";
    if (value == OBJECT_INFO        ) return "ExchangePost";
    if (value == OBJECT_ENERGY      ) return "PowerPlant";
    if (value == OBJECT_LABO        ) return "AutoLab";
    if (value == OBJECT_NUCLEAR     ) return "NuclearPlant";
    if (value == OBJECT_PARA        ) return "PowerCaptor";
    if (value == OBJECT_SAFE        ) return "Vault";
    if (value == OBJECT_HUSTON      ) return "Houston";
    if (value == OBJECT_TARGET1     ) return "Target1";
    if (value == OBJECT_TARGET2     ) return "Target2";
    if (value == OBJECT_START       ) return "StartArea";
    if (value == OBJECT_END         ) return "GoalArea";
    if (value == OBJECT_MOTHER      ) return "AlienQueen";
    if (value == OBJECT_EGG         ) return "AlienEgg";
    if (value == OBJECT_ANT         ) return "AlienAnt";
    if (value == OBJECT_SPIDER      ) return "AlienSpider";
    if (value == OBJECT_BEE         ) return "AlienWasp";
    if (value == OBJECT_WORM        ) return "AlienWorm";
    if (value == OBJECT_RUINmobilew1) return "WreckBotw1";
    if (value == OBJECT_RUINmobilew2) return "WreckBotw2";
    if (value == OBJECT_RUINmobilet1) return "WreckBott1";
    if (value == OBJECT_RUINmobilet2) return "WreckBott2";
    if (value == OBJECT_RUINmobiler1) return "WreckBotr1";
    if (value == OBJECT_RUINmobiler2) return "WreckBotr2";
    if (value == OBJECT_RUINfactory ) return "RuinBotFactory";
    if (value == OBJECT_RUINdoor    ) return "RuinDoor";
    if (value == OBJECT_RUINsupport ) return "RuinSupport";
    if (value == OBJECT_RUINradar   ) return "RuinRadar";
    if (value == OBJECT_RUINconvert ) return "RuinConvert";
    if (value == OBJECT_RUINbase    ) return "RuinBaseCamp";
    if (value == OBJECT_RUINhead    ) return "RuinHeadCamp";
    if (value == OBJECT_BARRIER0    ) return "Barrier0";
    if (value == OBJECT_BARRIER1    ) return "Barrier1";
    if (value == OBJECT_BARRIER2    ) return "Barrier2";
    if (value == OBJECT_BARRIER3    ) return "Barrier3";
    if (value == OBJECT_TEEN0       ) return "Teen0";
    if (value == OBJECT_TEEN1       ) return "Teen1";
    if (value == OBJECT_TEEN2       ) return "Teen2";
    if (value == OBJECT_TEEN3       ) return "Teen3";
    if (value == OBJECT_TEEN4       ) return "Teen4";
    if (value == OBJECT_TEEN5       ) return "Teen5";
    if (value == OBJECT_TEEN6       ) return "Teen6";
    if (value == OBJECT_TEEN7       ) return "Teen7";
    if (value == OBJECT_TEEN8       ) return "Teen8";
    if (value == OBJECT_TEEN9       ) return "Teen9";
    if (value == OBJECT_TEEN10      ) return "Teen10";
    if (value == OBJECT_TEEN11      ) return "Teen11";
    if (value == OBJECT_TEEN12      ) return "Teen12";
    if (value == OBJECT_TEEN13      ) return "Teen13";
    if (value == OBJECT_TEEN14      ) return "Teen14";
    if (value == OBJECT_TEEN15      ) return "Teen15";
    if (value == OBJECT_TEEN16      ) return "Teen16";
    if (value == OBJECT_TEEN17      ) return "Teen17";
    if (value == OBJECT_TEEN18      ) return "Teen18";
    if (value == OBJECT_TEEN19      ) return "Teen19";
    if (value == OBJECT_TEEN20      ) return "Teen20";
    if (value == OBJECT_TEEN21      ) return "Teen21";
    if (value == OBJECT_TEEN22      ) return "Teen22";
    if (value == OBJECT_TEEN23      ) return "Teen23";
    if (value == OBJECT_TEEN24      ) return "Teen24";
    if (value == OBJECT_TEEN25      ) return "Teen25";
    if (value == OBJECT_TEEN26      ) return "Teen26";
    if (value == OBJECT_TEEN27      ) return "Teen27";
    if (value == OBJECT_TEEN28      ) return "Teen28";
    if (value == OBJECT_TEEN29      ) return "Teen29";
    if (value == OBJECT_TEEN30      ) return "Teen30";
    if (value == OBJECT_TEEN31      ) return "Teen31";
    if (value == OBJECT_TEEN32      ) return "Teen32";
    if (value == OBJECT_TEEN33      ) return "Teen33";
    if (value == OBJECT_TEEN34      ) return "Stone";
    if (value == OBJECT_TEEN35      ) return "Teen35";
    if (value == OBJECT_TEEN36      ) return "Teen36";
    if (value == OBJECT_TEEN37      ) return "Teen37";
    if (value == OBJECT_TEEN38      ) return "Teen38";
    if (value == OBJECT_TEEN39      ) return "Teen39";
    if (value == OBJECT_TEEN40      ) return "Teen40";
    if (value == OBJECT_TEEN41      ) return "Teen41";
    if (value == OBJECT_TEEN42      ) return "Teen42";
    if (value == OBJECT_TEEN43      ) return "Teen43";
    if (value == OBJECT_TEEN44      ) return "Teen44";
    if (value == OBJECT_QUARTZ0     ) return "Quartz0";
    if (value == OBJECT_QUARTZ1     ) return "Quartz1";
    if (value == OBJECT_QUARTZ2     ) return "Quartz2";
    if (value == OBJECT_QUARTZ3     ) return "Quartz3";
    if (value == OBJECT_ROOT0       ) return "MegaStalk0";
    if (value == OBJECT_ROOT1       ) return "MegaStalk1";
    if (value == OBJECT_ROOT2       ) return "MegaStalk2";
    if (value == OBJECT_ROOT3       ) return "MegaStalk3";
    if (value == OBJECT_ROOT4       ) return "MegaStalk4";
    if (value == OBJECT_ROOT5       ) return "MegaStalk5";
    if (value == OBJECT_APOLLO1     ) return "ApolloLEM";
    if (value == OBJECT_APOLLO2     ) return "ApolloJeep";
    if (value == OBJECT_APOLLO3     ) return "ApolloFlag";
    if (value == OBJECT_APOLLO4     ) return "ApolloModule";
    if (value == OBJECT_APOLLO5     ) return "ApolloAntenna";
    if (value == OBJECT_HUMAN       ) return "Me";
    if (value == OBJECT_TECH        ) return "Tech";
    if (value == OBJECT_CONTROLLER  ) return "MissionController";
    return boost::lexical_cast<std::string>(static_cast<int>(value));
}

ObjectType CLevelParserParam::AsObjectType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToObjectType(m_value);
}

ObjectType CLevelParserParam::AsObjectType(ObjectType def)
{
    if (m_empty)
        return def;
    return AsObjectType();
}


DriveType CLevelParserParam::ToDriveType(std::string value)
{
    if (value == "Wheeled") return DriveType::Wheeled;
    if (value == "Tracked") return DriveType::Tracked;
    if (value == "Winged" ) return DriveType::Winged;
    if (value == "Legged" ) return DriveType::Legged;
    if (value == "BigTracked") return DriveType::BigTracked;
    if (value == "Other"  ) return DriveType::Other;
    return static_cast<DriveType>(Cast<int>(value, "drive"));
}

DriveType CLevelParserParam::AsDriveType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToDriveType(m_value);
}

DriveType CLevelParserParam::AsDriveType(DriveType def)
{
    if (m_empty)
        return def;
    return AsDriveType();
}


ToolType CLevelParserParam::ToToolType(std::string value)
{
    if (value == "Grabber"    ) return ToolType::Grabber;
    if (value == "Sniffer"    ) return ToolType::Sniffer;
    if (value == "Shooter"    ) return ToolType::Shooter;
    if (value == "OrgaShooter") return ToolType::OrganicShooter;
    if (value == "Other"      ) return ToolType::Other;
    return static_cast<ToolType>(Cast<int>(value, "tool"));
}

ToolType CLevelParserParam::AsToolType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToToolType(m_value);
}

ToolType CLevelParserParam::AsToolType(ToolType def)
{
    if (m_empty)
        return def;
    return AsToolType();
}


Gfx::WaterType CLevelParserParam::ToWaterType(std::string value)
{
    if (value == "nullptr") return Gfx::WATER_NULL;
    if (value == "TT"  ) return Gfx::WATER_TT;
    if (value == "TO"  ) return Gfx::WATER_TO;
    if (value == "CT"  ) return Gfx::WATER_CT;
    if (value == "CO"  ) return Gfx::WATER_CO;
    return static_cast<Gfx::WaterType>(Cast<int>(value, "watertype"));
}

Gfx::WaterType CLevelParserParam::AsWaterType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToWaterType(m_value);
}

Gfx::WaterType CLevelParserParam::AsWaterType(Gfx::WaterType def)
{
    if (m_empty)
        return def;
    return AsWaterType();
}


Gfx::EngineObjectType CLevelParserParam::ToTerrainType(std::string value)
{
    if (value == "Terrain") return Gfx::ENG_OBJTYPE_TERRAIN;
    if (value == "Object" ) return Gfx::ENG_OBJTYPE_FIX;
    if (value == "Quartz" ) return Gfx::ENG_OBJTYPE_QUARTZ;
    if (value == "Metal"  ) return Gfx::ENG_OBJTYPE_METAL;
    return static_cast<Gfx::EngineObjectType>(Cast<int>(value, "terraintype"));
}

Gfx::EngineObjectType CLevelParserParam::AsTerrainType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToTerrainType(m_value);
}

Gfx::EngineObjectType CLevelParserParam::AsTerrainType(Gfx::EngineObjectType def)
{
    if (m_empty)
        return def;
    return AsTerrainType();
}


int CLevelParserParam::ToBuildFlag(std::string value)
{
    if (value == "BotFactory"    ) return BUILD_FACTORY;
    if (value == "Derrick"       ) return BUILD_DERRICK;
    if (value == "Converter"     ) return BUILD_CONVERT;
    if (value == "RadarStation"  ) return BUILD_RADAR;
    if (value == "PowerPlant"    ) return BUILD_ENERGY;
    if (value == "NuclearPlant"  ) return BUILD_NUCLEAR;
    if (value == "FuelCellPlant" ) return BUILD_NUCLEAR;
    if (value == "PowerStation"  ) return BUILD_STATION;
    if (value == "RepairCenter"  ) return BUILD_REPAIR;
    if (value == "DefenseTower"  ) return BUILD_TOWER;
    if (value == "ResearchCenter") return BUILD_RESEARCH;
    if (value == "AutoLab"       ) return BUILD_LABO;
    if (value == "PowerCaptor"   ) return BUILD_PARA;
    if (value == "ExchangePost"  ) return BUILD_INFO;
    if (value == "Destroyer"     ) return BUILD_DESTROYER;
    if (value == "FlatGround"    ) return BUILD_GFLAT;
    if (value == "Flag"          ) return BUILD_FLAG;
    return Cast<int>(value, "buildflag");
}

int CLevelParserParam::AsBuildFlag()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToBuildFlag(m_value);
}

int CLevelParserParam::AsBuildFlag(int def)
{
    if (m_empty)
        return def;
    return AsBuildFlag();
}


int CLevelParserParam::ToResearchFlag(std::string value)
{
    if (value == "TRACKER" ) return RESEARCH_TANK;
    if (value == "WINGER"  ) return RESEARCH_FLY;
    if (value == "THUMPER" ) return RESEARCH_THUMP;
    if (value == "SHOOTER" ) return RESEARCH_CANON;
    if (value == "TOWER"   ) return RESEARCH_TOWER;
    if (value == "PHAZER"  ) return RESEARCH_PHAZER;
    if (value == "SHIELDER") return RESEARCH_SHIELD;
    if (value == "ATOMIC"  ) return RESEARCH_ATOMIC;
    if (value == "iPAW"    ) return RESEARCH_iPAW;
    if (value == "iGUN"    ) return RESEARCH_iGUN;
    if (value == "RECYCLER") return RESEARCH_RECYCLER;
    if (value == "SUBBER"  ) return RESEARCH_SUBM;
    if (value == "SNIFFER" ) return RESEARCH_SNIFFER;
    return Cast<int>(value, "researchflag");
}

int CLevelParserParam::AsResearchFlag()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToResearchFlag(m_value);
}

int CLevelParserParam::AsResearchFlag(int def)
{
    if (m_empty)
        return def;
    return AsResearchFlag();
}


Gfx::PyroType CLevelParserParam::ToPyroType(std::string value)
{
    if (value == "FRAGt" ) return Gfx::PT_FRAGT;
    if (value == "FRAGo" ) return Gfx::PT_FRAGO;
    if (value == "FRAGw" ) return Gfx::PT_FRAGW;
    if (value == "EXPLOt") return Gfx::PT_EXPLOT;
    if (value == "EXPLOo") return Gfx::PT_EXPLOO;
    if (value == "EXPLOw") return Gfx::PT_EXPLOW;
    if (value == "SHOTt" ) return Gfx::PT_SHOTT;
    if (value == "SHOTh" ) return Gfx::PT_SHOTH;
    if (value == "SHOTm" ) return Gfx::PT_SHOTM;
    if (value == "SHOTw" ) return Gfx::PT_SHOTW;
    if (value == "EGG"   ) return Gfx::PT_EGG;
    if (value == "BURNt" ) return Gfx::PT_BURNT;
    if (value == "BURNo" ) return Gfx::PT_BURNO;
    if (value == "SPIDER") return Gfx::PT_SPIDER;
    if (value == "FALL"  ) return Gfx::PT_FALL;
    if (value == "RESET" ) return Gfx::PT_RESET;
    if (value == "WIN"   ) return Gfx::PT_WIN;
    if (value == "LOST"  ) return Gfx::PT_LOST;
    return static_cast<Gfx::PyroType>(Cast<int>(value, "pyrotype"));
}

Gfx::PyroType CLevelParserParam::AsPyroType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToPyroType(m_value);
}

Gfx::PyroType CLevelParserParam::AsPyroType(Gfx::PyroType def)
{
    if (m_empty)
        return def;
    return AsPyroType();
}


Gfx::CameraType CLevelParserParam::ToCameraType(std::string value)
{
    if (value == "BACK"   ) return Gfx::CAM_TYPE_BACK;
    if (value == "PLANE"  ) return Gfx::CAM_TYPE_PLANE;
    if (value == "ONBOARD") return Gfx::CAM_TYPE_ONBOARD;
    if (value == "FIX"    ) return Gfx::CAM_TYPE_FIX;
    return static_cast<Gfx::CameraType>(Cast<int>(value, "camera"));
}

const std::string CLevelParserParam::FromCameraType(Gfx::CameraType value)
{
    if (value == Gfx::CAM_TYPE_ONBOARD) return "ONBOARD";
    if (value == Gfx::CAM_TYPE_FIX    ) return "FIX";
    return boost::lexical_cast<std::string>(static_cast<int>(value));
}

Gfx::CameraType CLevelParserParam::AsCameraType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToCameraType(m_value);
}

Gfx::CameraType CLevelParserParam::AsCameraType(Gfx::CameraType def)
{
    if (m_empty)
        return def;
    return AsCameraType();
}

MissionType CLevelParserParam::ToMissionType(std::string value)
{
    if (value == "NORMAL"     ) return MISSION_NORMAL;
    if (value == "RETRO"      ) return MISSION_RETRO;
    if (value == "CODE_BATTLE") return MISSION_CODE_BATTLE;
    return static_cast<MissionType>(Cast<int>(value, "MissionType"));
}

MissionType CLevelParserParam::AsMissionType()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);
    return ToMissionType(m_value);
}

MissionType CLevelParserParam::AsMissionType(MissionType def)
{
    if (m_empty)
        return def;
    return AsMissionType();
}

Gfx::PlanetType CLevelParserParam::AsPlanetType()
{
    Gfx::PlanetType planetType{};

    if (m_value == "0")
        planetType = Gfx::PlanetType::Sky;
    else if (m_value == "1")
        planetType = Gfx::PlanetType::OuterSpace;

    return planetType;
}

void CLevelParserParam::ParseArray()
{
    if (m_array.size() != 0)
        return;

    std::vector<std::string> values;
    boost::split(values, m_value, boost::is_any_of(";"));
    int i = 0;
    for (auto& value : values)
    {
        boost::algorithm::trim(value);
        if (value.empty()) continue;
        auto param = MakeUnique<CLevelParserParam>(m_name + "[" + boost::lexical_cast<std::string>(i) + "]", value);
        param->SetLine(m_line);
        m_array.push_back(std::move(param));
        i++;
    }
}

void CLevelParserParam::LoadArray()
{
    m_value = "";
    bool first = true;
    for (auto& value : m_array)
    {
        if (!first)
            m_value += ";";
        m_value += value->GetValue();
        first = false;
    }
}

const CLevelParserParamVec& CLevelParserParam::AsArray()
{
    if (m_empty)
        throw CLevelParserExceptionMissingParam(this);

    ParseArray();

    return m_array;
}
