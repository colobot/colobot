#pragma once

#include <math\point.h>
#include <math\vector.h>
#include <graphics\engine\particle.h>

namespace UI {

	class ParticlesGenerator {

	public:
		void generateMouseParticles(Math::Point& mousePosition, bool buttonPressed);
	private:
		Math::Point CreateRandomDimensionsForMouseParticles(bool buttonPressed);
		Math::Vector CreateRandomSpeedForMouseParticles();
		float CreateRandomDurationForMouseParticles();
	};
}