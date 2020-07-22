/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "app/moddata.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "common/resources/resourcemanager.h"
#include "common/resources/inputstream.h"

#include <boost/optional.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <istream>

namespace pt = boost::property_tree;

boost::optional<pt::ptree> LoadManifest(const std::string& path);
std::string GetStringProperty(const pt::ptree& manifest, const std::string& key);
std::unordered_map<Language, std::string> GetLanguageStringProperty(const pt::ptree& manifest, const std::string& key);

ModData LoadModData(const std::string& path)
{
    ModData modData{};

    auto manifestOptional = LoadManifest(path);
    if (!manifestOptional)
    {
        return modData;
    }
    auto manifest = manifestOptional.get();

    modData.displayName = GetLanguageStringProperty(manifest, "DisplayName");
    modData.version = GetStringProperty(manifest, "Version");
    modData.author = GetStringProperty(manifest, "Author");
    modData.website = GetStringProperty(manifest, "Website");
    modData.summary = GetLanguageStringProperty(manifest, "Summary");

    return modData;
}

boost::optional<pt::ptree> LoadManifest(const std::string& path)
{
    try
    {
        auto inputStream = MakeUnique<CInputStream>("manifest.json");
        if (!inputStream->is_open())
        {
            GetLogger()->Error("Error on parsing the manifest file %s: failed to open file\n");
            return {};
        }

        pt::ptree manifest{};
        boost::property_tree::json_parser::read_json(*inputStream, manifest);

        return manifest;
    }
    catch (std::exception& e)
    {
        GetLogger()->Warn("Error on parsing the manifest file %s: %s\n", path.c_str(), e.what());
        return {};
    }
    return {};
}

std::string GetStringProperty(const pt::ptree& manifest, const std::string& key)
{
    auto prop = manifest.get_optional<std::string>(key);
    if (prop)
    {
        return prop.get();
    }
    return {};
}

std::unordered_map<Language, std::string> GetLanguageStringProperty(const pt::ptree& manifest, const std::string& key)
{
    std::unordered_map<Language, std::string> ret;
    auto prop = manifest.get_child_optional(key);
    if (prop)
    {
        for (const auto& child : prop.get())
        {
            Language language = LANGUAGE_ENGLISH;
            std::string strLanguage = child.first.data();
            if (!ParseLanguage(strLanguage, language))
            {
                GetLogger()->Warn("Error on parsing the manifest file: %s language %s is not a valid language\n", key.c_str(), strLanguage.c_str());
                continue;
            }
            else
            {
                ret.insert(std::make_pair(language, child.second.data()));
            }
        }
    }
    return ret;
}
