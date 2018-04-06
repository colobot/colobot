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


#include "graphics/engine/text.h"

#include "app/app.h"

#include "common/image.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "graphics/engine/engine.h"

#include "math/func.h"

#include <SDL.h>
#include <SDL_ttf.h>


// Graphics module namespace
namespace Gfx
{

/**
 * \struct MultisizeFont
 * \brief Font with multiple possible sizes
 */
struct MultisizeFont
{
    std::string fileName;
    std::map<int, std::unique_ptr<CachedFont>> fonts;

    explicit MultisizeFont(const std::string &fn)
        : fileName(fn) {}
};

/**
 * \struct FontTexture
 * \brief Single texture filled with character textures
 */
struct FontTexture
{
    unsigned int id = 0;
    Math::IntPoint tileSize;
    int freeSlots = 0;
};

/**
 * \struct CachedFont
 * \brief Base TTF font with UTF-8 char cache
 */
struct CachedFont
{
    std::unique_ptr<CSDLMemoryWrapper> fontFile;
    TTF_Font* font = nullptr;
    std::map<UTF8Char, CharTexture> cache;

    CachedFont(std::unique_ptr<CSDLMemoryWrapper> fontFile, int pointSize)
        : fontFile(std::move(fontFile))
    {
        font = TTF_OpenFontRW(this->fontFile->GetHandler(), 0, pointSize);
    }

    ~CachedFont()
    {
        if (font != nullptr)
            TTF_CloseFont(font);
    }
};


namespace
{
const Math::IntPoint REFERENCE_SIZE(800, 600);
const Math::IntPoint FONT_TEXTURE_SIZE(256, 256);
} // anonymous namespace


CText::CText(CEngine* engine)
{
    m_device = nullptr;
    m_engine = engine;

    m_defaultSize = 12.0f;
    m_tabSize = 4;

    m_lastFontType = FONT_COLOBOT;
    m_lastFontSize = 0;
    m_lastCachedFont = nullptr;
}

CText::~CText()
{
    m_device = nullptr;
    m_engine = nullptr;
}

bool CText::Create()
{
    if (TTF_Init() != 0)
    {
        m_error = std::string("TTF_Init error: ") + std::string(TTF_GetError());
        return false;
    }

    m_fonts[FONT_COLOBOT]        = MakeUnique<MultisizeFont>("fonts/dvu_sans.ttf");
    m_fonts[FONT_COLOBOT_BOLD]   = MakeUnique<MultisizeFont>("fonts/dvu_sans_bold.ttf");
    m_fonts[FONT_COLOBOT_ITALIC] = MakeUnique<MultisizeFont>("fonts/dvu_sans_italic.ttf");

    m_fonts[FONT_COURIER]        = MakeUnique<MultisizeFont>("fonts/dvu_sans_mono.ttf");
    m_fonts[FONT_COURIER_BOLD]   = MakeUnique<MultisizeFont>("fonts/dvu_sans_mono_bold.ttf");

    for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it)
    {
        FontType type = (*it).first;
        CachedFont* cf = GetOrOpenFont(type, m_defaultSize);
        if (cf == nullptr || cf->font == nullptr)
            return false;
    }

    return true;
}

void CText::Destroy()
{
    m_fonts.clear();

    m_lastCachedFont = nullptr;
    m_lastFontType = FONT_COLOBOT;
    m_lastFontSize = 0;

    TTF_Quit();
}

void CText::SetDevice(CDevice* device)
{
    m_device = device;
}

std::string CText::GetError()
{
    return m_error;
}

void CText::FlushCache()
{
    for (auto& fontTexture : m_fontTextures)
    {
        Texture tex;
        tex.id = fontTexture.id;
        m_device->DestroyTexture(tex);
    }
    m_fontTextures.clear();

    for (auto& multisizeFont : m_fonts)
    {
        for (auto& cachedFont : multisizeFont.second->fonts)
        {
            cachedFont.second->cache.clear();
        }
    }

    m_lastCachedFont = nullptr;
    m_lastFontType = FONT_COLOBOT;
    m_lastFontSize = 0;

    Create();
}

