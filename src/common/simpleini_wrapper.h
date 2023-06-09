/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file common/simpleini_wrapper.h
 * \brief Class extending CSimpleIniA from simpleini library. Do not include in header files to speed up the compilation.
 */

#pragma once

#define SI_SUPPORT_IOSTREAMS
#define SI_NO_CONVERSION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <SimpleIni.h>
#pragma GCC diagnostic pop

namespace si {
    class SimpleIni : public CSimpleIniA {};
}
