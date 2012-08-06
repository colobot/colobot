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

// text.cpp

#include "graphics/engine/text.h"

#include "app/app.h"
#include "common/image.h"
#include "common/iman.h"
#include "common/logger.h"
#include "common/stringutils.h"
#include "math/func.h"

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>


namespace Gfx
{

/**
  \struct CachedFont
  \brief Base TTF font with UTF-8 char cache */
struct CachedFont
{
    TTF_Font* font;
    std::map<Gfx::UTF8Char, Gfx::CharTexture> cache;

    CachedFont() : font(nullptr) {}
};

};



Gfx::CText::CText(CInstanceManager *iMan, Gfx::CEngine* engine)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_TEXT, this);

    m_device = nullptr;
    m_engine = engine;

    m_defaultSize = 12.0f;
    m_fontPath = "fonts";

    m_lastFontType = Gfx::FONT_COLOBOT;
    m_lastFontSize = 0;
    m_lastCachedFont = nullptr;
}

Gfx::CText::~CText()
{
    m_iMan->DeleteInstance(CLASS_TEXT, this);

    m_iMan = nullptr;
    m_device = nullptr;
    m_engine = nullptr;
}

bool Gfx::CText::Create()
{
    if (TTF_Init() != 0)
    {
        m_error = std::string("TTF_Init error: ") + std::string(TTF_GetError());
        return false;
    }

    m_fonts[Gfx::FONT_COLOBOT]        = new MultisizeFont("dvu_sans.ttf");
    m_fonts[Gfx::FONT_COLOBOT_BOLD]   = new MultisizeFont("dvu_sans_bold.ttf");
    m_fonts[Gfx::FONT_COLOBOT_ITALIC] = new MultisizeFont("dvu_sans_italic.ttf");

    m_fonts[Gfx::FONT_COURIER]        = new MultisizeFont("dvu_sans_mono.ttf");
    m_fonts[Gfx::FONT_COURIER_BOLD]   = new MultisizeFont("dvu_sans_mono_bold.ttf");

    for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it)
    {
        Gfx::FontType type = (*it).first;
        CachedFont* cf = GetOrOpenFont(type, m_defaultSize);
        if (cf == nullptr || cf->font == nullptr)
            return false;
    }

    return true;
}

void Gfx::CText::Destroy()
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

void Gfx::CText::SetDevice(Gfx::CDevice* device)
{
    m_device = device;
}

std::string Gfx::CText::GetError()
{
    return m_error;
}

void Gfx::CText::FlushCache()
{
    for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it)
    {
        MultisizeFont *mf = (*it).second;
        for (auto jt = mf->fonts.begin(); jt != mf->fonts.end(); ++jt)
        {
            CachedFont *f = (*jt).second;
            f->cache.clear();
        }
    }
}

void Gfx::CText::DrawText(const std::string &text, const std::vector<FontMetaChar> &format,
                          float size, Math::Point pos, float width, Gfx::TextAlign align,
                          int eol)
{
    float sw = 0.0f;

    if (align == Gfx::TEXT_ALIGN_CENTER)
    {
        sw = GetStringWidth(text, format, size);
        if (sw > width) sw = width;
        pos.x -= sw / 2.0f;
    }
    else if (align == Gfx::TEXT_ALIGN_RIGHT)
    {
        sw = GetStringWidth(text, format, size);
        if (sw > width) sw = width;
        pos.x -= sw;
    }

    DrawString(text, format, size, pos, width, eol);
}

void Gfx::CText::DrawText(const std::string &text, Gfx::FontType font,
                          float size, Math::Point pos, float width, Gfx::TextAlign align,
                          int eol)
{
    float sw = 0.0f;

    if (align == Gfx::TEXT_ALIGN_CENTER)
    {
        sw = GetStringWidth(text, font, size);
        if (sw > width) sw = width;
        pos.x -= sw / 2.0f;
    }
    else if (align == Gfx::TEXT_ALIGN_RIGHT)
    {
        sw = GetStringWidth(text, font, size);
        if (sw > width) sw = width;
        pos.x -= sw;
    }

    DrawString(text, font, size, pos, width, eol);
}

