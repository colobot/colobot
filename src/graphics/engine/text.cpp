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
                          Math::Point pos, float width, Gfx::JustifyType justify, float size,
                          float stretch, int eol)
{
    // TODO
}

void Gfx::CText::DrawText(const std::string &text, Gfx::FontType font,
                          Math::Point pos, float width, Gfx::JustifyType justify, float size,
                          float stretch, int eol)
{
    // TODO
}

void Gfx::CText::SizeText(const std::string &text, const std::vector<FontMetaChar> &format,
                          Math::Point pos, Gfx::JustifyType justify, float size,
                          Math::Point &start, Math::Point &end)
{
    // TODO
}

void Gfx::CText::SizeText(const std::string &text, Gfx::FontType font,
                          Math::Point pos, Gfx::JustifyType justify, float size,
                          Math::Point &start, Math::Point &end)
{
    // TODO
}

float Gfx::CText::GetAscent(Gfx::FontType font, float size)
{
    // TODO
    return 0.0f;
}

float Gfx::CText::GetDescent(Gfx::FontType font, float size)
{
    // TODO
    return 0.0f;
}

float Gfx::CText::GetHeight(Gfx::FontType font, float size)
{
    // TODO
    return 0.0f;
}


float Gfx::CText::GetStringWidth(const std::string &text,
                                 const std::vector<FontMetaChar> &format, float size)
{
    // TODO
    return 0.0f;
}

float Gfx::CText::GetStringWidth(const std::string &text, Gfx::FontType font, float size)
{
    // TODO
    return 0.0f;
}

float Gfx::CText::GetCharWidth(int character, Gfx::FontType font, float size, float offset)
{
    // TODO
    return 0.0f;
}


int Gfx::CText::Justify(const std::string &text, const std::vector<FontMetaChar> &format,
                        float size, float width)
{
    // TODO
    return 0;
}

int Gfx::CText::Justify(const std::string &text, Gfx::FontType font, float size, float width)
{
    // TODO
    return 0;
}

int Gfx::CText::Detect(const std::string &text, const std::vector<FontMetaChar> &format,
                       float size, float offset)
{
    // TODO
    return 0;
}

int Gfx::CText::Detect(const std::string &text, Gfx::FontType font, float size, float offset)
{
    // TODO
    return 0;
}

void Gfx::CText::DrawString(const std::string &text, const std::vector<FontMetaChar> &format,
                           float size, Math::Point pos, float width, int eol)
{
    // TODO
}

void Gfx::CText::DrawString(const std::string &text, Gfx::FontType font,
                            float size, Math::Point pos, float width, int eol)
{
    m_device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, true);
    m_device->SetTextureEnabled(0, true);

    m_device->SetRenderState(Gfx::RENDER_STATE_BLENDING, true);
    m_device->SetBlendFunc(Gfx::BLEND_SRC_ALPHA, Gfx::BLEND_INV_SRC_ALPHA);

    unsigned int index = 0;
    Math::Point screenPos = pos;
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

        DrawChar(ch, font, size, screenPos);
    }
}

void Gfx::CText::DrawColor(int color, float size, Math::Point pos, float width)
{
    // TODO !!!
    /*
    float       h, u1, u2, v1, v2, dp;
    int         icon;

    int icon = -1;
    switch (color)
    {
        case Gfx::FONT_COLOR_LINK:
            icon = 9;
            break;
        case Gfx::FONT_COLOR_TOKEN:
            icon = 4;
            break;
        case Gfx::FONT_COLOR_TYPE:
            icon = 5;
            break;
    }
    icon = -1;
    if ( color == COLOR_LINK  )  icon =  9;  // blue
    if ( color == COLOR_TOKEN )  icon =  4;  // orange
    if ( color == COLOR_TYPE  )  icon =  5;  // green
    if ( color == COLOR_CONST )  icon =  8;  // red
    if ( color == COLOR_REM   )  icon =  6;  // magenta
    if ( color == COLOR_KEY   )  icon = 10;  // gray

    if ( icon == -1 )  return;

    if ( color == COLOR_LINK )
    {
        m_engine->SetState(D3DSTATENORMAL);
    }

    Math::IntSize vsize = m_engine->GetViewportSize();
    if (vsize.h <= 768.0f)  // 1024x768 or less?
        h = 1.01f / dim.y;  // 1 pixel
    else    // more than 1024x768?
        h = 2.0f / dim.y;  // 2 pixels

    Math::Point p1, p2;
    p1.x = pos.x;
    p2.x = pos.x + width;

    if (color == Gfx::FONT_COLOR_LINK)
    {
        p1.y = pos.y;
        p2.y = pos.y + h;  // just emphasized
    }
    else
    {
        p1.y = pos.y;
        p2.y = pos.y + (16.0f/256.0f)*(size/20.0f);
    }

    u1 = (16.0f/256.0f)*(icon%16);
    v1 = (240.0f/256.0f);
    u2 = (16.0f/256.0f)+u1;
    v2 = (16.0f/256.0f)+v1;

    dp = 0.5f/256.0f;
    u1 += dp;
    v1 += dp;
    u2 -= dp;
    v2 -= dp;

    Math::Vector n(0.0f, 0.0f, -1.0f);  // normal

    Gfx::Vertex quad[] =
    {
        Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2)),
        Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1)),
        Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2)),
        Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1)),
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);
    m_engine->AddStatisticTriangle(2);

    if (color == Gfx::FONT_COLOR_LINK)
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);*/
}

void Gfx::CText::DrawChar(UTF8Char character, Gfx::FontType font, float size, Math::Point &pos)
{
    CachedFont* cf = GetOrOpenFont(font, size);

    if (cf == nullptr)
        return;

    auto it = cf->cache.find(character);
    CharTexture tex;
    if (it != cf->cache.end())
    {
        tex = (*it).second;
    }
    else
    {
        char str[] = { character.c1, character.c2, character.c3, '\0' };
        tex = CreateCharTexture(str, cf);

        if (tex.id == 0) // invalid
            return;

        cf->cache[character] = tex;
    }

    Math::Vector n(0.0f, 0.0f, -1.0f);  // normal

    Gfx::Vertex quad[4] =
    {
        Gfx::Vertex(Math::Vector(pos.x, pos.y + tex.charSize.h, 0.0f),
                    n, Math::Point(0.0f, 0.0f)),
        Gfx::Vertex(Math::Vector(pos.x, pos.y + tex.charSize.h - tex.texSize.h, 0.0f),
                    n, Math::Point(0.0f, 1.0f)),
        Gfx::Vertex(Math::Vector(pos.x + tex.texSize.w, pos.y + tex.charSize.h, 0.0f),
                    n, Math::Point(1.0f, 0.0f)),
        Gfx::Vertex(Math::Vector(pos.x + tex.texSize.w, pos.y + tex.charSize.h - tex.texSize.h, 0.0f),
                    n, Math::Point(1.0f, 1.0f))
    };

    m_device->SetTexture(0, tex.id);
    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);

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

Gfx::CharTexture Gfx::CText::CreateCharTexture(const char* str, Gfx::CachedFont* font)
{
    CharTexture texture;

    SDL_Surface* textSurface = nullptr;
    SDL_Color white = {255, 255, 255, 0};
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
