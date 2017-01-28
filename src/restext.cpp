// restext.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdio.h>
#include "struct.h"
#include "D3DEngine.h"
#include "language.h"
#include "misc.h"
#include "event.h"
#include "object.h"
#include "cbot\resource.h"
#include "restext.h"




// Donne le pointeur au moteur.

void SetEngine(CD3DEngine *engine)
{
	g_engine = engine;
}

// Donne le nom du joueur.

void SetGlobalGamerName(char *name)
{
	strcpy(g_gamerName, name);
}



typedef struct
{
	KeyRank		key;
	char		name[20];
}
KeyDesc;

static KeyDesc keyTable[15] =
{
	{ KEYRANK_LEFT,		"left;"    },
	{ KEYRANK_RIGHT,	"right;"   },
	{ KEYRANK_UP,		"up;"      },
	{ KEYRANK_DOWN,		"down;"    },
	{ KEYRANK_ROTCW,	"rotcw;"   },
	{ KEYRANK_ROTCCW,	"rotccw;"  },
	{ KEYRANK_STOP,		"stop;"    },
	{ KEYRANK_HELP,		"help;"    },
	{ KEYRANK_NEAR,		"near;"    },
	{ KEYRANK_AWAY,		"away;"    },
	{ KEYRANK_QUIT,		"quit;"    },
	{ KEYRANK_SPEED10,	"speed10;" },
	{ KEYRANK_SPEED15,	"speed15;" },
	{ KEYRANK_SPEED20,	"speed20;" },
	{ KEYRANK_SPEED30,	"speed30;" },
};

// Cherche une touche.

BOOL SearchKey(char *cmd, KeyRank &key)
{
	int		i;

	for ( i=0 ; i<15 ; i++ )
	{
		if ( strstr(cmd, keyTable[i].name) == cmd )
		{
			key = keyTable[i].key;
			return TRUE;
		}
	}
	return FALSE;
}

// Remplace les commandes "\key name;" dans un texte.

void PutKeyName(char* dst, char* src)
{
	KeyRank	key;
	char	name[500];
	int		s, d, n, res;

	s = d = 0;
	while ( src[s] != 0 )
	{
		if ( src[s+0] == '\\' &&
			 src[s+1] == 'k'  &&
			 src[s+2] == 'e'  &&
			 src[s+3] == 'y'  &&
			 src[s+4] == ' '  )
		{
			if ( SearchKey(src+s+5, key) )
			{
				res = g_engine->RetKey(key, 0);
				if ( res != 0 )
				{
					if ( GetResource(RES_KEY, res, name) )
					{
						n = 0;
						while ( name[n] != 0 )
						{
							dst[d++] = name[n++];
						}
						while ( src[s++] != ';' );
						continue;
					}
				}
			}
		}

		dst[d++] = src[s++];
	}
	dst[d++] = 0;
}


// Retourne le texte d'une ressource.

BOOL GetResource(ResType type, int num, char* text)
{
	char	buffer[500];

	if ( !GetResourceBase(type, num, buffer) )
	{
		text[0] = 0;
		return FALSE;
	}

	PutKeyName(text, buffer);
	return TRUE;
}


// Retourne le texte d'une ressource.

BOOL GetResourceBase(ResType type, int num, char* text)
{
	text[0] = 0;

#if _ENGLISH
	if ( type == RES_TEXT )
	{
		#if _EGAMES
		#if _FULL
		if ( num == RT_VERSION_ID          )  strcpy(text, "Full 1.3 /e");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "VW 1.3 /e");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /e");
		#endif
		#else
		#if _FULL
		#if _EDU
		if ( num == RT_VERSION_ID          )  strcpy(text, "EDU 1.3 /e");
		#else
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /e");
		#endif
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /e");
		#endif
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximize");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimize");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normal size");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Close");

		if ( num == RT_KEY_OR              )  strcpy(text, " or ");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Select a puzzle:");
		if ( num == RT_PLAY_LISTd          )  strcpy(text, " Challenges:");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " %s's workshop:");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototypes:");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Drivers:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolution:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) First click on the key you want to redefine.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Then push the key you want to use instead.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Player list:");

		#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Do you want to quit Speedy Eggbert Mania?");
		#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Do you want to quit BlupiMania?");
		#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Abort\\Abort the puzzle");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continue\\Continue the current puzzlr");
		#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quit\\Quit Speedy Eggbert Mania");
		#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quit\\Quit BlupiMania");
		#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continue\\Continue the game");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Do you want to delete %s's progression and workshop?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Delete the existing file %s?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Delete");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Player name:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Create");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_PLAY         )  strcpy(text, "Start puzzle...");
		if ( num == RT_DIALOG_REPLAY       )  strcpy(text, "Play again just for fun...");
		if ( num == RT_DIALOG_QUITEDIT     )  strcpy(text, "Save modifications?");
		if ( num == RT_DIALOG_YESQUITEDIT  )  strcpy(text, "Yes\\Save and quit");
		if ( num == RT_DIALOG_NOQUITEDIT   )  strcpy(text, "No\\Quit without saving");
		if ( num == RT_DIALOG_CANQUITEDIT  )  strcpy(text, "Cancel\\Continue workshop");
		if ( num == RT_DIALOG_NEWPUZZLE    )  strcpy(text, "Do you want to create a new puzzle \"%s\"?");
		if ( num == RT_DIALOG_YESNEWP      )  strcpy(text, "Create");
		if ( num == RT_DIALOG_NONEWP       )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_DELPUZ       )  strcpy(text, "Do you want to delete the puzzle?");
		if ( num == RT_DIALOG_DELPUZd      )  strcpy(text, "(for all players)");
		if ( num == RT_DIALOG_DELPUZu      )  strcpy(text, "(only for your workshop)");
		if ( num == RT_DIALOG_YESDELPUZ    )  strcpy(text, "Delete");
		if ( num == RT_DIALOG_NODELPUZ     )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_RESUMEINFO   )  strcpy(text, "Instructions");
		if ( num == RT_DIALOG_AUTHORINFO   )  strcpy(text, "Author");
		if ( num == RT_DIALOG_SIGNINFO     )  strcpy(text, "Hints");
		if ( num == RT_DIALOG_YESINFO      )  strcpy(text, "OK");
		if ( num == RT_DIALOG_YESERROR     )  strcpy(text, "OK");
		if ( num == RT_DIALOG_EXPORTGAME   )  strcpy(text, "Where do you want do export the puzzle?");
		if ( num == RT_DIALOG_YESEXPORT    )  strcpy(text, "Export");
		if ( num == RT_DIALOG_NOEXPORT     )  strcpy(text, "Cancel");
		#if _EGAMES
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Puzzles in \"My documents/SpeedyEggbertMania\":");
		#else
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Puzzles in \"My documents/Blupimania2\":");
		#endif
		if ( num == RT_DIALOG_YESIMPORT    )  strcpy(text, "Import");
		if ( num == RT_DIALOG_NOIMPORT     )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_TEST         )  strcpy(text, "Test");
		if ( num == RT_DIALOG_DEFI         )  strcpy(text, "Take up the challenge...");
		if ( num == RT_DIALOG_FILE         )  strcpy(text, "File");
		if ( num == RT_DIALOG_RENAMEGAME   )  strcpy(text, "Rename puzzle file.");
		if ( num == RT_DIALOG_YESRENAME    )  strcpy(text, "Rename");
		if ( num == RT_DIALOG_NORENAME     )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_RENAMEOLD    )  strcpy(text, "Current name");
		if ( num == RT_DIALOG_RENAMENEW    )  strcpy(text, "New name");

		#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.egames.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "www.blupi.com");
		#if _SE
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "You have just played with the V.W. version of Speedy Eggbert Mania.");
		#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "You have just played with the DEMO version of Speedy Eggbert Mania.");
		#endif
		#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.epsitec.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Developed by EPSITEC");
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "You have just played with the DEMO version of Blupimania II.");
		#endif
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "The full version is now available ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... and contains 80 puzzles and 40 challenges...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... as well a built-in editor where you can create your own puzzles.");
		#if _EGAMES
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "More details on www.egames.com!");
		#else
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "More details on www.epsitec.com!");
		#endif

		if ( num == RT_ACTION_GOTO         )  strcpy(text, "Go to");
		if ( num == RT_ACTION_PUSH1        )  strcpy(text, "Push once");
		if ( num == RT_ACTION_PUSH2        )  strcpy(text, "Push twice");
		if ( num == RT_ACTION_PUSHx        )  strcpy(text, "Push %d times");
		if ( num == RT_ACTION_PUSHn        )  strcpy(text, "Push more");
		if ( num == RT_ACTION_PULL1        )  strcpy(text, "Pull once");
		if ( num == RT_ACTION_ROLL         )  strcpy(text, "Roll");
		if ( num == RT_ACTION_DOCKm        )  strcpy(text, "Activate");
		if ( num == RT_ACTION_DOCKm2       )  strcpy(text, "Activate twice");
		if ( num == RT_ACTION_DOCKmx       )  strcpy(text, "Activate %d times");
		if ( num == RT_ACTION_DOCKg        )  strcpy(text, "Grab/drop");
		if ( num == RT_ACTION_TRAXa        )  strcpy(text, "Move forward");
		if ( num == RT_ACTION_TRAXa2       )  strcpy(text, "Move forward twice");
		if ( num == RT_ACTION_TRAXax       )  strcpy(text, "Move forward %d times");
		if ( num == RT_ACTION_TRAXl        )  strcpy(text, "Turn right");
		if ( num == RT_ACTION_TRAXl2       )  strcpy(text, "Turn right twice");
		if ( num == RT_ACTION_TRAXlx       )  strcpy(text, "Turn right %d times");
		if ( num == RT_ACTION_TRAXr        )  strcpy(text, "Turn left");
		if ( num == RT_ACTION_TRAXr2       )  strcpy(text, "Turn left twice");
		if ( num == RT_ACTION_TRAXrx       )  strcpy(text, "Turn left %d times");
		if ( num == RT_ACTION_GUNa         )  strcpy(text, "Move forward then pull");
		if ( num == RT_ACTION_GUNa2        )  strcpy(text, "Move forward then pull twice");
		if ( num == RT_ACTION_GUNax        )  strcpy(text, "Move forward then pull %d times");
		if ( num == RT_ACTION_GUNl         )  strcpy(text, "Turn left then pull");
		if ( num == RT_ACTION_GUNl2        )  strcpy(text, "Turn left then pull twice");
		if ( num == RT_ACTION_GUNlx        )  strcpy(text, "Turn left then pull %d times");
		if ( num == RT_ACTION_GUNr         )  strcpy(text, "Turn right then pull");
		if ( num == RT_ACTION_GUNr2        )  strcpy(text, "Turn right then pull twice");
		if ( num == RT_ACTION_GUNrx        )  strcpy(text, "Turn right then pull %d times");
		if ( num == RT_ACTION_CATAPULT     )  strcpy(text, "Throw");

		if ( num == RT_ADVISE_LITGOTO      )  strcpy(text, "...");
		if ( num == RT_ADVISE_GOTOPUSH     )  strcpy(text, "...");
		if ( num == RT_ADVISE_NOSIGN       )  strcpy(text, "No hints available for this puzzle.");

		if ( num == RT_USER_HEADER         )  strcpy(text, "File\tUniverse\tTested\tInstructions");
		if ( num == RT_DEFI_HEADER         )  strcpy(text, "File\tUniverse\tAuthor\tInstructions");
		if ( num == RT_IMPORT_HEADER       )  strcpy(text, "File\tUniverse\tAuthor\tInstructions");

		#if _EGAMES
		if ( num == RT_DEMO                )  strcpy(text, "Not available in this version");
		#else
		if ( num == RT_DEMO                )  strcpy(text, "Not available in the DEMO");
		#endif

		if ( num == RT_UNIVERS0            )  strcpy(text, "Castella");
		if ( num == RT_UNIVERS1            )  strcpy(text, "Paradiso");
		if ( num == RT_UNIVERS2            )  strcpy(text, "Volcano");
		if ( num == RT_UNIVERS3            )  strcpy(text, "Mechano");
		if ( num == RT_UNIVERS4            )  strcpy(text, "Sahara");
		if ( num == RT_UNIVERS5            )  strcpy(text, "Glacia");
		if ( num == RT_UNIVERS6            )  strcpy(text, "Samba");
		if ( num == RT_UNIVERS7            )  strcpy(text, "Medieva");
		if ( num == RT_UNIVERS8            )  strcpy(text, "Vegeta");
		if ( num == RT_UNIVERS9            )  strcpy(text, "Inferno");
		if ( num == RT_UNIVERS10           )  strcpy(text, "Futura");
		if ( num == RT_UNIVERS11           )  strcpy(text, "Alibaba");
		if ( num == RT_UNIVERS12           )  strcpy(text, "Studio");
		if ( num == RT_UNIVERS13           )  strcpy(text, "Cata");
		if ( num == RT_UNIVERS14           )  strcpy(text, "Allegro");
		if ( num == RT_UNIVERS15           )  strcpy(text, "Concreto");
		if ( num == RT_UNIVERS16           )  strcpy(text, "Labo");
		if ( num == RT_UNIVERS17           )  strcpy(text, "Mystica");
		if ( num == RT_UNIVERS18           )  strcpy(text, "Fantasia");
		if ( num == RT_UNIVERS19           )  strcpy(text, "Natura");
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancel");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancel");

		if ( num == EVENT_INTERFACE_PUZZLE )  strcpy(text, "Puzzles\\Progressive puzzles");
		if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Challenges\\Other player's puzzles");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Workshop\\Create your own puzzles");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypes under development");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Change player\\Change the current player");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Options\\Options");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Restart\\Restart the puzzle from the beginning");
		#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Return to Speedy Eggbert Mania");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quit\\Quit Speedy Eggbert Mania");
		#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Return to BlupiMania");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quit\\Quit BlupiMania");
		#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Cancel\\Back to previous screen");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Return to the main menu");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Play\\Start the selected puzzle");
		if ( num == EVENT_INTERFACE_NEW    )  strcpy(text, "New\\Create a new puzzle");
		if ( num == EVENT_INTERFACE_DELETE )  strcpy(text, "Delete\\Delete the puzzle");
		if ( num == EVENT_INTERFACE_RENAME )  strcpy(text, "Rename\\Rename the puzzle");
		if ( num == EVENT_INTERFACE_EXPORT )  strcpy(text, "Export\\Export the puzzle");
		if ( num == EVENT_INTERFACE_IMPORT )  strcpy(text, "Import\\Import the puzzle");
		if ( num == EVENT_INTERFACE_EDIT   )  strcpy(text, "Modify\\Modify th puzzle");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Back to previous screen");

		if ( num == EVENT_INTERFACE_EXPORTdefi )  strcpy(text, "In the challanges (for all players)");
		#if _EGAMES
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "In \"My documents/SpeedyEggbertMania\" folder");
		#else
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "In the \"My documents/Blupimania2\" folder");
		#endif

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Device\\Driver and resolution settings");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Graphics\\Graphics settings");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Game\\Game settings");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controls\\Keyboard, wheel and gamepad settings");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sound\\Volume of music and sound effects");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolution");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Full screen\\Full screen or windowed mode");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Apply changes\\Activates the changed settings");

		if ( num == EVENT_INTERFACE_SHADOW      )  strcpy(text, "Shadows\\Display shadows on the ground");
		if ( num == EVENT_INTERFACE_DIRTY       )  strcpy(text, "Dirt\\Display dirt on objects");
		if ( num == EVENT_INTERFACE_SUNBEAM     )  strcpy(text, "Sunbeams\\Display sun beams above water");
		if ( num == EVENT_INTERFACE_LENSFLARE   )  strcpy(text, "Lens flare\\Lens flare of the virtual camera");
		if ( num == EVENT_INTERFACE_DECOR       )  strcpy(text, "Decorations\\Trees, plants, animals, ducts, and other stuff.");
		if ( num == EVENT_INTERFACE_DETAIL      )  strcpy(text, "Detailed objects\\Displays objects with much more details");
		if ( num == EVENT_INTERFACE_METEO       )  strcpy(text, "Weather\\Rain and snow");
		if ( num == EVENT_INTERFACE_AMBIANCE    )  strcpy(text, "Effects on water\\");
		if ( num == EVENT_INTERFACE_EXPLOVIB    )  strcpy(text, "Screen tilting during explosions\\");
		if ( num == EVENT_INTERFACE_SPEEDSCH    )  strcpy(text, "Horizontal scroll speed\\");
		if ( num == EVENT_INTERFACE_SPEEDSCV    )  strcpy(text, "Vertical scroll speed\\");
		if ( num == EVENT_INTERFACE_MOUSESCROLL )  strcpy(text, "Mouse scrolling\\When the mouse pointer bumps against the edge of the screen.");
		if ( num == EVENT_INTERFACE_INVSCH      )  strcpy(text, "Horizontal scrolling inverted\\When the mouse pointer bumps into the edges");
		if ( num == EVENT_INTERFACE_INVSCV      )  strcpy(text, "Vertical scrolling inverted\\When the mouse pointer bumps into the edges");
		if ( num == EVENT_INTERFACE_MOVIE       )  strcpy(text, "Film sequences\\Show non interactive film  sequences");
		if ( num == EVENT_INTERFACE_HELP        )  strcpy(text, "Show hints\\Show hints after some time");
		if ( num == EVENT_INTERFACE_TOOLTIPS    )  strcpy(text, "Tooltips\\Show tooltips when the mouse pointer stops over controls");
		if ( num == EVENT_INTERFACE_NICEMOUSE   )  strcpy(text, "Mouse pointer with shadow\\");
		if ( num == EVENT_INTERFACE_ACCEL       )  strcpy(text, "Accelerate when puzzle is long to solve\\Blupi runs faster after 10 minutes in the same puzzle");
		if ( num == EVENT_INTERFACE_VOLBLUPI    )  strcpy(text, "Blupi's voices\\");
		if ( num == EVENT_INTERFACE_VOLSOUND    )  strcpy(text, "Sound effects\\");
		if ( num == EVENT_INTERFACE_VOLAMBIANCE )  strcpy(text, "Ambiant sound\\");
		if ( num == EVENT_INTERFACE_SOUND3D     )  strcpy(text, "3D sound\\3D sound");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Default controls\\Resets all controls to default values");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Scroll left\\Scroll left when mouse pointer bumps against edge");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Scroll right\\Scroll right when mouse pointer bumps against edge");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Scroll up\\Scroll up when mouse pointer bumps against edge");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Scroll down\\Scroll down when mouse pointer bumps against edge");
		if ( num == EVENT_INTERFACE_KROTCW )  strcpy(text, "Rotate CW\\Rotate camera clockwise");
		if ( num == EVENT_INTERFACE_KROTCCW)  strcpy(text, "Rotate CCW\\Rotate camera counter clockwise");
		if ( num == EVENT_INTERFACE_KSTOP  )  strcpy(text, "Stop action\\Stop Blupi's current action");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Quit\\Quit the current puzzle");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instructions\\Show instructions for the current puzzle");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Lowest\\Minimum graphic quality (highest frame rate)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Normal graphic quality");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Highest\\Highest graphic quality (lowest frame rate)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silent\\No sound");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Normal sound volume");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\New player name");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Select\\Choose the selected player");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "New\\Create a new player");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Delete\\Delete the selected player from the list");

		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Previous screen");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Next screen");
		#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit Speedy Eggbert Mania");
		#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit BlupiMania");
		#endif

		if ( num == EVENT_INTERFACE_UNDO   )  strcpy(text, "Undo last action");
		if ( num == EVENT_INTERFACE_STOP   )  strcpy(text, "Stop (\\key stop;)");
		if ( num == EVENT_INTERFACE_SIGN   )  strcpy(text, "Get hints");
		if ( num == EVENT_LABEL_SIGN       )  strcpy(text, "Hints");

		if ( num == EVENT_EDIT_WATER       )  strcpy(text, "Water");
		if ( num == EVENT_EDIT_GROUND      )  strcpy(text, "Ground");
		if ( num == EVENT_EDIT_OBJECT      )  strcpy(text, "Objects");
		if ( num == EVENT_EDIT_INFO        )  strcpy(text, "Informations about the puzzle");
		if ( num == EVENT_EDIT_CLEAR       )  strcpy(text, "Delete");

		if ( num == EVENT_CMD              )  strcpy(text, "Command line");
		if ( num == EVENT_SPEED            )  strcpy(text, "Game speed");
	}

	if ( type == RES_OBJECT )
	{
		if ( num == OBJECT_BLUPI           )  strcpy(text, "Blupi");
		if ( num == OBJECT_BOX1            )  strcpy(text, "Crate");
		if ( num == OBJECT_BOX2            )  strcpy(text, "Crate");
		if ( num == OBJECT_BOX3            )  strcpy(text, "Crate");
		if ( num == OBJECT_BOX4            )  strcpy(text, "Crate");
		if ( num == OBJECT_BOX5            )  strcpy(text, "Crate");
		if ( num == OBJECT_BOX6            )  strcpy(text, "Crate");
		if ( num == OBJECT_BOX7            )  strcpy(text, "Big bowl");
		if ( num == OBJECT_BOX8            )  strcpy(text, "Heavy column");
		if ( num == OBJECT_BOX9            )  strcpy(text, "Cage");
		if ( num == OBJECT_BOX10           )  strcpy(text, "Bomb");
		if ( num == OBJECT_BOX11           )  strcpy(text, "Crate with wheels");
		if ( num == OBJECT_BOX12           )  strcpy(text, "Crate with wheels");
		if ( num == OBJECT_BOX13           )  strcpy(text, "Crate on air cushion");
		if ( num == OBJECT_KEY1            )  strcpy(text, "Orange key");
		if ( num == OBJECT_KEY2            )  strcpy(text, "Blue jey");
		if ( num == OBJECT_DOOR1           )  strcpy(text, "Orange door");
		if ( num == OBJECT_DOOR2           )  strcpy(text, "Blue door");
		if ( num == OBJECT_DOOR3           )  strcpy(text, "Orange door");
		if ( num == OBJECT_DOOR4           )  strcpy(text, "Blue door");
		if ( num == OBJECT_DOCK            )  strcpy(text, "Crane");
		if ( num == OBJECT_CATAPULT        )  strcpy(text, "Catapult");
		if ( num == OBJECT_TRAX            )  strcpy(text, "Pusher");
		if ( num == OBJECT_PERFO           )  strcpy(text, "Perforator");
		if ( num == OBJECT_GUN             )  strcpy(text, "Crate blaster");
		if ( num == OBJECT_MINE            )  strcpy(text, "Mine");
		if ( num == OBJECT_GLASS1          )  strcpy(text, "Reinforced pane");
		if ( num == OBJECT_GLASS2          )  strcpy(text, "Reinforced pane");
		if ( num == OBJECT_FIOLE           )  strcpy(text, "Magic potion");
		if ( num == OBJECT_GLU             )  strcpy(text, "Glue potion");
		if ( num == OBJECT_GOAL            )  strcpy(text, "Goal");
		if ( num == OBJECT_CRAZY           )  strcpy(text, "Crazy bot");
		if ( num == OBJECT_BOT1            )  strcpy(text, "Bot 1");
		if ( num == OBJECT_BOT2            )  strcpy(text, "Bot 2");
		if ( num == OBJECT_BOT3            )  strcpy(text, "Bot 3");
		if ( num == OBJECT_BOT4            )  strcpy(text, "Bot 4");
		if ( num == OBJECT_BOT5            )  strcpy(text, "Bot 5");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Error");
		if ( num == ERR_CMD             )  strcpy(text, "Unknown command");
		#if _EGAMES
		if ( num == ERR_INSTALL         )  strcpy(text, "Speedy Eggbert Mania has not been installed correctly.");
		if ( num == ERR_NOCD            )  strcpy(text, "Please insert the Speedy Eggbert Mania CD\nand re-run the game.");
		#else
		if ( num == ERR_INSTALL         )  strcpy(text, "BlupiMania has not been installed correctly.");
		if ( num == ERR_NOCD            )  strcpy(text, "Please insert the BlupiMania CD\nand re-run the game.");
		#endif
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Error in instruction move");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Goto: destination inaccessible");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Goto: destination inaccessible");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Goto: destination occupied");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Bot inappropriate");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Not enough energy");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "The puzzle has not yet been finished (push \\key help; for more details)");
		if ( num == ERR_PUZZLE_ZEROBLUPI)  strcpy(text, "There must be at least one Blupi.");
		if ( num == ERR_PUZZLE_ZEROGOAL )  strcpy(text, "There must be at leat one balloon.");
		if ( num == ERR_PUZZLE_LOGOAL   )  strcpy(text, "There are not enough balloons.");
		if ( num == ERR_PUZZLE_HIGOAL   )  strcpy(text, "There are too many balloons.");
		if ( num == ERR_PUZZLE_MAXBLUPI )  strcpy(text, "You can have at most 4 Blupis or robots per puzzle.");
		if ( num == ERR_PUZZLE_MAXGOAL  )  strcpy(text, "You can have at most 8 ballons per puzzle.");
		if ( num == ERR_PUZZLE_MAX      )  strcpy(text, "You cannot create any more objects of this type.");
		if ( num == ERR_CREATE          )  strcpy(text, "File already exist.");

		if ( num == INFO_WIN            )  strcpy(text, "<< Well done, mission accomplished >>");
		if ( num == INFO_LOST           )  strcpy(text, "<< Sorry, you have failed >>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<< Sorry, mission failed >>");
	}

	if ( type == RES_CBOT )
	{
		strcpy(text, "Error");
		if ( num == TX_OPENPAR       ) strcpy(text, "Opening bracket missing");
		if ( num == TX_CLOSEPAR      ) strcpy(text, "Closing bracket missing ");
		if ( num == TX_NOTBOOL       ) strcpy(text, "The expression must return a boolean value");
		if ( num == TX_UNDEFVAR      ) strcpy(text, "Variable not declared");
		if ( num == TX_BADLEFT       ) strcpy(text, "Assignment impossible");
		if ( num == TX_ENDOF         ) strcpy(text, "Semicolon terminator missing");
		if ( num == TX_OUTCASE       ) strcpy(text, "Instruction ""case"" outside a block ""switch""");
		if ( num == TX_NOTERM        ) strcpy(text, "Instructions after the final closing brace");
		if ( num == TX_CLOSEBLK      ) strcpy(text, "End of block missing");
		if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instruction ""else"" without corresponding ""if"" ");
		if ( num == TX_OPENBLK       ) strcpy(text, "Opening brace missing ");//début d'un bloc attendu?
		if ( num == TX_BADTYPE       ) strcpy(text, "Wrong type for the assignment");
		if ( num == TX_REDEFVAR      ) strcpy(text, "A variable can not be declared twice");
		if ( num == TX_BAD2TYPE      ) strcpy(text, "The types of the two operands are incompatible ");
		if ( num == TX_UNDEFCALL     ) strcpy(text, "Unknown function");
		if ( num == TX_MISDOTS       ) strcpy(text, "Sign "" : "" missing");
		if ( num == TX_WHILE         ) strcpy(text, "Keyword ""while"" missing");
		if ( num == TX_BREAK         ) strcpy(text, "Instruction ""break"" outside a loop");
		if ( num == TX_LABEL         ) strcpy(text, "A label must be followed by ""for"", ""while"", ""do"" or ""switch""");
		if ( num == TX_NOLABEL       ) strcpy(text, "This label does not exist");// Cette étiquette n'existe pas
		if ( num == TX_NOCASE        ) strcpy(text, "Instruction ""case"" missing");
		if ( num == TX_BADNUM        ) strcpy(text, "Number missing");
		if ( num == TX_VOID          ) strcpy(text, "Void parameter");
		if ( num == TX_NOTYP         ) strcpy(text, "Type declaration missing");
		if ( num == TX_NOVAR         ) strcpy(text, "Variable name missing");
		if ( num == TX_NOFONC        ) strcpy(text, "Function name missing");
		if ( num == TX_OVERPARAM     ) strcpy(text, "Too many parameters");
		if ( num == TX_REDEF         ) strcpy(text, "Function already exists");
		if ( num == TX_LOWPARAM      ) strcpy(text, "Parameters missing ");
		if ( num == TX_BADPARAM      ) strcpy(text, "No function of this name accepts this kind of parameter");
		if ( num == TX_NUMPARAM      ) strcpy(text, "No function of this name accepts this number of parameters");
		if ( num == TX_NOITEM        ) strcpy(text, "This is not a member of this class");
		if ( num == TX_DOT           ) strcpy(text, "This object is not a member of a class");
		if ( num == TX_NOCONST       ) strcpy(text, "Appropriate constructor missing");
		if ( num == TX_REDEFCLASS    ) strcpy(text, "This class already exists");
		if ( num == TX_CLBRK         ) strcpy(text, """ ] "" missing");
		if ( num == TX_RESERVED      ) strcpy(text, "Reserved keyword of CBOT language");
		if ( num == TX_BADNEW        ) strcpy(text, "Bad argument for ""new""");
		if ( num == TX_OPBRK         ) strcpy(text, """ [ "" expected");
		if ( num == TX_BADSTRING     ) strcpy(text, "String missing");
		if ( num == TX_BADINDEX      ) strcpy(text, "Incorrect index type");
		if ( num == TX_PRIVATE       ) strcpy(text, "Private element");
		if ( num == TX_NOPUBLIC      ) strcpy(text, "Public required");
		if ( num == TX_DIVZERO       ) strcpy(text, "Dividing through zero");
		if ( num == TX_NOTINIT       ) strcpy(text, "Variable not initialized");
		if ( num == TX_BADTHROW      ) strcpy(text, "Negative value rejected by ""throw""");//C'est quoi, ça?
		if ( num == TX_NORETVAL      ) strcpy(text, "The function returned no value ");
		if ( num == TX_NORUN         ) strcpy(text, "No function running");
		if ( num == TX_NOCALL        ) strcpy(text, "Calling an unknown function");
		if ( num == TX_NOCLASS       ) strcpy(text, "This class does not exist");
		if ( num == TX_NULLPT        ) strcpy(text, "Unknown Object");
		if ( num == TX_OPNAN         ) strcpy(text, "Operation impossible with value ""nan""");
		if ( num == TX_OUTARRAY      ) strcpy(text, "Access beyond array limit");
		if ( num == TX_STACKOVER     ) strcpy(text, "Stack overflow");
		if ( num == TX_DELETEDPT     ) strcpy(text, "Illegal object");
		if ( num == TX_FILEOPEN      ) strcpy(text, "Can't open file");
		if ( num == TX_NOTOPEN       ) strcpy(text, "File not open");
		if ( num == TX_ERRREAD       ) strcpy(text, "Read error");
		if ( num == TX_ERRWRITE      ) strcpy(text, "Write error");
	}

	if ( type == RES_KEY )
	{
		if ( num == 0                   )  strcpy(text, "< none >");
		if ( num == VK_LEFT             )  strcpy(text, "Left arrow");
		if ( num == VK_RIGHT            )  strcpy(text, "Right arrow");
		if ( num == VK_UP               )  strcpy(text, "Arrow up");
		if ( num == VK_DOWN             )  strcpy(text, "Arrow down");
		if ( num == VK_CANCEL           )  strcpy(text, "Control-break");
		if ( num == VK_BACK             )  strcpy(text, "<--");
		if ( num == VK_TAB              )  strcpy(text, "Tab");
		if ( num == VK_CLEAR            )  strcpy(text, "Clear");
		if ( num == VK_RETURN           )  strcpy(text, "Enter");
		if ( num == VK_SHIFT            )  strcpy(text, "Shift");
		if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
		if ( num == VK_MENU             )  strcpy(text, "Alt");
		if ( num == VK_PAUSE            )  strcpy(text, "Pause");
		if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
		if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
		if ( num == VK_SPACE            )  strcpy(text, "Space bar");
		if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
		if ( num == VK_NEXT             )  strcpy(text, "Page Down");
		if ( num == VK_END              )  strcpy(text, "End");
		if ( num == VK_HOME             )  strcpy(text, "Home");
		if ( num == VK_SELECT           )  strcpy(text, "Select");
		if ( num == VK_EXECUTE          )  strcpy(text, "Execute");
		if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
		if ( num == VK_INSERT           )  strcpy(text, "Insert");
		if ( num == VK_DELETE           )  strcpy(text, "Delete");
		if ( num == VK_HELP             )  strcpy(text, "Help");
		if ( num == VK_LWIN             )  strcpy(text, "Left Windows");
		if ( num == VK_RWIN             )  strcpy(text, "Right Windows");
		if ( num == VK_APPS             )  strcpy(text, "Application key");
		if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
		if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
		if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
		if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
		if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
		if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
		if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
		if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
		if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
		if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
		if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
		if ( num == VK_ADD              )  strcpy(text, "NumPad +");
		if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
		if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
		if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
		if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
		if ( num == VK_F1               )  strcpy(text, "F1");
		if ( num == VK_F2               )  strcpy(text, "F2");
		if ( num == VK_F3               )  strcpy(text, "F3");
		if ( num == VK_F4               )  strcpy(text, "F4");
		if ( num == VK_F5               )  strcpy(text, "F5");
		if ( num == VK_F6               )  strcpy(text, "F6");
		if ( num == VK_F7               )  strcpy(text, "F7");
		if ( num == VK_F8               )  strcpy(text, "F8");
		if ( num == VK_F9               )  strcpy(text, "F9");
		if ( num == VK_F10              )  strcpy(text, "F10");
		if ( num == VK_F11              )  strcpy(text, "F11");
		if ( num == VK_F12              )  strcpy(text, "F12");
		if ( num == VK_F13              )  strcpy(text, "F13");
		if ( num == VK_F14              )  strcpy(text, "F14");
		if ( num == VK_F15              )  strcpy(text, "F15");
		if ( num == VK_F16              )  strcpy(text, "F16");
		if ( num == VK_F17              )  strcpy(text, "F17");
		if ( num == VK_F18              )  strcpy(text, "F18");
		if ( num == VK_F19              )  strcpy(text, "F19");
		if ( num == VK_F20              )  strcpy(text, "F20");
		if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
		if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
		if ( num == VK_ATTN             )  strcpy(text, "Attn");
		if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
		if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
		if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
		if ( num == VK_PLAY             )  strcpy(text, "Play");
		if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
		if ( num == VK_PA1              )  strcpy(text, "PA1");
		if ( num == VK_OEM_CLEAR        )  strcpy(text, "Clear");
		if ( num == VK_BUTTON1          )  strcpy(text, "Button 1");
		if ( num == VK_BUTTON2          )  strcpy(text, "Button 2");
		if ( num == VK_BUTTON3          )  strcpy(text, "Button 3");
		if ( num == VK_BUTTON4          )  strcpy(text, "Button 4");
		if ( num == VK_BUTTON5          )  strcpy(text, "Button 5");
		if ( num == VK_BUTTON6          )  strcpy(text, "Button 6");
		if ( num == VK_BUTTON7          )  strcpy(text, "Button 7");
		if ( num == VK_BUTTON8          )  strcpy(text, "Button 8");
		if ( num == VK_BUTTON9          )  strcpy(text, "Button 9");
		if ( num == VK_BUTTON10         )  strcpy(text, "Button 10");
		if ( num == VK_BUTTON11         )  strcpy(text, "Button 11");
		if ( num == VK_BUTTON12         )  strcpy(text, "Button 12");
		if ( num == VK_BUTTON13         )  strcpy(text, "Button 13");
		if ( num == VK_BUTTON14         )  strcpy(text, "Button 14");
		if ( num == VK_BUTTON15         )  strcpy(text, "Button 15");
		if ( num == VK_BUTTON16         )  strcpy(text, "Button 16");
		if ( num == VK_BUTTON17         )  strcpy(text, "Button 17");
		if ( num == VK_BUTTON18         )  strcpy(text, "Button 18");
		if ( num == VK_BUTTON19         )  strcpy(text, "Button 19");
		if ( num == VK_BUTTON20         )  strcpy(text, "Button 20");
		if ( num == VK_BUTTON21         )  strcpy(text, "Button 21");
		if ( num == VK_BUTTON22         )  strcpy(text, "Button 22");
		if ( num == VK_BUTTON23         )  strcpy(text, "Button 23");
		if ( num == VK_BUTTON24         )  strcpy(text, "Button 24");
		if ( num == VK_BUTTON25         )  strcpy(text, "Button 25");
		if ( num == VK_BUTTON26         )  strcpy(text, "Button 26");
		if ( num == VK_BUTTON27         )  strcpy(text, "Button 27");
		if ( num == VK_BUTTON28         )  strcpy(text, "Button 28");
		if ( num == VK_BUTTON29         )  strcpy(text, "Button 29");
		if ( num == VK_BUTTON30         )  strcpy(text, "Button 30");
		if ( num == VK_BUTTON31         )  strcpy(text, "Button 31");
		if ( num == VK_BUTTON32         )  strcpy(text, "Button 32");
		if ( num == VK_WHEELUP          )  strcpy(text, "Wheel up");
		if ( num == VK_WHEELDOWN        )  strcpy(text, "Wheel down");
	}
