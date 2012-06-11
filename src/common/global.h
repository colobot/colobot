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

// global.h


enum BuildType
{
	BUILD_FACTORY		= (1<<0),		// factory
	BUILD_DERRICK		= (1<<1),		// derrick
	BUILD_CONVERT		= (1<<2),		// converter
	BUILD_RADAR			= (1<<3),		// radar
	BUILD_ENERGY		= (1<<4),		// factory of cells
	BUILD_NUCLEAR		= (1<<5),		// nuclear power plant
	BUILD_STATION		= (1<<6),		// base station
	BUILD_REPAIR		= (1<<7),		// repair center
	BUILD_TOWER			= (1<<8),		// defense tower
	BUILD_RESEARCH		= (1<<9),		// research center
	BUILD_LABO			= (1<<10),		// laboratory
	BUILD_PARA			= (1<<11),		// lightning protection
	BUILD_INFO			= (1<<12),		// information terminal
	BUILD_GFLAT			= (1<<16),		// flat floor
	BUILD_FLAG			= (1<<17)		// puts / removes colored flag
};


// Do not change values ​​was because of backups (bits = ...).
enum ResearchType
{
	RESEARCH_TANK		= (1<<0),		// caterpillars
	RESEARCH_FLY		= (1<<1),		// wings
	RESEARCH_CANON		= (1<<2),		// cannon
	RESEARCH_TOWER		= (1<<3),		// defense tower
	RESEARCH_ATOMIC		= (1<<4),		// nuclear
	RESEARCH_THUMP		= (1<<5),		// thumper
	RESEARCH_SHIELD		= (1<<6),		// shield
	RESEARCH_PHAZER		= (1<<7),		// phazer gun
	RESEARCH_iPAW		= (1<<8),		// legs of insects
	RESEARCH_iGUN		= (1<<9),		// cannon of insects
	RESEARCH_RECYCLER	= (1<<10),		// recycler
	RESEARCH_SUBM		= (1<<11),		// submarine
	RESEARCH_SNIFFER	= (1<<12)		// sniffer
};

extern long		g_id;					// unique identifier
extern long		g_build;				// constructible buildings
extern long		g_researchDone;			// research done
extern long		g_researchEnable;		// research available
extern float	g_unit;					// conversion factor


