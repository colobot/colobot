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

static KeyDesc keyTable[16] =
{
	{ KEYRANK_LEFT,		"left;"    },
	{ KEYRANK_RIGHT,	"right;"   },
	{ KEYRANK_UP,		"up;"      },
	{ KEYRANK_DOWN,		"down;"    },
	{ KEYRANK_BRAKE,	"brake;"   },
	{ KEYRANK_HORN,		"horn;"    },
	{ KEYRANK_CAMERA,	"camera;"  },
	{ KEYRANK_NEAR,		"near;"    },
	{ KEYRANK_AWAY,		"away;"    },
	{ KEYRANK_QUIT,		"quit;"    },
	{ KEYRANK_HELP,		"help;"    },
	{ KEYRANK_CBOT,		"cbot;"    },
	{ KEYRANK_SPEED10,	"speed10;" },
	{ KEYRANK_SPEED15,	"speed15;" },
	{ KEYRANK_SPEED20,	"speed20;" },
	{ KEYRANK_SPEED30,	"speed30;" },
};

// Cherche une touche.

BOOL SearchKey(char *cmd, KeyRank &key)
{
	int		i;

	for ( i=0 ; i<16 ; i++ )
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
	char	name[50];
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
	char	buffer[100];

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
		#if _FULL
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /e");
		#endif
		#if _NET | _SCHOOL
		if ( num == RT_VERSION_ID          )  strcpy(text, "School 1.3 /e");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /e");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "S-E 1.3 /e");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximize");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimize");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normal size");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Close");

		if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Player");
		if ( num == RT_KEY_OR              )  strcpy(text, " or ");

#if _EGAMES
		if ( num == RT_TITLE_BASE          )  strcpy(text, "Wild Wheels");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_TITLE_BASE          )  strcpy(text, "BuzzingCars");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missions");
		if ( num == RT_TITLE_FREE          )  strcpy(text, "Free levels");
		if ( num == RT_TITLE_USER          )  strcpy(text, "User levels");
		if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypes");
		if ( num == RT_TITLE_SETUP         )  strcpy(text, "Options");
		if ( num == RT_TITLE_NAME          )  strcpy(text, "Driver name");
		if ( num == RT_TITLE_PERSO         )  strcpy(text, "Select your car");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Select a mission:");
		if ( num == RT_PLAY_LISTf          )  strcpy(text, " Select a circuit:");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " User missions:");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototypes:");
		if ( num == RT_PLAY_PESETAS        )  strcpy(text, " Your credit account:");
		if ( num == RT_PLAY_PCREDIT        )  strcpy(text, "Your savings");
		if ( num == RT_PLAY_PMISSION       )  strcpy(text, "Max earnings");
		if ( num == RT_PLAY_PNEXT          )  strcpy(text, "Next level");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Drivers:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolution:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) First click on the key you want to redefine.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Then push the key you want to use instead.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Driver list:");
		if ( num == RT_PERSO_LEVEL         )  strcpy(text, "Level of difficulty:");
		if ( num == RT_PERSO_BUY           )  strcpy(text, "Buy");

#if _EGAMES
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Quit the mission ?");
#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Do you want to quit Wild Wheels ?");
#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Do you want to quit BuzzingCars ?");
#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Abort\\Abort the current mission");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continue\\Continue the current mission");
#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quit\\Quit Wild Wheels");
#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quit\\Quit BuzzingCars");
#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continue\\Continue the game");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Do you want to delete %s's saved games and cars ?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Delete the existing file %s ?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Delete");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Driver name:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Create");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Cancel");
		if ( num == RT_DIALOG_YESKID       )  strcpy(text, "Yes");
		if ( num == RT_DIALOG_NOKID        )  strcpy(text, "No");
		if ( num == RT_DIALOG_LOADING      )  strcpy(text, "LOADING");

		if ( num == RT_WIN_TIME            )  strcpy(text, "Chronometer");
		if ( num == RT_WIN_SOLID           )  strcpy(text, "Condition of the vehicle");
		if ( num == RT_WIN_CONE            )  strcpy(text, "Respect of surroundings");
		if ( num == RT_WIN_POINTS          )  strcpy(text, "Credits earned");
		if ( num == RT_WIN_BONUS           )  strcpy(text, "Time bonus");
		if ( num == RT_WIN_ARRAY           )  strcpy(text, "Best times:");
		if ( num == RT_WIN_HEADERm         )  strcpy(text, "\t  Car\tDriver\tChrono\tCredits");
		if ( num == RT_WIN_HEADERf         )  strcpy(text, "\t  Car\tDriver\tChrono\t");

		if ( num == RT_START_READY         )  strcpy(text, "Ready ?");
		if ( num == RT_START_QUICK         )  strcpy(text, "3, 2, 1  ...  Ready ?");
		if ( num == RT_START_123           )  strcpy(text, "%d ...");
		if ( num == RT_START_GO            )  strcpy(text, "GO !");
		if ( num == RT_START_REMOTE        )  strcpy(text, "Remote control your car");

		if ( num == RT_RECORD_GALL         )  strcpy(text, "Circuit record (-%.2f) !");
		if ( num == RT_RECORD_GONE         )  strcpy(text, "Lap record (-%.2f) !");
		if ( num == RT_RECORD_LONE         )  strcpy(text, "Best lap (-%.2f) !");
		if ( num == RT_RECORD_QALL         )  strcpy(text, "> Circuit record <");
		if ( num == RT_RECORD_QONE         )  strcpy(text, "> Best lap <");
		if ( num == RT_RACE_LAP4           )  strcpy(text, "4 laps to go");
		if ( num == RT_RACE_LAP3           )  strcpy(text, "3 laps to go");
		if ( num == RT_RACE_LAP2           )  strcpy(text, "2 laps to go");
		if ( num == RT_RACE_LAP1           )  strcpy(text, "Final lap");
		if ( num == RT_DUEL_WIN            )  strcpy(text, "Ghost car defeated (-%.2f) !");
		if ( num == RT_DUEL_LOST           )  strcpy(text, "Too slow (+%.2f) !");

		if ( num == RT_SPEC_MINSPEED       )  strcpy(text, "Car too slow !");
		if ( num == RT_SPEC_MAXSPEED       )  strcpy(text, "Car too fast !");
#if _SE
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "Not available in the special edition version");
#else
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "Not available in the demo version !");
#endif
		if ( num == RT_SPEC_DISPO          )  strcpy(text, "Not yet available !");

		if ( num == RT_MOTOR_REAR          )  strcpy(text, "R");
		if ( num == RT_MOTOR_NEUTRAL       )  strcpy(text, "N");
		if ( num == RT_MOTOR_S1            )  strcpy(text, "1");
		if ( num == RT_MOTOR_S2            )  strcpy(text, "2");
		if ( num == RT_MOTOR_S3            )  strcpy(text, "3");

		if ( num == RT_IO_LIST             )  strcpy(text, "Available duels:");
		if ( num == RT_IO_NAME             )  strcpy(text, "Filename :");
		if ( num == RT_IO_HEADER           )  strcpy(text, "Filename\tMission\tCar\tDriver\tChrono");
#if _EGAMES
		if ( num == RT_IO_RINFO            )  strcpy(text, "You can download new Duels on www.epsitec.ch/wildwheels");
#else
		if ( num == RT_IO_RINFO            )  strcpy(text, "You can download new Duels on www.ceebot.com");
#endif

#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Developed by EPSITEC");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Published by eGames, Inc.");
#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.ceebot.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Developed by EPSITEC");
#endif
#if _SE
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "You have played with the special edition version of Wild Wheels.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "The full version is now available ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... it contains 28 challenging missions and 9 races ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... as well as a \"duel\" mode for comparing performances.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "More details on www.epsitec.ch/wildwheels !");
#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "You have played with the DEMO version of BuzzingCars.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "The full version is now available ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... it contains 28 challenging missions and 9 races ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... as well as a \"duel\" mode for comparing performances.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "More details on www.ceebot.com !");
#endif
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancel");
		if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Next");
		if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Previous");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");
		if ( num == EVENT_BUTTON_CAMERA    )  strcpy(text, "Camera (\\key camera;)");
		if ( num == EVENT_BUTTON_HELP      )  strcpy(text, "Help  (\\key help;)");
		if ( num == EVENT_BUTTON_BRAKE     )  strcpy(text, "Handbrake (\\key brake;)");
		if ( num == EVENT_BUTTON_HORN      )  strcpy(text, "Horn (\\key horn;)");
		if ( num == EVENT_BONUS            )  strcpy(text, "Time Bonus");
		if ( num == EVENT_TIME             )  strcpy(text, "Chronometer");
		if ( num == EVENT_PROGRESS         )  strcpy(text, "Performed tasks");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancel");

		if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missions\\Select mission");
		if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Free races\\Free races without precise goal");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "User\\User levels");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypes under development");
		if ( num == EVENT_INTERFACE_DUEL   )  strcpy(text, "Duels\\Duels against ghost cars");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "New driver\\Choose driver name");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Options\\Preferences");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Restart\\Restart the mission from the beginning");
		if ( num == EVENT_INTERFACE_SAVE   )  strcpy(text, "Save\\Save the duel");
#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Return to Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Return to BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Previous screen");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Next screen");
#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit Wild Wheels");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quit\\Quit Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quit BuzzingCars");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quit\\Quit BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Cancel\\Back to previous screen");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Return to the main menu");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Play\\Start selected mission!");
		if ( num == EVENT_INTERFACE_GHOSTm )  strcpy(text, "Display best ghost car\\Show the ghost that has performed best on this circuit");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Back to previous screen");

		if ( num == EVENT_INTERFACE_WOK    )  strcpy(text, "Save the duel\\Save to disk");
		if ( num == EVENT_INTERFACE_WCANCEL)  strcpy(text, "Cancel\\Back to previous screen");
		if ( num == EVENT_INTERFACE_ROK    )  strcpy(text, "Start duel\\Start the selected duel");
		if ( num == EVENT_INTERFACE_RCANCEL)  strcpy(text, "Cancel\\Back to previous screen");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Device\\Driver and resolution settings");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Graphics\\Graphics settings");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Game\\Game settings");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controls\\Keyboard, wheel and gamepad settings");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sound\\Volume of music and sound effects");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolution");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Full screen\\Full screen or window mode");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Apply changes\\Activates the changed settings");

		if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Shadows\\Shadows on the ground");
		if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Dust\\Dust and dirt on cars and buildings");
		if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Fog\\Fog");
		if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Sunbeams\\Display sunbeams in the sky");
		if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Sky and clouds\\Display real sky images with clouds");
		if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planets and stars\\Display celestial bodies in the sky");
		if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Dynamic lighting\\Use mobile light sources");
		if ( num == EVENT_INTERFACE_SUPER  )  strcpy(text, "Super details\\Super details");  //DR
		if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Number of particles\\Explosions, dust, reflections, etc.");
		if ( num == EVENT_INTERFACE_WHEEL  )  strcpy(text, "Tyre marks\\Tyre marks on the ground");
		if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Depth of field\\Maximum visibility");
		if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Details\\Visual quality of 3D objects");
		if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Textures\\Quality of displayed textures");
		if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Number of decorative objects \\Number of purely ornamental objects");
		if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Particles in the interface\\Sparks springling from mounse pointer");
		if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Reflections on the buttons\\Shiny buttons");
		if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Tool tips\\Explain user interface items");
		if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Film sequences\\Films before and after the missions");
		if ( num == EVENT_INTERFACE_CBACK  )  strcpy(text, "Back view\\Camera behind the car");
		if ( num == EVENT_INTERFACE_CBOARD )  strcpy(text, "Hood view\\Camera on the hood");
		if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Tilt during explosions\\The screen tilts during explosions");
		if ( num == EVENT_INTERFACE_FLASH  )  strcpy(text, "Crash flash\\The screen flashes during a car crash");
		if ( num == EVENT_INTERFACE_BLAST  )  strcpy(text, "Motor may explode\\The motor explodes during high revs on the starting line");
		if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Mouse shadow\\Mouse shadow");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Default controls\\Resets all controls to default values");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Turn left\\Turns steering wheel left");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Turn right\\Turns steering wheel right");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Accelerate\\Speed up");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Brake and Backwards\\Slows down and eventually moves backwards");
		if ( num == EVENT_INTERFACE_KBRAKE )  strcpy(text, "Handbrake\\Handbrake");
		if ( num == EVENT_INTERFACE_KHORN  )  strcpy(text, "Horn\\Horn");
		if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Change camera\\Switches between hood camera and rear camera");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Quit\\Quit the current mission or race");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instructions\\Show instructions for the current mission");

		if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Sound effects:\\Volume of engines, voice, explosions, etc.");
		if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Background sound:\\Volume of audio tracks on the CD");
		if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "3D sound\\3D sound");
		if ( num == EVENT_INTERFACE_COMMENTS) strcpy(text, "Voice of the commentator\\Various comments during races");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Lowest\\Minimum graphic quality (highest frame rate)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Normal graphic quality");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Highest\\Highest graphic quality (lowest frame rate)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silent\\No sound");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Normal sound volume");

		if ( num == EVENT_INTERFACE_STEERING) strcpy(text, "Use a steering wheel or a joystick\\Use a steering wheel or a joystick");
		if ( num == EVENT_INTERFACE_JOYPAD  ) strcpy(text, "Use a gamepad\\Gamepad or keyboard");
		if ( num == EVENT_INTERFACE_FFBc    ) strcpy(text, "Force feedback\\Use force feedback effects");
		if ( num == EVENT_INTERFACE_FFBs    ) strcpy(text, "\\Feedback strength");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\New driver name");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Select\\Choose the selected driver");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "New\\Create a new driver");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Delete\\Delete the driver from the list");

		if ( num == EVENT_INTERFACE_LEVEL1 )  strcpy(text, "Kid\\Really easy kid level");
		if ( num == EVENT_INTERFACE_LEVEL2 )  strcpy(text, "Cool\\Easy playing level");
		if ( num == EVENT_INTERFACE_LEVEL3 )  strcpy(text, "Medium\\Medium playing level");
		if ( num == EVENT_INTERFACE_LEVEL4 )  strcpy(text, "Hard\\Really hard playing level");

		if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "Select\\Start with this car");
		if ( num == EVENT_INTERFACE_PNEXT  )  strcpy(text, "Next >\\Next car");
		if ( num == EVENT_INTERFACE_PPREV  )  strcpy(text, "< Previous\\Previous car");
		if ( num == EVENT_INTERFACE_PGSPEED)  strcpy(text, "Max speed\\");
		if ( num == EVENT_INTERFACE_PGACCEL)  strcpy(text, "Acceleration\\");
		if ( num == EVENT_INTERFACE_PGGRIP )  strcpy(text, "Grip\\");
		if ( num == EVENT_INTERFACE_PGSOLID)  strcpy(text, "Solidity\\");
		if ( num == EVENT_INTERFACE_PPRICE )  strcpy(text, "%s (vehicle price: %d credits)");
		if ( num == EVENT_INTERFACE_PPESETAS) strcpy(text, "Your savings: %d credits");
		if ( num == EVENT_INTERFACE_PSUBMOD0) strcpy(text, "A\\Change the look");
		if ( num == EVENT_INTERFACE_PSUBMOD1) strcpy(text, "B\\Change the look");
		if ( num == EVENT_INTERFACE_PSUBMOD2) strcpy(text, "C\\Change the look");
		if ( num == EVENT_INTERFACE_PSUBMOD3) strcpy(text, "D\\Change the look");
		if ( num >= EVENT_INTERFACE_PCOLOR0 &&
			 num <= EVENT_INTERFACE_PCOLOR19) strcpy(text, "\\Change the color");

		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Condition of the tyres");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Jet temperature");
		if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Task progression");
		if ( num == EVENT_OBJECT_GSPEED    )  strcpy(text, "Speedometer");
		if ( num == EVENT_OBJECT_GRPM      )  strcpy(text, "Rev-counter");
		if ( num == EVENT_OBJECT_GMOTOR    )  strcpy(text, "\\Gearbox");
		if ( num == EVENT_OBJECT_GCOMPASS  )  strcpy(text, "Compass");
		if ( num == EVENT_OBJECT_GGEIGER   )  strcpy(text, "Geiger counter");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "General condition");
		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Tyre condition");
