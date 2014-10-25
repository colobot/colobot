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

#include "object/level/parser.h"


#include "app/app.h"

#include "common/resources/resourcemanager.h"
#include "common/resources/inputstream.h"

#include "object/level/parserexceptions.h"

#include "object/robotmain.h"

#include <string>
#include <exception>
#include <sstream>
#include <iomanip>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

CLevelParser::CLevelParser()
{
    m_filename = "";
}

CLevelParser::CLevelParser(std::string filename)
{
    m_filename = filename;
}

CLevelParser::CLevelParser(std::string category, int chapter, int rank)
{
    m_filename = BuildSceneName(category, chapter, rank);
}

CLevelParser::~CLevelParser()
{
    for(auto line : m_lines)
    {
        delete line;
    }
}

std::string CLevelParser::BuildSceneName(std::string category, int chapter, int rank, bool sceneFile)
{
    std::ostringstream outstream;
    if(category == "custom")
    {
        outstream << "levels/custom/";
        outstream << CRobotMain::GetInstancePointer()->GetUserLevelName(chapter);
        if(rank == 000)
        {
            if(sceneFile)
            {
                outstream << "/chaptertitle.txt";
            }
        }
        else
        {
            outstream << "/level" << std::setfill('0') << std::setw(3) << rank;
            if(sceneFile)
            {
                outstream << "/scene.txt";
            }
        }
    }
    else if(category == "perso")
    {
        outstream << "levels/other/perso.txt";
    }
    else if(category == "win" || category == "lost")
    {
        outstream << "levels/other/";
        outstream << category << std::setfill('0') << std::setw(3) << chapter*100+rank << ".txt";
    }
    else
    {
        outstream << "levels/" << category << "/";
        outstream << "chapter" << std::setfill('0') << std::setw(3) << chapter;
        if(rank == 000)
        {
            if(sceneFile)
            {
                outstream << "/chaptertitle.txt";
            }
        }
        else
        {
            outstream << "/level" << std::setfill('0') << std::setw(3) << rank;
            if(sceneFile)
            {
                outstream << "/scene.txt";
            }
        }
    }
    return outstream.str();
}

bool CLevelParser::Exists()
{
    return CResourceManager::Exists(m_filename);
}

void CLevelParser::Load()
{
    CInputStream file;
    file.open(m_filename);
    if(!file.is_open())
        throw CLevelParserException("Failed to open file: "+m_filename);
    
    char lang = CApplication::GetInstancePointer()->GetLanguageChar();
    
    std::string line;
    int lineNumber = 0;
    std::map<std::string, CLevelParserLine*> translatableLines;
    while(getline(file,line))
    {
        lineNumber++;
        
        boost::replace_all(line, "\t", " "); // replace tab by space
        
        // ignore comments
        std::size_t comment = line.find("//");
        if(comment != std::string::npos)
            line = line.substr(0, comment);
        
        boost::algorithm::trim(line);
        
        std::size_t pos = line.find_first_of(" \t\n");
        std::string command = line.substr(0, pos);
        if(pos != std::string::npos) {
            line = line.substr(pos+1);
            boost::algorithm::trim(line);
        } else {
            line = "";
        }
        if(command.empty()) continue;
        
        CLevelParserLine* parserLine = new CLevelParserLine(lineNumber, command);
        
        std::string baseCommand = command;
        if(command[command.length()-2] == '.') {
            baseCommand = command.substr(0, command.length()-2);
            if(command[command.length()-1] == 'E' && translatableLines[baseCommand] == nullptr) {
                parserLine->SetCommand(baseCommand);
                translatableLines[baseCommand] = parserLine;
            } else if(command[command.length()-1] == lang) {
                if(translatableLines[baseCommand] != nullptr) {
                    m_lines.erase(std::remove(m_lines.begin(), m_lines.end(), translatableLines[baseCommand]), m_lines.end());
                    delete translatableLines[baseCommand];
                }
                parserLine->SetCommand(baseCommand);
                translatableLines[baseCommand] = parserLine;
            } else {
                delete parserLine;
                continue;
            }
        }
        
        while(!line.empty()) {
            pos = line.find_first_of("=");
            std::string paramName = line.substr(0, pos);
            boost::algorithm::trim(paramName);
            line = line.substr(pos+1);
            boost::algorithm::trim(line);
            
            if(line[0] == '\"') {
                pos = line.find_first_of("\"", 1);
                if(pos == std::string::npos)
                    throw CLevelParserException("Unclosed \" in "+m_filename+":"+boost::lexical_cast<std::string>(lineNumber));
            } else if(line[0] == '\'') {
                pos = line.find_first_of("'", 1);
                if(pos == std::string::npos)
                    throw CLevelParserException("Unclosed ' in "+m_filename+":"+boost::lexical_cast<std::string>(lineNumber));
            } else {
                pos = line.find_first_of("=");
                if(pos != std::string::npos) {
                    std::size_t pos2 = line.find_last_of(" \t\n", line.find_last_not_of(" \t\n", pos-1));
                    if(pos2 != std::string::npos)
                        pos = pos2;
                } else {
                    pos = line.length()-1;
                }
            }
            std::string paramValue = line.substr(0, pos+1);
            boost::algorithm::trim(paramValue);
            
            parserLine->AddParam(paramName, new CLevelParserParam(paramName, paramValue));
            
            if(pos == std::string::npos)
                break;
            line = line.substr(pos+1);
            boost::algorithm::trim(line);
        }
        
        AddLine(parserLine);
    }
    
    file.close();
}

void CLevelParser::Save(std::string filename)
{
    assert(false); //TODO
}

const std::string& CLevelParser::GetFilename()
{
    return m_filename;
}

std::vector<CLevelParserLine*> CLevelParser::GetLines()
{
    return m_lines;
}

void CLevelParser::AddLine(CLevelParserLine* line)
{
    line->SetLevel(this);
    m_lines.push_back(line);
}

CLevelParserLine* CLevelParser::Get(std::string command)
{
    for(auto& line : m_lines) {
        if(line->GetCommand() == command)
            return line;
    }
    throw CLevelParserException("Command not found: "+command);
}
