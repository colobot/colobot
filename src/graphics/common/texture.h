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

// texture.h

#pragma once

namespace Gfx {

/**
  \enum TexImgFormat
  \brief Format of image data */
enum TexImgFormat
{
    TEX_IMG_RGB,
    TEX_IMG_BGR,
    TEX_IMG_RGBA,
    TEX_IMG_BGRA
};

/**
  \enum TexMinFilter
  \brief Minification texture filter

  Corresponds to OpenGL modes but should translate to DirectX too. */
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
  \enum TexMagFilter
  \brief Magnification texture filter */
enum TexMagFilter
{
    TEX_MAG_FILTER_NEAREST,
    TEX_MAG_FILTER_LINEAR
};

/**
  \enum TexWrapMode
  \brief Wrapping mode for texture coords */
enum TexWrapMode
{
    TEX_WRAP_CLAMP,
    TEX_WRAP_REPEAT
};

/**
  \enum TexMixOperation
  \brief Multitexture mixing operation
 */
enum TexMixOperation
{
    TEX_MIX_OPER_MODULATE,
    TEX_MIX_OPER_ADD
};

/**
  \enum TexMixArgument
  \brief Multitexture mixing argument
  */
enum TexMixArgument
{
    TEX_MIX_ARG_CURRENT,
    TEX_MIX_ARG_TEXTURE,
    TEX_MIX_ARG_DIFFUSE,
    TEX_MIX_ARG_FACTOR
};

/**
  \struct TextureCreateParams
  \brief Parameters for texture creation
  */
struct TextureCreateParams
{
    //! Whether to generate mipmaps
    bool mipmap;
    //! Format of source image data
    Gfx::TexImgFormat format;
    //! Minification filter
    Gfx::TexMinFilter minFilter;
    //! Magnification filter
    Gfx::TexMagFilter magFilter;
    //! Wrap S coord mode
    Gfx::TexWrapMode wrapS;
    //! Wrap T coord mode
    Gfx::TexWrapMode wrapT;

    //! Constructor; calls LoadDefault()
    TextureCreateParams()
        { LoadDefault(); }

    //! Loads the default values
    inline void LoadDefault()
    {
        format = Gfx::TEX_IMG_RGB;
        mipmap = false;

        minFilter = Gfx::TEX_MIN_FILTER_NEAREST;
        magFilter = Gfx::TEX_MAG_FILTER_NEAREST;

        wrapS = Gfx::TEX_WRAP_REPEAT;
        wrapT = Gfx::TEX_WRAP_REPEAT;
    }
};

/**
  \struct TextureParams
  \brief Parameters for texture creation
 */
struct TextureParams
{
    //! Mixing operation done on color values
    Gfx::TexMixOperation colorOperation;
    //! 1st argument of color operations
    Gfx::TexMixArgument colorArg1;
    //! 2nd argument of color operations
    Gfx::TexMixArgument colorArg2;
    //! Mixing operation done on alpha values
    Gfx::TexMixOperation alphaOperation;
    //! 1st argument of alpha operations
    Gfx::TexMixArgument alphaArg1;
    //! 2nd argument of alpha operations
    Gfx::TexMixArgument alphaArg2;

    //! Constructor; calls LoadDefault()
    TextureParams()
        { LoadDefault(); }

    //! Loads the default values
    inline void LoadDefault()
    {
        colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
        colorArg1 = Gfx::TEX_MIX_ARG_CURRENT;
        colorArg2 = Gfx::TEX_MIX_ARG_TEXTURE;

        alphaOperation = Gfx::TEX_MIX_OPER_MODULATE;
        alphaArg1 = Gfx::TEX_MIX_ARG_CURRENT;
        alphaArg2 = Gfx::TEX_MIX_ARG_TEXTURE;
    }
};

/** \struct Texture*/
struct Texture
{
    //! Whether the texture was loaded
    bool valid;
    //! Id of the texture in graphics engine
    unsigned int id;

    Texture()
        { valid = false; id = 0; }
};

}; // namespace Gfx
