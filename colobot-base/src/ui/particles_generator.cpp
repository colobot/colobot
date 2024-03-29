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

#include "ui/particles_generator.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"

#include "math/func.h"

namespace UI
{
    CParticlesGenerator::CParticlesGenerator()
    {
        m_particleManager = Gfx::CEngine::GetInstancePointer()->GetParticle();
    }

    void CParticlesGenerator::GenerateMouseParticles(const glm::vec2& mousePosition, bool buttonPressed)
    {
        Gfx::ParticleType particleType = buttonPressed ? Gfx::ParticleType::PARTISCRAPS : Gfx::ParticleType::PARTILENS1;
        m_particleManager->CreateParticle(glm::vec3(mousePosition.x, mousePosition.y, 0), CreateRandomSpeedForMouseParticles(),
            CreateRandomDimensionsForMouseParticles(buttonPressed), particleType,
            CreateRandomDurationForMouseParticles(), 2.0f, 0.0f, Gfx::SH_INTERFACE);
    }

    glm::vec2 CParticlesGenerator::CreateRandomDimensionsForMouseParticles(bool buttonPressed)
    {
        float dimensionX = buttonPressed ? (0.005f + Math::Rand() * 0.005f) : (0.01f + Math::Rand() * 0.01f);
        return { dimensionX, dimensionX / 0.75f };
    }

    glm::vec3 CParticlesGenerator::CreateRandomSpeedForMouseParticles()
    {
        return glm::vec3((Math::Rand() - 0.5f) * 0.5f, 0.3f + Math::Rand() * 0.3f, 0.0f);
    }

    float CParticlesGenerator::CreateRandomDurationForMouseParticles()
    {
        return Math::Rand() * 0.5f + 0.5f;
    }

}
