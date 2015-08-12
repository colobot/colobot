/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#include "script/scriptfunc.h"

#include "common/config.h"

#include "app/app.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "math/all.h"

#include "object/object.h"
#include "object/object_manager.h"
#include "object/robotmain.h"
#include "object/trace_color.h"

#include "object/auto/auto.h"
#include "object/auto/autobase.h"
#include "object/auto/autofactory.h"

#include "object/interface/programmable_object.h"
#include "object/interface/task_executor_object.h"

#include "object/level/parser.h"

#include "object/motion/motionvehicle.h"

#include "object/subclass/exchange_post.h"

#include "object/task/taskmanager.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/script.h"

#include "sound/sound.h"

#include "ui/displaytext.h"


#if PLATFORM_WINDOWS
    #include "app/system_windows.h"
#endif


// Compiling a procedure without any parameters.

CBotTypResult CScriptFunctions::cNull(CBotVar* &var, void* user)
{
    if ( var != 0 )  return CBotErrOverParam;
    return CBotTypResult(CBotTypFloat);
}

CBotTypResult CScriptFunctions::cClassNull(CBotVar* thisclass, CBotVar* &var)
{
    return CScriptFunctions::cNull(var, nullptr);
}

// Compiling a procedure with a single real number.

CBotTypResult CScriptFunctions::cOneFloat(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

CBotTypResult CScriptFunctions::cClassOneFloat(CBotVar* thisclass, CBotVar* &var)
{
    return CScriptFunctions::cOneFloat(var, nullptr);
}

// Compiling a procedure with two real numbers.

CBotTypResult CScriptFunctions::cTwoFloat(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a "dot".

CBotTypResult CScriptFunctions::cPoint(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);

    if ( var->GetType() <= CBotTypDouble )
    {
        var = var->GetNext();
        if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
        if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();
        //?     if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
        //?     if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
        //?     var = var->GetNext();
        return CBotTypResult(0);
    }

    if ( var->GetType() == CBotTypClass )
    {
        if ( !var->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadParam);
        var = var->GetNext();
        return CBotTypResult(0);
    }

    return CBotTypResult(CBotErrBadParam);
}

// Compiling a procedure with a single "point".

CBotTypResult CScriptFunctions::cOnePoint(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single string.

CBotTypResult CScriptFunctions::cString(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
        var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single string, returning string.

CBotTypResult CScriptFunctions::cStringString(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
        var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypString);
}

// compilation of instruction with one int returning int

CBotTypResult CScriptFunctions::cOneInt(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypInt);
}

// compilation of instruction with one int returning boolean

CBotTypResult CScriptFunctions::cOneIntReturnBool(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}


// Seeking value in an array of integers.

bool FindList(CBotVar* array, int type)
{
    while ( array != 0 )
    {
        if ( type == array->GetValInt() )  return true;
        array = array->GetNext();
    }
    return false;
}


// Gives a parameter of type "point".

bool GetPoint(CBotVar* &var, int& exception, Math::Vector& pos)
{
    CBotVar     *pX, *pY, *pZ;

    if ( var->GetType() <= CBotTypDouble )
    {
        pos.x = var->GetValFloat()*g_unit;
        var = var->GetNext();

        pos.z = var->GetValFloat()*g_unit;
        var = var->GetNext();

        pos.y = 0.0f;
    }
    else
    {
        pX = var->GetItem("x");
        if ( pX == NULL )
        {
            exception = CBotErrUndefItem;  return true;
        }
        pos.x = pX->GetValFloat()*g_unit;

        pY = var->GetItem("y");
        if ( pY == NULL )
        {
            exception = CBotErrUndefItem;  return true;
        }
        pos.z = pY->GetValFloat()*g_unit;  // attention y -> z !

        pZ = var->GetItem("z");
        if ( pZ == NULL )
        {
            exception = CBotErrUndefItem;  return true;
        }
        pos.y = pZ->GetValFloat()*g_unit;  // attention z -> y !

        var = var->GetNext();
    }
    return true;
}


// Instruction "sin(degrees)".

bool CScriptFunctions::rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sinf(value*Math::PI/180.0f));
    return true;
}

// Instruction "cos(degrees)".

bool CScriptFunctions::rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(cosf(value*Math::PI/180.0f));
    return true;
}

// Instruction "tan(degrees)".

bool CScriptFunctions::rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(tanf(value*Math::PI/180.0f));
    return true;
}

// Instruction "asin(degrees)".

bool raSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(asinf(value)*180.0f/Math::PI);
    return true;
}

// Instruction "acos(degrees)".

bool raCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(acosf(value)*180.0f/Math::PI);
    return true;
}

// Instruction "atan(degrees)".

bool raTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(atanf(value)*180.0f/Math::PI);
    return true;
}

// Instruction "atan2(y,x)".

bool raTan2(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float y = var->GetValFloat();
    var = var->GetNext();
    float x = var->GetValFloat();

    result->SetValFloat(atan2(y, x) * 180.0f / Math::PI);
    return true;
}

// Instruction "sqrt(value)".

bool CScriptFunctions::rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sqrtf(value));
    return true;
}

// Instruction "pow(x, y)".

bool CScriptFunctions::rPow(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   x, y;

    x = var->GetValFloat();
    var = var->GetNext();
    y = var->GetValFloat();
    result->SetValFloat(powf(x, y));
    return true;
}

// Instruction "rand()".

bool CScriptFunctions::rRand(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValFloat(Math::Rand());
    return true;
}

// Instruction "abs()".

bool CScriptFunctions::rAbs(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(fabs(value));
    return true;
}

// Instruction "floor()"

bool CScriptFunctions::rFloor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(floor(value));
    return true;
}

// Instruction "ceil()"

bool CScriptFunctions::rCeil(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(ceil(value));
    return true;
}

// Instruction "round()"

bool CScriptFunctions::rRound(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(round(value));
    return true;
}

// Instruction "trunc()"

bool CScriptFunctions::rTrunc(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(trunc(value));
    return true;
}

// Compilation of the instruction "endmission(result, delay)"

CBotTypResult CScriptFunctions::cEndMission(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "endmission(result, delay)"

bool CScriptFunctions::rEndMission(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    Error ended;
    float delay;

    ended = static_cast<Error>(var->GetValInt());
    var = var->GetNext();

    delay = var->GetValFloat();

    CRobotMain::GetInstancePointer()->SetEndMission(ended, delay);
    return true;
}

// Compilation of the instruction "playmusic(filename, repeat)"

CBotTypResult CScriptFunctions::cPlayMusic(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "playmusic(filename, repeat)"

bool CScriptFunctions::rPlayMusic(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    std::string filename;
    CBotString cbs;
    bool repeat;

    cbs = var->GetValString();
    filename = std::string(cbs);
    var = var->GetNext();

    repeat = var->GetValInt();

    CApplication::GetInstancePointer()->GetSound()->StopMusic();
    CApplication::GetInstancePointer()->GetSound()->PlayMusic(filename, repeat);
    return true;
}

// Instruction "stopmusic()"

bool CScriptFunctions::rStopMusic(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CApplication::GetInstancePointer()->GetSound()->StopMusic();
    return true;
}

// Instruction "getbuild()"

bool CScriptFunctions::rGetBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(CRobotMain::GetInstancePointer()->GetEnableBuild());
    return true;
}

// Instruction "getresearchenable()"

bool CScriptFunctions::rGetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(CRobotMain::GetInstancePointer()->GetEnableResearch());
    return true;
}

// Instruction "getresearchdone()"

bool CScriptFunctions::rGetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;
    result->SetValInt(CRobotMain::GetInstancePointer()->GetDoneResearch(pThis->GetTeam()));
    return true;
}

// Instruction "setbuild()"

bool CScriptFunctions::rSetBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CRobotMain::GetInstancePointer()->SetEnableBuild(var->GetValInt());
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Instruction "setresearchenable()"

bool CScriptFunctions::rSetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CRobotMain::GetInstancePointer()->SetEnableResearch(var->GetValInt());
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Instruction "setresearchdone()"

bool CScriptFunctions::rSetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;
    CRobotMain::GetInstancePointer()->SetDoneResearch(var->GetValInt(), pThis->GetTeam());
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Compilation of the instruction "retobject(rank)".

CBotTypResult CScriptFunctions::cGetObject(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "retobjectbyid(rank)".

bool CScriptFunctions::rGetObjectById(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pObj;
    int         rank;

    rank = var->GetValInt();

    pObj = static_cast<CObject*>(CObjectManager::GetInstancePointer()->GetObjectById(rank));
    if ( pObj == 0 )
    {
        result->SetPointer(0);
    }
    else
    {
        result->SetPointer(pObj->GetBotVar());
    }

    return true;
}

// Instruction "retobject(rank)".

bool CScriptFunctions::rGetObject(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pObj;
    int         rank;

    rank = var->GetValInt();

    pObj = CObjectManager::GetInstancePointer()->GetObjectByRank(rank);
    if ( pObj == nullptr )
    {
        result->SetPointer(0);
    }
    else
    {
        result->SetPointer(pObj->GetBotVar());
    }
    return true;
}

// Compilation of instruction "object.busy()"
CBotTypResult CScriptFunctions::cBusy(CBotVar* thisclass, CBotVar* &var)
{
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "object.busy()"

bool CScriptFunctions::rBusy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;

    exception = 0;

    CObject* obj = static_cast<CObject*>(thisclass->GetUserPtr());
    CAuto* automat = obj->GetAuto();

    if ( pThis->GetTeam() != obj->GetTeam() && obj->GetTeam() != 0 )
    {
        exception = ERR_ENEMY_OBJECT;
        result->SetValInt(ERR_ENEMY_OBJECT);
        return false;
    }

    if ( automat != nullptr )
        result->SetValInt(automat->GetBusy());
    else
        exception = ERR_WRONG_OBJ;

    return true;
}

bool CScriptFunctions::rDestroy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    exception = 0;
    Error err;

    CObject* obj = static_cast<CObject*>(thisclass->GetUserPtr());
    CAuto* automat = obj->GetAuto();

    if ( pThis->GetTeam() != obj->GetTeam() && obj->GetTeam() != 0 )
    {
        exception = ERR_ENEMY_OBJECT;
        result->SetValInt(ERR_ENEMY_OBJECT);
        return false;
    }

    if ( obj->GetType() == OBJECT_DESTROYER )
        err = automat->StartAction(0);
    else
        err = ERR_WRONG_OBJ;

    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}


