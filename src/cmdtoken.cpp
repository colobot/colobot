// cmdtoken.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "event.h"
#include "water.h"
#include "pyro.h"
#include "camera.h"
#include "object.h"
#include "cmdtoken.h"





// Saute les espaces.

char* SkipSpace(char *line)
{
	while ( *line == ' ' )
	{
		line ++;
	}
	return line;
}

// Vérifie si une ligne contient une commande.

BOOL Cmd(char *line, char *token)
{
	char*	p;

	line = SkipSpace(line);
	p = strstr(line, token);
	return ( p == line );  // commande au début ?
}

// Cherche un opérateur.

char* SearchOp(char *line, char *op)
{
	char	opeq[50];
	char*	p;

	strcpy(opeq, " ");
	strcat(opeq, op);
	strcat(opeq, "=");

	p = strstr(line, opeq);
	if ( p == 0 )  // pas trouvé ?
	{
		return line+strlen(line);  // pointe le zéro terminateur
	}
	return p+strlen(opeq);  // pointe après le "="
}

// Cherche le nième argument.

char* SearchArg(char *line, int rank)
{
	int		i;
	char	c;

	for ( i=0 ; i<rank ; i++ )
	{
		while ( TRUE )
		{
			c = *line++;
			if ( c == ';' )  break;
			if ( c == '=' ||
				 c ==  0  )  return line+strlen(line);
		}
	}

	line = SkipSpace(line);
	return line;
}

// Lit un nombre entier, éventuellement hexadécimal.

int GetInt(char *line, int rank, int def)
{
	char*	p;
	int		n = 0;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return def;

	if ( p[0] == '0' &&
		 p[1] == 'x' )  // commence par "0x" (hexa) ?
	{
		p += 2;
		while ( TRUE )
		{
			if ( *p >= '0' && *p <= '9' )
			{
				n *= 16;
				n += (*p++)-'0';
				continue;
			}
			if ( *p >= 'a' && *p <= 'f' )
			{
				n *= 16;
				n += (*p++)-'a'+10;
				continue;
			}
			break;
		}
	}
	else	// nombre entier ?
	{
		sscanf(p, "%d", &n);
	}
	return n;
}

// Lit un nombre réel.

float GetFloat(char *line, int rank, float def)
{
	char*	p;
	float	n = 0.0f;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return def;

	sscanf(p, "%f", &n);
	return n;
}

// Lit une chaîne.

void GetString(char *line, int rank, char *buffer)
{
	char*	p;

	p = SearchArg(line, rank);
	*buffer = 0;
	if ( *p++ != '"' )  return;

	while ( p[0] != 0 )
	{
		if ( p[0] == '"' &&
			 p[1] == '"' )
		{
			*buffer++ = *p++;
			p++;
			continue;
		}
		if ( p[0] == '"' )  break;

		*buffer++ = *p++;
	}
	*buffer = 0;
}

// Retourne le type d'un objet.