//?		if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-map");
		if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom mini-map");
		if ( num == EVENT_CMD              )  strcpy(text, "Command line");
		if ( num == EVENT_SPEED            )  strcpy(text, "Game speed");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Error");
		if ( num == ERR_CMD             )  strcpy(text, "Unknown command");
		if ( num == ERR_INSTALL         )  strcpy(text, "BuzzingCars has not been installed correctly.");
		if ( num == ERR_NOCD            )  strcpy(text, "Please insert the BuzzingCars CD\nand re-run the game.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Error in instruction move");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Goto: destination inaccessible");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Goto: destination inaccessible");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Goto: destination occupied");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Bot inappropriate");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Not enough energy");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "The mission is not accomplished yet (push \\key help; for more details)");

		if ( num == INFO_WIN            )  strcpy(text, "<<< Well done, mission accomplished >>>");
		if ( num == INFO_LOST           )  strcpy(text, "<<< Sorry, mission failed >>>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<<< Sorry, mission failed >>>");
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
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /d");
		#endif
		#if _NET | _SCHOOL
		if ( num == RT_VERSION_ID          )  strcpy(text, "Ecole 1.3 /d");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /d");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "S-E 1.3 /d");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximale Größe");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Reduzierte Größe");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Normale Größe");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Schließen");

		if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Spieler");
		if ( num == RT_KEY_OR              )  strcpy(text, " oder ");

#if _EGAMES
		if ( num == RT_TITLE_BASE          )  strcpy(text, "Wild Wheels");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_TITLE_BASE          )  strcpy(text, "BuzzingCars");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missionen");
		if ( num == RT_TITLE_FREE          )  strcpy(text, "Freies Spiel");
		if ( num == RT_TITLE_USER          )  strcpy(text, "Zusätzliche Level");
		if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypen");
		if ( num == RT_TITLE_SETUP         )  strcpy(text, "Optionen");
		if ( num == RT_TITLE_NAME          )  strcpy(text, "Name des Fahrers");
		if ( num == RT_TITLE_PERSO         )  strcpy(text, "Wählen Sie ein Autos aus");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Wählen Sie eine Mission :");
		if ( num == RT_PLAY_LISTf          )  strcpy(text, " Wählen Sie eine Rennstrecke :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Zusätzliche Rennstrecken :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototypen :");
		if ( num == RT_PLAY_PESETAS        )  strcpy(text, " Ihr Kontostand :");
		if ( num == RT_PLAY_PCREDIT        )  strcpy(text, "Ihre Ersparnisse");
		if ( num == RT_PLAY_PMISSION       )  strcpy(text, "Max Gewinn");
		if ( num == RT_PLAY_PNEXT          )  strcpy(text, "Nächste Spielstufe");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Display-Driver :");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Auflösung :");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Klicken Sie auf den neu zu belegenden Befehl.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Drücken Sie auf die Taste, die benutzt werden soll.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Fahrerliste :");
		if ( num == RT_PERSO_LEVEL         )  strcpy(text, "Spielstärke :");
		if ( num == RT_PERSO_BUY           )  strcpy(text, "Kaufen");

#if _EGAMES
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Mission beenden ?");
#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Wollen Sie Wild Wheels verlassen ?");
#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Wollen Sie BuzzingCars verlassen ?");
#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Aufgeben\\Mission aufgeben");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Weiterspielen\\Mission weiterspielen");
#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Beenden\\Wild Wheels beenden");
#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Beenden\\BuzzingCars beenden");
#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Weiter spielen\\Weiter spielen");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Wollen Sie den Spielstand von %s löschen ?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Wollen Sie die Datei %s löschen ?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Löschen");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Neuer Fahrer :");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "OK");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Abbrechen");
		if ( num == RT_DIALOG_YESKID       )  strcpy(text, "Ja");
		if ( num == RT_DIALOG_NOKID        )  strcpy(text, "Nein");
		if ( num == RT_DIALOG_LOADING      )  strcpy(text, "Spiel wird geladen");

		if ( num == RT_WIN_TIME            )  strcpy(text, "Zeit");
		if ( num == RT_WIN_SOLID           )  strcpy(text, "Zustand des Autos");
		if ( num == RT_WIN_CONE            )  strcpy(text, "Beachtung der Gegenstände");
		if ( num == RT_WIN_POINTS          )  strcpy(text, "Krediteinheiten");
		if ( num == RT_WIN_BONUS           )  strcpy(text, "Zeitbonus");
		if ( num == RT_WIN_ARRAY           )  strcpy(text, "Beste Zeiten :");
		if ( num == RT_WIN_HEADERm         )  strcpy(text, "\t  Auto\tFahrer\tZeit\tKred.");
		if ( num == RT_WIN_HEADERf         )  strcpy(text, "\t  Auto\tFahrer\tZeit\t");

		if ( num == RT_START_READY         )  strcpy(text, "Fertig ?");
		if ( num == RT_START_QUICK         )  strcpy(text, "3, 2, 1  ...  fertig ?");
		if ( num == RT_START_123           )  strcpy(text, "%d ...");
		if ( num == RT_START_GO            )  strcpy(text, "LOS !");
		if ( num == RT_START_REMOTE        )  strcpy(text, "Steuern Sie Ihr Auto");

		if ( num == RT_RECORD_GALL         )  strcpy(text, "Streckenrekord (-%.2f) !");
		if ( num == RT_RECORD_GONE         )  strcpy(text, "Rundenrekord (-%.2f) !");
		if ( num == RT_RECORD_LONE         )  strcpy(text, "Beste Runde (-%.2f) !");
		if ( num == RT_RECORD_QALL         )  strcpy(text, "> Streckenrekord <");
		if ( num == RT_RECORD_QONE         )  strcpy(text, "> Rundenrekord <");
		if ( num == RT_RACE_LAP4           )  strcpy(text, "Noch 4 Runden");
		if ( num == RT_RACE_LAP3           )  strcpy(text, "Noch 3 Runden");
		if ( num == RT_RACE_LAP2           )  strcpy(text, "Noch 2 Runden");
		if ( num == RT_RACE_LAP1           )  strcpy(text, "Letzte Runde");
		if ( num == RT_DUEL_WIN            )  strcpy(text, "Phantomauto besiegt (-%.2f) !");
		if ( num == RT_DUEL_LOST           )  strcpy(text, "Zu langsam (+%.2f) !");

		if ( num == RT_SPEC_MINSPEED       )  strcpy(text, "Auto zu langsam !");
		if ( num == RT_SPEC_MAXSPEED       )  strcpy(text, "Auto zu schnell !");
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "In der Demo-Version nicht verfügbar !");
		if ( num == RT_SPEC_DISPO          )  strcpy(text, "In der Demo-Version nicht verfügbar !");

		if ( num == RT_MOTOR_REAR          )  strcpy(text, "R");
		if ( num == RT_MOTOR_NEUTRAL       )  strcpy(text, "N");
		if ( num == RT_MOTOR_S1            )  strcpy(text, "1");
		if ( num == RT_MOTOR_S2            )  strcpy(text, "2");
		if ( num == RT_MOTOR_S3            )  strcpy(text, "3");

		if ( num == RT_IO_LIST             )  strcpy(text, "Liste der Duelle :");
		if ( num == RT_IO_NAME             )  strcpy(text, "Name der Datei :");
		if ( num == RT_IO_HEADER           )  strcpy(text, "Datei\tMission\tAuto\tFahrer\tZeit");
#if _EGAMES
		if ( num == RT_IO_RINFO            )  strcpy(text, "Sie können neue Duelle bei www.epsitec.ch/wildwheels herunterladen");
#else
		if ( num == RT_IO_RINFO            )  strcpy(text, "Sie können neue Duelle bei www.ceebot.com herunterladen");
#endif

#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Entwickelt von EPSITEC");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Published by eGames, Inc.");
#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.ceebot.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Entwickelt von EPSITEC");
#endif
#if _EGAMES
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Sie haben mit der DEMO-Version von Wild Wheels gespielt.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "Die Vollversion ist ab sofort erhältlich ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... sie enthält 28 spannende Missionen und 9 Rennstrecken ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... sowie einen \"Duell\" Modus.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Mehr Details auf www.epsitec.ch/wildwheels !");
#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Sie haben mit der DEMO-Version von BuzzingCars gespielt.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "Die Vollversion ist ab sofort erhältlich ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... sie enthält 28 spannende Missionen und 9 Rennstrecken ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... sowie einen \"Duell\" Modus.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Mehr Details auf www.ceebot.com !");
#endif
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Abbrechen");
		if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Weiter");
		if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Zurück");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menü (\\key quit;)");
		if ( num == EVENT_BUTTON_CAMERA    )  strcpy(text, "Kamera (\\key camera;)");
		if ( num == EVENT_BUTTON_HELP      )  strcpy(text, "Hilfe (\\key help;)");
		if ( num == EVENT_BUTTON_BRAKE     )  strcpy(text, "Handbremse (\\key brake;)");
		if ( num == EVENT_BUTTON_HORN      )  strcpy(text, "Hupe (\\key horn;)");
		if ( num == EVENT_BONUS            )  strcpy(text, "Zeitbonus");
		if ( num == EVENT_TIME             )  strcpy(text, "Zeit");
		if ( num == EVENT_PROGRESS         )  strcpy(text, "Fo rtschritt");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Abbrechen");

		if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missionen\\Wählen Sie eine Mission");
		if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Freie Rennen\\Rennen außerhalb der Missionen");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Benutzerlevel\\Benutzerlevel");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypen");
		if ( num == EVENT_INTERFACE_DUEL   )  strcpy(text, "Duelle\\Duelle gegen Phantomautos");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Neuer Fahrer\\Neuen Fahrer auswählen");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Einstellungen\\Sämtliche Einstellungen für das Spiel");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Neustart\\Mission von vorne nocheinmal anfangen");
		if ( num == EVENT_INTERFACE_SAVE   )  strcpy(text, "Speichern\\Duell Speichern");
#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Zu Wild Wheels zurückkehren");
#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Zu BuzzingCars zurückkehren");
#endif
		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Vorhergehender Bildschirm");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Nächster Bildschirm");
