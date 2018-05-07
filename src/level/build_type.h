/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once


/**
 * \enum BuildType
 * \brief Construction actions (buildings, etc.) available to user
 *
 * TODO: refactor
 */
enum BuildType
{
    BUILD_FACTORY       = (1<<0),       //! < BotFactory
    BUILD_DERRICK       = (1<<1),       //! < Derrick
    BUILD_CONVERT       = (1<<2),       //! < Converter
    BUILD_RADAR         = (1<<3),       //! < RadarStation
    BUILD_ENERGY        = (1<<4),       //! < PowerPlant
    BUILD_NUCLEAR       = (1<<5),       //! < NuclearPlant
    BUILD_STATION       = (1<<6),       //! < PowerStation
    BUILD_REPAIR        = (1<<7),       //! < RepairCenter
    BUILD_TOWER         = (1<<8),       //! < DefenseTower
    BUILD_RESEARCH      = (1<<9),       //! < ResearchCenter
    BUILD_LABO          = (1<<10),      //! < AutoLab
    BUILD_PARA          = (1<<11),      //! < PowerCaptor
    BUILD_INFO          = (1<<12),      //! < ExchangePost
    BUILD_DESTROYER     = (1<<13),      //! < Destroyer
    BUILD_GFLAT         = (1<<16),      //! < checking flat ground
    BUILD_FLAG          = (1<<17)       //! < putting / removing flags
};
