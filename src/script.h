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

// script.h

#ifndef _SCRIPT_H_
#define _SCRIPT_H_


#include "event.h"


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CDisplayText;
class CEdit;
class CList;
class CObject;
class CTaskManager;
class CBotProgram;
class CRobotMain;
class CTerrain;
class CWater;



class CScript
{
public:
    CScript(CInstanceManager* iMan, CObject* object, CTaskManager** secondaryTask);
    ~CScript();

    static void InitFonctions();

    void        PutScript(CEdit* edit, char* name);
    BOOL        GetScript(CEdit* edit);
    BOOL        RetCompile();

    void        GetTitle(char* buffer);

    void        SetStepMode(BOOL bStep);
    BOOL        Run();
    BOOL        Continue(const Event &event);
    BOOL        Step(const Event &event);
    void        Stop();
    BOOL        IsRunning();
    BOOL        IsContinue();
    BOOL        GetCursor(int &cursor1, int &cursor2);
    void        UpdateList(CList* list);
    void        ColorizeScript(CEdit* edit);
    BOOL        IntroduceVirus();

    int         RetError();
    void        GetError(char* buffer);

    void        New(CEdit* edit, char* name);
    BOOL        SendScript(char* text);
    BOOL        ReadScript(char* filename);
    BOOL        WriteScript(char* filename);
    BOOL        ReadStack(FILE *file);
    BOOL        WriteStack(FILE *file);
    BOOL        Compare(CScript* other);

    void        SetFilename(char *filename);
    char*       RetFilename();

protected:
    BOOL        IsEmpty();
    BOOL        CheckToken();
    BOOL        Compile();

public:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CInterface*     m_interface;
    CDisplayText*   m_displayText;
    CBotProgram*    m_botProg;
    CRobotMain*     m_main;
    CTerrain*       m_terrain;
    CWater*         m_water;
    CTaskManager*   m_primaryTask;
    CTaskManager**  m_secondaryTask;
    CObject*        m_object;

    int         m_ipf;          // number of instructions/second
    int         m_errMode;      // what to do in case of error
    int         m_len;          // length of the script (without <0>)
    char*           m_script;       // script ends with <0>
    BOOL            m_bRun;         // program during execution?
    BOOL            m_bStepMode;        // step by step
    BOOL            m_bContinue;        // external function to continue
    BOOL            m_bCompile;     // compilation ok?
    char            m_title[50];        // script title
    char            m_filename[50];     // file name
    char            m_token[50];        // missing instruction
    int         m_error;        // error (0=ok)
    int         m_cursor1;
    int         m_cursor2;
    Event           m_event;
    float           m_returnValue;
};


#endif //_SCRIPT_H_