#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Wild Wheels verlassen");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Beenden\\Wild Wheels beenden");
#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\BuzzingCars verlassen");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Beenden\\BuzzingCars beenden");
#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Abbrechen\\Zurück zum Hauptmenü");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Zurück zum Hauptmenu");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Spielen ...\\Ausgewählte Mission jetzt spielen");
		if ( num == EVENT_INTERFACE_GHOSTm )  strcpy(text, "Bestes Phantomauto zeigen\\Zeigt das Phantomauto mit der besten Zeit");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Zurück zum vorhergehenden Bildschirm");

		if ( num == EVENT_INTERFACE_WOK    )  strcpy(text, "Duell speichern\\Speichert das Duell in der ausgewählten Datei");
		if ( num == EVENT_INTERFACE_WCANCEL)  strcpy(text, "Abbrechen\\Zurück zum vorhergehenden Bildschirm");
		if ( num == EVENT_INTERFACE_ROK    )  strcpy(text, "Duell starten\\Startet das ausgewählte Duell");
		if ( num == EVENT_INTERFACE_RCANCEL)  strcpy(text, "Abbrechen\\Zurück zum vorhergehenden Bildschirm");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "3D-Karte\\Treiber und 3D-Einstellungen");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Grafik\\Grafikoptionen");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Spiel\\Spieleinstellungen");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Steuerung\\Einstellungen für Taststur u. Steuergeräte");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sound\\Soundeinstellungen");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Auflösung");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Vollbildschirm\\Vollbildschirmmodus oder Fenstermodus");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Änderungen übernehmen\\Übernimmt die vorgenommennen Änderrungen");

		if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Schatten\\Schatten");
		if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Schmutz\\Schmutz auf Autos und Gebäuden");
		if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Nebel\\Nebel");
		if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Sonnenstrahlen\\Sonnenstrahlen je nach Blickrichtung");
		if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Himmel und Wolken\\Himmel und Wolken");
		if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planeten und Sterne\\Planeten und Sterne");
		if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Bewegliche Lichtquellen\\Benutzt bewegliche Lichtquellen");
		if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Anzahl Partikel\\Partikel für Exlosionen, Staub usw.");
		if ( num == EVENT_INTERFACE_WHEEL  )  strcpy(text, "Reifenspuren\\Ihr Auto hinterläßt Reinfenspuren");
		if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Sichtweite\\Maximale Sichtweite");
		if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Details\\Grafikqualität der 3D-Objekte");
		if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Texturen\\Grafikqualität der Texturen");
		if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Anzahl dekorativer Gegenstände\\Anzahl dekorativer Gegenstände");
		if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Funkenregen\\Funkenregen in der Benutzeroberfläche");
		if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Funkelnde Schaltflächen\\Funkelnde Schaltflächen");
		if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Hilfsblasen\\Hilfsblasen");
		if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Filmsequenzen\\Filme vor oder nach den Missionen");
		if ( num == EVENT_INTERFACE_CBACK  )  strcpy(text, "Kamera hinten\\Die Kamera befindet sich hinter dem Auto");
		if ( num == EVENT_INTERFACE_CBOARD )  strcpy(text, "Kamera auf der Motorhaube\\Die Kamera befindet sich auf der Motorhaube");
		if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Erdbeben bei Explosionen\\Der Bildschirm wackelt bei Explosionen");
		if ( num == EVENT_INTERFACE_FLASH  )  strcpy(text, "Blitz bei Zusammenstößen\\Der Bildschirm blitzt bei Zusammenstößen kurz auf");
		if ( num == EVENT_INTERFACE_BLAST  )  strcpy(text, "Motor kann explodieren\\Der Motor explodiert bei zu hohen Drehzahlen vor dem Start");
		if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Mauszeiger mit Schatten\\Mauszeiger mit Schatten");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Standardeinstellungen\\Benutzt wieder die Standardeinstellungen für die Steuerung");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Nach links\\Dreht das Lenkrad nach links");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Nach rechts\\Dreht das Lenkrad nach rechts");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Gas\\Gaspedal");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Bremse und Rückwärtsgang\\Bremse und Rückwärtsgang");
		if ( num == EVENT_INTERFACE_KBRAKE )  strcpy(text, "Handbremse\\Handbremse");
		if ( num == EVENT_INTERFACE_KHORN  )  strcpy(text, "Hupe\\Hupe");
		if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Kamerawechsel\\Wechselt den Standpunkt der Kamera");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Aktuelle Mission beenden\\Aktuelle Mission beenden");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Anweisungen\\Anweisungen für die Mission");

		if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Geräuscheffekte :\\Lautstärke sämtlicher Geräuscheffekte");
		if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Hintergrundmusik :\\Lautstärke der Hintergrundmusik");
		if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "3D-Sound\\3D-Sound");
		if ( num == EVENT_INTERFACE_COMMENTS) strcpy(text, "Stimme des Kommentators\\Diverse Kommentare bei Rennen");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mini\\Minimale Grafikqualität (schneller)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Standard Grafikqualität");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Maxi\\Höchste Grafikqualität (langsamer)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Ruhe\\Keinerlei Geräusche mehr");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Normale Laustärke");

		if ( num == EVENT_INTERFACE_STEERING) strcpy(text, "Joystick oder Lenkrad benutzen\\Joystick oder Lenkrad benutzen");
		if ( num == EVENT_INTERFACE_JOYPAD  ) strcpy(text, "Gamepad benutzen\\Gamepad benutzen");
		if ( num == EVENT_INTERFACE_FFBc    ) strcpy(text, "Force Feedback\\Benutzt Force Feedback Effekte");
		if ( num == EVENT_INTERFACE_FFBs    ) strcpy(text, "\\Feedbackstärke");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Name des Fahrers");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Auswählen\\Wählt den Fahrer in der Liste aus");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Neu\\Erstellt einen neuen Fahrer");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Löschen\\Löscht einen Fahrer in der Liste");

		if ( num == EVENT_INTERFACE_LEVEL1 )  strcpy(text, "Kid\\Für Kinder und Sonntagsfahrer");
		if ( num == EVENT_INTERFACE_LEVEL2 )  strcpy(text, "Cool\\Einfach");
		if ( num == EVENT_INTERFACE_LEVEL3 )  strcpy(text, "Hart\\Mittelschwer");
		if ( num == EVENT_INTERFACE_LEVEL4 )  strcpy(text, "Raser\\Echt schwer");

		if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "Benutzen\\Mit diesem Auto losfahren");
		if ( num == EVENT_INTERFACE_PNEXT  )  strcpy(text, "Zurück >\\Nächstes Auto");
		if ( num == EVENT_INTERFACE_PPREV  )  strcpy(text, "< Weiter\\Vorhergehendes Auto");
		if ( num == EVENT_INTERFACE_PGSPEED)  strcpy(text, "Geschwindigkeit\\");
		if ( num == EVENT_INTERFACE_PGACCEL)  strcpy(text, "Beschleunigung\\");
		if ( num == EVENT_INTERFACE_PGGRIP )  strcpy(text, "Straßenlage\\");
		if ( num == EVENT_INTERFACE_PGSOLID)  strcpy(text, "Robustheit\\");
		if ( num == EVENT_INTERFACE_PPRICE )  strcpy(text, "%s (Preis: %d Krediteinheiten)");
		if ( num == EVENT_INTERFACE_PPESETAS) strcpy(text, "Ihre Ersparnisse: %d Krediteinheiten");
		if ( num == EVENT_INTERFACE_PSUBMOD0) strcpy(text, "A\\Verzierung");
		if ( num == EVENT_INTERFACE_PSUBMOD1) strcpy(text, "B\\Verzierung");
		if ( num == EVENT_INTERFACE_PSUBMOD2) strcpy(text, "C\\Verzierung");
		if ( num == EVENT_INTERFACE_PSUBMOD3) strcpy(text, "D\\Verzierung");
		if ( num >= EVENT_INTERFACE_PCOLOR0 &&
			 num <= EVENT_INTERFACE_PCOLOR19) strcpy(text, "\\Ändert die Farbe");

		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Niveau du bouclier");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Température du réacteur");
		if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Travail en cours ...");
		if ( num == EVENT_OBJECT_GSPEED    )  strcpy(text, "Tacho");
		if ( num == EVENT_OBJECT_GRPM      )  strcpy(text, "Tourenzöhler");
		if ( num == EVENT_OBJECT_GMOTOR    )  strcpy(text, "\\Eingelegter Gang");
		if ( num == EVENT_OBJECT_GCOMPASS  )  strcpy(text, "Kompass");
		if ( num == EVENT_OBJECT_GGEIGER   )  strcpy(text, "Geigerzöhler");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Fahrzeugzustand");
		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Reifenzustand");
//?		if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-carte");
		if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom Mini-Karte");
		if ( num == EVENT_CMD              )  strcpy(text, "Befehlskonsole");
		if ( num == EVENT_SPEED            )  strcpy(text, "Spielgeschwindigkeit");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Erreur");
		if ( num == ERR_CMD             )  strcpy(text, "Unbekannter Befehl");
		if ( num == ERR_INSTALL         )  strcpy(text, "BuzzingCars ist nicht korrekt installiert worden.");
		if ( num == ERR_NOCD            )  strcpy(text, "Fügen Sie die BuzzingCars CD-Rom\nin das Laufwerk ein und starten\nSie das Spiel erneut.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Bewegung nicht möglich");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Weg kann nicht gefunden werden");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Position nicht erreichbar");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Position schon besetzt");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Roboter nicht geeignet");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Nicht genügend Energie");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "Die Mission ist noch nicht beendet (drücken Sie auf \\key help; für mehr Details)");

		if ( num == INFO_WIN            )  strcpy(text, "<<< Bravo, Mission erfolgreich beendet >>>");
		if ( num == INFO_LOST           )  strcpy(text, "<<< Tut mir Leid, Sie haben versagt >>>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<<< Tut mir Leid, Sie haben versagt >>>");
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
		if ( num == VK_CLEAR            )  strcpy(text, "Clear??");
		if ( num == VK_RETURN           )  strcpy(text, "Eingabe");
		if ( num == VK_SHIFT            )  strcpy(text, "Umschalt");
		if ( num == VK_CONTROL          )  strcpy(text, "Strg");
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
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /f");
		#endif
		#if _NET | _SCHOOL
		if ( num == RT_VERSION_ID          )  strcpy(text, "Ecole 1.3 /f");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /f");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "S-E 1.3 /f");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Taille maximale");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Taille réduite");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Taille normale");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Fermer");

		if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Joueur");
		if ( num == RT_KEY_OR              )  strcpy(text, " ou ");

#if _EGAMES
		if ( num == RT_TITLE_BASE          )  strcpy(text, "Wild Wheels");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_TITLE_BASE          )  strcpy(text, "BuzzingCars");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missions");
		if ( num == RT_TITLE_FREE          )  strcpy(text, "Jeu libre");
		if ( num == RT_TITLE_USER          )  strcpy(text, "Niveaux supplémentaires");
		if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototypes");
		if ( num == RT_TITLE_SETUP         )  strcpy(text, "Options");
		if ( num == RT_TITLE_NAME          )  strcpy(text, "Nom du pilote");
		if ( num == RT_TITLE_PERSO         )  strcpy(text, "Choix de votre voiture");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Choix de la mission à effectuer:");
		if ( num == RT_PLAY_LISTf          )  strcpy(text, " Choix du circuit :");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Missions personnelles :");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Liste des prototypes :");
		if ( num == RT_PLAY_PESETAS        )  strcpy(text, " Situation financière :");
		if ( num == RT_PLAY_PCREDIT        )  strcpy(text, "Vos économies");
		if ( num == RT_PLAY_PMISSION       )  strcpy(text, "Gain max possible");
		if ( num == RT_PLAY_PNEXT          )  strcpy(text, "Niveau suivant");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Pilotes :");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Résolutions :");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Cliquez d'abord sur la touche à redéfinir.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Appuyez ensuite sur la nouvelle touche souhaitée.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Liste des pilotes :");
		if ( num == RT_PERSO_LEVEL         )  strcpy(text, "Niveau de difficulté :");
		if ( num == RT_PERSO_BUY           )  strcpy(text, "Acheter");

#if _EGAMES
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Quitter la mission ?");
#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Voulez-vous quitter Wild Wheels ?");
#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Voulez-vous quitter BuzzingCars ?");
#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Abandonner\\Abandonner la mission en cours");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuer\\Continuer la mission en cours");
#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quitter\\Quitter Wild Wheels");
#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Quitter\\Quitter BuzzingCars");
#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuer\\Continuer de jouer");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Voulez-vous détruire la progression et les voitures de %s ?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Voulez-vous supprimer le fichier %s ?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Détruire");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nom du pilote à créer :");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Créer");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Annuler");
		if ( num == RT_DIALOG_YESKID       )  strcpy(text, "Oui");
		if ( num == RT_DIALOG_NOKID        )  strcpy(text, "Non");
		if ( num == RT_DIALOG_LOADING      )  strcpy(text, "CHARGEMENT");

		if ( num == RT_WIN_TIME            )  strcpy(text, "Chronomètre");
		if ( num == RT_WIN_SOLID           )  strcpy(text, "Etat du véhicule");
		if ( num == RT_WIN_CONE            )  strcpy(text, "Respect du décor");
		if ( num == RT_WIN_POINTS          )  strcpy(text, "Crédits gagnés");
		if ( num == RT_WIN_BONUS           )  strcpy(text, "Bonus temps");
		if ( num == RT_WIN_ARRAY           )  strcpy(text, "Classement des meilleurs chronos :");
		if ( num == RT_WIN_HEADERm         )  strcpy(text, "\t  Voiture\tPilote\tChrono\tCrédits");
		if ( num == RT_WIN_HEADERf         )  strcpy(text, "\t  Voiture\tPilote\tChrono\t");

		if ( num == RT_START_READY         )  strcpy(text, "Prêt ?");
		if ( num == RT_START_QUICK         )  strcpy(text, "3, 2, 1  ...  Prêt ?");
		if ( num == RT_START_123           )  strcpy(text, "%d ...");
		if ( num == RT_START_GO            )  strcpy(text, "GO !");
		if ( num == RT_START_REMOTE        )  strcpy(text, "Télécommandez votre voiture");

		if ( num == RT_RECORD_GALL         )  strcpy(text, "Record du circuit (-%.2f) !");
		if ( num == RT_RECORD_GONE         )  strcpy(text, "Record du tour (-%.2f) !");
		if ( num == RT_RECORD_LONE         )  strcpy(text, "Meilleur tour (-%.2f) !");
		if ( num == RT_RECORD_QALL         )  strcpy(text, "> Record du circuit <");
		if ( num == RT_RECORD_QONE         )  strcpy(text, "> Record du tour <");
		if ( num == RT_RACE_LAP4           )  strcpy(text, "Encore 4 tours");
		if ( num == RT_RACE_LAP3           )  strcpy(text, "Encore 3 tours");
		if ( num == RT_RACE_LAP2           )  strcpy(text, "Encore 2 tours");
		if ( num == RT_RACE_LAP1           )  strcpy(text, "Dernier tour");
		if ( num == RT_DUEL_WIN            )  strcpy(text, "Voiture fantôme battue (-%.2f) !");
		if ( num == RT_DUEL_LOST           )  strcpy(text, "Trop lent (+%.2f) !");

		if ( num == RT_SPEC_MINSPEED       )  strcpy(text, "Voiture trop lente !");
		if ( num == RT_SPEC_MAXSPEED       )  strcpy(text, "Voiture trop rapide !");
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "Pas disponible dans la version DEMO !");
		if ( num == RT_SPEC_DISPO          )  strcpy(text, "Pas disponible pour l'instant !");

		if ( num == RT_MOTOR_REAR          )  strcpy(text, "R");
		if ( num == RT_MOTOR_NEUTRAL       )  strcpy(text, "N");
		if ( num == RT_MOTOR_S1            )  strcpy(text, "1");
		if ( num == RT_MOTOR_S2            )  strcpy(text, "2");
		if ( num == RT_MOTOR_S3            )  strcpy(text, "3");

		if ( num == RT_IO_LIST             )  strcpy(text, "Liste des duels disponibles :");
		if ( num == RT_IO_NAME             )  strcpy(text, "Nom du fichier :");
		if ( num == RT_IO_HEADER           )  strcpy(text, "Fichier\tMission\tVoiture\tPilote\tChrono");
#if _EGAMES
		if ( num == RT_IO_RINFO            )  strcpy(text, "Vous pouvez télécharger de nouveaux duels sur www.epsitec.ch/wildwheels");
#else
		if ( num == RT_IO_RINFO            )  strcpy(text, "Vous pouvez télécharger de nouveaux duels sur www.ceebot.com");
#endif

