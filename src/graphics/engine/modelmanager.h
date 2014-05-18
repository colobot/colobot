// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012-2014, Polish Portal of Colobot (PPC)
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

#pragma once

#include "common/singleton.h"

#include "graphics/engine/modelfile.h"

#include <string>
#include <vector>
#include <map>

namespace Gfx {

class CEngine;
class CModelFile;

/**
 * \class CModelManager
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
class CModelManager : public CSingleton<CModelManager>
{
public:
    CModelManager(CEngine* engine);
    ~CModelManager();

    //! Loads a model from given file
    bool LoadModel(const std::string& fileName, bool mirrored);

    //! Adds an instance of model to the given object rank as a reference to base object
    bool AddModelReference(const std::string& fileName, bool mirrored, int objRank);

    //! Adds an instance of model to the given object rank as a copy (copied base object)
    bool AddModelCopy(const std::string& fileName, bool mirrored, int objRank);

    //! Returns true if given model is loaded
    bool IsModelLoaded(const std::string& fileName, bool mirrored);

    //! Returns the rank of base engine object of given loaded model
    int GetModelBaseObjRank(const std::string& fileName, bool mirrored);

    //! Deletes all copied objects
    void DeleteAllModelCopies();

    //! Unloads the given model
    void UnloadModel(const std::string& fileName, bool mirrored);
    //! Unloads all models
    void UnloadAllModels();

protected:
    //! Returns the height of model -- closest point to X and Z coords of \a pos
    float GetHeight(std::vector<ModelTriangle>& triangles, Math::Vector pos);

    //! Mirrors the model along the Z axis
    void Mirror(std::vector<ModelTriangle>& triangles);

private:
    struct ModelInfo
    {
        std::vector<ModelTriangle> triangles;
        int baseObjRank;
    };
    struct FileInfo
    {
        std::string fileName;
        bool mirrored;

        inline FileInfo(const std::string& _fileName, bool _mirrored)
         : fileName(_fileName)
         , mirrored(_mirrored)
        {}

        inline bool operator<(const FileInfo& other) const
        {
            int compare = fileName.compare(other.fileName);
            if (compare < 0)
                return true;
            if (compare > 0)
                return false;

            return !mirrored && mirrored != other.mirrored;
        }
    };
    std::map<FileInfo, ModelInfo> m_models;
    std::vector<int> m_copiesBaseRanks;
    CEngine* m_engine;
};

} // namespace Gfx

