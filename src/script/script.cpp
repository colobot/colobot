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


#include "script/script.h"

#include "app/app.h"
#include "app/gamedata.h"

#include "common/global.h"
#include "common/iman.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"
#include "graphics/engine/text.h"

#include "math/geometry.h"
#include "math/vector.h"

#include "object/object.h"
#include "object/robotmain.h"
#include "object/task/taskmanager.h"
#include "object/objman.h"

#include "object/auto/auto.h"
#include "object/auto/autofactory.h"
#include "object/auto/autobase.h"

#include "physics/physics.h"

#include "script/cbottoken.h"

#include "sound/sound.h"

#include "ui/interface.h"
#include "ui/edit.h"
#include "ui/list.h"
#include "ui/displaytext.h"


#include <stdio.h>



const int CBOT_IPF = 100;       // CBOT: number of instructions / frame

const int ERM_CONT = 0;     // if error -> continue
const int ERM_STOP = 1;     // if error -> stop




// Compiling a procedure without any parameters.

CBotTypResult CScript::cNull(CBotVar* &var, void* user)
{
    if ( var != 0 )  return CBotErrOverParam;
    return CBotTypResult(CBotTypFloat);
}

CBotTypResult CScript::cClassNull(CBotVar* thisclass, CBotVar* &var)
{
    return CScript::cNull(var, nullptr);
}

// Compiling a procedure with a single real number.

CBotTypResult CScript::cOneFloat(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

CBotTypResult CScript::cClassOneFloat(CBotVar* thisclass, CBotVar* &var)
{
    return CScript::cOneFloat(var, nullptr);
}

// Compiling a procedure with two real numbers.

CBotTypResult CScript::cTwoFloat(CBotVar* &var, void* user)
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

CBotTypResult CScript::cPoint(CBotVar* &var, void* user)
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

CBotTypResult CScript::cOnePoint(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single string.

CBotTypResult CScript::cString(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
         var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Compiling a procedure with a single string, returning string.

CBotTypResult CScript::cStringString(CBotVar* &var, void* user)
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

bool CScript::rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sinf(value*Math::PI/180.0f));
    return true;
}

// Instruction "cos(degrees)".

bool CScript::rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(cosf(value*Math::PI/180.0f));
    return true;
}

// Instruction "tan(degrees)".

bool CScript::rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
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

// Instruction "sqrt(value)".

bool CScript::rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sqrtf(value));
    return true;
}

// Instruction "pow(x, y)".

bool CScript::rPow(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   x, y;

    x = var->GetValFloat();
    var = var->GetNext();
    y = var->GetValFloat();
    result->SetValFloat(powf(x, y));
    return true;
}

// Instruction "rand()".

bool CScript::rRand(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValFloat(Math::Rand());
    return true;
}

// Instruction "abs()".

bool CScript::rAbs(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(fabs(value));
    return true;
}

// Compilation of the instruction "endmission(result, delay)"

CBotTypResult CScript::cEndMission(CBotVar* &var, void* user)
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

bool CScript::rEndMission(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    Error ended;
    float delay;

    ended = static_cast<Error>(var->GetValFloat());
    var = var->GetNext();

    delay = var->GetValFloat();

    CRobotMain::GetInstancePointer()->SetEndMission(ended, delay);
    return true;
}

// Compilation of the instruction "playmusic(filename, repeat)"

CBotTypResult CScript::cPlayMusic(CBotVar* &var, void* user)
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

bool CScript::rPlayMusic(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rStopMusic(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CApplication::GetInstancePointer()->GetSound()->StopMusic();
    return true;
}

// Instruction "getbuild()"

bool CScript::rGetBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(g_build);
    return true;
}

// Instruction "getresearchenable()"

bool CScript::rGetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(g_researchEnable);
    return true;
}

// Instruction "getresearchdone()"

bool CScript::rGetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValInt(g_researchDone);
    return true;
}

// Instruction "setbuild()"

bool CScript::rSetBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    g_build = var->GetValInt();
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Instruction "setresearchenable()"

bool CScript::rSetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    g_researchEnable = var->GetValInt();
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Instruction "setresearchdone()"

bool CScript::rSetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    g_researchDone = var->GetValInt();
    CApplication::GetInstancePointer()->GetEventQueue()->AddEvent(Event(EVENT_UPDINTERFACE));
    return true;
}

// Compilation of the instruction "retobject(rank)".

