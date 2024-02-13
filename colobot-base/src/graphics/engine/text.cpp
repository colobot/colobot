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


#include "graphics/engine/text.h"

#include "app/app.h"

#include "common/font_loader.h"
#include "common/image.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "graphics/core/device.h"
#include "graphics/core/renderers.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"

#include "math/func.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <utility>

// Graphics module namespace
namespace Gfx
{

/**
 * \struct MultisizeFont
 * \brief Font with multiple possible sizes
 */
struct MultisizeFont
{
    std::filesystem::path fileName;
    std::map<int, std::unique_ptr<CachedFont>> fonts;

    explicit MultisizeFont(const std::filesystem::path &fn)
        : fileName(fn) {}
};

/**
 * \struct FontTexture
 * \brief Single texture filled with character textures
 */
struct FontTexture
{
    unsigned int id = 0;
    glm::ivec2 tileSize;
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

    CachedFont(CachedFont&& other) noexcept
        : fontFile{std::move(other.fontFile)},
          font{std::exchange(other.font, nullptr)},
          cache{std::move(other.cache)}
    {
    }

    CachedFont& operator=(CachedFont&& other) noexcept
    {
        fontFile = std::move(other.fontFile);
        std::swap(font, other.font);
        cache = std::move(other.cache);
        return *this;
    }

    CachedFont(const CachedFont& other) = delete;
    CachedFont& operator=(const CachedFont& other) = delete;

    ~CachedFont()
    {
        if (font != nullptr)
            TTF_CloseFont(font);
    }
};

std::string ToString(FontType type)
{
    switch (type)
    {
        case FontType::FONT_COMMON:         return "FontCommon";
        case FontType::FONT_COMMON_BOLD:    return "FontCommonBold";
        case FontType::FONT_COMMON_ITALIC:  return "FontCommonItalic";
        case FontType::FONT_STUDIO:         return "FontStudio";
        case FontType::FONT_STUDIO_BOLD:    return "FontStudioBold";
        case FontType::FONT_STUDIO_ITALIC:  return "FontStudioItalic";
        case FontType::FONT_SATCOM:         return "FontSatCom";
        case FontType::FONT_SATCOM_BOLD:    return "FontSatComBold";
        case FontType::FONT_SATCOM_ITALIC:  return "FontSatComItalic";
        case FontType::FONT_BUTTON:         return "FontButton";
        default: throw std::invalid_argument("Unsupported value for Gfx::FontType -> std::string conversion: " + std::to_string(type));
    }
}

namespace
{
constexpr glm::ivec2 REFERENCE_SIZE(800, 600);
constexpr glm::ivec2 FONT_TEXTURE_SIZE(256, 256);

Gfx::FontType ToBoldFontType(Gfx::FontType type)
{
    return static_cast<Gfx::FontType>(type | FONT_BOLD);
}

Gfx::FontType ToItalicFontType(Gfx::FontType type)
{
    return static_cast<Gfx::FontType>(type | FONT_ITALIC);
}
} // anonymous namespace

/// The QuadBatch is responsible for collecting as many quad (aka rectangle) draws as possible and
/// sending them to the CDevice in one big batch. This avoids making one CDevice::DrawPrimitive call
/// for every CText::DrawCharAndAdjustPos call, which makes text rendering much faster.
/// Currently we only collect textured quads (ie. ones using Vertex), not untextured quads (which
/// use VertexCol). Untextured quads are only drawn via DrawHighlight, which happens much less often
/// than drawing textured quads.
class CText::CQuadBatch
{
public:
    explicit CQuadBatch(CEngine& engine)
        : m_engine(engine)
    {
        m_quads.reserve(1024);
    }

    /// Add a quad to be rendered.
    /// This may trigger a call to Flush() if necessary.
    void Add(Vertex2D vertices[4], unsigned int texID, TransparencyMode transparency, Color color)
    {
        if (texID != m_texID || transparency != m_transparency || color != m_color)
        {
            Flush();
            m_texID = texID;
            m_transparency = transparency;
            m_color = color;
        }
        m_quads.emplace_back(Quad{{vertices[0], vertices[1], vertices[2], vertices[3]}});
    }

