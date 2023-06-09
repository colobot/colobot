/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_setup_graphics.h"

#include "common/config.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/settings.h"
#include "core/stringutils.h"

#include "graphics/engine/camera.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/editvalue.h"
#include "ui/controls/enumslider.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenSetupGraphics::CScreenSetupGraphics()
{
}

void CScreenSetupGraphics::SetActive()
{
    m_tab = PHASE_SETUPg;
}

void CScreenSetupGraphics::CreateInterface()
{
    CWindow*        pw;
    CEditValue*     pv;
    CLabel*         pl;
    CCheck*         pc;
    CEnumSlider*    pes;
    CButton*        pb;
    Math::Point     pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;


    pos.x = ox+sx*3;
    pos.y = 0.65f;
    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_PARTI);
    pv->SetState(STATE_SHADOW);
    pv->SetMinValue(0.0f);
    pv->SetMaxValue(2.0f);
    pos.x += 0.13f;
    pos.y -= 0.015f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_PARTI, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL10, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*3;
    pos.y = 0.59f;
    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pv = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_CLIP);
    pv->SetState(STATE_SHADOW);
    pv->SetMinValue(0.5f);
    pv->SetMaxValue(2.0f);
    pos.x += 0.13f;
    pos.y -= 0.015f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_CLIP, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL11, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    ddim.x = dim.x*6;
    ddim.y = dim.y*0.5f;
    pos.x = ox+sx*3;
    pos.y = 0.53f;
    pos.y -= 0.048f*0.5f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_DIRTY);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_FOG);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_LIGHT);
    pc->SetState(STATE_SHADOW);
    if ( m_simulationSetup )
    {
        pc->SetState(STATE_DEAD);
    }
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_PAUSE_BLUR);
    pc->SetState(STATE_SHADOW);

    pos.x = ox+sx*8.5f;
    pos.y = 0.65f;
    ddim.x = dim.x*3;
    ddim.y = dim.y*0.5f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW_SPOTS);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW_MAPPING);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SHADOW_MAPPING_QUALITY);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f*1.5f;

    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_SHADOW_MAPPING_BUFFER);
    pes->SetState(STATE_SHADOW);
    std::map<float, std::string> shadowOptions = {
        {0, "Screen buffer"}
    };
    if (m_engine->GetDevice()->IsFramebufferSupported())
    {
        const int MAX_SHADOW_TEXTURE_SIZE = 8192;
        for(int i = 128; i <= Math::Min(m_engine->GetDevice()->GetMaxTextureSize(), MAX_SHADOW_TEXTURE_SIZE); i *= 2)
            shadowOptions[i] = StrUtils::ToString<int>(i)+"x"+StrUtils::ToString<int>(i);
        pes->SetPossibleValues(shadowOptions);
    }
    else
    {
        pes->ClearState(STATE_ENABLE);
    }
    pos.y += ddim.y/2;
    pos.x += 0.005f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_SHADOW_MAPPING_BUFFER, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);


    pos.x = ox+sx*12.5f;
    pos.y = 0.63f;
    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_MSAA);
    pes->SetState(STATE_SHADOW);
    std::vector<float> msaaOptions;
    for(int i = 1; i <= m_engine->GetDevice()->GetMaxSamples(); i *= 2)
        msaaOptions.push_back(i);
    pes->SetPossibleValues(msaaOptions);
    if(m_engine->GetDevice()->GetMaxSamples() < 2)
        pes->ClearState(STATE_ENABLE);
    pos.y += ddim.y/2;
    pos.x += 0.005f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_MSAA, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*12.5f;
    pos.y = 0.56f;
    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_TEXTURE_FILTER);
    pes->SetState(STATE_SHADOW);
    pes->SetPossibleValues({
        { Gfx::TEX_FILTER_NEAREST,   "Nearest"   },
        { Gfx::TEX_FILTER_BILINEAR,  "Bilinear"  },
        { Gfx::TEX_FILTER_TRILINEAR, "Trilinear" }
    });
    pos.y += ddim.y/2;
    pos.x += 0.005f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE_FILTER, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*12.5f;
    pos.y = 0.49f;
    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_TEXTURE_MIPMAP);
    pes->SetState(STATE_SHADOW);
    pes->SetPossibleValues({1, 4, 8, 16});
    pos.y += ddim.y/2;
    pos.x += 0.005f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE_MIPMAP, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*12.5f;
    pos.y = 0.42f;
    ddim.x = dim.x*2.2f;
    ddim.y = 18.0f/480.0f;
    pes = pw->CreateEnumSlider(pos, ddim, 0, EVENT_INTERFACE_TEXTURE_ANISOTROPY);
    pes->SetState(STATE_SHADOW);
    std::vector<float> anisotropyOptions;
    for(int i = 1; i <= m_engine->GetDevice()->GetMaxAnisotropyLevel(); i *= 2)
        anisotropyOptions.push_back(i);
    pes->SetPossibleValues(anisotropyOptions);
    if(!m_engine->GetDevice()->IsAnisotropySupported())
        pes->ClearState(STATE_ENABLE);
    pos.y += ddim.y/2;
    pos.x += 0.005f;
    ddim.x = 0.40f;
    GetResource(RES_EVENT, EVENT_INTERFACE_TEXTURE_ANISOTROPY, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL12, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);


    ddim.x = dim.x*2;
    ddim.y = dim.y*1;
    pos.x = ox+sx*10;
    pos.y = oy+sy*2;

    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MIN);
    pb->SetState(STATE_SHADOW);
    pos.x += ddim.x;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NORM);
    pb->SetState(STATE_SHADOW);
    pos.x += ddim.x;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_MAX);
    pb->SetState(STATE_SHADOW);

    UpdateSetupButtons();
}

