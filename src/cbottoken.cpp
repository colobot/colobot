// cbottoken.cpp

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
#include "object.h"
#include "cbottoken.h"




// Cherche le nom d'un objet.

char* RetObjectName(ObjectType type)
{
	if ( type == OBJECT_NULL        )  return "Anything";
	if ( type == OBJECT_DOOR1       )  return "Door1";
	if ( type == OBJECT_DOOR2       )  return "Door2";
	if ( type == OBJECT_DOOR3       )  return "Door3";
	if ( type == OBJECT_DOOR4       )  return "Door4";
	if ( type == OBJECT_DOOR5       )  return "Door5";
	if ( type == OBJECT_DOCK        )  return "Dock";
	if ( type == OBJECT_CATAPULT    )  return "Catapult";
	if ( type == OBJECT_START       )  return "StartArea";
	if ( type == OBJECT_END         )  return "GoalArea";
	if ( type == OBJECT_STONE       )  return "TitaniumOre";
	if ( type == OBJECT_URANIUM     )  return "UraniumOre";
	if ( type == OBJECT_METAL       )  return "Titanium";
	if ( type == OBJECT_BARRELa     )  return "BarrelAtomic";
	if ( type == OBJECT_BARREL      )  return "Barrel";
	if ( type == OBJECT_ATOMIC      )  return "NuclearCell";
	if ( type == OBJECT_BULLET      )  return "OrgaMatter";
	if ( type == OBJECT_BBOX        )  return "BlackBox";
	if ( type == OBJECT_FIOLE       )  return "Fiole";
	if ( type == OBJECT_GOAL        )  return "Goal";
	if ( type == OBJECT_BLUPI       )  return "Blupi";
	if ( type == OBJECT_BOT1        )  return "Bot1";
	if ( type == OBJECT_BOT2        )  return "Bot2";
	if ( type == OBJECT_BOT3        )  return "Bot3";
	if ( type == OBJECT_BOT4        )  return "Bot4";
	if ( type == OBJECT_BOT5        )  return "Bot5";
	if ( type == OBJECT_TNT         )  return "TNT";
	if ( type == OBJECT_MINE        )  return "Mine";
	if ( type == OBJECT_BARRIER1    )  return "Barrier";
	if ( type == OBJECT_WAYPOINT    )  return "WayPoint";
	if ( type == OBJECT_MOBILEtg    )  return "TargetBot";
	if ( type == OBJECT_MOBILEfb    )  return "FireballBot";
	if ( type == OBJECT_MOBILEob    )  return "OrgaballBot";
	if ( type == OBJECT_CARROT      )  return "Carrot";
	if ( type == OBJECT_WALKER      )  return "Walker";
	if ( type == OBJECT_RUINmobilew1)  return "Wreck";
	if ( type == OBJECT_PLANT0      )  return "Greenery";
	return "";
}


// Retourne le fichier d'aide à utiliser pour l'objet.

char* RetHelpFilename(ObjectType type)
{
	if ( type == OBJECT_START       )  return "help\\object\\start.txt";
	if ( type == OBJECT_END         )  return "help\\object\\goal.txt";
	if ( type == OBJECT_STONE       )  return "help\\object\\titanore.txt";
	if ( type == OBJECT_URANIUM     )  return "help\\object\\uranore.txt";
	if ( type == OBJECT_METAL       )  return "help\\object\\titan.txt";
	if ( type == OBJECT_ATOMIC      )  return "help\\object\\atomic.txt";
	if ( type == OBJECT_BULLET      )  return "help\\object\\bullet.txt";
	if ( type == OBJECT_BBOX        )  return "help\\object\\bbox.txt";
	if ( type == OBJECT_TNT         )  return "help\\object\\tnt.txt";
	if ( type == OBJECT_BARRIER1    )  return "help\\object\\barrier.txt";
	if ( type == OBJECT_WAYPOINT    )  return "help\\object\\waypoint.txt";
	if ( type == OBJECT_MOBILEtg    )  return "help\\object\\bottarg.txt";
	if ( type == OBJECT_RUINmobilew1)  return "help\\object\\wreck.txt";
	return "";
}


