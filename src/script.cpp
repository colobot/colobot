// script.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "cbot/cbotdll.h"
#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "pyro.h"
#include "robotmain.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "motion.h"
#include "motionblupi.h"
#include "interface.h"
#include "edit.h"
#include "list.h"
#include "text.h"
#include "sound.h"
#include "displaytext.h"
#include "taskmanager.h"
#include "task.h"
#include "taskgoto.h"
#include "cbottoken.h"
#include "script.h"



#define CBOT_IPF	100		// CBOT: number of instructions / frame

#define ERM_CONT	0		// si erreur -> continue
#define ERM_STOP	1		// si erreur -> stoppe




// Compilation d'une procédure sans ancun paramètre.

CBotTypResult cNull(CBotVar* &var, void* user)
{
	if ( var != 0 )  return CBotErrOverParam;
	return CBotTypResult(CBotTypFloat);
}

// Compilation d'une procédure avec un seul nombre réel.

CBotTypResult cOneFloat(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Compilation d'une procédure avec deux nombres réels.

CBotTypResult cTwoFloat(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Compilation d'une procédure avec un "point".

CBotTypResult cPoint(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);

	if ( var->GivType() <= CBotTypDouble )
	{
		var = var->GivNext();
		if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
		if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
		var = var->GivNext();
//?		if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
//?		if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
//?		var = var->GivNext();
		return CBotTypResult(0);
	}

	if ( var->GivType() == CBotTypClass )
	{
		if ( !var->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadParam);
		var = var->GivNext();
		return CBotTypResult(0);
	}

	return CBotTypResult(CBotErrBadParam);
}

// Compilation d'une procédure avec un seul "point".

CBotTypResult cOnePoint(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Compilation d'une procédure avec une seule chaîne.

CBotTypResult cString(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString &&
		 var->GivType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}


// Cherche une valeur dans un tableau d'entiers.

BOOL FindList(CBotVar* array, int type)
{
	while ( array != 0 )
	{
		if ( type == array->GivValInt() )  return TRUE;
		array = array->GivNext();
	}
	return FALSE;
}


// Donne un paramètre de type "point".

BOOL GetPoint(CBotVar* &var, int& exception, D3DVECTOR& pos)
{
	CBotVar		*pX, *pY, *pZ;

	if ( var->GivType() <= CBotTypDouble )
	{
		pos.x = var->GivValFloat()*UNIT;
		var = var->GivNext();

		pos.z = var->GivValFloat()*UNIT;
		var = var->GivNext();

		pos.y = 0.0f;
	}
	else
	{
		pX = var->GivItem("x");
		if ( pX == NULL )
		{
			exception = CBotErrUndefItem;  return TRUE;
		}
		pos.x = pX->GivValFloat()*UNIT;

		pY = var->GivItem("y");
		if ( pY == NULL )
		{
			exception = CBotErrUndefItem;  return TRUE;
		}
		pos.z = pY->GivValFloat()*UNIT;  // attention y -> z !

		pZ = var->GivItem("z");
		if ( pZ == NULL )
		{
			exception = CBotErrUndefItem;  return TRUE;
		}
		pos.y = pZ->GivValFloat()*UNIT;  // attention z -> y !

		var = var->GivNext();
	}
	return TRUE;
}


// Instruction "sin(degrés)".

BOOL rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(sinf(value*PI/180.0f));
	return TRUE;
}

// Instruction "cos(degrés)".

BOOL rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(cosf(value*PI/180.0f));
	return TRUE;
}

// Instruction "tan(degrés)".

BOOL rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(tanf(value*PI/180.0f));
	return TRUE;
}

// Instruction "asin(valeur)".

BOOL raSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(asinf(value)*180.0f/PI);
	return TRUE;
}

// Instruction "acos(valeur)".

BOOL raCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(acosf(value)*180.0f/PI);
	return TRUE;
}

// Instruction "atan(valeur)".

BOOL raTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(atanf(value)*180.0f/PI);
	return TRUE;
}

// Instruction "sqrt(valeur)".

BOOL rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(sqrtf(value));
	return TRUE;
}

// Instruction "pow(x, y)".

BOOL rPow(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	x, y;

	x = var->GivValFloat();
	var = var->GivNext();
	y = var->GivValFloat();
	result->SetValFloat(powf(x, y));
	return TRUE;
}

// Instruction "rand()".

BOOL rRand(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	result->SetValFloat(Rand());
	return TRUE;
}

// Instruction "abs()".

BOOL rAbs(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(Abs(value));
	return TRUE;
}


// Compilation de l'instruction "retobject(rank)".

CBotTypResult cRetObject(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "retobject(rank)".

BOOL rRetObject(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pObj;
	int			rank;

	rank = var->GivValInt();

	pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, rank);
	if ( pObj == 0 )
	{
		result->SetPointer(0);
	}
	else
	{
		result->SetPointer(pObj->RetBotVar());
	}
	return TRUE;
}


// Compilation de l'instruction "isfree(pos)".

CBotTypResult cIsFree(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypBoolean);
}

// Instruction "isfree(pos)".

BOOL rIsFree(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CObject*	pObj;
	CMotion*	motion;
	D3DVECTOR	pos;
	ObjectType	type;
	float		value;
	LockZone	lz;

	if ( !GetPoint(var, exception, pos) )  return TRUE;

	type = pThis->RetType();

	lz = script->m_terrain->RetLockZone(pos);

	if ( type == OBJECT_CRAZY )
	{
		if ( lz == LZ_BLUPI )
		{
			pObj = script->SearchBlupi(pos);
			{
				if ( pObj != 0 && pObj->RetType() == OBJECT_BLUPI )
				{
					motion = pObj->RetMotion();
					if ( motion != 0 )
					{
						motion->SetAction(MBLUPI_STOP);
					}
				}
			}
		}
		value = (lz==LZ_FREE || lz==LZ_MINE) ? 1.0f:0.0f;
	}
	else
	{
		value = (lz==LZ_FREE) ? 1.0f:0.0f;
	}

	result->SetValFloat(value);
	return TRUE;
}