ObjectType GetTypeObject(char *line, int rank, ObjectType def)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return def;

	if ( Cmd(p, "All"               ) )  return OBJECT_NULL;
	if ( Cmd(p, "Trax"              ) )  return OBJECT_TRAX;
	if ( Cmd(p, "Perfo"             ) )  return OBJECT_PERFO;
	if ( Cmd(p, "TargetBot"         ) )  return OBJECT_MOBILEtg;
	if ( Cmd(p, "FireballBot"       ) )  return OBJECT_MOBILEfb;
	if ( Cmd(p, "OrgaballBot"       ) )  return OBJECT_MOBILEob;
	if ( Cmd(p, "Carrot"            ) )  return OBJECT_CARROT;
	if ( Cmd(p, "Walker"            ) )  return OBJECT_WALKER;
	if ( Cmd(p, "Crazy"             ) )  return OBJECT_CRAZY;
	if ( Cmd(p, "WayPoint"          ) )  return OBJECT_WAYPOINT;
	if ( Cmd(p, "BarrelAtomic"      ) )  return OBJECT_BARRELa;
	if ( Cmd(p, "Barrel"            ) )  return OBJECT_BARREL;
	if ( Cmd(p, "NuclearCell"       ) )  return OBJECT_ATOMIC;
	if ( Cmd(p, "TitaniumOre"       ) )  return OBJECT_STONE;
	if ( Cmd(p, "UraniumOre"        ) )  return OBJECT_URANIUM;
	if ( Cmd(p, "Titanium"          ) )  return OBJECT_METAL;
	if ( Cmd(p, "OrgaMatter"        ) )  return OBJECT_BULLET;
	if ( Cmd(p, "BlackBox"          ) )  return OBJECT_BBOX;
	if ( Cmd(p, "Fiole"             ) )  return OBJECT_FIOLE;
	if ( Cmd(p, "Lift"              ) )  return OBJECT_LIFT;
	if ( Cmd(p, "Goal"              ) )  return OBJECT_GOAL;
	if ( Cmd(p, "Column1"           ) )  return OBJECT_COLUMN1;
	if ( Cmd(p, "Column2"           ) )  return OBJECT_COLUMN2;
	if ( Cmd(p, "Column3"           ) )  return OBJECT_COLUMN3;
	if ( Cmd(p, "Column4"           ) )  return OBJECT_COLUMN4;
	if ( Cmd(p, "Blupi"             ) )  return OBJECT_BLUPI;
	if ( Cmd(p, "Bot1"              ) )  return OBJECT_BOT1;
	if ( Cmd(p, "Bot2"              ) )  return OBJECT_BOT2;
	if ( Cmd(p, "Bot3"              ) )  return OBJECT_BOT3;
	if ( Cmd(p, "Bot4"              ) )  return OBJECT_BOT4;
	if ( Cmd(p, "Bot5"              ) )  return OBJECT_BOT5;
	if ( Cmd(p, "TNT"               ) )  return OBJECT_TNT;
	if ( Cmd(p, "Mine"              ) )  return OBJECT_MINE;
	if ( Cmd(p, "Glass1"            ) )  return OBJECT_GLASS1;
	if ( Cmd(p, "Glass2"            ) )  return OBJECT_GLASS2;
	if ( Cmd(p, "Bird"              ) )  return OBJECT_BIRD;
	if ( Cmd(p, "Pterodactyl"       ) )  return OBJECT_PTERO;
	if ( Cmd(p, "Fish"              ) )  return OBJECT_FISH;
	if ( Cmd(p, "Snake"             ) )  return OBJECT_SNAKE;
	if ( Cmd(p, "Subm"              ) )  return OBJECT_SUBM;
	if ( Cmd(p, "Jet"               ) )  return OBJECT_JET;
	if ( Cmd(p, "Max1x"             ) )  return OBJECT_MAX1X;
	if ( Cmd(p, "Firework"          ) )  return OBJECT_WINFIRE;
	if ( Cmd(p, "Mark"              ) )  return OBJECT_MARK;
	if ( Cmd(p, "Greenery10"        ) )  return OBJECT_PLANT10;
	if ( Cmd(p, "Greenery11"        ) )  return OBJECT_PLANT11;
	if ( Cmd(p, "Greenery12"        ) )  return OBJECT_PLANT12;
	if ( Cmd(p, "Greenery13"        ) )  return OBJECT_PLANT13;
	if ( Cmd(p, "Greenery14"        ) )  return OBJECT_PLANT14;
	if ( Cmd(p, "Greenery15"        ) )  return OBJECT_PLANT15;
	if ( Cmd(p, "Greenery16"        ) )  return OBJECT_PLANT16;
	if ( Cmd(p, "Greenery17"        ) )  return OBJECT_PLANT17;
	if ( Cmd(p, "Greenery18"        ) )  return OBJECT_PLANT18;
	if ( Cmd(p, "Greenery19"        ) )  return OBJECT_PLANT19;
	if ( Cmd(p, "Greenery0"         ) )  return OBJECT_PLANT0;
	if ( Cmd(p, "Greenery1"         ) )  return OBJECT_PLANT1;
	if ( Cmd(p, "Greenery2"         ) )  return OBJECT_PLANT2;
	if ( Cmd(p, "Greenery3"         ) )  return OBJECT_PLANT3;
	if ( Cmd(p, "Greenery4"         ) )  return OBJECT_PLANT4;
	if ( Cmd(p, "Greenery5"         ) )  return OBJECT_PLANT5;
	if ( Cmd(p, "Greenery6"         ) )  return OBJECT_PLANT6;
	if ( Cmd(p, "Greenery7"         ) )  return OBJECT_PLANT7;
	if ( Cmd(p, "Greenery8"         ) )  return OBJECT_PLANT8;
	if ( Cmd(p, "Greenery9"         ) )  return OBJECT_PLANT9;
	if ( Cmd(p, "Tree0"             ) )  return OBJECT_TREE0;
	if ( Cmd(p, "Tree1"             ) )  return OBJECT_TREE1;
	if ( Cmd(p, "Tree2"             ) )  return OBJECT_TREE2;
	if ( Cmd(p, "Tree3"             ) )  return OBJECT_TREE3;
	if ( Cmd(p, "Tree4"             ) )  return OBJECT_TREE4;
	if ( Cmd(p, "Tree5"             ) )  return OBJECT_TREE5;
	if ( Cmd(p, "Tree6"             ) )  return OBJECT_TREE6;
	if ( Cmd(p, "Tree7"             ) )  return OBJECT_TREE7;
	if ( Cmd(p, "Tree8"             ) )  return OBJECT_TREE8;
	if ( Cmd(p, "Tree9"             ) )  return OBJECT_TREE9;
	if ( Cmd(p, "Mushroom0"         ) )  return OBJECT_MUSHROOM0;
	if ( Cmd(p, "Mushroom1"         ) )  return OBJECT_MUSHROOM1;
	if ( Cmd(p, "Mushroom2"         ) )  return OBJECT_MUSHROOM2;
	if ( Cmd(p, "Mushroom3"         ) )  return OBJECT_MUSHROOM3;
	if ( Cmd(p, "Mushroom4"         ) )  return OBJECT_MUSHROOM4;
	if ( Cmd(p, "Mushroom5"         ) )  return OBJECT_MUSHROOM5;
	if ( Cmd(p, "Mushroom6"         ) )  return OBJECT_MUSHROOM6;
	if ( Cmd(p, "Mushroom7"         ) )  return OBJECT_MUSHROOM7;
	if ( Cmd(p, "Mushroom8"         ) )  return OBJECT_MUSHROOM8;
	if ( Cmd(p, "Mushroom9"         ) )  return OBJECT_MUSHROOM9;
	if ( Cmd(p, "Building10"        ) )  return OBJECT_BUILDING10;
	if ( Cmd(p, "Building1"         ) )  return OBJECT_BUILDING1;
	if ( Cmd(p, "Building2"         ) )  return OBJECT_BUILDING2;
	if ( Cmd(p, "Building3"         ) )  return OBJECT_BUILDING3;
	if ( Cmd(p, "Building4"         ) )  return OBJECT_BUILDING4;
	if ( Cmd(p, "Building5"         ) )  return OBJECT_BUILDING5;
	if ( Cmd(p, "Building6"         ) )  return OBJECT_BUILDING6;
	if ( Cmd(p, "Building7"         ) )  return OBJECT_BUILDING7;
	if ( Cmd(p, "Building8"         ) )  return OBJECT_BUILDING8;
	if ( Cmd(p, "Building9"         ) )  return OBJECT_BUILDING9;
	if ( Cmd(p, "Carcass10"         ) )  return OBJECT_CARCASS10;
	if ( Cmd(p, "Carcass1"          ) )  return OBJECT_CARCASS1;
	if ( Cmd(p, "Carcass2"          ) )  return OBJECT_CARCASS2;
	if ( Cmd(p, "Carcass3"          ) )  return OBJECT_CARCASS3;
	if ( Cmd(p, "Carcass4"          ) )  return OBJECT_CARCASS4;
	if ( Cmd(p, "Carcass5"          ) )  return OBJECT_CARCASS5;
	if ( Cmd(p, "Carcass6"          ) )  return OBJECT_CARCASS6;
	if ( Cmd(p, "Carcass7"          ) )  return OBJECT_CARCASS7;
	if ( Cmd(p, "Carcass8"          ) )  return OBJECT_CARCASS8;
	if ( Cmd(p, "Carcass9"          ) )  return OBJECT_CARCASS9;
	if ( Cmd(p, "Orga10"            ) )  return OBJECT_ORGA10;
	if ( Cmd(p, "Orga1"             ) )  return OBJECT_ORGA1;
	if ( Cmd(p, "Orga2"             ) )  return OBJECT_ORGA2;
	if ( Cmd(p, "Orga3"             ) )  return OBJECT_ORGA3;
	if ( Cmd(p, "Orga4"             ) )  return OBJECT_ORGA4;
	if ( Cmd(p, "Orga5"             ) )  return OBJECT_ORGA5;
	if ( Cmd(p, "Orga6"             ) )  return OBJECT_ORGA6;
	if ( Cmd(p, "Orga7"             ) )  return OBJECT_ORGA7;
	if ( Cmd(p, "Orga8"             ) )  return OBJECT_ORGA8;
	if ( Cmd(p, "Orga9"             ) )  return OBJECT_ORGA9;
	if ( Cmd(p, "Door1"             ) )  return OBJECT_DOOR1;
	if ( Cmd(p, "Door2"             ) )  return OBJECT_DOOR2;
	if ( Cmd(p, "Door3"             ) )  return OBJECT_DOOR3;
	if ( Cmd(p, "Door4"             ) )  return OBJECT_DOOR4;
	if ( Cmd(p, "Door5"             ) )  return OBJECT_DOOR5;
	if ( Cmd(p, "Dock"              ) )  return OBJECT_DOCK;
	if ( Cmd(p, "Catapult"          ) )  return OBJECT_CATAPULT;
	if ( Cmd(p, "StartArea"         ) )  return OBJECT_START;
	if ( Cmd(p, "GoalArea"          ) )  return OBJECT_END;
	if ( Cmd(p, "WreckBotw1"        ) )  return OBJECT_RUINmobilew1;
	if ( Cmd(p, "WreckBotw2"        ) )  return OBJECT_RUINmobilew2;
	if ( Cmd(p, "WreckBott1"        ) )  return OBJECT_RUINmobilet1;
	if ( Cmd(p, "WreckBott2"        ) )  return OBJECT_RUINmobilet2;
	if ( Cmd(p, "WreckBotr1"        ) )  return OBJECT_RUINmobiler1;
	if ( Cmd(p, "WreckBotr2"        ) )  return OBJECT_RUINmobiler2;
	if ( Cmd(p, "RuinBotFactory"    ) )  return OBJECT_RUINfactory;
	if ( Cmd(p, "RuinDoor"          ) )  return OBJECT_RUINdoor;
	if ( Cmd(p, "RuinSupport"       ) )  return OBJECT_RUINsupport;
	if ( Cmd(p, "RuinRadar"         ) )  return OBJECT_RUINradar;
	if ( Cmd(p, "RuinConvert"       ) )  return OBJECT_RUINconvert;
	if ( Cmd(p, "RuinBaseCamp"      ) )  return OBJECT_RUINbase;
	if ( Cmd(p, "RuinHeadCamp"      ) )  return OBJECT_RUINhead;
	if ( Cmd(p, "Barrier10"         ) )  return OBJECT_BARRIER10;
	if ( Cmd(p, "Barrier11"         ) )  return OBJECT_BARRIER11;
	if ( Cmd(p, "Barrier12"         ) )  return OBJECT_BARRIER12;
	if ( Cmd(p, "Barrier13"         ) )  return OBJECT_BARRIER13;
	if ( Cmd(p, "Barrier14"         ) )  return OBJECT_BARRIER14;
	if ( Cmd(p, "Barrier15"         ) )  return OBJECT_BARRIER15;
	if ( Cmd(p, "Barrier16"         ) )  return OBJECT_BARRIER16;
	if ( Cmd(p, "Barrier17"         ) )  return OBJECT_BARRIER17;
	if ( Cmd(p, "Barrier18"         ) )  return OBJECT_BARRIER18;
	if ( Cmd(p, "Barrier19"         ) )  return OBJECT_BARRIER19;
	if ( Cmd(p, "Barrier20"         ) )  return OBJECT_BARRIER20;
	if ( Cmd(p, "Barrier21"         ) )  return OBJECT_BARRIER21;
	if ( Cmd(p, "Barrier22"         ) )  return OBJECT_BARRIER22;
	if ( Cmd(p, "Barrier23"         ) )  return OBJECT_BARRIER23;
	if ( Cmd(p, "Barrier24"         ) )  return OBJECT_BARRIER24;
	if ( Cmd(p, "Barrier25"         ) )  return OBJECT_BARRIER25;
	if ( Cmd(p, "Barrier26"         ) )  return OBJECT_BARRIER26;
	if ( Cmd(p, "Barrier27"         ) )  return OBJECT_BARRIER27;
	if ( Cmd(p, "Barrier28"         ) )  return OBJECT_BARRIER28;
	if ( Cmd(p, "Barrier29"         ) )  return OBJECT_BARRIER29;
	if ( Cmd(p, "Barrier30"         ) )  return OBJECT_BARRIER30;
	if ( Cmd(p, "Barrier31"         ) )  return OBJECT_BARRIER31;
	if ( Cmd(p, "Barrier32"         ) )  return OBJECT_BARRIER32;
	if ( Cmd(p, "Barrier33"         ) )  return OBJECT_BARRIER33;
	if ( Cmd(p, "Barrier34"         ) )  return OBJECT_BARRIER34;
	if ( Cmd(p, "Barrier35"         ) )  return OBJECT_BARRIER35;
	if ( Cmd(p, "Barrier36"         ) )  return OBJECT_BARRIER36;
	if ( Cmd(p, "Barrier37"         ) )  return OBJECT_BARRIER37;
	if ( Cmd(p, "Barrier38"         ) )  return OBJECT_BARRIER38;
	if ( Cmd(p, "Barrier39"         ) )  return OBJECT_BARRIER39;
	if ( Cmd(p, "Barrier40"         ) )  return OBJECT_BARRIER40;
	if ( Cmd(p, "Barrier41"         ) )  return OBJECT_BARRIER41;
	if ( Cmd(p, "Barrier42"         ) )  return OBJECT_BARRIER42;
	if ( Cmd(p, "Barrier43"         ) )  return OBJECT_BARRIER43;
	if ( Cmd(p, "Barrier44"         ) )  return OBJECT_BARRIER44;
	if ( Cmd(p, "Barrier45"         ) )  return OBJECT_BARRIER45;
	if ( Cmd(p, "Barrier46"         ) )  return OBJECT_BARRIER46;
	if ( Cmd(p, "Barrier47"         ) )  return OBJECT_BARRIER47;
	if ( Cmd(p, "Barrier48"         ) )  return OBJECT_BARRIER48;
	if ( Cmd(p, "Barrier49"         ) )  return OBJECT_BARRIER49;
	if ( Cmd(p, "Barrier50"         ) )  return OBJECT_BARRIER50;
	if ( Cmd(p, "Barrier51"         ) )  return OBJECT_BARRIER51;
	if ( Cmd(p, "Barrier52"         ) )  return OBJECT_BARRIER52;
	if ( Cmd(p, "Barrier53"         ) )  return OBJECT_BARRIER53;
	if ( Cmd(p, "Barrier54"         ) )  return OBJECT_BARRIER54;
	if ( Cmd(p, "Barrier55"         ) )  return OBJECT_BARRIER55;
	if ( Cmd(p, "Barrier56"         ) )  return OBJECT_BARRIER56;
	if ( Cmd(p, "Barrier57"         ) )  return OBJECT_BARRIER57;
	if ( Cmd(p, "Barrier58"         ) )  return OBJECT_BARRIER58;
	if ( Cmd(p, "Barrier59"         ) )  return OBJECT_BARRIER59;
	if ( Cmd(p, "Barrier60"         ) )  return OBJECT_BARRIER60;
	if ( Cmd(p, "Barrier61"         ) )  return OBJECT_BARRIER61;
	if ( Cmd(p, "Barrier62"         ) )  return OBJECT_BARRIER62;
	if ( Cmd(p, "Barrier63"         ) )  return OBJECT_BARRIER63;
	if ( Cmd(p, "Barrier64"         ) )  return OBJECT_BARRIER64;
	if ( Cmd(p, "Barrier65"         ) )  return OBJECT_BARRIER65;
	if ( Cmd(p, "Barrier66"         ) )  return OBJECT_BARRIER66;
	if ( Cmd(p, "Barrier67"         ) )  return OBJECT_BARRIER67;
	if ( Cmd(p, "Barrier68"         ) )  return OBJECT_BARRIER68;
	if ( Cmd(p, "Barrier69"         ) )  return OBJECT_BARRIER69;
	if ( Cmd(p, "Barrier70"         ) )  return OBJECT_BARRIER70;
	if ( Cmd(p, "Barrier71"         ) )  return OBJECT_BARRIER71;
	if ( Cmd(p, "Barrier72"         ) )  return OBJECT_BARRIER72;
	if ( Cmd(p, "Barrier73"         ) )  return OBJECT_BARRIER73;
	if ( Cmd(p, "Barrier74"         ) )  return OBJECT_BARRIER74;
	if ( Cmd(p, "Barrier75"         ) )  return OBJECT_BARRIER75;
	if ( Cmd(p, "Barrier76"         ) )  return OBJECT_BARRIER76;
	if ( Cmd(p, "Barrier77"         ) )  return OBJECT_BARRIER77;
	if ( Cmd(p, "Barrier78"         ) )  return OBJECT_BARRIER78;
	if ( Cmd(p, "Barrier79"         ) )  return OBJECT_BARRIER79;
	if ( Cmd(p, "Barrier0"          ) )  return OBJECT_BARRIER0;
	if ( Cmd(p, "Barrier1"          ) )  return OBJECT_BARRIER1;
	if ( Cmd(p, "Barrier2"          ) )  return OBJECT_BARRIER2;
	if ( Cmd(p, "Barrier3"          ) )  return OBJECT_BARRIER3;
	if ( Cmd(p, "Barrier4"          ) )  return OBJECT_BARRIER4;
	if ( Cmd(p, "Barrier5"          ) )  return OBJECT_BARRIER5;
	if ( Cmd(p, "Barrier6"          ) )  return OBJECT_BARRIER6;
	if ( Cmd(p, "Barrier7"          ) )  return OBJECT_BARRIER7;
	if ( Cmd(p, "Barrier8"          ) )  return OBJECT_BARRIER8;
	if ( Cmd(p, "Barrier9"          ) )  return OBJECT_BARRIER9;
	if ( Cmd(p, "Scrap0"            ) )  return OBJECT_SCRAP0;
	if ( Cmd(p, "Scrap1"            ) )  return OBJECT_SCRAP1;
	if ( Cmd(p, "Scrap2"            ) )  return OBJECT_SCRAP2;
	if ( Cmd(p, "Scrap3"            ) )  return OBJECT_SCRAP3;
	if ( Cmd(p, "Scrap4"            ) )  return OBJECT_SCRAP4;
	if ( Cmd(p, "Scrap5"            ) )  return OBJECT_SCRAP5;
	if ( Cmd(p, "Scrap6"            ) )  return OBJECT_SCRAP6;
	if ( Cmd(p, "Scrap7"            ) )  return OBJECT_SCRAP7;
	if ( Cmd(p, "Scrap8"            ) )  return OBJECT_SCRAP8;
	if ( Cmd(p, "Scrap9"            ) )  return OBJECT_SCRAP9;
	if ( Cmd(p, "Box10"             ) )  return OBJECT_BOX10;
	if ( Cmd(p, "Box1"              ) )  return OBJECT_BOX1;
	if ( Cmd(p, "Box2"              ) )  return OBJECT_BOX2;
	if ( Cmd(p, "Box3"              ) )  return OBJECT_BOX3;
	if ( Cmd(p, "Box4"              ) )  return OBJECT_BOX4;
	if ( Cmd(p, "Box5"              ) )  return OBJECT_BOX5;
	if ( Cmd(p, "Box6"              ) )  return OBJECT_BOX6;
	if ( Cmd(p, "Box7"              ) )  return OBJECT_BOX7;
	if ( Cmd(p, "Box8"              ) )  return OBJECT_BOX8;
	if ( Cmd(p, "Box9"              ) )  return OBJECT_BOX9;
	if ( Cmd(p, "Key1"              ) )  return OBJECT_KEY1;
	if ( Cmd(p, "Key2"              ) )  return OBJECT_KEY2;
	if ( Cmd(p, "Key3"              ) )  return OBJECT_KEY3;
	if ( Cmd(p, "Key4"              ) )  return OBJECT_KEY4;
	if ( Cmd(p, "Key5"              ) )  return OBJECT_KEY5;
	if ( Cmd(p, "Ground10"          ) )  return OBJECT_GROUND10;
	if ( Cmd(p, "Ground11"          ) )  return OBJECT_GROUND11;
	if ( Cmd(p, "Ground12"          ) )  return OBJECT_GROUND12;
	if ( Cmd(p, "Ground13"          ) )  return OBJECT_GROUND13;
	if ( Cmd(p, "Ground14"          ) )  return OBJECT_GROUND14;
	if ( Cmd(p, "Ground15"          ) )  return OBJECT_GROUND15;
	if ( Cmd(p, "Ground16"          ) )  return OBJECT_GROUND16;
	if ( Cmd(p, "Ground17"          ) )  return OBJECT_GROUND17;
	if ( Cmd(p, "Ground18"          ) )  return OBJECT_GROUND18;
	if ( Cmd(p, "Ground19"          ) )  return OBJECT_GROUND19;
	if ( Cmd(p, "Ground0"           ) )  return OBJECT_GROUND0;
	if ( Cmd(p, "Ground1"           ) )  return OBJECT_GROUND1;
	if ( Cmd(p, "Ground2"           ) )  return OBJECT_GROUND2;
	if ( Cmd(p, "Ground3"           ) )  return OBJECT_GROUND3;
	if ( Cmd(p, "Ground4"           ) )  return OBJECT_GROUND4;
	if ( Cmd(p, "Ground5"           ) )  return OBJECT_GROUND5;
	if ( Cmd(p, "Ground6"           ) )  return OBJECT_GROUND6;
	if ( Cmd(p, "Ground7"           ) )  return OBJECT_GROUND7;
	if ( Cmd(p, "Ground8"           ) )  return OBJECT_GROUND8;
	if ( Cmd(p, "Ground9"           ) )  return OBJECT_GROUND9;
	if ( Cmd(p, "Stone10"           ) )  return OBJECT_STONE10;
	if ( Cmd(p, "Stone1"            ) )  return OBJECT_STONE1;
	if ( Cmd(p, "Stone2"            ) )  return OBJECT_STONE2;
	if ( Cmd(p, "Stone3"            ) )  return OBJECT_STONE3;
	if ( Cmd(p, "Stone4"            ) )  return OBJECT_STONE4;
	if ( Cmd(p, "Stone5"            ) )  return OBJECT_STONE5;
	if ( Cmd(p, "Stone6"            ) )  return OBJECT_STONE6;
	if ( Cmd(p, "Stone7"            ) )  return OBJECT_STONE7;
	if ( Cmd(p, "Stone8"            ) )  return OBJECT_STONE8;
	if ( Cmd(p, "Stone9"            ) )  return OBJECT_STONE9;
	if ( Cmd(p, "Quartz0"           ) )  return OBJECT_QUARTZ0;
	if ( Cmd(p, "Quartz1"           ) )  return OBJECT_QUARTZ1;
	if ( Cmd(p, "Quartz2"           ) )  return OBJECT_QUARTZ2;
	if ( Cmd(p, "Quartz3"           ) )  return OBJECT_QUARTZ3;
	if ( Cmd(p, "Quartz4"           ) )  return OBJECT_QUARTZ4;
	if ( Cmd(p, "Quartz5"           ) )  return OBJECT_QUARTZ5;
	if ( Cmd(p, "Quartz6"           ) )  return OBJECT_QUARTZ6;
	if ( Cmd(p, "Quartz7"           ) )  return OBJECT_QUARTZ7;
	if ( Cmd(p, "Quartz8"           ) )  return OBJECT_QUARTZ8;
	if ( Cmd(p, "Quartz9"           ) )  return OBJECT_QUARTZ9;
	if ( Cmd(p, "MegaStalk0"        ) )  return OBJECT_ROOT0;
	if ( Cmd(p, "MegaStalk1"        ) )  return OBJECT_ROOT1;
	if ( Cmd(p, "MegaStalk2"        ) )  return OBJECT_ROOT2;
	if ( Cmd(p, "MegaStalk3"        ) )  return OBJECT_ROOT3;
	if ( Cmd(p, "MegaStalk4"        ) )  return OBJECT_ROOT4;
	if ( Cmd(p, "MegaStalk5"        ) )  return OBJECT_ROOT5;
	if ( Cmd(p, "MegaStalk6"        ) )  return OBJECT_ROOT6;
	if ( Cmd(p, "MegaStalk7"        ) )  return OBJECT_ROOT7;
	if ( Cmd(p, "MegaStalk8"        ) )  return OBJECT_ROOT8;
	if ( Cmd(p, "MegaStalk9"        ) )  return OBJECT_ROOT9;

	return def;
}

