/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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

#include "CBot/CBot.h"

#include "app/app.h"

#include "common/global.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "level/parser/parser.h"

#include "math/all.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/auto/auto.h"
#include "object/auto/autobase.h"
#include "object/auto/autofactory.h"

#include "object/interface/destroyable_object.h"
#include "object/interface/programmable_object.h"
#include "object/interface/task_executor_object.h"
#include "object/interface/trace_drawing_object.h"

#include "object/subclass/base_alien.h"
#include "object/subclass/exchange_post.h"
#include "object/subclass/shielder.h"

#include "object/task/taskinfo.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/script.h"

#include "sound/sound.h"

#include "ui/displaytext.h"

#include <cmath>

using namespace CBot;

CBotTypResult CScriptFunctions::cClassNull(CBotVar* thisclass, CBotVar* &var)
{
    return cNull(var, nullptr);
}

CBotTypResult CScriptFunctions::cClassOneFloat(CBotVar* thisclass, CBotVar* &var)
{
    return cOneFloat(var, nullptr);
}

// Compile a parameter of type "point".

static CBotTypResult cPoint(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);

    if ( var->GetType() <= CBotTypDouble )
    {
        var = var->GetNext();
        if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
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

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Gives a parameter of type "point".

static bool GetPoint(CBotVar* &var, int& exception, glm::vec3& pos)
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
        if ( pX == nullptr )
        {
            exception = CBotErrUndefItem;  return true;
        }
        pos.x = pX->GetValFloat()*g_unit;

        pY = var->GetItem("y");
        if ( pY == nullptr )
        {
            exception = CBotErrUndefItem;  return true;
        }
        pos.z = pY->GetValFloat()*g_unit;  // attention y -> z !

        pZ = var->GetItem("z");
        if ( pZ == nullptr )
        {
            exception = CBotErrUndefItem;  return true;
        }
        pos.y = pZ->GetValFloat()*g_unit;  // attention z -> y !

        var = var->GetNext();
    }
    return true;
}


// Compilation of the instruction "endmission(result, delay)"

CBotTypResult CScriptFunctions::cEndMission(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
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

    CRobotMain::GetInstancePointer()->SetMissionResultFromScript(ended, delay);
    return true;
}

// Compilation of the instruction "playmusic(filename, repeat)"

CBotTypResult CScriptFunctions::cPlayMusic(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "playmusic(filename, repeat)"

bool CScriptFunctions::rPlayMusic(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    std::string filename;
    std::string cbs;
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
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypPointer, "object");
}

// Instruction "retobjectbyid(rank)".

bool CScriptFunctions::rGetObjectById(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pObj;
    int         rank;

    rank = var->GetValInt();

    pObj = static_cast<CObject*>(CObjectManager::GetInstancePointer()->GetObjectById(rank));
    if ( pObj == nullptr )
    {
        result->SetPointer(nullptr);
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
        result->SetPointer(nullptr);
    }
    else
    {
        result->SetPointer(pObj->GetBotVar());
    }
    return true;
}

// Compilation of instruction "isbusy( object )"

