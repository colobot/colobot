#include "ui/particlesGenerator.h"
#include <graphics\engine\particle.h>

namespace UI {

	void ParticlesGenerator::generateMouseParticles(Math::Point& mousePosition, bool buttonPressed)
	{
		Gfx::ParticleType particleType = buttonPressed ? Gfx::ParticleType::PARTISCRAPS : Gfx::ParticleType::PARTILENS1;
		Gfx::CEngine::GetInstancePointer()->GetParticle()->CreateParticle(Math::Vector(mousePosition.x, mousePosition.y, 0), CreateRandomSpeedForMouseParticles(),
			CreateRandomDimensionsForMouseParticles(buttonPressed), particleType,
			CreateRandomDurationForMouseParticles(), 2.0f, 0.0f, Gfx::SH_INTERFACE);
	}

	Math::Point ParticlesGenerator::CreateRandomDimensionsForMouseParticles(bool buttonPressed) {
		if (buttonPressed) {
			float dimensionX = 0.005f + Math::Rand() * 0.005f;
			return Math::Point(dimensionX, dimensionX / 0.75f);
		}
		float dimensionX = 0.01f + Math::Rand() * 0.01f;
		return Math::Point(dimensionX, dimensionX / 0.75f);
	}

	Math::Vector ParticlesGenerator::CreateRandomSpeedForMouseParticles() {
		return Math::Vector((Math::Rand() - 0.5f) * 0.5f, 0.3f + Math::Rand() * 0.3f, 0.0f);
	}

	float ParticlesGenerator::CreateRandomDurationForMouseParticles() {
		return Math::Rand() * 0.5f + 0.5f;
	}

}