int CText::GetTabSize()
{
    return m_tabSize;
}

void CText::SetTabSize(int tabSize)
{
    m_tabSize = tabSize;
}

void CText::DrawText(const std::string &text, std::vector<FontMetaChar>::iterator format,
                     std::vector<FontMetaChar>::iterator end,
                     float size, Math::Point pos, float width, TextAlign align,
                     int eol, Color color)
{
    float sw = 0.0f;

    if (align == TEXT_ALIGN_CENTER)
    {
        sw = GetStringWidth(text, format, end, size);
        if (sw > width) sw = width;
        pos.x -= sw / 2.0f;
    }
    else if (align == TEXT_ALIGN_RIGHT)
    {
        sw = GetStringWidth(text, format, end, size);
        if (sw > width) sw = width;
        pos.x -= sw;
    }

    Math::IntPoint intPos = m_engine->InterfaceToWindowCoords(pos);
    int intWidth = width * m_engine->GetWindowSize().x;
    DrawString(text, format, end, size, intPos, intWidth, eol, color);
}

void CText::DrawText(const std::string &text, FontType font,
                     float size, Math::Point pos, float width, TextAlign align,
                     int eol, Color color)
{
    float sw = 0.0f;

    if (align == TEXT_ALIGN_CENTER)
    {
        sw = GetStringWidth(text, font, size);
        if (sw > width) sw = width;
        pos.x -= sw / 2.0f;
    }
    else if (align == TEXT_ALIGN_RIGHT)
    {
        sw = GetStringWidth(text, font, size);
        if (sw > width) sw = width;
        pos.x -= sw;
    }

    Math::IntPoint intPos = m_engine->InterfaceToWindowCoords(pos);
    int intWidth = width * m_engine->GetWindowSize().x;
    DrawString(text, font, size, intPos, intWidth, eol, color);
}

void CText::SizeText(const std::string &text, std::vector<FontMetaChar>::iterator format,
                     std::vector<FontMetaChar>::iterator endFormat,
                     float size, Math::Point pos, TextAlign align,
                     Math::Point &start, Math::Point &end)
{
    start = end = pos;

    float sw = GetStringWidth(text, format, endFormat, size);
    end.x += sw;
    if (align == TEXT_ALIGN_CENTER)
    {
        start.x -= sw/2.0f;
        end.x   -= sw/2.0f;
    }
    else if (align == TEXT_ALIGN_RIGHT)
    {
        start.x -= sw;
        end.x   -= sw;
    }

    start.y -= GetDescent(FONT_COLOBOT, size);
    end.y   += GetAscent(FONT_COLOBOT, size);
}

void CText::SizeText(const std::string &text, FontType font,
                     float size, Math::Point pos, TextAlign align,
                     Math::Point &start, Math::Point &end)
{
    start = end = pos;

    float sw = GetStringWidth(text, font, size);
    end.x += sw;
    if (align == TEXT_ALIGN_CENTER)
    {
        start.x -= sw/2.0f;
        end.x   -= sw/2.0f;
    }
    else if (align == TEXT_ALIGN_RIGHT)
    {
        start.x -= sw;
        end.x   -= sw;
    }

    start.y -= GetDescent(font, size);
    end.y   += GetAscent(font, size);
}

float CText::GetAscent(FontType font, float size)
{
    assert(font != FONT_BUTTON);

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntPoint wndSize;
    wndSize.y = TTF_FontAscent(cf->font);
    Math::Point ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.y;
}

float CText::GetDescent(FontType font, float size)
{
    assert(font != FONT_BUTTON);

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntPoint wndSize;
    wndSize.y = TTF_FontDescent(cf->font);
    Math::Point ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.y;
}

float CText::GetHeight(FontType font, float size)
{
    assert(font != FONT_BUTTON);

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntPoint wndSize;
    wndSize.y = TTF_FontHeight(cf->font);
    Math::Point ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.y;
}