CBotTypResult CScriptFunctions::cIsBusy(CBot::CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "isbusy( object )"

bool CScriptFunctions::rIsBusy(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*    pThis = static_cast<CScript*>(user)->m_object;

    exception = 0;

    CObject* obj = static_cast<CObject*>(var->GetUserPtr());
    if (obj == nullptr)
    {
        exception = ERR_WRONG_OBJ;
        result->SetValInt(ERR_WRONG_OBJ);
        return false;
    }

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

bool CScriptFunctions::rDestroy(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    exception = 0;
    Error err;

    CObject* obj;
    if (var == nullptr)
        obj = CObjectManager::GetInstancePointer()->FindNearest(pThis, OBJECT_DESTROYER);
    else
        obj = static_cast<CObject*>(var->GetUserPtr());

    if (obj == nullptr)
    {
        exception = ERR_WRONG_OBJ;
        result->SetValInt(ERR_WRONG_OBJ);
        return false;
    }

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

    result->SetValInt(err); // indicates the error or ok
    if ( err != ERR_OK )
    {
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Compilation of instruction "factory(cat[, program , object])"

CBotTypResult CScriptFunctions::cFactory(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )
    {
        if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadParam);
        var = var->GetNext();
        if ( var != nullptr )
        {
            if ( var->GetType() != CBotTypPointer )  return CBotTypResult(CBotErrBadParam);
            var = var->GetNext();
            if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
        }
    }
    return CBotTypResult(CBotTypFloat);
}

// Instruction "factory(cat[, program , object])"

bool CScriptFunctions::rFactory(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    Error       err;

    exception = 0;

    ObjectType type = static_cast<ObjectType>(var->GetValInt());
    var = var->GetNext();

    std::string program;
    if ( var != nullptr )
    {
        program = var->GetValString();
        var = var->GetNext();
    }
    else
        program = "";

    CObject* factory;
    if (var == nullptr)
        factory = CObjectManager::GetInstancePointer()->FindNearest(pThis, OBJECT_FACTORY);
    else
        factory = static_cast<CObject*>(var->GetUserPtr());

    if (factory == nullptr)
    {
        exception = ERR_WRONG_OBJ;
        result->SetValInt(ERR_WRONG_OBJ);
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
            GetLogger()->Error("in factory() - automat is nullptr");
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

    result->SetValInt(err); // indicates the error or ok
    if ( err != ERR_OK )
    {
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Compilation of instruction "research(type[, object])"

CBotTypResult CScriptFunctions::cResearch(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )
    {
        if ( var->GetType() != CBotTypPointer )  return CBotTypResult(CBotErrBadParam);
        var = var->GetNext();
        if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    }
    return CBotTypResult(CBotTypFloat);
}
// Instruction "research(type[, object])"

bool CScriptFunctions::rResearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    Error       err;

    exception = 0;

    ResearchType type = static_cast<ResearchType>(var->GetValInt());
    var = var->GetNext();

    CObject* center;
    if (var == nullptr)
        center = CObjectManager::GetInstancePointer()->FindNearest(pThis, OBJECT_RESEARCH);
    else
        center = static_cast<CObject*>(var->GetUserPtr());

    if (center == nullptr)
    {
        exception = ERR_WRONG_OBJ;
        result->SetValInt(ERR_WRONG_OBJ);
        return false;
    }

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
             type == RESEARCH_iGUN       ||
             type == RESEARCH_TARGET      )
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

    result->SetValInt(err); // indicates the error or ok
    if ( err != ERR_OK )
    {
        if( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Instruction "takeoff(object)"

bool CScriptFunctions::rTakeOff(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;

    Error       err;

    exception = 0;
    CObject* base;
    if (var == nullptr)
        base = CObjectManager::GetInstancePointer()->FindNearest(pThis, OBJECT_BASE);
    else
        base = static_cast<CObject*>(var->GetUserPtr());

    if (base == nullptr)
    {
        exception = ERR_WRONG_OBJ;
        result->SetValInt(ERR_WRONG_OBJ);
        return false;
    }

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

    result->SetValInt(err); // indicates the error or ok
    if ( err != ERR_OK )
    {
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return true;
}

// Compilation of the instruction "delete(rank[, exploType])".

CBotTypResult CScriptFunctions::cDelete(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);

    if ( var->GetType() != CBotTypInt )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )
    {
        if ( var->GetType() != CBotTypInt ) return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();
    }

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "delete(rank[, exploType])".

bool CScriptFunctions::rDelete(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    int rank;
    DestructionType exploType = DestructionType::Explosion;

    rank = var->GetValInt();
    var = var->GetNext();
    if ( var != nullptr )
    {
        exploType = static_cast<DestructionType>(var->GetValInt());
    }

    CObject* obj = CObjectManager::GetInstancePointer()->GetObjectById(rank);
    if ( obj == nullptr || (obj->Implements(ObjectInterfaceType::Old) && dynamic_cast<COldObject&>(*obj).IsDying()) )
    {
        return true;
    }
    else
    {
        CScript* script = static_cast<CScript*>(user);
        bool deleteSelf = (obj == script->m_object);

        if ( exploType != DestructionType::NoEffect && obj->Implements(ObjectInterfaceType::Destroyable) )
        {
            dynamic_cast<CDestroyableObject&>(*obj).DestroyObject(static_cast<DestructionType>(exploType));
        }
        else
        {
            if (obj->Implements(ObjectInterfaceType::Slotted))
            {
                CSlottedObject* slotted = dynamic_cast<CSlottedObject*>(obj);
                for (int slotNum = slotted->GetNumSlots() - 1; slotNum >= 0; slotNum--)
                {
                    CObject* sub = slotted->GetSlotContainedObject(slotNum);
                    if (sub != nullptr)
                    {
                        slotted->SetSlotContainedObject(slotNum, nullptr);
                        CObjectManager::GetInstancePointer()->DeleteObject(sub);
                    }
                }
            }
            CObjectManager::GetInstancePointer()->DeleteObject(obj);
        }
        // Returning "false" here makes sure the program doesn't try to keep executing
        // if the robot just destroyed itself using delete(this.id)
        // See issue #925
        return !deleteSelf;
    }

    return true;
}

static CBotTypResult compileSearch(CBotVar* &var, void* user, CBotTypResult returnValue)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() == CBotTypArrayPointer )
    {
        CBotTypResult type = var->GetTypResult().GetTypElem();
        if ( type.GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadParam);  // type
    }
    else if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // type
    var = var->GetNext();

    if ( var == nullptr )  return returnValue;

    CBotTypResult ret = cPoint(var, user);                                       // pos
    if ( ret.GetType() != 0 ) return ret;

    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // min
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // max
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // sense
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // filter
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    return CBotTypResult(CBotErrOverParam);
}

// Compilation of "search(type, pos, min, max, sens, filter)".

CBotTypResult CScriptFunctions::cSearch(CBotVar* &var, void* user)
{
    return compileSearch(var, user, CBotTypResult(CBotTypPointer, "object"));
}

CBotTypResult CScriptFunctions::cSearchAll(CBotVar* &var, void* user)
{
    return compileSearch(var, user, CBotTypResult(CBotTypArrayPointer, CBotTypResult(CBotTypPointer, "object")));
}

static bool runSearch(CBotVar* var, glm::vec3 pos, int& exception, std::function<bool(std::vector<ObjectType>, glm::vec3, float, float, bool, RadarFilter)> code)
{
    CBotVar*    array;
    RadarFilter filter;
    float       minDist, maxDist, sens;
    int         type;
    bool        bArray = false;

    type    = OBJECT_NULL;
    array   = nullptr;
    minDist = 0.0f*g_unit;
    maxDist = 1000.0f*g_unit;
    sens    = 1.0f;
    filter  = FILTER_NONE;

    if ( var != nullptr )
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
        if ( var != nullptr )
        {
            if ( !GetPoint(var, exception, pos) ) return false;

            if ( var != nullptr )
            {
                minDist = var->GetValFloat();

                var = var->GetNext();
                if ( var != nullptr )
                {
                    maxDist = var->GetValFloat();

                    var = var->GetNext();
                    if ( var != nullptr )
                    {
                        sens = var->GetValFloat();

                        var = var->GetNext();
                        if ( var != nullptr )
                        {
                            filter = static_cast<RadarFilter>(var->GetValInt());
                        }
                    }
                }
            }
        }
    }

    std::vector<ObjectType> type_v;
    if (bArray)
    {
        while ( array != nullptr )
        {
            if (array->GetValInt() == OBJECT_MOBILEpr)
            {
                type_v.push_back(OBJECT_MOBILEwt);
                type_v.push_back(OBJECT_MOBILEtt);
                type_v.push_back(OBJECT_MOBILEft);
                type_v.push_back(OBJECT_MOBILEit);
                type_v.push_back(OBJECT_MOBILErp);
                type_v.push_back(OBJECT_MOBILEst);
            }
            type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
            array = array->GetNext();
        }
    }
    else
    {
        if (type != OBJECT_NULL && type != OBJECT_MOBILEpr)
        {
            type_v.push_back(static_cast<ObjectType>(type));
        }
        else if (type == OBJECT_MOBILEpr)
        {
           type_v.push_back(OBJECT_MOBILEwt);
           type_v.push_back(OBJECT_MOBILEtt);
           type_v.push_back(OBJECT_MOBILEft);
           type_v.push_back(OBJECT_MOBILEit);
           type_v.push_back(OBJECT_MOBILErp);
           type_v.push_back(OBJECT_MOBILEst);
        }
    }

    return code(type_v, pos, minDist, maxDist, sens < 0, filter);
}

bool CScriptFunctions::rSearch(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;

    return runSearch(var, pThis->GetPosition(), exception, [&result, pThis](std::vector<ObjectType> types, glm::vec3 pos, float minDist, float maxDist, bool furthest, RadarFilter filter)
    {
        CObject* pBest = CObjectManager::GetInstancePointer()->Radar(pThis, pos, 0.0f, types, 0.0f, Math::PI*2.0f, minDist, maxDist, furthest, filter, true);

        if ( pBest == nullptr )
        {
            result->SetPointer(nullptr);
        }
        else
        {
            result->SetPointer(pBest->GetBotVar());
        }

        return true;
    });
}

bool CScriptFunctions::rSearchAll(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;

    return runSearch(var, pThis->GetPosition(), exception, [&result, pThis](std::vector<ObjectType> types, glm::vec3 pos, float minDist, float maxDist, bool furthest, RadarFilter filter)
    {
        std::vector<CObject*> best = CObjectManager::GetInstancePointer()->RadarAll(pThis, pos, 0.0f, types, 0.0f, Math::PI*2.0f, minDist, maxDist, furthest, filter, true);

        int i = 0;
        result->SetInit(CBotVar::InitType::DEF);
        for (CObject* obj : best)
        {
            result->GetItem(i++, true)->SetPointer(obj->GetBotVar());
        }

        return true;
    });
}


static CBotTypResult compileRadar(CBotVar* &var, void* user, CBotTypResult returnValue)
{
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() == CBotTypArrayPointer )
    {
        CBotTypResult type = var->GetTypResult().GetTypElem();
        if ( type.GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadParam); //type
    }
    else if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadParam);  // type
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // angle
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // focus
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // min
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // max
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // sense
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);  // filter
    var = var->GetNext();
    if ( var == nullptr )  return returnValue;
    return CBotTypResult(CBotErrOverParam);
}

CBotTypResult CScriptFunctions::cRadarAll(CBotVar* &var, void* user)
{
    return compileRadar(var, user, CBotTypResult(CBotTypArrayPointer, CBotTypResult(CBotTypPointer, "object")));
}

// Compilation of instruction "radar(type, angle, focus, min, max, sens)".

CBotTypResult CScriptFunctions::cRadar(CBotVar* &var, void* user)
{
    return compileRadar(var, user, CBotTypResult(CBotTypPointer, "object"));
}

