#pragma once

#include "graphics/model/model.h"

#include <string>
#include <unordered_map>

namespace Gfx {

/**
 * \class CModelManager
 * \brief Manager for models read from model files
 */
class CModelManager
{
public:
    //! Returns a model read from \a fileName
    /** @throws CModelIOException on read error */
    CModel& GetModel(const std::string& fileName);

    //! Clears cached models
    void ClearCache();

private:
    std::unordered_map<std::string, CModel> m_models;
};

} // namespace Gfx
