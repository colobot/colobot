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
 * \file script/script.h
 * \brief CBot script runner
 */

#pragma once

#include "CBot/CBotDll.h"

#include "common/global.h"



class COldObject;
class CTaskExecutorObject;
class CRobotMain;
class CPauseManager;
class CScriptFunctions;

namespace Ui
{
class CEdit;
class CInterface;
class CList;
} /* Ui */

namespace Gfx
{
class CEngine;
class CTerrain;
class CWater;
} /* Gfx */


const int ERM_CONT = 0;     // if error -> continue
const int ERM_STOP = 1;     // if error -> stop


class CScript
{
friend class CScriptFunctions;
public:
    CScript(COldObject* object);
    ~CScript();

    void        PutScript(Ui::CEdit* edit, const char* name);
    bool        GetScript(Ui::CEdit* edit);
    bool        GetCompile();

    void        GetTitle(char* buffer);

    void        SetStepMode(bool bStep);
    bool        Run();
    bool        Continue();
    bool        Step();
    void        Stop();
    bool        IsRunning();
    bool        IsContinue();
    bool        GetCursor(int &cursor1, int &cursor2);
    void        UpdateList(Ui::CList* list);
    void        ColorizeScript(Ui::CEdit* edit);
    bool        IntroduceVirus();

    int         GetError();
    void        GetError(std::string& error);

    void        New(Ui::CEdit* edit, const char* name);
    bool        SendScript(const char* text);
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

protected:
    COldObject*          m_object;
    CTaskExecutorObject* m_taskExecutor;

    Gfx::CEngine*       m_engine;
    Ui::CInterface*     m_interface;
    CBotProgram*        m_botProg;
    CRobotMain*         m_main;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    CPauseManager*      m_pause;

    int     m_ipf;          // number of instructions/second
    int     m_errMode;      // what to do in case of error
    int     m_len;          // length of the script (without <0>)
    char*   m_script;       // script ends with <0>
    bool    m_bRun;         // program during execution?
    bool    m_bStepMode;        // step by step
    bool    m_bContinue;        // external function to continue
    bool    m_bCompile;     // compilation ok?
    char    m_title[50];        // script title
    char    m_mainFunction[50];
    char    m_filename[50];     // file name
    char    m_token[50];        // missing instruction
    int     m_error;        // error (0=ok)
    int     m_cursor1;
    int     m_cursor2;
    float   m_returnValue;
};
