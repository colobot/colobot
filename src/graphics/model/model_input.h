#pragma once

#include "graphics/model/model.h"
#include "graphics/model/model_format.h"

#include <istream>

namespace Gfx {

/**
 * \namespace ModelInput
 * \brief Namespace with functions to read model files
 */
namespace ModelInput
{
    //! Reads model from \a stream in given \a format
    /**
     * @throws CModelIOException on read/write error
     */
    CModel Read(std::istream &stream, ModelFormat format);
}

} // namespace Gfx