#endif

#if _DEUTSCH
	if ( type == RES_TEXT )
	{
		#if _FULL
		#if _EDU
		if ( num == RT_VERSION_ID          )  strcpy(text, "EDU 1.3 /d");
		#else
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /d");
		#endif
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /d");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximale Größe");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Reduzierte Größe");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normale Größe");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Schließen");

		if ( num == RT_KEY_OR              )  strcpy(text, " oder ");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Wählen Sie ein Rätsel :");
		if ( num == RT_PLAY_LISTd          )  strcpy(text, " Défis :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " %s's Werkstatt :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototypen :");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Display-Driver :");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Auflösung :");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Klicken Sie auf den neu zu belegenden Befehl.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Drücken Sie auf die Taste, die benutzt werden soll.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Spielerliste :");

		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Wollen Sie BlupiMania verlassen ?");
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Aufgeben\\Rätsel aufgeben");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Weiterspielen\\Rätsel weiterspielen");
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Beenden\\BlupiMania beenden");
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Weiter spielen\\Weiter spielen");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Wollen Sie den Spielstand von %s löschen ?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Wollen Sie die Datei %s löschen ?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Löschen");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Name des neuen Spielers :");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "OK");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_PLAY         )  strcpy(text, "Rätsel lösen...");
		if ( num == RT_DIALOG_REPLAY       )  strcpy(text, "Rätsel nochmal spielen...");
		if ( num == RT_DIALOG_QUITEDIT     )  strcpy(text, "Änderungen speichern ?");
		if ( num == RT_DIALOG_YESQUITEDIT  )  strcpy(text, "Ja\\Speichern und beenden");
		if ( num == RT_DIALOG_NOQUITEDIT   )  strcpy(text, "Nein\\Beenden ohne zu speichern");
		if ( num == RT_DIALOG_CANQUITEDIT  )  strcpy(text, "Abbrechen\\Rätsel weiterbearbeiten");
		if ( num == RT_DIALOG_NEWPUZZLE    )  strcpy(text, "Wollen Sie ein neues Rätsel vom Typ \"%s\" erstellen ?");
		if ( num == RT_DIALOG_YESNEWP      )  strcpy(text, "Neu erstellen");
		if ( num == RT_DIALOG_NONEWP       )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_DELPUZ       )  strcpy(text, "Wollen Sie das Rätsel löschen ?");
		if ( num == RT_DIALOG_DELPUZd      )  strcpy(text, "(für alle Spieler)");
		if ( num == RT_DIALOG_DELPUZu      )  strcpy(text, "(nur in Ihrer Werkstatt)");
		if ( num == RT_DIALOG_YESDELPUZ    )  strcpy(text, "Löschen");
		if ( num == RT_DIALOG_NODELPUZ     )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_RESUMEINFO   )  strcpy(text, "Anweisungen");
		if ( num == RT_DIALOG_AUTHORINFO   )  strcpy(text, "Autor");
		if ( num == RT_DIALOG_SIGNINFO     )  strcpy(text, "Hinweise");
		if ( num == RT_DIALOG_YESINFO      )  strcpy(text, "OK");
		if ( num == RT_DIALOG_YESERROR     )  strcpy(text, "OK");
		if ( num == RT_DIALOG_EXPORTGAME   )  strcpy(text, "Wohin wollen Sie das Rätsel exportieren ?");
		if ( num == RT_DIALOG_YESEXPORT    )  strcpy(text, "Exportieren");
		if ( num == RT_DIALOG_NOEXPORT     )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Rätsel im Ordner \"Eigene Dateien\\Blupimania2\" :");
		if ( num == RT_DIALOG_YESIMPORT    )  strcpy(text, "Importieren");
		if ( num == RT_DIALOG_NOIMPORT     )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_TEST         )  strcpy(text, "Testen");
		if ( num == RT_DIALOG_DEFI         )  strcpy(text, "Challenge annehmen...");
		if ( num == RT_DIALOG_FILE         )  strcpy(text, "Datei");
		if ( num == RT_DIALOG_RENAMEGAME   )  strcpy(text, "Umbenennung der Rätseldatei.");
		if ( num == RT_DIALOG_YESRENAME    )  strcpy(text, "Umbenennen");
		if ( num == RT_DIALOG_NORENAME     )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_RENAMEOLD    )  strcpy(text, "Aktueller Name");
		if ( num == RT_DIALOG_RENAMENEW    )  strcpy(text, "Neuer Name");

		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.epsitec.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Entwickelt von EPSITEC");
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Sie haben mit der DEMO-Version von BlupiMania II gespielt.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "Die Vollversion ist ab sofort erhältlich ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... sie enthält 80 Rätsel und 40 Challenges ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... sowie eine Werkstatt um Ihre eigenen Rätsel zu erstellen.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Mehr Details auf www.epsitec.com !");

		if ( num == RT_ACTION_GOTO         )  strcpy(text, "Dort hin gehen");
		if ( num == RT_ACTION_PUSH1        )  strcpy(text, "Einmal schieben");
		if ( num == RT_ACTION_PUSHx        )  strcpy(text, "%d mal schieben");
		if ( num == RT_ACTION_PUSHn        )  strcpy(text, "Immer schieben");
		if ( num == RT_ACTION_PULL1        )  strcpy(text, "Einmal ziehen");
		if ( num == RT_ACTION_ROLL         )  strcpy(text, "Anstoßen");
		if ( num == RT_ACTION_DOCKm        )  strcpy(text, "Betätigen");
		if ( num == RT_ACTION_DOCKmx       )  strcpy(text, "%d mal betätigen");
		if ( num == RT_ACTION_DOCKg        )  strcpy(text, "Aufnehmen/Ablegen");
		if ( num == RT_ACTION_TRAXa        )  strcpy(text, "Nach vorne");
		if ( num == RT_ACTION_TRAXax       )  strcpy(text, "%d mal nach vorne");
		if ( num == RT_ACTION_TRAXl        )  strcpy(text, "Nach rechts drehen");
		if ( num == RT_ACTION_TRAXlx       )  strcpy(text, "%d mal nach rechts drehen");
		if ( num == RT_ACTION_TRAXr        )  strcpy(text, "Nach links drehen");
		if ( num == RT_ACTION_TRAXrx       )  strcpy(text, "%d mal nach links drehen");
		if ( num == RT_ACTION_GUNa         )  strcpy(text, "Nach vorne und schießen");
		if ( num == RT_ACTION_GUNax        )  strcpy(text, "Nach vorne und %d mal schießen");
		if ( num == RT_ACTION_GUNl         )  strcpy(text, "Nach rechts drehen und schießen");
		if ( num == RT_ACTION_GUNlx        )  strcpy(text, "Nach rechts drehen und %d mal schießen");
		if ( num == RT_ACTION_GUNr         )  strcpy(text, "Nach links drehen und schießen");
		if ( num == RT_ACTION_GUNrx        )  strcpy(text, "Nach links drehen und %d mal schießen");
		if ( num == RT_ACTION_CATAPULT     )  strcpy(text, "Katapultieren");

		if ( num == RT_ADVISE_LITGOTO      )  strcpy(text, "...");
		if ( num == RT_ADVISE_GOTOPUSH     )  strcpy(text, "...");
		if ( num == RT_ADVISE_NOSIGN       )  strcpy(text, "Keine Hinweise für dieses Rätsel.");

		if ( num == RT_USER_HEADER         )  strcpy(text, "Datei\tUniversum\tGetestet\tAnweisungen");
		if ( num == RT_DEFI_HEADER         )  strcpy(text, "Datei\tUniversum\tAutor\tAnweisungen");
		if ( num == RT_IMPORT_HEADER       )  strcpy(text, "Datei\tUniversum\tAutor\tAnweisungen");

		if ( num == RT_DEMO                )  strcpy(text, "In der Demoversion nicht verfügbar");

		if ( num == RT_UNIVERS0            )  strcpy(text, "Castella");
		if ( num == RT_UNIVERS1            )  strcpy(text, "Paradiso");
		if ( num == RT_UNIVERS2            )  strcpy(text, "Vulkano");
		if ( num == RT_UNIVERS3            )  strcpy(text, "Mechano");
		if ( num == RT_UNIVERS4            )  strcpy(text, "Sahara");
		if ( num == RT_UNIVERS5            )  strcpy(text, "Hiverna");
		if ( num == RT_UNIVERS6            )  strcpy(text, "Samba");
		if ( num == RT_UNIVERS7            )  strcpy(text, "Medieva");
		if ( num == RT_UNIVERS8            )  strcpy(text, "Vegeta");
		if ( num == RT_UNIVERS9            )  strcpy(text, "Inferno");
		if ( num == RT_UNIVERS10           )  strcpy(text, "Futura");
		if ( num == RT_UNIVERS11           )  strcpy(text, "Alibaba");
		if ( num == RT_UNIVERS12           )  strcpy(text, "Studio");
		if ( num == RT_UNIVERS13           )  strcpy(text, "Cata");
		if ( num == RT_UNIVERS14           )  strcpy(text, "Allegro");
		if ( num == RT_UNIVERS15           )  strcpy(text, "Zemento");
		if ( num == RT_UNIVERS16           )  strcpy(text, "Labo");
		if ( num == RT_UNIVERS17           )  strcpy(text, "Mystica");
		if ( num == RT_UNIVERS18           )  strcpy(text, "Fantasia");
		if ( num == RT_UNIVERS19           )  strcpy(text, "Natura");
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Abbrechen");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menü (\\key quit;)");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Abbrechen");

		if ( num == EVENT_INTERFACE_PUZZLE )  strcpy(text, "Rätsel\\Standard Rätsel");
		if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Challenges\\Rätsel von anderen Spielern");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Werkstatt\\Machen Sie Ihren eigenen Rätsel");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypen");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Neuer Spieler\\Neuen Spieler auswählen");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Einstellungen\\Sämtliche Einstellungen für das Spiel");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Neustart\\Rätsel von vorne nocheinmal anfangen");
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Zu BlupiMania zurückkehren");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Beenden\\BlupiMania beenden");
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Abbrechen\\Zurück zum Hauptmenü");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Zurück zum Hauptmenu");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Spielen\\Ausgewähltes Rätsel jetzt spielen");
		if ( num == EVENT_INTERFACE_NEW    )  strcpy(text, "Neu\\Neues Rätsel erstellen");
		if ( num == EVENT_INTERFACE_DELETE )  strcpy(text, "Löschen\\Ausgewähltes Rätsel löschen");
		if ( num == EVENT_INTERFACE_RENAME )  strcpy(text, "Umbenennen\\Ausgewähltes Rätsel umbenennen");
		if ( num == EVENT_INTERFACE_EXPORT )  strcpy(text, "Exportieren\\Ausgewähltes Rätsel exportieren");
		if ( num == EVENT_INTERFACE_IMPORT )  strcpy(text, "Importieren\\Ein Rätsel importieren");
		if ( num == EVENT_INTERFACE_EDIT   )  strcpy(text, "Verändern\\Ausgewähltes Rätsel verändern");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Zurück zum vorhergehenden Bildschirm");

		if ( num == EVENT_INTERFACE_EXPORTdefi )  strcpy(text, "In die Challenges (für alle Spieler)");
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "Im den Ordner \"Eigene Dateien\\Blupimania2\"");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "3D-Karte\\Treiber und 3D-Einstellungen");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Grafik\\Grafikoptionen");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Spiel\\Spieleinstellungen");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Steuerung\\Einstellungen für die Taststur");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sound\\Soundeinstellungen");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Auflösung");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Vollbildschirm\\Vollbildschirmmodus oder Fenstermodus");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Änderungen übernehmen\\Übernimmt die vorgenommennen Änderrungen");

		if ( num == EVENT_INTERFACE_SHADOW      )  strcpy(text, "Schatten\\Schlagschatten auf dem Boden");
		if ( num == EVENT_INTERFACE_DIRTY       )  strcpy(text, "Schmutz\\Schmutz auf den Gegenständen");
		if ( num == EVENT_INTERFACE_SUNBEAM     )  strcpy(text, "Strahlen\\Senkrechte Strahlen über Wasserflächen");
		if ( num == EVENT_INTERFACE_LENSFLARE   )  strcpy(text, "Lichtreflexe\\Lichereflexe im Objektiv der virtuellen Kamera");
		if ( num == EVENT_INTERFACE_DECOR       )  strcpy(text, "Ziergegenstände\\Plfanzen, Bäume, Tiere, Rohre usw.");
		if ( num == EVENT_INTERFACE_DETAIL      )  strcpy(text, "Detaillierte Gegenstände\\Detaillierte Darstellung bestimmter Gegenstände");
		if ( num == EVENT_INTERFACE_METEO       )  strcpy(text, "Wetter\\Darstellung von Regen und Schnee");
		if ( num == EVENT_INTERFACE_AMBIANCE    )  strcpy(text, "Wassereffekte\\Wellen und sonstige Effekte auf Wasseroberflächen");
		if ( num == EVENT_INTERFACE_EXPLOVIB    )  strcpy(text, "Beben bei Explosionen\\Die Kamera bebt bei Explosionen");
		if ( num == EVENT_INTERFACE_SPEEDSCH    )  strcpy(text, "Horizontale Scrollgeschwindigkeit\\");
		if ( num == EVENT_INTERFACE_SPEEDSCV    )  strcpy(text, "Vertikale Scrollgeschwindigkeit\\");
		if ( num == EVENT_INTERFACE_MOUSESCROLL )  strcpy(text, "Scrollen mit der Maus\\Wenn die Maus den Rand des Bildschirms erreicht wird gescrollt");

		if ( num == EVENT_INTERFACE_INVSCH      )  strcpy(text, "Horizontal scrollen invertiert\\Wenn die Maus den Rand des Bildschirms erreicht wird anders herum gescrollt");
		if ( num == EVENT_INTERFACE_INVSCV      )  strcpy(text, "Vertikal scrollen invertiert\\Wenn die Maus den Rand des Bildschirms erreicht wird anders herum gescrollt");
		if ( num == EVENT_INTERFACE_MOVIE       )  strcpy(text, "Filmsequenzen\\Filmsequenzen vor und nach den Rätseln");
		if ( num == EVENT_INTERFACE_HELP        )  strcpy(text, "Hilfstexte\\Anweisungen und Hinweise für die Lésung der Rätsel");
		if ( num == EVENT_INTERFACE_TOOLTIPS    )  strcpy(text, "Hilfsblasen\\Zeigt Hilfsblasen solabd die Maus über einer Kontrollflächst stehen bleibt");
		if ( num == EVENT_INTERFACE_NICEMOUSE   )  strcpy(text, "Schatten unter der Maus\\Ein Schatten erscheint unter dem Mauszeiger");
		if ( num == EVENT_INTERFACE_ACCEL       )  strcpy(text, "Beschleunigt wenn es lang dauert\\Wenn ein Rätsel nach 10 Minuten noch nicht gelöst ist, dann geht Blupi schneller");
		if ( num == EVENT_INTERFACE_VOLBLUPI    )  strcpy(text, "Laustärke von Blupis Stimme\\");
		if ( num == EVENT_INTERFACE_VOLSOUND    )  strcpy(text, "Lautstärke der Geräuscheffekte\\");
		if ( num == EVENT_INTERFACE_VOLAMBIANCE )  strcpy(text, "Geräuschkulisse\\");
		if ( num == EVENT_INTERFACE_SOUND3D     )  strcpy(text, "3D-Sound\\Schaltet die 3D-Sound der Soundkarte ein");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Standardeinstellungen\\Benutzt wieder die Standardeinstellungen für die Steuerung");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Scroll links\\Nach links scrollen");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Scroll rechts\\Nach rechts scrollen");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Scroll oben\\Nach oben scrollen");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Scroll unten\\Nach unten scrollen");
		if ( num == EVENT_INTERFACE_KROTCW )  strcpy(text, "Nach rechts drehen\\Dreht die Kamera nach rechts");
		if ( num == EVENT_INTERFACE_KROTCCW)  strcpy(text, "Nach links drehen\\Dreht die Kamera nach links");
		if ( num == EVENT_INTERFACE_KSTOP  )  strcpy(text, "Handlung beenden\\Beendet die laufende Handlung");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Beenden\\Rätsel beenden");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Anweisungen\\Zeigt die Anweisungen für das Rätsel");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mini\\Minimale Grafikqualität (schneller)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Standard Grafikqualität");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Maxi\\Höchste Grafikqualität (langsamer)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Ruhe\\Keinerlei Geräusche mehr");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Normale Laustärke");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Name des Spielers");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Auswählen\\Wählt den Spieler in der Liste aus");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Neuer Spieler\\Erstellt einen neuen Spieler");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Spieler löschen\\Löscht einen Spieler aus der Liste");

		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Vorhergehender Bildschirm");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Nächster Bildschirm");
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\BlupiMania Beenden");

		if ( num == EVENT_INTERFACE_UNDO   )  strcpy(text, "Letzte Handlung widerrufen");
		if ( num == EVENT_INTERFACE_STOP   )  strcpy(text, "Laufende Handlung anhalten (\\key stop;)");
		if ( num == EVENT_INTERFACE_SIGN   )  strcpy(text, "Hinweise bekommen");
		if ( num == EVENT_LABEL_SIGN       )  strcpy(text, "Hinweise");

		if ( num == EVENT_EDIT_WATER       )  strcpy(text, "Wasser");
		if ( num == EVENT_EDIT_GROUND      )  strcpy(text, "Boden");
		if ( num == EVENT_EDIT_OBJECT      )  strcpy(text, "Gegenstände");
		if ( num == EVENT_EDIT_INFO        )  strcpy(text, "Rätselinfo bearbeiten");

		if ( num == EVENT_CMD              )  strcpy(text, "Befehlskonsole");
		if ( num == EVENT_SPEED            )  strcpy(text, "Spielgeschwindigkeit");
	}

	if ( type == RES_OBJECT )
	{
		if ( num == OBJECT_BLUPI           )  strcpy(text, "Blupi");
		if ( num == OBJECT_BOX1            )  strcpy(text, "Kiste");
		if ( num == OBJECT_BOX2            )  strcpy(text, "Kiste");
		if ( num == OBJECT_BOX3            )  strcpy(text, "Kiste");
		if ( num == OBJECT_BOX4            )  strcpy(text, "Kiste");
		if ( num == OBJECT_BOX5            )  strcpy(text, "Kiste");
		if ( num == OBJECT_BOX6            )  strcpy(text, "Kiste");
		if ( num == OBJECT_BOX7            )  strcpy(text, "Kugel");
		if ( num == OBJECT_BOX8            )  strcpy(text, "Säule");
		if ( num == OBJECT_BOX9            )  strcpy(text, "Käfig");
		if ( num == OBJECT_BOX10           )  strcpy(text, "Bombe");
		if ( num == OBJECT_BOX11           )  strcpy(text, "Wagen");
		if ( num == OBJECT_BOX12           )  strcpy(text, "Wagen");
		if ( num == OBJECT_BOX13           )  strcpy(text, "Kiste mit Luftkissen");
		if ( num == OBJECT_KEY1            )  strcpy(text, "Orangener Schlüssel");
		if ( num == OBJECT_KEY2            )  strcpy(text, "Blauer Schlüssel");
		if ( num == OBJECT_DOOR1           )  strcpy(text, "Orangene Tür");
		if ( num == OBJECT_DOOR2           )  strcpy(text, "Blaue Tür");
		if ( num == OBJECT_DOOR3           )  strcpy(text, "Orangene Tür");
		if ( num == OBJECT_DOOR4           )  strcpy(text, "Blaue Tür");
		if ( num == OBJECT_DOCK            )  strcpy(text, "Kran");
		if ( num == OBJECT_CATAPULT        )  strcpy(text, "Katapult");
		if ( num == OBJECT_TRAX            )  strcpy(text, "Schiebemaschine");
		if ( num == OBJECT_PERFO           )  strcpy(text, "Bohrer");
		if ( num == OBJECT_GUN             )  strcpy(text, "Blaster");
		if ( num == OBJECT_MINE            )  strcpy(text, "Mine");
		if ( num == OBJECT_GLASS1          )  strcpy(text, "Fensterscheibe");
		if ( num == OBJECT_GLASS2          )  strcpy(text, "Fensterscheibe");
		if ( num == OBJECT_FIOLE           )  strcpy(text, "Zaubertrank");
		if ( num == OBJECT_GLU             )  strcpy(text, "Klebetrank");
		if ( num == OBJECT_GOAL            )  strcpy(text, "Ballon");
		if ( num == OBJECT_CRAZY           )  strcpy(text, "Verückter Roboter");
		if ( num == OBJECT_BOT1            )  strcpy(text, "Bot 1");
		if ( num == OBJECT_BOT2            )  strcpy(text, "Bot 2");
		if ( num == OBJECT_BOT3            )  strcpy(text, "Bot 3");
		if ( num == OBJECT_BOT4            )  strcpy(text, "Bot 4");
		if ( num == OBJECT_BOT5            )  strcpy(text, "Bot 5");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Erreur");
		if ( num == ERR_CMD             )  strcpy(text, "Unbekannter Befehl");
		if ( num == ERR_INSTALL         )  strcpy(text, "BlupiMania ist nicht korrekt installiert worden.");
		if ( num == ERR_NOCD            )  strcpy(text, "Fügen Sie die BlupiMania CD-Rom\nin das Laufwerk ein und starten\nSie das Spiel erneut.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Bewegung nicht möglich");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Weg kann nicht gefunden werden");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Position nicht erreichbar");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Position schon besetzt");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Roboter nicht geeignet");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Nicht genügend Energie");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "Das Rätsel ist noch nicht beendet (drücken Sie auf \\key help; für mehr Details)");
		if ( num == ERR_PUZZLE_ZEROBLUPI)  strcpy(text, "Es muss mindestens eine Blupi vorhanden sein");
		if ( num == ERR_PUZZLE_ZEROGOAL )  strcpy(text, "Es muss mindestens eine Ballon vorhanden sein");
		if ( num == ERR_PUZZLE_LOGOAL   )  strcpy(text, "Es sind nicht genügend Ballons");
		if ( num == ERR_PUZZLE_HIGOAL   )  strcpy(text, "Es hat zu viele Ballons.");
		if ( num == ERR_PUZZLE_MAXBLUPI )  strcpy(text, "Maximal 4 Blupis oder Roboter pro Rätsel.");
		if ( num == ERR_PUZZLE_MAXGOAL  )  strcpy(text, "Maximal 8 Ballons pro Rätsel.");
		if ( num == ERR_PUZZLE_MAX      )  strcpy(text, "Es können keine Gegenstände dieses Typs mehr erstellt werden.");
		if ( num == ERR_CREATE          )  strcpy(text, "Diese Datei existiert schon.");

		if ( num == INFO_WIN            )  strcpy(text, "<< Bravo Sie haben das Rätsel erfolgreich gelöst >>");
		if ( num == INFO_LOST           )  strcpy(text, "<< Tut mir Leid, Sie haben versagt >>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<< Tut mir Leid, Sie haben versagt >>");
	}

	if ( type == RES_CBOT )
	{
		strcpy(text, "Erreur");
		if ( num == TX_OPENPAR       ) strcpy(text, "Il manque une parenthèse ouvrante");
		if ( num == TX_CLOSEPAR      ) strcpy(text, "Il manque une parenthèse fermante");
		if ( num == TX_NOTBOOL       ) strcpy(text, "L'expression doit être un boolean");
		if ( num == TX_UNDEFVAR      ) strcpy(text, "Variable non déclarée");
		if ( num == TX_BADLEFT       ) strcpy(text, "Assignation impossible");
		if ( num == TX_ENDOF         ) strcpy(text, "Terminateur point-virgule non trouvé");
		if ( num == TX_OUTCASE       ) strcpy(text, "Instruction ""case"" hors d'un bloc ""switch""");
		if ( num == TX_NOTERM        ) strcpy(text, "Instructions après la fin");
		if ( num == TX_CLOSEBLK      ) strcpy(text, "Il manque la fin du bloc");
		if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instruction ""else"" sans ""if"" correspondant");
		if ( num == TX_OPENBLK       ) strcpy(text, "Début d'un bloc attendu");
		if ( num == TX_BADTYPE       ) strcpy(text, "Mauvais type de résultat pour l'assignation");
		if ( num == TX_REDEFVAR      ) strcpy(text, "Redéfinition d'une variable");
		if ( num == TX_BAD2TYPE      ) strcpy(text, "Les deux opérandes ne sont pas de types compatibles");
		if ( num == TX_UNDEFCALL     ) strcpy(text, "Routine inconnue");
		if ( num == TX_MISDOTS       ) strcpy(text, "Séparateur "" : "" attendu");
		if ( num == TX_WHILE         ) strcpy(text, "Manque le mot ""while""");
		if ( num == TX_BREAK         ) strcpy(text, "Instruction ""break"" en dehors d'une boucle");
		if ( num == TX_LABEL         ) strcpy(text, "Un label ne peut se placer que devant un ""for"", un ""while"", un ""do"" ou un ""switch""");
		if ( num == TX_NOLABEL       ) strcpy(text, "Cette étiquette n'existe pas");
		if ( num == TX_NOCASE        ) strcpy(text, "Manque une instruction ""case""");
		if ( num == TX_BADNUM        ) strcpy(text, "Un nombre est attendu");
		if ( num == TX_VOID          ) strcpy(text, "Paramètre void");
		if ( num == TX_NOTYP         ) strcpy(text, "Déclaration de type attendu");
		if ( num == TX_NOVAR         ) strcpy(text, "Nom d'une variable attendu");
		if ( num == TX_NOFONC        ) strcpy(text, "Nom de la fonction attendu");
		if ( num == TX_OVERPARAM     ) strcpy(text, "Trop de paramètres");
		if ( num == TX_REDEF         ) strcpy(text, "Cette fonction existe déjà");
		if ( num == TX_LOWPARAM      ) strcpy(text, "Pas assez de paramètres");
		if ( num == TX_BADPARAM      ) strcpy(text, "Aucune fonction de ce nom n'accepte ce(s) type(s) de paramètre(s)");
		if ( num == TX_NUMPARAM      ) strcpy(text, "Aucune fonction de ce nom n'accepte ce nombre de paramètres");
		if ( num == TX_NOITEM        ) strcpy(text, "Cet élément n'existe pas dans cette classe");
		if ( num == TX_DOT           ) strcpy(text, "L'objet n'est pas une instance d'une classe");
		if ( num == TX_NOCONST       ) strcpy(text, "Il n'y a pas de constructeur approprié");
		if ( num == TX_REDEFCLASS    ) strcpy(text, "Cette classe existe déjà");
		if ( num == TX_CLBRK         ) strcpy(text, """ ] "" attendu");
		if ( num == TX_RESERVED      ) strcpy(text, "Ce mot est réservé");
		if ( num == TX_BADNEW        ) strcpy(text, "Mauvais argument pour ""new""");
		if ( num == TX_OPBRK         ) strcpy(text, """ [ "" attendu");
		if ( num == TX_BADSTRING     ) strcpy(text, "Une chaîne de caractère est attendue");
		if ( num == TX_BADINDEX      ) strcpy(text, "Mauvais type d'index");
		if ( num == TX_PRIVATE       ) strcpy(text, "Elément protégé");
		if ( num == TX_NOPUBLIC      ) strcpy(text, "Public requis");
		if ( num == TX_DIVZERO       ) strcpy(text, "Division par zéro");
		if ( num == TX_NOTINIT       ) strcpy(text, "Variable non initialisée");
		if ( num == TX_BADTHROW      ) strcpy(text, "Valeur négative refusée pour ""throw""");
		if ( num == TX_NORETVAL      ) strcpy(text, "La fonction n'a pas retourné de résultat");
		if ( num == TX_NORUN         ) strcpy(text, "Pas de fonction en exécution");
		if ( num == TX_NOCALL        ) strcpy(text, "Appel d'une fonction inexistante");
		if ( num == TX_NOCLASS       ) strcpy(text, "Cette classe n'existe pas");
		if ( num == TX_NULLPT        ) strcpy(text, "Objet n'existe pas");
		if ( num == TX_OPNAN         ) strcpy(text, "Opération sur un ""nan""");
		if ( num == TX_OUTARRAY      ) strcpy(text, "Accès hors du tableau");
		if ( num == TX_STACKOVER     ) strcpy(text, "Débordement de la pile");
		if ( num == TX_DELETEDPT     ) strcpy(text, "Objet inaccessible");
		if ( num == TX_FILEOPEN      ) strcpy(text, "Ouverture du fichier impossible");
		if ( num == TX_NOTOPEN       ) strcpy(text, "Le fichier n'est pas ouvert");
		if ( num == TX_ERRREAD       ) strcpy(text, "Erreur à la lecture");
		if ( num == TX_ERRWRITE      ) strcpy(text, "Erreur à l'écriture");
	}

	if ( type == RES_KEY )
	{
		if ( num == 0                   )  strcpy(text, "< nicht belegt >");
		if ( num == VK_LEFT             )  strcpy(text, "Pfeil nach links");
		if ( num == VK_RIGHT            )  strcpy(text, "Pfeil nach rechts");
		if ( num == VK_UP               )  strcpy(text, "Pfeil nach oben");
		if ( num == VK_DOWN             )  strcpy(text, "Pfeil nach unten");
		if ( num == VK_CANCEL           )  strcpy(text, "Strg-Pause");
		if ( num == VK_BACK             )  strcpy(text, "<--");
		if ( num == VK_TAB              )  strcpy(text, "Tab");
		if ( num == VK_CLEAR            )  strcpy(text, "Clear");
		if ( num == VK_RETURN           )  strcpy(text, "Eingabe");
		if ( num == VK_SHIFT            )  strcpy(text, "Umschalt");
		if ( num == VK_CONTROL          )  strcpy(text, "Ctrl/Strg");
		if ( num == VK_MENU             )  strcpy(text, "Alt");
		if ( num == VK_PAUSE            )  strcpy(text, "Pause");
		if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
		if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
		if ( num == VK_SPACE            )  strcpy(text, "Leertaste");
		if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
		if ( num == VK_NEXT             )  strcpy(text, "Page Down");
		if ( num == VK_END              )  strcpy(text, "End");
		if ( num == VK_HOME             )  strcpy(text, "Home");
		if ( num == VK_SELECT           )  strcpy(text, "Select");
		if ( num == VK_EXECUTE          )  strcpy(text, "Execute");
		if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
		if ( num == VK_INSERT           )  strcpy(text, "Insert");
		if ( num == VK_DELETE           )  strcpy(text, "Delete");
		if ( num == VK_HELP             )  strcpy(text, "Help");
		if ( num == VK_LWIN             )  strcpy(text, "Left Windows");
		if ( num == VK_RWIN             )  strcpy(text, "Right Windows");
		if ( num == VK_APPS             )  strcpy(text, "Application key");
		if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
		if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
		if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
		if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
		if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
		if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
		if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
		if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
		if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
		if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
		if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
		if ( num == VK_ADD              )  strcpy(text, "NumPad +");
		if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
		if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
		if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
		if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
		if ( num == VK_F1               )  strcpy(text, "F1");
		if ( num == VK_F2               )  strcpy(text, "F2");
		if ( num == VK_F3               )  strcpy(text, "F3");
		if ( num == VK_F4               )  strcpy(text, "F4");
		if ( num == VK_F5               )  strcpy(text, "F5");
		if ( num == VK_F6               )  strcpy(text, "F6");
		if ( num == VK_F7               )  strcpy(text, "F7");
		if ( num == VK_F8               )  strcpy(text, "F8");
		if ( num == VK_F9               )  strcpy(text, "F9");
		if ( num == VK_F10              )  strcpy(text, "F10");
		if ( num == VK_F11              )  strcpy(text, "F11");
		if ( num == VK_F12              )  strcpy(text, "F12");
		if ( num == VK_F13              )  strcpy(text, "F13");
		if ( num == VK_F14              )  strcpy(text, "F14");
		if ( num == VK_F15              )  strcpy(text, "F15");
		if ( num == VK_F16              )  strcpy(text, "F16");
		if ( num == VK_F17              )  strcpy(text, "F17");
		if ( num == VK_F18              )  strcpy(text, "F18");
		if ( num == VK_F19              )  strcpy(text, "F19");
		if ( num == VK_F20              )  strcpy(text, "F20");
		if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
		if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
		if ( num == VK_ATTN             )  strcpy(text, "Attn");
		if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
		if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
		if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
		if ( num == VK_PLAY             )  strcpy(text, "Play");
		if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
		if ( num == VK_PA1              )  strcpy(text, "PA1");
		if ( num == VK_OEM_CLEAR        )  strcpy(text, "Clear");
		if ( num == VK_BUTTON1          )  strcpy(text, "Knopf 1");
		if ( num == VK_BUTTON2          )  strcpy(text, "Knopf 2");
		if ( num == VK_BUTTON3          )  strcpy(text, "Knopf 3");
		if ( num == VK_BUTTON4          )  strcpy(text, "Knopf 4");
		if ( num == VK_BUTTON5          )  strcpy(text, "Knopf 5");
		if ( num == VK_BUTTON6          )  strcpy(text, "Knopf 6");
		if ( num == VK_BUTTON7          )  strcpy(text, "Knopf 7");
		if ( num == VK_BUTTON8          )  strcpy(text, "Knopf 8");
		if ( num == VK_BUTTON9          )  strcpy(text, "Knopf 9");
		if ( num == VK_BUTTON10         )  strcpy(text, "Knopf 10");
		if ( num == VK_BUTTON11         )  strcpy(text, "Knopf 11");
		if ( num == VK_BUTTON12         )  strcpy(text, "Knopf 12");
		if ( num == VK_BUTTON13         )  strcpy(text, "Knopf 13");
		if ( num == VK_BUTTON14         )  strcpy(text, "Knopf 14");
		if ( num == VK_BUTTON15         )  strcpy(text, "Knopf 15");
		if ( num == VK_BUTTON16         )  strcpy(text, "Knopf 16");
		if ( num == VK_BUTTON17         )  strcpy(text, "Knopf 17");
		if ( num == VK_BUTTON18         )  strcpy(text, "Knopf 18");
		if ( num == VK_BUTTON19         )  strcpy(text, "Knopf 19");
		if ( num == VK_BUTTON20         )  strcpy(text, "Knopf 20");
		if ( num == VK_BUTTON21         )  strcpy(text, "Knopf 21");
		if ( num == VK_BUTTON22         )  strcpy(text, "Knopf 22");
		if ( num == VK_BUTTON23         )  strcpy(text, "Knopf 23");
		if ( num == VK_BUTTON24         )  strcpy(text, "Knopf 24");
		if ( num == VK_BUTTON25         )  strcpy(text, "Knopf 25");
		if ( num == VK_BUTTON26         )  strcpy(text, "Knopf 26");
		if ( num == VK_BUTTON27         )  strcpy(text, "Knopf 27");
		if ( num == VK_BUTTON28         )  strcpy(text, "Knopf 28");
		if ( num == VK_BUTTON29         )  strcpy(text, "Knopf 29");
		if ( num == VK_BUTTON30         )  strcpy(text, "Knopf 30");
		if ( num == VK_BUTTON31         )  strcpy(text, "Knopf 31");
		if ( num == VK_BUTTON32         )  strcpy(text, "Knopf 32");
		if ( num == VK_WHEELUP          )  strcpy(text, "Mausrad nach oben");
		if ( num == VK_WHEELDOWN        )  strcpy(text, "Mausrad nach unten");
	}