    /// Draw all pending quads immediately.
    void Flush()
    {
        if (m_quads.empty()) return;

        auto renderer = m_engine.GetUIRenderer();
        renderer->SetTexture(Texture{ m_texID });
        renderer->SetTransparency(m_transparency);
        renderer->SetColor(m_color);

        if (m_counts.size() < m_quads.size())
        {
            m_counts.resize(m_quads.size(), 4);
        }

        auto vertices = renderer->BeginPrimitives(PrimitiveType::TRIANGLE_STRIP, m_quads.size(), m_counts.data());

        size_t offset = 0;

        for (const auto& quad : m_quads)
        {
            std::copy_n(quad.vertices, 4, vertices + offset);
            offset += 4;
        }

        renderer->EndPrimitive();

        m_engine.AddStatisticTriangle(static_cast<int>(m_quads.size() * 2));
        m_quads.clear();
    }
private:
    CEngine& m_engine;

    struct Quad { Vertex2D vertices[4]; };
    std::vector<Quad> m_quads;
    std::vector<int> m_counts;

    Color m_color;
    unsigned int m_texID{};
    TransparencyMode m_transparency = TransparencyMode::NONE;
};

class FontsCache
{
public:
    using Fonts = std::map<FontType, std::unique_ptr<MultisizeFont>>;

    FontsCache()
    {
        ClearLastCachedFont();
    }

    bool Reload(const CFontLoader& fontLoader, int pointSize)
    {
        Flush();
        if (!PrepareCache(fontLoader)) { Flush(); return false; }
        if (!LoadDefaultFonts(fontLoader, pointSize)) { Flush(); return false; }
        return true;
    }

    CachedFont* GetOrOpenFont(FontType type, int pointSize)
    {
        if (IsLastCachedFont(type, pointSize))
            return m_lastCachedFont;

        auto multisizeFontIt = m_fonts.find(type);
        if (multisizeFontIt == m_fonts.end())
        {
            m_error = std::string("Font type not found in cache: ") + ToString(type);
            return nullptr;
        }
        MultisizeFont* multisizeFont = multisizeFontIt->second.get();

        auto cachedFontIt = multisizeFont->fonts.find(pointSize);
        if (cachedFontIt != multisizeFont->fonts.end())
        {
            auto* cachedFont = cachedFontIt->second.get();
            SaveLastCachedFont(cachedFont, type, pointSize);
            return m_lastCachedFont;
        }

        auto newFont = LoadFont(multisizeFont, pointSize);
        if (!newFont) return nullptr;

        SaveLastCachedFont(newFont.get(), type, pointSize);
        multisizeFont->fonts[pointSize] = std::move(newFont);
        return m_lastCachedFont;
    }

    void Flush()
    {
        Clear();
        ClearLastCachedFont();
    }

    ~FontsCache()
    {
        Flush();
    }

    std::string GetError() const
    {
        return m_error;
    }

private:
    bool PrepareCache(const CFontLoader& fontLoader)
    {
        for (auto type : {FONT_COMMON, FONT_STUDIO, FONT_SATCOM})
        {
            if (!PrepareCacheForFontType(type, fontLoader)) return false;
            if (!PrepareCacheForFontType(ToBoldFontType(type), fontLoader)) return false;
            if (!PrepareCacheForFontType(ToItalicFontType(type), fontLoader)) return false;
        }
        return true;
    }

    bool PrepareCacheForFontType(Gfx::FontType type, const CFontLoader& fontLoader)
    {
        if (auto font = fontLoader.GetFont(type))
        {
            m_fonts[type] = std::make_unique<MultisizeFont>(font);
            return true;
        }
        m_error = "Error on loading fonts: font type " + ToString(type) + " is not configured";
        return false;
    }

