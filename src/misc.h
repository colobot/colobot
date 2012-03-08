// misc.h

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
	ERR_INSTALL			= 20,		// programme mal installé
	ERR_NOCD			= 21,		// CD pas trouvé
	ERR_MANIP_VEH		= 100,		// véhicule inadapté
	ERR_MANIP_FLY		= 101,		// impossible en vol
	ERR_MANIP_BUSY		= 102,		// prend: porte déjà qq chose
	ERR_MANIP_NIL		= 103,		// prend: rien à prendre
	ERR_MANIP_MOTOR		= 105,		// dépose: impossible en mouvement
	ERR_MANIP_OCC		= 106,		// dépose: emplacement déjà occupé
	ERR_MANIP_FRIEND	= 107,		// pas d'autre véhicule
	ERR_MANIP_RADIO		= 108,		// impossible car radioactif
	ERR_MANIP_WATER		= 109,		// impossible sous l'eau
	ERR_MANIP_EMPTY		= 110,		// rien à déposer
	ERR_BUILD_FLY		= 120,		// impossible en vol
	ERR_BUILD_WATER		= 121,		// impossible sous l'eau
	ERR_BUILD_ENERGY	= 122,		// pas assez d'énergie
	ERR_BUILD_METALAWAY	= 123,		// pas de métal (trop loin)
	ERR_BUILD_METALNEAR	= 124,		// pas de métal (trop proche)
	ERR_BUILD_METALINEX	= 125,		// métal inexistant
	ERR_BUILD_FLAT		= 126,		// sol pas assez plat
	ERR_BUILD_FLATLIT	= 127,		// sol plat pas assez grand
	ERR_BUILD_BUSY		= 128,		// enplacement occupé
	ERR_BUILD_BASE		= 129,		// trop proche de la fusée
	ERR_BUILD_NARROW	= 130,		// bâtiments trop serrés
	ERR_BUILD_MOTOR		= 131,		// construit: impossible en mouvement
	ERR_SEARCH_FLY		= 140,		// impossible en vol
	ERR_SEARCH_VEH		= 141,		// véhicule inadapté
	ERR_SEARCH_MOTOR	= 142,		// impossible en mouvement
	ERR_TERRA_VEH		= 150,		// véhicule inadapté
	ERR_TERRA_ENERGY	= 151,		// pas assez d'énergie
	ERR_TERRA_FLOOR		= 152,		// terrain inadapté
	ERR_TERRA_BUILDING	= 153,		// batiment trop proche
	ERR_TERRA_OBJECT	= 154,		// object trop proche
	ERR_FIRE_VEH		= 160,		// véhicule inadapté
	ERR_FIRE_ENERGY		= 161,		// pas assez d'énergie
	ERR_FIRE_FLY		= 162,		// impossible en vol
	ERR_RECOVER_VEH		= 170,		// véhicule inadapté
	ERR_RECOVER_ENERGY	= 171,		// pas assez d'énergie
	ERR_RECOVER_NULL	= 172,		// pas de ruine
	ERR_CONVERT_EMPTY	= 180,		// pas de pierre à transformer
	ERR_SHIELD_VEH		= 190,		// véhicule inadapté
	ERR_SHIELD_ENERGY	= 191,		// pas assez d'énergie
	ERR_MOVE_IMPOSSIBLE	= 200,		// move impossible
	ERR_FIND_IMPOSSIBLE	= 201,		// find impossible
	ERR_GOTO_IMPOSSIBLE	= 210,		// goto impossible
	ERR_GOTO_ITER		= 211,		// goto trop compliqué
	ERR_GOTO_BUSY		= 212,		// destination goto occupée
	ERR_DERRICK_NULL	= 300,		// pas de minerai en sous-sol
	ERR_STATION_NULL	= 301,		// pas d'énergie en sous-sol
	ERR_TOWER_POWER		= 310,		// pas de pile
	ERR_TOWER_ENERGY	= 311,		// plus d'énergie
	ERR_RESEARCH_POWER	= 320,		// pas de pile
	ERR_RESEARCH_ENERGY	= 321,		// plus d'énergie
	ERR_RESEARCH_TYPE	= 322,		// pas le bon type de pile
	ERR_RESEARCH_ALREADY= 323,		// recherche déjà faîte
	ERR_ENERGY_NULL		= 330,		// pas d'énergie en sous-sol
	ERR_ENERGY_LOW		= 331,		// pas encore assez d'énergie
	ERR_ENERGY_EMPTY	= 332,		// pas de métal à transformer
	ERR_ENERGY_BAD		= 333,		// ne transforme que le métal
	ERR_BASE_DLOCK		= 340,		// portes bloquées
	ERR_BASE_DHUMAN		= 341,		// vous devez embarquer
	ERR_LABO_NULL		= 350,		// rien à analyser
	ERR_LABO_BAD		= 351,		// pas de boulet à analyser
	ERR_LABO_ALREADY	= 352,		// analyse déjà faîte
	ERR_NUCLEAR_NULL	= 360,		// pas d'énergie en sous-sol
	ERR_NUCLEAR_LOW		= 361,		// pas encore assez d'énergie
	ERR_NUCLEAR_EMPTY	= 362,		// pas d'uranium à transformer
	ERR_NUCLEAR_BAD		= 363,		// ne transforme que l'uranium
	ERR_FACTORY_NULL	= 370,		// pas de métal
	ERR_FACTORY_NEAR	= 371,		// véhicule trop proche
	ERR_RESET_NEAR		= 380,		// véhicule trop proche
	ERR_INFO_NULL		= 390,		// pas de borne d'information
	ERR_VEH_VIRUS		= 400,		// véhicule infecté par un virus
	ERR_BAT_VIRUS		= 401,		// bâtiment infecté par un virus
	ERR_VEH_POWER		= 500,		// pas de pile
	ERR_VEH_ENERGY		= 501,		// plus d'énergie
	ERR_FLAG_FLY		= 510,		// impossible en vol
	ERR_FLAG_WATER		= 511,		// impossible en nageant
	ERR_FLAG_MOTOR		= 512,		// impossible en mouvement
	ERR_FLAG_BUSY		= 513,		// prend: porte déjà qq chose
	ERR_FLAG_CREATE		= 514,		// trop d'indicateurs
	ERR_FLAG_PROXY		= 515,		// trop proche
	ERR_FLAG_DELETE		= 516,		// rien à supprimer
	ERR_MISSION_NOTERM	= 600,		// mission pas terminée
	ERR_DELETEMOBILE	= 700,		// véhicule détruit
	ERR_DELETEBUILDING	= 701,		// bâtiment détruit
	ERR_TOOMANY			= 702,		// trop d'objets
	ERR_OBLIGATORYTOKEN	= 800,		// instruction obligatoire manquante
	ERR_PROHIBITEDTOKEN	= 801,		// instruction interdite

	INFO_FIRST			= 10000,	// première information
	INFO_BUILD			= 10001,	// construction terminée
	INFO_CONVERT		= 10002,	// métal disponible
	INFO_RESEARCH		= 10003,	// recherche terminée
	INFO_FACTORY		= 10004,	// véhicule fabriqué
	INFO_LABO			= 10005,	// analyse terminée
	INFO_ENERGY			= 10006,	// pile disponible
	INFO_NUCLEAR		= 10007,	// pile nucléaire disponible
	INFO_FINDING		= 10008,	// pile nucléaire disponible
	INFO_MARKPOWER		= 10020,	// emplacement pour station trouvé
	INFO_MARKURANIUM	= 10021,	// emplacement pour derrick trouvé
	INFO_MARKSTONE		= 10022,	// emplacement pour derrick trouvé
	INFO_MARKKEYa		= 10023,	// emplacement pour derrick trouvé
	INFO_MARKKEYb		= 10024,	// emplacement pour derrick trouvé
	INFO_MARKKEYc		= 10025,	// emplacement pour derrick trouvé
	INFO_MARKKEYd		= 10026,	// emplacement pour derrick trouvé
	INFO_RESEARCHTANK	= 10030,	// recherche terminée
	INFO_RESEARCHFLY	= 10031,	// recherche terminée
	INFO_RESEARCHTHUMP	= 10032,	// recherche terminée
	INFO_RESEARCHCANON	= 10033,	// recherche terminée
	INFO_RESEARCHTOWER	= 10034,	// recherche terminée
	INFO_RESEARCHPHAZER	= 10035,	// recherche terminée
	INFO_RESEARCHSHIELD	= 10036,	// recherche terminée
	INFO_RESEARCHATOMIC	= 10037,	// recherche terminée
	INFO_WIN			= 10040,	// gagné
	INFO_LOST			= 10041,	// perdu
	INFO_LOSTq			= 10042,	// perdu immédiatement
	INFO_WRITEOK		= 10043,	// enregistrement effectué
	INFO_DELETEPATH		= 10050,	// marque chemin supprimée
	INFO_DELETEMOTHER	= 10100,	// insecte tué
	INFO_DELETEANT		= 10101,	// insecte tué
	INFO_DELETEBEE		= 10102,	// insecte tué
	INFO_DELETEWORM		= 10103,	// insecte tué
	INFO_DELETESPIDER	= 10104,	// insecte tué
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


// Procédures.

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
