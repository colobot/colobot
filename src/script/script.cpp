// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// script.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "CBot/CBotDll.h"
#include "common/struct.h"
#include "graphics/d3d/d3dengine.h"
#include "math/old/d3dmath.h"
#include "common/global.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "common/restext.h"
#include "math/old/math3d.h"
#include "object/robotmain.h"
#include "graphics/common/terrain.h"
#include "graphics/common/water.h"
#include "object/object.h"
#include "physics/physics.h"
#include "ui/interface.h"
#include "ui/edit.h"
#include "ui/list.h"
#include "graphics/common/text.h"
#include "ui/displaytext.h"
#include "object/task/taskmanager.h"
#include "object/task/task.h"
#include "object/task/taskmanip.h"
#include "object/task/taskgoto.h"
#include "object/task/taskshield.h"
#include "script/cbottoken.h"
#include "script/script.h"



#define CBOT_IPF	100		// CBOT: number of instructions / frame

#define ERM_CONT	0		// if error -> continue
#define ERM_STOP	1		// if error -> stop




// Compiling a procedure without any parameters.

CBotTypResult cNull(CBotVar* &var, void* user)
{
	if ( var != 0 )  return CBotErrOverParam;
	return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single real number.

CBotTypResult cOneFloat(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with two real numbers.

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

// Compiling a procedure with a "dot".

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

// Compiling a procedure with a single "point".

CBotTypResult cOnePoint(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single string.

CBotTypResult cString(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString &&
		 var->GivType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}


// Seeking value in an array of integers.

bool FindList(CBotVar* array, int type)
{
	while ( array != 0 )
	{
		if ( type == array->GivValInt() )  return true;
		array = array->GivNext();
	}
	return false;
}


// Gives a parameter of type "point".

bool GetPoint(CBotVar* &var, int& exception, D3DVECTOR& pos)
{
	CBotVar		*pX, *pY, *pZ;

	if ( var->GivType() <= CBotTypDouble )
	{
		pos.x = var->GivValFloat()*g_unit;
		var = var->GivNext();

		pos.z = var->GivValFloat()*g_unit;
		var = var->GivNext();

		pos.y = 0.0f;
	}
	else
	{
		pX = var->GivItem("x");
		if ( pX == NULL )
		{
			exception = CBotErrUndefItem;  return true;
		}
		pos.x = pX->GivValFloat()*g_unit;

		pY = var->GivItem("y");
		if ( pY == NULL )
		{
			exception = CBotErrUndefItem;  return true;
		}
		pos.z = pY->GivValFloat()*g_unit;  // attention y -> z !

		pZ = var->GivItem("z");
		if ( pZ == NULL )
		{
			exception = CBotErrUndefItem;  return true;
		}
		pos.y = pZ->GivValFloat()*g_unit;  // attention z -> y !

		var = var->GivNext();
	}
	return true;
}


// Instruction "sin(degrees)".

bool rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(sinf(value*PI/180.0f));
	return true;
}

// Instruction "cos(degrees)".

bool rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(cosf(value*PI/180.0f));
	return true;
}

// Instruction "tan(degrees)".

bool rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(tanf(value*PI/180.0f));
	return true;
}

// Instruction "asin(degrees)".

bool raSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(asinf(value)*180.0f/PI);
	return true;
}

// Instruction "acos(degrees)".

bool raCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(acosf(value)*180.0f/PI);
	return true;
}

// Instruction "atan(degrees)".

bool raTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(atanf(value)*180.0f/PI);
	return true;
}

// Instruction "sqrt(value)".

bool rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(sqrtf(value));
	return true;
}

// Instruction "pow(x, y)".

bool rPow(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	x, y;

	x = var->GivValFloat();
	var = var->GivNext();
	y = var->GivValFloat();
	result->SetValFloat(powf(x, y));
	return true;
}

// Instruction "rand()".

bool rRand(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	result->SetValFloat(Rand());
	return true;
}

// Instruction "abs()".

bool rAbs(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	float	value;

	value = var->GivValFloat();
	result->SetValFloat(Abs(value));
	return true;
}


// Compilation of the instruction "retobject(rank)".

CBotTypResult cRetObject(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "retobject(rank)".

bool rRetObject(CBotVar* var, CBotVar* result, int& exception, void* user)
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
	return true;
}


// Compilation of the instruction "search(type, pos)".

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

bool rSearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject		*pObj, *pBest;
	CBotVar*	array;
	D3DVECTOR	pos, oPos;
	bool		bNearest = false;
	bool		bArray;
	float		min, dist;
	int			type, oType, i;

	if ( var->GivType() == CBotTypArrayPointer )
	{
		array = var->GivItemList();
		bArray = true;
	}
	else
	{
		type = var->GivValInt();
		bArray = false;
	}
	var = var->GivNext();
	if ( var != 0 )
	{
		if ( !GetPoint(var, exception, pos) )  return true;
		bNearest = true;
	}

	min = 100000.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetTruck() != 0 )  continue;  // object transported?
		if ( !pObj->RetActif() )  continue;

		oType = pObj->RetType();
		if ( oType == OBJECT_TOTO )  continue;

		if ( oType == OBJECT_RUINmobilew2 ||
			 oType == OBJECT_RUINmobilet1 ||
			 oType == OBJECT_RUINmobilet2 ||
			 oType == OBJECT_RUINmobiler1 ||
			 oType == OBJECT_RUINmobiler2 )
		{
			oType = OBJECT_RUINmobilew1;  // any ruin
		}

		if ( oType == OBJECT_SCRAP2 ||
			 oType == OBJECT_SCRAP3 ||
			 oType == OBJECT_SCRAP4 ||
			 oType == OBJECT_SCRAP5 )  // wastes?
		{
			oType = OBJECT_SCRAP1;  // any waste
		}

		if ( oType == OBJECT_BARRIER2 ||
			 oType == OBJECT_BARRIER3 )  // barriers?
		{
			oType = OBJECT_BARRIER1;  // any barrier
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
	return true;
}


// Compilation of instruction "radar(type, angle, focus, min, max, sens)".

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
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // sense
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // filter
	var = var->GivNext();
	if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
	return CBotTypResult(CBotErrOverParam);
}

// Instruction "radar(type, angle, focus, min, max, sens, filter)".