// Retourne le fichier d'aide à utiliser pour une instruction.

char* RetHelpFilename(const char *token)
{
	if ( strcmp(token, "if"            ) == 0 )  return "help\\cbot\\if.txt";
	if ( strcmp(token, "else"          ) == 0 )  return "help\\cbot\\if.txt";
	if ( strcmp(token, "for"           ) == 0 )  return "help\\cbot\\for.txt";
	if ( strcmp(token, "while"         ) == 0 )  return "help\\cbot\\while.txt";
	if ( strcmp(token, "do"            ) == 0 )  return "help\\cbot\\do.txt";
	if ( strcmp(token, "break"         ) == 0 )  return "help\\cbot\\break.txt";
	if ( strcmp(token, "continue"      ) == 0 )  return "help\\cbot\\continue.txt";
	if ( strcmp(token, "return"        ) == 0 )  return "help\\cbot\\return.txt";
	if ( strcmp(token, "sizeof"        ) == 0 )  return "help\\cbot\\sizeof.txt";
	if ( strcmp(token, "int"           ) == 0 )  return "help\\cbot\\int.txt";
	if ( strcmp(token, "float"         ) == 0 )  return "help\\cbot\\float.txt";
	if ( strcmp(token, "bool"          ) == 0 )  return "help\\cbot\\bool.txt";
	if ( strcmp(token, "string"        ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "point"         ) == 0 )  return "help\\cbot\\point.txt";
	if ( strcmp(token, "object"        ) == 0 )  return "help\\cbot\\object.txt";
	if ( strcmp(token, "file"          ) == 0 )  return "help\\cbot\\file.txt";
	if ( strcmp(token, "void"          ) == 0 )  return "help\\cbot\\void.txt";
	if ( strcmp(token, "null"          ) == 0 )  return "help\\cbot\\null.txt";
	if ( strcmp(token, "nan"           ) == 0 )  return "help\\cbot\\nan.txt";
	if ( strcmp(token, "true"          ) == 0 )  return "help\\cbot\\true.txt";
	if ( strcmp(token, "false"         ) == 0 )  return "help\\cbot\\false.txt";
	if ( strcmp(token, "sin"           ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "cos"           ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "tan"           ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "asin"          ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "acos"          ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "atan"          ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "sqrt"          ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "pow"           ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "rand"          ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "abs"           ) == 0 )  return "help\\cbot\\expr.txt";
	if ( strcmp(token, "retobject"     ) == 0 )  return "help\\cbot\\retobj.txt";
	if ( strcmp(token, "search"        ) == 0 )  return "help\\cbot\\search.txt";
	if ( strcmp(token, "radar"         ) == 0 )  return "help\\cbot\\radar.txt";
	if ( strcmp(token, "direction"     ) == 0 )  return "help\\cbot\\direct.txt";
	if ( strcmp(token, "distance"      ) == 0 )  return "help\\cbot\\dist.txt";
	if ( strcmp(token, "distance2d"    ) == 0 )  return "help\\cbot\\dist2d.txt";
	if ( strcmp(token, "space"         ) == 0 )  return "help\\cbot\\space.txt";
	if ( strcmp(token, "flatground"    ) == 0 )  return "help\\cbot\\flatgrnd.txt";
	if ( strcmp(token, "wait"          ) == 0 )  return "help\\cbot\\wait.txt";
	if ( strcmp(token, "move"          ) == 0 )  return "help\\cbot\\move.txt";
	if ( strcmp(token, "turn"          ) == 0 )  return "help\\cbot\\turn.txt";
	if ( strcmp(token, "goto"          ) == 0 )  return "help\\cbot\\goto.txt";
	if ( strcmp(token, "grab"          ) == 0 )  return "help\\cbot\\grab.txt";
	if ( strcmp(token, "drop"          ) == 0 )  return "help\\cbot\\drop.txt";
	if ( strcmp(token, "sniff"         ) == 0 )  return "help\\cbot\\sniff.txt";
	if ( strcmp(token, "receive"       ) == 0 )  return "help\\cbot\\receive.txt";
	if ( strcmp(token, "send"          ) == 0 )  return "help\\cbot\\send.txt";
	if ( strcmp(token, "deleteinfo"    ) == 0 )  return "help\\cbot\\delinfo.txt";
	if ( strcmp(token, "testinfo"      ) == 0 )  return "help\\cbot\\testinfo.txt";
	if ( strcmp(token, "thump"         ) == 0 )  return "help\\cbot\\thump.txt";
	if ( strcmp(token, "recycle"       ) == 0 )  return "help\\cbot\\recycle.txt";
	if ( strcmp(token, "shield"        ) == 0 )  return "help\\cbot\\shield.txt";
	if ( strcmp(token, "fire"          ) == 0 )  return "help\\cbot\\fire.txt";
	if ( strcmp(token, "antfire"       ) == 0 )  return "help\\cbot\\antfire.txt";
	if ( strcmp(token, "aim"           ) == 0 )  return "help\\cbot\\aim.txt";
	if ( strcmp(token, "motor"         ) == 0 )  return "help\\cbot\\motor.txt";
	if ( strcmp(token, "jet"           ) == 0 )  return "help\\cbot\\jet.txt";
	if ( strcmp(token, "topo"          ) == 0 )  return "help\\cbot\\topo.txt";
	if ( strcmp(token, "message"       ) == 0 )  return "help\\cbot\\message.txt";
	if ( strcmp(token, "abstime"       ) == 0 )  return "help\\cbot\\abstime.txt";
	if ( strcmp(token, "InFront"       ) == 0 )  return "help\\cbot\\grab.txt";
	if ( strcmp(token, "Behind"        ) == 0 )  return "help\\cbot\\grab.txt";
	if ( strcmp(token, "EnergyCell"    ) == 0 )  return "help\\cbot\\grab.txt";
	if ( strcmp(token, "DisplayError"  ) == 0 )  return "help\\cbot\\message.txt";
	if ( strcmp(token, "DisplayWarning") == 0 )  return "help\\cbot\\message.txt";
	if ( strcmp(token, "DisplayInfo"   ) == 0 )  return "help\\cbot\\message.txt";
	if ( strcmp(token, "DisplayMessage") == 0 )  return "help\\cbot\\message.txt";
	if ( strcmp(token, "strlen"        ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strleft"       ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strright"      ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strmid"        ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strval"        ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strfind"       ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strlower"      ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "strupper"      ) == 0 )  return "help\\cbot\\string.txt";
	if ( strcmp(token, "open"          ) == 0 )  return "help\\cbot\\open.txt";
	if ( strcmp(token, "close"         ) == 0 )  return "help\\cbot\\close.txt";
	if ( strcmp(token, "writeln"       ) == 0 )  return "help\\cbot\\writeln.txt";
	if ( strcmp(token, "readln "       ) == 0 )  return "help\\cbot\\readln.txt";
	if ( strcmp(token, "eof"           ) == 0 )  return "help\\cbot\\eof.txt";
	if ( strcmp(token, "deletefile"    ) == 0 )  return "help\\cbot\\deletef.txt";
	if ( strcmp(token, "openfile"      ) == 0 )  return "help\\cbot\\openfile.txt";
	if ( strcmp(token, "extern"        ) == 0 )  return "help\\cbot\\extern.txt";
	if ( strcmp(token, "class"         ) == 0 )  return "help\\cbot\\class.txt";
	if ( strcmp(token, "static"        ) == 0 )  return "help\\cbot\\static.txt";
	if ( strcmp(token, "public"        ) == 0 )  return "help\\cbot\\public.txt";
	if ( strcmp(token, "private"       ) == 0 )  return "help\\cbot\\private.txt";
	if ( strcmp(token, "synchronized"  ) == 0 )  return "help\\cbot\\synchro.txt";
	if ( strcmp(token, "new"           ) == 0 )  return "help\\cbot\\new.txt";
	if ( strcmp(token, "this"          ) == 0 )  return "help\\cbot\\this.txt";
	return "";
}


