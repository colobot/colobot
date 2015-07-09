#include "graphics/model/model_manager.h"

#include "common/resources/inputstream.h"

#include "graphics/model/model_input.h"
#include "graphics/model/model_io_exception.h"

namespace Gfx {

CModel& CModelManager::GetModel(const std::string& fileName)
{
    auto it = m_models.find(fileName);
    if (it != m_models.end())
        return it->second;

    CInputStream stream;
    stream.open("models-new/" + fileName);
    if (!stream.is_open())
        throw CModelIOException(std::string("Could not open file '") + fileName + "'");

    CModel model = ModelInput::Read(stream, ModelFormat::Text);
    m_models[fileName] = model;

    return m_models[fileName];
}

void CModelManager::ClearCache()
{
    m_models.clear();
}

} // namespace Gfx
