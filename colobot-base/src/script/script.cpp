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


#include "script/script.h"

#include "CBot/CBot.h"

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

#include <libintl.h>

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
    m_script = "";
    m_bRun = false;
    m_bStepMode = false;
    m_bCompile = false;
    m_error = CBot::CBotNoErr;
    m_cursor1 = 0;
    m_cursor2 = 0;
}

// Object's destructor.

CScript::~CScript()
{
    
}


// Gives the script editable block of text.

void CScript::PutScript(Ui::CEdit* edit, const std::string& name)
{
    if ( m_script.empty() )
    {
        New(edit, name);
    }
    else
    {
        edit->SetText(m_script);
        edit->SetCursor(m_cursor2, m_cursor1);
        edit->ShowSelect();
    }
    m_interface->SetFocus(edit);
}

// The script takes a paved text.

bool CScript::GetScript(Ui::CEdit* edit)
{
    int len = edit->GetTextLength();
    m_script = edit->GetText(len + 1);

    edit->GetCursor(m_cursor2, m_cursor1);

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
    for (std::size_t i = 0; i < m_script.size(); i++)
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
    if ( !m_object->GetCheckToken() )  return true;

    m_error = CBot::CBotNoErr;
    m_title.clear();
    m_mainFunction.clear();
    m_token.clear();
    m_bCompile = false;

    std::map<std::string, int> used;
    std::map<std::string, int> cursor1;
    std::map<std::string, int> cursor2;

    auto tokens = CBot::CBotToken::CompileTokens(m_script);
    CBot::CBotToken* bt = tokens.get();
    while ( bt != nullptr )
    {
        std::string token = bt->GetString();

        // Store only the last occurrence of the token
        cursor1[token] = bt->GetStart();
        cursor2[token] = bt->GetEnd();

        used[token]++;

        bt = bt->GetNext();
    }

    for (const auto& it : m_main->GetObligatoryTokenList())
    {
        Error error = ERR_OK;
        int allowed = 0;
        if (it.second.max >= 0 && used[it.first] > it.second.max)
        {
            error = ERR_PROHIBITEDTOKEN;
            allowed = it.second.max;
            m_cursor1 = cursor1[it.first];
            m_cursor2 = cursor2[it.first];
        }
        if (it.second.min >= 0 && used[it.first] < it.second.min)
        {
            error = ERR_OBLIGATORYTOKEN;
            allowed = it.second.min;
        }

        if (error != ERR_OK)
        {
            m_token = it.first;
            m_tokenUsed = used[it.first];
            m_tokenAllowed = allowed;
            m_error = static_cast<CBot::CBotError>(error);
            m_title = "<incorrect instructions>";
            m_mainFunction.clear();
            return false;
        }
    }

    return true;
}

// Compile the script of a paved text.

static bool isValidRange(const std::string& script, int cursor1, int cursor2)
{
    return 0 <= cursor1
        && static_cast<size_t>(cursor1) <= script.size()
        && 0 <= cursor2
        && static_cast<size_t>(cursor2) <= script.size();
}

bool CScript::Compile()
{
    std::vector<std::string> functionList;
    std::string     p;

    m_error = CBot::CBotNoErr;
    m_cursor1 = 0;
    m_cursor2 = 0;
    m_title.clear();
    m_mainFunction.clear();
    m_bCompile = false;

    if ( IsEmpty() )  // program exist?
    {
        m_botProg.reset();
        return true;
    }

    if (m_botProg == nullptr)
    {
        m_botProg = std::make_unique<CBot::CBotProgram>(m_object->GetBotVar());
    }

    if ( m_botProg->Compile(m_script, functionList, this) )
    {
        if (functionList.empty())
        {
            m_title = "<extern missing>";
            m_mainFunction.clear();
        }
        else
        {
            m_mainFunction = functionList[0];
            m_title = m_mainFunction;
            if (m_title.length() >= 20)
            {
                m_title = m_title.substr(0, 20)+"...";
            }
        }
        m_bCompile = true;
        return true;
    }
    else
    {
        m_botProg->GetError(m_error, m_cursor1, m_cursor2);
        if ( !isValidRange(m_script, m_cursor1, m_cursor2) )
        {
            m_cursor1 = 0;
            m_cursor2 = 0;
        }
        if ( m_error == 0 )
        {
            m_cursor1 = m_cursor2 = 0;
        }
        m_title = "<error>";
        m_mainFunction.clear();
        return false;
    }
}