bool rRadar(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CObject		*pObj, *pBest;
	CPhysics*	physics;
	CBotVar*	array;
	D3DVECTOR	iPos, oPos;
	RadarFilter	filter;
	float		best, minDist, maxDist, sens, iAngle, angle, focus, d, a;
	int			type, oType, i;
	bool		bArray;

	type    = OBJECT_NULL;
	angle   = 0.0f;
	focus   = PI*2.0f;
	minDist = 0.0f*g_unit;
	maxDist = 1000.0f*g_unit;
	sens    = 1.0f;
	filter  = FILTER_NONE;

	if ( var != 0 )
	{
		if ( var->GivType() == CBotTypArrayPointer )
		{
			array = var->GivItemList();
			bArray = true;
		}
		else
		{
			type = var->GivValInt();
			bArray = false;
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
					minDist = var->GivValFloat()*g_unit;

					var = var->GivNext();
					if ( var != 0 )
					{
						maxDist = var->GivValFloat()*g_unit;

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

		if ( pObj->RetTruck() != 0 )  continue;  // object transported?
		if ( !pObj->RetActif() )  continue;
		if ( pObj->RetProxyActivate() )  continue;

		oType = pObj->RetType();
		if ( oType == OBJECT_TOTO )  continue;

		if ( oType == OBJECT_RUINmobilew2 ||
			 oType == OBJECT_RUINmobilet1 ||
			 oType == OBJECT_RUINmobilet2 ||
			 oType == OBJECT_RUINmobiler1 ||
			 oType == OBJECT_RUINmobiler2 )
		{
			oType = OBJECT_RUINmobilew1;  // any ruin
		}

		if ( oType == OBJECT_SCRAP2 ||
			 oType == OBJECT_SCRAP3 ||
			 oType == OBJECT_SCRAP4 ||
			 oType == OBJECT_SCRAP5 )  // wastes?
		{
			oType = OBJECT_SCRAP1;  // any waste
		}

		if ( oType == OBJECT_BARRIER2 ||
			 oType == OBJECT_BARRIER3 )  // barriers?
		{
			oType = OBJECT_BARRIER1;  // any barrier
		}

		if ( filter == FILTER_ONLYLANDING )
		{
			physics = pObj->RetPhysics();
			if ( physics != 0 && !physics->RetLand() )  continue;
		}
		if ( filter == FILTER_ONLYFLYING )
		{
			physics = pObj->RetPhysics();
			if ( physics != 0 && physics->RetLand() )  continue;
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
		if ( d < minDist || d > maxDist )  continue;  // too close or too far?

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
	return true;
}


// Monitoring a task.

bool Process(CScript* script, CBotVar* result, int &exception)
{
	Error		err;

	err = script->m_primaryTask->IsEnded();
	if ( err != ERR_CONTINUE )  // task terminated?
	{
		delete script->m_primaryTask;
		script->m_primaryTask = 0;

		script->m_bContinue = false;

		if ( err == ERR_STOP )  err = ERR_OK;
		result->SetValInt(err);  // indicates the error or ok
		if ( err != ERR_OK && script->m_errMode == ERM_STOP )
		{
			exception = err;
			return false;
		}
		return true;  // it's all over
	}

	script->m_primaryTask->EventProcess(script->m_event);
	script->m_bContinue = true;
	return false;  // not done
}


// Compilation of the instruction "detect(type)".

CBotTypResult cDetect(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypBoolean);
}

// Instruction "detect(type)".

bool rDetect(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CObject		*pObj, *pGoal, *pBest;
	CPhysics*	physics;
	CBotVar*	array;
	D3DVECTOR	iPos, oPos;
	RadarFilter	filter;
	float		bGoal, best, minDist, maxDist, sens, iAngle, angle, focus, d, a;
	int			type, oType, i;
	bool		bArray;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		type    = OBJECT_NULL;
		angle   = 0.0f;
		focus   = 45.0f*PI/180.0f;
		minDist = 0.0f*g_unit;
		maxDist = 20.0f*g_unit;
		sens    = 1.0f;
		filter  = FILTER_NONE;

		if ( var != 0 )
		{
			if ( var->GivType() == CBotTypArrayPointer )
			{
				array = var->GivItemList();
				bArray = true;
			}
			else
			{
				type = var->GivValInt();
				bArray = false;
			}
		}

		iPos   = pThis->RetPosition(0);
		iAngle = pThis->RetAngleY(0)+angle;
		iAngle = NormAngle(iAngle);  // 0..2*PI

		bGoal = 100000.0f;
		pGoal = 0;
		if ( sens >= 0.0f )  best = 100000.0f;
		else                 best = 0.0f;
		pBest = 0;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;
			if ( pObj == pThis )  continue;

			if ( pObj->RetTruck() != 0 )  continue;  // object transported?
			if ( !pObj->RetActif() )  continue;
			if ( pObj->RetProxyActivate() )  continue;

			oType = pObj->RetType();
			if ( oType == OBJECT_TOTO )  continue;

			if ( oType == OBJECT_RUINmobilew2 ||
				 oType == OBJECT_RUINmobilet1 ||
				 oType == OBJECT_RUINmobilet2 ||
				 oType == OBJECT_RUINmobiler1 ||
				 oType == OBJECT_RUINmobiler2 )
			{
				oType = OBJECT_RUINmobilew1;  // any ruin
			}

			if ( oType == OBJECT_SCRAP2 ||
				 oType == OBJECT_SCRAP3 ||
				 oType == OBJECT_SCRAP4 ||
				 oType == OBJECT_SCRAP5 )  // wastes?
			{
				oType = OBJECT_SCRAP1;  // any waste
			}

			if ( oType == OBJECT_BARRIER2 ||
				 oType == OBJECT_BARRIER3 )  // barriers?
			{
				oType = OBJECT_BARRIER1;  // any barrier
			}

			if ( filter == FILTER_ONLYLANDING )
			{
				physics = pObj->RetPhysics();
				if ( physics != 0 && !physics->RetLand() )  continue;
			}
			if ( filter == FILTER_ONLYFLYING )
			{
				physics = pObj->RetPhysics();
				if ( physics != 0 && physics->RetLand() )  continue;
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
			a = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

			if ( d < bGoal &&
				 TestAngle(a, iAngle-(5.0f*PI/180.0f)/2.0f, iAngle+(5.0f*PI/180.0f)/2.0f) )
			{
				bGoal = d;
				pGoal = pObj;
			}

			if ( d < minDist || d > maxDist )  continue;  // too close or too far?

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

		pThis->StartDetectEffect(pGoal, pBest!=0);

		if ( pBest == 0 )
		{
			script->m_returnValue = 0.0f;
		}
		else
		{
			script->m_returnValue = 1.0f;
		}

		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		err = script->m_primaryTask->StartTaskWait(0.3f);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	if ( !Process(script, result, exception) )  return false;  // not finished
	result->SetValFloat(script->m_returnValue);
	return true;
}


// Compilation of the instruction "direction(pos)".

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

bool rDirection(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	D3DVECTOR	iPos, oPos;
	float		a, g;

	if ( !GetPoint(var, exception, oPos) )  return true;

	iPos = pThis->RetPosition(0);

	a = pThis->RetAngleY(0);
	g = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

	result->SetValFloat(-Direction(a, g)*180.0f/PI);
	return true;
}


// Compilation of the instruction "produce(pos, angle, type, scriptName)".

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

bool rProduce(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	object;
	CBotString	cbs;
	const char*	name;
	D3DVECTOR	pos;
	float		angle;
	ObjectType	type;

	if ( !GetPoint(var, exception, pos) )  return true;

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
		 type == OBJECT_POWER    ||
		 type == OBJECT_ATOMIC   ||
		 type == OBJECT_BULLET   ||
		 type == OBJECT_BBOX     ||
		 type == OBJECT_KEYa     ||
		 type == OBJECT_KEYb     ||
		 type == OBJECT_KEYc     ||
		 type == OBJECT_KEYd     ||
		 type == OBJECT_TNT      ||
		 type == OBJECT_SCRAP1   ||
		 type == OBJECT_SCRAP2   ||
		 type == OBJECT_SCRAP3   ||
		 type == OBJECT_SCRAP4   ||
		 type == OBJECT_SCRAP5   ||
		 type == OBJECT_BOMB     ||
		 type == OBJECT_WAYPOINT ||
		 type == OBJECT_SHOW     ||
		 type == OBJECT_WINFIRE  )
	{
		object = new CObject(script->m_iMan);
		if ( !object->CreateResource(pos, angle, type) )
		{
			delete object;
			result->SetValInt(1);  // error
			return true;
		}
	}
	else
	if ( type == OBJECT_MOTHER ||
		 type == OBJECT_ANT    ||
		 type == OBJECT_SPIDER ||
		 type == OBJECT_BEE    ||
		 type == OBJECT_WORM   )
	{
		CObject*	egg;

		object = new CObject(script->m_iMan);
		if ( !object->CreateInsect(pos, angle, type) )
		{
			delete object;
			result->SetValInt(1);  // error
			return true;
		}

		egg = new CObject(script->m_iMan);
		if ( !egg->CreateResource(pos, angle, OBJECT_EGG, 0.0f) )
		{
			delete egg;
		}
	}
	else
	{
		result->SetValInt(1);  // impossible
		return true;
	}
	object->SetActivity(false);
	object->ReadProgram(0, (char*)name);
	object->RunProgram(0);
	
	result->SetValInt(0);  // no error
	return true;
}


// Compilation of the instruction "distance(p1, p2)".

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

bool rDistance(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	D3DVECTOR	p1, p2;
	float		value;

	if ( !GetPoint(var, exception, p1) )  return true;
	if ( !GetPoint(var, exception, p2) )  return true;

	value = Length(p1, p2);
	result->SetValFloat(value/g_unit);
	return true;
}

// Instruction "distance2d(p1, p2)".

bool rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	D3DVECTOR	p1, p2;
	float		value;

	if ( !GetPoint(var, exception, p1) )  return true;
	if ( !GetPoint(var, exception, p2) )  return true;

	value = Length2d(p1, p2);
	result->SetValFloat(value/g_unit);
	return true;
}


// Compilation of the instruction "space(center, rMin, rMax, dist)".

CBotTypResult cSpace(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypIntrinsic, "point");
}

// Instruction "space(center, rMin, rMax, dist)".

bool rSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CBotVar*	pSub;
	D3DVECTOR	center;
	float		rMin, rMax, dist;

	rMin = 10.0f*g_unit;
	rMax = 50.0f*g_unit;
	dist =  4.0f*g_unit;

	if ( var == 0 )
	{
		center = pThis->RetPosition(0);
	}
	else
	{
		if ( !GetPoint(var, exception, center) )  return true;

		if ( var != 0 )
		{
			rMin = var->GivValFloat()*g_unit;
			var = var->GivNext();

			if ( var != 0 )
			{
				rMax = var->GivValFloat()*g_unit;
				var = var->GivNext();

				if ( var != 0 )
				{
					dist = var->GivValFloat()*g_unit;
					var = var->GivNext();
				}
			}
		}
	}
	script->m_main->FreeSpace(center, rMin, rMax, dist, pThis);

	if ( result != 0 )
	{
		pSub = result->GivItemList();
		if ( pSub != 0 )
		{
			pSub->SetValFloat(center.x/g_unit);
			pSub = pSub->GivNext();  // "y"
			pSub->SetValFloat(center.z/g_unit);
			pSub = pSub->GivNext();  // "z"
			pSub->SetValFloat(center.y/g_unit);
		}
	}
	return true;
}


