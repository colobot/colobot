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


#include "app/app.h"

#include "common/config.h"
#include "common/pathman.h"
#include "common/resources/inputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "math/all.h"

#include "object/brain.h"
#include "object/object.h"
#include "object/objman.h"
#include "object/robotmain.h"
#include "object/task/taskmanager.h"

#include "object/auto/auto.h"
#include "object/auto/autofactory.h"
#include "object/auto/autobase.h"

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
    
    ended = static_cast<Error>(static_cast<int>(var->GetValFloat())); // TODO: is this correct?!
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
    result->SetValInt(g_build);
    return true;
}

// Instruction "getresearchenable()"

bool CScriptFunctions::rGetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(g_researchEnable);
    return true;
}

// Instruction "getresearchdone()"

bool CScriptFunctions::rGetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(g_researchDone);
    return true;
}

// Instruction "setbuild()"

bool CScriptFunctions::rSetBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    g_build = var->GetValInt();
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Instruction "setresearchenable()"

bool CScriptFunctions::rSetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    g_researchEnable = var->GetValInt();
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Instruction "setresearchdone()"

bool CScriptFunctions::rSetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    g_researchDone = var->GetValInt();
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


// Instruction "progfunc(funcname)".

bool CScriptFunctions::rProgFunc(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CBotString  cbs;
    const char* funcname;
    std::string program;
    
    cbs = var->GetValString();
    funcname = cbs;
    
    //TODO: Translation :)
    program  = "extern void object::Auto()\n{\n\t\n\t//Automatically generated by progfunc(\"";
    program += funcname;
    program += "\");\n\t";
    program += funcname;
    program += "();\n\t\n}\n";
    
    result->SetValString(program.c_str());
    
    return true;
}