// Returns the title of the script.

const std::string& CScript::GetTitle()
{
    return m_title;
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
    if ( m_script.empty() )  return false;
    if ( m_mainFunction.empty() ) return false;

    if ( !m_botProg->Start(m_mainFunction.c_str()) )  return false;

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
                if ( !isValidRange(m_script, m_cursor1, m_cursor2) )
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
        if ( !isValidRange(m_script, m_cursor1, m_cursor2) )
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
        if ( !isValidRange(m_script, m_cursor1, m_cursor2) )
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
    std::string funcName;
    cursor1 = cursor2 = 0;

    if (m_botProg == nullptr)  return false;
    if ( !m_bRun )  return false;

    m_botProg->GetRunPos(funcName, cursor1, cursor2);

    if ( !isValidRange(m_script, cursor1, cursor2) )
    {
        cursor1 = 0;
        cursor2 = 0;
    }
    return true;
}


// Put of the variables in a list.

static void PutList(const std::string& baseName, bool bArray, CBot::CBotVar *var, Ui::CList *list, int &rankList, std::set<CBot::CBotVar*>& previous)
{
    if ( var == nullptr && !baseName.empty() )
    {
        list->SetItemName(rankList++, StrUtils::Format("%s = null;", baseName.c_str()).c_str());
        return;
    }

    int index = 0;
    while (var != nullptr)
    {
        var->Update(nullptr);
        CBot::CBotVar* pStatic = var->GetStaticVar();  // finds the static element

        std::string varName;
        if (baseName.empty())
        {
            varName = StrUtils::Format("%s", pStatic->GetName().c_str());
        }
        else
        {
            if (bArray)
            {
                varName = StrUtils::Format("%s[%d]", baseName.c_str(), index);
            }
            else
            {
                varName = StrUtils::Format("%s.%s", baseName.c_str(), pStatic->GetName().c_str());
            }
        }

        if (previous.find(pStatic) != previous.end())
        {
            list->SetItemName(rankList++, StrUtils::Format("%s = [circular reference]", varName.c_str()));
        }
        else
        {
            int type = pStatic->GetType();

            if (type <= CBot::CBotTypBoolean)
            {
                list->SetItemName(rankList++, StrUtils::Format("%s = %s;", varName.c_str(), pStatic->GetValString().c_str()));
            }
            else if (type == CBot::CBotTypString)
            {
                list->SetItemName(rankList++, StrUtils::Format("%s = \"%s\";", varName.c_str(), pStatic->GetValString().c_str()));
            }
            else if (type == CBot::CBotTypArrayPointer)
            {
                previous.insert(pStatic);
                PutList(varName, true, pStatic->GetItemList(), list, rankList, previous);
                previous.erase(pStatic);
            }
            else if (type == CBot::CBotTypClass ||
                     type == CBot::CBotTypPointer)
            {
                previous.insert(pStatic);
                PutList(varName, false, pStatic->GetItemList(), list, rankList, previous);
                previous.erase(pStatic);
            }
            else
            {
                //list->SetItemName(rankList++, StrUtils::Format("%s = ?;", varName.c_str()));
                assert(false);
            }
        }

        index ++;
        var = var->GetNext();
    }
}

// Fills a list with variables.

void CScript::UpdateList(Ui::CList* list)
{
    CBot::CBotVar     *var;
    std::string progName, funcName;
    int         total, select, level, cursor1, cursor2, rank;

    if (m_botProg == nullptr) return;

    total  = list->GetTotal();
    select = list->GetSelect();

    list->Flush();  // empty list
    m_botProg->GetRunPos(progName, cursor1, cursor2);
    if ( progName.empty() )  return;

    level = 0;
    rank  = 0;
    std::set<CBot::CBotVar*> previous;
    while ( true )
    {
        var = m_botProg->GetStackVars(funcName, level--);
        if ( funcName != progName )  break;

        PutList("", false, var, list, rank, previous);
    }
    assert(previous.empty());

    if ( total == list->GetTotal() )  // same total?
    {
        list->SetSelect(select);
    }

    list->SetTooltip("");
    list->SetState(Ui::STATE_ENABLE);
}

