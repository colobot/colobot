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

/**
 * \file script/scriptfunc.h
 * \brief CBot script functions
 */

#pragma once

#include "CBot/CBotDll.h"

#include "common/global.h"

#include <string>
#include <unordered_map>
#include <memory>

class CObject;
class CScript;
class CExchangePost;


class CScriptFunctions
{
public:
    static void Init();
    static CBotVar* CreateObjectVar(CObject* obj);
    static void DestroyObjectVar(CBotVar* botVar, bool permanent);

private:
    static CBotTypResult cNull(CBotVar* &var, void* user);
    static CBotTypResult cOneFloat(CBotVar* &var, void* user);
    static CBotTypResult cTwoFloat(CBotVar* &var, void* user);
    static CBotTypResult cString(CBotVar* &var, void* user);
    static CBotTypResult cStringString(CBotVar* &var, void* user);
    static CBotTypResult cOneInt(CBotVar* &var, void* user);
    static CBotTypResult cOneIntReturnBool(CBotVar* &var, void* user);
    static CBotTypResult cEndMission(CBotVar* &var, void* user);
    static CBotTypResult cPlayMusic(CBotVar* &var, void* user);
    static CBotTypResult cGetObject(CBotVar* &var, void* user);
    static CBotTypResult cDelete(CBotVar* &var, void* user);
    static CBotTypResult cSearch(CBotVar* &var, void* user);
    static CBotTypResult cRadar(CBotVar* &var, void* user);
    static CBotTypResult cDetect(CBotVar* &var, void* user);
    static CBotTypResult cDirection(CBotVar* &var, void* user);
    static CBotTypResult cProduce(CBotVar* &var, void* user);
    static CBotTypResult cDistance(CBotVar* &var, void* user);
    static CBotTypResult cSpace(CBotVar* &var, void* user);
    static CBotTypResult cFlatSpace(CBotVar* &var, void* user);
    static CBotTypResult cFlatGround(CBotVar* &var, void* user);
    static CBotTypResult cGoto(CBotVar* &var, void* user);
    static CBotTypResult cGrabDrop(CBotVar* &var, void* user);
    static CBotTypResult cReceive(CBotVar* &var, void* user);
    static CBotTypResult cSend(CBotVar* &var, void* user);
    static CBotTypResult cDeleteInfo(CBotVar* &var, void* user);
    static CBotTypResult cTestInfo(CBotVar* &var, void* user);
    static CBotTypResult cShield(CBotVar* &var, void* user);
    static CBotTypResult cFire(CBotVar* &var, void* user);
    static CBotTypResult cAim(CBotVar* &var, void* user);
    static CBotTypResult cMotor(CBotVar* &var, void* user);
    static CBotTypResult cTopo(CBotVar* &var, void* user);
    static CBotTypResult cMessage(CBotVar* &var, void* user);
    static CBotTypResult cPenDown(CBotVar* &var, void* user);
    static CBotTypResult cOnePoint(CBotVar* &var, void* user);
    static CBotTypResult cPoint(CBotVar* &var, void* user);
    static CBotTypResult cOneObject(CBotVar* &var, void* user);


    static bool rSin(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCos(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTan(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPow(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rRand(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rAbs(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rFloor(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCeil(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rRound(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTrunc(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rEndMission(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPlayMusic(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rStopMusic(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGetBuild(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSetBuild(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSetResearchEnable(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSetResearchDone(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGetObjectById(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGetObject(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDelete(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSearch(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rRadar(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDetect(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDirection(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCanBuild(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCanResearch(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rResearched(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rBuildingEnabled(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rBuild(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rProduce(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDistance(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSpace(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rFlatSpace(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rFlatGround(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rWait(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rMove(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTurn(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGoto(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rFind(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGrab(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDrop(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSniff(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rReceive(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSend(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDeleteInfo(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTestInfo(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rThump(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rRecycle(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rShield(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rFire(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rAim(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rMotor(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rJet(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTopo(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rMessage(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCmdline(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rIsMovie(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rErrMode(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rIPF(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rAbsTime(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDeleteFile(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPenDown(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPenUp(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPenColor(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPenWidth(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCameraFocus(CBotVar* var, CBotVar* result, int& exception, void* user);


    static CBotTypResult cBusy(CBotVar* thisclass, CBotVar* &var);
    static CBotTypResult cFactory(CBotVar* thisclass, CBotVar* &var);
    static CBotTypResult cClassNull(CBotVar* thisclass, CBotVar* &var);
    static CBotTypResult cClassOneFloat(CBotVar* thisclass, CBotVar* &var);

    static bool rBusy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rFactory(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rResearch(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTakeOff(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDestroy(CBotVar* thisclass, CBotVar* var, CBotVar* result, int& exception, void* user);


    static CBotTypResult cfconstruct (CBotVar* pThis, CBotVar* &pVar);
    static CBotTypResult cfopen (CBotVar* pThis, CBotVar* &pVar);
    static CBotTypResult cfclose (CBotVar* pThis, CBotVar* &pVar);
    static CBotTypResult cfwrite (CBotVar* pThis, CBotVar* &pVar);
    static CBotTypResult cfread (CBotVar* pThis, CBotVar* &pVar);
    static CBotTypResult cfeof (CBotVar* pThis, CBotVar* &pVar);
    static bool rfconstruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    static bool rfdestruct (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    static bool rfopen (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    static bool rfclose (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    static bool rfwrite (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    static bool rfread (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);
    static bool rfeof (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user);

    static CBotTypResult cPointConstructor(CBotVar* pThis, CBotVar* &var);
    static bool rPointConstructor(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception, void* user);

    static void uObject(CBotVar* botThis, void* user);

public:
    static int m_numberOfOpenFiles;

private:
    static bool     Process(CScript* script, CBotVar* result, int &exception);
    static bool     ShouldProcessStop(Error err, int errMode);
    static CExchangePost* FindExchangePost(CObject* object, float power);

    static bool     FileClassOpenFile(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception);

    static std::unordered_map<int, std::unique_ptr<std::ios>> m_files;
    static int m_nextFile;
};