bool CScreenSetupGraphics::EventProcess(const Event &event)
{
    if (!CScreenSetup::EventProcess(event)) return false;

    switch( event.type )
    {
        case EVENT_INTERFACE_PARTI:
            ChangeSetupButtons();
            break;

        case EVENT_INTERFACE_CLIP:
            ChangeSetupButtons();
            m_engine->ApplyChange();
            break;

        case EVENT_INTERFACE_DIRTY:
            m_engine->SetDirty(!m_engine->GetDirty());
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_FOG:
            m_engine->SetFog(!m_engine->GetFog());
            m_camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_LIGHT:
            m_engine->SetLightMode(!m_engine->GetLightMode());
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_PAUSE_BLUR:
            m_engine->SetPauseBlurEnabled(!m_engine->GetPauseBlurEnabled());
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_SHADOW_SPOTS:
            m_engine->SetShadowMapping(false);
            m_engine->SetShadowMappingQuality(false);
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_SHADOW_MAPPING:
            m_engine->SetShadowMapping(true);
            m_engine->SetShadowMappingQuality(false);
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_SHADOW_MAPPING_QUALITY:
            m_engine->SetShadowMapping(true);
            m_engine->SetShadowMappingQuality(true);
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_SHADOW_MAPPING_BUFFER:
        case EVENT_INTERFACE_TEXTURE_FILTER:
        case EVENT_INTERFACE_TEXTURE_MIPMAP:
        case EVENT_INTERFACE_TEXTURE_ANISOTROPY:
        case EVENT_INTERFACE_MSAA:
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_MIN:
            ChangeSetupQuality(-1);
            UpdateSetupButtons();
            break;
        case EVENT_INTERFACE_NORM:
            ChangeSetupQuality(0);
            UpdateSetupButtons();
            break;
        case EVENT_INTERFACE_MAX:
            ChangeSetupQuality(1);
            UpdateSetupButtons();
            break;

        default:
            return true;
    }
    return false;
}

// Updates the buttons during the setup phase.

void CScreenSetupGraphics::UpdateSetupButtons()
{
    CWindow*    pw;
    CCheck*     pc;
    CEditValue* pv;
    CEnumSlider* pes;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_PARTI));
    if ( pv != nullptr )
    {
        value = m_engine->GetParticleDensity();
        pv->SetValue(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_CLIP));
    if ( pv != nullptr )
    {
        value = m_engine->GetClippingDistance();
        pv->SetValue(value);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_DIRTY));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_engine->GetDirty());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_FOG));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_engine->GetFog());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_LIGHT));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_engine->GetLightMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_PAUSE_BLUR));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_engine->GetPauseBlurEnabled());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_SPOTS));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, !m_engine->GetShadowMapping());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_ENABLE, m_engine->IsShadowMappingSupported());
        pc->SetState(STATE_CHECK, m_engine->GetShadowMapping() && !m_engine->GetShadowMappingQuality());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING_QUALITY));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_ENABLE, m_engine->IsShadowMappingQualitySupported());
        pc->SetState(STATE_CHECK, m_engine->GetShadowMapping() && m_engine->GetShadowMappingQuality());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING_BUFFER));
    if ( pes != nullptr )
    {
        pes->SetState(STATE_ENABLE, m_engine->GetShadowMapping() && m_engine->GetDevice()->IsFramebufferSupported());
        if (!m_engine->GetShadowMappingOffscreen())
        {
            pes->SetVisibleValue(0);
        }
        else
        {
            pes->SetVisibleValue(m_engine->GetShadowMappingOffscreenResolution());
        }
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_FILTER));
    if ( pes != nullptr )
    {
        pes->SetVisibleValue(m_engine->GetTextureFilterMode());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_MIPMAP));
    if ( pes != nullptr )
    {
        pes->SetState(STATE_ENABLE, m_engine->GetTextureFilterMode() == Gfx::TEX_FILTER_TRILINEAR);
        pes->SetVisibleValue(m_engine->GetTextureMipmapLevel());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_ANISOTROPY));
    if ( pes != nullptr )
    {
        pes->SetVisibleValue(m_engine->GetTextureAnisotropyLevel());
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_MSAA));
    if ( pes != nullptr )
    {
        pes->SetVisibleValue(m_engine->GetMultiSample());
    }
}

