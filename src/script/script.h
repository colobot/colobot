/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file script/script.h
 * \brief CBot script runner
 */

#pragma once

#include "CBot/CBot.h"

#include <memory>
#include <string>
#include <boost/optional.hpp>


class COldObject;
class CTaskExecutorObject;
class CRobotMain;
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

    const std::string& GetTitle()const;

    void        SetStepMode(const bool bStep);
    bool        GetStepMode()const;
    bool        Run();
    bool        Continue();
    bool        Step();
    void        Stop();
    bool        IsRunning()const;
    bool        IsContinue()const;
    bool        GetCursor(std::size_t &cursor1, std::size_t &cursor2)const;
    void        UpdateList(Ui::CList* list);
    static void ColorizeScript(Ui::CEdit* edit, std::size_t rangeStart = 0, std::size_t rangeEnd = SIZE_MAX //std::numeric_limits<int>::max()
        );
    bool        IntroduceVirus();

    int         GetError()const;
    void        GetError(std::string& error)const;

    void        New(Ui::CEdit* edit, const char* name);
    bool        SendScript(const char* text);
    bool        ReadScript(const char* filename);
    bool        WriteScript(const char* filename);
    bool        ReadStack(FILE *file);
    bool        WriteStack(FILE *file);
    bool        Compare(const CScript* other)const;

    void        SetFilename(const std::string &filename);
    const std::string& GetFilename()const;

protected:
    bool        IsEmpty()const;
    bool        CheckToken();
    bool        Compile();

protected:
    COldObject*          m_object = nullptr;
    CTaskExecutorObject* m_taskExecutor = nullptr;

    Gfx::CEngine*       m_engine = nullptr;
    Ui::CInterface*     m_interface = nullptr;
    std::unique_ptr<CBot::CBotProgram> m_botProg;
    CRobotMain*         m_main = nullptr;
    Gfx::CTerrain*      m_terrain = nullptr;
    Gfx::CWater*        m_water = nullptr;

    int     m_ipf = 0;          // number of instructions/second
    int     m_errMode = 0;      // what to do in case of error
    std::size_t     m_len = 0;          // length of the script (without <0>)
    std::unique_ptr<char[]> m_script;       // script ends with <0>
    bool    m_bRun = false;         // program during execution?
    bool    m_bStepMode = false;        // step by step
    bool    m_bContinue = false;        // external function to continue
    bool    m_bCompile = false;     // compilation ok?
    std::string m_title = "";        // script title
    std::string m_mainFunction = "";
    std::string m_filename = "";     // file name
    std::string m_token = "";        // missing instruction
    int m_tokenUsed = 0, m_tokenAllowed = 0;
    CBot::CBotError m_error = CBot::CBotNoErr;        // error (0=ok)
    std::size_t   m_cursor1 = 0;
    std::size_t   m_cursor2 = 0;
    boost::optional<float> m_returnValue = boost::none;
};