// Compilation of the instruction "flatground(center, rMax)".

CBotTypResult cFlatGround(CBotVar* &var, void* user)
{
	CBotTypResult	ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	ret = cPoint(var, user);
	if ( ret.GivType() != 0 )  return ret;

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

	return CBotTypResult(CBotTypFloat);
}

// Instruction "flatground(center, rMax)".

bool rFlatGround(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	D3DVECTOR	center;
	float		rMax, dist;

	if ( !GetPoint(var, exception, center) )  return true;
	rMax = var->GivValFloat()*g_unit;
	var = var->GivNext();

	dist = script->m_main->RetFlatZoneRadius(center, rMax, pThis);
	result->SetValFloat(dist/g_unit);

	return true;
}


// Instruction "wait(t)".

bool rWait(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskWait(value);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "move(dist)".

bool rMove(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskAdvance(value*g_unit);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "turn(angle)".

bool rTurn(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskTurn(-value*PI/180.0f);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Compilation of the instruction "goto(pos, altitude, crash, goal)".

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

// Instruction "goto(pos, altitude, mode)".

bool rGoto(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*		script = ((CObject*)user)->RetRunScript();
	D3DVECTOR		pos;
	TaskGotoGoal	goal;
	TaskGotoCrash	crash;
	float			altitude;
	Error			err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		if ( !GetPoint(var, exception, pos) )  return true;

		goal  = TGG_DEFAULT;
		crash = TGC_DEFAULT;
		altitude = 0.0f*g_unit;

		if ( var != 0 )
		{
			altitude = var->GivValFloat()*g_unit;

			var = var->GivNext();
			if ( var != 0 )
			{
				goal = (TaskGotoGoal)var->GivValInt();

				var = var->GivNext();
				if ( var != 0 )
				{
					crash = (TaskGotoCrash)var->GivValInt();
				}
			}
		}

		err = script->m_primaryTask->StartTaskGoto(pos, altitude, goal, crash);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "find(type)".

bool rFind(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*		script = ((CObject*)user)->RetRunScript();
	D3DVECTOR		pos;
	TaskGotoGoal	goal;
	TaskGotoCrash	crash;
	float			altitude;
	Error			err;
	CObject*		pThis = (CObject*)user;
	CObject			*pObj, *pBest;
	CBotVar*		array;
	D3DVECTOR		iPos, oPos;
	float			best, minDist, maxDist, sens, iAngle, angle, focus, d, a;
	int				type, oType, i;
	bool			bArray;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		type    = OBJECT_NULL;
		angle   = 0.0f;
		focus   = PI*2.0f;
		minDist = 0.0f*g_unit;
		maxDist = 1000.0f*g_unit;
		sens    = 1.0f;

		if ( var->GivType() == CBotTypArrayPointer )
		{
			array = var->GivItemList();
			bArray = true;
		}
		else
		{
			type = var->GivValInt();
			bArray = false;
		}

		best = 100000.0f;
		pBest = 0;
		for ( i=0 ; i<1000000 ; i++ )
		{
			pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, i);
			if ( pObj == 0 )  break;
			if ( pObj == pThis )  continue;

			if ( pObj->RetTruck() != 0 )  continue;  // object transported?
			if ( !pObj->RetActif() )  continue;
			if ( pObj->RetProxyActivate() )  continue;

			oType = pObj->RetType();
			if ( oType == OBJECT_TOTO )  continue;

			if ( oType == OBJECT_RUINmobilew2 ||
				 oType == OBJECT_RUINmobilet1 ||
				 oType == OBJECT_RUINmobilet2 ||
				 oType == OBJECT_RUINmobiler1 ||
				 oType == OBJECT_RUINmobiler2 )
			{
				oType = OBJECT_RUINmobilew1;  // any ruin
			}

			if ( oType == OBJECT_SCRAP2 ||
				 oType == OBJECT_SCRAP3 ||
				 oType == OBJECT_SCRAP4 ||
				 oType == OBJECT_SCRAP5 )  // wastes?
			{
				oType = OBJECT_SCRAP1;  // any waste
			}

			if ( oType == OBJECT_BARRIER2 ||
				 oType == OBJECT_BARRIER3 )  // barriers?
			{
				oType = OBJECT_BARRIER1;  // any barrier
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
			if ( d < minDist || d > maxDist )  continue;  // too close or too far?

			if ( focus >= PI*2.0f )
			{
				if ( d < best )
				{
					best = d;
					pBest = pObj;
				}
				continue;
			}

			a = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
			if ( TestAngle(a, iAngle-focus/2.0f, iAngle+focus/2.0f) )
			{
				if ( d < best )
				{
					best = d;
					pBest = pObj;
				}
			}
		}

		if ( pBest == 0 )
		{
			exception = ERR_FIND_IMPOSSIBLE;
			return false;
		}

		pos = pBest->RetPosition(0);
		goal  = TGG_DEFAULT;
		crash = TGC_DEFAULT;
		altitude = 0.0f*g_unit;

		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		err = script->m_primaryTask->StartTaskGoto(pos, altitude, goal, crash);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Compilation "grab/drop(oper)".

CBotTypResult cGrabDrop(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Instruction "grab(oper)".

bool rGrab(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	ObjectType	oType;
	TaskManipArm type;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		if ( var == 0 )  type = TMA_FFRONT;
		else             type = (TaskManipArm)var->GivValInt();

		oType = pThis->RetType();
		if ( oType == OBJECT_HUMAN ||
			 oType == OBJECT_TECH  )
		{
			err = script->m_primaryTask->StartTaskTake();
		}
		else
		{
			err = script->m_primaryTask->StartTaskManip(TMO_GRAB, type);
		}

		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "drop(oper)".

bool rDrop(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	ObjectType	oType;
	TaskManipArm type;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		if ( var == 0 )  type = TMA_FFRONT;
		else             type = (TaskManipArm)var->GivValInt();

		oType = pThis->RetType();
		if ( oType == OBJECT_HUMAN ||
			 oType == OBJECT_TECH  )
		{
			err = script->m_primaryTask->StartTaskTake();
		}
		else
		{
			err = script->m_primaryTask->StartTaskManip(TMO_DROP, type);
		}

		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "sniff()".

bool rSniff(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		err = script->m_primaryTask->StartTaskSearch();
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Compilation of the instruction "receive(nom, power)".

CBotTypResult cReceive(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Instruction "receive(nom, power)".

bool rReceive(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CBotString	cbs;
	Error		err;
	const char*	p;
	float		value, power;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);

		cbs = var->GivValString();
		p = cbs;
		var = var->GivNext();

		power = 10.0f*g_unit;
		if ( var != 0 )
		{
			power = var->GivValFloat()*g_unit;
			var = var->GivNext();
		}

		err = script->m_primaryTask->StartTaskInfo((char*)p, 0.0f, power, false);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetInit(IS_NAN);
			return true;
		}
	}
	if ( !Process(script, result, exception) )  return false;  // not finished

	value = pThis->RetInfoReturn();
	if ( value == NAN )
	{
		result->SetInit(IS_NAN);
	}
	else
	{
		result->SetValFloat(value);
	}
	return true;
}

// Compilation of the instruction "send(nom, value, power)".

CBotTypResult cSend(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();
	
	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Instruction "send(nom, value, power)".

bool rSend(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CBotString	cbs;
	Error		err;
	const char*	p;
	float		value, power;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);

		cbs = var->GivValString();
		p = cbs;
		var = var->GivNext();

		value = var->GivValFloat();
		var = var->GivNext();

		power = 10.0f*g_unit;
		if ( var != 0 )
		{
			power = var->GivValFloat()*g_unit;
			var = var->GivNext();
		}

		err = script->m_primaryTask->StartTaskInfo((char*)p, value, power, true);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Seeks the nearest information terminal.

CObject* SearchInfo(CScript* script, CObject* object, float power)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	iPos, oPos;
	ObjectType	type;
	float		dist, min;
	int			i;

	iPos = object->RetPosition(0);

	min = 100000.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)script->m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type != OBJECT_INFO )  continue;

		if ( !pObj->RetActif() )  continue;

		oPos = pObj->RetPosition(0);
		dist = Length(oPos, iPos);
		if ( dist > power )  continue;  // too far?
		if ( dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}

	return pBest;
}

// Compilation of the instruction "deleteinfo(nom, power)".

CBotTypResult cDeleteInfo(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypFloat);
}

// Instruction "deleteinfo(nom, power)".

bool rDeleteInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CObject*	pInfo;
	CBotString	cbs;
	Info		info;
	const char*	p;
	float		power;
	int			i, total;

	exception = 0;

	cbs = var->GivValString();
	p = cbs;
	var = var->GivNext();

	power = 10.0f*g_unit;
	if ( var != 0 )
	{
		power = var->GivValFloat()*g_unit;
		var = var->GivNext();
	}

	pInfo = SearchInfo(script, pThis, power);
	if ( pInfo == 0 )
	{
		result->SetValFloat(0.0f);  // false
		return true;
	}

	total = pInfo->RetInfoTotal();
	for ( i=0 ; i<total ; i++ )
	{
		info = pInfo->RetInfo(i);
		if ( strcmp(info.name, p) == 0 )
		{
			pInfo->DeleteInfo(i);
			result->SetValFloat(1.0f);  // true
			return true;
		}
	}
	result->SetValFloat(0.0f);  // false
	return true;
}

// Compilation of the instruction "testinfo(nom, power)".

CBotTypResult cTestInfo(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
	if ( var->GivType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypBoolean);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
	return CBotTypResult(CBotTypBoolean);
}

// Instruction "testinfo(nom, power)".

bool rTestInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	CObject*	pInfo;
	CBotString	cbs;
	Info		info;
	const char*	p;
	float		power;
	int			i, total;

	exception = 0;

	cbs = var->GivValString();
	p = cbs;
	var = var->GivNext();

	power = 10.0f*g_unit;
	if ( var != 0 )
	{
		power = var->GivValFloat()*g_unit;
		var = var->GivNext();
	}

	pInfo = SearchInfo(script, pThis, power);
	if ( pInfo == 0 )
	{
		result->SetValInt(false);
		return true;
	}

	total = pInfo->RetInfoTotal();
	for ( i=0 ; i<total ; i++ )
	{
		info = pInfo->RetInfo(i);
		if ( strcmp(info.name, p) == 0 )
		{
			result->SetValInt(true);
			return true;
		}
	}
	result->SetValInt(false);
	return true;
}

// Instruction "thump()".

bool rThump(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		err = script->m_primaryTask->StartTaskTerraform();
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "recycle()".

bool rRecycle(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		err = script->m_primaryTask->StartTaskRecover();
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			if ( script->m_errMode == ERM_STOP )
			{
				exception = err;
				return false;
			}
			return true;
		}
	}
	return Process(script, result, exception);
}

// Compilation "shield(oper, radius)".

CBotTypResult cShield(CBotVar* &var, void* user)
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

// Instruction "shield(oper, radius)".

bool rShield(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		oper, radius;
	Error		err;

	oper = var->GivValFloat();  // 0=down, 1=up
	var = var->GivNext();

	radius = var->GivValFloat();
	if ( radius < 10.0f )  radius = 10.0f;
	if ( radius > 25.0f )  radius = 25.0f;
	radius = (radius-10.0f)/15.0f;

	if ( *script->m_secondaryTask == 0 )  // shield folds?
	{
		if ( oper == 0.0f )  // down?
		{
			result->SetValInt(1);  // shows the error
		}
		else	// up ?
		{
			pThis->SetParam(radius);

			*script->m_secondaryTask = new CTaskManager(script->m_iMan, script->m_object);
			err = (*script->m_secondaryTask)->StartTaskShield(TSM_UP, 1000.0f);
			if ( err != ERR_OK )
			{
				delete *script->m_secondaryTask;
				*script->m_secondaryTask = 0;
				result->SetValInt(err);  // shows the error
			}
		}
	}
	else	// shield deployed?
	{
		if ( oper == 0.0f )  // down?
		{
			(*script->m_secondaryTask)->StartTaskShield(TSM_DOWN, 0.0f);
		}
		else	// up?
		{
//?			result->SetValInt(1);  // shows the error
			pThis->SetParam(radius);
			(*script->m_secondaryTask)->StartTaskShield(TSM_UPDATE, 0.0f);
		}
	}

	return true;
}

// Compilation "fire(delay)".

CBotTypResult cFire(CBotVar* &var, void* user)
{
#if 0
	CObject*	pThis = (CObject*)user;
	ObjectType	type;

	type = pThis->RetType();

	if ( type == OBJECT_ANT )
	{
		return cOnePoint(var, user);
	}
	else if ( type == OBJECT_SPIDER )
	{
		return cNull(var, user);
	}
	else
	{
		if ( var == 0 )  return CBotTypResult(CBotTypFloat);
		if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
		var = var->GivNext();
		if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
		return CBotTypResult(CBotTypFloat);
	}
#else
	return CBotTypResult(CBotTypFloat);
#endif
}

// Instruction "fire(delay)".

bool rFire(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		delay;
	D3DVECTOR	impact;
	Error		err;
	ObjectType	type;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);

		type = pThis->RetType();

		if ( type == OBJECT_ANT )
		{
			if ( !GetPoint(var, exception, impact) )  return true;
			impact.y += pThis->RetWaterLevel();
			err = script->m_primaryTask->StartTaskFireAnt(impact);
		}
		else if ( type == OBJECT_SPIDER )
		{
			err = script->m_primaryTask->StartTaskSpiderExplo();
		}
		else
		{
			if ( var == 0 )  delay = 0.0f;
			else             delay = var->GivValFloat();
			err = script->m_primaryTask->StartTaskFire(delay);
		}

		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			return true;
		}
	}
	return Process(script, result, exception);
}

