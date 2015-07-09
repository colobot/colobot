#pragma once

#include <stdexcept>

namespace Gfx {

class CModelIOException : public std::runtime_error
{
public:
    explicit CModelIOException(const std::string& error)
        : std::runtime_error(error)
    {}
};

} // namespace Gfx
