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

#ifndef _MISC_H_
#define	_MISC_H_

#include <time.h>
#include "metafile.h"


extern CMetaFile	g_metafile;



// Classes existantes.

enum ClassType
{
	CLASS_EVENT			= 1,
	CLASS_INTERFACE		= 2,
	CLASS_MAIN			= 3,
	CLASS_ENGINE		= 4,
	CLASS_TERRAIN		= 5,
	CLASS_OBJECT		= 6,
	CLASS_PHYSICS		= 7,
	CLASS_BRAIN			= 8,
	CLASS_CAMERA		= 9,
	CLASS_LIGHT			= 10,
	CLASS_PARTICULE		= 11,
	CLASS_AUTO			= 12,
	CLASS_DISPLAYTEXT	= 13,
	CLASS_PYRO			= 14,
	CLASS_SCRIPT		= 15,
	CLASS_TEXT			= 16,
	CLASS_STUDIO		= 17,
	CLASS_WATER			= 18,
	CLASS_CLOUD			= 19,
	CLASS_MOTION		= 20,
	CLASS_SOUND			= 21,
	CLASS_PLANET		= 22,
	CLASS_TASKMANAGER   = 23,
	CLASS_DIALOG		= 24,
	CLASS_MAP			= 25,
	CLASS_SHORT			= 26,
	CLASS_BLITZ			= 27,
};

#define CLASS_MAX		30



