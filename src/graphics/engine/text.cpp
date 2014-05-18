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


#include "graphics/engine/text.h"

#include "app/app.h"
#include "app/gamedata.h"

#include "common/image.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "math/func.h"

#include <SDL.h>
#include <SDL_ttf.h>


// Graphics module namespace
namespace Gfx {


/**
 * \struct CachedFont
 * \brief Base TTF font with UTF-8 char cache
 */
struct CachedFont
{
    TTF_Font* font;
    std::map<UTF8Char, CharTexture> cache;

    CachedFont() : font(nullptr) {}
};


const Math::IntPoint REFERENCE_SIZE(800, 600);


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

    m_fonts[FONT_COLOBOT]        = new MultisizeFont("dvu_sans.ttf");
    m_fonts[FONT_COLOBOT_BOLD]   = new MultisizeFont("dvu_sans_bold.ttf");
    m_fonts[FONT_COLOBOT_ITALIC] = new MultisizeFont("dvu_sans_italic.ttf");

    m_fonts[FONT_COURIER]        = new MultisizeFont("dvu_sans_mono.ttf");
    m_fonts[FONT_COURIER_BOLD]   = new MultisizeFont("dvu_sans_mono_bold.ttf");

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
    for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it)
    {
        MultisizeFont* mf = (*it).second;

        for (auto jt = mf->fonts.begin(); jt != mf->fonts.end(); ++jt)
        {
            CachedFont* cf = (*jt).second;

            TTF_CloseFont(cf->font);

            cf->font = nullptr;
            delete cf;
        }

        mf->fonts.clear();
        delete mf;
    }

    m_fonts.clear();

    m_lastCachedFont = nullptr;

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
    for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it)
    {
        MultisizeFont *mf = (*it).second;
        for (auto jt = mf->fonts.begin(); jt != mf->fonts.end(); ++jt)
        {
            CachedFont *f = (*jt).second;
            for (auto ct = f->cache.begin(); ct != f->cache.end(); ++ct)
            {
                Texture tex;
                tex.id = (*ct).second.id;
                m_device->DestroyTexture(tex);
            }
            f->cache.clear();
        }
    }

    m_lastFontType = FONT_COLOBOT;
    m_lastFontSize = 0;
    m_lastCachedFont = nullptr;
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

    DrawString(text, format, end, size, pos, width, eol, color);
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

    DrawString(text, font, size, pos, width, eol, color);
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
    // TODO: if (font == FONT_BUTTON)
    if (font == FONT_BUTTON) return 0.0f;

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
        charSize = (*it).second.charSize;
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
            ch.c1 = 0xE2;
            ch.c2 = 0x86;
            ch.c3 = 0xB2;
            break;

        case CHAR_DOT:
            // Unicode: U+23C5
            ch.c1 = 0xE2;
            ch.c2 = 0x8F;
            ch.c3 = 0x85;
            break;

        case CHAR_SQUARE:
            // Unicode: U+25FD
            ch.c1 = 0xE2;
            ch.c2 = 0x97;
            ch.c3 = 0xBD;
            break;

        case CHAR_SKIP_RIGHT:
            // Unicode: U+25B6
            ch.c1 = 0xE2;
            ch.c2 = 0x96;
            ch.c3 = 0xB6;
            break;

        case CHAR_SKIP_LEFT:
            // Unicode: U+25C0
            ch.c1 = 0xE2;
            ch.c2 = 0x97;
            ch.c3 = 0x80;
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
                       float size, Math::Point pos, float width, int eol, Color color)
{
    m_engine->SetState(ENG_RSTATE_TEXT);

    float start = pos.x;

    unsigned int fmtIndex = 0;

    std::vector<UTF8Char> chars;
    StringToUTFCharList(text, chars);
    for (auto it = chars.begin(); it != chars.end(); ++it)
    {
        FontType font = FONT_COLOBOT;
        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        // TODO: if (font == FONT_BUTTON)
        if (font == FONT_BUTTON) continue;

        UTF8Char ch = *it;

        float offset = pos.x - start;
        float cw = GetCharWidth(ch, font, size, offset);
        if (offset + cw > width)  // exceeds the maximum width?
        {
            ch = TranslateSpecialChar(CHAR_SKIP_RIGHT);
            cw = GetCharWidth(ch, font, size, offset);
            pos.x = start + width - cw;
            color = Color(1.0f, 0.0f, 0.0f);
            DrawCharAndAdjustPos(ch, font, size, pos, color);
            break;
        }

        FontHighlight hl = static_cast<FontHighlight>(format[fmtIndex] & FONT_MASK_HIGHLIGHT);
        if (hl != FONT_HIGHLIGHT_NONE)
        {
            Math::Point charSize;
            charSize.x = GetCharWidth(ch, font, size, offset);
            charSize.y = GetHeight(font, size);
            DrawHighlight(hl, pos, charSize);
        }

        DrawCharAndAdjustPos(ch, font, size, pos, color);

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

void CText::DrawString(const std::string &text, FontType font,
                       float size, Math::Point pos, float width, int eol, Color color)
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

void CText::DrawHighlight(FontHighlight hl, Math::Point pos, Math::Point size)
{
    // Gradient colors
    Color grad[4];

    // TODO: switch to alpha factors

    switch (hl)
    {
        case FONT_HIGHLIGHT_LINK:
            grad[0] = grad[1] = grad[2] = grad[3] = Color(0.0f, 0.0f, 1.0f, 0.5f);
            break;

        case FONT_HIGHLIGHT_TOKEN:
            grad[0] = grad[1] = Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Color(248.0f / 256.0f, 220.0f / 256.0f, 188.0f / 256.0f, 0.5f);
            break;

        case FONT_HIGHLIGHT_TYPE:
            grad[0] = grad[1] = Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Color(169.0f / 256.0f, 234.0f / 256.0f, 169.0f / 256.0f, 0.5f);
            break;

        case FONT_HIGHLIGHT_CONST:
            grad[0] = grad[1] = Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Color(248.0f / 256.0f, 176.0f / 256.0f, 169.0f / 256.0f, 0.5f);
            break;

        case FONT_HIGHLIGHT_REM:
            grad[0] = grad[1] = Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Color(248.0f / 256.0f, 169.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            break;

        case FONT_HIGHLIGHT_KEY:
            grad[0] = grad[1] = grad[2] = grad[3] =
                Color(192.0f / 256.0f, 192.0f / 256.0f, 192.0f / 256.0f, 0.5f);
            break;

        default:
            return;
    }

    Math::IntPoint vsize = m_engine->GetWindowSize();
    float h = 0.0f;
    if (vsize.y <= 768.0f)    // 1024x768 or less?
        h = 1.01f / vsize.y;  // 1 pixel
    else                      // more than 1024x768?
        h = 2.0f / vsize.y;   // 2 pixels

    Math::Point p1, p2;
    p1.x = pos.x;
    p2.x = pos.x + size.x;

    if (hl == FONT_HIGHLIGHT_LINK)
    {
        p1.y = pos.y;
        p2.y = pos.y + h;  // just emphasized
    }
    else
    {
        p1.y = pos.y;
        p2.y = pos.y + size.y;
    }

    m_device->SetTextureEnabled(0, false);

    VertexCol quad[] =
    {
        VertexCol(Math::Vector(p1.x, p1.y, 0.0f), grad[3]),
        VertexCol(Math::Vector(p1.x, p2.y, 0.0f), grad[0]),
        VertexCol(Math::Vector(p2.x, p1.y, 0.0f), grad[2]),
        VertexCol(Math::Vector(p2.x, p2.y, 0.0f), grad[1])
    };

    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, quad, 4);
    m_engine->AddStatisticTriangle(2);

    m_device->SetTextureEnabled(0, true);
}

void CText::DrawCharAndAdjustPos(UTF8Char ch, FontType font, float size, Math::Point &pos, Color color)
{
    // TODO: if (font == FONT_BUTTON)
    if (font == FONT_BUTTON) return;

    CachedFont* cf = GetOrOpenFont(font, size);

    if (cf == nullptr)
        return;

    int width = 1;
    if (ch.c1 > 0 && ch.c1 < 32)
    {
        if (ch.c1 == '\t')
            width = m_tabSize;

        ch = TranslateSpecialChar(ch.c1);
    }

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
            return;

        cf->cache[ch] = tex;
    }

    Math::Point p1(pos.x, pos.y + tex.charSize.y - tex.texSize.y);
    Math::Point p2(pos.x + tex.texSize.x, pos.y + tex.charSize.y);

    Math::Vector n(0.0f, 0.0f, -1.0f);  // normal

    Vertex quad[4] =
    {
        Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(0.0f, 1.0f)),
        Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(0.0f, 0.0f)),
        Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(1.0f, 1.0f)),
        Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(1.0f, 0.0f))
    };

    m_device->SetTexture(0, tex.id);
    m_device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, quad, 4, color);
    m_engine->AddStatisticTriangle(2);

    pos.x += tex.charSize.x * width;
}

