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


#include "common/image.h"

#include "common/stringutils.h"

#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "math/func.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

#include <SDL.h>
#include <SDL_image.h>
#include <png.h>


/* <---------------------------------------------------------------> */

/* The following code is from savesurf program by Angelo "Encelo" Theodorou
   Source: http://encelo.netsons.org/old/sdl/
   The code was refactored and modified slightly to fit the needs.
   The copyright information below is kept unchanged. */


/* SaveSurf: an example on how to save a SDLSurface in PNG
   Copyright (C) 2006 Angelo "Encelo" Theodorou

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   NOTE:

   This program is part of "Mars, Land of No Mercy" SDL examples,
   you can find other examples on http://marsnomercy.org
*/

namespace
{
std::string PNG_ERROR = "";

void PNGUserError(png_structp ctx, png_const_charp str)
{
    PNG_ERROR = std::string(str);
}

int PNGColortypeFromSurface(SDL_Surface *surface)
{
    int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */

    if (surface->format->palette)
        colortype |= PNG_COLOR_MASK_PALETTE;
    else if (surface->format->Amask)
        colortype |= PNG_COLOR_MASK_ALPHA;

    return colortype;
}

bool PNGSaveSurface(const char *filename, SDL_Surface *surf)
{
    PNG_ERROR = "";

    /* Opening output file */
    COutputStream ostr(filename);
    if (!ostr.is_open())
    {
        PNG_ERROR = std::string("Could not open file '") + std::string(filename) + std::string("' for saving");
        return false;
    }

    /* Initializing png structures and callbacks */
    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, PNGUserError, nullptr);
    if (pngPtr == nullptr)
    {
        ostr.close();
        return false;
    }

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (infoPtr == nullptr)
    {
        png_destroy_write_struct(&pngPtr, static_cast<png_infopp>(nullptr));
        PNG_ERROR = "png_create_info_struct() error!";
        ostr.close();
        return false;
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        png_destroy_write_struct(&pngPtr, &infoPtr);
        ostr.close();
        return false;
    }

    png_set_write_fn(
        pngPtr, static_cast<std::ostream*>(&ostr),
        [](png_structp pngPtr, png_bytep data, png_size_t length)
        {
            auto* file = static_cast<std::ostream*>(png_get_io_ptr(pngPtr));
            file->write(reinterpret_cast<char*>(data), length);
        },
        [](png_structp pngPtr)
        {
            auto* file = static_cast<std::ostream*>(png_get_io_ptr(pngPtr));
            file->flush();
        }
    );

    int colortype = PNGColortypeFromSurface(surf);
    png_set_IHDR(pngPtr, infoPtr, surf->w, surf->h, 8, colortype, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* Writing the image */
    png_write_info(pngPtr, infoPtr);
    png_set_packing(pngPtr);

    auto rowPointers = std::make_unique<png_bytep[]>(surf->h);
    for (int i = 0; i < surf->h; i++)
        rowPointers[i] = static_cast<png_bytep>( static_cast<Uint8 *>(surf->pixels) ) + i*surf->pitch;
    png_write_image(pngPtr, rowPointers.get());
    png_write_end(pngPtr, infoPtr);

    png_destroy_write_struct(&pngPtr, &infoPtr);
    ostr.close();

    return true;
}

} // namespace

/* <---------------------------------------------------------------> */


CImage::CImage()
{
    m_data = nullptr;
}