// Colorize a string or character literal with escape sequences also colored

static void HighlightString(Ui::CEdit* edit, const std::string& s, int start)
{
    edit->SetFormat(start, start + 1, Gfx::FONT_HIGHLIGHT_STRING);

    auto it = s.cbegin();
    char endQuote = *(it++);

    ++start;
    while (it != s.cend() && *it != endQuote)
    {
        if (*(it++) != '\\') // not escape sequence
        {
            edit->SetFormat(start, start + 1, Gfx::FONT_HIGHLIGHT_STRING);
            ++start;
            continue;
        }

        if (it == s.cend()) break;

        int end = start + 2;

        if (CBot::CharIsOctalNum(*it))                   // octal escape sequence
        {
            for (int i = 0; ++it != s.cend() && i < 2; i++, end++)
            {
                if (!CBot::CharIsOctalNum(*it)) break;
            }
        }
        else if (*it == 'x' || *it == 'u' || *it == 'U') // hex or unicode escape
        {
            bool isHexCode = (*it == 'x');
            int maxlen = (*it == 'u') ? 4 : 8;

            for (int i = 0; ++it != s.cend(); i++, end++)
            {
                if (!isHexCode && i >= maxlen) break;
                if (!CBot::CharIsHexNum(*it)) break;
            }
        }
        else      // n, r, t, etc.
            ++it;

        edit->SetFormat(start, end, Gfx::FONT_HIGHLIGHT_NONE);
        start = end;
    }

    if (it != s.cend())
        edit->SetFormat(start, start + 1, Gfx::FONT_HIGHLIGHT_STRING);
}

// Colorize the text according to syntax.

void CScript::ColorizeScript(Ui::CEdit* edit, int rangeStart, int rangeEnd)
{
    if (rangeEnd > edit->GetTextLength())
        rangeEnd = edit->GetTextLength();

    edit->SetFormat(rangeStart, rangeEnd, Gfx::FONT_HIGHLIGHT_COMMENT); // anything not processed is a comment

    // NOTE: Images are registered as index in some array, and that can be 0 which normally ends the string!
    std::string text = edit->GetText();
    text = text.substr(rangeStart, rangeEnd-rangeStart);

    auto tokens = CBot::CBotToken::CompileTokens(text.c_str());
    CBot::CBotToken* bt = tokens.get();
    while ( bt != nullptr )
    {
        std::string token = bt->GetString();
        int type = bt->GetType();

        int cursor1 = bt->GetStart();
        int cursor2 = bt->GetEnd();

        if (cursor1 < 0 || cursor2 < 0 || cursor1 == cursor2 || type == 0) { bt = bt->GetNext(); continue; } // seems to be a bug in CBot engine (how does it even still work? D:)

        cursor1 += rangeStart;
        cursor2 += rangeStart;

        Gfx::FontHighlight color = Gfx::FONT_HIGHLIGHT_NONE;
        if ((type == CBot::TokenTypVar || (type >= CBot::TokenKeyWord && type < CBot::TokenKeyWord+100)) && IsType(token.c_str())) // types (basic types are TokenKeyWord, classes are TokenTypVar)
        {
            color = Gfx::FONT_HIGHLIGHT_TYPE;
        }
        else if (type == CBot::TokenTypVar && IsFunction(token.c_str())) // functions
        {
            color = Gfx::FONT_HIGHLIGHT_TOKEN;
        }
        else if (type == CBot::TokenTypVar && (token == "this" || token == "super")) // this, super
        {
            color = Gfx::FONT_HIGHLIGHT_THIS;
        }
        else if (type >= CBot::TokenKeyWord && type < CBot::TokenKeyWord+100) // builtin keywords
        {
            color = Gfx::FONT_HIGHLIGHT_KEYWORD;
        }
        else if (type >= CBot::TokenKeyVal && type < CBot::TokenKeyVal+100) // true, false, null, nan
        {
            color = Gfx::FONT_HIGHLIGHT_CONST;
        }
        else if (type == CBot::TokenTypDef) // constants (object types etc.)
        {
            color = Gfx::FONT_HIGHLIGHT_CONST;
        }
        else if (type == CBot::TokenTypNum) // numbers
        {
            color = Gfx::FONT_HIGHLIGHT_STRING;
        }
        else if (type == CBot::TokenTypString || type == CBot::TokenTypChar) // string literals and character literals
        {
            HighlightString(edit, token, cursor1);
            bt = bt->GetNext();
            continue;
        }

        assert(cursor1 < cursor2);
        edit->SetFormat(cursor1, cursor2, color);

        bt = bt->GetNext();
    }
}


