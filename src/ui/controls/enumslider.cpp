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

#include "ui/controls/enumslider.h"

#include "core/stringutils.h"

namespace Ui
{

CEnumSlider::CEnumSlider() : CSlider()
{
}

void CEnumSlider::SetPossibleValues(const std::vector<float>& values)
{
    m_values = values;
}

void CEnumSlider::SetPossibleValues(const std::map<float, std::string>& values)
{
    m_values.clear();
    m_labels.clear();
    for (auto it = values.begin(); it != values.end(); ++it)
    {
        m_values.push_back(it->first);
        m_labels.push_back(it->second);
    }
}

void CEnumSlider::SetVisibleValue(float value)
{
    for (unsigned int i = 0; i < m_values.size(); i++)
    {
        if (value == m_values[i])
        {
            m_visibleValue = static_cast<float>(i) / (m_values.size()-1);
        }
    }
}

unsigned int CEnumSlider::GetVisibleValueIndex()
{
    return round(m_visibleValue * (m_values.size()-1));
}

float CEnumSlider::GetVisibleValue()
{
    return m_values[GetVisibleValueIndex()];
}

std::string CEnumSlider::GetLabel()
{
    unsigned int value = GetVisibleValueIndex();
    if (value < m_labels.size())
    {
        return m_labels.at(value);
    }
    return StrUtils::ToString<int>(GetVisibleValue());
}

}
