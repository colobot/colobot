/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "script/script.h"

#include "common/restext.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/text.h"

#include "level/robotmain.h"

#include "object/old_object.h"

#include "script/cbottoken.h"

#include "ui/displaytext.h"

#include "ui/controls/edit.h"
#include "ui/controls/interface.h"
#include "ui/controls/list.h"

#include "CBot/CBot.h"


const int CBOT_IPF = 100;       // CBOT: default number of instructions / frame


// Object's constructor.

CScript::CScript(COldObject* object)
{
    assert(object->Implements(ObjectInterfaceType::Programmable));

    m_object        = object;
    assert(m_object->Implements(ObjectInterfaceType::TaskExecutor));
    m_taskExecutor  = dynamic_cast<CTaskExecutorObject*>(m_object);

    m_engine        = Gfx::CEngine::GetInstancePointer();
    m_main          = CRobotMain::GetInstancePointer();
    m_terrain       = m_main->GetTerrain();
    m_water         = m_engine->GetWater();
    m_interface     = m_main->GetInterface();

    m_ipf = CBOT_IPF;
    m_errMode = ERM_STOP;
    m_len = 0;
    m_bRun = false;
    m_bStepMode = false;
    m_bCompile = false;
    m_title[0] = 0;
    m_mainFunction[0] = 0;
    m_cursor1 = 0;
    m_cursor2 = 0;
    m_filename[0] = 0;
}

// Object's destructor.

CScript::~CScript()
{
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
        edit->SetText(m_script.get());
        edit->SetCursor(m_cursor2, m_cursor1);
        edit->ShowSelect();
    }
    m_interface->SetFocus(edit);
}

// The script takes a paved text.