#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Développé par EPSITEC");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Publié par eGames, Inc.");
#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.ceebot.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Développé par EPSITEC");
#endif
#if _EGAMES
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Vous avez joué à la version DEMO de Wild Wheels.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "La version complète est disponible dès aujourd'hui ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... elle comprend 28 missions passionnantes et 9 courses libres ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... ainsi qu'un mode \"duels\" pour se mesurer à d'autres.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Tous les détails sur www.epsitec.ch/wildwheels !");
#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Vous avez joué à la version DEMO de BuzzingCars.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "La version complète est disponible dès aujourd'hui ...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... elle comprend 28 missions passionnantes et 9 courses libres ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... ainsi qu'un mode \"duels\" pour se mesurer à d'autres.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Tous les détails sur www.ceebot.com !");
#endif
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "D'accord");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Annuler");
		if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Suivant");
		if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Précédent");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");
		if ( num == EVENT_BUTTON_CAMERA    )  strcpy(text, "Caméra (\\key camera;)");
		if ( num == EVENT_BUTTON_HELP      )  strcpy(text, "Instructions (\\key help;)");
		if ( num == EVENT_BUTTON_BRAKE     )  strcpy(text, "Frein à main (\\key brake;)");
		if ( num == EVENT_BUTTON_HORN      )  strcpy(text, "Claxon (\\key horn;)");
		if ( num == EVENT_BONUS            )  strcpy(text, "Bonus temps");
		if ( num == EVENT_TIME             )  strcpy(text, "Chronomètre");
		if ( num == EVENT_PROGRESS         )  strcpy(text, "Tâches accomplies");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "D'accord");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Annuler");

		if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missions\\La grande aventure");
		if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Courses libres\\Courses libres sans but précis");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Suppléments\\Niveaux supplémentaires");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Prototypes en cours d'élaboration");
		if ( num == EVENT_INTERFACE_DUEL   )  strcpy(text, "Duels\\Duels contre des voitures fantômes");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Autre pilote\\Choix du nom du pilote");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Options\\Réglages");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Recommencer\\Recommencer la mission au début");
		if ( num == EVENT_INTERFACE_SAVE   )  strcpy(text, "Enregistrer\\Enregistrer le duel");
#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Retourner dans Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Retourner dans BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Ecran précédent");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Ecran suivant");
#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quitter Wild Wheels");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quitter\\Quitter Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Quitter BuzzingCars");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Quitter\\Quitter BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Annuler\\Retour au niveau précédent");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Retour au menu");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Jouer ...\\Démarrer l'action");
		if ( num == EVENT_INTERFACE_GHOSTm )  strcpy(text, "Voir la meilleure voiture fantôme\\Voir la voiture fantôme du meilleur temps");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "D'accord\\Retour au niveau précédent");

		if ( num == EVENT_INTERFACE_WOK    )  strcpy(text, "Enregistrer le duel\\Enregistre dans le fichier choisi");
		if ( num == EVENT_INTERFACE_WCANCEL)  strcpy(text, "Annuler\\Retour à l'écran précédent");
		if ( num == EVENT_INTERFACE_ROK    )  strcpy(text, "Démarrer le duel\\Démarre le duel sélectionné dans la liste");
		if ( num == EVENT_INTERFACE_RCANCEL)  strcpy(text, "Annuler\\Retour à l'écran précédent");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Affichage\\Pilote et résolution d'affichage");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Graphique\\Options graphiques");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Jeu\\Options de jouabilité");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Commandes\\Touches du clavier");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Son\\Volumes bruitages & musiques");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Résolution");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Plein écran\\Plein écran ou fenêtré");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Appliquer les changements\\Active les changements effectués");

		if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Ombres\\Ombres projetées au sol");
		if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Salissures\\Salissures des voitures et bâtiments");
		if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Brouillard\\Nappes de brouillard");
		if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Rayons du soleil\\Rayons selon l'orientation");
		if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Ciel et nuages\\Ciel et nuages");
		if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planètes et étoiles\\Motifs mobiles dans le ciel");
		if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Lumières dynamiques\\Eclairages mobiles");
		if ( num == EVENT_INTERFACE_SUPER  )  strcpy(text, "Voitures super détaillées\\Voitures super détaillées");
		if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Quantité de particules\\Explosions, poussières, reflets, etc.");
		if ( num == EVENT_INTERFACE_WHEEL  )  strcpy(text, "Traces de pneu\\Traces de pneu sur le sol");
		if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Profondeur de champ\\Distance de vue maximale");
		if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Détails des objets\\Qualité des objets en 3D");
		if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Qualité des textures\\Qualité des images");
		if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Nombre d'objets décoratifs\\Quantité d'objets non indispensables");
		if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Particules dans l'interface\\Pluie de particules");
		if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Reflets sur les boutons\\Boutons brillants");
		if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Bulles d'aide\\Bulles explicatives");
		if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Séquences cinématiques\\Films avant ou après une mission");
		if ( num == EVENT_INTERFACE_CBACK  )  strcpy(text, "Caméra arrière\\Caméra derrière la voiture");
		if ( num == EVENT_INTERFACE_CBOARD )  strcpy(text, "Caméra sur le capot\\Caméra sur le capot de la voiture");
		if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Secousses lors d'explosions\\L'écran vibre lors d'une explosion");
		if ( num == EVENT_INTERFACE_FLASH  )  strcpy(text, "Flash lors des collisions\\L'écran flashe lors d'une collision");
		if ( num == EVENT_INTERFACE_BLAST  )  strcpy(text, "Explosion du moteur\\Explosion si démarrage trop violent");
		if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Souris ombrée\\Jolie souris avec une ombre");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Tout réinitialiser\\Remet toutes les touches standards");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Tourner à gauche\\Volant à gauche");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Tourner à droite\\Volant à droite");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Accélérer\\Pédale des gaz");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Freiner et reculer\\Pédale de frein");
		if ( num == EVENT_INTERFACE_KBRAKE )  strcpy(text, "Frein à main\\Frein à main");
		if ( num == EVENT_INTERFACE_KHORN  )  strcpy(text, "Klaxon\\Klaxon");
		if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Changement de caméra\\Autre de point de vue");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Quitter la mission en cours\\Terminer un exercice ou une mssion");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instructions mission\\Marche à suivre");

		if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Bruitages :\\Volume des moteurs, voix, etc.");
		if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Fond sonore :\\Volume des pistes audio du CD");
		if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "Bruitages 3D\\Positionnement sonore dans l'espace");
		if ( num == EVENT_INTERFACE_COMMENTS) strcpy(text, "Voix du commentateur\\Commentaires variés pendant les courses");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mini\\Qualité minimale (+ rapide)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Qualité standard");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Maxi\\Haute qualité (+ lent)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silencieux\\Totalement silencieux");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Niveaux normaux");

		if ( num == EVENT_INTERFACE_STEERING) strcpy(text, "Utilise un volant ou un joystick\\Volant, joystick ou clavier");
		if ( num == EVENT_INTERFACE_JOYPAD  ) strcpy(text, "Utilise une manette de jeu\\Manette de jeu ou clavier");
		if ( num == EVENT_INTERFACE_FFBc    ) strcpy(text, "Retour de force\\Utilise l'effet retour de force");
		if ( num == EVENT_INTERFACE_FFBs    ) strcpy(text, "\\Amplitude de l'effet");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nom du pilote à créer");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Sélectionner\\Choisir le pilote de la liste");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Nouveau\\Créer un nouveau pilote");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Supprimer\\Supprimer le pilote de la liste");

		if ( num == EVENT_INTERFACE_LEVEL1 )  strcpy(text, "Kid\\Niveau pour les petits");
		if ( num == EVENT_INTERFACE_LEVEL2 )  strcpy(text, "Cool\\Niveau facile");
		if ( num == EVENT_INTERFACE_LEVEL3 )  strcpy(text, "Costaud\\Niveau moyen");
		if ( num == EVENT_INTERFACE_LEVEL4 )  strcpy(text, "Maousse\\Niveau difficile");

		if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "Utiliser\\Départ de la course");
		if ( num == EVENT_INTERFACE_PNEXT  )  strcpy(text, "Suivante >\\Voiture suivante");
		if ( num == EVENT_INTERFACE_PPREV  )  strcpy(text, "< Précédente\\Voiture précédente");
		if ( num == EVENT_INTERFACE_PGSPEED)  strcpy(text, "Vitesse maximale\\");
		if ( num == EVENT_INTERFACE_PGACCEL)  strcpy(text, "Accélération\\");
		if ( num == EVENT_INTERFACE_PGGRIP )  strcpy(text, "Tenue de route\\");
		if ( num == EVENT_INTERFACE_PGSOLID)  strcpy(text, "Solidité\\");
		if ( num == EVENT_INTERFACE_PPRICE )  strcpy(text, "%s (prix du véhicule: %d crédits)");
		if ( num == EVENT_INTERFACE_PPESETAS) strcpy(text, "Vos économies: %d crédits");
		if ( num == EVENT_INTERFACE_PSUBMOD0) strcpy(text, "A\\Change le look");
		if ( num == EVENT_INTERFACE_PSUBMOD1) strcpy(text, "B\\Change le look");
		if ( num == EVENT_INTERFACE_PSUBMOD2) strcpy(text, "C\\Change le look");
		if ( num == EVENT_INTERFACE_PSUBMOD3) strcpy(text, "D\\Change le look");
		if ( num >= EVENT_INTERFACE_PCOLOR0 &&
			 num <= EVENT_INTERFACE_PCOLOR19) strcpy(text, "\\Change la couleur");

		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Niveau du bouclier");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Température du réacteur");
		if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Travail en cours ...");
		if ( num == EVENT_OBJECT_GSPEED    )  strcpy(text, "Vitesse");
		if ( num == EVENT_OBJECT_GRPM      )  strcpy(text, "Compte-tours");
		if ( num == EVENT_OBJECT_GMOTOR    )  strcpy(text, "\\Boîte de vitesses");
		if ( num == EVENT_OBJECT_GCOMPASS  )  strcpy(text, "Boussole");
		if ( num == EVENT_OBJECT_GGEIGER   )  strcpy(text, "Compteur Geiger");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Etat général");
		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Etat des pneus");
//?		if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-carte");
		if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom mini-carte");
		if ( num == EVENT_CMD              )  strcpy(text, "Console de commande");
		if ( num == EVENT_SPEED            )  strcpy(text, "Vitesse du jeu");
	}

	if ( type == RES_ERR )
	{
		strcpy(text, "Erreur");
		if ( num == ERR_CMD             )  strcpy(text, "Commande inconnue");
		if ( num == ERR_INSTALL         )  strcpy(text, "BuzzingCars n'est pas installé.");
		if ( num == ERR_NOCD            )  strcpy(text, "Veuillez mettre le CD de BuzzingCars\net relancer le jeu.");
		if ( num == ERR_MOVE_IMPOSSIBLE )  strcpy(text, "Déplacement impossible");
		if ( num == ERR_GOTO_IMPOSSIBLE )  strcpy(text, "Chemin introuvable");
		if ( num == ERR_GOTO_ITER       )  strcpy(text, "Position inaccessible");
		if ( num == ERR_GOTO_BUSY       )  strcpy(text, "Destination occupée");
		if ( num == ERR_FIRE_VEH        )  strcpy(text, "Robot inadapté");
		if ( num == ERR_FIRE_ENERGY     )  strcpy(text, "Pas assez d'énergie");
		if ( num == ERR_MISSION_NOTERM  )  strcpy(text, "La misssion n'est pas terminée (appuyez sur \\key help; pour plus de détails)");

		if ( num == INFO_WIN            )  strcpy(text, "<<< Bravo, mission terminée >>>");
		if ( num == INFO_LOST           )  strcpy(text, "<<< Désolé, mission échouée >>>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<<< Désolé, mission échouée >>>");
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
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /i");
		#endif
		#if _NET | _SCHOOL
		if ( num == RT_VERSION_ID          )  strcpy(text, "Scuola 1.3 /i");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /i");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "S-E 1.3 /i");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Ingrandisci");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Riduci a icona");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Dimensioni normali");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Chiudi");

		if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Giocatore");
		if ( num == RT_KEY_OR              )  strcpy(text, " o ");

#if _EGAMES
		if ( num == RT_TITLE_BASE          )  strcpy(text, "Wild Wheels");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_TITLE_BASE          )  strcpy(text, "BuzzingCars");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missioni");
		if ( num == RT_TITLE_FREE          )  strcpy(text, "Livelli liberi");
		if ( num == RT_TITLE_USER          )  strcpy(text, "Livelli utente");
		if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototipi");
		if ( num == RT_TITLE_SETUP         )  strcpy(text, "Opzioni");
		if ( num == RT_TITLE_NAME          )  strcpy(text, "Nome pilota");
		if ( num == RT_TITLE_PERSO         )  strcpy(text, "Seleziona un'automobile");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Seleziona una missione:");
		if ( num == RT_PLAY_LISTf          )  strcpy(text, " Seleziona un circuito:");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Missioni utente:");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototipi:");
		if ( num == RT_PLAY_PESETAS        )  strcpy(text, " Scheda punteggio:");
		if ( num == RT_PLAY_PCREDIT        )  strcpy(text, "Punteggio accumulato");
		if ( num == RT_PLAY_PMISSION       )  strcpy(text, "Punteggio massimo");
		if ( num == RT_PLAY_PNEXT          )  strcpy(text, "Livello successivo");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Driver:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Risoluzione:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Fai prima clic sul tasto da reimpostare.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Quindi premi il tasto che desideri utilizzare in alternativa.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Elenco piloti:");
		if ( num == RT_PERSO_LEVEL         )  strcpy(text, "Livello di difficoltà:");
		if ( num == RT_PERSO_BUY           )  strcpy(text, "Acquista");