// Compilation de l'instruction "search(type, pos)".

CBotTypResult cSearch(CBotVar* &var, void* user)
{
	CBotVar*		array;
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() == CBotTypArrayPointer )
	{
		array = var->GivItemList();
		if ( array == 0 )  return CBotTypResult(CBotTypPointer);
		if ( array->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	}
	else if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )
	{
		ret = cPoint(var, user);
		if ( ret.GivType() != 0 )  return ret;
		if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	}

	return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "search(type, pos)".

BOOL rSearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject		*pObj, *pBest;
	CBotVar*	array;
	D3DVECTOR	pos, oPos;
	BOOL		bNearest = FALSE;
	BOOL		bArray;
	float		min, dist;
	int			type, oType, i;

	if ( var->GivType() == CBotTypArrayPointer )
	{
		array = var->GivItemList();
		bArray = TRUE;
	}
	else
	{
		type = var->GivValInt();
		bArray = FALSE;
	}
	var = var->GivNext();
	if ( var != 0 )
	{
		if ( !GetPoint(var, exception, pos) )  return TRUE;
		bNearest = TRUE;
	}

	min = 100000.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetTruck() != 0 )  continue;  // objet transporté ?
		if ( !pObj->RetActif() )  continue;

		oType = pObj->RetType();

		if ( oType == OBJECT_RUINmobilew2 ||
			 oType == OBJECT_RUINmobilet1 ||
			 oType == OBJECT_RUINmobilet2 ||
			 oType == OBJECT_RUINmobiler1 ||
			 oType == OBJECT_RUINmobiler2 )
		{
			oType = OBJECT_RUINmobilew1;  // n'importe quelle ruine
		}

		if ( oType >= OBJECT_PLANT0  &&
			 oType <= OBJECT_PLANT19 )
		{
			oType = OBJECT_PLANT0;  // n'importe quelle ruine
		}

		if ( bArray )
		{
			if ( !FindList(array, oType) )  continue;
		}
		else
		{
			if ( type != oType && type != OBJECT_NULL )  continue;
		}

		if ( bNearest )
		{
			oPos = pObj->RetPosition(0);
			dist = Length2d(pos, oPos);
			if ( dist < min )
			{
				min = dist;
				pBest = pObj;
			}
		}
		else
		{
			pBest = pObj;
			break;
		}
	}

	if ( pBest == 0 )
	{
		result->SetPointer(0);
	}
	else
	{
		result->SetPointer(pBest->RetBotVar());
	}
	return TRUE;
}


// Compilation de l'instruction "radar(type, angle, focus, min, max, sens)".

CBotTypResult cRadar(CBotVar* &var, void* user)
{
	CBotVar*	array;

	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() == CBotTypArrayPointer )
	{
		array = var->GivItemList();
		if ( array == 0 )  return CBotTypResult(CBotTypPointer, "object");
		if ( array->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // type
	}
	else if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // type
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // angle
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // focus
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // min
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // max
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // sens
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // filtre
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	return CBotTypResult(CBotErrOverParam);
}

// Instruction "radar(type, angle, focus, min, max, sens, filter)".

BOOL rRadar(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CObject		*pObj, *pBest;
	CBotVar*	array;
	D3DVECTOR	iPos, oPos;
	RadarFilter	filter;
	float		best, minDist, maxDist, sens, iAngle, angle, focus, d, a;
	int			type, oType, i;
	BOOL		bArray;

	type    = OBJECT_NULL;
	angle   = 0.0f;
	focus   = PI*2.0f;
	minDist = 0.0f*UNIT;
	maxDist = 1000.0f*UNIT;
	sens    = 1.0f;
	filter  = FILTER_NONE;

	if ( var != 0 )
	{
		if ( var->GivType() == CBotTypArrayPointer )
		{
			array = var->GivItemList();
			bArray = TRUE;
		}
		else
		{
			type = var->GivValInt();
			bArray = FALSE;
		}

		var = var->GivNext();
		if ( var != 0 )
		{
			angle = -var->GivValFloat()*PI/180.0f;

			var = var->GivNext();
			if ( var != 0 )
			{
				focus = var->GivValFloat()*PI/180.0f;

				var = var->GivNext();
				if ( var != 0 )
				{
					minDist = var->GivValFloat()*UNIT;

					var = var->GivNext();
					if ( var != 0 )
					{
						maxDist = var->GivValFloat()*UNIT;

						var = var->GivNext();
						if ( var != 0 )
						{
							sens = var->GivValFloat();

							var = var->GivNext();
							if ( var != 0 )
							{
								filter = (RadarFilter)var->GivValInt();
							}
						}
					}
				}
			}
		}
	}

	iPos   = pThis->RetPosition(0);
	iAngle = pThis->RetAngleY(0)+angle;
	iAngle = NormAngle(iAngle);  // 0..2*PI

	if ( sens >= 0.0f )  best = 100000.0f;
	else                 best = 0.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj == pThis )  continue;

		if ( pObj->RetTruck() != 0 )  continue;  // objet transporté ?
		if ( !pObj->RetActif() )  continue;

		oType = pObj->RetType();

		if ( oType == OBJECT_RUINmobilew2 ||
			 oType == OBJECT_RUINmobilet1 ||
			 oType == OBJECT_RUINmobilet2 ||
			 oType == OBJECT_RUINmobiler1 ||
			 oType == OBJECT_RUINmobiler2 )
		{
			oType = OBJECT_RUINmobilew1;  // n'importe quelle ruine
		}

		if ( oType >= OBJECT_PLANT0  &&
			 oType <= OBJECT_PLANT19 )
		{
			oType = OBJECT_PLANT0;  // n'importe quelle ruine
		}

		if ( bArray )
		{
			if ( !FindList(array, oType) )  continue;
		}
		else
		{
			if ( type != oType && type != OBJECT_NULL )  continue;
		}

		oPos = pObj->RetPosition(0);
		d = Length2d(iPos, oPos);
		if ( d < minDist || d > maxDist )  continue;  // trop proche ou trop loin ?

		if ( focus >= PI*2.0f )
		{
			if ( (sens >= 0.0f && d < best) ||
				 (sens <  0.0f && d > best) )
			{
				best = d;
				pBest = pObj;
			}
			continue;
		}

		a = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
		if ( TestAngle(a, iAngle-focus/2.0f, iAngle+focus/2.0f) )
		{
			if ( (sens >= 0.0f && d < best) ||
				 (sens <  0.0f && d > best) )
			{
				best = d;
				pBest = pObj;
			}
		}
	}

	if ( pBest == 0 )
	{
		result->SetPointer(0);
	}
	else
	{
		result->SetPointer(pBest->RetBotVar());
	}
	return TRUE;
}


