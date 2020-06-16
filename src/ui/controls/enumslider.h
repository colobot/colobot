/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "ui/controls/slider.h"

#include <vector>
#include <map>

namespace Ui
{

class CEnumSlider : public CSlider
{
public:
    CEnumSlider();

    void SetPossibleValues(const std::vector<float>& values);
    void SetPossibleValues(const std::map<float, std::string>& values);

    void SetVisibleValue(float value);
    float GetVisibleValue();
    unsigned int GetVisibleValueIndex();

protected:
    std::string GetLabel() override;

protected:
    std::vector<float> m_values;
    std::vector<std::string> m_labels;
};

}