    bool LoadDefaultFonts(const CFontLoader& fontLoader, int pointSize)
    {
        for (auto& font : m_fonts)
        {
            auto type = font.first;
            auto* cachedFont = GetOrOpenFont(type, pointSize);
            if (cachedFont == nullptr || cachedFont->font == nullptr)
                return false;
        }
        return true;
    }

    std::unique_ptr<CachedFont> LoadFont(MultisizeFont* multisizeFont, int pointSize)
    {
        auto file = CResourceManager::GetSDLMemoryHandler(multisizeFont->fileName);
        if (!file->IsOpen())
        {
            m_error = "Unable to open file '" + StrUtils::ToString(multisizeFont->fileName) + "' (font size = " + std::to_string(pointSize) + ")";
            return nullptr;
        }
        GetLogger()->Debug("Loaded font file %% (font size = %%)", multisizeFont->fileName, pointSize);
        auto newFont = std::make_unique<CachedFont>(std::move(file), pointSize);
        if (newFont->font == nullptr)
        {
            m_error = std::string("TTF_OpenFont error ") + std::string(TTF_GetError());
            return nullptr;
        }
        return newFont;
    }

    void SaveLastCachedFont(CachedFont* font, FontType type, int pointSize)
    {
        m_lastCachedFont = font;
        m_lastFontType = type;
        m_lastFontSize = pointSize;
    }

    bool IsLastCachedFont(FontType font, int pointSize) const
    {
        return
            m_lastCachedFont != nullptr &&
            m_lastFontType == font &&
            m_lastFontSize == pointSize;
    }

    void Clear()
    {
        for (auto& [fontType, multisizeFont] : m_fonts)
        {
            for (auto& cachedFont : multisizeFont->fonts)
            {
                cachedFont.second->cache.clear();
            }
        }
        m_fonts.clear();
    }

    void ClearLastCachedFont()
    {
        m_lastCachedFont = nullptr;
        m_lastFontType = FONT_COMMON;
        m_lastFontSize = 0;
    }

private:
    Fonts m_fonts;

    CachedFont* m_lastCachedFont;
    FontType m_lastFontType;
    int m_lastFontSize;

    std::string m_error;
};


CText::CText(CEngine* engine)
{
    m_device = nullptr;
    m_engine = engine;

    m_defaultSize = 12.0f;
    m_tabSize = 4;

    m_fontsCache = std::make_unique<FontsCache>();

    m_quadBatch = std::make_unique<CQuadBatch>(*engine);
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

    if (!ReloadFonts())
    {
        return false;
    }

    return true;
}

bool CText::ReloadFonts()
{
    CFontLoader fontLoader;
    if (!fontLoader.Init())
    {
        m_error = "Error on parsing fonts config file: failed to open file";
        return false;
    }

    auto newCache = std::make_unique<FontsCache>();
    if (!newCache->Reload(fontLoader, GetFontPointSize(m_defaultSize)))
    {
        m_error = newCache->GetError();
        return false;
    }

    m_fontsCache = std::move(newCache);
    return true;
}

void CText::Destroy()
{
    m_fontsCache->Flush();
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

    m_fontsCache->Flush();
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
                     float size, glm::vec2 pos, float width, TextAlign align,
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

    glm::ivec2 intPos = m_engine->InterfaceToWindowCoords(pos);
    int intWidth = width * m_engine->GetWindowSize().x;
    DrawString(text, format, end, size, intPos, intWidth, eol, color);
}

void CText::DrawText(const std::string &text, FontType font,
                     float size, glm::vec2 pos, float width, TextAlign align,
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

    glm::ivec2 intPos = m_engine->InterfaceToWindowCoords(pos);
    int intWidth = width * m_engine->GetWindowSize().x;
    DrawString(text, font, size, intPos, intWidth, eol, color);
}

void CText::SizeText(const std::string &text, std::vector<FontMetaChar>::iterator format,
                     std::vector<FontMetaChar>::iterator endFormat,
                     float size, glm::vec2 pos, TextAlign align,
                     glm::vec2 &start, glm::vec2 &end)
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