int CText::GetHeightInt(FontType font, float size)
{
    assert(font != FONT_BUTTON);

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    return TTF_FontHeight(cf->font);
}

float CText::GetStringWidth(const std::string &text,
                            std::vector<FontMetaChar>::iterator format,
                            std::vector<FontMetaChar>::iterator end, float size)
{
    float width = 0.0f;
    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        FontType font = FONT_COLOBOT;
        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        width += GetCharWidth(ch, font, size, width);

        index += len;
        fmtIndex++;
    }

    return width;
}

float CText::GetStringWidth(std::string text, FontType font, float size)
{
    assert(font != FONT_BUTTON);

    // Skip special chars
    for (char& c : text)
    {
        if (c < 32 && c >= 0)
            c = ':';
    }

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntPoint wndSize;
    TTF_SizeUTF8(cf->font, text.c_str(), &wndSize.x, &wndSize.y);
    Math::Point ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.x;
}

float CText::GetCharWidth(UTF8Char ch, FontType font, float size, float offset)
{
    if (font == FONT_BUTTON)
    {
        Math::IntPoint windowSize = m_engine->GetWindowSize();
        float height = GetHeight(FONT_COLOBOT, size);
        float width = height*(static_cast<float>(windowSize.y)/windowSize.x);
        return width;
    }

    int width = 1;
    if (ch.c1 < 32 && ch.c1 >= 0)
    {
        if (ch.c1 == '\t')
            width = m_tabSize;

        // TODO: tab sizing at intervals?

        ch.c1 = ':';
    }

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);

    Math::Point charSize;
    auto it = cf->cache.find(ch);
    if (it != cf->cache.end())
    {
        charSize = m_engine->WindowToInterfaceSize((*it).second.charSize);
    }
    else
    {
        Math::IntPoint wndSize;
        std::string text;
        text.append({ch.c1, ch.c2, ch.c3});
        TTF_SizeUTF8(cf->font, text.c_str(), &wndSize.x, &wndSize.y);
        charSize = m_engine->WindowToInterfaceSize(wndSize);
    }

    return charSize.x * width;
}

int CText::GetCharWidthInt(UTF8Char ch, FontType font, float size, float offset)
{
    if (font == FONT_BUTTON)
    {
        Math::IntPoint windowSize = m_engine->GetWindowSize();
        int height = GetHeightInt(FONT_COLOBOT, size);
        int width = height*(static_cast<float>(windowSize.y)/windowSize.x);
        return width;
    }

    int width = 1;
    if (ch.c1 < 32 && ch.c1 >= 0)
    {
        if (ch.c1 == '\t')
            width = m_tabSize;

        // TODO: tab sizing at intervals?

        ch.c1 = ':';
    }

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);

    Math::IntPoint charSize;
    auto it = cf->cache.find(ch);
    if (it != cf->cache.end())
    {
        charSize = (*it).second.charSize;
    }
    else
    {
        std::string text;
        text.append({ch.c1, ch.c2, ch.c3});
        TTF_SizeUTF8(cf->font, text.c_str(), &charSize.x, &charSize.y);
    }

    return charSize.x * width;
}


int CText::Justify(const std::string &text, std::vector<FontMetaChar>::iterator format,
                   std::vector<FontMetaChar>::iterator end,
                   float size, float width)
{
    float pos = 0.0f;
    int cut = 0;
    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        FontType font = FONT_COLOBOT;
        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        if (font != FONT_BUTTON)
        {
            if (ch.c1 == '\n')
                return index+1;
            if (ch.c1 == ' ')
                cut = index+1;
        }

        pos += GetCharWidth(ch, font, size, pos);
        if (pos > width)
        {
            if (cut == 0) return index;
            else          return cut;
        }

        index += len;
        fmtIndex++;
    }

    return index;
}

