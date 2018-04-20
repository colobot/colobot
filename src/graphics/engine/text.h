/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/engine/text.h
 * \brief Text rendering - CText class
 */

#pragma once


#include "graphics/core/color.h"

#include "math/intpoint.h"
#include "math/point.h"

#include <map>
#include <memory>
#include <vector>


// Graphics module namespace
namespace Gfx
{

class CEngine;
class CDevice;

//! Standard small font size
const float FONT_SIZE_SMALL = 12.0f;
//! Standard big font size
const float FONT_SIZE_BIG = 18.0f;

/**
 * \enum TextAlign
 * \brief Type of text alignment
 */
enum TextAlign
{
    TEXT_ALIGN_RIGHT,
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER
};

/* Font meta char constants */

//! Type used for font character metainfo
typedef short FontMetaChar;

/**
 * \enum FontType
 * \brief Type of font
 *
 * Bitmask in lower 4 bits (mask 0x00f)
 */
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
 * \enum FontTitle
 * \brief Size of font title
 *
 * Used internally by CEdit
 *
 * Bitmask in 2 bits left shifted 4 (mask 0x030)
 */
enum FontTitle
{
    FONT_TITLE_BIG       = 0x01 << 4,
    FONT_TITLE_NORM      = 0x02 << 4,
    FONT_TITLE_LITTLE    = 0x03 << 4,
};

/**
 * \enum FontHighlight
 * \brief Type of color highlight for text
 *
 * Bitmask in 4 bits left shifted 6 (mask 0x3c0)
 */
enum FontHighlight
{
    FONT_HIGHLIGHT_NONE      = 0x00 << 6,
    FONT_HIGHLIGHT_TABLE     = 0x02 << 6, //!< code background in SatCom
    FONT_HIGHLIGHT_KEY       = 0x03 << 6, //!< background for keys in documentation in SatCom
    FONT_HIGHLIGHT_TOKEN     = 0x04 << 6, //!< keywords in CBot scripts
    FONT_HIGHLIGHT_TYPE      = 0x05 << 6, //!< types in CBot scripts
    FONT_HIGHLIGHT_CONST     = 0x06 << 6, //!< constants in CBot scripts
    FONT_HIGHLIGHT_THIS      = 0x07 << 6, //!< "this" keyword in CBot scripts
    FONT_HIGHLIGHT_COMMENT   = 0x08 << 6, //!< comments in CBot scripts
    FONT_HIGHLIGHT_KEYWORD   = 0x09 << 6, //!< builtin keywords in CBot scripts
    FONT_HIGHLIGHT_STRING    = 0x0A << 6, //!< string literals in CBot scripts
};

/**
 * \enum FontMask
 * \brief Masks in FontMetaChar for different attributes
 */
enum FontMask
{
    //! Mask for FontType
    FONT_MASK_FONT  = 0x00f,
    //! Mask for FontTitle
    FONT_MASK_TITLE = 0x030,
    //! Mask for FontHighlight
    FONT_MASK_HIGHLIGHT = 0x3c0,
    //! Mask for links
    FONT_MASK_LINK = 0x400,
    //! Mask for image bit (TODO: not used?)
    FONT_MASK_IMAGE = 0x800
};


/**
 * \struct UTF8Char
 * \brief UTF-8 character in font cache
 *
 * 4-bytes chars are supported (neither the rare 5 bytes nor 6 bytes)
 */
struct UTF8Char
{
    char c1, c2, c3, c4;
    //    // Padding for 4-byte alignment
    //    // It also seems to fix some problems reported by valgrind
    //    char pad;

    explicit UTF8Char(const char ch1 = '\0', const char ch2 = '\0',
        const char ch3 = '\0', const char ch4 = '\0')
        : c1(ch1), c2(ch2), c3(ch3), c4(ch4)
    {
    }

    explicit UTF8Char(const std::string &text,
                      const std::size_t index,
                      const char*optCallOrigin="")
        //: c1('\0'), c2('\0'), c3('\0'), c4('\0')
    {
        Init(text, index, optCallOrigin);
    }

    unsigned short Init(const std::string &text,
                        const std::size_t index,
                        const char*optCallOrigin="");

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

        if (c3 < other.c3)
            return true;
        else if (c3 > other.c3)
            return false;

        return c4 < other.c4;
    }

    inline bool operator==(const UTF8Char &other) const
    {
        return c1 == other.c1 && c2 == other.c2
            && c3 == other.c3 && c4 == other.c4;
    }
};

/**
 * \struct CharTexture
 * \brief Texture of font character
 */
struct CharTexture
{
    unsigned int id = 0;
    Math::IntPoint charPos;
    Math::IntPoint charSize;
};

// Definition is private - in text.cpp
struct CachedFont;
struct MultisizeFont;
struct FontTexture;

/**
 * \enum SpecialChar
 * \brief Special codes for certain characters
 */
enum SpecialChar
{
    CHAR_TAB        = '\t', //! Tab character - :
    CHAR_NEWLINE    = '\n', //! Newline character - arrow pointing down and left
    CHAR_DOT        = 1,    //! Single dot in the middle
    CHAR_SQUARE     = 2,    //! Square
    CHAR_SKIP_RIGHT = 5,    //! Filled triangle pointing right
    CHAR_SKIP_LEFT  = 6     //! Filled triangle pointing left
};

/**
 * \class CText
 * \brief Text rendering engine
 *
 * CText is responsible for drawing text in 2D interface. Font rendering is done using
 * textures generated by SDL_ttf from TTF font files.
 *
 * All functions rendering text are divided into two types:
 * - single font - function takes a single FontType argument that (along with size)
 *   determines the font to be used for all characters,
 * - multi-font - function takes the text as one argument and a std::vector of FontMetaChar
 *   with per-character formatting information (font, highlights and some other info used by CEdit)
 *
 * All font rendering is done in UTF-8.
 */