// Compilation de l'instruction "direction(pos)".

CBotTypResult cDirection(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypFloat);
}

// Instruction "direction(pos)".

BOOL rDirection(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	D3DVECTOR	iPos, oPos;
	float		a, g;

	if ( !GetPoint(var, exception, oPos) )  return TRUE;

	iPos = pThis->RetPosition(0);

	a = pThis->RetAngleY(0);
	g = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

	result->SetValFloat(-Direction(a, g)*180.0f/PI);
	return TRUE;
}


// Compilation de l'instruction "produce(pos, angle, type, scriptName)".

CBotTypResult cProduce(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypFloat);
}

// Instruction "produce(pos, angle, type, scriptName)".

BOOL rProduce(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	object;
	CBotString	cbs;
	const char*	name;
	D3DVECTOR	pos;
	float		angle;
	ObjectType	type;

	if ( !GetPoint(var, exception, pos) )  return TRUE;

	angle = var->GivValFloat()*PI/180.0f;
	var = var->GivNext();

	type = (ObjectType)var->GivValInt();
	var = var->GivNext();

	cbs = var->GivValString();
	name = cbs;

	if ( type == OBJECT_FRET     ||
		 type == OBJECT_STONE    ||
		 type == OBJECT_URANIUM  ||
		 type == OBJECT_METAL    ||
		 type == OBJECT_BARREL   ||
		 type == OBJECT_BARRELa  ||
		 type == OBJECT_ATOMIC   ||
		 type == OBJECT_BULLET   ||
		 type == OBJECT_BBOX     ||
		 type == OBJECT_TNT      ||
		 type == OBJECT_WAYPOINT ||
		 type == OBJECT_SHOW     ||
		 type == OBJECT_WINFIRE  )
	{
		object = new CObject(script->m_iMan);
		if ( !object->CreateObject(pos, angle, 1.0f, 0.0f, type) )
		{
			delete object;
			result->SetValInt(1);  // erreur
			return TRUE;
		}
	}
	else
	{
		result->SetValInt(1);  // impossible
		return TRUE;
	}
//?	object->SetActivity(FALSE);
//?	object->ReadProgram(0, (char*)name);
//?	object->RunProgram(0);
	
	result->SetValInt(0);  // pas d'erreur
	return TRUE;
}


// Compilation de l'instruction "distance(p1, p2)".

CBotTypResult cDistance(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypFloat);
}

// Instruction "distance(p1, p2)".

BOOL rDistance(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	D3DVECTOR	p1, p2;
	float		value;

	if ( !GetPoint(var, exception, p1) )  return TRUE;
	if ( !GetPoint(var, exception, p2) )  return TRUE;

	value = Length(p1, p2);
	result->SetValFloat(value/UNIT);
	return TRUE;
}

// Instruction "distance2d(p1, p2)".

BOOL rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	D3DVECTOR	p1, p2;
	float		value;

	if ( !GetPoint(var, exception, p1) )  return TRUE;
	if ( !GetPoint(var, exception, p2) )  return TRUE;

	value = Length2d(p1, p2);
	result->SetValFloat(value/UNIT);
	return TRUE;
}


// Suivi d'une tâche.

BOOL Process(CScript* script, CBotVar* result, int &exception)
{
	Error		err;

	err = script->m_primaryTask->IsEnded();
	if ( err != ERR_CONTINUE )  // tâche terminée ?
	{
		delete script->m_primaryTask;
		script->m_primaryTask = 0;

		script->m_bContinue = FALSE;

		if ( err == ERR_STOP )  err = ERR_OK;
		result->SetValInt(err);  // indique l'erreur ou ok
		if ( err != ERR_OK && script->m_errMode == ERM_STOP )
		{
			exception = err;
			return FALSE;
		}
		return TRUE;  // c'est fini
	}

	script->m_primaryTask->EventProcess(script->m_event);
	script->m_bContinue = TRUE;
	return FALSE;  // pas fini
}


// Instruction "wait(t)".

BOOL rWait(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // pas de tâche en cours ?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskWait(value);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // indique l'erreur
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return FALSE;
			}
			return TRUE;
		}
	}
	return Process(script, result, exception);
}

// Instruction "move(dist)".

