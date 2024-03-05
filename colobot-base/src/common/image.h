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
 * \file common/image.h
 * \brief Class for loading and saving images
 */

#pragma once

#include "graphics/core/color.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <string>


// Forward declaration without including headers to clutter the code
struct SDL_Surface;

//! Implementation-specific image data
/** Note that the struct has no destructor and the surface
    will not be freed at destruction. */
struct ImageData
{
    //! SDL surface with image data
    SDL_Surface* surface = nullptr;
};

/**
  \class CImage
  \brief Image loaded from file

  Wrapper around SDL_Image library to load images. Also contains
  function for saving images to PNG.
  */
class CImage
{
public:
    //! Constructs empty image (with nullptr data)
    CImage();
    //! Constructs a RGBA image of given size
    CImage(const glm::ivec2& size);
    //! Destroys image, calling Free()
    virtual ~CImage();

    CImage(const CImage &other) = delete;
    void operator=(const CImage &other) = delete;

    //! Frees the allocated image data
    void Free();

    //! Returns whether the image is empty (has null data)
    bool IsEmpty() const;

    //! Returns the image data; if empty - returns nullptr
    ImageData* GetData();

    //! Returns the image size
    glm::ivec2 GetSize() const;

    //! Fills the whole image with given color
    void Fill(Gfx::IntColor color);

    //! Sets the color at given pixel
    void SetPixel(const glm::ivec2& pixel, Gfx::Color color);

    //! Sets the precise color at given pixel
    void SetPixelInt(const glm::ivec2& pixel, Gfx::IntColor color);

    //! Returns the color at given pixel
    Gfx::Color GetPixel(const glm::ivec2& pixel);

    //! Returns the precise color at given pixel
    Gfx::IntColor GetPixelInt(const glm::ivec2& pixel);

    //! Pads the image to nearest power of 2 dimensions
    void PadToNearestPowerOfTwo();

    //! Convert the image to RGBA surface
    void ConvertToRGBA();

    //! Loads an image from the specified file
    bool Load(const std::filesystem::path& fileName);

    //! Saves the image to the specified file in PNG format
    bool SavePNG(const std::filesystem::path& fileName);

    //! Returns the last error
    std::string GetError();

    //! Flips the image vertically
    void FlipVertically();

    //! sets/replaces the pixels from the surface
    void SetDataPixels(void *pixels);

private:
    //! Blit to new RGBA surface with given size
    void BlitToNewRGBASurface(int width, int height);

    //! Last encountered error
    std::string m_error;
    //! Image data
    std::unique_ptr<ImageData> m_data;
};