#endif

#if _FRENCH
	if ( type == RES_TEXT )
	{
		#if _FULL
		#if _EDU
		if ( num == RT_VERSION_ID          )  strcpy(text, "EDU 1.3 /f");
		#else
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /f");
		#endif
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /f");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Taille maximale");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Taille réduite");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Taille normale");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Fermer");

		if ( num == RT_KEY_OR              )  strcpy(text, " ou ");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Choix du casse-tête à résoudre :");
		if ( num == RT_PLAY_LISTd          )  strcpy(text, " Défis :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Atelier de %s :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Liste des prototypes :");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Pilotes :");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Résolutions :");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Cliquez d'abord sur la touche à redéfinir.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Appuyez ensuite sur la nouvelle touche souhaitée.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Liste des joueurs :");

		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Voulez-vous quitter BlupiMania ?");
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Abandonner\\Abandonner le casse-tête en cours");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuer\\Continuer le casse-tête en cours");
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quitter\\Quitter BlupiMania");
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuer\\Continuer de jouer");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Voulez-vous supprimer la progression et l'atelier de %s ?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Voulez-vous supprimer le fichier %s ?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Supprimer");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nom du joueur à créer :");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Créer");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_PLAY         )  strcpy(text, "Résoudre le casse-tête...");
		if ( num == RT_DIALOG_REPLAY       )  strcpy(text, "Rejouer pour le plaisir...");
		if ( num == RT_DIALOG_QUITEDIT     )  strcpy(text, "Enregistrer les modifications ?");
		if ( num == RT_DIALOG_YESQUITEDIT  )  strcpy(text, "Oui\\Enregistrer et quitter");
		if ( num == RT_DIALOG_NOQUITEDIT   )  strcpy(text, "Non\\Quitter sans enregistrer");
		if ( num == RT_DIALOG_CANQUITEDIT  )  strcpy(text, "Annuler\\Continuer l'édition");
		if ( num == RT_DIALOG_NEWPUZZLE    )  strcpy(text, "Voulez-vous créer un nouveau casse-tête de type \"%s\" ?");
		if ( num == RT_DIALOG_YESNEWP      )  strcpy(text, "Créer");
		if ( num == RT_DIALOG_NONEWP       )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_DELPUZ       )  strcpy(text, "Voulez-vous supprimer le casse-tête ?");
		if ( num == RT_DIALOG_DELPUZd      )  strcpy(text, "(pour tous les joueurs)");
		if ( num == RT_DIALOG_DELPUZu      )  strcpy(text, "(seulement dans votre atelier)");
		if ( num == RT_DIALOG_YESDELPUZ    )  strcpy(text, "Supprimer");
		if ( num == RT_DIALOG_NODELPUZ     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RESUMEINFO   )  strcpy(text, "Instructions");
		if ( num == RT_DIALOG_AUTHORINFO   )  strcpy(text, "Auteur");
		if ( num == RT_DIALOG_SIGNINFO     )  strcpy(text, "Indices");
		if ( num == RT_DIALOG_YESINFO      )  strcpy(text, "OK");
		if ( num == RT_DIALOG_YESERROR     )  strcpy(text, "OK");
		if ( num == RT_DIALOG_EXPORTGAME   )  strcpy(text, "Où voulez-vous exporter le casse-tête ?");
		if ( num == RT_DIALOG_YESEXPORT    )  strcpy(text, "Exporter");
		if ( num == RT_DIALOG_NOEXPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Casse-tête contenus dans \"Mes documents/BlupiMania2\" :");
		if ( num == RT_DIALOG_YESIMPORT    )  strcpy(text, "Importer");
		if ( num == RT_DIALOG_NOIMPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_TEST         )  strcpy(text, "Tester");
		if ( num == RT_DIALOG_DEFI         )  strcpy(text, "Relever le défi...");
		if ( num == RT_DIALOG_FILE         )  strcpy(text, "Fichier");
		if ( num == RT_DIALOG_RENAMEGAME   )  strcpy(text, "Changement du nom du fichier du casse-tête.");
		if ( num == RT_DIALOG_YESRENAME    )  strcpy(text, "Renommer");
		if ( num == RT_DIALOG_NORENAME     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RENAMEOLD    )  strcpy(text, "Nom actuel");
		if ( num == RT_DIALOG_RENAMENEW    )  strcpy(text, "Nouveau nom");

		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.epsitec.ch");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Développé par EPSITEC");
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Vous avez joué à la version DEMO de BlupiMania II.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "La version complète est disponible dès aujourd'hui ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... elle comprend 80 casse-tête et 40 défis ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... ainsi qu'un \"atelier\" pour créer ses propres casse-tête.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Tous les détails sur www.epsitec.ch !");

		if ( num == RT_ACTION_GOTO         )  strcpy(text, "Aller");
		if ( num == RT_ACTION_PUSH1        )  strcpy(text, "Pousser une fois");
		if ( num == RT_ACTION_PUSHx        )  strcpy(text, "Pousser %d fois");
		if ( num == RT_ACTION_PUSHn        )  strcpy(text, "Pousser toujours");
		if ( num == RT_ACTION_PULL1        )  strcpy(text, "Tirer une fois");
		if ( num == RT_ACTION_ROLL         )  strcpy(text, "Rouler");
		if ( num == RT_ACTION_DOCKm        )  strcpy(text, "Actionner");
		if ( num == RT_ACTION_DOCKmx       )  strcpy(text, "Actionner %d fois");
		if ( num == RT_ACTION_DOCKg        )  strcpy(text, "Saisir/déposer");
		if ( num == RT_ACTION_TRAXa        )  strcpy(text, "Avancer");
		if ( num == RT_ACTION_TRAXax       )  strcpy(text, "Avancer %d fois");
		if ( num == RT_ACTION_TRAXl        )  strcpy(text, "Tourner à droite");
		if ( num == RT_ACTION_TRAXlx       )  strcpy(text, "Tourner à droite %d fois");
		if ( num == RT_ACTION_TRAXr        )  strcpy(text, "Tourner à gauche");
		if ( num == RT_ACTION_TRAXrx       )  strcpy(text, "Tourner à gauche %d fois");
		if ( num == RT_ACTION_GUNa         )  strcpy(text, "Avancer puis tirer");
		if ( num == RT_ACTION_GUNax        )  strcpy(text, "Avancer puis tirer %d fois");
		if ( num == RT_ACTION_GUNl         )  strcpy(text, "Tourner à droite puis tirer");
		if ( num == RT_ACTION_GUNlx        )  strcpy(text, "Tourner à droite puis tirer %d fois");
		if ( num == RT_ACTION_GUNr         )  strcpy(text, "Tourner à gauche puis tirer");
		if ( num == RT_ACTION_GUNrx        )  strcpy(text, "Tourner à gauche puis tirer %d fois");
		if ( num == RT_ACTION_CATAPULT     )  strcpy(text, "Lancer");

		if ( num == RT_ADVISE_LITGOTO      )  strcpy(text, "Conseil:\nCliquez directement sur la destination, même si elle est éloignée. Blupi est assez malin pour s'y rendre par le meilleur chemin.");
		if ( num == RT_ADVISE_GOTOPUSH     )  strcpy(text, "Conseil:\nCliquez directement sur la caisse. Blupi est assez malin pour d'abord se déplacer et ensuite pousser la caisse.");
		if ( num == RT_ADVISE_NOSIGN       )  strcpy(text, "Aucun indice disponible pour ce casse-tête.");

		if ( num == RT_USER_HEADER         )  strcpy(text, "Fichier\tUnivers\tTesté\tInstructions");
		if ( num == RT_DEFI_HEADER         )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");
		if ( num == RT_IMPORT_HEADER       )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");

		if ( num == RT_DEMO                )  strcpy(text, "Pas disponible dans la DEMO");

		if ( num == RT_UNIVERS0            )  strcpy(text, "Castella");
		if ( num == RT_UNIVERS1            )  strcpy(text, "Paradiso");
		if ( num == RT_UNIVERS2            )  strcpy(text, "Volcano");
		if ( num == RT_UNIVERS3            )  strcpy(text, "Mecano");
		if ( num == RT_UNIVERS4            )  strcpy(text, "Sahara");
		if ( num == RT_UNIVERS5            )  strcpy(text, "Aglagla");
		if ( num == RT_UNIVERS6            )  strcpy(text, "Samba");
		if ( num == RT_UNIVERS7            )  strcpy(text, "Medieva");
		if ( num == RT_UNIVERS8            )  strcpy(text, "Vegeta");
		if ( num == RT_UNIVERS9            )  strcpy(text, "Inferno");
		if ( num == RT_UNIVERS10           )  strcpy(text, "Futura");
		if ( num == RT_UNIVERS11           )  strcpy(text, "Alibaba");
		if ( num == RT_UNIVERS12           )  strcpy(text, "Studio");
		if ( num == RT_UNIVERS13           )  strcpy(text, "Cata");
		if ( num == RT_UNIVERS14           )  strcpy(text, "Allegro");
		if ( num == RT_UNIVERS15           )  strcpy(text, "Cemento");
		if ( num == RT_UNIVERS16           )  strcpy(text, "Labo");
		if ( num == RT_UNIVERS17           )  strcpy(text, "Mystica");
		if ( num == RT_UNIVERS18           )  strcpy(text, "Fantasia");
		if ( num == RT_UNIVERS19           )  strcpy(text, "Natura");
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Annuler");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Annuler");

		if ( num == EVENT_INTERFACE_PUZZLE )  strcpy(text, "Casse-tête\\Torture progressive");
		if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Défis\\Casse-tête d'autres joueurs");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Atelier\\Créez vos casse-tête");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypes en cours d'élaboration");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Autre joueur\\Choix du nom du joueur");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Options\\Réglages");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Recommencer\\Recommencer le casse-tête au début");
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Retourner dans BlupiMania");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quitter\\Quitter BlupiMania");
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Annuler\\Retour au niveau précédent");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Retour au menu");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Jouer\\Démarrer l'action");
		if ( num == EVENT_INTERFACE_NEW    )  strcpy(text, "Nouveau\\Créer un nouveau casse-tête");
		if ( num == EVENT_INTERFACE_DELETE )  strcpy(text, "Supprimer\\Supprimer le casse-tête");
		if ( num == EVENT_INTERFACE_RENAME )  strcpy(text, "Renommer\\Renommer le casse-tête");
		if ( num == EVENT_INTERFACE_EXPORT )  strcpy(text, "Exporter\\Exporter le casse-tête");
		if ( num == EVENT_INTERFACE_IMPORT )  strcpy(text, "Importer\\Importer un casse-tête");
		if ( num == EVENT_INTERFACE_EDIT   )  strcpy(text, "Modifier\\Modifier le casse-tête");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Retour au niveau précédent");

		if ( num == EVENT_INTERFACE_EXPORTdefi )  strcpy(text, "Dans les défis (pour tous les joueurs)");
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "Dans le dossier \"Mes documents/BlupiMania2\"");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Affichage\\Pilote et résolution d'affichage");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Graphique\\Options graphiques");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Jeu\\Options de jouabilité");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Commandes\\Touches du clavier");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Son\\Volumes bruitages & musiques");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Résolution");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Plein écran\\Plein écran ou fenêtré");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Appliquer les changements\\Active les changements effectués");

		if ( num == EVENT_INTERFACE_SHADOW      )  strcpy(text, "Ombres\\Ombres projetées au sol");
		if ( num == EVENT_INTERFACE_DIRTY       )  strcpy(text, "Salissures\\Salissures des objets");
		if ( num == EVENT_INTERFACE_SUNBEAM     )  strcpy(text, "Rayons du soleil\\Rayons verticaux sur l'eau");
		if ( num == EVENT_INTERFACE_LENSFLARE   )  strcpy(text, "Reflets dans l'objectif\\Reflets dans les lentilles de la caméra");
		if ( num == EVENT_INTERFACE_DECOR       )  strcpy(text, "Objets décoratifs\\Arbres, plantes, animaux, tuyaux, etc.");
		if ( num == EVENT_INTERFACE_DETAIL      )  strcpy(text, "Objets détaillés\\Formes compliquées des objets");
		if ( num == EVENT_INTERFACE_METEO       )  strcpy(text, "Conditions météo\\Pluie et neige");
		if ( num == EVENT_INTERFACE_AMBIANCE    )  strcpy(text, "Effets à la surface de l'eau\\Gouttes, ronds, jets de lave, etc.");
		if ( num == EVENT_INTERFACE_EXPLOVIB    )  strcpy(text, "Secousses lors d'explosions\\L'écran vibre lors d'une explosion");
		if ( num == EVENT_INTERFACE_SPEEDSCH    )  strcpy(text, "Vitesse de défilement horizontal\\Vitesse pour le clavier et la souris");
		if ( num == EVENT_INTERFACE_SPEEDSCV    )  strcpy(text, "Vitesse de défilement vertical\\Vitesse pour le clavier et la souris");
		if ( num == EVENT_INTERFACE_MOUSESCROLL )  strcpy(text, "Défilement avec la souris\\Lorsque la souris touche les bords");
		if ( num == EVENT_INTERFACE_INVSCH      )  strcpy(text, "Inversion défilement souris horizontal\\Lorsque la souris touche les bords gauche ou droite");
		if ( num == EVENT_INTERFACE_INVSCV      )  strcpy(text, "Inversion défilement souris vertical\\Lorsque la souris touche les bords haut ou bas");
		if ( num == EVENT_INTERFACE_MOVIE       )  strcpy(text, "Séquences cinématiques\\Séquences non interractives");
		if ( num == EVENT_INTERFACE_HELP        )  strcpy(text, "Instructions et indices\\Textes explicatifs");
		if ( num == EVENT_INTERFACE_TOOLTIPS    )  strcpy(text, "Bulles d'aide\\Bulles explicatives");
		if ( num == EVENT_INTERFACE_NICEMOUSE   )  strcpy(text, "Souris ombrée\\Jolie souris avec une ombre");
		if ( num == EVENT_INTERFACE_ACCEL       )  strcpy(text, "Accélère si long à résoudre\\Blupi va de plus en plus vite après 10 minutes de réflexion");
		if ( num == EVENT_INTERFACE_VOLBLUPI    )  strcpy(text, "Volume des voix de Blupi\\");
		if ( num == EVENT_INTERFACE_VOLSOUND    )  strcpy(text, "Volume des bruitages\\");
		if ( num == EVENT_INTERFACE_VOLAMBIANCE )  strcpy(text, "Volume d'ambiance\\");
		if ( num == EVENT_INTERFACE_SOUND3D     )  strcpy(text, "Bruitages 3D\\Positionnement sonore dans l'espace");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Tout réinitialiser\\Remet toutes les touches standards");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Défilement à gauche\\Défilement de l'écran à gauche");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Défilement à droite\\Défilement de l'écran à droite");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Défilement en haut\\Défilement de l'écran en haut");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Défilement en bas\\Défilement de l'écran en bas");
		if ( num == EVENT_INTERFACE_KROTCW )  strcpy(text, "Rotation horaire\\Rotation horaire de la caméra");
		if ( num == EVENT_INTERFACE_KROTCCW)  strcpy(text, "Rotation anti-horaire\\Rotation anti-horaire de la caméra");
		if ( num == EVENT_INTERFACE_KSTOP  )  strcpy(text, "Stoppe l'action en cours\\Stoppe l'action en cours");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Quitter le casse-tête en cours\\Terminer un casse-tête");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instructions sur le casse-tête\\Marche à suivre");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mini\\Qualité minimale (+ rapide)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Qualité standard");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Maxi\\Haute qualité (+ lent)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silencieux\\Totalement silencieux");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Niveaux normaux");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nom du joueur à créer");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Sélectionner\\Choisir le joueur de la liste");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Nouveau\\Créer un nouveau joueur");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Supprimer\\Supprimer le joueur de la liste");

		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Ecran précédent");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Ecran suivant");
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quitter BlupiMania");

		if ( num == EVENT_INTERFACE_UNDO   )  strcpy(text, "Annuler la dernière action");
		if ( num == EVENT_INTERFACE_STOP   )  strcpy(text, "Stopper l'action en cours (\\key stop;)");
		if ( num == EVENT_INTERFACE_SIGN   )  strcpy(text, "Obtenir des indices");
		if ( num == EVENT_LABEL_SIGN       )  strcpy(text, "Indices");

		if ( num == EVENT_EDIT_WATER       )  strcpy(text, "Eau");
		if ( num == EVENT_EDIT_GROUND      )  strcpy(text, "Sol");
		if ( num == EVENT_EDIT_OBJECT      )  strcpy(text, "Objets");
		if ( num == EVENT_EDIT_CLEAR       )  strcpy(text, "Supprimer");
		if ( num == EVENT_EDIT_INFO        )  strcpy(text, "Informations sur le casse-tête");

		if ( num == EVENT_CMD              )  strcpy(text, "Console de commande");
		if ( num == EVENT_SPEED            )  strcpy(text, "Vitesse du jeu");
	}

	if ( type == RES_OBJECT )
	{
		if ( num == OBJECT_BLUPI           )  strcpy(text, "Blupi");
		if ( num == OBJECT_BOX1            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX2            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX3            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX4            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX5            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX6            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX7            )  strcpy(text, "Grosse boule");
		if ( num == OBJECT_BOX8            )  strcpy(text, "Colonne lourde");
		if ( num == OBJECT_BOX9            )  strcpy(text, "Cage");
		if ( num == OBJECT_BOX10           )  strcpy(text, "Bombe");
		if ( num == OBJECT_BOX11           )  strcpy(text, "Chariot");
		if ( num == OBJECT_BOX12           )  strcpy(text, "Chariot");
		if ( num == OBJECT_BOX13           )  strcpy(text, "Caisse sur coussin d'air");
		if ( num == OBJECT_KEY1            )  strcpy(text, "Clé orange");
		if ( num == OBJECT_KEY2            )  strcpy(text, "Clé bleue");
		if ( num == OBJECT_DOOR1           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR2           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOOR3           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR4           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOCK            )  strcpy(text, "Pont-grue");
		if ( num == OBJECT_CATAPULT        )  strcpy(text, "Catapulte");
		if ( num == OBJECT_TRAX            )  strcpy(text, "Pousseur mécanique");
		if ( num == OBJECT_PERFO           )  strcpy(text, "Perforateur mécanique");
		if ( num == OBJECT_GUN             )  strcpy(text, "Destructeur de caisses");
		if ( num == OBJECT_MINE            )  strcpy(text, "Mine");
		if ( num == OBJECT_GLASS1          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_GLASS2          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_FIOLE           )  strcpy(text, "Potion de force");
		if ( num == OBJECT_GLU             )  strcpy(text, "Potion de glu");
		if ( num == OBJECT_GOAL            )  strcpy(text, "But à atteindre");
		if ( num == OBJECT_CRAZY           )  strcpy(text, "Robot fou");
		if ( num == OBJECT_BOT1            )  strcpy(text, "Bot 1");
		if ( num == OBJECT_BOT2            )  strcpy(text, "Bot 2");
		if ( num == OBJECT_BOT3            )  strcpy(text, "Bot 3");
		if ( num == OBJECT_BOT4            )  strcpy(text, "Bot 4");
		if ( num == OBJECT_BOT5            )  strcpy(text, "Bot 5");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Erreur");
		if ( num == ERR_CMD             )  strcpy(text, "Commande inconnue");
		if ( num == ERR_INSTALL         )  strcpy(text, "BlupiMania n'est pas installé.");
		if ( num == ERR_NOCD            )  strcpy(text, "Veuillez mettre le CD de BlupiMania\net relancer le jeu.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Déplacement impossible");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Chemin introuvable");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Position inaccessible");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Destination occupée");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Robot inadapté");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Pas assez d'énergie");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "Le casse-tête n'est pas terminé (appuyez sur \\key help; pour plus de détails)");
		if ( num == ERR_PUZZLE_ZEROBLUPI)  strcpy(text, "Il doit y avoir au moins un Blupi.");
		if ( num == ERR_PUZZLE_ZEROGOAL )  strcpy(text, "Il doit y avoir au moins un ballon.");
		if ( num == ERR_PUZZLE_LOGOAL   )  strcpy(text, "Il n'y a pas assez de ballons.");
		if ( num == ERR_PUZZLE_HIGOAL   )  strcpy(text, "Il y a trop de ballons.");
		if ( num == ERR_PUZZLE_MAXBLUPI )  strcpy(text, "4 Blupi ou robots au maximum par casse-tête.");
		if ( num == ERR_PUZZLE_MAXGOAL  )  strcpy(text, "8 ballons au maximum par casse-tête.");
		if ( num == ERR_PUZZLE_MAX      )  strcpy(text, "Il n'est pas possible de créer plus d'objets de ce type.");
		if ( num == ERR_CREATE          )  strcpy(text, "Ce fichier existe déjà.");

		if ( num == INFO_WIN            )  strcpy(text, "<< Bravo, casse-tête terminé >>");
		if ( num == INFO_LOST           )  strcpy(text, "<< Désolé, casse-tête échoué >>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<< Désolé, casse-tête échoué >>");
	}

	if ( type == RES_CBOT )
	{
		strcpy(text, "Erreur");
		if ( num == TX_OPENPAR       ) strcpy(text, "Il manque une parenthèse ouvrante");
		if ( num == TX_CLOSEPAR      ) strcpy(text, "Il manque une parenthèse fermante");
		if ( num == TX_NOTBOOL       ) strcpy(text, "L'expression doit être un boolean");
		if ( num == TX_UNDEFVAR      ) strcpy(text, "Variable non déclarée");
		if ( num == TX_BADLEFT       ) strcpy(text, "Assignation impossible");
		if ( num == TX_ENDOF         ) strcpy(text, "Terminateur point-virgule non trouvé");
		if ( num == TX_OUTCASE       ) strcpy(text, "Instruction ""case"" hors d'un bloc ""switch""");
		if ( num == TX_NOTERM        ) strcpy(text, "Instructions après la fin");
		if ( num == TX_CLOSEBLK      ) strcpy(text, "Il manque la fin du bloc");
		if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instruction ""else"" sans ""if"" correspondant");
		if ( num == TX_OPENBLK       ) strcpy(text, "Début d'un bloc attendu");
		if ( num == TX_BADTYPE       ) strcpy(text, "Mauvais type de résultat pour l'assignation");
		if ( num == TX_REDEFVAR      ) strcpy(text, "Redéfinition d'une variable");
		if ( num == TX_BAD2TYPE      ) strcpy(text, "Les deux opérandes ne sont pas de types compatibles");
		if ( num == TX_UNDEFCALL     ) strcpy(text, "Routine inconnue");
		if ( num == TX_MISDOTS       ) strcpy(text, "Séparateur "" : "" attendu");
		if ( num == TX_WHILE         ) strcpy(text, "Manque le mot ""while""");
		if ( num == TX_BREAK         ) strcpy(text, "Instruction ""break"" en dehors d'une boucle");
		if ( num == TX_LABEL         ) strcpy(text, "Un label ne peut se placer que devant un ""for"", un ""while"", un ""do"" ou un ""switch""");
		if ( num == TX_NOLABEL       ) strcpy(text, "Cette étiquette n'existe pas");
		if ( num == TX_NOCASE        ) strcpy(text, "Manque une instruction ""case""");
		if ( num == TX_BADNUM        ) strcpy(text, "Un nombre est attendu");
		if ( num == TX_VOID          ) strcpy(text, "Paramètre void");
		if ( num == TX_NOTYP         ) strcpy(text, "Déclaration de type attendu");
		if ( num == TX_NOVAR         ) strcpy(text, "Nom d'une variable attendu");
		if ( num == TX_NOFONC        ) strcpy(text, "Nom de la fonction attendu");
		if ( num == TX_OVERPARAM     ) strcpy(text, "Trop de paramètres");
		if ( num == TX_REDEF         ) strcpy(text, "Cette fonction existe déjà");
		if ( num == TX_LOWPARAM      ) strcpy(text, "Pas assez de paramètres");
		if ( num == TX_BADPARAM      ) strcpy(text, "Aucune fonction de ce nom n'accepte ce(s) type(s) de paramètre(s)");
		if ( num == TX_NUMPARAM      ) strcpy(text, "Aucune fonction de ce nom n'accepte ce nombre de paramètres");
		if ( num == TX_NOITEM        ) strcpy(text, "Cet élément n'existe pas dans cette classe");
		if ( num == TX_DOT           ) strcpy(text, "L'objet n'est pas une instance d'une classe");
		if ( num == TX_NOCONST       ) strcpy(text, "Il n'y a pas de constructeur approprié");
		if ( num == TX_REDEFCLASS    ) strcpy(text, "Cette classe existe déjà");
		if ( num == TX_CLBRK         ) strcpy(text, """ ] "" attendu");
		if ( num == TX_RESERVED      ) strcpy(text, "Ce mot est réservé");
		if ( num == TX_BADNEW        ) strcpy(text, "Mauvais argument pour ""new""");
		if ( num == TX_OPBRK         ) strcpy(text, """ [ "" attendu");
		if ( num == TX_BADSTRING     ) strcpy(text, "Une chaîne de caractère est attendue");
		if ( num == TX_BADINDEX      ) strcpy(text, "Mauvais type d'index");
		if ( num == TX_PRIVATE       ) strcpy(text, "Elément protégé");
		if ( num == TX_NOPUBLIC      ) strcpy(text, "Public requis");
		if ( num == TX_DIVZERO       ) strcpy(text, "Division par zéro");
		if ( num == TX_NOTINIT       ) strcpy(text, "Variable non initialisée");
		if ( num == TX_BADTHROW      ) strcpy(text, "Valeur négative refusée pour ""throw""");
		if ( num == TX_NORETVAL      ) strcpy(text, "La fonction n'a pas retourné de résultat");
		if ( num == TX_NORUN         ) strcpy(text, "Pas de fonction en exécution");
		if ( num == TX_NOCALL        ) strcpy(text, "Appel d'une fonction inexistante");
		if ( num == TX_NOCLASS       ) strcpy(text, "Cette classe n'existe pas");
		if ( num == TX_NULLPT        ) strcpy(text, "Objet n'existe pas");
		if ( num == TX_OPNAN         ) strcpy(text, "Opération sur un ""nan""");
		if ( num == TX_OUTARRAY      ) strcpy(text, "Accès hors du tableau");
		if ( num == TX_STACKOVER     ) strcpy(text, "Débordement de la pile");
		if ( num == TX_DELETEDPT     ) strcpy(text, "Objet inaccessible");
		if ( num == TX_FILEOPEN      ) strcpy(text, "Ouverture du fichier impossible");
		if ( num == TX_NOTOPEN       ) strcpy(text, "Le fichier n'est pas ouvert");
		if ( num == TX_ERRREAD       ) strcpy(text, "Erreur à la lecture");
		if ( num == TX_ERRWRITE      ) strcpy(text, "Erreur à l'écriture");
	}

	if ( type == RES_KEY )
	{
		if ( num == 0                   )  strcpy(text, "< aucune >");
		if ( num == VK_LEFT             )  strcpy(text, "Flèche Gauche");
		if ( num == VK_RIGHT            )  strcpy(text, "Flèche Droite");
		if ( num == VK_UP               )  strcpy(text, "Flèche Haut");
		if ( num == VK_DOWN             )  strcpy(text, "Flèche Bas");
		if ( num == VK_CANCEL           )  strcpy(text, "Control-break");
		if ( num == VK_BACK             )  strcpy(text, "<--");
		if ( num == VK_TAB              )  strcpy(text, "Tab");
		if ( num == VK_CLEAR            )  strcpy(text, "Clear");
		if ( num == VK_RETURN           )  strcpy(text, "Entrée");
		if ( num == VK_SHIFT            )  strcpy(text, "Shift");
		if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
		if ( num == VK_MENU             )  strcpy(text, "Alt");
		if ( num == VK_PAUSE            )  strcpy(text, "Pause");
		if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
		if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
		if ( num == VK_SPACE            )  strcpy(text, "Espace");
		if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
		if ( num == VK_NEXT             )  strcpy(text, "Page Down");
		if ( num == VK_END              )  strcpy(text, "End");
		if ( num == VK_HOME             )  strcpy(text, "Home");
		if ( num == VK_SELECT           )  strcpy(text, "Select");
		if ( num == VK_EXECUTE          )  strcpy(text, "Execute");
		if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
		if ( num == VK_INSERT           )  strcpy(text, "Insert");
		if ( num == VK_DELETE           )  strcpy(text, "Delete");
		if ( num == VK_HELP             )  strcpy(text, "Help");
		if ( num == VK_LWIN             )  strcpy(text, "Left Windows");
		if ( num == VK_RWIN             )  strcpy(text, "Right Windows");
		if ( num == VK_APPS             )  strcpy(text, "Application key");
		if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
		if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
		if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
		if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
		if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
		if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
		if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
		if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
		if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
		if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
		if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
		if ( num == VK_ADD              )  strcpy(text, "NumPad +");
		if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
		if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
		if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
		if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
		if ( num == VK_F1               )  strcpy(text, "F1");
		if ( num == VK_F2               )  strcpy(text, "F2");
		if ( num == VK_F3               )  strcpy(text, "F3");
		if ( num == VK_F4               )  strcpy(text, "F4");
		if ( num == VK_F5               )  strcpy(text, "F5");
		if ( num == VK_F6               )  strcpy(text, "F6");
		if ( num == VK_F7               )  strcpy(text, "F7");
		if ( num == VK_F8               )  strcpy(text, "F8");
		if ( num == VK_F9               )  strcpy(text, "F9");
		if ( num == VK_F10              )  strcpy(text, "F10");
		if ( num == VK_F11              )  strcpy(text, "F11");
		if ( num == VK_F12              )  strcpy(text, "F12");
		if ( num == VK_F13              )  strcpy(text, "F13");
		if ( num == VK_F14              )  strcpy(text, "F14");
		if ( num == VK_F15              )  strcpy(text, "F15");
		if ( num == VK_F16              )  strcpy(text, "F16");
		if ( num == VK_F17              )  strcpy(text, "F17");
		if ( num == VK_F18              )  strcpy(text, "F18");
		if ( num == VK_F19              )  strcpy(text, "F19");
		if ( num == VK_F20              )  strcpy(text, "F20");
		if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
		if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
		if ( num == VK_ATTN             )  strcpy(text, "Attn");
		if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
		if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
		if ( num == VK_EREOF            )  strcpy(text, "Erase EOF");
		if ( num == VK_PLAY             )  strcpy(text, "Play");
		if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
		if ( num == VK_PA1              )  strcpy(text, "PA1");
		if ( num == VK_OEM_CLEAR        )  strcpy(text, "Clear");
		if ( num == VK_BUTTON1          )  strcpy(text, "Bouton 1");
		if ( num == VK_BUTTON2          )  strcpy(text, "Bouton 2");
		if ( num == VK_BUTTON3          )  strcpy(text, "Bouton 3");
		if ( num == VK_BUTTON4          )  strcpy(text, "Bouton 4");
		if ( num == VK_BUTTON5          )  strcpy(text, "Bouton 5");
		if ( num == VK_BUTTON6          )  strcpy(text, "Bouton 6");
		if ( num == VK_BUTTON7          )  strcpy(text, "Bouton 7");
		if ( num == VK_BUTTON8          )  strcpy(text, "Bouton 8");
		if ( num == VK_BUTTON9          )  strcpy(text, "Bouton 9");
		if ( num == VK_BUTTON10         )  strcpy(text, "Bouton 10");
		if ( num == VK_BUTTON11         )  strcpy(text, "Bouton 11");
		if ( num == VK_BUTTON12         )  strcpy(text, "Bouton 12");
		if ( num == VK_BUTTON13         )  strcpy(text, "Bouton 13");
		if ( num == VK_BUTTON14         )  strcpy(text, "Bouton 14");
		if ( num == VK_BUTTON15         )  strcpy(text, "Bouton 15");
		if ( num == VK_BUTTON16         )  strcpy(text, "Bouton 16");
		if ( num == VK_BUTTON17         )  strcpy(text, "Bouton 17");
		if ( num == VK_BUTTON18         )  strcpy(text, "Bouton 18");
		if ( num == VK_BUTTON19         )  strcpy(text, "Bouton 19");
		if ( num == VK_BUTTON20         )  strcpy(text, "Bouton 20");
		if ( num == VK_BUTTON21         )  strcpy(text, "Bouton 21");
		if ( num == VK_BUTTON22         )  strcpy(text, "Bouton 22");
		if ( num == VK_BUTTON23         )  strcpy(text, "Bouton 23");
		if ( num == VK_BUTTON24         )  strcpy(text, "Bouton 24");
		if ( num == VK_BUTTON25         )  strcpy(text, "Bouton 25");
		if ( num == VK_BUTTON26         )  strcpy(text, "Bouton 26");
		if ( num == VK_BUTTON27         )  strcpy(text, "Bouton 27");
		if ( num == VK_BUTTON28         )  strcpy(text, "Bouton 28");
		if ( num == VK_BUTTON29         )  strcpy(text, "Bouton 29");
		if ( num == VK_BUTTON30         )  strcpy(text, "Bouton 30");
		if ( num == VK_BUTTON31         )  strcpy(text, "Bouton 31");
		if ( num == VK_BUTTON32         )  strcpy(text, "Bouton 32");
		if ( num == VK_WHEELUP          )  strcpy(text, "Molette haut");
		if ( num == VK_WHEELDOWN        )  strcpy(text, "Molette bas");
	}
