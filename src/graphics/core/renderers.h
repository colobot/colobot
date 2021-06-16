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
  * \file graphics/core/renderers.h
  * \brief Abstract classes representing renderers
  */

#pragma once

#include "graphics/core/vertex.h"

#include <glm/glm.hpp>

// Graphics module namespace
namespace Gfx
{

enum PrimitiveType;
struct Texture;

/**
 * \class CRenderer
 * \brief Common abstract interface for renderers
 */
class CRenderer
{
public:
    virtual ~CRenderer() { }

    //! Flush buffered content
    virtual void Flush() = 0;
};

/**
 * \class CRenderer
 * \brief Abstract interface for UI renderers
 */
class CUIRenderer : public CRenderer
{
public:
    virtual ~CUIRenderer() { }

    //! Sets ortographic projection with given parameters
    virtual void SetProjection(float left, float right, float bottom, float top) = 0;
    //! Sets texture, setting texture 0 means using white texture
    virtual void SetTexture(const Texture& texture) = 0;

    //! Draws primitive
    virtual void DrawPrimitive(PrimitiveType type, int count, const Vertex2D* vertices) = 0;
};

} // namespace Gfx