BOOL rMove(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // pas de tâche en cours ?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskMove(value*UNIT, FALSE);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // indique l'erreur
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return FALSE;
			}
			return TRUE;
		}
	}
	return Process(script, result, exception);
}

// Instruction "turn(angle)".

BOOL rTurn(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // pas de tâche en cours ?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskTurn(-value*PI/180.0f);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // indique l'erreur
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return FALSE;
			}
			return TRUE;
		}
	}
	return Process(script, result, exception);
}

// Instruction "turnim(angle)".

BOOL rTurnIm(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		value;

	exception = 0;

	value = var->GivValFloat();
	value = -value*PI/180.0f;
	pThis->SetAngleY(0, pThis->RetAngleY(0)+value);
	return TRUE;
}

// Compilation de l'instruction "goto(pos, altitude, crash, goal)".

CBotTypResult cGoto(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	return CBotTypResult(CBotErrOverParam);
}

// Instruction "goto(pos)".

BOOL rGoto(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*		script = ((CObject*)user)->RetRunScript();
	D3DVECTOR		pos;
	Error			err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // pas de tâche en cours ?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		if ( !GetPoint(var, exception, pos) )  return TRUE;

		err = script->m_primaryTask->StartTaskGoto(pos, 0, 0);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // indique l'erreur
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return FALSE;
			}
			return TRUE;
		}
	}
	return Process(script, result, exception);
}

// Instruction "gotoim(pos)".

BOOL rGotoIm(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	D3DVECTOR	pos;

	exception = 0;

	if ( !GetPoint(var, exception, pos) )  return TRUE;
	pThis->SetPosition(0, pos);
	return TRUE;
}

// Compilation "fire(delay)".

CBotTypResult cFire(CBotVar* &var, void* user)
{
	return CBotTypResult(CBotTypFloat);
}

// Instruction "fire(delay)".

BOOL rFire(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		delay;
	D3DVECTOR	impact;
	Error		err;
	ObjectType	type;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // pas de tâche en cours ?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);

		type = pThis->RetType();

		if ( var == 0 )  delay = 0.0f;
		else             delay = var->GivValFloat();
		err = script->m_primaryTask->StartTaskFire(delay);

		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // indique l'erreur
			return TRUE;
		}
	}
	return Process(script, result, exception);
}

// Compilation de l'instruction "action(n, delay)".

CBotTypResult cAction(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypFloat);
}

// Instruction "action(n, delay)".

BOOL rAction(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CObject*	pThis = (CObject*)user;
	CMotion*	motion = ((CObject*)user)->RetMotion();
	int			action;
	float		delay;

	delay = 0.2f;
	action = var->GivValInt();
	var = var->GivNext();
	if ( var != 0 )
	{
		delay = var->GivValFloat();
	}
	motion->SetAction(action, delay);
	return TRUE;
}

// Compilation de l'instruction "motor(left, right)".

CBotTypResult cMotor(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypFloat);
}

// Instruction "motor(left, right)".

BOOL rMotor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	return TRUE;
}

// Instruction "jet(power)".

BOOL rJet(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	return TRUE;
}

// Compilation de l'instruction "topo(pos)".

CBotTypResult cTopo(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	return CBotTypResult(CBotErrOverParam);
}

// Instruction "topo(pos)".

BOOL rTopo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	D3DVECTOR	pos;
	float		level;

	exception = 0;

	if ( !GetPoint(var, exception, pos) )  return TRUE;

	level = script->m_terrain->RetFloorLevel(pos);
	level -= script->m_water->RetLevel();
	result->SetValFloat(level/UNIT);
	return TRUE;
}

// Compilation de l'instruction "message(string, type)".

CBotTypResult cMessage(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString &&
		 var->GivType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	return CBotTypResult(CBotErrOverParam);
}

// Instruction "message(string, type)".

BOOL rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CBotString	cbs;
	const char*	p;
	TextType	type;

	cbs = var->GivValString();
	p = cbs;

	type = TT_MESSAGE;
	var = var->GivNext();
	if ( var != 0 )
	{
		type = (TextType)var->GivValInt();
	}

	script->m_displayText->DisplayText((char*)p, 10.0f, FONTSIZE, type);

	return TRUE;
}

// Instruction "cmdline(rank)".

BOOL rCmdline(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		value;
	int			rank;

	rank = var->GivValInt();
	value = pThis->RetCmdLine(rank);
	result->SetValFloat(value);

	return TRUE;
}

// Instruction "ismovie()".

BOOL rIsMovie(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;

	value = script->m_main->RetMovieLock()?1.0f:0.0f;
	result->SetValFloat(value);

	return TRUE;
}

// Instruction "islock()".

BOOL rIsLock(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		value;

	value = pThis->RetLock()?1.0f:0.0f;
	result->SetValFloat(value);

	return TRUE;
}

// Instruction "errmode(mode)".

BOOL rErrMode(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	int			value;

	value = var->GivValInt();
	if ( value < 0 )  value = 0;
	if ( value > 1 )  value = 1;
	script->m_errMode = value;

	return TRUE;
}

// Instruction "ipf(num)".

BOOL rIPF(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	int			value;

	value = var->GivValInt();
	if ( value <     1 )  value =     1;
	if ( value > 10000 )  value = 10000;
	script->m_ipf = value;

	return TRUE;
}

// Instruction "abstime()".

BOOL rAbsTime(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;

	value = script->m_main->RetGameTime();
	result->SetValFloat(value);
	return TRUE;
}


// Prépare un nom de fichier.

void PrepareFilename(CBotString &filename, char *dir)
{
	int			pos;

	pos = filename.ReverseFind('\\');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // enlève les dossiers
	}

	pos = filename.ReverseFind('/');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // aussi ceux avec /
	}

	pos = filename.ReverseFind(':');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // enlève aussi la lettre d'unité C:
	}

	filename = CBotString(dir) + CBotString("\\") + filename;
}