#endif

#if _ITALIAN
	if ( type == RES_TEXT )
	{
		#if _FULL
		#if _EDU
		if ( num == RT_VERSION_ID          )  strcpy(text, "EDU 1.3 /i");
		#else
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /i");
		#endif
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /i");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Ingrandisci");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Riduci a icona");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Dimensioni normali");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Chiudi");

		if ( num == RT_KEY_OR              )  strcpy(text, " o ");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Seleziona una missione:");
		if ( num == RT_PLAY_LISTd          )  strcpy(text, " Défis :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Atelier de %s :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototipi:");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Driver:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Risoluzione:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Fai prima clic sul tasto da reimpostare.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Quindi premi il tasto che desideri utilizzare in alternativa.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Elenco piloti:");

		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Uscire dal gioco?");
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Interrompi\\Interrompi missione corrente");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continua\\Continua missione corrente");
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Esci\\Esci da BlupiMania");
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continua\\Continua gioco");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Eliminare i giochi e le auto di %s salvati?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Eliminare il file %s esistente?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Elimina");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Annulla");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nome pilota:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Crea");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Annulla");
		if ( num == RT_DIALOG_PLAY         )  strcpy(text, "Résoudre le puzzle...");
		if ( num == RT_DIALOG_REPLAY       )  strcpy(text, "Rejouer pour le plaisir...");
		if ( num == RT_DIALOG_QUITEDIT     )  strcpy(text, "Enregistrer les modifications ?");
		if ( num == RT_DIALOG_YESQUITEDIT  )  strcpy(text, "Oui\\Enregistrer et quitter");
		if ( num == RT_DIALOG_NOQUITEDIT   )  strcpy(text, "Non\\Quitter sans enregistrer");
		if ( num == RT_DIALOG_CANQUITEDIT  )  strcpy(text, "Annuler\\Continuer l'édition");
		if ( num == RT_DIALOG_NEWPUZZLE    )  strcpy(text, "Voulez-vous créer un nouveau puzzle \"%s\" ?");
		if ( num == RT_DIALOG_YESNEWP      )  strcpy(text, "Créer");
		if ( num == RT_DIALOG_NONEWP       )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_DELPUZ       )  strcpy(text, "Voulez-vous supprimer le puzzle ?");
		if ( num == RT_DIALOG_DELPUZd      )  strcpy(text, "(pour tous les joueurs)");
		if ( num == RT_DIALOG_DELPUZu      )  strcpy(text, "(seulement dans votre atelier)");
		if ( num == RT_DIALOG_YESDELPUZ    )  strcpy(text, "Supprimer");
		if ( num == RT_DIALOG_NODELPUZ     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RESUMEINFO   )  strcpy(text, "Instructions");
		if ( num == RT_DIALOG_AUTHORINFO   )  strcpy(text, "Auteur");
		if ( num == RT_DIALOG_SIGNINFO     )  strcpy(text, "Indices");
		if ( num == RT_DIALOG_YESINFO      )  strcpy(text, "OK");
		if ( num == RT_DIALOG_YESERROR     )  strcpy(text, "OK");
		if ( num == RT_DIALOG_EXPORTGAME   )  strcpy(text, "Où voulez-vous exporter le casse-tête ?");
		if ( num == RT_DIALOG_YESEXPORT    )  strcpy(text, "Exporter");
		if ( num == RT_DIALOG_NOEXPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Casse-tête contenus dans \"Mes documents\" :");
		if ( num == RT_DIALOG_YESIMPORT    )  strcpy(text, "Importer");
		if ( num == RT_DIALOG_NOIMPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_TEST         )  strcpy(text, "Tester");
		if ( num == RT_DIALOG_DEFI         )  strcpy(text, "Relever le défi...");
		if ( num == RT_DIALOG_FILE         )  strcpy(text, "Fichier");
		if ( num == RT_DIALOG_RENAMEGAME   )  strcpy(text, "Changement du nom du fichier du casse-tête.");
		if ( num == RT_DIALOG_YESRENAME    )  strcpy(text, "Renommer");
		if ( num == RT_DIALOG_NORENAME     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RENAMEOLD    )  strcpy(text, "Nom actuel");
		if ( num == RT_DIALOG_RENAMENEW    )  strcpy(text, "Nouveau nom");

		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.epsitec.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Sviluppato da EPSITEC");
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Hai giocato con la versione DEMO di BlupiMania.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "È ora disponibile la versione completa...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... contiene 28 avvincenti missioni e 9 gare ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... nonché una modalità \"sfida\" per mettere a confronto le prestazioni.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Per ulteriori dettagli visitare il sito Web www.epsitec.com !");

		if ( num == RT_ACTION_GOTO         )  strcpy(text, "Aller");
		if ( num == RT_ACTION_PUSH1        )  strcpy(text, "Pousser une fois");
		if ( num == RT_ACTION_PUSHx        )  strcpy(text, "Pousser %d fois");
		if ( num == RT_ACTION_PUSHn        )  strcpy(text, "Pousser toujours");
		if ( num == RT_ACTION_PULL1        )  strcpy(text, "Tirer une fois");
		if ( num == RT_ACTION_ROLL         )  strcpy(text, "Rouler");
		if ( num == RT_ACTION_DOCKm        )  strcpy(text, "Actionner");
		if ( num == RT_ACTION_DOCKmx       )  strcpy(text, "Actionner %d fois");
		if ( num == RT_ACTION_DOCKg        )  strcpy(text, "Saisir/déposer");
		if ( num == RT_ACTION_TRAXa        )  strcpy(text, "Avancer");
		if ( num == RT_ACTION_TRAXax       )  strcpy(text, "Avancer %d fois");
		if ( num == RT_ACTION_TRAXl        )  strcpy(text, "Tourner à droite");
		if ( num == RT_ACTION_TRAXlx       )  strcpy(text, "Tourner à droite %d fois");
		if ( num == RT_ACTION_TRAXr        )  strcpy(text, "Tourner à gauche");
		if ( num == RT_ACTION_TRAXrx       )  strcpy(text, "Tourner à gauche %d fois");
		if ( num == RT_ACTION_GUNa         )  strcpy(text, "Avancer puis tirer");
		if ( num == RT_ACTION_GUNax        )  strcpy(text, "Avancer puis tirer %d fois");
		if ( num == RT_ACTION_GUNl         )  strcpy(text, "Tourner à droite puis tirer");
		if ( num == RT_ACTION_GUNlx        )  strcpy(text, "Tourner à droite puis tirer %d fois");
		if ( num == RT_ACTION_GUNr         )  strcpy(text, "Tourner à gauche puis tirer");
		if ( num == RT_ACTION_GUNrx        )  strcpy(text, "Tourner à gauche puis tirer %d fois");
		if ( num == RT_ACTION_CATAPULT     )  strcpy(text, "Lancer");

		if ( num == RT_ADVISE_LITGOTO      )  strcpy(text, "...");
		if ( num == RT_ADVISE_GOTOPUSH     )  strcpy(text, "...");
		if ( num == RT_ADVISE_NOSIGN       )  strcpy(text, "Aucun indice disponible pour ce casse-tête.");

		if ( num == RT_USER_HEADER         )  strcpy(text, "Fichier\tUnivers\tTesté\tInstructions");
		if ( num == RT_DEFI_HEADER         )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");
		if ( num == RT_IMPORT_HEADER       )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");

		if ( num == RT_DEMO                )  strcpy(text, "Pas disponible dans la DEMO");

		if ( num == RT_UNIVERS0            )  strcpy(text, "Castella");
		if ( num == RT_UNIVERS1            )  strcpy(text, "Paradiso");
		if ( num == RT_UNIVERS2            )  strcpy(text, "Volcano");
		if ( num == RT_UNIVERS3            )  strcpy(text, "Mecano");
		if ( num == RT_UNIVERS4            )  strcpy(text, "Sahara");
		if ( num == RT_UNIVERS5            )  strcpy(text, "Aglagla");
		if ( num == RT_UNIVERS6            )  strcpy(text, "Samba");
		if ( num == RT_UNIVERS7            )  strcpy(text, "Medieva");
		if ( num == RT_UNIVERS8            )  strcpy(text, "Vegeta");
		if ( num == RT_UNIVERS9            )  strcpy(text, "Inferno");
		if ( num == RT_UNIVERS10           )  strcpy(text, "Futura");
		if ( num == RT_UNIVERS11           )  strcpy(text, "Alibaba");
		if ( num == RT_UNIVERS12           )  strcpy(text, "Studio");
		if ( num == RT_UNIVERS13           )  strcpy(text, "Cata");
		if ( num == RT_UNIVERS14           )  strcpy(text, "Allegro");
		if ( num == RT_UNIVERS15           )  strcpy(text, "Cemento");
		if ( num == RT_UNIVERS16           )  strcpy(text, "Labo");
		if ( num == RT_UNIVERS17           )  strcpy(text, "Drinka");
		if ( num == RT_UNIVERS18           )  strcpy(text, "Woodo");
		if ( num == RT_UNIVERS19           )  strcpy(text, "Natura");
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Annulla");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Annulla");

		if ( num == EVENT_INTERFACE_PUZZLE )  strcpy(text, "Casse-tête\\Torture progressive");
		if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Défis\\Casse-tête d'autres joueurs");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Atelier\\Créez vos casse-tête");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Prototipi\\Prototipi in sviluppo");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Nuovo pilota\\Scegli nome pilota");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opzioni\\Preferenze");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Riavvia\\Riavvia la missione dall'inizio");
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Torna a BlupiMania");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Esci\\Esci da BlupiMania");
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Annulla\\Torna alla schermata precedente");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Torna al menu principale");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Gioca\\Inizia la missione selezionata");
		if ( num == EVENT_INTERFACE_NEW    )  strcpy(text, "Nouveau\\Créer un nouveau puzzle");
		if ( num == EVENT_INTERFACE_DELETE )  strcpy(text, "Supprimer\\Supprimer le puzzle");
		if ( num == EVENT_INTERFACE_RENAME )  strcpy(text, "Renommer\\Renommer le casse-tête");
		if ( num == EVENT_INTERFACE_EXPORT )  strcpy(text, "Exporter\\Exporter le casse-tête");
		if ( num == EVENT_INTERFACE_IMPORT )  strcpy(text, "Importer\\Importer un casse-tête");
		if ( num == EVENT_INTERFACE_EDIT   )  strcpy(text, "Modifier\\Modifier le puzzle");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Torna alla schermata precedente");

		if ( num == EVENT_INTERFACE_EXPORTdefi )  strcpy(text, "Dans les défis (pour tous les joueurs)");
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "Dans le dossier \"Mes documents\"");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Dispositivo\\Impostazioni driver e risoluzione");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Grafica\\Impostazioni grafiche");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Gioco\\Impostazioni di gioco");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controlli\\Impostazioni tastiera, volante e joypad");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Audio\\Volume di musica ed effetti sonori");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Risoluzione");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Schermo intero\\Schermo intero o modalità finestra");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Applica modifiche\\Attiva le impostazioni modificate");

		if ( num == EVENT_INTERFACE_SHADOW      )  strcpy(text, "Ombres\\Ombres projetées au sol");
		if ( num == EVENT_INTERFACE_DIRTY       )  strcpy(text, "Salissures\\Salissures des object");
		if ( num == EVENT_INTERFACE_SUNBEAM     )  strcpy(text, "Rayons du soleil\\Rayons selon l'orientation");
		if ( num == EVENT_INTERFACE_LENSFLARE   )  strcpy(text, "Reflets dans l'objectif\\Reflets selon l'orientation");
		if ( num == EVENT_INTERFACE_DECOR       )  strcpy(text, "Objets décoratifs\\Arbres, plantes, etc.");
		if ( num == EVENT_INTERFACE_DETAIL      )  strcpy(text, "Objets détaillés\\Objets détaillés");
		if ( num == EVENT_INTERFACE_METEO       )  strcpy(text, "Conditions météo\\Pluie et neige");
		if ( num == EVENT_INTERFACE_AMBIANCE    )  strcpy(text, "Effets d'ambiance\\");
		if ( num == EVENT_INTERFACE_EXPLOVIB    )  strcpy(text, "Secousses lors d'explosions\\L'écran vibre lors d'une explosion");
		if ( num == EVENT_INTERFACE_SPEEDSCH    )  strcpy(text, "Vitesse de défilement horizontal\\");
		if ( num == EVENT_INTERFACE_SPEEDSCV    )  strcpy(text, "Vitesse de défilement vertical\\");
		if ( num == EVENT_INTERFACE_MOUSESCROLL )  strcpy(text, "Défilement avec la souris\\");
		if ( num == EVENT_INTERFACE_INVSCH      )  strcpy(text, "Inversion défilement horizontal\\");
		if ( num == EVENT_INTERFACE_INVSCV      )  strcpy(text, "Inversion défilement vertical\\");
		if ( num == EVENT_INTERFACE_MOVIE       )  strcpy(text, "Séquences cinématiques\\Séquences d'introduction");
		if ( num == EVENT_INTERFACE_HELP        )  strcpy(text, "Instructions et indices\\Textes explicatifs");
		if ( num == EVENT_INTERFACE_TOOLTIPS    )  strcpy(text, "Bulles d'aide\\Bulles explicatives");
		if ( num == EVENT_INTERFACE_NICEMOUSE   )  strcpy(text, "Souris ombrée\\Jolie souris avec une ombre");
		if ( num == EVENT_INTERFACE_ACCEL       )  strcpy(text, "Accélère si long à résoudre\\Blupi va plus vite après 10 minutes de réflexion");
		if ( num == EVENT_INTERFACE_VOLBLUPI    )  strcpy(text, "Volume des voix de Blupi\\");
		if ( num == EVENT_INTERFACE_VOLSOUND    )  strcpy(text, "Volume des bruitages\\");
		if ( num == EVENT_INTERFACE_VOLAMBIANCE )  strcpy(text, "Volume d'ambiance\\");
		if ( num == EVENT_INTERFACE_SOUND3D     )  strcpy(text, "Suono 3D\\Suono 3D");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Controlli predefiniti\\Ripristina le impostazioni predefinite dei controlli");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Scroll left\\Scrolling to left");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Scroll right\\Scrolling to right");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Scroll up\\Scrolling to up");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Scroll down\\Scrolling to down");
		if ( num == EVENT_INTERFACE_KROTCW )  strcpy(text, "Rotate CW\\Rotate camera clockwise");
		if ( num == EVENT_INTERFACE_KROTCCW)  strcpy(text, "Rotate CCW\\Rotate camera counter clockwise");
		if ( num == EVENT_INTERFACE_KSTOP  )  strcpy(text, "Stoppe action\\Stoppe l'action en cours");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Esci\\Esci dalla missione o dalla gara corrente");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instruzioni\\Visualizza le istruzioni della missione corrente");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Minima\\Qualità grafica minima (frequenza di quadro massima)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normale\\Qualità grafica media");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Massima\\Qualità grafica massima (frequenza di quadro minima)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silenzio\\Disattivazione dell'audio");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normale\\Volume dell'audio normale");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nome nuovo pilota");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Seleziona\\Scegli il pilota selezionato");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Nuovo\\Crea un nuovo pilota");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Elimina\\Elimina il pilota dall'elenco");

		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Previous screen");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Next screen");
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit BlupiMania");

		if ( num == EVENT_INTERFACE_UNDO   )  strcpy(text, "Annule la dernière action");
		if ( num == EVENT_INTERFACE_STOP   )  strcpy(text, "Stop (\\key stop;)");
		if ( num == EVENT_INTERFACE_SIGN   )  strcpy(text, "Obtenir des indices");
		if ( num == EVENT_LABEL_SIGN       )  strcpy(text, "Indices");

		if ( num == EVENT_EDIT_WATER       )  strcpy(text, "Eau");
		if ( num == EVENT_EDIT_GROUND      )  strcpy(text, "Sol");
		if ( num == EVENT_EDIT_OBJECT      )  strcpy(text, "Objets");
		if ( num == EVENT_EDIT_INFO        )  strcpy(text, "Informations sur le puzzle");

		if ( num == EVENT_CMD              )  strcpy(text, "Riga di comando");
		if ( num == EVENT_SPEED            )  strcpy(text, "Velocità gioco");
	}

	if ( type == RES_OBJECT )
	{
		if ( num == OBJECT_BLUPI           )  strcpy(text, "Blupi");
		if ( num == OBJECT_BOX1            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX2            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX3            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX4            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX5            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX6            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX7            )  strcpy(text, "Grosse boule");
		if ( num == OBJECT_BOX8            )  strcpy(text, "Colonne lourde");
		if ( num == OBJECT_BOX9            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX10           )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX11           )  strcpy(text, "Caisse sur roues");
		if ( num == OBJECT_BOX12           )  strcpy(text, "Caisse sur roues");
		if ( num == OBJECT_BOX13           )  strcpy(text, "Caisse sur coussin d'air");
		if ( num == OBJECT_KEY1            )  strcpy(text, "Clé orange");
		if ( num == OBJECT_KEY2            )  strcpy(text, "Clé bleue");
		if ( num == OBJECT_DOOR1           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR2           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOOR3           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR4           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOCK            )  strcpy(text, "Pont-grue");
		if ( num == OBJECT_CATAPULT        )  strcpy(text, "Catapulte");
		if ( num == OBJECT_TRAX            )  strcpy(text, "Pousseur mécanique");
		if ( num == OBJECT_PERFO           )  strcpy(text, "Perforateur mécanique");
		if ( num == OBJECT_GUN             )  strcpy(text, "Destructeur de caisses");
		if ( num == OBJECT_MINE            )  strcpy(text, "Mine");
		if ( num == OBJECT_GLASS1          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_GLASS2          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_FIOLE           )  strcpy(text, "Potion de force");
		if ( num == OBJECT_GLU             )  strcpy(text, "Potion de glu");
		if ( num == OBJECT_GOAL            )  strcpy(text, "But à atteindre");
		if ( num == OBJECT_CRAZY           )  strcpy(text, "Crazy bot");
		if ( num == OBJECT_BOT1            )  strcpy(text, "Bot 1");
		if ( num == OBJECT_BOT2            )  strcpy(text, "Bot 2");
		if ( num == OBJECT_BOT3            )  strcpy(text, "Bot 3");
		if ( num == OBJECT_BOT4            )  strcpy(text, "Bot 4");
		if ( num == OBJECT_BOT5            )  strcpy(text, "Bot 5");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Errore");
		if ( num == ERR_CMD             )  strcpy(text, "Comando sconosciuto");
		if ( num == ERR_INSTALL         )  strcpy(text, "Wild Wheels non è stato installato correttamente.");
		if ( num == ERR_NOCD            )  strcpy(text, "Inserire il CD Wild Wheels\ne rieseguire il programma del gioco.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Errore nel trasferimento di istruzioni");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Vai a: destinazione inaccessibile");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Vai a: destinazione inaccessibile");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Vai a: destinazione occupata");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Bot errato");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Energia insufficiente");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "La missione non è stata ancora completata (premi \\key help; per ulteriori dettagli)");
		if ( num == ERR_PUZZLE_ZEROBLUPI)  strcpy(text, "Il doit y avoir au moins un Blupi !");
		if ( num == ERR_PUZZLE_ZEROGOAL )  strcpy(text, "Il doit y avoir au moins un ballon !");
		if ( num == ERR_PUZZLE_LOGOAL   )  strcpy(text, "Il n'y a pas assez de ballons !");
		if ( num == ERR_PUZZLE_HIGOAL   )  strcpy(text, "Il y a trop de ballons !");
		if ( num == ERR_PUZZLE_MAXBLUPI )  strcpy(text, "4 Blupi ou robots au maximum par puzzle !");
		if ( num == ERR_PUZZLE_MAXGOAL  )  strcpy(text, "8 ballons au maximum par puzzle !");
		if ( num == ERR_PUZZLE_MAX      )  strcpy(text, "Il n'est pas possible de créer plus d'objets de ce type !");
		if ( num == ERR_CREATE          )  strcpy(text, "Ce fichier existe déjà.");

		if ( num == INFO_WIN            )  strcpy(text, "<< Congratulazioni, missione compiuta >>");
		if ( num == INFO_LOST           )  strcpy(text, "<< Spiacente, missione non riuscita >>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<< Spiacente, missione non riuscita >>");
	}

	if ( type == RES_CBOT )
	{
		strcpy(text, "Errore");
		if ( num == TX_OPENPAR       ) strcpy(text, "Parentesi di apertura mancante");
		if ( num == TX_CLOSEPAR      ) strcpy(text, "Parentesi di chiusura mancante ");
		if ( num == TX_NOTBOOL       ) strcpy(text, "L'espressione deve restituire un valore boleano");
		if ( num == TX_UNDEFVAR      ) strcpy(text, "Variabile non dichiarata");
		if ( num == TX_BADLEFT       ) strcpy(text, "Assegnazione impossibile");
		if ( num == TX_ENDOF         ) strcpy(text, "Terminatore punto e virgola mancante");
		if ( num == TX_OUTCASE       ) strcpy(text, "Istruzione ""case"" fuori del blocco ""switch""");
		if ( num == TX_NOTERM        ) strcpy(text, "Istruzioni dopo la parentesi graffa di chiusura finale");
		if ( num == TX_CLOSEBLK      ) strcpy(text, "Fine del blocco mancante");
		if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Istruzione ""else"" senza corrispondente ""if"" ");
		if ( num == TX_OPENBLK       ) strcpy(text, "Parentesi graffa di apertura mancante ");//début d'un bloc attendu?
		if ( num == TX_BADTYPE       ) strcpy(text, "Tipo di assegnazione errato");
		if ( num == TX_REDEFVAR      ) strcpy(text, "Non è possibile dichiarare due volte una variabile");
		if ( num == TX_BAD2TYPE      ) strcpy(text, "Le tipologie dei due operandi non sono compatibili");
		if ( num == TX_UNDEFCALL     ) strcpy(text, "Funzione sconosciuta");
		if ( num == TX_MISDOTS       ) strcpy(text, "Segno "" : "" mancante");
		if ( num == TX_WHILE         ) strcpy(text, "Parola chiave ""while"" mancante");
		if ( num == TX_BREAK         ) strcpy(text, "Istruzione ""break"" fuori del cliclo");
		if ( num == TX_LABEL         ) strcpy(text, "Un'etichetta deve esser seguita da ""for"", ""while"", ""do"" o ""switch""");
		if ( num == TX_NOLABEL       ) strcpy(text, "Etichetta inesistente");// Cette étiquette n'existe pas
		if ( num == TX_NOCASE        ) strcpy(text, "Istruzione ""case"" mancante");
		if ( num == TX_BADNUM        ) strcpy(text, "Numero mancante");
		if ( num == TX_VOID          ) strcpy(text, "Parametro nullo");
		if ( num == TX_NOTYP         ) strcpy(text, "Dichiarazione Type mancante");
		if ( num == TX_NOVAR         ) strcpy(text, "Nome variabile mancante");
		if ( num == TX_NOFONC        ) strcpy(text, "Nome funzione mancante");
		if ( num == TX_OVERPARAM     ) strcpy(text, "Parametri in eccesso");
		if ( num == TX_REDEF         ) strcpy(text, "La funzione esiste già");
		if ( num == TX_LOWPARAM      ) strcpy(text, "Parametri mancanti ");
		if ( num == TX_BADPARAM      ) strcpy(text, "Nessuna funzione con questo nome accetta questo tipo di parametro");
		if ( num == TX_NUMPARAM      ) strcpy(text, "Nessuna funzione con questo nome accetta questo numero di parametri");
		if ( num == TX_NOITEM        ) strcpy(text, "Non appartiene a questa classe");
		if ( num == TX_DOT           ) strcpy(text, "Questo oggetto non appartiene ad alcuna classe");
		if ( num == TX_NOCONST       ) strcpy(text, "Costruttore appropriato mancante");
		if ( num == TX_REDEFCLASS    ) strcpy(text, "Questa classe esiste già");
		if ( num == TX_CLBRK         ) strcpy(text, """ ] "" mancante");
		if ( num == TX_RESERVED      ) strcpy(text, "Parola chiave riservata del linguaggio CBOT");
		if ( num == TX_BADNEW        ) strcpy(text, "Argomento errato per ""new""");
		if ( num == TX_OPBRK         ) strcpy(text, """ [ "" previsto");
		if ( num == TX_BADSTRING     ) strcpy(text, "Stringa mancante");
		if ( num == TX_BADINDEX      ) strcpy(text, "Tipo di indice errato");
		if ( num == TX_PRIVATE       ) strcpy(text, "Elemento privato");
		if ( num == TX_NOPUBLIC      ) strcpy(text, "Richiesto pubblico");
		if ( num == TX_DIVZERO       ) strcpy(text, "Divisione per zero");
		if ( num == TX_NOTINIT       ) strcpy(text, "Variabile non inizializzata");
		if ( num == TX_BADTHROW      ) strcpy(text, "Valore negativo respinto da ""throw""");//C'est quoi, ça?
		if ( num == TX_NORETVAL      ) strcpy(text, "La funzione non ha restituito alcun valore ");
		if ( num == TX_NORUN         ) strcpy(text, "Nessuna funzione in esecuzione");
		if ( num == TX_NOCALL        ) strcpy(text, "Richiamo di una funzione sconosciuta");
		if ( num == TX_NOCLASS       ) strcpy(text, "Classe inesistente");
		if ( num == TX_NULLPT        ) strcpy(text, "Oggetto sconosciuto");
		if ( num == TX_OPNAN         ) strcpy(text, "Operazione impossibile con il valore ""nan""");
		if ( num == TX_OUTARRAY      ) strcpy(text, "Accesso oltre il limite della matrice");
		if ( num == TX_STACKOVER     ) strcpy(text, "Overflow dello stack");
		if ( num == TX_DELETEDPT     ) strcpy(text, "Oggetto non valido");
		if ( num == TX_FILEOPEN      ) strcpy(text, "Impossibile aprire il file");
		if ( num == TX_NOTOPEN       ) strcpy(text, "Il file non è aperto");
		if ( num == TX_ERRREAD       ) strcpy(text, "Errore di lettura");
		if ( num == TX_ERRWRITE      ) strcpy(text, "Errore di scrittura");
	}

	if ( type == RES_KEY )
	{
		if ( num == 0                   )  strcpy(text, "< nessuno >");
		if ( num == VK_LEFT             )  strcpy(text, "Freccia a sinistra");
		if ( num == VK_RIGHT            )  strcpy(text, "Freccia a destra");
		if ( num == VK_UP               )  strcpy(text, "Freccia su");
		if ( num == VK_DOWN             )  strcpy(text, "Freccia giù");
		if ( num == VK_CANCEL           )  strcpy(text, "CTRL+INTER");
		if ( num == VK_BACK             )  strcpy(text, "<--");
		if ( num == VK_TAB              )  strcpy(text, "Tabulazione");
		if ( num == VK_CLEAR            )  strcpy(text, "Cancella");
		if ( num == VK_RETURN           )  strcpy(text, "Invio");
		if ( num == VK_SHIFT            )  strcpy(text, "Maiusc");
		if ( num == VK_CONTROL          )  strcpy(text, "CTRL");
		if ( num == VK_MENU             )  strcpy(text, "Alt");
		if ( num == VK_PAUSE            )  strcpy(text, "Pausa");
		if ( num == VK_CAPITAL          )  strcpy(text, "Bloc Maiusc");
		if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
		if ( num == VK_SPACE            )  strcpy(text, "Barra spaziatrice");
		if ( num == VK_PRIOR            )  strcpy(text, "PGSU");
		if ( num == VK_NEXT             )  strcpy(text, "PGGIÙ");
		if ( num == VK_END              )  strcpy(text, "Fine");
		if ( num == VK_HOME             )  strcpy(text, "Home");
		if ( num == VK_SELECT           )  strcpy(text, "Seleziona");
		if ( num == VK_EXECUTE          )  strcpy(text, "Esegui");
		if ( num == VK_SNAPSHOT         )  strcpy(text, "Stamp");
		if ( num == VK_INSERT           )  strcpy(text, "Ins");
		if ( num == VK_DELETE           )  strcpy(text, "Canc");
		if ( num == VK_HELP             )  strcpy(text, "?");
		if ( num == VK_LWIN             )  strcpy(text, "Finestre di sinistra");
		if ( num == VK_RWIN             )  strcpy(text, "Finestre di destra");
		if ( num == VK_APPS             )  strcpy(text, "Tasto Applicazione");
		if ( num == VK_NUMPAD0          )  strcpy(text, "Tast. num 0");
		if ( num == VK_NUMPAD1          )  strcpy(text, "Tast. num 1");
		if ( num == VK_NUMPAD2          )  strcpy(text, "Tast. num 2");
		if ( num == VK_NUMPAD3          )  strcpy(text, "Tast. num 3");
		if ( num == VK_NUMPAD4          )  strcpy(text, "Tast. num 4");
		if ( num == VK_NUMPAD5          )  strcpy(text, "Tast. num 5");
		if ( num == VK_NUMPAD6          )  strcpy(text, "Tast. num 6");
		if ( num == VK_NUMPAD7          )  strcpy(text, "Tast. num 7");
		if ( num == VK_NUMPAD8          )  strcpy(text, "Tast. num 8");
		if ( num == VK_NUMPAD9          )  strcpy(text, "Tast. num 9");
		if ( num == VK_MULTIPLY         )  strcpy(text, "Tast. num *");
		if ( num == VK_ADD              )  strcpy(text, "Tast. num +");
		if ( num == VK_SEPARATOR        )  strcpy(text, "Separatore tast. num");
		if ( num == VK_SUBTRACT         )  strcpy(text, "Tast. num -");
		if ( num == VK_DECIMAL          )  strcpy(text, "Tast. num .");
		if ( num == VK_DIVIDE           )  strcpy(text, "Tast. num /");
		if ( num == VK_F1               )  strcpy(text, "F1");
		if ( num == VK_F2               )  strcpy(text, "F2");
		if ( num == VK_F3               )  strcpy(text, "F3");
		if ( num == VK_F4               )  strcpy(text, "F4");
		if ( num == VK_F5               )  strcpy(text, "F5");
		if ( num == VK_F6               )  strcpy(text, "F6");
		if ( num == VK_F7               )  strcpy(text, "F7");
		if ( num == VK_F8               )  strcpy(text, "F8");
		if ( num == VK_F9               )  strcpy(text, "F9");
		if ( num == VK_F10              )  strcpy(text, "F10");
		if ( num == VK_F11              )  strcpy(text, "F11");
		if ( num == VK_F12              )  strcpy(text, "F12");
		if ( num == VK_F13              )  strcpy(text, "F13");
		if ( num == VK_F14              )  strcpy(text, "F14");
		if ( num == VK_F15              )  strcpy(text, "F15");
		if ( num == VK_F16              )  strcpy(text, "F16");
		if ( num == VK_F17              )  strcpy(text, "F17");
		if ( num == VK_F18              )  strcpy(text, "F18");
		if ( num == VK_F19              )  strcpy(text, "F19");
		if ( num == VK_F20              )  strcpy(text, "F20");
		if ( num == VK_NUMLOCK          )  strcpy(text, "Bloc Num");
		if ( num == VK_SCROLL           )  strcpy(text, "Bloc Scorr");
		if ( num == VK_ATTN             )  strcpy(text, "Attn");
		if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
		if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
		if ( num == VK_EREOF            )  strcpy(text, "Elimina EOF");
		if ( num == VK_PLAY             )  strcpy(text, "Gioca");
		if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
		if ( num == VK_PA1              )  strcpy(text, "PA1");
		if ( num == VK_OEM_CLEAR        )  strcpy(text, "Elimina");
		if ( num == VK_BUTTON1          )  strcpy(text, "Pulsante 1");
		if ( num == VK_BUTTON2          )  strcpy(text, "Pulsante 2");
		if ( num == VK_BUTTON3          )  strcpy(text, "Pulsante 3");
		if ( num == VK_BUTTON4          )  strcpy(text, "Pulsante 4");
		if ( num == VK_BUTTON5          )  strcpy(text, "Pulsante 5");
		if ( num == VK_BUTTON6          )  strcpy(text, "Pulsante 6");
		if ( num == VK_BUTTON7          )  strcpy(text, "Pulsante 7");
		if ( num == VK_BUTTON8          )  strcpy(text, "Pulsante 8");
		if ( num == VK_BUTTON9          )  strcpy(text, "Pulsante 9");
		if ( num == VK_BUTTON10         )  strcpy(text, "Pulsante 10");
		if ( num == VK_BUTTON11         )  strcpy(text, "Pulsante 11");
		if ( num == VK_BUTTON12         )  strcpy(text, "Pulsante 12");
		if ( num == VK_BUTTON13         )  strcpy(text, "Pulsante 13");
		if ( num == VK_BUTTON14         )  strcpy(text, "Pulsante 14");
		if ( num == VK_BUTTON15         )  strcpy(text, "Pulsante 15");
		if ( num == VK_BUTTON16         )  strcpy(text, "Pulsante 16");
		if ( num == VK_BUTTON17         )  strcpy(text, "Pulsante 17");
		if ( num == VK_BUTTON18         )  strcpy(text, "Pulsante 18");
		if ( num == VK_BUTTON19         )  strcpy(text, "Pulsante 19");
		if ( num == VK_BUTTON20         )  strcpy(text, "Pulsante 20");
		if ( num == VK_BUTTON21         )  strcpy(text, "Pulsante 21");
		if ( num == VK_BUTTON22         )  strcpy(text, "Pulsante 22");
		if ( num == VK_BUTTON23         )  strcpy(text, "Pulsante 23");
		if ( num == VK_BUTTON24         )  strcpy(text, "Pulsante 24");
		if ( num == VK_BUTTON25         )  strcpy(text, "Pulsante 25");
		if ( num == VK_BUTTON26         )  strcpy(text, "Pulsante 26");
		if ( num == VK_BUTTON27         )  strcpy(text, "Pulsante 27");
		if ( num == VK_BUTTON28         )  strcpy(text, "Pulsante 28");
		if ( num == VK_BUTTON29         )  strcpy(text, "Pulsante 29");
		if ( num == VK_BUTTON30         )  strcpy(text, "Pulsante 30");
		if ( num == VK_BUTTON31         )  strcpy(text, "Pulsante 31");
		if ( num == VK_BUTTON32         )  strcpy(text, "Pulsante 32");
		if ( num == VK_WHEELUP          )  strcpy(text, "Volante su");
		if ( num == VK_WHEELDOWN        )  strcpy(text, "Volante giù");
	}
