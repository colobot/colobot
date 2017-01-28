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

#define RT_VERSION_ID			1
#define RT_WINDOW_MAXIMIZED		3
#define RT_WINDOW_MINIMIZED		4
#define RT_WINDOW_STANDARD		5
#define RT_WINDOW_CLOSE			6

#define RT_KEY_OR				20

#define RT_PLAY_LISTm			67
#define RT_PLAY_LISTd			68
#define RT_PLAY_LISTp			69
#define RT_PLAY_LISTu			70

#define RT_SETUP_DEVICE			80
#define RT_SETUP_MODE			81
#define RT_SETUP_KEY1			82
#define RT_SETUP_KEY2			83

#define RT_PERSO_LIST			90

#define RT_DIALOG_QUIT			102
#define RT_DIALOG_YES			103
#define RT_DIALOG_NO			104
#define RT_DIALOG_DELGAME		106
#define RT_DIALOG_YESDEL		107
#define RT_DIALOG_NODEL			108
#define RT_DIALOG_NEWGAME		109
#define RT_DIALOG_YESNEW		110
#define RT_DIALOG_NONEW			111
#define RT_DIALOG_YESQUIT		116
#define RT_DIALOG_NOQUIT		117
#define RT_DIALOG_DELFILE		118
#define RT_DIALOG_PLAY			119
#define RT_DIALOG_REPLAY		120
#define RT_DIALOG_QUITEDIT		121
#define RT_DIALOG_YESQUITEDIT	122
#define RT_DIALOG_NOQUITEDIT	123
#define RT_DIALOG_CANQUITEDIT	124
#define RT_DIALOG_NEWPUZZLE		125
#define RT_DIALOG_YESNEWP		126
#define RT_DIALOG_NONEWP		127
#define RT_DIALOG_DELPUZ		128
#define RT_DIALOG_DELPUZd		129
#define RT_DIALOG_DELPUZu		130
#define RT_DIALOG_YESDELPUZ		131
#define RT_DIALOG_NODELPUZ		132
#define RT_DIALOG_RESUMEINFO	133
#define RT_DIALOG_SIGNINFO		134
#define RT_DIALOG_AUTHORINFO	135
#define RT_DIALOG_YESINFO		136
#define RT_DIALOG_YESERROR		137
#define RT_DIALOG_EXPORTGAME	138
#define RT_DIALOG_YESEXPORT		139
#define RT_DIALOG_NOEXPORT		140
#define RT_DIALOG_IMPORTGAME	141
#define RT_DIALOG_YESIMPORT		142
#define RT_DIALOG_NOIMPORT		143
#define RT_DIALOG_TEST			144
#define RT_DIALOG_DEFI			145
#define RT_DIALOG_FILE			146
#define RT_DIALOG_RENAMEGAME	147
#define RT_DIALOG_YESRENAME		148
#define RT_DIALOG_NORENAME		149
#define RT_DIALOG_RENAMEOLD		150
#define RT_DIALOG_RENAMENEW		151

#define RT_GENERIC_DEV1			170
#define RT_GENERIC_DEV2			171
#define RT_GENERIC_EDIT1		172
#define RT_GENERIC_EDIT2		173
#define RT_GENERIC_DEMO1		174
#define RT_GENERIC_DEMO2		175
#define RT_GENERIC_DEMO3		176
#define RT_GENERIC_DEMO4		177
#define RT_GENERIC_DEMO5		178

#define RT_MESSAGE_WIN			200
#define RT_MESSAGE_LOST			201

#define RT_ACTION_GOTO			300
#define RT_ACTION_PUSH1			301
#define RT_ACTION_PUSH2			302
#define RT_ACTION_PUSHx			303
#define RT_ACTION_PUSHn			304
#define RT_ACTION_PULL1			305
#define RT_ACTION_ROLL			306
#define RT_ACTION_DOCKm			307
#define RT_ACTION_DOCKm2		308
#define RT_ACTION_DOCKmx		309
#define RT_ACTION_DOCKg			310
#define RT_ACTION_TRAXa			311
#define RT_ACTION_TRAXa2		312
#define RT_ACTION_TRAXax		313
#define RT_ACTION_TRAXl			314
#define RT_ACTION_TRAXl2		315
#define RT_ACTION_TRAXlx		316
#define RT_ACTION_TRAXr			317
#define RT_ACTION_TRAXr2		318
#define RT_ACTION_TRAXrx		319
#define RT_ACTION_GUNa			320
#define RT_ACTION_GUNa2			321
#define RT_ACTION_GUNax			322
#define RT_ACTION_GUNl			323
#define RT_ACTION_GUNl2			324
#define RT_ACTION_GUNlx			325
#define RT_ACTION_GUNr			326
#define RT_ACTION_GUNr2			327
#define RT_ACTION_GUNrx			328
#define RT_ACTION_CATAPULT		329

#define RT_ADVISE_LITGOTO		400
#define RT_ADVISE_GOTOPUSH		401
#define RT_ADVISE_NOSIGN		402

#define RT_USER_HEADER			410
#define RT_DEFI_HEADER			411
#define RT_IMPORT_HEADER		412

#define RT_DEMO					420

#define RT_UNIVERS0				500
#define RT_UNIVERS1				501
#define RT_UNIVERS2				502
#define RT_UNIVERS3				503
#define RT_UNIVERS4				504
#define RT_UNIVERS5				505
#define RT_UNIVERS6				506
#define RT_UNIVERS7				507
#define RT_UNIVERS8				508
#define RT_UNIVERS9				509
#define RT_UNIVERS10			510
#define RT_UNIVERS11			511
#define RT_UNIVERS12			512
#define RT_UNIVERS13			513
#define RT_UNIVERS14			514
#define RT_UNIVERS15			515
#define RT_UNIVERS16			516
#define RT_UNIVERS17			517
#define RT_UNIVERS18			518
#define RT_UNIVERS19			519


static CD3DEngine*	g_engine = 0;
static char			g_gamerName[100];

extern void		SetEngine(CD3DEngine *engine);
extern void		SetGlobalGamerName(char *name);
extern BOOL		SearchKey(char *cmd, KeyRank &key);
extern void		PutKeyName(char* dst, char* src);
extern BOOL		GetResource(ResType type, int num, char* text);
extern BOOL		GetResourceBase(ResType type, int num, char* text);


#endif //_RESTEXT_H_