CImage::CImage(const glm::ivec2& size)
{
    m_data = std::make_unique<ImageData>();
    m_data->surface = SDL_CreateRGBSurface(0, size.x, size.y, 32,
                                           0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
}

CImage::~CImage()
{
    Free();
}

bool CImage::IsEmpty() const
{
    return m_data == nullptr;
}

void CImage::Free()
{
    if (m_data != nullptr)
    {
        if (m_data->surface != nullptr)
        {
            SDL_FreeSurface(m_data->surface);
            m_data->surface = nullptr;
        }
        m_data.reset();
    }
}

ImageData* CImage::GetData()
{
    return m_data.get();
}

glm::ivec2 CImage::GetSize() const
{
    if (m_data == nullptr)
        return { 0, 0 };

    return { m_data->surface->w, m_data->surface->h };
}

/** Image must be valid. */
void CImage::Fill(Gfx::IntColor color)
{
    assert(m_data != nullptr);

    Uint32 c = SDL_MapRGBA(m_data->surface->format, color.r, color.g, color.b, color.a);
    SDL_FillRect(m_data->surface, nullptr, c);
}

/**
 * Image must be valid.
 *
 * The dimensions are increased to nearest even power of two values.
 * If image is already in power-of-two format, nothing is done.
 */
void CImage::PadToNearestPowerOfTwo()
{
    assert(m_data != nullptr);

    if (Math::IsPowerOfTwo(m_data->surface->w) && Math::IsPowerOfTwo(m_data->surface->h))
        return;

    int w = Math::NextPowerOfTwo(m_data->surface->w);
    int h = Math::NextPowerOfTwo(m_data->surface->h);

    BlitToNewRGBASurface(w, h);
}

void CImage::ConvertToRGBA()
{
    assert(m_data != nullptr);

    int w = m_data->surface->w;
    int h = m_data->surface->h;

    BlitToNewRGBASurface(w, h);
}

void CImage::BlitToNewRGBASurface(int width, int height)
{
    SDL_Surface* convertedSurface = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00,
                                                         0x000000FF, 0xFF000000);
    assert(convertedSurface != nullptr);
    SDL_BlitSurface(m_data->surface, nullptr, convertedSurface, nullptr);

    SDL_FreeSurface(m_data->surface);

    m_data->surface = convertedSurface;
}

/**
 * Image must be valid and pixel coords in valid range.
 *
 * \param pixel pixel coords (range x: 0..width-1 y: 0..height-1)
 * \returns color
 */
Gfx::IntColor CImage::GetPixelInt(const glm::ivec2& pixel)
{
    assert(m_data != nullptr);
    assert(pixel.x >= 0 && pixel.x < m_data->surface->w);
    assert(pixel.y >= 0 && pixel.y < m_data->surface->h);

    int bpp = m_data->surface->format->BytesPerPixel;
    int index = pixel.y * m_data->surface->pitch + pixel.x * bpp;
    Uint8* p = &static_cast<Uint8*>(m_data->surface->pixels)[index];

    Uint32 u = 0;
    switch (bpp)
    {
        case 1:
            u = *p;
            break;

        case 2:
            u = *reinterpret_cast<Uint16*>(p);
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                u = (p[0] << 16) | (p[1] << 8) | p[2];
            else
                u = p[0] | (p[1] << 8) | (p[2] << 16);
            break;

        case 4:
            u = *reinterpret_cast<Uint32*>(p);
            break;

        default:
            assert(false);
    }

    Uint8 r = 0, g = 0, b = 0, a = 0;
    SDL_GetRGBA(u, m_data->surface->format, &r, &g, &b, &a);

    return Gfx::IntColor(r, g, b, a);
}

/**
 * Image must be valid and pixel coords in valid range.
 *
 * \param pixel pixel coords (range x: 0..width-1 y: 0..height-1)
 * \returns color
 */
Gfx::Color CImage::GetPixel(const glm::ivec2& pixel)
{
    return Gfx::IntColorToColor(GetPixelInt(pixel));
}


/**
 * Image must be valid and pixel coords in valid range.
 *
 * \param pixel pixel coords (range x: 0..width-1 y: 0..height-1)
 * \param color color
 */