class CText
{
public:
    CText(CEngine* engine);
    virtual ~CText();

    //! Sets the device to be used
    void        SetDevice(CDevice *device);

    //! Returns the last encountered error
    std::string GetError()const;

    //! Initializes the font engine; must be called after SetDevice()
    bool        Create();
    //! Frees resources before exit
    void        Destroy();

    //! Flushes cached textures
    void        FlushCache();

    //@{
    //! Tab size management
    void        SetTabSize(const int tabSize);
    int         GetTabSize()const;
    //@}

    //! Draws text (multi-format)
    void        DrawText(const std::string &text,
                         const std::vector<FontMetaChar>::iterator format,
                         const std::vector<FontMetaChar>::iterator end,
                         const float size,
                         Math::Point pos,
                         const float width,
                         const TextAlign align,
                         const int eol,
                         const Color color = Color(0.0f, 0.0f, 0.0f, 1.0f));
    //! Draws text (one font)
    void        DrawText(const std::string &text,
                         const FontType font,
                         const float size,
                         Math::Point pos,
                         const float width,
                         const TextAlign align,
                         const int eol,
                         const Color color = Color(0.0f, 0.0f, 0.0f, 1.0f));

    //! Calculates dimensions for text (multi-format)
    void        SizeText(const std::string &text,
                         const std::vector<FontMetaChar>::iterator format,
                         const std::vector<FontMetaChar>::iterator endFormat,
                         const float size,
                         const Math::Point pos,
                         const TextAlign align,
                         Math::Point &start, Math::Point &end);
    //! Calculates dimensions for text (one font)
    void        SizeText(const std::string &text,
                         const FontType font,
                         const float size,
                         const Math::Point pos,
                         const TextAlign align,
                         Math::Point &start,
                         Math::Point &end);

    //! Returns the ascent font metric
    float       GetAscent(const FontType font, const float size);
    //! Returns the descent font metric
    float       GetDescent(const FontType font, const float size);
    //! Returns the height font metric
    float       GetHeight(const FontType font, const float size);
    int GetHeightInt(const FontType font, const float size);

    //! Returns width of string (multi-format)
    TEST_VIRTUAL float GetStringWidth(const std::string& text,
                                      const std::vector<FontMetaChar>::iterator format,
                                      const std::vector<FontMetaChar>::iterator end,
                                      const float size);      // warning : TEST_VIRTUAL <-> const everywhere
    //! Returns width of string (single font)
    TEST_VIRTUAL float GetStringWidth(std::string text, const FontType font, const float size);      // warning : TEST_VIRTUAL <-> const everywhere
    //! Returns width of single character
    TEST_VIRTUAL float GetCharWidth(UTF8Char ch, const FontType font, const float size, const float offset);    // warning : TEST_VIRTUAL <-> const everywhere
    int GetCharWidthInt(UTF8Char ch, const FontType font, const float size, const float offset);

    //! Justifies a line of text (multi-format)
    int         Justify(const std::string &text,
                        const std::vector<FontMetaChar>::iterator format,
                        const std::vector<FontMetaChar>::iterator end,
                        const float size,
                        const float width);
    //! Justifies a line of text (one font)
    int         Justify(const std::string &text,
                        const FontType font,
                        const float size,
                        const float width);

    //! Returns the most suitable position to a given offset (multi-format)
    int         Detect(const std::string &text,
                       const std::vector<FontMetaChar>::iterator format,
                       const std::vector<FontMetaChar>::iterator end,
                       const float size,
                       const float offset);
    //! Returns the most suitable position to a given offset (one font)
    int         Detect(const std::string &text,
                       const FontType font,
                       const float size,
                       const float offset);

    UTF8Char    TranslateSpecialChar(const char specialChar)const;

    CharTexture GetCharTexture(const UTF8Char ch, const FontType font, const float size);
    Math::IntPoint GetFontTextureSize()const;

protected:
    CachedFont* GetOrOpenFont(const FontType font, const float size);
    CharTexture CreateCharTexture(UTF8Char ch, CachedFont* font);
    FontTexture* GetOrCreateFontTexture(Math::IntPoint tileSize);
    FontTexture CreateFontTexture(Math::IntPoint tileSize);
    Math::IntPoint GetNextTilePos(const FontTexture& fontTexture);

    void        DrawString(const std::string &text, std::vector<FontMetaChar>::iterator format,
                           std::vector<FontMetaChar>::iterator end,
                           float size, Math::IntPoint pos, int width, int eol, Color color);
    void        DrawString(const std::string &text, FontType font,
                           float size, Math::IntPoint pos, int width, int eol, Color color);
    void        DrawHighlight(FontMetaChar hl, Math::IntPoint pos, Math::IntPoint size);
    void        DrawCharAndAdjustPos(UTF8Char ch, const FontType font, const float size, Math::IntPoint &pos, Color color);
    void        StringToUTFCharList(const std::string &text, std::vector<UTF8Char> &chars);
    void        StringToUTFCharList(const std::string &text, std::vector<UTF8Char> &chars, std::vector<FontMetaChar>::iterator format, std::vector<FontMetaChar>::iterator end);

protected:
    CEngine*       m_engine;
    CDevice*       m_device;

    std::string  m_error;
    float        m_defaultSize;
    int          m_tabSize;

    std::map<FontType, std::unique_ptr<MultisizeFont>> m_fonts;
    std::vector<FontTexture> m_fontTextures;

    FontType     m_lastFontType;
    int          m_lastFontSize;
    CachedFont*  m_lastCachedFont;
};


} // namespace Gfx