// Instruction "aim(dir)".

bool rAim(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;
	Error		err;

	exception = 0;

	if ( script->m_primaryTask == 0 )  // no task in progress?
	{
		script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
		value = var->GivValFloat();
		err = script->m_primaryTask->StartTaskGunGoal(value*PI/180.0f, 0.0f);
		if ( err != ERR_OK )
		{
			delete script->m_primaryTask;
			script->m_primaryTask = 0;
			result->SetValInt(err);  // shows the error
			return true;
		}
	}
	return Process(script, result, exception);
}

// Compilation of the instruction "motor(left, right)".

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

bool rMotor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CObject*	pThis = (CObject*)user;
	CPhysics*	physics = ((CObject*)user)->RetPhysics();
	float		left, right, speed, turn;

	left = var->GivValFloat();
	var = var->GivNext();
	right = var->GivValFloat();

	speed = (left+right)/2.0f;
	if ( speed < -1.0f )  speed = -1.0f;
	if ( speed >  1.0f )  speed =  1.0f;

	turn = left-right;
	if ( turn < -1.0f )  turn = -1.0f;
	if ( turn >  1.0f )  turn =  1.0f;
	
	if ( pThis->RetFixed() )  // ant on the back?
	{
		speed = 0.0f;
		turn  = 0.0f;
	}

	physics->SetMotorSpeedX(speed);  // forward/backward
	physics->SetMotorSpeedZ(turn);  // turns

	return true;
}

