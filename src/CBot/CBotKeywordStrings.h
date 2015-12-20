#pragma once

#include "CBotEnums.h"

/**
 * \brief LoadString Maps given ID to its string equivalent.
 * \param id            Provided identifier.
 * \return              String if found, else NullString.
 */
const char* LoadString(EID id);