// Seeks a token at random in a script.
// Returns the index of the start of the token found, or -1.


static int SearchToken(std::string_view script, std::string_view token)
{
    std::array<int, 100> found;

    int count = 0;

    for (std::size_t offset = 0; offset < script.size();)
    {
        std::size_t index = script.find(token, offset);

        if (index == std::string::npos) break;

        found[count++] = index;

        offset = index + 1;
    }

    if ( count == 0 )  return -1;
    return found[rand() % count];
}

// Introduces a virus into a program.

bool CScript::IntroduceVirus()
{
    if (m_script.empty()) return false;

    using Pair = std::pair<std::string_view, std::string_view>;

    constexpr std::array<Pair, 11> names =
    {
        Pair { "==",         "!=" },
        Pair { "!=",         "==" },
        Pair { ">",          "<" },
        Pair { "<",          ">" },
        Pair { "true",       "false" },
        Pair { "false",      "true" },
        Pair { "grab",       "drop" },
        Pair { "drop",       "grab" },
        Pair { "InFront",    "Behind" },
        Pair { "Behind",     "EnergyCell" },
        Pair { "EnergyCell", "InFront" },
    };

    std::array<std::pair<int, int>, 11> found;
    int count = 0;

    for (size_t i = 0; i < names.size(); i++)
    {
        int start = SearchToken(m_script, names[i].first);

        if (start != -1)
        {
            found[count++] = std::make_pair(i, start);
        }
    }

    if (count == 0) return false;

    int i = rand() % count;
    int start = found[i].second;

    int j = found[i].first;

    m_script.replace(start, names[j].first.size(), names[j].second);

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
        if (m_error == static_cast<CBot::CBotError>(ERR_OBLIGATORYTOKEN))
        {
            error = StrUtils::Format(ngettext(
                "You have to use \"%1$s\" at least once in this exercise (used: %2$d)",
                "You have to use \"%1$s\" at least %3$d times in this exercise (used: %2$d)",
                m_tokenAllowed), m_token.c_str(), m_tokenUsed, m_tokenAllowed);
        }
        else if (m_error == static_cast<CBot::CBotError>(ERR_PROHIBITEDTOKEN))
        {
            if (m_tokenAllowed == 0)
            {
                error = StrUtils::Format(gettext("You cannot use \"%s\" in this exercise (used: %d)"), m_token.c_str(), m_tokenUsed);
            }
            else
            {
                error = StrUtils::Format(ngettext(
                    "You have to use \"%1$s\" at most once in this exercise (used: %2$d)",
                    "You have to use \"%1$s\" at most %3$d times in this exercise (used: %2$d)",
                    m_tokenAllowed), m_token.c_str(), m_tokenUsed, m_tokenAllowed);
            }
        }
        else if (m_error < 1000)
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

void CScript::New(Ui::CEdit* edit, const std::string& name)
{
    int     cursor1, cursor2, i;

    std::string resStr;
    GetResource(RES_TEXT, RT_SCRIPT_NEW, resStr);
    std::string res = resStr;

    std::string text;

    if ( name.empty() )  text = res;
    else                 text = name;

    std::string script = StrUtils::Format("extern void object::%s()\n{\n\t\n\t\n\t\n}\n", text.c_str());
    edit->SetText(script, false);

    if ( text == res )
    {
        cursor1 = 20;
        cursor2 = 20 + text.size();  // update "New"
    }
    else
    {
        if ( edit->GetAutoIndent() )
        {
            cursor1 = 20 + text.size() + 6;
            cursor2 = cursor1;  // cursor in { }
        }
        else
        {
            cursor1 = 20 + text.size() + 8;
            cursor2 = cursor1;  // cursor in { }
        }
    }

    edit->SetCursor(cursor2, cursor1);
    edit->ShowSelect();
    m_interface->SetFocus(edit);

    std::filesystem::path sf = m_main->GetScriptFile();
    if ( !sf.empty() )  // Load an empty program specific?
    {
        CInputStream stream;
        stream.open(sf);

        if (stream.is_open())
        {
            std::size_t len = stream.size();

            std::vector<char> buffer(len + 1, '\0');

            stream.read(buffer.data(), len);
            buffer[len] = 0;
            stream.close();

            cursor1 = 0;
            i = 0;

            script.clear();

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
                    script += text;
                    i += 2;
                    continue;
                }

                if ( buffer[i] == '#' )
                {
                    cursor1 = script.size();
                    i ++;
                    continue;
                }

                script += buffer[i++];
            }
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

bool CScript::SendScript(const std::string& text)
{
    /*m_len = strlen(text);
    m_script = new char[m_len+1];
    strcpy(m_script, text);
    if ( !CheckToken() )  return false;
    if ( !Compile() )  return false;*/

    Ui::CEdit* edit = m_interface->CreateEdit({ 0.0f, 0.0f }, { 0.0f, 0.0f }, 0, EVENT_EDIT9);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->SetText(text, true);
    GetScript(edit);
    m_interface->DeleteControl(EVENT_EDIT9);

    return true;
}

// Reads a script as a text file.

bool CScript::ReadScript(const std::filesystem::path& filename)
{
    Ui::CEdit*  edit;

    if (!CResourceManager::Exists(filename))  return false;

    m_script.clear();

    edit = m_interface->CreateEdit({ 0.0f, 0.0f }, { 0.0f, 0.0f }, 0, EVENT_EDIT9);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->ReadText(filename);
    GetScript(edit);
    m_interface->DeleteControl(EVENT_EDIT9);
    return true;
}

// Writes a script as a text file.

bool CScript::WriteScript(const std::filesystem::path& filename)
{
    if ( m_script.empty() )
    {
        CResourceManager::Remove(filename);
        return false;
    }

    Ui::CEdit* edit = m_interface->CreateEdit({ 0.0f, 0.0f }, { 0.0f, 0.0f }, 0, EVENT_EDIT9);
    edit->SetAutoIndent(m_engine->GetEditIndentMode());
    edit->SetText(m_script);
    edit->WriteText(filename);
    m_interface->DeleteControl(EVENT_EDIT9);
    return true;
}


// Reads a stack of script by execution as a file.

bool CScript::ReadStack(std::istream &istr)
{
    int     nb;

    if (!CBot::ReadInt(istr, nb)) return false;
    if (!CBot::ReadInt(istr, m_ipf)) return false;
    if (!CBot::ReadInt(istr, m_errMode)) return false;

    if (m_botProg == nullptr) return false;
    if (!m_botProg->RestoreState(istr)) return false;

    m_bRun = true;
    m_bContinue = false;
    return true;
}

// Writes a stack of script by execution as a file.

bool CScript::WriteStack(std::ostream &ostr)
{
    int     nb;

    nb = 2;
    if (!CBot::WriteInt(ostr, nb)) return false;
    if (!CBot::WriteInt(ostr, m_ipf)) return false;
    if (!CBot::WriteInt(ostr, m_errMode)) return false;

    return m_botProg->SaveState(ostr);
}


// Compares two scripts.

bool CScript::Compare(CScript* other)
{
    return m_script == other->m_script;
}


// Management of the file name when the script is saved.

void CScript::SetFilename(const std::filesystem::path& filename)
{
    m_filename = filename;
}

const std::filesystem::path& CScript::GetFilename() const
{
    return m_filename;
}
