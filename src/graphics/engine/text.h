// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// text.h

#pragma once

#include "math/point.h"
#include "math/size.h"

#include <vector>
#include <map>

class CInstanceManager;

namespace Gfx {

class CEngine;
class CDevice;

//! Standard small font size
const float FONT_SIZE_SMALL = 10.0f;
//! Standard big font size
const float FONT_SIZE_BIG = 15.0f;

/**
  \enum TextAlignType
  \brief Type of text alignment */
enum JustifyType
{
    TEXT_ALIGN_RIGHT,
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER
};

/* Font meta char constants */

//! Type used for font character metainfo
typedef short FontMetaChar;

/**
  \enum FontType
  \brief Type of font

  Bitmask in lower 4 bits (mask 0x00f) */
enum FontType
{
    //! Flag for bold font subtype
    FONT_BOLD       = 0x04,
    //! Flag for italic font subtype
    FONT_ITALIC     = 0x08,

    //! Default colobot font used for interface
    FONT_COLOBOT    = 0x00,
    //! Alias for bold colobot font
    FONT_COLOBOT_BOLD = FONT_COLOBOT | FONT_BOLD,
    //! Alias for italic colobot font
    FONT_COLOBOT_ITALIC = FONT_COLOBOT | FONT_ITALIC,

    //! Courier (monospace) font used mainly in code editor (only regular & bold)
    FONT_COURIER    = 0x01,
    //! Alias for bold courier font
    FONT_COURIER_BOLD = FONT_COURIER | FONT_BOLD,

    // 0x02 left for possible another font

    //! Pseudo-font loaded from textures for buttons, icons, etc.
    FONT_BUTTON     = 0x03,
};

/**
  \enum FontTitle
  \brief Size of font title

  Bitmask in 2 bits left shifted 4 (mask 0x030) */
enum FontTitle
{
    FONT_TITLE_BIG       = 0x01 << 4,
    FONT_TITLE_NORM      = 0x02 << 4,
    FONT_TITLE_LITTLE    = 0x03 << 4,
};

/**
  \enum FontColor
  \brief Font color type (?)

  Bitmask in 3 bits left shifted 6 (mask 0x1c0) */
enum FontColor
{
    FONT_COLOR_LINK      = 0x01 << 6,
    FONT_COLOR_TOKEN     = 0x02 << 6,
    FONT_COLOR_TYPE      = 0x03 << 6,
    FONT_COLOR_CONST     = 0x04 << 6,
    FONT_COLOR_REM       = 0x05 << 6,
    FONT_COLOR_KEY       = 0x06 << 6,
    FONT_COLOR_TABLE     = 0x07 << 6,
};

/**
  \enum FontMask
  \brief Masks in FontMetaChar for different attributes */
enum FontMask
{
    //! Mask for FontType
    FONT_MASK_FONT  = 0x00f,
    //! Mask for FontTitle
    FONT_MASK_TITLE = 0x030,
    //! Mask for FontColor
    FONT_MASK_COLOR = 0x1c0,
    //! Mask for image bit
    FONT_MASK_IMAGE = 0x200
};


/**
  \struct UTF8Char
  \brief UTF-8 character in font cache

  Only 3-byte chars are supported */
struct UTF8Char
{
    char c1, c2, c3;

    explicit UTF8Char(char ch1 = '\0', char ch2 = '\0', char ch3 = '\0')
        : c1(ch1), c2(ch2), c3(ch3) {}

    inline bool operator<(const UTF8Char &other) const
    {
        if (c1 < other.c1)
            return true;
        else if (c1 > other.c1)
            return false;

        if (c2 < other.c2)
            return true;
        else if (c2 > other.c2)
            return false;

        return c3 < other.c3;
    }

    inline bool operator==(const UTF8Char &other) const
    {
        return c1 == other.c1 && c2 == other.c2 && c3 == other.c3;
    }
};

/**
  \struct CharTexture
  \brief Texture of font character */
struct CharTexture
{
    unsigned int id;
    Math::Size texSize;
    Math::Size charSize;

