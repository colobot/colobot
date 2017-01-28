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
	CLASS_CAMERA		= 7,
	CLASS_LIGHT			= 8,
	CLASS_PARTICULE		= 9,
	CLASS_AUTO			= 10,
	CLASS_DISPLAYTEXT	= 11,
	CLASS_PYRO			= 12,
	CLASS_SCRIPT		= 13,
	CLASS_TEXT			= 14,
	CLASS_WATER			= 15,
	CLASS_CLOUD			= 16,
	CLASS_MOTION		= 17,
	CLASS_SOUND			= 18,
	CLASS_PLANET		= 19,
	CLASS_TASKMANAGER   = 20,
	CLASS_DIALOG		= 21,
	CLASS_EDIT			= 22,
	CLASS_MAP			= 23,
	CLASS_SHORT			= 24,
	CLASS_BLITZ			= 25,
	CLASS_TASKLIST		= 26,
	CLASS_FILEBUFFER	= 27,
	CLASS_UNDO			= 28,
	CLASS_GAMERFILE		= 29,
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
	ERR_FIRE_VEH		= 160,		// véhicule inadapté
	ERR_FIRE_ENERGY		= 161,		// pas assez d'énergie
	ERR_MOVE_IMPOSSIBLE	= 200,		// move impossible
	ERR_GOTO_IMPOSSIBLE	= 210,		// goto impossible
	ERR_GOTO_ITER		= 211,		// goto trop compliqué
	ERR_GOTO_BUSY		= 212,		// destination goto occupée
	ERR_MISSION_NOTERM	= 300,		// mission pas terminée
	ERR_PUZZLE_ZEROBLUPI= 301,		// aucun blupi
	ERR_PUZZLE_ZEROGOAL	= 302,		// aucun ballon
	ERR_PUZZLE_LOGOAL	= 303,		// trop peu de ballons
	ERR_PUZZLE_HIGOAL	= 304,		// trop de ballons
	ERR_PUZZLE_MAXBLUPI	= 305,		// limite de blupi dépassée
	ERR_PUZZLE_MAXGOAL	= 306,		// limite de ballons dépassée
	ERR_PUZZLE_MAX		= 307,		// limite d'objets dépassée
	ERR_CREATE			= 400,		// fichier existe déjà

	INFO_FIRST			= 10000,	// première information
	INFO_WIN			= 10010,	// gagné
	INFO_LOST			= 10011,	// perdu
	INFO_LOSTq			= 10012,	// perdu immédiatement
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

extern void RemoveEscape(char *dst, char *src);
extern void InsertEscape(char *dst, char *src);
extern void SpaceEscape(char *dst, char *src);

extern void TimeToAscii(time_t time, char *buffer);
extern void PutTime(char *buffer, float time);
extern void Put100(char *buffer, int value);
extern void PutPoints(char *buffer, int value);

extern BOOL CopyFileToTemp(char* filename);
extern BOOL CopyFileListToTemp(char* filename, int* list, int total);
extern void AddExt(char* filename, char* ext);
extern void UserDir(BOOL bUser, char* dir);
extern void UserDir(char* buffer, char* dir, char* def);

extern char RetLanguageLetter();



#endif //_MISC_H_
