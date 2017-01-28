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
	CLASS_RECORDER		= 28,
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
	ERR_MISSION_NOTERM	= 600,		// mission pas terminée

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