#if _EGAMES
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Uscire dalla missione?");
#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Uscire dal gioco?");
#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Uscire dal gioco?");
#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Interrompi\\Interrompi missione corrente");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continua\\Continua missione corrente");
#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Esci\\Esci da Wild Wheels");
#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Esci\\Esci da BuzzingCars");
#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continua\\Continua gioco");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Eliminare i giochi e le auto di %s salvati?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Eliminare il file %s esistente?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Elimina");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Annulla");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nome pilota:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Crea");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Annulla");
		if ( num == RT_DIALOG_YESKID       )  strcpy(text, "Sì");
		if ( num == RT_DIALOG_NOKID        )  strcpy(text, "No");
		if ( num == RT_DIALOG_LOADING      )  strcpy(text, "CARICAMENTO IN CORSO");

		if ( num == RT_WIN_TIME            )  strcpy(text, "Cronometro");
		if ( num == RT_WIN_SOLID           )  strcpy(text, "Condizioni del veicolo");
		if ( num == RT_WIN_CONE            )  strcpy(text, "Rispetto dell'ambinete circostante");
		if ( num == RT_WIN_POINTS          )  strcpy(text, "Punti guadagnati");
		if ( num == RT_WIN_BONUS           )  strcpy(text, "Bonus temporale");
		if ( num == RT_WIN_ARRAY           )  strcpy(text, "Tempi migliori:");
		if ( num == RT_WIN_HEADERm         )  strcpy(text, "\t  Automobile\tPilota\tCronometro\tPunti");
		if ( num == RT_WIN_HEADERf         )  strcpy(text, "\t  Automobile\tPilota\tCronometro\t");

		if ( num == RT_START_READY         )  strcpy(text, "Pronti ?");
		if ( num == RT_START_QUICK         )  strcpy(text, "3, 2, 1  ...  Pronti ?");
		if ( num == RT_START_123           )  strcpy(text, "%d ...");
		if ( num == RT_START_GO            )  strcpy(text, "VIA !");
		if ( num == RT_START_REMOTE        )  strcpy(text, "Controlla il veicolo a distanza");

		if ( num == RT_RECORD_GALL         )  strcpy(text, "Record circuito (-%.2f) !");
		if ( num == RT_RECORD_GONE         )  strcpy(text, "Record sul giro (-%.2f) !");
		if ( num == RT_RECORD_LONE         )  strcpy(text, "Miglior giro (-%.2f) !");
		if ( num == RT_RECORD_QALL         )  strcpy(text, "> Record circuito <");
		if ( num == RT_RECORD_QONE         )  strcpy(text, "> Migliore giro <");
		if ( num == RT_RACE_LAP4           )  strcpy(text, "4 giri rimanenti");
		if ( num == RT_RACE_LAP3           )  strcpy(text, "3 giri rimanenti");
		if ( num == RT_RACE_LAP2           )  strcpy(text, "2 giri rimanenti");
		if ( num == RT_RACE_LAP1           )  strcpy(text, "Giro finale");
		if ( num == RT_DUEL_WIN            )  strcpy(text, "Automobile fantasma sconfitta (-%.2f) !");
		if ( num == RT_DUEL_LOST           )  strcpy(text, "Troppo lento (+%.2f) !");

		if ( num == RT_SPEC_MINSPEED       )  strcpy(text, "Automobile troppo lenta!");
		if ( num == RT_SPEC_MAXSPEED       )  strcpy(text, "Automobile troppo veloce!");
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "Non disponibile nella versione demo !");
		if ( num == RT_SPEC_DISPO          )  strcpy(text, "Non disponibile nella versione demo !");

		if ( num == RT_MOTOR_REAR          )  strcpy(text, "R");
		if ( num == RT_MOTOR_NEUTRAL       )  strcpy(text, "N");
		if ( num == RT_MOTOR_S1            )  strcpy(text, "1");
		if ( num == RT_MOTOR_S2            )  strcpy(text, "2");
		if ( num == RT_MOTOR_S3            )  strcpy(text, "3");

		if ( num == RT_IO_LIST             )  strcpy(text, "Sfide disponibili:");
		if ( num == RT_IO_NAME             )  strcpy(text, "Nome file:");
		if ( num == RT_IO_HEADER           )  strcpy(text, "Nome file\tMissione\tAutomobile\tPilota\tCronometro");
#if _EGAMES
		if ( num == RT_IO_RINFO            )  strcpy(text, "Puoi scaricare nuove sfide dal sito Web www.epsitec.ch/wildwheels");
#else
		if ( num == RT_IO_RINFO            )  strcpy(text, "Puoi scaricare nuove sfide dal sito Web www.ceebot.com");
#endif

#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Sviluppato da EPSITEC");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Published by eGames, Inc.");
#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.ceebot.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Sviluppato da EPSITEC");
#endif
#if _EGAMES
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Hai giocato con la versione DEMO di Wild Wheels.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "È ora disponibile la versione completa...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... contiene 28 avvincenti missioni e 9 gare ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... nonché una modalità \"sfida\" per mettere a confronto le prestazioni.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Per ulteriori dettagli visitare il sito Web www.epsitec.ch/wildwheels !");
#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Hai giocato con la versione DEMO di BuzzingCars.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "È ora disponibile la versione completa...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... contiene 28 avvincenti missioni e 9 gare ...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... nonché una modalità \"sfida\" per mettere a confronto le prestazioni.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Per ulteriori dettagli visitare il sito Web www.ceebot.com !");
#endif
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Annulla");
		if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Successivo");
		if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Precedente");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");
		if ( num == EVENT_BUTTON_CAMERA    )  strcpy(text, "Videocamera (\\key camera;)");
		if ( num == EVENT_BUTTON_HELP      )  strcpy(text, "? (\\key help;)");
		if ( num == EVENT_BUTTON_BRAKE     )  strcpy(text, "Freno a mano (\\key brake;)");
		if ( num == EVENT_BUTTON_HORN      )  strcpy(text, "Clacson (\\key horn;)");
		if ( num == EVENT_BONUS            )  strcpy(text, "Bonus temporale");
		if ( num == EVENT_TIME             )  strcpy(text, "Cronometro");
		if ( num == EVENT_PROGRESS         )  strcpy(text, "Compiti svolti");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Annulla");

		if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missioni\\Seleziona missione");
		if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Gare libere\\Gare libere senza obiettivi precisi");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Utente\\Livelli utente");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Prototipi\\Prototipi in sviluppo");
		if ( num == EVENT_INTERFACE_DUEL   )  strcpy(text, "Sfide\\Sfide contro automobili fantasma");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Nuovo pilota\\Scegli nome pilota");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opzioni\\Preferenze");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Riavvia\\Riavvia la missione dall'inizio");
		if ( num == EVENT_INTERFACE_SAVE   )  strcpy(text, "Salva\\Salva la sfida");
#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Torna a Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Torna a BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Schermata precedente");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Schermata successiva");
#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Esci da Wild Wheels");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Esci\\Esci da Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Esci da BuzzingCars");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Esci\\Esci da BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Annulla\\Torna alla schermata precedente");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Torna al menu principale");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Gioca\\Inizia la missione selezionata");
		if ( num == EVENT_INTERFACE_GHOSTm )  strcpy(text, "Visualizza migliore auto fantasma\\Visualizza la migliore automobile fantasma del circuito");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Torna alla schermata precedente");

		if ( num == EVENT_INTERFACE_WOK    )  strcpy(text, "Salva la sfida\\Salva su disco");
		if ( num == EVENT_INTERFACE_WCANCEL)  strcpy(text, "Annulla\\Torna alla schermata precedente");
		if ( num == EVENT_INTERFACE_ROK    )  strcpy(text, "Avvia sfida\\Avvia la sfida selezionata");
		if ( num == EVENT_INTERFACE_RCANCEL)  strcpy(text, "Annulla\\Torna alla schermata precedente");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Dispositivo\\Impostazioni driver e risoluzione");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Grafica\\Impostazioni grafiche");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Gioco\\Impostazioni di gioco");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controlli\\Impostazioni tastiera, volante e joypad");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Audio\\Volume di musica ed effetti sonori");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Risoluzione");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Schermo intero\\Schermo intero o modalità finestra");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Applica modifiche\\Attiva le impostazioni modificate");

		if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Ombre\\Ombre sul terreno");
		if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Polvere\\Polvere e sporco su automobili ed edifici");
		if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Nebbia\\Nebbia");
		if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Raggi di sole\\Visualizza raggi di sole nel cielo");
		if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Cielo e nuvole\\Visualizza immagini reali di cielo con nuvole");
		if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Stelle e pianeti\\Visualizza corpi celesti nel cielo");
		if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Illuminazione dinamica\\Utilizza sorgenti luminose mobili");
		if ( num == EVENT_INTERFACE_SUPER  )  strcpy(text, "Dettagli particolareggiati\\Dettagli particolareggiati");  //DR
		if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Numero di particelle\\Esplosioni, polvere, riflessi e altro.");
		if ( num == EVENT_INTERFACE_WHEEL  )  strcpy(text, "Segni di frenate\\Segni di frenate sul tracciato");
		if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Profondità di campo\\Massima visibilità");
		if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Dettagli\\Qualità grafica degli oggetti 3D");
		if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Trame\\Qualità delle trame visualizzate");
		if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Numero di oggetti decorativi\\Numero di oggetti puramente ornamentali");
		if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Particelle dell'interfaccia\\Scintille che si diffondono dal puntatore del mouse");
		if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Riflessi sui pulsanti\\Pulsanti luccicanti");
		if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Descrizione comandi\\Spiegazione degli elementi dell'interfaccia");
		if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Sequenze cinematografiche\\Filmati all'inizio e al termine delle missioni");
		if ( num == EVENT_INTERFACE_CBACK  )  strcpy(text, "Vista posteriore\\Videocamera sul retro dell'automobile");
		if ( num == EVENT_INTERFACE_CBOARD )  strcpy(text, "Vista anteriore\\Videocamera sul cofano");
		if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Ribalta durante esplosioni\\La schermata viene ribaltata durante le esplosioni");
		if ( num == EVENT_INTERFACE_FLASH  )  strcpy(text, "Lampeggio durante gli scontri\\Lo schermo lampeggia durante gli scontri");
		if ( num == EVENT_INTERFACE_BLAST  )  strcpy(text, "Esplosione motore\\In partenza il motore può esplodere al massimo dei giri");
		if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Ombra del muose\\Riflette l'ombra del puntatore del mouse");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Controlli predefiniti\\Ripristina le impostazioni predefinite dei controlli");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Gira a sinistra\\Gira il volante a sinistra");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Gira a destra\\Gira il volante a destra");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Accelera\\Aumenta la velocità");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Freno e retromarcia\\Rallenta e va a retromarcia");
		if ( num == EVENT_INTERFACE_KBRAKE )  strcpy(text, "Freno a mano\\Aziona il freno a mano");
		if ( num == EVENT_INTERFACE_KHORN  )  strcpy(text, "Clacson\\Suona il clacson");
		if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Cambia videocamera\\Passa dalla videocamera del cofano alla videocamera posteriore");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Esci\\Esci dalla missione o dalla gara corrente");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instruzioni\\Visualizza le istruzioni della missione corrente");

		if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Effetti sonori:\\Volume di motori, voce, esplosioni e così via.");
		if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Audio di sfondo:\\Volume delle tracce audio del CD");
		if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "Suono 3D\\Suono 3D");
		if ( num == EVENT_INTERFACE_COMMENTS) strcpy(text, "Voce narratore\\Commenti vari durante le gare");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Minima\\Qualità grafica minima (frequenza di quadro massima)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normale\\Qualità grafica media");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Massima\\Qualità grafica massima (frequenza di quadro minima)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silenzio\\Disattivazione dell'audio");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normale\\Volume dell'audio normale");

		if ( num == EVENT_INTERFACE_STEERING) strcpy(text, "Utilizza un volante o un joystick\\Utilizza un volante o un joystick");
		if ( num == EVENT_INTERFACE_JOYPAD  ) strcpy(text, "Utilizza un joypad\\Joypad o tastiera");
		if ( num == EVENT_INTERFACE_FFBc    ) strcpy(text, "Ritorno di forza\\Usa il ritorno di forza");
		if ( num == EVENT_INTERFACE_FFBs    ) strcpy(text, "\\Intensità del ritorno di forza");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nome nuovo pilota");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Seleziona\\Scegli il pilota selezionato");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Nuovo\\Crea un nuovo pilota");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Elimina\\Elimina il pilota dall'elenco");

		if ( num == EVENT_INTERFACE_LEVEL1 )  strcpy(text, "Principianti\\Livello facilissimo");
		if ( num == EVENT_INTERFACE_LEVEL2 )  strcpy(text, "Intermedi\\Azione di gioco semplice");
		if ( num == EVENT_INTERFACE_LEVEL3 )  strcpy(text, "Esperti\\Azione di gioco intermedia");
		if ( num == EVENT_INTERFACE_LEVEL4 )  strcpy(text, "Campioni\\Livello ultradifficile");

		if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "Seleziona\\Comincia con questa automobile");
		if ( num == EVENT_INTERFACE_PNEXT  )  strcpy(text, "Successiva >\\Automobile successiva");
		if ( num == EVENT_INTERFACE_PPREV  )  strcpy(text, "< Precedente\\Automobile precedente");
		if ( num == EVENT_INTERFACE_PGSPEED)  strcpy(text, "Massima velocità\\");
		if ( num == EVENT_INTERFACE_PGACCEL)  strcpy(text, "Accelerazione\\");
		if ( num == EVENT_INTERFACE_PGGRIP )  strcpy(text, "Tenuta\\");
		if ( num == EVENT_INTERFACE_PGSOLID)  strcpy(text, "Solidità\\");
		if ( num == EVENT_INTERFACE_PPRICE )  strcpy(text, "%s (prezzo del veicolo: (%d punti)");
		if ( num == EVENT_INTERFACE_PPESETAS) strcpy(text, "Punteggio accumulato: %d punti");
		if ( num == EVENT_INTERFACE_PSUBMOD0) strcpy(text, "A\\Modifica l'aspetto");
		if ( num == EVENT_INTERFACE_PSUBMOD1) strcpy(text, "B\\Modifica l'aspetto");
		if ( num == EVENT_INTERFACE_PSUBMOD2) strcpy(text, "C\\Modifica l'aspetto");
		if ( num == EVENT_INTERFACE_PSUBMOD3) strcpy(text, "D\\Modifica l'aspetto");
		if ( num >= EVENT_INTERFACE_PCOLOR0 &&
			 num <= EVENT_INTERFACE_PCOLOR19) strcpy(text, "\\Modifica il colore");

		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Condizioni delle gomme");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Temperatura reattore");
		if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Progresso dei compiti");
		if ( num == EVENT_OBJECT_GSPEED    )  strcpy(text, "Tachimetro");
		if ( num == EVENT_OBJECT_GRPM      )  strcpy(text, "Contagiri");
		if ( num == EVENT_OBJECT_GMOTOR    )  strcpy(text, "\\Scatola del cambio");
		if ( num == EVENT_OBJECT_GCOMPASS  )  strcpy(text, "Bussola");
		if ( num == EVENT_OBJECT_GGEIGER   )  strcpy(text, "Contatore geiger");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Condizioni generali");
		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Condizioni gomme");
//?		if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-map");
		if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Ingrandisci minimappa");
		if ( num == EVENT_CMD              )  strcpy(text, "Riga di comando");
		if ( num == EVENT_SPEED            )  strcpy(text, "Velocità gioco");
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

		if ( num == INFO_WIN            )  strcpy(text, "<<< Congratulazioni, missione compiuta >>>");
		if ( num == INFO_LOST           )  strcpy(text, "<<< Spiacente, missione non riuscita >>>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<<< Spiacente, missione non riuscita >>>");
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
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /s");
		#endif
		#if _NET | _SCHOOL
		if ( num == RT_VERSION_ID          )  strcpy(text, "Colegio 1.3 /s");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /s");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "S-E 1.3 /s");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximizar");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimizar");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Tamaño normal");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Cerrar");

		if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Jugador");
		if ( num == RT_KEY_OR              )  strcpy(text, " o ");