// Compilation of instruction "object.factory(cat, program)"

CBotTypResult CScriptFunctions::cFactory(CBotVar* thisclass, CBotVar* &var)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )
    {
        if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();
        if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    }
    return CBotTypResult(CBotTypFloat);
}

// Instruction "object.factory(cat, program)"

bool CScriptFunctions::rFactory(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    Error       err;

    exception = 0;

    ObjectType type = static_cast<ObjectType>(var->GetValInt());
    var = var->GetNext();
    CBotString cbs;
    const char* program;
    if ( var != 0 )
    {
        cbs = var->GetValString();
        program = cbs;
    }
    else
        program = "";

    CObject* factory = static_cast<CObject*>(thisclass->GetUserPtr());
    if (factory == nullptr)
    {
        exception = ERR_UNKNOWN;
        result->SetValInt(ERR_UNKNOWN);
        GetLogger()->Error("in object.factory() - factory is nullptr");
        return false;
    }

    if ( pThis->GetTeam() != factory->GetTeam() && factory->GetTeam() != 0 )
    {
        exception = ERR_ENEMY_OBJECT;
        result->SetValInt(ERR_ENEMY_OBJECT);
        return false;
    }

    if ( factory->GetType() == OBJECT_FACTORY )
    {
        CAutoFactory* automat = static_cast<CAutoFactory*>(factory->GetAuto());
        if (automat == nullptr)
        {
            exception = ERR_UNKNOWN;
            result->SetValInt(ERR_UNKNOWN);
            GetLogger()->Error("in object.factory() - automat is nullptr");
            return false;
        }

        err = CRobotMain::GetInstancePointer()->CanFactoryError(type, factory->GetTeam());

        if ( err == ERR_OK )
        {
            if ( automat != nullptr )
            {
                err = automat->StartAction(type);
                if ( err == ERR_OK ) automat->SetProgram(program);
            }
            else
                err = ERR_UNKNOWN;
        }
    }
    else
        err = ERR_WRONG_OBJ;

    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Instruction "object.research(type)"

bool CScriptFunctions::rResearch(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    Error       err;

    exception = 0;

    ResearchType type = static_cast<ResearchType>(var->GetValInt());

    CObject* center = static_cast<CObject*>(thisclass->GetUserPtr());
    CAuto* automat = center->GetAuto();

    if ( pThis->GetTeam() != center->GetTeam() && center->GetTeam() != 0 )
    {
        exception = ERR_ENEMY_OBJECT;
        result->SetValInt(ERR_ENEMY_OBJECT);
        return false;
    }

    if ( center->GetType() == OBJECT_RESEARCH ||
         center->GetType() == OBJECT_LABO      )
    {
        bool ok = false;
        if ( type == RESEARCH_iPAW       ||
             type == RESEARCH_iGUN        )
        {
            if ( center->GetType() != OBJECT_LABO )
                err = ERR_WRONG_OBJ;
            else
                ok = true;
        }
        else
        {
            if ( center->GetType() != OBJECT_RESEARCH )
                err = ERR_WRONG_OBJ;
            else
                ok = true;
        }
        if ( ok )
        {
            bool bEnable = CRobotMain::GetInstancePointer()->IsResearchEnabled(type);
            if ( bEnable )
            {
                if ( automat != nullptr )
                {
                    err = automat->StartAction(type);
                }
                else
                    err = ERR_UNKNOWN;
            }
            else
                err = ERR_BUILD_DISABLED;
        }
    }
    else
        err = ERR_WRONG_OBJ;

    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        if( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Instruction "object.takeoff()"

bool CScriptFunctions::rTakeOff(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    Error       err;

    exception = 0;

    CObject* base = static_cast<CObject*>(thisclass->GetUserPtr());
    CAuto* automat = base->GetAuto();

    if ( pThis->GetTeam() != base->GetTeam() && base->GetTeam() != 0 )
    {
        exception = ERR_ENEMY_OBJECT;
        result->SetValInt(ERR_ENEMY_OBJECT);
        return false;
    }

    if ( base->GetType() == OBJECT_BASE )
        err = (static_cast<CAutoBase*>(automat))->TakeOff(false);
    else
        err = ERR_WRONG_OBJ;

    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Compilation of the instruction "delete(rank[, exploType[, force]])".

CBotTypResult CScriptFunctions::cDelete(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);

    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )
    {
        if ( var->GetType() != CBotTypInt ) return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();

        if ( var != 0 )
        {
            if ( var->GetType() > CBotTypDouble ) return CBotTypResult(CBotErrBadNum);
            var = var->GetNext();
        }
    }

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "delete(rank[, exploType[, force]])".

bool CScriptFunctions::rDelete(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    int         exploType = 0;
    float       force = 1.0f;

    int rank = var->GetValInt();
    var->GetNext();
    if ( var != 0 )
    {
        exploType = var->GetValInt();
        var->GetNext();
        if ( var != 0 )
        {
            force = var->GetValFloat();
        }
    }

    CObject* obj = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    if ( obj == nullptr )
    {
        return true;
    }
    else
    {
        if ( exploType != 0 )
        {
            obj->ExplodeObject(static_cast<ExplosionType>(exploType), force);
        }
        else
        {
            CObjectManager::GetInstancePointer()->DeleteObject(obj);
        }
    }
    return true;
}



// Compilation of the instruction "search(type, pos)".

CBotTypResult CScriptFunctions::cSearch(CBotVar* &var, void* user)
{
    CBotVar*        array;
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() == CBotTypArrayPointer )
    {
        array = var->GetItemList();
        if ( array == 0 )  return CBotTypResult(CBotTypPointer);
        if ( array->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    }
    else if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )
    {
        ret = cPoint(var, user);
        if ( ret.GetType() != 0 )  return ret;
        if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    }

    return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "search(type, pos)".

bool CScriptFunctions::rSearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;
    CObject     *pBest;
    CBotVar*    array;
    Math::Vector    pos, oPos;
    bool        bArray;
    int         type;

    if ( var->GetType() == CBotTypArrayPointer )
    {
        array = var->GetItemList();
        bArray = true;
    }
    else
    {
        type = var->GetValInt();
        bArray = false;
    }
    var = var->GetNext();
    if ( var != 0 )
    {
        if ( !GetPoint(var, exception, pos) )  return true;
    }
    else
    {
        pos = pThis->GetPosition();
    }

    std::vector<ObjectType> type_v;
    if (bArray)
    {
        while ( array != 0 )
        {
            type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
            array = array->GetNext();
        }
    }
    else
    {
        if (type != OBJECT_NULL)
        {
            type_v.push_back(static_cast<ObjectType>(type));
        }
    }

    pBest = CObjectManager::GetInstancePointer()->Radar(pThis, pos, 0.0f, type_v, 0.0f, Math::PI*2.0f, 0.0f, 1000.0f, false, FILTER_NONE, true);

    if ( pBest == 0 )
    {
        result->SetPointer(0);
    }
    else
    {
        result->SetPointer(pBest->GetBotVar());
    }

    return true;
}


// Compilation of instruction "radar(type, angle, focus, min, max, sens)".

CBotTypResult CScriptFunctions::cRadar(CBotVar* &var, void* user)
{
    CBotVar*    array;

    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() == CBotTypArrayPointer )
    {
        array = var->GetItemList();
        if ( array == 0 )  return CBotTypResult(CBotTypPointer, "object");
        if ( array->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // type
    }
    else if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // type
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // angle
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // focus
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // min
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // max
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // sense
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // filter
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypPointer, "object");
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "radar(type, angle, focus, min, max, sens, filter)".

bool CScriptFunctions::rRadar(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;
    CObject     *pBest;
    CBotVar*    array;
    Math::Vector    oPos;
    RadarFilter filter;
    float       minDist, maxDist, sens, angle, focus;
    int         type;
    bool        bArray = false;

    type    = OBJECT_NULL;
    array   = 0;
    angle   = 0.0f;
    focus   = Math::PI*2.0f;
    minDist = 0.0f*g_unit;
    maxDist = 1000.0f*g_unit;
    sens    = 1.0f;
    filter  = FILTER_NONE;

    if ( var != 0 )
    {
        if ( var->GetType() == CBotTypArrayPointer )
        {
            array = var->GetItemList();
            bArray = true;
        }
        else
        {
            type = var->GetValInt();
            bArray = false;
        }

        var = var->GetNext();
        if ( var != 0 )
        {
            angle = -var->GetValFloat()*Math::PI/180.0f;

            var = var->GetNext();
            if ( var != 0 )
            {
                focus = var->GetValFloat()*Math::PI/180.0f;

                var = var->GetNext();
                if ( var != 0 )
                {
                    minDist = var->GetValFloat();

                    var = var->GetNext();
                    if ( var != 0 )
                    {
                        maxDist = var->GetValFloat();

                        var = var->GetNext();
                        if ( var != 0 )
                        {
                            sens = var->GetValFloat();

                            var = var->GetNext();
                            if ( var != 0 )
                            {
                                filter = static_cast<RadarFilter>(var->GetValInt());
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<ObjectType> type_v;
    if (bArray)
    {
        while ( array != 0 )
        {
            type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
            array = array->GetNext();
        }
    }
    else
    {
        if (type != OBJECT_NULL)
        {
            type_v.push_back(static_cast<ObjectType>(type));
        }
    }

    pBest = CObjectManager::GetInstancePointer()->Radar(pThis, type_v, angle, focus, minDist, maxDist, sens < 0, filter, true); //TODO: why is "sens" done like that?

    if ( pBest == nullptr )
    {
        result->SetPointer(0);
    }
    else
    {
        result->SetPointer(pBest->GetBotVar());
    }
    return true;
}


// Monitoring a task.

bool CScriptFunctions::Process(CScript* script, CBotVar* result, int &exception)
{
    Error       err;

    err = script->m_taskExecutor->GetForegroundTask()->IsEnded();
    if ( err != ERR_CONTINUE )  // task terminated?
    {
        script->m_taskExecutor->StopForegroundTask();

        script->m_bContinue = false;

        if ( err == ERR_STOP )  err = ERR_OK;
        result->SetValInt(err);  // indicates the error or ok
        if ( ShouldProcessStop(err, script->m_errMode) )
        {
            exception = err;
            return false;
        }
        return true;  // it's all over
    }

    script->m_bContinue = true;
    return false;  // not done
}


// Returns true if error code means real error and exception must be thrown

bool CScriptFunctions::ShouldProcessStop(Error err, int errMode)
{
    // aim impossible  - not a real error
    if ( err == ERR_AIM_IMPOSSIBLE )
        return false;

    if ( err != ERR_OK && errMode == ERM_STOP )
        return true;

    return false;
}


// Compilation of the instruction "detect(type)".

CBotTypResult CScriptFunctions::cDetect(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "detect(type)".

bool CScriptFunctions::rDetect(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    CObject     *pBest;
    CBotVar*    array;
    int         type;
    bool        bArray = false;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        type    = OBJECT_NULL;
        array   = 0;

        if ( var != 0 )
        {
            if ( var->GetType() == CBotTypArrayPointer )
            {
                array = var->GetItemList();
                bArray = true;
            }
            else
            {
                type = var->GetValInt();
                bArray = false;
            }
        }

        std::vector<ObjectType> type_v;
        if (bArray)
        {
            while ( array != 0 )
            {
                type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
                array = array->GetNext();
            }
        }
        else
        {
            if (type != OBJECT_NULL)
            {
                type_v.push_back(static_cast<ObjectType>(type));
            }
        }

        pBest = CObjectManager::GetInstancePointer()->Radar(pThis, type_v, 0.0f, 45.0f*Math::PI/180.0f, 0.0f, 20.0f, false, FILTER_NONE, true);

        if (pThis->Implements(ObjectInterfaceType::Old))
        {
            script->m_main->StartDetectEffect(dynamic_cast<COldObject*>(pThis), pBest);
        }

        if ( pBest == 0 )
        {
            script->m_returnValue = 0.0f;
        }
        else
        {
            script->m_returnValue = 1.0f;
        }

        err = script->m_taskExecutor->StartTaskWait(0.3f);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

CBotTypResult CScriptFunctions::cDirection(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "direction(pos)".

bool CScriptFunctions::rDirection(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*        pThis = static_cast<CScript*>(user)->m_object;
    Math::Vector    iPos, oPos;
    float           a, g;

    if ( !GetPoint(var, exception, oPos) )  return true;

    iPos = pThis->GetPosition();

    a = pThis->GetRotationY();
    g = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

    result->SetValFloat(-Math::Direction(a, g)*180.0f/Math::PI);
    return true;
}

// Instruction "canbuild ( category );"
// returns true if this building can be built

bool CScriptFunctions::rCanBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;
    ObjectType category = static_cast<ObjectType>(var->GetValInt());
    exception = 0;

    result->SetValInt(CRobotMain::GetInstancePointer()->CanBuild(category, pThis->GetTeam()));
    return true;
}

bool CScriptFunctions::rCanResearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    ResearchType research = static_cast<ResearchType>(var->GetValInt());
    exception = 0;

    result->SetValInt(CRobotMain::GetInstancePointer()->IsResearchEnabled(research));
    return true;
}

bool CScriptFunctions::rResearched(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;
    ResearchType research = static_cast<ResearchType>(var->GetValInt());
    exception = 0;

    result->SetValInt(CRobotMain::GetInstancePointer()->IsResearchDone(research, pThis->GetTeam()));
    return true;
}

bool CScriptFunctions::rBuildingEnabled(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    ObjectType category = static_cast<ObjectType>(var->GetValInt());
    exception = 0;

    result->SetValInt(CRobotMain::GetInstancePointer()->IsBuildingEnabled(category));
    return true;
}

// Instruction "build(type)"
// draws error if can not build (wher errormode stop), otherwise 0 <- error

bool CScriptFunctions::rBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    ObjectType  oType;
    ObjectType  category;
    Error       err;

    exception = 0;

    oType = pThis->GetType();

    if ( oType != OBJECT_MOBILEfa &&  // allowed only for grabber bots && humans
         oType != OBJECT_MOBILEta &&
         oType != OBJECT_MOBILEwa &&
         oType != OBJECT_MOBILEia &&
         oType != OBJECT_HUMAN    &&
         oType != OBJECT_TECH      )
    {
        err = ERR_WRONG_BOT; // Wrong object
    }
    else
    {
        category = static_cast<ObjectType>(var->GetValInt()); // get category parameter
        err = CRobotMain::GetInstancePointer()->CanBuildError(category, pThis->GetTeam());

        if (err == ERR_OK && !script->m_taskExecutor->IsForegroundTask()) // if we can build
        {
            err = script->m_taskExecutor->StartTaskBuild(category);

            if (err != ERR_OK)
            {
                script->m_taskExecutor->StopForegroundTask();
            }
        }
    }

    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return Process(script, result, exception);

}

// Compilation of the instruction "produce(pos, angle, type[, scriptName[, power]])"
// or "produce(type[, power])".

CBotTypResult CScriptFunctions::cProduce(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);

    if ( var->GetType() <= CBotTypDouble )
    {
        var = var->GetNext();
        if ( var != 0 )
        {
            if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
            var = var->GetNext();
        }
    }
    else
    {
        ret = cPoint(var, user);
        if ( ret.GetType() != 0 )  return ret;

        if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
        if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();

        if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
        if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();

        if ( var != 0 )
        {
            if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
            var = var->GetNext();

            if ( var != 0 )
            {
                if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
                var = var->GetNext();
            }
        }
    }

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "produce(pos, angle, type[, scriptName[, power]])"
// or "produce(type[, power])".

bool CScriptFunctions::rProduce(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    me = script->m_object;
    const char* name = "";
    Math::Vector pos;
    float       angle = 0.0f;
    ObjectType  type = OBJECT_NULL;
    float       power = 0.0f;

    if ( var->GetType() <= CBotTypDouble )
    {
        type = static_cast<ObjectType>(var->GetValInt());
        var = var->GetNext();

        pos = me->GetPosition();

        Math::Vector rotation = me->GetRotation() + me->GetTilt();
        angle = rotation.y;

        if ( var != nullptr )
            power = var->GetValFloat();
        else
            power = -1.0f;

        name = "";
    }
    else
    {
        if ( !GetPoint(var, exception, pos) )  return true;

        angle = var->GetValFloat()*Math::PI/180.0f;
        var = var->GetNext();

        type = static_cast<ObjectType>(var->GetValInt());
        var = var->GetNext();

        if ( var != nullptr )
        {
            CBotString cbs = var->GetValString();
            name = cbs;
            var = var->GetNext();
            if ( var != nullptr )
            {
                power = var->GetValFloat();
            }
            else
            {
                power = -1.0f;
            }
        }
        else
        {
            name = "";
            power = -1.0f;
        }
    }

    CObject* object = nullptr;

    if ( type == OBJECT_ANT    ||
        type == OBJECT_SPIDER ||
        type == OBJECT_BEE    ||
        type == OBJECT_WORM   )
    {
        object = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type);
        CObjectManager::GetInstancePointer()->CreateObject(pos, angle, OBJECT_EGG, 0.0f);
        if (object->Implements(ObjectInterfaceType::Programmable))
        {
            dynamic_cast<CProgrammableObject*>(object)->SetActivity(false);
        }
    }
    else
    {
        if ((type == OBJECT_POWER || type == OBJECT_ATOMIC) && power == -1.0f)
        {
            power = 1.0f;
        }
        object = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type, power);
        if ( object == nullptr )
        {
            result->SetValInt(1);  // error
            return true;
        }
        if(type == OBJECT_MOBILEdr)
        {
            assert(object->Implements(ObjectInterfaceType::Old)); // TODO: temporary hack
            dynamic_cast<COldObject*>(object)->SetManual(true);
        }
        script->m_main->CreateShortcuts();
    }

    if (name[0] != 0)
    {
        std::string name2 = InjectLevelPathsForCurrentLevel(name, "ai");
        if (object->Implements(ObjectInterfaceType::Programmable))
        {
            CProgrammableObject* programmable = dynamic_cast<CProgrammableObject*>(object);
            Program* program = programmable->AddProgram();
            programmable->ReadProgram(program, name2.c_str());
            programmable->RunProgram(program);
        }
    }

    result->SetValInt(0);  // no error
    return true;
}


// Compilation of the instruction "distance(p1, p2)".

CBotTypResult CScriptFunctions::cDistance(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "distance(p1, p2)".

bool CScriptFunctions::rDistance(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    Math::Vector    p1, p2;
    float       value;

    if ( !GetPoint(var, exception, p1) )  return true;
    if ( !GetPoint(var, exception, p2) )  return true;

    value = Math::Distance(p1, p2);
    result->SetValFloat(value/g_unit);
    return true;
}

// Instruction "distance2d(p1, p2)".

bool CScriptFunctions::rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    Math::Vector    p1, p2;
    float       value;

    if ( !GetPoint(var, exception, p1) )  return true;
    if ( !GetPoint(var, exception, p2) )  return true;

    value = Math::DistanceProjected(p1, p2);
    result->SetValFloat(value/g_unit);
    return true;
}


// Compilation of the instruction "space(center, rMin, rMax, dist)".

CBotTypResult CScriptFunctions::cSpace(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypIntrinsic, "point");
}

// Instruction "space(center, rMin, rMax, dist)".

bool CScriptFunctions::rSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    CBotVar*    pSub;
    Math::Vector    center;
    float       rMin, rMax, dist;

    rMin = 10.0f*g_unit;
    rMax = 50.0f*g_unit;
    dist =  4.0f*g_unit;

    if ( var == 0 )
    {
        center = pThis->GetPosition();
    }
    else
    {
        if ( !GetPoint(var, exception, center) )  return true;

        if ( var != 0 )
        {
            rMin = var->GetValFloat()*g_unit;
            var = var->GetNext();

            if ( var != 0 )
            {
                rMax = var->GetValFloat()*g_unit;
                var = var->GetNext();

                if ( var != 0 )
                {
                    dist = var->GetValFloat()*g_unit;
                    var = var->GetNext();
                }
            }
        }
    }
    script->m_main->FreeSpace(center, rMin, rMax, dist, pThis);

    if ( result != 0 )
    {
        pSub = result->GetItemList();
        if ( pSub != 0 )
        {
            pSub->SetValFloat(center.x/g_unit);
            pSub = pSub->GetNext();  // "y"
            pSub->SetValFloat(center.z/g_unit);
            pSub = pSub->GetNext();  // "z"
            pSub->SetValFloat(center.y/g_unit);
        }
    }
    return true;
}

CBotTypResult CScriptFunctions::cFlatSpace(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypIntrinsic, "point");
}

bool CScriptFunctions::rFlatSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    CBotVar*    pSub;
    Math::Vector    center;
    float       flatMin, rMin, rMax, dist;

    rMin = 10.0f*g_unit;
    rMax = 50.0f*g_unit;
    dist =  4.0f*g_unit;


    if ( !GetPoint(var, exception, center) )  return true;

    flatMin = var->GetValFloat()*g_unit;
    var = var->GetNext();

    if ( var != 0 )
    {
        rMin = var->GetValFloat()*g_unit;
        var = var->GetNext();

        if ( var != 0 )
        {
            rMax = var->GetValFloat()*g_unit;
            var = var->GetNext();

            if ( var != 0 )
            {
                dist = var->GetValFloat()*g_unit;
                var = var->GetNext();
            }
        }
    }
    script->m_main->FlatFreeSpace(center, flatMin, rMin, rMax, dist, pThis);

    if ( result != 0 )
    {
        pSub = result->GetItemList();
        if ( pSub != 0 )
        {
            pSub->SetValFloat(center.x/g_unit);
            pSub = pSub->GetNext();  // "y"
            pSub->SetValFloat(center.z/g_unit);
            pSub = pSub->GetNext();  // "z"
            pSub->SetValFloat(center.y/g_unit);
        }
    }
    return true;
}


// Compilation of the instruction "flatground(center, rMax)".

CBotTypResult CScriptFunctions::cFlatGround(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "flatground(center, rMax)".

bool CScriptFunctions::rFlatGround(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    Math::Vector    center;
    float       rMax, dist;

    if ( !GetPoint(var, exception, center) )  return true;
    rMax = var->GetValFloat()*g_unit;
    var = var->GetNext();

    dist = script->m_main->GetFlatZoneRadius(center, rMax, pThis);
    result->SetValFloat(dist/g_unit);

    return true;
}


// Instruction "wait(t)".

bool CScriptFunctions::rWait(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       value;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        value = var->GetValFloat();
        err = script->m_taskExecutor->StartTaskWait(value);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

bool CScriptFunctions::rMove(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       value;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        value = var->GetValFloat();
        err = script->m_taskExecutor->StartTaskAdvance(value*g_unit);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

bool CScriptFunctions::rTurn(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       value;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        value = var->GetValFloat();
        err = script->m_taskExecutor->StartTaskTurn(-value*Math::PI/180.0f);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

CBotTypResult CScriptFunctions::cGoto(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "goto(pos, altitude, mode)".

bool CScriptFunctions::rGoto(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*        script = static_cast<CScript*>(user);
    Math::Vector        pos;
    TaskGotoGoal    goal;
    TaskGotoCrash   crash;
    float           altitude;
    Error           err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        if ( !GetPoint(var, exception, pos) )  return true;

        goal  = TGG_DEFAULT;
        crash = TGC_DEFAULT;
        altitude = 0.0f*g_unit;

        if ( var != 0 )
        {
            altitude = var->GetValFloat()*g_unit;

            var = var->GetNext();
            if ( var != 0 )
            {
                goal = static_cast<TaskGotoGoal>(var->GetValInt());

                var = var->GetNext();
                if ( var != 0 )
                {
                    crash = static_cast<TaskGotoCrash>(var->GetValInt());
                }
            }
        }

        err = script->m_taskExecutor->StartTaskGoto(pos, altitude, goal, crash);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

CBotTypResult CScriptFunctions::cGrabDrop(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "grab(oper)".

bool CScriptFunctions::rGrab(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    ObjectType  oType;
    TaskManipArm type;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        if ( var == 0 )
        {
            type = TMA_FFRONT;
        }
        else
        {
            type = static_cast<TaskManipArm>(var->GetValInt());
        }

        oType = pThis->GetType();
        if ( oType == OBJECT_HUMAN ||
            oType == OBJECT_TECH  )
        {
            err = script->m_taskExecutor->StartTaskTake();
        }
        else
        {
            err = script->m_taskExecutor->StartTaskManip(TMO_GRAB, type);
        }

        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

bool CScriptFunctions::rDrop(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    ObjectType  oType;
    TaskManipArm type;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        if ( var == 0 )  type = TMA_FFRONT;
        else             type = static_cast<TaskManipArm>(var->GetValInt());

        oType = pThis->GetType();
        if ( oType == OBJECT_HUMAN ||
            oType == OBJECT_TECH  )
        {
            err = script->m_taskExecutor->StartTaskTake();
        }
        else
        {
            err = script->m_taskExecutor->StartTaskManip(TMO_DROP, type);
        }

        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

bool CScriptFunctions::rSniff(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        err = script->m_taskExecutor->StartTaskSearch();
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

CBotTypResult CScriptFunctions::cReceive(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "receive(nom, power)".

bool CScriptFunctions::rReceive(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    CBotString  cbs;
    Error       err;
    const char* p;
    float       value, power;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        cbs = var->GetValString();
        p = cbs;
        var = var->GetNext();

        power = 10.0f*g_unit;
        if ( var != 0 )
        {
            power = var->GetValFloat()*g_unit;
            var = var->GetNext();
        }

        err = script->m_taskExecutor->StartTaskInfo(static_cast<const char*>(p), 0.0f, power, false);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
            result->SetInit(CBotVar::InitType::IS_NAN);
            return true;
        }
    }
    if ( !Process(script, result, exception) )  return false;  // not finished

    value = pThis->GetInfoReturn();
    if ( std::isnan(value) )
    {
        result->SetInit(CBotVar::InitType::IS_NAN);
    }
    else
    {
        result->SetValFloat(value);
    }
    return true;
}

// Compilation of the instruction "send(nom, value, power)".

CBotTypResult CScriptFunctions::cSend(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "send(nom, value, power)".

bool CScriptFunctions::rSend(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CBotString  cbs;
    Error       err;
    const char* p;
    float       value, power;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        cbs = var->GetValString();
        p = cbs;
        var = var->GetNext();

        value = var->GetValFloat();
        var = var->GetNext();

        power = 10.0f*g_unit;
        if ( var != 0 )
        {
            power = var->GetValFloat()*g_unit;
            var = var->GetNext();
        }

        err = script->m_taskExecutor->StartTaskInfo(static_cast<const char*>(p), value, power, true);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

CExchangePost* CScriptFunctions::FindExchangePost(CObject* object, float power)
{
    CObject* exchangePost = CObjectManager::GetInstancePointer()->FindNearest(object, OBJECT_INFO, power/g_unit);
    return dynamic_cast<CExchangePost*>(exchangePost);
}

// Compilation of the instruction "deleteinfo(name, power)".

CBotTypResult CScriptFunctions::cDeleteInfo(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypBoolean);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "deleteinfo(name, power)".

bool CScriptFunctions::rDeleteInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;

    exception = 0;

    CBotString infoNameCbs = var->GetValString();
    std::string infoName = std::string(static_cast<const char*>(infoNameCbs));
    var = var->GetNext();

    float power = 10.0f*g_unit;
    if (var != nullptr)
    {
        power = var->GetValFloat()*g_unit;
        var = var->GetNext();
    }

    CExchangePost* exchangePost = FindExchangePost(pThis, power);
    if (exchangePost == nullptr)
    {
        result->SetValInt(false);
        return true;
    }

    bool infoDeleted = exchangePost->DeleteInfo(infoName);
    result->SetValInt(infoDeleted);

    return true;
}

// Compilation of the instruction "testinfo(nom, power)".

CBotTypResult CScriptFunctions::cTestInfo(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypBoolean);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "testinfo(name, power)".

bool CScriptFunctions::rTestInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;

    exception = 0;

    CBotString infoNameCbs = var->GetValString();
    std::string infoName = std::string(static_cast<const char*>(infoNameCbs));
    var = var->GetNext();

    float power = 10.0f*g_unit;
    if (var != nullptr)
    {
        power = var->GetValFloat()*g_unit;
        var = var->GetNext();
    }

    CExchangePost* exchangePost = FindExchangePost(pThis, power);
    if (exchangePost == nullptr)
    {
        result->SetValInt(false);
        return true;
    }

    bool foundInfo = exchangePost->HasInfo(infoName);
    result->SetValInt(foundInfo);
    return true;
}

// Instruction "thump()".

bool CScriptFunctions::rThump(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        err = script->m_taskExecutor->StartTaskTerraform();
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

bool CScriptFunctions::rRecycle(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        err = script->m_taskExecutor->StartTaskRecover();
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
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

CBotTypResult CScriptFunctions::cShield(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "shield(oper, radius)".

bool CScriptFunctions::rShield(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    float       oper, radius;
    Error       err;

    // only shielder can use shield()
    if (pThis->GetType() != OBJECT_MOBILErs)
    {
        result->SetValInt(ERR_WRONG_BOT);  // return error
        if (script->m_errMode == ERM_STOP)
        {
            exception = ERR_WRONG_BOT;
            return false;
        }
        return true;
    }

    oper = var->GetValFloat();  // 0=down, 1=up
    var = var->GetNext();

    radius = var->GetValFloat();
    if ( radius < 10.0f )  radius = 10.0f;
    if ( radius > 25.0f )  radius = 25.0f;
    radius = (radius-10.0f)/15.0f;

    if ( !script->m_taskExecutor->IsBackgroundTask() )  // shield folds?
    {
        if ( oper == 0.0f )  // down?
        {
            result->SetValInt(1);  // shows the error
        }
        else    // up ?
        {
            pThis->SetParam(radius);
            err = script->m_taskExecutor->StartTaskShield(TSM_UP, 1000.0f);
            if ( err != ERR_OK )
            {
                script->m_taskExecutor->StopBackgroundTask();
                result->SetValInt(err);  // shows the error
            }
        }
    }
    else    // shield deployed?
    {
        if ( oper == 0.0f )  // down?
        {
            script->m_taskExecutor->StartTaskShield(TSM_DOWN, 0.0f);
        }
        else    // up?
        {
            //?         result->SetValInt(1);  // shows the error
            pThis->SetParam(radius);
            script->m_taskExecutor->StartTaskShield(TSM_UPDATE, 0.0f);
        }
    }

    return true;
}

// Compilation "fire(delay)".

CBotTypResult CScriptFunctions::cFire(CBotVar* &var, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;
    ObjectType  type;

    type = pThis->GetType();

    if ( type == OBJECT_ANT )
    {
        if ( var == 0 ) return CBotTypResult(CBotErrLowParam);
        CBotTypResult ret = cPoint(var, user);
        if ( ret.GetType() != 0 )  return ret;
        if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    }
    else if ( type == OBJECT_SPIDER )
    {
        if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    }
    else
    {
        if ( var != 0 )
        {
            if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
            var = var->GetNext();
            if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
        }
    }
    return CBotTypResult(CBotTypFloat);
}

// Instruction "fire(delay)".

bool CScriptFunctions::rFire(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    float       delay;
    Math::Vector    impact;
    Error       err;
    ObjectType  type;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        type = pThis->GetType();

        if ( type == OBJECT_ANT )
        {
            if ( !GetPoint(var, exception, impact) )  return true;
            float waterLevel = Gfx::CEngine::GetInstancePointer()->GetWater()->GetLevel();
            impact.y += waterLevel;
            err = script->m_taskExecutor->StartTaskFireAnt(impact);
        }
        else if ( type == OBJECT_SPIDER )
        {
            err = script->m_taskExecutor->StartTaskSpiderExplo();
        }
        else
        {
            if ( var == 0 )  delay = 0.0f;
            else             delay = var->GetValFloat();
            if ( delay < 0.0f ) delay = -delay;
            err = script->m_taskExecutor->StartTaskFire(delay);
        }

        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
            result->SetValInt(err);  // shows the error
            return true;
        }
    }
    return Process(script, result, exception);
}

// Compilation of the instruction "aim(x, y)".

CBotTypResult CScriptFunctions::cAim(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "aim(dir)".

bool CScriptFunctions::rAim(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       x, y;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        x = var->GetValFloat();
        var = var->GetNext();
        var == 0 ? y=0.0f : y=var->GetValFloat();
        err = script->m_taskExecutor->StartTaskGunGoal(x*Math::PI/180.0f, y*Math::PI/180.0f);
        if ( err == ERR_AIM_IMPOSSIBLE )
        {
            result->SetValInt(err);  // shows the error
        }
        else if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
            result->SetValInt(err);  // shows the error
            return true;
        }
    }
    return Process(script, result, exception);
}

// Compilation of the instruction "motor(left, right)".

CBotTypResult CScriptFunctions::cMotor(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "motor(left, right)".

bool CScriptFunctions::rMotor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;
    CPhysics*   physics = (static_cast<CScript*>(user)->m_object)->GetPhysics();
    float       left, right, speed, turn;

    left = var->GetValFloat();
    var = var->GetNext();
    right = var->GetValFloat();

    speed = (left+right)/2.0f;
    if ( speed < -1.0f )  speed = -1.0f;
    if ( speed >  1.0f )  speed =  1.0f;

    turn = left-right;
    if ( turn < -1.0f )  turn = -1.0f;
    if ( turn >  1.0f )  turn =  1.0f;

    if ( pThis->GetFixed() )  // ant on the back?
    {
        speed = 0.0f;
        turn  = 0.0f;
    }

    physics->SetMotorSpeedX(speed);  // forward/backward
    physics->SetMotorSpeedZ(turn);  // turns

    return true;
}

// Instruction "jet(power)".

bool CScriptFunctions::rJet(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CPhysics*   physics = (static_cast<CScript*>(user)->m_object)->GetPhysics();
    float       value;

    value = var->GetValFloat();
    if ( value > 1.0f ) value = 1.0f;

    physics->SetMotorSpeedY(value);

    return true;
}

// Compilation of the instruction "topo(pos)".

CBotTypResult CScriptFunctions::cTopo(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = CScriptFunctions::cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "topo(pos)".

bool CScriptFunctions::rTopo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    Math::Vector    pos;
    float       level;

    exception = 0;

    if ( !GetPoint(var, exception, pos) )  return true;

    level = script->m_terrain->GetFloorLevel(pos);
    level -= script->m_water->GetLevel();
    result->SetValFloat(level/g_unit);
    return true;
}

// Compilation of the instruction "message(string, type)".

CBotTypResult CScriptFunctions::cMessage(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
        var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "message(string, type)".

bool CScriptFunctions::rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CBotString  cbs;
    const char* p;
    Ui::TextType    type;

    cbs = var->GetValString();
    p = cbs;

    type = Ui::TT_MESSAGE;
    var = var->GetNext();
    if ( var != 0 )
    {
        type = static_cast<Ui::TextType>(var->GetValInt());
    }

    script->m_main->GetDisplayText()->DisplayText(p, script->m_object, 10.0f, type);

    return true;
}

// Instruction "cmdline(rank)".

bool CScriptFunctions::rCmdline(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;
    float       value;
    int         rank;

    assert(pThis->Implements(ObjectInterfaceType::Programmable));

    rank = var->GetValInt();
    value = dynamic_cast<CProgrammableObject*>(pThis)->GetCmdLine(rank);
    result->SetValFloat(value);

    return true;
}

// Instruction "ismovie()".

bool CScriptFunctions::rIsMovie(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       value;

    value = script->m_main->GetMovieLock()?1.0f:0.0f;
    result->SetValFloat(value);

    return true;
}

// Instruction "errmode(mode)".

bool CScriptFunctions::rErrMode(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    int         value;

    value = var->GetValInt();
    if ( value < 0 )  value = 0;
    if ( value > 1 )  value = 1;
    script->m_errMode = value;

    return true;
}

// Instruction "ipf(num)".

bool CScriptFunctions::rIPF(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    int         value;

    value = var->GetValInt();
    if ( value <     1 )  value =     1;
    if ( value > 10000 )  value = 10000;
    script->m_ipf = value;

    return true;
}

// Instruction "abstime()".

bool CScriptFunctions::rAbsTime(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       value;

    value = script->m_main->GetGameTime();
    result->SetValFloat(value);
    return true;
}

// Compilation of the instruction "pendown(color, width)".

CBotTypResult CScriptFunctions::cPenDown(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "pendown(color, width)".

bool CScriptFunctions::rPenDown(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    int         color;
    float       width;
    Error       err;

    CMotionVehicle* motionVehicle = dynamic_cast<CMotionVehicle*>(pThis->GetMotion());
    assert(motionVehicle != nullptr);

    exception = 0;

    if ( var != 0 )
    {
        color = var->GetValInt();
        if ( color <  0 )  color =  0;
        if ( color > static_cast<int>(TraceColor::Max) )  color = static_cast<int>(TraceColor::Max);
        motionVehicle->SetTraceColor(static_cast<TraceColor>(color));

        var = var->GetNext();
        if ( var != 0 )
        {
            width = var->GetValFloat();
            if ( width < 0.1f )  width = 0.1f;
            if ( width > 1.0f )  width = 1.0f;
            motionVehicle->SetTraceWidth(width);
        }
    }
    motionVehicle->SetTraceDown(true);

    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
        {
            err = script->m_taskExecutor->StartTaskPen(motionVehicle->GetTraceDown(), motionVehicle->GetTraceColor());
            if ( err != ERR_OK )
            {
                script->m_taskExecutor->StopForegroundTask();
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
        return true;
    }
}

// Instruction "penup()".

bool CScriptFunctions::rPenUp(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    Error       err;

    CMotionVehicle* motionVehicle = dynamic_cast<CMotionVehicle*>(pThis->GetMotion());
    assert(motionVehicle != nullptr);

    exception = 0;

    motionVehicle->SetTraceDown(false);

    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
        {
            motionVehicle->SetTraceDown(false);

            err = script->m_taskExecutor->StartTaskPen(motionVehicle->GetTraceDown(), motionVehicle->GetTraceColor());
            if ( err != ERR_OK )
            {
                script->m_taskExecutor->StopForegroundTask();
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
        return true;
    }
}

// Instruction "pencolor()".

bool CScriptFunctions::rPenColor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    int         color;
    Error       err;

    CMotionVehicle* motionVehicle = dynamic_cast<CMotionVehicle*>(pThis->GetMotion());
    assert(motionVehicle != nullptr);

    exception = 0;

    color = var->GetValInt();
    if ( color <  0 )  color =  0;
    if ( color > static_cast<int>(TraceColor::Max) )  color = static_cast<int>(TraceColor::Max);
    motionVehicle->SetTraceColor(static_cast<TraceColor>(color));

    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
        {
            err = script->m_taskExecutor->StartTaskPen(motionVehicle->GetTraceDown(), motionVehicle->GetTraceColor());
            if ( err != ERR_OK )
            {
                script->m_taskExecutor->StopForegroundTask();
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
        return true;
    }
}

// Instruction "penwidth()".

bool CScriptFunctions::rPenWidth(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;
    float       width;

    CMotionVehicle* motionVehicle = dynamic_cast<CMotionVehicle*>(pThis->GetMotion());
    assert(motionVehicle != nullptr);

    width = var->GetValFloat();
    if ( width < 0.1f )  width = 0.1f;
    if ( width > 1.0f )  width = 1.0f;
    motionVehicle->SetTraceWidth(width);
    return true;
}

// Compilation of the instruction with one object parameter

CBotTypResult CScriptFunctions::cOneObject(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypFloat);

    return CBotTypResult(CBotErrOverParam);
}

// Instruction "camerafocus(object)".

bool CScriptFunctions::rCameraFocus(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript* script = static_cast<CScript*>(user);

    CObject* object = static_cast<CObject*>(var->GetUserPtr());

    script->m_main->SelectObject(object, false);

    result->SetValInt(ERR_OK);
    exception = ERR_OK;
    return true;
}

// Static variables

int                                 CScriptFunctions::m_numberOfOpenFiles = 0;
std::unordered_map<int, std::unique_ptr<std::ios>> CScriptFunctions::m_files;
int                                 CScriptFunctions::m_nextFile = 1;



// Prepares a file name.

void PrepareFilename(CBotString &filename)
{
    CResourceManager::CreateDirectory("files");
    filename = CBotString("files/") + filename;
    GetLogger()->Debug("CBot accessing file '%s'\n", static_cast<const char*>(filename));
}


bool CScriptFunctions::FileClassOpenFile(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    CBotString  mode;

    // must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    CBotString  filename = pVar->GetValString();
    PrepareFilename(filename);

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != NULL )
    {
        // recover mode
        mode = pVar->GetValString();
        if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return false; }

        // no third parameter
        if ( pVar->GetNext() != NULL ) { Exception = CBotErrOverParam; return false; }
    }

    // saves the file name
    pVar = pThis->GetItem("filename");
    pVar->SetValString(filename);

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    // which must not be initialized
    if ( pVar->IsDefined()) { Exception = CBotErrFileOpen; return false; }

    if ( ! mode.IsEmpty() )
    {
        // opens the requested file
        bool ok = false;
        std::unique_ptr<std::ios> file;
        if (mode == "r")
        {
            auto is = MakeUnique<CInputStream>(static_cast<const char*>(filename));
            ok = is->is_open();
            file = std::move(is);
        }
        else if (mode == "w")
        {
            auto os = MakeUnique<COutputStream>(static_cast<const char*>(filename));
            ok = os->is_open();
            file = std::move(os);
        }
        if (!ok) { Exception = CBotErrFileOpen; return false; }

        m_numberOfOpenFiles ++;

        int fileHandle = m_nextFile++;

        m_files[fileHandle] = std::move(file);

        // save the file handle
        pVar = pThis->GetItem("handle");
        pVar->SetValInt(fileHandle);
    }
    return true;
}

// constructor of the class
// get the filename as a parameter

// execution
bool CScriptFunctions::rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // accepts no parameters
    if ( pVar == NULL ) return true;

    return FileClassOpenFile(pThis, pVar, pResult, Exception);
}

// compilation
CBotTypResult CScriptFunctions::cfconstruct (CBotVar* pThis, CBotVar* &pVar)
{
    // accepts no parameters
    if ( pVar == NULL ) return CBotTypResult( 0 );

    // must be a character string
    if ( pVar->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != NULL )
    {
        // which must be a string
        if ( pVar->GetType() != CBotTypString )
            return CBotTypResult( CBotErrBadString );
        // no third parameter
        if ( pVar->GetNext() != NULL ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is void (constructor)
    return CBotTypResult( 0 );
}


// destructor of the class

// execution
bool CScriptFunctions::rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    // don't open? no problem :)
    if ( pVar->IsDefined()) return true;

    int fileHandle = pVar->GetValInt();

    std::ios* file = m_files[fileHandle].get();
    CInputStream* is = dynamic_cast<CInputStream*>(file);
    if(is != nullptr) is->close();
    COutputStream* os = dynamic_cast<COutputStream*>(file);
    if(os != nullptr) os->close();

    m_numberOfOpenFiles--;

    pVar->SetInit(CBotVar::InitType::IS_NAN);

    m_files.erase(fileHandle);

    return true;
}


// process FILE :: open
// get the r/w mode as a parameter

// execution
bool CScriptFunctions::rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return false; }

    bool result = FileClassOpenFile(pThis, pVar, pResult, Exception);
    pResult->SetValInt(result);
    return result;
}

// compilation
CBotTypResult CScriptFunctions::cfopen (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

    // which must be a string
    if ( pVar->GetType() != CBotTypString )
        return CBotTypResult( CBotErrBadString );

    // there may be a second parameter
    pVar = pVar->GetNext();
    if ( pVar != NULL )
    {
        // which must be a string
        if ( pVar->GetType() != CBotTypString )
            return CBotTypResult( CBotErrBadString );

        // no third parameter
        if ( pVar->GetNext() != NULL ) return CBotTypResult( CBotErrOverParam );
    }

    // the result is bool
    return CBotTypResult(CBotTypBoolean);
}


// process FILE :: close

// execeution
bool CScriptFunctions::rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // it shouldn't be any parameters
    if (pVar != NULL) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( !pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = m_files.find(fileHandle);
    if (handleIter == m_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    assert(handleIter->second);

    std::ios* file = handleIter->second.get();
    CInputStream* is = dynamic_cast<CInputStream*>(file);
    if(is != nullptr) is->close();
    COutputStream* os = dynamic_cast<COutputStream*>(file);
    if(os != nullptr) os->close();

    m_numberOfOpenFiles--;

    pVar->SetInit(CBotVar::InitType::IS_NAN);

    m_files.erase(handleIter);

    return true;
}

// compilation
CBotTypResult CScriptFunctions::cfclose (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameters
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "void"
    return CBotTypResult( 0 );
}

// process FILE :: writeln

// execution
bool CScriptFunctions::rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return false; }

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }

    CBotString param = pVar->GetValString();

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( !pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = m_files.find(fileHandle);
    if (handleIter == m_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    COutputStream* os = dynamic_cast<COutputStream*>(handleIter->second.get());
    if (os == nullptr) { Exception = CBotErrWrite; return false; }

    *os << param << "\n";

    // if an error occurs generate an exception
    if ( os->bad() ) { Exception = CBotErrWrite; return false; }

    return true;
}

// compilation
CBotTypResult CScriptFunctions::cfwrite (CBotVar* pThis, CBotVar* &pVar)
{
    // there must be a parameter
    if ( pVar == NULL ) return CBotTypResult( CBotErrLowParam );

    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) return CBotTypResult( CBotErrBadString );

    // no other parameter
    if ( pVar->GetNext() != NULL ) return CBotTypResult( CBotErrOverParam );

    // the function returns a void result
    return CBotTypResult( 0 );
}

// process FILE :: readln

// execution
bool CScriptFunctions::rfread(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // it shouldn't be any parameters
    if (pVar != NULL) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if (!pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = m_files.find(fileHandle);
    if (handleIter == m_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    CInputStream* is = dynamic_cast<CInputStream*>(handleIter->second.get());
    if (is == nullptr) { Exception = CBotErrRead; return false; }

    std::string line;
    std::getline(*is, line);

    // if an error occurs generate an exception
    if ( is->bad() ) { Exception = CBotErrRead; return false; }

    pResult->SetValString( line.c_str() );

    return true;
}

// compilation
CBotTypResult CScriptFunctions::cfread (CBotVar* pThis, CBotVar* &pVar)
{
    // it should not be any parameter
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // function returns a result "string"
    return CBotTypResult( CBotTypString );
}
// process FILE :: readln


// execution
bool CScriptFunctions::rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user)
{
    // it should not be any parameter
    if ( pVar != NULL ) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if ( !pVar->IsDefined()) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    const auto handleIter = m_files.find(fileHandle);
    if (handleIter == m_files.end())
    {
        Exception = CBotErrNotOpen;
        return false;
    }

    pResult->SetValInt( handleIter->second->eof() );

    return true;
}

// compilation
CBotTypResult CScriptFunctions::cfeof (CBotVar* pThis, CBotVar* &pVar)
{
    // it shouldn't be any parameter
    if ( pVar != NULL ) return CBotTypResult( CBotErrOverParam );

    // the function returns a boolean result
    return CBotTypResult( CBotTypBoolean );
}

// Instruction "deletefile(filename)".

bool CScriptFunctions::rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CBotString  cbs;

    cbs = var->GetValString();
    PrepareFilename(cbs);
    std::string filename = static_cast<const char*>(cbs);
    return CResourceManager::Remove(filename);
}

// Compilation of class "point".

CBotTypResult CScriptFunctions::cPointConstructor(CBotVar* pThis, CBotVar* &var)
{
    if ( !pThis->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadNum);

    if ( var == NULL )  return CBotTypResult(0);  // ok if no parameter

    // First parameter (x):
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Second parameter (y):
    if ( var == NULL )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Third parameter (z):
    if ( var == NULL )  // only 2 parameters?
    {
        return CBotTypResult(0);  // this function returns void
    }

    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != NULL )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(0);  // this function returns void
}

//Execution of the class "point".

bool CScriptFunctions::rPointConstructor(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception, void* user)
{
    CBotVar     *pX, *pY, *pZ;

    if ( var == NULL )  return true;  // constructor with no parameters is ok

    if ( var->GetType() > CBotTypDouble )
    {
        Exception = CBotErrBadNum;  return false;
    }

    pX = pThis->GetItem("x");
    if ( pX == NULL )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pX->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var == NULL )
    {
        Exception = CBotErrLowParam;  return false;
    }

    if ( var->GetType() > CBotTypDouble )
    {
        Exception = CBotErrBadNum;  return false;
    }

    pY = pThis->GetItem("y");
    if ( pY == NULL )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pY->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var == NULL )
    {
        return true;  // ok with only two parameters
    }

    pZ = pThis->GetItem("z");
    if ( pZ == NULL )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pZ->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var != NULL )
    {
        Exception = CBotErrOverParam;  return false;
    }

    return  true;  // no interruption
}




// Initializes all functions for module CBOT.

void CScriptFunctions::Init()
{
    CBotProgram::SetTimer(100);
    CBotProgram::Init();

    for (int i = 0; i < OBJECT_MAX; i++)
    {
        ObjectType type = static_cast<ObjectType>(i);
        const char* token = GetObjectName(type);
        if (token[0] != 0)
            CBotProgram::DefineNum(token, type);

        token = GetObjectAlias(type);
        if (token[0] != 0)
            CBotProgram::DefineNum(token, type);
    }
    CBotProgram::DefineNum("Any", OBJECT_NULL);

    for (int i = 0; i < static_cast<int>(TraceColor::Max); i++)
    {
        TraceColor color = static_cast<TraceColor>(i);
        CBotProgram::DefineNum(TraceColorName(color).c_str(), static_cast<int>(color));
    }

    CBotProgram::DefineNum("InFront",    TMA_FFRONT);
    CBotProgram::DefineNum("Behind",     TMA_FBACK);
    CBotProgram::DefineNum("EnergyCell", TMA_POWER);

    CBotProgram::DefineNum("DisplayError",   Ui::TT_ERROR);
    CBotProgram::DefineNum("DisplayWarning", Ui::TT_WARNING);
    CBotProgram::DefineNum("DisplayInfo",    Ui::TT_INFO);
    CBotProgram::DefineNum("DisplayMessage", Ui::TT_MESSAGE);

    CBotProgram::DefineNum("FilterNone",        FILTER_NONE);
    CBotProgram::DefineNum("FilterOnlyLanding", FILTER_ONLYLANDING);
    CBotProgram::DefineNum("FilterOnlyFlying",  FILTER_ONLYFLYING);
    CBotProgram::DefineNum("FilterFriendly",    FILTER_FRIENDLY);
    CBotProgram::DefineNum("FilterEnemy",       FILTER_ENEMY);
    CBotProgram::DefineNum("FilterNeutral",     FILTER_NEUTRAL);

    CBotProgram::DefineNum("ExplosionNone",  0);
    CBotProgram::DefineNum("ExplosionBang",  static_cast<int>(ExplosionType::Bang));
    CBotProgram::DefineNum("ExplosionBurn",  static_cast<int>(ExplosionType::Burn));
    CBotProgram::DefineNum("ExplosionWater", static_cast<int>(ExplosionType::Water));

    CBotProgram::DefineNum("ResultNotEnded",  ERR_MISSION_NOTERM);
    CBotProgram::DefineNum("ResultLost",      INFO_LOST);
    CBotProgram::DefineNum("ResultLostQuick", INFO_LOSTq);
    CBotProgram::DefineNum("ResultWin",       ERR_OK);

    // NOTE: The Build___ constants are for use only with getbuild() and setbuild() for MissionController, not for normal players
    CBotProgram::DefineNum("BuildBotFactory",       BUILD_FACTORY);
    CBotProgram::DefineNum("BuildDerrick",          BUILD_DERRICK);
    CBotProgram::DefineNum("BuildConverter",        BUILD_CONVERT);
    CBotProgram::DefineNum("BuildRadarStation",     BUILD_RADAR);
    CBotProgram::DefineNum("BuildPowerPlant",       BUILD_ENERGY);
    CBotProgram::DefineNum("BuildNuclearPlant",     BUILD_NUCLEAR);
    CBotProgram::DefineNum("BuildPowerStation",     BUILD_STATION);
    CBotProgram::DefineNum("BuildRepairCenter",     BUILD_REPAIR);
    CBotProgram::DefineNum("BuildDefenseTower",     BUILD_TOWER);
    CBotProgram::DefineNum("BuildResearchCenter",   BUILD_RESEARCH);
    CBotProgram::DefineNum("BuildAutoLab",          BUILD_LABO);
    CBotProgram::DefineNum("BuildPowerCaptor",      BUILD_PARA);
    CBotProgram::DefineNum("BuildExchangePost",     BUILD_INFO);
    CBotProgram::DefineNum("BuildDestroyer",        BUILD_DESTROYER);
    CBotProgram::DefineNum("FlatGround",            BUILD_GFLAT);
    CBotProgram::DefineNum("UseFlags",              BUILD_FLAG);

    CBotProgram::DefineNum("ResearchTracked",       RESEARCH_TANK);
    CBotProgram::DefineNum("ResearchWinged",        RESEARCH_FLY);
    CBotProgram::DefineNum("ResearchShooter",       RESEARCH_CANON);
    CBotProgram::DefineNum("ResearchDefenseTower",  RESEARCH_TOWER);
    CBotProgram::DefineNum("ResearchNuclearPlant",  RESEARCH_ATOMIC);
    CBotProgram::DefineNum("ResearchThumper",       RESEARCH_THUMP);
    CBotProgram::DefineNum("ResearchShielder",      RESEARCH_SHIELD);
    CBotProgram::DefineNum("ResearchPhazerShooter", RESEARCH_PHAZER);
    CBotProgram::DefineNum("ResearchLegged",        RESEARCH_iPAW);
    CBotProgram::DefineNum("ResearchOrgaShooter",   RESEARCH_iGUN);
    CBotProgram::DefineNum("ResearchRecycler",      RESEARCH_RECYCLER);
    CBotProgram::DefineNum("ResearchSubber",        RESEARCH_SUBM);
    CBotProgram::DefineNum("ResearchSniffer",       RESEARCH_SNIFFER);

    CBotProgram::DefineNum("PolskiPortalColobota", 1337);

    CBotClass* bc;

    // Add the class Point.
    bc = new CBotClass("point", NULL, true);  // intrinsic class
    bc->AddItem("x", CBotTypFloat);
    bc->AddItem("y", CBotTypFloat);
    bc->AddItem("z", CBotTypFloat);
    bc->AddFunction("point", CScriptFunctions::rPointConstructor, CScriptFunctions::cPointConstructor);

    // Adds the class Object.
    bc = new CBotClass("object", NULL);
    bc->AddItem("category",    CBotTypResult(CBotTypInt), PR_READ);
    bc->AddItem("position",    CBotTypResult(CBotTypClass, "point"), PR_READ);
    bc->AddItem("orientation", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("pitch",       CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("roll",        CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("energyLevel", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("shieldLevel", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("temperature", CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("altitude",    CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("lifeTime",    CBotTypResult(CBotTypFloat), PR_READ);
    bc->AddItem("energyCell",  CBotTypResult(CBotTypPointer, "object"), PR_READ);
    bc->AddItem("load",        CBotTypResult(CBotTypPointer, "object"), PR_READ);
    bc->AddItem("id",          CBotTypResult(CBotTypInt), PR_READ);
    bc->AddItem("team",        CBotTypResult(CBotTypInt), PR_READ);
    bc->AddItem("velocity",    CBotTypResult(CBotTypClass, "point"), PR_READ);
    bc->AddFunction("busy",     CScriptFunctions::rBusy,     CScriptFunctions::cBusy);
    bc->AddFunction("factory",  CScriptFunctions::rFactory,  CScriptFunctions::cFactory);
    bc->AddFunction("research", CScriptFunctions::rResearch, CScriptFunctions::cClassOneFloat);
    bc->AddFunction("takeoff",  CScriptFunctions::rTakeOff,  CScriptFunctions::cClassNull);
    bc->AddFunction("destroy",  CScriptFunctions::rDestroy,  CScriptFunctions::cClassNull);

    // InitClassFILE:
    // create a class for file management
    // the use is as follows:
    // file canal( "NomFichier.txt" )
    // canal.open( "r" );   // open for read
    // s = canal.readln( ); // reads a line
    // canal.close();   // close the file

    // create the class FILE
    bc    = new CBotClass("file", NULL);
    // adds the component ".filename"
    bc->AddItem("filename", CBotTypString);
    // adds the component ".handle"
    bc->AddItem("handle", CBotTypInt, PR_PRIVATE);

    // define a constructor and a destructor
    bc->AddFunction("file", CScriptFunctions::rfconstruct, CScriptFunctions::cfconstruct );
    bc->AddFunction("~file", CScriptFunctions::rfdestruct, NULL );

    // end of the methods associated
    bc->AddFunction("open", CScriptFunctions::rfopen, CScriptFunctions::cfopen );
    bc->AddFunction("close", CScriptFunctions::rfclose, CScriptFunctions::cfclose );
    bc->AddFunction("writeln", CScriptFunctions::rfwrite, CScriptFunctions::cfwrite );
    bc->AddFunction("readln", CScriptFunctions::rfread, CScriptFunctions::cfread );
    bc->AddFunction("eof", CScriptFunctions::rfeof, CScriptFunctions::cfeof );

    //m_pFuncFile = new CBotProgram( );
    //CBotStringArray ListFonctions;
    //m_pFuncFile->Compile( "public file openfile(string name, string mode) {return new file(name, mode);}", ListFonctions);
    //m_pFuncFile->SetIdent(-2);  // restoreState in special identifier for this function

    CBotProgram::AddFunction("sin",       rSin,       CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("cos",       rCos,       CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("tan",       rTan,       CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("asin",      raSin,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("acos",      raCos,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("atan",      raTan,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("atan2",     raTan2,     CScriptFunctions::cTwoFloat);
    CBotProgram::AddFunction("sqrt",      rSqrt,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("pow",       rPow,       CScriptFunctions::cTwoFloat);
    CBotProgram::AddFunction("rand",      rRand,      CScriptFunctions::cNull);
    CBotProgram::AddFunction("abs",       rAbs,       CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("floor",     rFloor,     CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("ceil",      rCeil,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("round",     rRound,     CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("trunc",     rTrunc,     CScriptFunctions::cOneFloat);

    CBotProgram::AddFunction("endmission",rEndMission,CScriptFunctions::cEndMission);
    CBotProgram::AddFunction("playmusic", rPlayMusic ,CScriptFunctions::cPlayMusic);
    CBotProgram::AddFunction("stopmusic", rStopMusic ,CScriptFunctions::cNull);

    CBotProgram::AddFunction("getbuild",          rGetBuild,          CScriptFunctions::cNull);
    CBotProgram::AddFunction("getresearchenable", rGetResearchEnable, CScriptFunctions::cNull);
    CBotProgram::AddFunction("getresearchdone",   rGetResearchDone,   CScriptFunctions::cNull);
    CBotProgram::AddFunction("setbuild",          rSetBuild,          CScriptFunctions::cOneInt);
    CBotProgram::AddFunction("setresearchenable", rSetResearchEnable, CScriptFunctions::cOneInt);
    CBotProgram::AddFunction("setresearchdone",   rSetResearchDone,   CScriptFunctions::cOneInt);

    CBotProgram::AddFunction("canbuild",        rCanBuild,        CScriptFunctions::cOneIntReturnBool);
    CBotProgram::AddFunction("canresearch",     rCanResearch,     CScriptFunctions::cOneIntReturnBool);
    CBotProgram::AddFunction("researched",      rResearched,      CScriptFunctions::cOneIntReturnBool);
    CBotProgram::AddFunction("buildingenabled", rBuildingEnabled, CScriptFunctions::cOneIntReturnBool);

    CBotProgram::AddFunction("build",           rBuild,           CScriptFunctions::cOneInt);

    CBotProgram::AddFunction("retobject", rGetObject, CScriptFunctions::cGetObject);
    CBotProgram::AddFunction("retobjectbyid", rGetObjectById, CScriptFunctions::cGetObject);
    CBotProgram::AddFunction("delete",    rDelete,    CScriptFunctions::cDelete);
    CBotProgram::AddFunction("search",    rSearch,    CScriptFunctions::cSearch);
    CBotProgram::AddFunction("radar",     rRadar,     CScriptFunctions::cRadar);
    CBotProgram::AddFunction("detect",    rDetect,    CScriptFunctions::cDetect);
    CBotProgram::AddFunction("direction", rDirection, CScriptFunctions::cDirection);
    CBotProgram::AddFunction("produce",   rProduce,   CScriptFunctions::cProduce);
    CBotProgram::AddFunction("distance",  rDistance,  CScriptFunctions::cDistance);
    CBotProgram::AddFunction("distance2d",rDistance2d,CScriptFunctions::cDistance);
    CBotProgram::AddFunction("space",     rSpace,     CScriptFunctions::cSpace);
    CBotProgram::AddFunction("flatspace", rFlatSpace, CScriptFunctions::cFlatSpace);
    CBotProgram::AddFunction("flatground",rFlatGround,CScriptFunctions::cFlatGround);
    CBotProgram::AddFunction("wait",      rWait,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("move",      rMove,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("turn",      rTurn,      CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("goto",      rGoto,      CScriptFunctions::cGoto);
    CBotProgram::AddFunction("grab",      rGrab,      CScriptFunctions::cGrabDrop);
    CBotProgram::AddFunction("drop",      rDrop,      CScriptFunctions::cGrabDrop);
    CBotProgram::AddFunction("sniff",     rSniff,     CScriptFunctions::cNull);
    CBotProgram::AddFunction("receive",   rReceive,   CScriptFunctions::cReceive);
    CBotProgram::AddFunction("send",      rSend,      CScriptFunctions::cSend);
    CBotProgram::AddFunction("deleteinfo",rDeleteInfo,CScriptFunctions::cDeleteInfo);
    CBotProgram::AddFunction("testinfo",  rTestInfo,  CScriptFunctions::cTestInfo);
    CBotProgram::AddFunction("thump",     rThump,     CScriptFunctions::cNull);
    CBotProgram::AddFunction("recycle",   rRecycle,   CScriptFunctions::cNull);
    CBotProgram::AddFunction("shield",    rShield,    CScriptFunctions::cShield);
    CBotProgram::AddFunction("fire",      rFire,      CScriptFunctions::cFire);
    CBotProgram::AddFunction("aim",       rAim,       CScriptFunctions::cAim);
    CBotProgram::AddFunction("motor",     rMotor,     CScriptFunctions::cMotor);
    CBotProgram::AddFunction("jet",       rJet,       CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("topo",      rTopo,      CScriptFunctions::cTopo);
    CBotProgram::AddFunction("message",   rMessage,   CScriptFunctions::cMessage);
    CBotProgram::AddFunction("cmdline",   rCmdline,   CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("ismovie",   rIsMovie,   CScriptFunctions::cNull);
    CBotProgram::AddFunction("errmode",   rErrMode,   CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("ipf",       rIPF,       CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("abstime",   rAbsTime,   CScriptFunctions::cNull);
    CBotProgram::AddFunction("deletefile",rDeleteFile,CScriptFunctions::cString);
    CBotProgram::AddFunction("pendown",   rPenDown,   CScriptFunctions::cPenDown);
    CBotProgram::AddFunction("penup",     rPenUp,     CScriptFunctions::cNull);
    CBotProgram::AddFunction("pencolor",  rPenColor,  CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("penwidth",  rPenWidth,  CScriptFunctions::cOneFloat);

    CBotProgram::AddFunction("camerafocus", rCameraFocus, CScriptFunctions::cOneObject);
}


// Updates the class Object.

void CScriptFunctions::uObject(CBotVar* botThis, void* user)
{
    CPhysics*   physics;
    CBotVar     *pVar, *pSub;
    ObjectType  type;
    Math::Vector    pos;
    float       value;

    if ( user == nullptr )  return;

    CObject* obj = static_cast<CObject*>(user);
    assert(obj->Implements(ObjectInterfaceType::Old));
    COldObject* object = static_cast<COldObject*>(obj);

    physics = object->GetPhysics();

    // Updates the object's type.
    pVar = botThis->GetItemList();  // "category"
    type = object->GetType();
    pVar->SetValInt(type, object->GetName());

    // Updates the position of the object.
    pVar = pVar->GetNext();  // "position"
    if (IsObjectBeingTransported(object))
    {
        pSub = pVar->GetItemList();  // "x"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
        pSub = pSub->GetNext();  // "y"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
        pSub = pSub->GetNext();  // "z"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
    }
    else
    {
        pos = object->GetPosition();
        float waterLevel = Gfx::CEngine::GetInstancePointer()->GetWater()->GetLevel();
        pos.y -= waterLevel;  // relative to sea level!
        pSub = pVar->GetItemList();  // "x"
        pSub->SetValFloat(pos.x/g_unit);
        pSub = pSub->GetNext();  // "y"
        pSub->SetValFloat(pos.z/g_unit);
        pSub = pSub->GetNext();  // "z"
        pSub->SetValFloat(pos.y/g_unit);
    }

    // Updates the angle.
    pos = object->GetRotation();
    pos += object->GetTilt();
    pVar = pVar->GetNext();  // "orientation"
    pVar->SetValFloat(Math::NormAngle(2*Math::PI - pos.y)*180.0f/Math::PI);
    pVar = pVar->GetNext();  // "pitch"
    pVar->SetValFloat(Math::NormAngle(pos.z)*180.0f/Math::PI);
    pVar = pVar->GetNext();  // "roll"
    pVar->SetValFloat(Math::NormAngle(pos.x)*180.0f/Math::PI);

    // Updates the energy level of the object.
    pVar = pVar->GetNext();  // "energyLevel"
    value = object->GetEnergy();
    pVar->SetValFloat(value);

    // Updates the shield level of the object.
    pVar = pVar->GetNext();  // "shieldLevel"
    value = object->GetShield();
    pVar->SetValFloat(value);

    // Updates the temperature of the reactor.
    pVar = pVar->GetNext();  // "temperature"
    if ( physics == 0 )  value = 0.0f;
    else                 value = 1.0f-physics->GetReactorRange();
    pVar->SetValFloat(value);

    // Updates the height above the ground.
    pVar = pVar->GetNext();  // "altitude"
    if ( physics == 0 )  value = 0.0f;
    else                 value = physics->GetFloorHeight();
    pVar->SetValFloat(value/g_unit);

    // Updates the lifetime of the object.
    pVar = pVar->GetNext();  // "lifeTime"
    value = object->GetAbsTime();
    pVar->SetValFloat(value);

    // Updates the type of battery.
    pVar = pVar->GetNext();  // "energyCell"
    if (object->Implements(ObjectInterfaceType::Powered))
    {
        CObject* power = dynamic_cast<CPoweredObject*>(object)->GetPower();
        if (power == nullptr)
        {
            pVar->SetPointer(nullptr);
        }
        else if (power->Implements(ObjectInterfaceType::Old))
        {
            pVar->SetPointer(dynamic_cast<COldObject*>(power)->GetBotVar());
        }
    }

    // Updates the transported object's type.
    pVar = pVar->GetNext();  // "load"
    if (object->Implements(ObjectInterfaceType::Carrier))
    {
        CObject* cargo = dynamic_cast<CCarrierObject*>(object)->GetCargo();
        if (cargo == nullptr)
        {
            pVar->SetPointer(nullptr);
        }
        else if (cargo->Implements(ObjectInterfaceType::Old))
        {
            pVar->SetPointer(dynamic_cast<COldObject*>(cargo)->GetBotVar());
        }
    }

    pVar = pVar->GetNext();  // "id"
    value = object->GetID();
    pVar->SetValInt(value);

    pVar = pVar->GetNext();  // "team"
    value = object->GetTeam();
    pVar->SetValInt(value);

    // Updates the velocity of the object.
    pVar = pVar->GetNext();  // "velocity"
    if (IsObjectBeingTransported(object) || physics == nullptr)
    {
        pSub = pVar->GetItemList();  // "x"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
        pSub = pSub->GetNext();  // "y"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
        pSub = pSub->GetNext();  // "z"
        pSub->SetInit(CBotVar::InitType::IS_NAN);
    }
    else
    {
        Math::Matrix matRotate;
        Math::LoadRotationZXYMatrix(matRotate, object->GetRotation());
        pos = physics->GetLinMotion(MO_CURSPEED);
        pos = Transform(matRotate, pos);

        pSub = pVar->GetItemList();  // "x"
        pSub->SetValFloat(pos.x/g_unit);
        pSub = pSub->GetNext();  // "y"
        pSub->SetValFloat(pos.z/g_unit);
        pSub = pSub->GetNext();  // "z"
        pSub->SetValFloat(pos.y/g_unit);
    }
}

CBotVar* CScriptFunctions::CreateObjectVar(CObject* obj)
{
    CBotClass* bc = CBotClass::Find("object");
    if ( bc != 0 )
    {
        bc->AddUpdateFunc(CScriptFunctions::uObject);
    }

    CBotVar* botVar = CBotVar::Create("", CBotTypResult(CBotTypClass, "object"));
    botVar->SetUserPtr(obj);
    botVar->SetIdent(obj->GetID());
    return botVar;
}

void CScriptFunctions::DestroyObjectVar(CBotVar* botVar, bool permanent)
{
    if ( botVar == nullptr ) return;

    botVar->SetUserPtr(OBJECTDELETED);
    if(permanent)
        delete botVar;
}
