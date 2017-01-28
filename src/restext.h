// restext.h

#ifndef _RESTEXT_H_
#define	_RESTEXT_H_


#define STRICT
#define D3D_OVERLOADS



enum KeyRank;


// Types possibles pour les ressources texte.

enum ResType
{
	RES_TEXT		= 0,	// RT_*
	RES_EVENT		= 1,	// EVENT_*  (EventMsg)
	RES_OBJECT		= 2,	// OBJECT_* (ObjectType)
	RES_ERR			= 3,	// ERR_*    (Error)
	RES_KEY			= 4,	// VK_*		(touches)
	RES_CBOT		= 5,	// TX_*		(cbot.dll)
};


// Ressources de type RES_TEXT.

#define RT_VERSION_ID		1
#define RT_WINDOW_MAXIMIZED	3
#define RT_WINDOW_MINIMIZED	4
#define RT_WINDOW_STANDARD	5
#define RT_WINDOW_CLOSE		6

#define RT_NAME_DEFAULT		21
#define RT_KEY_OR			23

#define RT_TITLE_BASE		40
#define RT_TITLE_INIT		41
#define RT_TITLE_MISSION	42
#define RT_TITLE_FREE		43
#define RT_TITLE_PROTO		44
#define RT_TITLE_SETUP		45
#define RT_TITLE_NAME		46
#define RT_TITLE_PERSO		47
#define RT_TITLE_USER		50

#define RT_PLAY_LISTm		67
#define RT_PLAY_LISTf		68
#define RT_PLAY_LISTp		69
#define RT_PLAY_LISTu		70
#define RT_PLAY_PESETAS		71
#define RT_PLAY_PCREDIT		72
#define RT_PLAY_PMISSION	73
#define RT_PLAY_PNEXT		74

#define RT_SETUP_DEVICE		80
#define RT_SETUP_MODE		81
#define RT_SETUP_KEY1		82
#define RT_SETUP_KEY2		83

#define RT_PERSO_LIST		90
#define RT_PERSO_LEVEL		91
#define RT_PERSO_BUY		92

#define RT_DIALOG_TITLE		100
#define RT_DIALOG_ABORT		101
#define RT_DIALOG_QUIT		102
#define RT_DIALOG_YES		103
#define RT_DIALOG_NO		104
#define RT_DIALOG_DELGAME	106
#define RT_DIALOG_YESDEL	107
#define RT_DIALOG_NODEL		108
#define RT_DIALOG_NEWGAME	109
#define RT_DIALOG_YESNEW	110
#define RT_DIALOG_NONEW		111
#define RT_DIALOG_YESKID	113
#define RT_DIALOG_NOKID		114
#define RT_DIALOG_LOADING	115
#define RT_DIALOG_YESQUIT	116
#define RT_DIALOG_NOQUIT	117
#define RT_DIALOG_DELFILE	118

#define RT_GENERIC_DEV1		170
#define RT_GENERIC_DEV2		171
#define RT_GENERIC_EDIT1	172
#define RT_GENERIC_EDIT2	173
#define RT_GENERIC_DEMO1	174
#define RT_GENERIC_DEMO2	175
#define RT_GENERIC_DEMO3	176
#define RT_GENERIC_DEMO4	177
#define RT_GENERIC_DEMO5	178

#define RT_WIN_TIME			180
#define RT_WIN_SOLID		181
#define RT_WIN_CONE			182
#define RT_WIN_POINTS		183
#define RT_WIN_BONUS		184
#define RT_WIN_ARRAY		185
#define RT_WIN_HEADERm		186
#define RT_WIN_HEADERf		187

#define RT_START_READY		190
#define RT_START_QUICK		191
#define RT_START_123		192
#define RT_START_GO			193
#define RT_START_REMOTE		194

#define RT_MESSAGE_WIN		200
#define RT_MESSAGE_LOST		201

#define RT_RECORD_GALL		210
#define RT_RECORD_GONE		211
#define RT_RECORD_LONE		212
#define RT_RECORD_QALL		213
#define RT_RECORD_QONE		214
#define RT_RACE_LAP4		215
#define RT_RACE_LAP3		216
#define RT_RACE_LAP2		217
#define RT_RACE_LAP1		218
#define RT_DUEL_WIN			219
#define RT_DUEL_LOST		220

#define RT_SPEC_MINSPEED	225
#define RT_SPEC_MAXSPEED	226
#define RT_SPEC_DEMO		227
#define RT_SPEC_DISPO		228

#define RT_MOTOR_REAR		230
#define RT_MOTOR_NEUTRAL	231
#define RT_MOTOR_S1			232
#define RT_MOTOR_S2			233
#define RT_MOTOR_S3			234

#define RT_IO_LIST			240
#define RT_IO_NAME			241
#define RT_IO_HEADER		242
#define RT_IO_RINFO			243


static CD3DEngine*	g_engine = 0;
static char			g_gamerName[100];

extern void		SetEngine(CD3DEngine *engine);
extern void		SetGlobalGamerName(char *name);
extern BOOL		SearchKey(char *cmd, KeyRank &key);
extern void		PutKeyName(char* dst, char* src);
extern BOOL		GetResource(ResType type, int num, char* text);
extern BOOL		GetResourceBase(ResType type, int num, char* text);


#endif //_RESTEXT_H_
