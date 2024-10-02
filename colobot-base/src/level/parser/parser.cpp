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

#include "level/parser/parser.h"

#include "app/app.h"

#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "level/robotmain.h"

#include "level/parser/parserexceptions.h"

#include <string>
#include <exception>
#include <sstream>
#include <iomanip>
#include <set>

CLevelParser::CLevelParser()
{
    m_filename = "";

    m_pathCat  = "";
    m_pathChap = "";
    m_pathLvl  = "";
}

CLevelParser::CLevelParser(const std::filesystem::path& filename)
: CLevelParser()
{
    m_filename = filename;
}

CLevelParser::CLevelParser(std::string_view category, int chapter, int rank)
: CLevelParser(BuildScenePath(category, chapter, rank))
{}

CLevelParser::CLevelParser(LevelCategory category, int chapter, int rank)
: CLevelParser(GetLevelCategoryDir(category), chapter, rank)
{
    SetLevelPaths(category, chapter, rank);
}

std::filesystem::path CLevelParser::BuildCategoryPath(std::string_view category)
{
    std::filesystem::path path = "levels";

    if (category == "perso" || category == "win" || category == "lost")
    {
        path /= "other";
    }
    else
    {
        path /= StrUtils::ToPath(category);
    }

    return path;
}

std::filesystem::path CLevelParser::BuildCategoryPath(LevelCategory category)
{
    return BuildCategoryPath(GetLevelCategoryDir(category));
}

std::filesystem::path CLevelParser::BuildScenePath(std::string_view category, int chapter, int rank, bool sceneFile)
{
    std::filesystem::path path = BuildCategoryPath(category);

    if (category == "custom")
    {
        path /= TempToPath(CRobotMain::GetInstancePointer()->GetCustomLevelName(chapter));

        if (rank == 0)
        {
            if (sceneFile)
            {
                path /= "chaptertitle.txt";
            }
        }
        else
        {
            path /= StrUtils::ToPath(StrUtils::Format("level%03d", rank));

            if (sceneFile)
            {
                path /= "scene.txt";
            }
        }
    }
    else if (category == "perso")
    {
        assert(chapter == 0);
        assert(rank == 0);
        path /= "perso.txt";
    }
    else if (category == "win" || category == "lost")
    {
        assert(chapter == 0);
        path /= StrUtils::ToPath(StrUtils::Format("%s%03d.txt", category, rank));
    }
    else
    {
        path /= StrUtils::ToPath(StrUtils::Format("chapter%03d", chapter));
        if (rank == 000)
        {
            if (sceneFile)
            {
                path /= "chaptertitle.txt";
            }
        }
        else
        {
            path /= StrUtils::ToPath(StrUtils::Format("level%03d", rank));
            if (sceneFile)
            {
                path /= "scene.txt";
            }
        }
    }
    return path;
}

std::filesystem::path CLevelParser::BuildScenePath(LevelCategory category, int chapter, int rank, bool sceneFile)
{
    return BuildScenePath(GetLevelCategoryDir(category), chapter, rank, sceneFile);
}

bool CLevelParser::Exists() const
{
    return CResourceManager::Exists(m_filename);
}