void Gfx::CText::SizeText(const std::string &text, const std::vector<FontMetaChar> &format,
                          float size, Math::Point pos, Gfx::TextAlign align,
                          Math::Point &start, Math::Point &end)
{
    start = end = pos;

    float sw = GetStringWidth(text, format, size);
    end.x += sw;
    if (align == Gfx::TEXT_ALIGN_CENTER)
    {
        start.x -= sw/2.0f;
        end.x   -= sw/2.0f;
    }
    else if (align == Gfx::TEXT_ALIGN_RIGHT)
    {
        start.x -= sw;
        end.x   -= sw;
    }

    start.y -= GetDescent(Gfx::FONT_COLOBOT, size);
    end.y   += GetAscent(Gfx::FONT_COLOBOT, size);
}

void Gfx::CText::SizeText(const std::string &text, Gfx::FontType font,
                          float size, Math::Point pos, Gfx::TextAlign align,
                          Math::Point &start, Math::Point &end)
{
    start = end = pos;

    float sw = GetStringWidth(text, font, size);
    end.x += sw;
    if (align == Gfx::TEXT_ALIGN_CENTER)
    {
        start.x -= sw/2.0f;
        end.x   -= sw/2.0f;
    }
    else if (align == Gfx::TEXT_ALIGN_RIGHT)
    {
        start.x -= sw;
        end.x   -= sw;
    }

    start.y -= GetDescent(font, size);
    end.y   += GetAscent(font, size);
}

float Gfx::CText::GetAscent(Gfx::FontType font, float size)
{
    assert(font != Gfx::FONT_BUTTON);

    Gfx::CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntSize wndSize;
    wndSize.h = TTF_FontAscent(cf->font);
    Math::Size ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.h;
}

float Gfx::CText::GetDescent(Gfx::FontType font, float size)
{
    assert(font != Gfx::FONT_BUTTON);

    Gfx::CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntSize wndSize;
    wndSize.h = TTF_FontDescent(cf->font);
    Math::Size ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.h;
}

float Gfx::CText::GetHeight(Gfx::FontType font, float size)
{
    assert(font != Gfx::FONT_BUTTON);

    Gfx::CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntSize wndSize;
    wndSize.h = TTF_FontHeight(cf->font);
    Math::Size ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.h;
}


float Gfx::CText::GetStringWidth(const std::string &text,
                                 const std::vector<FontMetaChar> &format, float size)
{
    assert(StrUtils::Utf8StringLength(text) == format.size());

    float width = 0.0f;
    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        Gfx::FontType font = static_cast<Gfx::FontType>(format[fmtIndex] & Gfx::FONT_MASK_FONT);

        Gfx::UTF8Char ch;

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

float Gfx::CText::GetStringWidth(const std::string &text, Gfx::FontType font, float size)
{
    assert(font != Gfx::FONT_BUTTON);

    // TODO: special chars?

    Gfx::CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    Math::IntSize wndSize;
    TTF_SizeUTF8(cf->font, text.c_str(), &wndSize.w, &wndSize.h);
    Math::Size ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.w;
}

float Gfx::CText::GetCharWidth(Gfx::UTF8Char ch, Gfx::FontType font, float size, float offset)
{
    // TODO: if (font == Gfx::FONT_BUTTON)
    if (font == Gfx::FONT_BUTTON) return 0.0f;

    // TODO: special chars?
    // TODO: tab sizing

    Gfx::CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);

    Gfx::CharTexture tex;
    auto it = cf->cache.find(ch);
    if (it != cf->cache.end())
        tex = (*it).second;
    else
        tex = CreateCharTexture(ch, cf);

    return tex.charSize.w;
}