void CImage::SetPixelInt(const glm::ivec2& pixel, Gfx::IntColor color)
{
    assert(m_data != nullptr);
    assert(pixel.x >= 0 && pixel.x < m_data->surface->w);
    assert(pixel.y >= 0 && pixel.y < m_data->surface->h);

    int bpp = m_data->surface->format->BytesPerPixel;
    int index = pixel.y * m_data->surface->pitch + pixel.x * bpp;
    Uint8* p = &static_cast<Uint8*>(m_data->surface->pixels)[index];

    Uint32 u = SDL_MapRGBA(m_data->surface->format, color.r, color.g, color.b, color.a);

    switch (bpp)
    {
        case 1:
            *p = u;
            break;

        case 2:
            *reinterpret_cast<Uint16*>(p) = u;
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (u >> 16) & 0xFF;
                p[1] = (u >> 8) & 0xFF;
                p[2] = u & 0xFF;
            }
            else
            {
                p[0] = u & 0xFF;
                p[1] = (u >> 8) & 0xFF;
                p[2] = (u >> 16) & 0xFF;
            }
            break;

        case 4:
            *reinterpret_cast<Uint32*>(p) = u;
            break;

        default:
            assert(false);
    }
}

/**
 * Image must be valid and pixel coords in valid range.
 *
 * \param pixel pixel coords (range x: 0..width-1 y: 0..height-1)
 * \param color color
 */
void CImage::SetPixel(const glm::ivec2& pixel, Gfx::Color color)
{
    SetPixelInt(pixel, Gfx::ColorToIntColor(color));
}

std::string CImage::GetError()
{
    return m_error;
}

bool CImage::Load(const std::filesystem::path& fileName)
{
    if (! IsEmpty() )
        Free();

    m_data = std::make_unique<ImageData>();

    m_error = "";

    auto file = CResourceManager::GetSDLFileHandler(fileName);
    if (!file->IsOpen())
    {
        m_data.reset();

        m_error = "Unable to open file";
        return false;
    }
    m_data->surface = IMG_Load_RW(file->GetHandler(), 1);
    if (m_data->surface == nullptr)
    {
        m_data.reset();

        m_error = std::string(IMG_GetError());
        return false;
    }

    if (m_data->surface->format->palette != nullptr)
    {
        ConvertToRGBA();
    }

    return true;
}

bool CImage::SavePNG(const std::filesystem::path& fileName)
{
    if (IsEmpty())
    {
        m_error = "Empty image!";
        return false;
    }

    m_error = "";

    if (! PNGSaveSurface(StrUtils::ToString(fileName).c_str(), m_data->surface) )
    {
        m_error = PNG_ERROR;
        return false;
    }

    return true;
}

void CImage::SetDataPixels(void *pixels)
{
    Uint8* srcPixels = static_cast<Uint8*> (pixels);
    Uint8* resultPixels = static_cast<Uint8*> (m_data->surface->pixels);

    Uint32 pitch = m_data->surface->pitch;

    for (int line = 0; line < m_data->surface->h; ++line)
    {
        Uint32 pos = line * pitch;
        memcpy(&resultPixels[pos], &srcPixels[pos], pitch);
    }
}

void CImage::FlipVertically()
{
    SDL_Surface* result = SDL_CreateRGBSurface( m_data->surface->flags,
                                                m_data->surface->w,
                                                m_data->surface->h,
                                                m_data->surface->format->BytesPerPixel * 8,
                                                m_data->surface->format->Rmask,
                                                m_data->surface->format->Gmask,
                                                m_data->surface->format->Bmask,
                                                m_data->surface->format->Amask);

    assert(result != nullptr);

    Uint8* srcPixels = static_cast<Uint8*> (m_data->surface->pixels);
    Uint8* resultPixels = static_cast<Uint8*> (result->pixels);

    Uint32 pitch = m_data->surface->pitch;
    Uint32 pxLength = pitch*m_data->surface->h;

    for (int line = 0; line < m_data->surface->h; ++line)
    {
        Uint32 pos = line * pitch;
        memcpy(&resultPixels[pos], &srcPixels[(pxLength-pos)-pitch], pitch);
    }

    SDL_FreeSurface(m_data->surface);

    m_data->surface = result;
}
