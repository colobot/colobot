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


#include "ui/controls/image.h"

#include "common/event.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "graphics/core/renderers.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"


namespace Ui
{
// Object's constructor.

CImage::CImage() : CControl()
{
}

// Object's destructor.

CImage::~CImage()
{
    if (!m_filename.empty())
    {
        m_engine->DeleteTexture(m_filename);
    }
}


// Creates a new button.

bool CImage::Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);

    if ( icon == -1 )
    {
        std::string name = GetResourceName(eventType);
        SetName(name);
    }

    return true;
}


// Specifies the name of the image display.

void CImage::SetFilename(const std::filesystem::path& name)
{
    if (!m_filename.empty())
    {
        m_engine->DeleteTexture(m_filename);
    }

    m_filename = name;
}


// Management of an event.

bool CImage::EventProcess(const Event &event)
{
    return true;
}


// Draws button.

void CImage::Draw()
{
    glm::vec2   uv1,uv2, corner, pos, dim;
    float       dp;

    auto renderer = m_engine->GetUIRenderer();

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    dp = 0.5f/256.0f;

    if ( m_icon == 0 )  // hollow frame?
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 160.0f / 256.0f;
        uv1.y = 192.0f / 256.0f;  // u-v texture
        uv2.x = 192.0f / 256.0f;
        uv2.y = 224.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 10.0f / 640.0f;
        corner.y = 10.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);
    }

    if ( !m_filename.empty() )  // displays an image?
    {
        Gfx::TextureCreateParams params;
        params.format = Gfx::TextureFormat::AUTO;
        params.filter = Gfx::TextureFilter::BILINEAR;
        params.padToNearestPowerOfTwo = true;
        Gfx::Texture tex = m_engine->LoadTexture(StrUtils::ToString(m_filename), params);
        renderer->SetTexture(tex);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        pos = m_pos;
        dim = m_dim;
        pos.x +=  5.0f / 640.0f;
        pos.y +=  5.0f / 480.0f;
        dim.x -= 10.0f / 640.0f;
        dim.y -= 10.0f / 480.0f;
        uv1.x = 0.0f;
        uv1.y = 0.0f;
        uv2.x = static_cast<float>(tex.originalSize.x) / static_cast<float>(tex.size.x);
        uv2.y = static_cast<float>(tex.originalSize.y) / static_cast<float>(tex.size.y);
        DrawIcon(pos, dim, uv1, uv2);
    }
}


}