// Teste si un mot clé est un type de variable.

BOOL IsType(const char *token)
{
	if ( strcmp(token, "void"   ) == 0 )  return TRUE;
	if ( strcmp(token, "int"    ) == 0 )  return TRUE;
	if ( strcmp(token, "float"  ) == 0 )  return TRUE;
	if ( strcmp(token, "bool"   ) == 0 )  return TRUE;
	if ( strcmp(token, "string" ) == 0 )  return TRUE;
	if ( strcmp(token, "point"  ) == 0 )  return TRUE;
	if ( strcmp(token, "object" ) == 0 )  return TRUE;
	if ( strcmp(token, "file"   ) == 0 )  return TRUE;
	if ( strcmp(token, "this"   ) == 0 )  return TRUE;
	return FALSE;
}

// Teste si un mot clé est une fonction.

BOOL IsFunction(const char *token)
{
	if ( strcmp(token, "sin"          ) == 0 )  return TRUE;
	if ( strcmp(token, "cos"          ) == 0 )  return TRUE;
	if ( strcmp(token, "tan"          ) == 0 )  return TRUE;
	if ( strcmp(token, "asin"         ) == 0 )  return TRUE;
	if ( strcmp(token, "acos"         ) == 0 )  return TRUE;
	if ( strcmp(token, "atan"         ) == 0 )  return TRUE;
	if ( strcmp(token, "sqrt"         ) == 0 )  return TRUE;
	if ( strcmp(token, "pow"          ) == 0 )  return TRUE;
	if ( strcmp(token, "rand"         ) == 0 )  return TRUE;
	if ( strcmp(token, "abs"          ) == 0 )  return TRUE;
	if ( strcmp(token, "retobject"    ) == 0 )  return TRUE;
	if ( strcmp(token, "search"       ) == 0 )  return TRUE;
	if ( strcmp(token, "radar"        ) == 0 )  return TRUE;
	if ( strcmp(token, "direction"    ) == 0 )  return TRUE;
	if ( strcmp(token, "distance"     ) == 0 )  return TRUE;
	if ( strcmp(token, "distance2d"   ) == 0 )  return TRUE;
	if ( strcmp(token, "space"        ) == 0 )  return TRUE;
	if ( strcmp(token, "flatground"   ) == 0 )  return TRUE;
	if ( strcmp(token, "wait"         ) == 0 )  return TRUE;
	if ( strcmp(token, "move"         ) == 0 )  return TRUE;
	if ( strcmp(token, "turn"         ) == 0 )  return TRUE;
	if ( strcmp(token, "goto"         ) == 0 )  return TRUE;
	if ( strcmp(token, "grab"         ) == 0 )  return TRUE;
	if ( strcmp(token, "drop"         ) == 0 )  return TRUE;
	if ( strcmp(token, "sniff"        ) == 0 )  return TRUE;
	if ( strcmp(token, "receive"      ) == 0 )  return TRUE;
	if ( strcmp(token, "send"         ) == 0 )  return TRUE;
	if ( strcmp(token, "deleteinfo"   ) == 0 )  return TRUE;
	if ( strcmp(token, "testinfo"     ) == 0 )  return TRUE;
	if ( strcmp(token, "thump"        ) == 0 )  return TRUE;
	if ( strcmp(token, "recycle"      ) == 0 )  return TRUE;
	if ( strcmp(token, "shield"       ) == 0 )  return TRUE;
	if ( strcmp(token, "fire"         ) == 0 )  return TRUE;
	if ( strcmp(token, "antfire"      ) == 0 )  return TRUE;
	if ( strcmp(token, "aim"          ) == 0 )  return TRUE;
	if ( strcmp(token, "motor"        ) == 0 )  return TRUE;
	if ( strcmp(token, "jet"          ) == 0 )  return TRUE;
	if ( strcmp(token, "topo"         ) == 0 )  return TRUE;
	if ( strcmp(token, "message"      ) == 0 )  return TRUE;
	if ( strcmp(token, "abstime"      ) == 0 )  return TRUE;
	if ( strcmp(token, "ismovie"      ) == 0 )  return TRUE;
	if ( strcmp(token, "errmode"      ) == 0 )  return TRUE;
	if ( strcmp(token, "ipf"          ) == 0 )  return TRUE;
	if ( strcmp(token, "strlen"       ) == 0 )  return TRUE;
	if ( strcmp(token, "strleft"      ) == 0 )  return TRUE;
	if ( strcmp(token, "strright"     ) == 0 )  return TRUE;
	if ( strcmp(token, "strmid"       ) == 0 )  return TRUE;
	if ( strcmp(token, "strval"       ) == 0 )  return TRUE;
	if ( strcmp(token, "strfind"      ) == 0 )  return TRUE;
	if ( strcmp(token, "strlower"     ) == 0 )  return TRUE;
	if ( strcmp(token, "strupper"     ) == 0 )  return TRUE;
	if ( strcmp(token, "open"         ) == 0 )  return TRUE;
	if ( strcmp(token, "close"        ) == 0 )  return TRUE;
	if ( strcmp(token, "writeln"      ) == 0 )  return TRUE;
	if ( strcmp(token, "readln"       ) == 0 )  return TRUE;
	if ( strcmp(token, "eof"          ) == 0 )  return TRUE;
	if ( strcmp(token, "deletefile"   ) == 0 )  return TRUE;
	if ( strcmp(token, "openfile"     ) == 0 )  return TRUE;
	if ( strcmp(token, "sizeof"       ) == 0 )  return TRUE;
	return FALSE;
}