// Instruction "jet(power)".

bool rJet(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CPhysics*	physics = ((CObject*)user)->RetPhysics();
	float		value;

	value = var->GivValFloat();
	physics->SetMotorSpeedY(value);

	return true;
}

// Compilation of the instruction "topo(pos)".

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

bool rTopo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	D3DVECTOR	pos;
	float		level;

	exception = 0;

	if ( !GetPoint(var, exception, pos) )  return true;

	level = script->m_terrain->RetFloorLevel(pos);
	level -= script->m_water->RetLevel();
	result->SetValFloat(level/g_unit);
	return true;
}

// Compilation of the instruction "message(string, type)".

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

bool rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
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

	script->m_displayText->DisplayText((char*)p, script->m_object, 10.0f, type);
	script->m_main->CheckEndMessage((char*)p);

	return true;
}

// Instruction "cmdline(rank)".

bool rCmdline(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	float		value;
	int			rank;

	rank = var->GivValInt();
	value = pThis->RetCmdLine(rank);
	result->SetValFloat(value);

	return true;
}

// Instruction "ismovie()".

bool rIsMovie(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;

	value = script->m_main->RetMovieLock()?1.0f:0.0f;
	result->SetValFloat(value);

	return true;
}

// Instruction "errmode(mode)".

bool rErrMode(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	int			value;

	value = var->GivValInt();
	if ( value < 0 )  value = 0;
	if ( value > 1 )  value = 1;
	script->m_errMode = value;

	return true;
}

// Instruction "ipf(num)".

bool rIPF(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	int			value;

	value = var->GivValInt();
	if ( value <     1 )  value =     1;
	if ( value > 10000 )  value = 10000;
	script->m_ipf = value;

	return true;
}

// Instruction "abstime()".

bool rAbsTime(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	float		value;

	value = script->m_main->RetGameTime();
	result->SetValFloat(value);
	return true;
}


// Prepares a file name.

void PrepareFilename(CBotString &filename, char *dir)
{
	int			pos;

	pos = filename.ReverseFind('\\');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // removes folders
	}

	pos = filename.ReverseFind('/');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // also those with /
	}

	pos = filename.ReverseFind(':');
	if ( pos > 0 )
	{
		filename = filename.Mid(pos+1);  // also removes the drive letter C:
	}

	filename = CBotString(dir) + CBotString("\\") + filename;
}

// Instruction "deletefile(filename)".

bool rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user)
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

	return true;
}

// Compilation of the instruction "pendown(color, width)".

