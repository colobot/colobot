/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include <memory>

/**
 * A template function to make std::unique_ptr without naked new
 * It can be replaced with std::make_unique once we use C++14
 */
template<typename T, typename... Args>
inline std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * A template function to make std::unique_ptr of array type without naked new
 * It can be replaced with std::make_unique once we use C++14
 */
template<typename T>
inline std::unique_ptr<T[]> MakeUniqueArray(size_t size)
{
    return std::unique_ptr<T[]>(new T[size]);
}
