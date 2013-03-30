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

/**
 * \file script/script.h
 * \brief CBot script runner
 */

#pragma once


#include "common/event.h"

#include "CBot/CBotDll.h"

#include <stdio.h>


class CObject;
class CTaskManager;
class CRobotMain;

namespace Ui{
class CDisplayText;
class CEdit;
class CInterface;
class CList;
} /* Ui */ 

namespace Gfx {
class CEngine;
class CTerrain;
class CWater;
} /* Gfx */ 



class CScript
{
public:
    CScript(CObject* object, CTaskManager** secondaryTask);
    ~CScript();

    static void InitFonctions();

    void        PutScript(Ui::CEdit* edit, const char* name);
    bool        GetScript(Ui::CEdit* edit);
    bool        GetCompile();

    void        GetTitle(char* buffer);

    void        SetStepMode(bool bStep);
    bool        Run();
    bool        Continue(const Event &event);
    bool        Step(const Event &event);
    void        Stop();
    bool        IsRunning();
    bool        IsContinue();
    bool        GetCursor(int &cursor1, int &cursor2);
    void        UpdateList(Ui::CList* list);
    void        ColorizeScript(Ui::CEdit* edit);
    bool        IntroduceVirus();

    int         GetError();
    void        GetError(char* buffer);

    void        New(Ui::CEdit* edit, const char* name);
    bool        SendScript(char* text);
    bool        ReadScript(const char* filename);
    bool        WriteScript(const char* filename);
    bool        ReadStack(FILE *file);
    bool        WriteStack(FILE *file);
    bool        Compare(CScript* other);

    void        SetFilename(char *filename);
    char*       GetFilename();

protected:
    bool        IsEmpty();
    bool        CheckToken();
    bool        Compile();

private:

    static CBotTypResult cNull(CBotVar* &var, void* user);
    static CBotTypResult cOneFloat(CBotVar* &var, void* user);
    static CBotTypResult cTwoFloat(CBotVar* &var, void* user);
    static CBotTypResult cString(CBotVar* &var, void* user);
    static CBotTypResult cGetObject(CBotVar* &var, void* user);
    static CBotTypResult cDestroy(CBotVar* &var, void* user);
    static CBotTypResult cSearch(CBotVar* &var, void* user);
    static CBotTypResult cRadar(CBotVar* &var, void* user);
    static CBotTypResult cDetect(CBotVar* &var, void* user);
    static CBotTypResult cDirection(CBotVar* &var, void* user);
    static CBotTypResult cProduce(CBotVar* &var, void* user);
    static CBotTypResult cDistance(CBotVar* &var, void* user);
    static CBotTypResult cSpace(CBotVar* &var, void* user);
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


    static bool rSin(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rCos(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rTan(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rPow(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rRand(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rAbs(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rGetObject(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDestroy(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSearch(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rRadar(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDetect(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDirection(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rProduce(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDistance(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rDistance2d(CBotVar* var, CBotVar* result, int& exception, void* user);
    static bool rSpace(CBotVar* var, CBotVar* result, int& exception, void* user);
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

    static bool     Process(CScript* script, CBotVar* result, int &exception);
    static CObject* SearchInfo(CScript* script, CObject* object, float power);

protected:
    Gfx::CEngine*       m_engine;
    Ui::CInterface*         m_interface;
    Ui::CDisplayText*   m_displayText;
    CBotProgram*        m_botProg;
    CRobotMain*         m_main;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    CTaskManager*       m_primaryTask;
    CTaskManager**      m_secondaryTask;
    CObject*            m_object;

    int     m_ipf;          // number of instructions/second
    int     m_errMode;      // what to do in case of error
    int     m_len;          // length of the script (without <0>)
    char*   m_script;       // script ends with <0>
    bool    m_bRun;         // program during execution?
    bool    m_bStepMode;        // step by step
    bool    m_bContinue;        // external function to continue
    bool    m_bCompile;     // compilation ok?
    char    m_title[50];        // script title
    char    m_filename[50];     // file name
    char    m_token[50];        // missing instruction
    int     m_error;        // error (0=ok)
    int     m_cursor1;
    int     m_cursor2;
    Event   m_event;
    float   m_returnValue;
};