CBotTypResult cPenDown(CBotVar* &var, void* user)
{
	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	if ( var->GivType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
	var = var->GivNext();

	if ( var == 0 )  return CBotTypResult(CBotTypFloat);
	return CBotTypResult(CBotErrOverParam);
}

// Instruction "pendown(color, width)".

bool rPenDown(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	int			color;
	float		width;
	Error		err;

	if ( pThis->RetType() == OBJECT_MOBILEdr )
	{
		exception = 0;

		if ( script->m_primaryTask == 0 )  // no task in progress?
		{
			if ( var != 0 )
			{
				color = var->GivValInt();
				if ( color <  0 )  color =  0;
				if ( color > 17 )  color = 17;
				pThis->SetTraceColor(color);

				var = var->GivNext();
				if ( var != 0 )
				{
					width = var->GivValFloat();
					if ( width < 0.1f )  width = 0.1f;
					if ( width > 1.0f )  width = 1.0f;
					pThis->SetTraceWidth(width);
				}
			}
			pThis->SetTraceDown(true);

			script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
			err = script->m_primaryTask->StartTaskPen(pThis->RetTraceDown(), pThis->RetTraceColor());
			if ( err != ERR_OK )
			{
				delete script->m_primaryTask;
				script->m_primaryTask = 0;
				result->SetValInt(err);  // shows the error
				if ( script->m_errMode == ERM_STOP )
				{
					exception = err;
					return false;
				}
				return true;
			}
		}
		return Process(script, result, exception);
	}
	else
	{
		if ( var != 0 )
		{
			color = var->GivValInt();
			if ( color <  0 )  color =  0;
			if ( color > 17 )  color = 17;
			pThis->SetTraceColor(color);

			var = var->GivNext();
			if ( var != 0 )
			{
				width = var->GivValFloat();
				if ( width < 0.1f )  width = 0.1f;
				if ( width > 1.0f )  width = 1.0f;
				pThis->SetTraceWidth(width);
			}
		}
		pThis->SetTraceDown(true);

		return true;
	}
}

// Instruction "penup()".

bool rPenUp(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CObject*	pThis = (CObject*)user;
	Error		err;

	if ( pThis->RetType() == OBJECT_MOBILEdr )
	{
		exception = 0;

		if ( script->m_primaryTask == 0 )  // no task in progress?
		{
			pThis->SetTraceDown(false);

			script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
			err = script->m_primaryTask->StartTaskPen(pThis->RetTraceDown(), pThis->RetTraceColor());
			if ( err != ERR_OK )
			{
				delete script->m_primaryTask;
				script->m_primaryTask = 0;
				result->SetValInt(err);  // shows the error
				if ( script->m_errMode == ERM_STOP )
				{
					exception = err;
					return false;
				}
				return true;
			}
		}
		return Process(script, result, exception);
	}
	else
	{
		pThis->SetTraceDown(false);
		return true;
	}
}

// Instruction "pencolor()".

bool rPenColor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CScript*	script = ((CObject*)user)->RetRunScript();
	CPhysics*	physics = ((CObject*)user)->RetPhysics();
	CObject*	pThis = (CObject*)user;
	int			color;
	Error		err;

	if ( pThis->RetType() == OBJECT_MOBILEdr )
	{
		exception = 0;

		if ( script->m_primaryTask == 0 )  // no task in progress?
		{
			color = var->GivValInt();
			if ( color <  0 )  color =  0;
			if ( color > 17 )  color = 17;
			pThis->SetTraceColor(color);

			script->m_primaryTask = new CTaskManager(script->m_iMan, script->m_object);
			err = script->m_primaryTask->StartTaskPen(pThis->RetTraceDown(), pThis->RetTraceColor());
			if ( err != ERR_OK )
			{
				delete script->m_primaryTask;
				script->m_primaryTask = 0;
				result->SetValInt(err);  // shows the error
				if ( script->m_errMode == ERM_STOP )
				{
					exception = err;
					return false;
				}
				return true;
			}
		}
		return Process(script, result, exception);
	}
	else
	{
		color = var->GivValInt();
		if ( color <  0 )  color =  0;
		if ( color > 17 )  color = 17;
		pThis->SetTraceColor(color);

		return true;
	}
}

// Instruction "penwidth()".

bool rPenWidth(CBotVar* var, CBotVar* result, int& exception, void* user)
{
	CObject*	pThis = (CObject*)user;
	float		width;

	width = var->GivValFloat();
	if ( width < 0.1f )  width = 0.1f;
	if ( width > 1.0f )  width = 1.0f;
	pThis->SetTraceWidth(width);
	return true;
}



// Object's constructor.

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
	m_bRun = false;
	m_bStepMode = false;
	m_bCompile = false;
	m_title[0] = 0;
	m_cursor1 = 0;
	m_cursor2 = 0;
	m_filename[0] = 0;
}

// Initializes all functions for module CBOT.

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
	CBotProgram::AddFunction("search",    rSearch,    cSearch);
	CBotProgram::AddFunction("radar",     rRadar,     cRadar);
	CBotProgram::AddFunction("detect",    rDetect,    cDetect);
	CBotProgram::AddFunction("direction", rDirection, cDirection);
	CBotProgram::AddFunction("produce",   rProduce,   cProduce);
	CBotProgram::AddFunction("distance",  rDistance,  cDistance);
	CBotProgram::AddFunction("distance2d",rDistance2d,cDistance);
	CBotProgram::AddFunction("space",     rSpace,     cSpace);
	CBotProgram::AddFunction("flatground",rFlatGround,cFlatGround);
	CBotProgram::AddFunction("wait",      rWait,      cOneFloat);
	CBotProgram::AddFunction("move",      rMove,      cOneFloat);
	CBotProgram::AddFunction("turn",      rTurn,      cOneFloat);
	CBotProgram::AddFunction("goto",      rGoto,      cGoto);
	CBotProgram::AddFunction("find",      rFind,      cOneFloat);
	CBotProgram::AddFunction("grab",      rGrab,      cGrabDrop);
	CBotProgram::AddFunction("drop",      rDrop,      cGrabDrop);
	CBotProgram::AddFunction("sniff",     rSniff,     cNull);
	CBotProgram::AddFunction("receive",   rReceive,   cReceive);
	CBotProgram::AddFunction("send",      rSend,      cSend);
	CBotProgram::AddFunction("deleteinfo",rDeleteInfo,cDeleteInfo);
	CBotProgram::AddFunction("testinfo",  rTestInfo,  cTestInfo);
	CBotProgram::AddFunction("thump",     rThump,     cNull);
	CBotProgram::AddFunction("recycle",   rRecycle,   cNull);
	CBotProgram::AddFunction("shield",    rShield,    cShield);
	CBotProgram::AddFunction("fire",      rFire,      cFire);
	CBotProgram::AddFunction("aim",       rAim,       cOneFloat);
	CBotProgram::AddFunction("motor",     rMotor,     cMotor);
	CBotProgram::AddFunction("jet",       rJet,       cOneFloat);
	CBotProgram::AddFunction("topo",      rTopo,      cTopo);
	CBotProgram::AddFunction("message",   rMessage,   cMessage);
	CBotProgram::AddFunction("cmdline",   rCmdline,   cOneFloat);
	CBotProgram::AddFunction("ismovie",   rIsMovie,   cNull);
	CBotProgram::AddFunction("errmode",   rErrMode,   cOneFloat);
	CBotProgram::AddFunction("ipf",       rIPF,       cOneFloat);
	CBotProgram::AddFunction("abstime",   rAbsTime,   cNull);
	CBotProgram::AddFunction("deletefile",rDeleteFile,cString);
	CBotProgram::AddFunction("pendown",   rPenDown,   cPenDown);
	CBotProgram::AddFunction("penup",     rPenUp,     cNull);
	CBotProgram::AddFunction("pencolor",  rPenColor,  cOneFloat);
	CBotProgram::AddFunction("penwidth",  rPenWidth,  cOneFloat);
}

// Object's destructor.

CScript::~CScript()
{
	delete m_botProg;
	delete m_primaryTask;
	delete m_script;
	m_script = 0;
	m_len = 0;

	m_iMan->DeleteInstance(CLASS_SCRIPT, this);
}


// Gives the script editable block of text.

void CScript::PutScript(CEdit* edit, char* name)
{
	if ( m_script == 0 )
	{
		New(edit, name);
	}
	else
	{
		edit->SetText(m_script);
		edit->SetCursor(m_cursor2, m_cursor1);
		edit->ShowSelect();
	}
	edit->SetFocus(true);
}

// The script takes a paved text.

bool CScript::GetScript(CEdit* edit)
{
	int		len;

	delete m_script;
	m_script = 0;

	len = edit->RetTextLength();
	m_script = (char*)malloc(sizeof(char)*(len+1));

	edit->GetText(m_script, len+1);
	edit->GetCursor(m_cursor2, m_cursor1);
	m_len = strlen(m_script);

	if ( !CheckToken() )
	{
		edit->SetCursor(m_cursor2, m_cursor1);
		edit->ShowSelect();
		edit->SetFocus(true);
		return false;
	}

	if ( !Compile() )
	{
		edit->SetCursor(m_cursor2, m_cursor1);
		edit->ShowSelect();
		edit->SetFocus(true);
		return false;
	}

	return true;
}