#endif

#if _SPANISH
	if ( type == RES_TEXT )
	{
		#if _FULL
		#if _EDU
		if ( num == RT_VERSION_ID          )  strcpy(text, "EDU 1.3 /s");
		#else
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /s");
		#endif
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /s");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximizar");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimizar");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Tamaño normal");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Cerrar");

		if ( num == RT_KEY_OR              )  strcpy(text, " o ");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Selecciona una misión:");
		if ( num == RT_PLAY_LISTd          )  strcpy(text, " Défis :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Atelier de %s :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototipos:");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Drivers:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolución:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Haz clic primero en la tecla que quieres redefinir.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Pulsa la tecla que quieres usar en su lugar.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Lista de conductores:");

		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "¿Quieres salir de BlupiMania?");
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Anular\\Anular la misión actual");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuar\\Continuar la misión actual");
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Salir\\Salir de BlupiMania");
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuar\\Continuar el juego");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "¿Quieres eliminar juegos y coches guardados de %s?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "¿Eliminar el archivo %s existente?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Eliminar");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nombre del conductor:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Crear");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_PLAY         )  strcpy(text, "Résoudre le puzzle...");
		if ( num == RT_DIALOG_REPLAY       )  strcpy(text, "Rejouer pour le plaisir...");
		if ( num == RT_DIALOG_QUITEDIT     )  strcpy(text, "Enregistrer les modifications ?");
		if ( num == RT_DIALOG_YESQUITEDIT  )  strcpy(text, "Oui\\Enregistrer et quitter");
		if ( num == RT_DIALOG_NOQUITEDIT   )  strcpy(text, "Non\\Quitter sans enregistrer");
		if ( num == RT_DIALOG_CANQUITEDIT  )  strcpy(text, "Annuler\\Continuer l'édition");
		if ( num == RT_DIALOG_NEWPUZZLE    )  strcpy(text, "Voulez-vous créer un nouveau puzzle \"%s\" ?");
		if ( num == RT_DIALOG_YESNEWP      )  strcpy(text, "Créer");
		if ( num == RT_DIALOG_NONEWP       )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_DELPUZ       )  strcpy(text, "Voulez-vous supprimer le puzzle ?");
		if ( num == RT_DIALOG_DELPUZd      )  strcpy(text, "(pour tous les joueurs)");
		if ( num == RT_DIALOG_DELPUZu      )  strcpy(text, "(seulement dans votre atelier)");
		if ( num == RT_DIALOG_YESDELPUZ    )  strcpy(text, "Supprimer");
		if ( num == RT_DIALOG_NODELPUZ     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RESUMEINFO   )  strcpy(text, "Instructions");
		if ( num == RT_DIALOG_AUTHORINFO   )  strcpy(text, "Auteur");
		if ( num == RT_DIALOG_SIGNINFO     )  strcpy(text, "Indices");
		if ( num == RT_DIALOG_YESINFO      )  strcpy(text, "OK");
		if ( num == RT_DIALOG_YESERROR     )  strcpy(text, "OK");
		if ( num == RT_DIALOG_EXPORTGAME   )  strcpy(text, "Où voulez-vous exporter le casse-tête ?");
		if ( num == RT_DIALOG_YESEXPORT    )  strcpy(text, "Exporter");
		if ( num == RT_DIALOG_NOEXPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Casse-tête contenus dans \"Mes documents\" :");
		if ( num == RT_DIALOG_YESIMPORT    )  strcpy(text, "Importer");
		if ( num == RT_DIALOG_NOIMPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_TEST         )  strcpy(text, "Tester");
		if ( num == RT_DIALOG_DEFI         )  strcpy(text, "Relever le défi...");
		if ( num == RT_DIALOG_FILE         )  strcpy(text, "Fichier");
		if ( num == RT_DIALOG_RENAMEGAME   )  strcpy(text, "Changement du nom du fichier du casse-tête.");
		if ( num == RT_DIALOG_YESRENAME    )  strcpy(text, "Renommer");
		if ( num == RT_DIALOG_NORENAME     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RENAMEOLD    )  strcpy(text, "Nom actuel");
		if ( num == RT_DIALOG_RENAMENEW    )  strcpy(text, "Nouveau nom");

		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.epsitec.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Desarrollado por EPSITEC");
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Has jugado con la versión de DEMOSTRACIÓN de BlupiMania.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "La versión completa ya está disponible...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... consta de 28 misiones desafiantes y 9 carreras...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... así como de un modo de \"duelo\" para comparar actuaciones.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Más información en www.epsitec.com");

		if ( num == RT_ACTION_GOTO         )  strcpy(text, "Aller");
		if ( num == RT_ACTION_PUSH1        )  strcpy(text, "Pousser une fois");
		if ( num == RT_ACTION_PUSHx        )  strcpy(text, "Pousser %d fois");
		if ( num == RT_ACTION_PUSHn        )  strcpy(text, "Pousser toujours");
		if ( num == RT_ACTION_PULL1        )  strcpy(text, "Tirer une fois");
		if ( num == RT_ACTION_ROLL         )  strcpy(text, "Rouler");
		if ( num == RT_ACTION_DOCKm        )  strcpy(text, "Actionner");
		if ( num == RT_ACTION_DOCKmx       )  strcpy(text, "Actionner %d fois");
		if ( num == RT_ACTION_DOCKg        )  strcpy(text, "Saisir/déposer");
		if ( num == RT_ACTION_TRAXa        )  strcpy(text, "Avancer");
		if ( num == RT_ACTION_TRAXax       )  strcpy(text, "Avancer %d fois");
		if ( num == RT_ACTION_TRAXl        )  strcpy(text, "Tourner à droite");
		if ( num == RT_ACTION_TRAXlx       )  strcpy(text, "Tourner à droite %d fois");
		if ( num == RT_ACTION_TRAXr        )  strcpy(text, "Tourner à gauche");
		if ( num == RT_ACTION_TRAXrx       )  strcpy(text, "Tourner à gauche %d fois");
		if ( num == RT_ACTION_GUNa         )  strcpy(text, "Avancer puis tirer");
		if ( num == RT_ACTION_GUNax        )  strcpy(text, "Avancer puis tirer %d fois");
		if ( num == RT_ACTION_GUNl         )  strcpy(text, "Tourner à droite puis tirer");
		if ( num == RT_ACTION_GUNlx        )  strcpy(text, "Tourner à droite puis tirer %d fois");
		if ( num == RT_ACTION_GUNr         )  strcpy(text, "Tourner à gauche puis tirer");
		if ( num == RT_ACTION_GUNrx        )  strcpy(text, "Tourner à gauche puis tirer %d fois");
		if ( num == RT_ACTION_CATAPULT     )  strcpy(text, "Lancer");

		if ( num == RT_ADVISE_LITGOTO      )  strcpy(text, "...");
		if ( num == RT_ADVISE_GOTOPUSH     )  strcpy(text, "...");
		if ( num == RT_ADVISE_NOSIGN       )  strcpy(text, "Aucun indice disponible pour ce casse-tête.");

		if ( num == RT_USER_HEADER         )  strcpy(text, "Fichier\tUnivers\tTesté\tInstructions");
		if ( num == RT_DEFI_HEADER         )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");
		if ( num == RT_IMPORT_HEADER       )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");

		if ( num == RT_DEMO                )  strcpy(text, "Pas disponible dans la DEMO");

		if ( num == RT_UNIVERS0            )  strcpy(text, "Castella");
		if ( num == RT_UNIVERS1            )  strcpy(text, "Paradiso");
		if ( num == RT_UNIVERS2            )  strcpy(text, "Volcano");
		if ( num == RT_UNIVERS3            )  strcpy(text, "Mecano");
		if ( num == RT_UNIVERS4            )  strcpy(text, "Sahara");
		if ( num == RT_UNIVERS5            )  strcpy(text, "Aglagla");
		if ( num == RT_UNIVERS6            )  strcpy(text, "Samba");
		if ( num == RT_UNIVERS7            )  strcpy(text, "Medieva");
		if ( num == RT_UNIVERS8            )  strcpy(text, "Vegeta");
		if ( num == RT_UNIVERS9            )  strcpy(text, "Inferno");
		if ( num == RT_UNIVERS10           )  strcpy(text, "Futura");
		if ( num == RT_UNIVERS11           )  strcpy(text, "Alibaba");
		if ( num == RT_UNIVERS12           )  strcpy(text, "Studio");
		if ( num == RT_UNIVERS13           )  strcpy(text, "Cata");
		if ( num == RT_UNIVERS14           )  strcpy(text, "Allegro");
		if ( num == RT_UNIVERS15           )  strcpy(text, "Cemento");
		if ( num == RT_UNIVERS16           )  strcpy(text, "Labo");
		if ( num == RT_UNIVERS17           )  strcpy(text, "Drinka");
		if ( num == RT_UNIVERS18           )  strcpy(text, "Woodo");
		if ( num == RT_UNIVERS19           )  strcpy(text, "Natura");
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "Aceptar");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancelar");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menú (\\key quit;)");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "Aceptar");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancelar");

		if ( num == EVENT_INTERFACE_PUZZLE )  strcpy(text, "Casse-tête\\Torture progressive");
		if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Défis\\Casse-tête d'autres joueurs");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Atelier\\Créez vos casse-tête");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Prototipos\\Prototipos en construcción");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Conductor nuevo\\Elegir nombre de conductor");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opciones\\Preferencias");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Reiniciar\\Reiniciar la misión desde el principio");
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Volver a BlupiMania");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Salir\\Salir de BlupiMania");
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Cancelar\\Volver a la pantalla anterior");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menú\\Volver al menú principal");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Jugar\\Comenzar misión seleccionada");
		if ( num == EVENT_INTERFACE_NEW    )  strcpy(text, "Nouveau\\Créer un nouveau puzzle");
		if ( num == EVENT_INTERFACE_DELETE )  strcpy(text, "Supprimer\\Supprimer le puzzle");
		if ( num == EVENT_INTERFACE_RENAME )  strcpy(text, "Renommer\\Renommer le casse-tête");
		if ( num == EVENT_INTERFACE_EXPORT )  strcpy(text, "Exporter\\Exporter le casse-tête");
		if ( num == EVENT_INTERFACE_IMPORT )  strcpy(text, "Importer\\Importer un casse-tête");
		if ( num == EVENT_INTERFACE_EDIT   )  strcpy(text, "Modifier\\Modifier le puzzle");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "Aceptar\\Volver a la pantalla anterior");

		if ( num == EVENT_INTERFACE_EXPORTdefi )  strcpy(text, "Dans les défis (pour tous les joueurs)");
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "Dans le dossier \"Mes documents\"");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Dispositivo\\Configuración de controlador y resolución");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Gráficos\\Configuración de gráficos");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Juego\\Configuración de juego");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controles\\Configuración de teclado, volante y gamepad");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sonido\\Volumen de música y efectos de sonido");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolución");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Pantalla completa\\Modo de pantalla completa o ventana");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Aplicar cambios\\Activa la configuración modificada");

		if ( num == EVENT_INTERFACE_SHADOW      )  strcpy(text, "Ombres\\Ombres projetées au sol");
		if ( num == EVENT_INTERFACE_DIRTY       )  strcpy(text, "Salissures\\Salissures des object");
		if ( num == EVENT_INTERFACE_SUNBEAM     )  strcpy(text, "Rayons du soleil\\Rayons selon l'orientation");
		if ( num == EVENT_INTERFACE_LENSFLARE   )  strcpy(text, "Reflets dans l'objectif\\Reflets selon l'orientation");
		if ( num == EVENT_INTERFACE_DECOR       )  strcpy(text, "Objets décoratifs\\Arbres, plantes, etc.");
		if ( num == EVENT_INTERFACE_DETAIL      )  strcpy(text, "Objets détaillés\\Objets détaillés");
		if ( num == EVENT_INTERFACE_METEO       )  strcpy(text, "Conditions météo\\Pluie et neige");
		if ( num == EVENT_INTERFACE_AMBIANCE    )  strcpy(text, "Effets d'ambiance\\");
		if ( num == EVENT_INTERFACE_EXPLOVIB    )  strcpy(text, "Secousses lors d'explosions\\L'écran vibre lors d'une explosion");
		if ( num == EVENT_INTERFACE_SPEEDSCH    )  strcpy(text, "Vitesse de défilement horizontal\\");
		if ( num == EVENT_INTERFACE_SPEEDSCV    )  strcpy(text, "Vitesse de défilement vertical\\");
		if ( num == EVENT_INTERFACE_MOUSESCROLL )  strcpy(text, "Défilement avec la souris\\");
		if ( num == EVENT_INTERFACE_INVSCH      )  strcpy(text, "Inversion défilement horizontal\\");
		if ( num == EVENT_INTERFACE_INVSCV      )  strcpy(text, "Inversion défilement vertical\\");
		if ( num == EVENT_INTERFACE_MOVIE       )  strcpy(text, "Séquences cinématiques\\Séquences d'introduction");
		if ( num == EVENT_INTERFACE_HELP        )  strcpy(text, "Instructions et indices\\Textes explicatifs");
		if ( num == EVENT_INTERFACE_TOOLTIPS    )  strcpy(text, "Bulles d'aide\\Bulles explicatives");
		if ( num == EVENT_INTERFACE_NICEMOUSE   )  strcpy(text, "Souris ombrée\\Jolie souris avec une ombre");
		if ( num == EVENT_INTERFACE_ACCEL       )  strcpy(text, "Accélère si long à résoudre\\Blupi va plus vite après 10 minutes de réflexion");
		if ( num == EVENT_INTERFACE_VOLBLUPI    )  strcpy(text, "Volume des voix de Blupi\\");
		if ( num == EVENT_INTERFACE_VOLSOUND    )  strcpy(text, "Volume des bruitages\\");
		if ( num == EVENT_INTERFACE_VOLAMBIANCE )  strcpy(text, "Volume d'ambiance\\");
		if ( num == EVENT_INTERFACE_SOUND3D     )  strcpy(text, "Sonido 3D\\Sonido 3D");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Controles predeterminados\\Restablece todos los controles a los valores predeterminados");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Scroll left\\Scrolling to left");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Scroll right\\Scrolling to right");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Scroll up\\Scrolling to up");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Scroll down\\Scrolling to down");
		if ( num == EVENT_INTERFACE_KROTCW )  strcpy(text, "Rotate CW\\Rotate camera clockwise");
		if ( num == EVENT_INTERFACE_KROTCCW)  strcpy(text, "Rotate CCW\\Rotate camera counter clockwise");
		if ( num == EVENT_INTERFACE_KSTOP  )  strcpy(text, "Stoppe action\\Stoppe l'action en cours");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Salir\\Salir de la misión o carrera actual");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instrucciones\\Mostrar instrucciones para la misión actual");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Más baja\\Calidad de gráficos mínima (frecuencia de cuadro más alta)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Calidad de gráficos normal");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Más alta\\Calidad de gráficos más alta (frecuencia de cuadro más baja)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silencio\\Sin sonido");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Volumen de sonido normal");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nombre de nuevo conductor");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Seleccionar\\Elegir el conductor seleccionado");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Nuevo\\Crear un conductor nuevo");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Eliminar\\Eliminar al conductor de la lista");

		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Previous screen");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Next screen");
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit BlupiMania");

		if ( num == EVENT_INTERFACE_UNDO   )  strcpy(text, "Annule la dernière action");
		if ( num == EVENT_INTERFACE_STOP   )  strcpy(text, "Stop (\\key stop;)");
		if ( num == EVENT_INTERFACE_SIGN   )  strcpy(text, "Obtenir des indices");
		if ( num == EVENT_LABEL_SIGN       )  strcpy(text, "Indices");

		if ( num == EVENT_EDIT_WATER       )  strcpy(text, "Eau");
		if ( num == EVENT_EDIT_GROUND      )  strcpy(text, "Sol");
		if ( num == EVENT_EDIT_OBJECT      )  strcpy(text, "Objets");
		if ( num == EVENT_EDIT_INFO        )  strcpy(text, "Informations sur le puzzle");

		if ( num == EVENT_CMD              )  strcpy(text, "Línea de comandos");
		if ( num == EVENT_SPEED            )  strcpy(text, "Velocidad de juego");
	}

	if ( type == RES_OBJECT )
	{
		if ( num == OBJECT_BLUPI           )  strcpy(text, "Blupi");
		if ( num == OBJECT_BOX1            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX2            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX3            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX4            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX5            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX6            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX7            )  strcpy(text, "Grosse boule");
		if ( num == OBJECT_BOX8            )  strcpy(text, "Colonne lourde");
		if ( num == OBJECT_BOX9            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX10           )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX11           )  strcpy(text, "Caisse sur roues");
		if ( num == OBJECT_BOX12           )  strcpy(text, "Caisse sur roues");
		if ( num == OBJECT_BOX13           )  strcpy(text, "Caisse sur coussin d'air");
		if ( num == OBJECT_KEY1            )  strcpy(text, "Clé orange");
		if ( num == OBJECT_KEY2            )  strcpy(text, "Clé bleue");
		if ( num == OBJECT_DOOR1           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR2           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOOR3           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR4           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOCK            )  strcpy(text, "Pont-grue");
		if ( num == OBJECT_CATAPULT        )  strcpy(text, "Catapulte");
		if ( num == OBJECT_TRAX            )  strcpy(text, "Pousseur mécanique");
		if ( num == OBJECT_PERFO           )  strcpy(text, "Perforateur mécanique");
		if ( num == OBJECT_GUN             )  strcpy(text, "Destructeur de caisses");
		if ( num == OBJECT_MINE            )  strcpy(text, "Mine");
		if ( num == OBJECT_GLASS1          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_GLASS2          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_FIOLE           )  strcpy(text, "Potion de force");
		if ( num == OBJECT_GLU             )  strcpy(text, "Potion de glu");
		if ( num == OBJECT_GOAL            )  strcpy(text, "But à atteindre");
		if ( num == OBJECT_CRAZY           )  strcpy(text, "Crazy bot");
		if ( num == OBJECT_BOT1            )  strcpy(text, "Bot 1");
		if ( num == OBJECT_BOT2            )  strcpy(text, "Bot 2");
		if ( num == OBJECT_BOT3            )  strcpy(text, "Bot 3");
		if ( num == OBJECT_BOT4            )  strcpy(text, "Bot 4");
		if ( num == OBJECT_BOT5            )  strcpy(text, "Bot 5");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Error");
		if ( num == ERR_CMD             )  strcpy(text, "Comando desconocido");
		if ( num == ERR_INSTALL         )  strcpy(text, "Wild Wheels no se ha instalado correctamente.");
		if ( num == ERR_NOCD            )  strcpy(text, "Introduce el CD de Wild Wheels\ny ejecuta el juego.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Error al realizar la instrucción");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Ir a: destino no accesible");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Ir a: destino no accesible");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Ir a: destino ocupado");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "BOT inapropiado");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "No hay suficiente energía");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "Aún no se ha realizado la misión (pulsa la \\key help; para más información)");
		if ( num == ERR_PUZZLE_ZEROBLUPI)  strcpy(text, "Il doit y avoir au moins un Blupi !");
		if ( num == ERR_PUZZLE_ZEROGOAL )  strcpy(text, "Il doit y avoir au moins un ballon !");
		if ( num == ERR_PUZZLE_LOGOAL   )  strcpy(text, "Il n'y a pas assez de ballons !");
		if ( num == ERR_PUZZLE_HIGOAL   )  strcpy(text, "Il y a trop de ballons !");
		if ( num == ERR_PUZZLE_MAXBLUPI )  strcpy(text, "4 Blupi ou robots au maximum par puzzle !");
		if ( num == ERR_PUZZLE_MAXGOAL  )  strcpy(text, "8 ballons au maximum par puzzle !");
		if ( num == ERR_PUZZLE_MAX      )  strcpy(text, "Il n'est pas possible de créer plus d'objets de ce type !");
		if ( num == ERR_CREATE          )  strcpy(text, "Ce fichier existe déjà.");

		if ( num == INFO_WIN            )  strcpy(text, "<< Enhorabuena, misión cumplida >>");
		if ( num == INFO_LOST           )  strcpy(text, "<< Lo sentimos, misión no lograda >>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<< Lo sentimos, misión no lograda >>");
	}

	if ( type == RES_CBOT )
	{
		strcpy(text, "Error");
		if ( num == TX_OPENPAR       ) strcpy(text, "Falta paréntesis de apertura");
		if ( num == TX_CLOSEPAR      ) strcpy(text, "Falta paréntesis de cierre ");
		if ( num == TX_NOTBOOL       ) strcpy(text, "La expresión debe devolver un valor booleano");
		if ( num == TX_UNDEFVAR      ) strcpy(text, "No se ha declarado la variable");
		if ( num == TX_BADLEFT       ) strcpy(text, "Asignación imposible");
		if ( num == TX_ENDOF         ) strcpy(text, "Falta terminador de punto y coma");
		if ( num == TX_OUTCASE       ) strcpy(text, "Instrucción ""case"" fuera de un bloque ""switch""");
		if ( num == TX_NOTERM        ) strcpy(text, "Instrucciones después de la llave de cierre final");
		if ( num == TX_CLOSEBLK      ) strcpy(text, "Falta el final del bloque");
		if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instrucción ""else"" sin su correspondiente ""if"" ");
		if ( num == TX_OPENBLK       ) strcpy(text, "Abriendo la llave que falta ");//début d'un bloc attendu?
		if ( num == TX_BADTYPE       ) strcpy(text, "Tipo incorrecto para la asignación");
		if ( num == TX_REDEFVAR      ) strcpy(text, "Una variable no puede declararse dos veces");
		if ( num == TX_BAD2TYPE      ) strcpy(text, "Los tipos de los dos operadores son incompatibles ");
		if ( num == TX_UNDEFCALL     ) strcpy(text, "Función desconocida");
		if ( num == TX_MISDOTS       ) strcpy(text, "Falta el signo "" : """);
		if ( num == TX_WHILE         ) strcpy(text, "Falta la palabra clave ""while""");
		if ( num == TX_BREAK         ) strcpy(text, "Instrucción ""break"" fuera de bucle");
		if ( num == TX_LABEL         ) strcpy(text, "Una etiqueta debe ir seguida de ""for"", ""while"", ""do"" o ""switch""");
		if ( num == TX_NOLABEL       ) strcpy(text, "Esta etiqueta no existe");// Cette étiquette n'existe pas
		if ( num == TX_NOCASE        ) strcpy(text, "Falta la instrucción ""case""");
		if ( num == TX_BADNUM        ) strcpy(text, "Falta el número");
		if ( num == TX_VOID          ) strcpy(text, "Parámetro nulo");
		if ( num == TX_NOTYP         ) strcpy(text, "Falta la declaración de tipo");
		if ( num == TX_NOVAR         ) strcpy(text, "Falta el nombre de la variable");
		if ( num == TX_NOFONC        ) strcpy(text, "Falta el nombre de la función");
		if ( num == TX_OVERPARAM     ) strcpy(text, "Demasiados parámetros");
		if ( num == TX_REDEF         ) strcpy(text, "La función ya existe");
		if ( num == TX_LOWPARAM      ) strcpy(text, "Faltan parámetros ");
		if ( num == TX_BADPARAM      ) strcpy(text, "Ninguna función de este nombre acepta este tipo de parámetro");
		if ( num == TX_NUMPARAM      ) strcpy(text, "Ninguna función de este nombre acepta este número de parámetros");
		if ( num == TX_NOITEM        ) strcpy(text, "No es un miembro de esta clase");
		if ( num == TX_DOT           ) strcpy(text, "Este objeto no es un miembro de una clase");
		if ( num == TX_NOCONST       ) strcpy(text, "Falta el constructor adecuado");
		if ( num == TX_REDEFCLASS    ) strcpy(text, "Esta clase ya existe");
		if ( num == TX_CLBRK         ) strcpy(text, """ Falta ] """);
		if ( num == TX_RESERVED      ) strcpy(text, "Palabra clave reservada del idioma CBOT");
		if ( num == TX_BADNEW        ) strcpy(text, "Argumento incorrecto para ""new""");
		if ( num == TX_OPBRK         ) strcpy(text, """ Se esperaba [ """);
		if ( num == TX_BADSTRING     ) strcpy(text, "Falta cadena");
		if ( num == TX_BADINDEX      ) strcpy(text, "Tipo de índice incorrecto");
		if ( num == TX_PRIVATE       ) strcpy(text, "Elemento privado");
		if ( num == TX_NOPUBLIC      ) strcpy(text, "Se requiere público");
		if ( num == TX_DIVZERO       ) strcpy(text, "Dividiendo por cero");
		if ( num == TX_NOTINIT       ) strcpy(text, "Variable no inicializada");
		if ( num == TX_BADTHROW      ) strcpy(text, """throw"" rechaza el valor negativo");//C'est quoi, ça?
		if ( num == TX_NORETVAL      ) strcpy(text, "La función no devolvió ningún valor ");
		if ( num == TX_NORUN         ) strcpy(text, "No se ejecuta ninguna función");
		if ( num == TX_NOCALL        ) strcpy(text, "Llamando a una función desconocida");
		if ( num == TX_NOCLASS       ) strcpy(text, "Esta clase no existe");
		if ( num == TX_NULLPT        ) strcpy(text, "Objeto desconocido");
		if ( num == TX_OPNAN         ) strcpy(text, "Operación imposible con el valor ""nan""");
		if ( num == TX_OUTARRAY      ) strcpy(text, "Acceso fuera del límite de la matriz");
		if ( num == TX_STACKOVER     ) strcpy(text, "Pila desbordada");
		if ( num == TX_DELETEDPT     ) strcpy(text, "Objeto ilegal");
		if ( num == TX_FILEOPEN      ) strcpy(text, "No puede abrirse el archivo");
		if ( num == TX_NOTOPEN       ) strcpy(text, "Archivo sin abrir");
		if ( num == TX_ERRREAD       ) strcpy(text, "Error de lectura");
		if ( num == TX_ERRWRITE      ) strcpy(text, "Error de escritura");
	}

	if ( type == RES_KEY )
	{
		if ( num == 0                   )  strcpy(text, "< ninguno >");
		if ( num == VK_LEFT             )  strcpy(text, "Flecha Izquierda");
		if ( num == VK_RIGHT            )  strcpy(text, "Flecha Derecha");
		if ( num == VK_UP               )  strcpy(text, "Flecha Arriba");
		if ( num == VK_DOWN             )  strcpy(text, "Flecha Abajo");
		if ( num == VK_CANCEL           )  strcpy(text, "Control-interrupción");
		if ( num == VK_BACK             )  strcpy(text, "<--");
		if ( num == VK_TAB              )  strcpy(text, "Tab");
		if ( num == VK_CLEAR            )  strcpy(text, "Borrar");
		if ( num == VK_RETURN           )  strcpy(text, "Entrar");
		if ( num == VK_SHIFT            )  strcpy(text, "Mayús");
		if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
		if ( num == VK_MENU             )  strcpy(text, "Alt");
		if ( num == VK_PAUSE            )  strcpy(text, "Pausa");
		if ( num == VK_CAPITAL          )  strcpy(text, "Bloq Mayús");
		if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
		if ( num == VK_SPACE            )  strcpy(text, "Barra espaciadora");
		if ( num == VK_PRIOR            )  strcpy(text, "Re Pág");
		if ( num == VK_NEXT             )  strcpy(text, "Av Pág");
		if ( num == VK_END              )  strcpy(text, "Fin");
		if ( num == VK_HOME             )  strcpy(text, "Inicio");
		if ( num == VK_SELECT           )  strcpy(text, "Seleccionar");
		if ( num == VK_EXECUTE          )  strcpy(text, "Ejecutar");
		if ( num == VK_SNAPSHOT         )  strcpy(text, "Impr Pant");
		if ( num == VK_INSERT           )  strcpy(text, "Insertar");
		if ( num == VK_DELETE           )  strcpy(text, "Eliminar");
		if ( num == VK_HELP             )  strcpy(text, "Ayuda");
		if ( num == VK_LWIN             )  strcpy(text, "Ventanas izquierdas");
		if ( num == VK_RWIN             )  strcpy(text, "Ventanas derechas");
		if ( num == VK_APPS             )  strcpy(text, "Tecla de aplicación");
		if ( num == VK_NUMPAD0          )  strcpy(text, "Teclado numérico 0");
		if ( num == VK_NUMPAD1          )  strcpy(text, "Teclado numérico 1");
		if ( num == VK_NUMPAD2          )  strcpy(text, "Teclado numérico 2");
		if ( num == VK_NUMPAD3          )  strcpy(text, "Teclado numérico 3");
		if ( num == VK_NUMPAD4          )  strcpy(text, "Teclado numérico 4");
		if ( num == VK_NUMPAD5          )  strcpy(text, "Teclado numérico 5");
		if ( num == VK_NUMPAD6          )  strcpy(text, "Teclado numérico 6");
		if ( num == VK_NUMPAD7          )  strcpy(text, "Teclado numérico 7");
		if ( num == VK_NUMPAD8          )  strcpy(text, "Teclado numérico 8");
		if ( num == VK_NUMPAD9          )  strcpy(text, "Teclado numérico 9");
		if ( num == VK_MULTIPLY         )  strcpy(text, "Teclado numérico *");
		if ( num == VK_ADD              )  strcpy(text, "Teclado numérico +");
		if ( num == VK_SEPARATOR        )  strcpy(text, "Teclado numérico separador");
		if ( num == VK_SUBTRACT         )  strcpy(text, "Teclado numérico -");
		if ( num == VK_DECIMAL          )  strcpy(text, "Teclado numérico .");
		if ( num == VK_DIVIDE           )  strcpy(text, "Teclado numérico /");
		if ( num == VK_F1               )  strcpy(text, "F1");
		if ( num == VK_F2               )  strcpy(text, "F2");
		if ( num == VK_F3               )  strcpy(text, "F3");
		if ( num == VK_F4               )  strcpy(text, "F4");
		if ( num == VK_F5               )  strcpy(text, "F5");
		if ( num == VK_F6               )  strcpy(text, "F6");
		if ( num == VK_F7               )  strcpy(text, "F7");
		if ( num == VK_F8               )  strcpy(text, "F8");
		if ( num == VK_F9               )  strcpy(text, "F9");
		if ( num == VK_F10              )  strcpy(text, "F10");
		if ( num == VK_F11              )  strcpy(text, "F11");
		if ( num == VK_F12              )  strcpy(text, "F12");
		if ( num == VK_F13              )  strcpy(text, "F13");
		if ( num == VK_F14              )  strcpy(text, "F14");
		if ( num == VK_F15              )  strcpy(text, "F15");
		if ( num == VK_F16              )  strcpy(text, "F16");
		if ( num == VK_F17              )  strcpy(text, "F17");
		if ( num == VK_F18              )  strcpy(text, "F18");
		if ( num == VK_F19              )  strcpy(text, "F19");
		if ( num == VK_F20              )  strcpy(text, "F20");
		if ( num == VK_NUMLOCK          )  strcpy(text, "Bloq Num");
		if ( num == VK_SCROLL           )  strcpy(text, "Desplazar");
		if ( num == VK_ATTN             )  strcpy(text, "Atención");
		if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
		if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
		if ( num == VK_EREOF            )  strcpy(text, "Borrar EOF");
		if ( num == VK_PLAY             )  strcpy(text, "Jugar");
		if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
		if ( num == VK_PA1              )  strcpy(text, "PA1");
		if ( num == VK_OEM_CLEAR        )  strcpy(text, "Borrar");
		if ( num == VK_BUTTON1          )  strcpy(text, "Botón 1");
		if ( num == VK_BUTTON2          )  strcpy(text, "Botón 2");
		if ( num == VK_BUTTON3          )  strcpy(text, "Botón 3");
		if ( num == VK_BUTTON4          )  strcpy(text, "Botón 4");
		if ( num == VK_BUTTON5          )  strcpy(text, "Botón 5");
		if ( num == VK_BUTTON6          )  strcpy(text, "Botón 6");
		if ( num == VK_BUTTON7          )  strcpy(text, "Botón 7");
		if ( num == VK_BUTTON8          )  strcpy(text, "Botón 8");
		if ( num == VK_BUTTON9          )  strcpy(text, "Botón 9");
		if ( num == VK_BUTTON10         )  strcpy(text, "Botón 10");
		if ( num == VK_BUTTON11         )  strcpy(text, "Botón 11");
		if ( num == VK_BUTTON12         )  strcpy(text, "Botón 12");
		if ( num == VK_BUTTON13         )  strcpy(text, "Botón 13");
		if ( num == VK_BUTTON14         )  strcpy(text, "Botón 14");
		if ( num == VK_BUTTON15         )  strcpy(text, "Botón 15");
		if ( num == VK_BUTTON16         )  strcpy(text, "Botón 16");
		if ( num == VK_BUTTON17         )  strcpy(text, "Botón 17");
		if ( num == VK_BUTTON18         )  strcpy(text, "Botón 18");
		if ( num == VK_BUTTON19         )  strcpy(text, "Botón 19");
		if ( num == VK_BUTTON20         )  strcpy(text, "Botón 20");
		if ( num == VK_BUTTON21         )  strcpy(text, "Botón 21");
		if ( num == VK_BUTTON22         )  strcpy(text, "Botón 22");
		if ( num == VK_BUTTON23         )  strcpy(text, "Botón 23");
		if ( num == VK_BUTTON24         )  strcpy(text, "Botón 24");
		if ( num == VK_BUTTON25         )  strcpy(text, "Botón 25");
		if ( num == VK_BUTTON26         )  strcpy(text, "Botón 26");
		if ( num == VK_BUTTON27         )  strcpy(text, "Botón 27");
		if ( num == VK_BUTTON28         )  strcpy(text, "Botón 28");
		if ( num == VK_BUTTON29         )  strcpy(text, "Botón 29");
		if ( num == VK_BUTTON30         )  strcpy(text, "Botón 30");
		if ( num == VK_BUTTON31         )  strcpy(text, "Botón 31");
		if ( num == VK_BUTTON32         )  strcpy(text, "Botón 32");
		if ( num == VK_WHEELUP          )  strcpy(text, "Subir volante");
		if ( num == VK_WHEELDOWN        )  strcpy(text, "Bajar volante");
	}