// Retourne le type de l'eau.

WaterType GetTypeWater(char *line, int rank, WaterType def)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return def;

	if ( Cmd(p, "NULL" ) )  return WATER_NULL;
	if ( Cmd(p, "TT"   ) )  return WATER_TT;
	if ( Cmd(p, "TO"   ) )  return WATER_TO;
	if ( Cmd(p, "CT"   ) )  return WATER_CT;
	if ( Cmd(p, "CO"   ) )  return WATER_CO;

	return def;
}

// Retourne le type de terrain.

D3DTypeObj GetTypeTerrain(char *line, int rank, D3DTypeObj def)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return def;

	if ( Cmd(p, "Terrain" ) )  return TYPETERRAIN;
	if ( Cmd(p, "Object"  ) )  return TYPEOBJECT;
	if ( Cmd(p, "Quartz"  ) )  return TYPEQUARTZ;
	if ( Cmd(p, "Metal"   ) )  return TYPEMETAL;
	if ( Cmd(p, "Blupi"   ) )  return TYPEBLUPI;

	return def;
}

// Retourne le numéro d'un bâtiment.

int GetBuild(char *line, int rank)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return 0;

	if ( Cmd(p, "BotFactory"    ) )  return BUILD_FACTORY;
	if ( Cmd(p, "Derrick"       ) )  return BUILD_DERRICK;
	if ( Cmd(p, "Converter"     ) )  return BUILD_CONVERT;
	if ( Cmd(p, "RadarStation"  ) )  return BUILD_RADAR;
	if ( Cmd(p, "PowerPlant"    ) )  return BUILD_ENERGY;
	if ( Cmd(p, "NuclearPlant"  ) )  return BUILD_NUCLEAR;
	if ( Cmd(p, "PowerStation"  ) )  return BUILD_STATION;
	if ( Cmd(p, "RepairCenter"  ) )  return BUILD_REPAIR;
	if ( Cmd(p, "DefenseTower"  ) )  return BUILD_TOWER;
	if ( Cmd(p, "ResearchCenter") )  return BUILD_RESEARCH;
	if ( Cmd(p, "AutoLab"       ) )  return BUILD_LABO;
	if ( Cmd(p, "PowerCaptor"   ) )  return BUILD_PARA;
	if ( Cmd(p, "ExchangePost"  ) )  return BUILD_INFO;
	if ( Cmd(p, "FlatGround"    ) )  return BUILD_GFLAT;
	if ( Cmd(p, "Flag"          ) )  return BUILD_FLAG;

	return 0;
}