// Indicates whether a program is compiled correctly.

bool CScript::RetCompile()
{
	return m_bCompile;
}

// Indicates whether the program is empty.

bool CScript::IsEmpty()
{
	int		i;

	for ( i=0 ; i<m_len ; i++ )
	{
		if ( m_script[i] != ' '  &&
			 m_script[i] != '\n' )  return false;
	}
	return true;
}

// Checks if a program does not contain the prohibited instructions
// and if it contains well at least once every mandatory instructions.

bool CScript::CheckToken()
{
	CBotToken*	bt;
	CBotString	bs;
	const char*	token;
	int			error, type, cursor1, cursor2, i;
	char		used[100];

	if ( !m_object->RetCheckToken() )  return true;

	m_error = 0;
	m_title[0] = 0;
	m_token[0] = 0;
	m_bCompile = false;

	for ( i=0 ; i<m_main->RetObligatoryToken() ; i++ )
	{
		used[i] = 0;  // token not used
	}

	bt = CBotToken::CompileTokens(m_script, error);
	while ( bt != 0 )
	{
		bs = bt->GivString();
		token = bs;
		type = bt->GivType();

		cursor1 = bt->GivStart();
		cursor2 = bt->GivEnd();

		i = m_main->IsObligatoryToken((char*)token);
		if ( i != -1 )
		{
			used[i] = 1;  // token used
		}

		if ( !m_main->IsProhibitedToken((char*)token) )
		{
			m_error = ERR_PROHIBITEDTOKEN;
			m_cursor1 = cursor1;
			m_cursor2 = cursor2;
			strcpy(m_title, "<erreur>");
			CBotToken::Delete(bt);
			return false;
		}

		bt = bt->GivNext();
	}

	// At least once every obligatory instruction?
	for ( i=0 ; i<m_main->RetObligatoryToken() ; i++ )
	{
		if ( used[i] == 0 )  // token not used?
		{
			strcpy(m_token, m_main->RetObligatoryToken(i));
			m_error = ERR_OBLIGATORYTOKEN;
			strcpy(m_title, "<erreur>");
			CBotToken::Delete(bt);
			return false;
		}
	}

	CBotToken::Delete(bt);
	return true;
}

// Compile the script of a paved text.

bool CScript::Compile()
{
	CBotStringArray	liste;
	int				i;
	const char*		p;

	m_error = 0;
	m_cursor1 = 0;
	m_cursor2 = 0;
	m_title[0] = 0;
	m_bCompile = false;

	if ( IsEmpty() )  // program exist?
	{
		delete m_botProg;
		m_botProg = 0;
		return true;
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
			while ( true )
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
		m_bCompile = true;
		return true;
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
		return false;
	}
}


// Returns the title of the script.

void CScript::GetTitle(char* buffer)
{
	strcpy(buffer, m_title);
}


// Choice of mode of execution.

void CScript::SetStepMode(bool bStep)
{
	m_bStepMode = bStep;
}


// Runs the program from the beginning.

bool CScript::Run()
{
	if( m_botProg == 0 )  return false;
	if ( m_script == 0 || m_len == 0 )  return false;

	if ( !m_botProg->Start(m_title) )  return false;

	m_object->SetRunScript(this);
	m_bRun = true;
	m_bContinue = false;
	m_ipf = CBOT_IPF;
	m_errMode = ERM_STOP;

	if ( m_bStepMode )  // step by step mode?
	{
		Event	newEvent;
		ZeroMemory(&newEvent, sizeof(Event));
		Step(newEvent);
	}

	return true;
}

// Continues the execution of current program.
// Returns true when execution is finished.

bool CScript::Continue(const Event &event)
{
	if( m_botProg == 0 )  return true;
	if ( !m_bRun )  return true;

	m_event = event;

	if ( m_bStepMode )  // step by step mode?
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
				m_bRun = false;

				if ( m_error != 0 && m_errMode == ERM_STOP )
				{
					char	s[100];
					GetError(s);
					m_displayText->DisplayText(s, m_object, 10.0f, TT_ERROR);
				}
				m_engine->SetPause(true);  // gives pause
				return true;
			}
			if ( !m_bContinue )
			{
				m_engine->SetPause(true);  // gives pause
			}
		}

		return false;
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
		m_bRun = false;

		if ( m_error != 0 && m_errMode == ERM_STOP )
		{
			char	s[100];
			GetError(s);
			m_displayText->DisplayText(s, m_object, 10.0f, TT_ERROR);
		}
		return true;
	}

	return false;
}

// Continues the execution of current program.
// Returns true when execution is finished.

bool CScript::Step(const Event &event)
{
	if( m_botProg == 0 )  return true;
	if ( !m_bRun )  return true;
	if ( !m_bStepMode )  return false;

	m_engine->SetPause(false);
	m_engine->StepSimul(0.01f);  // advance of 10ms
	m_engine->SetPause(true);

	m_event = event;

	if ( m_botProg->Run(m_object, 0) )  // step mode
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
		m_bRun = false;

		if ( m_error != 0 && m_errMode == ERM_STOP )
		{
			char	s[100];
			GetError(s);
			m_displayText->DisplayText(s, m_object, 10.0f, TT_ERROR);
		}
		return true;
	}

	if ( m_bContinue )  // instuction "move", "goto", etc. ?
	{
		m_engine->SetPause(false);  // removes the pause
	}
	return false;
}

// Stops the program.

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

	m_bRun = false;
}

// Indicates whether the program runs.

bool CScript::IsRunning()
{
	return m_bRun;
}

// Indicates whether the program continues a step.

bool CScript::IsContinue()
{
	return m_bContinue;
}


// Gives the position of the cursor during the execution.

bool CScript::GetCursor(int &cursor1, int &cursor2)
{
	const char*	funcName;

	cursor1 = cursor2 = 0;

	if( m_botProg == 0 )  return false;
	if ( !m_bRun )  return false;

	m_botProg->GetRunPos(funcName, cursor1, cursor2);
	if ( cursor1 < 0 || cursor1 > m_len ||
		 cursor2 < 0 || cursor2 > m_len )
	{
		cursor1 = 0;
		cursor2 = 0;
	}
	return true;
}


// Put of the variables in a list.