// Compilation of instruction "object.busy()"
CBotTypResult CScriptFunctions::cBusy(CBotVar* thisclass, CBotVar* &var)
{
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "object.busy()"

bool CScriptFunctions::rBusy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
{
    exception = 0;
    
    CBotVar* classVars = thisclass->GetItemList();  // "category"
    classVars = classVars->GetNext();  // "position"
    classVars = classVars->GetNext();  // "orientation"
    classVars = classVars->GetNext();  // "pitch"
    classVars = classVars->GetNext();  // "roll"
    classVars = classVars->GetNext();  // "energyLevel"
    classVars = classVars->GetNext();  // "shieldLevel"
    classVars = classVars->GetNext();  // "temperature"
    classVars = classVars->GetNext();  // "altitude"
    classVars = classVars->GetNext();  // "lifeTime"
    classVars = classVars->GetNext();  // "material"
    classVars = classVars->GetNext();  // "energyCell"
    classVars = classVars->GetNext();  // "load"
    classVars = classVars->GetNext();  // "id"
    int rank = classVars->GetValInt();
    CObject* obj = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    CAuto* automat = obj->GetAuto();
    
    if ( automat != nullptr )
        result->SetValInt(automat->GetBusy());
    else
        exception = ERR_WRONG_OBJ;
    
    return true;
}

bool CScriptFunctions::rDestroy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
{
    exception = 0;
    Error err;
    
    CBotVar* classVars = thisclass->GetItemList();  // "category"
    ObjectType thisType = static_cast<ObjectType>(classVars->GetValInt());
    classVars = classVars->GetNext();  // "position"
    classVars = classVars->GetNext();  // "orientation"
    classVars = classVars->GetNext();  // "pitch"
    classVars = classVars->GetNext();  // "roll"
    classVars = classVars->GetNext();  // "energyLevel"
    classVars = classVars->GetNext();  // "shieldLevel"
    classVars = classVars->GetNext();  // "temperature"
    classVars = classVars->GetNext();  // "altitude"
    classVars = classVars->GetNext();  // "lifeTime"
    classVars = classVars->GetNext();  // "material"
    classVars = classVars->GetNext();  // "energyCell"
    classVars = classVars->GetNext();  // "load"
    classVars = classVars->GetNext();  // "id"
    int rank = classVars->GetValInt();
    CObject* obj = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    CAuto* automat = obj->GetAuto();
    
    if ( thisType == OBJECT_DESTROYER )
    {
        err = automat->StartAction(0);
    } else
        err = ERR_WRONG_OBJ;
    
    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        //TODO:        if ( script->m_errMode == ERM_STOP )
        if( true )
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

bool CScriptFunctions::rFactory(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
{
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
    
    CBotVar* classVars = thisclass->GetItemList();  // "category"
    ObjectType thisType = static_cast<ObjectType>(classVars->GetValInt());
    classVars = classVars->GetNext();  // "position"
    classVars = classVars->GetNext();  // "orientation"
    classVars = classVars->GetNext();  // "pitch"
    classVars = classVars->GetNext();  // "roll"
    classVars = classVars->GetNext();  // "energyLevel"
    classVars = classVars->GetNext();  // "shieldLevel"
    classVars = classVars->GetNext();  // "temperature"
    classVars = classVars->GetNext();  // "altitude"
    classVars = classVars->GetNext();  // "lifeTime"
    classVars = classVars->GetNext();  // "material"
    classVars = classVars->GetNext();  // "energyCell"
    classVars = classVars->GetNext();  // "load"
    classVars = classVars->GetNext();  // "id"
    int rank = classVars->GetValInt();
    CObject* factory = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    if (factory == nullptr) {
        exception = ERR_GENERIC;
        result->SetValInt(ERR_GENERIC);
        CLogger::GetInstancePointer()->Error("in object.factory() - factory is nullptr");
        return false;
    }
    
    if ( thisType == OBJECT_FACTORY )
    {
        CAutoFactory* automat = static_cast<CAutoFactory*>(factory->GetAuto());
        if(automat == nullptr) {
            exception = ERR_GENERIC;
            result->SetValInt(ERR_GENERIC);
            CLogger::GetInstancePointer()->Error("in object.factory() - automat is nullptr");
            return false;
        }
        
        bool bEnable = false;
        
        if ( type == OBJECT_MOBILEwa )
        {
            bEnable = true;
        }
        if ( type == OBJECT_MOBILEta )
        {
            bEnable = g_researchDone&RESEARCH_TANK;
        }
        if ( type == OBJECT_MOBILEfa )
        {
            bEnable = g_researchDone&RESEARCH_FLY;
        }
        if ( type == OBJECT_MOBILEia )
        {
            bEnable = g_researchDone&RESEARCH_iPAW;
        }
        
        if ( type == OBJECT_MOBILEws )
        {
            bEnable = g_researchDone&RESEARCH_SNIFFER;
        }
        if ( type == OBJECT_MOBILEts )
        {
            bEnable = ( (g_researchDone&RESEARCH_SNIFFER) &&
            (g_researchDone&RESEARCH_TANK)    );
        }
        if ( type == OBJECT_MOBILEfs )
        {
            bEnable = ( (g_researchDone&RESEARCH_SNIFFER) &&
            (g_researchDone&RESEARCH_FLY)     );
        }
        if ( type == OBJECT_MOBILEis )
        {
            bEnable = ( (g_researchDone&RESEARCH_SNIFFER) &&
            (g_researchDone&RESEARCH_iPAW)    );
        }
        
        if ( type == OBJECT_MOBILEwc )
        {
            bEnable = g_researchDone&RESEARCH_CANON;
        }
        if ( type == OBJECT_MOBILEtc )
        {
            bEnable = ( (g_researchDone&RESEARCH_CANON) &&
            (g_researchDone&RESEARCH_TANK)  );
        }
        if ( type == OBJECT_MOBILEfc )
        {
            bEnable = ( (g_researchDone&RESEARCH_CANON) &&
            (g_researchDone&RESEARCH_FLY)   );
        }
        if ( type == OBJECT_MOBILEic )
        {
            bEnable = ( (g_researchDone&RESEARCH_CANON) &&
            (g_researchDone&RESEARCH_iPAW)  );
        }
        
        if ( type == OBJECT_MOBILEwi )
        {
            bEnable = g_researchDone&RESEARCH_iGUN;
        }
        if ( type == OBJECT_MOBILEti )
        {
            bEnable = ( (g_researchDone&RESEARCH_iGUN) &&
            (g_researchDone&RESEARCH_TANK) );
        }
        if ( type == OBJECT_MOBILEfi )
        {
            bEnable = ( (g_researchDone&RESEARCH_iGUN) &&
            (g_researchDone&RESEARCH_FLY)  );
        }
        if ( type == OBJECT_MOBILEii )
        {
            bEnable = ( (g_researchDone&RESEARCH_iGUN) &&
            (g_researchDone&RESEARCH_iPAW) );
        }
        
        if ( type == OBJECT_MOBILErt )
        {
            bEnable = ( (g_researchDone&RESEARCH_THUMP) &&
            (g_researchDone&RESEARCH_TANK)  );
        }
        if ( type == OBJECT_MOBILErc )
        {
            bEnable = ( (g_researchDone&RESEARCH_PHAZER) &&
            (g_researchDone&RESEARCH_TANK)   );
        }
        if ( type == OBJECT_MOBILErr )
        {
            bEnable = ( (g_researchDone&RESEARCH_RECYCLER) &&
            (g_researchDone&RESEARCH_TANK)     );
        }
        if ( type == OBJECT_MOBILErs )
        {
            bEnable = ( (g_researchDone&RESEARCH_SHIELD) &&
            (g_researchDone&RESEARCH_TANK)   );
        }
        
        if ( type == OBJECT_MOBILEsa )
        {
            bEnable = g_researchDone&RESEARCH_SUBM;
        }
        
        if ( bEnable )
        {
            if ( automat != nullptr )
            {
                err = automat->StartAction(type);
                if ( err == ERR_OK ) automat->SetProgram(program);
            }
            else
                err = ERR_GENERIC;
        }
        else
            err = ERR_BUILD_DISABLED;
    }
    else
        err = ERR_WRONG_OBJ;
    
    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        //TODO:        if ( script->m_errMode == ERM_STOP )
        if( true )
        {
            exception = err;
            return false;
        }
        return true;
    }
    
    return true;
}

// Instruction "object.research(type)"

bool CScriptFunctions::rResearch(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
{
    Error       err;
    
    exception = 0;
    
    ResearchType type = static_cast<ResearchType>(var->GetValInt());
    
    CBotVar* classVars = thisclass->GetItemList();  // "category"
    ObjectType thisType = static_cast<ObjectType>(classVars->GetValInt());
    classVars = classVars->GetNext();  // "position"
    classVars = classVars->GetNext();  // "orientation"
    classVars = classVars->GetNext();  // "pitch"
    classVars = classVars->GetNext();  // "roll"
    classVars = classVars->GetNext();  // "energyLevel"
    classVars = classVars->GetNext();  // "shieldLevel"
    classVars = classVars->GetNext();  // "temperature"
    classVars = classVars->GetNext();  // "altitude"
    classVars = classVars->GetNext();  // "lifeTime"
    classVars = classVars->GetNext();  // "material"
    classVars = classVars->GetNext();  // "energyCell"
    classVars = classVars->GetNext();  // "load"
    classVars = classVars->GetNext();  // "id"
    int rank = classVars->GetValInt();
    CObject* center = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    CAuto* automat = center->GetAuto();
    
    if ( thisType == OBJECT_RESEARCH ||
        thisType == OBJECT_LABO      )
    {
        bool ok = false;
        if ( type == RESEARCH_iPAW       ||
            type == RESEARCH_iGUN        )
        {
            if ( thisType != OBJECT_LABO )
                err = ERR_WRONG_OBJ;
            else
                ok = true;
        }
        else
        {
            if ( thisType != OBJECT_RESEARCH )
                err = ERR_WRONG_OBJ;
            else
                ok = true;
        }
        if ( ok )
        {
            bool bEnable = ( g_researchEnable & type );
            if ( bEnable )
            {
                if ( automat != nullptr )
                {
                    err = automat->StartAction(type);
                }
                else
                    err = ERR_GENERIC;
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
        //TODO:        if ( script->m_errMode == ERM_STOP )
        if( true )
        {
            exception = err;
            return false;
        }
        return true;
    }
    
    return true;
}

// Instruction "object.takeoff()"

bool CScriptFunctions::rTakeOff(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
{
    Error       err;
    
    exception = 0;
    
    CBotVar* classVars = thisclass->GetItemList();  // "category"
    ObjectType thisType = static_cast<ObjectType>(classVars->GetValInt());
    classVars = classVars->GetNext();  // "position"
    classVars = classVars->GetNext();  // "orientation"
    classVars = classVars->GetNext();  // "pitch"
    classVars = classVars->GetNext();  // "roll"
    classVars = classVars->GetNext();  // "energyLevel"
    classVars = classVars->GetNext();  // "shieldLevel"
    classVars = classVars->GetNext();  // "temperature"
    classVars = classVars->GetNext();  // "altitude"
    classVars = classVars->GetNext();  // "lifeTime"
    classVars = classVars->GetNext();  // "material"
    classVars = classVars->GetNext();  // "energyCell"
    classVars = classVars->GetNext();  // "load"
    classVars = classVars->GetNext();  // "id"
    int rank = classVars->GetValInt();
    CObject* center = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    CAuto* automat = center->GetAuto();
    
    if ( thisType == OBJECT_BASE )
    {
        err = (static_cast<CAutoBase*>(automat))->TakeOff(false);
    } else
        err = ERR_WRONG_OBJ;
    
    if ( err != ERR_OK )
    {
        result->SetValInt(err);  // return error
        //TODO:        if ( script->m_errMode == ERM_STOP )
        if( true )
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
    CObject*    pObj;
    int         rank;
    int         exploType = 0;
    float       force = 1.0f;
    
    rank = var->GetValInt();
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
    
    pObj = static_cast<CObject*>(CObjectManager::GetInstancePointer()->GetObjectById(rank));
    if ( pObj == 0 )
    {
        return true;
    }
    else
    {
        if ( exploType )
        {
            pObj->ExploObject(static_cast<ExploType>(exploType), force);
        }
        else
        {
            pObj->DeleteObject(false);
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
    CObject*    pThis = static_cast<CObject *>(user);
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
    } else {
        pos = pThis->GetPosition(0);
    }
    
    std::vector<ObjectType> type_v;
    if(bArray)
    {
        while ( array != 0 )
        {
            type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
            array = array->GetNext();
        }
    } else {
        if(type != OBJECT_NULL)
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
    CObject*    pThis = static_cast<CObject *>(user);
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
    if(bArray)
    {
        while ( array != 0 )
        {
            type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
            array = array->GetNext();
        }
    } else {
        if(type != OBJECT_NULL)
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
    
    err = script->m_primaryTask->IsEnded();
    if ( err != ERR_CONTINUE )  // task terminated?
    {
        delete script->m_primaryTask;
        script->m_primaryTask = 0;
        
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
    
    script->m_primaryTask->EventProcess(script->m_event);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    CObject     *pBest;
    CBotVar*    array;
    int         type;
    bool        bArray = false;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
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
        if(bArray)
        {
            while ( array != 0 )
            {
                type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
                array = array->GetNext();
            }
        } else {
            if(type != OBJECT_NULL)
            {
                type_v.push_back(static_cast<ObjectType>(type));
            }
        }
        
        pBest = CObjectManager::GetInstancePointer()->Radar(pThis, type_v, 0.0f, 45.0f*Math::PI/180.0f, 0.0f, 20.0f, false, FILTER_NONE, true);
        
        pThis->StartDetectEffect(pBest, pBest != nullptr);
        
        if ( pBest == 0 )
        {
            script->m_returnValue = 0.0f;
        }
        else
        {
            script->m_returnValue = 1.0f;
        }
        
        script->m_primaryTask = new CTaskManager(script->m_object);
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
    CObject*        pThis = static_cast<CObject *>(user);
    Math::Vector    iPos, oPos;
    float           a, g;
    
    if ( !GetPoint(var, exception, oPos) )  return true;
    
    iPos = pThis->GetPosition(0);
    
    a = pThis->GetAngleY(0);
    g = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
    
    result->SetValFloat(-Math::Direction(a, g)*180.0f/Math::PI);
    return true;
}

// compilation of instruction "canbuild ( category );"

CBotTypResult CScriptFunctions::cCanBuild(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "canbuild ( category );"
// returns true if this building can be built

bool CScriptFunctions::rCanBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    ObjectType category  = static_cast<ObjectType>(var->GetValInt()); //get category parameter
    exception = 0;
    
    bool can = false;
    
    if ( (category == OBJECT_DERRICK   && (g_build & BUILD_DERRICK))   ||
        (category == OBJECT_FACTORY   && (g_build & BUILD_FACTORY))   ||
        (category == OBJECT_STATION   && (g_build & BUILD_STATION))   ||
        (category == OBJECT_CONVERT   && (g_build & BUILD_CONVERT))   ||
        (category == OBJECT_REPAIR    && (g_build & BUILD_REPAIR))    ||
        (category == OBJECT_TOWER     && (g_build & BUILD_TOWER))     ||
        (category == OBJECT_RESEARCH  && (g_build & BUILD_RESEARCH))  ||
        (category == OBJECT_RADAR     && (g_build & BUILD_RADAR))     ||
        (category == OBJECT_ENERGY    && (g_build & BUILD_ENERGY))    ||
        (category == OBJECT_LABO      && (g_build & BUILD_LABO))      ||
        (category == OBJECT_NUCLEAR   && (g_build & BUILD_NUCLEAR))   ||
        (category == OBJECT_INFO      && (g_build & BUILD_INFO))      ||
        (category == OBJECT_PARA      && (g_build & BUILD_PARA))      ||
        (category == OBJECT_DESTROYER && (g_build & BUILD_DESTROYER)))
    {
        
        // if we want to build not researched one
        if ( (category == OBJECT_TOWER   && !(g_researchDone & RESEARCH_TOWER)) ||
            (category == OBJECT_NUCLEAR && !(g_researchDone & RESEARCH_ATOMIC))
        )
        {
            can = false;
        }
        else
        {
            can = true;
        }
        
    }
    
    result->SetValInt(can);
    
    
    return true;
}

// Instruction "build(type)"
// draws error if can not build (wher errormode stop), otherwise 0 <- error

bool CScriptFunctions::rBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    ObjectType  oType;
    ObjectType  category;
    Error       err = ERR_BUILD_DISABLED;
    
    exception = 0;
    
    oType = pThis->GetType();
    
    if ( oType != OBJECT_MOBILEfa &&  // allowed only for grabber bots && humans
        oType != OBJECT_MOBILEta &&
        oType != OBJECT_MOBILEwa &&
        oType != OBJECT_MOBILEia &&
        oType != OBJECT_HUMAN &&
        oType != OBJECT_TECH)
    {
        err = ERR_MANIP_VEH; // Wrong object
    }
    else
    {
        category = static_cast<ObjectType>(var->GetValInt()); // get category parameter
        if ( (category == OBJECT_DERRICK   && (g_build & BUILD_DERRICK))   ||
            (category == OBJECT_FACTORY   && (g_build & BUILD_FACTORY))   ||
            (category == OBJECT_STATION   && (g_build & BUILD_STATION))   ||
            (category == OBJECT_CONVERT   && (g_build & BUILD_CONVERT))   ||
            (category == OBJECT_REPAIR    && (g_build & BUILD_REPAIR))    ||
            (category == OBJECT_TOWER     && (g_build & BUILD_TOWER))     ||
            (category == OBJECT_RESEARCH  && (g_build & BUILD_RESEARCH))  ||
            (category == OBJECT_RADAR     && (g_build & BUILD_RADAR))     ||
            (category == OBJECT_ENERGY    && (g_build & BUILD_ENERGY))    ||
            (category == OBJECT_LABO      && (g_build & BUILD_LABO))      ||
            (category == OBJECT_NUCLEAR   && (g_build & BUILD_NUCLEAR))   ||
            (category == OBJECT_INFO      && (g_build & BUILD_INFO))      ||
            (category == OBJECT_PARA      && (g_build & BUILD_PARA))      ||
            (category == OBJECT_DESTROYER && (g_build & BUILD_DESTROYER)))
        {
            
            // if we want to build not researched one
            if ( (category == OBJECT_TOWER   && !(g_researchDone & RESEARCH_TOWER)) ||
                (category == OBJECT_NUCLEAR && !(g_researchDone & RESEARCH_ATOMIC))
            )
            {
                err = ERR_BUILD_RESEARCH;
            }
            else
            {
                err = ERR_OK;
            }
            
        }
        
        if (pThis->GetIgnoreBuildCheck())
            err = ERR_OK;
        
        if (err == ERR_OK && script->m_primaryTask == 0) // if we can build and no task is present
        {
            script->m_primaryTask = new CTaskManager(script->m_object);
            err = script->m_primaryTask->StartTaskBuild(category);
            
            if (err != ERR_OK)
            {
                delete script->m_primaryTask;
                script->m_primaryTask = 0;
            }
        }
        // When script is waiting for finishing this task, it sets ERR_OK, and continues executing Process
        // without creating new task. I think, there was a problem with previous version in release configuration
        // It did not init error variable in this situation, and code tried to use variable with trash inside
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
        if( var != 0 )
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    object;
    CObject*    me = (static_cast<CObject *>(user));
    CBotString  cbs;
    const char* name;
    Math::Vector    pos;
    float       angle;
    ObjectType  type;
    float       power;
    
    if ( var->GetType() <= CBotTypDouble )
    {
        type = static_cast<ObjectType>(var->GetValInt());
        var = var->GetNext();
        
        pos = me->GetPosition(0);
        
        Math::Vector rotation = me->GetAngle(0) + me->GetInclinaison();
        angle = rotation.y;
        
        if( var != 0 )
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
        
        if ( var != 0 )
        {
            cbs = var->GetValString();
            name = cbs;
            var = var->GetNext();
            if ( var != 0 )
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
    
    if ( type == OBJECT_ANT    ||
        type == OBJECT_SPIDER ||
        type == OBJECT_BEE    ||
        type == OBJECT_WORM   )
    {
        CObject*    egg;
        
        object = new CObject();
        if ( !object->CreateInsect(pos, angle, type) )
        {
            delete object;
            result->SetValInt(1);  // error
            return true;
        }
        
        egg = new CObject();
        if ( !egg->CreateResource(pos, angle, OBJECT_EGG, 0.0f) )
        {
            delete egg;
        }
        object->SetActivity(false);
    } else {
        if ((type == OBJECT_POWER || type == OBJECT_ATOMIC) && power == -1.0f) power = 1.0f;
        object = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type, power);
        if ( object == nullptr )
        {
            result->SetValInt(1);  // error
            return true;
        }
        script->m_main->CreateShortcuts();
    }
    
    if (name[0] != 0)
    {
        std::string name2 = CPathManager::InjectLevelDir(name, "ai");
        CBrain* brain = object->GetBrain();
        if(brain != nullptr)
        {
            Program* program = brain->AddProgram();
            brain->ReadProgram(program, name2.c_str());
            brain->RunProgram(program);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    CBotVar*    pSub;
    Math::Vector    center;
    float       rMin, rMax, dist;
    
    rMin = 10.0f*g_unit;
    rMax = 50.0f*g_unit;
    dist =  4.0f*g_unit;
    
    if ( var == 0 )
    {
        center = pThis->GetPosition(0);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       value;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        value = var->GetValFloat();
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

bool CScriptFunctions::rMove(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       value;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        value = var->GetValFloat();
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

bool CScriptFunctions::rTurn(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       value;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        value = var->GetValFloat();
        err = script->m_primaryTask->StartTaskTurn(-value*Math::PI/180.0f);
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
    CScript*        script = (static_cast<CObject *>(user))->GetRunScript();
    Math::Vector        pos;
    TaskGotoGoal    goal;
    TaskGotoCrash   crash;
    float           altitude;
    Error           err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    ObjectType  oType;
    TaskManipArm type;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
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

bool CScriptFunctions::rDrop(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    ObjectType  oType;
    TaskManipArm type;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        if ( var == 0 )  type = TMA_FFRONT;
        else             type = static_cast<TaskManipArm>(var->GetValInt());
        
        oType = pThis->GetType();
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

bool CScriptFunctions::rSniff(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    CBotString  cbs;
    Error       err;
    const char* p;
    float       value, power;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        
        cbs = var->GetValString();
        p = cbs;
        var = var->GetNext();
        
        power = 10.0f*g_unit;
        if ( var != 0 )
        {
            power = var->GetValFloat()*g_unit;
            var = var->GetNext();
        }
        
        err = script->m_primaryTask->StartTaskInfo(static_cast<const char*>(p), 0.0f, power, false);
        if ( err != ERR_OK )
        {
            delete script->m_primaryTask;
            script->m_primaryTask = 0;
            result->SetInit(IS_NAN);
            return true;
        }
    }
    if ( !Process(script, result, exception) )  return false;  // not finished
    
    value = pThis->GetInfoReturn();
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CBotString  cbs;
    Error       err;
    const char* p;
    float       value, power;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        
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
        
        err = script->m_primaryTask->StartTaskInfo(static_cast<const char*>(p), value, power, true);
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

CObject* CScriptFunctions::SearchInfo(CScript* script, CObject* object, float power)
{
    CObject     *pBest;
    Math::Vector    iPos, oPos;
    
    iPos = object->GetPosition(0);
    pBest = CObjectManager::GetInstancePointer()->Radar(object, OBJECT_INFO);
    if(pBest == nullptr)
        return nullptr;
    oPos = object->GetPosition(0);
    
    if(Math::DistanceProjected(iPos, oPos) > power)
        return nullptr;
    
    return pBest;
}

// Compilation of the instruction "deleteinfo(nom, power)".

CBotTypResult CScriptFunctions::cDeleteInfo(CBotVar* &var, void* user)
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

// Instruction "deleteinfo(nom, power)".

bool CScriptFunctions::rDeleteInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    CObject*    pInfo;
    CBotString  cbs;
    Info        info;
    const char* p;
    float       power;
    int         i, total;
    
    exception = 0;
    
    cbs = var->GetValString();
    p = cbs;
    var = var->GetNext();
    
    power = 10.0f*g_unit;
    if ( var != 0 )
    {
        power = var->GetValFloat()*g_unit;
        var = var->GetNext();
    }
    
    pInfo = SearchInfo(script, pThis, power);
    if ( pInfo == 0 )
    {
        result->SetValFloat(0.0f);  // false
        return true;
    }
    
    total = pInfo->GetInfoTotal();
    for ( i=0 ; i<total ; i++ )
    {
        info = pInfo->GetInfo(i);
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

// Instruction "testinfo(nom, power)".

bool CScriptFunctions::rTestInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    CObject*    pInfo;
    CBotString  cbs;
    Info        info;
    const char* p;
    float       power;
    int         i, total;
    
    exception = 0;
    
    cbs = var->GetValString();
    p = cbs;
    var = var->GetNext();
    
    power = 10.0f*g_unit;
    if ( var != 0 )
    {
        power = var->GetValFloat()*g_unit;
        var = var->GetNext();
    }
    
    pInfo = SearchInfo(script, pThis, power);
    if ( pInfo == 0 )
    {
        result->SetValInt(false);
        return true;
    }
    
    total = pInfo->GetInfoTotal();
    for ( i=0 ; i<total ; i++ )
    {
        info = pInfo->GetInfo(i);
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

bool CScriptFunctions::rThump(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
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

bool CScriptFunctions::rRecycle(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    float       oper, radius;
    Error       err;
    
    oper = var->GetValFloat();  // 0=down, 1=up
    var = var->GetNext();
    
    radius = var->GetValFloat();
    if ( radius < 10.0f )  radius = 10.0f;
    if ( radius > 25.0f )  radius = 25.0f;
    radius = (radius-10.0f)/15.0f;
    
    if ( *script->m_secondaryTask == 0 )  // shield folds?
    {
        if ( oper == 0.0f )  // down?
        {
            result->SetValInt(1);  // shows the error
        }
        else    // up ?
        {
            pThis->SetParam(radius);
            
            *script->m_secondaryTask = new CTaskManager(script->m_object);
            err = (*script->m_secondaryTask)->StartTaskShield(TSM_UP, 1000.0f);
            if ( err != ERR_OK )
            {
                delete *script->m_secondaryTask;
                *script->m_secondaryTask = 0;
                result->SetValInt(err);  // shows the error
            }
        }
    }
    else    // shield deployed?
    {
        if ( oper == 0.0f )  // down?
        {
            (*script->m_secondaryTask)->StartTaskShield(TSM_DOWN, 0.0f);
        }
        else    // up?
        {
            //?         result->SetValInt(1);  // shows the error
            pThis->SetParam(radius);
            (*script->m_secondaryTask)->StartTaskShield(TSM_UPDATE, 0.0f);
        }
    }
    
    return true;
}

// Compilation "fire(delay)".

CBotTypResult CScriptFunctions::cFire(CBotVar* &var, void* user)
{
    CObject*    pThis = static_cast<CObject *>(user);
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    float       delay;
    Math::Vector    impact;
    Error       err;
    ObjectType  type;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        
        type = pThis->GetType();
        
        if ( type == OBJECT_ANT )
        {
            if ( !GetPoint(var, exception, impact) )  return true;
            impact.y += pThis->GetWaterLevel();
            err = script->m_primaryTask->StartTaskFireAnt(impact);
        }
        else if ( type == OBJECT_SPIDER )
        {
            err = script->m_primaryTask->StartTaskSpiderExplo();
        }
        else
        {
            if ( var == 0 )  delay = 0.0f;
            else             delay = var->GetValFloat();
            if ( delay < 0.0f ) delay = -delay;
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       x, y;
    Error       err;
    
    exception = 0;
    
    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        script->m_primaryTask = new CTaskManager(script->m_object);
        x = var->GetValFloat();
        var = var->GetNext();
        var == 0 ? y=0.0f : y=var->GetValFloat();
        err = script->m_primaryTask->StartTaskGunGoal(x*Math::PI/180.0f, y*Math::PI/180.0f);
        if ( err == ERR_AIM_IMPOSSIBLE )
        {
            result->SetValInt(err);  // shows the error
        }
        else if ( err != ERR_OK )
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
    CObject*    pThis = static_cast<CObject *>(user);
    CPhysics*   physics = (static_cast<CObject *>(user))->GetPhysics();
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
    CPhysics*   physics = (static_cast<CObject *>(user))->GetPhysics();
    float       value;
    
    value = var->GetValFloat();
    if( value > 1.0f ) value = 1.0f;
    
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
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
    script->m_main->CheckEndMessage(p);
    
    return true;
}

// Instruction "cmdline(rank)".

bool CScriptFunctions::rCmdline(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CObject *>(user);
    float       value;
    int         rank;
    
    rank = var->GetValInt();
    value = pThis->GetCmdLine(rank);
    result->SetValFloat(value);
    
    return true;
}

// Instruction "ismovie()".

bool CScriptFunctions::rIsMovie(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       value;
    
    value = script->m_main->GetMovieLock()?1.0f:0.0f;
    result->SetValFloat(value);
    
    return true;
}

// Instruction "errmode(mode)".

bool CScriptFunctions::rErrMode(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       value;
    
    value = script->m_main->GetGameTime();
    result->SetValFloat(value);
    return true;
}


// Prepares a file name.

void PrepareFilename(CBotString &filename, const char *dir)
{
    int         pos;
    
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

bool CScriptFunctions::rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CBotString  cbs;
    const char* filename;
    const char* dir;
    
    cbs = var->GetValString();
    dir = script->m_main->GetFilesDir();
    PrepareFilename(cbs, dir);
    filename = cbs;
    //std function that removes file.
    return (!remove(filename));
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
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    int         color;
    float       width;
    Error       err;
    
    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        exception = 0;
        
        if ( script->m_primaryTask == 0 )  // no task in progress?
        {
            if ( var != 0 )
            {
                color = var->GetValInt();
                if ( color <  0 )  color =  0;
                if ( color > 17 )  color = 17;
                pThis->SetTraceColor(color);
                
                var = var->GetNext();
                if ( var != 0 )
                {
                    width = var->GetValFloat();
                    if ( width < 0.1f )  width = 0.1f;
                    if ( width > 1.0f )  width = 1.0f;
                    pThis->SetTraceWidth(width);
                }
            }
            pThis->SetTraceDown(true);
            
            script->m_primaryTask = new CTaskManager(script->m_object);
            err = script->m_primaryTask->StartTaskPen(pThis->GetTraceDown(), pThis->GetTraceColor());
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
            color = var->GetValInt();
            if ( color <  0 )  color =  0;
            if ( color > 17 )  color = 17;
            pThis->SetTraceColor(color);
            
            var = var->GetNext();
            if ( var != 0 )
            {
                width = var->GetValFloat();
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

bool CScriptFunctions::rPenUp(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    Error       err;
    
    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        exception = 0;
        
        if ( script->m_primaryTask == 0 )  // no task in progress?
        {
            pThis->SetTraceDown(false);
            
            script->m_primaryTask = new CTaskManager(script->m_object);
            err = script->m_primaryTask->StartTaskPen(pThis->GetTraceDown(), pThis->GetTraceColor());
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

bool CScriptFunctions::rPenColor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    int         color;
    Error       err;
    
    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        exception = 0;
        
        if ( script->m_primaryTask == 0 )  // no task in progress?
        {
            color = var->GetValInt();
            if ( color <  0 )  color =  0;
            if ( color > 17 )  color = 17;
            pThis->SetTraceColor(color);
            
            script->m_primaryTask = new CTaskManager(script->m_object);
            err = script->m_primaryTask->StartTaskPen(pThis->GetTraceDown(), pThis->GetTraceColor());
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
        color = var->GetValInt();
        if ( color <  0 )  color =  0;
        if ( color > 17 )  color = 17;
        pThis->SetTraceColor(color);
        
        return true;
    }
}

// Instruction "penwidth()".

bool CScriptFunctions::rPenWidth(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CObject *>(user);
    float       width;
    
    width = var->GetValFloat();
    if ( width < 0.1f )  width = 0.1f;
    if ( width > 1.0f )  width = 1.0f;
    pThis->SetTraceWidth(width);
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
    CScript* script = (static_cast<CObject *>(user))->GetRunScript();
    
    CBotVar* classVars = var->GetItemList();  // "category"
    classVars = classVars->GetNext();  // "position"
    classVars = classVars->GetNext();  // "orientation"
    classVars = classVars->GetNext();  // "pitch"
    classVars = classVars->GetNext();  // "roll"
    classVars = classVars->GetNext();  // "energyLevel"
    classVars = classVars->GetNext();  // "shieldLevel"
    classVars = classVars->GetNext();  // "temperature"
    classVars = classVars->GetNext();  // "altitude"
    classVars = classVars->GetNext();  // "lifeTime"
    classVars = classVars->GetNext();  // "material"
    classVars = classVars->GetNext();  // "energyCell"
    classVars = classVars->GetNext();  // "load"
    classVars = classVars->GetNext();  // "id"
    int rank = classVars->GetValInt();
    CObject* object = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    
    script->m_main->SelectObject(object, false);
    
    result->SetValInt(ERR_OK);
    exception = ERR_OK;
    return true;
}

// Static variables

int                                 CScriptFunctions::m_CompteurFileOpen = 0;
std::string                         CScriptFunctions::m_filesDir;
std::unordered_map<int, FILE*>      CScriptFunctions::m_files;
int                                 CScriptFunctions::m_nextFile = 1;



// Prepares a file name.

void PrepareFilename(CBotString &filename)
{
    int pos = filename.ReverseFind('/');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // Remove files with /
    }
    
    pos = filename.ReverseFind(':');
    if ( pos > 0 )
    {
        filename = filename.Mid(pos+1);  // also removes the drive letter C:
    }
    
    #if PLATFORM_WINDOWS
    boost::filesystem::create_directories(CSystemUtilsWindows::UTF8_Decode(CScriptFunctions::m_filesDir));
    #else
    boost::filesystem::create_directories(CScriptFunctions::m_filesDir);
    #endif
    filename = CBotString(CScriptFunctions::m_filesDir.c_str()) + CBotString("/") + filename;
    CLogger::GetInstancePointer()->Debug("CBot accessing file '%s'\n", static_cast<const char*>(filename));
}


// constructor of the class
// get the filename as a parameter

// execution
bool CScriptFunctions::rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    CBotString  mode;
    
    // accepts no parameters
    if ( pVar == NULL ) return true;
    
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
    
    if ( ! mode.IsEmpty() )
    {
        // opens the requested file
        FILE*   pFile = fopen( filename, mode );
        if ( pFile == NULL ) { Exception = CBotErrFileOpen; return false; }
        
        m_CompteurFileOpen ++;

        int fileHandle = m_nextFile++;

        m_files[fileHandle] = pFile;
        
        // save the file handle
        pVar = pThis->GetItem("handle");
        pVar->SetValInt(fileHandle);
    }
    
    return true;
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
bool CScriptFunctions::rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    
    // don't open? no problem :)
    if ( pVar->GetInit() != IS_DEF) return true;

    int fileHandle = pVar->GetValInt();
    
    FILE* pFile = m_files[fileHandle];
    fclose(pFile);
    m_CompteurFileOpen--;
    
    pVar->SetInit(IS_NAN);

    m_files.erase(fileHandle);
    
    return true;
}


// process FILE :: open
// get the r/w mode as a parameter

// execution
bool CScriptFunctions::rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return false; }
    
    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }
    
    // There may be a second parameter
    if ( pVar->GetNext() != NULL )
    {
        // if the first parameter is the file name
        CBotString  filename = pVar->GetValString();
        PrepareFilename(filename);
        
        // saves the file name
        CBotVar* pVar2 = pThis->GetItem("filename");
        pVar2->SetValString(filename);
        
        // next parameter is the mode
        pVar = pVar -> GetNext();
    }
    
    CBotString  mode = pVar->GetValString();
    if ( mode != "r" && mode != "w" ) { Exception = CBotErrBadParam; return false; }
    
    // no third parameter
    if ( pVar->GetNext() != NULL ) { Exception = CBotErrOverParam; return false; }
    
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    
    // which must not be initialized
    if ( pVar->GetInit() == IS_DEF) { Exception = CBotErrFileOpen; return false; }
    
    // file contains the name
    pVar = pThis->GetItem("filename");
    CBotString  filename = pVar->GetValString();
    
    PrepareFilename(filename);  // if the name was h.filename attribute = "...";
    
    // opens the requested file
    FILE*   pFile = fopen( filename, mode );
    if ( pFile == NULL )
    {
        pResult->SetValInt(false);
        return true;
    }
    
    m_CompteurFileOpen ++;
    
    // save file handle
    int fileHandle = m_nextFile++;

    m_files[fileHandle] = pFile;

    pVar = pThis->GetItem("handle");
    pVar->SetValInt(fileHandle);
    
    pResult->SetValInt(true);
    return true;
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
bool CScriptFunctions::rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it shouldn't be any parameters
    if (pVar != NULL) { Exception = CBotErrOverParam; return false; }
    
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    
    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }
    
    int fileHandle = pVar->GetValInt();

    FILE* pFile = m_files[fileHandle];
    fclose(pFile);
    m_CompteurFileOpen--;
    
    pVar->SetInit(IS_NAN);

    m_files.erase(fileHandle);
    
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
bool CScriptFunctions::rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // there must be a parameter
    if ( pVar == NULL ) { Exception = CBotErrLowParam; return false; }
    
    // which must be a character string
    if ( pVar->GetType() != CBotTypString ) { Exception = CBotErrBadString; return false; }
    
    CBotString param = pVar->GetValString();
    
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    
    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }
    
    int fileHandle = pVar->GetValInt();

    FILE* pFile = m_files[fileHandle];
    
    int res = fputs(param+CBotString("\n"), pFile);
    
    // if an error occurs generate an exception
    if ( res < 0 ) { Exception = CBotErrWrite; return false; }
    
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
bool CScriptFunctions::rfread(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it shouldn't be any parameters
    if (pVar != NULL) { Exception = CBotErrOverParam; return false; }

    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");

    if (pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }

    int fileHandle = pVar->GetValInt();

    FILE* pFile = m_files[fileHandle];

    char    chaine[2000];
    int     i;
    for (i = 0; i < 2000; i++) chaine[i] = 0;

    if (fgets(chaine, 1999, pFile) != nullptr)
    {
        for (i = 0; i < 2000; i++) if (chaine[i] == '\n') chaine[i] = 0;
    }

    // if an error occurs generate an exception
    if ( ferror(pFile) ) { Exception = CBotErrRead; return false; }
    
    pResult->SetValString( chaine );
    
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
bool CScriptFunctions::rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    // it should not be any parameter
    if ( pVar != NULL ) { Exception = CBotErrOverParam; return false; }
    
    // retrieve the item "handle"
    pVar = pThis->GetItem("handle");
    
    if ( pVar->GetInit() != IS_DEF) { Exception = CBotErrNotOpen; return false; }
    
    int fileHandle = pVar->GetValInt();

    FILE* pFile = m_files[fileHandle];
    
    pResult->SetValInt( feof( pFile ) );
    
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

bool CScriptFunctions::rPointConstructor(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception)
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
    
    CBotProgram::DefineNum("White",      0);
    CBotProgram::DefineNum("Black",      1);
    CBotProgram::DefineNum("Gray",       2);
    CBotProgram::DefineNum("LightGray",  3);
    CBotProgram::DefineNum("Red",        4);
    CBotProgram::DefineNum("Pink",       5);
    CBotProgram::DefineNum("Purple",     6);
    CBotProgram::DefineNum("Orange",     7);
    CBotProgram::DefineNum("Yellow",     8);
    CBotProgram::DefineNum("Beige",      9);
    CBotProgram::DefineNum("Brown",      10);
    CBotProgram::DefineNum("Skin",       11);
    CBotProgram::DefineNum("Green",      12);
    CBotProgram::DefineNum("LightGreen", 13);
    CBotProgram::DefineNum("Blue",       14);
    CBotProgram::DefineNum("LightBlue",  15);
    CBotProgram::DefineNum("BlackArrow", 16);
    CBotProgram::DefineNum("RedArrow",   17);
    
    CBotProgram::DefineNum("Metal",   OM_METAL);
    CBotProgram::DefineNum("Plastic", OM_PLASTIC);
    
    CBotProgram::DefineNum("InFront",    TMA_FFRONT);
    CBotProgram::DefineNum("Behind",     TMA_FBACK);
    CBotProgram::DefineNum("EnergyCell", TMA_POWER);
    
    CBotProgram::DefineNum("DisplayError",   Ui::TT_ERROR);
    CBotProgram::DefineNum("DisplayWarning", Ui::TT_WARNING);
    CBotProgram::DefineNum("DisplayInfo",    Ui::TT_INFO);
    CBotProgram::DefineNum("DisplayMessage", Ui::TT_MESSAGE);
    
    CBotProgram::DefineNum("FilterNone",        FILTER_NONE);
    CBotProgram::DefineNum("FilterOnlyLanding", FILTER_ONLYLANDING);
    CBotProgram::DefineNum("FilterOnlyFliying", FILTER_ONLYFLYING);
    
    CBotProgram::DefineNum("ExploNone",  0);
    CBotProgram::DefineNum("ExploBoum",  EXPLO_BOUM);
    CBotProgram::DefineNum("ExploBurn",  EXPLO_BURN);
    CBotProgram::DefineNum("ExploWater", EXPLO_WATER);
    
    CBotProgram::DefineNum("ResultNotEnded",  ERR_MISSION_NOTERM);
    CBotProgram::DefineNum("ResultLost",      INFO_LOST);
    CBotProgram::DefineNum("ResultLostQuick", INFO_LOSTq);
    CBotProgram::DefineNum("ResultWin",       ERR_OK);
    
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
    bc->AddItem("material",    CBotTypResult(CBotTypInt), PR_READ);
    bc->AddItem("energyCell",  CBotTypResult(CBotTypPointer, "object"), PR_READ);
    bc->AddItem("load",        CBotTypResult(CBotTypPointer, "object"), PR_READ);
    bc->AddItem("id",          CBotTypResult(CBotTypInt), PR_READ);
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
    CBotProgram::AddFunction("setbuild",          rSetBuild,          CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("setresearchenable", rSetResearchEnable, CScriptFunctions::cOneFloat);
    CBotProgram::AddFunction("setresearchdone",   rSetResearchDone,   CScriptFunctions::cOneFloat);
    
    CBotProgram::AddFunction("progfunc",  rProgFunc,  CScriptFunctions::cStringString);
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
    
    CBotProgram::AddFunction("canbuild", rCanBuild, CScriptFunctions::cCanBuild);
    CBotProgram::AddFunction("build", rBuild, CScriptFunctions::cOneFloat);
}