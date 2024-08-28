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
 * \file graphics/engine/text.h
 * \brief Text rendering - CText class
 */

#pragma once


#include "graphics/core/color.h"

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

namespace StrUtils
{
class CodePoint;
}

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
enum FontType : unsigned char
{
    //! Flag for bold font subtype
    FONT_BOLD       = 0b0000'01'00,
    //! Flag for italic font subtype
    FONT_ITALIC     = 0b0000'10'00,

    //! Default colobot font used for interface
    FONT_COMMON    = 0b0000'00'00,
    //! Alias for bold colobot font
    FONT_COMMON_BOLD = FONT_COMMON | FONT_BOLD,
    //! Alias for italic colobot font
    FONT_COMMON_ITALIC = FONT_COMMON | FONT_ITALIC,

    //! Studio font used mainly in code editor
    FONT_STUDIO    = 0b0000'00'01,
    //! Alias for bold studio font
    FONT_STUDIO_BOLD = FONT_STUDIO | FONT_BOLD,
    //! Alias for italic studio font (at this point not used anywhere)
    FONT_STUDIO_ITALIC = FONT_STUDIO | FONT_ITALIC,

    //! SatCom font used for interface (currently bold and italic wariants aren't used anywhere)
    FONT_SATCOM = 0b0000'00'10,
    //! Alias for bold satcom font
    FONT_SATCOM_BOLD = FONT_SATCOM | FONT_BOLD,
    //! Alias for italic satcom font
    FONT_SATCOM_ITALIC = FONT_SATCOM | FONT_ITALIC,

    //! Pseudo-font loaded from textures for buttons, icons, etc.
    FONT_BUTTON     = 0b0000'00'11,
};

std::string ToString(FontType);

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
 * \struct CharTexture
 * \brief Texture of font character
 */
struct CharTexture
{
    unsigned int id = 0;
    glm::ivec2 charPos;
    glm::ivec2 charSize;
};

// Definition is private - in text.cpp
class FontsCache;
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
    std::string GetError();

    //! Initializes the font engine; must be called after SetDevice()
    bool        Create();
    //! Frees resources before exit
    void        Destroy();

    //! Flushes cached textures
    void        FlushCache();
    //! Try to load new font files
    bool        ReloadFonts();

    //@{
    //! Tab size management
    void        SetTabSize(int tabSize);
    int         GetTabSize();
    //@}

    //! Draws text (multi-format)
    void        DrawText(const std::string &text, std::vector<FontMetaChar>::iterator format,
                         std::vector<FontMetaChar>::iterator end,
                         float size, glm::vec2 pos, float width, TextAlign align,
                         char32_t eol, Color color = Color(0.0f, 0.0f, 0.0f, 1.0f));
    //! Draws text (one font)
    void        DrawText(const std::string &text, FontType font,
                         float size, glm::vec2 pos, float width, TextAlign align,
                         char32_t eol, Color color = Color(0.0f, 0.0f, 0.0f, 1.0f));

    //! Calculates dimensions for text (multi-format)
    void        SizeText(const std::string &text, std::vector<FontMetaChar>::iterator format,
                         std::vector<FontMetaChar>::iterator endFormat,
                         float size, glm::vec2 pos, TextAlign align,
                         glm::vec2&start, glm::vec2&end);
    //! Calculates dimensions for text (one font)
    void        SizeText(const std::string &text, FontType font,
                         float size, glm::vec2 pos, TextAlign align,
                         glm::vec2&start, glm::vec2&end);

    //! Returns the ascent font metric
    float       GetAscent(FontType font, float size);
    //! Returns the descent font metric
    float       GetDescent(FontType font, float size);
    //! Returns the height font metric
    float       GetHeight(FontType font, float size);
    int GetHeightInt(FontType font, float size);

    //! Returns width of string (multi-format)
    TEST_VIRTUAL float GetStringWidth(const std::string& text,
                                      std::vector<FontMetaChar>::iterator format,
                                      std::vector<FontMetaChar>::iterator end, float size);
    //! Returns width of string (single font)
    TEST_VIRTUAL float GetStringWidth(std::string text, FontType font, float size);
    //! Returns width of single character
    TEST_VIRTUAL float GetCharWidth(StrUtils::CodePoint ch, FontType font, float size, float offset);
    int GetCharWidthInt(StrUtils::CodePoint ch, FontType font, float size, float offset);

    //! Justifies a line of text (multi-format)
    int         Justify(const std::string &text, std::vector<FontMetaChar>::iterator format,
                        std::vector<FontMetaChar>::iterator end,
                        float size, float width);
    //! Justifies a line of text (one font)
    int         Justify(const std::string &text, FontType font, float size, float width);

    //! Returns the most suitable position to a given offset (multi-format)
    int         Detect(const std::string &text, std::vector<FontMetaChar>::iterator format,
                       std::vector<FontMetaChar>::iterator end,
                       float size, float offset);
    //! Returns the most suitable position to a given offset (one font)
    int         Detect(const std::string &text, FontType font, float size, float offset);

    StrUtils::CodePoint TranslateSpecialChar(char32_t specialChar);

    CharTexture GetCharTexture(StrUtils::CodePoint ch, FontType font, float size);
    glm::ivec2 GetFontTextureSize();

protected:
    int GetFontPointSize(float size) const;
    CachedFont* GetOrOpenFont(FontType type, float size);
    CharTexture CreateCharTexture(StrUtils::CodePoint ch, CachedFont* font);
    FontTexture* GetOrCreateFontTexture(const glm::ivec2& tileSize);
    FontTexture CreateFontTexture(const glm::ivec2& tileSize);
    glm::ivec2 GetNextTilePos(const FontTexture& fontTexture);

    void        DrawString(const std::string &text, std::vector<FontMetaChar>::iterator format,
                           std::vector<FontMetaChar>::iterator end,
                           float size, const glm::ivec2& pos, int width, char32_t eol, Color color);
    void        DrawString(const std::string &text, FontType font,
                           float size, const glm::ivec2& pos, int width, char32_t eol, Color color);
    void        DrawHighlight(FontMetaChar hl, const glm::ivec2& pos, const glm::ivec2& size);
    void        DrawCharAndAdjustPos(StrUtils::CodePoint ch, FontType font, float size, glm::ivec2&pos, Color color);
    void        StringToUTFCharList(std::string_view text, std::vector<StrUtils::CodePoint> &chars);
    void        StringToUTFCharList(std::string_view text, std::vector<StrUtils::CodePoint> &chars, std::vector<FontMetaChar>::iterator format, std::vector<FontMetaChar>::iterator end);

    int         GetCharSizeAt(Gfx::FontType font, std::string_view text) const;

protected:
    CEngine*       m_engine;
    CDevice*       m_device;

    std::string  m_error;
    float        m_defaultSize;
    int          m_tabSize;

    std::unique_ptr<FontsCache> m_fontsCache;
    std::vector<FontTexture> m_fontTextures;

    class CQuadBatch;
    std::unique_ptr<CQuadBatch> m_quadBatch;
};


} // namespace Gfx