static bool runRadar(CBotVar* var, std::function<bool(std::vector<ObjectType>, float, float, float, float, bool, RadarFilter)> code)
{
    CBotVar*    array;
    RadarFilter filter;
    float       minDist, maxDist, sens, angle, focus;
    int         type;
    bool        bArray = false;

    type    = OBJECT_NULL;
    array   = nullptr;
    angle   = 0.0f;
    focus   = Math::PI*2.0f;
    minDist = 0.0f*g_unit;
    maxDist = 1000.0f*g_unit;
    sens    = 1.0f;
    filter  = FILTER_NONE;

    if ( var != nullptr )
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
        if ( var != nullptr )
        {
            angle = -var->GetValFloat()*Math::PI/180.0f;

            var = var->GetNext();
            if ( var != nullptr )
            {
                focus = var->GetValFloat()*Math::PI/180.0f;

                var = var->GetNext();
                if ( var != nullptr )
                {
                    minDist = var->GetValFloat();

                    var = var->GetNext();
                    if ( var != nullptr )
                    {
                        maxDist = var->GetValFloat();

                        var = var->GetNext();
                        if ( var != nullptr )
                        {
                            sens = var->GetValFloat();

                            var = var->GetNext();
                            if ( var != nullptr )
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
        while ( array != nullptr )
        {
            if (array->GetValInt() == OBJECT_MOBILEpr)
            {
                type_v.push_back(OBJECT_MOBILEwt);
                type_v.push_back(OBJECT_MOBILEtt);
                type_v.push_back(OBJECT_MOBILEft);
                type_v.push_back(OBJECT_MOBILEit);
                type_v.push_back(OBJECT_MOBILErp);
                type_v.push_back(OBJECT_MOBILEst);
            }
            type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
            array = array->GetNext();
        }
    }
    else
    {
        if (type != OBJECT_NULL && type != OBJECT_MOBILEpr)
        {
            type_v.push_back(static_cast<ObjectType>(type));
        }
        else if (type == OBJECT_MOBILEpr)
        {
           type_v.push_back(OBJECT_MOBILEwt);
           type_v.push_back(OBJECT_MOBILEtt);
           type_v.push_back(OBJECT_MOBILEft);
           type_v.push_back(OBJECT_MOBILEit);
           type_v.push_back(OBJECT_MOBILErp);
           type_v.push_back(OBJECT_MOBILEst);
        }
    }

    return code(type_v, angle, focus, minDist, maxDist, sens < 0, filter);
}

// Instruction "radar(type, angle, focus, min, max, sens, filter)".

bool CScriptFunctions::rRadar(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    return runRadar(var, [&result, user](std::vector<ObjectType> types, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter)
    {
        CObject* pThis = static_cast<CScript*>(user)->m_object;
        CObject* best = CObjectManager::GetInstancePointer()->Radar(pThis, types, angle, focus, minDist, maxDist, furthest, filter, true);

        if (best == nullptr)
        {
            result->SetPointer(nullptr);
        }
        else
        {
            result->SetPointer(best->GetBotVar());
        }

        return true;
    });
}

bool CScriptFunctions::rRadarAll(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    return runRadar(var, [&result, user](std::vector<ObjectType> types, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter)
    {
        CObject* pThis = static_cast<CScript*>(user)->m_object;
        std::vector<CObject*> best = CObjectManager::GetInstancePointer()->RadarAll(pThis, types, angle, focus, minDist, maxDist, furthest, filter, true);

        int i = 0;
        result->SetInit(CBotVar::InitType::DEF);
        for (CObject* obj : best)
        {
            result->GetItem(i++, true)->SetPointer(obj->GetBotVar());
        }

        return true;
    });
}


// Monitoring a task.

bool CScriptFunctions::WaitForForegroundTask(CScript* script, CBotVar* result, int &exception)
{
    assert(script->m_taskExecutor->IsForegroundTask());
    Error err = script->m_taskExecutor->GetForegroundTask()->IsEnded();
    if ( err != ERR_CONTINUE )  // task terminated?
    {
        script->m_taskExecutor->StopForegroundTask();

        script->m_bContinue = false;

        if ( err == ERR_STOP )  err = ERR_OK;
        result->SetValInt(err);  // indicates the error or ok
        if ( ShouldTaskStop(err, script->m_errMode) )
        {
            exception = err;
            return false;
        }
        return true;  // it's all over
    }

    script->m_bContinue = true;
    return false;  // not done
}

bool CScriptFunctions::WaitForBackgroundTask(CScript* script, CBotVar* result, int &exception)
{
    assert(script->m_taskExecutor->IsBackgroundTask());
    Error err = script->m_taskExecutor->GetBackgroundTask()->IsEnded();
    if ( err != ERR_CONTINUE )  // task terminated?
    {
        script->m_taskExecutor->StopBackgroundTask();

        script->m_bContinue = false;

        if ( err == ERR_STOP )  err = ERR_OK;
        result->SetValInt(err);  // indicates the error or ok
        if ( ShouldTaskStop(err, script->m_errMode) )
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

bool CScriptFunctions::ShouldTaskStop(Error err, int errMode)
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
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
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
        array   = nullptr;

        if ( var != nullptr )
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
            while ( array != nullptr )
            {
                if (array->GetValInt() == OBJECT_MOBILEpr)
                {
                    type_v.push_back(OBJECT_MOBILEwt);
                    type_v.push_back(OBJECT_MOBILEtt);
                    type_v.push_back(OBJECT_MOBILEft);
                    type_v.push_back(OBJECT_MOBILEit);
                    type_v.push_back(OBJECT_MOBILErp);
                    type_v.push_back(OBJECT_MOBILEst);
                }
                type_v.push_back(static_cast<ObjectType>(array->GetValInt()));
                array = array->GetNext();
            }
        }
        else
        {
            if (type != OBJECT_NULL && type != OBJECT_MOBILEpr)
            {
                type_v.push_back(static_cast<ObjectType>(type));
            }
            else if (type == OBJECT_MOBILEpr)
            {
                type_v.push_back(OBJECT_MOBILEwt);
                type_v.push_back(OBJECT_MOBILEtt);
                type_v.push_back(OBJECT_MOBILEft);
                type_v.push_back(OBJECT_MOBILEit);
                type_v.push_back(OBJECT_MOBILErp);
                type_v.push_back(OBJECT_MOBILEst);
            }
        }

        pBest = CObjectManager::GetInstancePointer()->Radar(pThis, type_v, 0.0f, 45.0f*Math::PI/180.0f, 0.0f, 20.0f, false, FILTER_NONE, true);

        if (pThis->Implements(ObjectInterfaceType::Old))
        {
            script->m_main->StartDetectEffect(dynamic_cast<COldObject*>(pThis), pBest);
        }

        if ( pBest == nullptr )
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
    if ( !WaitForForegroundTask(script, result, exception) )  return false;  // not finished
    result->SetValFloat(*script->m_returnValue);
    return true;
}


// Compilation of the instruction "direction(pos)".

CBotTypResult CScriptFunctions::cDirection(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "direction(pos)".

bool CScriptFunctions::rDirection(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject*        pThis = static_cast<CScript*>(user)->m_object;
    glm::vec3    iPos, oPos;
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

    if ( oType != OBJECT_MOBILEfb &&  // allowed only for builder bots && humans
         oType != OBJECT_MOBILEtb &&
         oType != OBJECT_MOBILEwb &&
         oType != OBJECT_MOBILEib &&
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
    result->SetValInt(err); // indicates the error or ok
    if ( err != ERR_OK )
    {
        if ( script->m_errMode == ERM_STOP )
        {
            exception = err;
            return false;
        }
        return true;
    }

    return WaitForForegroundTask(script, result, exception);

}

// Instruction "flag(color)"

bool CScriptFunctions::rFlag(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    ObjectType  oType;
    int         color;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )
    {
        oType = pThis->GetType();
        if ( oType != OBJECT_MOBILEfs &&  // allowed only for sniffer bots && humans
             oType != OBJECT_MOBILEts &&
             oType != OBJECT_MOBILEws &&
             oType != OBJECT_MOBILEis &&
             oType != OBJECT_HUMAN    &&
             oType != OBJECT_TECH      )
        {
            err = ERR_WRONG_BOT; // Wrong object
        }
        else
        {
            if ( var == nullptr )
            {
                color = 0;
            }
            else
            {
                color = var->GetValInt();
                if ( color < 0 || color > static_cast<int>(TraceColor::Violet) ) color = 0;
            }
            err = script->m_taskExecutor->StartTaskFlag(TFL_CREATE, color);
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
    return WaitForForegroundTask(script, result, exception);
}

// Instruction "deflag()"

bool CScriptFunctions::rDeflag(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    ObjectType  oType;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )
    {
        oType = pThis->GetType();
        if ( oType != OBJECT_MOBILEfs &&  // allowed only for sniffer bots && humans
             oType != OBJECT_MOBILEts &&
             oType != OBJECT_MOBILEws &&
             oType != OBJECT_MOBILEis &&
             oType != OBJECT_HUMAN    &&
             oType != OBJECT_TECH      )
        {
            err = ERR_WRONG_BOT; // Wrong object
        }
        else
        {
            err = script->m_taskExecutor->StartTaskFlag(TFL_DELETE, 0);
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
    return WaitForForegroundTask(script, result, exception);
}

// Compilation of the instruction "produce(pos, angle, type[, scriptName[, power[, team]]])"
// or "produce(type[, power])".

CBotTypResult CScriptFunctions::cProduce(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);

    if ( var->GetType() <= CBotTypDouble )
    {
        var = var->GetNext();
        if ( var != nullptr )
        {
            if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
            var = var->GetNext();
        }
    }
    else
    {
        ret = cPoint(var, user);
        if ( ret.GetType() != 0 )  return ret;

        if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
        if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();

        if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
        if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
        var = var->GetNext();

        if ( var != nullptr )
        {
            if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
            var = var->GetNext();

            if ( var != nullptr )
            {
                if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
                var = var->GetNext();

                if ( var != nullptr )
                {
                    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
                    var = var->GetNext();
                }
            }
        }
    }

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "produce(pos, angle, type[, scriptName[, power[, team]]])"
// or "produce(type[, power])".

bool CScriptFunctions::rProduce(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    me = script->m_object;
    std::string name = "";

    ObjectCreateParams params;
    params.angle = 0.0f;
    params.type = OBJECT_NULL;
    params.power = 0.0f;
    params.team = 0;

    if ( var->GetType() <= CBotTypDouble )
    {
        params.type = static_cast<ObjectType>(var->GetValInt());
        var = var->GetNext();

        params.pos = me->GetPosition();

        glm::vec3 rotation = me->GetRotation() + me->GetTilt();
        params.angle = rotation.y;

        if ( var != nullptr )
            params.power = var->GetValFloat();
        else
            params.power = -1.0f;
    }
    else
    {
        if ( !GetPoint(var, exception, params.pos) )  return true;

        params.angle = var->GetValFloat()*Math::PI/180.0f;
        var = var->GetNext();

        params.type = static_cast<ObjectType>(var->GetValInt());
        var = var->GetNext();

        if ( var != nullptr )
        {
            name = var->GetValString();
            var = var->GetNext();
            if ( var != nullptr )
            {
                params.power = var->GetValFloat();
                var = var->GetNext();
                if ( var != nullptr )
                {
                    params.team = var->GetValInt();
                }
            }
            else
            {
                params.power = -1.0f;
            }
        }
        else
        {
            params.power = -1.0f;
        }
    }

    CObject* object = nullptr;

    if ( params.type == OBJECT_ANT    ||
        params.type == OBJECT_SPIDER ||
        params.type == OBJECT_BEE    ||
        params.type == OBJECT_WORM   )
    {
        object = CObjectManager::GetInstancePointer()->CreateObject(params);
        params.type = OBJECT_EGG;
        CObjectManager::GetInstancePointer()->CreateObject(params);
        if (object->Implements(ObjectInterfaceType::Programmable))
        {
            dynamic_cast<CProgrammableObject&>(*object).SetActivity(false);
        }
    }
    else
    {
        if ((params.type == OBJECT_POWER || params.type == OBJECT_ATOMIC) && params.power == -1.0f)
        {
            params.power = 1.0f;
        }
        bool exists = IsValidObjectTypeId(params.type) && params.type != OBJECT_NULL && params.type != OBJECT_MAX && params.type != OBJECT_MOBILEpr;
        if (exists)
        {
            object = CObjectManager::GetInstancePointer()->CreateObject(params);
        }
        if (object == nullptr)
        {
            result->SetValInt(1);  // error
            return true;
        }
        if (params.type == OBJECT_MOBILEdr)
        {
            assert(object->Implements(ObjectInterfaceType::Old)); // TODO: temporary hack
            dynamic_cast<COldObject&>(*object).SetManual(true);
        }
        script->m_main->CreateShortcuts();
    }

    if (!name.empty())
    {
        std::string name2 = StrUtils::ToString(InjectLevelPathsForCurrentLevel(name, "ai"));
        if (object->Implements(ObjectInterfaceType::Programmable))
        {
            CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(object);
            Program* program = programStorage->AddProgram();
            programStorage->ReadProgram(program, name2.c_str());
            program->readOnly = true;
            dynamic_cast<CProgrammableObject&>(*object).RunProgram(program);
        }
    }

    result->SetValInt(0);  // no error
    return true;
}


// Compilation of the instruction "distance(p1, p2)".

CBotTypResult CScriptFunctions::cDistance(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "distance(p1, p2)".

bool CScriptFunctions::rDistance(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    glm::vec3    p1, p2;
    float       value;

    if ( !GetPoint(var, exception, p1) )  return true;
    if ( !GetPoint(var, exception, p2) )  return true;

    value = glm::distance(p1, p2);
    result->SetValFloat(value/g_unit);
    return true;
}

// Instruction "distance2d(p1, p2)".

bool CScriptFunctions::rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    glm::vec3    p1, p2;
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

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypIntrinsic, "point");
}

// Instruction "space(center, rMin, rMax, dist)".

bool CScriptFunctions::rSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    CBotVar*    pSub;
    glm::vec3    center;
    float       rMin, rMax, dist;

    rMin = 10.0f*g_unit;
    rMax = 50.0f*g_unit;
    dist =  4.0f*g_unit;

    if ( var == nullptr )
    {
        center = pThis->GetPosition();
    }
    else
    {
        if ( !GetPoint(var, exception, center) )  return true;

        if ( var != nullptr )
        {
            rMin = var->GetValFloat()*g_unit;
            var = var->GetNext();

            if ( var != nullptr )
            {
                rMax = var->GetValFloat()*g_unit;
                var = var->GetNext();

                if ( var != nullptr )
                {
                    dist = var->GetValFloat()*g_unit;
                    var = var->GetNext();
                }
            }
        }
    }

    bool success = script->m_main->FreeSpace(center, rMin, rMax, dist, pThis);

    if ( result != nullptr )
    {
        pSub = result->GetItemList();
        if ( pSub != nullptr )
        {
            if (success)
            {
                pSub->SetValFloat(center.x / g_unit);
                pSub = pSub->GetNext();  // "y"
                pSub->SetValFloat(center.z / g_unit);
                pSub = pSub->GetNext();  // "z"
                pSub->SetValFloat(center.y / g_unit);
            }
            else
            {
                pSub->SetValFloat(center.x);
                pSub = pSub->GetNext();  // "y"
                pSub->SetValFloat(center.y);
                pSub = pSub->GetNext();  // "z"
                pSub->SetValFloat(center.z);
            }
        }
    }
    return true;
}

CBotTypResult CScriptFunctions::cFlatSpace(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypIntrinsic, "point");
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypIntrinsic, "point");
}

bool CScriptFunctions::rFlatSpace(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    CBotVar*    pSub;
    glm::vec3    center;
    float       flatMin, rMin, rMax, dist;

    rMin = 10.0f*g_unit;
    rMax = 50.0f*g_unit;
    dist =  4.0f*g_unit;


    if ( !GetPoint(var, exception, center) )  return true;

    flatMin = var->GetValFloat()*g_unit;
    var = var->GetNext();

    if ( var != nullptr )
    {
        rMin = var->GetValFloat()*g_unit;
        var = var->GetNext();

        if ( var != nullptr )
        {
            rMax = var->GetValFloat()*g_unit;
            var = var->GetNext();

            if ( var != nullptr )
            {
                dist = var->GetValFloat()*g_unit;
                var = var->GetNext();
            }
        }
    }
    script->m_main->FlatFreeSpace(center, flatMin, rMin, rMax, dist, pThis);

    if ( result != nullptr )
    {
        pSub = result->GetItemList();
        if ( pSub != nullptr )
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

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "flatground(center, rMax)".

bool CScriptFunctions::rFlatGround(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    glm::vec3    center;
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
    return WaitForForegroundTask(script, result, exception);
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
    return WaitForForegroundTask(script, result, exception);
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
    return WaitForForegroundTask(script, result, exception);
}

// Compilation of the instruction "goto(pos, altitude, crash, goal)".

CBotTypResult CScriptFunctions::cGoto(CBotVar* &var, void* user)
{
    CBotTypResult   ret;

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "goto(pos, altitude, mode)".

bool CScriptFunctions::rGoto(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*        script = static_cast<CScript*>(user);
    glm::vec3        pos;
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

        if ( var != nullptr )
        {
            altitude = var->GetValFloat()*g_unit;

            var = var->GetNext();
            if ( var != nullptr )
            {
                goal = static_cast<TaskGotoGoal>(var->GetValInt());

                var = var->GetNext();
                if ( var != nullptr )
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
    return WaitForForegroundTask(script, result, exception);
}

// Compilation "grab/drop(oper)".

CBotTypResult CScriptFunctions::cGrabDrop(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
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
        if ( var == nullptr )
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
    return WaitForForegroundTask(script, result, exception);
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
        if ( var == nullptr )  type = TMA_FFRONT;
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
    return WaitForForegroundTask(script, result, exception);
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
    return WaitForForegroundTask(script, result, exception);
}

// Compilation of the instruction "receive(nom, power)".

CBotTypResult CScriptFunctions::cReceive(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "receive(nom, power)".

bool CScriptFunctions::rReceive(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    Error       err;
    std::string p;
    float       power;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        p = var->GetValString();
        var = var->GetNext();

        power = 10.0f*g_unit;
        if ( var != nullptr )
        {
            power = var->GetValFloat()*g_unit;
            var = var->GetNext();
        }

        err = script->m_taskExecutor->StartTaskInfo(p.c_str(), 0.0f, power, false);
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
            result->SetValFloat(nanf(""));
            return true;
        }

        CExchangePost* exchangePost = dynamic_cast<CTaskInfo&>(*script->m_taskExecutor->GetForegroundTask()).FindExchangePost(power);
        script->m_returnValue = exchangePost->GetInfoValue(p);
    }
    if ( !WaitForForegroundTask(script, result, exception) )  return false;  // not finished

    if ( !script->m_returnValue.has_value() )
    {
        result->SetValFloat(nanf(""));
    }
    else
    {
        result->SetValFloat(*script->m_returnValue);
    }
    return true;
}

// Compilation of the instruction "send(nom, value, power)".

CBotTypResult CScriptFunctions::cSend(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypFloat);
}

// Instruction "send(nom, value, power)".

bool CScriptFunctions::rSend(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    Error       err;
    std::string p;
    float       value, power;

    exception = 0;

    if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
    {
        p = var->GetValString();
        var = var->GetNext();

        value = var->GetValFloat();
        var = var->GetNext();

        power = 10.0f*g_unit;
        if ( var != nullptr )
        {
            power = var->GetValFloat()*g_unit;
            var = var->GetNext();
        }

        err = script->m_taskExecutor->StartTaskInfo(p.c_str(), value, power, true);
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
    return WaitForForegroundTask(script, result, exception);
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
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypBoolean);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "deleteinfo(name, power)".

bool CScriptFunctions::rDeleteInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;

    exception = 0;

    std::string infoName = var->GetValString();
    var = var->GetNext();

    float power = 10.0f*g_unit;
    if (var != nullptr)
    {
        power = var->GetValFloat()*g_unit;
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
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString )  return CBotTypResult(CBotErrBadString);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypBoolean);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypBoolean);
}

// Instruction "testinfo(name, power)".

bool CScriptFunctions::rTestInfo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CObject* pThis = static_cast<CScript*>(user)->m_object;

    exception = 0;

    std::string infoName = var->GetValString();
    var = var->GetNext();

    float power = 10.0f*g_unit;
    if (var != nullptr)
    {
        power = var->GetValFloat()*g_unit;
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
    return WaitForForegroundTask(script, result, exception);
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
    return WaitForForegroundTask(script, result, exception);
}

// Compilation "shield(oper, radius)".

CBotTypResult CScriptFunctions::cShield(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

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
            dynamic_cast<CShielder&>(*pThis).SetShieldRadius(radius);
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
            dynamic_cast<CShielder&>(*pThis).SetShieldRadius(radius);
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
        if ( var == nullptr ) return CBotTypResult(CBotErrLowParam);
        CBotTypResult ret = cPoint(var, user);
        if ( ret.GetType() != 0 )  return ret;
        if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    }
    else if ( type == OBJECT_SPIDER )
    {
        if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
    }
    else
    {
        if ( var != nullptr )
        {
            if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
            var = var->GetNext();
            if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
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
    glm::vec3    impact;
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
            if ( var == nullptr )  delay = 0.0f;
            else             delay = var->GetValFloat();
            if ( delay < 0.0f ) delay = -delay;
            err = script->m_taskExecutor->StartTaskFire(delay);
        }
        result->SetValInt(err); // indicates the error or ok
        if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopForegroundTask();
            if ( script->m_errMode == ERM_STOP )
            {
                exception = err;
                return false;
            }
            return true;
        }
    }
    return WaitForForegroundTask(script, result, exception);
}

// Compilation of the instruction "aim(x, y)".

CBotTypResult CScriptFunctions::cAim(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypFloat);
}

// Instruction "aim(dir)".

bool CScriptFunctions::rAim(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    float       x, y;
    Error       err;

    exception = 0;

    if ( !script->m_taskExecutor->IsBackgroundTask() )  // no task in progress?
    {
        x = var->GetValFloat();
        var = var->GetNext();
        var == nullptr ? y=0.0f : y=var->GetValFloat();
        err = script->m_taskExecutor->StartTaskGunGoal(x*Math::PI/180.0f, y*Math::PI/180.0f);
        if ( err == ERR_AIM_IMPOSSIBLE )
        {
            result->SetValInt(err);  // shows the error
        }
        else if ( err != ERR_OK )
        {
            script->m_taskExecutor->StopBackgroundTask();
            result->SetValInt(err);  // shows the error
            return true;
        }
    }
    return WaitForBackgroundTask(script, result, exception);
}

// Compilation of the instruction "motor(left, right)".

CBotTypResult CScriptFunctions::cMotor(CBotVar* &var, void* user)
{
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

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

    if ( dynamic_cast<CBaseAlien*>(pThis) != nullptr && dynamic_cast<CBaseAlien&>(*pThis).GetFixed() )  // ant on the back?
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

    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    ret = cPoint(var, user);
    if ( ret.GetType() != 0 )  return ret;

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "topo(pos)".

bool CScriptFunctions::rTopo(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    glm::vec3    pos;
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
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() != CBotTypString &&
        var->GetType() != CBotTypBoolean &&
        var->GetType() >  CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    return CBotTypResult(CBotErrOverParam);
}

// Instruction "message(string, type)".

bool CScriptFunctions::rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    std::string p;
    Ui::TextType    type;

    p = var->GetValString();

    type = Ui::TT_MESSAGE;
    var = var->GetNext();
    if ( var != nullptr )
    {
        type = static_cast<Ui::TextType>(var->GetValInt());
    }

    script->m_main->GetDisplayText()->DisplayText(p.c_str(), script->m_object, 10.0f, type);

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
    value = dynamic_cast<CProgrammableObject&>(*pThis).GetCmdLine(rank);
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
    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
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

    if (!pThis->Implements(ObjectInterfaceType::TraceDrawing))
    {
        result->SetValInt(ERR_WRONG_OBJ);
        if ( script->m_errMode == ERM_STOP )
        {
            exception = ERR_WRONG_OBJ;
            return false;
        }
        return true;
    }

    CTraceDrawingObject* traceDrawing = dynamic_cast<CTraceDrawingObject*>(pThis);

    exception = 0;

    if ( var != nullptr )
    {
        color = var->GetValInt();
        if ( color <  0 )  color =  0;
        if ( color > static_cast<int>(TraceColor::Max) )  color = static_cast<int>(TraceColor::Max);
        traceDrawing->SetTraceColor(static_cast<TraceColor>(color));

        var = var->GetNext();
        if ( var != nullptr )
        {
            width = var->GetValFloat();
            if ( width < 0.1f )  width = 0.1f;
            if ( width > 1.0f )  width = 1.0f;
            traceDrawing->SetTraceWidth(width);
        }
    }
    traceDrawing->SetTraceDown(true);

    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
        {
            err = script->m_taskExecutor->StartTaskPen(traceDrawing->GetTraceDown(), traceDrawing->GetTraceColor());
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
        return WaitForForegroundTask(script, result, exception);
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

    exception = 0;

    if (!pThis->Implements(ObjectInterfaceType::TraceDrawing))
    {
        result->SetValInt(ERR_WRONG_OBJ);
        if ( script->m_errMode == ERM_STOP )
        {
            exception = ERR_WRONG_OBJ;
            return false;
        }
        return true;
    }

    CTraceDrawingObject* traceDrawing = dynamic_cast<CTraceDrawingObject*>(pThis);
    traceDrawing->SetTraceDown(false);

    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
        {
            err = script->m_taskExecutor->StartTaskPen(traceDrawing->GetTraceDown(), traceDrawing->GetTraceColor());
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
        return WaitForForegroundTask(script, result, exception);
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

    exception = 0;

    if (!pThis->Implements(ObjectInterfaceType::TraceDrawing))
    {
        result->SetValInt(ERR_WRONG_OBJ);
        if ( script->m_errMode == ERM_STOP )
        {
            exception = ERR_WRONG_OBJ;
            return false;
        }
        return true;
    }

    CTraceDrawingObject* traceDrawing = dynamic_cast<CTraceDrawingObject*>(pThis);

    color = var->GetValInt();
    if ( color <  0 )  color =  0;
    if ( color > static_cast<int>(TraceColor::Max) )  color = static_cast<int>(TraceColor::Max);
    traceDrawing->SetTraceColor(static_cast<TraceColor>(color));

    if ( pThis->GetType() == OBJECT_MOBILEdr )
    {
        if ( !script->m_taskExecutor->IsForegroundTask() )  // no task in progress?
        {
            err = script->m_taskExecutor->StartTaskPen(traceDrawing->GetTraceDown(), traceDrawing->GetTraceColor());
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
        return WaitForForegroundTask(script, result, exception);
    }
    else
    {
        return true;
    }
}

// Instruction "penwidth()".

bool CScriptFunctions::rPenWidth(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);
    CObject*    pThis = script->m_object;
    float       width;

    exception = 0;

    if (!pThis->Implements(ObjectInterfaceType::TraceDrawing))
    {
        result->SetValInt(ERR_WRONG_OBJ);
        if ( script->m_errMode == ERM_STOP )
        {
            exception = ERR_WRONG_OBJ;
            return false;
        }
        return true;
    }

    CTraceDrawingObject* traceDrawing = dynamic_cast<CTraceDrawingObject*>(pThis);

    width = var->GetValFloat();
    if ( width < 0.1f )  width = 0.1f;
    if ( width > 1.0f )  width = 1.0f;
    traceDrawing->SetTraceWidth(width);
    return true;
}

// Compilation of the instruction with one object parameter

CBotTypResult CScriptFunctions::cOneObject(CBotVar* &var, void* user)
{
    if ( var != nullptr )
    {
        var = var->GetNext();
        if ( var == nullptr )  return CBotTypResult(CBotTypFloat);
    }
    else
        return CBotTypResult(CBotTypFloat);

    return CBotTypResult(CBotErrOverParam);

}

// Instruction "camerafocus(object)".

bool CScriptFunctions::rCameraFocus(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    CScript*    script = static_cast<CScript*>(user);

    CObject* object;
    if (var == nullptr)
        object = script->m_object;
    else
        object = static_cast<CObject*>(var->GetUserPtr());

    script->m_main->SelectObject(object, false);

    result->SetValInt(ERR_OK);
    exception = ERR_OK;
    return true;
}


// Compilation of class "point".

CBotTypResult CScriptFunctions::cPointConstructor(CBotVar* pThis, CBotVar* &var)
{
    if ( !pThis->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadNum);

    if ( var == nullptr )  return CBotTypResult(0);  // ok if no parameter

    // First parameter (x):
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Second parameter (y):
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Third parameter (z):
    if ( var == nullptr )  // only 2 parameters?
    {
        return CBotTypResult(0);  // this function returns void
    }

    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(0);  // this function returns void
}

//Execution of the class "point".

bool CScriptFunctions::rPointConstructor(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception, void* user)
{
    CBotVar     *pX, *pY, *pZ;

    if ( var == nullptr )  return true;  // constructor with no parameters is ok

    if ( var->GetType() > CBotTypDouble )
    {
        Exception = CBotErrBadNum;  return false;
    }

    pX = pThis->GetItem("x");
    if ( pX == nullptr )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pX->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var == nullptr )
    {
        Exception = CBotErrLowParam;  return false;
    }

    if ( var->GetType() > CBotTypDouble )
    {
        Exception = CBotErrBadNum;  return false;
    }

    pY = pThis->GetItem("y");
    if ( pY == nullptr )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pY->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var == nullptr )
    {
        return true;  // ok with only two parameters
    }

    pZ = pThis->GetItem("z");
    if ( pZ == nullptr )
    {
        Exception = CBotErrUndefItem;  return false;
    }
    pZ->SetValFloat( var->GetValFloat() );
    var = var->GetNext();

    if ( var != nullptr )
    {
        Exception = CBotErrOverParam;  return false;
    }

    return  true;  // no interruption
}

class CBotFileColobot : public CBotFile
{
public:
    static int m_numFilesOpen;

    CBotFileColobot(const std::string& filename, CBotFileAccessHandler::OpenMode mode)
    {
        if (mode == CBotFileAccessHandler::OpenMode::Read)
        {
            auto is = std::make_unique<CInputStream>(filename);
            if (is->is_open())
            {
                m_file = std::move(is);
            }
        }
        else if (mode == CBotFileAccessHandler::OpenMode::Write)
        {
            auto os = std::make_unique<COutputStream>(filename);
            if (os->is_open())
            {
                m_file = std::move(os);
            }
        }
        else if (mode == CBotFileAccessHandler::OpenMode::Append)
        {
            auto os = std::make_unique<COutputStream>(filename, std::ios_base::app);
            if (os->is_open())
            {
                m_file = std::move(os);
            }
        }

        if (Opened())
        {
            GetLogger()->Info("CBot open file '%%', mode '%%'", filename, static_cast<char>(mode));
            m_numFilesOpen++;
        }
    }

    ~CBotFileColobot() override
    {
        if (Opened())
        {
            GetLogger()->Info("CBot close file");
            m_numFilesOpen--;
        }

        std::ios* file = m_file.get();
        CInputStream* is = dynamic_cast<CInputStream*>(file);
        if(is != nullptr) is->close();
        COutputStream* os = dynamic_cast<COutputStream*>(file);
        if(os != nullptr) os->close();
    }

    virtual bool Opened() override
    {
        return m_file != nullptr;
    }

    virtual bool Errored() override
    {
        return m_file->bad();
    }

    virtual bool IsEOF() override
    {
        return m_file->eof();
    }

    virtual std::string ReadLine() override
    {
        CInputStream* is = dynamic_cast<CInputStream*>(m_file.get());
        assert(is != nullptr);

        std::string line;
        std::getline(*is, line);
        return line;
    }

    virtual void Write(const std::string& s) override
    {
        COutputStream* os = dynamic_cast<COutputStream*>(m_file.get());
        assert(os != nullptr);

        *os << s;
    }

private:
    std::unique_ptr<std::ios> m_file;
};
int CBotFileColobot::m_numFilesOpen = 0;

class CBotFileAccessHandlerColobot : public CBotFileAccessHandler
{
public:
    virtual std::unique_ptr<CBotFile> OpenFile(const std::string& filename, OpenMode mode) override
    {
        return std::make_unique<CBotFileColobot>(PrepareFilename(filename), mode);
    }

    virtual bool DeleteFile(const std::string& filename) override
    {
        std::string fname = PrepareFilename(filename);
        GetLogger()->Info("CBot delete file '%%'", fname);
        return CResourceManager::Remove(fname);
    }

private:
    static std::string PrepareFilename(const std::string& filename)
    {
        CResourceManager::CreateNewDirectory("files");
        return "files/" + filename;
    }
};



// Initializes all functions for module CBOT.

void CScriptFunctions::Init()
{
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

    CBotProgram::DefineNum("DestructionNone",           static_cast<int>(DestructionType::NoEffect));
    CBotProgram::DefineNum("DestructionExplosion",      static_cast<int>(DestructionType::Explosion));
    CBotProgram::DefineNum("DestructionExplosionWater", static_cast<int>(DestructionType::ExplosionWater));
    CBotProgram::DefineNum("DestructionBurn",           static_cast<int>(DestructionType::Burn));
    CBotProgram::DefineNum("DestructionDrowned",        static_cast<int>(DestructionType::Drowned));

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
    CBotProgram::DefineNum("BuildVault",            BUILD_SAFE);
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
    CBotProgram::DefineNum("ResearchBuilder",       RESEARCH_BUILDER);
    CBotProgram::DefineNum("ResearchTarget",        RESEARCH_TARGET);

    CBotProgram::DefineNum("PolskiPortalColobota", 1337);

    CBotClass* bc;

    // Add the class Point.
    bc = CBotClass::Create("point", nullptr, true);  // intrinsic class
    bc->AddItem("x", CBotTypFloat);
    bc->AddItem("y", CBotTypFloat);
    bc->AddItem("z", CBotTypFloat);
    bc->AddFunction("point", rPointConstructor, cPointConstructor);

    // Adds the class Object.
    bc = CBotClass::Create("object", nullptr);
    bc->AddItem("category",    CBotTypResult(CBotTypInt), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("position",    CBotTypResult(CBotTypClass, "point"), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("orientation", CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("pitch",       CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("roll",        CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("energyLevel", CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("shieldLevel", CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("temperature", CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("altitude",    CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("lifeTime",    CBotTypResult(CBotTypFloat), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("energyCell",  CBotTypResult(CBotTypPointer, "object"), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("load",        CBotTypResult(CBotTypPointer, "object"), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("id",          CBotTypResult(CBotTypInt), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("team",        CBotTypResult(CBotTypInt), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("dead",        CBotTypResult(CBotTypBoolean), CBotVar::ProtectionLevel::ReadOnly);
    bc->AddItem("velocity",    CBotTypResult(CBotTypClass, "point"), CBotVar::ProtectionLevel::ReadOnly);

    CBotProgram::AddFunction("endmission",rEndMission,cEndMission);
    CBotProgram::AddFunction("playmusic", rPlayMusic ,cPlayMusic);
    CBotProgram::AddFunction("stopmusic", rStopMusic ,cNull);

    CBotProgram::AddFunction("getbuild",          rGetBuild,          cNull);
    CBotProgram::AddFunction("getresearchenable", rGetResearchEnable, cNull);
    CBotProgram::AddFunction("getresearchdone",   rGetResearchDone,   cNull);
    CBotProgram::AddFunction("setbuild",          rSetBuild,          cOneInt);
    CBotProgram::AddFunction("setresearchenable", rSetResearchEnable, cOneInt);
    CBotProgram::AddFunction("setresearchdone",   rSetResearchDone,   cOneInt);

    CBotProgram::AddFunction("canbuild",        rCanBuild,        cOneIntReturnBool);
    CBotProgram::AddFunction("canresearch",     rCanResearch,     cOneIntReturnBool);
    CBotProgram::AddFunction("researched",      rResearched,      cOneIntReturnBool);
    CBotProgram::AddFunction("buildingenabled", rBuildingEnabled, cOneIntReturnBool);

    CBotProgram::AddFunction("build",           rBuild,           cOneInt);
    CBotProgram::AddFunction("flag",            rFlag,            cGrabDrop);
    CBotProgram::AddFunction("deflag",          rDeflag,          cNull);

    CBotProgram::AddFunction("retobject", rGetObject, cGetObject);
    CBotProgram::AddFunction("retobjectbyid", rGetObjectById, cGetObject);
    CBotProgram::AddFunction("delete",    rDelete,    cDelete);
    CBotProgram::AddFunction("search",    rSearch,    cSearch);
    CBotProgram::AddFunction("searchall", rSearchAll, cSearchAll);
    CBotProgram::AddFunction("radar",     rRadar,     cRadar);
    CBotProgram::AddFunction("radarall",  rRadarAll,  cRadarAll);
    CBotProgram::AddFunction("detect",    rDetect,    cDetect);
    CBotProgram::AddFunction("direction", rDirection, cDirection);
    CBotProgram::AddFunction("produce",   rProduce,   cProduce);
    CBotProgram::AddFunction("distance",  rDistance,  cDistance);
    CBotProgram::AddFunction("distance2d",rDistance2d,cDistance);
    CBotProgram::AddFunction("space",     rSpace,     cSpace);
    CBotProgram::AddFunction("flatspace", rFlatSpace, cFlatSpace);
    CBotProgram::AddFunction("flatground",rFlatGround,cFlatGround);
    CBotProgram::AddFunction("wait",      rWait,      cOneFloat);
    CBotProgram::AddFunction("move",      rMove,      cOneFloat);
    CBotProgram::AddFunction("turn",      rTurn,      cOneFloat);
    CBotProgram::AddFunction("goto",      rGoto,      cGoto);
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
    CBotProgram::AddFunction("aim",       rAim,       cAim);
    CBotProgram::AddFunction("motor",     rMotor,     cMotor);
    CBotProgram::AddFunction("jet",       rJet,       cOneFloat);
    CBotProgram::AddFunction("topo",      rTopo,      cTopo);
    CBotProgram::AddFunction("message",   rMessage,   cMessage);
    CBotProgram::AddFunction("cmdline",   rCmdline,   cOneFloat);
    CBotProgram::AddFunction("ismovie",   rIsMovie,   cNull);
    CBotProgram::AddFunction("errmode",   rErrMode,   cOneFloat);
    CBotProgram::AddFunction("ipf",       rIPF,       cOneFloat);
    CBotProgram::AddFunction("abstime",   rAbsTime,   cNull);
    CBotProgram::AddFunction("pendown",   rPenDown,   cPenDown);
    CBotProgram::AddFunction("penup",     rPenUp,     cNull);
    CBotProgram::AddFunction("pencolor",  rPenColor,  cOneFloat);
    CBotProgram::AddFunction("penwidth",  rPenWidth,  cOneFloat);
    CBotProgram::AddFunction("factory",   rFactory,   cFactory);
    CBotProgram::AddFunction("camerafocus", rCameraFocus, cOneObject);
    CBotProgram::AddFunction("takeoff",   rTakeOff,   cOneObject);
    CBotProgram::AddFunction("isbusy",    rIsBusy,    cIsBusy);
    CBotProgram::AddFunction("research",  rResearch,  cResearch);
    CBotProgram::AddFunction("destroy",   rDestroy,   cOneObject);

    SetFileAccessHandler(std::make_unique<CBotFileAccessHandlerColobot>());
}


// Updates the class Object.

void CScriptFunctions::uObject(CBotVar* botThis, void* user)
{
    CPhysics*   physics;
    CBotVar     *pVar, *pSub;
    ObjectType  type;
    glm::vec3    pos;
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
        pSub->SetValFloat(nanf(""));
        pSub = pSub->GetNext();  // "y"
        pSub->SetValFloat(nanf(""));
        pSub = pSub->GetNext();  // "z"
        pSub->SetValFloat(nanf(""));
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
    pVar->SetValFloat((Math::NormAngle(pos.z + Math::PI) - Math::PI)*180.0f/Math::PI);
    pVar = pVar->GetNext();  // "roll"
    pVar->SetValFloat((Math::NormAngle(pos.x + Math::PI) - Math::PI)*180.0f/Math::PI);

    // Updates the energy level of the object.
    pVar = pVar->GetNext();  // "energyLevel"
    value = object->GetEnergyLevel();
    pVar->SetValFloat(value);

    // Updates the shield level of the object.
    pVar = pVar->GetNext();  // "shieldLevel"
    if ( !obj->Implements(ObjectInterfaceType::Shielded) ) value = 1.0f;
    else value = dynamic_cast<CShieldedObject*>(object)->GetShield();
    pVar->SetValFloat(value);

    // Updates the temperature of the reactor.
    pVar = pVar->GetNext();  // "temperature"
    if ( !obj->Implements(ObjectInterfaceType::JetFlying) )  value = 0.0f;
    else value = 1.0f-dynamic_cast<CJetFlyingObject*>(object)->GetReactorRange();
    pVar->SetValFloat(value);

    // Updates the height above the ground.
    pVar = pVar->GetNext();  // "altitude"
    if ( physics == nullptr )  value = 0.0f;
    else                 value = physics->GetFloorHeight();
    pVar->SetValFloat(value/g_unit);

    // Updates the lifetime of the object.
    pVar = pVar->GetNext();  // "lifeTime"
    value = object->GetAbsTime();
    pVar->SetValFloat(value);

    // Updates the type of battery.
    pVar = pVar->GetNext();  // "energyCell"
    CSlottedObject *asSlotted = object->Implements(ObjectInterfaceType::Slotted) ? dynamic_cast<CSlottedObject*>(object) : nullptr;
    if (asSlotted != nullptr && asSlotted->MapPseudoSlot(CSlottedObject::Pseudoslot::POWER) >= 0)
    {
        CObject *power = asSlotted->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::POWER);
        if (power == nullptr)
        {
            pVar->SetPointer(nullptr);
        }
        else if (power->Implements(ObjectInterfaceType::Old))
        {
            pVar->SetPointer(power->GetBotVar());
        }
    }

    // Updates the transported object's type.
    pVar = pVar->GetNext();  // "load"
    if (asSlotted != nullptr && asSlotted->MapPseudoSlot(CSlottedObject::Pseudoslot::CARRYING) >= 0)
    {
        CObject* cargo = asSlotted->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING);
        if (cargo == nullptr)
        {
            pVar->SetPointer(nullptr);
        }
        else if (cargo->Implements(ObjectInterfaceType::Old))
        {
            pVar->SetPointer(cargo->GetBotVar());
        }
    }

    pVar = pVar->GetNext();  // "id"
    value = object->GetID();
    pVar->SetValInt(value);

    pVar = pVar->GetNext();  // "team"
    value = object->GetTeam();
    pVar->SetValInt(value);

    pVar = pVar->GetNext();  // "dead"
    value = object->IsDying();
    pVar->SetValInt(value);

    // Updates the velocity of the object.
    pVar = pVar->GetNext();  // "velocity"
    if (IsObjectBeingTransported(object) || physics == nullptr)
    {
        pSub = pVar->GetItemList();  // "x"
        pSub->SetValFloat(nanf(""));
        pSub = pSub->GetNext();  // "y"
        pSub->SetValFloat(nanf(""));
        pSub = pSub->GetNext();  // "z"
        pSub->SetValFloat(nanf(""));
    }
    else
    {
        glm::mat4 matRotate;
        Math::LoadRotationZXYMatrix(matRotate, object->GetRotation());
        pos = physics->GetLinMotion(MO_CURSPEED);
        pos = Math::Transform(matRotate, pos);

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
    if ( bc != nullptr )
    {
        bc->SetUpdateFunc(CScriptFunctions::uObject);
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
    if (permanent)
        CBotVar::Destroy(botVar);
}

bool CScriptFunctions::CheckOpenFiles()
{
    return CBotFileColobot::m_numFilesOpen > 0;
}