// Instruction "deletefile(filename)".

BOOL rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CBotString	cbs;
	const char*	p;
	char*		dir;

	cbs = var->GivValString();
	dir = script->m_main->RetFilesDir();
	PrepareFilename(cbs, dir);
	p = cbs;
	DeleteFile(p);

	return TRUE;
}

// Instruction "language()".

BOOL rLanguage(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();

#if _FRENCH
	result->SetValFloat(1);
#endif
#if _ENGLISH
	result->SetValFloat(2);
#endif
#if _DEUTSCH
	result->SetValFloat(3);
#endif
#if _ITALIAN
	result->SetValFloat(4);
#endif
#if _SPANISH
	result->SetValFloat(5);
#endif
#if _PORTUGUESE
	result->SetValFloat(6);
#endif
	return TRUE;
}


// Constructeur de l'objet.

CScript::CScript(CInstanceManager* iMan, CObject* object, CTaskManager** secondaryTask)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_SCRIPT, this, 100);

	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_interface   = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_botProg = 0;
	m_object = object;
	m_primaryTask = 0;
	m_secondaryTask = secondaryTask;

	m_ipf = CBOT_IPF;
	m_errMode = ERM_STOP;
	m_len = 0;
	m_script = 0;
	m_bRun = FALSE;
	m_bStepMode = FALSE;
	m_bCompile = FALSE;
	m_title[0] = 0;
	m_cursor1 = 0;
	m_cursor2 = 0;
	m_filename[0] = 0;
}

// Initialise toutes les fonctions pour le module CBOT.

void CScript::InitFonctions()
{
	CBotProgram::AddFunction("sin",       rSin,       cOneFloat);
	CBotProgram::AddFunction("cos",       rCos,       cOneFloat);
	CBotProgram::AddFunction("tan",       rTan,       cOneFloat);
	CBotProgram::AddFunction("asin",      raSin,      cOneFloat);
	CBotProgram::AddFunction("acos",      raCos,      cOneFloat);
	CBotProgram::AddFunction("atan",      raTan,      cOneFloat);
	CBotProgram::AddFunction("sqrt",      rSqrt,      cOneFloat);
	CBotProgram::AddFunction("pow",       rPow,       cTwoFloat);
	CBotProgram::AddFunction("rand",      rRand,      cNull);
	CBotProgram::AddFunction("abs",       rAbs,       cOneFloat);

	CBotProgram::AddFunction("retobject", rRetObject, cRetObject);
	CBotProgram::AddFunction("isfree",    rIsFree,    cIsFree);
	CBotProgram::AddFunction("search",    rSearch,    cSearch);
	CBotProgram::AddFunction("radar",     rRadar,     cRadar);
	CBotProgram::AddFunction("direction", rDirection, cDirection);
	CBotProgram::AddFunction("produce",   rProduce,   cProduce);
	CBotProgram::AddFunction("distance",  rDistance,  cDistance);
	CBotProgram::AddFunction("distance2d",rDistance2d,cDistance);
	CBotProgram::AddFunction("wait",      rWait,      cOneFloat);
	CBotProgram::AddFunction("move",      rMove,      cOneFloat);
	CBotProgram::AddFunction("turn",      rTurn,      cOneFloat);
	CBotProgram::AddFunction("turnim",    rTurnIm,    cOneFloat);
	CBotProgram::AddFunction("goto",      rGoto,      cGoto);
	CBotProgram::AddFunction("gotoim",    rGotoIm,    cOnePoint);
	CBotProgram::AddFunction("action",    rAction,    cAction);
	CBotProgram::AddFunction("fire",      rFire,      cFire);
	CBotProgram::AddFunction("motor",     rMotor,     cMotor);
	CBotProgram::AddFunction("jet",       rJet,       cOneFloat);
	CBotProgram::AddFunction("topo",      rTopo,      cTopo);
	CBotProgram::AddFunction("message",   rMessage,   cMessage);
	CBotProgram::AddFunction("cmdline",   rCmdline,   cOneFloat);
	CBotProgram::AddFunction("ismovie",   rIsMovie,   cNull);
	CBotProgram::AddFunction("islock",    rIsLock,    cNull);
	CBotProgram::AddFunction("errmode",   rErrMode,   cOneFloat);
	CBotProgram::AddFunction("ipf",       rIPF,       cOneFloat);
	CBotProgram::AddFunction("abstime",   rAbsTime,   cNull);
	CBotProgram::AddFunction("deletefile",rDeleteFile,cString);
	CBotProgram::AddFunction("language",  rLanguage,  cNull);
}

// Destructeur de l'objet.

CScript::~CScript()
{
	delete m_botProg;
	delete m_primaryTask;
	delete m_script;
	m_script = 0;
	m_len = 0;

	m_iMan->DeleteInstance(CLASS_SCRIPT, this);
}


// Reprend le script d'un pavé de texte.

BOOL CScript::GetScript(CEdit* edit)
{
	int		len;

	delete m_script;
	m_script = 0;

	len = edit->RetTextLength();
	m_script = (char*)malloc(sizeof(char)*(len+1));

	edit->GetText(m_script, len+1);
	m_len = strlen(m_script);

	if ( !Compile() )
	{
		edit->SetCursor(m_cursor2, m_cursor1);
		edit->ShowSelect();
		edit->SetFocus(TRUE);
		return FALSE;
	}

	return TRUE;
}

// Indique si un programme est correctement compilé.

BOOL CScript::RetCompile()
{
	return m_bCompile;
}

// Indique si le programme est vide.