// Retourne le numéro d'une recherche.

int GetResearch(char *line, int rank)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return 0;

	if ( Cmd(p, "TRACKER" ) )  return RESEARCH_TANK;
	if ( Cmd(p, "WINGER"  ) )  return RESEARCH_FLY;
	if ( Cmd(p, "THUMPER" ) )  return RESEARCH_THUMP;
	if ( Cmd(p, "SHOOTER" ) )  return RESEARCH_CANON;
	if ( Cmd(p, "TOWER"   ) )  return RESEARCH_TOWER;
	if ( Cmd(p, "PHAZER"  ) )  return RESEARCH_PHAZER;
	if ( Cmd(p, "SHIELDER") )  return RESEARCH_SHIELD;
	if ( Cmd(p, "ATOMIC"  ) )  return RESEARCH_ATOMIC;
	if ( Cmd(p, "iPAW"    ) )  return RESEARCH_iPAW;
	if ( Cmd(p, "iGUN"    ) )  return RESEARCH_iGUN;
	if ( Cmd(p, "RECYCLER") )  return RESEARCH_RECYCLER;
	if ( Cmd(p, "SUBBER"  ) )  return RESEARCH_SUBM;
	if ( Cmd(p, "SNIFFER" ) )  return RESEARCH_SNIFFER;

	return 0;
}

