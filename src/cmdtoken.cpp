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
	if ( Cmd(p, "TargetBot"         ) )  return OBJECT_MOBILEtg;
	if ( Cmd(p, "FireballBot"       ) )  return OBJECT_MOBILEfb;
	if ( Cmd(p, "OrgaballBot"       ) )  return OBJECT_MOBILEob;
	if ( Cmd(p, "TraxFix"           ) )  return OBJECT_TRAXf;
	if ( Cmd(p, "Trax"              ) )  return OBJECT_TRAX;
	if ( Cmd(p, "UFO"               ) )  return OBJECT_UFO;
	if ( Cmd(p, "Carrot"            ) )  return OBJECT_CARROT;
	if ( Cmd(p, "Starter"           ) )  return OBJECT_STARTER;
	if ( Cmd(p, "Walker"            ) )  return OBJECT_WALKER;
	if ( Cmd(p, "Crazy"             ) )  return OBJECT_CRAZY;
	if ( Cmd(p, "Guide"             ) )  return OBJECT_GUIDE;
	if ( Cmd(p, "Evil1"             ) )  return OBJECT_EVIL1;
	if ( Cmd(p, "Evil2"             ) )  return OBJECT_EVIL2;
	if ( Cmd(p, "Evil3"             ) )  return OBJECT_EVIL3;
	if ( Cmd(p, "Evil4"             ) )  return OBJECT_EVIL4;
	if ( Cmd(p, "Evil5"             ) )  return OBJECT_EVIL5;
	if ( Cmd(p, "WayPoint"          ) )  return OBJECT_WAYPOINT;
	if ( Cmd(p, "Traject"           ) )  return OBJECT_TRAJECT;
	if ( Cmd(p, "Target"            ) )  return OBJECT_TARGET;
	if ( Cmd(p, "BarrelAtomic"      ) )  return OBJECT_BARRELa;
	if ( Cmd(p, "Barrel"            ) )  return OBJECT_BARREL;
	if ( Cmd(p, "NuclearCell"       ) )  return OBJECT_ATOMIC;
	if ( Cmd(p, "TitaniumOre"       ) )  return OBJECT_STONE;
	if ( Cmd(p, "UraniumOre"        ) )  return OBJECT_URANIUM;
	if ( Cmd(p, "Titanium"          ) )  return OBJECT_METAL;
	if ( Cmd(p, "OrgaMatter"        ) )  return OBJECT_BULLET;
	if ( Cmd(p, "BlackBox"          ) )  return OBJECT_BBOX;
	if ( Cmd(p, "KeyA"              ) )  return OBJECT_KEYa;
	if ( Cmd(p, "KeyB"              ) )  return OBJECT_KEYb;
	if ( Cmd(p, "KeyC"              ) )  return OBJECT_KEYc;
	if ( Cmd(p, "KeyD"              ) )  return OBJECT_KEYd;
	if ( Cmd(p, "Bot1"              ) )  return OBJECT_BOT1;
	if ( Cmd(p, "Bot2"              ) )  return OBJECT_BOT2;
	if ( Cmd(p, "Bot3"              ) )  return OBJECT_BOT3;
	if ( Cmd(p, "Bot4"              ) )  return OBJECT_BOT4;
	if ( Cmd(p, "Bot5"              ) )  return OBJECT_BOT5;
	if ( Cmd(p, "TNT"               ) )  return OBJECT_TNT;
	if ( Cmd(p, "Mine"              ) )  return OBJECT_MINE;
	if ( Cmd(p, "Pole"              ) )  return OBJECT_POLE;
	if ( Cmd(p, "Bomb"              ) )  return OBJECT_BOMB;
	if ( Cmd(p, "Cone"              ) )  return OBJECT_CONE;
	if ( Cmd(p, "Pipes"             ) )  return OBJECT_PIPES;
	if ( Cmd(p, "Firework"          ) )  return OBJECT_WINFIRE;
	if ( Cmd(p, "Fire"              ) )  return OBJECT_FIRE;
	if ( Cmd(p, "Helico"            ) )  return OBJECT_HELICO;
	if ( Cmd(p, "Compass"           ) )  return OBJECT_COMPASS;
	if ( Cmd(p, "Blitzer"           ) )  return OBJECT_BLITZER;
	if ( Cmd(p, "Hook"              ) )  return OBJECT_HOOK;
	if ( Cmd(p, "Aquarium"          ) )  return OBJECT_AQUA;
	if ( Cmd(p, "Bag"               ) )  return OBJECT_BAG;
	if ( Cmd(p, "Cross1"            ) )  return OBJECT_CROSS1;
	if ( Cmd(p, "Mark"              ) )  return OBJECT_MARK;
	if ( Cmd(p, "Crown"             ) )  return OBJECT_CROWN;
	if ( Cmd(p, "Toys1"             ) )  return OBJECT_TOYS1;
	if ( Cmd(p, "Toys2"             ) )  return OBJECT_TOYS2;
	if ( Cmd(p, "Toys3"             ) )  return OBJECT_TOYS3;
	if ( Cmd(p, "Toys4"             ) )  return OBJECT_TOYS4;
	if ( Cmd(p, "Toys5"             ) )  return OBJECT_TOYS5;
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
	if ( Cmd(p, "Home1"             ) )  return OBJECT_HOME1;
	if ( Cmd(p, "Home2"             ) )  return OBJECT_HOME2;
	if ( Cmd(p, "Home3"             ) )  return OBJECT_HOME3;
	if ( Cmd(p, "Home4"             ) )  return OBJECT_HOME4;
	if ( Cmd(p, "Home5"             ) )  return OBJECT_HOME5;
	if ( Cmd(p, "Factory10"         ) )  return OBJECT_FACTORY10;
	if ( Cmd(p, "Factory11"         ) )  return OBJECT_FACTORY11;
	if ( Cmd(p, "Factory12"         ) )  return OBJECT_FACTORY12;
	if ( Cmd(p, "Factory13"         ) )  return OBJECT_FACTORY13;
	if ( Cmd(p, "Factory14"         ) )  return OBJECT_FACTORY14;
	if ( Cmd(p, "Factory15"         ) )  return OBJECT_FACTORY15;
	if ( Cmd(p, "Factory16"         ) )  return OBJECT_FACTORY16;
	if ( Cmd(p, "Factory17"         ) )  return OBJECT_FACTORY17;
	if ( Cmd(p, "Factory18"         ) )  return OBJECT_FACTORY18;
	if ( Cmd(p, "Factory19"         ) )  return OBJECT_FACTORY19;
	if ( Cmd(p, "Factory20"         ) )  return OBJECT_FACTORY20;
	if ( Cmd(p, "Factory1"          ) )  return OBJECT_FACTORY1;
	if ( Cmd(p, "Factory2"          ) )  return OBJECT_FACTORY2;
	if ( Cmd(p, "Factory3"          ) )  return OBJECT_FACTORY3;
	if ( Cmd(p, "Factory4"          ) )  return OBJECT_FACTORY4;
	if ( Cmd(p, "Factory5"          ) )  return OBJECT_FACTORY5;
	if ( Cmd(p, "Factory6"          ) )  return OBJECT_FACTORY6;
	if ( Cmd(p, "Factory7"          ) )  return OBJECT_FACTORY7;
	if ( Cmd(p, "Factory8"          ) )  return OBJECT_FACTORY8;
	if ( Cmd(p, "Factory9"          ) )  return OBJECT_FACTORY9;
	if ( Cmd(p, "Gravel"            ) )  return OBJECT_GRAVEL;
	if ( Cmd(p, "Tub"               ) )  return OBJECT_TUB;
	if ( Cmd(p, "Alien10"           ) )  return OBJECT_ALIEN10;
	if ( Cmd(p, "Alien1"            ) )  return OBJECT_ALIEN1;
	if ( Cmd(p, "Alien2"            ) )  return OBJECT_ALIEN2;
	if ( Cmd(p, "Alien3"            ) )  return OBJECT_ALIEN3;
	if ( Cmd(p, "Alien4"            ) )  return OBJECT_ALIEN4;
	if ( Cmd(p, "Alien5"            ) )  return OBJECT_ALIEN5;
	if ( Cmd(p, "Alien6"            ) )  return OBJECT_ALIEN6;
	if ( Cmd(p, "Alien7"            ) )  return OBJECT_ALIEN7;
	if ( Cmd(p, "Alien8"            ) )  return OBJECT_ALIEN8;
	if ( Cmd(p, "Alien9"            ) )  return OBJECT_ALIEN9;
	if ( Cmd(p, "Inca10"            ) )  return OBJECT_INCA10;
	if ( Cmd(p, "Inca1"             ) )  return OBJECT_INCA1;
	if ( Cmd(p, "Inca2"             ) )  return OBJECT_INCA2;
	if ( Cmd(p, "Inca3"             ) )  return OBJECT_INCA3;
	if ( Cmd(p, "Inca4"             ) )  return OBJECT_INCA4;
	if ( Cmd(p, "Inca5"             ) )  return OBJECT_INCA5;
	if ( Cmd(p, "Inca6"             ) )  return OBJECT_INCA6;
	if ( Cmd(p, "Inca7"             ) )  return OBJECT_INCA7;
	if ( Cmd(p, "Inca8"             ) )  return OBJECT_INCA8;
	if ( Cmd(p, "Inca9"             ) )  return OBJECT_INCA9;
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
	if ( Cmd(p, "Car"               ) )  return OBJECT_CAR;
	if ( Cmd(p, "DefenseTower"      ) )  return OBJECT_TOWER;
	if ( Cmd(p, "NuclearPlant"      ) )  return OBJECT_NUCLEAR;
	if ( Cmd(p, "PowerCaptor"       ) )  return OBJECT_PARA;
	if ( Cmd(p, "Computer"          ) )  return OBJECT_COMPUTER;
	if ( Cmd(p, "RepairCenter"      ) )  return OBJECT_REPAIR;
	if ( Cmd(p, "Sweet"             ) )  return OBJECT_SWEET;
	if ( Cmd(p, "Door1"             ) )  return OBJECT_DOOR1;
	if ( Cmd(p, "Door2"             ) )  return OBJECT_DOOR2;
	if ( Cmd(p, "Door3"             ) )  return OBJECT_DOOR3;
	if ( Cmd(p, "Door4"             ) )  return OBJECT_DOOR4;
	if ( Cmd(p, "Door5"             ) )  return OBJECT_DOOR5;
	if ( Cmd(p, "Dock"              ) )  return OBJECT_DOCK;
	if ( Cmd(p, "Remote"            ) )  return OBJECT_REMOTE;
	if ( Cmd(p, "Stand"             ) )  return OBJECT_STAND;
	if ( Cmd(p, "Generator"         ) )  return OBJECT_GENERATOR;
	if ( Cmd(p, "StartArea"         ) )  return OBJECT_START;
	if ( Cmd(p, "GoalArea"          ) )  return OBJECT_END;
	if ( Cmd(p, "Support"           ) )  return OBJECT_SUPPORT;
	if ( Cmd(p, "RoadSign10"        ) )  return OBJECT_ROADSIGN10;
	if ( Cmd(p, "RoadSign11"        ) )  return OBJECT_ROADSIGN11;
	if ( Cmd(p, "RoadSign12"        ) )  return OBJECT_ROADSIGN12;
	if ( Cmd(p, "RoadSign13"        ) )  return OBJECT_ROADSIGN13;
	if ( Cmd(p, "RoadSign14"        ) )  return OBJECT_ROADSIGN14;
	if ( Cmd(p, "RoadSign15"        ) )  return OBJECT_ROADSIGN15;
	if ( Cmd(p, "RoadSign16"        ) )  return OBJECT_ROADSIGN16;
	if ( Cmd(p, "RoadSign17"        ) )  return OBJECT_ROADSIGN17;
	if ( Cmd(p, "RoadSign18"        ) )  return OBJECT_ROADSIGN18;
	if ( Cmd(p, "RoadSign19"        ) )  return OBJECT_ROADSIGN19;
	if ( Cmd(p, "RoadSign20"        ) )  return OBJECT_ROADSIGN20;
	if ( Cmd(p, "RoadSign21"        ) )  return OBJECT_ROADSIGN21;
	if ( Cmd(p, "RoadSign22"        ) )  return OBJECT_ROADSIGN22;
	if ( Cmd(p, "RoadSign23"        ) )  return OBJECT_ROADSIGN23;
	if ( Cmd(p, "RoadSign24"        ) )  return OBJECT_ROADSIGN24;
	if ( Cmd(p, "RoadSign25"        ) )  return OBJECT_ROADSIGN25;
	if ( Cmd(p, "RoadSign26"        ) )  return OBJECT_ROADSIGN26;
	if ( Cmd(p, "RoadSign27"        ) )  return OBJECT_ROADSIGN27;
	if ( Cmd(p, "RoadSign28"        ) )  return OBJECT_ROADSIGN28;
	if ( Cmd(p, "RoadSign29"        ) )  return OBJECT_ROADSIGN29;
	if ( Cmd(p, "RoadSign30"        ) )  return OBJECT_ROADSIGN30;
	if ( Cmd(p, "RoadSign1"         ) )  return OBJECT_ROADSIGN1;
	if ( Cmd(p, "RoadSign2"         ) )  return OBJECT_ROADSIGN2;
	if ( Cmd(p, "RoadSign3"         ) )  return OBJECT_ROADSIGN3;
	if ( Cmd(p, "RoadSign4"         ) )  return OBJECT_ROADSIGN4;
	if ( Cmd(p, "RoadSign5"         ) )  return OBJECT_ROADSIGN5;
	if ( Cmd(p, "RoadSign6"         ) )  return OBJECT_ROADSIGN6;
	if ( Cmd(p, "RoadSign7"         ) )  return OBJECT_ROADSIGN7;
	if ( Cmd(p, "RoadSign8"         ) )  return OBJECT_ROADSIGN8;
	if ( Cmd(p, "RoadSign9"         ) )  return OBJECT_ROADSIGN9;
	if ( Cmd(p, "Pub11"             ) )  return OBJECT_PUB11;
	if ( Cmd(p, "Pub12"             ) )  return OBJECT_PUB12;
	if ( Cmd(p, "Pub13"             ) )  return OBJECT_PUB13;
	if ( Cmd(p, "Pub14"             ) )  return OBJECT_PUB14;
	if ( Cmd(p, "Pub21"             ) )  return OBJECT_PUB21;
	if ( Cmd(p, "Pub22"             ) )  return OBJECT_PUB22;
	if ( Cmd(p, "Pub23"             ) )  return OBJECT_PUB23;
	if ( Cmd(p, "Pub24"             ) )  return OBJECT_PUB24;
	if ( Cmd(p, "Pub31"             ) )  return OBJECT_PUB31;
	if ( Cmd(p, "Pub32"             ) )  return OBJECT_PUB32;
	if ( Cmd(p, "Pub33"             ) )  return OBJECT_PUB33;
	if ( Cmd(p, "Pub34"             ) )  return OBJECT_PUB34;
	if ( Cmd(p, "Pub41"             ) )  return OBJECT_PUB41;
	if ( Cmd(p, "Pub42"             ) )  return OBJECT_PUB42;
	if ( Cmd(p, "Pub43"             ) )  return OBJECT_PUB43;
	if ( Cmd(p, "Pub44"             ) )  return OBJECT_PUB44;
	if ( Cmd(p, "Pub51"             ) )  return OBJECT_PUB51;
	if ( Cmd(p, "Pub52"             ) )  return OBJECT_PUB52;
	if ( Cmd(p, "Pub53"             ) )  return OBJECT_PUB53;
	if ( Cmd(p, "Pub54"             ) )  return OBJECT_PUB54;
	if ( Cmd(p, "Pub61"             ) )  return OBJECT_PUB61;
	if ( Cmd(p, "Pub62"             ) )  return OBJECT_PUB62;
	if ( Cmd(p, "Pub63"             ) )  return OBJECT_PUB63;
	if ( Cmd(p, "Pub64"             ) )  return OBJECT_PUB64;
	if ( Cmd(p, "Pub71"             ) )  return OBJECT_PUB71;
	if ( Cmd(p, "Pub72"             ) )  return OBJECT_PUB72;
	if ( Cmd(p, "Pub73"             ) )  return OBJECT_PUB73;
	if ( Cmd(p, "Pub74"             ) )  return OBJECT_PUB74;
	if ( Cmd(p, "Pub81"             ) )  return OBJECT_PUB81;
	if ( Cmd(p, "Pub82"             ) )  return OBJECT_PUB82;
	if ( Cmd(p, "Pub83"             ) )  return OBJECT_PUB83;
	if ( Cmd(p, "Pub84"             ) )  return OBJECT_PUB84;
	if ( Cmd(p, "Pub91"             ) )  return OBJECT_PUB91;
	if ( Cmd(p, "Pub92"             ) )  return OBJECT_PUB92;
	if ( Cmd(p, "Pub93"             ) )  return OBJECT_PUB93;
	if ( Cmd(p, "Pub94"             ) )  return OBJECT_PUB94;
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
	if ( Cmd(p, "Piece0"            ) )  return OBJECT_PIECE0;
	if ( Cmd(p, "Piece1"            ) )  return OBJECT_PIECE1;
	if ( Cmd(p, "Piece2"            ) )  return OBJECT_PIECE2;
	if ( Cmd(p, "Piece3"            ) )  return OBJECT_PIECE3;
	if ( Cmd(p, "Piece4"            ) )  return OBJECT_PIECE4;
	if ( Cmd(p, "Piece5"            ) )  return OBJECT_PIECE5;
	if ( Cmd(p, "Piece6"            ) )  return OBJECT_PIECE6;
	if ( Cmd(p, "Piece7"            ) )  return OBJECT_PIECE7;
	if ( Cmd(p, "Piece8"            ) )  return OBJECT_PIECE8;
	if ( Cmd(p, "Piece9"            ) )  return OBJECT_PIECE9;
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
	if ( Cmd(p, "Me"                ) )  return OBJECT_HUMAN;
	if ( Cmd(p, "Tech"              ) )  return OBJECT_TECH;

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
	if ( Cmd(p, "Object"  ) )  return TYPEFIX;
	if ( Cmd(p, "Quartz"  ) )  return TYPEQUARTZ;

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
	if ( Cmd(p, "FRAGw"    ) )  return PT_FRAGW;
	if ( Cmd(p, "EXPLOt"   ) )  return PT_EXPLOT;
	if ( Cmd(p, "EXPLOo"   ) )  return PT_EXPLOO;
	if ( Cmd(p, "EXPLOw"   ) )  return PT_EXPLOW;
	if ( Cmd(p, "EXPLOp"   ) )  return PT_EXPLOP;
	if ( Cmd(p, "SHOTt"    ) )  return PT_SHOTT;
	if ( Cmd(p, "SHOTh"    ) )  return PT_SHOTH;
	if ( Cmd(p, "SHOTm"    ) )  return PT_SHOTM;
	if ( Cmd(p, "SHOTw"    ) )  return PT_SHOTW;
	if ( Cmd(p, "EGG"      ) )  return PT_EGG;
	if ( Cmd(p, "BURNt"    ) )  return PT_BURNT;
	if ( Cmd(p, "BURNo"    ) )  return PT_BURNO;
	if ( Cmd(p, "SPIDER"   ) )  return PT_SPIDER;
	if ( Cmd(p, "WIN"      ) )  return PT_WIN;
	if ( Cmd(p, "LOST"     ) )  return PT_LOST;
	if ( Cmd(p, "EJECT"    ) )  return PT_EJECT;
	if ( Cmd(p, "ACROBATIC") )  return PT_ACROBATIC;
	if ( Cmd(p, "SABOTAGE" ) )  return PT_SABOTAGE;

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