    start.y -= GetDescent(FONT_COMMON, size);
    end.y   += GetAscent(FONT_COMMON, size);
}

void CText::SizeText(const std::string &text, FontType font,
                     float size, glm::vec2 pos, TextAlign align,
                     glm::vec2 &start, glm::vec2 &end)
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
    glm::ivec2 wndSize = { 0, TTF_FontAscent(cf->font) };
    glm::vec2 ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.y;
}

float CText::GetDescent(FontType font, float size)
{
    assert(font != FONT_BUTTON);

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    glm::ivec2 wndSize = { 0, TTF_FontDescent(cf->font) };
    glm::vec2 ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.y;
}

float CText::GetHeight(FontType font, float size)
{
    assert(font != FONT_BUTTON);

    CachedFont* cf = GetOrOpenFont(font, size);
    assert(cf != nullptr);
    glm::ivec2 wndSize = { 0, TTF_FontHeight(cf->font) };
    glm::vec2 ifSize = m_engine->WindowToInterfaceSize(wndSize);
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
        FontType font = FONT_COMMON;
        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        UTF8Char ch;

        int len = GetCharSizeAt(font, std::string_view(text.data() + index, text.size() - index));
        if (len >= 1)
            ch.c1 = text[index];
        if (len >= 2)
            ch.c2 = text[index+1];
        if (len >= 3)
            ch.c3 = text[index+2];

        width += GetCharWidth(ch, font, size, width);

        index += len;
        fmtIndex += len;
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
    glm::ivec2 wndSize{};
    TTF_SizeUTF8(cf->font, text.c_str(), &wndSize.x, &wndSize.y);
    glm::vec2 ifSize = m_engine->WindowToInterfaceSize(wndSize);
    return ifSize.x;
}