    CharTexture() : id(0) {}
};

// Definition is private - in text.cpp
struct CachedFont;

/**
  \struct MultisizeFont
  \brief Font with multiple possible sizes */
struct MultisizeFont
{
    std::string fileName;
    std::map<int, CachedFont*> fonts;

    MultisizeFont(const std::string &fn)
        : fileName(fn) {}
};

/**
  \class CText
  \brief Text rendering engine

  ... */
class CText
{
public:
    CText(CInstanceManager *iMan, Gfx::CEngine* engine);
    ~CText();

    //! Sets the device to be used
    void        SetDevice(Gfx::CDevice *device);

    //! Returns the last encountered error
    std::string GetError();

    //! Initializes the font engine; must be called after SetDevice()
    bool        Create();
    //! Frees resources before exit
    void        Destroy();

    //! Flushes cached textures
    void        FlushCache();

    //! Draws text (multi-format)
    void        DrawText(const std::string &text, const std::vector<Gfx::FontMetaChar> &format,
                         Math::Point pos, float width, Gfx::JustifyType justify, float size,
                         float stretch, int eol);
    //! Draws text (one font)
    void        DrawText(const std::string &text, Gfx::FontType font,
                         Math::Point pos, float width, Gfx::JustifyType justify, float size,
                         float stretch, int eol);

    //! Calculates dimensions for text (multi-format)
    void        SizeText(const std::string &text, const std::vector<Gfx::FontMetaChar> &format,
                         Math::Point pos, Gfx::JustifyType justify, float size,
                         Math::Point &start, Math::Point &end);
    //! Calculates dimensions for text (one font)
    void        SizeText(const std::string &text, Gfx::FontType font,
                         Math::Point pos, Gfx::JustifyType justify, float size,
                         Math::Point &start, Math::Point &end);

    //! Returns the ascent font metric
    float       GetAscent(Gfx::FontType font, float size);
    //! Returns the descent font metric
    float       GetDescent(Gfx::FontType font, float size);
    //! Returns the height font metric
    float       GetHeight(Gfx::FontType font, float size);

    //! Returns width of string (multi-format)
    float       GetStringWidth(const std::string &text,
                               const std::vector<Gfx::FontMetaChar> &format, float size);
    //! Returns width of string (single font)
    float       GetStringWidth(const std::string &text, Gfx::FontType font, float size);
    //! Returns width of single character
    float       GetCharWidth(int character, Gfx::FontType font, float size, float offset);

    //! Justifies a line of text (multi-format)
    int         Justify(const std::string &text, const std::vector<Gfx::FontMetaChar> &format,
                        float size, float width);
    //! Justifies a line of text (one font)
    int         Justify(const std::string &text, Gfx::FontType font, float size, float width);

    //! Returns the most suitable position to a given offset (multi-format)
    int         Detect(const std::string &text, const std::vector<Gfx::FontMetaChar> &format,
                       float size, float offset);
    //! Returns the most suitable position to a given offset (one font)
    int         Detect(const std::string &text, Gfx::FontType font, float size, float offset);

public: // for testing!
    Gfx::CachedFont* GetOrOpenFont(Gfx::FontType type, float size);
    Gfx::CharTexture CreateCharTexture(const char* utf8Char, Gfx::CachedFont* font);

    void        DrawString(const std::string &text, const std::vector<Gfx::FontMetaChar> &format,
                           float size, Math::Point pos, float width, int eol);
    void        DrawString(const std::string &text, Gfx::FontType font,
                           float size, Math::Point pos, float width, int eol);
    void        DrawColor(int color, float size, Math::Point pos, float width);
    void        DrawChar(UTF8Char character, Gfx::FontType font, float size, Math::Point &pos);

protected:
    CInstanceManager*   m_iMan;
    Gfx::CEngine*       m_engine;
    Gfx::CDevice*       m_device;

    std::string  m_error;
    float        m_defaultSize;
    std::string  m_fontPath;

    std::map<Gfx::FontType, Gfx::MultisizeFont*> m_fonts;

    Gfx::FontType     m_lastFontType;
    int               m_lastFontSize;
    Gfx::CachedFont*  m_lastCachedFont;
};

}; // namespace Gfx