int CText::Justify(const std::string &text, FontType font, float size, float width)
{
    assert(font != FONT_BUTTON);

    float pos = 0.0f;
    int cut = 0;
    unsigned int index = 0;
    while (index < text.length())
    {
        UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        if (ch.c1 == '\n')
        {
            return index+1;
        }

        if (ch.c1 == ' ' )
            cut = index+1;

        pos += GetCharWidth(ch, font, size, pos);
        if (pos > width)
        {
            if (cut == 0) return index;
            else          return cut;
        }
        index += len;
    }

    return index;
}

int CText::Detect(const std::string &text, std::vector<FontMetaChar>::iterator format,
                  std::vector<FontMetaChar>::iterator end,
                  float size, float offset)
{
    float pos = 0.0f;
    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        FontType font = FONT_COLOBOT;

        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        // TODO: if (font == FONT_BUTTON)
        //if (font == FONT_BUTTON) continue;

        UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        if (ch.c1 == '\n')
            return index;

        float width = GetCharWidth(ch, font, size, pos);
        if (offset <= pos + width/2.0f)
            return index;

        pos += width;
        index += len;
        fmtIndex++;
    }

    return index;
}

int CText::Detect(const std::string &text, FontType font, float size, float offset)
{
    assert(font != FONT_BUTTON);

    float pos = 0.0f;
    unsigned int index = 0;
    while (index < text.length())
    {
        UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        index += len;

        if (ch.c1 == '\n')
            return index;

        float width = GetCharWidth(ch, font, size, pos);
        if (offset <= pos + width/2.0f)
            return index;

        pos += width;
    }

    return index;
}

UTF8Char CText::TranslateSpecialChar(int specialChar)
{
    UTF8Char ch;

    switch (specialChar)
    {
        case CHAR_TAB:
            ch.c1 = ':';
            ch.c2 = 0;
            ch.c3 = 0;
            break;

        case CHAR_NEWLINE:
            // Unicode: U+21B2
            ch.c1 = static_cast<char>(0xE2);
            ch.c2 = static_cast<char>(0x86);
            ch.c3 = static_cast<char>(0xB2);
            break;

        case CHAR_DOT:
            // Unicode: U+23C5
            ch.c1 = static_cast<char>(0xE2);
            ch.c2 = static_cast<char>(0x8F);
            ch.c3 = static_cast<char>(0x85);
            break;

        case CHAR_SQUARE:
            // Unicode: U+25FD
            ch.c1 = static_cast<char>(0xE2);
            ch.c2 = static_cast<char>(0x97);
            ch.c3 = static_cast<char>(0xBD);
            break;

        case CHAR_SKIP_RIGHT:
            // Unicode: U+25B6
            ch.c1 = static_cast<char>(0xE2);
            ch.c2 = static_cast<char>(0x96);
            ch.c3 = static_cast<char>(0xB6);
            break;

        case CHAR_SKIP_LEFT:
            // Unicode: U+25C0
            ch.c1 = static_cast<char>(0xE2);
            ch.c2 = static_cast<char>(0x97);
            ch.c3 = static_cast<char>(0x80);
            break;

        default:
            ch.c1 = '?';
            ch.c2 = 0;
            ch.c3 = 0;
            break;
    }

    return ch;
}

