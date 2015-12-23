#pragma once

#include "CBotEnums.h"

#include <string>

/**
 * \brief LoadString Maps given ID to its string equivalent.
 * \param id            Provided identifier.
 * \return              String if found, else NullString.
 */
const std::string& LoadString(TokenId id);