void PutList(char *baseName, bool bArray, CBotVar *var, CList *list, int &rankList)
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
		var->Maj(NULL, false);
		pStatic = var->GivStaticVar();  // finds the static element

		bs = pStatic->GivName();  // variable name
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
			PutList(varName, true, svar, list, rankList);
		}
		else if ( type == CBotTypClass   ||
				  type == CBotTypPointer )
		{
			svar = pStatic->GivItemList();
			PutList(varName, false, svar, list, rankList);
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

// Fills a list with variables.

void CScript::UpdateList(CList* list)
{
	CBotVar		*var;
	const char	*progName, *funcName;
	int			total, select, level, cursor1, cursor2, rank;

	if( m_botProg == 0 )  return;

	total  = list->RetTotal();
	select = list->RetSelect();

	list->Flush();  // empty list
	m_botProg->GetRunPos(progName, cursor1, cursor2);
	if ( progName == 0 )  return;

	level = 0;
	rank  = 0;
	while ( true )
	{
		var = m_botProg->GivStackVars(funcName, level--);
		if ( funcName != progName )  break;

		PutList("", false, var, list, rank);
	}

	if ( total == list->RetTotal() )  // same total?
	{
		list->SetSelect(select);
	}

	list->SetTooltip("");
	list->SetState(STATE_ENABLE);
}


// Colorize the text according to syntax.

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


// Seeks a token at random in a script.
// Returns the index of the start of the token found, or -1.

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

// Removes a token in a script.

void DeleteToken(char* script, int pos, int len)
{
	while ( true )
	{
		script[pos] = script[pos+len];
		if ( script[pos++] == 0 )  break;
	}
}

// Inserts a token in a script.

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

// Introduces a virus into a program.

bool CScript::IntroduceVirus()
{
	int		i, start, iFound;
	int		found[11*2];
	char*	newScript;

	char*	names[11*2] =
	{
		"==",			"!=",
		"!=",			"==",
		">",			"<",
		"<",			">",
		"true",			"false",
		"false",		"true",
		"grab",			"drop",
		"drop",			"grab",
		"InFront",		"Behind",
		"Behind",		"EnergyCell",
		"EnergyCell",	"InFront",
	};

	iFound = 0;
	for ( i=0 ; i<11 ; i++ )
	{
		start = SearchToken(m_script, names[i*2]);
		if ( start != -1 )
		{
			found[iFound++] = i*2;
			found[iFound++] = start;
		}
	}
	if ( iFound == 0 )  return false;

	i = (rand()%(iFound/2))*2;
	start = found[i+1];
	i     = found[i+0];

	newScript = (char*)malloc(sizeof(char)*(m_len+strlen(names[i+1])+1));
	strcpy(newScript, m_script);
	delete m_script;
	m_script = newScript;

	DeleteToken(m_script, start, strlen(names[i]));
	InsertToken(m_script, start, names[i+1]);
	m_len = strlen(m_script);
	Compile();  // recompile with the virus

	return true;
}


// Returns the number of the error.

int CScript::RetError()
{
	return m_error;
}

// Returns the text of the error.

void CScript::GetError(char* buffer)
{
	if ( m_error == 0 )
	{
		buffer[0] = 0;
	}
	else
	{
		if ( m_error == ERR_OBLIGATORYTOKEN )
		{
			char s[100];
			GetResource(RES_ERR, m_error, s);
			sprintf(buffer, s, m_token);
		}
		else if ( m_error < 1000 )
		{
			GetResource(RES_ERR, m_error, buffer);
		}
		else
		{
			GetResource(RES_CBOT, m_error, buffer);
		}
	}
}


// New program.

void CScript::New(CEdit* edit, char* name)
{
	FILE	*file = NULL;
	char	res[100];
	char	text[100];
	char	filename[100];
	char	script[500];
	char	buffer[500];
	char	*sf;
	int		cursor1, cursor2, len, i, j;

	GetResource(RES_TEXT, RT_SCRIPT_NEW, res);
	if ( name[0] == 0 )  strcpy(text, res);
	else                 strcpy(text, name);

	sprintf(script, "extern void object::%s()\n{\n\t\n\t\n\t\n}\n", text);
	edit->SetText(script, false);

	if ( strcmp(text, res) == 0 )
	{
		cursor1 = 20;
		cursor2 = 20+strlen(text);  // update "New"
	}
	else
	{
		if ( edit->RetAutoIndent() )
		{
			cursor1 = 20+strlen(text)+6;
			cursor2 = cursor1;  // cursor in { }
		}
		else
		{
			cursor1 = 20+strlen(text)+8;
			cursor2 = cursor1;  // cursor in { }
		}
	}

	edit->SetCursor(cursor2, cursor1);
	edit->ShowSelect();
	edit->SetFocus(true);
	
	sf = m_main->RetScriptFile();
	if ( sf[0] != 0 )  // Load an empty program specific?
	{
		strcpy(filename, "script\\");
		strcat(filename, sf);
		file = fopen(filename, "rb");
		if ( file != NULL )
		{
			fseek(file, 0, SEEK_END);
			len = ftell(file);
			fseek(file, 0, SEEK_SET);

			if ( len > 500-1 )  len = 500-1;
			fread(buffer, 1, len, file);
			buffer[len] = 0;
			fclose(file);

			cursor1 = 0;
			i = 0;
			j = 0;
			while ( true )
			{
				if ( buffer[i] == 0 )  break;

				if ( buffer[i] == '\r' )
				{
					i ++;
					continue;
				}

				if ( buffer[i] == '\t' && edit->RetAutoIndent() )
				{
					i ++;
					continue;
				}

				if ( buffer[i+0] == '%' &&
					 buffer[i+1] == 's' )
				{
					strcpy(script+j, text);
					j += strlen(text);
					i += 2;
					continue;
				}

				if ( buffer[i] == '#' )
				{
					cursor1 = j;
					i ++;
					continue;
				}

				script[j++] = buffer[i++];
			}
			script[j] = 0;
			edit->SetText(script, false);

			cursor2 = cursor1;
			edit->SetCursor(cursor2, cursor1);
			edit->ShowSelect();
			edit->SetFocus(true);
		}
	}

	ColorizeScript(edit);
}


// Provided a script for all parts.

bool CScript::SendScript(char* text)
{
	m_len = strlen(text);
	m_script = (char*)malloc(sizeof(char)*(m_len+1));
	strcpy(m_script, text);
	if ( !CheckToken() )  return false;
	if ( !Compile() )  return false;

	return true;
}

// Reads a script as a text file.

bool CScript::ReadScript(char* filename)
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
	if ( file == NULL )  return false;
	fclose(file);

	delete m_script;
	m_script = 0;

	edit = m_interface->CreateEdit(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), 0, EVENT_EDIT9);
	edit->SetMaxChar(EDITSTUDIOMAX);
	edit->SetAutoIndent(m_engine->RetEditIndentMode());
	edit->ReadText(name);
	GetScript(edit);
	m_interface->DeleteControl(EVENT_EDIT9);
	return true;
}

// Writes a script as a text file.

bool CScript::WriteScript(char* filename)
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
		return false;
	}

	edit = m_interface->CreateEdit(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), 0, EVENT_EDIT9);
	edit->SetMaxChar(EDITSTUDIOMAX);
	edit->SetAutoIndent(m_engine->RetEditIndentMode());
	edit->SetText(m_script);
	edit->WriteText(name);
	m_interface->DeleteControl(EVENT_EDIT9);
	return true;
}


// Reads a stack of script by execution as a file.

bool CScript::ReadStack(FILE *file)
{
	int		nb;

	fRead(&nb, sizeof(int), 1, file);
	fRead(&m_ipf, sizeof(int), 1, file);
	fRead(&m_errMode, sizeof(int), 1, file);

	if ( m_botProg == 0 )  return false;
	if ( !m_botProg->RestoreState(file) )  return false;
	
	m_object->SetRunScript(this);
	m_bRun = true;
	m_bContinue = false;
	return true;
}

// Writes a stack of script by execution as a file.

bool CScript::WriteStack(FILE *file)
{
	int		nb;

	nb = 2;
	fWrite(&nb, sizeof(int), 1, file);
	fWrite(&m_ipf, sizeof(int), 1, file);
	fWrite(&m_errMode, sizeof(int), 1, file);

	return m_botProg->SaveState(file);
}


// Compares two scripts.

bool CScript::Compare(CScript* other)
{
	if ( m_len != other->m_len )  return false;

	return ( strcmp(m_script, other->m_script) == 0 );
}


// Management of the file name when the script is saved.

void CScript::SetFilename(char *filename)
{
	strcpy(m_filename, filename);
}

char* CScript::RetFilename()
{
	return m_filename;
}