void CText::DrawString(const std::string &text, std::vector<FontMetaChar>::iterator format,
                       std::vector<FontMetaChar>::iterator end,
                       float size, Math::IntPoint pos, int width, int eol, Color color)
{
    m_engine->SetState(ENG_RSTATE_TEXT);

    int start = pos.x;

    unsigned int fmtIndex = 0;

    std::vector<UTF8Char> chars;
    StringToUTFCharList(text, chars, format, end);
    for (auto it = chars.begin(); it != chars.end(); ++it)
    {
        FontType font = FONT_COLOBOT;
        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        UTF8Char ch = *it;

        int offset = pos.x - start;
        int cw = GetCharWidthInt(ch, font, size, offset);
        if (offset + cw > width)  // exceeds the maximum width?
        {
            ch = TranslateSpecialChar(CHAR_SKIP_RIGHT);
            cw = GetCharWidthInt(ch, font, size, offset);
            pos.x = start + width - cw;
            color = Color(1.0f, 0.0f, 0.0f);
            DrawCharAndAdjustPos(ch, font, size, pos, color);
            break;
        }

        Color c = color;
        FontHighlight hl = static_cast<FontHighlight>(format[fmtIndex] & FONT_MASK_HIGHLIGHT);
        if (hl == FONT_HIGHLIGHT_TOKEN)
        {
            c = Color(0.490f, 0.380f, 0.165f, 1.0f); // #7D612A
        }
        else if (hl == FONT_HIGHLIGHT_TYPE)
        {
            c = Color(0.31f, 0.443f, 0.196f, 1.0f); // #4F7132
        }
        else if (hl == FONT_HIGHLIGHT_CONST)
        {
            c = Color(0.882f, 0.176f, 0.176f, 1.0f); // #E12D2D
        }
        else if (hl == FONT_HIGHLIGHT_THIS)
        {
            c = Color(0.545f, 0.329f, 0.608f, 1.0f); // #8B549B
        }
        else if (hl == FONT_HIGHLIGHT_COMMENT)
        {
            c = Color(0.251f, 0.271f, 0.306f, 1.0f); // #40454E
        }
        else if (hl == FONT_HIGHLIGHT_KEYWORD)
        {
            c = Color(0.239f, 0.431f, 0.588f, 1.0f); // #3D6E96
        }
        else if (hl == FONT_HIGHLIGHT_STRING)
        {
            c = Color(0.239f, 0.384f, 0.341f, 1.0f); // #3D6257
        }

        // draw highlight background or link underline
        if (font != FONT_BUTTON)
        {
            Math::IntPoint charSize;
            charSize.x = GetCharWidthInt(ch, font, size, offset);
            charSize.y = GetHeightInt(font, size);
            DrawHighlight(format[fmtIndex], pos, charSize);
        }

        DrawCharAndAdjustPos(ch, font, size, pos, c);

        // increment fmtIndex for each byte in multibyte character
        if ( ch.c1 != 0 )
            fmtIndex++;
        if ( ch.c2 != 0 )
            fmtIndex++;
        if ( ch.c3 != 0 )
            fmtIndex++;
    }

    if (eol != 0)
    {
        FontType font = FONT_COLOBOT;
        UTF8Char ch = TranslateSpecialChar(eol);
        color = Color(1.0f, 0.0f, 0.0f);
        DrawCharAndAdjustPos(ch, font, size, pos, color);
    }
}

void CText::StringToUTFCharList(const std::string &text, std::vector<UTF8Char> &chars)
{
    unsigned int index = 0;
    unsigned int totalLength = text.length();
    while (index < totalLength)
    {
        UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        index += len;

        chars.push_back(ch);
    }
}

void CText::StringToUTFCharList(const std::string &text, std::vector<UTF8Char> &chars,
                                std::vector<FontMetaChar>::iterator format,
                                std::vector<FontMetaChar>::iterator end)
{
    unsigned int index = 0;
    unsigned int totalLength = text.length();
    while (index < totalLength)
    {
        UTF8Char ch;

        FontType font = FONT_COLOBOT;
        if (format + index != end)
            font = static_cast<FontType>(*(format + index) & FONT_MASK_FONT);

        int len;

        if(font == FONT_BUTTON)
        {
            len = 1;
        }
        else
        {
            len = StrUtils::Utf8CharSizeAt(text, index);
        }

        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        index += len;

        chars.push_back(ch);
    }
}

void CText::DrawString(const std::string &text, FontType font,
                       float size, Math::IntPoint pos, int width, int eol, Color color)
{
    assert(font != FONT_BUTTON);

    m_engine->SetState(ENG_RSTATE_TEXT);

    std::vector<UTF8Char> chars;
    StringToUTFCharList(text, chars);
    for (auto it = chars.begin(); it != chars.end(); ++it)
    {
        DrawCharAndAdjustPos(*it, font, size, pos, color);
    }
}