int Gfx::CText::Justify(const std::string &text, const std::vector<FontMetaChar> &format,
                        float size, float width)
{
    assert(StrUtils::Utf8StringLength(text) == format.size());

    float pos = 0.0f;
    int cut = 0;
    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        Gfx::FontType font = static_cast<Gfx::FontType>(format[fmtIndex] & Gfx::FONT_MASK_FONT);

        Gfx::UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        if (font != Gfx::FONT_BUTTON)
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

int Gfx::CText::Justify(const std::string &text, Gfx::FontType font, float size, float width)
{
    assert(font != Gfx::FONT_BUTTON);

    float pos = 0.0f;
    int cut = 0;
    unsigned int index = 0;
    while (index < text.length())
    {
        Gfx::UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        index += len;

        if (ch.c1 == '\n')
            return index+1;

        if (ch.c1 == ' ' )
            cut = index+1;

        pos += GetCharWidth(ch, font, size, pos);
        if (pos > width)
        {
            if (cut == 0) return index;
            else          return cut;
        }
    }

    return index;
}

int Gfx::CText::Detect(const std::string &text, const std::vector<FontMetaChar> &format,
                       float size, float offset)
{
    assert(StrUtils::Utf8StringLength(text) == format.size());

    float pos = 0.0f;
    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        Gfx::FontType font = static_cast<Gfx::FontType>(format[fmtIndex] & Gfx::FONT_MASK_FONT);

        // TODO: if (font == Gfx::FONT_BUTTON)
        if (font == Gfx::FONT_BUTTON) continue;

        Gfx::UTF8Char ch;

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

int Gfx::CText::Detect(const std::string &text, Gfx::FontType font, float size, float offset)
{
    assert(font != Gfx::FONT_BUTTON);

    float pos = 0.0f;
    unsigned int index = 0;
    while (index < text.length())
    {
        Gfx::UTF8Char ch;

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

void Gfx::CText::DrawString(const std::string &text, const std::vector<FontMetaChar> &format,
                           float size, Math::Point pos, float width, int eol)
{
    assert(StrUtils::Utf8StringLength(text) == format.size());

    m_engine->SetState(Gfx::ENG_RSTATE_TEXT);

    Gfx::FontType font = Gfx::FONT_COLOBOT;
    float start = pos.x;

    unsigned int index = 0;
    unsigned int fmtIndex = 0;
    while (index < text.length())
    {
        font = static_cast<Gfx::FontType>(format[fmtIndex] & Gfx::FONT_MASK_FONT);

        // TODO: if (font == Gfx::FONT_BUTTON)
        if (font == Gfx::FONT_BUTTON) continue;

        Gfx::UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        float offset = pos.x - start;
        float cw = GetCharWidth(ch, font, size, offset);
        if (offset + cw > width)  // exceeds the maximum width?
        {
            // TODO: special end-of-line char
            break;
        }

        Gfx::FontHighlight hl = static_cast<Gfx::FontHighlight>(format[fmtIndex] & Gfx::FONT_MASK_HIGHLIGHT);
        if (hl != Gfx::FONT_HIGHLIGHT_NONE)
        {
            Math::Size charSize;
            charSize.w = GetCharWidth(ch, font, size, offset);
            charSize.h = GetHeight(font, size);
            DrawHighlight(hl, pos, charSize);
        }

        DrawChar(ch, font, size, pos);

        index += len;
        fmtIndex++;
    }

    // TODO: eol
}

void Gfx::CText::DrawString(const std::string &text, Gfx::FontType font,
                            float size, Math::Point pos, float width, int eol)
{
    assert(font != Gfx::FONT_BUTTON);

    m_engine->SetState(Gfx::ENG_RSTATE_TEXT);

    unsigned int index = 0;
    while (index < text.length())
    {
        Gfx::UTF8Char ch;

        int len = StrUtils::Utf8CharSizeAt(text, index);
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        index += len;

        DrawChar(ch, font, size, pos);
    }
}

void Gfx::CText::DrawHighlight(Gfx::FontHighlight hl, Math::Point pos, Math::Size size)
{
    // Gradient colors
    Gfx::Color grad[4];

    // TODO: switch to alpha factors

    switch (hl)
    {
        case Gfx::FONT_HIGHLIGHT_LINK:
            grad[0] = grad[1] = grad[2] = grad[3] = Gfx::Color(0.0f, 0.0f, 1.0f, 0.5f);
            break;

        case Gfx::FONT_HIGHLIGHT_TOKEN:
            grad[0] = grad[1] = Gfx::Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Gfx::Color(248.0f / 256.0f, 220.0f / 256.0f, 188.0f / 256.0f, 0.5f);
            break;

        case Gfx::FONT_HIGHLIGHT_TYPE:
            grad[0] = grad[1] = Gfx::Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Gfx::Color(169.0f / 256.0f, 234.0f / 256.0f, 169.0f / 256.0f, 0.5f);
            break;

        case Gfx::FONT_HIGHLIGHT_CONST:
            grad[0] = grad[1] = Gfx::Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Gfx::Color(248.0f / 256.0f, 176.0f / 256.0f, 169.0f / 256.0f, 0.5f);
            break;

        case Gfx::FONT_HIGHLIGHT_REM:
            grad[0] = grad[1] = Gfx::Color(248.0f / 256.0f, 248.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            grad[2] = grad[3] = Gfx::Color(248.0f / 256.0f, 169.0f / 256.0f, 248.0f / 256.0f, 0.5f);
            break;

        case Gfx::FONT_HIGHLIGHT_KEY:
            grad[0] = grad[1] = grad[2] = grad[3] =
                Gfx::Color(192.0f / 256.0f, 192.0f / 256.0f, 192.0f / 256.0f, 0.5f);
            break;

        default:
            return;
    }

    Math::IntSize vsize = m_engine->GetWindowSize();
    float h = 0.0f;
    if (vsize.h <= 768.0f)    // 1024x768 or less?
        h = 1.01f / vsize.h;  // 1 pixel
    else                      // more than 1024x768?
        h = 2.0f / vsize.h;   // 2 pixels

    Math::Point p1, p2;
    p1.x = pos.x;
    p2.x = pos.x + size.w;

    if (hl == Gfx::FONT_HIGHLIGHT_LINK)
    {
        p1.y = pos.y;
        p2.y = pos.y + h;  // just emphasized
    }
    else
    {
        p1.y = pos.y;
        p2.y = pos.y + size.h;
    }

    m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, false);

    Gfx::VertexCol quad[] =
    {
        Gfx::VertexCol(Math::Vector(p1.x, p1.y, 0.0f), grad[3]),
        Gfx::VertexCol(Math::Vector(p2.x, p1.y, 0.0f), grad[2]),
        Gfx::VertexCol(Math::Vector(p1.x, p2.y, 0.0f), grad[0]),
        Gfx::VertexCol(Math::Vector(p2.x, p2.y, 0.0f), grad[1])
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);
    m_engine->AddStatisticTriangle(2);

    m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, true);
}

void Gfx::CText::DrawChar(Gfx::UTF8Char ch, Gfx::FontType font, float size, Math::Point &pos)
{
    // TODO: if (font == Gfx::FONT_BUTTON)
    if (font == Gfx::FONT_BUTTON) return;

    // TODO: special chars?

    CachedFont* cf = GetOrOpenFont(font, size);

    if (cf == nullptr)
        return;

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

    m_device->SetRenderState(Gfx::RENDER_STATE_CULLING, false);

    Math::Point p1(pos.x, pos.y + tex.charSize.h - tex.texSize.h);
    Math::Point p2(pos.x + tex.texSize.w, pos.y + tex.charSize.h);

    Math::Vector n(0.0f, 0.0f, -1.0f);  // normal

    Gfx::Vertex quad[4] =
    {
        Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(0.0f, 1.0f)),
        Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(1.0f, 1.0f)),
        Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(0.0f, 0.0f)),
        Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(1.0f, 0.0f))
    };

    m_device->SetTexture(0, tex.id);
    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);
    m_engine->AddStatisticTriangle(2);

    pos.x += tex.charSize.w;
}