#if _EGAMES
		if ( num == RT_TITLE_BASE          )  strcpy(text, "Wild Wheels");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_TITLE_BASE          )  strcpy(text, "BuzzingCars");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_TITLE_MISSION       )  strcpy(text, "Misiones");
		if ( num == RT_TITLE_FREE          )  strcpy(text, "Niveles libres");
		if ( num == RT_TITLE_USER          )  strcpy(text, "Niveles de usuario");
		if ( num == RT_TITLE_PROTO         )  strcpy(text, "Prototipos");
		if ( num == RT_TITLE_SETUP         )  strcpy(text, "Opciones");
		if ( num == RT_TITLE_NAME          )  strcpy(text, "Nombre del conductor");
		if ( num == RT_TITLE_PERSO         )  strcpy(text, "Selecciona el coche");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Selecciona una misión:");
		if ( num == RT_PLAY_LISTf          )  strcpy(text, " Selecciona un circuito:");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Misiones de usuario:");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Prototipos:");
		if ( num == RT_PLAY_PESETAS        )  strcpy(text, " Tu cuenta de crédito:");
		if ( num == RT_PLAY_PCREDIT        )  strcpy(text, "Tus ahorros");
		if ( num == RT_PLAY_PMISSION       )  strcpy(text, "Ganancias máximas");
		if ( num == RT_PLAY_PNEXT          )  strcpy(text, "Nivel siguiente");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Conductores:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolución:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Haz clic primero en la tecla que quieres redefinir.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Pulsa la tecla que quieres usar en su lugar.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Lista de conductores:");
		if ( num == RT_PERSO_LEVEL         )  strcpy(text, "Nivel de dificultad:");
		if ( num == RT_PERSO_BUY           )  strcpy(text, "Comprar");

#if _EGAMES
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_DIALOG_ABORT        )  strcpy(text, "¿Abandonar la misión?");
#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "¿Quieres salir de Wild Wheels?");
#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "¿Quieres salir de BuzzingCars?");
#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Anular\\Anular la misión actual");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuar\\Continuar la misión actual");
#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Salir\\Salir de Wild Wheels");
#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Salir\\Salir de BuzzingCars");
#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuar\\Continuar el juego");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "¿Quieres eliminar juegos y coches guardados de %s?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "¿Eliminar el archivo %s existente?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Eliminar");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nombre del conductor:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Crear");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_YESKID       )  strcpy(text, "Sí");
		if ( num == RT_DIALOG_NOKID        )  strcpy(text, "No");
		if ( num == RT_DIALOG_LOADING      )  strcpy(text, "CARGANDO");

		if ( num == RT_WIN_TIME            )  strcpy(text, "Cronómetro");
		if ( num == RT_WIN_SOLID           )  strcpy(text, "Estado del vehículo");
		if ( num == RT_WIN_CONE            )  strcpy(text, "Respeto del entorno");
		if ( num == RT_WIN_POINTS          )  strcpy(text, "Créditos ganados");
		if ( num == RT_WIN_BONUS           )  strcpy(text, "Bonificación de tiempo");
		if ( num == RT_WIN_ARRAY           )  strcpy(text, "Mejores tiempos:");
		if ( num == RT_WIN_HEADERm         )  strcpy(text, "\t  Coche\tConductor\tCronómetro\tCréditos");
		if ( num == RT_WIN_HEADERf         )  strcpy(text, "\t  Coche\tConductor\tCronómetro\t");

		if ( num == RT_START_READY         )  strcpy(text, "¿Listo?");
		if ( num == RT_START_QUICK         )  strcpy(text, "3, 2, 1  ...  ¿Listo?");
		if ( num == RT_START_123           )  strcpy(text, "%d ...");
		if ( num == RT_START_GO            )  strcpy(text, "¡ADELANTE!");
		if ( num == RT_START_REMOTE        )  strcpy(text, "Coche con control remoto");

		if ( num == RT_RECORD_GALL         )  strcpy(text, "Récord del circuito (-%.2f)");
		if ( num == RT_RECORD_GONE         )  strcpy(text, "Récord de vuelta (-%.2f)");
		if ( num == RT_RECORD_LONE         )  strcpy(text, "Mejor vuelta (-%.2f)");
		if ( num == RT_RECORD_QALL         )  strcpy(text, "> Récord del circuito <");
		if ( num == RT_RECORD_QONE         )  strcpy(text, "> Mejor vuelta <");
		if ( num == RT_RACE_LAP4           )  strcpy(text, "Quedan 4 vueltas");
		if ( num == RT_RACE_LAP3           )  strcpy(text, "Quedan 3 vueltas");
		if ( num == RT_RACE_LAP2           )  strcpy(text, "Quedan 2 vueltas");
		if ( num == RT_RACE_LAP1           )  strcpy(text, "Última vuelta");
		if ( num == RT_DUEL_WIN            )  strcpy(text, "Se ha derrotado al coche fantasma (-%.2f)");
		if ( num == RT_DUEL_LOST           )  strcpy(text, "Demasiado lento (+%.2f)");

		if ( num == RT_SPEC_MINSPEED       )  strcpy(text, "Coche demasiado lento");
		if ( num == RT_SPEC_MAXSPEED       )  strcpy(text, "Coche demasiado rápido");
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "No disponible en la versión de demostración.");
		if ( num == RT_SPEC_DISPO          )  strcpy(text, "No disponible en la versión de demostración.");

		if ( num == RT_MOTOR_REAR          )  strcpy(text, "R");
		if ( num == RT_MOTOR_NEUTRAL       )  strcpy(text, "N");
		if ( num == RT_MOTOR_S1            )  strcpy(text, "1");
		if ( num == RT_MOTOR_S2            )  strcpy(text, "2");
		if ( num == RT_MOTOR_S3            )  strcpy(text, "3");

		if ( num == RT_IO_LIST             )  strcpy(text, "Duelos disponibles:");
		if ( num == RT_IO_NAME             )  strcpy(text, "Nombre de archivo:");
		if ( num == RT_IO_HEADER           )  strcpy(text, "Nombre de archivo\tMisión\tCoche\tConductor\tCronómetro");
#if _EGAMES
		if ( num == RT_IO_RINFO            )  strcpy(text, "Puedes descargar duelos nuevos de www.epsitec.ch/wildwheels");
#else
		if ( num == RT_IO_RINFO            )  strcpy(text, "Puedes descargar duelos nuevos de www.ceebot.com");
#endif

#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Desarrollado por EPSITEC");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Published by eGames, Inc.");
#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.ceebot.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Desarrollado por EPSITEC");
#endif
#if _EGAMES
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Has jugado con la versión de DEMOSTRACIÓN de Wild Wheels.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "La versión completa ya está disponible...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... consta de 28 misiones desafiantes y 9 carreras...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... así como de un modo de \"duelo\" para comparar actuaciones.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Más información en www.epsitec.ch/wildwheels");
#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Has jugado con la versión de DEMOSTRACIÓN de BuzzingCars.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "La versión completa ya está disponible...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... consta de 28 misiones desafiantes y 9 carreras...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... así como de un modo de \"duelo\" para comparar actuaciones.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Más información en www.ceebot.com");
#endif
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "Aceptar");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancelar");
		if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Siguiente");
		if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Anterior");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menú (\\key quit;)");
		if ( num == EVENT_BUTTON_CAMERA    )  strcpy(text, "Cámara (\\key camera;)");
		if ( num == EVENT_BUTTON_HELP      )  strcpy(text, "Ayuda (\\key help;)");
		if ( num == EVENT_BUTTON_BRAKE     )  strcpy(text, "Freno de mano (\\key brake;)");
		if ( num == EVENT_BUTTON_HORN      )  strcpy(text, "Bocina (\\key horn;)");
		if ( num == EVENT_BONUS            )  strcpy(text, "Bonificación de tiempo");
		if ( num == EVENT_TIME             )  strcpy(text, "Cronómetro");
		if ( num == EVENT_PROGRESS         )  strcpy(text, "Tareas realizadas");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "Aceptar");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancelar");

		if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Misiones\\Seleccionar misión");
		if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Carreras libres\\Carreras libres sin objetivo preciso");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Uuarios\\Niveles de usuario");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Prototipos\\Prototipos en construcción");
		if ( num == EVENT_INTERFACE_DUEL   )  strcpy(text, "Duelos\\Duelos contra coches fantasma");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Conductor nuevo\\Elegir nombre de conductor");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opciones\\Preferencias");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Reiniciar\\Reiniciar la misión desde el principio");
		if ( num == EVENT_INTERFACE_SAVE   )  strcpy(text, "Guardar\\Guardar el duelo");
#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Volver a Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Volver a BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Pantalla anterior");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Pantalla siguiente");
#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Salir de Wild Wheels");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Salir\\Salir de Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Salir de BuzzingCars");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Salir\\Salir de BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Cancelar\\Volver a la pantalla anterior");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menú\\Volver al menú principal");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Jugar\\Comenzar misión seleccionada");
		if ( num == EVENT_INTERFACE_GHOSTm )  strcpy(text, "Mostrar mejor coche fantasma\\Mostrar el coche fantasma con la mejor actuación en este circuito");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "Aceptar\\Volver a la pantalla anterior");

		if ( num == EVENT_INTERFACE_WOK    )  strcpy(text, "Guardar el duelo\\Guardar en disco");
		if ( num == EVENT_INTERFACE_WCANCEL)  strcpy(text, "Cancelar\\Volver a la pantalla anterior");
		if ( num == EVENT_INTERFACE_ROK    )  strcpy(text, "Iniciar duelo\\Iniciar el duelo seleccionado");
		if ( num == EVENT_INTERFACE_RCANCEL)  strcpy(text, "Cancelar\\Volver a la pantalla anterior");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Dispositivo\\Configuración de controlador y resolución");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Gráficos\\Configuración de gráficos");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Juego\\Configuración de juego");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controles\\Configuración de teclado, volante y gamepad");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Sonido\\Volumen de música y efectos de sonido");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolución");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Pantalla completa\\Modo de pantalla completa o ventana");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Aplicar cambios\\Activa la configuración modificada");

		if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Sombras\\Sombras en el suelo");
		if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Polvo\\Polvo y suciedad en coches y edificios");
		if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Niebla\\Niebla");
		if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Rayos de sol\\Mostrar rayos de sol en el cielo");
		if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Cielo y nubes\\Mostrar imágenes reales de cielo con nubes");
		if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planetas y estrellas\\Mostrar cuerpos celestes en el cielo");
		if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Iluminación dinámica\\Usar fuentes de luz móviles");
		if ( num == EVENT_INTERFACE_SUPER  )  strcpy(text, "Estupendos detalles\\Estupendos detalles");  //DR
		if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Número de partículas\\Explosiones, polvo, reflejos, etc.");
		if ( num == EVENT_INTERFACE_WHEEL  )  strcpy(text, "Marcas de neumáticos\\Marcas de neumáticos en el suelo");
		if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Profundidad de campo\\Visibilidad máxima");
		if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Detalles\\Calidad visual de objetos 3D");
		if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Texturas\\Calidad de texturas mostradas");
		if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Número de objetos decorativos\\Número de objetos meramente decorativos");
		if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Partículas en la interfaz\\Chispas que salen del puntero del ratón");
		if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Reflejos en los botones\\Botones brillantes");
		if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Información sobre herramientas\\Explica elementos de la interfaz de usuario");
		if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Secuencias de cine\\Películas antes y después de las misiones");
		if ( num == EVENT_INTERFACE_CBACK  )  strcpy(text, "Vista desde atrás\\Cámara detrás del coche");
		if ( num == EVENT_INTERFACE_CBOARD )  strcpy(text, "Vista desde el capó\\Cámara en el capó");
		if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Inclinación durante explosiones\\La pantalla se inclina durante las explosiones");
		if ( num == EVENT_INTERFACE_FLASH  )  strcpy(text, "Parpadeo durante choques\\La pantalla parpadea durante un choque de coche");
		if ( num == EVENT_INTERFACE_BLAST  )  strcpy(text, "El motor puede explotar\\El motor explota durante la aceleración en la línea de salida");
		if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Sombra de ratón\\Sombra de ratón");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Controles predeterminados\\Restablece todos los controles a los valores predeterminados");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Girar a la izquierda\\Gira el volante a la izquierda");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Girar a la derecha\\Gira el volante a la derecha");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Acelerar\\Acelerar");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Frenar e ir hacia atrás\\Se detiene y se mueve hacia atrás");
		if ( num == EVENT_INTERFACE_KBRAKE )  strcpy(text, "Freno de mano\\Freno de mano");
		if ( num == EVENT_INTERFACE_KHORN  )  strcpy(text, "Bocina\\Bocina");
		if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Cambiar cámara\\Conmuta entre la cámara del capó y la cámara de atrás");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Salir\\Salir de la misión o carrera actual");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instrucciones\\Mostrar instrucciones para la misión actual");

		if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Efectos de sonido:\\Volumen de motores, voz, explosiones, etc.");
		if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Sonido de fondo:\\Volumen de las pistas de sonido del CD");
		if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "Sonido 3D\\Sonido 3D");
		if ( num == EVENT_INTERFACE_COMMENTS) strcpy(text, "Voz del comentarista\\Varios comentarios durante las carreras");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Más baja\\Calidad de gráficos mínima (frecuencia de cuadro más alta)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Calidad de gráficos normal");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Más alta\\Calidad de gráficos más alta (frecuencia de cuadro más baja)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silencio\\Sin sonido");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Volumen de sonido normal");

		if ( num == EVENT_INTERFACE_STEERING) strcpy(text, "Usar un volante o joystick\\Usar un volante o joystick");
		if ( num == EVENT_INTERFACE_JOYPAD  ) strcpy(text, "Usar un gamepad\\Gamepad o teclado");
		if ( num == EVENT_INTERFACE_FFBc    ) strcpy(text, "Force Feedback\\Usar efectos Force Feedback");
		if ( num == EVENT_INTERFACE_FFBs    ) strcpy(text, "\\Intensidad de Force Feedback");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Nombre de nuevo conductor");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Seleccionar\\Elegir el conductor seleccionado");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Nuevo\\Crear un conductor nuevo");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Eliminar\\Eliminar al conductor de la lista");

		if ( num == EVENT_INTERFACE_LEVEL1 )  strcpy(text, "Niño\\Nivel muy fácil para niños");
		if ( num == EVENT_INTERFACE_LEVEL2 )  strcpy(text, "Fácil\\Nivel de juego fácil");
		if ( num == EVENT_INTERFACE_LEVEL3 )  strcpy(text, "Medio\\Nivel de juego medio");
		if ( num == EVENT_INTERFACE_LEVEL4 )  strcpy(text, "Difícil\\Nivel de juego muy difícil");

		if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "Seleccionar\\Iniciar con este coche");
		if ( num == EVENT_INTERFACE_PNEXT  )  strcpy(text, "Siguiente >\\Siguiente coche");
		if ( num == EVENT_INTERFACE_PPREV  )  strcpy(text, "< Anterior\\Coche anterior");
		if ( num == EVENT_INTERFACE_PGSPEED)  strcpy(text, "Velocidad máxima\\");
		if ( num == EVENT_INTERFACE_PGACCEL)  strcpy(text, "Aceleración\\");
		if ( num == EVENT_INTERFACE_PGGRIP )  strcpy(text, "Agarre\\");
		if ( num == EVENT_INTERFACE_PGSOLID)  strcpy(text, "Solidez\\");
		if ( num == EVENT_INTERFACE_PPRICE )  strcpy(text, "%s (precio del vehículo: %d créditos)");
		if ( num == EVENT_INTERFACE_PPESETAS) strcpy(text, "Tus ahorros: %d créditos");
		if ( num == EVENT_INTERFACE_PSUBMOD0) strcpy(text, "A\\Cambiar la apariencia");
		if ( num == EVENT_INTERFACE_PSUBMOD1) strcpy(text, "B\\Cambiar la apariencia");
		if ( num == EVENT_INTERFACE_PSUBMOD2) strcpy(text, "C\\Cambiar la apariencia");
		if ( num == EVENT_INTERFACE_PSUBMOD3) strcpy(text, "D\\Cambiar la apariencia");
		if ( num >= EVENT_INTERFACE_PCOLOR0 &&
			 num <= EVENT_INTERFACE_PCOLOR19) strcpy(text, "\\Cambiar el color");

		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Estado de los neumáticos");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Temperatura del propulsor");
		if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Progresión de la tarea");
		if ( num == EVENT_OBJECT_GSPEED    )  strcpy(text, "Velocímetro");
		if ( num == EVENT_OBJECT_GRPM      )  strcpy(text, "Cuentarrevoluciones");
		if ( num == EVENT_OBJECT_GMOTOR    )  strcpy(text, "\\Caja de cambio");
		if ( num == EVENT_OBJECT_GCOMPASS  )  strcpy(text, "Brújula");
		if ( num == EVENT_OBJECT_GGEIGER   )  strcpy(text, "Contador Geiger");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Estado general");
		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Estado de neumáticos");
