// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file graphics/core/texture.h
 * \brief Texture struct and related enums
 */

#pragma once


#include "graphics/core/color.h"

#include "math/intpoint.h"


// Graphics module namespace
namespace Gfx {


/**
 * \enum TexImgFormat
 * \brief Format of image data
 */
enum TexImgFormat
{
    //! Try to determine automatically (may not work)
    TEX_IMG_AUTO,
    //! RGB triplet, 3 bytes
    TEX_IMG_RGB,
    //! BGR triplet, 3 bytes
    TEX_IMG_BGR,
    //! RGBA triplet, 4 bytes
    TEX_IMG_RGBA,
    //! BGRA triplet, 4 bytes
    TEX_IMG_BGRA
};

/**
 * \enum TexMinFilter
 * \brief Texture minification filter
 *
 * Corresponds to OpenGL modes but should translate to DirectX too.
 */
enum TexMinFilter
{
    TEX_MIN_FILTER_NEAREST,
    TEX_MIN_FILTER_LINEAR,
    TEX_MIN_FILTER_NEAREST_MIPMAP_NEAREST,
    TEX_MIN_FILTER_LINEAR_MIPMAP_NEAREST,
    TEX_MIN_FILTER_NEAREST_MIPMAP_LINEAR,
    TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR
};

/**
 * \enum TexMagFilter
 * \brief Texture magnification filter
 */
enum TexMagFilter
{
    TEX_MAG_FILTER_NEAREST,
    TEX_MAG_FILTER_LINEAR
};

/**
 * \enum TexWrapMode
 * \brief Wrapping mode for texture coords
 */
enum TexWrapMode
{
    TEX_WRAP_CLAMP,
    TEX_WRAP_REPEAT
};

/**
 * \enum TexMixOperation
 * \brief Multitexture mixing operation
 */
enum TexMixOperation
{
    //! Default operation on default params (modulate on computed & texture)
    TEX_MIX_OPER_DEFAULT,
    //! = Arg1
    TEX_MIX_OPER_REPLACE,
    //! = Arg1 * Arg2
    TEX_MIX_OPER_MODULATE,
    //! = Arg1 + Arg2
    TEX_MIX_OPER_ADD,
    //! = Arg1 - Arg2
    TEX_MIX_OPER_SUBTRACT
};

/**
 * \enum TexMixArgument
 * \brief Multitexture mixing argument
 */
enum TexMixArgument
{
    //! Color from current texture
    TEX_MIX_ARG_TEXTURE,
    //! Color computed by previous texture unit (current in DirectX; previous in OpenGL)
    TEX_MIX_ARG_COMPUTED_COLOR,
    //! (Source) color of textured fragment (diffuse in DirectX; primary color in OpenGL)
    TEX_MIX_ARG_SRC_COLOR,
    //! Constant color (texture factor in DirectX; texture env color in OpenGL)
    TEX_MIX_ARG_FACTOR
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
    bool mipmap;
    //! Format of source image data
    TexImgFormat format;
    //! Minification filter
    TexMinFilter minFilter;
    //! Magnification filter
    TexMagFilter magFilter;
    //! Pad the image to nearest power of 2 dimensions
    bool padToNearestPowerOfTwo;

    //! Constructor; calls LoadDefault()
    TextureCreateParams()
        { LoadDefault(); }

    //! Loads the default values
    inline void LoadDefault()
    {
        format = TEX_IMG_RGB;
        mipmap = false;
        padToNearestPowerOfTwo = false;

        minFilter = TEX_MIN_FILTER_NEAREST;
        magFilter = TEX_MAG_FILTER_NEAREST;
    }
};

/**
 * \struct TextureStageParams
 * \brief Parameters for a texture unit
 *
 * These params define the behavior of texturing units (stages).
 * They can be changed freely and are features of graphics engine, not any particular texture.
 */
struct TextureStageParams
{
    //! Mixing operation done on color values
    TexMixOperation colorOperation;
    //! 1st argument of color operations
    TexMixArgument colorArg1;
    //! 2nd argument of color operations
    TexMixArgument colorArg2;
    //! Mixing operation done on alpha values
    TexMixOperation alphaOperation;
    //! 1st argument of alpha operations
    TexMixArgument alphaArg1;
    //! 2nd argument of alpha operations
    TexMixArgument alphaArg2;
    //! Wrap mode for 1st tex coord
    TexWrapMode    wrapS;
    //! Wrap mode for 2nd tex coord
    TexWrapMode    wrapT;
    //! Constant color factor (for TEX_MIX_ARG_FACTOR)
    Color          factor;

    //! Constructor; calls LoadDefault()
    TextureStageParams()
        { LoadDefault(); }

    //! Loads the default values
    inline void LoadDefault()
    {
        colorOperation = TEX_MIX_OPER_DEFAULT;
        colorArg1 = TEX_MIX_ARG_COMPUTED_COLOR;
        colorArg2 = TEX_MIX_ARG_TEXTURE;

        alphaOperation = TEX_MIX_OPER_DEFAULT;
        alphaArg1 = TEX_MIX_ARG_COMPUTED_COLOR;
        alphaArg2 = TEX_MIX_ARG_TEXTURE;

        wrapS = wrapT = TEX_WRAP_REPEAT;
    }
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
    unsigned int id;
    //! Size of texture
    Math::IntPoint size;
    //! Original size of texture (as loaded from image)
    Math::IntPoint originalSize;
    //! Whether the texture has alpha channel
    bool alpha;

    Texture()
    {
        id = 0;
        alpha = false;
    }

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
    inline bool operator<(const Texture &other) const
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
    inline bool operator==(const Texture &other) const
    {
        if (Valid() != other.Valid())
            return false;
        if ( (! Valid()) && (! other.Valid()) )
            return true;

        return id == other.id;
    }
};


} // namespace Gfx

