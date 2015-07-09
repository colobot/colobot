#pragma once

#include "graphics/model/model_format.h"

#include <ostream>
#include <string>

namespace Gfx {

class CModel;

namespace ModelOutput
{
    //! Writes the given \a model to \a stream using \a format
    /**
     * @throws CModelIOException on read/write error
     */
    void Write(const CModel& model, std::ostream& stream, ModelFormat format);
}

} // namespace Gfx