BOOL CScript::IsEmpty()
{
	int		i;

	for ( i=0 ; i<m_len ; i++ )
	{
		if ( m_script[i] != ' '  &&
			 m_script[i] != '\n' )  return FALSE;
	}
	return TRUE;
}

// Compile le script d'un pavé de texte.

BOOL CScript::Compile()
{
	CBotStringArray	liste;
	int				i;
	const char*		p;

	m_error = 0;
	m_cursor1 = 0;
	m_cursor2 = 0;
	m_title[0] = 0;
	m_bCompile = FALSE;

	if ( IsEmpty() )  // programme inexistant ?
	{
		delete m_botProg;
		m_botProg = 0;
		return TRUE;
	}

	if ( m_botProg == 0 )
	{
		m_botProg = new CBotProgram(m_object->RetBotVar());
	}

	if ( m_botProg->Compile(m_script, liste, this) )
	{
		if ( liste.GivSize() == 0 )
		{
			strcpy(m_title, "<sans nom>");
		}
		else
		{
			p = liste[0];
			i = 0;
			while ( TRUE )
			{
				if ( p[i] == 0 || p[i] == '(' )  break;
				if ( i >= 20 )
				{
					m_title[i++] = '.';
					m_title[i++] = '.';
					m_title[i++] = '.';
					break;
				}
				m_title[i] = p[i];
				i ++;
			}
			m_title[i] = 0;
		}
		m_bCompile = TRUE;
		return TRUE;
	}
	else
	{
		m_botProg->GetError(m_error, m_cursor1, m_cursor2);
		if ( m_cursor1 < 0 || m_cursor1 > m_len ||
			 m_cursor2 < 0 || m_cursor2 > m_len )
		{
			m_cursor1 = 0;
			m_cursor2 = 0;
		}
		if ( m_error == 0 )
		{
			m_cursor1 = m_cursor2 = 0;
		}
		strcpy(m_title, "<erreur>");
		return FALSE;
	}
}


// Retourne le titre du script.

void CScript::GetTitle(char* buffer)
{
	strcpy(buffer, m_title);
}


// Choix du mode d'exécution.

void CScript::SetStepMode(BOOL bStep)
{
	m_bStepMode = bStep;
}


// Lance le programme depuis le début.

BOOL CScript::Run()
{
	if( m_botProg == 0 )  return FALSE;
	if ( m_script == 0 || m_len == 0 )  return FALSE;

	if ( !m_botProg->Start(m_title) )  return FALSE;

	m_object->SetRunScript(this);
	m_bRun = TRUE;
	m_bContinue = FALSE;
	m_ipf = CBOT_IPF;
	m_errMode = ERM_STOP;

	if ( m_bStepMode )  // mode step by step ?
	{
		Event	newEvent;
		ZeroMemory(&newEvent, sizeof(Event));
		Step(newEvent);
	}

	return TRUE;
}

// Continue le programme en cours d'exécution.
// Retourne TRUE lorsque l'exécution est terminée.

BOOL CScript::Continue(const Event &event)
{
	if( m_botProg == 0 )  return TRUE;
	if ( !m_bRun )  return TRUE;

	m_event = event;

	if ( m_bStepMode )  // mode step by step ?
	{
		if ( m_bContinue )  // instuction "move", "goto", etc. ?
		{
			if ( m_botProg->Run(m_object, 0) )
			{
				m_botProg->GetError(m_error, m_cursor1, m_cursor2);
				if ( m_cursor1 < 0 || m_cursor1 > m_len ||
					 m_cursor2 < 0 || m_cursor2 > m_len )
				{
					m_cursor1 = 0;
					m_cursor2 = 0;
				}
				if ( m_error == 0 )
				{
					m_cursor1 = m_cursor2 = 0;
				}
				m_bRun = FALSE;

				if ( m_error != 0 && m_errMode == ERM_STOP )
				{
					char	s[100];
					GetError(s);
					m_displayText->DisplayText(s, 10.0f, FONTSIZE, TT_ERROR);
				}
				m_engine->SetPause(TRUE);  // remet la pause
				return TRUE;
			}
			if ( !m_bContinue )
			{
				m_engine->SetPause(TRUE);  // remet la pause
			}
		}

		return FALSE;
	}

	if ( m_botProg->Run(m_object, m_ipf) )
	{
		m_botProg->GetError(m_error, m_cursor1, m_cursor2);
		if ( m_cursor1 < 0 || m_cursor1 > m_len ||
			 m_cursor2 < 0 || m_cursor2 > m_len )
		{
			m_cursor1 = 0;
			m_cursor2 = 0;
		}
		if ( m_error == 0 )
		{
			m_cursor1 = m_cursor2 = 0;
		}
		m_bRun = FALSE;

		if ( m_error != 0 && m_errMode == ERM_STOP )
		{
			char	s[100];
			GetError(s);
			m_displayText->DisplayText(s, 10.0f, FONTSIZE, TT_ERROR);
		}
		return TRUE;
	}

	return FALSE;
}

// Continue le programme en cours d'exécution.
// Retourne TRUE lorsque l'exécution est terminée.