void CText::DrawHighlight(FontMetaChar hl, Math::IntPoint pos, Math::IntPoint size)
{
    // Gradient colors
    Color grad[4];

    // TODO: switch to alpha factors

    if ((hl & FONT_MASK_LINK) != 0)
    {
        grad[0] = grad[1] = grad[2] = grad[3] = Color(0.0f, 0.0f, 1.0f, 0.5f);
    }
    else if ((hl & FONT_MASK_HIGHLIGHT) == FONT_HIGHLIGHT_KEY)
    {
        grad[0] = grad[1] = grad[2] = grad[3] =
            Color(192.0f / 256.0f, 192.0f / 256.0f, 192.0f / 256.0f, 0.5f);
    }
    else
    {
        return;
    }

    Math::IntPoint vsize = m_engine->GetWindowSize();
    float h = 0.0f;
    if (vsize.y <= 768.0f)    // 1024x768 or less?
        h = 1.01f;  // 1 pixel
    else                      // more than 1024x768?
        h = 2.0f;   // 2 pixels

    Math::Point p1, p2;
    p1.x = pos.x;
    p1.y = pos.y - size.y;
    p2.x = pos.x + size.x;
    p2.y = pos.y;

    if ((hl & FONT_MASK_LINK) != 0)
    {
        p1.y = pos.y - h;  // just emphasized
    }

    m_device->SetTextureEnabled(0, false);

    VertexCol quad[] =
    {
        VertexCol(Math::Vector(p1.x, p2.y, 0.0f), grad[3]),
        VertexCol(Math::Vector(p1.x, p1.y, 0.0f), grad[0]),
        VertexCol(Math::Vector(p2.x, p2.y, 0.0f), grad[2]),
        VertexCol(Math::Vector(p2.x, p1.y, 0.0f), grad[1])
    };

    m_engine->SetWindowCoordinates();
    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, quad, 4);
    m_engine->SetInterfaceCoordinates();
    m_engine->AddStatisticTriangle(2);

    m_device->SetTextureEnabled(0, true);
}

void CText::DrawCharAndAdjustPos(UTF8Char ch, FontType font, float size, Math::IntPoint &pos, Color color)
{
    if (font == FONT_BUTTON)
    {
        Math::IntPoint windowSize = m_engine->GetWindowSize();
        int height = GetHeightInt(FONT_COLOBOT, size);
        int width = height * (static_cast<float>(windowSize.y)/windowSize.x);

        Math::IntPoint p1(pos.x, pos.y - height);
        Math::IntPoint p2(pos.x + width, pos.y);

        Math::Vector n(0.0f, 0.0f, -1.0f);  // normal

        // For whatever reason ch.c1 is a SIGNED char, we need to fix that
        unsigned char icon = static_cast<unsigned char>(ch.c1);
        if ( icon >= 128 )
        {
            icon -= 128;
            m_engine->SetTexture("textures/interface/button3.png");
            m_engine->SetState(ENG_RSTATE_TTEXTURE_WHITE);
        }
        else if ( icon >= 64 )
        {
            icon -= 64;
            m_engine->SetTexture("textures/interface/button2.png");
            m_engine->SetState(ENG_RSTATE_TTEXTURE_WHITE);
        }
        else
        {
            m_engine->SetTexture("textures/interface/button1.png");
            m_engine->SetState(ENG_RSTATE_TTEXTURE_WHITE);
        }

        Math::Point uv1, uv2;
        uv1.x = (32.0f / 256.0f) * (icon%8);
        uv1.y = (32.0f / 256.0f) * (icon/8);
        uv2.x = (32.0f / 256.0f) + uv1.x;
        uv2.y = (32.0f / 256.0f) + uv1.y;

        float dp = 0.5f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        Vertex quad[4] =
        {
            Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x, uv2.y)),
            Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x, uv1.y)),
            Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x, uv2.y)),
            Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x, uv1.y))
        };

        m_engine->SetWindowCoordinates();
        m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, quad, 4, color);
        m_engine->SetInterfaceCoordinates();
        m_engine->AddStatisticTriangle(2);

        pos.x += width;

        // Don't forget to restore the state!
        m_engine->SetState(ENG_RSTATE_TEXT);
    }
    else
    {
        int width = 1;
        if (ch.c1 > 0 && ch.c1 < 32)
        {
            if (ch.c1 == '\t')
            {
                color = Color(1.0f, 0.0f, 0.0f, 1.0f);
                width = m_tabSize;
            }

            ch = TranslateSpecialChar(ch.c1);
        }

        CharTexture tex = GetCharTexture(ch, font, size);

        Math::Point p1(pos.x, pos.y - tex.charSize.y);
        Math::Point p2(pos.x + tex.charSize.x, pos.y);

        const float halfPixelMargin = 0.5f;
        Math::Point texCoord1(static_cast<float>(tex.charPos.x + halfPixelMargin) / FONT_TEXTURE_SIZE.x,
                              static_cast<float>(tex.charPos.y + halfPixelMargin) / FONT_TEXTURE_SIZE.y);
        Math::Point texCoord2(static_cast<float>(tex.charPos.x + tex.charSize.x - halfPixelMargin) / FONT_TEXTURE_SIZE.x,
                              static_cast<float>(tex.charPos.y + tex.charSize.y - halfPixelMargin) / FONT_TEXTURE_SIZE.y);
        Math::Vector n(0.0f, 0.0f, -1.0f);  // normal

        Vertex quad[4] =
        {
            Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(texCoord1.x, texCoord2.y)),
            Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(texCoord1.x, texCoord1.y)),
            Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(texCoord2.x, texCoord2.y)),
            Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(texCoord2.x, texCoord1.y))
        };

        m_device->SetTexture(0, tex.id);
        m_engine->SetWindowCoordinates();
        m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, quad, 4, color);
        m_engine->SetInterfaceCoordinates();
        m_engine->AddStatisticTriangle(2);

        pos.x += tex.charSize.x * width;
    }
}