enum Error
{
	ERR_OK				= 0,		// ok
	ERR_GENERIC			= 1,		// erreur quelconque
	ERR_CONTINUE		= 2,		// continue
	ERR_STOP			= 3,		// stoppe
	ERR_CMD				= 4,		// commande inconnue
	ERR_INSTALL			= 20,		// programme mal install�
	ERR_NOCD			= 21,		// CD pas trouv�
	ERR_MANIP_VEH		= 100,		// v�hicule inadapt�
	ERR_MANIP_FLY		= 101,		// impossible en vol
	ERR_MANIP_BUSY		= 102,		// prend: porte d�j� qq chose
	ERR_MANIP_NIL		= 103,		// prend: rien � prendre
	ERR_MANIP_MOTOR		= 105,		// d�pose: impossible en mouvement
	ERR_MANIP_OCC		= 106,		// d�pose: emplacement d�j� occup�
	ERR_MANIP_FRIEND	= 107,		// pas d'autre v�hicule
	ERR_MANIP_RADIO		= 108,		// impossible car radioactif
	ERR_MANIP_WATER		= 109,		// impossible sous l'eau
	ERR_MANIP_EMPTY		= 110,		// rien � d�poser
	ERR_BUILD_FLY		= 120,		// impossible en vol
	ERR_BUILD_WATER		= 121,		// impossible sous l'eau
	ERR_BUILD_ENERGY	= 122,		// pas assez d'�nergie
	ERR_BUILD_METALAWAY	= 123,		// pas de m�tal (trop loin)
	ERR_BUILD_METALNEAR	= 124,		// pas de m�tal (trop proche)
	ERR_BUILD_METALINEX	= 125,		// m�tal inexistant
	ERR_BUILD_FLAT		= 126,		// sol pas assez plat
	ERR_BUILD_FLATLIT	= 127,		// sol plat pas assez grand
	ERR_BUILD_BUSY		= 128,		// enplacement occup�
	ERR_BUILD_BASE		= 129,		// trop proche de la fus�e
	ERR_BUILD_NARROW	= 130,		// b�timents trop serr�s
	ERR_BUILD_MOTOR		= 131,		// construit: impossible en mouvement
	ERR_SEARCH_FLY		= 140,		// impossible en vol
	ERR_SEARCH_VEH		= 141,		// v�hicule inadapt�
	ERR_SEARCH_MOTOR	= 142,		// impossible en mouvement
	ERR_TERRA_VEH		= 150,		// v�hicule inadapt�
	ERR_TERRA_ENERGY	= 151,		// pas assez d'�nergie
	ERR_TERRA_FLOOR		= 152,		// terrain inadapt�
	ERR_TERRA_BUILDING	= 153,		// batiment trop proche
	ERR_TERRA_OBJECT	= 154,		// object trop proche
	ERR_FIRE_VEH		= 160,		// v�hicule inadapt�
	ERR_FIRE_ENERGY		= 161,		// pas assez d'�nergie
	ERR_FIRE_FLY		= 162,		// impossible en vol
	ERR_RECOVER_VEH		= 170,		// v�hicule inadapt�
	ERR_RECOVER_ENERGY	= 171,		// pas assez d'�nergie
	ERR_RECOVER_NULL	= 172,		// pas de ruine
	ERR_CONVERT_EMPTY	= 180,		// pas de pierre � transformer
	ERR_SHIELD_VEH		= 190,		// v�hicule inadapt�
	ERR_SHIELD_ENERGY	= 191,		// pas assez d'�nergie
	ERR_MOVE_IMPOSSIBLE	= 200,		// move impossible
	ERR_FIND_IMPOSSIBLE	= 201,		// find impossible
	ERR_GOTO_IMPOSSIBLE	= 210,		// goto impossible
	ERR_GOTO_ITER		= 211,		// goto trop compliqu�
	ERR_GOTO_BUSY		= 212,		// destination goto occup�e
	ERR_DERRICK_NULL	= 300,		// pas de minerai en sous-sol
	ERR_STATION_NULL	= 301,		// pas d'�nergie en sous-sol
	ERR_TOWER_POWER		= 310,		// pas de pile
	ERR_TOWER_ENERGY	= 311,		// plus d'�nergie
	ERR_RESEARCH_POWER	= 320,		// pas de pile
	ERR_RESEARCH_ENERGY	= 321,		// plus d'�nergie
	ERR_RESEARCH_TYPE	= 322,		// pas le bon type de pile
	ERR_RESEARCH_ALREADY= 323,		// recherche d�j� fa�te
	ERR_ENERGY_NULL		= 330,		// pas d'�nergie en sous-sol
	ERR_ENERGY_LOW		= 331,		// pas encore assez d'�nergie
	ERR_ENERGY_EMPTY	= 332,		// pas de m�tal � transformer
	ERR_ENERGY_BAD		= 333,		// ne transforme que le m�tal
	ERR_BASE_DLOCK		= 340,		// portes bloqu�es
	ERR_BASE_DHUMAN		= 341,		// vous devez embarquer
	ERR_LABO_NULL		= 350,		// rien � analyser
	ERR_LABO_BAD		= 351,		// pas de boulet � analyser
	ERR_LABO_ALREADY	= 352,		// analyse d�j� fa�te
	ERR_NUCLEAR_NULL	= 360,		// pas d'�nergie en sous-sol
	ERR_NUCLEAR_LOW		= 361,		// pas encore assez d'�nergie
	ERR_NUCLEAR_EMPTY	= 362,		// pas d'uranium � transformer
	ERR_NUCLEAR_BAD		= 363,		// ne transforme que l'uranium
	ERR_FACTORY_NULL	= 370,		// pas de m�tal
	ERR_FACTORY_NEAR	= 371,		// v�hicule trop proche
	ERR_RESET_NEAR		= 380,		// v�hicule trop proche
	ERR_INFO_NULL		= 390,		// pas de borne d'information
	ERR_VEH_VIRUS		= 400,		// v�hicule infect� par un virus
	ERR_BAT_VIRUS		= 401,		// b�timent infect� par un virus
	ERR_VEH_POWER		= 500,		// pas de pile
	ERR_VEH_ENERGY		= 501,		// plus d'�nergie
	ERR_FLAG_FLY		= 510,		// impossible en vol
	ERR_FLAG_WATER		= 511,		// impossible en nageant
	ERR_FLAG_MOTOR		= 512,		// impossible en mouvement
	ERR_FLAG_BUSY		= 513,		// prend: porte d�j� qq chose
	ERR_FLAG_CREATE		= 514,		// trop d'indicateurs
	ERR_FLAG_PROXY		= 515,		// trop proche
	ERR_FLAG_DELETE		= 516,		// rien � supprimer
	ERR_MISSION_NOTERM	= 600,		// mission pas termin�e
	ERR_DELETEMOBILE	= 700,		// v�hicule d�truit
	ERR_DELETEBUILDING	= 701,		// b�timent d�truit
	ERR_TOOMANY			= 702,		// trop d'objets
	ERR_OBLIGATORYTOKEN	= 800,		// instruction obligatoire manquante
	ERR_PROHIBITEDTOKEN	= 801,		// instruction interdite