BOOL CScript::Step(const Event &event)
{
	if( m_botProg == 0 )  return TRUE;
	if ( !m_bRun )  return TRUE;
	if ( !m_bStepMode )  return FALSE;

	m_engine->SetPause(FALSE);
	m_engine->StepSimul(0.01f);  // avance de 10ms
	m_engine->SetPause(TRUE);

	m_event = event;

	if ( m_botProg->Run(m_object, 0) )  // en mode step
	{
		m_botProg->GetError(m_error, m_cursor1, m_cursor2);
		if ( m_cursor1 < 0 || m_cursor1 > m_len ||
			 m_cursor2 < 0 || m_cursor2 > m_len )
		{
			m_cursor1 = 0;
			m_cursor2 = 0;
		}
		if ( m_error == 0 )
		{
			m_cursor1 = m_cursor2 = 0;
		}
		m_bRun = FALSE;

		if ( m_error != 0 && m_errMode == ERM_STOP )
		{
			char	s[100];
			GetError(s);
			m_displayText->DisplayText(s, 10.0f, FONTSIZE, TT_ERROR);
		}
		return TRUE;
	}

	if ( m_bContinue )  // instuction "move", "goto", etc. ?
	{
		m_engine->SetPause(FALSE);  // enlève la pause
	}
	return FALSE;
}

// Stoppe le programme.

void CScript::Stop()
{
	if ( !m_bRun )  return;

	if( m_botProg != 0 )
	{
		m_botProg->Stop();
	}

	if ( m_primaryTask != 0 )
	{
		m_primaryTask->Abort();
		delete m_primaryTask;
		m_primaryTask = 0;
	}

	m_bRun = FALSE;
}

// Indique si le programme tourne.

BOOL CScript::IsRunning()
{
	return m_bRun;
}

// Indique si le programme continue un step.

BOOL CScript::IsContinue()
{
	return m_bContinue;
}


// Donne la position des curseurs pendant l'exécution.

BOOL CScript::GetCursor(int &cursor1, int &cursor2)
{
	const char*	funcName;

	cursor1 = cursor2 = 0;

	if( m_botProg == 0 )  return FALSE;
	if ( !m_bRun )  return FALSE;

	m_botProg->GetRunPos(funcName, cursor1, cursor2);
	if ( cursor1 < 0 || cursor1 > m_len ||
		 cursor2 < 0 || cursor2 > m_len )
	{
		cursor1 = 0;
		cursor2 = 0;
	}
	return TRUE;
}


// Met des variables dans une liste.

void PutList(char *baseName, BOOL bArray, CBotVar *var, CList *list, int &rankList)
{
	CBotString	bs;
	CBotVar		*svar, *pStatic;
	char		varName[100];
	char		buffer[100];
	const char	*p;
	int			index, type;

	if ( var == 0 && baseName[0] != 0 )
	{
		sprintf(buffer, "%s = null;", baseName);
		list->SetName(rankList++, buffer);
		return;
	}

	index = 0;
	while ( var != 0 )
	{
		var->Maj(NULL, FALSE);
		pStatic = var->GivStaticVar();  // retrouve l'élément static

		bs = pStatic->GivName();  // nom de la variable
		p = bs;
//?		if ( strcmp(p, "this") == 0 )
//?		{
//?			var = var->GivNext();
//?			continue;
//?		}

		if ( baseName[0] == 0 )
		{
			sprintf(varName, "%s", p);
		}
		else
		{
			if ( bArray )
			{
				sprintf(varName, "%s[%d]", baseName, index);
			}
			else
			{
				sprintf(varName, "%s.%s", baseName, p);
			}
		}

		type = pStatic->GivType();

		if ( type < CBotTypBoolean )
		{
			CBotString	value;
			value = pStatic->GivValString();
			p = value;
			sprintf(buffer, "%s = %s;", varName, p);
			list->SetName(rankList++, buffer);
		}
		else if ( type == CBotTypString )
		{
			CBotString	value;
			value = pStatic->GivValString();
			p = value;
			sprintf(buffer, "%s = \"%s\";", varName, p);
			list->SetName(rankList++, buffer);
		}
		else if ( type == CBotTypArrayPointer )
		{
			svar = pStatic->GivItemList();
			PutList(varName, TRUE, svar, list, rankList);
		}
		else if ( type == CBotTypClass   ||
				  type == CBotTypPointer )
		{
			svar = pStatic->GivItemList();
			PutList(varName, FALSE, svar, list, rankList);
		}
		else
		{
			sprintf(buffer, "%s = ?;", varName);
			list->SetName(rankList++, buffer);
		}

		index ++;
		var = var->GivNext();
	}
}

// Rempli une liste avec les variables.

void CScript::UpdateList(CList* list)
{
	CBotVar		*var;
	const char	*progName, *funcName;
	int			total, select, level, cursor1, cursor2, rank;

	if( m_botProg == 0 )  return;

	total  = list->RetTotal();
	select = list->RetSelect();

	list->Flush();  // vide la liste
	m_botProg->GetRunPos(progName, cursor1, cursor2);
	if ( progName == 0 )  return;

	level = 0;
	rank  = 0;
	while ( TRUE )
	{
		var = m_botProg->GivStackVars(funcName, level--);
		if ( funcName != progName )  break;

		PutList("", FALSE, var, list, rank);
	}

	if ( total == list->RetTotal() )  // même total ?
	{
		list->SetSelect(select);
	}

	list->SetTooltip("");
	list->SetState(STATE_ENABLE);
}


// Colorise le texte selon la syntaxe.

void CScript::ColorizeScript(CEdit* edit)
{
	CBotToken*	bt;
	CBotString	bs;
	const char*	token;
	int			error, type, cursor1, cursor2, color;

	edit->ClearFormat();

	bt = CBotToken::CompileTokens(edit->RetText(), error);
	while ( bt != 0 )
	{
		bs = bt->GivString();
		token = bs;
		type = bt->GivType();

		cursor1 = bt->GivStart();
		cursor2 = bt->GivEnd();

		color = 0;
		if ( type >= TokenKeyWord && type < TokenKeyWord+100 )
		{
			color = COLOR_TOKEN;
		}
		if ( type >= TokenKeyDeclare && type < TokenKeyDeclare+100 )
		{
			color = COLOR_TYPE;
		}
		if ( type >= TokenKeyVal && type < TokenKeyVal+100 )
		{
			color = COLOR_CONST;
		}
		if ( type == TokenTypVar )
		{
			if ( IsType(token) )
			{
				color = COLOR_TYPE;
			}
			else if ( IsFunction(token) )
			{
				color = COLOR_TOKEN;
			}
		}
		if ( type == TokenTypDef )
		{
			color = COLOR_CONST;
		}

		if ( cursor1 < cursor2 && color != 0 )
		{
			edit->SetFormat(cursor1, cursor2, color);
		}

		bt = bt->GivNext();
	}

	CBotToken::Delete(bt);
}