bool CScript::GetScript(Ui::CEdit* edit)
{
    int len = edit->GetTextLength();
    m_script = MakeUniqueArray<char>(len+1);

    edit->GetText(m_script.get(), len+1);
    edit->GetCursor(m_cursor2, m_cursor1);
    m_len = strlen(m_script.get());

    if ( !CheckToken() )
    {
        edit->SetCursor(m_cursor2, m_cursor1);
        edit->ShowSelect();
        m_interface->SetFocus(edit);
        return false;
    }

    if ( !Compile() )
    {
        edit->SetCursor(m_cursor2, m_cursor1);
        edit->ShowSelect();
        m_interface->SetFocus(edit);
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
    for (int i = 0; i < m_len; i++)
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
    m_mainFunction[0] = 0;
    m_token[0] = 0;
    m_bCompile = false;

    for ( i=0 ; i<m_main->GetObligatoryToken() ; i++ )
    {
        used[i] = 0;  // token not used
    }

    allBt = CBotToken::CompileTokens(m_script.get(), error);
    bt = allBt;
    while ( bt != nullptr )
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
            strcpy(m_title, "<prohibited>");
            m_mainFunction[0] = 0;
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
            strcpy(m_title, "<obligatory>");
            m_mainFunction[0] = 0;
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
    m_mainFunction[0] = 0;
    m_bCompile = false;

    if ( IsEmpty() )  // program exist?
    {
        m_botProg.reset();
        return true;
    }

    if (m_botProg == nullptr)
    {
        m_botProg = MakeUnique<CBotProgram>(m_object->GetBotVar());
    }

    if ( m_botProg->Compile(m_script.get(), liste, this) )
    {
        if ( liste.GetSize() == 0 )
        {
            strcpy(m_title, "<extern missing>");
            m_mainFunction[0] = 0;
        }
        else
        {
            p = liste[0];
            i = 0;
            bool titleDone = false;
            while ( true )
            {
                if ( p[i] == 0 || p[i] == '(' )  break;
                if ( i >= 20 && !titleDone )
                {
                    m_title[i+0] = '.';
                    m_title[i+1] = '.';
                    m_title[i+2] = '.';
                    m_title[i+3] = 0;
                    titleDone = true;
                }
                if(!titleDone)
                    m_title[i] = p[i];
                m_mainFunction[i] = p[i];
                i ++;
            }
            if(!titleDone)
                m_title[i] = 0;
            m_mainFunction[i] = p[i];
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
        m_mainFunction[0] = 0;
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

bool CScript::GetStepMode()
{
    return m_bStepMode;
}


// Runs the program from the beginning.

bool CScript::Run()
{
    if (m_botProg == nullptr)  return false;
    if ( m_script == nullptr || m_len == 0 )  return false;
    if ( m_mainFunction[0] == 0 ) return false;

    if ( !m_botProg->Start(m_mainFunction) )  return false;

    m_bRun = true;
    m_bContinue = false;
    m_ipf = CBOT_IPF;
    m_errMode = ERM_STOP;

    if ( m_bStepMode )  // step by step mode?
    {
        Step();
    }

    return true;
}

// Continues the execution of current program.
// Returns true when execution is finished.

bool CScript::Continue()
{
    if (m_botProg == nullptr)  return true;
    if ( !m_bRun )  return true;

    if ( m_bStepMode )  // step by step mode?
    {
        if ( m_bContinue )  // instuction "move", "goto", etc. ?
        {
            if ( m_botProg->Run(this, 0) )
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
        }

        return false;
    }

    if ( m_botProg->Run(this, m_ipf) )
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

bool CScript::Step()
{
    if (m_botProg == nullptr)  return true;
    if ( !m_bRun )  return true;
    if ( !m_bStepMode )  return false;

    if ( m_botProg->Run(this, 0) )  // step mode
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

// Stops the program.

void CScript::Stop()
{
    if ( !m_bRun )  return;

    m_taskExecutor->StopForegroundTask();

    if (m_botProg != nullptr)
    {
        m_botProg->Stop();
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

    if (m_botProg == nullptr)  return false;
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

    if ( var == nullptr && baseName[0] != 0 )
    {
        sprintf(buffer, "%s = null;", baseName);
        list->SetItemName(rankList++, buffer);
        return;
    }

    index = 0;
    while ( var != nullptr )
    {
        var->Maj(nullptr, false);
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

    if (m_botProg == nullptr) return;

    total  = list->GetTotal();
    select = list->GetSelect();

    list->Flush();  // empty list
    m_botProg->GetRunPos(progName, cursor1, cursor2);
    if ( progName == nullptr )  return;

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

void CScript::ColorizeScript(Ui::CEdit* edit, int rangeStart, int rangeEnd)
{
    if (rangeEnd > edit->GetMaxChar())
        rangeEnd = edit->GetMaxChar();

    edit->SetFormat(rangeStart, rangeEnd, Gfx::FONT_HIGHLIGHT_COMMENT); // anything not processed is a comment

    // NOTE: Images are registered as index in some array, and that can be 0 which normally ends the string!
    std::string text = std::string(edit->GetText(), edit->GetMaxChar());
    text = text.substr(rangeStart, rangeEnd-rangeStart);

    int error;
    CBotToken* bt = CBotToken::CompileTokens(text.c_str(), error);
    while ( bt != nullptr )
    {
        CBotString bs = bt->GetString();
        const char* token = bs;
        int type = bt->GetType();

        int cursor1 = bt->GetStart();
        int cursor2 = bt->GetEnd();

        if (cursor1 < 0 || cursor2 < 0 || cursor1 == cursor2 || type == 0) { bt = bt->GetNext(); continue; } // seems to be a bug in CBot engine (how does it even still work? D:)

        cursor1 += rangeStart;
        cursor2 += rangeStart;

        Gfx::FontHighlight color = Gfx::FONT_HIGHLIGHT_NONE;
        if ((type == TokenTypVar || (type >= TokenKeyWord && type < TokenKeyWord+100)) && IsType(token)) // types (basic types are TokenKeyWord, classes are TokenTypVar)
        {
            color = Gfx::FONT_HIGHLIGHT_TYPE;
        }
        else if (type == TokenTypVar && IsFunction(token)) // functions
        {
            color = Gfx::FONT_HIGHLIGHT_TOKEN;
        }
        else if (type == TokenTypVar && (strcmp(token, "this") == 0 || strcmp(token, "super") == 0)) // this, super
        {
            color = Gfx::FONT_HIGHLIGHT_THIS;
        }
        else if (type >= TokenKeyWord && type < TokenKeyWord+100) // builtin keywords
        {
            color = Gfx::FONT_HIGHLIGHT_KEYWORD;
        }
        else if (type >= TokenKeyDeclare && type < TokenKeyDeclare+100) // TODO: no idea :P seems to never happen
        {
            color = Gfx::FONT_HIGHLIGHT_TYPE;
        }
        else if (type >= TokenKeyVal && type < TokenKeyVal+100) // true, false, null, nan
        {
            color = Gfx::FONT_HIGHLIGHT_CONST;
        }
        else if (type == TokenTypDef) // constants (object types etc.)
        {
            color = Gfx::FONT_HIGHLIGHT_CONST;
        }
        else if (type == TokenTypString || type == TokenTypNum) // string literals and numbers
        {
            color = Gfx::FONT_HIGHLIGHT_STRING;
        }

        assert(cursor1 < cursor2);
        edit->SetFormat(cursor1, cursor2, color);

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
        if ( p != nullptr )
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
    if (m_script == nullptr) return false;

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

    int iFound = 0;
    int     found[11*2];
    for ( int i=0 ; i<11 ; i++ )
    {
        int start = SearchToken(m_script.get(), names[i*2]);
        if ( start != -1 )
        {
            found[iFound++] = i*2;
            found[iFound++] = start;
        }
    }
    if ( iFound == 0 )  return false;

    int i = (rand()%(iFound/2))*2;
    int start = found[i+1];
    i     = found[i+0];

    auto newScript = MakeUniqueArray<char>(m_len + strlen(names[i+1]) + 1);
    strcpy(newScript.get(), m_script.get());
    m_script = std::move(newScript);

    DeleteToken(m_script.get(), start, strlen(names[i]));
    InsertToken(m_script.get(), start, names[i+1]);
    m_len = strlen(m_script.get());
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
    char    res[100];
    char    text[100];
    char    script[500];
    char    buffer[500];
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
    m_interface->SetFocus(edit);

    std::string sf = m_main->GetScriptFile();
    if ( !sf.empty() )  // Load an empty program specific?
    {
        CInputStream stream;
        stream.open(sf);

        if (stream.is_open())
        {
            len = stream.size();

            if ( len > 500-1 )  len = 500-1;
            stream.read(buffer, len);
            buffer[len] = 0;
            stream.close();

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
            m_interface->SetFocus(edit);
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
    Ui::CEdit*  edit;

    if (!CResourceManager::Exists(filename))  return false;

    m_script.reset();

    edit = m_interface->CreateEdit(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 0, EVENT_EDIT9);
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->ReadText(filename);
    GetScript(edit);
    m_interface->DeleteControl(EVENT_EDIT9);
    return true;
}

// Writes a script as a text file.

bool CScript::WriteScript(const char* filename)
{
    if ( m_script == nullptr )
    {
        CResourceManager::Remove(filename);
        return false;
    }

    Ui::CEdit* edit = m_interface->CreateEdit(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 0, EVENT_EDIT9);
    edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->SetText(m_script.get());
    edit->WriteText(filename);
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

    if (m_botProg == nullptr) return false;
    if ( !m_botProg->RestoreState(file) )  return false;

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

    return ( strcmp(m_script.get(), other->m_script.get()) == 0 );
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
