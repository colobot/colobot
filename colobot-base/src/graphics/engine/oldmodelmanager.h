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

#pragma once

#include "common/singleton.h"

#include "graphics/model/model_triangle.h"

#include <string>
#include <vector>
#include <map>

namespace Gfx
{

class CEngine;

/**
 * \class COldModelManager
 * \brief Manager for static models
 *
 * The manager allows for loading models as static objects and adding
 * new instances of models to the engine.
 *
 * The models are loaded from stanard application model directory and
 * they are identified by unique file names.
 *
 * The models are loaded by creating (if it doesn't exist yet)
 * a base engine object from the model geometry. This base object
 * is then shared among all instances of this model with the instances
 * being engine objects linked to the shared base object.
 *
 * There is also a possibility of creating a copy of model so it has
 * its own and unique base engine object. This is especially useful
 * for models where the geometry must be altered.
 */
class COldModelManager
{
public:
    COldModelManager(CEngine* engine);
    ~COldModelManager();

    //! Loads a model from given file
    bool LoadModel(const std::filesystem::path& fileName, bool mirrored, int team = 0);

    //! Adds an instance of model to the given object rank as a reference to base object
    bool AddModelReference(const std::string& fileName, bool mirrored, int objRank, int team = 0);

    //! Adds an instance of model to the given object rank as a copy (copied base object)
    bool AddModelCopy(const std::string& fileName, bool mirrored, int objRank, int team = 0);

    //! Returns true if given model is loaded
    bool IsModelLoaded(const std::string& fileName, bool mirrored, int team = 0);

    //! Returns the rank of base engine object of given loaded model
    int GetModelBaseObjRank(const std::string& fileName, bool mirrored, int team = 0);

    //! Deletes all copied objects
    void DeleteAllModelCopies();

    //! Unloads the given model
    void UnloadModel(const std::string& fileName, bool mirrored, int variant = 0);
    //! Unloads all models
    void UnloadAllModels();

protected:
    //! Mirrors the model along the Z axis
    void Mirror(std::vector<ModelTriangle>& triangles);

private:
    struct ModelInfo
    {
        std::vector<ModelTriangle> triangles;
        int baseObjRank = -1;
    };
    struct FileInfo
    {
        std::string fileName;
        bool mirrored;
        int variant;

        inline FileInfo(const std::string& _fileName, bool _mirrored, int _variant = 0)
         : fileName(_fileName)
         , mirrored(_mirrored)
         , variant(_variant)
        {}

        inline bool operator<(const FileInfo& other) const
        {
            int compare = fileName.compare(other.fileName);
            if (compare < 0)
                return true;
            if (compare > 0)
                return false;

            if (variant < other.variant)
                return true;
            if (variant > other.variant)
                return false;

            return !mirrored && mirrored != other.mirrored;
        }
    };
    std::map<FileInfo, ModelInfo> m_models;
    std::vector<int> m_copiesBaseRanks;
    CEngine* m_engine;
};

} // namespace Gfx