float CText::GetCharWidth(UTF8Char ch, FontType font, float size, float offset)
{
    if (font == FONT_BUTTON)
    {
        glm::ivec2 windowSize = m_engine->GetWindowSize();
        float height = GetHeight(FONT_COMMON, size);
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

    glm::vec2 charSize;
    auto it = cf->cache.find(ch);
    if (it != cf->cache.end())
    {
        charSize = m_engine->WindowToInterfaceSize((*it).second.charSize);
    }
    else
    {
        glm::ivec2 wndSize{};
        std::array<char, 4> text = {
            static_cast<char>(ch.c1),
            static_cast<char>(ch.c2),
            static_cast<char>(ch.c3),
            '\0'
        };

        TTF_SizeUTF8(cf->font, text.data(), &wndSize.x, &wndSize.y);
        charSize = m_engine->WindowToInterfaceSize(wndSize);
    }

    return charSize.x * width;
}

int CText::GetCharWidthInt(UTF8Char ch, FontType font, float size, float offset)
{
    if (font == FONT_BUTTON)
    {
        glm::ivec2 windowSize = m_engine->GetWindowSize();
        int height = GetHeightInt(FONT_COMMON, size);
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

    glm::ivec2 charSize{ 0, 0 };
    auto it = cf->cache.find(ch);
    if (it != cf->cache.end())
    {
        charSize = (*it).second.charSize;
    }
    else
    {
        std::array<char, 4> text = {
            static_cast<char>(ch.c1),
            static_cast<char>(ch.c2),
            static_cast<char>(ch.c3),
            '\0'
        };

        TTF_SizeUTF8(cf->font, text.data(), &charSize.x, &charSize.y);
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
        FontType font = FONT_COMMON;
        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        UTF8Char ch;

        int len = GetCharSizeAt(font, std::string_view(text.data() + index, text.size() - index));
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
        fmtIndex += len;
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

        int len = GetCharSizeAt(font, std::string_view(text.data() + index, text.size() - index));
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
        FontType font = FONT_COMMON;

        if (format + fmtIndex != end)
            font = static_cast<FontType>(*(format + fmtIndex) & FONT_MASK_FONT);

        UTF8Char ch;

        int len = GetCharSizeAt(font, std::string_view(text.data() + index, text.size() - index));
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
        fmtIndex += len;
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

        int len = GetCharSizeAt(font, std::string_view(text.data() + index, text.size() - index));
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
                       float size, const glm::ivec2& position, int width, int eol, Color color)
{
    m_engine->SetWindowCoordinates();

    glm::ivec2 pos = position;

    int start = pos.x;

    unsigned int fmtIndex = 0;

    std::vector<UTF8Char> chars;
    StringToUTFCharList(text, chars, format, end);
    for (auto it = chars.begin(); it != chars.end(); ++it)
    {
        FontType font = FONT_COMMON;
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
            glm::ivec2 charSize{};
            charSize.x = GetCharWidthInt(ch, font, size, offset);
            charSize.y = GetHeightInt(font, size);
            // NB. for quad batching to improve highlight drawing performance, this code would have
            // to be rearranged to draw all highlights before any characters are drawn.
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
        FontType font = FONT_COMMON;
        UTF8Char ch = TranslateSpecialChar(eol);
        color = Color(1.0f, 0.0f, 0.0f);
        DrawCharAndAdjustPos(ch, font, size, pos, color);
    }
    m_quadBatch->Flush();
    m_engine->SetInterfaceCoordinates();
}

void CText::StringToUTFCharList(std::string_view text, std::vector<UTF8Char> &chars)
{
    while (!text.empty())
    {
        UTF8Char ch;

        int len = StrUtils::UTF8CharLength(text);

        if (len == 0) break;

        if (len >= 1)
            ch.c1 = text[0];
        if (len >= 2)
            ch.c2 = text[1];
        if (len >= 3)
            ch.c3 = text[2];

        chars.push_back(ch);

        text.remove_prefix(len);
    }
}

void CText::StringToUTFCharList(std::string_view text, std::vector<UTF8Char> &chars,
                                std::vector<FontMetaChar>::iterator format,
                                std::vector<FontMetaChar>::iterator end)
{
    while (!text.empty())
    {
        UTF8Char ch;

        FontType font = FONT_COMMON;
        if (format != end)
            font = static_cast<FontType>(*format & FONT_MASK_FONT);

        int len = GetCharSizeAt(font, text);

        if (len >= 1)
            ch.c1 = text[0];
        if (len >= 2)
            ch.c2 = text[1];
        if (len >= 3)
            ch.c3 = text[2];

        format += len;

        chars.push_back(ch);

        text.remove_prefix(len);
    }
}

int CText::GetCharSizeAt(Gfx::FontType font, std::string_view text) const
{
    int len = 0;
    if (font == FONT_BUTTON)
    {
        len = 1;
    }
    else
    {
        len = StrUtils::UTF8CharLength(text);

        if (len == 0) len = 1;
    }
    return len;
}

void CText::DrawString(const std::string &text, FontType font,
                       float size, const glm::ivec2& position, int width, int eol, Color color)
{
    assert(font != FONT_BUTTON);

    glm::ivec2 pos = position;

    std::vector<UTF8Char> chars;
    StringToUTFCharList(text, chars);
    m_engine->SetWindowCoordinates();
    for (auto it = chars.begin(); it != chars.end(); ++it)
    {
        DrawCharAndAdjustPos(*it, font, size, pos, color);
    }
    m_quadBatch->Flush();
    m_engine->SetInterfaceCoordinates();
}

void CText::DrawHighlight(FontMetaChar hl, const glm::ivec2& pos, const glm::ivec2& size)
{
    // Gradient colors
    glm::u8vec4 grad[4];

    // TODO: switch to alpha factors

    if ((hl & FONT_MASK_LINK) != 0)
    {
        grad[0] = grad[1] = grad[2] = grad[3] = { 0, 0, 255, 128 };
    }
    else if ((hl & FONT_MASK_HIGHLIGHT) == FONT_HIGHLIGHT_KEY)
    {
        grad[0] = grad[1] = grad[2] = grad[3] = { 192, 192, 192, 128 };
    }
    else
    {
        return;
    }

    m_quadBatch->Flush();

    glm::ivec2 vsize = m_engine->GetWindowSize();
    float h = 0.0f;
    if (vsize.y <= 768.0f)    // 1024x768 or less?
        h = 1.01f;  // 1 pixel
    else                      // more than 1024x768?
        h = 2.0f;   // 2 pixels

    glm::vec2 p1, p2;
    p1.x = pos.x;
    p1.y = pos.y - size.y;
    p2.x = pos.x + size.x;
    p2.y = pos.y;

    if ((hl & FONT_MASK_LINK) != 0)
    {
        p1.y = pos.y - h;  // just emphasized
    }

    auto renderer = m_device->GetUIRenderer();
    renderer->SetTexture(Texture{});
    renderer->SetTransparency(TransparencyMode::ALPHA);
    auto vertices = renderer->BeginPrimitive(PrimitiveType::TRIANGLE_STRIP, 4);

    vertices[0] = { { p1.x, p2.y }, {}, grad[3] };
    vertices[1] = { { p1.x, p1.y }, {}, grad[0] };
    vertices[2] = { { p2.x, p2.y }, {}, grad[2] };
    vertices[3] = { { p2.x, p1.y }, {}, grad[1] };

    renderer->EndPrimitive();
    m_engine->AddStatisticTriangle(2);
}

void CText::DrawCharAndAdjustPos(UTF8Char ch, FontType font, float size, glm::ivec2&pos, Color color)
{
    if (font == FONT_BUTTON)
    {
        glm::ivec2 windowSize = m_engine->GetWindowSize();
        int height = GetHeightInt(FONT_COMMON, size);
        int width = height;// * (static_cast<float>(windowSize.y)/windowSize.x);

        glm::ivec2 p1(pos.x, pos.y - height);
        glm::ivec2 p2(pos.x + width, pos.y);

        // For whatever reason ch.c1 is a SIGNED char, we need to fix that
        const unsigned icon = static_cast<unsigned>(ch.c1) & 0xFF;

        const unsigned texIndex = 1 + icon / 64;
        const unsigned iconIndex = icon % 64;

        // TODO: A bit of code duplication, see CControl::SetButtonTextureForIcon()
        const unsigned int texID = m_engine->LoadTexture(
            "textures/interface/button" + StrUtils::ToString<int>(texIndex) + ".png").id;

        glm::vec2 uv1, uv2;
        uv1.x = (32.0f / 256.0f) * (iconIndex % 8);
        uv1.y = (32.0f / 256.0f) * (iconIndex / 8);
        uv2.x = (32.0f / 256.0f) + uv1.x;
        uv2.y = (32.0f / 256.0f) + uv1.y;

        float dp = 0.5f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        Gfx::IntColor col = Gfx::ColorToIntColor(color);

        Gfx::Vertex2D vertices[4];

        vertices[0] = { { p1.x, p2.y }, { uv1.x, uv2.y } };
        vertices[1] = { { p1.x, p1.y }, { uv1.x, uv1.y } };
        vertices[2] = { { p2.x, p2.y }, { uv2.x, uv2.y } };
        vertices[3] = { { p2.x, p1.y }, { uv2.x, uv1.y } };

        m_quadBatch->Add(vertices, texID, TransparencyMode::NONE, color);

        pos.x += width;
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

        glm::vec2 p1(pos.x, pos.y - tex.charSize.y);
        glm::vec2 p2(pos.x + tex.charSize.x, pos.y);

        const float halfPixelMargin = 0.5f;
        glm::vec2 texCoord1(static_cast<float>(tex.charPos.x + halfPixelMargin) / FONT_TEXTURE_SIZE.x,
                            static_cast<float>(tex.charPos.y + halfPixelMargin) / FONT_TEXTURE_SIZE.y);
        glm::vec2 texCoord2(static_cast<float>(tex.charPos.x + tex.charSize.x - halfPixelMargin) / FONT_TEXTURE_SIZE.x,
                            static_cast<float>(tex.charPos.y + tex.charSize.y - halfPixelMargin) / FONT_TEXTURE_SIZE.y);

        Gfx::IntColor col = Gfx::ColorToIntColor(color);

        Gfx::Vertex2D vertices[4];

        vertices[0] = { { p1.x, p2.y }, { texCoord1.x, texCoord2.y }, col };
        vertices[1] = { { p1.x, p1.y }, { texCoord1.x, texCoord1.y }, col };
        vertices[2] = { { p2.x, p2.y }, { texCoord2.x, texCoord2.y }, col };
        vertices[3] = { { p2.x, p1.y }, { texCoord2.x, texCoord1.y }, col };

        m_quadBatch->Add(vertices, tex.id, TransparencyMode::ALPHA, color);

        pos.x += tex.charSize.x * width;
    }
}

int CText::GetFontPointSize(float size) const
{
    glm::ivec2 windowSize = m_engine->GetWindowSize();
    return static_cast<int>(size * (glm::length(glm::vec2(windowSize)) / glm::length(glm::vec2(REFERENCE_SIZE))));
}

CachedFont* CText::GetOrOpenFont(FontType type, float size)
{
    auto* cachedFont = m_fontsCache->GetOrOpenFont(type, GetFontPointSize(size));
    if (!cachedFont)
    {
        m_error = m_fontsCache->GetError();
        return nullptr;
    }
    return cachedFont;
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

glm::ivec2 CText::GetFontTextureSize()
{
    return FONT_TEXTURE_SIZE;
}

CharTexture CText::CreateCharTexture(UTF8Char ch, CachedFont* font)
{
    CharTexture texture;

    SDL_Surface* textSurface = nullptr;
    SDL_Color white = {255, 255, 255, 0};
    std::array<char, 4> str = {
        static_cast<char>(ch.c1),
        static_cast<char>(ch.c2),
        static_cast<char>(ch.c3),
        '\0'
    };

    textSurface = TTF_RenderUTF8_Blended(font->font, str.data(), white);

    if (textSurface == nullptr)
    {
        m_error = "TTF_Render error";
        return texture;
    }

    const int pixelMargin = 1;
    glm::ivec2 tileSize(Math::Max(16, Math::NextPowerOfTwo(textSurface->w)) + pixelMargin,
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
        texture.charSize = { textSurface->w, textSurface->h };

        ImageData imageData;
        imageData.surface = textSurface;

        Texture tex;
        tex.id = texture.id;
        m_device->UpdateTexture(tex, texture.charPos, &imageData, TextureFormat::RGBA);

        imageData.surface = nullptr;

        --fontTexture->freeSlots;
    }

    SDL_FreeSurface(textSurface);

    return texture;
}

FontTexture* CText::GetOrCreateFontTexture(const glm::ivec2& tileSize)
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

FontTexture CText::CreateFontTexture(const glm::ivec2& tileSize)
{
    SDL_Surface* textureSurface = SDL_CreateRGBSurface(0, FONT_TEXTURE_SIZE.x, FONT_TEXTURE_SIZE.y, 32,
                                                       0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    ImageData data;
    data.surface = textureSurface;

    TextureCreateParams createParams;
    createParams.format = TextureFormat::RGBA;
    createParams.filter = TextureFilter::NEAREST;
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

glm::ivec2 CText::GetNextTilePos(const FontTexture& fontTexture)
{
    int horizontalTiles = FONT_TEXTURE_SIZE.x / std::max(1, fontTexture.tileSize.x); //this should prevent crashes in some combinations of resolution and font size, see issue #1128
    int verticalTiles = FONT_TEXTURE_SIZE.y / std::max(1, fontTexture.tileSize.y);

    int totalTiles = horizontalTiles * verticalTiles;
    int tileNumber = totalTiles - fontTexture.freeSlots;

    int verticalTileIndex = tileNumber / std::max(1, horizontalTiles);
    int horizontalTileIndex = tileNumber % horizontalTiles;

    return { horizontalTileIndex * fontTexture.tileSize.x,
             verticalTileIndex * fontTexture.tileSize.y };
}

} // namespace Gfx