#endif

#if _PORTUGUESE
	if ( type == RES_TEXT )
	{
		#if _FULL
		#if _EDU
		if ( num == RT_VERSION_ID          )  strcpy(text, "EDU 1.3 /p");
		#else
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /p");
		#endif
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /p");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximizar");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimizar");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Tamanho normal");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Fechar");

		if ( num == RT_KEY_OR              )  strcpy(text, " ou ");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Selecione uma missão:");
		if ( num == RT_PLAY_LISTd          )  strcpy(text, " Défis :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Atelier de %s :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Protótipos:");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Drivers:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolução:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Primeiro clique na tecla que deseja redefinir.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Depois pressione a tecla que deseja usar.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Lista de pilotos:");

		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Deseja sair de BlupiMania?");
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Anular\\Anular a missão atual");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuar\\Continuar a missão atual");
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Sair\\Sair de BlupiMania");
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuar\\Continuar o jogo");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Deseja excluir os jogos e os carros salvos de %s?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Excluir o arquivo existente %s?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Excluir");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nome do piloto:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Criar");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_PLAY         )  strcpy(text, "Résoudre le puzzle...");
		if ( num == RT_DIALOG_REPLAY       )  strcpy(text, "Rejouer pour le plaisir...");
		if ( num == RT_DIALOG_QUITEDIT     )  strcpy(text, "Enregistrer les modifications ?");
		if ( num == RT_DIALOG_YESQUITEDIT  )  strcpy(text, "Oui\\Enregistrer et quitter");
		if ( num == RT_DIALOG_NOQUITEDIT   )  strcpy(text, "Non\\Quitter sans enregistrer");
		if ( num == RT_DIALOG_CANQUITEDIT  )  strcpy(text, "Annuler\\Continuer l'édition");
		if ( num == RT_DIALOG_NEWPUZZLE    )  strcpy(text, "Voulez-vous créer un nouveau puzzle \"%s\" ?");
		if ( num == RT_DIALOG_YESNEWP      )  strcpy(text, "Créer");
		if ( num == RT_DIALOG_NONEWP       )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_DELPUZ       )  strcpy(text, "Voulez-vous supprimer le puzzle ?");
		if ( num == RT_DIALOG_DELPUZd      )  strcpy(text, "(pour tous les joueurs)");
		if ( num == RT_DIALOG_DELPUZu      )  strcpy(text, "(seulement dans votre atelier)");
		if ( num == RT_DIALOG_YESDELPUZ    )  strcpy(text, "Supprimer");
		if ( num == RT_DIALOG_NODELPUZ     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RESUMEINFO   )  strcpy(text, "Instructions");
		if ( num == RT_DIALOG_AUTHORINFO   )  strcpy(text, "Auteur");
		if ( num == RT_DIALOG_SIGNINFO     )  strcpy(text, "Indices");
		if ( num == RT_DIALOG_YESINFO      )  strcpy(text, "OK");
		if ( num == RT_DIALOG_YESERROR     )  strcpy(text, "OK");
		if ( num == RT_DIALOG_EXPORTGAME   )  strcpy(text, "Où voulez-vous exporter le casse-tête ?");
		if ( num == RT_DIALOG_YESEXPORT    )  strcpy(text, "Exporter");
		if ( num == RT_DIALOG_NOEXPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_IMPORTGAME   )  strcpy(text, "Casse-tête contenus dans \"Mes documents\" :");
		if ( num == RT_DIALOG_YESIMPORT    )  strcpy(text, "Importer");
		if ( num == RT_DIALOG_NOIMPORT     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_TEST         )  strcpy(text, "Tester");
		if ( num == RT_DIALOG_DEFI         )  strcpy(text, "Relever le défi...");
		if ( num == RT_DIALOG_FILE         )  strcpy(text, "Fichier");
		if ( num == RT_DIALOG_RENAMEGAME   )  strcpy(text, "Changement du nom du fichier du casse-tête.");
		if ( num == RT_DIALOG_YESRENAME    )  strcpy(text, "Renommer");
		if ( num == RT_DIALOG_NORENAME     )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_RENAMEOLD    )  strcpy(text, "Nom actuel");
		if ( num == RT_DIALOG_RENAMENEW    )  strcpy(text, "Nouveau nom");

		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.epsitec.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Desenvolvido por EPSITEC");
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Você jogou com a versão DEMO de BlupiMania.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "A versão completa está agora disponível...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... contém 28 missões cheias de desafios e 9 corridas...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... bem como um modo \"duelo\" para comparar desempenhos.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Mais informações em www.epsitec.com!");

		if ( num == RT_ACTION_GOTO         )  strcpy(text, "Aller");
		if ( num == RT_ACTION_PUSH1        )  strcpy(text, "Pousser une fois");
		if ( num == RT_ACTION_PUSHx        )  strcpy(text, "Pousser %d fois");
		if ( num == RT_ACTION_PUSHn        )  strcpy(text, "Pousser toujours");
		if ( num == RT_ACTION_PULL1        )  strcpy(text, "Tirer une fois");
		if ( num == RT_ACTION_ROLL         )  strcpy(text, "Rouler");
		if ( num == RT_ACTION_DOCKm        )  strcpy(text, "Actionner");
		if ( num == RT_ACTION_DOCKmx       )  strcpy(text, "Actionner %d fois");
		if ( num == RT_ACTION_DOCKg        )  strcpy(text, "Saisir/déposer");
		if ( num == RT_ACTION_TRAXa        )  strcpy(text, "Avancer");
		if ( num == RT_ACTION_TRAXax       )  strcpy(text, "Avancer %d fois");
		if ( num == RT_ACTION_TRAXl        )  strcpy(text, "Tourner à droite");
		if ( num == RT_ACTION_TRAXlx       )  strcpy(text, "Tourner à droite %d fois");
		if ( num == RT_ACTION_TRAXr        )  strcpy(text, "Tourner à gauche");
		if ( num == RT_ACTION_TRAXrx       )  strcpy(text, "Tourner à gauche %d fois");
		if ( num == RT_ACTION_GUNa         )  strcpy(text, "Avancer puis tirer");
		if ( num == RT_ACTION_GUNax        )  strcpy(text, "Avancer puis tirer %d fois");
		if ( num == RT_ACTION_GUNl         )  strcpy(text, "Tourner à droite puis tirer");
		if ( num == RT_ACTION_GUNlx        )  strcpy(text, "Tourner à droite puis tirer %d fois");
		if ( num == RT_ACTION_GUNr         )  strcpy(text, "Tourner à gauche puis tirer");
		if ( num == RT_ACTION_GUNrx        )  strcpy(text, "Tourner à gauche puis tirer %d fois");
		if ( num == RT_ACTION_CATAPULT     )  strcpy(text, "Lancer");

		if ( num == RT_ADVISE_LITGOTO      )  strcpy(text, "...");
		if ( num == RT_ADVISE_GOTOPUSH     )  strcpy(text, "...");
		if ( num == RT_ADVISE_NOSIGN       )  strcpy(text, "Aucun indice disponible pour ce casse-tête.");

		if ( num == RT_USER_HEADER         )  strcpy(text, "Fichier\tUnivers\tTesté\tInstructions");
		if ( num == RT_DEFI_HEADER         )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");
		if ( num == RT_IMPORT_HEADER       )  strcpy(text, "Fichier\tUnivers\tAuteur\tInstructions");

		if ( num == RT_DEMO                )  strcpy(text, "Pas disponible dans la DEMO");

		if ( num == RT_UNIVERS0            )  strcpy(text, "Castella");
		if ( num == RT_UNIVERS1            )  strcpy(text, "Paradiso");
		if ( num == RT_UNIVERS2            )  strcpy(text, "Volcano");
		if ( num == RT_UNIVERS3            )  strcpy(text, "Mecano");
		if ( num == RT_UNIVERS4            )  strcpy(text, "Sahara");
		if ( num == RT_UNIVERS5            )  strcpy(text, "Aglagla");
		if ( num == RT_UNIVERS6            )  strcpy(text, "Samba");
		if ( num == RT_UNIVERS7            )  strcpy(text, "Medieva");
		if ( num == RT_UNIVERS8            )  strcpy(text, "Vegeta");
		if ( num == RT_UNIVERS9            )  strcpy(text, "Inferno");
		if ( num == RT_UNIVERS10           )  strcpy(text, "Futura");
		if ( num == RT_UNIVERS11           )  strcpy(text, "Alibaba");
		if ( num == RT_UNIVERS12           )  strcpy(text, "Studio");
		if ( num == RT_UNIVERS13           )  strcpy(text, "Cata");
		if ( num == RT_UNIVERS14           )  strcpy(text, "Allegro");
		if ( num == RT_UNIVERS15           )  strcpy(text, "Cemento");
		if ( num == RT_UNIVERS16           )  strcpy(text, "Labo");
		if ( num == RT_UNIVERS17           )  strcpy(text, "Drinka");
		if ( num == RT_UNIVERS18           )  strcpy(text, "Woodo");
		if ( num == RT_UNIVERS19           )  strcpy(text, "Natura");
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancelar");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancelar");

		if ( num == EVENT_INTERFACE_PUZZLE )  strcpy(text, "Casse-tête\\Torture progressive");
		if ( num == EVENT_INTERFACE_DEFI   )  strcpy(text, "Défis\\Casse-tête d'autres joueurs");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Atelier\\Créez vos casse-tête");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Protótipos em desenvolvimento");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Novo piloto\\Escolha o nome do piloto");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opções\\Preferências");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Reiniciar\\Reinicia a missão a partir do começo");
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Volta ao BlupiMania");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Sair\\Sai de BlupiMania");
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Cancelar\\Volta à tela anterior");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Volta ao menu principal");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Reproduzir\\Inicia missão selecionada!");
		if ( num == EVENT_INTERFACE_NEW    )  strcpy(text, "Nouveau\\Créer un nouveau puzzle");
		if ( num == EVENT_INTERFACE_DELETE )  strcpy(text, "Supprimer\\Supprimer le puzzle");
		if ( num == EVENT_INTERFACE_RENAME )  strcpy(text, "Renommer\\Renommer le casse-tête");
		if ( num == EVENT_INTERFACE_EXPORT )  strcpy(text, "Exporter\\Exporter le casse-tête");
		if ( num == EVENT_INTERFACE_IMPORT )  strcpy(text, "Importer\\Importer un casse-tête");
		if ( num == EVENT_INTERFACE_EDIT   )  strcpy(text, "Modifier\\Modifier le puzzle");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Volta à tela anterior");

		if ( num == EVENT_INTERFACE_EXPORTdefi )  strcpy(text, "Dans les défis (pour tous les joueurs)");
		if ( num == EVENT_INTERFACE_EXPORTdoc  )  strcpy(text, "Dans le dossier \"Mes documents\"");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Dispositivo\\Configurações de driver e resolução");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Gráficos\\Configurações de gráficos");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Jogo\\Configurações do jogo");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controles\\Configurações do teclado, volante e gamepad");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Som\\Volume de música e efeitos sonoros");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolução");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Tela cheia\\Tela cheia ou modo de janela");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Aplicar alterações\\Ativa as configurações alteradas");

		if ( num == EVENT_INTERFACE_SHADOW      )  strcpy(text, "Ombres\\Ombres projetées au sol");
		if ( num == EVENT_INTERFACE_DIRTY       )  strcpy(text, "Salissures\\Salissures des object");
		if ( num == EVENT_INTERFACE_SUNBEAM     )  strcpy(text, "Rayons du soleil\\Rayons selon l'orientation");
		if ( num == EVENT_INTERFACE_LENSFLARE   )  strcpy(text, "Reflets dans l'objectif\\Reflets selon l'orientation");
		if ( num == EVENT_INTERFACE_DECOR       )  strcpy(text, "Objets décoratifs\\Arbres, plantes, etc.");
		if ( num == EVENT_INTERFACE_DETAIL      )  strcpy(text, "Objets détaillés\\Objets détaillés");
		if ( num == EVENT_INTERFACE_METEO       )  strcpy(text, "Conditions météo\\Pluie et neige");
		if ( num == EVENT_INTERFACE_AMBIANCE    )  strcpy(text, "Effets d'ambiance\\");
		if ( num == EVENT_INTERFACE_EXPLOVIB    )  strcpy(text, "Secousses lors d'explosions\\L'écran vibre lors d'une explosion");
		if ( num == EVENT_INTERFACE_SPEEDSCH    )  strcpy(text, "Vitesse de défilement horizontal\\");
		if ( num == EVENT_INTERFACE_SPEEDSCV    )  strcpy(text, "Vitesse de défilement vertical\\");
		if ( num == EVENT_INTERFACE_MOUSESCROLL )  strcpy(text, "Défilement avec la souris\\");
		if ( num == EVENT_INTERFACE_INVSCH      )  strcpy(text, "Inversion défilement horizontal\\");
		if ( num == EVENT_INTERFACE_INVSCV      )  strcpy(text, "Inversion défilement vertical\\");
		if ( num == EVENT_INTERFACE_MOVIE       )  strcpy(text, "Séquences cinématiques\\Séquences d'introduction");
		if ( num == EVENT_INTERFACE_HELP        )  strcpy(text, "Instructions et indices\\Textes explicatifs");
		if ( num == EVENT_INTERFACE_TOOLTIPS    )  strcpy(text, "Bulles d'aide\\Bulles explicatives");
		if ( num == EVENT_INTERFACE_NICEMOUSE   )  strcpy(text, "Souris ombrée\\Jolie souris avec une ombre");
		if ( num == EVENT_INTERFACE_ACCEL       )  strcpy(text, "Accélère si long à résoudre\\Blupi va plus vite après 10 minutes de réflexion");
		if ( num == EVENT_INTERFACE_VOLBLUPI    )  strcpy(text, "Volume des voix de Blupi\\");
		if ( num == EVENT_INTERFACE_VOLSOUND    )  strcpy(text, "Volume des bruitages\\");
		if ( num == EVENT_INTERFACE_VOLAMBIANCE )  strcpy(text, "Volume d'ambiance\\");
		if ( num == EVENT_INTERFACE_SOUND3D     )  strcpy(text, "Som 3D\\Som 3D");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Controles padrão\\Redefine todos os controles para os valores padrão");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Scroll left\\Scrolling to left");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Scroll right\\Scrolling to right");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Scroll up\\Scrolling to up");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Scroll down\\Scrolling to down");
		if ( num == EVENT_INTERFACE_KROTCW )  strcpy(text, "Rotate CW\\Rotate camera clockwise");
		if ( num == EVENT_INTERFACE_KROTCCW)  strcpy(text, "Rotate CCW\\Rotate camera counter clockwise");
		if ( num == EVENT_INTERFACE_KSTOP  )  strcpy(text, "Stoppe action\\Stoppe l'action en cours");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Sair\\Sai da missão ou da corrida atual");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instruções\\Mostra as instruções para a missão atual");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mínima\\Qualidade gráfica mínima (a mais alta taxa de quadros)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Qualidade gráfica normal");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Máxima\\Qualidade gráfica mais alta (a mais baixa taxa de quadros)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silêncio\\Sem som");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Volume de som normal");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Novo nome de piloto");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Selecionar\\Escolha o piloto selecionado");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Novo\\Cria um novo piloto");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Excluir\\Exclui o piloto da lista");

		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Previous screen");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Next screen");
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit BlupiMania");

		if ( num == EVENT_INTERFACE_UNDO   )  strcpy(text, "Annule la dernière action");
		if ( num == EVENT_INTERFACE_STOP   )  strcpy(text, "Stop (\\key stop;)");
		if ( num == EVENT_INTERFACE_SIGN   )  strcpy(text, "Obtenir des indices");
		if ( num == EVENT_LABEL_SIGN       )  strcpy(text, "Indices");

		if ( num == EVENT_EDIT_WATER       )  strcpy(text, "Eau");
		if ( num == EVENT_EDIT_GROUND      )  strcpy(text, "Sol");
		if ( num == EVENT_EDIT_OBJECT      )  strcpy(text, "Objets");
		if ( num == EVENT_EDIT_INFO        )  strcpy(text, "Informations sur le puzzle");

		if ( num == EVENT_CMD              )  strcpy(text, "Linha de comando");
		if ( num == EVENT_SPEED            )  strcpy(text, "Velocidade do jogo");
	}

	if ( type == RES_OBJECT )
	{
		if ( num == OBJECT_BLUPI           )  strcpy(text, "Blupi");
		if ( num == OBJECT_BOX1            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX2            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX3            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX4            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX5            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX6            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX7            )  strcpy(text, "Grosse boule");
		if ( num == OBJECT_BOX8            )  strcpy(text, "Colonne lourde");
		if ( num == OBJECT_BOX9            )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX10           )  strcpy(text, "Caisse");
		if ( num == OBJECT_BOX11           )  strcpy(text, "Caisse sur roues");
		if ( num == OBJECT_BOX12           )  strcpy(text, "Caisse sur roues");
		if ( num == OBJECT_BOX13           )  strcpy(text, "Caisse sur coussin d'air");
		if ( num == OBJECT_KEY1            )  strcpy(text, "Clé orange");
		if ( num == OBJECT_KEY2            )  strcpy(text, "Clé bleue");
		if ( num == OBJECT_DOOR1           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR2           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOOR3           )  strcpy(text, "Porte orange");
		if ( num == OBJECT_DOOR4           )  strcpy(text, "Porte bleue");
		if ( num == OBJECT_DOCK            )  strcpy(text, "Pont-grue");
		if ( num == OBJECT_CATAPULT        )  strcpy(text, "Catapulte");
		if ( num == OBJECT_TRAX            )  strcpy(text, "Pousseur mécanique");
		if ( num == OBJECT_PERFO           )  strcpy(text, "Perforateur mécanique");
		if ( num == OBJECT_GUN             )  strcpy(text, "Destructeur de caisses");
		if ( num == OBJECT_MINE            )  strcpy(text, "Mine");
		if ( num == OBJECT_GLASS1          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_GLASS2          )  strcpy(text, "Vitre blindée");
		if ( num == OBJECT_FIOLE           )  strcpy(text, "Potion de force");
		if ( num == OBJECT_GLU             )  strcpy(text, "Potion de glu");
		if ( num == OBJECT_GOAL            )  strcpy(text, "But à atteindre");
		if ( num == OBJECT_CRAZY           )  strcpy(text, "Crazy bot");
		if ( num == OBJECT_BOT1            )  strcpy(text, "Bot 1");
		if ( num == OBJECT_BOT2            )  strcpy(text, "Bot 2");
		if ( num == OBJECT_BOT3            )  strcpy(text, "Bot 3");
		if ( num == OBJECT_BOT4            )  strcpy(text, "Bot 4");
		if ( num == OBJECT_BOT5            )  strcpy(text, "Bot 5");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Erro");
		if ( num == ERR_CMD             )  strcpy(text, "Comando desconhecido");
		if ( num == ERR_INSTALL         )  strcpy(text, "Wild Wheels não foi instalado corretamente.");
		if ( num == ERR_NOCD            )  strcpy(text, "Insira o CD de Wild Wheels\ne reexecute o jogo.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Erro em movimento de instrução");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Ir a: destino inacessível");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Ir a: destino inacessível");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Ir a: destino ocupado");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Bot inapropriado");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Energia insuficiente");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "A missão ainda não foi concluída (pressione \\key help; para mais detalhes)");
		if ( num == ERR_PUZZLE_ZEROBLUPI)  strcpy(text, "Il doit y avoir au moins un Blupi !");
		if ( num == ERR_PUZZLE_ZEROGOAL )  strcpy(text, "Il doit y avoir au moins un ballon !");
		if ( num == ERR_PUZZLE_LOGOAL   )  strcpy(text, "Il n'y a pas assez de ballons !");
		if ( num == ERR_PUZZLE_HIGOAL   )  strcpy(text, "Il y a trop de ballons !");
		if ( num == ERR_PUZZLE_MAXBLUPI )  strcpy(text, "4 Blupi ou robots au maximum par puzzle !");
		if ( num == ERR_PUZZLE_MAXGOAL  )  strcpy(text, "8 ballons au maximum par puzzle !");
		if ( num == ERR_PUZZLE_MAX      )  strcpy(text, "Il n'est pas possible de créer plus d'objets de ce type !");
		if ( num == ERR_CREATE          )  strcpy(text, "Ce fichier existe déjà.");

		if ( num == INFO_WIN            )  strcpy(text, "<< Parabéns, missão concluída >>");
		if ( num == INFO_LOST           )  strcpy(text, "<< Desculpe, missão fracassada >>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<< Desculpe, missão fracassada >>");
	}

	if ( type == RES_CBOT )
	{
		strcpy(text, "Erro");
		if ( num == TX_OPENPAR       ) strcpy(text, "Parêntese de abertura faltando");
		if ( num == TX_CLOSEPAR      ) strcpy(text, "Parêntese de fechamento faltando ");
		if ( num == TX_NOTBOOL       ) strcpy(text, "A expressão deve retornar um valor booleano");
		if ( num == TX_UNDEFVAR      ) strcpy(text, "Variável não declarada");
		if ( num == TX_BADLEFT       ) strcpy(text, "Atribuição impossível");
		if ( num == TX_ENDOF         ) strcpy(text, "Finalizador de ponto e vírgula faltando");
		if ( num == TX_OUTCASE       ) strcpy(text, "Instrução ""case"" fora de um bloco ""switch""");
		if ( num == TX_NOTERM        ) strcpy(text, "Instruções depois do colchete de fechamento final");
		if ( num == TX_CLOSEBLK      ) strcpy(text, "Fim de bloco faltando");
		if ( num == TX_ELSEWITHOUTIF ) strcpy(text, "Instrução ""else"" sem ""if"" correspondente ");
		if ( num == TX_OPENBLK       ) strcpy(text, "Colchete de abertura faltando ");//début d'un bloc attendu?
		if ( num == TX_BADTYPE       ) strcpy(text, "Tipo errado para a atribuição");
		if ( num == TX_REDEFVAR      ) strcpy(text, "Uma variável não pode ser declarada duas vezes");
		if ( num == TX_BAD2TYPE      ) strcpy(text, "Os tipos dos dois operandos são incompatíveis ");
		if ( num == TX_UNDEFCALL     ) strcpy(text, "Função desconhecida");
		if ( num == TX_MISDOTS       ) strcpy(text, "Sinal "" : "" faltando");
		if ( num == TX_WHILE         ) strcpy(text, "Palavra-chave ""while"" faltando");
		if ( num == TX_BREAK         ) strcpy(text, "Instrução ""break"" fora de um loop");
		if ( num == TX_LABEL         ) strcpy(text, "Uma etiqueta deve ser seguida de ""for"", ""while"", ""do"" ou ""switch""");
		if ( num == TX_NOLABEL       ) strcpy(text, "Esta etiqueta não existe");// Cette étiquette n'existe pas
		if ( num == TX_NOCASE        ) strcpy(text, "Instrução ""case"" faltando");
		if ( num == TX_BADNUM        ) strcpy(text, "Número faltando");
		if ( num == TX_VOID          ) strcpy(text, "Parâmetro nulo");
		if ( num == TX_NOTYP         ) strcpy(text, "Declaração de tipo faltando");
		if ( num == TX_NOVAR         ) strcpy(text, "Nome de variável faltando");
		if ( num == TX_NOFONC        ) strcpy(text, "Nome de função faltando");
		if ( num == TX_OVERPARAM     ) strcpy(text, "Muitos parâmetros");
		if ( num == TX_REDEF         ) strcpy(text, "A função já existe");
		if ( num == TX_LOWPARAM      ) strcpy(text, "Parâmetros faltando ");
		if ( num == TX_BADPARAM      ) strcpy(text, "Nenhuma função deste nome aceita este tipo de parâmetro");
		if ( num == TX_NUMPARAM      ) strcpy(text, "Nenhuma função deste nome aceita este número de parâmetros");
		if ( num == TX_NOITEM        ) strcpy(text, "Este não é um membro desta classe");
		if ( num == TX_DOT           ) strcpy(text, "Este objeto não é um membro de uma classe");
		if ( num == TX_NOCONST       ) strcpy(text, "Construtor apropriado faltando");
		if ( num == TX_REDEFCLASS    ) strcpy(text, "Esta classe já existe");
		if ( num == TX_CLBRK         ) strcpy(text, """ ] "" faltando");
		if ( num == TX_RESERVED      ) strcpy(text, "Palavra-chave reservada de linguagem CBOT");
		if ( num == TX_BADNEW        ) strcpy(text, "Argumento incorreto para ""new""");
		if ( num == TX_OPBRK         ) strcpy(text, """ [ "" esperado");
		if ( num == TX_BADSTRING     ) strcpy(text, "Seqüência faltando");
		if ( num == TX_BADINDEX      ) strcpy(text, "Tipo de índice incorreto");
		if ( num == TX_PRIVATE       ) strcpy(text, "Elementro privado");
		if ( num == TX_NOPUBLIC      ) strcpy(text, "Público requerido");
		if ( num == TX_DIVZERO       ) strcpy(text, "Dividindo por zero");
		if ( num == TX_NOTINIT       ) strcpy(text, "Variável não inicializada");
		if ( num == TX_BADTHROW      ) strcpy(text, "Valor negativo rejeitado por ""throw""");//C'est quoi, ça?
		if ( num == TX_NORETVAL      ) strcpy(text, "A função não retornou um valor ");
		if ( num == TX_NORUN         ) strcpy(text, "Nenhuma função em execução");
		if ( num == TX_NOCALL        ) strcpy(text, "Chamando uma função desconhecida");
		if ( num == TX_NOCLASS       ) strcpy(text, "Esta classe não existe");
		if ( num == TX_NULLPT        ) strcpy(text, "Objeto desconhecido");
		if ( num == TX_OPNAN         ) strcpy(text, "Operação impossível com valor ""nan""");
		if ( num == TX_OUTARRAY      ) strcpy(text, "Acesso além do limite de matriz");
		if ( num == TX_STACKOVER     ) strcpy(text, "Estouro de pilha");
		if ( num == TX_DELETEDPT     ) strcpy(text, "Objeto ilegal");
		if ( num == TX_FILEOPEN      ) strcpy(text, "Não é possível abrir arquivo");
		if ( num == TX_NOTOPEN       ) strcpy(text, "O arquivo não está aberto");
		if ( num == TX_ERRREAD       ) strcpy(text, "Erro de leitura");
		if ( num == TX_ERRWRITE      ) strcpy(text, "Erro de gravação");
	}

	if ( type == RES_KEY )
	{
		if ( num == 0                   )  strcpy(text, "< nenhum >");
		if ( num == VK_LEFT             )  strcpy(text, "Seta esquerda");
		if ( num == VK_RIGHT            )  strcpy(text, "Seta direita");
		if ( num == VK_UP               )  strcpy(text, "Seta acima");
		if ( num == VK_DOWN             )  strcpy(text, "Seta abaixo");
		if ( num == VK_CANCEL           )  strcpy(text, "Control-break");
		if ( num == VK_BACK             )  strcpy(text, "<--");
		if ( num == VK_TAB              )  strcpy(text, "Guia");
		if ( num == VK_CLEAR            )  strcpy(text, "Limpar");
		if ( num == VK_RETURN           )  strcpy(text, "Enter");
		if ( num == VK_SHIFT            )  strcpy(text, "Shift");
		if ( num == VK_CONTROL          )  strcpy(text, "Ctrl");
		if ( num == VK_MENU             )  strcpy(text, "Alt");
		if ( num == VK_PAUSE            )  strcpy(text, "Pause");
		if ( num == VK_CAPITAL          )  strcpy(text, "Caps Lock");
		if ( num == VK_ESCAPE           )  strcpy(text, "Esc");
		if ( num == VK_SPACE            )  strcpy(text, "Barra de espaçamento");
		if ( num == VK_PRIOR            )  strcpy(text, "Page Up");
		if ( num == VK_NEXT             )  strcpy(text, "Page Down");
		if ( num == VK_END              )  strcpy(text, "End");
		if ( num == VK_HOME             )  strcpy(text, "Home");
		if ( num == VK_SELECT           )  strcpy(text, "Selecionar");
		if ( num == VK_EXECUTE          )  strcpy(text, "Executar");
		if ( num == VK_SNAPSHOT         )  strcpy(text, "Print Scrn");
		if ( num == VK_INSERT           )  strcpy(text, "Insert");
		if ( num == VK_DELETE           )  strcpy(text, "Delete");
		if ( num == VK_HELP             )  strcpy(text, "Ajuda");
		if ( num == VK_LWIN             )  strcpy(text, "Janelas à esquerda");
		if ( num == VK_RWIN             )  strcpy(text, "Janelas à direita");
		if ( num == VK_APPS             )  strcpy(text, "Tecla do aplicativo");
		if ( num == VK_NUMPAD0          )  strcpy(text, "NumPad 0");
		if ( num == VK_NUMPAD1          )  strcpy(text, "NumPad 1");
		if ( num == VK_NUMPAD2          )  strcpy(text, "NumPad 2");
		if ( num == VK_NUMPAD3          )  strcpy(text, "NumPad 3");
		if ( num == VK_NUMPAD4          )  strcpy(text, "NumPad 4");
		if ( num == VK_NUMPAD5          )  strcpy(text, "NumPad 5");
		if ( num == VK_NUMPAD6          )  strcpy(text, "NumPad 6");
		if ( num == VK_NUMPAD7          )  strcpy(text, "NumPad 7");
		if ( num == VK_NUMPAD8          )  strcpy(text, "NumPad 8");
		if ( num == VK_NUMPAD9          )  strcpy(text, "NumPad 9");
		if ( num == VK_MULTIPLY         )  strcpy(text, "NumPad *");
		if ( num == VK_ADD              )  strcpy(text, "NumPad +");
		if ( num == VK_SEPARATOR        )  strcpy(text, "NumPad sep");
		if ( num == VK_SUBTRACT         )  strcpy(text, "NumPad -");
		if ( num == VK_DECIMAL          )  strcpy(text, "NumPad .");
		if ( num == VK_DIVIDE           )  strcpy(text, "NumPad /");
		if ( num == VK_F1               )  strcpy(text, "F1");
		if ( num == VK_F2               )  strcpy(text, "F2");
		if ( num == VK_F3               )  strcpy(text, "F3");
		if ( num == VK_F4               )  strcpy(text, "F4");
		if ( num == VK_F5               )  strcpy(text, "F5");
		if ( num == VK_F6               )  strcpy(text, "F6");
		if ( num == VK_F7               )  strcpy(text, "F7");
		if ( num == VK_F8               )  strcpy(text, "F8");
		if ( num == VK_F9               )  strcpy(text, "F9");
		if ( num == VK_F10              )  strcpy(text, "F10");
		if ( num == VK_F11              )  strcpy(text, "F11");
		if ( num == VK_F12              )  strcpy(text, "F12");
		if ( num == VK_F13              )  strcpy(text, "F13");
		if ( num == VK_F14              )  strcpy(text, "F14");
		if ( num == VK_F15              )  strcpy(text, "F15");
		if ( num == VK_F16              )  strcpy(text, "F16");
		if ( num == VK_F17              )  strcpy(text, "F17");
		if ( num == VK_F18              )  strcpy(text, "F18");
		if ( num == VK_F19              )  strcpy(text, "F19");
		if ( num == VK_F20              )  strcpy(text, "F20");
		if ( num == VK_NUMLOCK          )  strcpy(text, "Num Lock");
		if ( num == VK_SCROLL           )  strcpy(text, "Scroll");
		if ( num == VK_ATTN             )  strcpy(text, "Attn");
		if ( num == VK_CRSEL            )  strcpy(text, "CrSel");
		if ( num == VK_EXSEL            )  strcpy(text, "ExSel");
		if ( num == VK_EREOF            )  strcpy(text, "Apagar EOF");
		if ( num == VK_PLAY             )  strcpy(text, "Reproduzir");
		if ( num == VK_ZOOM             )  strcpy(text, "Zoom");
		if ( num == VK_PA1              )  strcpy(text, "PA1");
		if ( num == VK_OEM_CLEAR        )  strcpy(text, "Limpar");
		if ( num == VK_BUTTON1          )  strcpy(text, "Botão 1");
		if ( num == VK_BUTTON2          )  strcpy(text, "Botão 2");
		if ( num == VK_BUTTON3          )  strcpy(text, "Botão 3");
		if ( num == VK_BUTTON4          )  strcpy(text, "Botão 4");
		if ( num == VK_BUTTON5          )  strcpy(text, "Botão 5");
		if ( num == VK_BUTTON6          )  strcpy(text, "Botão 6");
		if ( num == VK_BUTTON7          )  strcpy(text, "Botão 7");
		if ( num == VK_BUTTON8          )  strcpy(text, "Botão 8");
		if ( num == VK_BUTTON9          )  strcpy(text, "Botão 9");
		if ( num == VK_BUTTON10         )  strcpy(text, "Botão 10");
		if ( num == VK_BUTTON11         )  strcpy(text, "Botão 11");
		if ( num == VK_BUTTON12         )  strcpy(text, "Botão 12");
		if ( num == VK_BUTTON13         )  strcpy(text, "Botão 13");
		if ( num == VK_BUTTON14         )  strcpy(text, "Botão 14");
		if ( num == VK_BUTTON15         )  strcpy(text, "Botão 15");
		if ( num == VK_BUTTON16         )  strcpy(text, "Botão 16");
		if ( num == VK_BUTTON17         )  strcpy(text, "Botão 17");
		if ( num == VK_BUTTON18         )  strcpy(text, "Botão 18");
		if ( num == VK_BUTTON19         )  strcpy(text, "Botão 19");
		if ( num == VK_BUTTON20         )  strcpy(text, "Botão 20");
		if ( num == VK_BUTTON21         )  strcpy(text, "Botão 21");
		if ( num == VK_BUTTON22         )  strcpy(text, "Botão 22");
		if ( num == VK_BUTTON23         )  strcpy(text, "Botão 23");
		if ( num == VK_BUTTON24         )  strcpy(text, "Botão 24");
		if ( num == VK_BUTTON25         )  strcpy(text, "Botão 25");
		if ( num == VK_BUTTON26         )  strcpy(text, "Botão 26");
		if ( num == VK_BUTTON27         )  strcpy(text, "Botão 27");
		if ( num == VK_BUTTON28         )  strcpy(text, "Botão 28");
		if ( num == VK_BUTTON29         )  strcpy(text, "Botão 29");
		if ( num == VK_BUTTON30         )  strcpy(text, "Botão 30");
		if ( num == VK_BUTTON31         )  strcpy(text, "Botão 31");
		if ( num == VK_BUTTON32         )  strcpy(text, "Botão 32");
		if ( num == VK_WHEELUP          )  strcpy(text, "Roda acima");
		if ( num == VK_WHEELDOWN        )  strcpy(text, "Roda abaixo");
	}
#endif

	return ( text[0] != 0 );
}