//?		if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-map");
		if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Acercar minimapa");
		if ( num == EVENT_CMD              )  strcpy(text, "Línea de comandos");
		if ( num == EVENT_SPEED            )  strcpy(text, "Velocidad de juego");
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

		if ( num == INFO_WIN            )  strcpy(text, "<<< Enhorabuena, misión cumplida >>>");
		if ( num == INFO_LOST           )  strcpy(text, "<<< Lo sentimos, misión no lograda >>>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<<< Lo sentimos, misión no lograda >>>");
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
		if ( num == RT_VERSION_ID          )  strcpy(text, "1.3 /p");
		#endif
		#if _NET | _SCHOOL
		if ( num == RT_VERSION_ID          )  strcpy(text, "Escola 1.3 /p");
		#endif
		#if _DEMO
		if ( num == RT_VERSION_ID          )  strcpy(text, "Demo 1.3 /p");
		#endif
		#if _SE
		if ( num == RT_VERSION_ID          )  strcpy(text, "S-E 1.3 /p");
		#endif
		if ( num == RT_WINDOW_MAXIMIZED    )  strcpy(text, "Maximizar");
		if ( num == RT_WINDOW_MINIMIZED    )  strcpy(text, "Minimizar");
		if ( num == RT_WINDOW_STANDARD     )  strcpy(text, "Tamanho normal");
		if ( num == RT_WINDOW_CLOSE        )  strcpy(text, "Fechar");

		if ( num == RT_NAME_DEFAULT        )  strcpy(text, "Jogador");
		if ( num == RT_KEY_OR              )  strcpy(text, " ou ");

#if _EGAMES
		if ( num == RT_TITLE_BASE          )  strcpy(text, "Wild Wheels");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_TITLE_BASE          )  strcpy(text, "BuzzingCars");
		if ( num == RT_TITLE_INIT          )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_TITLE_MISSION       )  strcpy(text, "Missões");
		if ( num == RT_TITLE_FREE          )  strcpy(text, "Níveis livres");
		if ( num == RT_TITLE_USER          )  strcpy(text, "Níveis do usuário");
		if ( num == RT_TITLE_PROTO         )  strcpy(text, "Protótipos");
		if ( num == RT_TITLE_SETUP         )  strcpy(text, "Opções");
		if ( num == RT_TITLE_NAME          )  strcpy(text, "Nome do piloto");
		if ( num == RT_TITLE_PERSO         )  strcpy(text, "Selecione seu carro");

		if ( num == RT_PLAY_LISTm          )  strcpy(text, " Selecione uma missão:");
		if ( num == RT_PLAY_LISTf          )  strcpy(text, " Selecione um circuito:");
		if ( num == RT_PLAY_LISTu          )  strcpy(text, " Missões do usuário:");
		if ( num == RT_PLAY_LISTp          )  strcpy(text, " Protótipos:");
		if ( num == RT_PLAY_PESETAS        )  strcpy(text, " Sua conta de créditos:");
		if ( num == RT_PLAY_PCREDIT        )  strcpy(text, "Suas economias");
		if ( num == RT_PLAY_PMISSION       )  strcpy(text, "Ganhos máximos");
		if ( num == RT_PLAY_PNEXT          )  strcpy(text, "Próximo nível");

		if ( num == RT_SETUP_DEVICE        )  strcpy(text, " Drivers:");
		if ( num == RT_SETUP_MODE          )  strcpy(text, " Resolução:");
		if ( num == RT_SETUP_KEY1          )  strcpy(text, "1) Primeiro clique na tecla que deseja redefinir.");
		if ( num == RT_SETUP_KEY2          )  strcpy(text, "2) Depois pressione a tecla que deseja usar.");

		if ( num == RT_PERSO_LIST          )  strcpy(text, "Lista de pilotos:");
		if ( num == RT_PERSO_LEVEL         )  strcpy(text, "Nível de dificuldade:");
		if ( num == RT_PERSO_BUY           )  strcpy(text, "Comprar");

#if _EGAMES
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "Wild Wheels");
#else
		if ( num == RT_DIALOG_TITLE        )  strcpy(text, "BuzzingCars");
#endif
		if ( num == RT_DIALOG_ABORT        )  strcpy(text, "Sair da missão?");
#if _EGAMES
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Deseja sair de Wild Wheels?");
#else
		if ( num == RT_DIALOG_QUIT         )  strcpy(text, "Deseja sair de BuzzingCars?");
#endif
		if ( num == RT_DIALOG_YES          )  strcpy(text, "Anular\\Anular a missão atual");
		if ( num == RT_DIALOG_NO           )  strcpy(text, "Continuar\\Continuar a missão atual");
#if _EGAMES
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Sair\\Sair de Wild Wheels");
#else
		if ( num == RT_DIALOG_YESQUIT      )  strcpy(text, "Sair\\Sair de BuzzingCars");
#endif
		if ( num == RT_DIALOG_NOQUIT       )  strcpy(text, "Continuar\\Continuar o jogo");
		if ( num == RT_DIALOG_DELGAME      )  strcpy(text, "Deseja excluir os jogos e os carros salvos de %s?");
		if ( num == RT_DIALOG_DELFILE      )  strcpy(text, "Excluir o arquivo existente %s?");
		if ( num == RT_DIALOG_YESDEL       )  strcpy(text, "Excluir");
		if ( num == RT_DIALOG_NODEL        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_NEWGAME      )  strcpy(text, "Nome do piloto:");
		if ( num == RT_DIALOG_YESNEW       )  strcpy(text, "Criar");
		if ( num == RT_DIALOG_NONEW        )  strcpy(text, "Cancelar");
		if ( num == RT_DIALOG_YESKID       )  strcpy(text, "Sim");
		if ( num == RT_DIALOG_NOKID        )  strcpy(text, "Não");
		if ( num == RT_DIALOG_LOADING      )  strcpy(text, "CARREGANDO");

		if ( num == RT_WIN_TIME            )  strcpy(text, "Cronômetro");
		if ( num == RT_WIN_SOLID           )  strcpy(text, "Condição do veículo");
		if ( num == RT_WIN_CONE            )  strcpy(text, "Respeito ao ambiente");
		if ( num == RT_WIN_POINTS          )  strcpy(text, "Créditos acumulados");
		if ( num == RT_WIN_BONUS           )  strcpy(text, "Bônus de tempo");
		if ( num == RT_WIN_ARRAY           )  strcpy(text, "Melhores tempos:");
		if ( num == RT_WIN_HEADERm         )  strcpy(text, "\t  Carro\tPiloto\tCrono\tCréditos");
		if ( num == RT_WIN_HEADERf         )  strcpy(text, "\t  Carro\tPiloto\tCrono\t");

		if ( num == RT_START_READY         )  strcpy(text, "Pronto?");
		if ( num == RT_START_QUICK         )  strcpy(text, "3, 2, 1  ...  Pronto?");
		if ( num == RT_START_123           )  strcpy(text, "%d ...");
		if ( num == RT_START_GO            )  strcpy(text, "Já!");
		if ( num == RT_START_REMOTE        )  strcpy(text, "Controle seu carro remotamente");

		if ( num == RT_RECORD_GALL         )  strcpy(text, "Recorde do circuito (-%.2f)!");
		if ( num == RT_RECORD_GONE         )  strcpy(text, "Recorde de volta (-%.2f)!");
		if ( num == RT_RECORD_LONE         )  strcpy(text, "Melhor volta (-%.2f)!");
		if ( num == RT_RECORD_QALL         )  strcpy(text, "> Recorde do circuito <");
		if ( num == RT_RECORD_QONE         )  strcpy(text, "> Melhor volta <");
		if ( num == RT_RACE_LAP4           )  strcpy(text, "Faltam 4 voltas");
		if ( num == RT_RACE_LAP3           )  strcpy(text, "Faltam 3 voltas");
		if ( num == RT_RACE_LAP2           )  strcpy(text, "Faltam 2 voltas");
		if ( num == RT_RACE_LAP1           )  strcpy(text, "Última volta");
		if ( num == RT_DUEL_WIN            )  strcpy(text, "Carro fantasma derrotado (-%.2f)!");
		if ( num == RT_DUEL_LOST           )  strcpy(text, "Muito lento (+%.2f)!");

		if ( num == RT_SPEC_MINSPEED       )  strcpy(text, "Carro muito lento!");
		if ( num == RT_SPEC_MAXSPEED       )  strcpy(text, "Carro muito veloz!");
		if ( num == RT_SPEC_DEMO           )  strcpy(text, "Não disponível na versão demo!");
		if ( num == RT_SPEC_DISPO          )  strcpy(text, "Não disponível na versão demo!");

		if ( num == RT_MOTOR_REAR          )  strcpy(text, "R");
		if ( num == RT_MOTOR_NEUTRAL       )  strcpy(text, "N");
		if ( num == RT_MOTOR_S1            )  strcpy(text, "1");
		if ( num == RT_MOTOR_S2            )  strcpy(text, "2");
		if ( num == RT_MOTOR_S3            )  strcpy(text, "3");

		if ( num == RT_IO_LIST             )  strcpy(text, "Duelos disponíveis:");
		if ( num == RT_IO_NAME             )  strcpy(text, "Nome do arquivo:");
		if ( num == RT_IO_HEADER           )  strcpy(text, "Nome do arquivo\tMissão\tCarro\tPiloto\tCrono");
#if _EGAMES
		if ( num == RT_IO_RINFO            )  strcpy(text, "Você pode descarregar novos duelos em www.epsitec.ch/wildwheels");
#else
		if ( num == RT_IO_RINFO            )  strcpy(text, "Você pode descarregar novos duelos em www.ceebot.com");
#endif

#if _EGAMES
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "Desenvolvido por EPSITEC");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Published by eGames, Inc.");
#else
		if ( num == RT_GENERIC_DEV1        )  strcpy(text, "www.ceebot.com");
		if ( num == RT_GENERIC_DEV2        )  strcpy(text, "Desenvolvido por EPSITEC");
#endif
#if _EGAMES
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Você jogou com a versão DEMO de Wild Wheels.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "A versão completa está agora disponível...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... contém 28 missões cheias de desafios e 9 corridas...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... bem como um modo \"duelo\" para comparar desempenhos.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Mais informações em www.epsitec.ch/wildwheels!");
#else
		if ( num == RT_GENERIC_DEMO1       )  strcpy(text, "Você jogou com a versão DEMO de BuzzingCars.");
		if ( num == RT_GENERIC_DEMO2       )  strcpy(text, "A versão completa está agora disponível...");
		if ( num == RT_GENERIC_DEMO3       )  strcpy(text, "... contém 28 missões cheias de desafios e 9 corridas...");
		if ( num == RT_GENERIC_DEMO4       )  strcpy(text, "... bem como um modo \"duelo\" para comparar desempenhos.");
		if ( num == RT_GENERIC_DEMO5       )  strcpy(text, "Mais informações em www.ceebot.com!");