Gfx::CachedFont* Gfx::CText::GetOrOpenFont(Gfx::FontType font, float size)
{
    // TODO: sizing
    int pointSize = static_cast<int>(size);

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

    std::string path = CApplication::GetInstance().GetDataFilePath(m_fontPath, mf->fileName);

    m_lastCachedFont = new CachedFont();
    m_lastCachedFont->font = TTF_OpenFont(path.c_str(), pointSize);
    if (m_lastCachedFont->font == nullptr)
        m_error = std::string("TTF_OpenFont error ") + std::string(TTF_GetError());

    mf->fonts[pointSize] = m_lastCachedFont;

    return m_lastCachedFont;
}

Gfx::CharTexture Gfx::CText::CreateCharTexture(Gfx::UTF8Char ch, Gfx::CachedFont* font)
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

    Gfx::TextureCreateParams createParams;
    createParams.format = Gfx::TEX_IMG_RGBA;
    createParams.minFilter = Gfx::TEX_MIN_FILTER_NEAREST;
    createParams.magFilter = Gfx::TEX_MAG_FILTER_NEAREST;
    createParams.mipmap = false;

    Gfx::Texture tex = m_device->CreateTexture(&data, createParams);

    data.surface = nullptr;

    SDL_FreeSurface(textSurface);
    SDL_FreeSurface(textureSurface);

    if (! tex.valid)
    {
        m_error = "Texture create error";
        return texture;
    }

    texture.id = tex.id;
    texture.texSize =  m_engine->WindowToInterfaceSize(Math::IntSize(textureSurface->w, textureSurface->h));
    texture.charSize = m_engine->WindowToInterfaceSize(Math::IntSize(textSurface->w, textSurface->h));

    return texture;
}