CBotTypResult CScript::cGetObject(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "retobjectbyid(rank)".

bool CScript::rGetObjectById(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pObj;
    int         rank;

    rank = var->GetValInt();

    pObj = static_cast<CObject*>(CObjectManager::GetInstancePointer()->SearchInstance(rank));
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

bool CScript::rGetObject(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pObj;
    int         rank;

    rank = var->GetValInt();

    pObj = static_cast<CObject*>(CInstanceManager::GetInstancePointer()->SearchInstance(CLASS_OBJECT, rank));
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


// Instruction "progfunc(funcname)".

bool CScript::rProgFunc(CBotVar* var, CBotVar* result, int& exception, void* user)
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
CBotTypResult CScript::cBusy(CBotVar* thisclass, CBotVar* &var)
{
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "object.busy()"

bool CScript::rBusy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
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
    CObject* obj = CObjectManager::GetInstancePointer()->SearchInstance(rank);
    CAuto* automat = obj->GetAuto();

    if ( automat != nullptr )
        result->SetValInt(automat->GetBusy());
    else
        exception = ERR_WRONG_OBJ;

    return true;
}

bool CScript::rDestroy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
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
    CObject* obj = CObjectManager::GetInstancePointer()->SearchInstance(rank);
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

CBotTypResult CScript::cFactory(CBotVar* thisclass, CBotVar* &var)
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

bool CScript::rFactory(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
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
    CObject* factory = CObjectManager::GetInstancePointer()->SearchInstance(rank);
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

bool CScript::rResearch(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
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
    CObject* center = CObjectManager::GetInstancePointer()->SearchInstance(rank);
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

bool CScript::rTakeOff(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception)
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
    CObject* center = CObjectManager::GetInstancePointer()->SearchInstance(rank);
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

CBotTypResult CScript::cDelete(CBotVar* &var, void* user)
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

bool CScript::rDelete(CBotVar* var, CBotVar* result, int& exception, void* user)
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

    pObj = static_cast<CObject*>(CObjectManager::GetInstancePointer()->SearchInstance(rank));
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

CBotTypResult CScript::cSearch(CBotVar* &var, void* user)
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

bool CScript::rSearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject     *pObj, *pBest;
    CBotVar*    array;
    Math::Vector    pos, oPos;
    bool        bNearest = false;
    bool        bArray;
    float       min, dist;
    int         type, oType, i;

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
        bNearest = true;
    }

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    min = 100000.0f;
    pBest = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( pObj->GetTruck() != 0 )  continue;  // object transported?
        if ( !pObj->GetActif() )  continue;

        oType = pObj->GetType();
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
            oPos = pObj->GetPosition(0);
            dist = Math::DistanceProjected(pos, oPos);
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
        result->SetPointer(pBest->GetBotVar());
    }
    return true;
}


// Compilation of instruction "radar(type, angle, focus, min, max, sens)".

CBotTypResult CScript::cRadar(CBotVar* &var, void* user)
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

bool CScript::rRadar(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CObject *>(user);
    CObject     *pObj, *pBest;
    CPhysics*   physics;
    CBotVar*    array;
    Math::Vector    iPos, oPos;
    RadarFilter filter;
    float       best, minDist, maxDist, sens, iAngle, angle, focus, d, a;
    int         type, oType, i;
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
                    minDist = var->GetValFloat()*g_unit;

                    var = var->GetNext();
                    if ( var != 0 )
                    {
                        maxDist = var->GetValFloat()*g_unit;

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

    iPos   = pThis->GetPosition(0);
    iAngle = pThis->GetAngleY(0)+angle;
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    if ( sens >= 0.0f )  best = 100000.0f;
    else                 best = 0.0f;
    pBest = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;
        if ( pObj == pThis )  continue;

        if ( pObj->GetTruck() != 0 )  continue;  // object transported?
        if ( !pObj->GetActif() )  continue;
        if ( pObj->GetProxyActivate() )  continue;

        oType = pObj->GetType();
        if ( oType == OBJECT_TOTO || oType == OBJECT_CONTROLLER )  continue;

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
            physics = pObj->GetPhysics();
            if ( physics != 0 && !physics->GetLand() )  continue;
        }
        if ( filter == FILTER_ONLYFLYING )
        {
            physics = pObj->GetPhysics();
            if ( physics != 0 && physics->GetLand() )  continue;
        }

        if ( bArray )
        {
            if ( !FindList(array, oType) )  continue;
        }
        else
        {
            if ( type != oType && type != OBJECT_NULL )  continue;
        }

        oPos = pObj->GetPosition(0);
        d = Math::DistanceProjected(iPos, oPos);
        if ( d < minDist || d > maxDist )  continue;  // too close or too far?

        if ( focus >= Math::PI*2.0f )
        {
            if ( (sens >= 0.0f && d < best) ||
                 (sens <  0.0f && d > best) )
            {
                best = d;
                pBest = pObj;
            }
            continue;
        }

        a = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        //TODO uninitialized variable
        if ( Math::TestAngle(a, iAngle-focus/2.0f, iAngle+focus/2.0f) )
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
        result->SetPointer(pBest->GetBotVar());
    }
    return true;
}


// Monitoring a task.

bool CScript::Process(CScript* script, CBotVar* result, int &exception)
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

bool CScript::ShouldProcessStop(Error err, int errMode)
{
    // aim impossible  - not a real error
    if ( err == ERR_AIM_IMPOSSIBLE )
        return false;

    if ( err != ERR_OK && errMode == ERM_STOP )
        return true;

    return false;
}


// Compilation of the instruction "detect(type)".

CBotTypResult CScript::cDetect(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "detect(type)".

bool CScript::rDetect(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    CObject     *pObj, *pGoal, *pBest;
    CPhysics*   physics;
    CBotVar*    array;
    Math::Vector    iPos, oPos;
    RadarFilter filter;
    float       bGoal, best, minDist, maxDist, sens, iAngle, angle, focus, d, a;
    int         type, oType, i;
    bool        bArray = false;
    Error       err;

    exception = 0;

    if ( script->m_primaryTask == 0 )  // no task in progress?
    {
        type    = OBJECT_NULL;
        array   = 0;
        angle   = 0.0f;
        focus   = 45.0f*Math::PI/180.0f;
        minDist = 0.0f*g_unit;
        maxDist = 20.0f*g_unit;
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
        }

        iPos   = pThis->GetPosition(0);
        iAngle = pThis->GetAngleY(0)+angle;
        iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

        CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

        bGoal = 100000.0f;
        pGoal = 0;
        if ( sens >= 0.0f )  best = 100000.0f;
        else                 best = 0.0f;
        pBest = 0;
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
            if ( pObj == 0 )  break;
            if ( pObj == pThis )  continue;

            if ( pObj->GetTruck() != 0 )  continue;  // object transported?
            if ( !pObj->GetActif() )  continue;
            if ( pObj->GetProxyActivate() )  continue;

            oType = pObj->GetType();
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
                physics = pObj->GetPhysics();
                if ( physics != 0 && !physics->GetLand() )  continue;
            }
            if ( filter == FILTER_ONLYFLYING )
            {
                physics = pObj->GetPhysics();
                if ( physics != 0 && physics->GetLand() )  continue;
            }

            if ( bArray )
            {
                if ( !FindList(array, oType) )  continue;
            }
            else
            {
                if ( type != oType && type != OBJECT_NULL )  continue;
            }

            oPos = pObj->GetPosition(0);
            d = Math::DistanceProjected(iPos, oPos);
            a = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

            if ( d < bGoal &&
                 Math::TestAngle(a, iAngle-(5.0f*Math::PI/180.0f)/2.0f, iAngle+(5.0f*Math::PI/180.0f)/2.0f) )
            {
                bGoal = d;
                pGoal = pObj;
            }

            if ( d < minDist || d > maxDist )  continue;  // too close or too far?

            if ( focus >= Math::PI*2.0f )
            {
                if ( (sens >= 0.0f && d < best) ||
                     (sens <  0.0f && d > best) )
                {
                    best = d;
                    pBest = pObj;
                }
                continue;
            }

            if ( Math::TestAngle(a, iAngle-focus/2.0f, iAngle+focus/2.0f) )
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

CBotTypResult CScript::cDirection(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "direction(pos)".

bool CScript::rDirection(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cCanBuild(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "canbuild ( category );"
// returns true if this building can be built

bool CScript::rCanBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rBuild(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    CObject*    pThis = static_cast<CObject *>(user);
    ObjectType  oType;
    ObjectType  category;
    Error       err = ERR_BUILD_DISABLED;

    exception = 0;

    oType = pThis->GetType();

    if ( oType != OBJECT_MOBILEfa &&  // allowed only for grabber bots
         oType != OBJECT_MOBILEta &&
         oType != OBJECT_MOBILEwa &&
         oType != OBJECT_MOBILEia)
    {
        err = ERR_MANIP_VEH; //Wrong vehicle;
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

CBotTypResult CScript::cProduce(CBotVar* &var, void* user)
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

bool CScript::rProduce(CBotVar* var, CBotVar* result, int& exception, void* user)
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
        object->ReadProgram(0, static_cast<const char*>(name));
        object->RunProgram(0);
    }

    result->SetValInt(0);  // no error
    return true;
}


// Compilation of the instruction "distance(p1, p2)".

CBotTypResult CScript::cDistance(CBotVar* &var, void* user)
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

bool CScript::rDistance(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cSpace(CBotVar* &var, void* user)
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

bool CScript::rSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cFlatGround(CBotVar* &var, void* user)
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

bool CScript::rFlatGround(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rWait(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rMove(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rTurn(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cGoto(CBotVar* &var, void* user)
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

bool CScript::rGoto(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cGrabDrop(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != 0 )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "grab(oper)".

bool CScript::rGrab(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rDrop(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rSniff(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cReceive(CBotVar* &var, void* user)
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

bool CScript::rReceive(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cSend(CBotVar* &var, void* user)
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

bool CScript::rSend(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CObject* CScript::SearchInfo(CScript* script, CObject* object, float power)
{
    CObject     *pObj, *pBest;
    Math::Vector    iPos, oPos;
    ObjectType  type;
    float       dist, min;
    int         i;

    iPos = object->GetPosition(0);

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    min = 100000.0f;
    pBest = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        type = pObj->GetType();
        if ( type != OBJECT_INFO )  continue;

        if ( !pObj->GetActif() )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, iPos);
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

CBotTypResult CScript::cDeleteInfo(CBotVar* &var, void* user)
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

bool CScript::rDeleteInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cTestInfo(CBotVar* &var, void* user)
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

bool CScript::rTestInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rThump(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rRecycle(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cShield(CBotVar* &var, void* user)
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

bool CScript::rShield(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cFire(CBotVar* &var, void* user)
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

bool CScript::rFire(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cAim(CBotVar* &var, void* user)
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

bool CScript::rAim(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cMotor(CBotVar* &var, void* user)
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

bool CScript::rMotor(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rJet(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CPhysics*   physics = (static_cast<CObject *>(user))->GetPhysics();
    float       value;

    value = var->GetValFloat();
    if( value > 1.0f ) value = 1.0f;

    physics->SetMotorSpeedY(value);

    return true;
}

// Compilation of the instruction "topo(pos)".

CBotTypResult CScript::cTopo(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    ret = CScript::cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "topo(pos)".

bool CScript::rTopo(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cMessage(CBotVar* &var, void* user)
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

bool CScript::rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rCmdline(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rIsMovie(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = (static_cast<CObject *>(user))->GetRunScript();
    float       value;

    value = script->m_main->GetMovieLock()?1.0f:0.0f;
    result->SetValFloat(value);

    return true;
}

// Instruction "errmode(mode)".

bool CScript::rErrMode(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rIPF(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rAbsTime(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cPenDown(CBotVar* &var, void* user)
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

bool CScript::rPenDown(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rPenUp(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rPenColor(CBotVar* var, CBotVar* result, int& exception, void* user)
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

bool CScript::rPenWidth(CBotVar* var, CBotVar* result, int& exception, void* user)
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

CBotTypResult CScript::cOneObject(CBotVar* &var, void* user)
{
    if ( var == 0 )  return CBotTypResult(CBotErrLowParam);
    var = var->GetNext();
    if ( var == 0 )  return CBotTypResult(CBotTypFloat);
    
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "camerafocus(object)".

bool CScript::rCameraFocus(CBotVar* var, CBotVar* result, int& exception, void* user)
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
    CObject* object = CObjectManager::GetInstancePointer()->SearchInstance(rank);
    
    script->m_main->SelectObject(object, false);
    
    result->SetValInt(ERR_OK);
    exception = ERR_OK;
    return true;
}



// Object's constructor.

CScript::CScript(CObject* object, CTaskManager** secondaryTask)
{
    m_engine        = Gfx::CEngine::GetInstancePointer();
    m_main          = CRobotMain::GetInstancePointer();
    m_terrain       = m_main->GetTerrain();
    m_water         = m_engine->GetWater();
    m_botProg       = nullptr;
    m_object        = object;
    m_primaryTask   = nullptr;
    m_secondaryTask = secondaryTask;

    m_interface = m_main->GetInterface();
    m_pause = CPauseManager::GetInstancePointer();

    m_ipf = CBOT_IPF;
    m_errMode = ERM_STOP;
    m_len = 0;
    m_script = nullptr;
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
    CBotProgram::AddFunction("sin",       rSin,       CScript::cOneFloat);
    CBotProgram::AddFunction("cos",       rCos,       CScript::cOneFloat);
    CBotProgram::AddFunction("tan",       rTan,       CScript::cOneFloat);
    CBotProgram::AddFunction("asin",      raSin,      CScript::cOneFloat);
    CBotProgram::AddFunction("acos",      raCos,      CScript::cOneFloat);
    CBotProgram::AddFunction("atan",      raTan,      CScript::cOneFloat);
    CBotProgram::AddFunction("sqrt",      rSqrt,      CScript::cOneFloat);
    CBotProgram::AddFunction("pow",       rPow,       CScript::cTwoFloat);
    CBotProgram::AddFunction("rand",      rRand,      CScript::cNull);
    CBotProgram::AddFunction("abs",       rAbs,       CScript::cOneFloat);

    CBotProgram::AddFunction("endmission",rEndMission,CScript::cEndMission);
    CBotProgram::AddFunction("playmusic", rPlayMusic ,CScript::cPlayMusic);
    CBotProgram::AddFunction("stopmusic", rStopMusic ,CScript::cNull);

    CBotProgram::AddFunction("getbuild",          rGetBuild,          CScript::cNull);
    CBotProgram::AddFunction("getresearchenable", rGetResearchEnable, CScript::cNull);
    CBotProgram::AddFunction("getresearchdone",   rGetResearchDone,   CScript::cNull);
    CBotProgram::AddFunction("setbuild",          rSetBuild,          CScript::cOneFloat);
    CBotProgram::AddFunction("setresearchenable", rSetResearchEnable, CScript::cOneFloat);
    CBotProgram::AddFunction("setresearchdone",   rSetResearchDone,   CScript::cOneFloat);

    CBotProgram::AddFunction("progfunc",  rProgFunc,  CScript::cStringString);
    CBotProgram::AddFunction("retobject", rGetObject, CScript::cGetObject);
    CBotProgram::AddFunction("retobjectbyid", rGetObjectById, CScript::cGetObject);
    CBotProgram::AddFunction("delete",    rDelete,    CScript::cDelete);
    CBotProgram::AddFunction("search",    rSearch,    CScript::cSearch);
    CBotProgram::AddFunction("radar",     rRadar,     CScript::cRadar);
    CBotProgram::AddFunction("detect",    rDetect,    CScript::cDetect);
    CBotProgram::AddFunction("direction", rDirection, CScript::cDirection);
    CBotProgram::AddFunction("produce",   rProduce,   CScript::cProduce);
    CBotProgram::AddFunction("distance",  rDistance,  CScript::cDistance);
    CBotProgram::AddFunction("distance2d",rDistance2d,CScript::cDistance);
    CBotProgram::AddFunction("space",     rSpace,     CScript::cSpace);
    CBotProgram::AddFunction("flatground",rFlatGround,CScript::cFlatGround);
    CBotProgram::AddFunction("wait",      rWait,      CScript::cOneFloat);
    CBotProgram::AddFunction("move",      rMove,      CScript::cOneFloat);
    CBotProgram::AddFunction("turn",      rTurn,      CScript::cOneFloat);
    CBotProgram::AddFunction("goto",      rGoto,      CScript::cGoto);
    CBotProgram::AddFunction("grab",      rGrab,      CScript::cGrabDrop);
    CBotProgram::AddFunction("drop",      rDrop,      CScript::cGrabDrop);
    CBotProgram::AddFunction("sniff",     rSniff,     CScript::cNull);
    CBotProgram::AddFunction("receive",   rReceive,   CScript::cReceive);
    CBotProgram::AddFunction("send",      rSend,      CScript::cSend);
    CBotProgram::AddFunction("deleteinfo",rDeleteInfo,CScript::cDeleteInfo);
    CBotProgram::AddFunction("testinfo",  rTestInfo,  CScript::cTestInfo);
    CBotProgram::AddFunction("thump",     rThump,     CScript::cNull);
    CBotProgram::AddFunction("recycle",   rRecycle,   CScript::cNull);
    CBotProgram::AddFunction("shield",    rShield,    CScript::cShield);
    CBotProgram::AddFunction("fire",      rFire,      CScript::cFire);
    CBotProgram::AddFunction("aim",       rAim,       CScript::cAim);
    CBotProgram::AddFunction("motor",     rMotor,     CScript::cMotor);
    CBotProgram::AddFunction("jet",       rJet,       CScript::cOneFloat);
    CBotProgram::AddFunction("topo",      rTopo,      CScript::cTopo);
    CBotProgram::AddFunction("message",   rMessage,   CScript::cMessage);
    CBotProgram::AddFunction("cmdline",   rCmdline,   CScript::cOneFloat);
    CBotProgram::AddFunction("ismovie",   rIsMovie,   CScript::cNull);
    CBotProgram::AddFunction("errmode",   rErrMode,   CScript::cOneFloat);
    CBotProgram::AddFunction("ipf",       rIPF,       CScript::cOneFloat);
    CBotProgram::AddFunction("abstime",   rAbsTime,   CScript::cNull);
    CBotProgram::AddFunction("deletefile",rDeleteFile,CScript::cString);
    CBotProgram::AddFunction("pendown",   rPenDown,   CScript::cPenDown);
    CBotProgram::AddFunction("penup",     rPenUp,     CScript::cNull);
    CBotProgram::AddFunction("pencolor",  rPenColor,  CScript::cOneFloat);
    CBotProgram::AddFunction("penwidth",  rPenWidth,  CScript::cOneFloat);
    
    CBotProgram::AddFunction("camerafocus", rCameraFocus, CScript::cOneObject);

    CBotProgram::AddFunction("canbuild", rCanBuild, CScript::cCanBuild);
    CBotProgram::AddFunction("build", rBuild, CScript::cOneFloat);

}

// Object's destructor.

CScript::~CScript()
{
    delete m_botProg;
    m_botProg = nullptr;

    delete m_primaryTask;
    m_primaryTask = nullptr;

    delete[] m_script;
    m_script = nullptr;

    m_len = 0;
}


// Gives the script editable block of text.

void CScript::PutScript(Ui::CEdit* edit, const char* name)
{
    if ( m_script == nullptr )
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

bool CScript::GetScript(Ui::CEdit* edit)
{
    int     len;

    delete[] m_script;
    m_script = nullptr;

    len = edit->GetTextLength();
    m_script = new char[len+1];

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

bool CScript::GetCompile()
{
    return m_bCompile;
}

// Indicates whether the program is empty.

bool CScript::IsEmpty()
{
    int     i;

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
    CBotToken*  bt;
    CBotToken*  allBt;
    CBotString  bs;
    const char* token;
    int         error, cursor1, cursor2, i;
    char        used[100];

    if ( !m_object->GetCheckToken() )  return true;

    m_error = 0;
    m_title[0] = 0;
    m_token[0] = 0;
    m_bCompile = false;

    for ( i=0 ; i<m_main->GetObligatoryToken() ; i++ )
    {
        used[i] = 0;  // token not used
    }

    allBt = CBotToken::CompileTokens(m_script, error);
    bt = allBt;
    while ( bt != 0 )
    {
        bs = bt->GetString();
        token = bs;

        cursor1 = bt->GetStart();
        cursor2 = bt->GetEnd();

        i = m_main->IsObligatoryToken(token);
        if ( i != -1 )
        {
            used[i] = 1;  // token used
        }

        if ( !m_main->IsProhibitedToken(token) )
        {
            m_error = ERR_PROHIBITEDTOKEN;
            m_cursor1 = cursor1;
            m_cursor2 = cursor2;
            strcpy(m_title, "<erreur>");
            CBotToken::Delete(allBt);
            return false;
        }

        bt = bt->GetNext();
    }

    // At least once every obligatory instruction?
    for ( i=0 ; i<m_main->GetObligatoryToken() ; i++ )
    {
        if ( used[i] == 0 )  // token not used?
        {
            strcpy(m_token, m_main->GetObligatoryToken(i));
            m_error = ERR_OBLIGATORYTOKEN;
            strcpy(m_title, "<erreur>");
            CBotToken::Delete(allBt);
            return false;
        }
    }

    CBotToken::Delete(allBt);
    return true;
}

// Compile the script of a paved text.

bool CScript::Compile()
{
    CBotStringArray liste;
    int             i;
    const char*     p;

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
        m_botProg = new CBotProgram(m_object->GetBotVar());
    }

    if ( m_botProg->Compile(m_script, liste, this) )
    {
        if ( liste.GetSize() == 0 )
        {
            strcpy(m_title, "<extern missing>");
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
        strcpy(m_title, "<error>");
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
    if ( m_script == nullptr || m_len == 0 )  return false;

    if ( !m_botProg->Start(m_title) )  return false;

    m_object->SetRunScript(this);
    m_bRun = true;
    m_bContinue = false;
    m_ipf = CBOT_IPF;
    m_errMode = ERM_STOP;

    if ( m_bStepMode )  // step by step mode?
    {
        Event   newEvent;
        memset(&newEvent, 0, sizeof(Event));
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
                    std::string s;
                    GetError(s);
                    m_main->GetDisplayText()->DisplayText(s.c_str(), m_object, 10.0f, Ui::TT_ERROR);
                }
                m_pause->SetPause(PAUSE_EDITOR);  // gives pause
                return true;
            }
            if ( !m_bContinue )
            {
                m_pause->SetPause(PAUSE_EDITOR);  // gives pause
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
            std::string s;
            GetError(s);
            m_main->GetDisplayText()->DisplayText(s.c_str(), m_object, 10.0f, Ui::TT_ERROR);
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

    // ??? m_engine->SetPause(false);
    // TODO: m_app StepSimulation??? m_engine->StepSimulation(0.01f);  // advance of 10ms
    // ??? m_engine->SetPause(true);

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
            std::string s;
            GetError(s);
            m_main->GetDisplayText()->DisplayText(s.c_str(), m_object, 10.0f, Ui::TT_ERROR);
        }
        return true;
    }

    if ( m_bContinue )  // instuction "move", "goto", etc. ?
    {
        m_pause->ClearPause();  // removes the pause
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
    const char* funcName;

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

void PutList(const char *baseName, bool bArray, CBotVar *var, Ui::CList *list, int &rankList)
{
    CBotString  bs;
    CBotVar     *svar, *pStatic;
    char        varName[100];
    char        buffer[100];
    const char  *p;
    int         index, type;

    if ( var == 0 && baseName[0] != 0 )
    {
        sprintf(buffer, "%s = null;", baseName);
        list->SetItemName(rankList++, buffer);
        return;
    }

    index = 0;
    while ( var != 0 )
    {
        var->Maj(NULL, false);
        pStatic = var->GetStaticVar();  // finds the static element

        bs = pStatic->GetName();  // variable name
        p = bs;
//?     if ( strcmp(p, "this") == 0 )
//?     {
//?         var = var->GetNext();
//?         continue;
//?     }

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

        type = pStatic->GetType();

        if ( type < CBotTypBoolean )
        {
            CBotString  value;
            value = pStatic->GetValString();
            p = value;
            sprintf(buffer, "%s = %s;", varName, p);
            list->SetItemName(rankList++, buffer);
        }
        else if ( type == CBotTypString )
        {
            CBotString  value;
            value = pStatic->GetValString();
            p = value;
            sprintf(buffer, "%s = \"%s\";", varName, p);
            list->SetItemName(rankList++, buffer);
        }
        else if ( type == CBotTypArrayPointer )
        {
            svar = pStatic->GetItemList();
            PutList(varName, true, svar, list, rankList);
        }
        else if ( type == CBotTypClass   ||
                  type == CBotTypPointer )
        {
            svar = pStatic->GetItemList();
            PutList(varName, false, svar, list, rankList);
        }
        else
        {
            sprintf(buffer, "%s = ?;", varName);
            list->SetItemName(rankList++, buffer);
        }

        index ++;
        var = var->GetNext();
    }
}

// Fills a list with variables.

void CScript::UpdateList(Ui::CList* list)
{
    CBotVar     *var;
    const char  *progName, *funcName;
    int         total, select, level, cursor1, cursor2, rank;

    if( m_botProg == 0 )  return;

    total  = list->GetTotal();
    select = list->GetSelect();

    list->Flush();  // empty list
    m_botProg->GetRunPos(progName, cursor1, cursor2);
    if ( progName == 0 )  return;

    level = 0;
    rank  = 0;
    while ( true )
    {
        var = m_botProg->GetStackVars(funcName, level--);
        if ( funcName != progName )  break;

        PutList("", false, var, list, rank);
    }

    if ( total == list->GetTotal() )  // same total?
    {
        list->SetSelect(select);
    }

    list->SetTooltip("");
    list->SetState(Ui::STATE_ENABLE);
}


// Colorize the text according to syntax.

void CScript::ColorizeScript(Ui::CEdit* edit)
{
    CBotToken*  bt;
    CBotString  bs;
    const char* token;
    int         error, type, cursor1, cursor2;
    Gfx::FontHighlight color;

    edit->ClearFormat();

    bt = CBotToken::CompileTokens(edit->GetText(), error);
    while ( bt != 0 )
    {
        bs = bt->GetString();
        token = bs;
        type = bt->GetType();

        cursor1 = bt->GetStart();
        cursor2 = bt->GetEnd();
        color = Gfx::FONT_HIGHLIGHT_NONE;
        if ( type >= TokenKeyWord && type < TokenKeyWord+100 )
        {
            color = Gfx::FONT_HIGHLIGHT_TOKEN;
        }
        if ( type >= TokenKeyDeclare && type < TokenKeyDeclare+100 )
        {
            color = Gfx::FONT_HIGHLIGHT_TYPE;
        }
        if ( type >= TokenKeyVal && type < TokenKeyVal+100 )
        {
            color = Gfx::FONT_HIGHLIGHT_CONST;
        }
        if ( type == TokenTypVar )
        {
            if ( IsType(token) )
            {
                color = Gfx::FONT_HIGHLIGHT_TYPE;
            }
            else if ( IsFunction(token) )
            {
                color = Gfx::FONT_HIGHLIGHT_TOKEN;
            }
        }
        if ( type == TokenTypDef )
        {
            color =Gfx::FONT_HIGHLIGHT_CONST;
        }

        if ( cursor1 < cursor2 && color != Gfx::FONT_HIGHLIGHT_NONE )
        {
            edit->SetFormat(cursor1, cursor2, color);
        }

        bt = bt->GetNext();
    }

    CBotToken::Delete(bt);
}


// Seeks a token at random in a script.
// Returns the index of the start of the token found, or -1.


int SearchToken(char* script, const char* token)
{
    int     lScript, lToken, i, iFound;
    int     found[100];
    char*   p;

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

void InsertToken(char* script, int pos, const char* token)
{
    int     lScript, lToken, i;

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
    int     i, start, iFound;
    int     found[11*2];
    char*   newScript;

    const char*   names[11*2] =
    {
        "==",           "!=",
        "!=",           "==",
        ">",            "<",
        "<",            ">",
        "true",         "false",
        "false",        "true",
        "grab",         "drop",
        "drop",         "grab",
        "InFront",      "Behind",
        "Behind",       "EnergyCell",
        "EnergyCell",   "InFront",
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

    newScript = new char[m_len+strlen(names[i+1])+1];
    strcpy(newScript, m_script);
    delete[] m_script;
    m_script = newScript;

    DeleteToken(m_script, start, strlen(names[i]));
    InsertToken(m_script, start, names[i+1]);
    m_len = strlen(m_script);
    Compile();  // recompile with the virus

    return true;
}


// Returns the number of the error.

int CScript::GetError()
{
    return m_error;
}

// Returns the text of the error.

void CScript::GetError(std::string& error)
{
    if ( m_error == 0 )
    {
        error.clear();
    }
    else
    {
        if ( m_error == ERR_OBLIGATORYTOKEN )
        {
            std::string s;
            GetResource(RES_ERR, m_error, s);
            error = StrUtils::Format(s.c_str(), m_token);
        }
        else if ( m_error < 1000 )
        {
            GetResource(RES_ERR, m_error, error);
        }
        else
        {
            GetResource(RES_CBOT, m_error, error);
        }
    }
}


// New program.

void CScript::New(Ui::CEdit* edit, const char* name)
{
    FILE    *file = NULL;
    char    res[100];
    char    text[100];
    char    script[500];
    char    buffer[500];
    char    *sf;
    int     cursor1, cursor2, len, i, j;

    std::string resStr;
    GetResource(RES_TEXT, RT_SCRIPT_NEW, resStr);
    strcpy(res, resStr.c_str());
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
        if ( edit->GetAutoIndent() )
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

    sf = m_main->GetScriptFile();
    if ( sf[0] != 0 )  // Load an empty program specific?
    {
        std::string filename = CGameData::GetInstancePointer()->GetFilePath(DIR_AI, sf);
        file = fopen(filename.c_str(), "rb");
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

                if ( buffer[i] == '\t' && edit->GetAutoIndent() )
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

bool CScript::SendScript(const char* text)
{
    /*m_len = strlen(text);
    m_script = new char[m_len+1];
    strcpy(m_script, text);
    if ( !CheckToken() )  return false;
    if ( !Compile() )  return false;*/

    Ui::CEdit* edit = m_interface->CreateEdit(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 0, EVENT_EDIT9);
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->SetText(text, true);
    GetScript(edit);
    m_interface->DeleteControl(EVENT_EDIT9);

    return true;
}

// Reads a script as a text file.

bool CScript::ReadScript(const char* filename)
{
    FILE*       file;
    Ui::CEdit*  edit;
    std::string name;

    if ( strchr(filename, '/') == 0 ) //we're reading non user script
    {
        name = CGameData::GetInstancePointer()->GetFilePath(DIR_AI, filename);
    }
    else
    {
        name = filename;
        //TODO: is this needed?
        // UserDir(name, filename, "");
    }

    file = fopen(name.c_str(), "rb");
    if ( file == NULL )  return false;
    fclose(file);

    delete[] m_script;
    m_script = nullptr;

    edit = m_interface->CreateEdit(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 0, EVENT_EDIT9);
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->ReadText(name.c_str());
    GetScript(edit);
    m_interface->DeleteControl(EVENT_EDIT9);
    return true;
}

// Writes a script as a text file.

bool CScript::WriteScript(const char* filename)
{
    Ui::CEdit*  edit;
    std::string name;

    if ( strchr(filename, '/') == 0 ) //we're writing non user script
    {
        name = CGameData::GetInstancePointer()->GetFilePath(DIR_AI, filename);
    }
    else
    {
        name = filename;
    }

    if ( m_script == nullptr )
    {
        remove(filename);
        return false;
    }

    edit = m_interface->CreateEdit(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 0, EVENT_EDIT9);
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->SetText(m_script);
    edit->WriteText(name);
    m_interface->DeleteControl(EVENT_EDIT9);
    return true;
}


// Reads a stack of script by execution as a file.

bool CScript::ReadStack(FILE *file)
{
    int     nb;

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
    int     nb;

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

char* CScript::GetFilename()
{
    return m_filename;
}