CachedFont* CText::GetOrOpenFont(FontType font, float size)
{
    Math::IntPoint windowSize = m_engine->GetWindowSize();
    int pointSize = static_cast<int>(size * (windowSize.Length() / REFERENCE_SIZE.Length()));

    if (m_lastCachedFont != nullptr)
    {
        if (m_lastFontType == font && m_lastFontSize == pointSize)
            return m_lastCachedFont;
    }

    auto it = m_fonts.find(font);
    if (it == m_fonts.end())
    {
        m_error = std::string("Invalid font type ") + StrUtils::ToString<int>(static_cast<int>(font));
        return nullptr;
    }

    MultisizeFont* mf = (*it).second;

    auto jt = mf->fonts.find(pointSize);
    if (jt != mf->fonts.end())
    {
        m_lastCachedFont = (*jt).second;
        m_lastFontType = font;
        m_lastFontSize = pointSize;
        return m_lastCachedFont;
    }

    std::string path = CGameData::GetInstancePointer()->GetFilePath(DIR_FONT, mf->fileName);

    m_lastCachedFont = new CachedFont();
    m_lastCachedFont->font = TTF_OpenFont(path.c_str(), pointSize);
    if (m_lastCachedFont->font == nullptr)
        m_error = std::string("TTF_OpenFont error ") + std::string(TTF_GetError());

    mf->fonts[pointSize] = m_lastCachedFont;

    return m_lastCachedFont;
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

    int w = Math::NextPowerOfTwo(textSurface->w);
    int h = Math::NextPowerOfTwo(textSurface->h);

    textSurface->flags = textSurface->flags & (~SDL_SRCALPHA);
    SDL_Surface* textureSurface = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00,
                                                       0x000000ff, 0xff000000);
    SDL_BlitSurface(textSurface, NULL, textureSurface, NULL);

    ImageData data;
    data.surface = textureSurface;

    TextureCreateParams createParams;
    createParams.format = TEX_IMG_RGBA;
    createParams.minFilter = TEX_MIN_FILTER_NEAREST;
    createParams.magFilter = TEX_MAG_FILTER_NEAREST;
    createParams.mipmap = false;

    Texture tex = m_device->CreateTexture(&data, createParams);

    data.surface = nullptr;

    if (! tex.Valid())
    {
        m_error = "Texture create error";
    }
    else
    {
        texture.id = tex.id;
        texture.texSize =  m_engine->WindowToInterfaceSize(Math::IntPoint(textureSurface->w, textureSurface->h));
        texture.charSize = m_engine->WindowToInterfaceSize(Math::IntPoint(textSurface->w, textSurface->h));
    }

    SDL_FreeSurface(textSurface);
    SDL_FreeSurface(textureSurface);

    return texture;
}


} // namespace Gfx