// Retourne le type d'un effet pyrotechnique.

PyroType GetPyro(char *line, int rank)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return PT_NULL;

	if ( Cmd(p, "FRAGt"    ) )  return PT_FRAGT;
	if ( Cmd(p, "FRAGo"    ) )  return PT_FRAGO;
	if ( Cmd(p, "EXPLOt"   ) )  return PT_EXPLOT;
	if ( Cmd(p, "EXPLOo"   ) )  return PT_EXPLOO;
	if ( Cmd(p, "EXPLOp"   ) )  return PT_EXPLOP;
	if ( Cmd(p, "SHOTt"    ) )  return PT_SHOTT;
	if ( Cmd(p, "BURNt"    ) )  return PT_BURNT;
	if ( Cmd(p, "BURNo"    ) )  return PT_BURNO;

	return PT_NULL;
}

// Retourne le type d'une caméra.

CameraType GetCamera(char *line, int rank)
{
	char*	p;

	p = SearchArg(line, rank);
	if ( *p == 0 )  return CAMERA_NULL;

	if ( Cmd(p, "BACK"    ) )  return CAMERA_BACK;
	if ( Cmd(p, "ONBOARD" ) )  return CAMERA_ONBOARD;
	if ( Cmd(p, "FIX"     ) )  return CAMERA_FIX;
	if ( Cmd(p, "FREE"    ) )  return CAMERA_FREE;

	return CAMERA_NULL;
}