// Cherche un token au hazard dans un script.
// Retourne l'index du début du token trouvé, ou -1.

int SearchToken(char* script, char* token)
{
	int		lScript, lToken, i, iFound;
	int		found[100];
	char*	p;

	lScript = strlen(script);
	lToken  = strlen(token);
	iFound  = 0;
	for ( i=0 ; i<lScript-lToken ; i++ )
	{
		p = strstr(script+i, token);
		if ( p != 0 )
		{
			found[iFound++] = p-script;
			if ( iFound >= 100 )  break;
		}
	}

	if ( iFound == 0 )  return -1;
	return found[rand()%iFound];
}

// Supprime un token dans un script.

void DeleteToken(char* script, int pos, int len)
{
	while ( TRUE )
	{
		script[pos] = script[pos+len];
		if ( script[pos++] == 0 )  break;
	}
}

// Insère un token dans un script.

void InsertToken(char* script, int pos, char* token)
{
	int		lScript, lToken, i;

	lScript = strlen(script);
	lToken  = strlen(token);
	for ( i=lScript ; i>=pos ; i-- )
	{
		script[i+lToken] = script[i];
	}
	memcpy(script+pos, token, lToken);
}


// Retourne le numéro de l'erreur.

int CScript::RetError()
{
	return m_error;
}

// Retourne le texte de l'erreur.

void CScript::GetError(char* buffer)
{
	if ( m_error == 0 )
	{
		buffer[0] = 0;
	}
	else
	{
		if ( m_error < 1000 )
		{
			GetResource(RES_ERR, m_error, buffer);
		}
		else
		{
			GetResource(RES_CBOT, m_error, buffer);
		}
	}
}


// Lit un script sous la forme d'un fichier texte.

BOOL CScript::ReadScript(char* filename)
{
	FILE*		file;
	CEdit*		edit;
	char		name[100];

	if ( strchr(filename, '\\') == 0 )
	{
		strcpy(name, "script\\");
		strcat(name, filename);
	}
	else
	{
//?		strcpy(name, filename);
		UserDir(name, filename, "");
	}

	file = fopen(name, "rb");
	if ( file == NULL )  return FALSE;
	fclose(file);

	delete m_script;
	m_script = 0;

	edit = m_interface->CreateEdit(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), 0, EVENT_EDIT9);
	edit->SetMaxChar(EDITSTUDIOMAX);
	edit->SetAutoIndent(m_engine->RetEditIndentMode());
	edit->ReadText(name);
	GetScript(edit);
	m_interface->DeleteControl(EVENT_EDIT9);
	return TRUE;
}

// Ecrit un script sous la forme d'un fichier texte.

BOOL CScript::WriteScript(char* filename)
{
	CEdit*		edit;
	char		name[100];

	if ( strchr(filename, '\\') == 0 )
	{
		strcpy(name, "script\\");
		strcat(name, filename);
	}
	else
	{
		strcpy(name, filename);
	}

	if ( m_script == 0 )
	{
		remove(filename);
		return FALSE;
	}

	edit = m_interface->CreateEdit(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), 0, EVENT_EDIT9);
	edit->SetMaxChar(EDITSTUDIOMAX);
	edit->SetAutoIndent(m_engine->RetEditIndentMode());
	edit->SetText(m_script);
	edit->WriteText(name);
	m_interface->DeleteControl(EVENT_EDIT9);
	return TRUE;
}


// Lit un stack de script en exécution sous la forme d'un fichier.

BOOL CScript::ReadStack(FILE *file)
{
	int		nb;

	fRead(&nb, sizeof(int), 1, file);
	fRead(&m_ipf, sizeof(int), 1, file);
	fRead(&m_errMode, sizeof(int), 1, file);

	if ( m_botProg == 0 )  return FALSE;
	if ( !m_botProg->RestoreState(file) )  return FALSE;
	
	m_object->SetRunScript(this);
	m_bRun = TRUE;
	m_bContinue = FALSE;
	return TRUE;
}

// Ecrit un stack de script en exécution sous la forme d'un fichier.

BOOL CScript::WriteStack(FILE *file)
{
	int		nb;

	nb = 2;
	fWrite(&nb, sizeof(int), 1, file);
	fWrite(&m_ipf, sizeof(int), 1, file);
	fWrite(&m_errMode, sizeof(int), 1, file);

	return m_botProg->SaveState(file);
}


// Compare deux scripts.

BOOL CScript::Compare(CScript* other)
{
	if ( m_len != other->m_len )  return FALSE;

	return ( strcmp(m_script, other->m_script) == 0 );
}


// Gestion du nom de fichier lorsque le script est sauvegardé.

void CScript::SetFilename(char *filename)
{
	strcpy(m_filename, filename);
}

char* CScript::RetFilename()
{
	return m_filename;
}

// Cherche un blupi.

CObject* CScript::SearchBlupi(D3DVECTOR center)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj == m_object )  continue;  // soi-même ?
		if ( pObj->RetLock() )  continue;
		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();
		if ( type != OBJECT_BLUPI )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= 2.0f && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