// Retourne l'aide compacte pour une instruction.

char* RetHelpText(const char *token)
{
	if ( strcmp(token, "if"        ) == 0 )  return "if ( condition ) { bloc }";
	if ( strcmp(token, "else"      ) == 0 )  return "else { bloc }";
	if ( strcmp(token, "for"       ) == 0 )  return "for ( before ; condition ; end )";
	if ( strcmp(token, "while"     ) == 0 )  return "while ( condition ) { bloc }";
	if ( strcmp(token, "do"        ) == 0 )  return "do { bloc } while ( condition );";
	if ( strcmp(token, "break"     ) == 0 )  return "break;";
	if ( strcmp(token, "continue"  ) == 0 )  return "continue;";
	if ( strcmp(token, "return"    ) == 0 )  return "return;";
	if ( strcmp(token, "sizeof"    ) == 0 )  return "sizeof( array );";
	if ( strcmp(token, "int"       ) == 0 )  return "int";
	if ( strcmp(token, "sin"       ) == 0 )  return "sin ( angle );";
	if ( strcmp(token, "cos"       ) == 0 )  return "cos ( angle );";
	if ( strcmp(token, "tan"       ) == 0 )  return "tan ( angle );";
	if ( strcmp(token, "asin"      ) == 0 )  return "asin ( value );";
	if ( strcmp(token, "acos"      ) == 0 )  return "acos ( value );";
	if ( strcmp(token, "atan"      ) == 0 )  return "atan ( value );";
	if ( strcmp(token, "sqrt"      ) == 0 )  return "sqrt ( value );";
	if ( strcmp(token, "pow"       ) == 0 )  return "pow ( x, y );";
	if ( strcmp(token, "rand"      ) == 0 )  return "rand ( );";
	if ( strcmp(token, "abs"       ) == 0 )  return "abs ( value );";
	if ( strcmp(token, "retobject" ) == 0 )  return "retobjet ( );";
	if ( strcmp(token, "search"    ) == 0 )  return "search ( );";
	if ( strcmp(token, "radar"     ) == 0 )  return "radar ( cat, angle, focus, min, max, sens );";
	if ( strcmp(token, "direction" ) == 0 )  return "direction ( position );";
	if ( strcmp(token, "distance2d") == 0 )  return "distance2d ( p1, p2 );";
	if ( strcmp(token, "distance"  ) == 0 )  return "distance ( p1, p2 );";
	if ( strcmp(token, "space"     ) == 0 )  return "space ( center, rmin, rmax, dist );";
	if ( strcmp(token, "flatground") == 0 )  return "flatground ( center, rmax );";
	if ( strcmp(token, "wait"      ) == 0 )  return "wait ( time );";
	if ( strcmp(token, "move"      ) == 0 )  return "move ( distance );";
	if ( strcmp(token, "turn"      ) == 0 )  return "turn ( angle );";
	if ( strcmp(token, "goto"      ) == 0 )  return "goto ( position, altitude );";
	if ( strcmp(token, "grab"      ) == 0 )  return "grab ( order );";
	if ( strcmp(token, "drop"      ) == 0 )  return "drop ( order );";
	if ( strcmp(token, "sniff"     ) == 0 )  return "sniff ( );";
	if ( strcmp(token, "receive"   ) == 0 )  return "receive ( name, power );";
	if ( strcmp(token, "send"      ) == 0 )  return "send ( name, value, power );";
	if ( strcmp(token, "deleteinfo") == 0 )  return "deleteinfo ( name, power );";
	if ( strcmp(token, "testinfo"  ) == 0 )  return "testinfo ( name, power );";
	if ( strcmp(token, "thump"     ) == 0 )  return "thump ( );";
	if ( strcmp(token, "recycle"   ) == 0 )  return "recycle ( );";
	if ( strcmp(token, "shield"    ) == 0 )  return "shield ( oper, radius );";
	if ( strcmp(token, "fire"      ) == 0 )  return "fire ( time );";
	if ( strcmp(token, "antfire"   ) == 0 )  return "antfire ( );";
	if ( strcmp(token, "aim"       ) == 0 )  return "aim ( angle );";
	if ( strcmp(token, "motor"     ) == 0 )  return "motor ( left, right );";
	if ( strcmp(token, "jet"       ) == 0 )  return "jet ( power );";
	if ( strcmp(token, "topo"      ) == 0 )  return "topo ( position );";
	if ( strcmp(token, "message"   ) == 0 )  return "message ( string, type );";
	if ( strcmp(token, "abstime"   ) == 0 )  return "abstime ( );";
	if ( strcmp(token, "ismovie"   ) == 0 )  return "ismovie ( );";
	if ( strcmp(token, "errmode"   ) == 0 )  return "errmode ( mdoe );";
	if ( strcmp(token, "ipf"       ) == 0 )  return "ipf ( number );";
	if ( strcmp(token, "strlen"    ) == 0 )  return "strlen ( string );";
	if ( strcmp(token, "strleft"   ) == 0 )  return "strleft ( string, len );";
	if ( strcmp(token, "strright"  ) == 0 )  return "strright ( string, len );";
	if ( strcmp(token, "strmid"    ) == 0 )  return "strmid ( string, pos, len );";
	if ( strcmp(token, "strval"    ) == 0 )  return "strval ( string );";
	if ( strcmp(token, "strfind"   ) == 0 )  return "strfind ( string, substring );";
	if ( strcmp(token, "strlower"  ) == 0 )  return "strlower ( string );";
	if ( strcmp(token, "strupper"  ) == 0 )  return "strupper ( string );";
	if ( strcmp(token, "open"      ) == 0 )  return "open ( filename, mode );";
	if ( strcmp(token, "close"     ) == 0 )  return "close ( );";
	if ( strcmp(token, "writeln"   ) == 0 )  return "writeln ( string );";
	if ( strcmp(token, "readln"    ) == 0 )  return "readln ( );";
	if ( strcmp(token, "eof"       ) == 0 )  return "eof ( );";
	if ( strcmp(token, "deletefile") == 0 )  return "deletefile ( filename );";
	if ( strcmp(token, "openfile"  ) == 0 )  return "openfile ( filename, mode );";
	return "";
}