// Retourne le nom d'une camera.

char* GetCamera(CameraType type)
{
	if ( type == CAMERA_ONBOARD )  return "ONBOARD";
	if ( type == CAMERA_FIX     )  return "FIX";
	return "BACK";
}

// Retourne un nombre entier.

int OpInt(char *line, char *op, int def)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return def;
	return GetInt(line, 0, def);
}

// Retourne un nombre réel.

float OpFloat(char *line, char *op, float def)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return def;
	return GetFloat(line, 0, def);
}

// Retourne une chaîne de caractères.

void OpString(char *line, char *op, char *buffer)
{
	line = SearchOp(line, op);
	if ( *line == 0 )
	{
		buffer[0] = 0;
	}
	else
	{
		GetString(line, 0, buffer);
	}
}

// Retourne le type d'un objet.

ObjectType OpTypeObject(char *line, char *op, ObjectType def)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return def;
	return GetTypeObject(line, 0, def);
}

// Retourne le type d'un objet.

WaterType OpTypeWater(char *line, char *op, WaterType def)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return def;
	return GetTypeWater(line, 0, def);
}

// Retourne le type d'un objet.

D3DTypeObj OpTypeTerrain(char *line, char *op, D3DTypeObj def)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return def;
	return GetTypeTerrain(line, 0, def);
}

