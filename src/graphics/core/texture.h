/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/core/texture.h
 * \brief Texture struct and related enums
 */

#pragma once

#include <glm/glm.hpp>


// Graphics module namespace
namespace Gfx
{


/**
 * \enum TextureFormat
 * \brief Format of image data
 */
enum class TextureFormat : unsigned char
{
    //! Try to determine automatically (may not work)
    AUTO,
    //! RGB triplet, 3 bytes
    RGB,
    //! BGR triplet, 3 bytes
    BGR,
    //! RGBA triplet, 4 bytes
    RGBA,
    //! BGRA triplet, 4 bytes
    BGRA,
};

/**
 * \enum TextureFilter
 * \brief General texture filtering mode
 *
 * Corresponds to typical options in game graphics settings.
 */
enum class TextureFilter : unsigned char
{
    //! Nearest-neighbor filtering
    NEAREST,
    //! Linear filtering
    BILINEAR,
    //! Linear filtering with mipmapping
    TRILINEAR,
};

/**
 * \enum TexWrapMode
 * \brief Wrapping mode for texture coords
 */
enum class TextureWrapMode : unsigned char
{
    //! UVs are clamped to edges
    CLAMP,
    //! UVs are repeated
    REPEAT,
};

/**
 * \struct TextureCreateParams
 * \brief Parameters for texture creation
 *
 * These params define how particular texture is created and later displayed.
 * They must be specified at texture creation time and cannot be changed later.
 */
struct TextureCreateParams
{
    //! Whether to generate mipmaps
    bool mipmap = false;
    //! Format of source image data
    TextureFormat format = TextureFormat::RGB;
    //! General texture filtering mode
    TextureFilter filter = TextureFilter::NEAREST;
    //! Wrap mode for texture coordinates
    TextureWrapMode wrap = TextureWrapMode::REPEAT;
    //! Pad the image to nearest power of 2 dimensions
    bool padToNearestPowerOfTwo = false;
};

/**
 * \struct Texture
 * \brief Info about a texture
 *
 * Identifies (through id) a texture created in graphics engine.
 * Also contains some additional data.
 */
struct Texture
{
    //! ID of the texture in graphics engine; 0 = invalid texture
    unsigned int id = 0;
    //! Size of texture
    glm::ivec2 size = { 0, 0 };
    //! Original size of texture (as loaded from image)
    glm::ivec2 originalSize = { 0, 0 };
    //! Whether the texture has alpha channel
    bool alpha = false;

    //! Returns whether the texture is valid (ID != 0)
    bool Valid() const
    {
        return id != 0;
    }

    //! Sets the ID to invalid value (0)
    void SetInvalid()
    {
        id = 0;
    }

    //! Comparator for use in texture maps and sets
    bool operator<(const Texture &other) const
    {
        // Invalid textures are always "less than" every other texture

        if ( (! Valid()) && (! other.Valid()) )
            return false;

        if (! Valid())
            return true;

        if (! other.Valid())
            return false;

        return id < other.id;
    }

    //! Comparator
    bool operator==(const Texture &other) const
    {
        if (Valid() != other.Valid())
            return false;
        if ( (! Valid()) && (! other.Valid()) )
            return true;

        return id == other.id;
    }
};


} // namespace Gfx

