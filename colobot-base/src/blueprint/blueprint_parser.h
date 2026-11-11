/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "blueprint/blueprint_parser_exceptions.h"
#include "blueprint/blueprint_parser_utils.h"

#include "level/parser/parser.h"

#include "object/object_type.h"

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define UPDATE_COMMAND_PREFIX "Set"

template<typename T>
class BlueprintParser
{
private:

    struct Param
    {
        std::function<void(CLevelParserLine*, void*)> read;
        std::function<bool(CLevelParserLine*, const void*)> write;
    };

    typedef std::map<std::string, Param> ParamMap;

    struct Command
    {
        std::string updCmd;
        ParamMap params;
        std::function<void*(T*)> getTarget;
    };

    std::vector<Command> commands;

public:
    void line(const std::string& type)
    {
        Command command;
        command.updCmd = command_name(UPDATE_COMMAND_PREFIX, type);

        command.getTarget = [](T* blueprint) -> void* { return blueprint; };

        commands.push_back(std::move(command));
    }

    template<typename Struct, typename Value, typename Converter>
    void param(const std::string& key, Converter converter, Value Struct::* member)
    {
        Param param;

        param.read = [key, converter, member](CLevelParserLine* line, void* t)
        {
            CLevelParserParam* value = line->GetParam(key);

            if (!value->IsDefined())
                return;

            Struct* target = static_cast<Struct*>(t);
            Struct defaults;
            target->*member = converter(value, defaults.*member);
        };

        param.write = [key, member](CLevelParserLine* line, const void* t)
        {
            const Struct* target = static_cast<const Struct*>(t);

            Struct defaults;
            if (target->*member == defaults.*member)
                return false;

            line->AddParam(key, std::unique_ptr<CLevelParserParam>(new CLevelParserParam(target->*member)));

            return true;
        };

        assert(!commands.empty());
        assert(commands.back().params.count(key) == 0);
        commands.back().params[key] = std::move(param);
    }

    bool read(CLevelParserLine* line, T* blueprint)
    {
        const std::string name = line->GetCommand();
        for (const Command& command : commands)
        {
            if ( name == command.updCmd )
            {
                void* target = command.getTarget(blueprint);
                read_inner(line, command, target);
                return true;
            }
        }
        return false;
    }

    void write(CLevelParser* parser, ObjectType type, T* blueprint)
    {
        for (const Command& command : commands)
        {
            write_command(parser, type, command.updCmd, command.params, command.getTarget(blueprint));
        }
    }

private:
    void check_for_unused_keys(CLevelParserLine* line, const Command& command)
    {
        for (const auto& key : line->GetKeys())
        {
            if ( key == "type" )
                continue;
            if ( command.params.find(key) != command.params.end() )
                continue;
            throw UnusedParamException(line, key);
        }
    }

    void read_inner(CLevelParserLine* line, const Command& command, void* target)
    {
        assert(target);
        check_for_unused_keys(line, command);

        for (const auto& item : command.params)
        {
            item.second.read(line, target);
        }
    }

    void write_command(CLevelParser* parser, ObjectType type, const std::string& cmd, const ParamMap& params, const void* target)
    {
        std::unique_ptr<CLevelParserLine> line(new CLevelParserLine(cmd));
        line->AddParam("type", std::unique_ptr<CLevelParserParam>(new CLevelParserParam(type)));

        bool changed = false;
        for (const auto& item : params)
        {
            changed = item.second.write(line.get(), target) || changed;
        }

        if (changed)
        {
            parser->AddLine(std::move(line));
        }
    }

    std::string command_name(const std::string& verb, const std::string& type)
    {
        return verb + "<" + type + ">";
    }
};

template<typename T>
void ConfigBlueprintParser(BlueprintParser<T>& p);

template<typename T>
bool ParseBlueprintFromLine(CLevelParserLine* line, T* blueprint)
{
    BlueprintParser<T> p;
    ConfigBlueprintParser<T>(p);
    return p.read(line, blueprint);
}

template<typename T>
void WriteBlueprintToLines(CLevelParser* parser, ObjectType type, T* blueprint)
{
    BlueprintParser<T> p;
    ConfigBlueprintParser<T>(p);
    p.write(parser, type, blueprint);
}