// Retourne le numéro d'une recherche.

int OpResearch(char *line, char *op)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return 0;
	return GetResearch(line, 0);
}

// Retourne le type d'un effet pyrotechnique.

PyroType OpPyro(char *line, char *op)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return PT_NULL;
	return GetPyro(line, 0);
}

// Retourne le type d'une caméra.

CameraType OpCamera(char *line, char *op)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return CAMERA_NULL;
	return GetCamera(line, 0);
}

// Retourne le numéro d'un bâtiment.

int OpBuild(char *line, char *op)
{
	line = SearchOp(line, op);
	if ( *line == 0 )  return 0;
	return GetBuild(line, 0);
}

// Retourne une position dans le plan XZ (vue d'en haut).

D3DVECTOR OpPos(char *line, char *op)
{
	D3DVECTOR	pos;

	line = SearchOp(line, op);
	if ( *line == 0 )
	{
		pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
		return pos;
	}
	pos.x = GetFloat(line, 0, 0.0f);
	pos.y = 0.0f;
	pos.z = GetFloat(line, 1, 0.0f);
	return pos;
}

// Retourne une direction.

D3DVECTOR OpDir(char *line, char *op)
{
	D3DVECTOR	dir;

	line = SearchOp(line, op);
	if ( *line == 0 )
	{
		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		return dir;
	}
	dir.x = GetFloat(line, 0, 0.0f);
	dir.y = GetFloat(line, 1, 0.0f);
	dir.z = GetFloat(line, 2, 0.0f);
	return dir;
}

// Lit une couleur (0..255).

D3DCOLOR OpColor(char *line, char *op, D3DCOLOR def)
{
	D3DCOLOR	color;

	line = SearchOp(line, op);
	if ( *line == 0 )  return def;

	color = 0;
	color |= (GetInt(line, 0, 0)&0xff)<<16;  // r
	color |= (GetInt(line, 1, 0)&0xff)<<8;   // g
	color |= (GetInt(line, 2, 0)&0xff)<<0;   // b
	color |= (GetInt(line, 3, 0)&0xff)<<24;  // a
	return color;
}

// Lit une couleur (-1..1).

D3DCOLORVALUE OpColorValue(char *line, char *op, D3DCOLORVALUE def)
{
	D3DCOLORVALUE	color;

	line = SearchOp(line, op);
	if ( *line == 0 )  return def;

	color.r = GetFloat(line, 0, 0.0f);
	color.g = GetFloat(line, 1, 0.0f);
	color.b = GetFloat(line, 2, 0.0f);
	color.a = GetFloat(line, 3, 0.0f);
	return color;
}