void CLevelParser::Load()
{
    CInputStream file;
    file.open(m_filename);
    if (!file.is_open())
        throw CLevelParserException("Failed to open file: " + StrUtils::ToString(m_filename));

    char lang = CApplication::GetInstancePointer()->GetLanguageChar();

    std::string line;
    int lineNumber = 0;
    std::set<std::string> translatableLines;
    while (getline(file, line))
    {
        lineNumber++;

        line = StrUtils::Replace(line, "\t", " "); // replace tab by space

        StrUtils::RemoveComments(line);

        StrUtils::Trim(line);

        size_t pos = line.find_first_of(" \t\n");
        std::string command = line.substr(0, pos);
        if (pos != std::string::npos)
        {
            line = line.substr(pos + 1);
            StrUtils::Trim(line);
        }
        else
        {
            line = "";
        }

        if (command.empty())
            continue;

        auto parserLine = std::make_unique<CLevelParserLine>(lineNumber, command);
        parserLine->SetLevel(this);

        if (command.length() > 2 && command[command.length() - 2] == '.')
        {
            std::string baseCommand = command.substr(0, command.length() - 2);
            parserLine->SetCommand(baseCommand);

            char languageChar = command[command.length() - 1];
            if (languageChar == 'E' && translatableLines.count(baseCommand) == 0)
            {
                translatableLines.insert(baseCommand);
            }
            else if (languageChar == lang)
            {
                if (translatableLines.count(baseCommand) > 0)
                {
                    auto it = std::remove_if(
                        m_lines.begin(),
                        m_lines.end(),
                        [&baseCommand](const CLevelParserLineUPtr& line)
                        {
                            return line->GetCommand() == baseCommand;
                        });
                    m_lines.erase(it, m_lines.end());
                }

                translatableLines.insert(baseCommand);
            }
            else
            {
                continue;
            }
        }

        while (!line.empty())
        {
            pos = line.find_first_of("=");
            std::string paramName = line.substr(0, pos);
            StrUtils::Trim(paramName);
            line = line.substr(pos + 1);
            StrUtils::Trim(line);

            if (line[0] == '\"')
            {
                pos = line.find_first_of("\"", 1);
                if (pos == std::string::npos)
                    throw CLevelParserException("Unclosed \" in " + StrUtils::ToString(m_filename) + ":" + StrUtils::ToString(lineNumber));
            }
            else if (line[0] == '\'')
            {
                pos = line.find_first_of("'", 1);
                if (pos == std::string::npos)
                    throw CLevelParserException("Unclosed ' in " + StrUtils::ToString(m_filename) + ":" + StrUtils::ToString(lineNumber));
            }
            else
            {
                pos = line.find_first_of("=");
                if (pos != std::string::npos)
                {
                    std::size_t pos2 = line.find_last_of(" \t\n", line.find_last_not_of(" \t\n", pos-1));
                    if (pos2 != std::string::npos)
                        pos = pos2;
                }
                else
                {
                    pos = line.length()-1;
                }
            }
            std::string paramValue = line.substr(0, pos + 1);
            StrUtils::Trim(paramValue);

            parserLine->AddParam(paramName, std::make_unique<CLevelParserParam>(paramName, paramValue));

            if (pos == std::string::npos)
                break;
            line = line.substr(pos + 1);
            StrUtils::Trim(line);
        }

        if (parserLine->GetCommand().length() > 1 && parserLine->GetCommand()[0] == '#')
        {
            std::string cmd = parserLine->GetCommand().substr(1, std::string::npos);
            if(cmd == "Include")
            {
                std::unique_ptr<CLevelParser> includeParser = std::make_unique<CLevelParser>(parserLine->GetParam("file")->AsPath(""));
                includeParser->Load();
                for(CLevelParserLineUPtr& line : includeParser->m_lines)
                {
                    AddLine(std::move(line));
                }
            }
            else
            {
                throw CLevelParserException("Unknown preprocessor command '#" + cmd + "' (in " + StrUtils::ToString(m_filename) + ":" + StrUtils::ToString<int>(lineNumber) + ")");
            }
        }
        else
        {
            AddLine(std::move(parserLine));
        }
    }

    file.close();
}

void CLevelParser::Save()
{
    COutputStream file;
    file.open(m_filename);
    if (!file.is_open())
        throw CLevelParserException("Failed to open file: " + StrUtils::ToString(m_filename));

    for (auto& line : m_lines)
    {
        file << *(line.get()) << "\n";
    }

    file.close();
}

void CLevelParser::SetLevelPaths(LevelCategory category, int chapter, int rank)
{
    m_pathCat  = BuildCategoryPath(category);
    m_pathChap = chapter != 0 ? BuildScenePath(category, chapter, 0, false) : "";
    m_pathLvl  = chapter != 0 && rank != 0 ? BuildScenePath(category, chapter, rank, false) : "";
}

std::filesystem::path CLevelParser::InjectLevelPaths(const std::filesystem::path& path, const std::filesystem::path& defaultDir)
{
    std::string newPath = StrUtils::ToString(path);
    if(!m_pathLvl.empty() ) newPath = StrUtils::Replace(newPath, "%lvl%",  StrUtils::ToString(m_pathLvl));
    if(!m_pathChap.empty()) newPath = StrUtils::Replace(newPath, "%chap%", StrUtils::ToString(m_pathChap));
    if(!m_pathCat.empty() ) newPath = StrUtils::Replace(newPath, "%cat%",  StrUtils::ToString(m_pathCat));
    if(newPath == StrUtils::ToString(path) && !path.empty())
    {
        newPath = StrUtils::ToString(defaultDir) + (!defaultDir.empty() ? "/" : "") + newPath;
    }

    std::string langPath = newPath;
    std::string langStr(1, CApplication::GetInstancePointer()->GetLanguageChar());
    langPath = StrUtils::Replace(langPath, "%lng%", langStr);
    if(CResourceManager::Exists(StrUtils::ToPath(langPath)))
        return StrUtils::ToPath(langPath);

    // Fallback to English if file doesn't exist
    newPath = StrUtils::Replace(newPath, "%lng%", "E");
    if(CResourceManager::Exists(StrUtils::ToPath(newPath)))
        return StrUtils::ToPath(newPath);

    return StrUtils::ToPath(langPath); // Return current language file if none of the files exist
}

const std::filesystem::path& CLevelParser::GetFilename() const
{
    return m_filename;
}

void CLevelParser::AddLine(CLevelParserLineUPtr line)
{
    line->SetLevel(this);
    m_lines.push_back(std::move(line));
}

CLevelParserLine* CLevelParser::Get(const std::string& command)
{
    CLevelParserLine* line = GetIfDefined(command);
    if (line == nullptr)
        throw CLevelParserException("Command not found: " + command);
    return line;
}

CLevelParserLine* CLevelParser::GetIfDefined(const std::string& command)
{
    for (auto& line : m_lines)
    {
        if (line->GetCommand() == command)
            return line.get();
    }
    return nullptr;
}

int CLevelParser::CountLines(const std::string& command)
{
    int count = 0;
    for (auto& line : m_lines)
    {
        if (line->GetCommand() == command)
            count++;
    }
    return count;
}