#endif
	}

	if ( type == RES_EVENT )
	{
		if ( num == EVENT_BUTTON_OK        )  strcpy(text, "OK");
		if ( num == EVENT_BUTTON_CANCEL    )  strcpy(text, "Cancelar");
		if ( num == EVENT_BUTTON_NEXT      )  strcpy(text, "Próximo");
		if ( num == EVENT_BUTTON_PREV      )  strcpy(text, "Anterior");
		if ( num == EVENT_BUTTON_QUIT      )  strcpy(text, "Menu (\\key quit;)");
		if ( num == EVENT_BUTTON_CAMERA    )  strcpy(text, "Câmara (\\key camera;)");
		if ( num == EVENT_BUTTON_HELP      )  strcpy(text, "Ajuda (\\key help;)");
		if ( num == EVENT_BUTTON_BRAKE     )  strcpy(text, "Freio de mão (\\key brake;)");
		if ( num == EVENT_BUTTON_HORN      )  strcpy(text, "Buzina (\\key horn;)");
		if ( num == EVENT_BONUS            )  strcpy(text, "Bônus de tempo");
		if ( num == EVENT_TIME             )  strcpy(text, "Cronômetro");
		if ( num == EVENT_PROGRESS         )  strcpy(text, "Tarefas realizadas");

		if ( num == EVENT_DIALOG_OK        )  strcpy(text, "OK");
		if ( num == EVENT_DIALOG_CANCEL    )  strcpy(text, "Cancelar");

		if ( num == EVENT_INTERFACE_MISSION)  strcpy(text, "Missões\\Selecionar missão");
		if ( num == EVENT_INTERFACE_FREE   )  strcpy(text, "Corridas livres\\Corridas livres sem objetivo preciso");
		if ( num == EVENT_INTERFACE_USER   )  strcpy(text, "Usuário\\Níveis do usuário");
		if ( num == EVENT_INTERFACE_PROTO  )  strcpy(text, "Proto\\Protótipos em desenvolvimento");
		if ( num == EVENT_INTERFACE_DUEL   )  strcpy(text, "Duelos\\Duelos contra carros fantasmas");
		if ( num == EVENT_INTERFACE_NAME   )  strcpy(text, "Novo piloto\\Escolha o nome do piloto");
		if ( num == EVENT_INTERFACE_SETUP  )  strcpy(text, "Opções\\Preferências");
		if ( num == EVENT_INTERFACE_AGAIN  )  strcpy(text, "Reiniciar\\Reinicia a missão a partir do começo");
		if ( num == EVENT_INTERFACE_SAVE   )  strcpy(text, "Salvar\\Salva o duelo");
#if _EGAMES
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Volta ao Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_ABORT  )  strcpy(text, "\\Volta ao BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_PREV   )  strcpy(text, "\\Tela anterior");
		if ( num == EVENT_INTERFACE_NEXT   )  strcpy(text, "\\Próxima tela");
#if _EGAMES
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Sai de Wild Wheels");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Sair\\Sai de Wild Wheels");
#else
		if ( num == EVENT_INTERFACE_EXIT   )  strcpy(text, "\\Sai de BuzzingCars");
		if ( num == EVENT_INTERFACE_QUIT   )  strcpy(text, "Sair\\Sai de BuzzingCars");
#endif
		if ( num == EVENT_INTERFACE_BACK   )  strcpy(text, "Cancelar\\Volta à tela anterior");
		if ( num == EVENT_INTERFACE_TERM   )  strcpy(text, "Menu\\Volta ao menu principal");
		if ( num == EVENT_INTERFACE_PLAY   )  strcpy(text, "Reproduzir\\Inicia missão selecionada!");
		if ( num == EVENT_INTERFACE_GHOSTm )  strcpy(text, "Exibir o melhor carro fantasma\\Mostra o fantasma com o melhor desempenho neste circuito");
		if ( num == EVENT_INTERFACE_OK     )  strcpy(text, "OK\\Volta à tela anterior");

		if ( num == EVENT_INTERFACE_WOK    )  strcpy(text, "Salvar o duelo\\Salva em disco");
		if ( num == EVENT_INTERFACE_WCANCEL)  strcpy(text, "Cancelar\\Volta à tela anterior");
		if ( num == EVENT_INTERFACE_ROK    )  strcpy(text, "Iniciar duelo\\Inicia o duelo selecionado");
		if ( num == EVENT_INTERFACE_RCANCEL)  strcpy(text, "Cancelar\\Volta à tela anterior");

		if ( num == EVENT_INTERFACE_SETUPd )  strcpy(text, "Dispositivo\\Configurações de driver e resolução");
		if ( num == EVENT_INTERFACE_SETUPg )  strcpy(text, "Gráficos\\Configurações de gráficos");
		if ( num == EVENT_INTERFACE_SETUPp )  strcpy(text, "Jogo\\Configurações do jogo");
		if ( num == EVENT_INTERFACE_SETUPc )  strcpy(text, "Controles\\Configurações do teclado, volante e gamepad");
		if ( num == EVENT_INTERFACE_SETUPs )  strcpy(text, "Som\\Volume de música e efeitos sonoros");
		if ( num == EVENT_INTERFACE_RESOL  )  strcpy(text, "Resolução");
		if ( num == EVENT_INTERFACE_FULL   )  strcpy(text, "Tela cheia\\Tela cheia ou modo de janela");
		if ( num == EVENT_INTERFACE_APPLY  )  strcpy(text, "Aplicar alterações\\Ativa as configurações alteradas");

		if ( num == EVENT_INTERFACE_SHADOW )  strcpy(text, "Sombras\\Sombras no chão");
		if ( num == EVENT_INTERFACE_DIRTY  )  strcpy(text, "Poeira\\Poeira e sujeira nos carros e prédios");
		if ( num == EVENT_INTERFACE_FOG    )  strcpy(text, "Nevoeiro\\Nevoeiro");
		if ( num == EVENT_INTERFACE_LENS   )  strcpy(text, "Raios de sol\\Exibe raios de sol no céu");
		if ( num == EVENT_INTERFACE_SKY    )  strcpy(text, "Céu e nuvens\\Exibe imagens reais de céu com nuvens");
		if ( num == EVENT_INTERFACE_PLANET )  strcpy(text, "Planetas e astros\\Exibe corpos celestes no céu");
		if ( num == EVENT_INTERFACE_LIGHT  )  strcpy(text, "Iluminação dinâmica\\Usa fontes de luz móveis");
		if ( num == EVENT_INTERFACE_SUPER  )  strcpy(text, "Superdetalhes\\Superdetalhes");  //DR
		if ( num == EVENT_INTERFACE_PARTI  )  strcpy(text, "Várias partículas\\Explosões, poeira, reflexos, etc.");
		if ( num == EVENT_INTERFACE_WHEEL  )  strcpy(text, "Marcas de pneus\\Marcas de pneus no chão");
		if ( num == EVENT_INTERFACE_CLIP   )  strcpy(text, "Profundidade de campo\\Máxima visibilidade");
		if ( num == EVENT_INTERFACE_DETAIL )  strcpy(text, "Detalhes\\Qualidade visual de objetos em 3D");
		if ( num == EVENT_INTERFACE_TEXTURE)  strcpy(text, "Texturas\\Qualidade das texturas exibidas");
		if ( num == EVENT_INTERFACE_GADGET )  strcpy(text, "Vários objetos decorativos\\Vários objetos puramente ornamentais");
		if ( num == EVENT_INTERFACE_RAIN   )  strcpy(text, "Partículas na interface\\Faíscas saltando do ponteiro do mouse");
		if ( num == EVENT_INTERFACE_GLINT  )  strcpy(text, "Reflexos nos botões\\Botões brilhantes");
		if ( num == EVENT_INTERFACE_TOOLTIP)  strcpy(text, "Dicas de ferramentas\\Explica os itens da interface");
		if ( num == EVENT_INTERFACE_MOVIES )  strcpy(text, "Seqüências de filmes\\Filmes antes e depois das missões");
		if ( num == EVENT_INTERFACE_CBACK  )  strcpy(text, "Visão de trás\\Câmara atrás do carro");
		if ( num == EVENT_INTERFACE_CBOARD )  strcpy(text, "Visão do capô\\Câmara no capô do carro");
		if ( num == EVENT_INTERFACE_EFFECT )  strcpy(text, "Inclinação durante explosões\\A tela inclina-se durante explosões");
		if ( num == EVENT_INTERFACE_FLASH  )  strcpy(text, "Clarões de colisão\\Clarões na tela quando um carro bate");
		if ( num == EVENT_INTERFACE_BLAST  )  strcpy(text, "O motor pode explodir\\O motor explode durante revoluções aceleradas na linha de partida");
		if ( num == EVENT_INTERFACE_MOUSE  )  strcpy(text, "Sombra do mouse\\Sombra do mouse");

		if ( num == EVENT_INTERFACE_KDEF   )  strcpy(text, "Controles padrão\\Redefine todos os controles para os valores padrão");
		if ( num == EVENT_INTERFACE_KLEFT  )  strcpy(text, "Virar para a esquerda\\Vira o volante para a esquerda");
		if ( num == EVENT_INTERFACE_KRIGHT )  strcpy(text, "Virar para a direita\\Vira o volante para a direita");
		if ( num == EVENT_INTERFACE_KUP    )  strcpy(text, "Acelerar\\Aumentar a velocidade");
		if ( num == EVENT_INTERFACE_KDOWN  )  strcpy(text, "Freio e retrocesso\\Diminui a velocidade e por fim move-se para trás");
		if ( num == EVENT_INTERFACE_KBRAKE )  strcpy(text, "Freio de mão\\Freio de mão");
		if ( num == EVENT_INTERFACE_KHORN  )  strcpy(text, "Buzina\\Buzina");
		if ( num == EVENT_INTERFACE_KCAMERA)  strcpy(text, "Alterar câmara\\Alterna entre a câmara no capô e a câmara de trás");
		if ( num == EVENT_INTERFACE_KQUIT  )  strcpy(text, "Sair\\Sai da missão ou da corrida atual");
		if ( num == EVENT_INTERFACE_KHELP  )  strcpy(text, "Instruções\\Mostra as instruções para a missão atual");

		if ( num == EVENT_INTERFACE_VOLSOUND) strcpy(text, "Efeitos sonoros:\\Volume de motores, voz, explosões, etc.");
		if ( num == EVENT_INTERFACE_VOLMUSIC) strcpy(text, "Som de fundo:\\Volume das trilhas de áudio no CD");
		if ( num == EVENT_INTERFACE_SOUND3D)  strcpy(text, "Som 3D\\Som 3D");
		if ( num == EVENT_INTERFACE_COMMENTS) strcpy(text, "Voz do comentador\\Vários comentários durante as corridas");

		if ( num == EVENT_INTERFACE_MIN    )  strcpy(text, "Mínima\\Qualidade gráfica mínima (a mais alta taxa de quadros)");
		if ( num == EVENT_INTERFACE_NORM   )  strcpy(text, "Normal\\Qualidade gráfica normal");
		if ( num == EVENT_INTERFACE_MAX    )  strcpy(text, "Máxima\\Qualidade gráfica mais alta (a mais baixa taxa de quadros)");

		if ( num == EVENT_INTERFACE_SILENT )  strcpy(text, "Silêncio\\Sem som");
		if ( num == EVENT_INTERFACE_NOISY  )  strcpy(text, "Normal\\Volume de som normal");

		if ( num == EVENT_INTERFACE_STEERING) strcpy(text, "Usar um volante ou um joystick\\Usa um volante ou um joystick");
		if ( num == EVENT_INTERFACE_JOYPAD  ) strcpy(text, "Usar um gamepad\\Gamepad ou teclado");
		if ( num == EVENT_INTERFACE_FFBc    ) strcpy(text, "Força reativa\\Usa efeitos de força reativa");
		if ( num == EVENT_INTERFACE_FFBs    ) strcpy(text, "\\Força reativa");

		if ( num == EVENT_INTERFACE_NEDIT  )  strcpy(text, "\\Novo nome de piloto");
		if ( num == EVENT_INTERFACE_NOK    )  strcpy(text, "Selecionar\\Escolha o piloto selecionado");
		if ( num == EVENT_INTERFACE_NCREATE)  strcpy(text, "Novo\\Cria um novo piloto");
		if ( num == EVENT_INTERFACE_NDELETE)  strcpy(text, "Excluir\\Exclui o piloto da lista");

		if ( num == EVENT_INTERFACE_LEVEL1 )  strcpy(text, "Infantil\\Nível infantil realmente fácil");
		if ( num == EVENT_INTERFACE_LEVEL2 )  strcpy(text, "Moderado\\Nível de jogo fácil");
		if ( num == EVENT_INTERFACE_LEVEL3 )  strcpy(text, "Médio\\Nível de jogo médio");
		if ( num == EVENT_INTERFACE_LEVEL4 )  strcpy(text, "Difícil\\Nível de jogo realmente difícil");

		if ( num == EVENT_INTERFACE_POK    )  strcpy(text, "Selecionar\\Inicia com este carro");
		if ( num == EVENT_INTERFACE_PNEXT  )  strcpy(text, "Próximo >\\Próximo carro");
		if ( num == EVENT_INTERFACE_PPREV  )  strcpy(text, "< Anterior\\Carro anterior");
		if ( num == EVENT_INTERFACE_PGSPEED)  strcpy(text, "Velocidade máx\\");
		if ( num == EVENT_INTERFACE_PGACCEL)  strcpy(text, "Aceleração\\");
		if ( num == EVENT_INTERFACE_PGGRIP )  strcpy(text, "Empunhadura\\");
		if ( num == EVENT_INTERFACE_PGSOLID)  strcpy(text, "Solidez\\");
		if ( num == EVENT_INTERFACE_PPRICE )  strcpy(text, "%s (preço do veículo: %d créditos)");
		if ( num == EVENT_INTERFACE_PPESETAS) strcpy(text, "Suas economias: %d créditos");
		if ( num == EVENT_INTERFACE_PSUBMOD0) strcpy(text, "A\\Altera a aparência");
		if ( num == EVENT_INTERFACE_PSUBMOD1) strcpy(text, "B\\Altera a aparência");
		if ( num == EVENT_INTERFACE_PSUBMOD2) strcpy(text, "C\\Altera a aparência");
		if ( num == EVENT_INTERFACE_PSUBMOD3) strcpy(text, "D\\Altera a aparência");
		if ( num >= EVENT_INTERFACE_PCOLOR0 &&
			 num <= EVENT_INTERFACE_PCOLOR19) strcpy(text, "\\Altera a cor");

		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Condição dos pneus");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Temperatura do jato");
		if ( num == EVENT_OBJECT_GPROGRESS )  strcpy(text, "Progressão da tarefa");
		if ( num == EVENT_OBJECT_GSPEED    )  strcpy(text, "Velocímetro");
		if ( num == EVENT_OBJECT_GRPM      )  strcpy(text, "Contador de revoluções");
		if ( num == EVENT_OBJECT_GMOTOR    )  strcpy(text, "\\Caixa de câmbio");
		if ( num == EVENT_OBJECT_GCOMPASS  )  strcpy(text, "Bússola");
		if ( num == EVENT_OBJECT_GGEIGER   )  strcpy(text, "Medidor Geiger");
		if ( num == EVENT_OBJECT_GLEVEL1   )  strcpy(text, "Condição geral");
		if ( num == EVENT_OBJECT_GLEVEL2   )  strcpy(text, "Condição dos pneus");
//?		if ( num == EVENT_OBJECT_MAP       )  strcpy(text, "Mini-map");
		if ( num == EVENT_OBJECT_MAPZOOM   )  strcpy(text, "Zoom em minimapa");
		if ( num == EVENT_CMD              )  strcpy(text, "Linha de comando");
		if ( num == EVENT_SPEED            )  strcpy(text, "Velocidade do jogo");
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

		if ( num == INFO_WIN            )  strcpy(text, "<<< Parabéns, missão concluída >>>");
		if ( num == INFO_LOST           )  strcpy(text, "<<< Desculpe, missão fracassada >>>");
		if ( num == INFO_LOSTq          )  strcpy(text, "<<< Desculpe, missão fracassada >>>");
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