	INFO_FIRST			= 10000,	// premi�re information
	INFO_BUILD			= 10001,	// construction termin�e
	INFO_CONVERT		= 10002,	// m�tal disponible
	INFO_RESEARCH		= 10003,	// recherche termin�e
	INFO_FACTORY		= 10004,	// v�hicule fabriqu�
	INFO_LABO			= 10005,	// analyse termin�e
	INFO_ENERGY			= 10006,	// pile disponible
	INFO_NUCLEAR		= 10007,	// pile nucl�aire disponible
	INFO_FINDING		= 10008,	// pile nucl�aire disponible
	INFO_MARKPOWER		= 10020,	// emplacement pour station trouv�
	INFO_MARKURANIUM	= 10021,	// emplacement pour derrick trouv�
	INFO_MARKSTONE		= 10022,	// emplacement pour derrick trouv�
	INFO_MARKKEYa		= 10023,	// emplacement pour derrick trouv�
	INFO_MARKKEYb		= 10024,	// emplacement pour derrick trouv�
	INFO_MARKKEYc		= 10025,	// emplacement pour derrick trouv�
	INFO_MARKKEYd		= 10026,	// emplacement pour derrick trouv�
	INFO_RESEARCHTANK	= 10030,	// recherche termin�e
	INFO_RESEARCHFLY	= 10031,	// recherche termin�e
	INFO_RESEARCHTHUMP	= 10032,	// recherche termin�e
	INFO_RESEARCHCANON	= 10033,	// recherche termin�e
	INFO_RESEARCHTOWER	= 10034,	// recherche termin�e
	INFO_RESEARCHPHAZER	= 10035,	// recherche termin�e
	INFO_RESEARCHSHIELD	= 10036,	// recherche termin�e
	INFO_RESEARCHATOMIC	= 10037,	// recherche termin�e
	INFO_WIN			= 10040,	// gagn�
	INFO_LOST			= 10041,	// perdu
	INFO_LOSTq			= 10042,	// perdu imm�diatement
	INFO_WRITEOK		= 10043,	// enregistrement effectu�
	INFO_DELETEPATH		= 10050,	// marque chemin supprim�e
	INFO_DELETEMOTHER	= 10100,	// insecte tu�
	INFO_DELETEANT		= 10101,	// insecte tu�
	INFO_DELETEBEE		= 10102,	// insecte tu�
	INFO_DELETEWORM		= 10103,	// insecte tu�
	INFO_DELETESPIDER	= 10104,	// insecte tu�
	INFO_BEGINSATCOM	= 10105,	// utilisez votre SatCom
};


// Etat du clavier.

#define KS_PAGEUP			(1<<4)
#define KS_PAGEDOWN			(1<<5)
#define KS_SHIFT			(1<<6)
#define KS_CONTROL			(1<<7)
#define KS_MLEFT			(1<<8)
#define KS_MRIGHT			(1<<9)
#define KS_NUMUP			(1<<10)
#define KS_NUMDOWN			(1<<11)
#define KS_NUMLEFT			(1<<12)
#define KS_NUMRIGHT			(1<<13)
#define KS_NUMPLUS			(1<<14)
#define KS_NUMMINUS			(1<<15)


// Proc�dures.

enum EventMsg;

extern EventMsg	GetUniqueEventMsg();

extern char RetNoAccent(char letter);
extern char RetToUpper(char letter);
extern char RetToLower(char letter);

extern void TimeToAscii(time_t time, char *buffer);

extern BOOL CopyFileToTemp(char* filename);
extern BOOL CopyFileListToTemp(char* filename, int* list, int total);
extern void AddExt(char* filename, char* ext);
extern void UserDir(BOOL bUser, char* dir);
extern void UserDir(char* buffer, char* dir, char* def);

extern char RetLanguageLetter();



#endif //_MISC_H_