CachedFont* CText::GetOrOpenFont(FontType font, float size)
{
    Math::IntPoint windowSize = m_engine->GetWindowSize();
    int pointSize = static_cast<int>(size * (windowSize.Length() / REFERENCE_SIZE.Length()));

    if (m_lastCachedFont != nullptr &&
        m_lastFontType == font &&
        m_lastFontSize == pointSize)
    {
        return m_lastCachedFont;
    }

    auto it = m_fonts.find(font);
    if (it == m_fonts.end())
    {
        m_error = std::string("Invalid font type ") + StrUtils::ToString<int>(static_cast<int>(font));
        return nullptr;
    }

    MultisizeFont* mf = it->second.get();

    auto jt = mf->fonts.find(pointSize);
    if (jt != mf->fonts.end())
    {
        m_lastCachedFont = jt->second.get();
        m_lastFontType = font;
        m_lastFontSize = pointSize;
        return m_lastCachedFont;
    }

    auto file = CResourceManager::GetSDLMemoryHandler(mf->fileName);
    if (!file->IsOpen())
    {
        m_error = std::string("Unable to open file '") + mf->fileName + "' (font size = " + StrUtils::ToString<float>(size) + ")";
        return nullptr;
    }
    GetLogger()->Debug("Loaded font file %s (font size = %.1f)\n", mf->fileName.c_str(), size);

    auto newFont = MakeUnique<CachedFont>(std::move(file), pointSize);
    if (newFont->font == nullptr)
    {
        m_error = std::string("TTF_OpenFont error ") + std::string(TTF_GetError());
        return nullptr;
    }

    m_lastCachedFont = newFont.get();
    mf->fonts[pointSize] = std::move(newFont);
    return m_lastCachedFont;
}

CharTexture CText::GetCharTexture(UTF8Char ch, FontType font, float size)
{
    CachedFont* cf = GetOrOpenFont(font, size);

    if (cf == nullptr)
        return CharTexture();

    auto it = cf->cache.find(ch);
    CharTexture tex;
    if (it != cf->cache.end())
    {
        tex = (*it).second;
    }
    else
    {
        tex = CreateCharTexture(ch, cf);

        if (tex.id == 0) // invalid
            return CharTexture();

        cf->cache[ch] = tex;
    }
    return tex;
}