// Updates the engine function of the buttons after the setup phase.

void CScreenSetupGraphics::ChangeSetupButtons()
{
    CWindow*    pw;
    CEditValue* pv;
    CEnumSlider* pes;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_PARTI));
    if ( pv != nullptr )
    {
        value = pv->GetValue();
        m_engine->SetParticleDensity(value);
    }

    pv = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_CLIP));
    if ( pv != nullptr )
    {
        value = pv->GetValue();
        m_engine->SetClippingDistance(value);
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_FILTER));
    if ( pes != nullptr )
    {
        int valueIndex = pes->GetVisibleValueIndex();
        m_engine->SetTextureFilterMode(static_cast<Gfx::TexFilter>(valueIndex));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_MIPMAP));
    if ( pes != nullptr )
    {
        value = pes->GetVisibleValue();
        m_engine->SetTextureMipmapLevel(static_cast<int>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_TEXTURE_ANISOTROPY));
    if ( pes != nullptr )
    {
        value = pes->GetVisibleValue();
        m_engine->SetTextureAnisotropyLevel(static_cast<int>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_MSAA));
    if ( pes != nullptr )
    {
        value = pes->GetVisibleValue();
        m_engine->SetMultiSample(static_cast<int>(value));
    }

    pes = static_cast<CEnumSlider*>(pw->SearchControl(EVENT_INTERFACE_SHADOW_MAPPING_BUFFER));
    if ( pes != nullptr )
    {
        value = pes->GetVisibleValue();
        if(value == 0)
        {
            m_engine->SetShadowMappingOffscreen(false);
        }
        else
        {
            m_engine->SetShadowMappingOffscreen(true);
            m_engine->SetShadowMappingOffscreenResolution(value);
        }
    }
}


// Changes the general level of quality.

void CScreenSetupGraphics::ChangeSetupQuality(int quality)
{
    bool    bEnable;
    float   value;

    bEnable = true; //(quality >= 0);
    m_engine->SetDirty(bEnable);
    m_engine->SetFog(bEnable);
    m_engine->SetLightMode(bEnable);
    m_camera->SetOverBaseColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f)); // TODO: color ok?

    if ( quality <  0 )  value = 0.0f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 2.0f;
    m_engine->SetParticleDensity(value);

    if ( quality <  0 )  value = 0.5f;
    if ( quality == 0 )  value = 1.0f;
    if ( quality >  0 )  value = 2.0f;
    m_engine->SetClippingDistance(value);

    if ( quality <  0 ) m_engine->SetMultiSample(1);
    if ( quality == 0 ) m_engine->SetMultiSample(2);
    if ( quality >  0 ) m_engine->SetMultiSample(4);

    if ( quality <  0 ) m_engine->SetTextureAnisotropyLevel(1);
    if ( quality == 0 ) m_engine->SetTextureAnisotropyLevel(2);
    if ( quality >  0 ) m_engine->SetTextureAnisotropyLevel(8);

    if ( quality <  0 ) { m_engine->SetTextureFilterMode(Gfx::TEX_FILTER_BILINEAR); }
    if ( quality == 0 ) { m_engine->SetTextureFilterMode(Gfx::TEX_FILTER_TRILINEAR); m_engine->SetTextureMipmapLevel(4); m_engine->SetTextureAnisotropyLevel(4); }
    if ( quality >  0 ) { m_engine->SetTextureFilterMode(Gfx::TEX_FILTER_TRILINEAR); m_engine->SetTextureMipmapLevel(8); m_engine->SetTextureAnisotropyLevel(8); }

    if ( quality <  0 ) { m_engine->SetShadowMapping(false); m_engine->SetShadowMappingQuality(false); }
    else { m_engine->SetShadowMapping(true); m_engine->SetShadowMappingQuality(true); m_engine->SetShadowMappingOffscreen(true); }
    if ( quality == 0 ) m_engine->SetShadowMappingOffscreenResolution(1024);
    if ( quality >  0 ) m_engine->SetShadowMappingOffscreenResolution(2048);

    // TODO: first execute adapt?
    //m_engine->FirstExecuteAdapt(false);
}

} // namespace Ui