Math::IntPoint CText::GetFontTextureSize()
{
    return FONT_TEXTURE_SIZE;
}

CharTexture CText::CreateCharTexture(UTF8Char ch, CachedFont* font)
{
    CharTexture texture;

    SDL_Surface* textSurface = nullptr;
    SDL_Color white = {255, 255, 255, 0};
    char str[] = { ch.c1, ch.c2, ch.c3, '\0' };
    textSurface = TTF_RenderUTF8_Blended(font->font, str, white);

    if (textSurface == nullptr)
    {
        m_error = "TTF_Render error";
        return texture;
    }

    const int pixelMargin = 1;
    Math::IntPoint tileSize(Math::Max(16, Math::NextPowerOfTwo(textSurface->w)) + pixelMargin,
                            Math::Max(16, Math::NextPowerOfTwo(textSurface->h)) + pixelMargin);

    FontTexture* fontTexture = GetOrCreateFontTexture(tileSize);

    if (fontTexture == nullptr)
    {
        m_error = "Texture create error";
    }
    else
    {
        texture.id = fontTexture->id;
        texture.charPos = GetNextTilePos(*fontTexture);
        texture.charSize = Math::IntPoint(textSurface->w, textSurface->h);

        ImageData imageData;
        imageData.surface = textSurface;

        Texture tex;
        tex.id = texture.id;
        m_device->UpdateTexture(tex, texture.charPos, &imageData, TEX_IMG_RGBA);

        imageData.surface = nullptr;

        --fontTexture->freeSlots;
    }

    SDL_FreeSurface(textSurface);

    return texture;
}

FontTexture* CText::GetOrCreateFontTexture(Math::IntPoint tileSize)
{
    for (auto& fontTexture : m_fontTextures)
    {
       if (fontTexture.tileSize == tileSize && fontTexture.freeSlots > 0)
           return &fontTexture;
    }

    FontTexture newFontTexture = CreateFontTexture(tileSize);
    if (newFontTexture.id == 0)
    {
        return nullptr;
    }

    m_fontTextures.push_back(newFontTexture);
    return &m_fontTextures.back();
}

FontTexture CText::CreateFontTexture(Math::IntPoint tileSize)
{
    SDL_Surface* textureSurface = SDL_CreateRGBSurface(0, FONT_TEXTURE_SIZE.x, FONT_TEXTURE_SIZE.y, 32,
                                                       0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    ImageData data;
    data.surface = textureSurface;

    TextureCreateParams createParams;
    createParams.format = TEX_IMG_RGBA;
    createParams.filter = TEX_FILTER_NEAREST;
    createParams.mipmap = false;

    Texture tex = m_device->CreateTexture(&data, createParams);

    data.surface = nullptr;
    SDL_FreeSurface(textureSurface);

    FontTexture fontTexture;
    fontTexture.id = tex.id;
    fontTexture.tileSize = tileSize;
    int horizontalTiles = FONT_TEXTURE_SIZE.x / tileSize.x;
    int verticalTiles = FONT_TEXTURE_SIZE.y / tileSize.y;
    fontTexture.freeSlots = horizontalTiles * verticalTiles;
    return fontTexture;
}

Math::IntPoint CText::GetNextTilePos(const FontTexture& fontTexture)
{
    int horizontalTiles = FONT_TEXTURE_SIZE.x / fontTexture.tileSize.x;
    int verticalTiles = FONT_TEXTURE_SIZE.y / fontTexture.tileSize.y;

    int totalTiles = horizontalTiles * verticalTiles;
    int tileNumber = totalTiles - fontTexture.freeSlots;

    int verticalTileIndex = tileNumber / horizontalTiles;
    int horizontalTileIndex = tileNumber % horizontalTiles;

    return Math::IntPoint(horizontalTileIndex * fontTexture.tileSize.x,
                          verticalTileIndex * fontTexture.tileSize.y);
}

} // namespace Gfx
